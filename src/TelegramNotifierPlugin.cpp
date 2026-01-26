#include "TelegramNotifierPlugin.hpp"
#include <Logger/Log.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <vector>
#include "Metrics.hpp"
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>

void TelegramNotifierPlugin::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("TelegramNotifierPlugin " + std::string(TelegramNotifierPlugin_VERSION))
        .addOption(configPath, "TelegramNotifierPath", "path to config for TelegramNotifier.json");
}
void TelegramNotifierPlugin::postInit()
{
    if (token.empty()) return;
    pusher = std::make_unique<d3156::EasyHttpClient>(*io, "https://api.telegram.org");
    pusher->setBasePath("/bot" + token + "/sendMessage");
    pusher->setContentType("application/json");
}

std::string formatAlertMessage(const std::string &tmpl, Metrics::Metric *metric)
{
    std::string msg = tmpl;
    size_t pos      = msg.find("{metric}");
    if (pos != std::string::npos) msg.replace(pos, 8, metric->name);
    pos = msg.find("{value}");
    if (pos != std::string::npos) msg.replace(pos, 7, std::to_string(metric->value_));
    pos = msg.find("{tags}");
    if (pos != std::string::npos) {
        std::string tags;
        for (const auto &t : metric->tags) {
            if (!tags.empty()) tags += ",";
            tags += t.first + "=" + t.second;
        }
        msg.replace(pos, 6, tags);
    }
    return msg;
}

void TelegramNotifierPlugin::upload(std::set<Metrics::Metric *> &statistics)
{
    if (!pusher) return;
    std::vector<std::string> alerts;
    if (alerts_count.size() == 0)
        for (auto metric : statistics) alerts_count[metric] = 0;
    for (auto metric : statistics) {
        auto notifier = notifiers.find(metric->name);
        if (notifier == notifiers.end()) continue;
        if (!notifier->second.tags.empty() &&
            !std::all_of(notifier->second.tags.begin(), notifier->second.tags.end(), [&](const std::string &nt) {
                return std::any_of(metric->tags.begin(), metric->tags.end(),
                                   [&](const Metrics::Tag &mt) { return mt.second == nt; });
            })) {
            continue;
        }
        size_t &current_count = alerts_count[metric];

        if (check_condition(notifier->second.condition, metric->value_)) {
            current_count++;
            if (current_count == notifier->second.alert_count)
                alerts.emplace_back(formatAlertMessage(notifier->second.alertStartMessage, metric));
        } else {
            if (current_count >= notifier->second.alert_count)
                alerts.emplace_back(formatAlertMessage(notifier->second.alertStoppedMessage, metric));
            current_count = 0;
        }
    }
    for (auto &alert : alerts)
        for (auto &chat : chatIds) {
            boost::json::object message = {{"chat_id", chat}, {"text", alert}, {"parse_mode", "HTML"}};
            std::cout << pusher->post("", boost::json::serialize(message)) << std::endl;
        }
}

void TelegramNotifierPlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = RegisterModel("MetricsModel", new MetricsModel(), MetricsModel);
    MetricsModel::instance()->registerUploader(this);
    parseSettings();
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new TelegramNotifierPlugin(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

using boost::property_tree::ptree;
namespace fs = std::filesystem;

void TelegramNotifierPlugin::parseSettings()
{
    if (!fs::exists(configPath)) {
        Y_LOG(1, " Config file " << configPath << " not found. Creating default config...");
        fs::create_directories(fs::path(configPath).parent_path());
        ptree pt, chats, notifiers, notifier, tags;
        pt.put("token", token);
        pt.add_child("chat_ids", chats);
        notifier.put("metric", "");
        notifier.put("alert_count", 1);
        notifier.put("condition", "");
        notifier.add_child("tags", tags);
        notifier.put("alertStartMessage", "Alert! {metric}:{value} {tags}");
        notifier.put("alertStoppedMessage", "Alert stopped! {metric}:{value} {tags}");
        notifiers.push_back(std::make_pair("", notifier));
        pt.add_child("notifiers", notifiers);
        write_json(configPath, pt);
        G_LOG(1, " Default config created at " << configPath);
        return;
    }
    try {
        ptree pt;
        read_json(configPath, pt);
        token = pt.get<std::string>("token", "");

        for (auto &v : pt.get_child("chat_ids", ptree{})) chatIds.insert(v.second.get_value<std::string>());
        for (auto &n : pt.get_child("notifiers", ptree{})) {
            Notify notify;
            notify.metric              = n.second.get<std::string>("metric", "");
            notify.alertStartMessage   = n.second.get<std::string>("alertStartMessage", " ");
            notify.alertStoppedMessage = n.second.get<std::string>("alertStoppedMessage", "");

            if (notify.metric.empty()) continue;
            notify.condition = parse_condition(n.second.get<std::string>("condition", ""));
            std::cout << (int)notify.condition.type << " " << notify.condition.value << "\n";
            if (notify.condition.type == ConditionType::Error) {
                R_LOG(1, " invalid condition in notifier for metric " << n.second.get<std::string>("metric", ""));
                continue;
            }
            notify.alert_count = std::max<size_t>(1, n.second.get<size_t>("alert_count", 1));
            for (auto &t : n.second.get_child("tags", ptree{})) notify.tags.insert(t.second.get_value<std::string>());
            notifiers[notify.metric] = std::move(notify);
        }
    } catch (const std::exception &e) {
        R_LOG(1, " error on load config " << configPath << " " << e.what());
    }
}

TelegramNotifierPlugin::~TelegramNotifierPlugin() { MetricsModel::instance()->unregisterUploader(this); }

TelegramNotifierPlugin::Condition TelegramNotifierPlugin::parse_condition(const std::string &s)
{
    if (s.empty()) return {.type = ConditionType::Error};
    // [a;b]
    if (s.front() == '[' && s.back() == ']') {
        Condition c;
        auto sep = s.find(';');
        if (sep == std::string::npos) return {.type = ConditionType::Error};
        try {
            c.min_value = std::stod(s.substr(1, sep - 1));
            c.max_value = std::stod(s.substr(sep + 1, s.size() - sep - 2));
            if (c.min_value > c.max_value) return {.type = ConditionType::Error};
            c.type = ConditionType::Range;
        } catch (...) {
            c.type = ConditionType::Error;
        }
        c.text = s;
        return c;
    }
    if (s.starts_with(">=")) return {.text = s, .type = ConditionType::GreaterEqual, .value = std::stoul(s.substr(2))};
    if (s.starts_with("<=")) return {.text = s, .type = ConditionType::LessEqual, .value = std::stoul(s.substr(2))};
    if (s.starts_with(">")) return {.text = s, .type = ConditionType::Greater, .value = std::stoul(s.substr(1))};
    if (s.starts_with("<")) return {.text = s, .type = ConditionType::Less, .value = std::stoul(s.substr(1))};
    if (s.starts_with("=")) return {.text = s, .type = ConditionType::Equal, .value = std::stoul(s.substr(1))};
    return {.type = ConditionType::Error};
}

bool TelegramNotifierPlugin::check_condition(const Condition &c, size_t metric_value)
{
    switch (c.type) {
        case ConditionType::Greater: return metric_value > c.value;
        case ConditionType::Less: return metric_value < c.value;
        case ConditionType::Equal: return metric_value == c.value;
        case ConditionType::Range: return metric_value >= c.min_value && metric_value <= c.max_value;
        case ConditionType::GreaterEqual: return metric_value >= c.value;
        case ConditionType::LessEqual: return metric_value <= c.value;
        case ConditionType::Error: break;
    }
    return false;
}

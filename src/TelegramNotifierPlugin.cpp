#include "TelegramNotifierPlugin.hpp"
#include <Logger/Log.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdlib>
#include <filesystem>
#include <string>
#include "Metrics.hpp"
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <PluginCore/Logger/Log.hpp>

void TelegramNotifierPlugin::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("TelegramNotifierPlugin " + std::string(TelegramNotifierPlugin_VERSION))
        .addOption(configPath, "TelegramNotifierPath", "path to config for TelegramNotifier.json");
}
void TelegramNotifierPlugin::postInit()
{
    if (token.empty()) return;
    pusher = std::make_unique<d3156::EasyHttpClient>(MetricsModel::instance()->getIO(), "https://api.telegram.org");
    pusher->setBasePath("/bot" + token + "/sendMessage");
    pusher->setContentType("application/json");
}
void TelegramNotifierPlugin::alert(const std::string &alert)
{
    for (auto &chat : chatIds) {
        boost::json::object message = {{"chat_id", chat}, {"text", alert}, {"parse_mode", "HTML"}};
        pusher->post("", boost::json::serialize(message));
    }
}

void TelegramNotifierPlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = RegisterModel("MetricsModel", new MetricsModel(), MetricsModel);
    MetricsModel::instance()->registerAlertProvider(this);
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
        ptree pt, chats;
        pt.put("token", token);
        pt.add_child("chat_ids", chats);
        write_json(configPath, pt);
        G_LOG(1, " Default config created at " << configPath);
        return;
    }
    try {
        ptree pt;
        read_json(configPath, pt);
        token = pt.get<std::string>("token", "");

        for (auto &v : pt.get_child("chat_ids", ptree{})) chatIds.insert(v.second.get_value<std::string>());

    } catch (const std::exception &e) {
        R_LOG(1, " error on load config " << configPath << " " << e.what());
    }
}

TelegramNotifierPlugin::~TelegramNotifierPlugin() { MetricsModel::instance()->unregisterAlertProvider(this); }

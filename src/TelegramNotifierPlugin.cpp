#include "TelegramNotifierPlugin.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <MetricsModel/Metrics>
#include <boost/json/serialize.hpp>
#include <PluginCore/Logger/Log>
#include <ConfiguratorModel>
#include <chrono>

void TelegramNotifierPlugin::registerArgs(d3156::Args::Builder &bldr) { bldr.setVersion(FULL_NAME); }

void TelegramNotifierPlugin::postInit()
{
    if (conf.token.value.empty()) return;
    pusher = std::make_unique<d3156::AsyncHttpClient>(MetricsModel::instance()->getIO(), "https://api.telegram.org");
    pusher->setBasePath("/bot" + conf.token.value + "/sendMessage");
    pusher->setContentType("application/json");
}
void TelegramNotifierPlugin::alert(const std::string &alert)
{
    for (auto &chat : conf.chat_id.items) {
        boost::json::object message = {{"chat_id", *chat}, {"text", alert}, {"parse_mode", "HTML"}};
        net::co_spawn(MetricsModel::instance()->getIO(),
                      pusher->postAsync("", boost::json::serialize(message), 5, std::chrono::seconds(2)),
                      net::detached);
    }
}

void TelegramNotifierPlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = models.registerModel<MetricsModel>();
    MetricsModel::instance()->registerAlertProvider(this);
    models.registerModel<ConfiguratorModel>()->registerConfig("TelegramNotifier", conf);
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new TelegramNotifierPlugin(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

TelegramNotifierPlugin::~TelegramNotifierPlugin() { MetricsModel::instance()->unregisterAlertProvider(this); }

#pragma once
#include <MetricsModel/NotifierSystem>
#include <PluginCore/IPlugin>
#include <MetricsModel/MetricsModel>
#include <EasyHttpLib/AsyncHttpClient>
#include <string>
#include <BaseConfig>

class TelegramNotifierPlugin final : public d3156::PluginCore::IPlugin, public NotifierSystem::NotifierProvider
{
    struct Config : public d3156::Config {
        Config() : d3156::Config("") {}
        CONFIG_ARRAY(chat_id, std::string);
        CONFIG_STRING(token, "");
    } conf;

    void alert(const std::string &) override;

    std::unique_ptr<d3156::AsyncHttpClient> pusher;

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;

    ~TelegramNotifierPlugin();
};

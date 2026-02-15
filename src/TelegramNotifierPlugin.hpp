#pragma once
#include <MetricsModel/NotifierSystem>
#include <PluginCore/IPlugin>
#include <MetricsModel/MetricsModel>
#include <EasyHttpLib/AsyncHttpClient>
#include <string>
class TelegramNotifierPlugin final : public d3156::PluginCore::IPlugin, public NotifierSystem::NotifierProvider
{
    std::string configPath        = "./configs/TelegramNotifier.json";
    std::set<std::string> chatIds = {};
    std::string token             = "";

    void parseSettings();

    void alert(const std::string &) override;

    std::unique_ptr<d3156::AsyncHttpClient> pusher;

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;

    ~TelegramNotifierPlugin();
};

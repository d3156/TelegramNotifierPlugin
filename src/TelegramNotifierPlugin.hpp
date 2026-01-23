#pragma once
#include <PluginCore/IPlugin.hpp>
#include <PluginCore/IModel.hpp>
#include <MetricsModel.hpp>
#include <MetricUploader.hpp>
#include <EasyHttpClient.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <map>

#define Y_TelegramNotifier "\033[33m[TelegramNotifier]\033[0m "
#define R_TelegramNotifier "\033[31m[TelegramNotifier]\033[0m "
#define G_TelegramNotifier "\033[32m[TelegramNotifier]\033[0m "
#define W_TelegramNotifier "[TelegramNotifier] "

class TelegramNotifierPlugin final : public d3156::PluginCore::IPlugin, public Metrics::Uploader
{
    std::string configPath = "./configs/TelegramNotifier.json";

    enum class ConditionType { Greater, Less, GreaterEqual, LessEqual, Equal, Range, Error };

    std::map<Metrics::Metric *, size_t> alerts_count;

    struct Condition {
        std::string text = "";
        ConditionType type;
        size_t value;     // для > < >= <= =
        size_t min_value; // для Range
        size_t max_value; // для Range
    };

    struct Notify {
        /// From config
        std::string metric = "";
        size_t alert_count = 0; /// Количество повторов для срабатывания
        Condition condition;
        std::set<std::string> tags = {}; //optional
        
        /// Runtime
        std::string alertStartMessage = "Alert! {metric}:{value} {tags}";  
        std::string alertStoppedMessage = "Alert stopped! {metric}:{value} {tags}";
    };

    std::set<std::string> chatIds = {};
    std::string token = "";
    std::unordered_map<std::string, Notify> notifiers;

    void parseSettings();

    void upload(std::set<Metrics::Metric *> &statistics) override;

    std::unique_ptr<d3156::EasyHttpClient> pusher;

    bool check_condition(const Condition &c, size_t metric_value);

    Condition parse_condition(const std::string &s);

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;

    ~TelegramNotifierPlugin();
};

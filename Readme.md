# TelegramNotifierPlugin

A plugin for monitoring metrics and sending alerts to Telegram.  
Плагин для мониторинга метрик и отправки алертов в Telegram.

### Overview

TelegramNotifierPlugin allows you to:  

- Monitor specific metrics. (Мониторить конкретные метрики)
- Set alert conditions (`>`, `<`, `>=`, `<=`, `=`, `[min;max]`). (Задавать условия срабатывания)
- Filter metrics by tags (AND logic: all tags must be present).  (Фильтровать метрики по тегам (логика AND: все теги должны присутствовать))
- Use `alert_count` to trigger alerts only after repeated occurrences.  (Использовать alert_count для определения количество повторов события для срабатывания)
- Customize alert messages with placeholders: `{metric}`, `{value}`, `{tags}`, `{duraion}`.  

### Configuration

Default config file: `TelegramNotifier.json`

Example:

```json
{
  "token": "YOUR_TELEGRAM_BOT_TOKEN",
  "chat_ids": ["123456789", "987654321"],
  "notifiers": [
    {
      "metric": "cpu_usage",
      "alert_count": 3,
      "condition": ">=80",
      "tags": ["production", "db"],
      "alertStartMessage": "Alert! {metric} is {value}% high [{tags}]",
      "alertStoppedMessage": "Alert stopped for {metric} [{tags}]"
    }
  ]
}
```
- `token` — Telegram bot token.
- `chat_ids` — Telegram chats to send alerts to (Telegram-чаты для отправки алертов).
- `metric` — metric name to monitor (название метрики для мониторинга).
- `alert_count` — number of consecutive occurrences to trigger alert (количество последовательных срабатываний для триггера алерта).
- `condition` — alert condition (условие срабатывания алерта).
- `tags` — optional tags filter (необязательный фильтр по тегам).
- `alertStartMessage / alertStoppedMessage` — customizable messages with placeholders (настраиваемые сообщения с плейсхолдерами `{metric}`, `{value}`, `{tags}`, `{duraion}`).
## Usage
- 1. Place `TelegramNotifier.json` in the config folder (default: `./configs/`) (поместите `TelegramNotifier.json` в папку конфигурации (по умолчанию: `./configs/`)).
- 2. Run the plugin — it will automatically create a default config if missing (запустите плагин — он автоматически создаст файл конфигурации, если его нет).
- 3. Alerts are sent to configured chats when conditions are met (алерты отправляются в указанные чаты при выполнении условий).

## Example config for [PingNode](https://github.com/d3156/PingNode)
Config `./configs/TelegramNotifier.json`
```json
{
  "token": "YOUR_TELEGRAM_BOT_TOKEN",
  "chat_ids": ["123456789", "987654321"],
  "notifiers": [
    {
      "metric": "PingNodeLatency_gauge",
      "alert_count": 3,
      "condition": ">=500",
      "tags": [],
      "alertStartMessage": "⚠️ Внимание! Задержка сервера [{tags}] превысила порог: {value}мс ",
      "alertStoppedMessage": "✅ Задержка сервера [{tags}] вернулась в норму: {value}мс"
    },
    {
      "metric": "PingNodeAvailable",
      "alert_count": 5,
      "condition": "=0",
      "tags": [],
      "alertStartMessage": "⚠️ Внимание! Сервер [{tags}] недоступен!",
      "alertStoppedMessage": "✅ Сервер снова [{tags}] доступен!"
    }
  ]
}
```
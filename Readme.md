# TelegramNotifierPlugin

TelegramNotifierPlugin receives pre-evaluated alerts from MetricsModel and delivers notifications to Telegram chats.
TelegramNotifierPlugin получает предварительно обработанные алерты от MetricsModel и доставляет уведомления в Telegram-чаты.

### Configuration

Default config file: `TelegramNotifier.json`

Example:

```json
{
  "token": "YOUR_TELEGRAM_BOT_TOKEN",
  "chat_ids": ["123456789", "987654321"]
}
```
- `token` — Telegram bot token.
- `chat_ids` — Telegram chats to send alerts to (Telegram-чаты для отправки алертов).

## Usage
- 1. Place `TelegramNotifier.json` in the config folder (default: `./configs/`) (поместите `TelegramNotifier.json` в папку конфигурации (по умолчанию: `./configs/`)).
- 2. Run the plugin — it will automatically create a default config if missing (запустите плагин — он автоматически создаст файл конфигурации, если его нет).
- 3. Alerts are sent to configured chats when conditions are met (алерты отправляются в указанные чаты при выполнении условий).

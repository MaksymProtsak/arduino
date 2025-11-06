#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define EEPROM_SIZE 4096
#define MAX_CHAT_IDS 100
#define DEVICE_NAME_ADDR 0

const char* ssid="OnePlus 13R A83E";
const char* password="max123456";
const char* bot_token = "8109829011:AAHDIZi3GLXulMbi6h43z7YT22FPNho5Z6Q";
const char* chatId = "606063499";
const int chipId = ESP.getChipId();

const char* commandStart = "/start";
const char* commandLedOn = "/led_on";
const char* commandLedOff = "/led_off";
const char* commandGetChipId = "/get_chip_id";
const char* commandDevice = "/device";
const char* commendForeverSleepOn = "/foreverSleepOn";
const char* commendForeverSleepOff = "/foreverSleepOff";

bool ledPinStatus = HIGH;
int LedPin = 2;
String startMessage = "This is WiFi button.";
String devName;
bool foreverSleep;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

struct DeviceData {
  char wifiSSID[64];
  char wifiPassword[64];
  char deviceName[256];  // 124 кириличних символи
  bool deepSleepEnabled;
  int64_t chatIds[MAX_CHAT_IDS];
  int chatIdCount;
};

DeviceData data;

void setup() {
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, ledPinStatus);

  EEPROM.begin(EEPROM_SIZE);
  String EEPROMresponse = readFromEEPROM();
  if (EEPROMresponse) {
    devName = EEPROMresponse;
    startMessage = devName;
  }
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  secured_client.setInsecure();
  bot.sendMessage(chatId, startMessage, "");
  if (foreverSleep) {
    ESP.deepSleep(0);
  }
}

int writeInEEPROM(const String& text) {
  size_t maxLen = sizeof(data.deviceName) - 1;
  size_t len = text.length() * 2;
  
  if (len >= maxLen || len == 0) {
    return -1;
  }

  if (text == readFromEEPROM()) {
    return 0;
  }

  for (size_t i = 0; i < len; i=i+1){
    data.deviceName[i] = text[i];
  }
  data.deviceName[len] = '\0';

  for (size_t i = 0; i < sizeof(data.deviceName); ++i) {
    EEPROM.write(DEVICE_NAME_ADDR + i, data.deviceName[i]);
  }
  
  EEPROM.commit();
  return 1;
}

String readFromEEPROM() {
  String result = "";
  for (int i = 0; i < EEPROM_SIZE; i = i + 1) {
    char c = EEPROM.read(i);

    if (c == '\0') break;
    result += c;
  }
  return result;
}

void parseMessageText(int newMessages) {
  for (int i = 0; i < newMessages; i = i + 1) {
    String messageText = bot.messages[i].text;

      if (messageText == commandStart){
        String response =
          "Команди:\n" +
          String(commandLedOn) + " - увімкнути світлодіод.\n" +
          String(commandLedOff) + " - вимкнути світлодіод.\n" +
          String(commandGetChipId) + " - дізнатися ID номер.\n" +
          "/" + String(chipId) + "- перевірити назву девайса\n" +
          String(commandDevice) + String(chipId) + "_" + " - Змінити назву девайса. Після `_` дописати майбутню назву";
        bot.sendMessage(
          chatId, response, "");
      }

    if (messageText == commandLedOn) {  // /led_on
      ledPinStatus = LOW;
      bot.sendMessage(chatId, F("LED IS ON"), "");
      digitalWrite(LedPin, ledPinStatus);
      return;
    }

    if (messageText == commandLedOff) {  // led_off
      ledPinStatus = HIGH;
      bot.sendMessage(chatId, F("LED IS OFF"), "");
      digitalWrite(LedPin, ledPinStatus);
      return;
    }

    if (messageText == commandGetChipId) {  // get_chip_id
      bot.sendMessage(chatId, String(chipId), "");
      return;
    }

    if (messageText.indexOf(String(chipId)) != -1) {
      if (messageText == "/" + String(chipId)) {
        bot.sendMessage(chatId, devName, "");
        return;
      }

      if (messageText.indexOf(String(commandDevice) + String(chipId)) != -1) {
        int underscoreIndex = messageText.indexOf('_');

        if (underscoreIndex != -1) {
          devName = messageText.substring(underscoreIndex + 1);
          int result = writeInEEPROM(devName);
          if (result == -1) {
            bot.sendMessage(chatId, "Помилка назви.", "");
          }
          if (result == 0) {
            bot.sendMessage(chatId, "Назва не змінена.", "");
          }
          if (result == 1) {
            bot.sendMessage(chatId, "Назва успішно змінена!", "");
          }
        } else {
          bot.sendMessage(chatId, "Не вказано розділовий знак `_`", "");
        }
      }
    }
  }
}

void loop() {
  int lastMessageId = bot.last_message_received + 1;
  int newMessages = bot.getUpdates(lastMessageId);
  if (newMessages) {
    parseMessageText(newMessages);
  }
}

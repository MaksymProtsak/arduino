#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define EEPROM_SIZE 512
// #define MAX_CHAT_IDS 100
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
const char* commandForeverSleepOn = "/foreverSleepOn";
const char* commandForeverSleepOff = "/foreverSleepOff";
const char* commandGetFromEEPROM = "/getFromEEPROM";
const char* commandSetWiFiName = "/setWiFiName_";
const char* commandGetWiFiName = "/getWiFiName";
const char* commandSetWiFiPassword = "/setWifiPassword_";
const char* commandGetWiFiPassword = "/getWifiPassword";
const char* commandSleepOn = "/setSleepOn";
const char* commandSleepOff = "/setSleepOff";


bool ledPinStatus = HIGH;
int LedPin = 2;
String startMessage = "This is WiFi button.";
String devName;
String wifiSSID;
bool foreverSleep;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

struct DeviceData {
  char deviceName[256];  // 124 кириличних символи
  char wifiSSID[64];
  char wifiPassword[64];
  bool deepSleepStatus;
  // int64_t chatIds[MAX_CHAT_IDS];
  // int chatIdCount;
};

DeviceData data;

void setup() {
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, ledPinStatus);
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  DeviceData EEPROMresponse = readFromEEPROM();
  if (EEPROMresponse.deviceName) {
     devName = EEPROMresponse.deviceName;
     startMessage = devName;
   }
    if (EEPROMresponse.deviceName) {
    wifiSSID = EEPROMresponse.wifiSSID;
   }

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

void writeDataInEEPROM() {
  EEPROM.put(0, data);
  EEPROM.commit();
}

int writeDeviceNameInDeviceData(String& deviceName) {
  size_t maxLen = sizeof(data.deviceName) - 1;
  size_t len = deviceName.length() * 2;
  if (len >= maxLen || len == 0) {
    return -1;
  }

  if (deviceName == (readFromEEPROM().deviceName)) {
    return 0;
  }
  for (size_t i = 0; i < len; i=i+1){
    data.deviceName[i] = deviceName[i];
  }

  data.deviceName[len] = '\0';
  return 1;
}

int writeWiFiNameInDeviceData(String& wifiName) {
  size_t maxLen = sizeof(data.wifiSSID) - 1;
  size_t len = wifiName.length() * 2;
  if (len >= maxLen || len == 0) {
    return -1;
  }
  if (wifiName == (readFromEEPROM().wifiSSID)) {
    return 0;
  }

  for (size_t i = 0; i < len; i=i+1){
    data.wifiSSID[i] = wifiName[i];
  }

  data.wifiSSID[len] = '\0';
  return 1;
}

int writeWiFiPasswordInDeviceData(String& wifiPassword) {
  size_t maxLen = sizeof(data.wifiPassword) - 1;
  size_t len = wifiPassword.length() * 2;
  if (len >= maxLen || len == 0) {
    return -1;
  }
  if (wifiPassword == (readFromEEPROM().wifiSSID)) {
    return 0;
  }

  for (size_t i = 0; i < len; i=i+1){
    data.wifiPassword[i] = wifiPassword[i];
  }

  data.wifiPassword[len] = '\0';
  return 1;
}

int writeSlpeepStatusInDeviceData(bool deepSleepStatus) {
  if (deepSleepStatus == (readFromEEPROM().deepSleepStatus)) {
    return 0;
  }
  data.deepSleepStatus = deepSleepStatus;
  return 1;
}


DeviceData readFromEEPROM() {
  EEPROM.get(0, data);
  return data;
}

void parseMessageText(int newMessages) {
  for (int i = 0; i < newMessages; i = i + 1) {
    String messageText = bot.messages[i].text;

    if (messageText == commandStart){ // /start
      String response =
        "Команди:\n" +
        String(commandLedOn) + " - увімкнути світлодіод.\n" +
        String(commandLedOff) + " - вимкнути світлодіод.\n" +
        String(commandGetChipId) + " - дізнатися ID номер.\n" +
        "/" + String(chipId) + "- перевірити назву девайса\n" +
        String(commandDevice) + String(chipId) + "_" + " - Змінити назву девайса. Після `_` дописати майбутню назву\n" +
        String(commandSetWiFiName) + " - встановити назву WiFi. Після `_` дописати майбутню назву\n" +
        String(commandGetWiFiName) + " - переглянути назву WiFi\n" +
        String(commandSetWiFiPassword) + " - встановити пароль WiFi\n" + 
        String(commandGetWiFiPassword) + " - перегляути пароль WiFi\n" +
        String(commandSleepOn) + " - девайс засинає після надсилання повідомлення про натискання кнопки RESET\n" +
        String(commandSleepOff) + " - девайс працює в режимі без сну\n" +
        String(commandGetFromEEPROM) + " - дістати всі дані з EEPROM\n";

      
      bot.sendMessage(chatId, response, "");
      return;
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

    if (messageText == commandGetChipId) {  // /get_chip_id
      bot.sendMessage(chatId, String(chipId), "");
      return;
    }

    if (messageText.indexOf(String(chipId))) { // /<chipId>
      if (messageText == "/" + String(chipId)) {
        bot.sendMessage(chatId, devName, "");
        return;
      }

      if (messageText.indexOf(String(commandDevice) + String(chipId)) != -1) { // /device<chipId>
        int underscoreIndex = messageText.indexOf('_'); // device<chipId>_<text>
        int result = 0;
        if (underscoreIndex != -1) {
          devName = messageText.substring(underscoreIndex + 1);
          if (devName == startMessage){
            bot.sendMessage(chatId, "Помилка назви. Майбутня назва співпадає з поточною.", "");
          return;
          }
          if (devName.length() > 0) {
            result = writeDeviceNameInDeviceData(devName);
          }
        }
        if (underscoreIndex == -1) {
          bot.sendMessage(chatId, "Помилка назви. Має бути розділовий знак `_` після " + String(commandDevice) + String(chipId), "");
          return;
        }
        if (result == 0) {
          bot.sendMessage(chatId, "Назва не змінена. Після символу `_` має бути майбутня назва девайсу", "");
          return;
        }
        if (result == 1) {
          writeDataInEEPROM();
          bot.sendMessage(chatId, "Назва успішно змінена на `" + String(devName) + "`!", "Markdown");
          return;
        }
      }
    }
    
    if (messageText.indexOf(String(commandSetWiFiName)) != -1) { // /setWiFiName_
      int underscoreIndex = messageText.indexOf('_'); // /setWiFiName_<text>
      int result = 0;
      if (underscoreIndex != -1) {
        String wifiName = messageText.substring(underscoreIndex + 1);
        if (wifiName == wifiSSID){
            bot.sendMessage(chatId, "Помилка назви. Майбутня назва співпадає з поточною.", "");
          return;
          }
        if (wifiName.length() > 0) {
            result = writeWiFiNameInDeviceData(wifiName);
          }
        if (underscoreIndex == -1) {
          bot.sendMessage(chatId, "Помилка назви. Має бути розділовий знак `_` після " + String(commandSetWiFiName) + ".", "");
          return;
        }

        if (result == 0) {
          bot.sendMessage(chatId, "Назва не змінена. Після символу `_` має бути майбутня назва WiFi мережі.", "");
          return;
        }
        if (result == 1) {
          writeDataInEEPROM();
        }
        bot.sendMessage(chatId, "Назва успішно змінена на `" + String(wifiName) + "`!", "Markdown");
      }
      return;
    }

    if (messageText == commandGetWiFiName) {  // /getWiFiName
      DeviceData result = readFromEEPROM();
      bot.sendMessage(chatId, String("Назва WiFi мережі: ") + "`" + String(result.wifiSSID) + "`", "Markdown");
      return;
    }

    // const char* commandSetWiFiPassword = "/setWifiPassword";
    if (messageText.indexOf(String(commandSetWiFiPassword)) != -1) { // /setWifiPassword_
    int underscoreIndex = messageText.indexOf('_'); // /setWifiPassword_<text>
      int result = 0;
      if (underscoreIndex != -1) {
        String wifiPassword = messageText.substring(underscoreIndex + 1);
        if (wifiPassword == wifiSSID){
            bot.sendMessage(chatId, "Помилка назви. Майбутня назва співпадає з поточною.", "");
          return;
          }
        if (wifiPassword.length() > 0) {
            result = writeWiFiPasswordInDeviceData(wifiPassword);
          }
        if (underscoreIndex == -1) {
          bot.sendMessage(chatId, "Помилка паролю. Має бути розділовий знак `_` після " + String(commandSetWiFiPassword) + ".", "");
          return;
        }

        if (result == 0) {
          bot.sendMessage(chatId, "Пароль не змінено. Після символу `_` має бути пароль мережі.", "");
          return;
        }
        if (result == 1) {
          writeDataInEEPROM();
        }
        bot.sendMessage(chatId, "Пароль успішно змінена на `" + String(wifiPassword) + "`!", "Markdown");
      }
      return;
    }

    if (messageText == commandGetWiFiPassword) {  // /getWifiPassword
      DeviceData result = readFromEEPROM();
      bot.sendMessage(chatId, String("Пароль WiFi мережі: ") + "`" + String(result.wifiPassword) + "`", "Markdown");
      return;
    }

    if (messageText == commandSleepOn) { // /setSleepOn
      int result = writeSlpeepStatusInDeviceData(1);
      if (result == 0) {
        bot.sendMessage(chatId, "Стан сну вже активований.", "");
        return;
      }
      if (result == 1) {
        writeDataInEEPROM();
      }
        bot.sendMessage(chatId, "Стан сну змінений на `" + String("ON") + "`!", "Markdown");
    return;
    }

    if (messageText == commandSleepOff) { // /setSleepOff
      int result = writeSlpeepStatusInDeviceData(0);
      if (result == 0) {
        bot.sendMessage(chatId, "Стан сну вже неактивний.", "");
        return;
      }
      if (result == 1) {
        writeDataInEEPROM();
      }
        bot.sendMessage(chatId, "Стан сну змінений на `" + String("OFF") + "`!", "Markdown");
    return;
    }

    if (messageText == commandGetFromEEPROM) { // /getFromEEPROM
      DeviceData result = readFromEEPROM();
      String response = String("Назва девайса: ") + "`" + String(result.deviceName) + "`" + "\n" +
      String("Назва WiFi мережі: ") + "`" + String(result.wifiSSID) + "`" + "\n" +
      String("Пароль WiFi мережі: ") + "`" + String(result.wifiPassword) + "`" "\n" +
      String("Стан сну: ") + "`" + String(result.deepSleepStatus ? "ON" : "OFF") + "`" + "\n";
      bot.sendMessage(chatId, response, "Markdown");
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

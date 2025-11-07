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
const char* commendForeverSleepOn = "/foreverSleepOn";
const char* commendForeverSleepOff = "/foreverSleepOff";
const char* commendGetFromEEPROM = "/getFromEEPROM";

bool ledPinStatus = HIGH;
int LedPin = 2;
String startMessage = "This is WiFi button.";
String devName;
bool foreverSleep;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

struct DeviceData {
  char deviceName[256];  // 124 кириличних символи
   //   char wifiSSID[64];
   //   char wifiPassword[64];
  // bool deepSleepEnabled;
  // int64_t chatIds[MAX_CHAT_IDS];
  // int chatIdCount;
};

DeviceData data;

void setup() {
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, ledPinStatus);
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  String EEPROMresponse = readFromEEPROM();
  Serial.println("Data from EEPROM");
  Serial.println(EEPROMresponse);
  // if (EEPROMresponse) {
  //   devName = EEPROMresponse;
  //   startMessage = devName;
  // }

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
  const byte* p = (const byte*)(const void*)&data;
  for (unsigned int i = 0; i < sizeof(DeviceData); i++) {
    EEPROM.write(i, *p++);
  }
  EEPROM.commit();
}

void writeDeviceNameInDeviceData(String& deviceName) {
  size_t maxLen = sizeof(data.deviceName) - 1;
  size_t len = deviceName.length() * 2;

  for (size_t i = 0; i < len; i=i+1){
    data.deviceName[i] = deviceName[i];
  }

  data.deviceName[len] = '\0';
}


int writeDeviceNameInEEPROM(const String& text) {
  size_t maxLen = sizeof(data.deviceName) - 1;
  size_t len = text.length() * 2;
  
  if (len >= maxLen || len == 0) {
    return -1;
  }

  if (text == readFromEEPROM()) { // Треба поправиви
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
          String(commandDevice) + String(chipId) + "_" + " - Змінити назву девайса. Після `_` дописати майбутню назву\n" +
          String(commendGetFromEEPROM) + " - дістати усі дані з EEPROM";
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

    if (messageText.indexOf(String(chipId)) != -1) { // <chipId>
      if (messageText == "/" + String(chipId)) {
        bot.sendMessage(chatId, devName, "");
        return;
      }

      if (messageText.indexOf(String(commandDevice) + String(chipId)) != -1) { // device<chipId>
        int underscoreIndex = messageText.indexOf('_'); // device<chipId>_<text>

        if (underscoreIndex != -1) {
          devName = messageText.substring(underscoreIndex + 1);
          writeDeviceNameInDeviceData(devName);
          writeDataInEEPROM();
        //   if (result == -1) {
        //     bot.sendMessage(chatId, "Помилка назви.", "");
        //   }
        //   if (result == 0) {
        //     bot.sendMessage(chatId, "Назва не змінена.", "");
        //   }
        //   if (result == 1) {
        //     bot.sendMessage(chatId, "Назва успішно змінена!", "");
        //   }
        // } else {
        //   bot.sendMessage(chatId, "Не вказано розділовий знак `_`", "");
        }
      }
    }

    if (messageText == commendGetFromEEPROM) { // getFromEEPROM
      bot.sendMessage(chatId, readFromEEPROM(), "");
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

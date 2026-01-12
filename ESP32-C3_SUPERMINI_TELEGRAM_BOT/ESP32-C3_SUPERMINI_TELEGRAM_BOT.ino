#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define EEPROM_SIZE 512
// #define MAX_CHAT_IDS 100
#define DEVICE_NAME_ADDR 0

const char* defaultSSID = "OnePlus 13R A83E";
const char* defaultPassword = "max123456";
const char* ssid=defaultSSID;
const char* password=defaultPassword;
const char* bot_token = "******";
const char* chatId = "606063499";
const uint32_t chipId = ESP.getEfuseMac() >> 24;

const char* commandStart = "/start";
const char* commandLedOn = "/led_on";
const char* commandLedOnRed = "/led_on_red";
const char* commandLedOnGreen = "/led_on_green";
const char* commandLedOnBlue = "/led_on_blue";
const char* commandLedOff = "/led_off";
const char* commandLedOffRed = "/led_off_red";
const char* commandLedOffGreen = "/led_off_green";
const char* commandLedOffBlue = "/led_off_blue";
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
const char* commandTestWiFiConnect = "/testWiFiConnect";
const char* commandWiFiAutoconnectOn = "/wifiAutoconnectOn";
const char* commandWiFiAutoconnectOff = "/wifiAutoconnectOff";


bool ledPinStatus = HIGH;
bool allMessegesSent = true;
int LedPin = 8;
int redPin = 5;
int greenPin = 6;
int bluePin = 7;
String startMessage = "This is WiFi button.";
String devName;
String wifiSSID;
bool foreverSleep;
bool connectToWiFi;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

struct DeviceData {
  char deviceName[256];  // 124 кириличних символи
  char wifiSSID[64];
  char wifiPassword[64];
  bool deepSleepStatus;
  bool connectToWiFi;
  // int64_t chatIds[MAX_CHAT_IDS];
  // int chatIdCount;
};

DeviceData data;

void setup() {
  pinMode(LedPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  digitalWrite(LedPin, ledPinStatus);
  digitalWrite(bluePin, LOW);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  Serial.begin(115200);
  delay(1000);

  EEPROM.begin(EEPROM_SIZE);
  DeviceData EEPROMresponse = readFromEEPROM();
  if (EEPROMresponse.deviceName) {
     devName = EEPROMresponse.deviceName;
     startMessage = devName;
   }
  if (EEPROMresponse.wifiSSID) {
    wifiSSID = EEPROMresponse.wifiSSID;
   }
  if (EEPROMresponse.connectToWiFi) {
    Serial.println("Підключаюся до вказаної мережі");
    if (EEPROMresponse.wifiSSID && EEPROMresponse.wifiPassword) {
      ssid = EEPROMresponse.wifiSSID;
      password = EEPROMresponse.wifiPassword;
    }
   }
  WiFi.begin(ssid, password);
  delayUntilConnectToWiFi(50);
  digitalWrite(redPin, LOW);

  if (WiFi.status() != WL_CONNECTED) {
    ssid = defaultSSID;
    password = defaultPassword;
    WiFi.disconnect(true);
    delay(200);
    WiFi.begin(ssid, password);
    Serial.println("Підключаюся до дефолтної мережі");
    delayUntilConnectToWiFi(50);
    if (WiFi.status() != WL_CONNECTED) {
      ESP.restart();
    }
  }
  Serial.println("Підключено до вказаної мережі");
  digitalWrite(bluePin, HIGH);
  delay(500);
  secured_client.setInsecure();
  bot.sendMessage(chatId, startMessage, "");
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, HIGH);
  delay(500);
  digitalWrite(greenPin, LOW);

  if (EEPROMresponse.deepSleepStatus) {
    nonLockReceiveMessages(10000);
    if (allMessegesSent) {
      esp_deep_sleep_start();
    }
  }
}

void delayUntilConnectToWiFi(int timesForDelay){
  for (int j = 0; j < timesForDelay; j++) {
    if (WiFi.status() != WL_CONNECTED) {
      analogWrite(redPin, 0);
      delay(450);
      analogWrite(redPin, 512);
      delay(50);
      analogWrite(redPin, 0);
      continue;
    }
    if (WiFi.status() == WL_CONNECTED){
      analogWrite(bluePin, 512);
      delay(500);
      analogWrite(bluePin, 0);
      break;
    }
  }
}

String testConnectToWiFi() {
  Serial.println("testConnectToWiFi");
  nonLockReceiveMessages(1000);
  digitalWrite(redPin, HIGH);
  DeviceData EEPROMresponse = readFromEEPROM();
  String wifiSSID = EEPROMresponse.wifiSSID;
  String wifiPassword = EEPROMresponse.wifiPassword;
  Serial.println(wifiSSID + " " + wifiPassword);
  String connectionResult = "";
  if (wifiSSID == "") {
    return wifiSSID + "не вдалося зчитати назву WiFi мережі з внутнішньої пам'яті";
  }

  if (wifiPassword == "") {
    return wifiSSID + "не вдалося зчитати пароль WiFi мережі з внутнішньої пам'яті";
  }

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == wifiSSID) {
      WiFi.mode(WIFI_STA); // Переключаємося в режим станції
      WiFi.disconnect(); // Відключаємося від поточного підключення
      WiFi.begin(wifiSSID, wifiPassword); // Виконуємо підключення до мережі, передаємо дані з конвертацією у C рядок
      delayUntilConnectToWiFi(10);
      break;
    }
  }
  digitalWrite(redPin, LOW);
  if (WiFi.status() != WL_CONNECTED) {
    connectionResult = "Не вдалося підключитися до мережі " + wifiSSID + " з паролем " + wifiPassword + ".";
  }
  else if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(bluePin, HIGH);
    connectionResult = "Було успішно підключено до мережі " + wifiSSID + " з паролем " + wifiPassword + "! Зараз підключено до стандартної мережі.";
  }
  Serial.println(connectionResult);
  digitalWrite(bluePin, LOW);
  digitalWrite(redPin, HIGH);
  WiFi.disconnect(); // Відключаємося від поточного підключення
  WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //}
  delayUntilConnectToWiFi(50);
  digitalWrite(redPin, LOW);

  return connectionResult;
}

void nonLockReceiveMessages(int time) {
  unsigned long start = millis();
  while (millis() - start < time) {
    readMessagesIfPresent();
    yield();
  }
}

void readMessagesIfPresent() {
  int lastMessageId = bot.last_message_received + 1;
  int newMessages = bot.getUpdates(lastMessageId);
  allMessegesSent = false;
  if (newMessages) {
    parseMessageText(newMessages);
  }
  allMessegesSent = true;
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

int writeWiFiAutoconnecctStatusInDeviceData(bool autoconnectStatus) {
  data.connectToWiFi = autoconnectStatus;
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
        String(commandLedOnRed) + " - увімкнути червоний світлодіод.\n" +
        String(commandLedOnGreen) + " - увімкнути зелений світлодіод.\n" +
        String(commandLedOnBlue) + " - увімкнути синій світлодіод.\n" +
        String(commandLedOff) + " - вимкнути світлодіод.\n" +
        String(commandLedOffRed) + " - вимкнути червоний світлодіод.\n" +
        String(commandLedOffGreen) + " - вимкнути зелений світлодіод.\n" +
        String(commandLedOffBlue) + " - вимкнути синій світлодіод.\n" +
        String(commandGetChipId) + " - дізнатися ID номер.\n" +
        "/" + String(chipId) + "- перевірити назву девайса\n" +
        String(commandDevice) + String(chipId) + "_" + " - Змінити назву девайса. Після `_` дописати майбутню назву\n" +
        String(commandSetWiFiName) + " - встановити назву WiFi. Після `_` дописати майбутню назву\n" +
        String(commandGetWiFiName) + " - переглянути назву WiFi\n" +
        String(commandSetWiFiPassword) + " - встановити пароль WiFi\n" +
        String(commandGetWiFiPassword) + " - перегляути пароль WiFi\n" +
        String(commandSleepOn) + " - девайс засинає після надсилання повідомлення про натискання кнопки RESET\n" +
        String(commandSleepOff) + " - девайс працює в режимі без сну\n" +
        String(commandGetFromEEPROM) + " - дістати всі дані з EEPROM\n" +
        String(commandTestWiFiConnect) + " - перевірка вказаної мережі WiFi\n" +
        String(commandWiFiAutoconnectOn) + " - автоматичне підключення до вказаної мережі після увімкнення девайса\n" +
        String(commandWiFiAutoconnectOff) + " - підключення до мережі за замовчуванням після увімкнення";

      bot.sendMessage(chatId, response, "");
    }

    if (messageText == commandLedOn) {  // /led_on
      ledPinStatus = LOW;
      bot.sendMessage(chatId, F("LED IS ON"), "");
      digitalWrite(LedPin, ledPinStatus);
    }

    if (messageText == commandLedOnRed) {  // /led_on_red
      bot.sendMessage(chatId, F("LED RED IS ON"), "");
      digitalWrite(redPin, HIGH);
    }

    if (messageText == commandLedOnGreen) {  // /led_on_green
      bot.sendMessage(chatId, F("LED GREEN IS ON"), "");
      digitalWrite(greenPin, HIGH);
    }

    if (messageText == commandLedOnBlue) {  // /led_on_blue
      bot.sendMessage(chatId, F("LED BLUE IS ON"), "");
      digitalWrite(bluePin, HIGH);
    }

    if (messageText == commandLedOff) {  // led_off
      ledPinStatus = HIGH;
      bot.sendMessage(chatId, F("LED IS OFF"), "");
      digitalWrite(LedPin, ledPinStatus);
    }

    if (messageText == commandLedOffRed) {  // led_off_red
      bot.sendMessage(chatId, F("LED RED IS OFF"), "");
      digitalWrite(redPin, LOW);
    }

    if (messageText == commandLedOffGreen) {  // led_off_green
      bot.sendMessage(chatId, F("LED GREEN IS OFF"), "");
      digitalWrite(greenPin, LOW);
    }

    if (messageText == commandLedOffBlue) {  // led_off_blue
      bot.sendMessage(chatId, F("LED BLUE IS OFF"), "");
      digitalWrite(bluePin, LOW);
    }

    if (messageText == commandGetChipId) {  // /get_chip_id
      bot.sendMessage(chatId, String(chipId), "");
    }

    if (messageText.indexOf(String(chipId))) { // /<chipId>
      if (messageText == "/" + String(chipId)) {
        bot.sendMessage(chatId, devName, "");
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
    }

    if (messageText == commandGetWiFiName) {  // /getWiFiName
      DeviceData result = readFromEEPROM();
      bot.sendMessage(chatId, String("Назва WiFi мережі: ") + "`" + String(result.wifiSSID) + "`", "Markdown");
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
    }

    if (messageText == commandGetWiFiPassword) {  // /getWifiPassword
      DeviceData result = readFromEEPROM();
      bot.sendMessage(chatId, String("Пароль WiFi мережі: ") + "`" + String(result.wifiPassword) + "`", "Markdown");
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
    }

    if (messageText == commandGetFromEEPROM) { // /getFromEEPROM
      DeviceData result = readFromEEPROM();
      String response = String("Назва девайса: ") + "`" + String(result.deviceName) + "`" + "\n" +
      String("Назва WiFi мережі: ") + "`" + String(result.wifiSSID) + "`" + "\n" +
      String("Пароль WiFi мережі: ") + "`" + String(result.wifiPassword) + "`" "\n" +
      String("Стан сну: ") + "`" + String(result.deepSleepStatus ? "ON" : "OFF") + "`" + "\n" +
      String("Стан автопідключеня до встановленої WiFi мережі: ") + "`" + String(result.connectToWiFi ? "ON" : "OFF") + "`" + "\n";
      bot.sendMessage(chatId, response, "Markdown");
    }

    if (messageText == commandTestWiFiConnect) { // /testWiFiConnect   
      bot.sendMessage(chatId, "Починаю спробу підключення до вказаної WiFi мережі", "");
      bot.sendMessage(chatId, testConnectToWiFi(), "");
    }

    if (messageText == commandWiFiAutoconnectOn) { // /wifiAutoconnectOn
      int result = writeWiFiAutoconnecctStatusInDeviceData(1);
      if (result == 1) {
        writeDataInEEPROM();
      }
        bot.sendMessage(chatId, "Стан автопідключення до вказаної мережі змінений на `" + String("ON") + "`!", "Markdown");
    }

    if (messageText == commandWiFiAutoconnectOff) { // /wifiAutoconnectOff
      int result = writeWiFiAutoconnecctStatusInDeviceData(0);
      if (result == 0) {
        bot.sendMessage(chatId, "Стан автопідключення до WiFi мережі за замовченням вже активований.", "");
        return;
      }
      if (result == 1) {
        writeDataInEEPROM();
      }
        bot.sendMessage(chatId, "Стан автопідключення до вказаної мережі змінений на `" + String("OFF") + "`!", "Markdown");
    }
  }
}

void loop() {
  readMessagesIfPresent();
}

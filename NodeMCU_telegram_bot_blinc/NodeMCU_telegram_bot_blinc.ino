#include<EEPROM.h>
#include<ESP8266WiFi.h>
#include<WiFiClientSecure.h>
#include<UniversalTelegramBot.h>

#define EEPROM_SIZE 512

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

bool ledPinStatus = HIGH;
int LedPin = 2;
String startMessage = "This is WiFi button.";
String devName;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

void setup() {
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, ledPinStatus);

  EEPROM.begin(512);
  String EEPROMresponse = readFromEEPROM();
  if (EEPROMresponse){
    devName = EEPROMresponse;
    startMessage = devName;
  }
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }

  secured_client.setInsecure();
  bot.sendMessage(chatId, startMessage, "");
  // ESP.deepSleep(0);
}

int writeInEEPROM(const String & text){
  int len = text.length();
  if (len >= EEPROM_SIZE | len == 0){
    return -1;
    }

  if (text == readFromEEPROM()){
    return 0;
  }

  for (int i = 0; i < text.length(); ++i) {
    EEPROM.write(i, text[i]);
  }
  EEPROM.write(text.length(), '\0');
  EEPROM.commit();
  return 1;
}

String readFromEEPROM(){
  String result = "";
  for (int i=0; i <EEPROM_SIZE; i=i+1){
    char c = EEPROM.read(i);
    if (c == '\0') break;
    result += c;
  }
  return result;
}

void parseMessageText(int newMessages){
    for (int i = 0; i < newMessages; i=i+1){
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

      if (messageText == commandLedOn){
        ledPinStatus = LOW;
        bot.sendMessage(chatId, F("LED IS ON"), "");
        digitalWrite(LedPin, ledPinStatus);
        return;
      }

      if (messageText == commandLedOff){
        ledPinStatus = HIGH;
        bot.sendMessage(chatId, F("LED IS OFF"), "");
        digitalWrite(LedPin, ledPinStatus);
        return;
      }
      
      if (messageText == commandGetChipId){
        bot.sendMessage(chatId, String(chipId), "");
        return;
      }   
      
      if (messageText.indexOf(String(chipId)) != -1){
        if (messageText == "/" + String(chipId)){
          bot.sendMessage(chatId, devName, "");
          return;
        }

        if (messageText.indexOf(String(commandDevice) + String(chipId)) != -1){
          int underscoreIndex = messageText.indexOf('_');

          if (underscoreIndex != -1){
            devName = messageText.substring(underscoreIndex + 1);
            int result = writeInEEPROM(devName);
            if (result == -1){
              bot.sendMessage(chatId, "Помилка назви.", "");
            }
            if (result == 0){
              bot.sendMessage(chatId, "Назва не змінена.", "");
            }
            if (result == 1){
              bot.sendMessage(chatId, "Назва успішно змінена!", "");
            }
          }
          else{
            bot.sendMessage(chatId, "Не вказано розділовий знак `_`", "");
          }
        }
      }
    }
}

void loop() {
  int lastMessageId = bot.last_message_received + 1;
  int newMessages = bot.getUpdates(lastMessageId);
  if (newMessages){
    parseMessageText(newMessages);
  }
}

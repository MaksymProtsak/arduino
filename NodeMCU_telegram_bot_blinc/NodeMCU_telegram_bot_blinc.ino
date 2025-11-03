#include<ESP8266WiFi.h>
#include<WiFiClientSecure.h>
#include<UniversalTelegramBot.h>

const char* ssid="OnePlus 13R A83E";
const char* password="max123456";
const char* bot_token = "8109829011:AAHDIZi3GLXulMbi6h43z7YT22FPNho5Z6Q";
const char* chatId = "606063499";
bool ledPinStatus = HIGH;
String chipId = String(ESP.getChipId());

int LedPin = 2;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(bot_token, secured_client);

int handleMessage(int numMessages){
  return bot.getUpdates(numMessages);
}

void setup() {
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, ledPinStatus);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  // Serial.print("NodeMCU IP Address:");
  // Serial.println(WiFi.localIP());

  secured_client.setInsecure();
  // ESP.deepSleep(0);
}

void loop() {
  int lastMessageId = bot.last_message_received + 1;
  int new_messages = bot.getUpdates(lastMessageId);

  if (new_messages){
    for (int i = 0; i < new_messages; i=i+1){
      String messageText = bot.messages[i].text;
      if (messageText == "ON"){
        ledPinStatus = LOW;
        bot.sendMessage(chatId, "LED IS ON", "");
      }
      if (messageText == "OFF"){
        ledPinStatus = HIGH;
        bot.sendMessage(chatId, "LED IS OFF", "");
      }
    }
    digitalWrite(LedPin, ledPinStatus);
  }
}

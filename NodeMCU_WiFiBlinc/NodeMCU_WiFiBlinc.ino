#include<ESP8266WiFi.h>
#include<WiFiClientSecure.h>
#include<UniversalTelegramBot.h>

const char* ssid="OnePlus 13R A83E";
const char* password="max123456";
const char* bot_token = "8109829011:AAHDIZi3GLXulMbi6h43z7YT22FPNho5Z6Q";
const char* chatId = "606063499";
String chipId = String(ESP.getChipId());

int LedPin = 2;

void setup() {
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, LOW);

  Serial.begin(115200);

  Serial.println();
  Serial.print("Connecting to WiFi");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println(".");
  }
  Serial.print("NodeMCU IP Address:");
  Serial.println(WiFi.localIP());

  // X509List cert(TELEGRAM_CERTIFICATE_ROOT);
  WiFiClientSecure secured_client;
  UniversalTelegramBot bot(bot_token, secured_client);
  secured_client.setInsecure();

  bot.sendMessage(chatId, chipId, "");
  Serial.print("The message was sent.");
  ESP.deepSleep(0);
}

void loop() {
  digitalWrite(LedPin, HIGH);
  delay(1000);
  digitalWrite(LedPin, LOW);
  delay(1000);
}

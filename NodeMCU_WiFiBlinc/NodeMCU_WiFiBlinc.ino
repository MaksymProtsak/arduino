#include<ESP8266WiFi.h>

const char* ssid="OnePlus 13R A83E";
const char* password="max123456";

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
    delay(500);
    Serial.println(".");
  }
  Serial.print("NodeMCU IP Address:");
  Serial.print(WiFi.localIP());
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LedPin, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LedPin, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}

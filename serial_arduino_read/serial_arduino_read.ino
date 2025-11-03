int x;

void setup() { 
	Serial.begin(9600); 
	Serial.setTimeout(1); 
  pinMode(LED_BUILTIN, OUTPUT);
} 

void loop() { 
	while (!Serial.available()); 
	x = Serial.readString().toInt(); 

    if (x) {
      digitalWrite(LED_BUILTIN, HIGH);
	    Serial.print("Led is ON\n"); 

    } else {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print("Led is OFF\n"); 
    }
} 

#include <SoftwareSerial.h>
SoftwareSerial ESP8266(6, 5);

#define LDR 3
int cnt = 0;
int sentCnt = 0;
bool ack = true;
unsigned long oldMillis = 0;
unsigned long interruptChangeMillis = 0;
unsigned long cntMillis = 0;
String incomingStr;
boolean oldPinState = LOW;

void setup() {
  pinMode(LDR, INPUT_PULLUP);
  ESP8266.begin(9600);
  Serial.begin(115200);
  //attachInterrupt(digitalPinToInterrupt(LDR), handleInterrupt, RISING); //funktioniert nicht bei Arduino Pro Mini
}

void loop() {
  if (digitalRead(LDR) == HIGH) {
    if (millis() - cntMillis > 200  && oldPinState == LOW) {
      cnt++;
      Serial.println("StateChange! "+String(millis() - cntMillis));
      cntMillis = millis();
      oldPinState = HIGH;
    }
  } else {
    oldPinState = LOW;
    cntMillis = millis();
  }

  if (millis() - oldMillis > 10000) {
    if (cnt > 0 & ack) {
      Serial.println("Sende Count " + String(cnt, DEC));
      ESP8266.print(String(cnt));
      ack = false;
      sentCnt = cnt;
      cnt = 0;
    }
    oldMillis = millis();
  }

  if (ESP8266.available() > 0) {
    incomingStr = ESP8266.readString();
    Serial.println("Incoming:" + incomingStr);
    if (incomingStr.toInt() == sentCnt)  {
      sentCnt = 0;
      ack = true;
      Serial.println("Empfang bestaetigt.");
    }
  }
}

void handleInterrupt() {
  if (millis() - interruptChangeMillis > 100) {
    if (digitalRead(LDR) == LOW)
      cnt++;
    interruptChangeMillis = millis();
  }
}


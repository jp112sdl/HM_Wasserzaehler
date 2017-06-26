#include <SoftwareSerial.h>
#define LDR 3
#define RX  6
#define TX  5

#define TransmitDelaySeconds        30
#define ReTransmitDeslaySeconds     10
#define InputPulseDelayMilliSeconds 350

SoftwareSerial ESP8266(RX, TX);

int cnt = 0;
int sentCnt = 0;
bool ack = true;
unsigned long oldMillis = 0;
unsigned long cntMillis = 0;
unsigned long timeoutMillis = 0;
String incomingStr;
boolean oldPinState = LOW;

void setup() {
  pinMode(LDR, INPUT_PULLUP);
  ESP8266.begin(9600);
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  if (oldMillis > millis())
    oldMillis = 0;

  if (cntMillis > millis())
    cntMillis = 0;

  if (timeoutMillis > millis())
    timeoutMillis = 0;

  //Eingangs-Pin überwachen (attachInterrupt funktioniert am Arduino Pro mini nicht zuverlässig!)
  if (digitalRead(LDR) == LOW) {
    if (oldPinState == HIGH) {
      //Serial.println("LDR = LOW!");
      if ((millis() - cntMillis > InputPulseDelayMilliSeconds) ) {
        cnt++;
        Serial.println("IMPULS erkannt");
        Serial.println("Millis Diff = " + String((millis() - cntMillis)));
      }
      oldPinState = LOW;
    }
  } else {
    if (oldPinState == LOW) {
      //Serial.println("LDR = HIGH!");
      oldPinState = HIGH;
      cntMillis = millis();
    }
  }

  if (millis() - oldMillis > 1000 * TransmitDelaySeconds) {
    if (cnt > 0 & ack) {
      Serial.println("Sende Count " + String(cnt));
      ESP8266.print(String(cnt));
      ack = false;
      sentCnt = cnt;
      cnt = 0;
      timeoutMillis = millis();
    }
    oldMillis = millis();
  }

  if (!ack && millis() - timeoutMillis > 1000 * ReTransmitDeslaySeconds) {
    Serial.println("Transmit Timeout -> Retransmit");
    ESP8266.print(String(sentCnt));
    timeoutMillis = millis();
  }

  if (ESP8266.available()) {
    incomingStr = ESP8266.readString();
    Serial.println("Incoming:" + incomingStr);
    if (incomingStr.toInt() == sentCnt)  {
      sentCnt = 0;
      ack = true;
      Serial.println("Empfang bestaetigt.");
    }
  }
  delay(10);
}



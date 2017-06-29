#include <SoftwareSerial.h>
#define LDR A0
#define RX  6
#define TX  5

#define DEBUG                        false
#define TransmitDelaySeconds         15
#define ReTransmitDeslaySeconds      10
#define PulseDelayMilliSeconds       500
#define LDRHIGH                      540  //Anbau = 250; Haus = 610
#define LDRLOW                       490   //Anbau = 50; Haus = 550

SoftwareSerial ESP8266(RX, TX);

int cnt = 0;
int sentCnt = 0;
bool ack = true;
unsigned long oldMillis = 0;
unsigned long pulseDiffMillis = 0;
unsigned long timeoutMillis = 0;
unsigned long debugMillis = 0;
String incomingStr;
boolean oldPinState = HIGH;
boolean LDRstate = LOW;

void setup() {
  pinMode(LDR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  ESP8266.begin(9600);
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  if (oldMillis > millis())
    oldMillis = 0;

  if (pulseDiffMillis > millis())
    pulseDiffMillis = 0;

  if (timeoutMillis > millis())
    timeoutMillis = 0;

  if (debugMillis > millis())
    timeoutMillis = 0;

  int LDRval = analogRead(LDR);
  if (LDRval > LDRHIGH) LDRstate = HIGH;
  if (LDRval < LDRLOW) LDRstate = LOW;
  digitalWrite(LED_BUILTIN, LDRstate);

  if (LDRstate == HIGH) {
    if (oldPinState == LOW) {
      Serial.println("LOW -> HIGH (" + String(LDRval) + ")");
      if ((millis() - pulseDiffMillis > PulseDelayMilliSeconds) ) {
        if (!DEBUG) cnt++;
        Serial.print("IMPULS erkannt ");
        Serial.println("Millis Diff = " + String((millis() - pulseDiffMillis)));
      }
      oldPinState = HIGH;
    }
  } else {
    if (oldPinState == HIGH) {
      Serial.println("HIGH -> LOW (" + String(LDRval) + ")");
      oldPinState = LOW;
      pulseDiffMillis = millis();
    }
  }

  if (DEBUG && millis() - debugMillis > 100) {
    debugMillis = millis();
    Serial.println("Analogwert = " + String(LDRval));
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
}



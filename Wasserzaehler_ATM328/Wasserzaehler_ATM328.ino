#include <SoftwareSerial.h>
#define LDR A0
#define RX  6
#define TX  5

#define DEBUG                        false
#define TransmitDelaySeconds         15
#define ReTransmitDelaySeconds       10
#define PulseDelayMilliSeconds       500
#define LDRHIGH                      220   //Anbau = 220; Haus = 540
#define LDRLOW                       50   //Anbau = 50;  Haus = 490

SoftwareSerial ESP8266(RX, TX);

unsigned int pulseCount = 0;
unsigned int sentPulseCount = 0;

bool transmitAck = true;
bool oldPinState = HIGH;
bool LDRstate = LOW;

unsigned long oldTransmitDelayMillis = 0;
unsigned long pulseDiffMillis = 0;
unsigned long transmitTimeoutMillis = 0;
unsigned long debugMillis = 0;

String incomingStr;

void setup() {
  pinMode(LDR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  ESP8266.begin(9600);
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  if (oldTransmitDelayMillis > millis())
    oldTransmitDelayMillis = 0;

  if (pulseDiffMillis > millis())
    pulseDiffMillis = 0;

  if (transmitTimeoutMillis > millis())
    transmitTimeoutMillis = 0;

  if (debugMillis > millis())
    debugMillis = 0;

  int LDRval = analogRead(LDR);
  if (LDRval > LDRHIGH) LDRstate = HIGH;
  if (LDRval < LDRLOW) LDRstate = LOW;
  digitalWrite(LED_BUILTIN, LDRstate);

  if (LDRstate == HIGH) {
    if (oldPinState == LOW) {
      Serial.println("LOW -> HIGH (" + String(LDRval) + ")");
      if ((millis() - pulseDiffMillis > PulseDelayMilliSeconds) ) {
        if (!DEBUG) pulseCount++;
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

  if (millis() - oldTransmitDelayMillis > 1000 * TransmitDelaySeconds) {
    if (pulseCount > 0 & transmitAck) {
      Serial.println("Sende Count " + String(pulseCount));
      ESP8266.print(";"+String(pulseCount)+";");
      transmitAck = false;
      sentPulseCount = pulseCount;
      pulseCount = 0;
      transmitTimeoutMillis = millis();
    }
    oldTransmitDelayMillis = millis();
  }

  if (!transmitAck && millis() - transmitTimeoutMillis > 1000 * ReTransmitDelaySeconds) {
    Serial.println("Transmit Timeout -> Retransmit");
    ESP8266.print(";"+String(sentPulseCount)+";");
    transmitTimeoutMillis = millis();
  }

  if (ESP8266.available()) {
    incomingStr = ESP8266.readString();
    Serial.println("Incoming:" + incomingStr);
    if (incomingStr.indexOf("ACK") != -1)  {
      sentPulseCount = 0;
      transmitAck = true;
      Serial.println("Empfang bestaetigt.");
    }
  }
}



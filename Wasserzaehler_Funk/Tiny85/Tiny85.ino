#define LED          1
#define PulseDelayMilliSeconds       500
#define LDRHIGH                      740   //Anbau = 320; Haus = 540
#define LDRLOW                       590   //Anbau = 50;  Haus = 490
unsigned int pulseCount = 0;

bool oldPinState = HIGH;
bool LDRstate = LOW;

unsigned long pulseDiffMillis = 0;

void setup() {
  pinMode(2, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  int LDRval = analogRead(1);

  if (LDRval > LDRHIGH) LDRstate = HIGH;
  if (LDRval < LDRLOW) LDRstate = LOW;
  if (LDRstate == HIGH) {
    if (oldPinState == LOW) {
      if ((millis() - pulseDiffMillis > PulseDelayMilliSeconds) ) {
        digitalWrite(LED, HIGH);
      }
      oldPinState = HIGH;
    }
  } else {
    if (oldPinState == HIGH) {
      digitalWrite(LED, LOW);
      oldPinState = LOW;
      pulseDiffMillis = millis();
    }
  }
}

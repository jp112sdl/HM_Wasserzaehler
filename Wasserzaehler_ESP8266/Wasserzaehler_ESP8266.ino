#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

Adafruit_SSD1306 display(0);
SoftwareSerial ATM328(D5, D6);

#define             Taster        D7
#define             HTTPTIMEOUT   3000
//#define             UDPDEBUG
#define             SERIALDEBUG
#define             DEVICENAME    "WASSERZAEHLER_HAUS "

#define IPSize        16
#define variableSize 255

#ifdef UDPDEBUG
const char * SYSLOGIP = "192.168.1.251";
#define SYSLOGPORT          514
#define UDPALIVEMILLIS 1000 * 60 * 15
#else
#define UDPALIVEMILLIS 0
#endif

enum _SyslogSeverity {
  _slEmergency,
  _slAlert,
  _slCritical,
  _slError,
  _slWarning,
  _slNotice,
  _slInformational,
  _slDebug
};

unsigned long lastMillis = 0;
unsigned long lastUDPALIVEMillis = 0;
unsigned long keyPressMillis = 0;

bool oldKeyState = HIGH;
bool counterReset = false;

String incomingStr;

unsigned int ZaehlerWert = 0;

String configFilename = "sysconf.json";

char ccuip[IPSize] = "";
char variable[variableSize]  = "";

//WifiManager - don't touch
bool shouldSaveConfig        = false;
#define wifiManagerDebugOutput   true
char ip[IPSize]      = "0.0.0.0";
char netmask[IPSize] = "0.0.0.0";
char gw[IPSize]      = "0.0.0.0";
bool startWifiManager = false;

bool OTAStart = false;

#define UDPPORT 6678
struct udp_t {
  WiFiUDP UDP;
  char incomingPacket[255];
  bool Ready = false;
} UDPClient;

void setup() {
  ATM328.begin(9600);
  Serial.begin(115200);
  pinMode(Taster, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.display();
  display.println("Start...");
  display.display();

  DEBUG("Begin");
  if (digitalRead(Taster) == LOW) {
    DEBUG("Taster = LOW");
    startWifiManager = true;
  }

  if (!SPIFFS.begin()) {
    display.println("SPIFFS");
    display.println("mnt error");
    display.display();
    DEBUG("Failed to mount file system");
  } else {
    if (!loadSysConfig()) {
      DEBUG("Failed to load config");
      display.println("Failed to");
      display.println("load conf");
      display.display();
    } else {
      DEBUG("Config loaded");
      display.println("Config");
      display.println("loaded");
      display.display();
    }
  }

  if (doWifiConnect() == true) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Wifi");
    display.println("Connected");
    display.display();
    setStateCCUCUxD(variable, String(ZaehlerWert));
  }
  else ESP.restart();

  UDPClient.UDP.begin(UDPPORT);
  UDPClient.Ready = true;

  startOTAhandling();

  DEBUG("START", "loop()", _slInformational);
}

void loop() {
  ArduinoOTA.handle();

  if (lastUDPALIVEMillis > millis())
    lastUDPALIVEMillis = millis();

  if (lastMillis > millis())
    lastMillis = millis();

  if (UDPALIVEMILLIS > 0 && millis() - lastUDPALIVEMillis > UDPALIVEMILLIS) {
    lastUDPALIVEMillis = millis();
    DEBUG("ALIVE", "loop()", _slInformational);
  }

  if (ATM328.available()) {
    incomingStr = ATM328.readString();
    if (incomingStr.startsWith(";")) {
      incomingStr = incomingStr.substring(1, incomingStr.length());
      incomingStr = incomingStr.substring(0, incomingStr.indexOf(";"));
      DEBUG("incomingStr = " + incomingStr, "loop()", _slInformational);
      if (incomingStr.toInt() < 2147483646) {
        ZaehlerWert = ZaehlerWert + incomingStr.toInt();
        saveSysConfig();
        if (!setStateCCUCUxD(variable, String(ZaehlerWert))) {
          DEBUG("1. Versuch setStateCCUCUxD() fehlgeschlagen. 2. Versuch...", "loop()", _slWarning);
          setStateCCUCUxD(variable, String(ZaehlerWert));
        }
      } else DEBUG("incomingStr = " + incomingStr, "loop()", _slWarning);
    }
    ATM328.print("ACK;ACK;ACK");
  }

  if (digitalRead(Taster) == LOW) {
    if (!counterReset) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextColor(WHITE, BLACK);
      display.println("Hold to");
      display.println("reset");
      display.println("counter");
      display.display();
    }

    if (oldKeyState == HIGH) {
      oldKeyState = LOW;
      keyPressMillis = millis();
    }
    if (millis() - keyPressMillis > 5000 && !counterReset) {
      ZaehlerWert = 0;
      saveSysConfig();
      if (!setStateCCUCUxD(variable, String(ZaehlerWert))) {
        setStateCCUCUxD(variable, String(ZaehlerWert));
      }
      display.println( ""); display.println( "- OK -"); display.display();
      counterReset = true;
    }
  } else {
    oldKeyState = HIGH;
    counterReset = false;
  }

  if (millis() - lastMillis > 1000 && digitalRead(Taster) != LOW) {
    display.clearDisplay();
    display.setCursor(0, 5);
    display.setTextColor(BLACK, WHITE);
    display.println("          ");
    display.setTextColor(WHITE, BLACK);
    display.println("          ");
    for (int i = 10; i > ((String)ZaehlerWert).length(); i--)
      display.print(" ");
    display.println(ZaehlerWert);
    display.println("          ");
    display.setTextColor(BLACK, WHITE);
    display.println("          ");
    display.display();
  }
}

bool doWifiConnect() {
  String _ssid = WiFi.SSID();
  String _psk = WiFi.psk();

  String _pskMask = "";
  for (int i = 0; i < _psk.length(); i++) {
    _pskMask += "*";
  }
  Serial.println("ssid = " + _ssid + ", psk = " + _pskMask);

  const char* ipStr = ip; byte ipBytes[4]; parseBytes(ipStr, '.', ipBytes, 4, 10);
  const char* netmaskStr = netmask; byte netmaskBytes[4]; parseBytes(netmaskStr, '.', netmaskBytes, 4, 10);
  const char* gwStr = gw; byte gwBytes[4]; parseBytes(gwStr, '.', gwBytes, 4, 10);


  if (!startWifiManager && _ssid != "" && _psk != "" ) {
    Serial.println("Connecting WLAN the classic way...");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _psk.c_str());
    int waitCounter = 0;
    if (String(ip) != "0.0.0.0")
      WiFi.config(IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]), IPAddress(gwBytes[0], gwBytes[1], gwBytes[2], gwBytes[3]), IPAddress(netmaskBytes[0], netmaskBytes[1], netmaskBytes[2], netmaskBytes[3]));

    while (WiFi.status() != WL_CONNECTED) {
      waitCounter++;
      Serial.print(".");
      if (waitCounter == 20) {
        return false;
      }
      delay(500);
    }
    Serial.println("Wifi Connected");
    return true;
  } else {
    WiFiManager wifiManager;
    wifiManager.setDebugOutput(wifiManagerDebugOutput);
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    WiFiManagerParameter custom_ccuip("ccuip", "CCU IP", ccuip, IPSize);
    WiFiManagerParameter custom_variable("variable", "Variablenname", variable, variableSize);

    WiFiManagerParameter custom_ip("custom_ip", "IP-Adresse", "", IPSize);
    WiFiManagerParameter custom_netmask("custom_netmask", "Netzmaske", "", IPSize);
    WiFiManagerParameter custom_gw("custom_gw", "Gateway", "", IPSize);
    WiFiManagerParameter custom_text("<br/><br>Statische IP (wenn leer, dann DHCP):");
    wifiManager.addParameter(&custom_ccuip);
    wifiManager.addParameter(&custom_variable);

    wifiManager.addParameter(&custom_text);
    wifiManager.addParameter(&custom_ip);
    wifiManager.addParameter(&custom_netmask);
    wifiManager.addParameter(&custom_gw);

    String Hostname = "ESP-WZ-" + WiFi.macAddress();
    wifiManager.setConfigPortalTimeout(180);

    if (startWifiManager == true) {
      if (_ssid == "" || _psk == "" ) {
        wifiManager.resetSettings();
      }
      else {
        if (!wifiManager.startConfigPortal(Hostname.c_str())) {
          Serial.println("failed to connect and hit timeout");
          delay(1000);
          ESP.restart();
        }
      }
    }

    wifiManager.setSTAStaticIPConfig(IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]), IPAddress(gwBytes[0], gwBytes[1], gwBytes[2], gwBytes[3]), IPAddress(netmaskBytes[0], netmaskBytes[1], netmaskBytes[2], netmaskBytes[3]));

    wifiManager.autoConnect(Hostname.c_str());

    Serial.println("Wifi Connected");
    Serial.println("CUSTOM STATIC IP: " + String(ip) + " Netmask: " + String(netmask) + " GW: " + String(gw));
    if (shouldSaveConfig) {
      if (String(custom_ip.getValue()).length() > 5) {
        Serial.println("Custom IP Address is set!");
        strcpy(ip, custom_ip.getValue());
        strcpy(netmask, custom_netmask.getValue());
        strcpy(gw, custom_gw.getValue());
      } else {
        strcpy(ip,      "0.0.0.0");
        strcpy(netmask, "0.0.0.0");
        strcpy(gw,      "0.0.0.0");
      }

      strcpy(ccuip, custom_ccuip.getValue());
      strcpy(variable, custom_variable.getValue());

      saveSysConfig();
      SPIFFS.end();
      delay(100);
      ESP.restart();
    }

    return true;
  }
}
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("AP-Mode active!");
  display.clearDisplay();  display.setCursor(0, 0); display.println("AP-MODE!"); display.display();
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
  }
}

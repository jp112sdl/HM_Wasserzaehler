bool doWifiConnect() {
  String _ssid = WiFi.SSID();
  String _psk = WiFi.psk();

  const char* ipStr = ip; byte ipBytes[4]; parseBytes(ipStr, '.', ipBytes, 4, 10);
  const char* netmaskStr = netmask; byte netmaskBytes[4]; parseBytes(netmaskStr, '.', netmaskBytes, 4, 10);
  const char* gwStr = gw; byte gwBytes[4]; parseBytes(gwStr, '.', gwBytes, 4, 10);

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(wifiManagerDebugOutput);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_ccuip("ccuip", "CCU IP", ccuip, 15);
  WiFiManagerParameter custom_variable("variable", "Variablenname", variable, 255);

  WiFiManagerParameter custom_ip("custom_ip", "IP-Adresse", "", 15);
  WiFiManagerParameter custom_netmask("custom_netmask", "Netzmaske", "", 15);
  WiFiManagerParameter custom_gw("custom_gw", "Gateway", "", 15);
  WiFiManagerParameter custom_text("<br/><br>Statische IP (wenn leer, dann DHCP):");
  wifiManager.addParameter(&custom_ccuip);
  wifiManager.addParameter(&custom_variable);

  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_ip);
  wifiManager.addParameter(&custom_netmask);
  wifiManager.addParameter(&custom_gw);

  String Hostname = "ESP-WZ-" + WiFi.macAddress();
  char a[] = "";
  Hostname.toCharArray(a, 30);

  if (startWifiManager == true) {
    if (_ssid == "" || _psk == "" ) {
      wifiManager.resetSettings();
    }
    else {
      if (!wifiManager.startConfigPortal(a)) {
        Serial.println("failed to connect and hit timeout");
        delay(1000);
        ESP.restart();
      }
    }
  }

  wifiManager.setSTAStaticIPConfig(IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]), IPAddress(gwBytes[0], gwBytes[1], gwBytes[2], gwBytes[3]), IPAddress(netmaskBytes[0], netmaskBytes[1], netmaskBytes[2], netmaskBytes[3]));

  wifiManager.autoConnect(a);

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

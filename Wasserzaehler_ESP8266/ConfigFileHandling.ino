bool loadSysConfig() {
  File configFile = SPIFFS.open("/" + configFilename, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  json.printTo(Serial);

  strcpy(ip,             json["ip"]);
  strcpy(netmask,        json["netmask"]);
  strcpy(gw,             json["gw"]);

  strcpy(ccuip,            json["ccuip"]);
  strcpy(variable,         json["variable"]);

  ZaehlerWert = json["ZaehlerWert"];

  return true;
}

bool saveSysConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["ip"] = ip;
  json["netmask"] = netmask;
  json["gw"] = gw;
  
  json["ccuip"] = ccuip;
  json["variable"] = variable;
  
  json["ZaehlerWert"] = ZaehlerWert;

  File configFile = SPIFFS.open("/" + configFilename, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  json.printTo(Serial);
  json.printTo(configFile);
  return true;
}

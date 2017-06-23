bool setStateCCUCUxD(String id, String value) {
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.setTimeout(5000);
    String url = "http://" + String(ccuip) + ":8181/cuxd.exe?ret=dom.GetObject(%22" + id + "%22).State(" + value + ")";
    Serial.println("setStateCCU url: " + url);
    http.begin(url);
    int httpCode = http.GET();
    String payload = "";

    if (httpCode > 0) {
      Serial.println("HTTP " + id + " success");
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("HTTP " + id + " fail");
    }
    http.end();

    payload = payload.substring(payload.indexOf("<ret>"));
    payload = payload.substring(5, payload.indexOf("</ret>"));
    Serial.println("setStateFromCCUCUxD payload = " + payload);

    return (payload != "null");

  } else ESP.restart();

}

String getStateFromCCUCUxD(String id, String type) {
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.setTimeout(5000);
    String url = "http://" + String(ccuip) + ":8181/cuxd.exe?ret=dom.GetObject(%22" + id + "%22)." + type + "()";
    Serial.println("getStateCCU url: " + url);
    http.begin(url);
    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("HTTP " + id + " fail");
    }
    http.end();

    payload = payload.substring(payload.indexOf("<ret>"));
    payload = payload.substring(5, payload.indexOf("</ret>"));
    Serial.println("getStateFromCCUCUxD payload = " + payload);
    return payload;
  } else ESP.restart();
}


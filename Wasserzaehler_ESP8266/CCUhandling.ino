bool setStateCCUCUxD(String id, String value) {
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.setTimeout(5000);
    String url = "http://" + String(ccuip) + ":8181/cuxd.exe?ret=dom.GetObject(%22" + id + "%22).State(" + value + ")";
    Serial.println("setStateCCUCUxD url: " + url);
    http.begin(url);
    int httpCode = http.GET();
    String payload = "";
    Serial.println("setStateCCUCUxD HTTP ReturnCode = " + String(httpCode));

    if (httpCode != 200) {
      Serial.println("HTTP " + id +  " / " + value + " fail");
    } else {
      Serial.println("HTTP " + id +  " / " + value + " success");
      payload = http.getString();
    }

    http.end();

    payload = payload.substring(payload.indexOf("<ret>"));
    payload = payload.substring(5, payload.indexOf("</ret>"));
    Serial.println("setStateCCUCUxD payload = " + payload);

    return (payload != "null" && httpCode == 200);

  } else ESP.restart();
}

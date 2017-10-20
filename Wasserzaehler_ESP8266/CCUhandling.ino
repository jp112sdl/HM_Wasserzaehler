bool setStateCCUCUxD(String id, String value) {
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.setTimeout(HTTPTIMEOUT);
    String url = "http://" + String(ccuip) + ":8181/cuxd.exe?ret=dom.GetObject(%22" + id + "%22).State(" + value + ")";
    http.begin(url);
    int httpCode = http.GET();
    String payload = "";

    if (httpCode != 200) {
      DEBUG("HTTP " + id +  " / " + value + " fail ("+String(httpCode)+")","setStateCCUCUxD()",_slError);
    } else {
      DEBUG("HTTP " + id +  " / " + value + " success", "setStateCCUCUxD()",_slInformational);
      payload = http.getString();
    }

    http.end();

    payload = payload.substring(payload.indexOf("<ret>"));
    payload = payload.substring(5, payload.indexOf("</ret>"));
    //DEBUG("setStateCCUCUxD payload = " + payload);

    return (payload != "null" && httpCode == 200);

  } else ESP.restart();
}

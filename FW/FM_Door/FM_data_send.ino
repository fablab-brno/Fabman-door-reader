int lockVersion = 0;
String http_address = "";
String http_header = "https://fabman.io/api/v1/bridge/access";

void get_lockVersion() {
  Serial.print("HTTP header is: ");
  Serial.println(http_address);
  Serial.println("Section: get_lockVersion");
  http.begin(client, http_address);//, root_ca );  //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  http.addHeader("Accept", "application/json");
  http.addHeader("Authorization", API_key_user);
  int httpResponseCode = http.GET();   //Send the actual POST request
  String payload_ = http.getString();
  Serial.println("JSON from server:");
  Serial.println(payload_);
  Serial.println();
  if (httpResponseCode == 200) {
    DynamicJsonDocument jsonBuffer(200);
    deserializeJson(jsonBuffer, payload_);
    JsonObject root = jsonBuffer.as<JsonObject>();
    JsonObject metadata = root["metadata"];
    lockVersion = root["lockVersion"];
    Serial.print("lockVersion is: ");
    Serial.println(lockVersion);
  }
  http.end();  //Free resources
}

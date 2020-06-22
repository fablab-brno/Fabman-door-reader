/*
  sending heartbeat to Fabman
*/
void heartBeat() {
    if (wifiStatus == WL_CONNECTED) {
      String heartMessage = "";
      heartMessage += "{\"configVersion\":";
      heartMessage += configVersion;
      heartMessage += "}";
      char heartMessageChar[heartMessage.length() + 1];
      heartMessage.toCharArray(heartMessageChar, heartMessage.length() + 1);

      http.begin(client, "https://fabman.io/api/v1/bridge/heartbeat");//, root_ca );  //Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");             //Specify content-type header
      http.addHeader("Accept", "application/json");
      http.addHeader("Authorization", API_key);

      int httpResponseCode = http.POST(heartMessageChar);   //Send the actual POST request
        Serial.print("Heartbeat message: ");
        Serial.println(heartMessageChar);
        payload = http.getString();
        Serial.print("JSON from server:" + (payload));
        //Serial.println(payload);
        Serial.println();
        if (httpResponseCode < 0) {
          online = 0;
          connection();
        }
        else if (httpResponseCode > 0) {
          Serial.print("Responde code: ");
          Serial.println(httpResponseCode);   //Print return code
        } else {
          Serial.print("Error on sending request: ");
          Serial.println(httpResponseCode);
        }
      get_configVersion();
      http.end();  //Free resources
    }
}

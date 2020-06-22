/*
  sending stop message to Fabman
*/
void sendStop() {
  String stopMessage = "";
  stopMessage += "{\"stopType\":\"normal\",\"currentSession\":{\"id\":";
  stopMessage += currSessId;
  stopMessage += "}\}";
  char stopMessageChar[stopMessage.length() + 1];
  stopMessage.toCharArray(stopMessageChar, stopMessage.length() + 1);

#ifdef DEBUG
  Serial.print("currSessId was: ");
  Serial.println(currSessId);
#endif

  http.begin(client, "https://fabman.io/api/v1/bridge/stop");//, root_ca );  //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  http.addHeader("Accept", "application/json");
  http.addHeader("Authorization", API_key);

#ifdef DEBUG
  Serial.println(stopMessageChar);
#endif

  int httpResponseCode = http.POST(stopMessageChar);   //Send the actual POST request

  if (httpResponseCode == 200 || httpResponseCode == 204) {
    Serial.print("Responde code: ");
    Serial.println(httpResponseCode);   //Print return code
    Serial.println("Succesful logof");
    Serial.println();
    granted = 0;
  } else {
    Serial.print("Error on sending request: ");
    Serial.println(httpResponseCode);
  }
  http.end();  //Free resources
  currSessId = 0;
}

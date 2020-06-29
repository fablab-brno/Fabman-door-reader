bool userLogedIn = 0;
unsigned long timeOfAction = 0;
unsigned long timoutForCheck = 300000;
unsigned long nextTimout = 60000;
char query [0];
String tempCardId = "";
extern int temp_treshold;
DynamicJsonDocument keyBuffer(2048);
JsonObject root = keyBuffer.as<JsonObject>();

void getMember() {
  /*** Sending sw stop, to clear currSessId***/
  if (currSessId != 0) {
    sendStop();
    Serial.println("Sending stop at the end of card reading");
  }
  String cardId = "";
  cardId += "\"";
  cardId += data2;
  cardId += "\"";
  Serial.print("ID of card is: ");
  Serial.println(cardId);
  Serial.println();
  if (tempCardId != cardId) {
    tempCardId = "";
    tempCardId = cardId;
  }
  if (cardId != "\"FFFFFFFFFFFF\"") {
    query[cardId.length() + 1];
    cardId.toCharArray(query, cardId.length() + 1);
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      if (checkOH != 1) { //check if member have any active package
        String http_header = "https://fabman.io/api/v1/members?keyType=em4102&keyToken=" + String(data2) + "&embed=activePackages";
        http.begin(client, http_header);  //Specify destination for HTTP request
        http.addHeader("Content-Type", "application/json"); //Specify content-type header
        http.addHeader("Accept", "application/json");
        http.addHeader("Authorization", API_key_user);
        Serial.print("HTTP header is: ");
        Serial.println(http_header);
        int httpResponseCode = http.GET();   //Send the actual POST request
        String payload_ = http.getString();
        Serial.println("JSON from server:");
        Serial.println(payload_);
        Serial.println();
        yield();
        if (httpResponseCode == 200) {
          const size_t capacity = 2 * JSON_ARRAY_SIZE(1) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(15) + JSON_OBJECT_SIZE(20) + JSON_OBJECT_SIZE(45) + 1480;
          DynamicJsonDocument jsonBuffer(capacity);
          deserializeJson(jsonBuffer, payload_);
          JsonObject root = jsonBuffer[0];
          JsonObject activePackage = root["_embedded"]["memberPackages"][0];
          Serial.println("Active package(bool): " + activePackage);
          if (activePackage.isNull()) {
            Serial.println("No active package found");
            granted = 2;
          } else {
            Serial.println("Active package found");
            granted = 1;
          }
        }
        http.end();  //Free resources
      } else {
        String message = "";
        message += "{\"keys\":[{\"type\":\"em4102\",\"token\":";
        message += query;
        message += "}],\"currentSession\":{\"id\":";
        message += currSessId;
        message += "},\"previousSession\":{\"id\":";
        message += prevSessId;
        message += "},\"configVersion\":";
        message += configVersion;
        message += "}";
        char messageChar[message.length() + 1];
        message.toCharArray(messageChar, message.length() + 1);
        http.begin(client, "https://fabman.io/api/v1/bridge/access/");  //Specify destination for HTTP request
        http.addHeader("Content-Type", "application/json");             //Specify content-type header
        http.addHeader("Accept", "application/json");
        http.addHeader("Authorization", API_key);
#ifdef DEBUG
        Serial.println(messageChar);
#endif
        int httpResponseCode = http.POST(messageChar);   //Send the actual POST request

        if (httpResponseCode > 0) {
          Serial.print("Responde code: ");
          Serial.println(httpResponseCode);   //Print return code
          Serial.println();

          payload = http.getString();
#ifdef DEBUG
          Serial.println("JSON from server:");
          Serial.println(payload);
          Serial.println();
          Serial.print("Granted: ");
          Serial.println(granted);
#endif
          DynamicJsonDocument jsonBuffer(2048);
          String JSONinput = payload;
          deserializeJson(jsonBuffer, JSONinput);
          JsonObject config = jsonBuffer["config"];
          Serial.println();
#ifdef DEBUG
          Serial.println(JSONinput);
#endif
          Serial.println();
          Serial.print("configVersion: ");
          Serial.println(configVersion);
          String type = jsonBuffer["type"];
          String allowed = "allowed";
          String denied = "denied";
          String outOfOrder = "Out of order";
          String outsideOH = "Outside opening hours";
          String requiredTraining = "Requires training course";
          String requiredPackage = "Requires active package";
          String outsideAllowedTime = "Outside allowed timeframe";

          Serial.print ("Member is: ");
          Serial.println(type);
          Serial.println();
          String messages = jsonBuffer["messages"][0];
          String messages_ = jsonBuffer["messages"][1];
          Serial.print("Name of the member is: ");
          Serial.print(messages);
          Serial.print(" ");
          Serial.println(messages_);
          Serial.println();
          firstName = messages;
          lastName = messages_;

          if ((type == denied) && (lastName == outOfOrder)) {
            Serial.println("Equipment is offline");
            FM_mode_timout();
          }
          if ((type == denied) && (lastName == outsideOH)) {
            Serial.println("User not allowed: Outside OH");
            FM_mode_timout();
          }
          if ((type == denied) && (lastName == requiredTraining)) {
            Serial.println("User not allowed: Requires training course");
            FM_mode_timout();
          }
          if ((type == denied) && (lastName == requiredPackage)) {
            Serial.println("User not allowed: Requires active package");
            FM_mode_timout();
          }
          if ((type == denied) && (lastName == outsideAllowedTime)) {
            Serial.println("User not allowed: Outside allowed timeframe");
            FM_mode_timout();
          }
          Serial.println();

          if (type == allowed) {
            granted = 1;
            neopixelAllowed();
            Serial.println("Granted: " + String(granted));

            int sessionId = jsonBuffer["sessionId"];
            Serial.print("Session ID is: ");
            Serial.println(sessionId);
            currSessId = sessionId;
            /*
              Clear array of printed data & print what is in it..
            */

            Serial.println();
            get_configVersion();
            userLogedIn = 1;
            timeOfAction = millis();
          }  else { // user not llowed
            Serial.println("User is not allowed");
            Serial.println();
            granted = 2;
            neopixelDenied();
            Serial.println("Granted (not allowed): " + String(granted));
          }
          http.end();  //Free resources

        } else if (httpResponseCode < 0) { // response with negative value = no connection
          online = 0;
          connection();
        }
      }
      // end of the part where is FM connected
    } else { // no network to connect -> offline mode
      Serial.println("Error in WiFi connection");
      Serial.println("Checking Admin keys");
      compareKeys(offlineKeys, data2); // compare card which was read to known admin cards
      if ((adminKey == 1) || (httpResponseCode == -11)) {
        //  if  (offlineKeys.equalsIgnoreCase(cardId)) {
        Serial.println("Admin card found.");
        Serial.println();
        granted = 1;
        neopixelAllowed();
        userLogedIn = 1;
        timeOfAction = millis();
        Serial.println("Start of countdown..");
      } else {
        Serial.println("Admin card not found");
        Serial.println("User is not allowed");
        Serial.println();
        FM_mode_timout();
      }
      // }
    }
    Serial.println("--------------------------------------------------");
  } else {
    Serial.println("ID of the card do not include number, skipping.");
  }
}


/*
   Function to compare card which was read in offline mode to check if is Admin card
*/

void compareKeys(String aa, String bb) {

  int maxIndex = aa.length() - 1;
  int index = 0;
  int next_index;
  String data_word;
  do {
    next_index = aa.indexOf(',', index);
    data_word = aa.substring(index, next_index);
    if (data_word.equalsIgnoreCase(bb)) {
      adminKey = 1;
    }

    index = next_index + 1;
  } while ((next_index != -1) && (next_index < maxIndex));
}

void FM_mode_timout() {
  unsigned long now_millis = 0;
  unsigned long timout = 5000;

  if ((millis() - now_millis) > timout) {
  }
}

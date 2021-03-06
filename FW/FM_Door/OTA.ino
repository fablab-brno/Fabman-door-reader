String version;
bool versionChecked = 0;

//HTTPClient http_OTA;
void OTA() {
    if (versionChecked != 1) {
      getVersion();
      parseVersion(fwVersion, version);
    } else {
      Serial.println("Version alredy checked, skipping..");
    }
}


void getVersion() {
  HTTPClient newHttp; //create new instance of HTTPclient, otherwize non SSL request will break communication with Fabman.io
  unsigned long nowMillis = millis();
      newHttp.begin(version_url);
    //Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = newHttp.GET();
    Serial.println("Response from server: " + String(httpCode));
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      //Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        version = newHttp.getString();
        //Serial.println(version);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    newHttp.end();
}

void parseVersion(String a, String b) {
    int maxIndex = a.length() - 1;
    int new_maxIndex = b.length() - 1;
    int index = 0;
    int next_index;
    String a_word;
    String b_word;
    do {
      next_index = a.indexOf('.', index);
      a_word += a.substring(index, next_index);
      // uncomment for debug
      //Serial.println(a_word);
      index = next_index + 1;
    } while ((next_index != -1) && (next_index < maxIndex));

    do {
      next_index = b.indexOf('.', index);
      b_word += b.substring(index, next_index);
      // uncomment for debug
      //Serial.println(b_word);
      index = next_index + 1;
    } while ((next_index != -1) && (next_index < new_maxIndex));

    a_word.toInt();
    b_word.toInt();
    int number = a_word.toInt();
    int new_number = b_word.toInt();
    Serial.println("Actual FW version: " + String(number));
    Serial.println("FW version on server: " + String(new_number));
    if (new_number > number) {
      Serial.println("Detected new version, updating...");
      updateFirmware();
      //execOTA();
    } else {
      Serial.println("No new version, continue to boot..");
      versionChecked = 1;
      Serial.println();
    }
}

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName) {
  return header.substring(strlen(headerName.c_str()));
}

String FIRMWARE_URL = "";

void updateFirmware() {
  HTTPClient newHttp; //create new instance of HTTPclient, otherwize non SSL request will break communication with Fabman.io
      newHttp.begin(binURL);
    // Start pulling down the firmware binary.
    //    newHttp.begin(FIRMWARE_URL);
    int httpCode = newHttp.GET();
    if (httpCode <= 0) {
      Serial.printf("HTTP failed, error: %s\n",
                   http.errorToString(httpCode).c_str());
      return;
    }
    // Check that we have enough space for the new binary.
    int contentLen = newHttp.getSize();
    Serial.printf("Content-Length: %d\n", contentLen);
    bool canBegin = Update.begin(contentLen);
    if (!canBegin) {
      Serial.println("Not enough space to begin OTA");
      return;
    }
    // Write the HTTP stream to the Update library.
    WiFiClient* client_ = newHttp.getStreamPtr();
    size_t written = Update.writeStream(*client_);
    Serial.printf("OTA: %d/%d bytes written.\n", written, contentLen);
    if (written != contentLen) {
      Serial.println("Wrote partial binary. Giving up.");
      return;
    }
    if (!Update.end()) {
      Serial.println("Error from Update.end(): " +
                    String(Update.getError()));
      return;
    }
    if (Update.isFinished()) {
      Serial.println("Update successfully completed. Rebooting.");
      // This line is specific to the ESP32 platform:
      ESP.restart();
    } else {
      Serial.println("Error from Update.isFinished(): " +
                    String(Update.getError()));
      return;
    }
    newHttp.end();
}

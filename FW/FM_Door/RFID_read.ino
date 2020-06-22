/*
  function to get ID of the card
*/
bool readTagsTimeout = 0;
unsigned long readTagsTimeoutTime = 0;
unsigned long readTagsTimeoutTimeMax = 5000;

void readTags() {
    if (readTagsTimeout == 1) {
      while (RFID.available() > 0) {
        RFID.read();
        //Serial.println("Throwing away data");
        readTagsTimeoutTime = millis();
      }
    } else if (RFID.available() > 0) { // read tag numbers
      byte full_tag[10];
      byte chsum_byte = 0;
      byte chsum_reader_byte = 0;
      byte chsum_reader_calc_byte = 0;
      char tisk[3];
      String full_tag_id;
      delay(100); // needed to allow time for the data to come in from the serial buffer.
      for (int z = 0 ; z < 10 ; z++) {
        data1 = RFID.read();
        full_tag[z] = data1;
        sprintf(tisk, "%02X", data1);
        // Working with bytes 1-7, for correct checksum calculation
        if (z > 0 && z < 8) {
          // Calculation checksum of reader and our tags
          // according to ISO14443
          chsum_reader_calc_byte ^= data1;
          // Working with only TAG bytes for correct RFID checksum
          if (z > 2) {
            chsum_byte ^= data1;
            full_tag_id += tisk;
          }
        }
        // Saved reader reported checksum
        if (z == 8) {
          chsum_reader_byte = data1;
        }
      }
      sprintf(tisk, "%02X", chsum_byte);
      full_tag_id += tisk;
      // Full ID for Fabman readout
#ifdef DEBUG
      Serial.print("Full ID String: ");
      Serial.println(full_tag_id);
#endif
      Serial.println();
      if ((chsum_reader_byte == chsum_reader_calc_byte) &&
          (full_tag[0] == 0x02) && // Login to sector
          (full_tag[1] == 0x0A) && // Length = 'd10('hA)
          (full_tag[2] == 0x02)) { // Login success
        data2 = full_tag_id;
        getMember();
      }
#ifdef DEBUG
      else {
        Serial.print("Full RFID tag readout: ");
        for (int z = 0 ; z < 10 ; z++) {
          Serial.print(full_tag[z]);
          Serial.print(" ");
        }
        Serial.println();
        if (chsum_reader_byte == chsum_reader_calc_byte) {
          Serial.print("Incorrect checksum - calculated: ");
          Serial.print(chsum_reader_calc_byte);
          Serial.print(", observed: ");
          Serial.println(chsum_reader_byte);
        }
      }
#endif
      readTagsTimeout = 1;
      readTagsTimeoutTime = millis();
    }
    if ((millis() - readTagsTimeoutTime) > readTagsTimeoutTimeMax) {
      readTagsTimeout = 0;
    }
}

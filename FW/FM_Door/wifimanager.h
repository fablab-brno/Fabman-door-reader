#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager (development branch)

char api_key_char[40];
//String api_key = "";
//char mqtt_port[6] = "8080";
//char blynk_token[34] = "YOUR_BLYNK_TOKEN";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

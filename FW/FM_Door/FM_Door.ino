#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h> // install it from Library Manager
#include <SimpleTimer.h> // install from ZIP: https://github.com/jfturcot/SimpleTimer
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <Update.h>
#include "FS.h"
#include "SPIFFS.h"
#include "esp_system.h"
#include "SPI.h"
#include "config.h"
#include "cert.h"
#include "wifimanager.h"
#include "RGB_led.h"
#include <NTPClient.h> //https://github.com/taranais/NTPClient

#include <ETH.h>

// Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)
#define ETH_PHY_POWER 14

// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE ETH_PHY_LAN8720

// I2C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR 1
#define ETH_PHY_ADDR 1

// Pin# of the I2C clock signal for the Ethernet PHY
#define ETH_MDC_PIN 23

// Pin# of the I2C IO signal for the Ethernet PHY
#define ETH_MDIO_PIN 18

#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN

static bool eth_connected = false;

extern DynamicJsonDocument keyBuffer;

HardwareSerial RFID(2);

WiFiMulti wifiMulti;
SimpleTimer heartbeatTimer;
SimpleTimer wifiTimer;
SimpleTimer activityTimer;
SimpleTimer checkUser;
Preferences preferences;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000); // change 3600 offset to your timezone

int wifiStatus;
byte data1 = 0;
String data2 = "";
bool setupButtonState = 0;
unsigned long currSessId;
unsigned long prevSessId;
bool online;
bool offline;
String firstName = "";
String lastName = "";
char* config_offlineKeys_token;
String offlineKeys;
String mac = "";
String API_key = "";
String machineName = "";
bool adminKey = 0;
int granted = 0;
int machineId;
int memberId;
bool useEthernet = 0;

WiFiClientSecure client;
HTTPClient http;
int httpResponseCode = 0;
String JSONinput = "";
String payload = "";
unsigned int configVersion;
unsigned int currentConfigVersion;

#define ARRAY_SIZE 20
String ssid[ARRAY_SIZE];
String pass[ARRAY_SIZE];
String SSIDString, passwordString = "";
bool resetSettings = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting serial..");

  preferences.begin("store", false);

  configVersion = preferences.getUInt("configVersion", 0);
  offlineKeys = preferences.getString("offlineKeys", "");
  API_key = preferences.getString("API_key", "");
  prevSessId = preferences.getULong("prevSessId", 0);
  machineName = preferences.getString("machineName", "");
  SSIDString = preferences.getString("SSIDString", "");
  passwordString = preferences.getString("passwordString", "");
  useEthernet = preferences.getBool("useEthernet", useEthernet);
  machineId = preferences.getInt("machineId", 0);

  preferences.end();

  if (useEthernet != 0) {
    mac = getMacAddress(1);
  } else {
    mac = getMacAddress(0);
  }

  Serial.println("MAC Address is: " + (mac));
  if (API_key != NULL) {
    Serial.println("Using API key from NVM.");
  } else {
    setAPIkey();
  }
  Serial.println("Name of the machine is: " + (machineName));
  Serial.println("FW version is: " + String(fwVersion) + "_&_" + String(__DATE__) + "_&_" + String(__TIME__));
  Serial.println("configVersion is: " + String(configVersion));
  Serial.println("offlineKeys: " + (offlineKeys));
  Serial.println("API key is : " + (API_key));
  Serial.println("Machine ID is: " + String(machineId));
  Serial.println("Previous session ID was: " + String(prevSessId));
  Serial.println("SSIDs are: " + SSIDString);
  Serial.println("passwords are: " + passwordString);

  RFID.begin(9600); // start serial to RFID reader
  Serial.println("FM version:" + (fwVersion));

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  setDevice();

  if (useEthernet != 0) {
    WiFi.onEvent(WiFiEvent);
    ETH.begin( PHY1 , 14, 23, 18 , ETH_PHY_LAN8720);
  } else {
    setWiFi();

    Serial.println("Connecting Wifi...");

    int8_t scanResult;
    scanResult = WiFi.scanNetworks();
    if (scanResult == 0) {
      Serial.println("No known WiFi found, going offline.");
      offline = 1;
    } else {
      connection();
    }
    wifiStatus = WiFi.status();
  }
  timeClient.begin();
  timeClient.update();

  Serial.print("configVersion is: ");
  Serial.println(configVersion);

  Serial.println("Swipe your card...");
  startShow(granted);
}

void setDevice() {

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  machine_name_char[39] = '\0';
  api_key_char[39] = '\0';
  WiFiManagerParameter custom_device_name("device_name", "Device name (max 40 chars)", machine_name_char, 40);
  WiFiManagerParameter custom_api_key("API_key", "API key", api_key_char, 40);
  IntParameter custom_machine_id("machine_id", "Machine ID from Fabman", machine_id_int);
  IntParameter custom_use_lan("use_lan", "Use LAN port (0 = no, 1 = yes)", use_lan_int);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_device_name);
  wifiManager.addParameter(&custom_api_key);
  wifiManager.addParameter(&custom_machine_id);
  wifiManager.addParameter(&custom_use_lan);

  //reset settings - for testing
  setupButtonState = digitalRead(setupButtonPin);
  if (setupButtonState == 0) { // right code for FL FM PCB
    //if (stopButtonState == 1) { //uncomment for testing on ESP32 dev board
    //    Serial.println("Calling WiFiManager resetSettings function");
    wifiManager.resetSettings();
    if (!wifiManager.autoConnect("FabmanAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }

  //read updated parameters
  strcpy(machine_name_char, custom_device_name.getValue());
  strcpy(api_key_char, custom_api_key.getValue());
  machine_name_char[39] = '\0';
  api_key_char[39] = '\0';
  machine_id_int = custom_machine_id.getValue();
  use_lan_int = custom_use_lan.getValue();
  String API_key(api_key_char); //convert char array to String
  String customName = (machine_name_char);

  Serial.println("Device name is: " + customName);
  Serial.println("API key is: " + API_key);
  Serial.println("Bridge ID from WM is: " + String(machine_id_int));
  Serial.println("Use LAN is: " + String(use_lan_int));

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Saving config from WiFiManager");
    Serial.println();

    preferences.begin("store", false);
    machineName = customName;
    preferences.putString("machineName", machineName);
    String API_key_ = "Bearer " + API_key;// combine predefined String "Bearer " with String from WiFiManager
    preferences.putString("API_key", API_key_);
    machineId = machine_id_int;
    preferences.putInt("machineId", machineId);
    useEthernet = use_lan_int;
    preferences.putBool("useEthernet", useEthernet);
    preferences.end();
    Serial.println("Custom parameters saved to preferences");
    //end save
  }
}

void device() {
  if ((granted == 1) && (relayOpen == 0) && ((millis() - lastRelayOpenTime) >= relayOpenTime)) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Granted: " + String(granted));
    lastRelayOpenTime = millis();
    relayOpen = 1;
  }
  if ((relayOpen == 1) && ((millis() - lastRelayOpenTime) >= relayOpenTime)) {
    digitalWrite(relayPin, LOW);
    relayOpen = 0;
    sendStop();
  }
}

void checkLastDenied() {
  if (lastDenied == 1) {
    if ((millis() - lastLastDeniedTime) >= lastDeniedTime) {
      lastDenied = 0;
    }
  }
}

void clearGranted() {
  if ((granted == 1) || (granted == 2)) {
    if ((millis() - grantedTimoutLong) >= grantedTimeout) {
      granted = 0;
    }
  }
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      online = 1;
      OTA();
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void loop() {
  readTags();
  heartbeatTimer.run();
  wifiTimer.run();
  checkLastDenied();
  device();
  neopixelLoop();
  clearGranted();
  if (granted == 0) {
    updateBreath();
  }
}

/*function which is called when device is not connected to network*/
void connection() {
  wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED) {
    online = 0;
    Serial.println("Trying to connect.. ");
    Serial.println("");
    wifiMulti.run();
  } else if ((wifiStatus == WL_CONNECTED) && (online != 1)) {
    online = 1;
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Starting OTA..");
    OTA();
  }
}

/* parse Config version from JSON */
void get_configVersion() {
  DynamicJsonDocument jsonBuffer(2048);
  String JSONinput = payload;

  Serial.print("Config version payload: ");
  Serial.println(payload);

  //Serial.println(payload);
  Serial.println();
  deserializeJson(jsonBuffer, JSONinput);
  JsonObject root = jsonBuffer.as<JsonObject>();

  JsonObject config = root["config"];
  currentConfigVersion = root["config"]["configVersion"];
  JsonArray offlineKeys_array = config["offlineKeys"];
  for (offlineKeys : offlineKeys_array) {
    const char* token = offlineKeys["token"];
  }
  currentConfigVersion = root["config"]["configVersion"];
  Serial.print("currentConfigVersion which is read: ");
  Serial.println(currentConfigVersion);
  if (currentConfigVersion != 0) {
    if (currentConfigVersion != configVersion) {
      Serial.print("configVersion which is read: ");
      Serial.println(configVersion);
      preferences.begin("store", false);
      preferences.putUInt("configVersion", currentConfigVersion);
      preferences.end();
      Serial.println("Writing configVersion to NVS");
      configVersion = currentConfigVersion;
      offlineKeys = "";
      Serial.print("offlineKeys: ");
      Serial.println(offlineKeys);
      for (byte i = 0; i < offlineKeys_array.size(); i++) {
        const char* config_offlineKeys_token = offlineKeys_array[i]["token"];
        offlineKeys += config_offlineKeys_token;
        offlineKeys += (",");
      }
      preferences.begin("store", false);
      preferences.putString("offlineKeys", offlineKeys);
      preferences.end();
      Serial.println("Offline keys are stored in NVS");
      Serial.print("offlineKeys: ");
      Serial.println(offlineKeys);
      Serial.println();
      Serial.print("There should be total of ");
      Serial.print(offlineKeys_array.size());
      Serial.println(" keys.");
    }
  } else {
    Serial.println("No need to change configVersion in NVS");
  }
  Serial.println();
}


/* get String of device MAC address  */
String getMacAddress(bool device) { // 0 = wifi, 1 = ethernet
  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  if (device == 1) {
    esp_read_mac(baseMac, ESP_MAC_ETH);
  } else {
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  }
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}

/* Sorting devices by HW MAC address of ESP32 */
/* when is matching device not found, it will be used the Test machine, which should not be device in production enviroment */
void setAPIkey() {

  if (mac == "F4:CF:A2:82:FC:D4") {
    API_key = "Bearer some_API_key"; // First device
    machineName = "First device name";
    machineId = 0; //change this number with your real equipment number
    Serial.print("Set to ");
    Serial.println(machineName);
  }
  else if (mac == "F4:CF:A2:82:FC:EC") {
    API_key = "Bearer some_API_key"; // Second device
    machineName = "Second device name";
    machineId = 0; //change this number with your real equipment number
    Serial.print("Set to ");
    Serial.println(machineName);
  }
  else {
    API_key = "Bearer Second device name"; // Test machine
    machineName = "Test machine";
    machineId = 0; //change this number with your real equipment number
    Serial.print("Set to ");
    Serial.println(machineName);
  }
}

void wifiScan() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
}

void setWiFi() {
  if (SSIDString != NULL) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
      ssid[i] = getValue(SSIDString, ',', i);
      Serial.print(ssid[i] + " ");
    }
    Serial.println();
    for (int i = 0; i < ARRAY_SIZE; i++) {
      pass[i] = getValue(passwordString, ',', i);
      Serial.print(pass[i] + " ");
    }
    Serial.println();
    for (int i = 0; i < ARRAY_SIZE; i++) {
      wifiMulti.addAP(ssid[i].c_str(), pass[i].c_str());
      Serial.print("WiFi credentials are: " + pass[i] + "," + ssid[i]);
    }
  } else {
    wifiMulti.addAP("ssid", "pass");
    wifiMulti.addAP("ssid", "pass");
    wifiMulti.addAP("ssid", "pass");
    wifiMulti.addAP("ssid", "pass");
    wifiMulti.addAP("ssid", "pass");
    wifiMulti.addAP("ssid", "pass");
  }
}

// String  var = getValue( StringVar, ',', 2); // if  a,4,D,r  would return D
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length();

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}  // END

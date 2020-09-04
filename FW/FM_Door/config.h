// FW info
const String fwVersion = "0.1.0.2_beta";
String version_url  = "http://your_domain/door_version.txt";
String binURL = "http://your_domain/name_of_your.bin";

// Setup
const String API_key_user = "Bearer some_admin_key"; //admin key
const int setupButtonPin = 4;
const int relayPin = 13;
bool relayOpen = 0;
int relayOpenTime = 5000;
unsigned long lastRelayOpenTime = relayOpenTime;
bool lastDenied = 0;
int lastDeniedTime = 2000;
unsigned long lastLastDeniedTime = lastDeniedTime;
int grantedTimeout = 5000;
unsigned long grantedTimoutLong = grantedTimeout;
bool checkOH = 0; // if this value is 1, it will check for actual membership, not if member has any package

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager (development branch)

class IntParameter : public WiFiManagerParameter {
public:
    IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
        : WiFiManagerParameter("") {
        init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    long getValue() {
        return String(WiFiManagerParameter::getValue()).toInt();
    }
};

char api_key_char[40];
char machine_name_char[40];
int machine_id_int;
int use_lan_int;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

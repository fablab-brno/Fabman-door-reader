extern int granted;

#include <SmartLeds.h> // https://platformio.org/lib/show/1740/SmartLeds

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        17 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // because we are using just one, we are incrementing it by one so it can go through loop

const int CHANNEL = 0;
SmartLed pixels( LED_WS2812B  , NUMPIXELS, PIN, CHANNEL, DoubleBuffer );

unsigned long lastUpdate;
byte previousState = 0;

// breathing pixel variables
unsigned long startTime = 0;
int colorR = 0, colorG = 0, colorB = 0;
int breathTime = 5000; // adjust for your desired speed
float breathTimeFloat = breathTime;
float pi = 3.14;

void setPixelColor(int r, int g, int b, uint8_t wait) {
  for (uint16_t i = 0; i < NUMPIXELS; i++) { //  ******i=0 - 0 is the start pixel
    pixels[i] = Rgb(r, g, b);      // *****change i to led\pixel number
  }
  pixels.show();
}

void neopixelAllowed() {
  Serial.println("neopixelAllowed reached..");
  grantedTimoutLong = millis();
  pixels[0] = Rgb{0, 255, 0};
  pixels.show();   // Send the updated pixel colors to the hardware.
}

void neopixelDenied() {
  Serial.println("neopixelDenied reached..");
  grantedTimoutLong = millis();
  pixels[0] = Rgb{255, 0, 0};
  pixels.show();   // Send the updated pixel colors to the hardware.
}

// initialize the breath timer
void startBreath(int r, int g, int b) {
  Serial.println("startBreath reached..");
  colorR = r;
  colorG = g;
  colorB = b;
  startTime = millis();
}

// call this inside your loop to update the color of the pixels
void updateBreath() {
  const float pi = 3.14;
  float frac; // ratio of color to use, based on time
  int r, g, b;

  // calculate a brightness fraction, based on a sine curve changing over time from 0 to 1 and back
  frac = (sin((((millis() - startTime) % breathTime) / breathTimeFloat - 0.25f) * 2.0f * pi) + 1.0f) / 2.0f;

  // multiply each color by the brightness fraction
  r = colorR * frac;
  g = colorG * frac;
  b = colorB * frac;
  //pixels[0] = Rgb(r, g, b);
  setPixelColor(r, g, b, 0);
  delay(1); // 100us was not working
//  Serial.println(String(r) + ", " + String(g) + ", " + String (b));
}

void startShow(int i) {
  switch (i) {
    case 0:
      startBreath(0, 0, 255); // no action, just breath
      break;
    case 1:
      neopixelAllowed(); // alowed, green light
      break;
    case 2:
      neopixelDenied(); // denied, red light
      break;
  }
}

void neopixelLoop() {
  if (previousState != granted) {
    Serial.println("Granted: " + String(granted));
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels[i] = Rgb(0, 0, 0);
    }
    startShow(granted);
    previousState = granted;
    Serial.println("State changed!");
    Serial.println("Granted: " + String(granted));
  }
}

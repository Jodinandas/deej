#include "FastLED.h"
#include "math.h"
#define DATA_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
// per meter
#define NUM_LEDS 10
#define BRIGHTNESS 10
CRGB leds[NUM_LEDS*2];

// slider defines
const int NUM_SLIDERS = 5;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A4};
int analogSliderValues[NUM_SLIDERS];
unsigned long time_now = 0;

const byte numChars = 32;
char receivedChars[numChars];

boolean newValues = false;

void setup() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }
  Serial.begin(115200);
  delay(3000); // initial delay of a few seconds is recommended
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS*2).setCorrection(TypicalLEDStrip); // initializes LED strip
  FastLED.setBrightness(BRIGHTNESS);// global brightness
}

void loop() {
  time_now = millis();
  // non-blocking delay so that level meter is always prioritized.
  while(millis() < time_now + 50){
    recvWithStartEndMarkers();
    showNewValues();
  }
  updateSliderValues();
  sendSliderValues();
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
     analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String(mapToLinear(resistorCorrection((int)analogSliderValues[i])));

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  
  Serial.println(builtString);
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newValues == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newValues = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewValues() {
    if (newValues == true) {
        newValues = false;

        int i = find_char(receivedChars, sizeof(receivedChars)/sizeof(char), '|');
        if (i < 0 || i > 2) return;

        // converting for first value
        int ia = receivedChars[0] - '0';
        if (receivedChars[1] != '|'){
          ia = 10*ia + int((receivedChars[1] - '0'));
        }

        //converting for second value
        int ib = receivedChars[i+1] - '0';
        if (receivedChars[i+2] != '\0'){
          ib = 10*ib + int((receivedChars[i+2] - '0'));
        }
        set_meters(ia, ib);
    }
}

int find_char(char str[], int length, char c) {
  for (int i = 0; i < length; i++) {
    if (str[i] == c) {
      return i;
    }
  }
  return -1;
}

void set_meters(int levela, int levelb) {
  float vola = float(levela)/100.0;
  int vola_mapped = int(ceil(vola*NUM_LEDS));
  for (int i=0; i<vola_mapped; i++){
    if (i < (NUM_LEDS/2)) {
      leds[i] = CRGB(0,255,0);
    } else if (i < (NUM_LEDS - 2)){
      leds[i] = CRGB(226,180,0);
    } else {
      leds[i] = CRGB(255,0,0);
    }
  }
  for (int i=vola_mapped; i<NUM_LEDS; i++){
    leds[i]= CRGB::Black;
  }
  // second meter
  float volb = float(levelb)/100.0;
  int volb_mapped = int(ceil(volb*NUM_LEDS));
  for (int i=0; i<volb_mapped; i++){
    if (i < (NUM_LEDS/2)) {
      leds[NUM_LEDS + i] = CRGB(0,255,0);
    } else if (i < (NUM_LEDS - 2)){
      leds[NUM_LEDS + i] = CRGB(226,180,0);
    } else {
      leds[NUM_LEDS + i] = CRGB(255,0,0);
    }
  }
  for (int i=volb_mapped; i<NUM_LEDS; i++){
    leds[NUM_LEDS + i]= CRGB::Black;
  }
  FastLED.show();
  delay(10);
}

// correct the incoming values, because we need to use a resistor to keep the potis from overheating
int resistorCorrection(int value){
  return int(6.82*float(value));
}

// the received potentiometers are not linear. This function roughly maps the input data to a linear range
int mapToLinear(int value){
  float val = float(value);
  float pos = 0.0000000000031 * pow(val,4) + 0.0000000038191 * pow(val, 3) - 0.0000113360619 * pow(val, 2) + 0.0102746785178 * val;
  return min(int(pos*167.5), 1023);
}
// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// pin assignments
#define LED_PIN   A5 
#define LED_COUNT 60
#define AUDIO_PIN A0
#define KNOB_PIN  A1
#define BUTTON_1_PIN  6
#define BUTTON_2_PIN  5
#define BUTTON_3_PIN  4

// TODO need to add button LED pins
// TODO need to add second potentiometer
// TODO remove audio pin

// constants for LED brightness and effect speeds
uint8_t WAIT_TIMES[] = {100, 75, 50, 25, 10};
uint8_t BRIGHTNESSES[] = {10, 20, 30, 40, 50};
#define POT_DIVIDER 51                          // converts [0, 255) to [0, 4]

// create array of effect functions to easily cycle through
// NOTE: problem with this method is that every function has
// to have the same function signature (unused parameters at times)
#define NUM_OF_EFFECTS 4
uint8_t effectIndex = 0;
typedef void (*func_pointer)(uint32_t, int);
void colorWipe(uint32_t, int);
void theaterChase(uint32_t, int);
void rainbow(uint32_t, int);
void theaterChaseRainbow(uint32_t, int);
const func_pointer EFFECTS[] = {colorWipe, theaterChase, rainbow, theaterChaseRainbow};

// variables for hardware components
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
uint8_t measuredVolume = 0;
uint8_t measuredPot = 0;        // [0, 255)
bool buttonPressed1 = 0;
bool buttonPressed2 = 0;
bool buttonPressed3 = 0;

// array of chosen colors to cycle through
# define NUM_OF_COLORS 12
uint8_t colorIndex = 0;
uint32_t COLORS[NUM_OF_COLORS];

// variables for calculations (start at reasonable values
uint8_t wait_time = 25;
uint8_t brightness = 10;      // shouldn't change this after initialization... but works fine

void setup() {
  // start serial data transmision
  Serial.begin(9600);

  // create colors that can be be cycled through
  COLORS[0] = strip.Color(  0, 127, 255);     // light blue
  COLORS[1] = strip.Color(  0,   0, 255);     // dark blue
  COLORS[2] = strip.Color(127,   0, 255);     // violet
  COLORS[3] = strip.Color(255,   0, 255);     // hot pink
  COLORS[4] = strip.Color(255,   0, 127);     // pink
  COLORS[5] = strip.Color(255,   0,   0);     // red
  COLORS[6] = strip.Color(255, 127,   0);     // orange
  COLORS[7] = strip.Color(255, 255,   0);     // yellow
  COLORS[8] = strip.Color(127, 255,   0);     // light green
  COLORS[9] = strip.Color(  0, 255,   0);     // dark green
  COLORS[10] = strip.Color(  0, 255, 127);    // lime green
  COLORS[11] = strip.Color(  0, 255, 255);    // cyan

  // initialize button pins
  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
  pinMode(BUTTON_3_PIN, INPUT);
  digitalWrite(BUTTON_1_PIN, HIGH);
  digitalWrite(BUTTON_2_PIN, HIGH);
  digitalWrite(BUTTON_3_PIN, HIGH);

  // initialize LED strip
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)

  // main program loop ready to start
  Serial.write("Initialization complete!");
}

void loop() {
  // store pin values
  measuredPot = analogRead(KNOB_PIN);
  measuredVolume = analogRead(AUDIO_PIN);
  buttonPressed1 = digitalRead(BUTTON_1_PIN) == 0;
  buttonPressed2 = digitalRead(BUTTON_2_PIN) == 0;
  buttonPressed3 = digitalRead(BUTTON_3_PIN) == 0;

  // display sound value
  Serial.println(measuredPot);
  Serial.println();

  // calculate display speed and brightness
  int index = measuredPot / POT_DIVIDER;
  wait_time = WAIT_TIMES[index];
  brightness = BRIGHTNESSES[index];
  Serial.println();

  // cycle through colors
  if(buttonPressed1){
    colorIndex += 1;
    if(colorIndex == NUM_OF_COLORS){
      colorIndex = 0;
    }
  }
  
  // cycle through effects
  if(buttonPressed2){
    effectIndex += 1;
    if(effectIndex == NUM_OF_EFFECTS){
      effectIndex = 0;
    }
  }
  
  // start pomodoro timer
  if(buttonPressed3){
    // TODO implement
  }

  // show chosen effects
  strip.setBrightness(brightness);
  EFFECTS[effectIndex](COLORS[colorIndex], wait_time);

  // check changes in input approximately once per second
  delay(1000);
}

// ------------- EFFECT FUNCTIONS --------------



// TODO CHANGE ALL THESE EFFECTS TO WORK WITH MY NEW STRUCTURE, NOT STRIP



// fill strip with single color
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

// Theater-marquee-style chasing lights
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {
    for(int b=0; b<3; b++) {
      strip.clear();
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color);
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
// TODO the wait being passed currently is way to long, need to divide by some constant
void rainbow(uint32_t color, int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i < strip.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
// TODO the wait being passed currently is way to long, need to divide by some constant
void theaterChaseRainbow(uint32_t color, int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

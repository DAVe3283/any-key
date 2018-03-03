// -----------------------------------------------------------------------------
// any-key-teensy.ino
//
// The firmware for a Teensy 3.2 (or similar) to run the key.
//
// Teensy should be configured to USB Keyboard mode.
// -----------------------------------------------------------------------------

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 0

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <Bounce2.h>
#include <FastLED.h>

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// Pins
const uint8_t led_pin(13);
const uint8_t switch_pin(2);
const uint8_t led_data_pin(7);
const uint8_t led_clock_pin(14);

// LED(s)
const size_t num_leds(1);
const CRGB led_default_color(CRGB::White);

// Gamma correction table
// Taken from https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// -----------------------------------------------------------------------------
// Declarations
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
Bounce button = Bounce();
CRGB leds[num_leds];

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

// Initial setup routine
void setup()
{
  // Configure I/O
  pinMode(led_pin, OUTPUT);
  pinMode(switch_pin, INPUT_PULLUP);

  // Configure button debounce
  button.attach(switch_pin);
  button.interval(5); // interval in ms

  // Configure LED(s)
  FastLED.addLeds<APA102, led_data_pin, led_clock_pin, BGR>(leds, num_leds);
  for (size_t i(0); i < num_leds; ++i)
  {
    leds[i] = led_default_color;
  }
  digitalWrite(led_pin, LOW);
  FastLED.show();
}

bool down = false;
// Main program loop
void loop()
{
  if (button.update())
  {
    // Button was pressed down
    if (button.fell())
    {
      Keyboard.press(KEY_CAPS_LOCK);
      down = true;
      leds[0] = CRGB::Blue;
    }

    // Button was released
    else if (button.rose())
    {
      Keyboard.release(KEY_CAPS_LOCK);
      down = false;
    }
  }

  if (!down)
  {
      int val = millis();
      val /= 10;
      leds[0].r = gamma8[sin8(val/2)];
      leds[0].g = gamma8[triwave8(val/5)];
      leds[0].b = gamma8[cos8(val/3)];
  }

  FastLED.show();
}

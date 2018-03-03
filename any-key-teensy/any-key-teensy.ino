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

// Main program loop
void loop()
{
  if (button.update())
  {
    // Button was pressed down
    if (button.fell())
    {
      Keyboard.press(KEY_CAPS_LOCK);
      leds[0] = CRGB::Blue;
    }

    // Button was released
    else if (button.rose())
    {
      Keyboard.release(KEY_CAPS_LOCK);
      leds[0] = CRGB::Yellow;
    }

    FastLED.show();
  }
}

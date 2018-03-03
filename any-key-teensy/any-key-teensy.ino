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
#include "FastLED.h"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// Pins
const uint8_t led_pin(13);
const uint8_t switch_pin(2);
const uint8_t data_pin(7);
const uint8_t clk_pin(14);
const uint8_t num_leds(1);

// -----------------------------------------------------------------------------
// Declarations
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
Bounce debouncer = Bounce();
CRGB leds[num_leds];

// -----------------------------------------------------------------------------
// Function Definitions
// -----------------------------------------------------------------------------

// Initial setup routine
void setup()
{
  // Configure LED
  pinMode(led_pin, OUTPUT);
  pinMode(switch_pin, INPUT_PULLUP);

  debouncer.attach(switch_pin);
  debouncer.interval(5); // interval in ms

  FastLED.addLeds<APA102, data_pin, clk_pin, BGR>(leds, num_leds);
}

// Main program loop
void loop()
{
  debouncer.update();
  int value = debouncer.read();
  if ( value == LOW ) {
    digitalWrite(led_pin, HIGH );
    leds[0] = CRGB::Blue;
  } 
  else {
    digitalWrite(led_pin, LOW );
    leds[0] = CRGB::Red;
  }

  FastLED.show();
  //Keyboard.print("Hello World "); 
  //Keyboard.println(count);
  //delay(5000);
}

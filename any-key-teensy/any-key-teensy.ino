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

// Key
const uint16_t key(KEY_MEDIA_PLAY_PAUSE);
const CRGB key_down_color(CRGB::Yellow);

// Color rotation rate
const uint32_t led_color_rate(20); // milliseconds between color steps

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

// Button state
Bounce button = Bounce();
bool button_down(false);

// LED state
CRGB led_color;
elapsedMillis led_timer;
int led_val(-1);


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
  FastLED.addLeds<APA102, led_data_pin, led_clock_pin, BGR>(&led_color, 1);
  digitalWrite(led_pin, LOW);
}

// Main program loop
void loop()
{
  // Check for button activity
  if (button.update())
  {
    // Button was pressed down
    if (button.fell())
    {
      Keyboard.press(key);
      button_down = true;
      led_color = key_down_color;
      FastLED.show();
    }

    // Button was released
    else if (button.rose())
    {
      Keyboard.release(key);
      button_down = false;
      led_timer = led_color_rate; // Force a color update
    }
  }

  // Rotate LED color when idle
  if (!button_down && (led_timer >= led_color_rate))
  {
    led_timer -= led_color_rate;
    led_val++;

    // Calculate new color
    led_color.r = gamma8[sin8(led_val/2)];
    led_color.g = gamma8[triwave8(led_val/5)];
    led_color.b = gamma8[cos8(led_val/3)];
    FastLED.show();
  }
}

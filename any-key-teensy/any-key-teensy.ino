// -----------------------------------------------------------------------------
// any-key-teensy.ino
//
// The firmware for a Teensy 3.2 (or similar) to run the key.
//
// Compile Options:
// - USB Type: Keyboard
// - CPU Speed: 24 MHz (or faster)
// - Optimize: Faster with LTO (other probably work too)
// -----------------------------------------------------------------------------

#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VERSION_REVISION 0

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <Bounce2.h>
#include <FastLED.h>
#include <float.h> // for FLT_MAX
#include "MAC_Address.h"
#include "flashbits.h"
#include "fast_type.h"
#include "I_Robot.h"


// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// How much resolution do we want on the analog ports, in bits?
#define ANALOG_RESOLUTION_BITS 16

// Verify analog resolution is reasonable
#if ANALOG_RESOLUTION_BITS <= 0
  #error ANALOG_RESOLUTION_BITS must be > 0.
#endif
#if ANALOG_RESOLUTION_BITS < 8
  #warning ANALOG_RESOLUTION_BITS is < 8. There is no reason to do this.
  // Anything <= 8 configures the hardware to 8 bits. Therefore, there is no
  // benefit to using less than 8 bits of analog resolution.
#endif
#if ANALOG_RESOLUTION_BITS > 16
  #error ANALOG_RESOLUTION_BITS is > 16, but the ADC is only 16 bits.
#endif

// Pins
const uint8_t led_pin(13);
const uint8_t switch_pin(2);
const uint8_t led_data_pin(7);
const uint8_t led_clock_pin(14);
const uint8_t analog_temperature_pin(38);

// Key
const uint16_t key(KEY_MEDIA_PLAY_PAUSE);
const CRGB key_down_color(CRGB::Yellow);
const CRGB debug_color(CRGB::Red);

// Color rotation rate
const uint32_t led_color_rate(20); // milliseconds between color steps

// Temperature measurement delay between samples (milliseconds)
const uint32_t temperature_delay(1000);

// Debug delays (milliseconds)
const uint32_t debug_delay(10 * 1000);
const uint32_t e_egg_delay(30 * 1000);

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

// Measure current temperature of the Teensy 3.x
void measure_temperature();

// Get uptime
uint64_t get_uptime();

// Print (really type out) diagnostic information
void print_diagnostics();

// Print LZSS compressed string
void print_lzss(const prog_uchar* compressed);

// Update a rotating buffer with new data
void buffer_update(char *const buffer, const size_t& buffer_size, size_t& buffer_used, const char *const new_data, const size_t& new_size);


// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

// Button state
Bounce button = Bounce();
bool button_down(false);
elapsedMillis button_down_timer;

// LED state
CRGB led_color;
elapsedMillis led_timer;
int led_val(-1);

// MAC Address
MAC_Address mac_address;

// Temperature & maximum temperature measured
elapsedMillis temperature_timer;
float temperature(-FLT_MAX);
float temperature_max(-FLT_MAX);

// Uptime! Because why not?
uint32_t uptime_last(0);      // Last seen uptime value (to catch overflow).
uint32_t uptime_overflows(0); // millis() 32-bit overflows (every 49.7 days).
// 64-bits worth of uptime gives us 584,554,531 years. Should be enough. :P

// Debug mode / Easter Egg
bool debug(false);
char e_egg_buffer[i_robot_required_buffer_size] = { 0 };


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

  // Configure ADC
  analogReadAveraging(32);                                                      // Let the hardware take care of averaging!
  analogReadRes(ANALOG_RESOLUTION_BITS);                                        // Set measurment resolution (in bits)

  // Initial temperature measurement
  measure_temperature();

  // Check for debug mode
  // No need to debounce in this case, button has been held down a "long" time
  if (digitalRead(switch_pin) == LOW)
  {
    debug = true;
    button_down = true;
    led_color = debug_color;
    FastLED.show();
  }
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
      button_down_timer = 0;
      led_color = key_down_color;
      FastLED.show();
    }

    // Button was released
    else if (button.rose())
    {
      Keyboard.release(key);
      button_down = false;

      // Debug
      if (debug)
      {
        led_color = debug_color;
        FastLED.show();

        // Print debug info
        if (button_down_timer >= debug_delay)
        {
          print_diagnostics();
        }

        // Print easter egg text (yes, also)
        if (button_down_timer >= e_egg_delay)
        {
          // Decompress and print I, Robot by Isaac Asimov
          print_lzss(i_robot, e_egg_buffer, sizeof(e_egg_buffer)/sizeof(*e_egg_buffer));
        }
      }

      // Force an immediate color update & prevent wind-up from debug printing
      led_timer = led_color_rate;
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

  // Periodically measure temperature & stuff
  if (temperature_timer >= temperature_delay)
  {
    temperature_timer -= temperature_delay;
    measure_temperature();

    // Update uptime, to check for millis() overflow
    get_uptime();
  }
}

void measure_temperature()
{
  // Analog conversion calibration is dependant on how many bits we use!
  const float resolution_correction_factor(1 << (16 - ANALOG_RESOLUTION_BITS));

  // Constants for temperature calculation:
  // From http://dangerousprototypes.com/forum/viewtopic.php?f=2&t=4606#p45039
  const float gradient(-35.7f / resolution_correction_factor);                  // Counts per °C
  const float intercept(15296.37f / resolution_correction_factor);              // Counts at 0°C

  // Measure and calculate temperature
  const float analogValue(analogRead(analog_temperature_pin));
  temperature = (analogValue - intercept) / gradient;

  // Update maximum temperature
  if (temperature > temperature_max)
  {
    temperature_max = temperature;
  }
}

// Get the uptime and check for overflows
uint64_t get_uptime()
{
  // Get current uptime
  const uint32_t uptime_current(millis());

  // Check for overflow
  if (uptime_current < uptime_last)
  {
    // Overflow!
    uptime_overflows++;
    // Let the overflow counter wrap every 584,554,531 years. I'll fix it then.
  }

  // Store last seen uptime (for overflow detection)
  uptime_last = uptime_current;

  // Return the 64-bit uptime
  return (static_cast<uint64_t>(uptime_overflows) << 32) | static_cast<uint64_t>(uptime_current);
}

void print_diagnostics()
{
  // Degree symbol
  const uint8_t degree[] = { 0, 1, 7, 6, };

  // Header
  Keyboard.printf("any-key-teensy v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
  Keyboard.println("https://github.com/DAVe3283/any-key");
  Keyboard.println();
  Keyboard.printf("Compiled %s %s\n", __DATE__, __TIME__);
  Keyboard.println();
  Keyboard.println("Teensy 3.x Hardware Information:");

  // Unique ID
  Keyboard.printf("- CPU Unique ID: %08lX-%08lX-%08lX-%08lX\n", SIM_UIDH, SIM_UIDMH, SIM_UIDML, SIM_UIDL);

  // MAC Address
  Keyboard.print("- MAC Address: ");
  Keyboard.println(mac_address);

  // Temperature
  Keyboard.printf("- Temperature: %.1f", temperature);
  FT.TypeAltCode(degree, sizeof(degree)/sizeof(*degree));
  Keyboard.printf("C (maximum since startup: %.1f", temperature_max);
  FT.TypeAltCode(degree, sizeof(degree)/sizeof(*degree));
  Keyboard.println("C)");

  // Uptime
  const uint64_t uptime(get_uptime());
  const unsigned int ms((uptime                                             ) % 1000);
  const unsigned int s ((uptime / (1000ull)                                 ) %   60);
  const unsigned int m ((uptime / (1000ull * 60ull)                         ) %   60);
  const unsigned int h ((uptime / (1000ull * 60ull * 60ull)                 ) %   24);
  const unsigned int d ((uptime / (1000ull * 60ull * 60ull * 24ull)         ) %  365);
  const uint32_t     y ((uptime / (1000ull * 60ull * 60ull * 24ull * 365ull))       );
  // Keyboard.printf("- Uptime: %llu ms (%lu:%u:%u:%u:%02u.%03u)\n", uptime, y, d, h, m, s, ms);
  Keyboard.printf("- Uptime: %llu ms (", uptime);
  if (y)
  {
    Keyboard.printf("%lu:", y);
  }
  if (d || y)
  {
    Keyboard.printf((y ? "%03u:" : "%u:"), d);
  }
  Keyboard.printf("%02u:%02u:%02u.%03u)\n\n", h, m, s, ms);
}

void print_lzss(const prog_uchar* compressed, char *const buffer, const size_t& buffer_size)
{
  // This is loosely based on http://excamera.com/sphinx/article-compression.html
  // which is a particularly meh LZSS scheme with an improbably slow compressor.
  // One notable change is I assumed all data is ASCII text, and I store 7 bit
  // literals, so high ASCII will get corrupted. But since I control the source
  // content, this isn't a problem.
  // Since this only operates on one hard-coded dataset, I do almost no error
  // checking, bounds checking, etc.
  // If you feed this arbitrary data, bad things WILL happen.

  // Read header
  BS.begin(compressed);
  const size_t offset_bits(BS.getn<size_t>( 4)); // Number of bits for offset field
  const size_t length_bits(BS.getn<size_t>( 4)); // Number of bits for length field
  const size_t min_length( BS.getn<size_t>( 2)); // Minimum length of a back-reference
  const size_t num_chunks( BS.getn<size_t>(32)); // Number of chunks in the stream

  // Verify we have a big enough buffer
  const size_t required_buffer_size(static_cast<size_t>(1) << offset_bits);
  if (buffer_size < required_buffer_size)
  {
    Keyboard.println("Insufficient buffer size to decompress LZSS string!");
    Keyboard.printf("Buffer is %lu bytes, but archive requires %lu bytes.\n", buffer_size, required_buffer_size);
    return;
  }
  char backreference_buffer[(1 << length_bits) + min_length + 1]; // Add 1 for terminating null

  // Decompress chunks
  size_t buffer_used(0);
  for (size_t chunk(0); chunk < num_chunks; ++chunk)
  {
    // Back-reference chunk
    if (BS.get1())
    {
      // Indicate what kind of chunk we are dealing with
      if (led_color != static_cast<CRGB>(CRGB::Blue))
      {
        led_color = CRGB::Blue;
        FastLED.show();
      }

      // Get the back-reference
      const size_t offset(BS.getn<size_t>(offset_bits) + 1);
      const size_t length(BS.getn<size_t>(length_bits) + min_length);
      const char *const backreference(buffer + buffer_used - offset);
      memcpy(backreference_buffer, backreference, length);

      // Print the back-reference
      FT.Type(backreference_buffer, length);

      // Update the buffer
      buffer_update(buffer, buffer_size, buffer_used, backreference_buffer, length);
    }

    // Literal chunk
    else
    {
      if (led_color != static_cast<CRGB>(CRGB::Red))
      {
        led_color = CRGB::Red;
        FastLED.show();
      }

      const char c(BS.getn<char>(7)); // Get the literal character from the stream
      FT.Type(c);
      buffer_update(buffer, buffer_size, buffer_used, &c, 1); // Update buffer
    }

    // Allow button press to abort
    if (button.update() && button.fell())
    {
      Keyboard.println();
      Keyboard.println();
      Keyboard.println("---- Aborted! ----");
      Keyboard.println();
      button_down = true;
      button_down_timer = 0;
      led_color = debug_color;
      FastLED.show();
      FT.EndSession();
      return;
    }
  }
  FT.EndSession();
}

void buffer_update(char *const buffer, const size_t& buffer_size, size_t& buffer_used, const char *const new_data, const size_t& new_size)
{
  // This function does no error checking. You ask for stupid things, you get them.
  // Assumptions this function makes:
  // - new_size <= buffer_size
  // - buffer and new_data don't overlap
  // - probably more, I am too lazy to think of other possible side-effects

  // Rotate buffer by enough to fit the new content
  if (new_size > (buffer_size - buffer_used))
  {
    memmove(buffer, buffer + new_size, buffer_size - new_size);
    buffer_used -= new_size;
  }

  // Add new content
  memcpy(buffer + buffer_used, new_data, new_size);
  buffer_used += new_size;
}

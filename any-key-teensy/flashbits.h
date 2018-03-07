////////////////////////////////////////////////////////////////////////////////
// Get bits from a stream stored in Flash memory
// Based on: http://excamera.com/sphinx/article-compression.html
////////////////////////////////////////////////////////////////////////////////

// Header guard
#if !defined __FLASHBITS_H
#define __FLASHBITS_H

#include <arduino.h>

class Flashbits {
public:
  // Attach the bitstream to a memory address
  void begin(const prog_uchar* s) {
    src = s;
    mask = 0x01;
  }

  // Get a single bit from the stream
  uint8_t get1() {
    const uint8_t r((pgm_read_byte_near(src) & mask) != 0);
    mask <<= 1;
    if (!mask) {
      mask = 1;
      src++;
    }
    return r;
  }

  // Gen n bits from the stream
  template <typename T>
  T getn(size_t n) {
    T r(0);
    while (n--) {
      r <<= 1;
      r |= get1();
    }
    return r;
  }

private:
  const prog_uchar* src;
  uint8_t mask;
};

static Flashbits BS;

#endif // __FLASHBITS_H

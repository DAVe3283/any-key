////////////////////////////////////////////////////////////////////////////////
// FastType
// Author: DAVe3283
//
// Class that pulls cheap tricks to get closer to 1000 characters per second
// through the USB Keyboard, instead of the normal 500.
////////////////////////////////////////////////////////////////////////////////

// Header guard
#if !defined __FAST_TYPE_H
#define __FAST_TYPE_H

#include <arduino.h>

class FastType {
public:
	// Constructor
	FastType();

	// Type a single character
	void Type(const char& c);

	// Type a string
	void Type(const char* c, const size_t& length);

	// Type an Alt code (e.g. Alt+0176)
	void TypeAltCode(const uint8_t* code, const size_t& size);

	// End typing session (cleans up any held-down keys)
	void EndSession();

private:
	// Clear keyboard state back to default
	void ClearState();

	// Send a keycode
	void SendRaw(const uint16_t& key, const uint16_t& modifier = 0);

	// Send a character as a keystroke
	void SendKey(const char& c);

	// State variables
	bool first_key;
	uint16_t modifier_state;
	uint16_t previous_key;
};

static FastType FT;

#endif // __FAST_TYPE_H

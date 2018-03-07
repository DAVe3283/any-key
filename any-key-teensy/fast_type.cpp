////////////////////////////////////////////////////////////////////////////////
// FastType
// Author: DAVe3283
//
// Class that pulls cheap tricks to get closer to 1000 characters per second
// through the USB Keyboard, instead of the normal 500.
////////////////////////////////////////////////////////////////////////////////

#include "fast_type.h"

FastType::FastType()
	: first_key(true)
	, modifier_state(0)
	, previous_key(0)
{}

void FastType::Type(const char& c)
{
	SendKey(c);
}

void FastType::Type(const char* c, const size_t& length)
{
	for (size_t i(0); i < length; ++i)
	{
		SendKey(c[i]);
	}
}

void FastType::TypeAltCode(const uint8_t* code, const size_t& size)
{
	// Send code
	for (size_t i(0); i < size; ++i)
	{
		// Get keycode
		uint16_t key(0);
		switch (code[i])
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				key = KEYPAD_1 + (code[i] - 1);
				break;

			case 0:
				key = KEYPAD_0;
				break;

			// Skip invalid keys
			default:
				continue;
		}

		// Send keycode
		SendRaw(key, MODIFIERKEY_ALT);
	}

	// Cleanup after last character
	ClearState();
}

void FastType::EndSession()
{
	ClearState();
}

void FastType::ClearState()
{
	Keyboard.set_modifier(0);
	Keyboard.set_key1(0);
	Keyboard.set_key2(0);
	Keyboard.send_now();
	first_key = true;
	modifier_state = 0;
	previous_key = 0;
}

void FastType::SendRaw(const uint16_t& key, const uint16_t& modifier)
{
	// If the modifier state changes, or we are hammering one key, we need to flush
	// Otherwise, the OS can assume we reported data differently and skips keys
	if ((key == previous_key) || (modifier != modifier_state))
	{
		// Release all keys
		Keyboard.set_key1(0);
		Keyboard.set_key2(0);
		Keyboard.send_now();
		first_key = true;

		// Update modifier state
		Keyboard.set_modifier(modifier);
		modifier_state = modifier;
	}
	previous_key = key;

	// Press key & release previous key
	Keyboard.set_key1(first_key ? key : 0);
	Keyboard.set_key2(first_key ? 0 : key);
	Keyboard.send_now();
	first_key = !first_key;
}

void FastType::SendKey(const char& c)
{
	// Get keycode
	uint16_t key(0);
	uint16_t mod(0);
	switch (c)
	{
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
			key = KEY_A + (c - 'a');
			break;

		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			key = KEY_A + (c - 'A');
			mod = MODIFIERKEY_SHIFT;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			key = KEY_1 + (c - '1');
			break;
		case '0':
			key = KEY_0;
			break;

		case '!':
			key = KEY_1;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '@':
			key = KEY_2;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '#':
			key = KEY_3;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '$':
			key = KEY_4;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '%':
			key = KEY_5;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '^':
			key = KEY_6;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '&':
			key = KEY_7;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '*':
			key = KEY_8;
			mod = MODIFIERKEY_SHIFT;
			break;
		case '(':
			key = KEY_9;
			mod = MODIFIERKEY_SHIFT;
			break;
		case ')':
			key = KEY_0;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '\n':
			key = KEY_ENTER;
			// mod = (modifier_state & MODIFIERKEY_SHIFT); // Enter & Shift+Enter aren't always equivalent
			break;

		case '\x1B':
			key = KEY_ESC;
			break;

		case '\b':
			key = KEY_BACKSPACE;
			mod = (modifier_state & MODIFIERKEY_SHIFT); // Backspace & Shift+Backspace are treated the same
			break;

		case '\t':
			key = KEY_TAB;
			break;

		case ' ':
			key = KEY_SPACE;
			mod = (modifier_state & MODIFIERKEY_SHIFT); // Space & Shift+Space are treated the same
			break;

		case '-':
			key = KEY_MINUS;
			break;
		case '_':
			key = KEY_MINUS;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '=':
			key = KEY_EQUAL;
			break;
		case '+':
			key = KEY_EQUAL;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '[':
			key = KEY_LEFT_BRACE;
			break;
		case '{':
			key = KEY_LEFT_BRACE;
			mod = MODIFIERKEY_SHIFT;
			break;

		case ']':
			key = KEY_RIGHT_BRACE;
			break;
		case '}':
			key = KEY_RIGHT_BRACE;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '\\':
			key = KEY_BACKSLASH;
			break;
		case '|':
			key = KEY_BACKSLASH;
			mod = MODIFIERKEY_SHIFT;
			break;

		case ';':
			key = KEY_SEMICOLON;
			break;
		case ':':
			key = KEY_SEMICOLON;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '\'':
			key = KEY_QUOTE;
			break;
		case '"':
			key = KEY_QUOTE;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '`':
			key = KEY_TILDE;
			break;
		case '~':
			key = KEY_TILDE;
			mod = MODIFIERKEY_SHIFT;
			break;

		case ',':
			key = KEY_COMMA;
			break;
		case '<':
			key = KEY_COMMA;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '.':
			key = KEY_PERIOD;
			break;
		case '>':
			key = KEY_PERIOD;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '/':
			key = KEY_SLASH;
			break;
		case '?':
			key = KEY_SLASH;
			mod = MODIFIERKEY_SHIFT;
			break;

		case '\r':
			key = KEY_HOME;
			break;

		// Ignore invalid / unknown characters
		default:
			return;
	}

	// Send keycode
	SendRaw(key, mod);
}

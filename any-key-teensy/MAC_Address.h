////////////////////////////////////////////////////////////////////////////////
// Retrieve Ethernet MAC from Teensy 3
// Authors: cmason, Gene Reeves, DAVe3283
// From: https://forum.pjrc.com/threads/91-teensy-3-MAC-address
////////////////////////////////////////////////////////////////////////////////

// Header guard
#if !defined MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <Arduino.h> // for Printable

class MAC_Address : public Printable
{
protected:
    uint8_t _m[6];

public:
    MAC_Address()
        : _m { 0 }
    {
        // Disable interrupts
        cli();

        // Retrieve the 6 byte MAC address Paul burnt into two 32 bit words
        // at the end of the "READ ONCE" area of the flash controller.
        read(0xe,0);
        read(0xf,3);

        // Enable interrupts
        sei();
    }
    
    // gives access to individual elements 
    uint8_t operator[](int index) const { return _m[index]; }
    uint8_t& operator[](int index) { return _m[index]; }
    
    // gives access to uint8_t array, use like this: Ethernet.begin(mac_address)
    operator uint8_t *() { return &(_m[0]); }

    virtual size_t printTo(Print & p) const
    {
        size_t count(0);
        for (uint8_t i(0); i < 6; ++i)
        {
            if (i != 0) { count += p.print(":"); }
            count += p.print((*(_m+i) & 0xF0) >> 4, 16);
            count += p.print(*(_m+i) & 0x0F, 16);
        }
        return count;
    }

    friend class EthernetClass;
    friend class UDP;
    friend class Client;
    friend class Server;
    friend class DhcpClass;
    friend class DNSClient;

private:
    void read(uint8_t word, uint8_t loc)
    {
        // To understand what's going on here, see
        // "Kinetis Peripheral Module Quick Reference" page 85 and
        // "K20 Sub-Family Reference Manual" page 548.

        FTFL_FCCOB0 = 0x41;             // Selects the READONCE command
        FTFL_FCCOB1 = word;             // read the given word of read once area
                                        // -- this is one half of the mac addr.
        FTFL_FSTAT = FTFL_FSTAT_CCIF;   // Launch command sequence
        while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) { } // Wait for command completion
        *(_m+loc) =   FTFL_FCCOB5;       // collect only the top three bytes,
        *(_m+loc+1) = FTFL_FCCOB6;       // in the right orientation (big endian).
        *(_m+loc+2) = FTFL_FCCOB7;       // Skip FTFL_FCCOB4 as it's always 0.
    }
};

#endif // MAC_ADDRESS_H
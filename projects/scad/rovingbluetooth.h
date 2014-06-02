#ifndef SRLM_PROPGCC_ROVING_BLUETOOTH_H__
#define SRLM_PROPGCC_ROVING_BLUETOOTH_H__

#include "libpropeller/serial/serial.h"
#include "libpropeller/pin/pin.h"

class Bluetooth : public libpropeller::Serial {
public:

    void Start(const int rxpin, const int txpin, const int ctspin, const int connectpin) {
        connection = libpropeller::Pin(connectpin);
        connection.input();
        libpropeller::Serial::Start(rxpin, txpin, kBAUD, ctspin);
    }

    void Put(const char character) {
        if (connection.input() == 1) {
            libpropeller::Serial::Put(character);
        }
    }

    int Put(const char * buffer_pointer, const int count) {
        if (connection.input() == 1) {
            return libpropeller::Serial::Put(buffer_pointer, count);
        } else {
            return 0;
        }
    }

    int Put(const char * buffer_pointer) {
        if (connection.input() == 1) {
            return libpropeller::Serial::PutFormatted(buffer_pointer);
        } else {
            return 0;
        }
    }
    
    //TODO(SRLM): Add PutFormatted!!!

private:
    static const int kBAUD = 460800;
    libpropeller::Pin connection;

};

#endif //SRLM_PROPGCC_ROVING_BLUETOOTH_H__

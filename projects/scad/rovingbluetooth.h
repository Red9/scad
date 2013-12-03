#ifndef SRLM_PROPGCC_ROVING_BLUETOOTH_H__
#define SRLM_PROPGCC_ROVING_BLUETOOTH_H__

#include "libpropeller/serial/serial.h"
#include "libpropeller/pin/pin.h"

class Bluetooth : public Serial {
public:

    void Start(const int rxpin, const int txpin, const int ctspin, const int connectpin) {
        connection = Pin(connectpin);
        connection.input();
        Serial::Start(rxpin, txpin, kBAUD, ctspin);
    }

    void Put(const char character) {
        if (connection.input() == 1) {
            Serial::Put(character);
        }
    }

    int Put(const char * buffer_pointer, const int count) {
        if (connection.input() == 1) {
            return Serial::Put(buffer_pointer, count);
        } else {
            return 0;
        }
    }

    int Put(const char * buffer_pointer) {
        if (connection.input() == 1) {
            return Serial::PutFormatted(buffer_pointer);
        } else {
            return 0;
        }
    }


private:
    static const int kBAUD = 460800;
    Pin connection;

};

#endif //SRLM_PROPGCC_ROVING_BLUETOOTH_H__

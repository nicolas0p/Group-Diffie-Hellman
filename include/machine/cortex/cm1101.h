// EPOS CM1101 Sensor Mediator Declarations

#ifndef __cm1101_h
#define __cm1101_h

#include <uart.h>

__BEGIN_SYS

class CM1101
{
public:
    enum Status {
        OK      = 0x00,
        BAD_LEN = 0x01,
        BAD_CMD = 0x02,
        BAD_STT = 0x03,
        BAD_CRC = 0x04
    };

public:
    CM1101(UART * u): _uart(u), _firmware_version(0), _co2(0),
          _temp(0), _humid(0), _status(OK) { check_firmware_version(); }

    ~CM1101() {
        if(!_firmware_version)
            delete[] _firmware_version;
    }

    char * firmware_version() { return _firmware_version; }

    // FIXME: Results doesn't match protocol specification (contact supplier)
    unsigned int serial_number();

    int co2() {
        update_data();
        return _co2;
    }

    int temp() {
        update_data();
        return _temp;
    }

    int humid() {
        update_data();
        return _humid;
    }

    char status() { return _status; }

    int get() { return co2(); }
    int sample() { return co2(); }

    bool enable() { return false; /*return _uart.enable(); */}
    void disable() { /*_uart.disable(); */ }
    //bool data_ready() { return _uart->has_data(); }

private:
    void check_firmware_version() {
        send_command(0x1E);
        char ck = _uart->get(); // ACK/NACK

        if(ck == 0x16) { // ACK
            char len = _uart->get();
            _firmware_version = new (SYSTEM) char[len-1];
            char cmd = _uart->get();
            char cs = ck + len + cmd;

            for(int i = 0; i < (len-1); i++) {
                _firmware_version[i] = _uart->get();
                cs += _firmware_version[i];
            }

            char c = _uart->get();

            if(c == (0x100 - cs)) {
                _status = OK;
                return; // SUCCESS
            }
        }

        // ELSE
        _firmware_version = new (SYSTEM) char[5];

        if(ck == 0x16) { // BAD CS
            _status = BAD_CRC;
            memcpy(_firmware_version, "CSerr", 5);
        } else { // NAK
            char len = _uart->get();
            char cmd = _uart->get();
            char err = _uart->get();
            char cs = _uart->get();

            if(cs == (0x100 - (ck+len+cmd+err))) {
                _status = (Status)err;
                memcpy(_firmware_version, "NACK", 4);
                _firmware_version[4] = err;
            } else {
                _status = BAD_CRC;
                memcpy(_firmware_version, "CSerr", 5);
            }
        }
    }

    void update_data() {
        send_command(0x01);
        char ck = _uart->get(); // ACK/NACK

        if(ck == 0x06) { // NACK
            char len = _uart->get();
            char cmd = _uart->get();
            char err = _uart->get();
            char cs = _uart->get();
            if(cs == (0x100 - (ck + len + cmd + err)))
                _status = (Status)err;
            else
                _status = BAD_CRC;
        }

        // ACK!
        char len = _uart->get();
        char cmd = _uart->get(); // CMD
        char * resp = new char[len-1];
        char cs = ck + len + cmd;

        for(int i = 0; i < (len - 1); i++) {
            resp[i] = _uart->get();
            cs += resp[i];
        }

        char c = _uart->get();

        // FAIL
        if(c != (0x100 - cs)) {
            _status = BAD_CRC;
            delete[] resp;
            return;
        }

        // TODO: CM1101 v1.21 measures only CO2 concentration, v3.03 also measures
        // temperature, check sensor version before writing to _temp and _humid
        _co2 = resp[0]*0x100 + resp[1];
        // Temperature seem to be in Fahrenheit despite the manual stating that
        // they're in Celsius!
        _temp = 10*((resp[2]*0x100 + resp[3])/10.0 - 32)/18;
        //_humid = (resp[4]*0x100 + resp[5])/10;
        _status = OK;

        delete[] resp;
    }

    void send_command(unsigned char cmd, unsigned char * df = 0,
            unsigned int df_len = 0) {
        unsigned char df_sum = 0;

        _uart->put(0x11);
        _uart->put(df_len + 1);
        _uart->put(cmd);

        for(unsigned int i = 0; i < df_len; i++) {
            _uart->put(df[i]);
            df_sum += df[i];
        }

        _uart->put(0x100 - (0x11 + (df_len + 1) + cmd + df_sum));
    }

private:
    UART * _uart;
    char * _firmware_version;
    int _co2;
    int _temp;
    int _humid;
    char _status;
};

__END_SYS

#endif

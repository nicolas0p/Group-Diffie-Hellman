// EPOS Quectel M95 GPRS NIC Mediator Implementation

#include <system/config.h>
#if defined(__NIC_H) && defined(__mmod_emote3__)

#include <machine/cortex/machine.h>
#include <machine/cortex/m95.h>
#include <machine/cortex/uart.h>

__BEGIN_SYS

// Class Attributes
M95::Device M95::_devices[M95::UNITS];

// Methods
int M95::send_command(const char *command, unsigned int size)
{
    db<M95>(TRC) << "M95::send_command(c=" << command << ",sz=" << size << ")" << endl;

    if(_uart->ready_to_get())
        while(_uart->ready_to_get())
            db<M95>(TRC) << _uart->get();

    /*
    // TODO: Quectel HTTP manual says we need to do this, but it works fine without it.
    // Switch back to command mode
    if(_http_data_mode) {
        // First '+' needs to be sent 500ms after the last data char
        TSC::Time_Stamp end = _last_send + TSC::frequency() / 2;
        while(TSC::time_stamp() <= end);
        _uart->put('+');
        _uart->put('+');
        _uart->put('+');
        // First command char needs to be sent 500ms after the last '+'
        Machine::delay(5000000);
        _http_data_mode = false;
    }
    */

    for(unsigned int i = 0; i < size; i++)
        _uart->put(command[i]);
    _uart->put('\r');

    if((!memcmp(command, "AT+QHTTPURL=", strlen("AT+QHTTPURL="))) || (!memcmp(command, "AT+QHTTPPOST=", strlen("AT+QHTTPPOST=")))) {
        _http_data_mode = wait_response("CONNECT", 5000000);
        return _http_data_mode * size;
    }

    return size;
}

int M95::send_data(const char * data, unsigned int size)
{
    db<M95>(TRC) << "M95::send_data: " << size << " bytes" << endl;

    if(_uart->ready_to_get())
        while(_uart->ready_to_get())
            db<M95>(TRC) << _uart->get();

    for (unsigned int i = 0u; i < size; ++i)
        _uart->put(data[i]);

    _last_send = TSC::time_stamp();

    return size * wait_response("OK", 150000000);
}

// Assumes "expected" is null-terminated.
bool M95::wait_response(const char * expected, const RTC::Microsecond & timeout, char * response, unsigned int response_size) {
    db<M95>(TRC) << "M95::wait_response(ex=" << expected << ",tmt=" << timeout << ")" << endl;

    TSC::Time_Stamp end = TSC::time_stamp() + timeout * TSC::frequency() / 1000000;

    const char error[] = "ERROR";
    bool ret = true;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int len = strlen(expected);
    while(i < len) {
        while(!(_uart->ready_to_get()) && ((timeout == 0) || (TSC::time_stamp() < end)));

        if(!_uart->ready_to_get()) {
            db<M95>(WRN) << "M95::wait_response(ex=" << expected << ",tmt=" << timeout << ") => Timeout!" << endl;
            ret = false;
            break;
        }

        char c = _uart->get();
        db<M95>(TRC) << c;
        if(c == expected[i])
            i++;
        else if(c == expected[0])
            i = 1;
        else
            i = 0;

        if(c == error[j]) {
            j++;
            if(j >= strlen(error)) {
                db<M95>(WRN) << "M95::wait_response(ex=" << expected << ",tmt=" << timeout << ") => Error!" << endl;
                ret = false;
                break;
            }
        }
        else if(c == error[0])
            j = 1;
        else
            j = 0;
    }

    if(response) {
        for(unsigned int i = 0; i < response_size; i++) {
            while(!(_uart->ready_to_get()) && ((timeout == 0) || (TSC::time_stamp() < end)));

            if(!_uart->ready_to_get()) {
                db<M95>(WRN) << "M95::wait_response(ex=" << expected << ",tmt=" << timeout << ") => Timeout!" << endl;
                ret = false;
                break;
            }
            response[i] = _uart->get();
            db<M95>(TRC) << response[i];
            if(response[i] == '\n')
                break;
        }
    }

    while(_uart->ready_to_get())
        db<M95>(TRC) << _uart->get();

    return ret;
}

RTC::Microsecond M95::now()
{
    char buf[32];

    do {
        const char cmd[] = "AT+CCLK?";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("+CCLK: ", 300000, buf, 32));

    wait_response("OK\r\n", 300000);

    // adapted from an Aplication Note by Microchip for a PIC 18 MCU
    // http://ww1.microchip.com/downloads/en/AppNotes/01412A.pdf

    //Code from http://www.oryx-embedded.com/doc/date__time_8c_source.html

    unsigned int year, month, day, hour, minute, second;
    RTC::Microsecond time_stamp;

    year = 2000 + (buf[1] - '0') * 10 + (buf[2] - '0');

    month = (buf[4] - '0') * 10 + (buf[5] - '0');
    //January and February are counted as months 13 and 14 of the previous year
    if(month <= 2) {
        month += 12;
        year -= 1;
    }
    day = (buf[7] - '0') * 10 + (buf[8] - '0');
    hour = (buf[10] - '0') * 10 + (buf[11] - '0');
    minute = (buf[13] - '0') * 10 + (buf[14] - '0');
    second = (buf[16] - '0') * 10 + (buf[17] - '0');

    // Convert years to days
    time_stamp = (365 * year) + (year / 4) - (year / 100) + (year / 400);
    // Convert months to days
    time_stamp += (30 * month) + (3 * (month + 1) / 5) + day;
    // Unix time starts on January 1st, 1970
    time_stamp -= 719561;
    // Convert days to seconds
    time_stamp *= 86400;
    // Add hours, minutes and seconds
    time_stamp += (3600 * hour) + (60 * minute) + second;

    // Return Unix time in microseconds
    return time_stamp * 1000000;
}

M95::RSSI M95::rssi()
{
    char buf[16];
    const char cmd[] = "AT+CSQ";
    send_command(cmd, strlen(cmd));
    bool csq = wait_response("+CSQ: ", 300000, buf, 16);
    wait_response("OK\r\n", 300000);
    RSSI ret = 0;
    if(csq) {
        ret = (buf[0] - '0') * 10 + (buf[1] - '0');
        if(ret == 99) // 99 = Not known or not detectable
            ret = 0;
        else
            ret = 2 * ret - 113;
    }
    return ret;
}

__END_SYS

#endif

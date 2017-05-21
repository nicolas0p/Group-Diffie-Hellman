// EPOS YYYY UART Mediator Declarations

#ifndef __yyyy_uart_h
#define __yyyy_uart_h

#include <uart.h>
#include <cpu.h>

__BEGIN_SYS

// CHIP_YYYY_UART
class CHIP_YYYY_UART
{
};

class YYYY_UART: public UART_Common, private CHIP_YYYY_UART
{
    friend class YYYY;

private:
    static const unsigned int CLOCK = Traits<YYYY_UART>::CLOCK;

public:
    YYYY_UART(unsigned int unit = 0) : YYYY_UART(unit) {}
    YYYY_UART(unsigned int baud, unsigned int data_bits, unsigned int parity, unsigned int stop_bits, unsigned int unit = 0):
        CHIP_YYYY_UART(unit, CLOCK / baud, data_bits, parity, stop_bits) {}

    void config(unsigned int baud, unsigned int data_bits, unsigned int parity, unsigned int stop_bits) {
        CHIP_YYYY_UART::config(CLOCK / baud, data_bits, parity, stop_bits);
    }
    void config(unsigned int * baud, unsigned int * data_bits, unsigned int * parity, unsigned int * stop_bits) {
        CHIP_YYYY_UART::config(*baud, *data_bits, *parity, *stop_bits);
        *baud = CLOCK / *baud;
    }

    char get() {
        /* This method must block until a character is received through the serial port and then return it */
    }
    void put(char c) {
        /* This method must block until a character is sent through the serial port */
    }

    void loopback(bool flag) { CHIP_YYYY_UART::loopback(flag); }

private:
    static void init();
};

__END_SYS

#endif

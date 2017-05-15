// EPOS Cortex UART Mediator Declarations

#ifndef __cortex_uart_h__
#define __cortex_uart_h__

#include <cpu.h>
#include <uart.h>
#include __MODEL_H

__BEGIN_SYS

#ifdef __mmod_zynq__

class UART_Engine: protected Machine_Model
{
private:
    static const unsigned int UNITS = Traits<UART>::UNITS;
    static const unsigned int CLOCK = Traits<UART>::CLOCK;

public:
    UART_Engine(unsigned int unit, unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits)
        : _base(reinterpret_cast<Log_Addr *>(unit ? UART1_BASE : UART0_BASE)) {
        assert(unit < UNITS);
        config(baud_rate, data_bits, parity, stop_bits);
    }

    void config(unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits) {
        // Configure the number of stop bits, data bits, and the parity
        Reg32 mode = reg(MODE_REG0) & ~0xff;

        mode |= stop_bits == 2 ? NBSTOP2 : NBSTOP1;
        mode |= data_bits == 8 ? CHRL8 : data_bits == 7 ? CHRL7 : CHRL6;
        mode |= parity == 2 ? PAREVEN : parity == 1 ? PARODD : PARNONE;
        reg(MODE_REG0) = mode;

        // Set the baud rate
        Reg32 br = CLOCK / (7 * baud_rate);
        reg(BAUD_RATE_DIVIDER_REG0) = 6;
        reg(BAUD_RATE_GEN_REG0) = br;

        // Set the receiver trigger level to 1
        reg(RCVR_FIFO_TRIGGER_LEVEL0) = 1;

        // Enable and reset RX and TX data paths
        reg(CONTROL_REG0) = RXRES | TXRES | RXEN | TXEN;
    }
    void config(unsigned int * baud_rate, unsigned int * data_bits, unsigned int * parity, unsigned int * stop_bits) {
        Reg32 mode = reg(MODE_REG0);

        *baud_rate = CLOCK/(7*reg(BAUD_RATE_GEN_REG0));
        *data_bits = (mode & 0x6) == CHRL8 ? 8 : (mode & 0x6) == CHRL7 ? 7 : 6;
        *parity = (mode & 0x38) == PAREVEN ? 2 : (mode & 0x38) == PARODD ? 1 : 0;
        *stop_bits = (mode & 0xC0) == NBSTOP2 ? 2 : 1;
    }

    unsigned char rxd() { return reg(TX_RX_FIFO0); }
    void txd(unsigned char ch) { reg(TX_RX_FIFO0) = ch; }

    void int_enable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        reg(INTRPT_EN_REG0) |= (receive ? INTRPT_RTRIG : 0) | (send ? INTRPT_TTRIG : 0);
        reg(INTRPT_DIS_REG0) &= ~(receive ? INTRPT_RTRIG : 0) & ~(send ? INTRPT_TTRIG : 0);
    }
    void int_disable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        reg(INTRPT_EN_REG0) &= ~(receive ? INTRPT_RTRIG : 0) & ~(send ? INTRPT_TTRIG : 0);
        reg(INTRPT_DIS_REG0) |= (receive ? INTRPT_RTRIG : 0) | (send ? INTRPT_TTRIG : 0);
    }

    void reset() {
        unsigned int b, db, p, sb;
        config(&b, &db, &p, &sb);
        config(b, db, p, sb);
    }

    void loopback(bool flag) {
        Reg32 mode = reg(MODE_REG0) & ~0x300;

        if(flag)
            mode |= CHMODELB;
        else
            mode |= CHMODENORM;

        reg(MODE_REG0) = mode;
    }

    bool rxd_ok() { return reg(CHANNEL_STS_REG0) & STS_RTRIG; }
    bool txd_ok() { return !(reg(CHANNEL_STS_REG0) & STS_TFUL); }

private:
    volatile Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile Reg32*>(_base)[o / sizeof(Reg32)]; }

private:
    volatile Log_Addr * _base;
};

#else

// PrimeCell UART (PL011)
class UART_Engine: protected Machine_Model
{
private:
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg32 Reg32;

    static const unsigned int UNITS = Traits<UART>::UNITS;
    static const unsigned int CLOCK = Traits<UART>::CLOCK / 16;

public:
    // Register offsets
    enum {                              // Description                  Type    Value after reset
        DR              = 0x000,        // Data                         r/w     0x00000000
        RSR             = 0x004,        // Receive Status               r/w     0x00000000
        ECR             = 0x004,        // Error Clear                  r/w     0x00000000
        FR              = 0x018,        // Flag                         ro      0x00000090
        IBRD            = 0x024,        // Integer Baud-Rate Divisor    r/w     0x00000000
        FBRD            = 0x028,        // Fractional Baud-Rate Divisor r/w     0x00000000
        LCRH            = 0x02c,        // Line Control                 r/w     0x00000000
        UCR             = 0x030,        // Control                      r/w     0x00000300
        IFLS            = 0x034,        // Interrupt FIFO Level Select  r/w     0x00000012
        UIM             = 0x038,        // Interrupt Mask               r/w     0x00000000
        RIS             = 0x03c,        // Raw Interrupt Status         ro      0x0000000f
        MIS             = 0x040,        // Masked Interrupt Status      ro      0x00000000
        ICR             = 0x044,        // Interrupt Clear              w1c     0x00000000
        DMACR           = 0x048,        // DMA Control                  rw      0x00000000
        PeriphID4       = 0xfd0,        // Peripheral Identification 4  ro      0x00000000
        PeriphID5       = 0xfd4,        // Peripheral Identification 5  ro      0x00000000
        PeriphID6       = 0xfd8,        // Peripheral Identification 6  ro      0x00000000
        PeriphID7       = 0xfdc,        // Peripheral Identification 7  ro      0x00000000
        PeriphID0       = 0xfe0,        // Peripheral Identification 0  ro      0x00000011
        PeriphID1       = 0xfe4,        // Peripheral Identification 1  ro      0x00000000
        PeriphID2       = 0xfe8,        // Peripheral Identification 2  ro      0x00000018
        PeriphID3       = 0xfec,        // Peripheral Identification 3  ro      0x00000001
        PCellID0        = 0xff0,        // PrimeCell Identification 0   ro      0x0000000d
        PCellID1        = 0xff4,        // PrimeCell Identification 1   ro      0x000000f0
        PCellID2        = 0xff8,        // PrimeCell Identification 2   ro      0x00000005
        PCellID3        = 0xffc         // PrimeCell Identification 3   ro      0x000000b1
    };

    // Useful Bits in the Flag Register
    enum {                              // Description                  Type    Value after reset
        CTS             = 1 <<  0,      // Clear to Send                r/w     0
        DSR             = 1 <<  1,      // Data Set Ready               r/w     0
        DCD             = 1 <<  2,      // Data Carrier Detect          r/w     0
        BUSY            = 1 <<  3,      // Busy transmitting data       r/w     0
        RXFE            = 1 <<  4,      // Receive FIFO Empty           r/w     1
        TXFF            = 1 <<  5,      // Transmit FIFO Full           r/w     0
        RXFF            = 1 <<  6,      // Receive FIFO Full            r/w     0
        TXFE            = 1 <<  7,      // Transmit FIFO Empty          r/w     1
        RI              = 1 <<  8,      // Ring Indicator               r/w     0
    };

    // Useful Bits in the Line Control
    enum {                              // Description                  Type    Value after reset
        BRK             = 1 <<  0,      // Send Break                   r/w     0
        PEN             = 1 <<  1,      // Parity Enable                r/w     0
        EPS             = 1 <<  2,      // Even Parity Select           r/w     0
        STP2            = 1 <<  3,      // Two Stop Bits Select         r/w     0
        FEN             = 1 <<  4,      // FIFOs Enable                 r/w     0
        WLEN5           = 0 <<  5,      // Word Length 5 bits           r/w     0
        WLEN6           = 1 <<  5,      // Word Length 6 bits           r/w     0
        WLEN7           = 2 <<  5,      // Word Length 7 bits           r/w     0
        WLEN8           = 3 <<  5,      // Word Length 8 bits           r/w     0
        SPS             = 1 <<  7       // Stick Parity Select          r/w     0
    };

    // Useful Bits in the Control Register
    enum {                              // Description                  Type    Value after reset
        UEN             = 1 <<  0,      // Enable                       r/w     0
        LBE             = 1 <<  7,      // Loop Back Enable             r/w     0
        TXE             = 1 <<  8,      // Transmit Enable              r/w     1
        RXE             = 1 <<  9       // Receive Enable               r/w     1
    };

    // Useful Bits in the Interrupt Mask Register
    enum {                              // Description                  Type    Value after reset
        UIMRX           = 1 <<  4,      // Receive                      r/w     0
        UIMTX           = 1 <<  5,      // Transmit                     r/w     0
        UIMRT           = 1 <<  6,      // Receive Time-Out             r/w     0
        UIMFE           = 1 <<  7,      // Framing Error                r/w     0
        UIMPE           = 1 <<  8,      // Parity Error                 r/w     0
        UIMBE           = 1 <<  9,      // Break Error                  r/w     0
        UIMOE           = 1 << 10,      // Overrun Error                r/w     0
        UIMALL          = 0
    };

public:
    UART_Engine(unsigned int unit, unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits)
    : _base(reinterpret_cast<Log_Addr *>(unit ? UART1_BASE : UART0_BASE)) {
        assert(unit < UNITS);
        config(baud_rate, data_bits, parity, stop_bits);
    }

    void config(unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits) {
        Reg32 lcrh = data_bits == 8 ? WLEN8 : data_bits == 7 ? WLEN7 : data_bits = 6 ? WLEN6 : WLEN5; // config data bits
        lcrh |= FEN; // always use FIFO
        lcrh |= stop_bits == 2 ? STP2 : 0; // config stop bits
        lcrh |= (parity == 2) ? (EPS | PEN) : (parity == 1) ? PEN : 0; // config and enable even/odd parity

        reg(UCR) &= ~UEN;                       // Disable UART for configuration
        reg(ICR) = ~0;                          // Clear all interrupts
        reg(UIM) = UIMALL;                      // Disable all interrupts
        Reg32 br = CLOCK / (baud_rate / 300);   // Factor by the minimum BR to preserve meaningful bits of FBRD
        reg(IBRD) = br / 300;                   // IBRD = int(CLOCK / baud_rate)
        reg(FBRD) = br / 1000;                  // FBRD = int(0.1267 * 64 + 0.5) = 8
        reg(LCRH) = lcrh;                       // Write the serial parameters configuration
        reg(UIM) = UIMTX | UIMRX;               // Mask TX and RX interrupts for polling operation
        reg(UCR) |= UEN | TXE | RXE;            // Enable UART
    }

    void config(unsigned int * baud_rate, unsigned int * data_bits, unsigned int * parity, unsigned int * stop_bits) {
        Reg32 lcrh = reg(LCRH);
        *data_bits = 5 + (lcrh & WLEN8);
        *parity = (lcrh & PEN) ? (1 + (lcrh & EPS)) : 0;
        *baud_rate = (CLOCK * 300) / (reg(FBRD) * 1000 + reg(IBRD) * 300);
        *stop_bits = (reg(LCRH) & STP2) ? 2 : 1;
    }

    Reg8 rxd() { return reg(DR); }
    void txd(Reg8 c) { reg(DR) = c; }

    void int_enable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        reg(UIM) &= ~((receive ? UIMRX : 0) | (send ? UIMTX : 0));
    }
    void int_disable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        reg(UCR) |= (receive ? UIMRX : 0) | (send ? UIMTX : 0);
    }

    void reset() {
        unsigned int b, db, p, sb;
        config(&b, &db, &p, &sb);
        config(b, db, p, sb);
    }

    void loopback(bool flag) {
        if(flag)
            reg(UCR) |= int(LBE);
        else
            reg(UCR) &= ~LBE;
    }

    bool rxd_ok() { return !(reg(FR) & RXFE); }
    bool txd_ok() { return !(reg(FR) & TXFF); }

    bool busy() { return (reg(FR) & BUSY); }

private:
    volatile Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile Reg32*>(_base)[o / sizeof(Reg32)]; }

private:
    volatile Log_Addr * _base;
};

#endif

class UART: private UART_Common, private UART_Engine
{
private:
    typedef UART_Engine Engine;

    static const unsigned int UNIT = Traits<UART>::DEF_UNIT;
    static const unsigned int BAUD_RATE = Traits<UART>::DEF_BAUD_RATE;
    static const unsigned int DATA_BITS = Traits<UART>::DEF_DATA_BITS;
    static const unsigned int PARITY = Traits<UART>::DEF_PARITY;
    static const unsigned int STOP_BITS = Traits<UART>::DEF_STOP_BITS;

public:
    UART(unsigned int unit = UNIT, unsigned int baud_rate = BAUD_RATE, unsigned int data_bits = DATA_BITS, unsigned int parity = PARITY, unsigned int stop_bits = STOP_BITS)
    : Engine((enable_uart(unit), unit), baud_rate, data_bits, parity, stop_bits), _unit(unit) {}

    void config(unsigned int baud_rate, unsigned int data_bits, unsigned int parity, unsigned int stop_bits) {
        Engine::config(baud_rate, data_bits, parity, stop_bits);
    }
    void config(unsigned int * baud_rate, unsigned int * data_bits, unsigned int * parity, unsigned int * stop_bits) {
        Engine::config(*baud_rate, *data_bits, *parity, *stop_bits);
    }

    char get() { while(!rxd_ok()); return rxd(); }
    void put(char c) { while(!txd_ok()); txd(c); }

    bool ready_to_get() { return rxd_ok(); }
    bool ready_to_put() { return txd_ok(); }

    void int_enable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        Engine::int_enable(receive, send, line, modem);
    }
    void int_disable(bool receive = true, bool send = true, bool line = true, bool modem = true) {
        Engine::int_disable(receive, send, line, modem);
    }

    void reset() { Engine::reset(); }
    void loopback(bool flag) { Engine::loopback(flag); }

    void power(const Power_Mode & mode) { power_uart(_unit, mode); }

private:
    unsigned int _unit;
};

__END_SYS

#endif

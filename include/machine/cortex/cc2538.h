// EPOS TI CC2538 IEEE 802.15.4 NIC Mediator Declarations

#include <system/config.h>
#if !defined(__cc2538_h) && defined(__NIC_H) && !defined(__mmod_zynq__)
#define __cc2538_h

#include <ieee802_15_4.h>
#include "../common/ieee802_15_4_mac.h"
#include "../common/tstp_mac.h"

__BEGIN_SYS

// TI CC2538 IEEE 802.15.4 RF Transceiver
class CC2538RF: protected Machine_Model
{
public:
    struct Reset_Backdoor_Message {

        Reset_Backdoor_Message(unsigned char * id) : _header_code(0xbe), _footer_code(0xef) {
            memcpy(_id, id, 8);
        }

    private:
        unsigned char _header_code;
        unsigned char _id[8];
        unsigned char _footer_code;
        unsigned short _crc;
    }__attribute__((packed));

protected:
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef CPU::IO_Irq IO_Irq;
    typedef MMU::DMA_Buffer DMA_Buffer;
    typedef RTC::Microsecond Microsecond;

    static const bool promiscuous = Traits<CC2538>::promiscuous;

    static const unsigned int TX_TO_RX_DELAY = 2; // Radio takes extra 2us to go from TX to RX or idle
    static const unsigned int RX_TO_TX_DELAY = 0;

public:
    // Bases
    enum {
        FFSM_BASE       = 0x40088500,
        XREG_BASE       = 0x40088600,
        SFR_BASE        = 0x40088800,
        MACTIMER_BASE   = SFR_BASE,
        ANA_BASE        = SFR_BASE,
        RXFIFO          = 0x40088000,
        TXFIFO          = 0x40088200,
        CCTEST_BASE     = 0x44010000,
    };

    // Useful FFSM register offsets
    enum {
        SRCRESINDEX = 0x8c,
        PAN_ID0     = 0xc8,
        PAN_ID1     = 0xcc,
        SHORT_ADDR0 = 0xd0,
        SHORT_ADDR1 = 0xd4,
    };

    // XREG register offsets
    enum {
        FRMFILT0    = 0x000,
        FRMFILT1    = 0x004,
        SRCMATCH    = 0x008,
        FRMCTRL0    = 0x024,
        FRMCTRL1    = 0x028,
        RXENABLE    = 0x02c,
        RXMASKSET   = 0x030,
        RXMASKCLR   = 0x034,
        FREQCTRL    = 0x03C,   // RF carrier frequency (f = 11 + 5 (k – 11), with k [11, 26])  ro      0x0000000b
        FSMSTAT1    = 0x04C,   // Radio status register                                        ro      0x00000000
        FIFOPCTRL   = 0x050,
        FSMCTRL     = 0x054,
        RXFIRST     = 0x068,
        RXFIFOCNT   = 0x06C,   // Number of bytes in RX FIFO                                   ro      0x00000000
        TXFIFOCNT   = 0x070,   // Number of bytes in TX FIFO                                   ro      0x00000000
        RXFIRST_PTR = 0x074,
        RXLAST_PTR  = 0x078,
        RXP1_PTR    = 0x07C,
        RFIRQM0     = 0x08c,
        RFIRQM1     = 0x090,
        RFERRM      = 0x094,
        CSPT        = 0x194,
        AGCCTRL1    = 0x0c8,
        TXFILTCFG   = 0x1e8,
        FSCAL1      = 0x0b8,
        CCACTRL0    = 0x058,
        TXPOWER     = 0x040,
        RSSI        = 0x060,
        RSSISTAT    = 0x064,   // RSSI valid status register                                   ro      0x00000000
        RFC_OBS_CTRL0 = 0x1AC, // Select which signal is represented by OBSSEL_SIG0
        RFC_OBS_CTRL1 = 0x1B0, // Select which signal is represented by OBSSEL_SIG1
        RFC_OBS_CTRL2 = 0x1B4, // Select which signal is represented by OBSSEL_SIG2
    };

    // CCTEST register offsets
    enum {                     // Description                                           Type Size Reset
        CCTEST_IO      = 0x00, // Output strength control                               RW   32   0x0000 0000
        CCTEST_OBSSEL0 = 0x14, // Select output signal on GPIO C 0                      RW   32   0x0000 0000
        CCTEST_OBSSEL1 = 0x18, // Select output signal on GPIO C 1                      RW   32   0x0000 0000
        CCTEST_OBSSEL2 = 0x1C, // Select output signal on GPIO C 2                      RW   32   0x0000 0000
        CCTEST_OBSSEL3 = 0x20, // Select output signal on GPIO C 3                      RW   32   0x0000 0000
        CCTEST_OBSSEL4 = 0x24, // Select output signal on GPIO C 4                      RW   32   0x0000 0000
        CCTEST_OBSSEL5 = 0x28, // Select output signal on GPIO C 5                      RW   32   0x0000 0000
        CCTEST_OBSSEL6 = 0x2C, // Select output signal on GPIO C 6                      RW   32   0x0000 0000
        CCTEST_OBSSEL7 = 0x30, // Select output signal on GPIO C 7                      RW   32   0x0000 0000
        CCTEST_TR0     = 0x34, // Used to connect the temperature sensor to the SOC_ADC RW   32   0x0000 0000
        CCTEST_USBCTRL = 0x50, // USB PHY stand-by control                              RW   32   0x0000 0000
    };

    enum OBSSEL {
        OBSSEL_EN = 1 << 7, // Signal X enable bit
        OBSSEL_SIG0 = 0,    // Select signal 0
        OBSSEL_SIG1 = 1,    // Select signal 1
        OBSSEL_SIG2 = 2,    // Select signal 2
        OBSSEL_SIG0_EN = OBSSEL_EN | OBSSEL_SIG0, // Select and enable signal 0
        OBSSEL_SIG1_EN = OBSSEL_EN | OBSSEL_SIG1, // Select and enable signal 1
        OBSSEL_SIG2_EN = OBSSEL_EN | OBSSEL_SIG2, // Select and enable signal 2
    };

    // Controls which observable signal from RF Core is to be muxed out to OBSSEL_SIGx
    enum SIGNAL {                       // Description
        SIGNAL_LOW              = 0x00, // 0 Constant value
        SIGNAL_HIGH             = 0x01, // 1 Constant value
        SIGNAL_RFC_SNIFF_DATA   = 0x08, // Data from packet sniffer. Sample data on rising edges of sniff_clk.
        SIGNAL_RFC_SNIFF_CLK    = 0x09, // 250kHz clock for packet sniffer data.
        SIGNAL_RSSI_VALID       = 0x0C, // Pin is high when the RSSI value has been
                                        // updated at least once since RX was started. Cleared when leaving RX.
        SIGNAL_DEMOD_CCA        = 0x0D, // Clear channel assessment. See FSMSTAT1 register for details
                                        // on how to configure the behavior of this signal.
        SIGNAL_SAMPLED_CCA      = 0x0E, // A sampled version of the CCA bit from demodulator. The value is
                                        // updated whenever a SSAMPLECCA or STXONCCA strobe is issued.
        SIGNAL_SFD_SYNC         = 0x0F, // Pin is high when a SFD has been received or transmitted.
                                        // Cleared when leaving RX/TX respectively.
                                        // Not to be confused with the SFD exception.
        SIGNAL_TX_ACTIVE        = 0x10, // Indicates that FFCTRL is in one of the TX states. Active-high.
                                        // Note: This signal might have glitches, because it has no output flip-
                                        // flop and is based
                                        // on the current state register of the FFCTRL FSM.
        SIGNAL_RX_ACTIVE        = 0x11, // Indicates that FFCTRL is in one of the RX states. Active-high.
                                        // Note: This signal might have glitches, because it has no output flip-
                                        // flop and is based
                                        // on the current state register of the FFCTRL FSM.
        SIGNAL_FFCTRL_FIFO      = 0x12, // Pin is high when one or more bytes are in the RXFIFO.
                                        // Low during RXFIFO overflow.
        SIGNAL_FFCTRL_FIFOP     = 0x13, // Pin is high when the number of bytes in the RXFIFO exceeds the
                                        // programmable threshold or at least one complete frame is in the RXFIFO.
                                        // Also high during RXFIFO overflow.
                                        // Not to be confused with the FIFOP exception.
        SIGNAL_PACKET_DONE      = 0x14, // A complete frame has been received. I.e.,
                                        // the number of bytes set by the frame-length field has been received.
        SIGNAL_RFC_XOR_RAND_I_Q = 0x16, // XOR between I and Q random outputs. Updated at 8 MHz.
        SIGNAL_RFC_RAND_Q       = 0x17, // Random data output from the Q channel of the receiver. Updated at 8 MHz.
        SIGNAL_RFC_RAND_I       = 0x18, // Random data output from the I channel of the receiver. Updated at 8 MHz.
        SIGNAL_LOCK_STATUS      = 0x19, // 1 when PLL is in lock, otherwise 0
        SIGNAL_PA_PD            = 0x28, // Power amplifier power-down signal
        SIGNAL_LNA_PD           = 0x2A, // LNA power-down signal
    };

    // SFR register offsets
    enum {
        RFDATA  = 0x28,
        RFERRF  = 0x2c,
        RFIRQF1 = 0x30,
        RFIRQF0 = 0x34,
        RFST    = 0x38,         // RF CSMA-CA/strobe processor                                  rw       0x000000d0
    };

    // MACTIMER register offsets
    enum {       //Offset   Description                               Type    Value after reset
        MTCSPCFG = 0x00, // MAC Timer event configuration              RW     0x0
        MTCTRL   = 0x04, // MAC Timer control register                 RW     0x2
        MTIRQM   = 0x08, // MAC Timer interrupt mask                   RW     0x0
        MTIRQF   = 0x0C, // MAC Timer interrupt flags                  RW     0x0
        MTMSEL   = 0x10, // MAC Timer multiplex select                 RW     0x0
        MTM0     = 0x14, // MAC Timer multiplexed register 0           RW     0x0
        MTM1     = 0x18, // MAC Timer multiplexed register 1           RW     0x0
        MTMOVF2  = 0x1C, // MAC Timer multiplexed overflow register 2  RW     0x0
        MTMOVF1  = 0x20, // MAC Timer multiplexed overflow register 1  RW     0x0
        MTMOVF0  = 0x24, // MAC Timer multiplexed overflow register 0  RW     0x0
    };

    // ANA_REGS register offsets
    enum {
        IVCTRL    = 0x04,
    };

    // Radio commands
    enum {
        SRXON       = 0xd3,
        STXON       = 0xd9,
        SFLUSHRX    = 0xdd,
        SFLUSHTX    = 0xde,
        SRFOFF      = 0xdf,
        ISSTART     = 0xe1,
        ISSTOP      = 0xe2,
        ISRXON      = 0xe3,
        ISTXON      = 0xe9,
        ISTXONCCA   = 0xea,
        ISSAMPLECCA = 0xeb,
        ISFLUSHRX   = 0xed,
        ISFLUSHTX   = 0xee,
        ISRFOFF     = 0xef,
        ISCLEAR     = 0xff,
    };

    // Useful bits in RXENABLE
    enum {
        RXENABLE_SRXON         = 1 << 7,
        RXENABLE_STXON         = 1 << 6,
        RXENABLE_SRXMASKBITSET = 1 << 5,
    };
    // Useful bits in RSSISTAT
    enum {
        RSSI_VALID = 1 << 0,
    };
    // Useful bits in XREG_FRMFILT0
    enum {
        MAX_FRAME_VERSION = 1 << 2,
        PAN_COORDINATOR   = 1 << 1,
        FRAME_FILTER_EN   = 1 << 0,
    };
    // Useful bits in XREG_FRMFILT1
    enum {
        ACCEPT_FT3_MAC_CMD = 1 << 6,
        ACCEPT_FT2_ACK     = 1 << 5,
        ACCEPT_FT1_DATA    = 1 << 4,
        ACCEPT_FT0_BEACON  = 1 << 3,
    };
    // Useful bits in XREG_SRCMATCH
    enum {
        SRC_MATCH_EN   = 1 << 0,
    };

    // Useful bits in XREG_FSMCTRL
    enum {
        SLOTTED_ACK    = 1 << 1,
        RX2RX_TIME_OFF = 1 << 0,
    };

    // Useful bits in XREG_FRMCTRL0
    enum {
        APPEND_DATA_MODE = 1 << 7,
        AUTO_CRC         = 1 << 6,
        AUTO_ACK         = 1 << 5,
        ENERGY_SCAN      = 1 << 4,
        RX_MODE          = 1 << 2,
        TX_MODE          = 1 << 0,
    };

    enum RX_MODES {
        RX_MODE_NORMAL = 0,
        RX_MODE_OUTPUT_TO_IOC,
        RX_MODE_CYCLIC,
        RX_MODE_NO_SYMBOL_SEARCH,
    };

    // Bit set by hardware in FCS field when AUTO_CRC is set
    enum {
        AUTO_CRC_OK = 1 << 7,
    };

    // Useful bits in XREG_FRMCTRL1
    enum {
        PENDING_OR         = 1 << 2,
        IGNORE_TX_UNDERF   = 1 << 1,
        SET_RXENMASK_ON_TX = 1 << 0,
    };

    // Useful bits in XREG_FSMSTAT1
    enum {
        FIFO        = 1 << 7,
        FIFOP       = 1 << 6,
        SFD         = 1 << 5,
        CCA         = 1 << 4,
        SAMPLED_CCA = 1 << 3,
        LOCK_STATUS = 1 << 2,
        TX_ACTIVE   = 1 << 1,
        RX_ACTIVE   = 1 << 0,
    };

    // Useful bits in SFR_RFIRQF1
    enum {
        TXDONE = 1 << 1,
    };

    // RFIRQF0 Interrupts
    enum {
        INT_RXMASKZERO      = 1 << 7,
        INT_RXPKTDONE       = 1 << 6,
        INT_FRAME_ACCEPTED  = 1 << 5,
        INT_SRC_MATCH_FOUND = 1 << 4,
        INT_SRC_MATCH_DONE  = 1 << 3,
        INT_FIFOP           = 1 << 2,
        INT_SFD             = 1 << 1,
        INT_ACT_UNUSED      = 1 << 0,
    };

    // RFIRQF1 Interrupts
    enum {
        INT_CSP_WAIT   = 1 << 5,
        INT_CSP_STOP   = 1 << 4,
        INT_CSP_MANINT = 1 << 3,
        INT_RFIDLE     = 1 << 2,
        INT_TXDONE     = 1 << 1,
        INT_TXACKDONE  = 1 << 0,
    };

    // RFERRF Interrupts
    enum {
        INT_STROBEERR = 1 << 6,
        INT_TXUNDERF  = 1 << 5,
        INT_TXOVERF   = 1 << 4,
        INT_RXUNDERF  = 1 << 3,
        INT_RXOVERF   = 1 << 2,
        INT_RXABO     = 1 << 1,
        INT_NLOCK     = 1 << 0,
    };

    // Useful bits in MTCTRL
    enum {                  //Offset   Description                                                             Type    Value after reset
        MTCTRL_LATCH_MODE = 1 << 3, // 0: Reading MTM0 with MTMSEL.MTMSEL = 000 latches the high               RW      0
                                    // byte of the timer, making it ready to be read from MTM1. Reading
                                    // MTMOVF0 with MTMSEL.MTMOVFSEL = 000 latches the two
                                    // most-significant bytes of the overflow counter, making it possible to
                                    // read these from MTMOVF1 and MTMOVF2.
                                    // 1: Reading MTM0 with MTMSEL.MTMSEL = 000 latches the high
                                    // byte of the timer and the entire overflow counter at once, making it
                                    // possible to read the values from MTM1, MTMOVF0, MTMOVF1, and MTMOVF2.
        MTCTRL_STATE      = 1 << 2, // State of MAC Timer                                                      RO      0
                                    // 0: Timer idle
                                    // 1: Timer running
        MTCTRL_SYNC       = 1 << 1, // 0: Starting and stopping of timer is immediate; that is, synchronous    RW      1
                                    // with clk_rf_32m.
                                    // 1: Starting and stopping of timer occurs at the first positive edge of
                                    // the 32-kHz clock. For more details regarding timer start and stop,
                                    // see Section 22.4.
        MTCTRL_RUN        = 1 << 0, // Write 1 to start timer, write 0 to stop timer. When read, it returns    RW      0
                                    // the last written value.
    };

    // Useful bits in MSEL
    enum {
        MSEL_MTMOVFSEL = 1 << 4, // See possible values below
        MSEL_MTMSEL    = 1 << 0, // See possible values below
    };
    enum {
        OVERFLOW_COUNTER  = 0x00,
        OVERFLOW_CAPTURE  = 0x01,
        OVERFLOW_PERIOD   = 0x02,
        OVERFLOW_COMPARE1 = 0x03,
        OVERFLOW_COMPARE2 = 0x04,
    };
    enum {
        TIMER_COUNTER  = 0x00,
        TIMER_CAPTURE  = 0x01,
        TIMER_PERIOD   = 0x02,
        TIMER_COMPARE1 = 0x03,
        TIMER_COMPARE2 = 0x04,
    };
    enum {
        INT_OVERFLOW_COMPARE2 = 1 << 5,
        INT_OVERFLOW_COMPARE1 = 1 << 4,
        INT_OVERFLOW_PER      = 1 << 3,
        INT_COMPARE2          = 1 << 2,
        INT_COMPARE1          = 1 << 1,
        INT_PER               = 1 << 0
    };

    class Timer
    {
        friend class CC2538;
        friend class CC2538RF;
        friend class IC;
    private:
        const static unsigned int CLOCK = 32 * 1000 * 1000; // 32MHz

    public:
        typedef unsigned long long Time_Stamp;
        typedef long long Offset;

        static unsigned int frequency() { return CLOCK; }

    public:
        Timer() {}

        static Time_Stamp read() { CPU::int_disable(); auto ret = read_raw(); CPU::int_enable(); return ret + _offset; }

        static Time_Stamp sfd() {
            CPU::int_disable();
            mactimer(MTMSEL) = (OVERFLOW_CAPTURE * MSEL_MTMOVFSEL) | (TIMER_CAPTURE * MSEL_MTMSEL);

            Time_Stamp ts;

            ts = static_cast<Time_Stamp>(_overflow_count) << 40;
            ts += mactimer(MTM0); // M0 must be read first
            ts += mactimer(MTM1) << 8;
            ts += static_cast<Time_Stamp>(mactimer(MTMOVF0)) << 16;
            ts += static_cast<Time_Stamp>(mactimer(MTMOVF1)) << 24;
            ts += static_cast<Time_Stamp>(mactimer(MTMOVF2)) << 32;

            // This check is not enough only if sfd() is called more than 9.5 hours after the last frame arrival
            if(read_raw() <= ts)
                ts -= (1ll << 40);

            ts += _offset;
            CPU::int_enable();

            return ts;
        }

        static void adjust(const Offset & o) { _offset += o; }

        // TODO
        static void adjust_frequency(const Offset & time_diff, const Offset & period) {
            Offset diff = ((time_diff << 16) / period);
            _periodic_update += diff;

            Offset diff_diff = ((diff << 16) / period);
            _periodic_update_update += diff_diff;

            Offset diff_diff_diff = ((diff_diff << 16) / period);
            _periodic_update_update_update += diff_diff_diff;
        }

        static void interrupt(const Time_Stamp & when, const IC::Interrupt_Handler & h) {

            CPU::int_disable();

            // Clear any pending compare interrupts
            mactimer(MTIRQF) = mactimer(MTIRQF) & INT_OVERFLOW_PER;
            _ints = _ints & INT_OVERFLOW_PER;

            Time_Stamp ts = when - _offset;

            mactimer(MTMSEL) = (OVERFLOW_COMPARE1 * MSEL_MTMOVFSEL) | (TIMER_COMPARE1 * MSEL_MTMSEL);
            mactimer(MTM0) = ts;
            mactimer(MTM1) = ts >> 8;
            mactimer(MTMOVF0) = ts >> 16;
            mactimer(MTMOVF1) = ts >> 24;
            mactimer(MTMOVF2) = ts >> 32;

            _handler = h;
            _int_request_time = ts;

            int_enable(INT_OVERFLOW_COMPARE1 | INT_COMPARE1);
            CPU::int_enable();
        }

        static void int_disable() {
            _handler = 0;
            mactimer(MTIRQM) = INT_OVERFLOW_PER;
        }

        static Time_Stamp us2count(const Microsecond & us) { return static_cast<Time_Stamp>(us) * (CLOCK / 1000000); }
        static Microsecond count2us(const Time_Stamp & ts) { return ts / (CLOCK / 1000000); }

    private:
        static void int_enable(const Reg32 & interrupt) { mactimer(MTIRQM) |= interrupt; }

        static Time_Stamp read_raw() {
            assert(CPU::int_disabled());
            mactimer(MTMSEL) = (OVERFLOW_COUNTER * MSEL_MTMOVFSEL) | (TIMER_COUNTER * MSEL_MTMSEL);
            Time_Stamp oc, ts;

            ts = (oc = _overflow_count) << 40;
            ts += mactimer(MTM0); // M0 must be read first
            ts += mactimer(MTM1) << 8;
            ts += static_cast<Time_Stamp>(mactimer(MTMOVF0)) << 16;
            ts += static_cast<Time_Stamp>(mactimer(MTMOVF1)) << 24;
            ts += static_cast<Time_Stamp>(mactimer(MTMOVF2)) << 32;

            // The 40-bit counter overflows every 9.5 hours,
            // so we assume at most one happened inside this method
            if(_overflow_count != oc)
                ts += 1ll << 40;

            return ts;
        }

        static void int_handler(const IC::Interrupt_Id & interrupt) {
            CPU::int_disable();
            Reg32 ints = _ints;
            _ints &= ~ints;

            // TODO
            //if(ints & INT_PER) {
            //    _offset += _periodic_update;
            //    _periodic_update += _periodic_update_update;
            //    _periodic_update_update += _periodic_update_update_update;
            //}

            if(ints & INT_OVERFLOW_PER)
                _overflow_count++;

            IC::Interrupt_Handler h;
            if((ints & INT_COMPARE1) && (h = _handler) && (_int_request_time <= read_raw())) {
                if(TSTP_MAC<CC2538RF>::state_machine_debugged)
                    kout << 't';
                int_disable();
                CPU::int_enable();
                h(interrupt);
                IC::enable(IC::INT_NIC0_RX); // Make sure radio and MAC timer don't preempt one another
            } else {
                IC::enable(IC::INT_NIC0_RX); // Make sure radio and MAC timer don't preempt one another
                CPU::int_enable();
            }
        }

        static void eoi(const IC::Interrupt_Id & interrupt) {
            _ints |= mactimer(MTIRQF);
            mactimer(MTIRQF) = 0;
            IC::disable(IC::INT_NIC0_RX); // Make sure radio and MAC timer don't preempt one another
        }

        static void init();

    private:
        static volatile Offset _offset;
        static volatile Offset _periodic_update;
        static volatile Offset _periodic_update_update;
        static volatile Offset _periodic_update_update_update;
        static volatile IC::Interrupt_Handler _handler;
        static volatile Reg32 _overflow_count;
        static volatile Reg32 _ints;
        static volatile Time_Stamp _int_request_time;
        static bool _overflow_match;
        static bool _msb_match;
    };

public:
    CC2538RF() {
        sfr(RFST) = ISSTOP;
        sfr(RFST) = ISRFOFF;

        // Disable interrupts
        xreg(RFIRQM0) = 0;
        xreg(RFIRQM1) = 0;
        xreg(RFERRM) = 0;

        // Change recommended in the user guide (CCACTRL0 register description)
        xreg(CCACTRL0) = 0xf8;

        // Changes recommended in the user guide (Section 23.15 Register Settings Update)
        xreg(TXFILTCFG) = 0x09;
        xreg(AGCCTRL1) = 0x15;
        ana(IVCTRL) = 0x0b;
        xreg(FSCAL1) = 0x01;

        // Clear FIFOs
        sfr(RFST) = ISFLUSHTX;
        sfr(RFST) = ISFLUSHRX;

        // Disable receiver deafness for 192us after frame reception
        xreg(FSMCTRL) &= ~RX2RX_TIME_OFF;

        // Reset result of source matching (value undefined on reset)
        ffsm(SRCRESINDEX) = 0;

        // Set FIFOP threshold to maximum
        xreg(FIFOPCTRL) = 0xff;

        // Maximize transmission power
        xreg(TXPOWER) = 0xff;

        // Disable counting of MAC overflows
        xreg(CSPT) = 0xff;

        // Clear interrupts
        sfr(RFIRQF0) = 0;
        sfr(RFIRQF1) = 0;

        // Clear error flags
        sfr(RFERRF) = 0;
    }

    void address(const IEEE802_15_4::Address & address) {
        ffsm(SHORT_ADDR0) = address[0];
        ffsm(SHORT_ADDR1) = address[1];
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static void backoff(const Microsecond & time) {
        Timer::Time_Stamp end = Timer::read() + Timer::us2count(time);
        while(Timer::read() <= end);
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static bool cca(const Microsecond & time) {
        Timer::Time_Stamp end = Timer::read() + Timer::us2count(time);
        while(!(xreg(RSSISTAT) & RSSI_VALID));
        bool channel_free = xreg(FSMSTAT1) & CCA;
        while((Timer::read() <= end) && (channel_free = channel_free && (xreg(FSMSTAT1) & CCA)));
        return channel_free;
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static void transmit_no_cca() {
        xreg(RXMASKCLR) = RXENABLE_SRXON; // Don't return to receive mode after TX
        sfr(RFST) = ISTXON;
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static bool transmit() {
        xreg(RXMASKCLR) = RXENABLE_SRXON; // Don't return to receive mode after TX
        sfr(RFST) = ISTXONCCA;
        return (xreg(FSMSTAT1) & SAMPLED_CCA);
    }

    bool wait_for_ack(const Microsecond & timeout, Reg8 sequence_number) {
        // Disable and clear FIFOP int. We'll poll the interrupt flag
        xreg(RFIRQM0) &= ~INT_FIFOP;
        sfr(RFIRQF0) &= ~INT_FIFOP;

        // Save radio configuration
        Reg32 saved_filter_settings = xreg(FRMFILT1);

        // Accept only ACK frames now
        if(promiscuous) // Exit promiscuous mode for now
            xreg(FRMFILT0) |= FRAME_FILTER_EN;
        xreg(FRMFILT1) = ACCEPT_FT2_ACK;

        while(!tx_done());

        // Wait for either ACK or timeout
        bool acked = false;
        for(Timer::Time_Stamp end = Timer::read() + Timer::us2count(timeout); (Timer::read() < end);) {
            if(sfr(RFIRQF0) & INT_FIFOP) {
                unsigned int first = xreg(RXP1_PTR);
                volatile unsigned int * rxfifo = reinterpret_cast<volatile unsigned int*>(RXFIFO);
                if(rxfifo[first + 3] == sequence_number) {
                    acked = true;
                    break;
                }
                drop();
                sfr(RFIRQF0) &= ~INT_FIFOP;
            }
        }

        // Restore radio configuration
        xreg(FRMFILT1) = saved_filter_settings;
        if(promiscuous)
            xreg(FRMFILT0) &= ~FRAME_FILTER_EN;
        xreg(RFIRQM0) |= INT_FIFOP;

        return acked;
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static void listen() { sfr(RFST) = ISRXON; }

    /*
    // TODO
    static void listen_until_timer_interrupt() {
        sfr(RFST) = ISSTOP;
        sfr(RFST) = SRXON;
        unsigned int end = Timer::_int_request_time - Timer::us2count(320) - Timer::read_raw();
        sfr(RFST) = 0x80 | ((end >> 16) & 0x1f);
        sfr(RFST) = 0xB8;
        sfr(RFST) = SRFOFF;
        sfr(RFST) = ISSTART;
    }
    */

    // FIXME: methods changed to static because of TSTP_MAC
    static bool tx_done() {
        bool tx_ok = (sfr(RFIRQF1) & INT_TXDONE);
        if(tx_ok)
            sfr(RFIRQF1) &= ~INT_TXDONE;
        bool tx_error = (sfr(RFERRF) & (INT_TXUNDERF | INT_TXOVERF));
        if(tx_error)
            sfr(RFERRF) &= ~(INT_TXUNDERF | INT_TXOVERF);
        return tx_ok || tx_error;
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static bool rx_done() {
        bool ret = (sfr(RFIRQF0) & INT_RXPKTDONE);
        if(ret)
            sfr(RFIRQF0) &= ~INT_RXPKTDONE;
        return ret;
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static void channel(unsigned int c) {
        assert((c > 10) && (c < 27));
        xreg(FREQCTRL) = 11 + 5 * (c - 11);
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static void copy_to_nic(const void * frame, unsigned int size) {
        assert(size <= 127);
        // Clear TXFIFO
        sfr(RFST) = ISFLUSHTX;
        while(xreg(TXFIFOCNT) != 0);

        // Copy Frame to TXFIFO
        const char * f = reinterpret_cast<const char *>(frame);
        sfr(RFDATA) = size; // len
        for(unsigned int i = 0; i < size - sizeof(IEEE802_15_4::CRC); i++)
            sfr(RFDATA) = f[i];
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static unsigned int copy_from_nic(void * frame) {
        char * f = reinterpret_cast<char *>(frame);
        unsigned int first = xreg(RXP1_PTR);
        volatile unsigned int * rxfifo = reinterpret_cast<volatile unsigned int*>(RXFIFO);
        unsigned int sz = rxfifo[first]; // First byte is the length of MAC frame
        for(unsigned int i = 0; i < sz; ++i)
            f[i] = rxfifo[first + i + 1];
        drop();
        return sz;
    }

    // FIXME: methods changed to static because of TSTP_MAC
    static void drop() { sfr(RFST) = ISFLUSHRX; }

    bool filter() {
        bool valid_frame = false;
        unsigned int first = xreg(RXP1_PTR);
        unsigned int last = xreg(RXLAST_PTR);
        volatile unsigned int * rxfifo = reinterpret_cast<volatile unsigned int*>(RXFIFO);
        unsigned char mac_frame_size = rxfifo[first];
        if(last - first > 1 + sizeof(IEEE802_15_4::CRC)) {
            // On RX, last two bytes in the frame are replaced by info like CRC result
            // (obs: mac frame is preceded by one byte containing the frame length,
            // so total RXFIFO data size is 1 + mac_frame_size)
            valid_frame = (first + mac_frame_size < last) && (rxfifo[first + mac_frame_size] & AUTO_CRC_OK);
        }
        if(!valid_frame)
            drop();

        else if(Traits<CC2538>::reset_backdoor && (mac_frame_size == sizeof(Reset_Backdoor_Message) + sizeof(IEEE802_15_4::Header))) {
            if((rxfifo[first + sizeof(IEEE802_15_4::Header) + 1] == 0xbe) && (rxfifo[first + sizeof(IEEE802_15_4::Header) + 10] == 0xef)) {
                unsigned char id[8];
                for(unsigned int i = 0; i < 8u; i++)
                    id[i] = rxfifo[first + sizeof(IEEE802_15_4::Header) + 2 + i];
                if(!memcmp(id, Machine::id(), 8))
                    Machine::reboot();
            }
        }

        return valid_frame;
    }

    static void power(const Power_Mode & mode) {
         switch(mode) {
         case FULL: // Able to receive and transmit
             power_ieee802_15_4(FULL);
             xreg(FRMCTRL0) = (xreg(FRMCTRL0) & ~(3 * RX_MODE)) | (RX_MODE_NORMAL * RX_MODE);
             xreg(RFIRQM0) &= ~INT_FIFOP;
             xreg(RFIRQM0) |= INT_FIFOP;
             break;
         case LIGHT: // Able to sense channel and transmit
             power_ieee802_15_4(LIGHT);
             xreg(FRMCTRL0) = (xreg(FRMCTRL0) & ~(3 * RX_MODE)) | (RX_MODE_NO_SYMBOL_SEARCH * RX_MODE);
             break;
         case SLEEP: // Receiver off
             sfr(RFST) = ISSTOP;
             sfr(RFST) = ISRFOFF;
             xreg(RFIRQM0) &= ~INT_FIFOP;
             xreg(RFIRQF0) &= ~INT_FIFOP;
             power_ieee802_15_4(SLEEP);
             break;
         case OFF: // Radio unit shut down
             sfr(RFST) = ISSTOP;
             sfr(RFST) = ISRFOFF;
             xreg(RFIRQM0) &= ~INT_FIFOP;
             xreg(RFIRQF0) &= ~INT_FIFOP;
             power_ieee802_15_4(OFF);
             break;
         }
     }

protected:
    static volatile Reg32 & ana     (unsigned int offset) { return *(reinterpret_cast<volatile Reg32 *>(ANA_BASE + offset)); }
    static volatile Reg32 & xreg    (unsigned int offset) { return *(reinterpret_cast<volatile Reg32 *>(XREG_BASE + offset)); }
    static volatile Reg32 & ffsm    (unsigned int offset) { return *(reinterpret_cast<volatile Reg32 *>(FFSM_BASE + offset)); }
    static volatile Reg32 & sfr     (unsigned int offset) { return *(reinterpret_cast<volatile Reg32 *>(SFR_BASE  + offset)); }
    static volatile Reg32 & cctest  (unsigned int offset) { return *(reinterpret_cast<volatile Reg32 *>(CCTEST_BASE + offset)); }
    static volatile Reg32 & mactimer(unsigned int offset) { return *(reinterpret_cast<volatile Reg32 *>(MACTIMER_BASE + offset)); }

    // FIXME: Static because of TSTP_MAC
    static bool _cca_done;
};

// CC2538 IEEE 802.15.4 EPOSMote III NIC Mediator
class CC2538: public IEEE802_15_4::NIC_Base<IEEE802_15_4, Traits<NIC>::NICS::Polymorphic>, public IF<EQUAL<Traits<Network>::NETWORKS::Get<Traits<NIC>::NICS::Find<CC2538>::Result>::Result, TSTP>::Result, TSTP_MAC<CC2538RF>, IEEE802_15_4_MAC<CC2538RF>>::Result
{
    template <typename Type, int unit> friend void call_init();

private:
    typedef IF<EQUAL<Traits<Network>::NETWORKS::Get<Traits<NIC>::NICS::Find<CC2538>::Result>::Result, _SYS::TSTP>::Result, TSTP_MAC<CC2538RF>, IEEE802_15_4_MAC<CC2538RF>>::Result MAC;

    typedef MAC::Buffer Buffer;
    typedef IEEE802_15_4::Statistics Statistics;
    typedef IEEE802_15_4::Address Address;
    typedef IEEE802_15_4::Protocol Protocol;

    static const bool ieee802_15_4_mac = EQUAL<MAC, IEEE802_15_4_MAC<CC2538RF>>::Result;

    // Transmit and Receive Ring sizes
    static const unsigned int UNITS = Traits<CC2538>::UNITS;
    static const unsigned int RX_BUFS = Traits<CC2538>::RECEIVE_BUFFERS;

    // Size of the DMA Buffer that will host the ring buffers
    static const unsigned int DMA_BUFFER_SIZE = RX_BUFS * sizeof(Buffer);

    // Interrupt dispatching binding
    struct Device {
        CC2538 * device;
        unsigned int interrupt;
        unsigned int error_interrupt;
    };

protected:
    CC2538(unsigned int unit);

public:
    typedef CC2538RF::Timer Timer;

    ~CC2538();

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size);
    int receive(Address * src, Protocol * prot, void * data, unsigned int size);

    Buffer * alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    void free(Buffer * buf);
    int send(Buffer * buf);

    const Address & address() { return _address; }
    void address(const Address & address) { _address = address; CC2538RF::address(address); }

    unsigned int channel() { return _channel; }
    void channel(unsigned int channel) {
        if((channel > 10) && (channel < 27)) {
            _channel = channel;
            CC2538RF::channel(_channel);
        }
    }

    const Statistics & statistics() { return _statistics; }

    void reset();

    static CC2538 * get(unsigned int unit = 0) { return get_by_unit(unit); }

    static void eoi(const IC::Interrupt_Id & interrupt) {
        IC::disable(IC::INT_NIC0_TIMER); // Make sure radio and MAC timer don't preempt one another
    }

private:
    void handle_int();

    static void int_handler(const IC::Interrupt_Id & interrupt);

    static CC2538 * get_by_unit(unsigned int unit) {
        assert(unit < UNITS);
        return _devices[unit].device;
    }

    static CC2538 * get_by_interrupt(unsigned int interrupt) {
        CC2538 * tmp = 0;
        for(unsigned int i = 0; i < UNITS; i++)
            if((_devices[i].interrupt == interrupt) || (_devices[i].error_interrupt == interrupt))
                tmp = _devices[i].device;
        return tmp;
    };

    static void init(unsigned int unit);

private:
    unsigned int _unit;

    Address _address;
    unsigned int _channel;
    Statistics _statistics;

    Buffer * _rx_bufs[RX_BUFS];
    unsigned int _rx_cur_consume;
    unsigned int _rx_cur_produce;
    static Device _devices[UNITS];
};

__END_SYS

#endif

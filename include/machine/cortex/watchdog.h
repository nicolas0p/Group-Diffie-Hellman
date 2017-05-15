// EPOS Cortex Watchdog Mediator Declarations

#ifndef __cortex_watchdog_h
#define __cortex_watchdog_h

#include <cpu.h>
#include <watchdog.h>
#include __MODEL_H

__BEGIN_SYS

class Watchdog_Engine: protected Machine_Model
{
    typedef CPU::Reg32 Reg32;

private:
    enum Base {
        WATCHDOG_BASE = 0x400D5000,
    };

    enum {
     // Name    Offset   Type  Width   Reset Value    Physical Address
        WDCTL = 0x00, //   RW   32     0x0000 0000    0x400D 5000
    };

    // WDCTL register offsets
    enum {
      //Name  Offset     Description                                                           Type Reset Value
        CLR = 1 << 4, // Clear timer                                                             RW 0
                      // When 0xA followed by 0x5 is written to these bits, the timer is
                      // loaded with 0x0000. Note that 0x5 must be written within one
                      // watchdog clock period Twdt after 0xA was written for the clearing to
                      // take effect (ensured).
                      // If 0x5 is written between Twdt and 2Twdt after 0xA was written, the
                      // clearing may take effect, but there is no guarantee. If 0x5 is written
                      // > 2Twdt after 0xA was written, the timer will not be cleared.
                      // If a value other than 0x5 is written after 0xA has been written, the
                      // clear sequence is aborted. If 0xA is written, this starts a new clear
                      // sequence.
                      // Writing to these bits when EN = 0 has no effect.
        EN  = 1 << 3, // Enable timer                                                            RW 0
                      // When 1 is written to this bit the timer is enabled and starts
                      // incrementing. The interval setting specified by INT[1:0] is used.
                      // Writing 0 to this bit have no effect.
        INT = 1 << 0, // Timer interval select                                                   RW 0
                      // These bits select the timer interval as follows:
                      // 00: Twdt x 32768
                      // 01: Twdt x 8192
                      // 10: Twdt x 512
                      // 11: Twdt x 64
                      // Writing these bits when EN = 1 has no effect.
    };

    enum PERIOD {
        MS_1_9    = 3, // 1.9ms
        MS_15_625 = 2, // 15.625ms
        S_0_25    = 1, // 0.25s
        S_1       = 0, // 1s
    };

    static volatile Reg32 & reg (unsigned int offset) { return *(reinterpret_cast<volatile Reg32*>(WATCHDOG_BASE + offset)); }

public:
    static void enable() {
        switch(Traits<Watchdog>::PERIOD) {
            case Traits<Watchdog>::MS_1_9:    reg(WDCTL) = PERIOD::MS_1_9;    break;
            case Traits<Watchdog>::MS_15_625: reg(WDCTL) = PERIOD::MS_15_625; break;
            case Traits<Watchdog>::S_0_25:    reg(WDCTL) = PERIOD::S_0_25;    break;
            default:
            case Traits<Watchdog>::S_1:       reg(WDCTL) = PERIOD::S_1;       break;
        }
        reg(WDCTL) |= EN;
    }

    static void kick() {
        reg(WDCTL) = 0xA * CLR;
        reg(WDCTL) = 0x5 * CLR;
    }
};

class Watchdog: private Watchdog_Common, public Watchdog_Engine { };

__END_SYS

#endif

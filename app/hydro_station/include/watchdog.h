#ifndef WATCHDOG_H_
#define WATCHDOG_H_

class Watchdog {
    const static unsigned int BASE = 0x400d5000;

    enum Offset {
        SMWDTHROSC_WDCTL = 0x0
    };
    typedef unsigned int Reg32;
    static volatile Reg32 &reg(Offset o) {
        return *(reinterpret_cast<Reg32*>(BASE + o));
    }

public:
    static volatile Reg32 &wdctl() {
        return reg(SMWDTHROSC_WDCTL);
    }

    enum Mask {
        ENABLE_2MS = 0xB, // really 1.9ms
        ENABLE_15MS = 0xA, // really 15.625ms
        ENABLE_25MS = 0x9,
        ENABLE_1S = 0x8
    };

    Watchdog(Mask timeout = ENABLE_1S)
    {
        wdctl() = timeout;
    }

    void kick() const
    {
        wdctl() = (wdctl() & 0xF) | (0xA << 4);
        wdctl() = (wdctl() & 0xF) | (0x5 << 4);
    }
};

#endif

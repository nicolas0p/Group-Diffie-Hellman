// EPOS YYYY Machine Metainfo and Configuration
#ifndef __yyyy_traits_h
#define __yyyy_traits_h

#include <system/config.h>

__BEGIN_SYS

class YYYY_Common;
template <> struct Traits<YYYY_Common>: public Traits<void>
{
    static const bool debugged = Traits<void>::debugged;
};

template <> struct Traits<YYYY>: public Traits<YYYY_Common>
{
    static const unsigned int CPUS = Traits<Build>::CPUS;

    // Boot Image
    static const unsigned int BOOT_LENGTH_MIN   = 128;
    static const unsigned int BOOT_LENGTH_MAX   = 512;
    static const unsigned int BOOT_IMAGE_ADDR   = 0x00008000;

    // Physical Memory
    static const unsigned int MEM_BASE  = 0x00000000;
    static const unsigned int MEM_TOP   = 0x10000000; // 256 MB (MAX for 32-bit is 0x70000000 / 1792 MB)

    // Logical Memory Map
    static const unsigned int BOOT      = 0x00007c00;
    static const unsigned int SETUP     = 0x00100000; // 1 MB
    static const unsigned int INIT      = 0x00200000; // 2 MB

    static const unsigned int APP_LOW   = 0x00000000;
    static const unsigned int APP_CODE  = 0x00000000;
    static const unsigned int APP_DATA  = 0x00400000; // 4 MB
    static const unsigned int APP_HIGH  = 0x0fffffff; // 256 MB

    static const unsigned int PHY_MEM   = 0x80000000; // 2 GB
    static const unsigned int IO_BASE   = 0xf0000000; // 4 GB - 256 MB
    static const unsigned int IO_TOP    = 0xff400000; // 4 GB - 12 MB

    static const unsigned int SYS       = IO_TOP;     // 4 GB - 12 MB
    static const unsigned int SYS_CODE  = 0xff700000;
    static const unsigned int SYS_DATA  = 0xff740000;
};

template <> struct Traits<YYYY_Bus>: public Traits<YYYY_Common>
{
};

template <> struct Traits<YYYY_IC>: public Traits<YYYY_Common>
{
};

template <> struct Traits<YYYY_Timer>: public Traits<YYYY_Common>
{
    static const bool debugged = hysterically_debugged;

    // Meaningful values for the YYYY's timer frequency range from 100 to
    // 10000 Hz. The choice must respect the scheduler time-slice, i. e.,
    // it must be higher than the scheduler invocation frequency.
    static const int FREQUENCY = 1000; // Hz
};

template <> struct Traits<YYYY_RTC>: public Traits<YYYY_Common>
{
    static const unsigned int EPOCH_DAY = 1;
    static const unsigned int EPOCH_MONTH = 1;
    static const unsigned int EPOCH_YEAR = 1970;
    static const unsigned int EPOCH_DAYS = 719499;
};

template <> struct Traits<YYYY_EEPROM>: public Traits<YYYY_Common>
{
};

template <> struct Traits<YYYY_UART>: public Traits<YYYY_Common>
{
    static const unsigned int CLOCK = 9600;
};

template <> struct Traits<YYYY_Display>: public Traits<YYYY_Common>
{
    static const int COLUMNS = 80;
    static const int LINES = 25;
    static const int TAB_SIZE = 8;
    static const unsigned int FRAME_BUFFER_ADDRESS = 0xb8000;
};

template <> struct Traits<YYYY_Scratchpad>: public Traits<YYYY_Common>
{
    static const bool enabled = false;
    static const unsigned int ADDRESS = 0xf0000000;
    static const unsigned int SIZE = 256 * 1024;
};

__END_SYS

#endif

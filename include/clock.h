// EPOS Clock Component Declarations

#ifndef __clock_h
#define __clock_h

#include <rtc.h>

__BEGIN_SYS

class Clock
{
public:
    typedef RTC::Microsecond Microsecond;
    typedef RTC::Second Second;
    typedef RTC::Date Date;

public:
    Clock() {}

    Microsecond resolution() { return 1000000; }

    Second now() { return RTC::seconds_since_epoch(); }

    Date date() { return RTC::date(); }
    void date(const Date & d) { return RTC::date(d); }
};

__END_SYS

#endif

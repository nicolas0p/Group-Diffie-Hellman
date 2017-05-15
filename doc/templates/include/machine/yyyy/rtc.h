// EPOS YYYY Real-Time Clock Mediator Declarations

#ifndef __yyyy_rtc_h
#define __yyyy_rtc_h

#include <cpu.h>
#include <rtc.h>

__BEGIN_SYS

// CHIP_YYYY_RTC
class CHIP_YYYY_RTC
{
};

class YYYY_RTC: public RTC_Common, private CHIP_YYYY_RTC
{
    friend class YYYY;

private:
    static const unsigned int EPOCH_YEAR = Traits<YYYY_RTC>::EPOCH_YEAR;
    static const unsigned int EPOCH_DAYS = Traits<YYYY_RTC>::EPOCH_DAYS;

public:
    YYYY_RTC() {} 

    static Date date();
    static void date(const Date & d);

    static Second seconds_since_epoch() { return date().to_offset(EPOCH_DAYS); }

private:
    static void init();
};

__END_SYS

#endif

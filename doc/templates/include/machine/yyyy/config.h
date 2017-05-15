// EPOS YYYY Mediators Configuration

#ifndef __yy_config_h
#define __yy_config_h

#include <system/meta.h>
#include __APPLICATION_TRAITS_H

#define __CPU_H         __HEADER_ARCH(cpu)
#define __TSC_H         __HEADER_ARCH(tsc)
#define __MMU_H         __HEADER_ARCH(mmu)

#define __MACH_H        __HEADER_MACH(machine)
//#define __BUS_H         __HEADER_MACH(bus)
#define __IC_H          __HEADER_MACH(ic)
#define __TIMER_H       __HEADER_MACH(timer)
//#define __RTC_H         __HEADER_MACH(rtc)
//#define __EEPROM_H      __HEADER_MACH(eeprom)
#define __UART_H        __HEADER_MACH(uart)
//#define __DISPLAY_H     __HEADER_MACH(display)
//#define __NIC_H         __HEADER_MACH(nic)
//#define __SCRATCHPAD_H  __HEADER_MACH(scratchpad)

__BEGIN_SYS

typedef XXXX            CPU;
typedef XXXX_MMU        MMU;
typedef XXXX_TSC        TSC;

typedef YYYY            Machine;
//typedef YYYY_BUS        BUS;
typedef YYYY_IC         IC;
typedef YYYY_Timer      Timer;
//typedef YYYY_RTC        RTC;
//typedef YYYY_EEPROM     EEPROM;
typedef YYYY_UART       UART;
typedef IF<Traits<Serial_Display>::enabled, Serial_Display, YYYY_Display>::Result Display;
//typedef YYYY_NIC        NIC;
//typedef YYYY_Scratchpad Scratchpad;

__END_SYS

#endif

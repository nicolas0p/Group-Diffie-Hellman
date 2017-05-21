// EPOS ARM Cortex Mediator Declarations

#ifndef __cortex_h
#define __cortex_h

#include <utility/list.h>
#include <cpu.h>
#include <mmu.h>
#include <tsc.h>
#include <machine.h>
#include <rtc.h>
#include __MODEL_H
#include "info.h"
#include "memory_map.h"
#include "ic.h"
#include <display.h>

__BEGIN_SYS

class Machine: private Machine_Common, private Machine_Model
{
    friend class Init_System;
    friend class First_Object;

public:
    Machine() {}

    static void delay(const RTC::Microsecond & time) { Machine_Model::delay(time); }

    static void panic();
    static void reboot();
    static void poweroff() { reboot(); }

    static unsigned int n_cpus() { return 1; }
    static unsigned int cpu_id() { return 0; }

    static void smp_barrier() {};
    static void smp_init(unsigned int) {};

    static const unsigned char * id() { return Machine_Model::id(); }

private:
    static void pre_init(System_Info * si) {
        Machine_Model::pre_init();

        Display::init();

        if(Traits<System>::multicore)
            smp_init(si->bm.n_cpus);
    }

    static void init();
};

__END_SYS

#ifdef __TIMER_H
#include __TIMER_H
#endif
#ifdef __RTC_H
#include __RTC_H
#endif
#ifdef __UART_H
#include __UART_H
#endif
#ifdef __USB_H
#include __USB_H
#endif
#ifdef __DISPLAY_H
#include __DISPLAY_H
#endif
#ifdef __GPIO_H
#include __GPIO_H
#endif
#ifdef __NIC_H
#include __NIC_H
#endif
#ifdef __ADC_H
#include __ADC_H
#endif

#endif

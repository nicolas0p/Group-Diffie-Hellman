// EPOS YYYY Mediator Declarations

#ifndef __yyyy_h
#define __yyyy_h

#include <utility/list.h>
#include <cpu.h>
#include <mmu.h>
#include <tsc.h>
#include <machine.h>
#include <rtc.h>
#include "info.h"
#include "memory_map.h"
#include "ic.h"

__BEGIN_SYS

class YYYY: public Machine_Common
{
    friend class Init_System;

private:
    static const bool smp = Traits<System>::multicore;
    
public:
    YYYY() {}
  
    static void delay(const RTC::Microsecond & time);

    static void panic();
    static void reboot();
    static void poweroff();

    static unsigned int n_cpus() { return smp ? _n_cpus : 1; }
    static unsigned int cpu_id() { return smp ? YYYY_SMP_Controller::id() : 0; }

    static void smp_init(unsigned int n_cpus) {
        if(smp) {
            _n_cpus = n_cpus;
            YYYY_SMP_Controller::init();
        }
    };

    static void smp_barrier(unsigned long n_cpus = _n_cpus) {
        static volatile unsigned long ready[2];
        static volatile unsigned long i;

        if(smp) {
            int j = i;

            CPU::finc(ready[j]);

            if(cpu_id() == 0) {
        	while(ready[j] < n_cpus); // wait for all CPUs to be ready
        	i = !i;                   // toggle ready
        	ready[j] = 0;             // signalizes waiting CPUs
            } else
        	while(ready[j]);          // wait for CPU[0] signal
        }
    }

private:
    static void init();

private:
    static volatile unsigned int _n_cpus;
};

__END_SYS

#include "pci.h"
#include "timer.h"
#include "rtc.h"
#include "eeprom.h"
#include "uart.h"
#include "display.h"
#include "scratchpad.h"

#endif

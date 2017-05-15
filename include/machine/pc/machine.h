// EPOS PC Mediator Declarations

#ifndef __pc_h
#define __pc_h

#include <utility/list.h>
#include <cpu.h>
#include <mmu.h>
#include <tsc.h>
#include <machine.h>
#include <rtc.h>
#include "info.h"
#include "memory_map.h"
#include "ic.h"
#include <display.h>
#include <system.h>

__BEGIN_SYS

class Machine: public Machine_Common
{
    friend class Init_System;
    friend class First_Object;

private:
    static const bool smp = Traits<System>::multicore;

    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;

public:
    Machine() {}

    static void delay(const RTC::Microsecond & time) {
        TSC::Time_Stamp end = TSC::time_stamp() + time * (TSC::frequency() / 1000000);
        while(end > TSC::time_stamp());
    }

    static void panic();
    static void reboot();
    static void poweroff();

    static unsigned int n_cpus() { return smp ? _n_cpus : 1; }
    static unsigned int cpu_id() { return smp ? APIC::id() : 0; }

    static void smp_init(unsigned int n_cpus) {
        if(smp) {
            _n_cpus = n_cpus;
            APIC::remap();
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

    static const unsigned char * id() { return System::info()->bm.uuid; }

private:
    static void pre_init(System_Info * si) {
        Display::init();

        if(Traits<System>::multicore)
            smp_init(si->bm.n_cpus);
    }

    static void init();

private:
    static volatile unsigned int _n_cpus;
};

__END_SYS

#include "pci.h"
#include "timer.h"

#ifdef __RTC_H
#include __RTC_H
#endif
#ifdef __EEPROM_H
#include __EEPROM_H
#endif
#ifdef __UART_H
#include __UART_H
#endif
#ifdef __DISPLAY_H
#include __DISPLAY_H
#endif
#ifdef __KEYBOARD_H
#include __KEYBOARD_H
#endif
#ifdef __SCRATCHPAD_H
#include __SCRATCHPAD_H
#endif
#ifdef __NIC_H
#include __NIC_H
#endif
#ifdef __FPGA_H
#include __FPGA_H
#endif

#endif

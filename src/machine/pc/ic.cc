// EPOS PC IC Mediator Implementation

#include <machine/pc/ic.h>
#include <machine.h>

__BEGIN_SYS

// Class attributes
APIC::Log_Addr APIC::_base;
IC::Interrupt_Handler IC::_int_vector[IC::INTS];


// APIC class methods
void APIC::ipi_init(volatile int * status)
{
//    // Broadcast INIT IPI to all APs excluding self
//    write(ICR0_31, ICR_OTHERS | ICR_LEVEL | ICR_ASSERT | ICR_INIT);
//    while((read(ICR0_31) & ICR_PENDING));

    status[0] = 0;

    // Send INIT IPI to all APs excluding self
    for(unsigned int n = 1; n < Traits<Machine>::CPUS; n++) {
        status[n] = 0;
        write(ICR32_63, n << 24);
        write(ICR0_31, ICR_LEVEL | ICR_ASSERT | ICR_INIT);
        while((read(ICR0_31) & ICR_PENDING));
    }

    i8255::ms_delay(100);
};

void APIC::ipi_start(Log_Addr entry, volatile int * status)
{
//    unsigned int vector = (entry >> 12) & 0xff;
//
//    // Broadcast STARTUP IPI to all APs excluding self twice
//    write(ICR0_31, ICR_OTHERS | ICR_LEVEL | ICR_ASSERT | ICR_STARTUP | vector);
//    while((read(ICR0_31) & ICR_PENDING));
//
//    i8255::ms_delay(10); // ~ 10ms delay
//
//    write(ICR0_31, ICR_OTHERS | ICR_LEVEL | ICR_ASSERT | ICR_STARTUP | vector);
//    while((read(ICR0_31) & ICR_PENDING));
//
//    // Give other CPUs a time to wake up (> 100ms)
//    i8255::ms_delay(100);

    unsigned int vector = (entry >> 12) & 0xff;

    // Send STARTUP IPI to all APs
    for(unsigned int n = 1; n < Traits<Machine>::CPUS; n++) {
        write(ICR32_63, n << 24);
        write(ICR0_31, ICR_LEVEL | ICR_ASSERT | ICR_STARTUP | vector);
        while((read(ICR0_31) & ICR_PENDING));
    }

    // Give them time to wake up (> 100ms)
    i8255::ms_delay(500);

    for(unsigned int n = 1; n < Traits<Machine>::CPUS; n++) {
        if(status[n] == 1) { // CPU is up
            CPU::finc(status[n]);
            i8255::ms_delay(30);
        } else { // send a second STARTUP IPI to the CPU
            write(ICR32_63, n << 24);
            write(ICR0_31, ICR_LEVEL | ICR_ASSERT | ICR_STARTUP | vector);
            i8255::ms_delay(500);
            if(status[n] == 1) // CPU is up
                CPU::finc(status[n]);
            // else CPU was not initialized
        }
    }
}

void APIC::ipi_send(unsigned int cpu, unsigned int interrupt)
{
    write(ICR32_63, (cpu << 24));
    write(ICR0_31, ICR_LEVEL | ICR_ASSERT | ICR_FIXED | interrupt);
    while((read(ICR0_31) & ICR_PENDING));
}

__END_SYS

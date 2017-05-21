// EPOS eMote3 (ARM Cortex-M3) MCU Mediator Initialization

#include <machine.h>

#ifdef __mmod_emote3__

#include <ic.h>

__BEGIN_SYS

void Machine_Model::pre_init()
{
    db<Init, Machine>(TRC) << "Machine_Model::pre_init()" << endl;

    // Clock setup
    Reg32 sys_div;
    switch(Traits<CPU>::CLOCK) {
        default:
        case 32000000: sys_div = 0; break;
        case 16000000: sys_div = 1; break;
        case  8000000: sys_div = 2; break;
        case  4000000: sys_div = 3; break;
        case  2000000: sys_div = 4; break;
        case  1000000: sys_div = 5; break;
        case   500000: sys_div = 6; break;
        case   250000: sys_div = 7; break;
    }

    // Set pins PD6 and PD7 to enable external oscillator
    Reg32 pin_bit = (1 << 6) | (1 << 7);
    gpiod(AFSEL) &= ~pin_bit; // Set pins D6 and D7 as software-controlled
    gpiod(DIR) &= ~pin_bit; // Set pins D6 and D7 as output
    ioc(PD6_OVER) = ANA;
    ioc(PD7_OVER) = ANA;

    Reg32 clock_ctrl = scr(CLOCK_CTRL) & ~(SYS_DIV * 7);
    clock_ctrl |= sys_div * SYS_DIV; // Set system clock rate
    clock_ctrl |= AMP_DET; // Enable AMP detect to make sure XOSC starts correctly
    clock_ctrl |= OSC_PD; // Power down unused oscillator
    clock_ctrl &= ~OSC; // Select 32Mhz oscillator
    clock_ctrl &= ~OSC32K; // Select 32Khz crystal oscillator

    scr(CLOCK_CTRL) = clock_ctrl; // Write back to register

    // Wait until oscillators stabilize
    while((scr(CLOCK_STA) & (STA_OSC | STA_OSC32K)));

    clock_ctrl = scr(CLOCK_CTRL) & ~(IO_DIV * 7);
    scr(CLOCK_CTRL) = clock_ctrl | (sys_div * IO_DIV); // Set IO clock rate

    while(!(scr(CLOCK_STA) & (STA_SYNC_32K)));
}

void Machine_Model::init()
{
    db<Init, Machine>(TRC) << "Machine_Model::init()" << endl;

    scs(CCR) |= BASETHR; // The processor can enter thread mode from any level

    scr(I_MAP) |= I_MAP_ALTMAP; // Enable alternate interrupt mapping

    scs(VTOR) = (Traits<Machine>::SYS_CODE) & ~(1 << 29); // Set the vector table offset (must be 512-byte aligned)
}

__END_SYS

#endif

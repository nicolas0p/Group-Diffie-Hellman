// EPOS LM3S811 (ARM Cortex-M3) MCU Mediator Declarations

#ifndef __lm3s811_h
#define __lm3s811_h

#include <cpu.h>
#include <tsc.h>
#include <rtc.h>
#include <system.h>

__BEGIN_SYS

class LM3S811
{
    friend class TSC;

protected:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;

public:
    static const unsigned int IRQS = 30;
    static const unsigned int TIMERS = 2; // This model has 3 timers, but QEMU (v2.7.50) only implements 2
    static const unsigned int UARTS = 2;
    static const unsigned int GPIO_PORTS = 5;
    static const bool supports_gpio_power_up = false;

// SCR
    // Base address for memory-mapped System Control Register
    enum {
        SCR_BASE        = 0x400fe000
    };

    // System Control Registers offsets
    enum {                              // Description                                          Type    Value after reset
        DID0            = 0x000,        // Device Identification 0                              ro      -
        DID1            = 0x004,        // Device Identification 1                              ro      -
        DC0             = 0x008,        // Device Capabilities 0                                ro      0x001f001f
        DC1             = 0x010,        // Device Capabilities 1                                ro      0x001132bf
        DC2             = 0x014,        // Device Capabilities 2                                ro      0x01071013
        DC3             = 0x018,        // Device Capabilities 3                                ro      0xbf0f01ff
        DC4             = 0x01c,        // Device Capabilities 4                                ro      0x0000001f
        PBORCTL         = 0x030,        // Power-On and Brown-Out Reset Control                 rw      0x00007ffd
        LDOPCTL         = 0x034,        // LDO Power Control                                    rw      0x00000000
        SRCR0           = 0x040,        // Software Reset Control 0                             rw      0x00000000
        SRCR1           = 0x044,        // Software Reset Control 1                             rw      0x00000000
        SRCR2           = 0x048,        // Software Reset Control 2                             rw      0x00000000
        RIS             = 0x050,        // Raw Interrupt Status                                 ro      0x00000000
        IMC             = 0x054,        // Interrupt Mask Control                               rw      0x00000000
        MISC            = 0x058,        // Masked Interrupt Status and Clear                    rw 1C   0x00000000
        RESC            = 0x05c,        // Reset Cause  rw      -
        RCC             = 0x060,        // Run-Mode Clock Configuration                         rw      0x078e3ac0
        PLLCFG          = 0x064,        // XTAL to PLL Translation                              ro      -
        RCGC0           = 0x100,        // Run Mode Clock Gating Control Register 0             rw      0x00000040
        RCGC1           = 0x104,        // Run Mode Clock Gating Control Register 1             rw      0x00000000
        RCGC2           = 0x108,        // Run Mode Clock Gating Control Register 2             rw      0x00000000
        SCGC0           = 0x110,        // Sleep Mode Clock Gating Control Register 0           rw      0x00000040
        SCGC1           = 0x114,        // Sleep Mode Clock Gating Control Register 1           rw      0x00000000
        SCGC2           = 0x118,        // Sleep Mode Clock Gating Control Register 2           rw      0x00000000
        DCGC0           = 0x120,        // Deep Sleep Mode Clock Gating Control Register 0      rw      0x00000040
        DCGC1           = 0x124,        // Deep Sleep Mode Clock Gating Control Register 1      rw      0x00000000
        DCGC2           = 0x128,        // Deep Sleep Mode Clock Gating Control Register 2      rw      0x00000000
        DSLPCLKCFG      = 0x144,        // Sleep Clock Configuration                            rw      0x07800000
        CLKVCLR         = 0x150,        // Clock Verification Clear                             rw      0x00000000
        LDOARST         = 0x160         // Allow Unregulated LDO to Reset the Part              rw      0x00000000
    };

    // Useful Bits in the Run Mode Clock Gating Control Register 0
    enum RCGC0 {                        // Description                                          Type    Value after reset
        RCGC0_WDT       = 1 <<  3,      // Watch Dog Timer                                      rw      0
        RCGC0_ADC_125K  = 0 <<  8,      // ADC Max Speed = 125K samp/s                          rw      0
        RCGC0_ADC_250K  = 1 <<  8,      // ADC Max Speed = 250K samp/s                          rw      0
        RCGC0_ADC_500K  = 2 <<  8,      // ADC Max Speed = 500K samp/s                          rw      0
        RCGC0_ADC       = 1 << 16,      // ADC                                                  rw      0
        RCGC0_PWM       = 1 << 20       // PWM                                                  rw      0
    };

    // Useful Bits in the Run Mode Clock Gating Control Register 1
    enum RCGC1 {                        // Description                                          Type    Value after reset
        RCGC1_UART0     = 1 <<  0,      // UART0                                                rw      0
        RCGC1_UART1     = 1 <<  1,      // UART1                                                rw      0
        RCGC1_SSI       = 1 <<  4,      // Synchronous Serial Interface                         rw      0
        RCGC1_I2C       = 1 << 12,      // I2C                                                  rw      0
        RCGC1_TIMER0    = 1 << 16,      // Timer 0                                              rw      0
        RCGC1_TIMER1    = 1 << 17,      // Timer 1                                              rw      0
        RCGC1_TIMER2    = 1 << 18,      // Timer 2                                              rw      0
        RCGC1_COMP0     = 1 << 24       // Analog Comparator 0                                  rw      0
    };

    // Useful Bits in the Run Mode Clock Gating Control Register 2
    enum RCGC2 {                        // Description                                          Type    Value after reset
        RCGC2_GPIOA     = 1 <<  0,      // GPIOA                                                rw      0
        RCGC2_GPIOB     = 1 <<  1,      // GPIOB                                                rw      0
        RCGC2_GPIOC     = 1 <<  2,      // GPIOC                                                rw      0
        RCGC2_GPIOD     = 1 <<  3,      // GPIOD                                                rw      0
        RCGC2_GPIOE     = 1 <<  4       // GPIOE                                                rw      0
    };

    // Useful Bits in the Run-Mode Clock Configuration
    enum RCC {                          // Description                                          Type    Value after reset
        RCC_MOSCDIS     = 1 <<  0,      // Main Oscillator Disable                              rw      0
        RCC_IOSCDIS     = 1 <<  1,      // Internal Oscillator Disable                          rw      0
        RCC_MOSCVER     = 1 <<  2,      // Main Oscillator Verification Timer                   rw      0
        RCC_IOSCVER     = 1 <<  3,      // Internal Oscillator Verification Timer               rw      0
        RCC_OSCSRC      = 0 <<  4,      // Oscillator Source                                    rw      0
        RCC_MOSC        = 0 <<  4,      // Oscillator Source = Main oscillator                  rw      0
        RCC_IOSC        = 1 <<  4,      // Oscillator Source = Internal oscillator              rw      0
        RCC_IOSC4       = 2 <<  4,      // Oscillator Source = Internal oscillator / 4          rw      0
        RCC_XTAL        = 0x0 << 6,     // Crystal Frequency                                    rw      0xb
        RCC_XTAL_1000   = 0x0 << 6,     // Crystal Frequency = 1 MHz (no PLL)
        RCC_XTAL_1843   = 0x1 << 6,     // Crystal Frequency = 1.8432 MHz (no PLL)
        RCC_XTAL_2000   = 0x2 << 6,     // Crystal Frequency = 2 MHz (no PLL)
        RCC_XTAL_2457   = 0x3 << 6,     // Crystal Frequency = 2.4576 MHz (no PLL)
        RCC_XTAL_3579   = 0x4 << 6,     // Crystal Frequency = 3.579545 MHz
        RCC_XTAL_3686   = 0x5 << 6,     // Crystal Frequency = 3.6864 MHz
        RCC_XTAL_4000   = 0x6 << 6,     // Crystal Frequency = 4 MHz
        RCC_XTAL_4096   = 0x7 << 6,     // Crystal Frequency = 4.096 MHz
        RCC_XTAL_4915   = 0x8 << 6,     // Crystal Frequency = 4.9152 MHz
        RCC_XTAL_5000   = 0x9 << 6,     // Crystal Frequency = 5 MHz
        RCC_XTAL_5120   = 0xa << 6,     // Crystal Frequency = 5.12 MHz
        RCC_XTAL_6000   = 0xb << 6,     // Crystal Frequency = 6 MHz
        RCC_XTAL_6144   = 0xc << 6,     // Crystal Frequency = 6.144 MHz
        RCC_XTAL_7378   = 0xd << 6,     // Crystal Frequency = 7.3728 MHz
        RCC_XTAL_8000   = 0xe << 6,     // Crystal Frequency = 8 MHz
        RCC_XTAL_8192   = 0xf << 6,     // Crystal Frequency = 8.129 MHz
        RCC_PLLVER      = 1 << 10,      // PLL Verification                                     rw      0
        RCC_BYPASS      = 1 << 11,      // PLL Bypass                                           rw      1
        RCC_OEN         = 1 << 12,      // PLL Output Enable                                    rw      1
        RCC_PWRDN       = 1 << 13,      // PLL Power Down                                       rw      1
        RCC_PWMDIV      = 1 << 17,      // PWM Unit Clock Divisor                               rw      0x7
        RCC_USEPWMDIV   = 1 << 20,      // Enable PWM Clock Divisor                             rw      0
        RCC_USESYSDIV   = 1 << 22,      // Enable System Clock Divider                          rw      0
        RCC_SYSDIV      = 0x0 << 23,    // System Clock Divisor                                 rw      0xf
        RCC_SYSDIV_1    = 0x0 << 23,    // System Clock Divisor = 2
        RCC_SYSDIV_4    = 0x3 << 23,    // System Clock Divisor = 4 -> 50 MHz
        RCC_SYSDIV_5    = 0x4 << 23,    // System Clock Divisor = 5 -> 40 HMz
        RCC_SYSDIV_8    = 0x7 << 23,    // System Clock Divisor = 8 -> 25 MHz
        RCC_SYSDIV_10   = 0x9 << 23,    // System Clock Divisor = 10 -> 20 MHz
        RCC_SYSDIV_16   = 0xf << 23,    // System Clock Divisor = 16 -> 12.5 HMz
        RCC_ACG         = 1 << 27       // ACG                                                  rw      0
    };

    // Useful Bits in the Raw Interrupt Status
    enum RIS {                          // Description                                          Type    Value after reset
        RIS_PLLLRIS     = 1 <<  6       // PLL Lock Raw Interrupt Status                        ro      0
    };


// SCS
    // Base address for memory-mapped System Control Space
    enum {
        SCS_BASE        = 0xe000e000
    };

    // System Control Space offsets
    enum {                              // Description                                          Type    Value after reset
        MCR             = 0x000,        // Master Control Register                              -       0x00000000
        ICTR            = 0x004,        // Interrupt Controller Type Register                   ro      0x????????
        ACTLR           = 0x008,        // Auxiliary Control Register                           rw      0x????????
        STCTRL          = 0x010,        // SysTick Control and Status Register                  rw      0x00000000
        STRELOAD        = 0x014,        // SysTick Reload Value Register                        rw      0x00000000
        STCURRENT       = 0x018,        // SysTick Current Value Register                       rw      0x00000000
        IRQ_ENABLE0     = 0x100,        // Interrupt 0-31 Set Enable                            rw      0x00000000
        IRQ_ENABLE1     = 0x100,        // Inexistent in this model, defined for machine completion
        IRQ_ENABLE2     = 0x100,        // Inexistent in this model, defined for machine completion
        IRQ_DISABLE0    = 0x180,        // Interrupt 0-31 Clear Enable                          rw      0x00000000
        IRQ_DISABLE1    = 0x180,        // Inexistent in this model, defined for machine completion
        IRQ_DISABLE2    = 0x180,        // Inexistent in this model, defined for machine completion
        IRQ_PEND0       = 0x200,        // Interrupt 0-31 Set Pending                           rw      0x00000000
        IRQ_PEND1       = 0x200,        // Inexistent in this model, defined for machine completion
        IRQ_PEND2       = 0x200,        // Inexistent in this model, defined for machine completion
        IRQ_UNPEND0     = 0x280,        // Interrupt 0-31 Clear Pending                         rw      0x00000000
        IRQ_UNPEND1     = 0x280,        // Inexistent in this model, defined for machine completion
        IRQ_UNPEND2     = 0x280,        // Inexistent in this model, defined for machine completion
        IRQ_ACTIVE0     = 0x300,        // Interrupt 0-31 Active Bit                            rw      0x00000000
        IRQ_ACTIVE1     = 0x300,        // Inexistent in this model, defined for machine completion
        IRQ_ACTIVE2     = 0x300,        // Inexistent in this model, defined for machine completion
        CPUID           = 0xd00,        // CPUID Base Register                                  ro      0x410fc231
        INTCTRL         = 0xd04,        // Interrupt Control and State Register                 rw      0x00000000
        VTOR            = 0xd08,        // Vector Table Offset Register                         rw      0x00000000
        AIRCR           = 0xd0c,        // Application Interrupt/Reset Control Register         rw
        SCR             = 0xd10,        // System Control Register                              rw      0x00000000
        CCR             = 0xd14,        // Configuration Control Register                       rw      0x00000000
        SHPR1           = 0xd18,        // System Handlers 4-7 Priority                         rw      0x00000000
        SHPR2           = 0xd1c,        // System Handlers 8-11 Priority                        rw      0x00000000
        SHPR3           = 0xd20,        // System Handlers 12-15 Priority                       rw      0x00000000
        SHCSR           = 0xd24,        // System Handler Control and State Register            rw      0x00000000
        CFSR            = 0xd28,        // Configurable Fault Status Register                   rw      0x00000000
        SWTRIG          = 0xf00         // Software Trigger Interrupt Register                  wo      0x00000000
    };

    // Useful Bits in the ISysTick Control and Status Register
    enum STCTRL {                       // Description                                          Type    Value after reset
        ENABLE          = 1 <<  0,      // Enable / disable                                     rw      0
        INTEN           = 1 <<  1,      // Interrupt pending                                    rw      0
        CLKSRC          = 1 <<  2,      // Clock source (0 -> external, 1 -> core)              rw      0
        COUNT           = 1 << 16       // Count underflow                                      ro      0
    };

    // Useful Bits in the Interrupt Control and State Register
    enum INTCTRL {                      // Description                                          Type    Value after reset
        ICSR_ACTIVE     = 1 <<  0,      // Active exceptions (IPSR mirror, 0 -> thread mode)    ro
        ICSR_PENDING    = 1 << 12,      // Pending exceptions (0 -> none)                       ro
        ICSR_ISRPENDING = 1 << 22,      // Pending NVIC IRQ                                     ro
        ICSR_SYSPENDING = 1 << 25       // Clear pending SysTick                                wo
    };

    // Useful Bits in the Application Interrupt/Reset Control Register
    enum AIRCR {                        // Description                                          Type    Value after reset
        VECTRESET       = 1 << 0,       // Reserved for debug                                   wo      0
        VECTCLRACT      = 1 << 1,       // Reserved for debug                                   wo      0
        SYSRESREQ       = 1 << 2,       // System Reset Request                                 wo      0
        VECTKEY         = 1 << 16,      // Register Key                                         rw      0xfa05
                                        // This field is used to guard against accidental
                                        // writes to this register.  0x05FA must be written
                                        // to this field in order to change the bits in this
                                        // register. On a read, 0xFA05 is returned.
    };

    // Useful Bits in the Configuration Control Register
    enum CCR {                          // Description                                          Type    Value after reset
        BASETHR         = 1 <<  0,      // Thread state can be entered at any level of int.     rw      0
        MAINPEND        = 1 <<  1,      // SWTRIG can be written to in user mode                rw      0
        UNALIGNED       = 1 <<  3,      // Trap on unaligned memory access                      rw      0
        DIV0            = 1 <<  4,      // Trap on division by zero                             rw      0
        BFHFNMIGN       = 1 <<  8,      // Ignore Precise Data Access Faults for pri -1 and -2  rw      0
        STKALIGN        = 1 <<  9       // Align stack point on exception entry to 8 butes      rw      0
    };


// GPTM
    // Base address for memory-mapped GPTM registers
    enum {
        TIMER0_BASE     = 0x40030000,
        TIMER1_BASE     = 0x40031000,
        TIMER2_BASE     = 0x40032000,
        TIMER3_BASE     = 0, // Not present in this model
    };

    // GPTM registers offsets
    enum {                              // Description                                          Type    Value after reset
        GPTMCFG         =   0x00,       //                                                      rw      0x00000000
        GPTMTAMR        =   0x04,       //                                                      rw      0x00000000
        GPTMTBMR        =   0x08,       //                                                      rw      0x00000000
        GPTMCTL         =   0x0C,       //                                                      rw      0x00000000
        GPTMIMR         =   0x18,       //                                                      rw      0x00000000
        GPTMRIS         =   0x1C,       //                                                      ro      0x00000000
        GPTMMIS         =   0x20,       //                                                      ro      0x00000000
        GPTMICR         =   0x24,       //                                                      rw      0x00000000
        GPTMTAILR       =   0x28,       //                                                      rw      0xffffffff
        GPTMTBILR       =   0x2C,       //                                                      rw      0x0000ffff
        GPTMTAMATCHR    =   0x30,       //                                                      rw      0xffffffff
        GPTMTBMATCHR    =   0x34,       //                                                      rw      0x0000ffff
        GPTMTAPR        =   0x38,       //                                                      rw      0x00000000
        GPTMTBPR        =   0x3C,       //                                                      rw      0x00000000
        GPTMTAPMR       =   0x40,       //                                                      rw      0x00000000
        GPTMTBPMR       =   0x44,       //                                                      rw      0x00000000
        GPTMTAR         =   0x48,       //                                                      ro      0xffffffff
        GPTMTBR         =   0x4C,       //                                                      ro      0x0000ffff
    };

    enum GPTMCTL {                      // Description
        TAEN            = 1 << 0,       // Timer A enable
        TASTALL         = 1 << 1,       // Timer A stall enable (0 -> continues counting while the processor is halted by the debugger, 1 -> freezes)
        TAEVENT         = 1 << 2,       // Timer A event mode (0 -> positive edge, 1 -> negative edge, 2 -> reserved, 3 -> both edges)
        TAOTE           = 1 << 5,       // Timer A output ADC trigger enable
        TAPWML          = 1 << 6,       // Timer A PWM output level (0 -> direct, 1 -> inverted)
        TBEN            = 1 << 8,       // Timer B enable
        TBSTALL         = 1 << 9,       // Timer B stall enable (0 -> continues counting while the processor is halted by the debugger, 1 -> freezes)
        TBEVENT         = 1 << 10,      // Timer B event mode (0 -> positive edge, 1 -> negative edge, 2 -> reserved, 3 -> both edges)
        TBOTE           = 1 << 13,      // Timer B output ADC trigger enable
        TBPWML          = 1 << 14,      // Timer B PWM output level (0 -> direct, 1 -> inverted)
    };

    enum GPTMTMR {                      // Description
        TMR             = 1 << 0,       // Timer A mode (0 -> reserved, 1 -> one-shot, 2 -> periodic, 3 -> capture)
        TCMR            = 1 << 2,       // Timer A capture mode (0 -> edge-count, 1 -> edge-time)
        TAMS            = 1 << 3,       // Timer A alternate mode (0 -> capture, 1 -> PWM) mode is enabled.
        TCDIR           = 1 << 4,       // Timer A count direction (0 -> descending, 1 -> ascending from 0)
        TMIE            = 1 << 5,       // Timer A match interrupt enable (GPTMTAMATCHR register is reached)
        TWOT            = 1 << 6,       // Timer A wait-on-trigger (wait for a trigger from the Timer in the previous position in the daisy-chain to start counting; must be clear for Timer A0)
        TSNAPS          = 1 << 7,       // Timer A snap-shot mode enable (in periodic mode, the actual free-running value of Timer A is loaded at the time-out event into GPTMTAR
        TILD            = 1 << 8,       // Timer A PWM interval load write (0 -> next cycle, 1 -> next time-out)
        TPWMIE          = 1 << 9,       // Timer A PWM interrupt enable (valid only in PWM mode)
        TMRSU           = 1 << 10,      // Timer A match register update mode (0 -> next cycle, 1 -> next time-out)
        TPLO            = 1 << 11,      // Legacy PWM operation (0 -> legacy operation, 1 -> CCP is set to 1 on time-out)
    };

    enum GPTMIR {           // Description                    Type Reset value
        TATO_INT = 1 << 0,  // Timer A time-out interrupt       RW 0
        CAM_INT  = 1 << 1,  // Timer A capture match interrupt  RW 0
        CAE_INT  = 1 << 2,  // Timer A capture event Interrupt  RW 0
        TAM_INT  = 1 << 4,  // Timer A match interrupt          RW 0
        TBTO_INT = 1 << 8,  // Timer B time-out interrupt       RW 0
        CBM_INT  = 1 << 9,  // Timer B capture match interrupt  RW 0
        CBE_INT  = 1 << 10, // Timer B capture event Interrupt  RW 0
        TBM_INT  = 1 << 11, // Timer B match interrupt          RW 0
        WUE_INT  = 1 << 16, // write update error interrupt     RW 0
    };

// GPIO
    // Base address for memory-mapped GPIO Ports Registers
    enum {
        GPIOA_BASE      = 0x40004000,   // GPIO Port A
        GPIOB_BASE      = 0x40005000,   // GPIO Port B
        GPIOC_BASE      = 0x40006000,   // GPIO Port C
        GPIOD_BASE      = 0x40007000,   // GPIO Port D
        GPIOE_BASE      = 0x40024000    // GPIO Port E
    };

    // GPIO Ports Registers offsets
    enum {                              // Description                  Type    Value after reset
        DATA		= 0x000,	// Data  	                rw 	0x0000.0000
        DIR		= 0x400,	// Direction    	        rw 	0x0000.0000
        IS		= 0x404,	// Interrupt Sense      	rw 	0x0000.0000
        IBE		= 0x408,	// Interrupt Both Edges 	rw 	0x0000.0000
        IEV		= 0x40c,	// Interrupt Event 	        rw 	0x0000.0000
        IM		= 0x410,	// Interrupt Mask 	        rw 	0x0000.0000
        GRIS		= 0x414,	// Raw Interrupt Status 	ro	0x0000.0000
        MIS		= 0x418,	// Masked Interrupt Status	ro	0x0000.0000
        ICR		= 0x41c,	// Interrupt Clear 	        W1C	0x0000.0000
        AFSEL		= 0x420,	// Alternate Function Select	rw 	-
        DR2R		= 0x500,	// 2-mA Drive Select	        rw 	0x0000.00ff
        DR4R		= 0x504,	// 4-mA Drive Select	        rw 	0x0000.0000
        DR8R		= 0x508,	// 8-mA Drive Select	        rw 	0x0000.0000
        ODR		= 0x50c,	// Open Drain Select	        rw 	0x0000.0000
        PUR		= 0x510,	// Pull-Up Select 	        rw 	0x0000.00ff
        PDR		= 0x514,	// Pull-Down Select 	        rw 	0x0000.0000
        SLR		= 0x518,	// Slew Rate Control Select	rw 	0x0000.0000
        DEN		= 0x51c,	// Digital Enable 	        rw 	0x0000.00ff
        P_EDGE_CTRL     = 0x704,        // Power-up Int. Edge Control   rw      0x0000.0000
        PI_IEN          = 0x710,	// Power-up Interrupt Enable    rw 	0x0000.0000
        IRQ_DETECT_ACK  = 0x718, // Power-up Interrupt Status/Clear R/W 0x0000.0000
        PeriphID4	= 0xfd0,	// Peripheral Identification 4	ro	0x0000.0000
        PeriphID5	= 0xfd4,	// Peripheral Identification 5 	ro	0x0000.0000
        PeriphID6	= 0xfd8,	// Peripheral Identification 6	ro	0x0000.0000
        PeriphID7	= 0xfdc,	// Peripheral Identification 7	ro	0x0000.0000
        PeriphID0	= 0xfe0,	// Peripheral Identification 0	ro	0x0000.0061
        PeriphID1	= 0xfe4,	// Peripheral Identification 1	ro	0x0000.0000
        PeriphID2	= 0xfe8,	// Peripheral Identification 2	ro	0x0000.0018
        PeriphID3	= 0xfec,	// Peripheral Identification 3	ro	0x0000.0001
        PCellID0	= 0xff0,	// PrimeCell Identification 0	ro	0x0000.000d
        PCellID1	= 0xff4,	// PrimeCell Identification 1	ro	0x0000.00f0
        PCellID2	= 0xff8,	// PrimeCell Identification 2	ro	0x0000.0005
        PCellID3	= 0xffc		// PrimeCell Identification 3	ro	0x0000.00b1
    };

    // Useful Bits in the Alternate Function Select Register
    enum AFSEL {                        // Description                  Type    Value after reset
        AFSEL_ALTP0     = 1 <<  0,      // Pin 0 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP1     = 1 <<  1,      // Pin 1 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP2     = 1 <<  2,      // Pin 2 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP3     = 1 <<  3,      // Pin 3 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP4     = 1 <<  4,      // Pin 4 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP5     = 1 <<  5,      // Pin 5 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP6     = 1 <<  6,      // Pin 6 (0 -> GPIO | 1 -> Alt) rw      0
        AFSEL_ALTP7     = 1 <<  7       // Pin 7 (0 -> GPIO | 1 -> Alt) rw      0
    };

    // Useful Bits in the Digital Enable Register
    enum DEN {                        // Description                    Type    Value after reset
        DEN_DIGP0     = 1 <<  0,      // Pin 0 (1 -> Digital Enable)    rw      1
        DEN_DIGP1     = 1 <<  1,      // Pin 1 (1 -> Digital Enable)    rw      1
        DEN_DIGP2     = 1 <<  2,      // Pin 2 (1 -> Digital Enable)    rw      1
        DEN_DIGP3     = 1 <<  3,      // Pin 3 (1 -> Digital Enable)    rw      1
        DEN_DIGP4     = 1 <<  4,      // Pin 4 (1 -> Digital Enable)    rw      1
        DEN_DIGP5     = 1 <<  5,      // Pin 5 (1 -> Digital Enable)    rw      1
        DEN_DIGP6     = 1 <<  6,      // Pin 6 (1 -> Digital Enable)    rw      1
        DEN_DIGP7     = 1 <<  7       // Pin 7 (1 -> Digital Enable)    rw      1
    };


// UART
    // Base address for memory-mapped UART Register
    enum {
        UART0_BASE      = 0x4000c000,
        UART1_BASE      = 0x4000d000
    };

protected:
    LM3S811() {}

    static void reboot() {
        Reg32 val = scs(AIRCR) & (~((-1u / VECTKEY) * VECTKEY));
        val |= SYSRESREQ;
        val |= 0x05fa * VECTKEY;
        scs(AIRCR) = val;
    }

    static void delay(const RTC::Microsecond & time) {
        assert(Traits<TSC>::enabled);
        unsigned long long ts = static_cast<unsigned long long>(time) * TSC::frequency() / 1000000;
        tsc(GPTMTAILR) = ts;
        tsc(GPTMTAPR) = ts >> 32;
        tsc(GPTMCTL) |= TAEN;
        while(!(tsc(GPTMRIS) & TATO_INT));
        tsc(GPTMCTL) &= ~TAEN;
        tsc(GPTMICR) |= TATO_INT;
    }

    static const unsigned char * id() { return System::info()->bm.uuid; } // TODO: System_Info is not populated in this machine

// Device enabling
    static void enable_uart(unsigned int unit) {
        assert(unit < UARTS);
        power_uart(unit, FULL);
        gpioa(AFSEL) |= 3 << (unit * 2);                // Pins A[1:0] are multiplexed between GPIO and UART 0. Select UART.
        gpioa(DEN) |= 3 << (unit * 2);                  // Enable digital I/O on Pins A[1:0]
    }
    static void enable_usb(unsigned int unit) {}

// PM
    static void power_uart(unsigned int unit, const Power_Mode & mode) {
        assert(unit < UARTS);
        switch(mode) {
        case FULL:
        case LIGHT:
        case SLEEP:
            scr(RCGC1) |= 1 << unit;                   // Activate UART "unit" clock
            scr(RCGC2) |= 1 << unit;                   // Activate port "unit" clock
            break;
        case OFF:
            scr(RCGC1) &= ~(1 << unit);                // Deactivate UART "unit" clock
            scr(RCGC2) &= ~(1 << unit);                // Deactivate port "unit" clock
            break;
        }
    }

    static void power_user_timer(unsigned int unit, const Power_Mode & mode) {
        assert(unit < TIMERS);
        switch(mode) {
        case FULL:
        case LIGHT:
        case SLEEP:
            scr(RCGC1) |= 1 << (unit + 16);             // Activate GPTM "unit" clock
            break;
        case OFF:
            scr(RCGC1) &= ~(1 << (unit + 16));          // Deactivate GPTM "unit" clock
            break;
        }
    }

    static void power_usb(unsigned int unit, const Power_Mode & mode) {}


// GPIO
    static void gpio_init() {}
    static void power_gpio(unsigned int unit, const Power_Mode & mode) {
        assert(unit < UARTS);
        switch(mode) {
        case FULL:
        case LIGHT:
        case SLEEP:
            scr(RCGC2) |= 1 << unit;                   // Activate port "unit" clock
            break;
        case OFF:
            scr(RCGC2) &= ~(1 << unit);                // Deactivate port "unit" clock
            break;
        }
    }
    void gpio_pull_up(unsigned int port, unsigned int pin) { gpio(port, PUR) &= 1 << pin; }
    void gpio_pull_down(unsigned int port, unsigned int pin) { gpio(port, PDR) &= 1 << pin; }
    void gpio_floating(unsigned int port, unsigned int pin) { gpio(port, ODR) &= 1 << pin; }


// PWM (not implemented for this model)
    static void pwm_config(unsigned int timer, char gpio_port, unsigned int gpio_pin) {}

// IEEE 802.15.4 (not present in this model)
    static void power_ieee802_15_4(const Power_Mode & mode) {}

public:
    static volatile Reg32 & scr(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(SCR_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & scs(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(SCS_BASE)[o / sizeof(Reg32)]; }

    static volatile Reg32 & gpio(unsigned int port, unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIOA_BASE + 0x1000*(port))[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpioa(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIOA_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpiob(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIOB_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpioc(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIOC_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpiod(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIOD_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & gpioe(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(GPIOE_BASE)[o / sizeof(Reg32)]; }
    static volatile Reg32 & tsc(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(TIMER1_BASE)[o / sizeof(Reg32)]; }

protected:
    static void pre_init();
    static void init();
};

typedef LM3S811 Machine_Model;

__END_SYS

#endif

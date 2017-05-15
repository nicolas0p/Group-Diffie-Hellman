// EPOS eMote3 Interrupt Controller Initialization

#include <cpu.h>
#include <ic.h>
#include <timer.h>
#include <usb.h>
#include <nic.h>

__BEGIN_SYS

// Class attributes
#ifdef __mmod_lm3s811__

IC::Interrupt_Handler IC::_eoi_vector[INTS] = {
    0, // Reset
    0, // NMI
    0, // Hard fault
    0, // Memory management fault
    0, // Bus fault
    0, // Usage fault
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // SVCall
    0, // Reserved
    0, // Reserved
    0, // PendSV
    0, // Systick
    0, // IRQ0
    0, // IRQ1
    0, // IRQ2
    0, // IRQ3
    0, // IRQ4
    0, // IRQ5
    0, // IRQ6
    0, // IRQ7
    0, // IRQ8
    0, // IRQ9
    0, // IRQ10
    0, // IRQ11
    0, // IRQ12
    0, // IRQ13
    0, // IRQ14
    0, // IRQ15
    0, // IRQ16
    0, // IRQ17
    0, // IRQ18
    User_Timer::eoi, // IRQ19
    0, // IRQ20
    User_Timer::eoi, // IRQ21
    0, // IRQ22
    User_Timer::eoi, // IRQ23
    0, // IRQ24
    0, // IRQ25
    0, // IRQ26
    0, // IRQ27
    0, // IRQ28
    0, // IRQ29
    0, // IRQ30
    0, // IRQ31
    0, // IRQ32
    0, // IRQ33
    0, // IRQ34
    0, // IRQ35
    0, // IRQ36
    0, // IRQ37
    0, // IRQ38
    0, // IRQ39
    0, // IRQ40
    0, // IRQ41
    0, // IRQ42
    0, // IRQ43
    0, // IRQ44
    0  // IRQ45
};

#elif defined(__mmod_zynq__)

IC::Interrupt_Handler IC::_eoi_vector[INTS] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Timer::eoi, // 29
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

#else

IC::Interrupt_Handler IC::_eoi_vector[INTS];

#endif

// Class methods
void IC::init()
{
    db<Init, IC>(TRC) << "IC::init()" << endl;

    CPU::int_disable(); // will be reenabled at Thread::init()
    Engine::init();

    disable(); // will be enabled on demand as handlers are registered

    // Set all interrupt handlers to int_not()
    for(Interrupt_Id i = 0; i < INTS; i++)
        _int_vector[i] = int_not;

#ifdef __mmod_emote3__
    // Initialize eoi vector (must be done at runtime because of .hex image format)
    for(unsigned int i = 0; i < INTS; i++)
        _eoi_vector[i] = 0;
    _eoi_vector[INT_USER_TIMER0] = User_Timer::eoi;
    _eoi_vector[INT_USER_TIMER1] = User_Timer::eoi;
    _eoi_vector[INT_USER_TIMER2] = User_Timer::eoi;
    _eoi_vector[INT_USER_TIMER3] = User_Timer::eoi;
    _eoi_vector[INT_NIC0_RX] = CC2538::eoi;
    _eoi_vector[INT_NIC0_TIMER] = CC2538RF::Timer::eoi;
    _eoi_vector[INT_USB0] = USB::eoi;
    _eoi_vector[INT_GPIOA] = GPIO::eoi;
    _eoi_vector[INT_GPIOB] = GPIO::eoi;
    _eoi_vector[INT_GPIOC] = GPIO::eoi;
    _eoi_vector[INT_GPIOD] = GPIO::eoi;
#endif
#if defined(__mmod_emote3__) || defined(__mmod_lm3s811__)

    _int_vector[NVIC::INT_HARD_FAULT] = hard_fault;

    // TSC is initialized before IC, so we register its interrupt now
    if(Traits<TSC>::enabled) {

        static const Interrupt_Id int_id =
              Machine_Model::TIMERS == 1 ? INT_USER_TIMER0
            : Machine_Model::TIMERS == 2 ? INT_USER_TIMER1
            : Machine_Model::TIMERS == 3 ? INT_USER_TIMER2
            : INT_USER_TIMER3;

        int_vector(int_id, TSC::int_handler);
        enable(int_id);
    }

#endif
}

__END_SYS

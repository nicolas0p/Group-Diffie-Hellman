// EPOS Cortex-M GPIO Mediator Implementation

#include <ic.h>
#include <gpio.h>

#ifdef __GPIO_H

//#include <machine/cortex/transducer.h>

__BEGIN_SYS

// Smart Data bindings
//Switch_Sensor::Observed Switch_Sensor::_observed;
//Switch_Sensor * Switch_Sensor::_dev[MAX_DEVICES];


// Class attributes
GPIO * GPIO::_devices[GPIO_PORTS][8];
unsigned char GPIO::_mis[GPIO_PORTS];
unsigned int GPIO::_irq_detect_ack[GPIO_PORTS];

// Class methods
void GPIO::handle_int(const IC::Interrupt_Id & id)
{
    /*
    unsigned int port = id - IC::INT_GPIOA;

    unsigned int mis = _mis[port];
    _mis[port] = 0;
    unsigned int irq_detect_ack = _irq_detect_ack[port];
    _irq_detect_ack[port] = 0;

    for(unsigned int i = 0; i < 8; ++i) {
        bool regular_interrupt = mis & (1 << i);
        bool power_up_interrupt = irq_detect_ack & ((1 << i) << (8 * port));
        if(regular_interrupt || power_up_interrupt) {
            GPIO * dev = _devices[port][i];
            if(dev && dev->_handler)
                dev->_handler(id);
        }
    }
    */
}

// FIXME: GPIO interrupt handling is done at eoi, because some devices generate interrupts faster than the processor can handle
void GPIO::eoi(const IC::Interrupt_Id & id)
{
    /*
    unsigned int port = id - IC::INT_GPIOA;
    _mis[port] |= gpio(port, MIS);
    _irq_detect_ack[port] |= gpio(port, IRQ_DETECT_ACK);
    */

    unsigned int port = id - IC::INT_GPIOA;
    unsigned int mis = gpio(port, MIS);
    unsigned int irq_detect_ack = gpio(port, IRQ_DETECT_ACK);

    for(unsigned int i = 0; i < 8; ++i) {
        bool regular_interrupt = mis & (1 << i);
        bool power_up_interrupt = irq_detect_ack & ((1 << i) << (8 * port));
        if(regular_interrupt || power_up_interrupt) {
            GPIO * dev = _devices[port][i];
            if(dev && dev->_handler)
                dev->_handler(id);
        }
    }

    // Clear regular interrupts even if no handler is available
    gpio(port, ICR) = -1;

    // Clear power-up interrupts even if no handler is available
    // There is something weird going on here.
    // The manual says: "There is a self-clearing function to this register that generates a
    // reset pulse to clear any interrupt which has its corresponding bit set to 1."
    // But this is not happening!
    // Also, clearing only the bit that is set or replacing the statement below with
    // regs[irq_number](IRQ_DETECT_ACK) = 0;
    // do not work!
    gpio(port, IRQ_DETECT_ACK) &= -1;
}

void GPIO::int_enable(const Edge & edge, bool power_up, const Edge & power_up_edge)
{
    IC::Interrupt_Id int_id = IC::INT_GPIOA + _port;
    IC::disable(int_id);
    int_disable();
    IC::int_vector(int_id, GPIO::handle_int);

    gpio(_port, IS) &= ~_pin_bit; // Set interrupt to edge-triggered

    switch(edge) {
    case RISING:
        gpio(_port, IBE) &= ~_pin_bit; // Interrupt on single edge, defined by IEV
        gpio(_port, IEV) |= _pin_bit;
        break;
    case FALLING:
        gpio(_port, IBE) &= ~_pin_bit; // Interrupt on single edge, defined by IEV
        gpio(_port, IEV) &= ~_pin_bit;
        break;
    case BOTH:
        gpio(_port, IBE) |= _pin_bit;
        break;
    }

    clear_interrupt();
    int_enable();

    if(supports_power_up && power_up) {
        assert(power_up_edge != BOTH);
        if (power_up_edge == FALLING)
            gpio(_port, P_EDGE_CTRL) |= (_pin_bit << (8 * _port));
        else if (power_up_edge == RISING)
            gpio(_port, P_EDGE_CTRL) &= ~(_pin_bit << (8 * _port));
        gpio(_port, PI_IEN) |= (_pin_bit << (8 * _port));
    }

    IC::enable(int_id);
}

__END_SYS

#endif

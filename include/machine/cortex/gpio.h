// EPOS ARM Cortex GPIO Mediator Declarations

#include <system/config.h>
#if !defined(__cortex_gpio_h_) && defined(__GPIO_H)
#define __cortex_gpio_h_

#include <machine.h>
#include <ic.h>
#include <gpio.h>

__BEGIN_SYS

class GPIO: public GPIO_Common, private Machine_Model
{
    friend class PWM;
private:
    static const bool supports_power_up = Machine_Model::supports_gpio_power_up;

public:
    GPIO(char port, unsigned int pin, const Direction & dir, const Pull & p = UP, const IC::Interrupt_Handler & handler = 0, const Edge & int_edge = RISING)
    : _port(port - 'A'), _pin(pin), _pin_bit(1 << pin), _data(&gpio(_port, _pin_bit << 2)), _handler(handler) {
        assert((port >= 'A') && (port <= 'A' + GPIO_PORTS));
        gpio(_port, AFSEL) &= ~_pin_bit; // Set pin as software controlled
        direction(dir);
        pull(p);
        clear_interrupt();
        if(_handler) {
            _devices[_port][_pin] = this;
            int_enable(int_edge);
        }
    }

    ~GPIO() {
        if(_handler)
            int_disable();
        _devices[_port][_pin] = 0;
    }

    bool get() const {
        assert(_direction == IN || _direction == INOUT);
        return *_data;
    }

    void handler(const IC::Interrupt_Handler & handler, const Edge & int_edge = RISING) {
        int_disable();
        _handler = handler;
        _devices[_port][_pin] = this;
        int_enable(int_edge);
    }

    void set(bool value = true) {
        assert(_direction == INOUT || _direction == OUT);
        if(_direction == INOUT) {
            gpio(_port, DIR) |= _pin_bit;
            *_data = 0xff * value;
            gpio(_port, DIR) &= ~_pin_bit;
        } else
            *_data = 0xff * value;
    }

    void clear() { set(false); }

    void direction(const Direction & dir) {
        _direction = dir;
        switch(dir) {
            case OUT:
                gpio(_port, DIR) |= _pin_bit;
                break;
            case IN:
            case INOUT:
                gpio(_port, DIR) &= ~_pin_bit;
                break;
        }
    }

    void pull(const Pull & p) {
        switch(p) {
            case UP:
                gpio_pull_up(_port, _pin);
                break;
            case DOWN:
                gpio_pull_down(_port, _pin);
                break;
            case FLOATING:
                gpio_floating(_port, _pin);
                break;
        }
    }

    void int_enable() { gpio(_port, IM) |= _pin_bit; }
    void int_enable(const Level & level, bool power_up = false, const Level & power_up_level = HIGH);
    void int_enable(const Edge & edge, bool power_up = false, const Edge & power_up_edge = RISING);
    void int_disable() { gpio(_port, IM) &= ~_pin_bit; }

    static void eoi(const IC::Interrupt_Id & i);

private:
    void clear_interrupt() {
        gpio(_port, ICR) = _pin_bit;
        gpio(_port, IRQ_DETECT_ACK) &= ~(_pin_bit << (8 * _port));
    }

    static void handle_int(const IC::Interrupt_Id & i);

private:
    unsigned char _port;
    unsigned char _pin;
    unsigned int _pin_bit;
    Direction _direction;
    volatile Reg32 * _data;
    IC::Interrupt_Handler _handler;

    static GPIO * _devices[GPIO_PORTS][8];
    static unsigned char _mis[GPIO_PORTS];
    static unsigned int _irq_detect_ack[GPIO_PORTS];
};

__END_SYS

#endif

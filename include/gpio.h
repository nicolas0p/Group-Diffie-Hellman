// EPOS GPIO Mediator Common package

#ifndef __gpio_h
#define __gpio_h

#include <system/config.h>

__BEGIN_SYS

class GPIO_Common
{
protected:
    GPIO_Common() {}

public:
    enum Level {
        HIGH,
        LOW
    };

    enum Edge {
        RISING,
        FALLING,
        BOTH
    };

    enum Direction {
        IN,
        OUT,
        INOUT
    };

    enum Pull {
        UP,
        DOWN,
        FLOATING
    };
};

__END_SYS

#ifdef __GPIO_H
#include __GPIO_H
#endif

#endif

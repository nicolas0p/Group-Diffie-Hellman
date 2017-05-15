// EPOS YYYY UART Mediator Implementation

#include <mach/yyyy/uart.h>

__BEGIN_SYS

const PC_UART::IO_Port PC_UART::_ports[] = { Traits<PC_UART>::COM1, 
        				     Traits<PC_UART>::COM2, 
        				     Traits<PC_UART>::COM3,
        				     Traits<PC_UART>::COM4 };

__END_SYS

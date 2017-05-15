// EPOS Zynq (Cortex-A9) Initialization

#include <system/config.h>
#include __MODEL_H
#ifdef __zynq_h

__BEGIN_SYS

void Zynq::init()
{
    unlock_slcr();
    fpga_reset();
}

__END_SYS
#endif

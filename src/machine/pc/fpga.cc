// EPOS PC FPGA Mediator Implementation

#include <machine/pc/machine.h>
#include <machine/pc/ic.h>
#include <machine/pc/fpga.h>

__BEGIN_SYS

// Class attributes
CPU::Log_Addr FPGA::Engine::_base;
MMU::DMA_Buffer * FPGA::Engine::_dma_buf;

// Class methods
void FPGA::int_handler(const IC::Interrupt_Id & i)
{

}

__END_SYS

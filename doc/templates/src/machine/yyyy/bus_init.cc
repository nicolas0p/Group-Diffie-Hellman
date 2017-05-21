// EPOS YYYY PCI Mediator

#include <pci.h>
#include <system.h>

__BEGIN_SYS

void PC_PCI::init()
{
    _phy_io_mem = System::info()->pmm.io_base;

    db<Init, PCI>(TRC) << "PCI::init(pmm.io_mem=" << _phy_io_mem << ")" << endl;

    CPU::int_disable();

    CPU::out8(0xcfb, 0x01);
    Reg32 tmp = CPU::in32(CONFADDR);
    CPU::out32(CONFADDR, 0x80000000);
    if(CPU::in32(CONFADDR) != 0x80000000) {
        db<Init, PC_PCI>(WRN) << "PC_PCI::init: failed!" << endl;
    }
    CPU::out32(CONFADDR, tmp);

    CPU::int_enable();
}

__END_SYS

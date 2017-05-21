// EPOS PC Scratchpad Memory Initialization

#include <system/config.h>
#include <system.h>
#include <address_space.h>
#include <segment.h>
#include <scratchpad.h>

__BEGIN_SYS

void Scratchpad::init()
{
    db<Init, Scratchpad>(TRC) << "Scratchpad::init(a=" << ADDRESS << ",s=" << SIZE << ")" << endl;

    _segment = new (SYSTEM) Segment(CPU::Phy_Addr(ADDRESS), SIZE, MMU::IA32_Flags::PCD);
    _heap = new (SYSTEM) Heap(Address_Space(MMU::current()).attach(_segment), _segment->size());
}

__END_SYS

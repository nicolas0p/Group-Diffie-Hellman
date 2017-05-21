// EPOS XXXX MMU Mediator Implementation

#include <arch/xxxx/mmu.h>

__BEGIN_SYS

// Class attributes
XXXX_MMU::List XXXX_MMU::_free;
XXXX_MMU::Page_Directory * XXXX_MMU::_master;

__END_SYS

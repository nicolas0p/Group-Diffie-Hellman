// EPOS XXXX MMU Mediator Initialization

#include <mmu.h>
#include <system.h>

__BEGIN_SYS

void XXXX_MMU::init()
{
    System_Info<YYYY> * si = System::info();

    db<Init, XXXX_MMU>(INF) << "XXXX_MMU::memory={base=" 
        		    << (void *) si->pmm.mem_base << ",size="
        		    << (si->bm.mem_top - si->bm.mem_base) / 1024
        		    << "KB}" << endl;
    db<Init, XXXX_MMU>(INF) << "XXXX_MMU::free1={base=" 
        		    << (void *) si->pmm.free1_base << ",size="
        		    << (si->pmm.free1_top - si->pmm.free1_base) / 1024
        		    << "KB}" << endl;
    db<Init, XXXX_MMU>(INF) << "XXXX_MMU::free2={base=" 
        		    << (void *) si->pmm.free2_base << ",size="
        		    << (si->pmm.free2_top - si->pmm.free2_base) / 1024
        		    << "KB}" << endl;
    db<Init, XXXX_MMU>(INF) << "XXXX_MMU::free3={base="
                            << (void *) si->pmm.free3_base << ",size="
                            << (si->pmm.free3_top - si->pmm.free3_base) / 1024
                            << "KB}" << endl;
    
    // BIG WARING HERE: INIT (i.e. this program) will be part of the free
    // storage after the following is executed, but it will remain alive
    // This only works because the _free.insert_merging() only
    // touchs the first page of each chunk and INIT is not there

    // Insert all free memory into the _free list
    free(si->pmm.free1_base, pages(si->pmm.free1_top - si->pmm.free1_base));
    free(si->pmm.free2_base, pages(si->pmm.free2_top - si->pmm.free2_base));
    free(si->pmm.free3_base, pages(si->pmm.free3_top - si->pmm.free3_base));

    // Remeber the master page directory (created during SETUP)
    _master = reinterpret_cast<Page_Directory *>(CPU::pdp());

    db<Init, XXXX_MMU>(INF) << "XXXX_MMU::master page directory=" << _master << endl;
}

__END_SYS


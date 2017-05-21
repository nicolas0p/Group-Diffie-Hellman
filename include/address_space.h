// EPOS Address_Space Component Declarations

#ifndef __address_space_h
#define __address_space_h

#include <mmu.h>
#include <segment.h>

__BEGIN_SYS

class Address_Space: private MMU::Directory
{
    friend class Task;

private:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

    using MMU::Directory::activate;

public:
    Address_Space();
    Address_Space(MMU::Page_Directory * pd);
    ~Address_Space();

    using MMU::Directory::pd;

    Log_Addr attach(Segment * seg);
    Log_Addr attach(Segment * seg, const Log_Addr & addr);
    void detach(Segment * seg);
    void detach(Segment * seg, const Log_Addr & addr);

    Phy_Addr physical(const Log_Addr & address);
};

__END_SYS

#endif

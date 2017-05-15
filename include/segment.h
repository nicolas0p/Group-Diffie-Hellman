// EPOS Memory Segment Component Declarations

#ifndef __segment_h
#define __segment_h

#include <mmu.h>

__BEGIN_SYS

class Segment: public MMU::Chunk
{
private:
    typedef MMU::Chunk Chunk;

public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef MMU::Flags Flags;

public:
    Segment(unsigned int bytes, const Color & color = Color::WHITE, const Flags & flags = Flags::APP);
    Segment(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags);
    ~Segment();

    unsigned int size() const;
    Phy_Addr phy_address() const;
    int resize(int amount);
};

__END_SYS

#endif

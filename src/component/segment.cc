// EPOS Memory Segment Component Implementation

#include <segment.h>

__BEGIN_SYS

// Methods
Segment::Segment(unsigned int bytes, const Color & color, const Flags & flags): Chunk(bytes, flags, color)
{
    db<Segment>(TRC) << "Segment(bytes=" << bytes << ",color=" << color << ",flags=" << flags << ") [Chunk::_pt=" << Chunk::pt() << "] => " << this << endl;
}


Segment::Segment(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags): Chunk(phy_addr, bytes, flags | Flags::IO)
// The MMU::IO flag signalizes the MMU that the attached memory shall
// not be released when the chunk is deleted
{
    db<Segment>(TRC) << "Segment(bytes=" << bytes << ",phy_addr=" << phy_addr << ",flags=" << flags << ") [Chunk::_pt=" << Chunk::pt() << "] => " << this << endl;
}


Segment::~Segment()
{
    db<Segment>(TRC) << "~Segment() [Chunk::_pt=" << Chunk::pt() << "]" << endl;
}


unsigned int Segment::size() const
{
    return Chunk::size();
}


Segment::Phy_Addr Segment::phy_address() const
{
    return Chunk::phy_address();
}


int Segment::resize(int amount)
{
    db<Segment>(TRC) << "Segment::resize(amount=" << amount << ")" << endl;

    return Chunk::resize(amount);
}

__END_SYS

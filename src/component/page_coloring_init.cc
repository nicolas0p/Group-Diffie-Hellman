// EPOS Page Coloring Initialization

#include <utility/malloc.h>
#include <utility/heap.h>
#include <segment.h>
#include <address_space.h>

__BEGIN_SYS

void Page_Coloring::init()
{
    db<Init, Heaps>(TRC) << "Page_Coloring::init(colors=" << COLORS << ",colsize=" << HEAP_SIZE << ")" << endl;

    // Color 0, WHITE, is reserved for the system
    for(unsigned int i = 1; i < COLORS; i++) {
        _segment[i] = new (SYSTEM) Segment(HEAP_SIZE, Color(i), Segment::Flags::APP);
        _heap[i] = new (SYSTEM) Heap(Address_Space(MMU::current()).attach(_segment[i]), _segment[i]->size());
   }
}

__END_SYS

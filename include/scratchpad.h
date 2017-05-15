// EPOS Scratchpad Memory Mediator Declarations

#ifndef __scratchpad_h
#define __scratchpad_h

#include <utility/heap.h>
#include <segment.h>

__BEGIN_SYS

class Scratchpad_Common
{
    friend void * ::operator new(size_t, const EPOS::Scratchpad_Allocator &);
    friend void * ::operator new[](size_t, const EPOS::Scratchpad_Allocator &);

public:
    static void * alloc(unsigned int bytes) { return _heap->alloc(bytes); }

protected:
    static Segment * _segment;
    static Heap * _heap;
};

__END_SYS

#ifdef __SCRATCHPAD_H
#include __SCRATCHPAD_H

inline void * operator new(size_t bytes, const EPOS::Scratchpad_Allocator & allocator) {
    return _SYS::Scratchpad::_heap->alloc(bytes);
}

inline void * operator new[](size_t bytes, const EPOS::Scratchpad_Allocator & allocator) {
    return _SYS::Scratchpad::_heap->alloc(bytes);
}

#endif

#endif

// EPOS Global System Component Declarations

#ifndef __system_h
#define __system_h

#include <system/info.h>
#include <utility/heap.h>
#include <segment.h>

__BEGIN_SYS

class System
{
    friend class Init_System;
    friend class Init_Application;
    friend void CPU::Context::load() const volatile;
    friend void * ::malloc(size_t);
    friend void ::free(void *);
    friend void * ::operator new(size_t, const EPOS::System_Allocator &);
    friend void * ::operator new[](size_t, const EPOS::System_Allocator &);
    friend void ::operator delete(void *);
    friend void ::operator delete[](void *);

public:
    static System_Info * const info() { assert(_si); return _si; }

private:
    static void init();

private:
    static System_Info * _si;
    static char _preheap[(Traits<System>::multiheap ? sizeof(Segment) : 0) + sizeof(Heap)];
    static Segment * _heap_segment;
    static Heap * _heap;
};

__END_SYS

inline void * operator new(size_t bytes, const EPOS::System_Allocator & allocator) {
    return _SYS::System::_heap->alloc(bytes);
}

inline void * operator new[](size_t bytes, const EPOS::System_Allocator & allocator) {
    return _SYS::System::_heap->alloc(bytes);
}

#endif

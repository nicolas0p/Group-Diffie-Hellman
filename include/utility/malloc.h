// EPOS Application-level Dynamic Memory Utility Declarations

#ifndef __malloc_h
#define __malloc_h

#include <utility/string.h>
#include <system.h>
#include <application.h>

extern "C"
{
    // Standard C Library allocators
    inline void * malloc(size_t bytes) {
        __USING_SYS;
        if(Traits<System>::multiheap)
            return Application::_heap->alloc(bytes);
        else
            return System::_heap->alloc(bytes);
    }

    inline void * calloc(size_t n, unsigned int bytes) {
        void * ptr = malloc(n * bytes);
        memset(ptr, 0, n * bytes);
        return ptr;
    }

    inline void free(void * ptr) {
        __USING_SYS;
        if(Traits<System>::multiheap)
            Heap::typed_free(ptr);
        else
            Heap::untyped_free(System::_heap, ptr);
    }
}

// C++ dynamic memory allocators and deallocators
inline void * operator new(size_t bytes) {
    return malloc(bytes);
}

inline void * operator new[](size_t bytes) {
    return malloc(bytes);
}

// Delete cannot be declared inline due to virtual destructors
void operator delete(void * ptr);
void operator delete[](void * ptr);


__BEGIN_SYS

class Page_Coloring
{
    friend class System;

    friend void * ::operator new(size_t, const EPOS::Color &);
    friend void * ::operator new[](size_t, const EPOS::Color &);

private:
    static const unsigned int HEAP_SIZE = Traits<Application>::HEAP_SIZE;
    static const unsigned int COLORS = Traits<MMU>::COLORS;

public:
    static void * alloc(unsigned int bytes, const EPOS::Color & allocator) {
        assert(static_cast<unsigned int>(allocator) <= COLORS);
        return _heap[allocator]->alloc(bytes);
    }

private:
    static void init();

protected:
    static Segment * _segment[COLORS];
    static Heap * _heap[COLORS];
};

__END_SYS

inline void * operator new(size_t bytes, const EPOS::Color & allocator) {
    return _SYS::Page_Coloring::alloc(bytes, allocator);
}

inline void * operator new[](size_t bytes, const EPOS::Color & allocator) {
    return _SYS::Page_Coloring::alloc(bytes, allocator);
}

#endif


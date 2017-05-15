// EPOS Global Application Component Declarations

#ifndef __application_h
#define __application_h

#include <utility/heap.h>

extern "C"
{
    void * malloc(size_t);
    void free(void *);
}

__BEGIN_SYS

class Application
{
    friend class Init_Application;
    friend void * ::malloc(size_t);
    friend void ::free(void *);

private:
    static void init();

private:
    static char _preheap[sizeof(Heap)];
    static Heap * _heap;
};

__END_SYS

#include <utility/malloc.h>

#endif

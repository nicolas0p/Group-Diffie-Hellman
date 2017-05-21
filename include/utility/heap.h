// EPOS Heap Utility Declarations

#ifndef __heap_h
#define __heap_h

#include <utility/debug.h>
#include <utility/list.h>
#include <utility/spin.h>

__BEGIN_UTIL

// Heap
class Simple_Heap: private Grouping_List<char>
{
protected:
    static const bool typed = Traits<System>::multiheap;

public:
    using Grouping_List<char>::empty;
    using Grouping_List<char>::size;

    Simple_Heap() {
        db<Init, Heaps>(TRC) << "Heap() => " << this << endl;
    }

    Simple_Heap(void * addr, unsigned int bytes) {
        db<Init, Heaps>(TRC) << "Heap(addr=" << addr << ",bytes=" << bytes << ") => " << this << endl;

        free(addr, bytes);
    }

    void * alloc(unsigned int bytes) {
        db<Heaps>(TRC) << "Heap::alloc(this=" << this << ",bytes=" << bytes;

        if(!bytes)
            return 0;

        if(!Traits<CPU>::unaligned_memory_access)
            while((bytes % sizeof(void *)))
                ++bytes;

        if(typed)
            bytes += sizeof(void *);  // add room for heap pointer
        bytes += sizeof(int);         // add room for size
        if(bytes < sizeof(Element))
            bytes = sizeof(Element);

        Element * e = search_decrementing(bytes);
        if(!e) {
            out_of_memory();
            return 0;
        }

        int * addr = reinterpret_cast<int *>(e->object() + e->size());

        if(typed)
            *addr++ = reinterpret_cast<int>(this);
        *addr++ = bytes;

        db<Heaps>(TRC) << ") => " << reinterpret_cast<void *>(addr) << endl;

        return addr;
    }

    void free(void * ptr, unsigned int bytes) {
        db<Heaps>(TRC) << "Heap::free(this=" << this << ",ptr=" << ptr << ",bytes=" << bytes << ")" << endl;

        if(ptr && (bytes >= sizeof(Element))) {
            Element * e = new (ptr) Element(reinterpret_cast<char *>(ptr), bytes);
            Element * m1, * m2;
            insert_merging(e, &m1, &m2);
        }
    }

    static void typed_free(void * ptr) {
        int * addr = reinterpret_cast<int *>(ptr);
        unsigned int bytes = *--addr;
        Simple_Heap * heap = reinterpret_cast<Simple_Heap *>(*--addr);
        heap->free(addr, bytes);
    }

    static void untyped_free(Simple_Heap * heap, void * ptr) {
        int * addr = reinterpret_cast<int *>(ptr);
        unsigned int bytes = *--addr;
        heap->free(addr, bytes);
    }

private:
    void out_of_memory();
};


// Wrapper for non-atomic heap
template<typename T, bool atomic>
class Heap_Wrapper: public T
{
public:
    Heap_Wrapper() {}
    Heap_Wrapper(void * addr, unsigned int bytes): T(addr, bytes) {}
};


// Wrapper for atomic heap
extern "C" {
    void _heap_lock();
    void _heap_unlock();
}

template<typename T>
class Heap_Wrapper<T, true>: public T
{
public:
    Heap_Wrapper() {}
    Heap_Wrapper(void * addr, unsigned int bytes): T(addr, bytes) {}

    bool empty() {
        enter();
        bool tmp = T::empty();
        leave();
        return tmp;
    }

    unsigned int size() {
        enter();
        unsigned int tmp = T::size();
        leave();
        return tmp;
    }

    void * alloc(unsigned int bytes) {
        enter();
        void * tmp = T::alloc(bytes);
        leave();
        return tmp;
    }

    void free(void * ptr) {
        enter();
        T::free(ptr);
        leave();
    }

    void free(void * ptr, unsigned int bytes) {
        enter();
        T::free(ptr, bytes);
        leave();
    }

private:
    void enter() { _heap_lock(); }
    void leave() { _heap_unlock(); }
};


// Heap
class Heap: public Heap_Wrapper<Simple_Heap, Traits<System>::multicore>
{
private:
    typedef Heap_Wrapper<Simple_Heap, Traits<System>::multicore> Base;

public:
    Heap() {}
    Heap(void * addr, unsigned int bytes): Base(addr, bytes) {}
};

__END_UTIL

#endif

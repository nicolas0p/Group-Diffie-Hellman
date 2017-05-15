// EPOS Buffer Declarations
// This Buffer was designed to move data across a zero-copy communication stack, but can be used for several other purposes

#ifndef __buffer_h
#define __buffer_h

#include <cpu.h>

__BEGIN_UTIL

template<typename Owner, typename Data, typename Shadow = void, typename Metadata = Dummy>
class Buffer: private Data, public Metadata
{
public:
    typedef Simple_List<Buffer<Owner, Data, Shadow, Metadata> > List;
    typedef typename List::Element Element;

public:
    // This constructor is meant to be used at initialization time to correlate shadow data structures (e.g. NIC ring buffers)
    Buffer(Shadow * s): _lock(false), _owner(0), _shadow(s), _size(sizeof(Data)), _link1(this), _link2(this) {}

    // These constructors are used whenever a Buffer receives new data
    Buffer(Owner * o, unsigned int s): _lock(false), _owner(o), _size(s), _link1(this), _link2(this) {}
    template<typename ... Tn>
    Buffer(Owner * o, unsigned int s, Tn ... an): Data(an ...), _lock(false), _owner(o), _size(s), _link1(this), _link2(this) {}

    Data * data() { return this; }
    Data * frame() { return data(); }
    Data * message() { return data(); }

    bool lock() { return !CPU::tsl(_lock); }
    void unlock() { _lock = 0; }

    Owner * owner() const { return _owner; }
    Owner * nic() const { return owner(); }
    void owner(Owner * o) { _owner = o; }
    void nic(Owner * o) { owner(o); }

    Shadow * shadow() const { return _shadow; }
    Shadow * back() const { return shadow(); }

    unsigned int size() const { return _size; }
    void size(unsigned int s) { _size = s; }

    Element * link1() { return &_link1; }
    Element * link() { return link1(); }
    Element * lint() { return link1(); }
    Element * link2() { return &_link2; }
    Element * lext() { return link2(); }

    friend Debug & operator<<(Debug & db, const Buffer & b) {
        db << "{md=" << b._owner << ",lk=" << b._lock << ",sz=" << b._size << ",sd=" << b._shadow << "}";
        return db;
    }

private:
    volatile bool _lock;
    Owner * _owner;
    Shadow * _shadow;
    unsigned int _size;
    Element _link1;
    Element _link2;
};

__END_UTIL

#endif

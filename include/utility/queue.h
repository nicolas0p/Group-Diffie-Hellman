// EPOS Queue Utility Declarations

// Queue is a traditional queue, with insertions at the tail
// and removals either from the head or from specific objects

// Ordered_Queue is an ordered queue, i.e. objects are inserted
// in-order based on the integral value of "element.rank". Note that "rank"
// implies an order, but does not necessarily need to be "the absolute order"
// in the queue; it could, for instance, be a priority information or a
// time-out specification. Insertions must first tag "element" with "rank".
// Removals are just like in the traditional queue. Elements of both Queues
// may be exchanged.
// Example: insert(B,7);insert(C,9);insert(A,4)
// +---+		+---+	+---+	+---+
// |obj|		| A |-->| B |-->| C |
// + - +       head -->	+ - +	+ - +	+ - + <-- tail
// |ord|		| 4 |<--| 7 |<--| 9 |
// +---+ 		+---+	+---+	+---+

// Relative Queue is an ordered queue, i.e. objects are inserted
// in-order based on the integral value of "element.rank" just like above.
// But differently from that, a Relative Queue handles "rank" as relative
// offsets. This is very useful for alarm queues. Elements of Relative Queue
// cannot be exchanged with elements of the other queues described earlier.
// Example: insert(B,7);insert(C,9);insert(A,4)
// +---+		+---+	+---+	+---+
// |obj|		| A |-->| B |-->| C |
// + - +       head -->	+ - +	+ - +	+ - + <-- tail
// |ord|		| 4 |<--| 3 |<--| 2 |
// +---+ 		+---+	+---+	+---+

// Scheduling Queue is an ordered queue whose ordering criterion is externally
// definable and for which selecting methods are defined (e.g. choose). This
// utility is most useful for schedulers, such as CPU or I/O.

#ifndef __queue_h
#define __queue_h

#include <cpu.h>
#include "list.h"
#include "spin.h"

__BEGIN_UTIL

// Wrapper for non-atomic queues
template<typename T, bool atomic>
class Queue_Wrapper: private T
{
public:
    typedef typename T::Object_Type Object_Type;
    typedef typename T::Element Element;

public:
    bool empty() { return T::empty(); }
    unsigned int size() { return T::size(); }

    Element * head() { return T::head(); }
    Element * tail() { return T::tail(); }

    void insert(Element * e) { T::insert(e); }

    Element * remove() { return T::remove(); }
    Element * remove(Element * e) { return T::remove(e); }
    Element * remove(const Object_Type * obj) { return T::remove(obj); }

    Element * search(const Object_Type * obj) {	return T::search(obj); }

    Element * volatile & chosen() { return T::chosen(); }

    Element * choose() { return T::choose(); }
    Element * choose_another() { return T::choose_another(); }
    Element * choose(Element * e) { return T::choose(e); }
    Element * choose(const Object_Type * obj) {	return T::choose(obj); }
};

// Wrapper for atomic queues
template<typename T>
class Queue_Wrapper<T, true>: private T
{
public:
    typedef typename T::Object_Type Object_Type;
    typedef typename T::Element Element;

public:
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

    Element * head() {
        enter();
        Element * tmp = T::head();
        leave();
        return tmp;
    }

    Element * tail() {
        enter();
        Element * tmp = T::tail();
        leave();
        return tmp;
    }

    void insert(Element * e) {
        enter();
        T::insert(e);
        leave();
    }

    Element * remove() {
        enter();
        Element * tmp = T::remove();
        leave();
        return tmp;
    }

    Element * remove(const Object_Type * obj) {
        enter();
        Element * tmp = T::remove(obj);
        leave();
        return tmp;
    }

    Element * search(const Object_Type * obj) {
        enter();
        Element * tmp = T::search(obj);
        leave();
        return tmp;
    }

    Element * volatile & chosen() {
        enter();
        Element * volatile & tmp = T::chosen();
        leave();
        return tmp;
    }

    Element * choose() {
        enter();
        Element * tmp = T::choose();
        leave();
        return tmp;
    }

    Element * choose_another() {
        enter();
        Element * tmp = T::choose_another();
        leave();
        return tmp;
    }
    Element * choose(Element * e) {
        enter();
        Element * tmp = T::choose(e);
        leave();
        return tmp;
    }

    Element * choose(const Object_Type * obj) {
        enter();
        Element * tmp = T::choose(obj);
        leave();
        return tmp;
    }

private:
    void enter() {
        CPU::int_disable();
        _lock.acquire();
    }

    void leave() {
        _lock.release();
        CPU::int_enable();
    }

private:
    Spin _lock;
};


// Queue
template<typename T,
          typename El = List_Elements::Doubly_Linked<T> >
class Queue: public Queue_Wrapper<List<T, El>, false> {};


// Ordered Queue
template<typename T,
          typename R = List_Element_Rank,
          typename El = List_Elements::Doubly_Linked_Ordered<T, R> >
class Ordered_Queue: public Queue_Wrapper<Ordered_List<T, R, El>, false> {};


// Relatively-Ordered Queue
template<typename T,
          typename R = List_Element_Rank,
          typename El = List_Elements::Doubly_Linked_Ordered<T, R> >
class Relative_Queue: public Queue_Wrapper<Relative_List<T, R, El>, false> {};

__END_UTIL

#endif

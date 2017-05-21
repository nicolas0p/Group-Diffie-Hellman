// EPOS Component Identification Aspect Program

#ifndef __id_h
#define __id_h

#include <utility/debug.h>

__BEGIN_SYS

class Pointer_Id
{
public:
    enum Null { NULL = 0 };

    enum {
        ANY_UNIT = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
    };

    typedef void Host_Id;
    typedef unsigned int Type_Id;
    typedef unsigned int Unit_Id;
    typedef Type_Id Local;

public:
    Pointer_Id() {}
    Pointer_Id(const Null &): _type(NULL), _unit(NULL) {}
    Pointer_Id(const Type_Id & t, const Unit_Id & u): _type(t), _unit(u) {}
    template<typename Component>
    Pointer_Id(const Component * c): _type(Type<Component>::ID), _unit(reinterpret_cast<Unit_Id>(c)) {}

    const Pointer_Id & id() const { return *this; }
    void id(const Type_Id & t, const Unit_Id & u) { _type = t; _unit = u; }

    const Type_Id & type() const { return _type; }
    const Unit_Id & unit() const { return _unit; }

    friend Debug & operator << (Debug & db, const Pointer_Id & id) {
        db << "{t=" << id.type() << ",u=" << reinterpret_cast<void *>(id.unit()) << "}" ; return db;
    }

private:
    Type_Id _type;
    Unit_Id _unit;
};

typedef Pointer_Id Id;

__END_SYS

#endif

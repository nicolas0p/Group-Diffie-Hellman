// EPOS Component Framework - Proxy x Agent Message

#ifndef __message_h
#define __message_h

#include <utility/observer.h>
#include <utility/buffer.h>
#include "id.h"

extern "C" { void _syscall(void *); }

__BEGIN_SYS

class Message
{
private:
    static const unsigned int MAX_PARAMETERS_SIZE = 20;

public:
    enum {
        CREATE,
        CREATE1,
        CREATE2,
        CREATE3,
        CREATE4,
        CREATE5,
        CREATE6,
        CREATE7,
        CREATE8,
        CREATE9,
        DESTROY,
        SELF,

        COMPONENT = 0x10,

        THREAD_STATE = COMPONENT,
        THREAD_PRIORITY,
        THREAD_PRIORITY1,
        THREAD_JOIN,
        THREAD_PASS,
        THREAD_SUSPEND,
        THREAD_RESUME,
        THREAD_YIELD,
        THREAD_EXIT,
        THREAD_WAIT_NEXT,

        TASK_ADDRESS_SPACE = COMPONENT,
        TASK_CODE_SEGMENT,
        TASK_DATA_SEGMENT,
        TASK_CODE,
        TASK_DATA,
        TASK_MAIN,

        ADDRESS_SPACE_PD = COMPONENT,
        ADDRESS_SPACE_ATTACH1,
        ADDRESS_SPACE_ATTACH2,
        ADDRESS_SPACE_DETACH1,
        ADDRESS_SPACE_DETACH2,
        ADDRESS_SPACE_PHYSICAL,

        SEGMENT_SIZE = COMPONENT,
        SEGMENT_PHY_ADDRESS,
        SEGMENT_RESIZE,
        CREATE_SEGMENT_IN_PLACE,
        CREATE_HEAP_IN_PLACE,

        SYNCHRONIZER_LOCK = COMPONENT,
        SYNCHRONIZER_UNLOCK,
        SYNCHRONIZER_P,
        SYNCHRONIZER_V,
        SYNCHRONIZER_WAIT,
        SYNCHRONIZER_SIGNAL,
        SYNCHRONIZER_BROADCAST,

        ALARM_DELAY = COMPONENT,
        ALARM_GET_PERIOD,
        ALARM_SET_PERIOD,
        ALARM_FREQUENCY,

        COMMUNICATOR_SEND = COMPONENT,
        COMMUNICATOR_REPLY,
        COMMUNICATOR_RECEIVE,

        PRINT = COMPONENT,

        UNDEFINED = -1
    };
    typedef int Method;
    typedef Method Result;

    typedef Simple_List<Message> List;
    typedef List::Element Element;

public:
    Message(): _link(this) {}
    Message(const Message & msg): _link(this) { *this = msg; _link = this; }
    Message(const Id & id): _id(id), _link(this) {}
    template<typename ... Tn>
    Message(const Id & id, const Method & m, Tn && ... an): _id(id), _method(m), _link(this) { out(an ...); }

    const Id & id() const { return _id; }
    void id(const Id & id) { _id = id; }

    const Method & method() const { return _method; }
    void method(const Method & m) { _method = m; }
    const Result & result() const { return _method; }
    void result(const Result & r) { _method = r; }

    template<typename ... Tn>
    void in(Tn && ... an) {
        // Force a compilation error in case out is called with too many arguments
        typename IF<(SIZEOF<Tn ...>::Result <= MAX_PARAMETERS_SIZE), int, void>::Result index = 0;
        DESERIALIZE(_parms, index, an ...);
    }
    template<typename ... Tn>
    void out(const Tn & ... an) {
        // Force a compilation error in case out is called with too many arguments
        typename IF<(SIZEOF<Tn ...>::Result <= MAX_PARAMETERS_SIZE), int, void>::Result index = 0;
        SERIALIZE(_parms, index, an ...);
    }

    void reply_to(const Id & id) { _reply_to = id; }
    const Id & reply_to() { return _reply_to; }

    void act() { _syscall(this); }

    Element * lext() { return &_link; }

    friend Debug & operator << (Debug & db, const Message & m) {
          db << "{id=" << m._id << ",m=" << hex << m._method << ",rt=" << m._reply_to
             << ",p={" << reinterpret_cast<void *>(*static_cast<const int *>(reinterpret_cast<const void *>(&m._parms[0]))) << ","
             << reinterpret_cast<void *>(*static_cast<const int *>(reinterpret_cast<const void *>(&m._parms[4]))) << ","
             << reinterpret_cast<void *>(*static_cast<const int *>(reinterpret_cast<const void *>(&m._parms[8]))) << "}}";
          return db;
      }

public:
    Id _id;
    Method _method;
    char _parms[MAX_PARAMETERS_SIZE];

    Id _reply_to;
    Element _link;
};

__END_SYS

#endif

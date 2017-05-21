// EPOS Component Framework - Component Agent

#ifndef __agent_h
#define __agent_h

#include <thread.h>
#include <task.h>
#include <active.h>
#include <address_space.h>
#include <segment.h>
#include <mutex.h>
#include <semaphore.h>
#include <condition.h>
#include <clock.h>
#include <alarm.h>
#include <chronometer.h>
#include <communicator.h>

#include "message.h"
#include "ipc.h"

__BEGIN_SYS

class Agent: public Message
{
private:
    typedef void (Agent:: * Member)();

public:
    void exec() {
        if(id().type() != UTILITY_ID)
            db<Framework>(TRC) << ":=>" << *reinterpret_cast<Message *>(this) << endl;

        if(id().type() < LAST_TYPE_ID) // in-kernel services
            (this->*_handlers[id().type()])();
        else { // out-of-kernel (i.e. Dom0 or server) services
                Message msg(*this); // copy message from user space to kernel
                msg.id(Id(IPC_COMMUNICATOR_ID, id().unit()));
                if(IPC::send(&msg)) { // 0 => no one listening
                    Port<IPC> * comm = reinterpret_cast<Port<IPC> *>(IPC::observer(id().type())); // recall the Port<IPC> that got us here
                    comm->receive(this); // copy from kernel to user
                } else
                    result(UNDEFINED);
        }

        if(id().type() != UTILITY_ID)
            db<Framework>(TRC) << "<=:" << *reinterpret_cast<Message *>(this) << endl;
    }

private:
    void handle_thread();
    void handle_task();
    void handle_active();
    void handle_address_space();
    void handle_segment();
    void handle_mutex();
    void handle_semaphore();
    void handle_condition();
    void handle_clock();
    void handle_alarm();
    void handle_chronometer();
    void handle_ipc();
    void handle_utility();

private:
    static Member _handlers[LAST_TYPE_ID];
};


void Agent::handle_thread()
{
    Adapter<Thread> * thread = reinterpret_cast<Adapter<Thread> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE1: {
        int (*entry)();
        in(entry);
        id(Id(THREAD_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Thread>(Thread::Configuration(Thread::READY, Thread::NORMAL, WHITE, 0, 0), entry))));
    } break;
    case DESTROY:
        delete thread;
        break;
    case SELF:
        id(Id(THREAD_ID, reinterpret_cast<Id::Unit_Id>(Adapter<Thread>::self())));
        break;
    case THREAD_PRIORITY:
        res = thread->priority();
        break;
    case THREAD_PRIORITY1: {
        int p;
        in(p);
        thread->priority(p);
    } break;
    case THREAD_JOIN:
        res = thread->join();
        break;
    case THREAD_PASS:
        thread->pass();
        break;
    case THREAD_SUSPEND:
        thread->suspend();
        break;
    case THREAD_RESUME:
        thread->resume();
        break;
    case THREAD_YIELD:
        Thread::yield();
        break;
    case THREAD_WAIT_NEXT:
        //            Periodic_Thread::wait_next();
        break;
    case THREAD_EXIT: {
        int r;
        in(r);
        Thread::exit(r);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_task()
{
    Adapter<Task> * task = reinterpret_cast<Adapter<Task> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE3: {
        Segment * cs, * ds;
        int (*entry)();
        in(cs, ds, entry);
        id(Id(TASK_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Task>(cs, ds, entry))));
    } break;
    case DESTROY:
        delete task;
        break;
    case SELF:
        id(Id(TASK_ID, reinterpret_cast<Id::Unit_Id>(Adapter<Task>::self())));
        break;
    case TASK_ADDRESS_SPACE:
        res = reinterpret_cast<int>(task->address_space());
        break;
    case TASK_CODE_SEGMENT:
        res = reinterpret_cast<int>(task->code_segment());
        break;
    case TASK_DATA_SEGMENT:
        res = reinterpret_cast<int>(task->data_segment());
        break;
    case TASK_CODE:
        res = task->code();
        break;
    case TASK_DATA:
        res = task->data();
        break;
    case TASK_MAIN:
        res = reinterpret_cast<int>(task->main());
        break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_active()
{
    result(UNDEFINED);
};


void Agent::handle_address_space()
{
    Adapter<Address_Space> * as = reinterpret_cast<Adapter<Address_Space> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE:
        id(Id(ADDRESS_SPACE_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Address_Space>())));
        break;
    case CREATE1:
        MMU::Page_Directory * pd;
        in(pd);
        id(Id(ADDRESS_SPACE_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Address_Space>(pd))));
        break;
    case DESTROY:
        delete as;
        break;
    case ADDRESS_SPACE_PD:
        res = as->pd();
        break;
    case ADDRESS_SPACE_ATTACH1: {
        Segment * seg;
        in(seg);
        res = as->attach(seg);
    } break;
    case ADDRESS_SPACE_ATTACH2: {
        Segment * seg;
        CPU::Log_Addr addr;
        in(seg, addr);
        res = as->attach(seg, addr);
    } break;
    case ADDRESS_SPACE_DETACH1: {
        Segment * seg;
        in(seg);
        as->detach(seg);
    } break;
    case ADDRESS_SPACE_DETACH2: {
        Segment * seg;
        CPU::Log_Addr addr;
        in(seg, addr);
        as->detach(seg, addr);
    } break;
    case ADDRESS_SPACE_PHYSICAL: {
        CPU::Log_Addr addr;
        in(addr);
        res = as->physical(addr);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_segment()
{
    Adapter<Segment> * seg = reinterpret_cast<Adapter<Segment> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE1: {
        unsigned int bytes;
        in(bytes);
        id(Id(SEGMENT_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Segment>(bytes))));
    } break;
    case CREATE2: { // *** indistinguishable ***
        unsigned int bytes;
    Segment::Flags flags;
    in(bytes, flags);
    id(Id(SEGMENT_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Segment>(bytes, WHITE, flags))));
    } break;
    case CREATE3: { // *** indistinguishable ***
        Segment::Phy_Addr phy_addr;
    unsigned int bytes;
    Segment::Flags flags;
    in(phy_addr, bytes, flags);
    id(Id(SEGMENT_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Segment>(phy_addr, bytes, flags))));
    } break;
    case DESTROY:
        delete seg;
        break;
    case SEGMENT_SIZE:
        res = seg->size();
        break;
    case SEGMENT_PHY_ADDRESS:
        res = seg->phy_address();
        break;
    case SEGMENT_RESIZE: {
        int amount;
        in(amount);
        res = seg->resize(amount);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_mutex()
{
    result(UNDEFINED);
};


void Agent::handle_semaphore()
{
    result(UNDEFINED);
};


void Agent::handle_condition()
{
    result(UNDEFINED);
};


void Agent::handle_clock()
{
    result(UNDEFINED);
};


void Agent::handle_alarm()
{
    Adapter<Alarm> * alarm = reinterpret_cast<Adapter<Alarm> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE2: {
        Alarm::Microsecond time;
        Handler * handler;
        in(time, handler);
        id(Id(ALARM_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Alarm>(time, handler))));
    } break;
    case CREATE3: {
        Alarm::Microsecond time;
        Handler * handler;
        int times;
        in(time, handler, times);
        id(Id(ALARM_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Alarm>(time, handler, times))));
    } break;
    case DESTROY:
        delete alarm;
        break;
    case ALARM_GET_PERIOD:
        res = alarm->period();
    break;
    case ALARM_SET_PERIOD: {
        Alarm::Microsecond p;
        in(p);
        alarm->period(p);
    } break;
    case ALARM_FREQUENCY:
        res = Adapter<Alarm>::alarm_frequency();
    break;
    case ALARM_DELAY: {
        Alarm::Microsecond time;
        in(time);
        Adapter<Alarm>::delay(time);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_chronometer()
{
    result(UNDEFINED);
};


//void Agent::handle_communicator()
//{
//    Adapter<Port<IPC>> * comm = reinterpret_cast<Adapter<Port<IPC>> *>(id().unit());
//    Result res = 0;
//
//    switch(method()) {
//    case CREATE1: {
//        Port<IPC>::Local_Address local;
//        in(local);
//        id(Id(COMMUNICATOR_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Port<IPC>>(local))));
//
//        if((local == DOM0_ID) && !_dom0)
//            _dom0 = reinterpret_cast<Port<IPC> *>(id().unit());
//    } break;
//    case DESTROY: {
//        delete comm;
//    } break;
//    case COMMUNICATOR_SEND: {
//        IPC::Address to;
//        void * data;
//        unsigned int size;
//        in(to, data, size);
//        comm->send(to, data, size);
//    } break;
//    case COMMUNICATOR_RECEIVE: {
//        IPC::Address from;
//        void * data;
//        unsigned int size;
//        in(from, data, size);
//        comm->receive(&from, data, size);
//        out(from);
//    } break;
//    case COMMUNICATOR_BIND: {
//        IPC::Address addr;
//        Port<IPC> * comm;
//        in(addr, comm);
//        _dom0 = comm;
//    } break;
//    default:
//        res = UNDEFINED;
//    }
//};


void Agent::handle_ipc()
{
    Adapter<Port<IPC>> * comm = reinterpret_cast<Adapter<Port<IPC>> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE1: {
        Port<IPC>::Local_Address local;
        in(local);
        id(Id(IPC_COMMUNICATOR_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Port<IPC>>(local))));
    } break;
    case DESTROY: {
        delete comm;
    } break;
    case COMMUNICATOR_SEND: {
        Message * usr_msg;
        in(usr_msg);
        Message sys_msg(*usr_msg);
        if(id().unit() != IPC_COMMUNICATOR_ID)
            sys_msg.id(Id(IPC_COMMUNICATOR_ID, id().unit()));
        comm->send(&sys_msg);
    } break;
    case COMMUNICATOR_RECEIVE: {
        Message * usr_msg;
        in(usr_msg);
        comm->receive(usr_msg);
    } break;
    case COMMUNICATOR_REPLY: {
        Message * usr_msg;
        in(usr_msg);
        Message sys_msg(*usr_msg);
        if(id().unit() != IPC_COMMUNICATOR_ID)
            sys_msg.id(Id(IPC_COMMUNICATOR_ID, id().unit()));
        comm->reply(&sys_msg);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
}


void Agent::handle_utility()
{
    Result res = 0;

    switch(method()) {
    case PRINT: {
        const char * s;
        in(s);
        _print(s);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};

__END_SYS

#endif

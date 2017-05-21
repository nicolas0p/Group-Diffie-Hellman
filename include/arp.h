// EPOS ARP Protocol Declarations

#ifndef __arp_h
#define __arp_h

#include <utility/hash.h>
#include <utility/spin.h>
#include <system.h>
#include <alarm.h>
#include <semaphore.h>

__BEGIN_SYS

template<typename NIC, typename Network, unsigned int HTYPE = 1>  // Ethernet Hardware Type (HTYPE) = 1
class ARP: private NIC::Observer
{
public:
    // Addresses
    typedef typename Network::Address PA;
    typedef typename NIC::Address HA;

private:
    static const unsigned int ENTRIES = Traits<Build>::NODES;

    class Mapping;
    typedef Simple_Hash<Mapping, ENTRIES, PA> Table;
    typedef typename Table::Element Element;

public:
    // ARP/RARP Operations
    typedef unsigned short Oper;
    enum {
        REQUEST      = 1,
        REPLY        = 2
    };


    // ARP Packet
    class Packet
    {
    public:
        Packet(Oper op, const HA & sha, const PA & spa, const HA & tha, const PA & tpa)
        : _htype(htons(HTYPE)), _ptype(htons(Network::PROTOCOL)), _hlen(sizeof(HA)), _plen(sizeof(PA)), _oper(htons(op)), _sha(sha), _spa(spa), _tha(tha), _tpa(tpa) {}
        ~Packet() {}

        void op(const Oper & o) { _oper = htons(o); }
        void sha(const HA & a) { _sha = a; }
        void spa(const PA & a) { _spa = a; }
        void tha(const HA & a) { _tha = a; }
        void tpa(const PA & a) { _tpa = a; }

        Oper op() const { return ntohs(_oper); }
        const HA & sha() const { return _sha; }
        const PA & spa() const { return _spa; }
        const HA & tha() const { return _tha; }
        const PA & tpa() const { return _tpa; }

        friend Debug & operator<<(Debug & db, const Packet & p) {
            db  << "{htp=" << ntohs(p._htype)
                << ",ptp=" << hex << ntohs(p._ptype) << dec
                << ",hln=" << p._hlen
                << ",pln=" << p._plen
                << ",opc=" << ntohs(p._oper)
                << ",sha=" << p._sha
                << ",spa=" << p._spa
                << ",tha=" << p._tha
                << ",tpa=" << p._tpa << "}";
            return db;
        }

    private:
        unsigned short  _htype; // Hardware Type
        unsigned short  _ptype; // Protocol Type
        unsigned char   _hlen;  // Hardware Address Length
        unsigned char   _plen;  // Protocol Address Length
        unsigned short  _oper;  // Operation
        HA              _sha;   // Sender Hardware Address (48 bits)
        PA              _spa;   // Sender Protocol Address (32 bits)
        HA              _tha;   // Target Hardware Address (48 bits)
        PA              _tpa;   // Target Protocol Address (32 bits)
    } __attribute__((packed));


private:
    class Mapping
    {
    public:
        Mapping(const PA & pa, const HA & ha): _ha(ha), _sem(0), _link(this, pa) {}
        Mapping(const PA & pa, Semaphore * sem): _ha(HA::NULL), _sem(sem), _link(this, pa) {}

        const HA & ha() const { return _ha; }
        Semaphore * semaphore() { return _sem; }
        Element * link() { return &_link; }

        void update(const HA & ha) {
            _ha = ha;
            if(_sem) {
                _sem->v();
                _sem = 0;
            }
        }

        friend Debug & operator<<(Debug & db, const Mapping & m) {
            db  << "{pa=" << m._link.key() << ",ha=" << m._ha << ",sem=" << m._sem << "}";
            return db;
        }

    private:
        HA _ha;
        Semaphore * _sem;
        Element _link; // PA is the key
    };


public:
    ARP(NIC * nic, Network * net): _nic(nic), _net(net) {
        db<ARP>(TRC) << "ARP::ARP(nic=" << nic << ",net=" << net << ") => " << this << endl;

        _nic->attach(this, NIC::ARP);
    }

    ~ARP() {
        db<ARP>(TRC) << "ARP::~ARP(this=" << this << ")" << endl;

        _nic->detach(this, NIC::ARP);

        lock();
        for(typename Table::Iterator it = _table.begin(); it != _table.end(); it++) {
            if(it) {
                db<ARP>(INF) << "ARP::~ARP: removing and deleting " << *it->object() << endl;

                _table.remove(it);
                delete it->object();
            }
        }
        unlock();
    }

    void insert(const PA & pa, const HA & ha) {
        db<ARP>(TRC) << "ARP::insert(pa=" << pa << ",ha=" << ha << ")" << endl;

        Mapping * map = new (SYSTEM) Mapping(pa, ha);

        lock();
        _table.insert(map->link());
        unlock();
    }

    void remove(const PA & pa) {
        db<ARP>(TRC) << "ARP::remove(pa=" << pa << ")" << endl;

        Element * el = _table.remove_key(pa);
        if(el) {
            db<ARP>(INF) << "ARP::remove: removing and deleting " << *el->object() << endl;
            delete el->object();
        }
    }

    HA resolve(const PA & pa) {
        db<ARP>(TRC) << "ARP::resolve(pa=" << pa << ") => ";

        volatile HA ha = HA(HA::NULL);

        lock();
        Element * el = _table.search_key(pa);
        unlock();

        if(el)
            ha = el->object()->ha();
        else {
            db<ARP>(TRC) << "sending requests" << endl;

            Semaphore sem(0);
            Mapping * map = new (SYSTEM) Mapping(pa, &sem);
            lock();
            _table.insert(map->link());
            unlock();

            for(unsigned int i = 0; (i < Traits<Network>::RETRIES) && !ha; i++) {
                Packet request(REQUEST, _nic->address(), _net->address(), HA::BROADCAST, pa);
                db<ARP>(INF) << "ARP::resolve:request=" << request << endl;
                _nic->send(HA::BROADCAST, NIC::ARP, &request, sizeof(Packet));

                Semaphore_Handler handler(&sem);
                Alarm alarm(Traits<Network>::TIMEOUT * 1000000, &handler, 1);
                sem.p();

                ha = map->ha();
            }

            lock();
            if(!ha) {
                _table.remove(map->link());
                delete map;
            }
            unlock();

            db<ARP>(TRC) << "ARP::resolve(pa=" << pa << ") => ";
        }

        // Even being declared volatile, "ha" gets messed up and a PF occurs without the memcpy
        HA ha2;
        memcpy(&ha2, const_cast<HA *>(&ha), sizeof(HA));

        db<ARP>(TRC) << ha2 << endl;

        return ha2;
    }

//    PA resolve(const HA & ha) {
//        db<ARP>(TRC) << "RARP::resolve(pa=" << ha << ")" << endl;
//
//        Condition * cond = _table.insert(pa);
//        for(unsigned int i = 0; (i < Traits<Network>::RETRIES) && !ha; i++) {
//            Packet request(REQUEST, _nic->address(), _net->address(), HA::BROADCAST, pa);
//            _nic->send(HA::BROADCAST, NIC::RARP, &request, sizeof(Packet));
//            db<ARP>(INF) << "ARP::resolve:sent packet=" << &request << " => " << request << endl;
//
//            Condition_Handler handler(cond);
//            Alarm alarm(Traits<Network>::TIMEOUT * 1000000, &handler, 1);
//            cond->wait();
//
//            ha = _table.search(pa);
//        }
//
//        db<ARP>(INF) << "RARP::resolve(pa=" << pa << ") => " << ha << endl;
//
//        return ha;
//    }

    void update(typename NIC::Observed * obs, typename NIC::Protocol prot, typename NIC::Buffer * buf)
    {
        db<ARP>(TRC) << "ARP::update(obs=" << obs << ",prot=" << prot << ",buf=" << buf << ")" << endl;

        Packet * packet = buf->frame()->template data<Packet>();
        db<ARP>(INF) << "ARP::update:pkt=" << packet << " => " << *packet << endl;

        if((packet->op() == REQUEST) && (packet->tpa() == _net->address())) {

            Packet reply(REPLY, _nic->address(), _net->address(), packet->sha(), packet->spa());
            db<ARP>(TRC) << "ARP::update: replying query for " << packet->tpa() << " with " << reply << endl;
            _nic->send(packet->sha(), NIC::ARP, &reply, sizeof(Packet));

        } else if((packet->op() == REPLY) && (packet->tha() == _nic->address())) {

            lock();
            Element * el = _table.search_key(packet->spa());
            if(el) {
                db<ARP>(TRC) << "ARP::update: got reply for query on " << packet->spa() << ": " << packet->sha() << endl;
                el->object()->update(packet->sha());
            } else
                db<ARP>(WRN) << "ARP::update: got reply for query on " << packet->spa() << ", which is not in table!" << endl;
            unlock();

        }

        _nic->free(buf);
    }

    void dump() {
        db<ARP>(INF) << "ARP::Table => {" << endl;
        for(typename Table::Iterator it = _table.begin(); it != _table.end(); it++) {
            if(it)
                db<ARP>(INF) << hex << it << " => {" << it->key() << "," << it->object()->ha() << "}" << endl;
            else
                db<ARP>(INF) << hex << it << " => EMPTY" << endl;
        }
        db<ARP>(INF) << "}" << endl;

    }

private:
    void lock() {
        CPU::int_disable();
        if(Traits<System>::multicore)
            _lock.acquire();
    }

    void unlock() {
        if(Traits<System>::multicore)
            _lock.release();
        CPU::int_enable();
    }

private:
    Table _table;
    Spin _lock;
    NIC * _nic;
    Network * _net;
};

__END_SYS

#endif

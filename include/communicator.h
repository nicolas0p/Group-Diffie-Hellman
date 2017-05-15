// EPOS Communicator Declarations

#ifndef __communicator_h
#define __communicator_h

#include <channel.h>

__BEGIN_SYS

// Commonalities for connectionless channels
template<typename Channel, bool connectionless>
class Communicator_Common: protected Channel::Observer, private Semaphore_Observer<typename Channel::Observer::Observed_Data, typename Channel::Observer::Observing_Condition>
{
private:
    static const unsigned int HEADERS_SIZE = Channel::HEADERS_SIZE;

    typedef typename Channel::Observer::Observing_Condition Observing_Condition;
    typedef Semaphore_Observer<typename Channel::Observer::Observed_Data, typename Channel::Observer::Observing_Condition> Observer;

public:
    // List to hold received Buffers
    typedef typename Channel::Buffer Buffer;
    typedef typename Buffer::List List;
    typedef typename List::Element Element;

    // Addresses
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

protected:
    Communicator_Common(const Local_Address & local): _local(local) {
        Channel::attach(this, local);
    }

public:
    ~Communicator_Common() {
        Channel::detach(this, _local);
    }

    template<typename Message>
    int send(const Message & message) {
        return Channel::send(message);
    }
    int send(const Address & to, const void * data, unsigned int size) {
        return Channel::send(_local, to, data, size);
    }
    int send(const Local_Address & from, const Address & to, const void * data, unsigned int size) {
        return Channel::send(from, to, data, size);
    }

    template<typename Message>
    int receive(const Message & message) {
        Buffer * buf = updated();
        return Channel::receive(buf, message);
    }
    int receive(void * data, unsigned int size) {
        Buffer * buf = updated();
        return Channel::receive(buf, data, size);
    }
    int receive(Address * from, void * data, unsigned int size) {
        Buffer * buf = updated();
        return Channel::receive(buf, from, data, size);
    }

    int receive_all(void * data, unsigned int size) {
        int r = 0;
        for(unsigned int received = 0, coppied = 0; received < size; received += coppied) {
            Buffer * buf = updated();
            r += Channel::receive(buf, data + received, coppied = ((received + (buf->size() - HEADERS_SIZE)) > size ? (size - received) : (buf->size() - HEADERS_SIZE)));
        }
        return r;
    }
    int receive_all(Address * from, void * data, unsigned int size) {
        int r = 0;
        for(unsigned int received = 0, coppied = 0; received < size; received += coppied) {
            Buffer * buf = updated();
            r += Channel::receive(buf, data + received, coppied = ((received + (buf->size() - HEADERS_SIZE)) > size ? (size - received) : (buf->size() - HEADERS_SIZE)));
        }
        return r;
    }

    template<typename Message>
    int reply(const Message & message) {
        return Channel::reply(message);
    }

private:
    void update(typename Channel::Observed * obs, Observing_Condition c, Buffer * buf) { Observer::update(c, buf); }
    Buffer * updated() { return Observer::updated(); }

private:
    Local_Address _local;
};

// Commonalities for connection-oriented channels
template<typename Channel>
class Communicator_Common<Channel, false>: protected Channel::Observer, private Semaphore_Observer<typename Channel::Observer::Observed_Data, typename Channel::Observer::Observing_Condition>
{
private:
    static const unsigned int HEADERS_SIZE = Channel::HEADERS_SIZE;

    typedef typename Channel::Observer::Observing_Condition Observing_Condition;
    typedef Semaphore_Observer<typename Channel::Observer::Observed_Data, typename Channel::Observer::Observing_Condition> Observer;

public:
    // List to hold received Buffers
    typedef typename Channel::Buffer Buffer;
    typedef typename Buffer::List List;
    typedef typename List::Element Element;

    // Addresses
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

protected:
    Communicator_Common(const Local_Address & local, const Address & peer): _local(local) {
        _connection = Channel::attach(this, local, peer);
    }

public:
    ~Communicator_Common() {
        Channel::detach(this, _connection);
    }

    int send(const void * data, unsigned int size) {
        return _connection->send(data, size);
    }

    int receive_some(void * data, unsigned int size) {
        Buffer * buf = updated();
        return _connection->receive(buf, data, size);
    }

    int receive(void * d, unsigned int size) {
        char * data = reinterpret_cast<char *>(d);
        unsigned int received = 0;
        do {
            Buffer * buf = updated();
            unsigned int segment_size = _connection->receive(buf, data, size);
            data += segment_size;
            received += segment_size;
        } while(received <= size);
        return size;
    }

    int receive_all(void * d, unsigned int size) {
        char * data = reinterpret_cast<char *>(d);
        int r = 0;
        for(unsigned int received = 0, coppied = 0; received < size; received += coppied) {
            Buffer * buf = updated();
            r += _connection->receive(buf, data + received, coppied = ((received + (buf->size() - HEADERS_SIZE)) > size ? (size - received) : (buf->size() - HEADERS_SIZE)));
        }
        return r;
    }

private:
    void update(typename Channel::Observed * obs, Observing_Condition c, Buffer * buf) { Observer::update(c, buf); }
    Buffer * updated() { return Observer::updated(); }

protected:
    Local_Address _local;

    typename Channel::Connection * _connection;
};


// Link (point-to-point communicator) connectionless channels
template<typename Channel, bool connectionless>
class Link: public Communicator_Common<Channel, connectionless>
{
private:
    typedef Communicator_Common<Channel, connectionless> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Link(const Local_Address & local, const Address & peer = Address::NULL): Base(local), _peer(peer) {}
    ~Link() {}

    int send(const void * data, unsigned int size) { return Base::send(_peer, data, size); }
    int receive(void * data, unsigned int size) { return Base::receive(data, size); }
    int receive_all(void * data, unsigned int size) { return Base::receive_all(data, size); }

    int read(void * data, unsigned int size) { return receive_all(data, size); }
    int write(const void * data, unsigned int size) { return send(data, size); }

    const Address & peer() const { return _peer;}

private:
    Address _peer;
};

// Link (point-to-point communicator) for connection-oriented channels
template<typename Channel>
class Link<Channel, false>: public Communicator_Common<Channel, false>
{
private:
    typedef Communicator_Common<Channel, false> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Link(const Local_Address & local, const Address & peer = Address::NULL): Base(local, peer), _peer(peer) {}
    ~Link() {}

    int send(const void * data, unsigned int size) { return Base::send(data, size); }
    int receive(void * data, unsigned int size) { return Base::receive(data, size); }
    int receive_all(void * data, unsigned int size) { return Base::receive_all(data, size); }

    int read(void * data, unsigned int size) { return receive_all(data, size); }
    int write(const void * data, unsigned int size) { return send(data, size); }

    const Address & peer() const { return _peer;}

private:
    Address _peer;
};


// Port (1-to-N communicator) for connectionless channels
template<typename Channel, bool connectionless>
class Port: public Communicator_Common<Channel, connectionless>
{
private:
    typedef Communicator_Common<Channel, connectionless> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Port(const Local_Address & local): Base(local) {}
    ~Port() {}

    template<typename Message>
    int send(const Message & message) { return Base::send(message); }
    int send(const Address & to, const void * data, unsigned int size) { return Base::send(to, data, size); }

    template<typename Message>
    int receive(const Message & message) { return Base::receive(message); }
    int receive(Address * from, void * data, unsigned int size) { return Base::receive(from, data, size); }

    template<typename Message>
    int reply(const Message & message) { return Base::reply(message); }
};

// Port (1-to-N communicator) for connection-oriented channels
template<typename Channel>
class Port<Channel, false>: public Communicator_Common<Channel, false>
{
private:
    typedef Communicator_Common<Channel, false> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Port(const Local_Address & local): Base(local) {}
    ~Port() {}

    Link<Channel> * listen() { return new (SYSTEM) Link<Channel>(Channel::listen(this->_local)); }
    Link<Channel> * connect(const Address & to) { return new (SYSTEM) Link<Channel>(Channel::connect(this->_local, to)); }
};

__END_SYS

#endif

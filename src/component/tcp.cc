// EPOS Transmission Control Protocol (RFC 793) Implementation

#include <system/config.h>
#ifndef __no_networking__

#include <tcp.h>

__BEGIN_SYS

// Class attributes
TCP::Observed TCP::_observed;

TCP::Connection::State_Handler TCP::Connection::_handlers[] = {&TCP::Connection::listening,
                                                               &TCP::Connection::syn_sent,
                                                               &TCP::Connection::syn_received,
                                                               &TCP::Connection::established,
                                                               &TCP::Connection::fin_wait1,
                                                               &TCP::Connection::fin_wait2,
                                                               &TCP::Connection::close_wait,
                                                               &TCP::Connection::closing,
                                                               &TCP::Connection::last_ack,
                                                               &TCP::Connection::time_wait,
                                                               &TCP::Connection::closed};

// Methods
void TCP::update(IP::Observed * obs, IP::Protocol prot, NIC::Buffer * pool)
{
    db<TCP>(TRC) << "TCP::update(obs=" << obs << ",prot=" << prot << ",buf=" << pool << ")" << endl;
    db<TCP>(INF) << "TCP::update:buf=" << pool << " => " << *pool << endl;

    Packet * packet = pool->frame()->data<Packet>();
    Segment * segment = packet->data<Segment>();

    unsigned int size = pool->size() - sizeof(IP::Header) - sizeof(TCP::Header);
    if(size && !segment->check(size)) { // FIXME there should not be a check for "size", it should always check the sum. However, it doesn't seem to work when size == 0
        db<TCP>(WRN) << "TCP::update: wrong message checksum!" << endl;
        pool->nic()->free(pool);
        return;
    }

    unsigned long long id;
    if(segment->header()->flags() == Header::SYN) // try to notify any eventual listener
        id = Connection::id(segment->header()->to(), 0, IP::Address::NULL);
    else
        id = Connection::id(segment->header()->to(), segment->header()->from(), packet->header()->from());

    db<TCP>(INF) << "TCP::update::condition=" << hex << id << endl;

    if(!_observed.notify(id, pool))
        pool->nic()->free(pool);
}

void TCP::Segment::sum(const IP::Address & from, const IP::Address & to, const void * data, unsigned int size)
{
    _checksum = 0;

    IP::Pseudo_Header pseudo(from, to, IP::TCP, sizeof(Header) + size);

    unsigned long sum = 0;
    const unsigned char * ptr = reinterpret_cast<const unsigned char *>(&pseudo);
    for(unsigned int i = 0; i < sizeof(IP::Pseudo_Header); i += 2)
        sum += (ptr[i] << 8) | ptr[i+1];

    ptr = reinterpret_cast<const unsigned char *>(header());
    for(unsigned int i = 0; i < sizeof(Header); i += 2)
        sum += (ptr[i] << 8) | ptr[i+1];

    if(data) {
        ptr = reinterpret_cast<const unsigned char *>(data);
        for(unsigned int i = 0; i < size; i += 2)
            sum += (ptr[i] << 8) | ptr[i+1];
        if(size & 1)
            sum += ptr[size - 1];
    }

    while(sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    _checksum = htons(~sum);
}

void TCP::Connection::fsend(const Flags & flags)
{
    _flags = flags;
    if(!_retransmiting) _sequence = htonl(_next);

    db<TCP>(TRC) << "TCP::Connection::send(flags=" << ((flags & ACK) ? 'A' : '-') << ((flags & RST) ? 'R' : '-') << ((flags & SYN) ? 'S' : '-') << ((flags & FIN) ? 'F' : '-') << "): SND.NXT=" << _next << ",SND.SEQ=" << sequence() << endl;

    Buffer * buf = IP::alloc(peer(), IP::TCP, sizeof(Header), 0);
    if(!buf) {
        db<TCP>(WRN) << "TCP::send: failed to alloc a NIC buffer to send a TCP control segment!" << endl;
        return;
    }
    db<TCP>(INF) << "TCP::send:buf=" << buf << " => " << *buf<< endl;

    Packet * packet = buf->frame()->data<Packet>();
    Segment * segment = packet->data<Segment>();
    memcpy(segment, header(), sizeof(Header));
    segment->sum(packet->from(), packet->to(), 0, 0);

    db<TCP>(INF) << "TCP::Connection::send:conn=" << this << " => " << *this << endl;

    if((_flags & FIN) || (_flags & SYN))
        _next++; // We do not test if there's a retransmission going on, because there's no chance whatsoever in this current implementation
    // a flag such as FIN or SYN to be sent whilst a stream (solo case in which there could be a retransmission) is going on

    // FIXME what if we increment the SND.NXT and soon after we receive a segment with data?
    // We'd ack it with incremented sequence number and only then send the segment that caused the SND.NXT variable to be incremented

    IP::send(buf); // implicitly releases the buffer
}

int TCP::Connection::send(const void * d, unsigned int size)
{
    const unsigned char * data = reinterpret_cast<const unsigned char *>(d);

    db<TCP>(TRC) << "TCP::Connection::send(f=" << from() << ",t=" << peer() << ":" << to() << ",d=" << data << ",s=" << size << ")" << endl;

    unsigned int allowed = WINDOW;
    unsigned int left = size; // bytes that have not been sent at all
    unsigned int acknowledged = 0; // bytes that were sent AND acknowledged
    unsigned int initial_seq = sequence(); // sequence number when stream is started

    unsigned int tries = 0;
    for(; (tries < RETRIES) && (acknowledged != size) && (_state == ESTABLISHED || _state == CLOSE_WAIT);
        acknowledged = _current->header()->acknowledgment() - initial_seq, allowed = _peer_window - (sequence() - _current->header()->acknowledgment())) {
        _streaming = true;
        allowed = (allowed > MSS) ? MSS: allowed;

        if(allowed && left) {
            db<TCP>(TRC) << "TCP::Connection::send: send" << endl;

            int payload = (allowed > left) ? left : allowed;

            if(!dsend(data, payload)) // FIXME we should wait until there are available buffers
                return -1;

            data += payload;
            left -= payload;
            if(sequence() == _next)
                _retransmiting = false;
        } else { // Either window's full or we've sent all there was to
            db<TCP>(TRC) << "TCP::Connection::send: wait" << endl;

            unsigned int old_ack = _current->header()->acknowledgment();

            Condition_Handler h(&_stream);
            Alarm a(TIMEOUT, &h);

            _stream.wait();

            if(_current->header()->acknowledgment() == old_ack) {
                // Retransmission
                db<TCP>(TRC) << "TCP::Connection::send: retransmission" << endl;

                _retransmiting = true;
                _sequence = htonl(_current->header()->acknowledgment());
                _unacknowledged = _current->header()->acknowledgment();
                data = reinterpret_cast<const unsigned char*>(d) + acknowledged;
                left = size - acknowledged;

                tries++;
            } else
                tries = 0;
        }
    }

    _streaming = false;
    _retransmiting = false;

    if(tries == RETRIES) {
        db<TCP>(TRC) << "TCP::send: Enough tries already!" << endl;

        return -1;
    } else if(_state != ESTABLISHED && _state != CLOSE_WAIT) {
        db<TCP>(WRN) << "TCP::send: The connection is not open. It is not possible to stream." << endl;

        return -1;
    }


    return size;
}

int TCP::Connection::dsend(const void * d, unsigned int size)
{
    const unsigned char * data = reinterpret_cast<const unsigned char *>(d);

    db<TCP>(TRC) << "TCP::dsend(f=" << from() << ",t=" << peer() << ":" << to() << ",d=" << data << ",s=" << size << ")" << endl;

    _flags = ACK;
    if(!_retransmiting)
        _sequence = htonl(_next);

    db<TCP>(TRC) << "TCP::Connection::dsend: SND.NXT=" << _next << ",SND.SEQ=" << sequence() << ",payload=" << size << endl;

    Buffer * pool = IP::alloc(peer(), IP::TCP, sizeof(Header), size);
    if(!pool)
        return 0;

    unsigned int headers = sizeof(Header);
    for(Buffer::Element * el = pool->link(); el; el = el->next()) {
        Buffer * buf = el->object();
        Packet * packet = buf->frame()->data<Packet>();

        db<TCP>(INF) << "TCP::send:buf=" << buf << " => " << *buf<< endl;

        if(el == pool->link()) {
            Segment * segment = packet->data<Segment>();
            memcpy(segment, header(), sizeof(Header));
            segment->sum(packet->from(), packet->to(), data, buf->size() - sizeof(Header) - sizeof(IP::Header));
            memcpy(segment->data<void>(), data, buf->size() - sizeof(Header) - sizeof(IP::Header));
            data += buf->size() - sizeof(Header) - sizeof(IP::Header);

            db<TCP>(INF) << "TCP::send:msg=" << segment << " => " << *segment << endl;
        } else {
            memcpy(packet->data<void>(), data, buf->size() - sizeof(IP::Header));
            data += buf->size() - sizeof(IP::Header);
        }

        headers += sizeof(IP::Header);
    }

    if(!_retransmiting)
        _next += size;
    else
        _sequence = htonl(header()->sequence() + size);

    return IP::send(pool) - headers; // implicitly releases the pool
}

int TCP::Connection::receive(Buffer * pool, void * d, unsigned int s)
{
    unsigned char * data = reinterpret_cast<unsigned char *>(d);

    db<TCP>(TRC) << "TCP::receive(buf=" << pool << ",d=" << d << ",s=" << s << ")" << endl;

    Buffer::Element * head = pool->link();
    Packet * packet = head->object()->frame()->data<Packet>();
    Segment * segment = packet->data<Segment>();
    unsigned int size = 0;

    for(Buffer::Element * el = head; el && (size <= s); el = el->next()) {
        Buffer * buf = el->object();

        db<TCP>(INF) << "TCP::receive:buf=" << buf << " => " << *buf << endl;

        packet = buf->frame()->data<Packet>();

        unsigned int len = buf->size() - sizeof(IP::Header);
        if(el == head) {
            len -= sizeof(Header);
            memcpy(data, segment->data<void>(), len);

            db<TCP>(INF) << "TCP::receive:msg=" << segment << " => " << *segment << endl;
        } else
            memcpy(data, packet->data<void>(), len);

        db<TCP>(INF) << "TCP::receive:len=" << len << endl;

        data += len;
        size += len;
    }

    pool->nic()->free(pool);

    return size;
}

void TCP::Connection::update(TCP::Observed * obs, unsigned long long socket, NIC::Buffer * pool)
{
    db<TCP>(TRC) << "TCP::Connection::update(obs=" << obs << ",sock=" << hex << socket << ",buf=" << pool << ")" << endl;

    Packet * packet = pool->frame()->data<Packet>();

    _current = packet->data<Segment>(); // FIXME should free the previous buffer
    _length = pool->size() - sizeof(IP::Header) - sizeof(TCP::Header);
    _peer_window = _current->header()->window();

    db<TCP>(INF) << "TCP::Connection::update:" <<
        "SEQ.SEQ=" << _current->header()->sequence() <<
        ",RCV.NXT=" << acknowledgment() <<
        ",SEG.ACK=" << _current->header()->acknowledgment() <<
        ",SND.NXT=" << _next << endl;

    if(_state == LISTENING) {
        TCP::_observed.detach(this, id());
        _peer = packet->from();
        _to = htons(_current->header()->from());
        TCP::_observed.attach(this, id());
    }

    db<TCP>(INF) << "TCP::Connection::update:conn=" << this << " => " << *this << endl;

    if(!((_state == LISTENING) || (_state == SYN_SENT)) && _current->header()->sequence() > acknowledgment()) {
        // SEG.SEQ musn't be > than RCV.NXT, this forces segments to be accepted in order, except when connecting or listening, then one may receive stuff out of the blue
        // If SEG.SEQ < RCV.NXT, i.e. delayed or repeated segment, the treatment happens later
        pool->nic()->free(pool);
        return;
    }

    if(_current->header()->acknowledgment() > _next) {
        // SEG.ACK must be <= to SND.NXT, for one cannot ack what one is yet to receive
        fsend(RST);
        state(CLOSED);
        pool->nic()->free(pool);
        return;
    }

    bool relevant = false; // The segment is relevant to the sliding window
    if(_streaming) {
        if(_current->header()->acknowledgment() <= sequence()) {
            // Regular ack, i.e. SEG.ACK is <= than the last sequence I sent
            if(_current->header()->acknowledgment() > _unacknowledged)
                relevant = true; // A segment must ack something in order to be relevant

            _unacknowledged = _current->header()->acknowledgment();
        } else if(_current->header()->acknowledgment() > sequence()) {
            // Forward ack, i.e. SEG.ACK > SEG.SEQ, but not > than the SND.NXT. This scenario is only possible in this code and in the FSM during retransmission
            _sequence = htonl(_current->header()->acknowledgment());
            _unacknowledged = _current->header()->acknowledgment();
            relevant = true;
        }
    }

    State state_at_arrival = _state;

    (this->*_handler)();

    if(!_valid) {
        pool->nic()->free(pool);
        return;
    }

    if((state_at_arrival == ESTABLISHED)
        || (state_at_arrival == SYN_RECEIVED)
        || (state_at_arrival == FIN_WAIT1)
        || (state_at_arrival == FIN_WAIT2))
        if(_length)
            if(!notify(socket, pool))
                pool->nic()->free(pool);

    if(_streaming && relevant)
        _stream.signal();
}

void TCP::Connection::listen()
{
    db<TCP>(TRC) << "TCP::Connection::listen(at=" << hex << from() << ")" << endl;

    state(LISTENING);
    _transition.wait();

    fsend(SYN | ACK);
    _unacknowledged = _initial;
    state(SYN_RECEIVED);

    _transition.wait();
}

void TCP::Connection::connect()
{
    db<TCP>(TRC) << "TCP::Connection::connect(from=" << hex << from() << ",to=" << peer() << ":" << to() << ")" << endl;

    state(SYN_SENT);
    fsend(SYN);
    _unacknowledged = sequence();
    set_timeout();
    _transition.wait();
}

void TCP::Connection::close()
{
    db<TCP>(TRC) << "TCP::Connection::close()" << endl;

    if(_state == CLOSED)
        return;

    if(_state == LISTENING || _state == SYN_SENT)
        state(CLOSED);
    else {
        if(_state == ESTABLISHED || _state == SYN_RECEIVED)
            state(FIN_WAIT1);
        else if(_state == CLOSE_WAIT)
            state(LAST_ACK);
        fsend(ACK | FIN);
        set_timeout(TIMEOUT / 2);
        _transition.wait();
    }
}

void TCP::Connection::listening()
{
    db<TCP>(TRC) << "TCP::Connection::listening()" << endl;

    if((_current->header()->flags() & RST) || (_current->header()->flags() & FIN)) {
        _valid = false;
        return;
    }

    if(_current->header()->flags() & ACK) {
        _valid = false;
        fsend(RST);
        return;
    }

    if(_current->header()->flags() & SYN) {
        _to = htons(_current->header()->from());
        _acknowledgment = htonl(_current->header()->sequence() + 1);
        _transition.signal();
    }
}

void TCP::Connection::syn_sent()
{
    db<TCP>(TRC) << "TCP::Connection::syn_sent()" << endl;

    if(_current->header()->flags() & ACK) {
        if((_current->header()->acknowledgment() <= _initial) || (_current->header()->acknowledgment() > _next)) {
            db<TCP>(WRN) << "TCP::Connection::syn_sent: bad acknowledgment number!" << endl;

            _valid = false;
            fsend(RST);

            return;
        }

        if((_current->header()->acknowledgment() >= _unacknowledged)
            && (_current->header()->acknowledgment() <= _next)) {
            if(_current->header()->flags() & RST) {
                _valid = false;
                state(CLOSED);

                return;
            }

            if(_current->header()->flags() & SYN) {
                _acknowledgment = htonl(_current->header()->sequence() + 1);
                _unacknowledged = _current->header()->acknowledgment();
                _peer_window = _current->header()->window();

                if(_unacknowledged > _initial) {
                    db<TCP>(INF) << "TCP::Connection::syn_sent: connection established!" << endl;

                    fsend(ACK);
                    state(ESTABLISHED);
                    _tries = 0;
                    _transition.signal();

                    return;
                }

                fsend(SYN | ACK);
                state(SYN_RECEIVED);

                return;
            }

            return;
        }

        db<TCP>(WRN) << "TCP::Connection::syn_sent: bad acknowledgment number!" << endl;

        _valid = false;
        fsend(RST);

        return;
    }

    if(!(_current->header()->flags() & RST) && (_current->header()->flags() & SYN)) { // Simultaneous SYN
        _acknowledgment = htonl(_current->header()->sequence() + 1);

        fsend(SYN | ACK);
        state(SYN_RECEIVED);

        return;
    }

    db<TCP>(WRN) << "TCP::Connection::syn_sent: bad FSM transaction! Closing connection!" << endl;

    close();
}

void TCP::Connection::syn_received()
{
    db<TCP>(TRC) << "TCP::Connection::syn_received()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        _valid = false;
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        _valid = false;
        fsend(RST);
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & ACK) {
        if((_unacknowledged <= _current->header()->acknowledgment())
            && (_current->header()->acknowledgment() <= _next)) {
            db<TCP>(INF) << "TCP::Connection::syn_received: connection established!" << endl;

            state(ESTABLISHED);
            _tries = 0;

            if(_length) {
                _acknowledgment = htonl(acknowledgment() + _length);
                fsend(ACK);
            }

            _transition.signal();
        } else if(_current->header()->flags() & FIN) {
            process_fin();
            state(CLOSE_WAIT);

            db<TCP>(INF) << "TCP::Connection::syn_received-->close_wait" << endl;
        } else {
            _valid = false;
            fsend(RST);
            state(CLOSED);
        }
    }
}

void TCP::Connection::established()
{
    db<TCP>(TRC) << "TCP::Connection::established()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        db<TCP>(WRN) << "TCP::Connection::established: reset received! Closing connection!" << endl;

        _valid = false;
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        db<TCP>(WRN) << "TCP::Connection::established: SYN received! Closing connection!" << endl;

        _valid = false;
        fsend(RST);
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & ACK) {
        if(_unacknowledged <= _current->header()->acknowledgment()
            && _current->header()->acknowledgment() <= _next) { // implicit reject out-of-order segments
            db<TCP>(TRC) << "TCP::Connection::established: ACK received"
                << endl;

            if(_length) {
                _acknowledgment = htonl(acknowledgment() + _length);
                fsend(ACK);
            }

            if(_current->header()->flags() & FIN) {
                process_fin();
                state(CLOSE_WAIT);

                db<TCP>(TRC) << "TCP::Connection::established-->close_wait" << endl;
            }
        }
    }
}

void TCP::Connection::fin_wait1()
{
    db<TCP>(TRC) << "TCP::Connection::fin_wait1()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        _valid = false;
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        _valid = false;
        fsend(RST);
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & ACK) {
        db<TCP>(TRC) << "TCP::Connection::fin_wait1: ACK received" << endl;

        if(_length) {
            _acknowledgment = htonl(acknowledgment() + _length);
            fsend(ACK);
        }

        if(_current->header()->acknowledgment() >= _next) { // our FIN has been acknowledged
            db<TCP>(TRC) << "TCP::Connection::fin_wait1: our FIN has been acknowledged" << endl;

            if(_current->header()->flags() & FIN) {
                process_fin();
                state(TIME_WAIT);
                set_timeout();

                db<TCP>(TRC) << "TCP::Connection::fin_wait1-->time_wait" << endl;
            } else {
                state(FIN_WAIT2);

                db<TCP>(TRC) << "TCP::Connection::fin_wait1-->fin_wait2" << endl;
            }
        } else {
            if(_current->header()->flags() & FIN) {
                process_fin();
                state(CLOSING);

                db<TCP>(TRC) << "TCP::Connection::fin_wait1-->closing" << endl;
            }
        }
    }
}

void TCP::Connection::fin_wait2()
{
    db<TCP>(TRC) << "TCP::Connection::fin_wait2()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        _valid = false;
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        _valid = false;
        fsend(RST);
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & ACK) {
        if(_length) {
            _acknowledgment = htonl(acknowledgment() + _length);
            fsend(ACK);
        }

        if(_current->header()->flags() & FIN) {
            process_fin();
            state(TIME_WAIT);
            set_timeout();

            db<TCP>(TRC) << "TCP::Connection::fin_wait2-->time_wait" << endl;
        }
    }
}

void TCP::Connection::close_wait()
{
    db<TCP>(TRC) << "TCP::Connection::close_wait()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!_current->header()->flags() & RST)
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        _valid = false;
        state(CLOSED);
        closed();

        return;
    }

    if(_current->header()->flags() & SYN) {
        _valid = false;
        fsend(RST);
        state(CLOSED);

        return;
    }

    if((_current->header()->flags() & ACK)
        && (_unacknowledged < _current->header()->acknowledgment())
        && (_current->header()->acknowledgment() <= _next)) {
        _unacknowledged = _current->header()->acknowledgment();

        if(_current->header()->flags() & FIN) {
            process_fin();
        }
    }
}

void TCP::Connection::closing()
{
    db<TCP>(TRC) << "TCP::Connection::closing()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        _valid = false;
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        _valid = false;
        fsend(RST);
        state(CLOSED);

        return;
    }

    if((_current->header()->flags() & ACK)
        && (_unacknowledged < _current->header()->acknowledgment())
        && (_current->header()->acknowledgment() <= _next)
        && (_next <= _current->header()->acknowledgment())) { // check if our FIN has been acknowledged
        db<TCP>(TRC) << "TCP::Connection::closing: our FIN has been acknowledged" << endl;
        db<TCP>(TRC) << "TCP::Connection:closing-->time_wait" << endl;

        state(TIME_WAIT);
        set_timeout();
    }
}

void TCP::Connection::last_ack()
{
    db<TCP>(TRC) << "TCP::Connection::last_ack()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        fsend(RST);
        state(CLOSED);

        return;
    }

    if((_current->header()->flags() & ACK)
        && (_unacknowledged < _current->header()->acknowledgment())
        && (_current->header()->acknowledgment() <= _next)
        && (_next <= _current->header()->acknowledgment())) { // check if our FIN has been acknowledged
        db<TCP>(TRC) << "TCP::Connection::last_ack: our FIN has been acknowledged" << endl;

        state(CLOSED);
        _tries = 0;
        _transition.signal();
    }
}

void TCP::Connection::time_wait()
{
    db<TCP>(TRC) << "TCP::Connection::time_wait()" << endl;

    if(!check_sequence()) {
        _valid = false;

        if(!(_current->header()->flags() & RST))
            fsend(ACK);

        return;
    }

    if(_current->header()->flags() & RST) {
        state(CLOSED);

        return;
    }

    if(_current->header()->flags() & SYN) {
        fsend(RST);
        state(CLOSED);

        return;
    }

    if((_current->header()->flags() & ACK)
        && (_unacknowledged < _current->header()->acknowledgment())
        && (_current->header()->acknowledgment() <= _next)
        && (_current->header()->flags() & FIN)) {
        process_fin();
        set_timeout();
    }
}

void TCP::Connection::closed()
{
    // does nothing
}

bool TCP::Connection::check_sequence()
{
//    if (WINDOW == 0) {
//        if (_length) {
//            db<TCP>(TRC) << "TCP::Connection::check_seq() == false: RCV.WND == 0 AND SEG.LEN > 0" << endl;
//            return _valid = false;
//        }
//
//        if (_current->header()->sequence() == acknowledgment())
//            return _valid = true;
//
//        db<TCP>(TRC) << "TCP::Connection::check_seq() == false: RCV.WND == 0 AND SEG.SEQ != RCV.NXT" << endl;
//
//        return _valid = false;
//    }
//
//    if (_length) {
//        if (acknowledgment() <= _current->header()->sequence() && _current->header()->sequence() < (acknowledgment() + WINDOW))
//            return _valid = true;
//
//        db<TCP>(TRC) << "TCP::Connection::check_seq() == false: SEG.LEN > 0 AND !(RCV.NXT <= SEG.SEQ < (RCV.NXT + RCV.WND))" << endl;
//
//        return _valid = false;
//    }
//
//    if ((acknowledgment() <= _current->header()->sequence() && _current->header()->sequence() < (acknowledgment() + WINDOW))
//        || (acknowledgment() <= (_current->header()->sequence() + _length - 1) && (_current->header()->sequence() + _length - 1) < (acknowledgment() + WINDOW)))
//        return _valid = true;
//
//    db<TCP>(TRC) << "TCP::Connection::check_seq() == false" << endl;
//
//    return _valid = false;

    if(WINDOW == 0) {
        if(_length) {
            db<TCP>(TRC) << "TCP::Connection::check_seq() == false: RCV.WND == 0 AND SEG.LEN > 0" << endl;
            return (_valid = false);
        }

        if(_current->header()->sequence() == acknowledgment())
            return (_valid = true);

        db<TCP>(TRC) << "TCP::Connection::check_seq() == false: RCV.WND == 0 AND SEG.SEQ != RCV.NXT" << endl;

        return (_valid = false);
    }

    if(_length) {
        if((acknowledgment() <= _current->header()->sequence()) && (_current->header()->sequence() < (acknowledgment() + WINDOW)))
            return (_valid = true);

        db<TCP>(TRC) << "TCP::Connection::check_seq() == false: SEG.LEN > 0 AND !(RCV.NXT <= SEG.SEQ < (RCV.NXT + RCV.WND))" << endl;

        return (_valid = false);
    }

    if(((acknowledgment() <= _current->header()->sequence()) && (_current->header()->sequence() < (acknowledgment() + WINDOW)))
        || (acknowledgment() <= (_current->header()->sequence() + _length - 1) && (_current->header()->sequence() + _length - 1) < (acknowledgment() + WINDOW)))
        return (_valid = true);

    db<TCP>(TRC) << "TCP::Connection::check_seq() == false" << endl;

    return (_valid = false);
}

void TCP::Connection::process_fin()
{
    db<TCP>(TRC) << "TCP::Connection::process_fin(): FIN received" << endl;

    _acknowledgment = htonl(_current->header()->sequence() + 1);

    fsend(ACK);
}

void TCP::Connection::timeout(Connection* c)
{
    db<TCP>(TRC) << "TCP::Connection::timeout(connection=" << c << ",state=" << c->_state << ")" << endl;

    delete c->_alarm;
    c->_alarm = 0;

    if(c->_state == TIME_WAIT) {
        // TIME-WAIT timeout
        c->state(CLOSED);
        c->_tries = 0;
        c->_transition.signal();
        return;
    }

    if(c->_tries == 3) {
        c->state(CLOSED);
        c->_transition.signal();
        return;
    }

    if(c->_state == FIN_WAIT1 || c->_state == LAST_ACK || c->_state == CLOSING) {
        // close() call timeout
        c->_tries++;
        c->_next--;
        c->fsend(FIN | ACK);
        c->set_timeout(1000 * 1000 * 2.5);
        return;
    }

    if(c->_state == SYN_SENT) {
        // open() call timeout
        c->_tries++;
        c->_next--;
        c->fsend(SYN);
        c->set_timeout();
        return;
    }
}

void TCP::Connection::set_timeout(const Alarm::Microsecond & time)
{
    db<TCP>(TRC) << "TCP::Connection::set_timeout" << endl;

    if(_alarm) {
        delete _alarm;
        _alarm = 0;
    }

    _alarm = new (SYSTEM) Alarm(time, &_timeout_handler);
}

__END_SYS

#endif

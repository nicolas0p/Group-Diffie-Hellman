// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Implementation

#include <machine/pc/machine.h>
#include <machine/pc/pcnet32.h>
#include <utility/malloc.h>
#include <alarm.h>

__BEGIN_SYS

// Class attributes
PCNet32::Device PCNet32::_devices[UNITS];


// Methods
PCNet32::~PCNet32()
{
    db<PCNet32>(TRC) << "~PCNet32(unit=" << _unit << ")" << endl;
}


int PCNet32::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    // Wait for a buffer to become free and seize it
    unsigned int i = _tx_cur;
    for(bool locked = false; !locked; ) {
        for(; _tx_ring[i].status & Tx_Desc::OWN; ++i %= TX_BUFS);
        locked = _tx_buffer[i]->lock();
    }
    _tx_cur = (i + 1) % TX_BUFS; // _tx_cur and _rx_cur are simple accelerators to avoid scanning the ring buffer from the beginning.
                                 // Losing a write in a race condition is assumed to be harmless. The FINC + CAS alternative seems too expensive.
    Tx_Desc * desc = &_tx_ring[i];
    Buffer * buf = _tx_buffer[i];

    db<PCNet32>(TRC) << "PCNet32::send(s=" << _address << ",d=" << dst << ",p=" << hex << prot << dec << ",d=" << data << ",s=" << size << ")" << endl;

    // Assemble the Ethernet frame
    new (buf->frame()) Frame(_address, dst, prot, data, size);

    desc->size = -(size + sizeof(Header)); // 2's comp.

    // Status must be set last, since it can trigger a send
    desc->status = Tx_Desc::OWN | Tx_Desc::STP | Tx_Desc::ENP;

    // Trigger an immediate send poll
    csr(0, csr(0) | CSR0_TDMD);

    _statistics.tx_packets++;
    _statistics.tx_bytes += size;

    // Wait for packet to be sent
    // while(desc->status & Tx_Desc::OWN);

    db<PCNet32>(INF) << "PCNet32::send:desc[" << i << "]=" << desc << " => " << *desc << endl;

    buf->unlock();

    return size;
}


int PCNet32::receive(Address * src, Protocol * prot, void * data, unsigned int size)
{
    db<PCNet32>(TRC) << "PCNet32::receive(s=" << *src << ",p=" << hex << *prot << dec << ",d=" << data << ",s=" << size << ") => " << endl;

    // Wait for a received frame and seize it
    unsigned int i = _rx_cur;
    for(bool locked = false; !locked; ) {
        for(; _rx_ring[i].status & Rx_Desc::OWN; ++i %= RX_BUFS);
        locked = _rx_buffer[i]->lock();
    }
    _rx_cur = (i + 1) % RX_BUFS;
    Buffer * buf = _rx_buffer[i];
    Rx_Desc * desc = &_rx_ring[i];

    // Disassemble the Ethernet frame
    Frame * frame = buf->frame();
    *src = frame->src();
    *prot = frame->prot();

    // For the upper layers, size will represent the size of frame->data<T>()
    buf->size((desc->misc & 0x00000fff) - sizeof(Header) - sizeof(CRC));

    // Copy the data
    memcpy(data, frame->data<void>(), (buf->size() > size) ? size : buf->size());

    // Release the buffer to the NIC
    desc->status = Rx_Desc::OWN;

    _statistics.rx_packets++;
    _statistics.rx_bytes += buf->size();

    db<PCNet32>(INF) << "PCNet32::receive:desc[" << i << "]=" << desc << " => " << *desc << endl;

    int tmp = buf->size();

    buf->unlock();

    return tmp;
}


// Allocated buffers must be sent or release IN ORDER as assumed by the PCNet32
PCNet32::Buffer * PCNet32::alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
{
    db<PCNet32>(TRC) << "PCNet32::alloc(s=" << _address << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",ld=" << payload << ")" << endl;

    int max_data = MTU - always;

    if((payload + once) / max_data > TX_BUFS) {
        db<PCNet32>(WRN) << "PCNet32::alloc: sizeof(Network::Packet::Data) > sizeof(NIC::Frame::Data) * TX_BUFS!" << endl;
        return 0;
    }

    Buffer::List pool;

    // Calculate how many frames are needed to hold the transport PDU and allocate enough buffers
    for(int size = once + payload; size > 0; size -= max_data) {
        // Wait for the next buffer to become free and seize it
        unsigned int i = _tx_cur;
        for(bool locked = false; !locked; ) {
            for(; _tx_ring[i].status & Tx_Desc::OWN; ++i %= TX_BUFS);
            locked = _tx_buffer[i]->lock();
        }
        _tx_cur = (i + 1) % TX_BUFS;
        Tx_Desc * desc = &_tx_ring[i];
        Buffer * buf = _tx_buffer[i];

        // Initialize the buffer and assemble the Ethernet Frame Header
        new (buf) Buffer(nic, (size > max_data) ? MTU : size + always, _address, dst, prot);

        db<PCNet32>(INF) << "PCNet32::alloc:desc[" << i << "]=" << desc << " => " << *desc << endl;
        db<PCNet32>(INF) << "PCNet32::alloc:buf=" << buf << " => " << *buf << endl;

        pool.insert(buf->link());
    }

    return pool.head()->object();
}


int PCNet32::send(Buffer * buf)
{
    unsigned int size = 0;

    for(Buffer::Element * el = buf->link(); el; el = el->next()) {
        buf = el->object();
        Tx_Desc * desc = reinterpret_cast<Tx_Desc *>(buf->back());

        db<PCNet32>(TRC) << "PCNet32::send(buf=" << buf << ")" << endl;

        db<PCNet32>(INF) << "PCNet32::send:buf=" << buf << " => " << *buf << endl;

        desc->size = -(buf->size() + sizeof(Header)); // 2's comp.

        // Status must be set last, since it can trigger a send
        desc->status = Tx_Desc::OWN | Tx_Desc::STP | Tx_Desc::ENP;

        // Trigger an immediate send poll
        csr(0, csr(0) | CSR0_TDMD);

        size += buf->size();

        _statistics.tx_packets++;
        _statistics.tx_bytes += buf->size();

        db<PCNet32>(INF) << "PCNet32::send:desc=" << desc << " => " << *desc << endl;

        // Wait for packet to be sent and unlock the respective buffer
        while(desc->status & Tx_Desc::OWN);
        buf->unlock();
    }

    return size;
}


void PCNet32::free(Buffer * buf)
{
    db<PCNet32>(TRC) << "PCNet32::free(buf=" << buf << ")" << endl;

    db<PCNet32>(INF) << "PCNet32::free:buf=" << buf << " => " << *buf << endl;

    for(Buffer::Element * el = buf->link(); el; el = el->next()) {
        buf = el->object();
        Rx_Desc * desc = reinterpret_cast<Rx_Desc *>(buf->back());

        _statistics.rx_packets++;
        _statistics.rx_bytes += buf->size();

        // Release the buffer to the NIC
        desc->size = Reg16(-sizeof(Frame)); // 2's comp.
        desc->status = Rx_Desc::OWN; // Owned by NIC

        // Release the buffer to the OS
        buf->unlock();

        db<PCNet32>(INF) << "PCNet32::free:desc=" << desc << " => " << *desc << endl;
    }
}


void PCNet32::reset()
{
    db<PCNet32>(TRC) << "PCNet32::reset()" << endl;

    // Reset the device
    s_reset();

    // Software style => PCI, 32 bits, burst mode (pg 147)
    bcr(20, BCR20_SSIZE32 | BCR20_SWSTYLE2);

    // Get MAC address from PROM
    _address[0] = prom(0);
    _address[1] = prom(1);
    _address[2] = prom(2);
    _address[3] = prom(3);
    _address[4] = prom(4);
    _address[5] = prom(5);
    db<PCNet32>(INF) << "PCNet32::reset: MAC=" << _address << endl;

    // Enable auto-select port
    bcr(2, BCR2_ASEL);

    // Enable full-duplex
    bcr(9, BCR9_FDEN);

    // Disable INIT interrupt and transmit stop on underflow and two part deferral
    csr(3, CSR3_TINTM | CSR3_IDONM | CSR3_DXMT2PD | CSR3_LAPPEN | CSR3_DXSUFLO);

    // Enable frame auto padding/stripping and auto CRC handling
    csr(4, CSR4_DMAPLUS | CSR4_DPOLL | CSR4_APAD_XMT | CSR4_ASTRP_RCV | CSR4_TXSTRTM);

    // Adjust interrupts
    csr(5, CSR5_TOKINTD | CSR5_SINTE | CSR5_EXDINTE);

    // Enable burst read and write
    bcr(18, bcr(18) | BCR18_BREADE | BCR18_BWRITE);

    // Promiscuous mode
//    if(promiscuous)
//        csr(15, csr(15) | CSR15_PROM);

    // Set transmit start point to full frame
    csr(80, csr(80) | 0x0c00); // XMTSP = 11

    // Setup a init block
    _iblock->mode = 0x0000;
    _iblock->rlen = log2(RX_BUFS) << 4;
    _iblock->tlen = log2(TX_BUFS) << 4;
    _iblock->mac_addr = _address;
    _iblock->filter1 = 0;
    _iblock->filter2 = 0;
    _iblock->rx_ring = _rx_ring_phy;
    _iblock->tx_ring = _tx_ring_phy;
    csr(1, _iblock_phy & 0xffff);
    csr(2, _iblock_phy >> 16);

    // Initialize the device
    csr(0, CSR0_IENA | CSR0_INIT);
    for(int i = 0; (i < 100) && !(csr(0) & CSR0_IDON); i++);
    if(!(csr(0) & CSR0_IDON))
        db<PCNet32>(WRN) << "PCNet32::reset: initialization failed!" << endl;

    // Get MAC address from CSR
    csr(0, CSR0_IDON | CSR0_STOP);
    Address csr_addr;
    csr_addr[0] = csr(PADR0) & 0x00ff;
    csr_addr[1] = (csr(PADR0) & 0xff00) >> 8;
    csr_addr[2] = csr(PADR1) & 0x00ff;
    csr_addr[3] = (csr(PADR1) & 0xff00) >> 8;
    csr_addr[4] = csr(PADR2) & 0x00ff;
    csr_addr[5] = (csr(PADR2) & 0xff00) >> 8;

    if(_address != csr_addr) {
        db<PCNet32>(WRN) << "PCNet32::reset: initialization failed!" << endl;
        db<PCNet32>(WRN) << "PCNet32::reset: MAC(ROM)=" << _address << endl;
        db<PCNet32>(WRN) << "PCNet32::reset: MAC(CSR)=" << csr_addr << endl;
    }

    // Activate sending and receiving
    csr(0, CSR0_IENA | CSR0_STRT);

    // Reset statistics
    new (&_statistics) Statistics;
}


void PCNet32::handle_int()
{
    if(csr(0) & CSR0_INTR) {
        int csr0 = csr(0);
        int csr4 = csr(4);
        int csr5 = csr(5);

        // Clear interrupts (i.e. acknowledge them)
        csr(0, csr0);
        csr(4, csr4);
        csr(5, csr5);

        if(csr0 & CSR0_IDON) { // Initialization done
            // This should never happen, since IDON is disabled in reset()
            // and all the initialization is conctrolled via polling, so if
            // we are here, it must be due to a hardware induced reset.
            // All we can do is to try to reset the nic!
            db<PCNet32>(WRN) << "PCNet32::handle_int: initialization done!" << endl;
            reset();
        }

        if(csr0 & CSR0_RINT) { // Frame received (possibly multiple, let's handle a whole round on the ring buffer)

            // Note that ISRs in EPOS are reentrant, that's why locking was carefully made atomic
            // Therefore, several instances of this code can compete to handle received buffers

            for(unsigned int count = RX_BUFS, i = _rx_cur; count && !(_rx_ring[i].status & Rx_Desc::OWN); count--, ++i %= RX_BUFS, _rx_cur = i) {
                // NIC received a frame in _rx_buffer[_rx_cur], let's check if it has already been handled
                if(_rx_buffer[i]->lock()) { // if it wasn't, let's handle it
                    Buffer * buf = _rx_buffer[i];
                    Rx_Desc * desc = &_rx_ring[i];
                    Frame * frame = buf->frame();

                    // For the upper layers, size will represent the size of frame->data<T>()
                    buf->size((desc->misc & 0x00000fff) - sizeof(Header) - sizeof(CRC));

                    db<PCNet32>(TRC) << "PCNet32::int:receive(s=" << frame->src() << ",p=" << hex << frame->header()->prot() << dec
                                     << ",d=" << frame->data<void>() << ",s=" << buf->size() << ")" << endl;

                    db<PCNet32>(INF) << "PCNet32::handle_int:desc[" << i << "]=" << desc << " => " << *desc << endl;

                    IC::disable(IC::irq2int(_irq));
                    if(!notify(frame->header()->prot(), buf)) // No one was waiting for this frame, so let it free for receive()
                        free(buf);
                    // TODO: this serialization is much too restrictive. It was done this way for students to play with
                    IC::enable(IC::irq2int(_irq));
                }
            }
 	}

        if(csr0 & CSR0_ERR) { // Error
            db<PCNet32>(WRN) << "PCNet32::int:error =>";

            if(csr0 & CSR0_MERR) { // Memory
        	db<PCNet32>(WRN) << " memory";
            }

            if(csr0 & CSR0_MISS) { // Missed Frame
        	db<PCNet32>(WRN) << " missed frame";
        	_statistics.rx_overruns++;
            }

            if(csr0 & CSR0_CERR) { // Collision
        	db<PCNet32>(WRN) << " collision";
        	_statistics.collisions++;
            }

            if(csr0 & CSR0_BABL) { // Bable transmitter time-out
        	db<PCNet32>(WRN) << " overrun";
        	_statistics.tx_overruns++;
            }

            db<PCNet32>(WRN) << endl;
        }
    }
}


void PCNet32::int_handler(const IC::Interrupt_Id & interrupt)
{
    PCNet32 * dev = get_by_interrupt(interrupt);

    db<PCNet32>(TRC) << "PCNet32::int_handler(int=" << interrupt << ",dev=" << dev << ")" << endl;

    if(!dev)
        db<PCNet32>(WRN) << "PCNet32::int_handler: handler not assigned!" << endl;
    else
        dev->handle_int();
}

__END_SYS

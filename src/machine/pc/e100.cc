// EPOS PC Intel PRO/100 (i82559) Ethernet NIC Mediator Implementation

#include <machine/pc/machine.h>
#include <machine/pc/e100.h>
#include <task.h>

__BEGIN_SYS

/// i8255x ---------------------------------------------------------------------
int i8255x::exec_command(Reg8 cmd, Reg32 dma_addr)
{
    unsigned int i;

    // previous command is accepted when SCB clears
    for(i = 0; i < i82559_WAIT_SCB_TIMEOUT; i++) {
        if(!read8(&_csr->scb.cmd_lo))
            break;
        udelay(5);
        if(i > i82559_WAIT_SCB_FAST)
            udelay(10);
    }
    if(cmd != cuc_resume && cmd != ruc_resume)
        write32(dma_addr, &_csr->scb.gen_ptr);

    if(i == i82559_WAIT_SCB_TIMEOUT)
        return 1;

    write8(cmd, &_csr->scb.cmd_lo);

    return 0;
}

/// i82559ER -------------------------------------------------------------------

/// i82559c --------------------------------------------------------------------

/// E100 -----------------------------------------------------------------------

// Class attributes
E100::Device E100::_devices[UNITS];

// Class Methods
void E100::int_handler(const IC::Interrupt_Id & interrupt)
{
    db<E100>(TRC) << "E100::int_handler" << endl;

    E100 * dev = get_by_interrupt(interrupt);

    db<E100>(TRC) << "E100::int_handler(int=" << interrupt << ",dev=" << dev << ")" << endl;

    if (HYSTERICALLY_DEBUGGED && dev) {
        dev->print_status();
    }

    if(!dev)
        db<E100>(WRN) << "E100::int_handler: handler not found" << endl;
    else
        dev->handle_int();
}

// Methods
E100::~E100()
{
    db<E100>(TRC) << "~E100(unit=" << _unit << ")" << endl;
}

E100::E100(unsigned int unit, const Log_Addr & io_mem, const IO_Irq & irq, DMA_Buffer * dma_buf)
{
    db<E100>(TRC) << "E100(unit=" << unit << ",io=" << io_mem << ",irq=" << irq << ",dma=" << dma_buf << ")" << endl;

    _unit = unit;
    _io_mem = io_mem;
    _irq = irq;
    _csr = static_cast<CSR_Desc *>(io_mem);
    _dma_buffer = dma_buf;

    // Distribute the DMA_Buffer allocated by init()
    Log_Addr log = dma_buf->log_address();
    Phy_Addr phy = dma_buf->phy_address();

    // Initialization Block
    _configCB_phy = phy;
    configCB = log;
    log += align128(sizeof(ConfigureCB));
    phy += align128(sizeof(ConfigureCB));

    _macAddrCB_phy = phy;
    macAddrCB = log;
    log += align128(sizeof(macAddrCB));
    phy += align128(sizeof(macAddrCB));

    _dmadump_phy = phy;
    dmadump = log;
    log += align128(sizeof(struct mem));
    phy += align128(sizeof(struct mem));

    // Rx_Desc Ring
    _rx_cur = 0;
    _rx_last_el = RX_BUFS - 1;
    _rx_ring = log;
    _rx_ring_phy = phy;

    db<E100> (TRC) << "E100(_rx_ring malloc of " << RX_BUFS << " units)" << endl;

    // Rx (RFDs)
    unsigned int i;
    for(i = 0; i < RX_BUFS; i++) {
        log += align128(sizeof(Rx_Desc));
        phy += align128(sizeof(Rx_Desc));
        _rx_ring[i].command = 0;
        _rx_ring[i].size = sizeof(Frame);
        _rx_ring[i].status = Rx_RFD_AVAILABLE;
        _rx_ring[i].actual_count = 0;
        _rx_ring[i].link = phy; //next RFD
    }
    _rx_ring[i-1].command = cb_el;
    _rx_ring[i-1].link = _rx_ring_phy;

    // Tx_Desc Ring
    _tx_cur = 1;
    _tx_prev = 0;
    _tx_ring = log;
    _tx_ring_phy = phy;

    db<E100> (TRC) << "E100(_tx_ring malloc of " << TX_BUFS << " units)" << endl;
    // TxCBs
    for(i = 0; i < TX_BUFS; i++) {
        log += align128(sizeof(Tx_Desc));
        phy += align128(sizeof(Tx_Desc));

        new (&_tx_ring[i]) Tx_Desc(phy);
    }
    _tx_ring[i-1].link = _tx_ring_phy;


    // Rx Buffer
    for(i = 0; i < RX_BUFS; i++) {
        _rx_buffer[i] = new (log) Buffer(&_rx_ring[i]);

        log += align128(sizeof(Buffer));
        phy += align128(sizeof(Buffer));
    }

    // Tx Buffer
    for(i = 0; i < TX_BUFS; i++) {
        _tx_buffer[i] = new (log) Buffer(&_tx_ring[i]);

        log += align128(sizeof(Buffer));
        phy += align128(sizeof(Buffer));
    }

    _tx_buffer_prev = _tx_buffer[_tx_prev];

    // reset
    reset();
}

void E100::reset()
{
    db<E100>(TRC) << "E100::reset (software reset and self-test)" << endl;

    // Reset the device
    software_reset();

    // Do a self-test on the device
    self_test();

    // Get MAC address from EEPROM
    _address[0] = eeprom_mac_address(0);
    _address[1] = eeprom_mac_address(1);
    _address[2] = eeprom_mac_address(2);
    _address[3] = eeprom_mac_address(3);
    _address[4] = eeprom_mac_address(4);
    _address[5] = eeprom_mac_address(5);
    db<E100>(INF) << "E100::reset():MAC=" << _address << endl;

    // load zero on NIC's internal CU
    while(exec_command(cuc_load_base, 0));
    // load zero on NIC's internal RU
    while(exec_command(ruc_load_base, 0));

    // set new configuration
    i82559_configure();
    configCB->command &= ~cb_s;
    configCB->link = _macAddrCB_phy;

    // set MAC address
    macAddrCB->command = cb_iaaddr;
    macAddrCB->command &= ~cb_s;
    macAddrCB->command |= cb_el;
    macAddrCB->iaaddr = _address;

    i82559_disable_irq();

    while(exec_command(cuc_start, _configCB_phy));
    udelay(2 * 1000);

    while (!(macAddrCB->status & cb_complete));

    _statistics.tx_packets = 0;
    _statistics.rx_bytes = 0;
    _rx_ruc_no_more_resources = 0;
    _tx_cuc_suspended = 1;
    _tx_frames_sent = 0;

    // start send unit
    while(exec_command(cuc_start, _tx_ring_phy));
    udelay(10);

    // start receive unit
    while(exec_command(ruc_start, _rx_ring_phy));
    udelay(10);

    i82559_enable_irq();
    //i82559_disable_irq();
    udelay(10);
}

int E100::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    // wait for a free TxCB
    while(!(_tx_ring[_tx_cur].status & cb_complete)) {
        if (_tx_cuc_suspended) {
            //_int_lock.acquire();
            _tx_cuc_suspended = 0;
            while(exec_command(cuc_resume, 0));
            //_int_lock.release();
        }
    }

    _tx_ring[_tx_cur].status = Tx_CB_IN_USE;
    _tx_ring[_tx_cur].tcb_byte_count = (size + HEADER_SIZE);

    // put the ethernet frame into de TxCB
    // new (_tx_ring[_tx_cur].frame) Frame(_address, dst, htons(prot), data, size); // original
    /* New implementation, double copy: parameters to buffer and buffer to Tx_Desc.frame.
     * Making Tx_Desc.frame a pointer does not work (I believe E100 demands for
     * a contiguous allocated memory for Tx ring descriptors).
     */
    Buffer * buf = _tx_buffer[_tx_cur];
    Frame * frame = buf->frame();
    new (frame) Frame(_address, dst, htons(prot), data, size);
    new (_tx_ring[_tx_cur].frame()) Frame(_address, frame->dst(), htons(frame->prot()), frame->data<void>(), size);
    // ----
    // TODO: the above doesn't make sense!

    db<E100>(TRC) << "E100::send(dst=" << dst << ", prot=" << prot << ", data=" << (char *) data << ", size=" << size << ")";
    db<E100>(INF) << "E100::send:frame={dst=" << frame->dst() << ", prot=" << frame->prot() << ", data=" << (char *) frame->data<void>() << ", size=" << buf->size() << "}" << endl;

    _tx_ring[_tx_cur].command = cb_s; // suspend bit
    _tx_ring[_tx_cur].command |= (cb_tx | cb_cid); // transmit command

    _tx_ring[_tx_prev].command &= ~cb_s; // remove suspend bit
    ++_tx_prev %= TX_BUFS;

    // we have no guarantee that this command will be accepted by the adapter
    exec_command(cuc_resume, 0);

    ++_tx_cur %= TX_BUFS;

    _statistics.tx_packets++;
    _statistics.tx_bytes += size;

    return size;
}

bool E100::verifyPendingInterrupts(void)
{
    if (_rx_ruc_no_more_resources) {
        unsigned short i;

        //_int_lock.acquire();

        _rx_ruc_no_more_resources--;
        for (i = (_rx_cur + 1) % RX_BUFS; i != _rx_cur; ++i %= RX_BUFS) {
            if ((_rx_ring[i].status & cb_complete)) break;
        }

        if (i) --i;
        else i = RX_BUFS - 1;

        if (i != _rx_last_el) {
            _rx_ring[i].command = cb_el;
            _rx_ring[_rx_last_el].command &= ~cb_el;
            _rx_last_el = i;
        }

        while(exec_command(ruc_start, _rx_ring_phy + _rx_cur * sizeof(Rx_Desc)));

        //_int_lock.release();

        return true;
    }

    return false;
}

int E100::receive(Address * src, Protocol * prot, void * data, unsigned int size)
{
    // wait until the RFD is filled up by the device
    while(!(_rx_ring[_rx_cur].status & cb_complete))
        verifyPendingInterrupts();

    // fill up receive areas/variables
    Frame * frame = (Frame *)_rx_ring[_rx_cur].frame;
    *src = frame->header()->src();
    *prot = ntohs(frame->header()->prot());

    if (_rx_ring[_rx_cur].actual_count & (RFD_EOF_MASK | RFD_F_MASK))
        size = _rx_ring[_rx_cur].actual_count & RFD_ACTUAL_COUNT_MASK;
    else
        size = 0;

    if (! (_rx_ring[_rx_cur].status & RFD_OK_MASK))
        db<E100>(WRN) << "Error on frame reception" << endl;

    memcpy(data, frame->data<void>(), size);

    //kout << "recv (size: " << size << " prot: " << *prot <<")" << endl;

    _rx_ring[_rx_cur].command = cb_el;
    _rx_ring[_rx_cur].status = Rx_RFD_NOT_FILLED;

    // try to avoid ruc stop interrupts by "walking" the el bit
    _rx_ring[_rx_last_el].command &= ~cb_el; // remove previous el bit
    _rx_last_el = _rx_cur;

    ++_rx_cur %= RX_BUFS;

    _statistics.rx_packets++;
    _statistics.rx_bytes += size;

    return size;
}

/*! NOTE: this method is not thread-safe as _tx_cur is shared by all threads
 * that use this object and the access to _tx_cur is not atomic. */
// TODO: solve this!
E100::Buffer * E100::alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload)
{
    db<E100>(TRC) << "E100::alloc(s=" << _address << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",ld=" << payload << ")" << endl;

    int max_data = MTU - always;

    if((payload + once) / max_data > TX_BUFS) {
        db<E100>(WRN) << "E100::alloc: sizeof(Network::Packet::Data) > sizeof(NIC::Frame::Data) * TX_BUFS!" << endl;
        return 0;
    }

    Buffer::List pool;

    // Calculate how many frames are needed to hold the transport PDU and allocate enough buffers
    for(int size = once + payload; size > 0; size -= max_data) {
        // Wait for the next buffer to become free and seize it
        for(bool locked = false; !locked; ) {
            for(; !(_tx_ring[_tx_cur].status & cb_complete); ++_tx_cur %= TX_BUFS);
            locked = _tx_buffer[_tx_cur]->lock();
        }
        Tx_Desc * desc = &_tx_ring[_tx_cur];
        Buffer * buf = _tx_buffer[_tx_cur];

        // Initialize the buffer and assemble the Ethernet Frame Header
        new (buf) Buffer(nic, (size > max_data) ? MTU : size + always, _address, dst, prot);

        db<E100>(INF) << "E100::alloc:desc[" << _tx_cur << "]=" << desc << " => " << *desc << endl;

        ++_tx_cur %= TX_BUFS;

        pool.insert(buf->link());
    }

    return pool.head()->object();
}

void E100::free(Buffer * buf)
{
    db<E100>(TRC) << "E100::free(buf=" << buf << ")" << endl;

    for(Buffer::Element * el = buf->link(); el; el = el->next()) {
        buf = el->object();
        Rx_Desc * desc = reinterpret_cast<Rx_Desc *>(buf->back());

        _statistics.rx_packets++;
        _statistics.rx_bytes += buf->size();

        // Release the buffer to the NIC
        desc->size = Reg16(-sizeof(Frame)); // 2's comp.
        desc->status = Rx_RFD_AVAILABLE; // Owned by NIC
        desc->actual_count = 0x0; // Clears EOF, F, and Actual Count

        // Release the buffer to the OS
        buf->unlock();

        db<E100>(INF) << "E100::free:desc=" << desc << " => " << *desc << endl;
    }
}

/*! NOTE: this method is not thread-safe because _tx_buffer_prev is shared by
 * all threads that use this object. */
int E100::send(Buffer * buf)
{
    db<E100>(TRC) << "E100::send new api" << endl;
    /// assumes: buf->is_locked();

    unsigned int size = 0;

    for(Ethernet::Buffer::Element * el = buf->link(); el; el = el->next()) {
        buf = el->object();
        Tx_Desc * desc = reinterpret_cast<Tx_Desc *>(buf->back());
        Ethernet::Frame * frame = buf->frame();

        db<E100>(TRC) << "E100::send(buf=" << buf << ")" << endl;

        desc->tcb_byte_count = buf->size() + sizeof(Ethernet::Header);

        db<E100>(TRC) << "E100::send-zc:\n" << "(dst=" << frame->dst() << ", prot=" << frame->prot() << ", data=" << (char *) frame->data<void>() << ", size=" << buf->size() << ")" << endl;

        new (desc->frame()) Ethernet::Frame(_address, frame->dst(), frame->prot(), frame->data<void>(), buf->size()); // TODO: FIXME. That is creating a copy on a Zero-copy implementation. :P

        // Status must be set last, since it can trigger a send
        desc->status = Tx_CB_IN_USE;

        // Trigger an immediate send poll
        desc->command = cb_s; // suspend bit
        desc->command |= (cb_tx | cb_cid); // transmit command
        reinterpret_cast<Tx_Desc *>(_tx_buffer_prev->back())->command &= ~cb_s; // remove suspend bit of the previous frame

        while(exec_command(cuc_resume, 0));

        size += buf->size();

        _statistics.tx_packets++;
        _statistics.tx_bytes += buf->size();

        db<E100>(INF) << "E100::send:desc=" << desc << " => " << *desc << endl;

        // Wait for packet to be sent and unlock the respective buffer
        while(! (desc->status & cb_complete)) {
            if (_tx_cuc_suspended) {
                _tx_cuc_suspended = 0;
                exec_command(cuc_resume, 0);
            }
        }

        db<E100>(TRC) << "E100::send will unlock" << endl;

        _tx_buffer_prev = buf;
        buf->unlock();

        db<E100>(TRC) << "E100::send unlocked" << endl;
    }

    db<E100>(TRC) << "E100::send size=" << size << endl;

    return size;
}

unsigned short E100::eeprom_read(unsigned short *addr_len, unsigned short addr) {

    unsigned long cmd_addr_data;
    cmd_addr_data = (EE_READ_CMD(*addr_len) | addr) << 16;

    _csr->eeprom_ctrl_lo = eecs | eesk;
    i82559_flush();
    udelay(200);

    unsigned short data = 0;
    unsigned char ctrl;
    for (int i = 31; i >= 0; i--) {
        ctrl = (cmd_addr_data & (1 << i)) ? eecs | eedi : eecs;
        _csr->eeprom_ctrl_lo = ctrl;
        i82559_flush();
        udelay(200);

        _csr->eeprom_ctrl_lo = ctrl | eesk;
        i82559_flush();
        udelay(200);

        ctrl = _csr->eeprom_ctrl_lo;
        if (!(ctrl & eedo) && i > 16) {
            *addr_len -= (i - 16);
            i = 17;
        }

        data = (data << 1) | (ctrl & eedo ? 1 : 0);
    }

    _csr->eeprom_ctrl_lo = 0;
    i82559_flush();
    udelay(200);

    return data;
}

unsigned char E100::eeprom_mac_address(Reg16 addr) {
    Reg16 addr_len = 8;
    Reg16 two_words; // two words of 8 bits (one EEPROM word)
    Reg8 which_word; // first or second word of 8 bits

    // try reading with an 8-bit addr len to discover actual addr len
    eeprom_read(&addr_len, 0);

    which_word = addr % 2; // read the first (0) or second (1) word of 8 bits

    addr = (unsigned short) (addr / 2);
    two_words = eeprom_read(&addr_len, addr);

    if (which_word != 0) // second word (bits 8 to 15)
        return (two_words >> 8);
    else // first word (bits 0 to 7)
        return (two_words & 0x00FF);
}

void E100::handle_int() {
    CPU::int_disable();
    // IC::disable(IC::irq2int(_irq));

    db<E100>(TRC) << "E100::handle_int()" << endl;
    db<E100>(TRC) << ">" << endl;

    Reg8 stat_ack = read8(&_csr->scb.stat_ack);
    Reg8 status = read8(&_csr->scb.status);

    if ((stat_ack != NOT_OURS) && (stat_ack != NOT_PRESENT)) {
        db<E100>(TRC) << "...if" << endl;

        // acknowledge interrupt(s) in one PCI write cycle
        write8((stat_ack & ~SWI), &_csr->scb.stat_ack); /* Writing 1 back to a
            * STAT/ACK bit that was set will acknowledge that particular
            * interrupt bit.
            * The device will de-assert its interrupt line only when all pending
            * interrupt STAT bits are acknowledged.
            * In this case, acknowledging all interrupts at once, except the
            * SWI interrupt.
            */

        if ((((status & RUS_MASK) >> RUS_SHIFT) == RUS_NO_RESOURCES) && (stat_ack & RNR)) {
            _rx_ruc_no_more_resources++;
        }

        for(int count = RX_BUFS; count && (_rx_ring[_rx_cur].status & cb_complete); count--, ++_rx_cur %= RX_BUFS) {
            db<E100>(TRC) << "@ count = " << count << ", _rx_cur = " << _rx_cur << endl;

            // NIC received a frame in _rx_buffer[_rx_cur], let's check if it has already been handled
            if(_rx_buffer[_rx_cur]->lock()) { // if it wasn't, let's handle it
                Buffer * buf = _rx_buffer[_rx_cur];
                Rx_Desc * desc = &_rx_ring[_rx_cur];
                Frame * frame = buf->frame();

                Frame * desc_frame = reinterpret_cast<Frame *>(desc->frame);

                // For the upper layers, size will represent the size of frame->data<T>()
                unsigned int size = 0;
                if (_rx_ring[_rx_cur].actual_count & (RFD_EOF_MASK | RFD_F_MASK)) {
                    size = _rx_ring[_rx_cur].actual_count & RFD_ACTUAL_COUNT_MASK;
                }
                else if (_rx_ring[_rx_cur].actual_count & RFD_F_MASK) {
                    db<E100>(WRN) << "HDS size" << endl;
                }
                else if (! (_rx_ring[_rx_cur].actual_count & RFD_F_MASK)) {
                    db<E100>(WRN) << "Invalid RFD" << endl;
                    // Workaround if QEMU patch not applied
                    // http://patchwork.ozlabs.org/patch/662355/
                    db<E100>(WRN) << "Assuming size to be 1500" << endl;
                    size = 1500;
                    // ----
                }
                buf->size(size);

                if (! (_rx_ring[_rx_cur].status & RFD_OK_MASK))
                    db<E100>(WRN) << "Error on frame reception" << endl;

                db<E100>(INF) << "E100::int:receive desc_frame(s=" << desc_frame->src() << ",d=" << desc_frame->dst() << ",p=" << hex << desc_frame->prot() << dec << ",t=" << (char *) desc_frame->data<void>() << ",s=" << buf->size() << ")" << endl;

                new (frame) Frame(desc_frame->src(), desc_frame->dst(), desc_frame->prot(), desc_frame->data<void>(), buf->size()); // TODO: FIXME. That is creating a copy on a Zero-copy implementation. :P

                db<E100>(INF) << "E100::int:receive(s=" << frame->src() << ",d=" << frame->dst() << ",p=" << hex << frame->header()->prot() << dec << ",t=" << (char *) frame->data<void>() << ",s=" << buf->size() << ")" << endl;

                db<E100>(INF) << "E100::handle_int:desc[" << _rx_cur << "]=" << desc << " => " << *desc << endl;

                _rx_ring[_rx_cur].command = cb_el;
                _rx_ring[_rx_cur].status = Rx_RFD_NOT_FILLED;

                // try to avoid ruc stop interrupts by "walking" the el bit
                _rx_ring[_rx_last_el].command &= ~cb_el; // remove previous el bit
                _rx_last_el = _rx_cur;

                _statistics.rx_packets++;
                _statistics.rx_bytes += size;

                db<E100>(TRC) << "Will notify!" << endl;
                if(!notify(frame->header()->prot(), buf)) { // No one was waiting for this frame, so let it free for receive()
                    free(buf);
                    db<E100>(TRC) << "Not notified!" << endl;
                }
                else {
                    db<E100>(TRC) << "Notified!" << endl;
                }
            }
        }
    }

    db<E100>(TRC) << "<" << endl;

    CPU::int_enable();
    // IC::enable(IC::irq2int(_irq));
}

void E100::i82559_configure(void)
{
    configCB->command = cb_config;
    memset(&(configCB->config), 0, sizeof(struct config));
    configCB->config.byte_count = 0x16;              /* bytes in this struct */
    configCB->config.rx_fifo_limit = 0x8;            /* bytes in FIFO before DMA */
    configCB->config.direct_rx_dma = 0x1;            /* reserved */
    configCB->config.standard_tcb = 0x1;             /* 1=standard, 0=extended */
    configCB->config.standard_stat_counter = 0x1;    /* 1=standard, 0=extended */
    // zero => recommended in promiscuous mode - FIXME - padding is enabled
    configCB->config.rx_discard_short_frames = 0x0;  /* 1=discard, 0=pass */
    configCB->config.tx_underrun_retry = 0x3;        /* 3 underrun retries */
    configCB->config.tx_dynamic_tbd = 0x0;           /* 1=yes, 0=no FIXME */
    configCB->config.mii_mode = 0x1;                 /* 1=MII mode, 0=503 mode */
    configCB->config.rx_tcpudp_checksum = 0x0;       /* 1=yes 0=no */
    configCB->config.link_status_wake = 0x1;         /* 1=yes 0=no */
    configCB->config.pad10 = 0x6;
    // if you comment the next line it won't work anymore
    configCB->config.no_source_addr_insertion = 0x1; /* 1=no, 0=yes */
    configCB->config.preamble_length = 0x2;          /* 0=1, 1=3, 2=7, 3=15 bytes */
    configCB->config.ifs = 0x6;                      /* x16 = inter frame spacing */
    configCB->config.ip_addr_hi = 0xF2;              /* ARP IP filter - not used */
    configCB->config.pad15_1 = 0x1;
    configCB->config.pad15_2 = 0x1;
    configCB->config.crs_or_cdt = 0x0;               /* 0=CRS only, 1=CRS or CDT */
    //configCB->config.fc_delay_hi = 0x40;           /* time delay for fc frame */
    configCB->config.rx_stripping = 0x1;             /* 1=strip long frames */
    configCB->config.tx_padding = 0x1;               /* 1=pad short frames */
    configCB->config.fc_priority_threshold = 0x7;    /* 7=priority fc disabled */
    configCB->config.pad18 = 0x1;
    configCB->config.pad20_1 = 0x1F;
    configCB->config.fc_priority_location = 0x1;     /* 1=byte#31, 0=byte#19 */
    configCB->config.multi_ia = 0x1;
    configCB->config.pad21_1 = 0x5;
    configCB->config.full_duplex_pin = 0x1;          /* 1=examine FDX# pin */
    configCB->config.full_duplex_force = 0x0;        /* 1=force, 0=auto */
    // specially for promiscuous mode
    configCB->config.rx_save_bad_frames = 0x1;       /* 1=save, 0=discard */
    configCB->config.pad12_0 = 0x1;                  /* 1=yes, 0=no */
    configCB->config.promiscuous_mode = 0x1;         /* 1=on, 0=off */
    configCB->config.wait_after_win = 0x1;           /* 1=on, 0=off */
    configCB->config.multicast_all = 0x0;            /* 1=accept, 0=no */
    configCB->config.magic_packet_disable = 0x0;     /* 1=off, 0=on */
    configCB->config.fc_disable = 0x0;               /* 1=Tx fc off, 0=Tx fc on */
    configCB->config.mwi_enable = 0x1;               /* 1=enable, 0=disable */
    configCB->config.rx_long_ok = 0x0;               /* 1=VLANs ok, 0=standard */
    configCB->config.tco_statistics = 0x1;           /* TCO stats enable */
}

int E100::self_test()
{
    Reg32 dma_addr = _dmadump_phy + offsetof(struct mem, selftest);

    dmadump->selftest.signature = 0;
    dmadump->selftest.result = 0xFFFFFFFF;

    write32(SELFTEST | dma_addr, &_csr->port);
    i82559_flush();
    udelay(20 * 1000); // wait for 10 miliseconds

    i82559_disable_irq();

    // Check results of self-test
    if(dmadump->selftest.result != 0) {
        db<E100>(WRN) << "E100:self_test() => failed with code " << dmadump->selftest.result << "!" << endl;
        return -1;
    } else {
        db<E100>(TRC) << "E100:self_test() => PASSED with code " << dmadump->selftest.result << "!" << endl;
    }

    if(dmadump->selftest.signature == 0) {
        db<E100>(WRN) << "E100:self_test() => timed out!" << endl;
        return -1;
    }

    return 0;
}


__END_SYS

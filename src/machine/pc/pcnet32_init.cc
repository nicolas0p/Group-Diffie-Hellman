// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Initialization

#include <system.h>
#include <machine/pc/machine.h>
#include <machine/pc/pcnet32.h>

__BEGIN_SYS

PCNet32::PCNet32(unsigned int unit, IO_Port io_port, IO_Irq irq, DMA_Buffer * dma_buf)
{
    db<PCNet32>(TRC) << "PCNet32(unit=" << unit << ",io=" << io_port << ",irq=" << irq << ",dma=" << dma_buf << ")" << endl;

    _unit = unit;
    _io_port = io_port;
    _irq = irq;
    _dma_buf = dma_buf;

    // Distribute the DMA_Buffer allocated by init()
    Log_Addr log = _dma_buf->log_address();
    Phy_Addr phy = _dma_buf->phy_address();

    // Initialization Block
    _iblock = log;
    _iblock_phy = phy;
    log += align128(sizeof(Init_Block));
    phy += align128(sizeof(Init_Block));

    // Rx_Desc Ring
    _rx_cur = 0;
    _rx_ring = log;
    _rx_ring_phy = phy;
    log += RX_BUFS * align128(sizeof(Rx_Desc));
    phy += RX_BUFS * align128(sizeof(Rx_Desc));

    // Tx_Desc Ring
    _tx_cur = 0;
    _tx_ring = log;
    _tx_ring_phy = phy;
    log += TX_BUFS * align128(sizeof(Tx_Desc));
    phy += TX_BUFS * align128(sizeof(Tx_Desc));

    // Rx_Buffer Ring
    for(unsigned int i = 0; i < RX_BUFS; i++) {
        _rx_buffer[i] = new (log) Buffer(&_rx_ring[i]);
        _rx_ring[i].phy_addr = phy;
        _rx_ring[i].size = Reg16(-sizeof(Frame)); // 2's comp.
        _rx_ring[i].misc = 0;
        _rx_ring[i].status = Desc::OWN; // Owned by NIC

        log += align128(sizeof(Buffer));
        phy += align128(sizeof(Buffer));
    }

    // Tx_Buffer Ring
    for(unsigned int i = 0; i < TX_BUFS; i++) {
        _tx_buffer[i] = new (log) Buffer(&_tx_ring[i]);
        _tx_ring[i].phy_addr = phy;
        _tx_ring[i].size = 0;
        _tx_ring[i].misc = 0;
        _tx_ring[i].status = 0; // Owned by host

        log += align128(sizeof(Buffer));
        phy += align128(sizeof(Buffer));
    }

    // Reset device
    reset();
}


void PCNet32::init(unsigned int unit)
{
    db<Init, PCNet32>(TRC) << "PCNet32::init(unit=" << unit << ")" << endl;

    // Scan the PCI bus for device
    PCI::Locator loc = PCI::scan(PCI_VENDOR_ID, PCI_DEVICE_ID, unit);
    if(!loc) {
        db<Init, PCNet32>(WRN) << "PCNet32::init: PCI scan failed!" << endl;
        return;
    }

    // Try to enable IO regions and bus master
    PCI::command(loc, PCI::command(loc) | PCI::COMMAND_IO | PCI::COMMAND_MASTER);

    // Get the config space header and check if we got IO and MASTER
    PCI::Header hdr;
    PCI::header(loc, &hdr);
    if(!hdr) {
        db<Init, PCNet32>(WRN) << "PCNet32::init: PCI header failed!" << endl;
        return;
    }
    db<Init, PCNet32>(INF) << "PCNet32::init: PCI header=" << hdr << endl;
    if(!(hdr.command & PCI::COMMAND_IO))
        db<Init, PCNet32>(WRN) << "PCNet32::init: I/O unaccessible!" << endl;
    if(!(hdr.command & PCI::COMMAND_MASTER))
        db<Init, PCNet32>(WRN) << "PCNet32::init: not master capable!" << endl;

    // Get I/O base port
    IO_Port io_port = hdr.region[PCI_REG_IO].phy_addr;
    db<Init, PCNet32>(INF) << "PCNet32::init: I/O port at " << (void *)(int)io_port << endl;

    // Get I/O irq
    IO_Irq irq = hdr.interrupt_line;
    db<Init, PCNet32>(INF) << "PCNet32::init: PCI interrut pin " << hdr.interrupt_pin << " routed to IRQ " << hdr.interrupt_line << endl;

    // Allocate a DMA Buffer for init block, rx and tx rings
    DMA_Buffer * dma_buf = new (SYSTEM) DMA_Buffer(DMA_BUFFER_SIZE);

    // Initialize the device
    PCNet32 * dev = new (SYSTEM) PCNet32(unit, io_port, irq, dma_buf);

    // Register the device
    _devices[unit].device = dev;
    _devices[unit].interrupt = IC::irq2int(irq);

    // Install interrupt handler
    IC::int_vector(_devices[unit].interrupt, &int_handler);

    // Enable interrupts for device
    IC::enable(_devices[unit].interrupt);
}

__END_SYS

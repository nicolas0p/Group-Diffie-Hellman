// EPOS PC Intel PRO/100 (i82559) Ethernet NIC Mediator Initialization

#include <system.h>
#include <machine/pc/machine.h>
#include <machine/pc/e100.h>

#include <segment.h>
#include <address_space.h>

__BEGIN_SYS

void E100::init(unsigned int unit)
{
    db<Init, E100>(TRC) << "E100::init(unit=" << unit << ")" << endl;

    // Scan the PCI bus for device
    PCI::Locator loc = PCI::scan(PCI_VENDOR_ID, PCI_DEVICE_ID, unit);
    if(!loc) {
        db<Init, E100>(WRN) << "E100::init: PCI scan failed!" << endl;
        return;
    }

    // Try to enable IO regions and bus master
    PCI::command(loc, PCI::command(loc) | PCI::COMMAND_MEMORY | PCI::COMMAND_MASTER);

    // Get the config space header and check it we got MEMORY and MASTER
    PCI::Header hdr;
    PCI::header(loc, &hdr);
    if(!hdr) {
        db<Init, E100>(WRN) << "E100::init: PCI header failed!" << endl;
        return;
    }
    db<Init, E100>(INF) << "E100::init: PCI header=" << hdr << endl;
    if(!(hdr.command & PCI::COMMAND_MEMORY))
        db<Init, E100>(WRN) << "E100::init: I/O memory unaccessible!" << endl;
    if(!(hdr.command & PCI::COMMAND_MASTER))
        db<Init, E100>(WRN) << "E100::init: not master capable!" << endl;

    // Get I/O base port
    Log_Addr io_mem = hdr.region[PCI_REG_MEM].log_addr;

    db<Init, E100>(INF) << "E100::init: I/O memory at "
                        << hdr.region[PCI_REG_MEM].phy_addr
                        << " mapped to "
                        << hdr.region[PCI_REG_MEM].log_addr
                        << " io_mem=" << io_mem << endl;

    // Get I/O irq
    IO_Irq irq = hdr.interrupt_line;
    db<Init, E100>(INF) << "E100::init: PCI interrut pin "
                        << hdr.interrupt_pin << " routed to IRQ "
                        << hdr.interrupt_line << endl;

    // Allocate a DMA Buffer for init block, rx and tx rings
    DMA_Buffer * dma_buf = new (SYSTEM) DMA_Buffer(DMA_BUFFER_SIZE);
    db<Init, E100>(INF) << "E100::init: DMA_Buffer=" << reinterpret_cast<void *>(dma_buf) << " : " << *dma_buf << endl;

    // Initialize the device
    E100 * dev = new (SYSTEM) E100(unit, io_mem, irq, dma_buf);

    // Register the device
    _devices[unit].device = dev;
    _devices[unit].interrupt = IC::irq2int(irq);

    db<E100>(INF) << "E100::init: interrupt: " << _devices[unit].interrupt << ", irq: " << irq << endl;

    // Install interrupt handler
    IC::int_vector(_devices[unit].interrupt, &int_handler);

    // Enable interrupts for device
    IC::enable(_devices[unit].interrupt);
}

__END_SYS

// EPOS PC FPGA Mediator Initialization

#include <machine/pc/machine.h>
#include <machine/pc/fpga.h>

__BEGIN_SYS

void FPGA::init()
{
    db<Init, FPGA>(TRC) << "FPGA::init()" << endl;

    // Scan the PCI bus for device
    PCI::Locator loc = PCI::scan(PCI_VENDOR_ID, PCI_DEVICE_ID, 0);
    if(!loc) {
        db<Init, FPGA>(WRN) << "FPGA::init: PCI scan failed!" << endl;
        return;
    }

    // Try to enable IO regions and bus master
    PCI::command(loc, PCI::command(loc) | PCI::COMMAND_MEMORY | PCI::COMMAND_MASTER);

    // Get the config space header and check if we got Regio[0] memory mapped and MASTER
    PCI::Header hdr;
    PCI::header(loc, &hdr);
    if(!hdr) {
        db<Init, FPGA>(WRN) << "FPGA::init: PCI header failed!" << endl;
        return;
    }
    db<Init, FPGA>(INF) << "FPGA::init: PCI header=" << hdr << endl;
    if(!(hdr.command & PCI::COMMAND_MEMORY)) {
        db<Init, FPGA>(WRN) << "FPGA::init: memory unaccessible!" << endl;
        return;
    }
    if(!(hdr.command & PCI::COMMAND_MASTER)) {
        db<Init, FPGA>(WRN) << "FPGA::init: not master capable!" << endl;
        return;
    }
    if(!hdr.region[PCI_REG_CTRL] || !hdr.region[PCI_REG_CTRL].memory) {
        db<Init, FPGA>(WRN) << "FPGA::init: control block unaccessible!" << endl;
        return;
    }

    // Get I/O base port
    Phy_Addr phy_addr = hdr.region[PCI_REG_CTRL].phy_addr;
    Log_Addr log_addr = hdr.region[PCI_REG_CTRL].log_addr;
    unsigned int size = hdr.region[PCI_REG_CTRL].size;
    db<Init, FPGA>(INF) << "FPGA::init: control block of " << size << " bytes at " << phy_addr << "(phy) mapped to " << log_addr << "(log)" << endl;

    // Get I/O irq
    IO_Irq irq = hdr.interrupt_line;
    unsigned int interrupt = IC::irq2int(irq);
    db<Init, FPGA>(INF) << "FPGA::init: PCI interrut pin " << hdr.interrupt_pin << " routed to IRQ " << hdr.interrupt_line << " to trigger INT " << interrupt << endl;

    // Allocate a DMA Buffer for init block, rx and tx rings
    _dma_buf = new (SYSTEM) DMA_Buffer(DMA_BUFFER_SIZE);

    // Initialize the device
    _base = log_addr;
    reset();

    // Install interrupt handler
    IC::int_vector(interrupt, &int_handler);

    // Enable interrupts for device
    IC::enable(interrupt);
}

__END_SYS

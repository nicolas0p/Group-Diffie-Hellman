// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Declarations

#ifndef __pcnet32_h
#define __pcnet32_h

#include <ethernet.h>

__BEGIN_SYS

class Am79C970A
{
protected:
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::IO_Port IO_Port;
    typedef CPU::IO_Irq IO_Irq;
    typedef MMU::DMA_Buffer DMA_Buffer;
    typedef Ethernet::Address MAC_Address;

public:
    // Offsets from base I/O address
    enum {
        PROM       = 0x00, // 16 bytes EEPROM
        PROM_MAC   = 0x00, // MAC Address (6 bytes)
        PROM_CHK   = 0x0c, // PROM Check Sum (bytes 0-11 and 14-15)
        PROM_SIG   = 0x0e, // PROM Signature (ASCII W)

        // Word I/O (16 bits) mode
        WIO_RDP    = 0x10, // Register Data Port (CSR - Control/Status)
        WIO_RAP    = 0x12, // Register Address Port (CSR/BCR)
        WIO_RESET  = 0x14, // Software Reset Trigger (r, r + w on LANCE)
        WIO_BDP    = 0x16, // Register Data Port (BCR - Bus Configuration)

        // Double Word (32 bits) I/O mode
        DWIO_RDP   = 0x10, // Register Data Port (CSR - Control/Status)
        DWIO_RAP   = 0x14, // Register Address Port (CSR/BCR)
        DWIO_RESET = 0x18, // Software Reset Trigger (r, r + w on LANCE)
        DWIO_BDP   = 0x1C  // Register Data Port (BCR - Bus Configuration)
    };

    // Control and Status Registers (CSR)
    enum {
        CSC      =   0, // Controller Status and Control
        IADR_L   =   1, // Lower IADR (maps to loc. 16)
        IADR_H   =   2, // Upper IADR (maps to loc. 17)
        IMDC     =   3, // Interrupt Masks and Deferral Control
        TFC      =   4, // Test and Features Control
        ECI1     =   5, // Extended Control and Interrupt 1
        CSR_RES0 =   6, // Reserved
        ECI2     =   7, // Extended Control and Interrupt 2
        LADRF0   =   8, // Logical Address Filter [15:0]
        LADRF1   =   9, // Logical Address Filter [31:16]
        LADRF2   =  10, // Logical Address Filter [47:32]
        LADRF3   =  11, // Logical Address Filter [63:48]
        PADR0    =  12, // Physical Address [15:0]
        PADR1    =  13, // Physical Address [31:16]
        PADR2    =  14, // Physical Address [47:32]
        MODE     =  15, // Mode
        CSR_RES1 =  16, // Reserved -> 23
        BADR_L   =  24, // Base Address of RCV Ring (lower)
        BADR_H   =  25, // Base Address of RCV Ring (upper)
        CSR_RES2 =  26, // Reserved -> 29
        BADX_L   =  30, // Base Address of XMT Ring (lower)
        BADX_H   =  31, // Base Address of XMT Ring (upper)
        CSR_RES3 =  32, // Reserved -> 46
        TXPOLINT =  47, // Transmit Polling Interval
        CSR_RES4 =  48, // Reserved
        CHPOLINT =  49, // Chain Polling Interval
        CSR_RES5 =  50, // Reserved -> 57
        SWS      =  58, // Software Style
        CSR_RES6 =  59, // Reserved -> 75
        RCVRL    =  76, // RCV Ring Length
        CSR_RES7 =  77, // Reserved
        XMTRL    =  78, // XMT Ring Length
        CSR_RES8 =  79, // Reserved
        DMATCFW  =  80, // DMA Transfer Counter and FIFO Threshold Control
        CSR_RES9 =  81, // Reserved -> 87
        CHID_L   =  88, // Chip ID (lower)
        CHID_H   =  89, // Chip ID (upper)
        CSR_RESA =  90, // Reserved -> 99
        BTOUT    = 100, // Bus Timeout
        CSR_RESB = 101, // Reserved -> 111
        MFC      = 112, // Missed Frame Count
        CSR_RESC = 113, // Reserved
        RCC      = 112, // Receive Collision Count
        CSR_RESD = 115, // Reserved
        ONNOW    = 116, // OnNow Power Mode
        CSR_RESE = 117, // Reserved -> 123
        TR1      = 124, // Test Register 1
        MACEC    = 125, // MAC Enhanced Configuration Control
        CSR_RESF = 126  // Reserved -> 127
    };

    // CSR0 bits
    enum {
        CSR0_INIT = 0x0001,
        CSR0_STRT = 0x0002,
        CSR0_STOP = 0x0004,
        CSR0_TDMD = 0x0008,
        CSR0_TXON = 0x0010,
        CSR0_RXON = 0x0020,
        CSR0_IENA = 0x0040,
        CSR0_INTR = 0x0080,
        CSR0_IDON = 0x0100,
        CSR0_TINT = 0x0200,
        CSR0_RINT = 0x0400,
        CSR0_MERR = 0x0800,
        CSR0_MISS = 0x1000,
        CSR0_CERR = 0x2000,
        CSR0_BABL = 0x4000,
        CSR0_ERR  = 0x8000
    };

    // CSR3 bits
    enum {
        CSR3_BSWP    = 0x0004,
        CSR3_EMBA    = 0x0008,
        CSR3_DXMT2PD = 0x0010,
        CSR3_LAPPEN  = 0x0020,
        CSR3_DXSUFLO = 0x0040,
        CSR3_IDONM   = 0x0100,
        CSR3_TINTM   = 0x0200,
        CSR3_RINTM   = 0x0400,
        CSR3_MERRM   = 0x0800,
        CSR3_MISSM   = 0x1000,
        CSR3_BABLM   = 0x4000
    };

    // CSR4 bits
    enum {
        CSR4_JABM      = 0x0001,
        CSR4_JAB       = 0x0002,
        CSR4_TXSTRTM   = 0x0004,
        CSR4_TXSTRT    = 0x0008,
        CSR4_RCVCCOM   = 0x0010,
        CSR4_RCVCCO    = 0x0020,
        CSR4_UINT      = 0x0040,
        CSR4_UINTCMD   = 0x0080,
        CSR4_MFCOM     = 0x0100,
        CSR4_MFCO      = 0x0200,
        CSR4_ASTRP_RCV = 0x0400,
        CSR4_APAD_XMT  = 0x0800,
        CSR4_DPOLL     = 0x1000,
        CSR4_TIMER     = 0x2000,
        CSR4_DMAPLUS   = 0x4000,
        CSR4_EN124     = 0x8000
    };

    // CSR5 bits
    enum {
        CSR5_SPND    = 0x0001,
        CSR5_MPMODE  = 0x0002,
        CSR5_MPEN    = 0x0004,
        CSR5_MPINTE  = 0x0008,
        CSR5_MPINT   = 0x0010,
        CSR5_MPPLBA  = 0x0020,
        CSR5_EXDINTE = 0x0040,
        CSR5_EXDINT  = 0x0080,
        CSR5_SLPINTE = 0x0100,
        CSR5_SLPINT  = 0x0200,
        CSR5_SINTE   = 0x0400,
        CSR5_SINT    = 0x0800,
        CSR5_LTINTEN = 0x4000,
        CSR5_TOKINTD = 0x8000
    };

    // CSR15 bits
    enum {
        CSR15_DRX     = 0x0001,
        CSR15_DTX     = 0x0002,
        CSR15_LOOP    = 0x0004,
        CSR15_DXMTFCS = 0x0008,
        CSR15_FCOLL   = 0x0010,
        CSR15_DRTY    = 0x0020,
        CSR15_INTL    = 0x0040,
        CSR15_PROM    = 0x8000
    };

    // Bus Configuration Registers (BCR)
    enum {
        BCR_RES0 =   0, // Reserved -> 1
        MC       =   2, // Miscellaneous Configuration
        BCR_RES1 =   3, // Reserved
        LED0     =   4, // Led0 Status
        LED1     =   5, // Led1 Status
        LED2     =   6, // Led2 Status
        LED3     =   7, // Led3 Status
        BCR_RES2 =   8, // Reserved
        FDC      =   9, // Full-Duplex Control
        BCR_RES3 =  10, // Reserved -> 17
        BSBC     =  18, // Burst and Bus Control
        EECAS    =  19, // EEPROM Control and Status
        SWSC     =  20, // Software Style Control
        BCR_RES4 =  21, // Reserved
        PCILAT   =  22, // PCI Latency
        PCISID   =  23, // PCI Subsystem ID
        PCISVID  =  24, // PCI Subsystem Vendor ID
        SRAMSIZE =  25, // SRAM Size
        SRAMBND  =  26, // SRAM Boundary
        BCR_RES5 =  27, // Reserved
        EBADDR_L =  28, // Expansion Bus Address (lower)
        EBADDR_H =  29, // Expansion Bus Address (upper)
        EBDATA   =  30, // Expansion Bus Data Port
        STVAL    =  31, // Software Timer Value
        MIICAS   =  32, // MII Control and Status
        MIIADDR  =  33, // MII Address
        MIIMDR   =  34, // MII Management Data
        PCIVID   =  35, // PCI Vendor ID
        PMC_A    =  36, // PCI Power Management Copabilities Alias
        DATA0    =  37, // PCI Data Register 0 Alias
        DATA1    =  38, // PCI Data Register 1 Alias
        DATA2    =  39, // PCI Data Register 2 Alias
        DATA3    =  40, // PCI Data Register 3 Alias
        DATA4    =  41, // PCI Data Register 4 Alias
        DATA5    =  42, // PCI Data Register 5 Alias
        DATA6    =  43, // PCI Data Register 6 Alias
        DATA7    =  44, // PCI Data Register 7 Alias
        PMR1     =  45, // Pattern Matching 1
        PMR2     =  46, // Pattern Matching 2
        PMR3     =  47  // Pattern Matching 3
    };

    // BCR2 bits
    enum {
        BCR2_XMAUSEL  = 0x0001,
        BCR2_ASEL     = 0x0002,
        BCR2_AWAKE    = 0x0004,
        BCR2_EADISEL  = 0x0008,
        BCR2_DXCVRPOL = 0x0010,
        BCR2_DXCVRCTL = 0x0020,
        BCR2_INTLEVEL = 0x0080,
        BCR2_APROMWE  = 0x0100,
        BCR2_TMAULOOP = 0x4000
    };

    // BCR9 bits
    enum {
        BCR9_FDEN = 0x0001
    };

    // BCR18 bits
    enum {
        BCR18_BWRITE = 0x0020,
        BCR18_BREADE = 0x0040,
        BCR18_DWIO   = 0x0080
    };

    // BCR20 bits
    enum {
        BCR20_SWSTYLE2 = 0x0002,
        BCR20_SSIZE32  = 0x0100,
        BCR20_CSRPCNET = 0x0200
    };

    // Initialization block (pg 156)
    struct Init_Block {
        Reg16 mode;		// (pg 120)
        Reg8 rlen;		// log2(n buf), e.g. 4 -> 16 Rx_Desc
        Reg8 tlen;		// log2(n buf), e.g. 3 ->  8 Tx_Desc
        MAC_Address  mac_addr;	// MAC address
        Reg16 reserved;
        Reg32 filter1;
        Reg32 filter2;
        Reg32 rx_ring;		// Tx Ring DMA physical address
        Reg32 tx_ring;		// Rx Ring DMA physical address
    };

    // Transmit and Receive Descriptors (in the Ring Buffers)
    struct Desc {
        enum {
            OWN = 0x8000,
            ERR = 0x4000,
            STP = 0x0200,
            ENP = 0x0100,
            BPE = 0x0080
        };

        Reg32 phy_addr;
        volatile Reg16 size; // 2's complement
        volatile Reg16 status;
        volatile Reg32 misc;
        volatile Reg32 reserved;
    };

    // Receive Descriptor
    struct Rx_Desc: public Desc {
        enum {
            BUFF = 0x0400,
            CRC  = 0x0800,
            OFLO = 0x1000,
            FRAM = 0x2000
        };

        friend Debug & operator<<(Debug & db, const Rx_Desc & d) {
            db << "{" << hex << d.phy_addr << dec
               << "," << 65536 - d.size
               << "," << hex << d.status
               << "," << d.misc << dec << "}";
            return db;
        }
    };

    // Transmit Descriptor
    struct Tx_Desc: public Desc {
        friend Debug & operator<<(Debug & db, const Tx_Desc & d) {
            db << "{" << hex << d.phy_addr << dec
               << "," << 65536 - d.size
               << "," << hex << d.status
               << "," << d.misc << dec << "}";
            return db;
        }
    };

public:
    Reg8 prom(int a) {
        return CPU::in8(_io_port + PROM + a);
    }
    void s_reset() { // pg 96
        // Assert S_RESET
        CPU::in16(_io_port + WIO_RESET);

        // Wait for STOP
 	for(int i = 0; (i < 100) && !(csr(CSC) & 0x0004); i++);
    }

    Reg16 rap() volatile {
        return CPU::in16(_io_port + WIO_RAP);
    }
    void rap(Reg16 v) {
        CPU::out16(_io_port + WIO_RAP, v);
    }
    Reg16 csr(int a) volatile {
        CPU::out16(_io_port + WIO_RAP, a);
        return CPU::in16(_io_port + WIO_RDP);
    }
    void csr(int a, Reg16 v) {
        CPU::out16(_io_port + WIO_RAP, a);
        CPU::out16(_io_port + WIO_RDP, v);
    }
    Reg16 bcr(int a) volatile {
        CPU::out16(_io_port + WIO_RAP, a);
        return CPU::in16(_io_port + WIO_BDP);
    }
    void  bcr(int a, Reg16 v) {
        CPU::out16(_io_port + WIO_RAP, a);
        CPU::out16(_io_port + WIO_BDP, v);
    }

    Reg16 dwio_rap() volatile {
        return (CPU::in32(_io_port + DWIO_RAP) & 0xffff);
    }
    void dwio_rap(Reg16 v) {
        CPU::out32(_io_port + DWIO_RAP, v);
    }
    void dwio_s_reset() {
        CPU::in32(_io_port + DWIO_RESET);
    }
    Reg16 dwio_csr(int a) volatile {
        CPU::out32(_io_port + DWIO_RAP, a);
        return (CPU::in32(_io_port + DWIO_RDP) & 0xffff);
    }
    void dwio_csr(int a, Reg16 v) {
        CPU::out32(_io_port + DWIO_RAP, a);
        CPU::out32(_io_port + DWIO_RDP, v);
    }
    Reg16 dwio_bcr(int a) volatile {
        CPU::out32(_io_port + DWIO_RAP, a);
        return (CPU::in32(_io_port + DWIO_BDP) & 0xffff);
    }
    void dwio_bcr(int a, Reg16 v) volatile {
        CPU::out32(_io_port + DWIO_RAP, a);
        CPU::out32(_io_port + DWIO_BDP, v);
    }

    int log2(int n) {
        int log2_n = 0;
        for(; n > 1; n >>= 1, log2_n++);
        return log2_n;
    }

protected:
    IO_Port _io_port;
};


// PCNet32 PC Ethernet NIC
class PCNet32: public Ethernet::NIC_Base<Ethernet, Traits<NIC>::NICS::Polymorphic>, private Am79C970A
{
    template<int unit> friend void call_init();

private:
    // PCI ID
    static const unsigned int PCI_VENDOR_ID = 0x1022;
    static const unsigned int PCI_DEVICE_ID = 0x2000;
    static const unsigned int PCI_REG_IO = 0;

    // Transmit and Receive Ring sizes
    static const unsigned int UNITS = Traits<PCNet32>::UNITS;
    static const unsigned int TX_BUFS = Traits<PCNet32>::SEND_BUFFERS;
    static const unsigned int RX_BUFS =	Traits<PCNet32>::RECEIVE_BUFFERS;
    static const bool promiscuous = Traits<PCNet32>::promiscuous;

    // Size of the DMA Buffer that will host the ring buffers and the init block
    static const unsigned int DMA_BUFFER_SIZE = ((sizeof(Init_Block) + 15) & ~15U) +
        RX_BUFS * ((sizeof(Rx_Desc) + 15) & ~15U) + TX_BUFS * ((sizeof(Tx_Desc) + 15) & ~15U) +
        RX_BUFS * ((sizeof(Buffer) + 15) & ~15U) + TX_BUFS * ((sizeof(Buffer) + 15) & ~15U); // align128() cannot be used here

    // Interrupt dispatching binding
    struct Device {
        PCNet32 * device;
        unsigned int interrupt;
    };

protected:
    PCNet32(unsigned int unit, IO_Port io_port, IO_Irq irq, DMA_Buffer * dma);

public:
    ~PCNet32();

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size);
    int receive(Address * src, Protocol * prot, void * data, unsigned int size);

    Buffer * alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    void free(Buffer * buf);
    int send(Buffer * buf);

    const Address & address() { return _address; }
    void address(const Address & address) { _address = address; }

    const Statistics & statistics() { return _statistics; }

    void reset();

    static PCNet32 * get(unsigned int unit = 0) { return get_by_unit(unit); }

private:
    void handle_int();

    static void int_handler(const IC::Interrupt_Id & interrupt);

    static PCNet32 * get_by_unit(unsigned int unit) {
        assert(unit < UNITS);
        return _devices[unit].device;
    }

    static PCNet32 * get_by_interrupt(unsigned int interrupt) {
        for(unsigned int i = 0; i < UNITS; i++)
            if(_devices[i].interrupt == interrupt)
        	return _devices[i].device;
        db<PCNet32>(WRN) << "PCNet32::get_by_interrupt(" << interrupt << ") => no device bound!" << endl;
        return 0;
    };

    static void init(unsigned int unit);

private:
    unsigned int _unit;

    Address _address;
    Statistics _statistics;

    IO_Irq _irq;
    DMA_Buffer * _dma_buf;

    Init_Block * _iblock;
    Phy_Addr  _iblock_phy;

    int _rx_cur;
    Rx_Desc * _rx_ring;
    Phy_Addr _rx_ring_phy;

    int _tx_cur;
    Tx_Desc * _tx_ring;
    Phy_Addr _tx_ring_phy;

    Buffer * _rx_buffer[RX_BUFS];
    Buffer * _tx_buffer[TX_BUFS];

    static Device _devices[UNITS];
};

__END_SYS

#endif

// EPOS PC Intel PRO/100 (i8255x) Ethernet NIC Mediator Declarations

#ifndef __e100_h
#define __e100_h

#include <ic.h>
#include <ethernet.h>

__BEGIN_SYS

/* Reference: Intel:8255x-OSSDM:2006
 * Intel 8255x 10/100 Mbps Ethernet Controller Family
 * Open Source Software Developer Manual
 * 2006. */
class i8255x
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

    #define FRAME_SIZE 1514

    #define offsetof(TYPE, MEMBER) ((Reg32) &((TYPE *)0)->MEMBER)
    #define write32(b,addr) (*reinterpret_cast<volatile Reg32 *>(addr) = (b))
    #define read32(addr) (*reinterpret_cast<volatile Reg32 *>(addr))
    #define MII_LED_CONTROL 0x1B
    #define ETH_ALEN 6
    #define i82559_WAIT_SCB_TIMEOUT 20000
    #define i82559_WAIT_SCB_FAST 20
    #define i82559_WAIT_SCB_TIMEOUT_TRY 20
    #define i82559_WAIT_SCB_FAST_TRY 4

    //  EEPROM_Ctrl bits.
    #define EE_SHIFT_CLK    0x01            // EEPROM shift clock.
    #define EE_CS           0x02            // EEPROM chip select.
    #define EE_DATA_WRITE   0x04            // EEPROM chip data in.
    #define EE_DATA_READ    0x08            // EEPROM chip data out.
    #define EE_ENB          (0x4800 | EE_CS)

    // The EEPROM commands include the always-set leading bit.
    #define EE_WRITE_CMD(a)     (5 << (a))
    #define EE_READ_CMD(a)      (6 << (a))
    #define EE_ERASE_CMD(a)     (7 << (a))
    #define EE_WRITE_EN_CMD(a)  (19 << ((a) - 2))
    #define EE_WRITE_DIS_CMD(a) (16 << ((a) - 2))
    #define EE_ERASE_ALL_CMD(a) (18 << ((a) - 2))
    #define EE_TOP_CMD_BIT(a)   ((a) + 2) // Counts down to zero
    #define EE_TOP_DATA_BIT     (15)      // Counts down to zero
    #define EEPROM_ENABLE_DELAY (1000)    //(10) // Delay at chip select
    #define EEPROM_SK_DELAY     (1000)    // Delay between clock edges *and* data read or transition; 3 of these per bit.
    #define EEPROM_DONE_DELAY   (1000)    // Delay when all done

    enum eeprom_ctrl_lo {
        eesk = 0x01, //EEPROM Data Output (Flash Address[15])
        eecs = 0x02, //EEPROM Chip Select
        eedi = 0x04, //EEPROM Data Input
        eedo = 0x08, //EEPROM Data Output (Flash Address[14])
    };

    /* CSR (Control/Status Registers) */
    typedef struct CSR {
        struct {
            volatile Reg8 status;
            volatile Reg8 stat_ack;
            volatile Reg8 cmd_lo;
            volatile Reg8 cmd_hi;
            volatile Reg32 gen_ptr;
        } scb;
        volatile Reg32 port;
        volatile Reg16 flash_ctrl;
        volatile Reg8 eeprom_ctrl_lo;
        volatile Reg8 eeprom_ctrl_hi;
        volatile Reg32 mdi_ctrl;
        volatile Reg32 rx_dma_count;
    } CSR_Desc;

    enum port {
        SOFTWARE_RESET  = 0x0000,
        SELFTEST        = 0x0001,
        SELECTIVE_RESET = 0x0002,
    };

    /* System Control Block (SCB) */
    // System Command Word
    enum { /* Taking int account a word (16-bit) */
        // SIMB: Specific Interrupt Mask Bits: (1) masks, (0) unmasks
        SIMB_CX_MASK  = 0x8000, /* Bit[31] (Bit[15] of upper word) */
        SIMB_FR_MASK  = 0x4000, /* Bit[30] (Bit[14] of upper word) */
        SIMB_CNA_MASK = 0x2000, /* Bit[29] (Bit[13] of upper word) */
        SIMB_RNR_MASK = 0x1000, /* Bit[28] (Bit[12] of upper word) */
        SIMB_ER_MASK  = 0x0800, /* Bit[27] (Bit[11] of upper word) */
        SIMB_FCP_MASK = 0x0400, /* Bit[26] (Bit[10] of upper word) */

        // SI: Software generated Interrupt: (1) generates interrupt
        SI            = 0x0200, /* Bit[25] (Bit[10] of upper word) */

        // M: Mask interrupt: (1) masks, (0) unmasks
        M             = 0x0100, /* Bit[24] (Bit[9] of upper word) */
    };

    // System Status Word
    // Upper half-word
    enum { /* Taking into account a half-word (8-bit) */
        CX_TNO               = 0x80, /* Bit[15] (Bit[7] of half word), CU finished executing a command with its interrupt bit set */
        FR                   = 0x40, /* RU has finished receiving a frame or the header portion of a frame */
        CNA                  = 0x20, /* CU has left the active state or has entered the idle state */
        RNR                  = 0x10, /* RU not ready */
        MDI                  = 0x08, /* Management Data Interface (MDI) read or write cycle has completed */
        SWI                  = 0x04, /* Software generated interrupt */
        FCP                  = 0x01, /* Flow Control Pause */
        NOT_OURS             = 0x00,
        NOT_PRESENT          = 0xFF,
        STAT_ACK_RX          = (SWI | RNR | FR),
        STAT_ACK_TX          = (CNA | CX_TNO),
    };
    // Lower half-word
    enum {
        // CU Status
        CUS_MASK             = 0xc0, /* AND-mask */
        CUS_SHIFT            = 6,
        // possible values
        CUS_HQP_ACTIVE       = 0x3,
        CUS_LPQ_ACTIVE       = 0x2,
        CUS_SUSPENDED        = 0x1,
        CUS_IDLE             = 0x0,

        // RU Status
        RUS_MASK             = 0x3c, /* AND-mask */
        RUS_SHIFT            = 2,
        // possible values
        RUS_READY            = 0x4,
        RUS_NO_RESOURCES     = 0x2,
        RUS_SUSPENDED        = 0x1,
        RUS_IDLE             = 0x0
    };

    /* --- */

    /* Receive Frame Descriptor */
    // Command Word
    enum { /* Taking into account a word (16-bit) */
        /* Not taking into account the offsets.
         * Taking into account the work in isolation.
         *  */
        // RDF offset 0x00
        RFD_EL_MASK             = 0x8000,
        RFD_S_MASK              = 0x4000,
        RFD_H_MASK              = 0x0010,
        RFD_SF_MASK             = 0x0008,

        // RDF offset 0x0c
        RFD_SIZE_MASK           = 0x3fff
    };

    // Status Word
    enum { /* Taking into account a word (16-bit) */
        /* Not taking into account the offsets.
         * Taking into account the work in isolation.
         *  */
        // RDF offset 0x00
        RFD_C_MASK              = 0x8000, /* Bit[15] */
        RFD_OK_MASK             = 0x2000, /* Bit[13] */
        RFD_STATUS_BITS_MASK    = 0x1fff, /* Bit[12:0] */

        // RDF offset 0x0c
        RFD_EOF_MASK            = 0x8000,
        RFD_F_MASK              = 0x4000,
        RFD_ACTUAL_COUNT_MASK   = 0x3fff
    };

    // Values for RFD_STATUS_BITS field
    enum {
        RFD_STATUS_BITS_CRC_ERROR       = 0x800,
        RFD_STATUS_BITS_ALIGN_ERROR     = 0x400,
        RFD_STATUS_BITS_OUT_SPACE       = 0x200,
        RFD_STATUS_BITS_DMA_OVERRUN     = 0x100,
        RFD_STATUS_BITS_SHORT_FRAME     = 0x080,
        RFD_STATUS_BITS_TYPE_LENGTH     = 0x020,
        RFD_STATUS_BITS_RECV_ERROR      = 0x010,
        RFD_STATUS_BITS_NO_ADDR_MATCH   = 0x004,
        RFD_STATUS_BITS_IA_MATCH        = 0x002,
        RFD_STATUS_BITS_RECV_COLLI      = 0x001
    };

    /* --- */

    enum tx_rx_status {
        Rx_RFD_NOT_FILLED = 0x0,
        Rx_RFD_AVAILABLE = 0x1,
        Tx_CB_AVAILABLE = 0x0,
        Tx_CB_IN_USE = 0x1,
    };

    enum cb_status {
        cb_complete = 0x8000,
        cb_ok       = 0x2000,
    };

    enum scb_cmd_hi {
        irq_mask_none = 0x00,
        irq_mask_all  = 0x01,
        irq_sw_gen    = 0x02,
    };

    enum scb_cmd_lo {
        cuc_nop        = 0x00,
        ruc_start      = 0x01,
        ruc_resume     = 0x02,
        ruc_dma_redir  = 0x03,
        ruc_about      = 0x04,
        ruc_load_head  = 0x05,
        ruc_load_base  = 0x06,
        cuc_start      = 0x10,
        cuc_resume     = 0x20,
        cuc_dump_addr  = 0x40,
        cuc_dump_stats = 0x50,
        cuc_load_base  = 0x60,
        cuc_dump_reset = 0x70,
    };

    struct config {
    /*0*/   Reg8 byte_count:6, pad0:2;
    /*1*/   Reg8 rx_fifo_limit:4, tx_fifo_limit:3, pad1:1;
    /*2*/   Reg8 adaptive_ifs;
    /*3*/   Reg8 mwi_enable:1, type_enable:1, read_align_enable:1,term_write_cache_line:1, pad3:4;
    /*4*/   Reg8 rx_dma_max_count:7, pad4:1;
    /*5*/   Reg8 tx_dma_max_count:7, dma_max_count_enable:1;
    /*6*/   Reg8 pad6:1, direct_rx_dma:1, tco_statistics:1, ci_intr:1, standard_tcb:1, standard_stat_counter:1,
                 rx_discard_overruns:1, rx_save_bad_frames:1;
    /*7*/   Reg8 rx_discard_short_frames:1, tx_underrun_retry:2, pad7:3, tx_two_frames_in_fifo:1, tx_dynamic_tbd:1;
    /*8*/   Reg8 mii_mode:1, pad8:6, csma_disabled:1;
    /*9*/   Reg8 rx_tcpudp_checksum:1, pad9_1:3, vlan_arp_tco:1, link_status_wake:1, pad9_2:2;
    /*10*/  Reg8 pad10:3, no_source_addr_insertion:1, preamble_length:2, loopback:2;
    /*11*/  Reg8 pad11:8;
    /*12*/  Reg8 pad12_0:1, pad12_1:3, ifs:4;
    /*13*/  Reg8 ip_addr_lo;
    /*14*/  Reg8 ip_addr_hi;
    /*15*/  Reg8 promiscuous_mode:1, broadcast_disabled:1, wait_after_win:1,
                 pad15_1:1, ignore_ul_bit:1, crc_16_bit:1, pad15_2:1, crs_or_cdt:1;
    /*16*/  Reg8 fc_delay_lo;
    /*17*/  Reg8 fc_delay_hi;
    /*18*/  Reg8 rx_stripping:1, tx_padding:1, rx_crc_transfer:1, rx_long_ok:1, fc_priority_threshold:3, pad18:1;
    /*19*/  Reg8 pad19:1, magic_packet_disable:1, fc_disable:1, fc_restop:1,
                 fc_restart:1, fc_reject:1, full_duplex_force:1, full_duplex_pin:1;
    /*20*/  Reg8 pad20_1:5, fc_priority_location:1, multi_ia:1, pad20_2:1;
    /*21*/  Reg8 pad21_1:3, multicast_all:1, pad21_2:4;
    };

    struct stats {
        Reg32 tx_good_frames, tx_max_collisions, tx_late_collisions,
              tx_underruns, tx_lost_crs, tx_deferred, tx_single_collisions,
              tx_multiple_collisions, tx_total_collisions;
        Reg32 rx_good_frames, rx_crc_errors, rx_alignment_errors,
              rx_resource_errors, rx_overrun_errors, rx_cdt_errors,
              rx_short_frame_errors;
        Reg32 fc_xmt_pause, fc_rcv_pause, fc_rcv_unsupported;
        Reg16 xmt_tco_frames, rcv_tco_frames;
        Reg32 complete;
    };

    enum cb_command {
        cb_nop    = 0x0000,
        cb_iaaddr = 0x0001,
        cb_config = 0x0002,
        cb_multi  = 0x0003,
        cb_tx     = 0x0004,
        cb_ucode  = 0x0005,
        cb_dump   = 0x0006,
        cb_tx_sf  = 0x0008,
        cb_cid    = 0x1f00,
        cb_i      = 0x2000,
        cb_s      = 0x4000,
        cb_el     = 0x8000,
    };

    enum mdi_ctrl {
        mdi_write = 0x04000000,
        mdi_read  = 0x08000000,
        mdi_ready = 0x10000000,
    };

    enum led_state {
        led_on     = 0x01,
        led_off    = 0x04,
        led_on_559 = 0x05,
        led_on_82559 = 0x07,
        led_on_557 = 0x07,
    };

    struct mem {
        struct {
            volatile Reg32 signature;
            volatile Reg32 result;
        } selftest;
        struct stats stats;
        Reg8 dump_buf[596];
    };

    struct Control {
        volatile Reg16 status;
        volatile Reg16 command;
        volatile Reg32 link;
    };

    struct ConfigureCB: public Control {
        struct config config;
    };

    struct MACaddrCB: public Control {
        MAC_Address iaaddr;
    };

    // Transmit and Receive Descriptors (in the Receive Ring Buffer)
    struct Desc: public Control {
    };

    // Receive Descriptor
    struct Rx_Desc: public Desc {
        volatile Reg32 rbd;
        volatile Reg16 actual_count;
        volatile Reg16 size;
        char frame[FRAME_SIZE];

        Reg32 _pad[1];

        friend Debug & operator<<(Debug & db, const Rx_Desc & d) {
            db << "{"
               << "}";

            return db;
        }
    };

    // Transmit Descriptor
    struct Base_Tx_Desc: public Desc {
        volatile Reg32 tbd_array;
        volatile Reg16 tcb_byte_count;
        volatile Reg8 threshold;
        volatile Reg8 tbd_number;

        Base_Tx_Desc(Reg32 phy_of_next) {
            command = cb_s | cb_cid;
            status = cb_complete;
            tbd_array = 0xFFFFFFFF; // simplified mode
            tcb_byte_count = 0;
            threshold = 0xE0;
            tbd_number = 0;
            link = phy_of_next; //next TxCB
        }
    };

public:
    int log2(int n) {
        int log2_n = 0;
        for(; n > 1; n >>= 1, log2_n++);
        return log2_n;
    }

protected:
    void udelay(long long d) {
        TSC::Time_Stamp end;
        d *= TSC::frequency() / 1000000;
        end = TSC::time_stamp() + d;
        while(end > TSC::time_stamp());
    }

    static inline unsigned char read8(const volatile void *addr) {
        return *((volatile unsigned char*) addr);
    };

    static inline void write8(unsigned char b, volatile void *addr) {
        *((volatile unsigned char*) addr) = b;
    };

    static inline unsigned short read16(const volatile void *addr) {
        return *((volatile unsigned short*) addr);
    };

    static inline void write16(unsigned short b, volatile void *addr) {
        *((volatile unsigned short*) addr) = b;
    };


    int exec_command(Reg8 cmd, Reg32 dma_addr);

protected:
    CSR_Desc * _csr;

    volatile unsigned int _tx_cuc_suspended;
    Ethernet::Address _address;
    Ethernet::Statistics _statistics;
    Ethernet::Buffer * _tx_buffer_prev; // Previously transmitted buffer
};

class i82559ER: public i8255x // Works with QEMU
{
protected:
    // PCI ID
    static const unsigned int PCI_VENDOR_ID = 0x8086;
    static const unsigned int PCI_DEVICE_ID = 0x1209;
    static const unsigned int PCI_REG_IO = 1;
    static const unsigned int PCI_REG_MEM = 0;

    static const unsigned int TBD_ARRAY_SIZE = 1;

protected:

    struct Transmit_Buffer_Descriptor {
        volatile Reg32 address;
        volatile Reg16 even_word;
        volatile Reg16 odd_word;

        enum {
            // AND-masks
            SIZE_MASK   =   0x7fff, // Take into account half word
            EL_MASK     =   0x1     // Take into account half word
        };

        bool el() {
            return odd_word & EL_MASK;
        }

        Reg16 size() {
            return even_word & SIZE_MASK;
        }

        void size(Reg16 size) {
            even_word = SIZE_MASK & size; // using SIZE_MASK to ensure that the last bit of even_word is zero.
        }
    };
    typedef Transmit_Buffer_Descriptor TBD;

    /* QEMU 2.4 requires from the TCB to use a TBD even while using simplified
     * memory structure.
     * Because of that, we keep tbd_array = 0xFFFFFFFF and tbd_number = 0
     * Such workaround is not expected to work on a physical E100.
     * For that, prefer using i82559c instead.
     * */
    struct Tx_Desc: public i8255x::Base_Tx_Desc {
        TBD tbds[TBD_ARRAY_SIZE];

        char _frame[FRAME_SIZE]; // XXX: Since TBD is in use, this could be placed at another place (e.g. TX Buffer).

        Reg32 _pad[3];

        Tx_Desc(Reg32 phy_of_next) : i8255x::Base_Tx_Desc(phy_of_next) {
            for (unsigned int i = 0; i < TBD_ARRAY_SIZE; i++) {
                /// tbds[i].address = reinterpret_cast<Reg32>(_frame); /// XXX: maybe must be the physical address of _frame here.
                tbds[i].address = (phy_of_next - align128(sizeof(Tx_Desc))) + (reinterpret_cast<Reg32>(_frame) - reinterpret_cast<Reg32>(this));
                // db<void>(WRN) << "(0) frame: " << reinterpret_cast<void *>(_frame) << " this: " << this << endl;
                // db<void>(WRN) << "(1) _frame: " << reinterpret_cast<void *>(tbds[i].address) << endl;
                // unsigned long present;
                // db<void>(WRN) << "(2) _frame: " << reinterpret_cast<void *>(MMU_Aux::physical_address(reinterpret_cast<Reg32>(_frame), &present)) << endl;

                tbds[i].size(FRAME_SIZE);
            }
        }

        friend Debug & operator<<(Debug & db, const Tx_Desc & d) {
            db << "{" << reinterpret_cast<void *>(d.tbd_array) << ", "
               << d.tcb_byte_count << ", "
               << reinterpret_cast<void *>(d.threshold) << ", "
               << reinterpret_cast<void *>(d.tbd_number)
               << "}";

            return db;
        }

        char * frame() {
            return _frame;
        }
    };
};

class i82559c: public i8255x // Works on E100 (i82559c) physical hardware
{
protected:
    // PCI ID
    static const unsigned int PCI_VENDOR_ID = 0x8086;
    static const unsigned int PCI_DEVICE_ID = 0x1229;
    static const unsigned int PCI_REG_IO = 1;
    static const unsigned int PCI_REG_MEM = 0;

protected:

    struct Tx_Desc: public i8255x::Base_Tx_Desc {
        char _frame[FRAME_SIZE];

        Tx_Desc(Reg32 phy_of_next) : i8255x::Base_Tx_Desc(phy_of_next) {
        }

        friend Debug & operator<<(Debug & db, const Tx_Desc & d) {
            db << "{" << reinterpret_cast<void *>(d.tbd_array) << ", "
               << d.tcb_byte_count << ", "
               << reinterpret_cast<void *>(d.threshold) << ", "
               << reinterpret_cast<void *>(d.tbd_number) << ", "
               << "frame: ";

            for (unsigned int i = 0; i < FRAME_SIZE; i++) {
                db << (unsigned char) (*(d._frame + i));
            }

            db << "}";

            return db;
        }

         char * frame() {
            return _frame;
    }
    };

};

// typedef i82559c _Base_Dev; // Designed to work with physical E100 (i82559c version)
typedef i82559ER _Base_Dev; // Designed to work with QEMU

class E100: public Ethernet::NIC_Base<Ethernet, Traits<NIC>::NICS::Polymorphic>, private _Base_Dev
{
    template<int unit> friend void call_init();

private:
    typedef _Base_Dev Base_Dev;

private:
    // PCI ID
    static const unsigned int PCI_VENDOR_ID = Base_Dev::PCI_VENDOR_ID;
    static const unsigned int PCI_DEVICE_ID = Base_Dev::PCI_DEVICE_ID;
    static const unsigned int PCI_REG_IO = Base_Dev::PCI_REG_IO;
    static const unsigned int PCI_REG_MEM = Base_Dev::PCI_REG_MEM;

    // Transmit and Receive Ring Buffer sizes
    static const unsigned int UNITS = Traits<E100>::UNITS;
    static const unsigned int TX_BUFS = Traits<E100>::SEND_BUFFERS;
    static const unsigned int RX_BUFS = Traits<E100>::RECEIVE_BUFFERS;
    static const unsigned int DMA_BUFFER_SIZE =
        ((sizeof(ConfigureCB) + 15) & ~15U) +
        ((sizeof(MACaddrCB) + 15) & ~15U) +
        ((sizeof(struct mem) + 15) & ~15U) +
         RX_BUFS * ((sizeof(Rx_Desc) + 15) & ~15U) +
         TX_BUFS * ((sizeof(Tx_Desc) + 15) & ~15U) +
         RX_BUFS * ((sizeof(Buffer) + 15) & ~15U)  +
         TX_BUFS * ((sizeof(Buffer) + 15) & ~15U); // align128() cannot be used here

    // Interrupt dispatching binding
    struct Device {
        E100 * device;
        unsigned int interrupt;
    };

protected:
    E100(unsigned int unit, const Log_Addr & io_mem, const IO_Irq & irq, DMA_Buffer * dma_buf);

public:
    ~E100();

    int send(const Address & dst, const Protocol & prot, const void * data, unsigned int size);
    int receive(Address * src, Protocol * prot, void * data, unsigned int size);

    Buffer * alloc(NIC * nic, const Address & dst, const Protocol & prot, unsigned int once, unsigned int always, unsigned int payload);
    void free(Buffer * buf);
    int send(Buffer * buf);

    const Address & address() { return _address; }
    void address(const Address & address) { _address = address; }

    const Statistics & statistics() { return _statistics; }

    void reset();

    static E100 * get(unsigned int unit = 0) { return get_by_unit(unit); }

private:
    void handle_int();

    static void int_handler(const IC::Interrupt_Id & interrupt);

    bool verifyPendingInterrupts(void);

    unsigned short eeprom_read(unsigned short * addr_len, unsigned short addr);
    unsigned char eeprom_mac_address(Reg16 addr);

    void i82559_flush() { read8(&_csr->scb.status); }
    void i82559_disable_irq() { write8(irq_mask_all, &_csr->scb.cmd_hi); }
    void i82559_enable_irq() { write8(irq_mask_none, &_csr->scb.cmd_hi); }

    int self_test();

    void software_reset() {
        write32(SELECTIVE_RESET, &_csr->port);
        i82559_flush(); udelay(20 * 1000);
        write32(SOFTWARE_RESET, &_csr->port);
        i82559_flush(); udelay(20 * 1000);
        // disable IRQs
        i82559_disable_irq();
        i82559_flush(); udelay(1000);
    }

    void i82559_configure(void);

    static E100 * get_by_unit(unsigned int unit) {
        assert(unit < UNITS);
        return _devices[unit].device;
    }

    static E100 * get_by_interrupt(unsigned int interrupt) {
        for(unsigned int i = 0; i < UNITS; i++)
            if(_devices[i].interrupt == interrupt)
                return _devices[i].device;

        db<E100>(WRN) << "E100::get_by_interrupt(" << interrupt << ") => no device bound!" << endl;
        return 0;
    }

    static void init(unsigned int unit);

private:
   void print_csr() {
        db<E100>(WRN) << "CSR = " << _csr << endl;
        // db<E100>(WRN) << "CSR.SCB: " << csr->scb << endl;
        db<E100>(WRN) << "status = " << hex << _csr->scb.status << endl;
        db<E100>(WRN) << "stat_ack = " << hex << _csr->scb.stat_ack << endl;
        db<E100>(WRN) << "SCB Command: " << reinterpret_cast<void *>(*reinterpret_cast<unsigned long *>(_csr)) << endl;
        db<E100>(WRN) << "cmd_lo: [" << _csr->scb.cmd_lo << "] => " << _csr->scb.cmd_lo << endl;
        db<E100>(WRN) << "cmd_hi: [" << _csr->scb.cmd_hi << "] => " << _csr->scb.cmd_hi << endl;
        db<E100>(WRN) << "gen_ptr: [" << _csr->scb.gen_ptr << "] => " << *reinterpret_cast<volatile Reg32 *>(&_csr->scb.gen_ptr) << endl;
        db<E100>(WRN) << "port: [" << _csr->port << "] => " << *reinterpret_cast<volatile Reg32 *>(&_csr->port) << endl;
        // db<E100>(WRN) << "flash_ctrl: [" << _csr->flash_ctrl << "] => " << *reinterpret_cast<volatile Reg16 *>(&_csr->flash_ctrl) << endl;
        db<E100>(WRN) << "eeprom_ctrl_lo: [" << _csr->eeprom_ctrl_lo << "] => " << *reinterpret_cast<volatile Reg8 *>(&_csr->eeprom_ctrl_lo) << endl;
        db<E100>(WRN) << "eeprom_ctrl_hi: [" << _csr->eeprom_ctrl_hi << "] => " << *reinterpret_cast<volatile Reg8 *>(&_csr->eeprom_ctrl_hi) << endl;
        db<E100>(WRN) << "mdi_ctrl: [" << _csr->mdi_ctrl << "] => " << *reinterpret_cast<volatile Reg32 *>(&_csr->mdi_ctrl) << endl;
        db<E100>(WRN) << "rx_dma_count: [" << _csr->rx_dma_count << "] => " << *reinterpret_cast<volatile Reg32 *>(&_csr->rx_dma_count) << endl;
    }

    void print_status() {
        /* The SCB Status word is not updated immediately in response to SCB
         * commands and this method is not currently taking that into account. */

        // SCB Status Word, STAT/ACK bits
        db<E100>(WRN) << "STAT/ACK = ";

        Reg8 stat_ack = read8(&_csr->scb.stat_ack);

        if (stat_ack == NOT_PRESENT) {
            db<E100>(WRN) << "NOT_PRESENT" << endl;
        } else if (stat_ack == NOT_OURS) {
            db<E100>(WRN) << "NOT_OURS" << endl;
        } else {
            if (stat_ack & CX_TNO) db<E100>(WRN) << "CX_TNO ";
            if (stat_ack & FR) db<E100>(WRN) << "FR ";
            if (stat_ack & CNA) db<E100>(WRN) << "CNA ";
            if (stat_ack & RNR) db<E100>(WRN) << "RNR ";
            if (stat_ack & MDI) db<E100>(WRN) << "MDI ";
            if (stat_ack & SWI) db<E100>(WRN) << "SWI ";
            if (stat_ack & FCP) db<E100>(WRN) << "FCP ";

            db<E100>(WRN) << endl;
        }

        Reg8 status = read8(&_csr->scb.status);
        db<E100>(WRN) << "CU = ";
        if (((status & CUS_MASK) >> CUS_SHIFT) == CUS_HQP_ACTIVE)
            db<E100>(WRN) << "CUS_HQP_ACTIVE" << endl;
        else if (((status & CUS_MASK) >> CUS_SHIFT) == CUS_LPQ_ACTIVE)
            db<E100>(WRN) << "CUS_LPQ_ACTIVE" << endl;
        else if (((status & CUS_MASK) >> CUS_SHIFT) == CUS_SUSPENDED)
            db<E100>(WRN) << "CUS_SUSPENDED" << endl;
        else if (((status & CUS_MASK) >> CUS_SHIFT) == CUS_IDLE)
            db<E100>(WRN) << "CUS_IDLE" << endl;
        else
            db<E100>(WRN) << "Invalid CU status " << endl;

        db<E100>(WRN) << "RU = ";
        if (((status & RUS_MASK) >> RUS_SHIFT) == RUS_READY)
            db<E100>(WRN) << "RUS_READY" << endl;
        else if (((status & RUS_MASK) >> RUS_SHIFT) == RUS_NO_RESOURCES)
            db<E100>(WRN) << "RUS_NO_RESOURCES" << endl;
        else if (((status & RUS_MASK) >> RUS_SHIFT) == RUS_SUSPENDED)
            db<E100>(WRN) << "RUS_SUSPENDED" << endl;
        else if (((status & RUS_MASK) >> RUS_SHIFT) == RUS_IDLE)
            db<E100>(WRN) << "RUS_IDLE" << endl;
        else
            db<E100>(WRN) << "Invalid RU status" << endl;
    }

private:
    unsigned int _unit;

    Log_Addr _io_mem;
    IO_Irq _irq;

    volatile unsigned int _rx_ruc_no_more_resources;

    ConfigureCB * configCB;
    Phy_Addr _configCB_phy;

    MACaddrCB * macAddrCB;
    Phy_Addr _macAddrCB_phy;

    struct mem * dmadump;
    Phy_Addr _dmadump_phy;

    int _rx_cur, _rx_last_el;
    Rx_Desc * _rx_ring;
    Phy_Addr _rx_ring_phy;

    int _tx_cur, _tx_prev;
    Tx_Desc * _tx_ring;
    Phy_Addr _tx_ring_phy;

    unsigned int _tx_frames_sent;

    Buffer * _rx_buffer[RX_BUFS];
    Buffer * _tx_buffer[TX_BUFS];

    DMA_Buffer * _dma_buffer;

    static Device _devices[UNITS];

private:
    static const bool HYSTERICALLY_DEBUGGED = true;

};

__END_SYS

#endif

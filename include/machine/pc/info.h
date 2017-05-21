// EPOS PC Run-Time System Information

#ifndef __pc_info_h
#define __pc_info_h

#include <system/info.h>

__BEGIN_SYS

struct System_Info
{
private:
    typedef unsigned int LAddr;
    typedef unsigned int PAddr;
    typedef unsigned int Size;

public:
    // The information we have at boot time (built by MKBI)
    // Modifications to this map requires adjustments at MKBI
    struct Boot_Map
    {
        volatile unsigned int n_cpus;     // Number of CPUs in SMPs
        PAddr mem_base;                   // Memory base address
        PAddr mem_top;                    // Memory top address
        PAddr io_base;                    // I/O Memory base address
        PAddr io_top;                     // I/O Memory top address
        short node_id;                    // Local node id in SAN (-1 => RARP)
        short n_nodes;                    // Number of nodes in SAN (-1 => dynamic)
        unsigned char uuid[8];            // EPOS image Universally Unique Identifier
        Size img_size;                    // Boot image size (in bytes)
        int setup_offset;                 // Image offsets (-1 => not present)
        int init_offset;
        int system_offset;
        int application_offset;
        int extras_offset;
        volatile int cpu_status[Traits<Machine>::CPUS]; // CPUs initialization status
    };

    // Physical Memory Map (built by SETUP)
    struct Physical_Memory_Map
    {
        PAddr mem_base;         // Memory base address
        PAddr mem_top;          // Memory top address
        PAddr io_base;          // I/O Memory base address
        PAddr io_top;           // I/O Memory top address
        PAddr ext_base;         // Boot Image EXTRA segment base address
        PAddr ext_top;          // Boot Image EXTRA segment top address
        PAddr idt;              // IDT
        PAddr gdt;              // GDT
        PAddr tss;              // TSSs (only for system call; 1 per CPU)
        PAddr sys_pt;           // System Page Table
        PAddr sys_pd;           // System Page Directory
        PAddr sys_info;         // System Info
        PAddr phy_mem_pts;      // Page tables to map the whole physical memory
        PAddr io_pts;           // Page tables to map the I/O address space
        PAddr sys_code;         // OS Code Segment
        PAddr sys_data;         // OS Data Segment
        PAddr sys_stack;        // OS Stack Segment
        PAddr free1_base;       // First free memory chunk base address
        PAddr free1_top;        // First free memory chunk top address
        PAddr free2_base;       // Second free memory chunk base address
        PAddr free2_top;        // Second free memory chunk top address
        PAddr free3_base;       // Third free memory chunk base address
        PAddr free3_top;        // Third free memory chunk top address
    };

    // Load Map (built by SETUP)
    struct Load_Map
    {
        bool  has_stp;
        bool  has_ini;
        bool  has_sys;
        bool  has_app;
        bool  has_ext;
        LAddr stp_entry;
        Size  stp_segments;
        LAddr stp_code;
        Size  stp_code_size;
        LAddr stp_data;
        Size  stp_data_size;
        LAddr ini_entry;
        Size  ini_segments;
        LAddr ini_code;
        Size  ini_code_size;
        LAddr ini_data;
        Size  ini_data_size;
        LAddr sys_entry;
        Size  sys_segments;
        LAddr sys_code;
        Size  sys_code_size;
        LAddr sys_data;
        Size  sys_data_size;
        LAddr sys_stack;
        Size  sys_stack_size;
        LAddr app_entry;
        Size  app_segments;
        LAddr app_code;
        Size  app_code_size;
        LAddr app_data;
        LAddr app_stack;
        LAddr app_heap;
        Size  app_data_size;
        PAddr app_extra;
        Size  app_extra_size;
    };

    // Time Map (built by SETUP)
    struct Time_Map
    {
        unsigned int cpu_clock;
        unsigned int bus_clock;
    };

public:
    friend Debug & operator<<(Debug & db, const System_Info & si);

public:
    Boot_Map bm;
    Physical_Memory_Map pmm;
    Load_Map lm;
    Time_Map tm;
};

__END_SYS

#endif

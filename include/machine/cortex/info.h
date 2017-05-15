// EPOS ARM Cortex Run-Time System Information

#ifndef __cortex_info_h
#define __cortex_info_h

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
        volatile unsigned int n_cpus;   // Number of CPUs in SMPs
        PAddr mem_base;                 // Memory base address
        PAddr mem_top;                  // Memory top address
        PAddr io_base;                  // I/O Memory base address
        PAddr io_top;                   // I/O Memory top address
        short node_id;                  // Local node id in SAN (-1 => RARP)
        short n_nodes;                  // Number of nodes in SAN (-1 => dynamic)
        unsigned char uuid[8];          // EPOS image Universally Unique Identifier
        Size img_size;                  // Boot image size (in bytes)
        int setup_offset;               // Image offsets (-1 => not present)
        int init_offset;
        int system_offset;
        int application_offset;
        int extras_offset;
    };

    // Load Map (not used in this machine, but kept for architectural transparency)
    struct Load_Map
    {
        bool  has_ext;
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

public:
    friend Debug & operator<<(Debug & db, const System_Info & si) { return db; }

public:
    Boot_Map bm;
    Load_Map lm;
};

__END_SYS

#endif

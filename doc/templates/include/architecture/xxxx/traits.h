// EPOS XXXX Architecture Metainfo
#ifndef __xxxx_traits_h
#define __xxxx_traits_h

#include <system/config.h>

__BEGIN_SYS

template <> struct Traits<XXXX>: public Traits<void>
{
    enum {LITTLE, BIG};
    static const unsigned int ENDIANESS         = LITTLE;
    static const unsigned int WORD_SIZE         = 32;
    static const unsigned int CLOCK             = 2000000000;
    static const bool unaligned_memory_access   = true;
};

template <> struct Traits<XXXX_TSC>: public Traits<void>
{
};

template <> struct Traits<XXXX_MMU>: public Traits<void>
{
};

__END_SYS

#endif

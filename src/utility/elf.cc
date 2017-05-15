// EPOS ELF Utility Implementation

#include <utility/elf.h>
#include <utility/string.h>

__BEGIN_UTIL

int ELF::load_segment(int i, Elf32_Addr addr)
{
    if((i > segments()) || (segment_type(i) != PT_LOAD))
        return 0;

    char * src = (char *)(unsigned(this) + seg(i)->p_offset);
    char * dst = (char *)((addr) ? addr : segment_address(i));

    memcpy(dst, src, seg(i)->p_filesz);
    memset(dst + seg(i)->p_filesz, 0, seg(i)->p_memsz - seg(i)->p_filesz);

    return seg(i)->p_memsz;
}

__END_UTIL

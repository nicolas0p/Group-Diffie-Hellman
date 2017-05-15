# EPOS Boot Makefile

include ../../makedefs

TARGET := $(if $(shell find $(MACH)* 2> /dev/null), $(MACH)_install)
ELF_HEADER_OFFSET := sh -c "$(OBJDUMP) -p $(IMG)/$(MACH)_setup | sed -n -e 's/^ *LOAD off *\(0x.*\) vaddr.*$$/\1/ p' | head -1"

all: 		$(TARGET)

pc_boot:        pc_boot.o
		ld86 -0 -s -T $(BOOT_ADDR) -o $@ $<

pc_boot.o:      pc_boot.s
		sed -e 's/^ELF_HDR_SIZE =.*$$/ELF_HDR_SIZE = $(shell $(ELF_HEADER_OFFSET))/' -i $<
		as86 -0 -o $@ $<

pc_install:	pc_boot
		$(DD) if=$< of=$<.tmp ibs=32 skip=1 obs=512 >& /dev/null
		$(INSTALL) $(MACH)_boot.tmp $(IMG)/$(MACH)_boot
		$(CLEAN) $(MACH)_boot.tmp

clean:
		$(CLEAN) *.s *.o *_boot *.tmp


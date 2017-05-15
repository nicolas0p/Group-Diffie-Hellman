# EPOS Setup Makefile

include ../../makedefs

TARGET := $(if $(shell find $(MACH)* 2> /dev/null), $(if $(SETUP_ADDR), install, install.o))

all:		$(TARGET)

ifeq ($(MACH),cortex)
$(MACH)_setup.s: $(MACH)_setup.S
		$(CPP) $(CPPFLAGS) -D $(MMOD) $< -o $@

$(MACH)_setup.o: $(MACH)_setup.s
		sed -e 's/MEM_TOP/$(call GETTK,MEM_TOP,$(MACH_TRAITS))/' -i $<
		$(AS) $(ASFLAGS) -o $@ $<
endif

$(MACH)_setup:	$(MACH)_setup.o
		$(LD) $(LDFLAGS) -L$(CCLIB) --omagic --section-start .init=$(SETUP_ADDR) -o $@ $^ -l$(LINIT) -l$(LMACH) -l$(LARCH) -l$(LUTIL) -lgcc

install:	$(MACH)_setup
		$(INSTALL) $< $(IMG)

install.o:	$(MACH)_setup.o
		$(INSTALL) $< $(LIB)

clean:
		$(CLEAN) *.o *.s *_setup

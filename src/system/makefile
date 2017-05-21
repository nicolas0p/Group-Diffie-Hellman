# EPOS System Makefile

include ../../makedefs

OBJS := $(subst .cc,.o,$(shell find *.cc))

all:		install

install: $(MACH)_install_$(MODE)

$(MACH)_install_library: $(MACH)_system_library
		$(INSTALL) $(MACH)_system.o $(LIB)
		$(INSTALL) $(MACH)_application.o $(LIB)

$(MACH)_install_builtin: $(MACH)_system_builtin
		$(OBJCOPY) -L _end $(MACH)_system
		$(INSTALL) $(MACH)_system $(IMG)
		$(INSTALL) $(MACH)_application.o $(LIB)

$(MACH)_install_kernel: $(MACH)_system_kernel
		$(INSTALL) $(MACH)_system $(IMG)
		$(INSTALL) $(MACH)_application.o $(LIB)

$(MACH)_system_library: system_scaffold.o system_binding.o application_scaffold.o
		$(LD) $(LDFLAGS) -i system_scaffold.o system_binding.o -o $(MACH)_system.o
		$(INSTALL) application_scaffold.o $(MACH)_application.o

$(MACH)_system_builtin: $(OBJS)
		$(LD) $(LDFLAGS) -L$(CCLIB) --nmagic \
		--section-start $(MACH_CODE_NAME)=$(SYS_CODE_ADDR) \
		--section-start $(MACH_DATA_NAME)=$(SYS_DATA_ADDR) \
		--entry=_init -o $(MACH)_system \
		$(LIB)/$(ARCH)_crtbegin.o \
		system_scaffold.o system_binding.o \
		$(LIB)/$(ARCH)_crtend.o \
		--whole-archive \
		-l$(LSYS) -l$(LMACH) -l$(LARCH) \
		--no-whole-archive \
		-l$(LUTIL) -lgcc
		$(INSTALL) application_scaffold.o $(MACH)_application.o

$(MACH)_system_kernel: $(OBJS)
		$(LD) $(LDFLAGS) -L$(CCLIB) --nmagic \
		--section-start $(MACH_CODE_NAME)=$(SYS_CODE_ADDR) \
		--section-start $(MACH_DATA_NAME)=$(SYS_DATA_ADDR) \
		--entry=_init -o $(MACH)_system \
		$(LIB)/$(ARCH)_crtbegin.o \
		system_scaffold.o system_binding.o kernel_binding.o \
		$(LIB)/$(ARCH)_crtend.o \
		--whole-archive \
		-l$(LSYS) -l$(LMACH) -l$(LARCH) \
		--no-whole-archive \
		-l$(LUTIL) -lgcc
		$(LD) $(LDFLAGS) -i application_scaffold.o application_binding.o -o $(MACH)_application.o

clean:
		$(CLEAN) *.o *_system

FORCE:

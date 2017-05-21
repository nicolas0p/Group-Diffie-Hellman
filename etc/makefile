# EPOS Configuration Makefile

include	../makedefs

all: $(APP)/$(APPLICATION)_traits.h eposcc.conf eposmkbi.conf $(INCLUDE)/system/config.h
		sed -e 's/^#define MODE.*$$/#define MODE $(MODE)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define ARCH.*$$/#define ARCH $(ARCH)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define MACH.*$$/#define MACH $(MACH)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define MMOD.*$$/#define MMOD $(MMOD)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define APPL.*$$/#define APPL $(APPLICATION)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __mode_.*__$$/#define __mode_$(MODE)__/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __arch_.*__$$/#define __arch_$(ARCH)__/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __mach_.*__$$/#define __mach_$(MACH)__/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __mmod_.*__$$/#define __mmod_$(MMOD)__/' -i $(INCLUDE)/system/config.h

$(APP)/$(APPLICATION)_traits.h:
		$(INSTALL) $(INCLUDE)/system/traits.h $@

eposcc.conf: FORCE
		@echo "MODE=$(MODE)" > $@
		@echo "ARCH=$(ARCH)" >> $@
		@echo "MACH=$(MACH)" >> $@
		@echo "MMOD=$(MMOD)" >> $@
		@echo "TOOLS_PREFIX=$(COMP_PREFIX)" >> $@
		@echo "APP_CODE=$(APP_CODE_ADDR)" >> $@
		@echo "APP_DATA=$(APP_DATA_ADDR)" >> $@
		@echo "SETUP=$(SETUP_ADDR)" >> $@
		@echo "MACH_CC_FLAGS='$(MACH_CC_FLAGS)'" >> $@
		@echo "MACH_LD_FLAGS='$(MACH_LD_FLAGS)'" >> $@
		@echo "MACH_CODE_NAME='$(MACH_CODE_NAME)'" >> $@
		@echo "MACH_DATA_NAME='$(MACH_DATA_NAME)'" >> $@

eposmkbi.conf:	FORCE
		@echo "MODE=$(MODE)" > $@
		@echo "ARCH=$(ARCH)" >> $@
		@echo "MACH=$(MACH)" >> $@
		@echo "MMOD=$(MMOD)" >> $@
		@echo "CPUS=$(CPUS)" >> $@
		@echo "CLOCK=$(ARCH_CLOCK)" >> $@
		@echo "WORD_SIZE=$(ARCH_WORD_SIZE)" >> $@
		@echo "ENDIANESS=$(ARCH_ENDIANESS)" >> $@
		@echo "MEM_BASE=$(MEM_BASE)" >> $@
		@echo "MEM_TOP=$(MEM_TOP)" >> $@
		@echo "BOOT_LENGTH_MIN=$(BOOT_LENGTH_MIN)" >> $@
		@echo "BOOT_LENGTH_MAX=$(BOOT_LENGTH_MAX)" >> $@
		@echo "NODE_ID=$(NODE_ID)" >> $@
		@echo "N_NODES=$(N_NODES)" >> $@
		@echo "UUID=$(UUID)" >> $@

clean: $(APP)/$(APPLICATION)_traits.h
		sed -e 's/^#define MODE.*$$/#define MODE xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define ARCH.*$$/#define ARCH xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define MACH.*$$/#define MACH xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define MMOD.*$$/#define MMOD xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define APPL.*$$/#define APPL xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __mode_.*__$$/#define __mode_xxx__/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __arch_.*__$$/#define __arch_xxx__/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __mach_.*__$$/#define __mach_xxx__/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define __mmod_.*__$$/#define __mmod_xxx__/' -i $(INCLUDE)/system/config.h
		$(CLEAN) *.conf

FORCE:
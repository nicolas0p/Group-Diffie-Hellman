# EPOS ARMv7 Architecture Makefile

include ../../../makedefs

OBJS := $(subst .cc,.o,$(shell find *.cc | grep -v _init | grep -v _test))
CRTS := $(subst .S,.o,$(shell find *.S | grep crt))
CRTS += $(subst .c,.o,$(shell find *.c | grep crt))
CRTSI := $(subst .S,.s,$(shell find *.S | grep crt))
INITS := $(subst .cc,.o,$(shell find *.cc | grep _init))

all:		crts $(LIBARCH) $(LIBINIT)

crts:		$(CRTS)
		$(INSTALL) $^ $(LIB)

.INTERMEDIATE:	$(CRTSI)

$(LIBARCH):	$(LIBARCH)($(OBJS))

$(LIBINIT):	$(LIBINIT)($(INITS))

clean:
		$(CLEAN) *.o *.s *_test

# EPOS XXXX Architecture Makefile

include ../../../makedefs

OBJS := $(subst .cc,.o,$(shell find *.cc | grep -v _init | grep -v _test))
CRTS := $(subst .S,.o,$(shell find *.S | grep crt))
CRTSI := $(subst .S,.s,$(shell find *.S | grep crt))
INITS := $(subst .cc,.o,$(shell find *.cc | grep _init))

all:		crts $(LIBARCH) $(LIBINIT)

crts:		$(CRTS)
		$(INSTALL) $^ $(LIB)

.INTERMEDIATE:	$(CRTSI)

$(LIBARCH):	$(LIBARCH)($(OBJS))	

$(LIBINIT):	$(LIBINIT)($(INITS))

cpu.o		: cpu.cc
		$(CXX) $(CXXFLAGS) -fomit-frame-pointer $<

clean:
		$(CLEAN) *.o *.s *_test

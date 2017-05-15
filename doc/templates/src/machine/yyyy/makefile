# EPOS YYYY Machine Makefile

include ../../../makedefs

OBJS := $(subst .cc,.o,$(shell find *.cc | grep -v _test | grep -v _init))
INITS := $(subst .cc,.o,$(shell find *.cc | grep _init))

all:		$(LIBMACH) $(LIBINIT)

$(LIBMACH):	$(LIBMACH)($(OBJS))	

$(LIBINIT):	$(LIBINIT)($(INITS))

pc_interrupt.o	: pc_interrupt.cc
		$(CXX) $(CXXFLAGS) -fomit-frame-pointer $<

clean:
		$(CLEAN) *.o *_test

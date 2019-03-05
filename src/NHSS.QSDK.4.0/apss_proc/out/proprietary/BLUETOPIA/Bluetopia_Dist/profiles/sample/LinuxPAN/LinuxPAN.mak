
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../PAN/include      \
	    -I../../VNET/include     \
	    -I../../../include       \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../PAN/lib -L../../../VNET/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTPAN -lSS1VNET -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxPAN.o

.PHONY:
all: LinuxPAN

LinuxPAN: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxPAN


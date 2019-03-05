
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../HFRE/include     \
	    -I../../../include       \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../HFRE/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTHFR -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxHFR.o

.PHONY:
all: LinuxHFR

LinuxHFR: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxHFR


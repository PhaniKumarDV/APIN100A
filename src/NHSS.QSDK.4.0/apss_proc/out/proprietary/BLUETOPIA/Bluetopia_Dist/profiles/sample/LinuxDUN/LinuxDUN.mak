
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../DUN/include      \
	    -I../../VSER/include     \
	    -I../../../include       \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../DUN/lib -L../../../VSER/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTDUN -lSS1SER -lSS1VSER -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxDUN.o

.PHONY:
all: LinuxDUN

LinuxDUN: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxDUN


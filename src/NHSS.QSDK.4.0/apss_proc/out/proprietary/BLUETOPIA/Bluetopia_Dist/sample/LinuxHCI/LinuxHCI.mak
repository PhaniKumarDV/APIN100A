
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../include       \
	    -I../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../lib -L../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxHCI.o

.PHONY:
all: LinuxHCI

LinuxHCI: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxHCI



ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../SAP/include      \
	    -I../../../include       \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../SAP/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTSAP -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxSAP.o

.PHONY:
all: LinuxSAP

LinuxSAP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxSAP


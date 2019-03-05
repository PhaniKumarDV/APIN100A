
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../../profiles/GATT/include      \
	    -I../../../profiles_gatt/WSS/include  \
	    -I../../../profiles_gatt/UDS/include  \
	    -I../../DIS/include                   \
	    -I../../GAPS/include                  \
	    -I../../../include                    \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../../debug/lib -L../../UDS/lib -L../../WSS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTDIS -lSS1BTGAP -lBTPSVEND -lSS1BTDBG -lSS1BTUDS -lSS1BTWSS $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxWSP.o

.PHONY:
all: LinuxWSP

LinuxWSP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxWSP


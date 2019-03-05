
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../../profiles/GATT/include      \
	    -I../../../profiles_gatt/BCS/include  \
	    -I../../DIS/include                   \
	    -I../../GAPS/include                  \
	    -I../../../include                    \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../../debug/lib -L../../BCS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTDIS -lSS1BTGAP -lBTPSVEND -lSS1BTDBG -lSS1BTBCS $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxBCS.o

.PHONY:
all: LinuxBCS

LinuxBCS: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxBCS


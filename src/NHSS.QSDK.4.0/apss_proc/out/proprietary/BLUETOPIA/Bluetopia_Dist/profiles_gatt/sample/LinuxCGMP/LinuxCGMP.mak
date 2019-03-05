
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../../profiles/GATT/include      \
	    -I../../../profiles_gatt/CGMS/include \
	    -I../../DIS/include                   \
	    -I../../GAPS/include                  \
	    -I../../BMS/include                   \
	    -I../../../include                    \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../BMS/lib -L../../../debug/lib -L../../../profiles_gatt/CGMS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTDIS -lSS1BTGAP -lSS1BTBMS -lBTPSVEND -lSS1BTDBG -lSS1BTCGM $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxCGMP.o

.PHONY:
all: LinuxCGMP

LinuxCGMP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxCGMP


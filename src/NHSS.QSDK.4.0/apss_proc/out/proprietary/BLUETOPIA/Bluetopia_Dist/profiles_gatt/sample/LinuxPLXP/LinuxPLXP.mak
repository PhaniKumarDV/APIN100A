
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I$../../../profiles/GATT/include   \
        -I$../../../profiles_gatt/GAPS/include  \
        -I$../../../profiles_gatt/DIS/include   \
        -I../../../profiles_gatt/PLXS/include   \
        -I../../../include                      \
        -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../../profiles_gatt/DIS/lib -L../../../profiles_gatt/GAPS/lib -L../../../profiles_gatt/PLXS/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTGAP -lSS1BTDIS -lSS1BTPLXS -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxPLXP.o

.PHONY:
all: LinuxPLXP

LinuxPLXP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxPLXP

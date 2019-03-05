
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I$../../../profiles/GATT/include   \
        -I$../../../profiles_gatt/GAPS/include  \
        -I$../../../profiles_gatt/DIS/include   \
        -I../../../profiles_gatt/OTS/include    \
        -I../../../include                      \
        -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../../profiles_gatt/DIS/lib -L../../../profiles_gatt/GAPS/lib -L../../../profiles_gatt/OTS/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTGAP -lSS1BTDIS -lSS1BTOTS -lBTPSVEND -lBTPSFILE -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxOTP.o

.PHONY:
all: LinuxOTP

LinuxOTP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxOTP


ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../../profiles/GATT/include  \
	    -I../../../profiles_gatt/CTS/include \
	    -I../../../profiles_gatt/NDCS/include \
	    -I../../../profiles_gatt/RTUS/include \
	    -I../../DIS/include               \
	    -I../../GAPS/include              \
	    -I../../../include                \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../../debug/lib -L../../../profiles_gatt/CTS/lib -L../../../profiles_gatt/NDCS/lib -L../../../profiles_gatt/RTUS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTDIS -lSS1BTGAP -lBTPSVEND -lSS1BTDBG -lSS1BTCTS -lSS1BTNDC -lSS1BTRTU $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxTIP.o

.PHONY:
all: LinuxTIP

LinuxTIP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxTIP


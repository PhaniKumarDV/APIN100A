
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../../profiles/GATT/include  \
	    -I../../../profiles_gatt/TPS/include \
	    -I../../../profiles_gatt/IAS/include \
	    -I../../../profiles_gatt/LLS/include \
	    -I../../DIS/include               \
	    -I../../GAPS/include              \
	    -I../../../include                \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../../debug/lib -L../../../profiles_gatt/TPS/lib -L../../../profiles_gatt/IAS/lib -L../../../profiles_gatt/LLS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTDIS -lSS1BTGAP -lBTPSVEND -lSS1BTDBG -lSS1BTTPS -lSS1BTIAS -lSS1BTLLS $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxPXP.o

.PHONY:
all: LinuxPXP

LinuxPXP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxPXP


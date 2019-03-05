
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../../profiles/GATT/include  \
	    -I../../../profiles_gatt/HIDS/include \
	    -I../../BAS/include               \
	    -I../../DIS/include               \
	    -I../../GAPS/include              \
	    -I../../../include                \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../../debug/lib -L../../BAS/lib -L../../HIDS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTGAP -lBTPSVEND -lSS1BTDBG -lSS1BTDIS -lSS1BTBAS -lSS1BTHIDS $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxHOGP.o

.PHONY:
all: LinuxHOGP

LinuxHOGP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxHOGP

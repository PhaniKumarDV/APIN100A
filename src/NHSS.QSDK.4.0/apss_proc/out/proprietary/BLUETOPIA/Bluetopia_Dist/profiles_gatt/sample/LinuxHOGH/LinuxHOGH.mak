
ifndef CC
   CC = gcc
endif

ifdef __USE_UHID__
   UHID_PACKAGE=`pkg-config --cflags glib-2.0`
   UHID_LIB=-lglib-2.0
   UHID_OBJ=UHID.o
   UHID_DEFINE=-D__USE_UHID__
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

CFLAGS = -Wall $(UHID_PACKAGE) $(UHID_DEFINE) $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../../profiles/GATT/lib -L../../DIS/lib -L../../GAPS/lib -L../../../debug/lib -L../../BAS/lib -L../../HIDS/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTGAT -lSS1BTGAP -lBTPSVEND -lSS1BTDBG -lSS1BTDIS -lSS1BTBAS -lSS1BTHIDS  $(UHID_PACKAGE) $(UHID_LIB) $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxHOGH.o $(UHID_OBJ)

.PHONY:
all: LinuxHOGH

LinuxHOGH: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxHOGH

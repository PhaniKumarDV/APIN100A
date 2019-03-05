
ifndef CC
   CC = gcc
endif

ifndef AR
   AR = ar
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../../Bluetopia
endif

ifndef VPATH
   VPATH = ./ ../
endif

INCLDDIRS = -I../../../include                              \
	    -I../../../include/server                            \
	    -I$(BLUETOPIA_PATH)/include                          \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include            \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/CTS/include        \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/RTUS/include  \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/NDCS/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMTIPM.o TIPMGR.o TIPMGRI.o

.PHONY:
all: libBTPMTIPM.a

libBTPMTIPM.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libBTPMTIPM.a


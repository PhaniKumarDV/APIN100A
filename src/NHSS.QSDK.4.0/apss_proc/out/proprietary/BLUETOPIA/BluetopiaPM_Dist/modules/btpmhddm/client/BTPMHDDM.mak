
ifndef CC
   CC = gcc
endif

ifndef AR
   AR = ar
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../../Bluetopia
endif

INCLDDIRS = -I../../../include                            \
	    -I../../../include/client                     \
	    -I$(BLUETOPIA_PATH)/include                   \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include     \
	    -I$(BLUETOPIA_PATH)/profiles/HID/include      
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMHDDM.o HDDMGR.o

.PHONY:
all: libBTPMHDDM.a

libBTPMHDDM.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libBTPMHDDM.a


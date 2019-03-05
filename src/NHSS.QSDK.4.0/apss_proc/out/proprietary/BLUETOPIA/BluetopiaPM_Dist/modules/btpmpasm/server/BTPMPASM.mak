
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

INCLDDIRS = -I../../../include                             \
	    -I../../../include/server                      \
	    -I$(BLUETOPIA_PATH)/include                    \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include      \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/PASS/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMPASM.o PASMGR.o

.PHONY:
all: libBTPMPASM.a

libBTPMPASM.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libBTPMPASM.a


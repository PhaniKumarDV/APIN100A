
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

INCLDDIRS = -I..                                         \
	    -I../../../include                           \
	    -I../../../include/server                    \
	    -I$(BLUETOPIA_PATH)/include                  \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include    \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/ANS/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMANPM.o ANPMGR.o

.PHONY:
all: libBTPMANPM.a

libBTPMANPM.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libBTPMANPM.a


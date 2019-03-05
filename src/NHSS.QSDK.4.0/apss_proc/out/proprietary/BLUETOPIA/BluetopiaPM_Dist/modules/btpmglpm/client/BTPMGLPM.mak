
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

INCLDDIRS = -I../../../include                    \
	    -I../../../include/client                  \
	    -I$(BLUETOPIA_PATH)/include                \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMGLPM.o GLPMGR.o

.PHONY:
all: libBTPMGLPM.a

libBTPMGLPM.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libBTPMGLPM.a


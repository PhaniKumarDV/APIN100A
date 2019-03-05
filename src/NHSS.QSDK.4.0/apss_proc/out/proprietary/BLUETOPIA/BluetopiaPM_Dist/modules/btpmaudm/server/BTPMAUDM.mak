
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

INCLDDIRS = -I..                                       \
	    -I../../../include                         \
	    -I../../../include/server                  \
	    -I$(BLUETOPIA_PATH)/include                \
	    -I$(BLUETOPIA_PATH)/profiles/A2DP/include  \
	    -I$(BLUETOPIA_PATH)/profiles/Audio/include \
	    -I$(BLUETOPIA_PATH)/profiles/AVCTP/include \
	    -I$(BLUETOPIA_PATH)/profiles/AVRCP/include \
	    -I$(BLUETOPIA_PATH)/profiles/GAVD/include  \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMAUDM.o AUDMGR.o AUDMUTIL.o

.PHONY:
all: libBTPMAUDM.a

libBTPMAUDM.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libBTPMAUDM.a


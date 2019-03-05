
ifndef CC
   CC = gcc
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../Bluetopia
endif

INCLDDIRS = -I../../include                        \
	    -I../../include/client                      \
	    -I$(BLUETOPIA_PATH)/include                 \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../lib/client $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPM $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxSPPLE_PM.o

.PHONY:
all: LinuxSPPLE_PM

LinuxSPPLE_PM: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxSPPLE_PM


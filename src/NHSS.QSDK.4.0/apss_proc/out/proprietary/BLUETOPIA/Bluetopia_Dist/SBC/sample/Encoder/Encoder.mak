
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../include    \
	    -I../../../include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../lib -L../../../lib $(GLOBLDFLAGS)

LDLIBS = -lSS1SBC -lSS1BTPS $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = Encoder.o

.PHONY:
all: Encoder

Encoder: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f Encoder


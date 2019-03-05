
ifndef CC
   CC = gcc
endif

ifndef AR
   AR = ar
endif

INCLDDIRS = -I../include    \
	    -I../../include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = IACPTRAN.o

.PHONY:
all: libSS1IACP.a

libSS1IACP.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libSS1IACP.a


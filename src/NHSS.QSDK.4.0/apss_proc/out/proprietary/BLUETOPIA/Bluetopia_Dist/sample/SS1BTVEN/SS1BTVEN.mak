
ifndef CC
   CC = gcc
endif

ifndef AR
   AR = ar
endif

INCLDDIRS = -I../../include
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTVEND.o

.PHONY:
all: libSS1BTVEN.a

libSS1BTVEN.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libSS1BTVEN.a


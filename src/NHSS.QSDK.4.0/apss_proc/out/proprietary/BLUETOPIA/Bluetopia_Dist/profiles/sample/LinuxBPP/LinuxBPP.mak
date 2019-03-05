
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../BPP/include     \
	    -I../../../include      \
	    -I../../../XML/SS1SXMLP \
	    -I../../../debug/include

ifndef VPATH
   VPATH = ./ ../../../XML/SS1SXMLP
endif

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../BPP/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTBPP -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxBPP.o SS1SXMLP.o

.PHONY:
all: LinuxBPP

LinuxBPP: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxBPP


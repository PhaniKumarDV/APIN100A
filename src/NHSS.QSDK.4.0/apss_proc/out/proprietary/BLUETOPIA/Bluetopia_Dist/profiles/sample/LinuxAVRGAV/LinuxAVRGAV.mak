
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../AVRCP/include    \
	    -I../../AVCTP/include    \
	    -I../../GAVD/include     \
	    -I../../A2DP/include     \
	    -I../../../include       \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../AVCTP/lib -L../../AVRCP/lib -L../../GAVD/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTAVC -lSS1BTAVR -lSS1BTGAV -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxAVRGAV.o

.PHONY:
all: LinuxAVRGAV

LinuxAVRGAV: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxAVRGAV


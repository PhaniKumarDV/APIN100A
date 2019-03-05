
ifndef CC
   CC = gcc
endif

INCLDDIRS = -I../../Audio/include    \
	    -I../../A2DP/include     \
	    -I../../AVRCP/include    \
	    -I../../AVCTP/include    \
	    -I../../GAVD/include     \
	    -I../../../SBC/include   \
	    -I../../../include       \
	    -I../../../debug/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm -lrt
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../../lib -L../../Audio/lib -L ../../AVRCP/lib -L../../AVCTP/lib -L../../GAVD/lib -L../../../SBC/lib -L../../../debug/lib $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPS -lSS1BTAUD -lSS1BTAVC -lSS1BTAVR -lSS1BTGAV -lSS1SBC -lBTPSVEND -lSS1BTDBG $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxAUD.o AudioEncoder.o AudioDecoder.o

ifdef DISABLE_AUDIO_SINK_AUDIO_PROCESSING
   DISABLE_AUDIO_SINK_ALSA_OUTPUT := 1
endif

ifdef DISABLE_AUDIO_SINK_ALSA_OUTPUT
   CFLAGS += -DDISABLE_AUDIO_SINK_ALSA_OUTPUT
else
   LDLIBS += -lasound
endif


.PHONY:
all: LinuxAUD

LinuxAUD: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxAUD


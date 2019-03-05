
ifndef CC
   CC = gcc
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../Bluetopia
endif

INCLDDIRS = -I../../include                            \
	    -I../../include/client                     \
	    -I$(BLUETOPIA_PATH)/include                \
	    -I$(BLUETOPIA_PATH)/profiles/A2DP/include  \
	    -I$(BLUETOPIA_PATH)/profiles/Audio/include \
	    -I$(BLUETOPIA_PATH)/profiles/AVCTP/include \
	    -I$(BLUETOPIA_PATH)/profiles/AVRCP/include \
	    -I$(BLUETOPIA_PATH)/profiles/GAVD/include  \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include  \
	    -I$(BLUETOPIA_PATH)/SBC/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm -lrt
endif
	
CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDFLAGS = -L../../lib/client $(GLOBLDFLAGS)

LDLIBS = -lSS1BTPM $(SYSTEMLIBS) $(GLOBLDLIBS)

OBJS = LinuxAUDM.o AudioEncoder.o AudioDecoder.o

ifdef DISABLE_AUDIO_SINK_AUDIO_PROCESSING
   DISABLE_AUDIO_SINK_ALSA_OUTPUT := 1
endif

ifdef DISABLE_AUDIO_SINK_ALSA_OUTPUT
   CFLAGS += -DDISABLE_AUDIO_SINK_ALSA_OUTPUT
else
   LDLIBS += -lasound
endif

.PHONY:
all: LinuxAUDM

LinuxAUDM: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f LinuxAUDM


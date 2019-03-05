
ifndef CC
   CC = gcc
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../Bluetopia
endif

INCLDDIRS = -I../../include             \
	    -I../../include/server                    \
	    -I$(BLUETOPIA_PATH)/include               \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include

ifndef SYSTEMLIBS
   SYSTEMLIBS = -lpthread -lm
endif

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LIBRARIES = ./libSS1BTPML.a 				             \
	    ../../btpmerr/libBTPMERR.a                               \
	    ../../btpmmodc/server/libBTPMMODC.a                      \
	    $(wildcard ../../modules/*/lib*.a)                       \
	    $(wildcard ../../modules/*/server/lib*.a)                \
	    $(BLUETOPIA_PATH)/lib/libSS1BTPS.a                       \
	    $(BLUETOPIA_PATH)/lib/libBTPSFILE.a                      \
	    $(BLUETOPIA_PATH)/lib/libBTPSVEND.a                      \
	    $(BLUETOPIA_PATH)/debug/lib/libSS1BTDBG.a                \
	    $(wildcard $(BLUETOPIA_PATH)/profiles/*/lib/lib*.a)      \
	    $(wildcard $(BLUETOPIA_PATH)/profiles_gatt/*/lib/lib*.a) \
	    $(wildcard $(BLUETOPIA_PATH)/VNET/lib/lib*.a)            \
	    $(wildcard $(BLUETOPIA_PATH)/iacptrans/lib/lib*.a)

LDFLAGS = $(addprefix -L,$(dir $(LIBRARIES))) \
	  $(GLOBLDFLAGS)

LDLIBS = $(patsubst lib%,-l%,$(basename $(notdir $(LIBRARIES)))) \
	 $(SYSTEMLIBS) \
	 $(GLOBLDLIBS)

OBJS = BTPMSRVR.o

.PHONY:
all: SS1BTPM
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o SS1BTPM

SS1BTPM: $(OBJS)

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f SS1BTPM


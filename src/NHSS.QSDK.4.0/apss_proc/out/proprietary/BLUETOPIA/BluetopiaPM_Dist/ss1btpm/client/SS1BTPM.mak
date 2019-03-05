
ifndef CC
   CC = gcc
endif

ifndef AR
   AR = ar
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../Bluetopia
endif

INCLDDIRS = -I../../include                           \
            -I../../include/client                    \
            -I$(BLUETOPIA_PATH)/include               \
            -I$(BLUETOPIA_PATH)/profiles/GATT/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS =  BTPMCLT.o

SRC_LIBS = libSS1BTPML.a                               \
           ../../btpmerr/libBTPMERR.a                  \
           ../../btpmmodc/client/libBTPMMODC.a         \
	   $(wildcard ../../modules/*/lib*.a)          \
	   $(wildcard ../../modules/*/client/lib*.a)

.PHONY:
all: libSS1BTPM.a

libSS1BTPM.a: $(OBJS) $(SRC_LIBS)
	$(AR) cr $@ $(OBJS)
	@for i in $?; do                                  \
	   if echo "$$i" | grep -q '\.a$$'; then          \
	      echo -n "Merging library $$i ... " &&       \
	      DIR=`basename "_$$i"` &&                    \
	      mkdir -p "$$DIR" &&                         \
	      cd "$$DIR" &&                               \
	      $(AR) x "../$$i" &&                         \
	      $(AR) cr "../$@" * &&                       \
	      cd .. &&                                    \
	      rm -rf "$$DIR" &&                           \
	      echo "Success" || (echo "Failed!" && exit); \
	   fi;                                            \
	done 2>/dev/null

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f *~
	-rm -f libSS1BTPM.a


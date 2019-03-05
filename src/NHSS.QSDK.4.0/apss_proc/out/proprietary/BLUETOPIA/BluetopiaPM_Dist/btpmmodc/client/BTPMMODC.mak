
ifndef CC
   CC = gcc
endif

ifndef AR
   AR = ar
endif

ifndef BLUETOPIA_PATH
   BLUETOPIA_PATH = ../../../Bluetopia
endif

INCLDDIRS = -I../../include                               \
	    -I../../include/client

ifneq ($(wildcard ../../modules/btpmancm),)
INCLDDIRS += \
	    -I../../modules/btpmancm
GLOBCFLAGS += -DMODC_SUPPORT_ANCM
endif

ifneq ($(wildcard ../../modules/btpmanpm),)
INCLDDIRS += \
	    -I../../modules/btpmanpm/client
GLOBCFLAGS += -DMODC_SUPPORT_ANPM
endif

ifneq ($(wildcard ../../modules/btpmantm),)
INCLDDIRS += \
	    -I../../modules/btpmantm/client
GLOBCFLAGS += -DMODC_SUPPORT_ANTM
endif

ifneq ($(wildcard ../../modules/btpmaudm),)
INCLDDIRS += \
	    -I../../modules/btpmaudm/client
GLOBCFLAGS += -DMODC_SUPPORT_AUDM
endif

ifneq ($(wildcard ../../modules/btpmbasm),)
INCLDDIRS += \
	    -I../../modules/btpmbasm/client
GLOBCFLAGS += -DMODC_SUPPORT_BASM
endif

ifneq ($(wildcard ../../modules/btpmblpm),)
INCLDDIRS += \
	    -I../../modules/btpmblpm/client
GLOBCFLAGS += -DMODC_SUPPORT_BLPM
endif

ifneq ($(wildcard ../../modules/btpmcppm),)
INCLDDIRS += \
	    -I../../modules/btpmcppm/client
GLOBCFLAGS += -DMODC_SUPPORT_CPPM
endif

ifneq ($(wildcard ../../modules/btpmcscm),)
INCLDDIRS += \
	    -I../../modules/btpmcscm/client
GLOBCFLAGS += -DMODC_SUPPORT_CSCM
endif

ifneq ($(wildcard ../../modules/btpmfmpm),)
INCLDDIRS += \
	    -I../../modules/btpmfmpm/client
GLOBCFLAGS += -DMODC_SUPPORT_FMPM
endif

ifneq ($(wildcard ../../modules/btpmftpm),)
INCLDDIRS += \
	    -I../../modules/btpmftpm/client
GLOBCFLAGS += -DMODC_SUPPORT_FTPM
endif

ifneq ($(wildcard ../../modules/btpmglpm),)
INCLDDIRS += \
	    -I../../modules/btpmglpm/client
GLOBCFLAGS += -DMODC_SUPPORT_GLPM
endif

ifneq ($(wildcard ../../modules/btpmhddm),)
INCLDDIRS += \
	    -I../../modules/btpmhddm/client
GLOBCFLAGS += -DMODC_SUPPORT_HDDM
endif

ifneq ($(wildcard ../../modules/btpmhdpm),)
INCLDDIRS += \
	    -I../../modules/btpmhdpm/client
GLOBCFLAGS += -DMODC_SUPPORT_HDPM
endif

ifneq ($(wildcard ../../modules/btpmhdsm),)
INCLDDIRS += \
	    -I../../modules/btpmhdsm/client
GLOBCFLAGS += -DMODC_SUPPORT_HDSM
endif

ifneq ($(wildcard ../../modules/btpmhfrm),)
INCLDDIRS += \
	    -I../../modules/btpmhfrm/client
GLOBCFLAGS += -DMODC_SUPPORT_HFRM
endif

ifneq ($(wildcard ../../modules/btpmhidm),)
INCLDDIRS += \
	    -I../../modules/btpmhidm/client
GLOBCFLAGS += -DMODC_SUPPORT_HIDM
endif

ifneq ($(wildcard ../../modules/btpmhogm),)
INCLDDIRS += \
	    -I../../modules/btpmhogm/client
GLOBCFLAGS += -DMODC_SUPPORT_HOGM
endif

ifneq ($(wildcard ../../modules/btpmhrpm),)
INCLDDIRS += \
	    -I../../modules/btpmhrpm/client
GLOBCFLAGS += -DMODC_SUPPORT_HRPM
endif

ifneq ($(wildcard ../../modules/btpmhtpm),)
INCLDDIRS += \
	    -I../../modules/btpmhtpm/client
GLOBCFLAGS += -DMODC_SUPPORT_HTPM
endif

ifneq ($(wildcard ../../modules/btpmmapm),)
INCLDDIRS += \
	    -I../../modules/btpmmapm/client
GLOBCFLAGS += -DMODC_SUPPORT_MAPM
endif

ifneq ($(wildcard ../../modules/btpmoppm),)
INCLDDIRS += \
	    -I../../modules/btpmoppm/client
GLOBCFLAGS += -DMODC_SUPPORT_OPPM
endif

ifneq ($(wildcard ../../modules/btpmpanm),)
INCLDDIRS += \
	    -I../../modules/btpmpanm/client
GLOBCFLAGS += -DMODC_SUPPORT_PANM
endif

ifneq ($(wildcard ../../modules/btpmpasm),)
INCLDDIRS += \
	    -I../../modules/btpmpasm/client
GLOBCFLAGS += -DMODC_SUPPORT_PASM
endif

ifneq ($(wildcard ../../modules/btpmpbam),)
INCLDDIRS += \
	    -I../../modules/btpmpbam/client
GLOBCFLAGS += -DMODC_SUPPORT_PBAM
endif

ifneq ($(wildcard ../../modules/btpmpxpm),)
INCLDDIRS += \
	    -I../../modules/btpmpxpm/client
GLOBCFLAGS += -DMODC_SUPPORT_PXPM
endif

ifneq ($(wildcard ../../modules/btpmrscm),)
INCLDDIRS += \
	    -I../../modules/btpmrscm/client
GLOBCFLAGS += -DMODC_SUPPORT_RSCM
endif

ifneq ($(wildcard ../../modules/btpmtdsm),)
INCLDDIRS += \
	    -I../../modules/btpmtdsm/client
GLOBCFLAGS += -DMODC_SUPPORT_TDSM
endif

ifneq ($(wildcard ../../modules/btpmtipm),)
INCLDDIRS += \
	    -I../../modules/btpmtipm/client
GLOBCFLAGS += -DMODC_SUPPORT_TIPM
endif

INCLDDIRS += \
	    -I$(BLUETOPIA_PATH)/include                   \
	    -I$(BLUETOPIA_PATH)/profiles/ANT/include      \
	    -I$(BLUETOPIA_PATH)/profiles/A2DP/include     \
	    -I$(BLUETOPIA_PATH)/profiles/Audio/include    \
	    -I$(BLUETOPIA_PATH)/profiles/AVRCP/include    \
	    -I$(BLUETOPIA_PATH)/profiles/AVCTP/include    \
	    -I$(BLUETOPIA_PATH)/profiles/GATT/include     \
	    -I$(BLUETOPIA_PATH)/SBC/include               \
	    -I$(BLUETOPIA_PATH)/profiles/HDP/include      \
	    -I$(BLUETOPIA_PATH)/profiles/HDSET/include    \
	    -I$(BLUETOPIA_PATH)/profiles/HFRE/include     \
	    -I$(BLUETOPIA_PATH)/profiles/MAP/include      \
	    -I$(BLUETOPIA_PATH)/profiles/OPP/include      \
	    -I$(BLUETOPIA_PATH)/profiles/PAN/include      \
	    -I$(BLUETOPIA_PATH)/profiles/PBAP/include     \
	    -I$(BLUETOPIA_PATH)/profiles/HID/include      \
	    -I$(BLUETOPIA_PATH)/profiles/HID_Host/include \
	    -I$(BLUETOPIA_PATH)/profiles/TDS/include      \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/ANCS/include \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/CPS/include  \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/CSCS/include \
	    -I$(BLUETOPIA_PATH)/profiles_gatt/RSCS/include

CFLAGS = -Wall $(DEFS) $(INCLDDIRS) $(GLOBINCLDDIRS) -O2 -fno-strict-aliasing $(GLOBCFLAGS)

LDLIBS = -lpthread

OBJS = BTPMMODC.o

.PHONY:
all: libBTPMMODC.a

libBTPMMODC.a: $(OBJS)
	$(AR) r $@ $?

.PHONY: clean veryclean realclean
clean veryclean realclean:
	-rm -f *.o
	-rm -f libBTPMMODC.a


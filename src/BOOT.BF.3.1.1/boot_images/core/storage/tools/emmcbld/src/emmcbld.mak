#============================================================================
#  Name:
#    hostdl.mak
#
#  Description:
#    Makefile to build the FLASHPRG software for all MSM targets
#    Target specfic information is defined in hostdl.min
#
#  Execution:
#    To execute this make file on command line "make -f hostdl.mak".
#
#  Targets:
#    Valid targets to build are:   clean depend all test
#
#
# Copyright (c) 2008-2011 Qualcomm Incorporated. 
#  All Rights Reserved.
# Qualcomm Confidential and Proprietary
#----------------------------------------------------------------------------

#============================================================================
#
#                          EDIT HISTORY FOR MODULE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
#  $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld.mak#1 $ $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
#  
# when       who     what, where, why
# --------   ---     --------------------------------------------------------
# 2011-03-30 vj      Unicorn bringup changes
# 2011-01-07 vj      Change SDCC_DRIVENO to EMMCBLD_USE_DRIVENO
# 2011-01-17 vj      RVCT 4.1 support added
# 2010-01-12 vj      Added Support for 7x27 target
# 06/19/09   bb      Fixes and cleanup for via option
# 05/08/09   sv      Add define for buffered transfer from USB
# 04/13/09   sv      Add Ehosdl support/Fix PRINT LOG issues
# 04/13/09   sv      Fix multiple dependency file generation issue
# 04/02/09   jz      Use flash_hal.lib, do inc path file generation here
# 03/31/09   sv      Fix issues with clean
# 03/03/09   jz      Use via files mechanism to handle long include paths
# 01/09/09   mm      Cleanup of HSU includes from shared file
# 10/21/08   mm      Created 
#============================================================================

MAKEFILE=emmcbld
BUILD_TOOL_CHAIN=yes
BUILD_TARGET_TOOL=1

SRCROOT=../../../..
#----------------------------------------------------------------------------
# Get configurations from flash_incpath.min
#----------------------------------------------------------------------------

FLASH_TOOLS_UTILS=$(STORAGE_TOOLS_UTILS)
#----------------------------------------------------------------------------
#   Directory for objects and .dep files
#	TARGET must be defined BEFORE including hostdl_flash.min
#
#  Include FLASH specific Make configuration file
#  This min file can condition values in hostdl.min as necessary, so
#  it is mandatory that it be included first.
#
#  The included MIN file will define the flash object list depending on
#  flash type in use.
#
#  Define TARGET: Directory for objects and .dep files
#                 Must be defined BEFORE including hostdl_flash.min
#                 TARGET will be passed in while doing ehostdl compilation
#----------------------------------------------------------------------------

FLASHTYPE=SD_MMC
TARGET=emmcbld
TARGET_BSP=emmcbld/bsp

#----------------------------------------------------------------------------
# Generate name of the cust file using build ID passed to the script
#----------------------------------------------------------------------------
ifdef CUSTNAME
 ifeq ($(BUILD_UNIX), yes)
   CUSTFILE = $(shell echo CUST$(CUSTNAME).H | tr "[:upper:]" "[:lower:]")
 else
   CUSTFILE = cust$(CUSTNAME).h
 endif
endif

CUSTH   = -DCUST_H=\"$(CUSTFILE)\" # Feature include file definition

#----------------------------------------------------------------------------
# Use passed CPU type if present, otherwise use default ARM7TDMI
#----------------------------------------------------------------------------
ifdef ARM_CPU_TYPE
CPU_TYPE=$(ARM_CPU_TYPE)
else
CPU_TYPE=ARM926EJ-S
endif

#------------------------------------------------------------------------------
#
# Set USES_PRINTF to "YES" to build and use printf() as part of binary
# Any value other than exactly "YES" will not build in printf() or use of it
# 
#   Independent of test code, but if test NOISY is set, this will be
#   automatically turned on for you
#------------------------------------------------------------------------------
USES_PRINTF=NO

TNOISYLEVEL=3

ifeq '$(USES_PRINTF)' 'YES'
	PRINT_FLAGS += -DNOISY -DTNOISY=$(TNOISYLEVEL)
endif 


#===============================================================================
#                             TOOL DEFINITIONS
#===============================================================================
# The following environment variables must be defined prior to using this make
# file: ARMBIN, ARMLIB, and ARMINC. In addition the PATH must be updated for
# the ARM tools.

#------------------------------------------------------------------------------
# Tool path definitions (may be tool set dependent - ADS vs RVCT)
#------------------------------------------------------------------------------
ifndef ARMTOOLS
    $(error ARMTOOLS undefined - need to run batch file to select compiler?)
endif

ifndef ARMBIN
ARMBIN     = $(ARMHOME)/bin
endif
ARMBIN := $(subst \,/,$(ARMBIN))

ifndef ARMLIB
ARMLIB     = $(ARMHOME)/lib
endif
ARMLIB := $(subst \,/,$(ARMLIB))


#-------------------------------------------------------------------------------
# Software tool and environment definitions
#-------------------------------------------------------------------------------
ifeq ($(findstring RVCT41,$(ARMTOOLS)),RVCT41)
CC         = @$(ARMBIN)/armcc --thumb       # Thumb 16-bit inst. set ANSI C compiler
else
CC         = @$(ARMBIN)/tcc#        # Thumb 16-bit inst. set ANSI C compiler
endif
ARMCC      = @$(ARMBIN)/armcc#      # ARM 32-bit inst. set ANSI C compiler
ARMCPP     = @$(ARMBIN)/armcpp#     # ARM 32-bit inst. set ANSI C++ compiler
TCPP       = @$(ARMBIN)/tcpp#       # Thumb 16-bit inst. set ANSI C++ compiler
ASM        = @$(ARMBIN)/armasm#     # ARM assembler
ARMAR      = @$(ARMBIN)/armar#      # ARM library archiver
LD         = @$(ARMBIN)/armlink#    # ARM linker
HEXTOOL    = @$(ARMBIN)/fromelf#    # Utility to create hex file from image
BINTOOL    = @$(ARMBIN)/fromelf#    # Utility to create binary file from image

ASM_SCRIPT = $(FLASH_TOOLS_UTILS)/asm.pl# # Script includes .h files in assembly
CAT_SCRIPT = cat.pl#           # Perl script to print output for assembly files

#-------------------------------------------------------------------------------
# Defined GEN, the tool which concatenates the hex files generated by the HEXTOOL.
# This also generates the start record and appends it to the end of this file.
#
# Define SHELL, the type of command interpreter used by the environment
#------------------------------------------------------------------------------
ifeq ($(BUILD_UNIX),yes)
   SHELL=sh
   GEN=hostdl_gen
else
   SHELL=bash.exe
   GEN=hostdl_gen.exe
endif
GEN_PATH=$(SRCROOT)/drivers/flash/tools/src/hostdl
MDEPEND_SCRIPT=$(FLASH_TOOLS_UTILS)/mdepend.pl
GETDEP_SCRIPT=$(FLASH_TOOLS_UTILS)/getdep.pl
REDIRECT_SCRIPT=$(FLASH_TOOLS_UTILS)/redirect.pl
TEMP=aptemp


#----------------------------------------------------------------------------
# These are used for automatic generation of the Hex file.
#----------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Target options:
#----------------------------------------------------------------------------
EXE=elf

#------------------------------------------------------------------------------
#   C code inference rules
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#  optimize options
#  -O2 = full optimize
#  -Ospace = optimize for smallest code space
#  -O0 = no optimize
#------------------------------------------------------------------------------
ifeq ($(findstring RVCT41,$(ARMTOOLS)),RVCT41)
  OPT_FLAGS=-Otime -O3  #this is the default
else
  OPT_FLAGS=-Ospace -O2  #this is the default
endif


#------------------------------------------------------------------------------
# Additional INCLUDES
#------------------------------------------------------------------------------
EXTRAINC= $(ALL_INCLUDES)


OBJ_CMD=-o

#------------------------------------------------------------------------------
# Compiler Options
#------------------------------------------------------------------------------
ifeq ($(findstring RVCT,$(ARMTOOLS)),RVCT)
  CPU=--cpu $(CPU_TYPE)
  APCS=--apcs /noswst/interwork
  ENDIAN=--littleend
  DWARF=--dwarf2
  SPLIT_SECTIONS=--split_sections
  WARNS=
  XREF=--xref
  MAP=--map
  ELF=--elf
  INFO=--info
  SCATTER=--scatter
  DEBUG=--debug
  SYMBOLS=--symbols
  LIST=--list
  VIA=--via
  I32=--i32
  VERSION=--vsn
  SUPRESS=--diag_suppress 6730
else
  CPU=-cpu $(CPU_TYPE)
  APCS=-apcs /noswst/interwork
  ENDIAN=-littleend
  DWARF=-dwarf2
  SPLIT_SECTIONS=-zo
  WARNS=-Wb -fa
  XREF=-xref
  MAP=-map
  ELF=-elf
  INFO=-info
  SCATTER=-scatter
  DEBUG=-debug
  SYMBOLS=-symbols
  LIST=-list
  VIA=-via
  I32=-i32
  VERSION=-vsn
  SUPRESS=
endif

#------------------------------------------------------------------------------
# CFLAGS for ARM
#------------------------------------------------------------------------------
# C-Compiler flag options:
# Define HOSTDL_PROCESS_USB_BUFFERS to transfer data from usb in buffers 
# instead of single character at a time
DMSS_CFLAGS=-DT_ARM -DBUILD_HOSTDL -DBUILD_EHOSTDL $(CONFIG_TYPE_CFLAGS) \
            $(PRINT_FLAGS) -I. $(EXTRAINC)

ifeq ($(USES_HSU),yes)
DMSS_CFLAGS += $(HSU_FLAGS) -DHOSTDL_PROCESS_USB_BUFFERS -DFEATURE_EMMCBLD_USE_DDR
endif

CFLAGS = -c $(CPU) $(APCS) $(ENDIAN) -g $(DWARF) $(WARNS) \
         $(OPT_FLAGS) $(SPLIT_SECTIONS) -DDAL_CONFIG_MMU_SCHEME=DAL_CONFIG_MMU_SCHEME_NONE \
         -DEMMCBLD_USE_DRIVENO=$(EMMCBLD_BSP_SLOTNO) -DFEATURE_EMMCBLD_HAVE_WDOG

# Defined in builds file
ifeq ($(USES_SDCC_BOOT_PARTITION), yes)
CFLAGS += -DHOSTDLSD_USES_BOOT_PARTITON
endif

include $(SRCROOT)/build/ms/incpaths.min
include ./emmcbld.min
include ./bsp/emmcbld_bsp.min
include $(SRCROOT)/drivers/sdcc/sdcc.min

OBJ_CMD=-o

#------------------------------------------------------------------------------
# AFLAGS for ARM
#------------------------------------------------------------------------------
# Assembler flag options:


DMSS_AFLAGS=$(CPU) -DT_ARM -D_ARM_ASM_ -DBUILD_HOSTDL -DBUILD_EHOSTDL -I. $(EXTRAINC) \
             $(CONFIG_TYPE_CFLAGS) $(PRINT_FLAGS) $(CUSTH) -DEMMCBLD_IMEM_STACK_BASE=$(IMEM_STACK_ADDR)
ifeq ($(USES_HSU),yes)
DMSS_AFLAGS += -DFEATURE_EMMCBLD_USE_DDR
endif

AFLAGS= $(CPU) $(APCS) $(ENDIAN) -g $(DWARF) 

# Linker flag options:
#------------------------------------------------------------------------------

LINKFLAGS=$(XREF) $(MAP) $(INFO) sizes,totals,veneers $(SCATTER) \
          $(SCATTERFILE_OUT) $(SUPRESS)

#----------------------------------------------------------------------------
#   Directory for library archives
#----------------------------------------------------------------------------
LIBDIR = $(TARGET)/lib

#----------------------------------------------------------------------------
# Define Scatter load file as hostdl.scl
#----------------------------------------------------------------------------

##-- Default address to load -------------------------------------------------
START_ADDR= $(EHOSTDL_CODE_ADDR)
DATA_ADDR=+0x0
IMEM_STACK_ADDR= $(EHOSTDL_IMEM_STACK_BASE)
ifeq ($(USES_HSU),yes)
DATA_ADDR=0x0
endif

#----------------------------------------------------------------------------
# Define Scatter load file as hostdl.scl for HOSTDL and ehostdl.scl
# for EHOSTDL
# Set CODE_START_ADDR and DATA_ADDR based on HOSTDL or EHOSTDL
#----------------------------------------------------------------------------
SCATTERFILE_OUT=emmcbld.scl

#------------------------------------------------------------------------------
#   Define TARGET_FLASH_OBJS to over-ride standard definition for flash layer 
#   objects
#------------------------------------------------------------------------------
TARGET_FLASH_OBJS=	$(TARGET_FLASH_OBJS) $(SDCC_OBJS) $(EXTRA_FLASH_OBJS)

#------------------------------------------------------------------------------
# New compile rules
#------------------------------------------------------------------------------
$(TARGET)/%.o %.o: %.c
	@echo -----------------------------------------------------------------
	@echo OBJECT $@
	@$(ARMCC) $(CFLAGS) $(DMSS_CFLAGS) $(OBJ_CMD) $@ $<
	@echo -----------------------------------------------------------

$(LIBDIR)/%.o : %.c
	@echo -----------------------------------------------------------------
	@echo OBJECT $@
	@$(ARMCC) $(CFLAGS) $(DMSS_CFLAGS) $(OBJ_CMD) $@ $<
	@echo -----------------------------------------------------------

#------------------------------------------------------------------------------
# Assembly code inference rules
#------------------------------------------------------------------------------
$(TARGET)/%.o %.o: %.s
	@echo -----------------------------------------------------------------
	@echo OBJECT $@
	$(ARMCC) -E $(DMSS_AFLAGS) $(TARGET_TYPE_CFLAGS) < $<  | perl $(ASM_SCRIPT) - >  \
	$(TARGET)/$*.i
	$(ASM) $(AFLAGS) $(LIST) $(TARGET)/$*.lst $(TARGET)/$*.i -o $@
	@echo -----------------------------------------------------------------

	 
#------------------------------------------------------------------------------
# Expanded macros inference rule
#------------------------------------------------------------------------------
%.i: %.c
	@echo -----------------------------------------------------------------
	@echo OBJECT $@
	$(ARMCC) -E $(CFLAGS) $(DMSS_CFLAGS) $(OBJ_CMD) $@ $<
	@echo -----------------------------------------------------------------



#------------------------------------------------------------------------------
#   All OBJECTS list
#------------------------------------------------------------------------------
OBJECTS =	$(TARGET)/emmcbld_start.o \
		$(TARGET)/emmcbld_imem_init.o \
		$(TARGET)/emmcbld_main.o \
		$(TARGET)/emmcbld_memctl.o \
		$(TARGET)/emmcbld_packet.o \
		$(SDCC_MINI_OBJS) \
		$(EMMCBLD_BSP_OBJS) \
		$(TRANSPORT_OBJS) \
		$(TARGET)/crc.o

#----------------------------------------------------------------------------
#   Rules to produce OBJS and DEPS from SOURCES
#----------------------------------------------------------------------------
ELF_OBJS :=  $(addprefix $(TARGET)/, $(EMMCBLD_ARM_SOURCES:%.c=%.o))
ELF_OBJS :=  $(ELF_OBJS:%.s=%.o)


#----------------------------------------------------------------------------
#   All Libraries
#----------------------------------------------------------------------------
LIBS= $(DAL_LIBS)
LIBS+= $(HSU_LIBS)

##-- Default file to build ---------------------------------------------------
all: create_incpaths_file $(TARGET)/exist depend $(TARGET).$(EXE)

##-- Target to build libraries ---------------------------------------------

libs : $(LIBDIR)/exist $(LIBS)

$(TARGET)/exist:
	@echo
	@echo
	@echo ========================================================
	@echo BUILDING $(TARGET)
	@echo ========================================================
	@if [ -f   $(TARGET)/exist ]; then "" ; \
	elif [ -d   $(TARGET) ]; then echo Building   $(TARGET) >   $(TARGET)/exist ;\
	else mkdir $(TARGET); echo Building $(TARGET) >   $(TARGET)/exist ; fi 
	

$(LIBDIR)/exist:  
	@echo
	@echo ========================================================
	@echo BUILDING libdir/exist
	@echo ========================================================
	@if [ -f   $(LIBDIR)/exist ]; then "" ; \
	elif [ -d   $(LIBDIR) ]; then echo Building   $(LIBDIR) >   $(LIBDIR)/exist ;\
	else mkdir $(LIBDIR); echo Building $(LIBDIR) >   $(LIBDIR)/exist ; fi 

ifeq ($(MAKE_LIBS), yes)
  include $(HSU_MIN_FILE)
endif


#-- Target linking ----------------------------------------------------------
OBJECT_LISTFILE = objects.txt
#------------------------------------------------------------------------------

#----------------------------------------------------
#  ARMPRG.HEX target
#
#     link, create HEX file, add S-Record to HEX file
#-----------------------------------------------------

$(TARGET).$(EXE): force.frc $(SCATTERFILE_OUT) libs $(OBJECTS)
	@echo
	@echo ------------------------------------------------------------------
	@echo LINKING  $(TARGET) $@
	@echo ------------------------------------------------------------------
	@-if [ -f $(OBJECT_LISTFILE) ]; then rm $(OBJECT_LISTFILE); fi	
	@perl $(REDIRECT_SCRIPT) $(OBJECT_LISTFILE) $(OBJECTS) $(LIBS)
	$(LD) $(ELF) $(LINKFLAGS) $(DEBUG) $(SYMBOLS) $(LIST) $@.map -o $@ $(LIBS) \
		$(VIA) $(OBJECT_LISTFILE)
	@$(HEXTOOL) $(I32) -o $(TARGET).hex $(TARGET).$(EXE) 
	@mv $(TARGET).hex $(TEMP).hex
	@chmod +x $(GEN_PATH)/$(GEN)
	@$(GEN_PATH)/$(GEN) -t $(TARGET).hex -s $(S_ADDR) -f $(TEMP).hex
	@chmod +rwx $(TARGET).hex
	@echo ------------------------------------------------------------------
	@echo CREATING   $(FINALNAME).hex
	@echo ------------------------------------------------------------------
	@cp $(TARGET).hex $(FINALNAME).hex
	@echo
	@echo


clean:
ifeq ($(USES_STRIP_NO_ODM),yes)
	-if test -f $(LIBDIR)/hsu_core_basic.lib; then cp $(LIBDIR)/hsu_core_basic.lib ./hsu_core_basic.lib; fi
	rm -rf $(TARGET)/*
	mkdir $(LIBDIR)
	mv hsu_core_basic.lib $(LIBDIR)/hsu_core_basic.lib
endif
	rm -f $(TARGET).sym
	rm -f $(TARGET).map
	rm -f $(TARGET).elf
	rm -f *.hex
	rm -f $(TARGET).elf.map
	rm -f $(TARGET).inc
	rm -f emmcbld.scl
	rm -f *objects.txt
	rm -rf emmcbld
	rm -f emmcbld.elf
	rm -f emmcbld.elf.map
	rm -f emmcbld.inc
#-------------------------------------------------------------------------------
# Create include paths file to handle the long include paths
# The tools include file is generated based on the ALL_TOOL_INCLUDES
# variable. The build assumes that this variable is set with all the
# required tool paths before this rule.
#-------------------------------------------------------------------------------
TOOL_INCPATHS_FILE := $(TARGET).inc

create_incpaths_file:
	@echo ------------------------------------------------------------------
	@echo Creating include paths file $(TOOL_INCPATHS_FILE)
	@echo $(ALL_TOOL_INCLUDES) > $(TOOL_INCPATHS_FILE)
	@echo ------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Setting the ALL_INCLUDES to use the tools include file
#-------------------------------------------------------------------------------

ALL_INCLUDES=$(VIA) $(TOOL_INCPATHS_FILE)

#------------------------------------------------------------------------------
# Install target
#------------------------------------------------------------------------------
install:
	@echo ========================================================
	@echo INSTALLING $(TARGET)
	@cp -f $(FINALNAME).hex $(SRCROOT)/build/ms/bin/$(patsubst %M,%,$(BUILD))/
	@echo
	@echo DONE
	@echo ========================================================


#------------------------------------------------------------------------------
# Scatterload file target
#------------------------------------------------------------------------------
$(SCATTERFILE_OUT):    force.frc emmcbld_in.scl 
	@echo ---------------------------------------------------------------
	@echo "========================================================== "
	@echo Creating Scatter Load File for $(TARGET)
	@echo "========================================================== "
	@echo "###################################################" > \
		$(SCATTERFILE_OUT) 
	@echo "##  GENERATED FILE - DO NOT EDIT" >> $(SCATTERFILE_OUT) 
	@echo "##                                    " >> $(SCATTERFILE_OUT)
	@echo "## generated:  `date`                 " >> $(SCATTERFILE_OUT)
	@echo "###################################################" >> \
		$(SCATTERFILE_OUT) 
	@$(CC) -E  -DCODE_START_ADDR=$(START_ADDR) -DDATA_START_ADDR=$(DATA_ADDR)\
            	< emmcbld_in.scl | perl $(ASM_SCRIPT) - >> $(SCATTERFILE_OUT)
	@echo Done
	@echo ---------------------------------------------------------------

 
#------------------------------------------------------------------------------
# Test targets
#------------------------------------------------------------------------------

# The flags and symbol definitions for the compiler, assembler and linker are
# listed for makefile test purposes.

test :
	@echo -----------------------------------------------------------------
	@echo AFLAGS : $(AFLAGS)
	@echo -----------------------------------------------------------------
	@echo DMSS_AFLAGS : $(DMSS_AFLAGS)
	@echo -----------------------------------------------------------------
	@echo CFLAGS : $(CFLAGS)
	@echo -----------------------------------------------------------------
	@echo DMSS_CFLAGS : $(DMSS_CFLAGS)
	@echo -----------------------------------------------------------------
	@echo TARGET_TYPE_CFLAGS : $(TARGET_TYPE_CFLAGS)
	@echo -----------------------------------------------------------------
	@echo LINKFLAGS : $(LINKFLAGS)
	@echo -----------------------------------------------------------------
	@echo LIBS : $(LIBS)
	@echo -----------------------------------------------------------------
	@echo MAKE_LIBS : $(MAKE_LIBS)
	@echo -----------------------------------------------------------------
	@echo FLASH_OBJS : $(FLASH_OBJS)
	@echo -----------------------------------------------------------------
	@echo OBJECTS : $(OBJECTS)
	@echo -----------------------------------------------------------------
	@echo FLASHINC : $(FLASHINC)
	@echo -----------------------------------------------------------------
	@echo EXTRAINC : $(EXTRAINC)
	@echo -----------------------------------------------------------------
	@echo FLASH_OBJS : $(FLASH_OBJS)
	@echo -----------------------------------------------------------------
	@echo OBJECTS : $(OBJECTS)
	@echo -----------------------------------------------------------------
	@echo ALL_INCLUDES : $(ALL_INCLUDES)
	@echo -----------------------------------------------------------------

 
#===============================================================================
#                               DEPENDENCIES
#===============================================================================

# The dependencies included at the end of this makefile can be automatically
# updated by making the 'depend' target to invoke the following rules.

DEPFILE=emmcbld_depend
DEPFILE_NAME   = $(TARGET)/$(DEPFILE).dep
DEPFILE_BACKUP = $(TARGET)/$(DEPFILE).bak
DEPFILE_TMP    = $(TARGET)/$(DEPFILE).dep.tmp

.SUFFIXES:
.SUFFIXES: .s .o .c .dep .h


%.dep:%.c 
	@echo -----------------------------------------------------------------
	@echo DEPENDENCY $@
	$(CC) $(CFLAGS) $(DMSS_CFLAGS) -E < $< | perl $(GETDEP_SCRIPT) \
	$(basename $@).o $< > $*.de_
	@rm -f  $(TARGET)/$(@F).dep
	@mv $*.de_ $*.dep
	@echo -----------------------------------------------------------------
	

$(TARGET)/%.dep:%.c
	@echo -----------------------------------------------------------------
	@echo DEPENDENCY $@
	$(CC) $(CFLAGS) $(DMSS_CFLAGS) -E < $< | perl $(GETDEP_SCRIPT) \
	$(basename $@).o $< >  $(TARGET)/$*.de_
	@rm -f  $(TARGET)/$*.dep
	@mv  $(TARGET)/$*.de_  $(TARGET)/$*.dep
	@echo -----------------------------------------------------------------


$(TARGET)/%.dep:%.s
	@echo -----------------------------------------------------------------
	@echo DEPENDENCY $@
	$(CC) $(AFLAGS) $(DMSS_AFLAGS) -E < $< | \
	perl $(GETDEP_SCRIPT) $(basename $@).o $< >  $(TARGET)/$*.de_
	@rm -f  $(TARGET)/$*.dep
	@mv  $(TARGET)/$*.de_  $(TARGET)/$*.dep
	@echo -----------------------------------------------------------------


depend:  $(TARGET)/exist $(OBJECTS:.o=.dep) 
	@echo -----------------------------------------------------------------
	@echo Processing Dependencies
	@perl $(MDEPEND_SCRIPT) $(DEPFILE_NAME)  $(TARGET) > $(DEPFILE_TMP)
	@-rm -f $(DEPFILE_BACKUP)
	@-mv $(DEPFILE_NAME) $(DEPFILE_BACKUP)
	@mv $(DEPFILE_TMP) $(DEPFILE_NAME)
	@echo -----------------------------------------------------------------

$(TARGET)/emmcbld_depend.dep: $(TARGET)/exist
	@echo " "
	@echo "Creating emmcbld.dep"
	@echo " "
	@echo "# ------------------------------" > $(TARGET)/emmcbld_depend.dep
	@echo "# DO NOT EDIT BELOW THIS LINE" >> $(TARGET)/emmcbld_depend.dep

force.frc:

# If dependency file is not present, do not complain. There is a rule to make
# one.
-include $(TARGET)/emmcbld_depend.dep

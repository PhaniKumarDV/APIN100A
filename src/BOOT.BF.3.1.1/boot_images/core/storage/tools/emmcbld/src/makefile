!if 0
   Copyright (c) 2010 by QUALCOMM, Incorporated.  All Rights Reserved.
!endif
#
# This is a dummy makefile used only for the pack process, it does not 
# make anything under the Windows build system.
#
# Because the Windows build system does not allow a dirs and a sources
# file to exist in the same directory, this file includes all packing
# information here, instead of using a sources file

SOURCES_PACKOUT_HY11=\
        makefile\
        emmcbld_hw_init_8660.s\

SOURCES_PACKOUT_HD11 = $(SOURCES_PACKOUT_HY11)
SOURCES_PACKOUT_HY22 = $(SOURCES_PACKOUT_HY11) 

!IF EXIST($(_TARGETPLATROOT)\makefile.def)
!  INCLUDE $(_TARGETPLATROOT)\makefile.def
!ENDIF

#ifndef __EMMCBLD_DEBUG_H
#define __EMMCBLD_DEBUG_H

/*=================================================================================================
 *
 * FILE:        emmcbld_debug.h
 *
 * DESCRIPTION:
 *    This file provide some defines required for debugging
 *
 *
 * Copyright (c) 2009-2010 Qualcomm Technologies Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *================================================================================================*/

/*================================================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_debug.h#1 $ 
 *   $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2010-06-23   rh      Clean up the debug bookmark definition
 * 2009-07-27   rh      Initial Revision
 *===============================================================================================*/

/*===============================================================================================
 *
 *                     Include Files
 *
 ===============================================================================================*/

#include "msm.h"

#ifdef EMMCBLD_OUTPUT_DEBUG_MSG
/*-----------------------------------------------------------------------------------------------
  This macro setup the memory controller for EBI2 so the CS5 can be
  used to access the FPGA memory lcation
-----------------------------------------------------------------------------------------------*/
#define CONFIGURE_EBI2_CS5_DEFAULT() HWIO_OUT(EBI2_CHIP_SELECT_CFG0, 0x0B01);

/*-----------------------------------------------------------------------------------------------
  This macro can be used to write to the HAPPY_LED location
-----------------------------------------------------------------------------------------------*/
#define HAPPY_LED_ADDR               0x8e00017c
#define WRITE_HAPPY_LED(x)           out_word(HAPPY_LED_ADDR, x)

#else  /* EMMCBLD_OUTPUT_DEBUG_MSG */
   
#define CONFIGURE_EBI2_CS5_DEFAULT() 
//#define CONFIGURE_EBI2_CS5_DEFAULT()
#define WRITE_HAPPY_LED(x) 

#endif /* EMMCBLD_OUTPUT_DEBUG_MSG */


/*-----------------------------------------------------------------------------------------------
  Basic init packet
-----------------------------------------------------------------------------------------------*/
#ifdef __GNUC__
  #define _INLINE_ __inline__
#else
  #define _INLINE_ __inline
#endif

static _INLINE_ void BPROFILE_WRITE_SYNC ()
{
   CONFIGURE_EBI2_CS5_DEFAULT();

   /* Send the start up sync pattern */
   WRITE_HAPPY_LED(0x0001);
   WRITE_HAPPY_LED(0x0002);
   WRITE_HAPPY_LED(0x0004);
   WRITE_HAPPY_LED(0x0008);
   WRITE_HAPPY_LED(0x0010);
   WRITE_HAPPY_LED(0x0020);
   WRITE_HAPPY_LED(0x0040);
   WRITE_HAPPY_LED(0x0080);
   WRITE_HAPPY_LED(0x0100);
   WRITE_HAPPY_LED(0x0200);
   WRITE_HAPPY_LED(0x0400);
   WRITE_HAPPY_LED(0x0800);
   WRITE_HAPPY_LED(0x1000);
   WRITE_HAPPY_LED(0x2000);
   WRITE_HAPPY_LED(0x4000);
   WRITE_HAPPY_LED(0x8000);
}


static _INLINE_ void BPROFILE_DUMP_DATA (uint32 length, byte *data)
{
   int i;

   WRITE_HAPPY_LED (length);          
   for (i = 0; i < length; i++)
   {
      WRITE_HAPPY_LED (data[i]);          
   }
}

/* Map Bookmark write to the happly LED write in 1 to 1 mode */
#define BPROFILE_WRITE_BMRK(x)    WRITE_HAPPY_LED(x)

/* Define a set of BMRK messages */
#define EMMCBLD_BMRK_RC_SUCCESS        0x00F0
#define EMMCBLD_BMRK_RC_FAIL           0x000F
#define EMMCBLD_BMRK_HIGHLIGHT         0xCCCC
#define EMMCBLD_BMRK_DO_PARTITION      0x0100
#define EMMCBLD_BMRK_SET_ACTIVE_PRTI   0x0101
#define EMMCBLD_BMRK_DO_OPEN_MULTI_PL  0x1102         // With payload
#define EMMCBLD_BMRK_DO_STREAM_WRITE   0x1103
#define EMMCBLD_BMRK_PACKET_DDUMP      0x1110         // With payload
#define EMMCBLD_BMRK_PACKET_DDUMP_END  0x1111
#define EMMCBLD_BMRK_PACKET_WRITE      0x1112
#define EMMCBLD_BMRK_PACKET_WRITE_DONE 0x1113
#define EMMCBLD_BMRK_PART_LOCATE       0x1114

#endif /* __EMMCBLD_DEBUG_H */

/*****< ftpchcvt.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FTPCHCVT - Stonestreet One Bluetooth Stack Platform Manager Object        */
/*             Transfer Protocol Character Conversion Implementation Type     */
/*             Definitions, Constants, and Prototypes (UTF-8 <-> UTF-16).     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/03/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FTPCHCVTH__
#define __FTPCHCVTH__

#include "BTTypes.h"            /* Bluetooth Type Definitions/Constants.      */

   /* The following function is a utility function that converts the    */
   /* specified UTF-16 string (with each character considered a BIG     */
   /* ENDIAN non-aligned 16 bit value) into a UTF-8 encoded string.  The*/
   /* conversion is done IN PLACE and it will not be extended past the  */
   /* specified buffer size (i.e. it will be truncated if the resultant */
   /* string is too large).                                             */
   /* * NOTE * This function is NOT generic and it's use is tailored to */
   /*          how OTP/GOEP/OBEX uses it (hence the BIG ENDIAN          */
   /*          non-aligned format).                                     */
Boolean_t ConvertUTF16ToUTF8(Byte_t *UTF16String, unsigned int UTF16StringLength, unsigned int UTF16StringBufferSize);

   /* The following function is a utility function that converts the    */
   /* specified UTF-8 string (considered non NULL terminated) of        */
   /* UTF8StringLength to a UTF-16 string (with each character          */
   /* considered a BIG ENDIAN non-aligned 16 bit value).                */
   /* * NOTE * This function is NOT generic and it's use is tailored to */
   /*          how OTP/GOEP/OBEX uses it (hence the BIG ENDIAN          */
   /*          non-aligned format).                                     */
Boolean_t ConvertUTF8ToUTF16(Byte_t *UTF8String, unsigned int UTF8StringLength, Byte_t *UTF16String, unsigned int UTF16BufferSize);

#endif

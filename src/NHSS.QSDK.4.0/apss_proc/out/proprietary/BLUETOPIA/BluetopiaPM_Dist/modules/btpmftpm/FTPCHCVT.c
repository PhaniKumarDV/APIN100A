/*****< ftpchcvt.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FTPCHCVT - Stonestreet One Bluetooth Stack Platform Manager Object        */
/*             Transfer Protocol Character Conversion Implementation (UTF-8   */
/*             <-> UTF-16).                                                   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/03/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "FTPCHCVT.h"           /* Module Prototype Defintions/Constants.     */

   /* The following function is a utility function that converts the    */
   /* specified UTF-16 string (with each character considered a BIG     */
   /* ENDIAN non-aligned 16 bit value) into a UTF-8 encoded string.  The*/
   /* conversion is done IN PLACE and it will not be extended past the  */
   /* specified buffer size (i.e. it will be truncated if the resultant */
   /* string is too large).                                             */
   /* * NOTE * This function is NOT generic and it's use is tailored to */
   /*          how OTP/GOEP/OBEX uses it (hence the BIG ENDIAN          */
   /*          non-aligned format).                                     */
Boolean_t ConvertUTF16ToUTF8(Byte_t *UTF16String, unsigned int UTF16StringLength, unsigned int UTF16StringBufferSize)
{
   Byte_t    *UTF8String;
   Boolean_t  ret_val;

   if((UTF16String) && (UTF16StringBufferSize))
   {
      UTF8String = UTF16String;

      if(UTF16StringLength > UTF16StringBufferSize)
         UTF16StringLength = UTF16StringBufferSize;

      while(UTF16StringLength--)
      {
         *UTF8String = (Byte_t)(READ_UNALIGNED_WORD_BIG_ENDIAN(UTF16String));

         UTF8String++;
         UTF16String += 2;
      }

      ret_val = TRUE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is a utility function that converts the    */
   /* specified UTF-8 string (considered non NULL terminated) of        */
   /* UTF8StringLength to a UTF-16 string (with each character          */
   /* considered a BIG ENDIAN non-aligned 16 bit value).                */
   /* * NOTE * This function is NOT generic and it's use is tailored to */
   /*          how OTP/GOEP/OBEX uses it (hence the BIG ENDIAN          */
   /*          non-aligned format).                                     */
Boolean_t ConvertUTF8ToUTF16(Byte_t *UTF8String, unsigned int UTF8StringLength, Byte_t *UTF16String, unsigned int UTF16BufferSize)
{
   Boolean_t ret_val;

   /* First, make sure the input parameters appear to be semi-valid.    */
   if((UTF8String) && (UTF16BufferSize >= (UTF8StringLength*2)))
   {
      /* For each character in the Character string, store 2 bytes in   */
      /* the Unicode Text buffer.  The 1st byte will be a 0 and the     */
      /* second byte is a character from the Character sequence.  The   */
      /* Length that is passed into this function should include the    */
      /* Null terminator of the character string.  This will provide a  */
      /* double 0 terminator for the Unicode string.                    */
      while(UTF8StringLength--)
      {
         *UTF16String = 0;
         UTF16String++;

         *UTF16String = (Byte_t)*UTF8String;

         UTF16String++;
         UTF8String++;
      }

      ret_val = TRUE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}


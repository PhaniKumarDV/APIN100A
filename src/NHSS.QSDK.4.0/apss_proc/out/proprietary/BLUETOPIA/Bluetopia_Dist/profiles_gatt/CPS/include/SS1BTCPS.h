/*****< ss1btcps.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SS1BTCPS - Stonestreet One Bluetooth Cycling Power Service (GATT based)   */
/*             Type Definitions, Prototypes, and Constants.                   */
/*                                                                            */
/*  Author:  Zahid Khan                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/25/13  Z. Khan        Initial creation.                               */
/******************************************************************************/
#ifndef __SS1BTCPSH__
#define __SS1BTCPSH__

   /* Force ALL Structure Declarations to be Byte Packed (noting the    */
   /* current Structure Packing).                                       */
   /* * NOTE * This is only done to force backwards compatibility with  */
   /*          older versions of Bluetopia that were built with         */
   /*          with a packing of 1 (byte packed).                       */
#ifdef _MSC_VER

   #pragma pack(push, __SS1BTCPS_PUSH__)
   #pragma pack(1)

#endif

#include "CPSAPI.h"      /* Bluetooth CPS API Prototypes/Constants.     */
#include "CPSTypes.h"    /* Bluetooth CPS Service Types.                */

   /* Restore Structure Packing.                                        */
#ifdef _MSC_VER

   #pragma pack(pop, __SS1BTCPS_PUSH__)

#endif

#endif

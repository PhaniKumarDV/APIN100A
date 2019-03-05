/*****< hdpmtype.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDPMTYPE - Health Device Manager API Type Definitions and Constants for   */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HDPMTYPEH__
#define __HDPMTYPEH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "HDPMMSG.h"             /* BTPM HDP Manager Message Formats.         */

   /* The following structure is used to define the information that    */
   /* describes an HDP Endpoint. The EndpointID is used to identify     */
   /* an endpoint for the HDP Instance. The Data Type is the Device     */
   /* Data Synchronization Code used by the endpoint, which defines the */
   /* type of data supported by this endpoint, and is assigned by the   */
   /* Bluetooth SIG. The Role indicates whether the endpoint is a source*/
   /* or sync.                                                          */
typedef struct _tagHDPM_Endpoint_Info_t
{
   Byte_t            EndpointID;
   Word_t            DataType;
   HDP_Device_Role_t Role;
} HDPM_Endpoint_Info_t;

#define HDPM_ENDPOINT_INFO_DATA_SIZE                           (sizeof(HDPM_Endpoint_Info_t))

#endif

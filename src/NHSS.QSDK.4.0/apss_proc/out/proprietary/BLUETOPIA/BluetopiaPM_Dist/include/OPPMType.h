/*****< oppmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPMTYPE - Object Push Manager API Type Definitions and Constants         */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/05/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __OPPMTYPEH__
#define __OPPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */
#include "SS1BTOPP.h"     /* Object Push Prototypes/Constants                 */

   /* The following enum represents the types of supported OPP objects. */
typedef enum 
{
   obtvCard,
   obtvCalendar,
   obtiCalendar,
   obtvNote,
   obtvMessage,
   obtUnknownObject
} OPPM_Object_Type_t;

   /* The following constants represent the error codes that can be     */
   /* returned by the server.                                           */
#define OPPM_RESPONSE_STATUS_CODE_SUCCESS                      0x00000000
#define OPPM_RESPONSE_STATUS_CODE_NOT_FOUND                    0x00000001
#define OPPM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE          0x00000002
#define OPPM_RESPONSE_STATUS_CODE_BAD_REQUEST                  0x00000003
#define OPPM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED              0x00000004
#define OPPM_RESPONSE_STATUS_CODE_UNAUTHORIZED                 0x00000005
#define OPPM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED          0x00000006
#define OPPM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE               0x00000007
#define OPPM_RESPONSE_STATUS_CODE_FORBIDDEN                    0x00000008
#define OPPM_RESPONSE_STATUS_CODE_SERVER_ERROR                 0x00000009
#define OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED            0x0000000A
#define OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES  0x0000000B
#define OPPM_RESPONSE_STATUS_CODE_DEVICE_POWER_OFF             0x0000000C
#define OPPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST     0x0000000D
#define OPPM_RESPONSE_STATUS_CODE_UNKNOWN                      0x0000000E

   /* The following structure represents the profile-specific data      */
   /* advertised for a single Object Push Server.                       */
typedef struct _tagOPPM_Parsed_Service_Details_t
{
   unsigned int   ServerPort;
   unsigned long  SupportedObjectTypes;
   char          *ServiceName;
} OPPM_Parsed_Service_Details_t;

   /* The following structure is used with the                          */
   /* OPPM_Query_Remote_Object_Push_Service_Info() and                  */
   /* OPPM_Free_Object_Push_Service_Info() functions to query the       */
   /* details of the profile services offered by the remote device.     */
typedef struct _tagOPPM_Parsed_Service_Info_t
{
   unsigned int                   NumberServices;
   OPPM_Parsed_Service_Details_t *ServiceDetails;
   void                          *RESERVED;
} OPPM_Parsed_Service_Info_t;

   /* The following bit flags represent the types of supported objects  */
   /* that can be represented in an Object Push Service Record.         */
#define OPPM_OBJECT_TYPE_VCARD2_1                              (OPP_SUPPORTED_FORMAT_VCARD_2_1)
#define OPPM_OBJECT_TYPE_VCARD3_0                              (OPP_SUPPORTED_FORMAT_VCARD_3_0)
#define OPPM_OBJECT_TYPE_VCALENDAR1_0                          (OPP_SUPPORTED_FORMAT_VCALENDAR_1_0)
#define OPPM_OBJECT_TYPE_ICALENDAR1_0                          (OPP_SUPPORTED_FORMAT_ICALENDAR_1_0)
#define OPPM_OBJECT_TYPE_VNOTE                                 (OPP_SUPPORTED_FORMAT_VNOTE)
#define OPPM_OBJECT_TYPE_VMESSAGE                              (OPP_SUPPORTED_FORMAT_VMESSAGE)
#define OPPM_OBJECT_TYPE_ANY_OBJECT                            (OPP_SUPPORTED_FORMAT_ALL_OBJECTS)

#endif

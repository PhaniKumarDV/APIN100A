/*****< mapmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MAPMTYPE - Message Access Manager API Type Definitions and Constants      */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/25/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __MAPMTYPEH__
#define __MAPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "MAPMMSG.h"      /* BTPM Message Access Manager Message Formats.     */

   /* The following enumerated type is used to indicate the specific    */
   /* type of Message Access connection.  A notification value indicates*/
   /* the connection is for a Notification Server.  A Server value      */
   /* indicates the the connection is an incoming connection on the     */
   /* local MAS Server.  A client value indicates the connection is an  */
   /* outgoing connection to a remote MAS Server.                       */
typedef enum
{
   mctNotificationServer,
   mctNotificationClient,
   mctMessageAccessServer,
   mctMessageAccessClient
} MAPM_Connection_Type_t;

   /* The following constants represent the minimum and maximum Intance */
   /* ID values that can be specified for Instance ID's (respectively). */
#define MAPM_INSTANCE_ID_MINIMUM_VALUE                         0x00000000
#define MAPM_INSTANCE_ID_MAXIMUM_VALUE                         0x000000FF

   /* The following constants represent the error codes that can be     */
   /* returned by the server.                                           */
#define MAPM_RESPONSE_STATUS_CODE_SUCCESS                      0x00000000
#define MAPM_RESPONSE_STATUS_CODE_NOT_FOUND                    0x00000001
#define MAPM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE          0x00000002
#define MAPM_RESPONSE_STATUS_CODE_BAD_REQUEST                  0x00000003
#define MAPM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED              0x00000004
#define MAPM_RESPONSE_STATUS_CODE_UNAUTHORIZED                 0x00000005
#define MAPM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED          0x00000006
#define MAPM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE               0x00000007
#define MAPM_RESPONSE_STATUS_CODE_FORBIDDEN                    0x00000008
#define MAPM_RESPONSE_STATUS_CODE_SERVER_ERROR                 0x00000009
#define MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED            0x0000000A
#define MAPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES  0x0000000B
#define MAPM_RESPONSE_STATUS_CODE_DEVICE_POWER_OFF             0x0000000C
#define MAPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST     0x0000000D
#define MAPM_RESPONSE_STATUS_CODE_UNKNOWN                      0x0000000E

   /* The following structure represents the profile-specific data      */
   /* advertised for a single Message Access Server.                    */
typedef struct _tagMAPM_Message_Access_Service_Details_t
{
   unsigned int   ServerPort;
   unsigned int   InstanceID;
   unsigned long  SupportedMessageTypes;
   char          *ServiceName;
} MAPM_MAS_Service_Details_t;

   /* The following structure is used with the                          */
   /* MAPM_Query_Remote_Message_Access_Service_Info() and               */
   /* MAPM_Free_Message_Access_Service_Info() functions to query the    */
   /* details of profile services offered by the remote device.         */
typedef struct _tagMAPM_Message_Access_Services_t
{
   unsigned int                NumberServices;
   MAPM_MAS_Service_Details_t *ServiceDetails;
   void                       *RESERVED;
} MAPM_Parsed_Message_Access_Service_Info_t;

#endif


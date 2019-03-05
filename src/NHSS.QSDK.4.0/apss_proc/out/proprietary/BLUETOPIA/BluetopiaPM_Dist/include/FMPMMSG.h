/*****< fmpmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FMPMMSG - Defined Interprocess Communication Messages for the Find Me     */
/*            Profile (FMP) Manager for Stonestreet One Bluetopia Protocol    */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FMPMMSGH__
#define __FMPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTFMPM.h"           /* FMP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "FMPMType.h"            /* BTPM FMP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Find Me Profile */
   /* (FMP) Manager.                                                    */
#define BTPM_MESSAGE_GROUP_FIND_ME_MANAGER                     0x00001104

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Find Me Profile  */
   /* (FMP) Manager.                                                    */

   /* Find Me Profile (FMP) Manager Commands.                           */
#define FMPM_MESSAGE_FUNCTION_REGISTER_TARGET_EVENTS           0x00001001
#define FMPM_MESSAGE_FUNCTION_UN_REGISTER_TARGET_EVENTS        0x00001002

   /* Find Me Profile (FMP) Manager Asynchronous Events.                */
#define FMPM_MESSAGE_FUNCTION_FMP_ALERT                        0x00010001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Find Me Profile (FMP)   */
   /* Manager.                                                          */

   /* Find Me Profile (FMP) Manager Command/Response Message Formats.   */

   /* The following structure represents the Message definition for a   */
   /* FMP Manager Message to register for FMP Target Manager events     */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             FMPM_MESSAGE_FUNCTION_REGISTER_TARGET_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagFMPM_Register_Target_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} FMPM_Register_Target_Events_Request_t;

#define FMPM_REGISTER_TARGET_EVENTS_REQUEST_SIZE               (sizeof(FMPM_Register_Target_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* FMP Manager Message to register for FMP Target Manager events     */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             FMPM_MESSAGE_FUNCTION_REGISTER_TARGET_EVENTS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagFMPM_Register_Target_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          FMPTargetEventHandlerID;
} FMPM_Register_Target_Events_Response_t;

#define FMPM_REGISTER_TARGET_EVENTS_RESPONSE_SIZE              (sizeof(FMPM_Register_Target_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* FMP Manager Message to un-register for FMP Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             FMPM_MESSAGE_FUNCTION_UN_REGISTER_TARGET_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagFMPM_Un_Register_Target_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          FMPTargetEventHandlerID;
} FMPM_Un_Register_Target_Events_Request_t;

#define FMPM_UN_REGISTER_TARGET_EVENTS_REQUEST_SIZE            (sizeof(FMPM_Un_Register_Target_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* FMP Manager Message to un-register for FMP Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             FMPM_MESSAGE_FUNCTION_UN_REGISTER_TARGET_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagFMPM_Un_Register_Target_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} FMPM_Un_Register_Target_Events_Response_t;

#define FMPM_UN_REGISTER_TARGET_EVENTS_RESPONSE_SIZE           (sizeof(FMPM_Un_Register_Target_Events_Response_t))

   /* FMP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* FMP Manager Message that informs the client that a FMP Alert has  */
   /* been issued (asynchronously).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             FMPM_MESSAGE_FUNCTION_FMP_ALERT                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagFMPM_Alert_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           TargetEventHandlerID;
   FMPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   FMPM_Alert_Level_t     AlertLevel;
} FMPM_Alert_Message_t;

#define FMPM_ALERT_MESSAGE_SIZE                                (sizeof(FMPM_Alert_Message_t))

#endif


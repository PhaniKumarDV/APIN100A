/*****< btpmmsgt.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMSGT - Interprocess Communication Abstraction layer Message Format    */
/*            for Stonestreet One Bluetooth Protocol Stack Platform Manager.  */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/04/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMMSGTH__
#define __BTPMMSGTH__

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

   /* The following structure defines the message header format of ALL  */
   /* messages that are exchanged between the Bluetopia Platform Manager*/
   /* Server and Clients.                                               */
   /* * NOTE * The first member, the AddressID, is required and is used */
   /*          to store the Source/Destination Address ID of the        */
   /*          Message.                                                 */
   /* * NOTE * The MessageLength member is the total Message Length size*/
   /*          EXCLUDING the Message Header (i.e. none of the bytes in  */
   /*          in this Header (or any padding bytes) are accounted for  */
   /*          in this Length.                                          */
typedef struct _tagBTPM_Message_Header_t
{
   unsigned int AddressID;
   unsigned int MessageID;
   unsigned int MessageGroup;
   unsigned int MessageFunction;
   unsigned int MessageLength;
} BTPM_Message_Header_t;

#define BTPM_MESSAGE_HEADER_SIZE                               (sizeof(BTPM_Message_Header_t))

   /* The following bit-mask constants define Bit Mask values that are  */
   /* applied to the MessageID member of the BTPM Message Header (this  */
   /* header is the beginning part of ALL messages that are exchanged   */
   /* between the Bluetopia Platform Manager Server and Clients.        */
#define BTPM_MESSAGE_MESSAGE_ID_MESSAGE_ID_MASK                0x7FFFFFFF
#define BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK                  0x80000000

   /* The following constants represent the minimum and maximum Message */
   /* Group values.  Any values outside of this range are reserved and  */
   /* cannot be used.                                                   */
#define BTPM_MESSAGE_GROUP_MINIMUM                             0x00000100
#define BTPM_MESSAGE_GROUP_MAXIMUM                             0x0000FFFF

   /* The following constants represent the minimum and maximum Message */
   /* Function values.  Any values outside of this range are reserved   */
   /* and cannot be used.                                               */
   /* * NOTE * Values that are located below the Minimum value are      */
   /*          reserved by the Messaging System and will be dispatched  */
   /*          to all Message Handlers for specific tasks.  These       */
   /*          Functions (and the Message Format) are defined elsewhere */
   /*          in this file.                                            */
#define BTPM_MESSAGE_FUNCTION_MINIMUM                          0x00001000
#define BTPM_MESSAGE_FUNCTION_MAXIMUM                          0xFFFFFFFF

   /* The following Message Function constant represents the reserved   */
   /* Bluetopia Platform Manager Message that signifies a specific      */
   /* Address has been Registered or Un-Registered.                     */
#define BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION              0x00000001

   /* The following Message Function constant represents the reserved   */
   /* Bluetopia Platform Manager Message that is used internally by     */
   /* Bluetopia Platform Manager to inform a client that an error       */
   /* occurred while trying to process a received message.  This is a   */
   /* a general message that will contain information about a received  */
   /* message that was not able to be processed (such as the reason).   */
#define BTPM_MESSAGE_FUNCTION_CLIENT_ERROR                     0x00000011

   /* The following structure defines the message format of ALL messages*/
   /* that are exchanged between the Bluetopia Platform Manager Server  */
   /* and Clients.                                                      */
typedef struct _tagBTPM_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned char         VariableData[1];
} BTPM_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire BTPM Message (Header and Data*/
   /* Payload) given the number of Payload bytes.  This function accepts*/
   /* as it's input the total number of Data Payload bytes that are     */
   /* present starting from the VariableData member of the              */
   /* BTPM_Message_t structure and returns the total number of bytes    */
   /* required to hold the Message Header and the Message Data.         */
#define BTPM_MESSAGE_SIZE(_x)                                  (STRUCTURE_OFFSET(BTPM_Message_t, VariableData) + (unsigned int)(_x))

   /* The following constants and/or definitions define specific        */
   /* Messages that Message Handlers must process to handle Bluetopia   */
   /* Platform Manager specific functionality.                          */

   /* The following structure represents the Message definition for a   */
   /* Client Registration/Un-Registration.  This message is dispatched  */
   /* whenever a new Client is registered or un-registered.  This allows*/
   /* the ability for Message Handlers to clean up any resources that   */
   /* might have been allocated for a specific Client Address (specified*/
   /* in the Message).  The final member, Registered, is a BOOLEAN value*/
   /* that specifies if the specified Address ID is Registered (TRUE) or*/
   /* Un-Registered (FALSE).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBTPM_Client_Registration_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          AddressID;
   Boolean_t             Registered;
} BTPM_Client_Registration_Message_t;

#define BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE            (sizeof(BTPM_Client_Registration_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Client Error Message.  This message is dispatched from the server */
   /* (AND ONLY BY THE SERVER) when an error occurs while trying to     */
   /* dispatch a particular message.  This structure contains the error */
   /* code and the information about the message that the error         */
   /* occurred.                                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BTPM_MESSAGE_FUNCTION_CLIENT_ERROR                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBTPM_Client_Error_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ClientError;
   unsigned int          AddressID;
   unsigned int          MessageID;
   unsigned int          MessageGroup;
   unsigned int          MessageFunction;
} BTPM_Client_Error_Message_t;

#define BTPM_CLIENT_ERROR_MESSAGE_SIZE                         (sizeof(BTPM_Client_Error_Message_t))

   /* The following constants represent the defined Error codes that can*/
   /* be present in the ClientError member of the                       */
   /* BTPM_Client_Error_Message_t structure.  These values represent the*/
   /* error type that occurred for the specified message.               */
#define BTPM_CLIENT_ERROR_MESSAGE_ERROR_HANDLER_NOT_REGISTERED 0x00000001

#endif

/*****< msgapi.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  MSGAPI - Interprocess Communication Abstraction layer Message Handling    */
/*           API for Stonestreet One Bluetooth Protocol Stack Platform        */
/*           Manager.                                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/04/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __MSGAPIH__
#define __MSGAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Message Sub-system is         */
   /* initialized.                                                      */
typedef struct _tagMSG_Initialization_Data_t
{
   void *PlatformSpecificInitData;
} MSG_Initialization_Data_t;

#define MSG_INITIALIZATION_DATA_SIZE                        (sizeof(MSG_Initialization_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a Message Callback.  This function will be called whenever an IPC */
   /* Message is received.  This function passes to the caller the IPC  */
   /* Message and the Message Callback Parameter that was specified when*/
   /* this Callback was installed.  This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  Because of this, the processing in this function     */
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another Message will not be processed while this function call is */
   /* outstanding).                                                     */
   /* ** NOTE ** The callee is responsible for freeing the message      */
   /*            that is passed ONLY FOR Message Groups that were       */
   /*            registered.  This means that any messages that are     */
   /*            received that are within the following range:          */
   /*               - BTPM_MESSAGE_GROUP_MINIMUM                        */
   /*               - BTPM_MESSAGE_GROUP_MAXIMUM                        */
   /*            require that the callee *MUST* call the                */
   /*            MSG_FreeReceivedMessageGroupHandlerMessage() function  */
   /*            when finished processing the message.  Failure to do   */
   /*            this will result in a leak.  Any messages that ARE NOT */
   /*            in the above range are owned by the Message Dispatcher */
   /*            and *MUST* be copied into another buffer if the        */
   /*            contents of the Message are required to be used AFTER  */
   /*            this function returns.  This means that for ANY        */
   /*            message within the range above, the callee owns the    */
   /*            message, outside that range, the caller owns the       */
   /*            message.                                               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Messages.  A  */
   /*            deadlock WILL occur because NO Message Callbacks will  */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *MSG_Message_Callback_t)(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is provided to allow a mechanism for       */
   /* callers of this module to ascertain the Broadcast Address ID that */
   /* is used by the Messaging System.                                  */
BTPSAPI_DECLARATION unsigned int BTPSAPI MSG_GetBroadcastAddressID(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MSG_GetBroadcastAddressID_t)(void);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* callers of this module to ascertain the Address ID of the IPC     */
   /* Server that is used by the Messaging System.                      */
BTPSAPI_DECLARATION unsigned int BTPSAPI MSG_GetServerAddressID(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MSG_GetServerAddressID_t)(void);
#endif

   /* The following function is responsible for registering a Callback  */
   /* to handle the specified BTPM Message Group.  If this function is  */
   /* successful, then this callback will be invoked (by this module)   */
   /* whenever a message with the specified Message Group is received by*/
   /* the Message system.  This function accepts as input, the Message  */
   /* Group that is being registered, followed by the actual Callback   */
   /* and Callback parameter (passed to the Callback Function) that is  */
   /* to be invoked when a Message of the specified Message Group is    */
   /* received.  This function returns zero if successful, or a negative*/
   /* return error code if there was an error.                          */
   /* * NOTE * See descriptions of the Callback Function and the        */
   /*          MSG_FreeReceivedMessageGroupHandlerMessage() function for*/
   /*          important information regarding Message Ownership !!!!!! */
BTPSAPI_DECLARATION int BTPSAPI MSG_RegisterMessageGroupHandler(unsigned int MessageGroup, MSG_Message_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MSG_RegisterMessageGroupHandler_t)(unsigned int MessageGroup, MSG_Message_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is responsible for un-registering a        */
   /* previously registered Message Group Handler that was registered   */
   /* via a successful call to the MSG_RegisterMessageGroupHandler()    */
   /* function.  Once this function call is made, no further Message    */
   /* Handlers for the specified Message Group will be invoked upon     */
   /* receiving a Message for the specified Message Group.              */
BTPSAPI_DECLARATION void BTPSAPI MSG_UnRegisterMessageGroupHandler(unsigned int MessageGroup);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_MSG_UnRegisterMessageGroupHandler_t)(unsigned int MessageGroup);
#endif

   /* The following function is the function that *MUST* be called for  */
   /* every received message (through a Message Group Handler that was  */
   /* registered with the MSG_RegisterMessageGroupHandler() function).  */
   /* After this function is called the Message can NO LONGER be        */
   /* referenced or accessed.  This function accepts as input a pointer */
   /* to a message that was received from a Registered Message Group    */
   /* Handler.                                                          */
   /* * NOTE * Messages that are ONLY within the following range:       */
   /*             - BTPM_MESSAGE_GROUP_MINIMUM                          */
   /*             - BTPM_MESSAGE_GROUP_MAXIMUM                          */
   /*          and                                                      */
   /*             - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
   /*             - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
   /*          should be passed to this function.  Messages outside of  */
   /*          this range SHOULD NEVER be passed to this function       */
   /*          because they are owned by the Message Sub-system.        */
BTPSAPI_DECLARATION void BTPSAPI MSG_FreeReceivedMessageGroupHandlerMessage(BTPM_Message_t *Message);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_MSG_FreeReceivedMessageGroupHandlerMessage_t)(BTPM_Message_t *Message);
#endif

   /* The following function is provided to allow a mechanism to fetch a*/
   /* a unique Message ID that can be used when sending messages.  This */
   /* ID is guaranteed to be unique so that responses can be matched up */
   /* with requests in a reliable manner.                               */
   /* * NOTE * This function *MUST* be used when specifying a Message   */
   /*          to send with MSG_SendMessage().                          */
BTPSAPI_DECLARATION int BTPSAPI MSG_GetNextMessageID(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MSG_GetNextMessageID_t)(void);
#endif

   /* The following function is responsible for sending the specified   */
   /* BTPM Message through the Bluetopia Platform Manager services.     */
   /* This function accepts the message that is to be sent (must be     */
   /* preformatted and be valid).  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI MSG_SendMessage(BTPM_Message_t *Message);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MSG_SendMessage_t)(BTPM_Message_t *Message);
#endif

   /* The following function is responsible for sending the specified   */
   /* BTPM Message through the Bluetopia Platform Manager services.     */
   /* This function can (optionally) wait for a response to the         */
   /* specified message and return this result as well.  This function  */
   /* accepts as input the Message to send (must be Non-NULL), followed */
   /* by a timeout (specified in milli-seconds) to wait for a response  */
   /* to the message.  The final parameter specifies a pointer to a     */
   /* Message structure that will contain the Message result if there   */
   /* was a response to the message.  If zero is specified for the      */
   /* Timeout parameter than this function will NOT wait for a response */
   /* and will simply return the result of sending the message.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error sending/receiving the message.         */
BTPSAPI_DECLARATION int BTPSAPI MSG_SendMessageResponse(BTPM_Message_t *SendMessage, unsigned int MessageTimeout, BTPM_Message_t **ResponseMessage);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_MSG_SendMessageResponse_t)(BTPM_Message_t *SendMessage, unsigned int MessageTimeout, BTPM_Message_t **ResponseMessage);
#endif

#endif

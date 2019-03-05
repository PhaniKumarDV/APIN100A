/*****< audmutil.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDMUTIL - Audio Manager Utility functions for Stonestreet One Bluetooth  */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/31/10  D. Lange       Initial creation.                               */
/*   10/21/13  S. Hishmeh     Added AVRCP 1.4 Support.                        */
/******************************************************************************/
#ifndef __AUDMUTILH__
#define __AUDMUTILH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTAUD.h"            /* Audio Framework Prototypes/Constants.     */
#include "SS1BTAVR.h"            /* Bluetopia's AVRCP API Type Definitions.   */

// TODO: comments.
typedef struct _tagRemoteControlDecodeInformation_t
{
   AVRCP_Message_Information_t  MessageInformation;
   void                        *FragmentBuffer;
} RemoteControlDecodeInformation_t;

   /* The following function is a utility function that exists to       */
   /* convert the specified Decoded AVRCP Command to it's Stream Format.*/
   /* This format can be used to send the message through the messaging */
   /* sub-system.  This function accepts as input the AVRCP Command Data*/
   /* to convert, followed by the buffer length and buffer to convert   */
   /* the data into.  This function returns the total number of bytes   */
   /* that were copied into the specified input buffer (greater than    */
   /* zero) if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * If the final two parameters are passed as zero and NULL  */
   /*          (respectively), this instructs this function to determine*/
   /*          the total length (in bytes) that the data buffer must be */
   /*          to hold the converted Command.                           */
int ConvertDecodedAVRCPCommandToStream(AUD_Remote_Control_Command_Data_t *CommandData, unsigned int BufferLength, unsigned char *Buffer);

   /* The following function is a utility function that exists to       */
   /* convert the specified Decoded AVRCP Command Response to it's      */
   /* Stream Format.  This format can be used to send the message       */
   /* through the messaging sub-system.  This function accepts as input */
   /* the AVRCP Command Response Data to convert, followed by the buffer*/
   /* length and buffer to convert the data into.  This function returns*/
   /* the total number of bytes that were copied into the specified     */
   /* input buffer (greater than zero) if successful, or a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * If the final two parameters are passed as zero and NULL  */
   /*          (respectively), this instructs this function to determine*/
   /*          the total length (in bytes) that the data buffer must be */
   /*          to hold the converted Command Response.                  */
int ConvertDecodedAVRCPResponseToStream(AUD_Remote_Control_Response_Data_t *ResponseData, unsigned int BufferLength, unsigned char *Buffer);

   /* The following function is a utility function that exists to       */
   /* convert the specified AVRCP Stream Command to an AVRCP Decoded    */
   /* Command.  This function is useful for converting Command Data that*/
   /* has been received through the messaging sub-system into the format*/
   /* required by the Audio Manager.  This function accepts the Message */
   /* Type of the Message to convert, followed by the length and a      */
   /* pointer to the AVRCP stream data, followed by a pointer to Remote */
   /* Control Decode Information Structure, followed by a pointer to a  */
   /* buffer that will hold the Decoded data.  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * After a successful call, and after the caller has        */
   /*          finished using the decoded data, the caller should call  */
   /*          FreeAVRCPDecodedCommand() to free the memory allocated by*/
   /*          this function.                                           */
int ConvertStreamAVRCPCommandToDecoded(AVRCP_Message_Type_t MessageType, unsigned int BufferLength, unsigned char *Buffer, RemoteControlDecodeInformation_t *DecodeInformation, AUD_Remote_Control_Command_Data_t *CommandData);

   /* The following function frees all memory allocated by a successful */
   /* call to ConvertStreamAVRCPCommandToDecoded().                     */
void FreeAVRCPDecodedCommand(RemoteControlDecodeInformation_t *DecodeInformation);

   /* The following function is a utility function that exists to       */
   /* convert the specified AVRCP Stream Command Response to an AVRCP   */
   /* Decoded Command Response.  This function is useful for converting */
   /* Command Response Data that has been received through the messaging*/
   /* sub-system into the format required by the Audio Manager.  This   */
   /* function accepts the Message Type of the Message to convert,      */
   /* followed by the length and a pointer to the AVRCP stream data,    */
   /* followed by a pointer to Remote Control Decode Information        */
   /* Structure, followed by a pointer to a buffer that will hold the   */
   /* Decoded data.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * After a successful call, and after the caller has        */
   /*          finished using the decoded data, the caller should call  */
   /*          FreeAVRCPDecodedResponse() to free the memory allocated  */
   /*          by this function.                                        */
int ConvertStreamAVRCPResponseToDecoded(AVRCP_Message_Type_t MessageType, unsigned int BufferLength, unsigned char *Buffer, RemoteControlDecodeInformation_t *DecodeInformation, AUD_Remote_Control_Response_Data_t *ResponseData);

   /* The following function frees all memory allocated by a successful */
   /* call to ConvertStreamAVRCPResponseToDecoded().                    */
void FreeAVRCPDecodedResponse(RemoteControlDecodeInformation_t *DecodeInformation);

#endif

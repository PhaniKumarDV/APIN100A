/*****< audmutil.c >***********************************************************/
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
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "AUDMAPI.h"
#include "AUDMUTIL.h"            /* BTPM Audio Manager Utility Prototypes.    */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */

   /* The following type definitions specifies the data type that is    */
   /* inserted at the start of every AVRCP 1.4 message.                 */
typedef Word_t FragmentInfo_t;

   /* The following defines specify the flags and fields of the data    */
   /* that is packed in to FragmentInfo_t-typed data.                   */
#define FRAGMENT_INFO_FLAGS_FRAGMENTED             0x8000
#define FRAGMENT_INFO_FLAGS_NUMBER_FRAGMENTS_MASK  0x7FFF

   /* The following type definition specifies the data type that is     */
   /* inserted at the start of an AVRCP fragment.  It is used to specify*/
   /* the fragment's length.                                            */
typedef Word_t FragmentLength_t;

   /* The following definition specifies the header size of every AVRCP */
   /* IPC message.                                                      */
#define AVRCP_IPC_MESSAGE_HEADER_SIZE              (sizeof(FragmentInfo_t) + sizeof(FragmentLength_t))

   /* The following helper macro returns the minimum of 2 values.       */
#define MIN(_x, _y)                                ((_x) < (_y) ? (_x) : (_y))

   /* The following constant definition specifies the Bluetooth Stack ID*/
   /* that is passed into the AVRCP Message Parsing functions.  The     */
   /* AVRCP Message Parsing functions take the Bluetooth Stack ID as    */
   /* input but do not actually use the ID for anything other than error*/
   /* checking that its value is non-zero.                              */
#define RFA_BLUETOOTH_STACK_ID                     1

   /* Internal Function Prototypes.                                     */
static int FormatUnitInfoCommand(unsigned int BluetoothStackID, AVRCP_Unit_Info_Command_Data_t *UnitInfoCommandData, unsigned int BufferLength, Byte_t *Buffer);
static int MapAVRCPErrorCodeToAUDMErrorCode(int ErrorCode);
static int AddFragmentInfo(int Result, Boolean_t LastFragment, int *in_ret_val, unsigned int *BufferLength, unsigned char **Buffer);
static Boolean_t ServiceRecordContainsServiceClass(SDP_Service_Attribute_Response_Data_t *ServiceRecord, UUID_128_t ServiceClass);
static SDP_Data_Element_t *FindSDPAttribute(SDP_Service_Attribute_Response_Data_t *ServiceRecord, Word_t AttributeID);
static int ConvertSDPDataElementToUUID128(SDP_Data_Element_t DataElement, UUID_128_t *UUID);


   /* The following function is a utility function that exists to format*/
   /* an AVRCP Unit Info Command.  This function accepts the size of the*/
   /* buffer, and the buffer to build the command into.  This function  */
   /* returns the amount of the data that was copied into the specified */
   /* buffer to build the command (if successful, positive).  This      */
   /* function returns a negative return code if there was an error.    */
   /* * NOTE * Passing zero and NULL for the BufferLength and Buffer    */
   /*          parameters (respectively) instructs this function to     */
   /*          simply calculate the number of bytes that are required to*/
   /*          hold the specified command.                              */
   /* * NOTE * This function is added here because AVRCP.c's            */
   /*          implementation of AVRCP_Format_Unit_Info_Command() does  */
   /*          not take an AVRCP_Unit_Info_Command_Data_t pointer as its*/
   /*          input.  That function uses hard coded values for the Unit*/
   /*          Info Command Parameters.                                 */
static int FormatUnitInfoCommand(unsigned int BluetoothStackID, AVRCP_Unit_Info_Command_Data_t *UnitInfoCommandData, unsigned int BufferLength, Byte_t *Buffer)
{
#if BTPS_CONFIGURATION_AVRCP_SUPPORT_CONTROLLER_ROLE

   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure the input parameters are semi-valid.             */
   if((BluetoothStackID) && (UnitInfoCommandData))
   {
      /* Determine if the correct buffer information was passed to this */
      /* function.                                                      */
      if(((!BufferLength) && (!Buffer)) || ((BufferLength >= AVRCP_UNIT_INFO_COMMAND_SIZE) && (Buffer)))
      {
         /* If a buffer was specified, build the command.               */
         if(BufferLength)
         {
            /* Buffer specified, build the command.                     */
            BTPS_MemInitialize(Buffer, 0, AVRCP_UNIT_INFO_COMMAND_SIZE);

            AVRCP_ASSIGN_COMMAND_HEADER_CTYPE(Buffer, UnitInfoCommandData->CommandType);
            AVRCP_ASSIGN_COMMAND_HEADER_SUBUNIT_TYPE(Buffer, UnitInfoCommandData->SubunitType);
            AVRCP_ASSIGN_COMMAND_HEADER_SUBUNIT_ID(Buffer, UnitInfoCommandData->SubunitID);
            AVRCP_ASSIGN_COMMAND_HEADER_OPCODE(Buffer, AVRCP_OPCODE_UNIT_INFO);

            AVRCP_ASSIGN_UNIT_INFO_COMMAND_OPERAND_PADDING(Buffer);
         }

         /* Return the size of the data required for the command.       */
         ret_val = AVRCP_UNIT_INFO_COMMAND_SIZE;
      }
      else
      {
         if(BufferLength)
            ret_val = BTAVRCP_ERROR_BUFFER_TOO_SMALL;
         else
            ret_val = BTAVRCP_ERROR_INVALID_PARAMETER;
      }
   }
   else
      ret_val = BTAVRCP_ERROR_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);

#else

   return(BTPS_ERROR_FEATURE_NOT_AVAILABLE);

#endif
}

   /* The following function maps an AVRCP Bluetopia Error Code to a    */
   /* Bluetopia Platform Manager Error Code.                            */
static int MapAVRCPErrorCodeToAUDMErrorCode(int ErrorCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   switch(ErrorCode)
   {
      case BTAVRCP_ERROR_INVALID_PARAMETER:
      case BTAVRCP_ERROR_INVALID_BLUETOOTH_STACK_ID:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;

      case BTAVRCP_ERROR_NOT_INITIALIZED:
         ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;
         break;

      case BTAVRCP_ERROR_INSUFFICIENT_RESOURCES:
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         break;

      case BTAVRCP_ERROR_MESSAGE_TOO_LONG:
      case BTAVRCP_ERROR_UNABLE_TO_DECODE_MESSAGE:
      case BTAVRCP_ERROR_INVALID_MESSAGE_FRAGMENT_DATA:
      case BTAVRCP_ERROR_UNABLE_TO_DECODE_VENDOR_DEPENDENT:
         ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
         break;

      case BTAVRCP_ERROR_BUFFER_TOO_SMALL:
         ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         break;

      case BTAVRCP_ERROR_BROWSING_CHANNEL_MTU_SIZE_TO_SMALL:
         ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         break;

      default:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following helper function adds fragment information for a     */
   /* single fragment to the stream buffer and updates the state of     */
   /* ret_val, BufferLength, and Buffer.  This function returns 0 on    */
   /* success and a negative error code otherwise.                      */
static int AddFragmentInfo(int Result, Boolean_t LastFragment, int *in_ret_val, unsigned int *BufferLength, unsigned char **Buffer)
{
   int              out_ret_val;
   FragmentLength_t FragmentLength;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if there is room to add the data to the buffer.             */
   if((*BufferLength))
   {
      /* Set the length of the fragment.                                */
      FragmentLength = (FragmentLength_t)Result;

      /* Copy the length information to the buffer.                     */
      BTPS_MemCopy(((*Buffer) - sizeof(FragmentLength_t)), &FragmentLength, sizeof(FragmentLength_t));

      /* Decrement the buffer's length and increment the buffer's       */
      /* pointer.                                                       */
      (*BufferLength) -= Result;
      (*Buffer)       += Result;
   }

   /* Increment the variable that stores the total length of the data in*/
   /* the buffer.                                                       */
   (*in_ret_val) += Result;

   /* Check if this is the last fragment.                               */
   if(LastFragment)
   {
      /* This is the last fragment and there is nothing left to do here,*/
      /* flag success to the caller.                                    */
      out_ret_val = 0;
   }
   else
   {
      /* This is not the last fragment and we need to prepare the next  */
      /* fragment.  First check if the buffer length is non-zero or if  */
      /* the length of the buffer is greater than the length of a       */
      /* fragment length information variable.                          */
      if((!(*BufferLength)) || ((*BufferLength) >= sizeof(FragmentLength_t)))
      {
         /* Check if there is space to add data to the buffer.          */
         if((*BufferLength))
         {
            /* We need to save space in the buffer for the fragment     */
            /* length, decrement the buffer's length and increment the  */
            /* buffer pointer to account for the fragment length.       */
            (*BufferLength) -= sizeof(FragmentLength_t);
            (*Buffer)       += sizeof(FragmentLength_t);
         }

         /* Increment the variable that stores the total length of the  */
         /* data in the buffer.                                         */
         (*in_ret_val) += sizeof(FragmentLength_t);

         /* Flag success to the caller.                                 */
         out_ret_val    = 0;
      }
      else
      {
         out_ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", out_ret_val));

   return(out_ret_val);
}

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
int ConvertDecodedAVRCPCommandToStream(AUD_Remote_Control_Command_Data_t *CommandData, unsigned int BufferLength, unsigned char *Buffer)
{
   int ret_val;
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for valid input.                                            */
   if((CommandData) && ((!BufferLength) || ((BufferLength) && (Buffer))) && (((unsigned int)CommandData->MessageType) <= ((unsigned int)amtGetTotalNumberOfItems)))
   {
      /* The input parameters are valid, next check if this is a        */
      /* pass-through command.                                          */
      if(CommandData->MessageType == amtPassThrough)
      {
         /* This is a pass-through command, handle it differently than  */
         /* the other commands for legacy support in case the client    */
         /* library is updated but the PM server is not or vice versa.  */
         if((!(CommandData->MessageData.PassThroughCommandData.OperationDataLength)) || (CommandData->MessageData.PassThroughCommandData.OperationData))
         {
            /* Initialize the buffer length to the offset of the        */
            /* Operation Data within the Pass-Through Command Data      */
            /* Structure.                                               */
            ret_val = STRUCTURE_OFFSET(AVRCP_Pass_Through_Command_Data_t, OperationData) + CommandData->MessageData.PassThroughCommandData.OperationDataLength;

            /* Check if the user specified a buffer length.             */
            if(BufferLength)
            {
               /* The user specified a buffer length, see if we have    */
               /* enough space in the buffer to add the data.           */
               if(BufferLength >= ((unsigned int)ret_val))
               {
                  /* Buffer is large enough, go ahead and convert the   */
                  /* data.                                              */

                  /* First copy everything EXCEPT the Operation Data.   */
                  BTPS_MemCopy(Buffer, &(CommandData->MessageData.PassThroughCommandData), STRUCTURE_OFFSET(AVRCP_Pass_Through_Command_Data_t, OperationData));

                  /* Now append the Operation Data.                     */
                  if(CommandData->MessageData.PassThroughCommandData.OperationDataLength)
                     BTPS_MemCopy(&(Buffer[STRUCTURE_OFFSET(AVRCP_Pass_Through_Command_Data_t, OperationData)]), CommandData->MessageData.PassThroughCommandData.OperationData, CommandData->MessageData.PassThroughCommandData.OperationDataLength);
               }
               else
               {
                  ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
               }
            }
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
      {
         /* Check if the user specified the buffer length parameter or  */
         /* if the space if the buffer is greater than the IPC message  */
         /* header.                                                     */
         if((!BufferLength) || (BufferLength >= AVRCP_IPC_MESSAGE_HEADER_SIZE))
         {
            /* The user did not specify a buffer length or the buffer   */
            /* length is greater than the size of the IPC Message       */
            /* Header.  Next, check if the user specified the buffer's  */
            /* length.                                                  */
            if(BufferLength)
            {
               /* The user specified the buffer's length, initialize the*/
               /* memory where the IPC Message Header will be stored.   */
               /* Note that IPC Message Header is used to add fragment  */
               /* information to the buffer, however, to date, none of  */
               /* the AVRCP commands can be fragmented, so we just write*/
               /* zeros to the buffer.  The IPC Message Header is left  */
               /* in the IPC buffer here as place holder in case future */
               /* AVRCP specifications add message fragmentation for any*/
               /* of the commands.                                      */
               BTPS_MemInitialize(Buffer, 0x00, AVRCP_IPC_MESSAGE_HEADER_SIZE);

               /* Decrement the buffer's length and increment the buffer*/
               /* pointer.                                              */
               BufferLength -= AVRCP_IPC_MESSAGE_HEADER_SIZE;
               Buffer       += AVRCP_IPC_MESSAGE_HEADER_SIZE;
            }

            /* Initialize the number of bytes in buffer to the size of  */
            /* the IPC Message Header.                                  */
            ret_val = AVRCP_IPC_MESSAGE_HEADER_SIZE;

            switch(CommandData->MessageType)
            {
               case amtUnitInfo:
                  Result = FormatUnitInfoCommand(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.UnitInfoCommandData, BufferLength, Buffer);
                  break;
               case amtSubunitInfo:
                  Result = AVRCP_Format_Subunit_Info_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.SubunitInfoCommandData, BufferLength, Buffer);
                  break;
               case amtVendorDependent_Generic:
                  Result = AVRCP_Format_Vendor_Dependent_Generic_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.VendorDependentGenericCommandData, BufferLength, Buffer);
                  break;
               case amtBrowsingChannel_Generic:
                  Result = AVRCP_Format_Browsing_Channel_Generic_Message(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.BrowsingChannelGenericMessageData, BufferLength, Buffer);
                  break;
               case amtGroupNavigation:
                  Result = AVRCP_Format_Group_Navigation_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GroupNavigationCommandData, BufferLength, Buffer);
                  break;
               case amtGetCapabilities:
                  Result = AVRCP_Format_Get_Capabilities_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GetCapabilitiesCommandData, BufferLength, Buffer);
                  break;
               case amtListPlayerApplicationSettingAttributes:
                  Result = AVRCP_Format_List_Player_Application_Setting_Attributes_Command(RFA_BLUETOOTH_STACK_ID, BufferLength, Buffer);
                  break;
               case amtListPlayerApplicationSettingValues:
                  Result = AVRCP_Format_List_Player_Application_Setting_Values_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.ListPlayerApplicationSettingValuesCommandData, BufferLength, Buffer);
                  break;
               case amtGetCurrentPlayerApplicationSettingValue:
                  Result = AVRCP_Format_Get_Current_Player_Application_Setting_Value_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GetCurrentPlayerApplicationSettingValueCommandData, BufferLength, Buffer);
                  break;
               case amtSetPlayerApplicationSettingValue:
                  Result = AVRCP_Format_Set_Player_Application_Setting_Value_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.SetPlayerApplicationSettingValueCommandData, BufferLength, Buffer);
                  break;
               case amtGetPlayerApplicationSettingAttributeText:
                  Result = AVRCP_Format_Get_Player_Application_Setting_Attribute_Text_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GetPlayerApplicationSettingAttributeTextCommandData, BufferLength, Buffer);
                  break;
               case amtGetPlayerApplicationSettingValueText:
                  Result = AVRCP_Format_Get_Player_Application_Setting_Value_Text_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GetPlayerApplicationSettingValueTextCommandData, BufferLength, Buffer);
                  break;
               case amtInformDisplayableCharacterSet:
                  Result = AVRCP_Format_Inform_Displayable_Character_Set_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.InformDisplayableCharacterSetCommandData, BufferLength, Buffer);
                  break;
               case amtInformBatteryStatusOfCT:
                  Result = AVRCP_Format_Inform_Battery_Status_Of_CT_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.InformBatteryStatusOfCTCommandData, BufferLength, Buffer);
                  break;
               case amtGetElementAttributes:
                  Result = AVRCP_Format_Get_Element_Attributes_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GetElementAttributesCommandData, BufferLength, Buffer);
                  break;
               case amtAbortContinuingResponse:
                  Result = AVRCP_Format_Abort_Continuing_Response_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.AbortContinuingResponseCommandData, BufferLength, Buffer);
                  break;
               case amtGetPlayStatus:
                  Result = AVRCP_Format_Get_Play_Status_Command(RFA_BLUETOOTH_STACK_ID, BufferLength, Buffer);
                  break;
               case amtRegisterNotification:
                  Result = AVRCP_Format_Register_Notification_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.RegisterNotificationCommandData, BufferLength, Buffer);
                  break;
               case amtSetAbsoluteVolume:
                  Result = AVRCP_Format_Set_Absolute_Volume_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.SetAbsoluteVolumeCommandData, BufferLength, Buffer);
                  break;
               case amtSetAddressedPlayer:
                  Result = AVRCP_Format_Set_Addressed_Player_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.SetAddressedPlayerCommandData, BufferLength, Buffer);
                  break;
               case amtPlayItem:
                  Result = AVRCP_Format_Play_Item_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.PlayItemCommandData, BufferLength, Buffer);
                  break;
               case amtAddToNowPlaying:
                  Result = AVRCP_Format_Add_To_Now_Playing_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.AddToNowPlayingCommandData, BufferLength, Buffer);
                  break;
               case amtSetBrowsedPlayer:
                  Result = AVRCP_Format_Set_Browsed_Player_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.SetBrowsedPlayerCommandData, BufferLength, Buffer);
                  break;
               case amtChangePath:
                  Result = AVRCP_Format_Change_Path_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.ChangePathCommandData, BufferLength, Buffer);
                  break;
               case amtGetItemAttributes:
                  Result = AVRCP_Format_Get_Item_Attributes_Command(RFA_BLUETOOTH_STACK_ID, BTPS_CONFIGURATION_AVCTP_MAXIMUM_SUPPORTED_MTU, &CommandData->MessageData.GetItemAttributesCommandData, BufferLength, Buffer);
                  break;
               case amtSearch:
                  Result = AVRCP_Format_Search_Command(RFA_BLUETOOTH_STACK_ID, BTPS_CONFIGURATION_AVCTP_MAXIMUM_SUPPORTED_MTU, &CommandData->MessageData.SearchCommandData, BufferLength, Buffer);
                  break;
               case amtGetFolderItems:
                  Result = AVRCP_Format_Get_Folder_Items_Command(RFA_BLUETOOTH_STACK_ID, BTPS_CONFIGURATION_AVCTP_MAXIMUM_SUPPORTED_MTU, &CommandData->MessageData.GetFolderItemsCommandData, BufferLength, Buffer);
                  break;
               case amtGetTotalNumberOfItems:
                  Result = AVRCP_Format_Get_Total_Number_Of_Items_Command(RFA_BLUETOOTH_STACK_ID, &CommandData->MessageData.GetTotalNumberOfItemsCommandData, BufferLength, Buffer);
                  break;
               default:
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
            }

            /* Check the result of the switch statement above.          */
            if(Result >= 0)
            {
               /* We have a successful result, increment the variable   */
               /* that stores the number of bytes in the buffer.        */
               ret_val += Result;
            }
            else
            {
               /* Check if the return code is less than the first PM    */
               /* error code which means the error code is a Bluetopia  */
               /* error code.                                           */
               if(Result > BTPM_ERROR_CODE_INVALID_PARAMETER)
               {
                  /* This a Bluetopia error code, map it to a PM error  */
                  /* code.                                              */
                  ret_val = MapAVRCPErrorCodeToAUDMErrorCode(Result);
               }
               else
               {
                  /* Set error code so that it is passed back to the    */
                  /* caller.                                            */
                  ret_val = Result;
               }
            }
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         }
      }
   }
   else
   {
      if(CommandData)
      {
         /* An invalid parameter was passed in, determine which         */
         /* parameter caused the error and set the return code          */
         /* appropriately.                                              */
         if(!(((unsigned int)CommandData->MessageType) <= ((unsigned int)amtGeneralReject)))
            ret_val = BTPM_ERROR_CODE_UNKNOWN_REMOTE_CONTROL_EVENT_TYPE;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int ConvertDecodedAVRCPResponseToStream(AUD_Remote_Control_Response_Data_t *ResponseData, unsigned int BufferLength, unsigned char *Buffer)
{
   int                                  ret_val;
   int                                  Result;
   unsigned char                       *BufferStartPtr;
   DWord_t                              NumberFragments;
   FragmentInfo_t                       FragmentInfo;
   AVRCP_Response_Message_State_Info_t  MessageStateInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for valid input.                                            */
   if((ResponseData) && ((!BufferLength) || ((BufferLength) && (Buffer))) && (((unsigned int)ResponseData->MessageType) <= ((unsigned int)amtGetTotalNumberOfItems)))
   {
      /* The input parameters are valid, next check if this is a        */
      /* pass-through command.                                          */
      if(ResponseData->MessageType == amtPassThrough)
      {
         /* This is a pass-through command, handle it differently than  */
         /* the other commands for legacy support in case the client    */
         /* library is updated but the PM server is not or vice versa.  */
         if((!(ResponseData->MessageData.PassThroughResponseData.OperationDataLength)) || (ResponseData->MessageData.PassThroughResponseData.OperationData))
         {
            /* Initialize the buffer length to the offset of the        */
            /* Operation Data within the Pass-Through Command Data      */
            /* Structure.                                               */
            ret_val = STRUCTURE_OFFSET(AVRCP_Pass_Through_Response_Data_t, OperationData) + ResponseData->MessageData.PassThroughResponseData.OperationDataLength;

            /* Check if the user specified a buffer length.             */
            if(BufferLength)
            {
               /* The user specified a buffer length, see if we have    */
               /* enough space in the buffer to add the data.           */
               if(BufferLength >= ((unsigned int)ret_val))
               {
                  /* Buffer is large enough, go ahead and convert the   */
                  /* data.                                              */

                  /* First copy everything EXCEPT the Operation Data.   */
                  BTPS_MemCopy(Buffer, &(ResponseData->MessageData.PassThroughResponseData), STRUCTURE_OFFSET(AVRCP_Pass_Through_Response_Data_t, OperationData));

                  /* Now append the Operation Data.                     */
                  if(ResponseData->MessageData.PassThroughResponseData.OperationDataLength)
                     BTPS_MemCopy(&(Buffer[STRUCTURE_OFFSET(AVRCP_Pass_Through_Response_Data_t, OperationData)]), ResponseData->MessageData.PassThroughResponseData.OperationData, ResponseData->MessageData.PassThroughResponseData.OperationDataLength);
               }
               else
               {
                  ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
               }
            }
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
      }
      else
      {
         /* Check if the user specified the buffer length parameter or  */
         /* if the space if the buffer is greater than the IPC message  */
         /* header.                                                     */
         if((!BufferLength) || (BufferLength >= AVRCP_IPC_MESSAGE_HEADER_SIZE))
         {
            /* The user did not specify a buffer length or the buffer   */
            /* length is greater than the size of the IPC Message       */
            /* Header.  Next, check if the user specified the buffer's  */
            /* length.                                                  */
            if(BufferLength)
            {
               /* The user specified the buffer's length, initialize the*/
               /* memory where the IPC Message Header will be stored.   */
               /* Note that IPC Message Header is used to add fragment  */
               /* information to the buffer.                            */
               BTPS_MemInitialize(Buffer, 0x00, AVRCP_IPC_MESSAGE_HEADER_SIZE);

               /* Save the buffer start, we may need it later if the    */
               /* message is fragmented to add the fragmentation        */
               /* information.                                          */
               BufferStartPtr   = Buffer;

               /* Decrement the buffer's length and increment the buffer*/
               /* pointer.                                              */
               BufferLength    -= AVRCP_IPC_MESSAGE_HEADER_SIZE;
               Buffer          += AVRCP_IPC_MESSAGE_HEADER_SIZE;
            }
            else
            {
               /* Set the buffer start pointer to null so that we can   */
               /* determine if we need add the fragment information to  */
               /* the buffer below.                                     */
               BufferStartPtr = NULL;
            }

            /* Initialize the number of bytes in buffer to the size of  */
            /* the IPC Message Header.                                  */
            ret_val                   = AVRCP_IPC_MESSAGE_HEADER_SIZE;

            /* Initialize the Message State Information that will be    */
            /* used if the message is fragmented.                       */
            MessageStateInfo.Complete = FALSE;
            MessageStateInfo.Offset   = 0;

            /* Initialize the number of fragments and the result code   */
            /* that is used in the switch statement below.              */
            NumberFragments           = 0;
            Result                    = 0;

            switch(ResponseData->MessageType)
            {
               case amtUnitInfo:
                  Result = AVRCP_Format_Unit_Info_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.UnitInfoResponseData, BufferLength, Buffer);
                  break;
               case amtSubunitInfo:
                  Result = AVRCP_Format_Subunit_Info_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.SubunitInfoResponseData, BufferLength, Buffer);
                  break;
               case amtVendorDependent_Generic:
                  Result = AVRCP_Format_Vendor_Dependent_Generic_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.VendorDependentGenericResponseData, BufferLength, Buffer);
                  break;
               case amtBrowsingChannel_Generic:
                  Result = AVRCP_Format_Browsing_Channel_Generic_Message(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.BrowsingChannelGenericMessageData, BufferLength, Buffer);
                  break;
               case amtGroupNavigation:
                  Result = AVRCP_Format_Group_Navigation_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.GroupNavigationResponseData, BufferLength, Buffer);
                  break;
               case amtGetCapabilities:
                  while((!Result) && ((Result = AVRCP_Format_Get_Capabilities_Response(RFA_BLUETOOTH_STACK_ID, &MessageStateInfo, &ResponseData->MessageData.GetCapabilitiesResponseData, BufferLength, Buffer)) > 0) && (!MessageStateInfo.Complete))
                  {
                     if(!(Result = AddFragmentInfo(Result, FALSE, &ret_val, &BufferLength, &Buffer)))
                        NumberFragments++;
                  }
                  break;
               case amtListPlayerApplicationSettingAttributes:
                  Result = AVRCP_Format_List_Player_Application_Setting_Attributes_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.ListPlayerApplicationSettingAttributesResponseData, BufferLength, Buffer);
                  break;
               case amtListPlayerApplicationSettingValues:
                  Result = AVRCP_Format_List_Player_Application_Setting_Values_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.ListPlayerApplicationSettingValuesResponseData, BufferLength, Buffer);
                  break;
               case amtGetCurrentPlayerApplicationSettingValue:
                  while((!Result) && ((Result = AVRCP_Format_Get_Current_Player_Application_Setting_Value_Response(RFA_BLUETOOTH_STACK_ID, &MessageStateInfo, &ResponseData->MessageData.GetCurrentPlayerApplicationSettingValueResponseData, BufferLength, Buffer)) > 0) && (!MessageStateInfo.Complete))
                  {
                     if(!(Result = AddFragmentInfo(Result, FALSE, &ret_val, &BufferLength, &Buffer)))
                        NumberFragments++;
                  }
                  break;
               case amtSetPlayerApplicationSettingValue:
                  Result = AVRCP_Format_Set_Player_Application_Setting_Value_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.SetPlayerApplicationSettingValueResponseData, BufferLength, Buffer);
                  break;
               case amtGetPlayerApplicationSettingAttributeText:
                  while((!Result) && ((Result = AVRCP_Format_Get_Player_Application_Setting_Attribute_Text_Response(RFA_BLUETOOTH_STACK_ID, &MessageStateInfo, &ResponseData->MessageData.GetPlayerApplicationSettingAttributeTextResponseData, BufferLength, Buffer)) > 0) && (!MessageStateInfo.Complete))
                  {
                     if(!(Result = AddFragmentInfo(Result, FALSE, &ret_val, &BufferLength, &Buffer)))
                        NumberFragments++;
                  }
                  break;
               case amtGetPlayerApplicationSettingValueText:
                  while((!Result) && ((Result = AVRCP_Format_Get_Player_Application_Setting_Value_Text_Response(RFA_BLUETOOTH_STACK_ID, &MessageStateInfo, &ResponseData->MessageData.GetPlayerApplicationSettingValueTextResponseData, BufferLength, Buffer)) > 0) && (!MessageStateInfo.Complete))
                  {
                     if(!(Result = AddFragmentInfo(Result, FALSE, &ret_val, &BufferLength, &Buffer)))
                        NumberFragments++;
                  }
                  break;
               case amtInformDisplayableCharacterSet:
                  Result = AVRCP_Format_Inform_Displayable_Character_Set_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.InformDisplayableCharacterSetResponseData, BufferLength, Buffer);
                  break;
               case amtInformBatteryStatusOfCT:
                  Result = AVRCP_Format_Inform_Battery_Status_Of_CT_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.InformBatteryStatusOfCTResponseData, BufferLength, Buffer);
                  break;
               case amtGetElementAttributes:
                  while((!Result) && ((Result = AVRCP_Format_Get_Element_Attributes_Response(RFA_BLUETOOTH_STACK_ID, &MessageStateInfo, &ResponseData->MessageData.GetElementAttributesResponseData, BufferLength, Buffer)) > 0) && (!MessageStateInfo.Complete))
                  {
                     if(!(Result = AddFragmentInfo(Result, FALSE, &ret_val, &BufferLength, &Buffer)))
                        NumberFragments++;
                  }
                  break;
               case amtGetPlayStatus:
                  Result = AVRCP_Format_Get_Play_Status_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.GetPlayStatusResponseData, BufferLength, Buffer);
                  break;
               case amtRegisterNotification:
                  Result = AVRCP_Format_Register_Notification_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.RegisterNotificationResponseData, BufferLength, Buffer);
                  break;
               case amtAbortContinuingResponse:
                  Result = AVRCP_Format_Abort_Continuing_Response_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.AbortContinuingResponseResponseData, BufferLength, Buffer);
                  break;
               case amtSetAbsoluteVolume:
                  Result = AVRCP_Format_Set_Absolute_Volume_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.SetAbsoluteVolumeResponseData, BufferLength, Buffer);
                  break;
               case amtSetAddressedPlayer:
                  Result = AVRCP_Format_Set_Addressed_Player_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.SetAddressedPlayerResponseData, BufferLength, Buffer);
                  break;
               case amtPlayItem:
                  Result = AVRCP_Format_Play_Item_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.PlayItemResponseData, BufferLength, Buffer);
                  break;
               case amtAddToNowPlaying:
                  Result = AVRCP_Format_Add_To_Now_Playing_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.AddToNowPlayingResponseData, BufferLength, Buffer);
                  break;
               case amtCommandRejectResponse:
                  Result = AVRCP_Format_Command_Reject_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.CommandRejectResponseData, BufferLength, Buffer);
                  break;
               case amtSetBrowsedPlayer:
                  Result = AVRCP_Format_Set_Browsed_Player_Response(RFA_BLUETOOTH_STACK_ID, BTPS_CONFIGURATION_AVCTP_MAXIMUM_SUPPORTED_MTU, &ResponseData->MessageData.SetBrowsedPlayerResponseData, BufferLength, Buffer);
                  break;
               case amtChangePath:
                  Result = AVRCP_Format_Change_Path_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.ChangePathResponseData, BufferLength, Buffer);
                  break;
               case amtGetItemAttributes:
                  Result = AVRCP_Format_Get_Item_Attributes_Response(RFA_BLUETOOTH_STACK_ID, BTPS_CONFIGURATION_AVCTP_MAXIMUM_SUPPORTED_MTU, &ResponseData->MessageData.GetItemAttributesResponseData, BufferLength, Buffer);
                  break;
               case amtSearch:
                  Result = AVRCP_Format_Search_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.SearchResponseData, BufferLength, Buffer);
                  break;
               case amtGetFolderItems:
                  Result = AVRCP_Format_Get_Folder_Items_Response(RFA_BLUETOOTH_STACK_ID, BTPS_CONFIGURATION_AVCTP_MAXIMUM_SUPPORTED_MTU, &ResponseData->MessageData.GetFolderItemsResponseData, BufferLength, Buffer);
                  break;
               case amtGeneralReject:
                  Result = AVRCP_Format_General_Reject_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.GeneralRejectResponseData, BufferLength, Buffer);
                  break;
               case amtGetTotalNumberOfItems:
                  Result = AVRCP_Format_Get_Total_Number_Of_Items_Response(RFA_BLUETOOTH_STACK_ID, &ResponseData->MessageData.GetTotalNumberOfItemsResponseData, BufferLength, Buffer);
                  break;
               default:
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  break;
            }

            /* Check the result of the switch statement above.          */
            if(Result >= 0)
            {
               /* We have a successful result, check if this is a       */
               /* fragmented message.                                   */
               if(!NumberFragments)
               {
                  /* This is not a fragmented message, increment the    */
                  /* variable that stores the total length of the data. */
                  ret_val += Result;
               }
               else
               {
                  /* This is a fragmented message, increment the number */
                  /* of fragments.                                      */
                  NumberFragments++;

                  /* Check if the number of fragments is less than or   */
                  /* equal to the maximum possible number of fragments  */
                  /* that will fit into our IPC Message.                */
                  if(NumberFragments <= FRAGMENT_INFO_FLAGS_NUMBER_FRAGMENTS_MASK)
                  {
                     /* The number of fragments will fit into our IPC   */
                     /* Message, add the fragment information to the    */
                     /* buffer.                                         */
                     if(!(Result = AddFragmentInfo(Result, TRUE, &ret_val, &BufferLength, &Buffer)))
                     {
                        /* The fragment's information was successfully  */
                        /* added, next see if we need to add the        */
                        /* message's fragment information to the buffer.*/
                        if(BufferStartPtr)
                        {
                           /* The buffer start pointer was specified,   */
                           /* set the fragment information              */
                           /* appropriately.                            */
                           FragmentInfo  = (FragmentInfo_t)(FRAGMENT_INFO_FLAGS_FRAGMENTED);
                           FragmentInfo |= (FragmentInfo_t)(((FragmentInfo_t)NumberFragments) & FRAGMENT_INFO_FLAGS_NUMBER_FRAGMENTS_MASK);

                           /* Copy the fragment information to the      */
                           /* buffer.                                   */
                           BTPS_MemCopy(BufferStartPtr, &FragmentInfo, sizeof(FragmentInfo_t));
                        }
                     }
                     else
                     {
                        /* The fragment information could not be added, */
                        /* set the return value to the error code.      */
                        ret_val = Result;
                     }
                  }
                  else
                  {
                     ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
                  }
               }
            }
            else
            {
               /* Check if the return code is less than the first PM    */
               /* error code which means the error code is a Bluetopia  */
               /* error code.                                           */
               if(Result > BTPM_ERROR_CODE_INVALID_PARAMETER)
               {
                  /* This a Bluetopia error code, map it to a PM error  */
                  /* code.                                              */
                  ret_val = MapAVRCPErrorCodeToAUDMErrorCode(Result);
               }
               else
               {
                  /* This is not a Bluetopia error code, set the return */
                  /* value to the error code.                           */
                  ret_val = Result;
               }
            }
         }
         else
         {
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         }
      }
   }
   else
   {
      if(ResponseData)
      {
         /* An invalid parameter was passed in, determine which         */
         /* parameter caused the error and set the return code          */
         /* appropriately.                                              */
         if(!(((unsigned int)ResponseData->MessageType) <= ((unsigned int)amtGeneralReject)))
            ret_val = BTPM_ERROR_CODE_UNKNOWN_REMOTE_CONTROL_EVENT_TYPE;
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

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
int ConvertStreamAVRCPCommandToDecoded(AVRCP_Message_Type_t MessageType, unsigned int BufferLength, unsigned char *Buffer, RemoteControlDecodeInformation_t *DecodeInformation, AUD_Remote_Control_Command_Data_t *CommandData)
{
   int            ret_val;
   Boolean_t      BrowsingChannelMessage;
   FragmentInfo_t FragmentInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CommandData) && (BufferLength) && (Buffer) && (DecodeInformation) && (((unsigned int)MessageType) <= ((unsigned int)amtGetTotalNumberOfItems)))
   {
      /* Initialize the decode information that will be used to free any*/
      /* resources that are allocated by the decode process.            */
      DecodeInformation->MessageInformation.MessageType = MessageType;
      DecodeInformation->FragmentBuffer                 = NULL;

      /* Check if this is a pass through command.                       */
      /* * NOTE * We handle pass through commands differently here for  */
      /*          legacy support for those who may using an updated     */
      /*          client library with an older PM server or vice versa. */
      if(MessageType == amtPassThrough)
      {
         /* Next, determine if the buffer is large enough to store a    */
         /* pass-through command.                                       */
         if(BufferLength >= (STRUCTURE_OFFSET(AVRCP_Pass_Through_Command_Data_t, OperationData)))
         {
            /* The buffer is large enough, note the offset of the       */
            /* Operation Data within the Pass-Through Command Structure.*/
            ret_val = (int)(STRUCTURE_OFFSET(AVRCP_Pass_Through_Command_Data_t, OperationData));

            /* Check if the buffer is large enough to specify all of the*/
            /* Operation Data.                                          */
            if(BufferLength >= ((unsigned int)ret_val + ((AVRCP_Pass_Through_Command_Data_t *)Buffer)->OperationDataLength))
            {
               /* The buffer is sufficient, go ahead and convert the    */
               /* data.                                                 */
               CommandData->MessageType = MessageType;

               BTPS_MemCopy(&(CommandData->MessageData.PassThroughCommandData), Buffer, ret_val);

               if(CommandData->MessageData.PassThroughCommandData.OperationDataLength)
                  CommandData->MessageData.PassThroughCommandData.OperationData = &(Buffer[ret_val]);
               else
                  CommandData->MessageData.PassThroughCommandData.OperationData = NULL;

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         }
         else
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
      }
      else
      {
         /* This is not a Pass-Through command, check if the buffer is  */
         /* large enough to store the IPC Message Header.               */
         if(BufferLength >= AVRCP_IPC_MESSAGE_HEADER_SIZE)
         {
            /* Determine if this is a browsing channel message.         */
            switch(MessageType)
            {
               case amtBrowsingChannel_Generic:
               case amtSetBrowsedPlayer:
               case amtChangePath:
               case amtGetItemAttributes:
               case amtSearch:
               case amtGetFolderItems:
               case amtGeneralReject:
               case amtGetTotalNumberOfItems:
                  BrowsingChannelMessage = TRUE;
                  break;

               default:
                  BrowsingChannelMessage = FALSE;
                  break;
            }

            /* Copy the fragment information from the buffer.           */
            BTPS_MemCopy(&FragmentInfo, Buffer, sizeof(FragmentInfo_t));

            /* Decrement the buffer's length and increment the buffer   */
            /* pointer.                                                 */
            BufferLength -= sizeof(FragmentInfo_t);
            Buffer       += sizeof(FragmentInfo_t);

            /* Initialize the return value to 0.                        */
            ret_val       = 0;

            /* Check if this is a fragmented message.                   */
            if(!(FragmentInfo & FRAGMENT_INFO_FLAGS_FRAGMENTED))
            {
               /* This is not a fragmented message, decrement the buffer*/
               /* length and increment the buffer's pointer so that we  */
               /* skip over the un-used, in this case, fragment length  */
               /* information.                                          */
               BufferLength -= sizeof(FragmentLength_t);
               Buffer       += sizeof(FragmentLength_t);

               /* Decode the message.                                   */
               if(!(ret_val = AVRCP_Decode_Message(RFA_BLUETOOTH_STACK_ID, BrowsingChannelMessage, FALSE, BufferLength, Buffer, &DecodeInformation->MessageInformation)))
               {
                  /* Check if we successfully decoded the message.      */
                  if((DecodeInformation->MessageInformation.MessageType != amtFragmentedMessage) && (DecodeInformation->MessageInformation.FinalMessage))
                  {
                     /* The message was successfully decoded, note the  */
                     /* message type.                                   */
                     CommandData->MessageType = MessageType;

                     /* The CommandData->MessageData is a subset of the */
                     /* DecodeInformation->MessageInformation.-         */
                     /* MessageInformation so just copy the             */
                     /* sizeof(CommandData->MessageData) data bytes to  */
                     /* CommandData->MessageData.  Note that we could   */
                     /* use the relevant event's size to determine how  */
                     /* many bytes to copy, but as of the today,        */
                     /* 10/15/2013, the sizeof(CommandData->MessageData)*/
                     /* was only 14 bytes, the CPU might spend more time*/
                     /* in the switch statement to determine the event's*/
                     /* size than it would while it were copying the 14 */
                     /* bytes (as a workaround for an if statement you  */
                     /* could use a look-up-table to determine the size,*/
                     /* but this adds code space and complexity with    */
                     /* very little, if any, benefit).                  */
                     BTPS_MemCopy(&CommandData->MessageData, &DecodeInformation->MessageInformation.MessageInformation, sizeof(CommandData->MessageData));
                  }
                  else
                  {
                     /* The result from AVRCP_Decode_Message() indicated*/
                     /* a success but the message is either fragmented  */
                     /* or not the final message, neither of which      */
                     /* should occur, free the decoded message.         */
                     AVRCP_Free_Decoded_Message(RFA_BLUETOOTH_STACK_ID, &DecodeInformation->MessageInformation);

                     /* Specify the error for the caller.               */
                     ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
                  }
               }
               else
               {
                  /* An error occurred, map the Bluetopia error code to */
                  /* a PM error code.                                   */
                  ret_val = MapAVRCPErrorCodeToAUDMErrorCode(ret_val);
               }
            }
            else
            {
               /* We are decoding a command and have received a         */
               /* fragmented message.  To-date there are not any        */
               /* fragmented commands, specify the error for the caller.*/
               ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
            }
         }
         else
         {
            /* The buffer specified is not large enough to store the IPC*/
            /* Header, specify the error for the caller.                */
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         }
      }
   }
   else
   {
      /* An invalid parameter was passed in, determine which parameter  */
      /* caused the error and set the return code appropriately.        */
      if(!(((unsigned int)MessageType) <= ((unsigned int)amtGeneralReject)))
         ret_val = BTPM_ERROR_CODE_UNKNOWN_REMOTE_CONTROL_EVENT_TYPE;
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function frees all memory allocated by a successful */
   /* call to ConvertStreamAVRCPCommandToDecoded().                     */
void FreeAVRCPDecodedCommand(RemoteControlDecodeInformation_t *DecodeInformation)
{
   if((DecodeInformation) && (DecodeInformation->MessageInformation.MessageType != amtPassThrough))
   {
      AVRCP_Free_Decoded_Message(RFA_BLUETOOTH_STACK_ID, &DecodeInformation->MessageInformation);

      if(DecodeInformation->FragmentBuffer)
         BTPS_FreeMemory(DecodeInformation->FragmentBuffer);
   }
}

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
int ConvertStreamAVRCPResponseToDecoded(AVRCP_Message_Type_t MessageType, unsigned int BufferLength, unsigned char *Buffer, RemoteControlDecodeInformation_t *DecodeInformation, AUD_Remote_Control_Response_Data_t *ResponseData)
{
   int                          ret_val;
   long                         FragmentedResult;
   unsigned int                 FragmentBufferLength;
   unsigned int                 NumberFragments;
   unsigned int                 NumberDecodedFragments;
   Boolean_t                    BrowsingChannelMessage;
   FragmentInfo_t               FragmentInfo;
   FragmentLength_t             FragmentLength;
   AVRCP_Message_Information_t *FragmentedMessageInformation;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((ResponseData) && (BufferLength) && (Buffer) && (((unsigned int)MessageType) <= ((unsigned int)amtGetTotalNumberOfItems)))
   {
      /* Initialize the decode information that will be used to free any*/
      /* resources that are allocated by the decode process.            */
      DecodeInformation->MessageInformation.MessageType = MessageType;
      DecodeInformation->FragmentBuffer                 = NULL;

      /* Check to make sure it is a Message we know how to process.     */
      if(MessageType == amtPassThrough)
      {
         /* Next, determine if the buffer is large enough to store a    */
         /* pass-through response.                                      */
         if(BufferLength >= (STRUCTURE_OFFSET(AVRCP_Pass_Through_Response_Data_t, OperationData)))
         {
            /* The buffer is large enough, note the offset of the       */
            /* Operation Data within the Pass-Through Response          */
            /* Structure.                                               */
            ret_val = (int)(STRUCTURE_OFFSET(AVRCP_Pass_Through_Response_Data_t, OperationData));

            /* Check if the buffer is large enough to specify all of the*/
            /* Operation Data.                                          */
            if(BufferLength >= ((unsigned int)ret_val + ((AVRCP_Pass_Through_Response_Data_t *)Buffer)->OperationDataLength))
            {
               /* The buffer is sufficient, go ahead and convert the    */
               /* data.                                                 */
               ResponseData->MessageType = MessageType;

               BTPS_MemCopy(&(ResponseData->MessageData.PassThroughResponseData), Buffer, ret_val);

               if(ResponseData->MessageData.PassThroughResponseData.OperationDataLength)
                  ResponseData->MessageData.PassThroughResponseData.OperationData = &(Buffer[ret_val]);
               else
                  ResponseData->MessageData.PassThroughResponseData.OperationData = NULL;

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         }
         else
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
      }
      else
      {
         /* This is not a Pass-Through command, check if the buffer is  */
         /* large enough to store the IPC Message Header.               */
         if(BufferLength >= AVRCP_IPC_MESSAGE_HEADER_SIZE)
         {
            /* Determine if this is a browsing channel message.         */
            switch(MessageType)
            {
               case amtBrowsingChannel_Generic:
               case amtSetBrowsedPlayer:
               case amtChangePath:
               case amtGetItemAttributes:
               case amtSearch:
               case amtGetFolderItems:
               case amtGeneralReject:
               case amtGetTotalNumberOfItems:
                  BrowsingChannelMessage = TRUE;
                  break;

               default:
                  BrowsingChannelMessage = FALSE;
                  break;
            }

            /* Copy the fragment information from the buffer.           */
            BTPS_MemCopy(&FragmentInfo, Buffer, sizeof(FragmentInfo_t));

            /* Decrement the buffer's length and increment the buffer   */
            /* pointer.                                                 */
            BufferLength -= sizeof(FragmentInfo_t);
            Buffer       += sizeof(FragmentInfo_t);

            /* Initialize the return value to 0.                        */
            ret_val       = 0;

            /* Check if this is a fragmented message.                   */
            if(!(FragmentInfo & FRAGMENT_INFO_FLAGS_FRAGMENTED))
            {
               /* This is not a fragmented message, decrement the buffer*/
               /* length and increment the buffer's pointer so that we  */
               /* skip over the un-used, in this case, fragment length  */
               /* information.                                          */
               BufferLength -= sizeof(FragmentLength_t);
               Buffer       += sizeof(FragmentLength_t);

               /* Decode the message.                                   */
               if(!(ret_val = AVRCP_Decode_Message(RFA_BLUETOOTH_STACK_ID, BrowsingChannelMessage, TRUE, BufferLength, Buffer, &DecodeInformation->MessageInformation)))
               {
                  /* Check if we successfully decoded the message.      */
                  if((DecodeInformation->MessageInformation.MessageType != amtFragmentedMessage) && (DecodeInformation->MessageInformation.FinalMessage))
                  {
                     /* The message was successfully decoded, note the  */
                     /* message type.                                   */
                     ResponseData->MessageType = MessageType;

                     /* The CommandData->MessageData is a subset of the */
                     /* DecodeInformation->MessageInformation.-         */
                     /* MessageInformation so just copy the             */
                     /* sizeof(CommandData->MessageData) data bytes to  */
                     /* CommandData->MessageData.  Note that we could   */
                     /* use the currently used event to determine how   */
                     /* many bytes to copy, but as of the today,        */
                     /* 10/15/2013, the sizeof(CommandData->MessageData)*/
                     /* was only 14 bytes, the CPU might spend more time*/
                     /* in the switch statement to determine the event's*/
                     /* size than it would while it were copying the 14 */
                     /* bytes (as a workaround for an if statement you  */
                     /* could use a look-up-table to determine the size,*/
                     /* but this adds code space and complexity with    */
                     /* very little, if any, benefit).                  */
                     BTPS_MemCopy(&ResponseData->MessageData, &DecodeInformation->MessageInformation.MessageInformation, sizeof(ResponseData->MessageData));
                  }
                  else
                  {
                     /* The result from AVRCP_Decode_Message() indicated*/
                     /* a success but the message is either fragmented  */
                     /* or not the final message, neither of which      */
                     /* should occur, free the decoded message.         */
                     AVRCP_Free_Decoded_Message(RFA_BLUETOOTH_STACK_ID, &DecodeInformation->MessageInformation);

                     /* Specify the error for the caller.               */
                     ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
                  }
               }
               else
               {
                  /* An error occurred, map the Bluetopia error code to */
                  /* a PM error code.                                   */
                  ret_val = MapAVRCPErrorCodeToAUDMErrorCode(ret_val);
               }
            }
            else
            {
               /* This is a fragmented message, save the number of      */
               /* fragments.                                            */
               if((NumberFragments = (unsigned int)(FragmentInfo & FRAGMENT_INFO_FLAGS_NUMBER_FRAGMENTS_MASK)) > 0)
               {
                  /* Allocate memory for the fragment information.      */
                  if((FragmentedMessageInformation = BTPS_AllocateMemory(sizeof(AVRCP_Message_Information_t) * NumberFragments)) != NULL)
                  {
                     /* Loop through each of the fragments.             */
                     for(NumberDecodedFragments = 0; (NumberDecodedFragments < NumberFragments) && (!ret_val); NumberDecodedFragments++)
                     {
                        /* Check if there is space in the stream for the*/
                        /* fragment length information.                 */
                        if(BufferLength > sizeof(FragmentLength_t))
                        {
                           /* There is space for the fragment length    */
                           /* information, copy the information from the*/
                           /* stream.                                   */
                           BTPS_MemCopy(&FragmentLength, Buffer, sizeof(FragmentLength_t));

                           /* Decrement the buffer's length and         */
                           /* increment the buffer's pointer.           */
                           BufferLength -= sizeof(FragmentLength_t);
                           Buffer       += sizeof(FragmentLength_t);

                           /* Decode the fragment.                      */
                           if(!(ret_val = AVRCP_Decode_Message(RFA_BLUETOOTH_STACK_ID, BrowsingChannelMessage, TRUE, (unsigned int)MIN(((unsigned int)FragmentLength), ((unsigned int)BufferLength)), Buffer, &FragmentedMessageInformation[NumberDecodedFragments])))
                           {
                              /* Check if this is indeed a fragmented   */
                              /* message.                               */
                              if(FragmentedMessageInformation[NumberDecodedFragments].MessageType == amtFragmentedMessage)
                              {
                                 /* This is a fragmented message,       */
                                 /* decrement the buffer's length and   */
                                 /* increment the buffer's pointer by   */
                                 /* the length of the fragment.         */
                                 BufferLength -= FragmentLength;
                                 Buffer       += FragmentLength;
                              }
                              else
                              {
                                 /* This is not a fragmented message    */
                                 /* even though it should be, specify   */
                                 /* the error for the caller.           */
                                 ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;

                                 /* An error occurred and we want to    */
                                 /* stop processing, but don't break    */
                                 /* from the loop just yet, wait until  */
                                 /* the loop finishes so that the number*/
                                 /* of decoded fragments is incremented,*/
                                 /* we will use this information to free*/
                                 /* the fragment list below.            */
                              }
                           }
                           else
                           {
                              /* An error occurred, break out of the    */
                              /* loop.                                  */
                              break;
                           }
                        }
                        else
                        {
                           /* There is not enough space in the stream   */
                           /* for the fragment length information,      */
                           /* specify the error for the caller.         */
                           ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;

                           /* Break out of the loop.                    */
                           break;
                        }
                     }

                     /* Check if any errors occurred above.             */
                     if(!ret_val)
                     {
                        /* No error occurred, check if the last fragment*/
                        /* was the final message.                       */
                        if(FragmentedMessageInformation[NumberFragments - 1].FinalMessage)
                        {
                           /* The last fragment was the final message,  */
                           /* get the required length of the fragmented */
                           /* message.                                  */
                           if((FragmentedResult = AVRCP_Rebuild_Fragmented_Message(RFA_BLUETOOTH_STACK_ID, NumberFragments, FragmentedMessageInformation, 0, NULL)) > 0)
                           {
                              /* Note the required length of the        */
                              /* fragmented message.                    */
                              FragmentBufferLength = FragmentedResult;

                              /* Allocate memory to store the fragmented*/
                              /* message.                               */
                              if((DecodeInformation->FragmentBuffer = BTPS_AllocateMemory((unsigned long)FragmentBufferLength)) != NULL)
                              {
                                 /* The memory allocation was           */
                                 /* successful, next attempt to re-build*/
                                 /* the fragmented message.             */
                                 if((FragmentedResult = AVRCP_Rebuild_Fragmented_Message(RFA_BLUETOOTH_STACK_ID, NumberFragments, FragmentedMessageInformation, FragmentBufferLength, DecodeInformation->FragmentBuffer)) > 0)
                                 {
                                    /* We were able to re-build the     */
                                    /* fragmented message, next attempt */
                                    /* to decode the message.           */
                                    if(!(ret_val = AVRCP_Decode_Message(RFA_BLUETOOTH_STACK_ID, BrowsingChannelMessage, TRUE, FragmentBufferLength, DecodeInformation->FragmentBuffer, &DecodeInformation->MessageInformation)))
                                    {
                                       /* Check that the message is     */
                                       /* indeed not fragmented and that*/
                                       /* this is the final message.    */
                                       if((DecodeInformation->MessageInformation.MessageType != amtFragmentedMessage) && (DecodeInformation->MessageInformation.FinalMessage))
                                       {
                                          /* The message is not         */
                                          /* fragmented and this is the */
                                          /* final message, note the    */
                                          /* message type.              */
                                          ResponseData->MessageType = MessageType;

                                          /* Copy the resulting         */
                                          /* structure to the location  */
                                          /* of the user-specified      */
                                          /* structure.                 */
                                          BTPS_MemCopy(&ResponseData->MessageData, &DecodeInformation->MessageInformation.MessageInformation, sizeof(ResponseData->MessageData));
                                       }
                                       else
                                       {
                                          /* The result from            */
                                          /* AVRCP_Decode_Message()     */
                                          /* indicated a success but the*/
                                          /* message is either          */
                                          /* fragmented or not the final*/
                                          /* message, neither of which  */
                                          /* should occur, free the     */
                                          /* decoded message.           */
                                          AVRCP_Free_Decoded_Message(RFA_BLUETOOTH_STACK_ID, &DecodeInformation->MessageInformation);

                                          /* Specify the error for the  */
                                          /* caller.                    */
                                          ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
                                       }
                                    }
                                 }
                                 else
                                 {
                                    /* An error occurred while          */
                                    /* re-building the fragmented       */
                                    /* message, specify the error for   */
                                    /* the caller.                      */
                                    ret_val = (int)FragmentedResult;
                                 }
                              }
                              else
                              {
                                 /* We were unable to allocate memory   */
                                 /* for the fragmented message, specify */
                                 /* the error for the caller.           */
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                              }
                           }
                           else
                           {
                              /* We could not get the required length of*/
                              /* the fragmented message, specify the    */
                              /* error for the caller.                  */
                              ret_val = (int)FragmentedResult;
                           }
                        }
                        else
                        {
                           /* The last fragment was not the final       */
                           /* message, an error has occurred, specify   */
                           /* the error for the caller.                 */
                           ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
                        }
                     }

                     /* Free the list of fragmented messages.           */
                     AVRCP_Free_Fragmented_Message_List(RFA_BLUETOOTH_STACK_ID, NumberDecodedFragments, FragmentedMessageInformation);

                     /* Free the pointer allocated for the fragmented   */
                     /* messages.                                       */
                     BTPS_FreeMemory(FragmentedMessageInformation);

                     /* Check if any errors occurred and if the error is*/
                     /* in the range of PM's error codes.               */
                     if((ret_val < 0) && (ret_val > BTPM_ERROR_CODE_INVALID_PARAMETER))
                     {
                        /* The error is not in the range of PM's error  */
                        /* codes therefore it must be a Bluetopia error */
                        /* code, map the Bluetopia error code to a PM   */
                        /* error code.                                  */
                        ret_val = MapAVRCPErrorCodeToAUDMErrorCode(ret_val);
                     }
                  }
                  else
                  {
                     /* We were unable to allocate memory for the       */
                     /* fragmented message, specify the error for the   */
                     /* caller.                                         */
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                  }
               }
               else
               {
                  /* The flag was set indicating that this was a        */
                  /* fragmented message but the number of fragments     */
                  /* specified in the fragment information was 0, set   */
                  /* the return value to pass the error back to the     */
                  /* caller.                                            */
                  ret_val = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
               }
            }
         }
         else
         {
            /* The buffer specified is not large enough to store the IPC*/
            /* Header, specify the error for the caller.                */
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
         }
      }
   }
   else
   {
      /* An invalid parameter was passed in, determine which parameter  */
      /* caused the error and set the return code appropriately.        */
      if(!(((unsigned int)MessageType) <= ((unsigned int)amtGeneralReject)))
         ret_val = BTPM_ERROR_CODE_UNKNOWN_REMOTE_CONTROL_EVENT_TYPE;
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function frees all memory allocated by a successful */
   /* call to ConvertStreamAVRCPResponseToDecoded().                    */
void FreeAVRCPDecodedResponse(RemoteControlDecodeInformation_t *DecodeInformation)
{
   if((DecodeInformation) && (DecodeInformation->MessageInformation.MessageType != amtPassThrough))
   {
      AVRCP_Free_Decoded_Message(RFA_BLUETOOTH_STACK_ID, &DecodeInformation->MessageInformation);

      if(DecodeInformation->FragmentBuffer)
         BTPS_FreeMemory(DecodeInformation->FragmentBuffer);
   }
}

   /* The following function is a utility function that exists to       */
   /* determine whether a service record contains a given Service Class */
   /* UUID. This function returns TRUE if the given Service Class UUID  */
   /* was found in the Service Class List attribute of the service      */
   /* record.                                                           */
static Boolean_t ServiceRecordContainsServiceClass(SDP_Service_Attribute_Response_Data_t *ServiceRecord, UUID_128_t ServiceClass)
{
   Boolean_t           ret_val;
   UUID_128_t          RecordServiceClass;
   unsigned int        ServiceClassIndex;
   SDP_Data_Element_t *ServiceClassAttribute;

   ret_val = FALSE;

   if(ServiceRecord)
   {
      if((ServiceClassAttribute = FindSDPAttribute(ServiceRecord, SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST)) != NULL)
      {
         /* The Service Class Attribute has been located. Make sure that*/
         /* it contains a Sequence, as required.                        */
         if(ServiceClassAttribute->SDP_Data_Element_Type == deSequence)
         {
            /* The attribute looks valid. Now search for the appropriate*/
            /* Service Class UUID.                                      */
            for(ServiceClassIndex = 0; ServiceClassIndex < ServiceClassAttribute->SDP_Data_Element_Length; ServiceClassIndex++)
            {
               /* Normalize the Service Class to a 128-bit UUID for     */
               /* comparison.                                           */
               if(ConvertSDPDataElementToUUID128(ServiceClassAttribute->SDP_Data_Element.SDP_Data_Element_Sequence[ServiceClassIndex], &RecordServiceClass) == 0)
               {
                  /* The Service Class UUID was successfully located, so*/
                  /* see if it is the Service Class we are looking for. */
                  if(COMPARE_UUID_128(RecordServiceClass, ServiceClass))
                  {
                     /* The Service Class UUID has been found in the    */
                     /* current service record.                         */
                     ret_val = TRUE;
                     break;
                  }
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to locate*/
   /* a particular Attribute within a parsed SDP record. This function  */
   /* returns a pointer to the Attribute data, or NULL if the attribute */
   /* does not exist in the given SDP record.                           */
static SDP_Data_Element_t *FindSDPAttribute(SDP_Service_Attribute_Response_Data_t *ServiceRecord, Word_t AttributeID)
{
   unsigned int        AttributeIndex;
   SDP_Data_Element_t *AttributePtr = NULL;

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(ServiceRecord)
   {
      /* Loop through all available attributes in the record to find the*/
      /* requested attribute.                                           */
      for(AttributeIndex=0;AttributeIndex<(unsigned int)ServiceRecord->Number_Attribute_Values;AttributeIndex++)
      {
         /* Check whether we have found the requested attribute.        */
         if(ServiceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == AttributeID)
         {
            /* The attribute has been found. Save the attribute and stop*/
            /* searching the record.                                    */
            AttributePtr = ServiceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;
            break;
         }
      }
   }

   return(AttributePtr);
}

   /* The following function is a utility function that exists to       */
   /* convert an SDP Data Element, which contains a UUID, into a 128-bit*/
   /* UUID. This function returns zero if successful, or a negative     */
   /* error code on failure.                                            */
static int ConvertSDPDataElementToUUID128(SDP_Data_Element_t DataElement, UUID_128_t *UUID)
{
   int ret_val;

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UUID)
   {
      switch(DataElement.SDP_Data_Element_Type)
      {
         case deUUID_128:
            *UUID   = DataElement.SDP_Data_Element.UUID_128;
            ret_val = 0;
            break;
         case deUUID_32:
            SDP_ASSIGN_BASE_UUID(*UUID);
            ASSIGN_SDP_UUID_32_TO_SDP_UUID_128(*UUID, DataElement.SDP_Data_Element.UUID_32);
            ret_val = 0;
            break;
         case deUUID_16:
            SDP_ASSIGN_BASE_UUID(*UUID);
            ASSIGN_SDP_UUID_16_TO_SDP_UUID_128(*UUID, DataElement.SDP_Data_Element.UUID_16);
            ret_val = 0;
            break;
         default:
            /* No other data type is allowed for this parameter, so     */
            /* return an error.                                         */
            ASSIGN_SDP_UUID_128(*UUID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            break;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* query and parse AVRCP services from a remote device. This first   */
   /* parameter is the Bluetooth Address of the remote device. The      */
   /* second parameter is a pointer to the structure where the parsed   */
   /* service information will be placed. This function returns zero if */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
int BTPSAPI AUDM_Query_Remote_Control_Services_Info(BD_ADDR_t RemoteDeviceAddress, AUDM_Remote_Control_Services_Info_t *ServicesInfo)
{
   int                              ret_val;
   Word_t                           TempVersion;
   Boolean_t                        VersionValid;
   UUID_128_t                       ControllerServiceClass;
   UUID_128_t                       TargetServiceClass;
   UUID_128_t                       AVRCPProfile;
   UUID_128_t                       ProfileUUID;
   unsigned int                     ServiceDataLength;
   unsigned int                     ServiceRecordIndex;
   unsigned int                     DataElementIndex;
   unsigned char                   *ServiceData;
   unsigned long                    ServiceFlag;
   AVRCP_Version_t                  Version;
   SDP_Data_Element_t              *VersionAttribute;
   SDP_Data_Element_t              *FeaturesAttribute;
   SDP_Data_Element_t              *DataElement;
   DEVM_Parsed_SDP_Data_t           ParsedSDPData;
   AUDM_Remote_Control_Role_Info_t *RoleInfo = NULL;

   SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_CONTROLLER_UUID_128(ControllerServiceClass);
   SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_TARGET_UUID_128(TargetServiceClass);
   SDP_ASSIGN_AUDIO_VIDEO_REMOTE_CONTROL_PROFILE_UUID_128(AVRCPProfile);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ServicesInfo))
   {
      if(DEVM_AcquireLock())
      {
         if((ret_val = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, 0, 0, NULL, &ServiceDataLength)) == 0)
         {
            if((ServiceData = BTPS_AllocateMemory(ServiceDataLength)) != NULL)
            {
               if((ret_val = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, 0, ServiceDataLength, ServiceData, NULL)) >= 0)
               {
                  DEVM_ReleaseLock();

                  if((ret_val = DEVM_ConvertRawSDPStreamToParsedSDPData(ServiceDataLength, ServiceData, &ParsedSDPData)) == 0)
                  {
                     BTPS_MemInitialize(ServicesInfo, 0, sizeof(AUDM_Remote_Control_Services_Info_t));

                     for(ServiceRecordIndex = 0; ServiceRecordIndex < ParsedSDPData.NumberServiceRecords; ServiceRecordIndex++)
                     {
                        if((ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], ControllerServiceClass)) || (ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], AVRCPProfile)))
                        {
                           ServiceFlag = AUDM_REMOTE_CONTROL_SERVICES_FLAGS_CONTROLLER_ROLE_SUPPORTED;
                           RoleInfo    = &ServicesInfo->ControllerInfo;
                        }
                        else
                        {
                           if(ServiceRecordContainsServiceClass(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], TargetServiceClass))
                           {
                              ServiceFlag = AUDM_REMOTE_CONTROL_SERVICES_FLAGS_TARGET_ROLE_SUPPORTED;
                              RoleInfo    = &ServicesInfo->TargetInfo;
                           }
                           else
                              RoleInfo = NULL;
                        }

                        if(RoleInfo)
                        {
                           VersionAttribute  = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST);
                           FeaturesAttribute = FindSDPAttribute(&ParsedSDPData.SDPServiceAttributeResponseData[ServiceRecordIndex], SDP_ATTRIBUTE_ID_SUPPORTED_FEATURES);

                           if((VersionAttribute) && (FeaturesAttribute))
                           {
                              if((VersionAttribute->SDP_Data_Element_Type == deSequence) && (FeaturesAttribute->SDP_Data_Element_Type == deUnsignedInteger2Bytes))
                              {
                                 for(DataElementIndex = 0; DataElementIndex < VersionAttribute->SDP_Data_Element_Length; DataElementIndex++)
                                 {
                                    DataElement = &VersionAttribute->SDP_Data_Element.SDP_Data_Element_Sequence[DataElementIndex];

                                    if((DataElement->SDP_Data_Element_Type == deSequence) && (DataElement->SDP_Data_Element_Length >= 2))
                                    {
                                       if((ConvertSDPDataElementToUUID128(DataElement->SDP_Data_Element.SDP_Data_Element_Sequence[0], &ProfileUUID) == 0) && (DataElement->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type = deUnsignedInteger2Bytes))
                                       {
                                          if(COMPARE_UUID_128(ProfileUUID, AVRCPProfile))
                                          {
                                             TempVersion = DataElement->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element.UnsignedInteger2Bytes;

                                             switch(TempVersion)
                                             {
                                                case AVRCP_PROFILE_VERSION_1_0:
                                                   Version      = apvVersion1_0;
                                                   VersionValid = TRUE;
                                                   break;
                                                case AVRCP_PROFILE_VERSION_1_3:
                                                   Version      = apvVersion1_3;
                                                   VersionValid = TRUE;
                                                   break;
                                                case AVRCP_PROFILE_VERSION_1_4:
                                                   Version      = apvVersion1_4;
                                                   VersionValid = TRUE;
                                                   break;
                                                case AVRCP_PROFILE_VERSION_1_5:
                                                   Version      = apvVersion1_5;
                                                   VersionValid = TRUE;
                                                   break;
                                                case AVRCP_PROFILE_VERSION_1_6:
                                                   Version      = apvVersion1_6;
                                                   VersionValid = TRUE;
                                                   break;
                                                default:
                                                   if(TempVersion > AVRCP_PROFILE_VERSION_1_6)
                                                   {
                                                      Version      = apvVersion1_6;
                                                      VersionValid = TRUE;
                                                   }
                                                   else
                                                      VersionValid = FALSE;
                                                   break;
                                             }

                                             if(VersionValid)
                                             {
                                                ServicesInfo->ServiceFlags      |= ServiceFlag;
                                                RoleInfo->Version                = Version;
                                                RoleInfo->SupportedFeaturesFlags = (unsigned long)FeaturesAttribute->SDP_Data_Element.UnsignedInteger2Bytes;

                                                break;
                                             }
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }

                     DEVM_FreeParsedSDPData(&ParsedSDPData);
                  }
               }
               else
                  DEVM_ReleaseLock();

               BTPS_FreeMemory(ServiceData);
            }
            else
            {
               DEVM_ReleaseLock();
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
         }
         else
            DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}


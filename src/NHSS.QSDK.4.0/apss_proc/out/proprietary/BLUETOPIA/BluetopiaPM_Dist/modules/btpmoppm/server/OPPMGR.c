/*****< oppmgr.c >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPMGR - Object Push Manager Implementation for Stonestreet One           */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/11/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMOPPM.h"            /* BTPM OPP Manager Prototypes/Constants.    */
#include "OPPMMSG.h"             /* BTPM OPP Manager Message Formats.         */
#include "OPPMGR.h"              /* OPP Manager Impl. Prototypes/Constants.   */
#include "BTPMMODC.h"            /* BTPM MODC Prototypes/Constants.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Bluetooth Stack ID of the        */
   /* currently open Bluetooth Stack.  This value is set via the        */
   /* _OPPM_SetBluetoothStackID() function.                             */
static unsigned int _BluetoothStackID;

   /* Internal Function Prototypes.                                     */
static unsigned int CalculateObjectNameSize(Word_t *ObjectName);

static int MapOPPErrorCodeToOPPM(int ErrorCode);

static void BTPSAPI OPP_Event_Callback(unsigned int BluetoothStackID, OPP_Event_Data_t *OPP_Event_Data, unsigned long CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* determine the size (in bytes) required to hold the specified      */
   /* Object Name (including terminating NULL character).               */
static unsigned int CalculateObjectNameSize(Word_t *ObjectName)
{
   unsigned int ret_val;

   ret_val = 0;

   if(ObjectName)
   {
      while(*(ObjectName++))
         ret_val += NON_ALIGNED_WORD_SIZE;

      /* Make sure we count the NULL terminator.                        */
      ret_val += NON_ALIGNED_WORD_SIZE;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* This function is a utility function to convert an error returned  */
   /* from OPP to a PM error code.                                      */
static int MapOPPErrorCodeToOPPM(int ErrorCode)
{
   int ret_val;

   switch(ErrorCode)
   {
      case BTOPP_ERROR_INVALID_PARAMETER:
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         break;
      case BTOPP_ERROR_NOT_INITIALIZED:
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;
         break;
      case BTOPP_ERROR_REQUEST_ALREADY_OUTSTANDING:
      case BTOPP_ERROR_ACTION_NOT_ALLOWED:
      default:
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION;
         break;
   }

   return(ret_val);
}

   /* OPP_Event_Callback processes bluetopia message access events.     */
static void BTPSAPI OPP_Event_Callback(unsigned int BluetoothStackID, OPP_Event_Data_t *OPP_Event_Data, unsigned long CallbackParameter)
{
   unsigned int        AdditionalSize;
   OPPM_Update_Data_t *OPPMUpdateData;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the event data pointer is not null and contains data. */
   if((OPP_Event_Data) && (OPP_Event_Data->Event_Data_Size))
   {
      /* Flag that there is no event to dispatch.                       */
      OPPMUpdateData = NULL;

      switch(OPP_Event_Data->Event_Data_Type)
      {
         case etOPP_Open_Request_Indication:
         case etOPP_Open_Port_Indication:
         case etOPP_Open_Port_Confirmation:
         case etOPP_Close_Port_Indication:
         case etOPP_Push_Object_Confirmation:
         case etOPP_Pull_Business_Card_Indication:
         case etOPP_Abort_Confirmation:
         case etOPP_Abort_Indication:

            /* Allocate memory and copy the Bluetopia Event data to pass*/
            /* on to the PM client.                                     */
            /* * NOTE * Since both are unions, we do not need to handle */
            /*          each event case separately (because we have the */
            /*          length of the event data).                      */
            if((OPP_Event_Data->Event_Data.OPP_Open_Request_Indication_Data) && (OPPMUpdateData = (OPPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(OPPM_Update_Data_t))) != NULL)
            {
               /* Note the event type and copy the event data into the  */
               /* notification structure.                               */
               OPPMUpdateData->UpdateType                        = utOPPEvent;
               OPPMUpdateData->UpdateData.OPPEventData.EventType = OPP_Event_Data->Event_Data_Type;

               BTPS_MemCopy(&(OPPMUpdateData->UpdateData.OPPEventData.EventData), OPP_Event_Data->Event_Data.OPP_Open_Request_Indication_Data, OPP_Event_Data->Event_Data_Size);
            }
            break;
         case etOPP_Push_Object_Indication:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->DataBuffer))
                  AdditionalSize = OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->DataLength;
               else
                  AdditionalSize  = 0;

               if((OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->ObjectName))
                  AdditionalSize += CalculateObjectNameSize(OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->ObjectName);

               if((OPPMUpdateData = (OPPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(OPPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  OPPMUpdateData->UpdateType                        = utOPPEvent;
                  OPPMUpdateData->UpdateData.OPPEventData.EventType = OPP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(OPPMUpdateData->UpdateData.OPPEventData.EventData), OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data, OPP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     /* Reset the AdditionalSize variable (we will use  */
                     /* this to be an offset of where we need to copy   */
                     /* data).                                          */
                     AdditionalSize = 0;

                     if(OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->ObjectName)
                     {
                        OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.ObjectName = (Word_t *)(((Byte_t *)OPPMUpdateData) + sizeof(OPPM_Update_Data_t));

                        /* Note the Object Name Size (in bytes).        */
                        AdditionalSize = CalculateObjectNameSize(OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->ObjectName);

                        /* Now copy the buffer.                         */
                        BTPS_MemCopy(OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.ObjectName, OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->ObjectName, AdditionalSize);

                        DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("ObjectName Length: %d\n", AdditionalSize));
                     }
                     else
                        OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.ObjectName = NULL;

                     if((OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->DataBuffer) && (OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->DataLength))
                     {
                        OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.DataBuffer = ((Byte_t *)OPPMUpdateData) + sizeof(OPPM_Update_Data_t) + AdditionalSize;

                        /* Now copy the buffer.                         */
                        BTPS_MemCopy(OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.DataBuffer, OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->DataBuffer, OPP_Event_Data->Event_Data.OPP_Push_Object_Indication_Data->DataLength);

                        DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.DataLength));
                     }
                     else
                        OPPMUpdateData->UpdateData.OPPEventData.EventData.PushObjectIndicationData.DataBuffer = NULL;
                  }
               }
            }
            break;
         case etOPP_Pull_Business_Card_Confirmation:
            /* We need to handle this case specially as there are       */
            /* pointers located in these structures.                    */
            if(OPP_Event_Data->Event_Data.OPP_Pull_Business_Card_Confirmation_Data)
            {
               /* Determine how much extra space we need to allocate.   */
               /* Allocate memory to hold the Event Data (we will       */
               /* process it later).                                    */
               if((OPP_Event_Data->Event_Data.OPP_Pull_Business_Card_Confirmation_Data->DataBuffer))
                  AdditionalSize = OPP_Event_Data->Event_Data.OPP_Pull_Business_Card_Confirmation_Data->DataLength;
               else
                  AdditionalSize  = 0;

               if((OPPMUpdateData = (OPPM_Update_Data_t *)BTPS_AllocateMemory(sizeof(OPPM_Update_Data_t) + AdditionalSize)) != NULL)
               {
                  /* Note the event type and copy the event data into   */
                  /* the notification structure.                        */
                  OPPMUpdateData->UpdateType                        = utOPPEvent;
                  OPPMUpdateData->UpdateData.OPPEventData.EventType = OPP_Event_Data->Event_Data_Type;

                  BTPS_MemCopy(&(OPPMUpdateData->UpdateData.OPPEventData.EventData), OPP_Event_Data->Event_Data.OPP_Pull_Business_Card_Confirmation_Data, OPP_Event_Data->Event_Data_Size);

                  if(AdditionalSize)
                  {
                     OPPMUpdateData->UpdateData.OPPEventData.EventData.PullBusinessCardConfirmationData.DataBuffer = ((Byte_t *)OPPMUpdateData) + sizeof(OPPM_Update_Data_t);

                     /* Now copy the buffer.                            */
                     BTPS_MemCopy(OPPMUpdateData->UpdateData.OPPEventData.EventData.PullBusinessCardConfirmationData.DataBuffer, OPP_Event_Data->Event_Data.OPP_Pull_Business_Card_Confirmation_Data->DataBuffer, AdditionalSize);

                     DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("DataBuffer Length: %d\n", AdditionalSize));
                  }
                  else
                     OPPMUpdateData->UpdateData.OPPEventData.EventData.PullBusinessCardConfirmationData.DataBuffer = NULL;
               }
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_WARNING), ("Unhandled Event %d\n", OPP_Event_Data->Event_Data_Type));
            break;
      }

      /* If there is an event to dispatch, go ahead and dispatch it.    */
      if(OPPMUpdateData)
      {
         if(!OPPM_NotifyUpdate(OPPMUpdateData))
            BTPS_FreeMemory((void *)OPPMUpdateData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Object Push Manager implementation.  This function */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Object Push Manager implementation.                               */
int _OPPM_Initialize(void)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has already been initialized.          */
   if(!Initialized)
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Message Access Manager \n"));

      /* Flag that the module is initialized.                           */
      Initialized       = TRUE;

      /* Flag that that Bluetooth Stack is not currently open.          */
      _BluetoothStackID = 0;

      /* Flag success to the caller.                                    */
      ret_val           = 0;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for shutting down the Object*/
   /* Push Manager implementation.  After this function is called the   */
   /* Object Push Manager implementation will no longer operate until   */
   /* it is initialized again via a call to the _OPPM_Initialize()      */
   /* function.                                                         */
void _OPPM_Cleanup(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Flag that this module is no longer initialized.                */
      Initialized       = FALSE;

      /* Flag that the Stack is not open.                               */
      _BluetoothStackID = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for informing the Object    */
   /* Push Manager implementation of that Bluetooth stack ID of the     */
   /* currently opened Bluetooth stack.  When this parameter is set     */
   /* to non-zero, this function will actually initialize the Object    */
   /* Push Manager with the specified Bluetooth stack ID.  When this    */
   /* parameter is set to zero, this function will actually clean up all*/
   /* resources associated with the prior initialized Bluetooth Stack.  */
void _OPPM_SetBluetoothStackID(unsigned int BluetoothStackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Note the Bluetooth Stack ID (regardless if it's zero.          */
      _BluetoothStackID = BluetoothStackID;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for opening a local OPP     */
   /* Server.  The parameter to this function is the Port on which to   */
   /* open this server, and *MUST* be between OPP_PORT_NUMBER_MINIMUM   */
   /* and OPP_PORT_NUMBER_MAXIMUM.  This function returns a positive,   */
   /* non zero value if successful or a negative return error code if an*/
   /* error occurs.  A successful return code will be a OPP Profile ID  */
   /* that can be used to reference the Opened OPP Profile Server Port  */
   /* in ALL other OPP Server functions in this module.  Once an OPP    */
   /* Profile Server is opened, it can only be Un-Registered via a call */
   /* to the _OPP_Close_Server() function (passing the return value from*/
   /* this function).                                                   */
int _OPPM_Open_Object_Push_Server(unsigned int ServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the port is in the allowed range.              */
      if((ServerPort >= OPP_PORT_NUMBER_MINIMUM) && (ServerPort <= OPP_PORT_NUMBER_MAXIMUM))
      {
         if((ret_val = OPP_Open_Object_Push_Server(_BluetoothStackID, ServerPort, OPP_Event_Callback, 0)) > 0)
         {
            /* Set the server to manual connect mode.                   */
            OPP_Set_Server_Mode(_BluetoothStackID, ret_val, OPP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION);
         }
         else
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for closing a               */
   /* currently open/registered Object Push Profile server.  This       */
   /* function is capable of closing servers opened via a call to       */
   /* _OPPM_Open_Object_Oush_Server().  The parameter to this function  */
   /* is the OPPM ID of the Profile Server to be closed.  This function */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
int _OPPM_Close_Server(unsigned int OPPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Close_Server(_BluetoothStackID, OPPID)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function adds a OPP Server (MSE) Service Record to  */
   /* the SDP Database.  The first parameter is the OPP ID that was     */
   /* returned by a previous call to _OPP_Open_Object_Push_Server().    */
   /* The second parameter is a pointer to ASCII, NULL terminated string*/
   /* containing the Service Name to include within the SDP Record.     */
   /* The third parameter is the Supported Object Type Bitmask value.   */
   /* The final parameter will be set to the service record handle on   */
   /* success.  This function returns zero on success or a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * This function should only be called with the OPP ID that */
   /*          was returned from the _OPP_Open_Object_Push_Server()     */
   /*          function.  This function should NEVER                    */
   /*          be used with OPP ID returned from the                    */
   /*          _OPP_Open_Remote_Object_Push_Server_Port() function.     */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the _OPP_Un_Register_SDP_Record()  */
   /*          function.                                                */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
int _OPPM_Register_Object_Push_Server_SDP_Record(unsigned int OPPID, char *ServiceName, DWord_t SupportedObjectTypes, DWord_t *RecordHandle)
{
   int          ret_val;
   Byte_t       EIRData[2 + UUID_16_SIZE];
   UUID_16_t    tempUUID;
   unsigned int EIRDataLength;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %s, %08X, %p\n", OPPID, ServiceName, (unsigned int)SupportedObjectTypes, RecordHandle));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if((OPPID) && (RecordHandle))
      {
         if((ret_val = OPP_Register_Object_Push_Server_SDP_Record(_BluetoothStackID, OPPID, ServiceName, SupportedObjectTypes, RecordHandle)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
         else
         {
            /* Configure the EIR Data.                                  */
            EIRDataLength = NON_ALIGNED_BYTE_SIZE + UUID_16_SIZE;

            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[0], EIRDataLength);
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&EIRData[1], HCI_EXTENDED_INQUIRY_RESPONSE_DATA_TYPE_16_BIT_SERVICE_CLASS_UUID_PARTIAL);

            /* Assign the OBEX Object Push UUID (in big-endian format). */
            SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_16(tempUUID);

            /* Convert the UUID to little endian as required by EIR     */
            /* data.                                                    */
            CONVERT_SDP_UUID_16_TO_BLUETOOTH_UUID_16(*((UUID_16_t *)&(EIRData[2])), tempUUID);

            /* Increment the length we pass to the internal function to */
            /* take into account the length byte.                       */
            EIRDataLength += NON_ALIGNED_BYTE_SIZE;

            /* Configure the EIR data.                                  */
            MOD_AddEIRData(EIRDataLength, EIRData);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for deleting a previously   */
   /* registered OPP SDP Service Record.  This function accepts as its  */
   /* parameter the SDP Service Record Handle of the SDP Service Record */
   /* to delete from the SDP Database.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
int _OPPM_Un_Register_SDP_Record(unsigned int OPPID, DWord_t ServiceRecordHandle)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if((OPPID) && (ServiceRecordHandle))
      {
         /* Note, this will not return an OPP error code, as it is only */
         /* a macro.                                                    */
         if((ret_val = OPP_Un_Register_SDP_Record(_BluetoothStackID, OPPID, ServiceRecordHandle)) < 0)
            ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for opening a connection    */
   /* from a local Object Push Client to a remote Object Push Server.   */
   /* The first parameter to this function is the Bluetooth Stack ID of */
   /* the Bluetooth Protocol Stack Instance to be associated with this  */
   /* Object Push Profile connection.  The second parameter to this     */
   /* function is the BD_ADDR of the remote Object Push Server in which */
   /* to connect.  The third parameter to this function is the Remote   */
   /* Server Port where the Push Server is registered.  The fourth and  */
   /* fifth parameters are the Event Callback function and application  */
   /* defined Callback Parameter to be used when OPP Events occur.  This*/
   /* function returns a non-zero, positive, number on success or a     */
   /* negative return value if there was an error.  A successful return */
   /* value will be a OPP ID that can used to reference the Opened      */
   /* Object Push Profile connection to a remote Object Push Server in  */
   /* ALL other applicable functions in this module.                    */
   /* ** NOTE ** The Object Push Server Port value must be specified    */
   /*            and must be a value between OPP_PORT_NUMBER_MINIMUM and*/
   /*            OPP_PORT_NUMBER_MAXIMUM.                               */
int _OPPM_Open_Remote_Object_Push_Server(BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (RemoteServerPort >= OPP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= OPP_PORT_NUMBER_MAXIMUM))
      {
         if((ret_val = OPP_Open_Remote_Object_Push_Server(_BluetoothStackID, BD_ADDR, RemoteServerPort, OPP_Event_Callback, 0)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Object Push Profile      */
   /* Server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Stack associated with the Object Push   */
   /* Profile Server that is responding to the request.  The second     */
   /* parameter to this function is the OPP ID of the Object Push       */
   /* Profile for which a connection request was received.  The final   */
   /* parameter to this function specifies whether to accept the pending*/
   /* connection request (or to reject the request).  This function     */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etOPP_Open_Imaging_Service_Port_Indication event has   */
   /*            occurred.                                              */
int _OPPM_Open_Request_Response(unsigned int OPPID, Boolean_t AcceptConnection)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Open_Request_Response(_BluetoothStackID, OPPID, AcceptConnection)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for closing a currently     */
   /* on-going Object Push Profile connection.  The first parameter to  */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Object Push Profile    */
   /* connection being closed.  The second parameter to this function is*/
   /* the OPP ID of the Object Push Profile connection to be closed.    */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** If this function is called with a server OPP ID (value */
   /*            returned from OPP_Open_Object_Push_Server()) any       */
   /*            clients current connection to this server will be      */
   /*            terminated, but the server will remained registered.   */
   /*            If this function is call using a client OPP ID (value  */
   /*            returned from OPP_Open_Remote_Object_Push_Server()) the*/
   /*            client connection shall be terminated.                 */
int _OPPM_Close_Connection(unsigned int OPPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Close_Connection(_BluetoothStackID, OPPID)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Object Push Server.  The first parameter to this    */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Client   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Client making this call.  This  */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on-going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
int _OPPM_Abort_Request(unsigned int OPPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Abort_Request(_BluetoothStackID, OPPID)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter to */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Object Push Client     */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Client making this call.  The third     */
   /* parameter to this function is the actual type of object that is   */
   /* being pushed.  The fourth parameter to this function is the Name  */
   /* of the Object being put (in NULL terminated UNICODE format).  The */
   /* fifth parameter is the total length of the object being put.  The */
   /* sixth and seventh parameters to this function specify the length  */
   /* of the Object Data and a pointer to the Object Data being Pushed. */
   /* The eighth parameter to this function is a pointer to a length    */
   /* variable that will receive the total amount of data actually sent */
   /* in the request.  The final parameter to this function is a Boolean*/
   /* Flag indicating if this is to be the final segment of the object. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** This function should be used to initiate the Push      */
   /*            Object transaction as well as to continue a previously */
   /*            initiated, on-going, Push Object transaction.          */
   /* ** NOTE ** The Object Name is a pointer to a NULL Terminated      */
   /*            UNICODE String.                                        */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Object Push Profile function.     */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Object Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            not all parameters to this function are used in each   */
   /*            segment of the transaction.                            */
int _OPPM_Push_Object_Request(unsigned int OPPID, OPP_Object_Type_t ObjectType, Word_t *ObjectName, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Push_Object_Request(_BluetoothStackID, OPPID, ObjectType, ObjectName, ObjectTotalLength, DataLength, DataBuffer, AmountWritten, Final)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending an Object Push  */
   /* Response to the remote Client.  The first parameter to this       */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Server making this call.  The   */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
int _OPPM_Push_Object_Response(unsigned int OPPID, Byte_t ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Push_Object_Response(_BluetoothStackID, OPPID, ResponseCode)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The first         */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Object Push Profile Client making this call.  The second parameter*/
   /* to this function is the OPP ID of the Object Push Client making   */
   /* this call.  This function returns zero if successful, or a        */
   /* negative return value if there was an error.                      */
   /* ** NOTE ** This function should be used to initiate the Pull      */
   /*            Business Card transaction as well as to continue a     */
   /*            previously initiated, on-going, Pull Business Card     */
   /*            transaction.                                           */
int _OPPM_Pull_Business_Card_Request(unsigned int OPPID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Pull_Business_Card_Request(_BluetoothStackID, OPPID)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter to this  */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Server making this call.  The   */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth parameter specifies the*/
   /* Total Length of the Business card being pulled.  The fifth and    */
   /* sixth parameters to this function specify the length of the data  */
   /* being sent and a pointer to the data being sent.  The seventh     */
   /* parameter to this function is a pointer to a length variable that */
   /* will receive the amount of data actually sent.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** If the value returned in AmountWritten is less than the*/
   /*            value specified in DataLength, then an additional call */
   /*            to this function must be made to send the remaining    */
   /*            data.  Note that an additional call cannot be made     */
   /*            until AFTER another Pull Business Card Request Event is*/
   /*            received.                                              */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Object Push Profile function.     */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Object Total       */
   /*            Length), others don't appear in the first segment but  */
   /*            may appear in later segments (i.e. Body).  This being  */
   /*            the case, not all parameters to this function are used */
   /*            in each segment of the transaction.                    */
int _OPPM_Pull_Business_Card_Response(unsigned int OPPID, Byte_t ResponseCode, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the module has been initialized                   */
   if(Initialized)
   {
      /* Check to see if the parameters are semi-valid.                 */
      if(OPPID)
      {
         if((ret_val = OPP_Pull_Business_Card_Response(_BluetoothStackID, OPPID, ResponseCode, ObjectTotalLength, DataLength, DataBuffer, AmountWritten)) < 0)
            ret_val = MapOPPErrorCodeToOPPM(ret_val);
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Return the result to the caller.                                  */
   return(ret_val);
}


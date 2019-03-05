/*****< btpmhrpm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHRPM - HRP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHRPM.h"            /* BTPM HRP Manager Prototypes/Constants.    */
#include "HRPMAPI.h"             /* HRP Manager Prototypes/Constants.         */
#include "HRPMMSG.h"             /* BTPM HRP Manager Message Formats.         */
#include "HRPMGR.h"              /* HRP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following define represents the Callback ID that will be      */
   /* issued to a registered HRPM Client.  If a future implementation   */
   /* requires multiple HRPM Clients, then this functionality can be    */
   /* augmented.                                                        */
#define HRPM_COLLECTOR_CALLBACK_ID                    1

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHRPM_Event_Callback_Info_t
{
   unsigned int           ClientID;
   HRPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} HRPM_Event_Callback_Info_t;

   /* Structure which is used to track information pertaining to known  */
   /* connected LE devices supporting Heart Rate Profile.               */
typedef struct _tagDevice_Entry_t
{
   BD_ADDR_t                  BluetoothAddress;
   HRS_Client_Information_t   SensorInformationHandles;
   struct _tagDevice_Entry_t *NextDeviceEntryPtr;
} Device_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which represents a pointer to a callback.  If the pointer*/
   /* is NULL, then a callback has not been registered.                 */
static HRPM_Event_Callback_Info_t *EventCallbackInfo;

   /* Variable which holds a pointer to the first element of the Device */
   /* Information List (which holds connected HRP LE devices).          */
static Device_Entry_t *DeviceEntryList;

   /* List used to decode the Body Sensor Location byte into an         */
   /* enumerated type.                                                  */
static BTPSCONST HRPM_Body_Sensor_Location_t BodySensorLocationTable[] =
{
   bslOther,
   bslChest,
   bslWrist,
   bslFinger,
   bslHand,
   bslEarLobe,
   bslFoot
} ;

   /* Internal Function Prototypes.                                     */
static Boolean_t AddDeviceEntryActual(Device_Entry_t *EntryToAdd);
static Device_Entry_t *SearchDeviceEntry(BD_ADDR_t *Address);
static Device_Entry_t *DeleteDeviceEntry(BD_ADDR_t *Address);
static void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree);
static void FreeDeviceEntryList(void);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);
static void ProcessGetBodySensorRequestMessage(HRPM_Get_Body_Sensor_Location_Request_t *Message);
static void ProcessResetEnergyExpendedRequestMessage(HRPM_Reset_Energy_Expended_Request_t *Message);
static void ProcessRegisterCollectorEventsRequestMessage(HRPM_Register_Collector_Events_Request_t *Message);
static void ProcessUnRegisterCollectorEventsRequestMessage(HRPM_Un_Register_Collector_Events_Request_t *Message);

static void DispatchLocalHRPMEvent(HRPM_Event_Data_t *HRPMEventData);
static void DispatchLocalHeartRateMeasurementEvent(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent);
static void DispatchRemoteHeartRateMeasurementEvent(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent);
static void DispatchLocalGetBodySensorLocationResponse(Get_Body_Sensor_Location_Response_t *GetBodySensorLocationResponse);
static void DispatchRemoteGetBodySensorLocationResponse(Get_Body_Sensor_Location_Response_t *GetBodySensorLocationResponse);
static void DispatchLocalResetEnergyExpendedResponse(Reset_Energy_Expended_Response_t *ResetEnergyExpendedResponse);
static void DispatchRemoteResetEnergyExpendedResponse(Reset_Energy_Expended_Response_t *ResetEnergyExpendedResponse);
static void DispatchLocalConnectedEvent(Write_Measurement_CCD_Response_t *WriteMeasurementCCDResponse);
static void DispatchRemoteConnectedEvent(Write_Measurement_CCD_Response_t *WriteMeasurementCCDResponse);
static void DispatchLocalDisconnectedEvent(BD_ADDR_t *RemoteSensor);
static void DispatchRemoteDisconnectedEvent(BD_ADDR_t *RemoteSensor);

static Device_Entry_t *DiscoverHeartRateSensor(BD_ADDR_t *BluetoothAddress);
static void DisconnectDeleteDeviceEntry(BD_ADDR_t *BluetoothAddress);

static int GetBodySensorLocation(BD_ADDR_t *RemoteSensor, unsigned int AddressID);
static int ResetEnergyExpended(BD_ADDR_t *RemoteSensor, unsigned int AddressID);
static int WriteMeasurementCCD(Device_Entry_t *DeviceEntry);

static void BTPSAPI BTPMDispatchCallback_HRPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);
static void BTPSAPI BTPSDispatchCallback_GATT(void *CallbackParameter);
static void BTPSAPI BTPSDispatchCallback_HRP_Collector(void *CallbackParameter);

static void BTPSAPI HRPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function adds the specified Device Entry to the     */
   /* module's list.  This function simply adds the entry to the list,  */
   /* it does not allocate a new buffer to store the entry.  This       */
   /* function will return NULL if NO Entry was added.  This can occur  */
   /* if the element passed in was deemed invalid.                      */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Boolean_t AddDeviceEntryActual(Device_Entry_t *EntryToAdd)
{
   return(BSC_AddGenericListEntry_Actual(ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void **)&DeviceEntryList, (void *)EntryToAdd));
}

   /* The following function searches the module's Device Entry List for*/
   /* a Device Entry based on the specified Bluetooth Address.  This    */
   /* function returns NULL if either the Bluetooth Device Address is   */
   /* invalid, or the specified Entry was NOT present in the list.      */
static Device_Entry_t *SearchDeviceEntry(BD_ADDR_t *Address)
{
   return((Device_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)Address, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void **)&DeviceEntryList));
}

   /* The following function searches the module's Device Entry List for*/
   /* the Device Entry with the specified Bluetooth Address and removes */
   /* it from the List.  This function returns NULL if either the       */
   /* Bluetooth Device Address is invalid, or the specified Entry was   */
   /* NOT present in the list.  The entry returned will have the Next   */
   /* Entry field set to NULL, and the caller is responsible for        */
   /* deleting the memory associated with this entry by calling         */
   /* FreeDeviceEntryMemory().                                          */
static Device_Entry_t *DeleteDeviceEntry(BD_ADDR_t *Address)
{
   return((Device_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)Address, BTPS_STRUCTURE_OFFSET(Device_Entry_t, BluetoothAddress), BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr), (void **)&DeviceEntryList));
}

   /* This function frees the specified Device Entry member.  No check  */
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the module's Device Entry List.  Upon return of this   */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceEntryList(void)
{
   BSC_FreeGenericListEntryList((void **)&DeviceEntryList, BTPS_STRUCTURE_OFFSET(Device_Entry_t, NextDeviceEntryPtr));
}

   /* The following function processes a Get Body Sensor Request Message*/
   /* and responds to the message accordingly.  This function does not  */
   /* verify the integrity of the Message (i.e.  the length) because it */
   /* is the caller's responsibility to verify the Message before       */
   /* calling this function.                                            */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessGetBodySensorRequestMessage(HRPM_Get_Body_Sensor_Location_Request_t *Message)
{
   HRPM_Get_Body_Sensor_Location_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status = GetBodySensorLocation(&(Message->RemoteDeviceAddress), Message->MessageHeader.AddressID);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Reset Energy Expended Request  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessResetEnergyExpendedRequestMessage(HRPM_Reset_Energy_Expended_Request_t *Message)
{
   HRPM_Reset_Energy_Expended_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HRPM_RESET_ENERGY_EXPENDED_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status = ResetEnergyExpended(&(Message->RemoteDeviceAddress), Message->MessageHeader.AddressID);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes a Register Collector Events      */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessRegisterCollectorEventsRequestMessage(HRPM_Register_Collector_Events_Request_t *Message)
{
   int                                       Result;
   HRPM_Register_Collector_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that there is not a callback already registered.        */
      if(!EventCallbackInfo)
      {
         if((EventCallbackInfo = BTPS_AllocateMemory(sizeof(HRPM_Event_Callback_Info_t))) != NULL)
         {
            /* Initialize the Event Callback.                           */
            EventCallbackInfo->ClientID          = Message->MessageHeader.AddressID;
            EventCallbackInfo->EventCallback     = NULL;
            EventCallbackInfo->CallbackParameter = 0;

            Result                               = HRPM_COLLECTOR_CALLBACK_ID;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         Result = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_ALREADY_REGISTERED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HRPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes an Un-Register Collector Events  */
   /* Request Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.                                               */
static void ProcessUnRegisterCollectorEventsRequestMessage(HRPM_Un_Register_Collector_Events_Request_t *Message)
{
   int                                          Result;
   HRPM_Un_Register_Collector_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that there is currently a callback registered.          */
      if((EventCallbackInfo) && (EventCallbackInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Free the memory allocated for this event callback.          */
         BTPS_FreeMemory(EventCallbackInfo);

         EventCallbackInfo = NULL;

         /* Return success.                                             */
         Result = 0;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HRPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Module Manager's */
   /*          Lock held.  This function will release the Lock before it*/
   /*          exits (i.e.  the caller SHOULD NOT RELEASE THE LOCK).    */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Body Sensor Location Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_GET_BODY_SENSOR_LOCATION_REQUEST_SIZE)
               ProcessGetBodySensorRequestMessage((HRPM_Get_Body_Sensor_Location_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Reset Energy Expended Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_RESET_ENERGY_EXPENDED_REQUEST_SIZE)
               ProcessResetEnergyExpendedRequestMessage((HRPM_Reset_Energy_Expended_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register HRP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
               ProcessRegisterCollectorEventsRequestMessage((HRPM_Register_Collector_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register HRP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterCollectorEventsRequestMessage((HRPM_Un_Register_Collector_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   /* Verify that a client has registered a callback.                   */
   if(EventCallbackInfo)
   {
      /* Check to see if the current Client Information is the one that */
      /* is being un-registered and it is not running on the server     */
      /* side.                                                          */
      if((EventCallbackInfo->ClientID == ClientID) && (ClientID != MSG_GetServerAddressID()))
      {
         /* Delete the callback.                                        */
         BTPS_FreeMemory(EventCallbackInfo);

         EventCallbackInfo = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process the Module Manager's Asynchronous Events.   */
static void BTPSAPI BTPMDispatchCallback_HRPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Note we do not have to release the Lock because          */
            /* ProcessReceivedMessage() is documented that it will be   */
            /* called with the Lock being held and it will release the  */
            /* Lock when it is finished with it.                        */
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Process the Client Un-Register Message.                  */
            ProcessClientUnRegister((unsigned int)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to the registered event       */
   /* callback.  The function will verify that a callback has been      */
   /* registered.                                                       */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalHRPMEvent(HRPM_Event_Data_t *HRPMEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that a client has registered a callback and the input   */
   /* parameters appear to be semi-valid.                               */
   if(EventCallbackInfo)
   {
      /* Assign the Callback ID before dispatching.                     */
      HRPMEventData->EventCallbackID = HRPM_COLLECTOR_CALLBACK_ID;

      /* Release the Lock before dispatching the callback.              */
      DEVM_ReleaseLock();

      __BTPSTRY
      {
         if(EventCallbackInfo->EventCallback)
            (*EventCallbackInfo->EventCallback)(HRPMEventData, EventCallbackInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }

      /* Re-acquire the Lock.                                           */
      DEVM_AcquireLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Heart Rate Measurement Event.  It is the caller's      */
   /* responsibility to verify the Event before calling this function.  */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalHeartRateMeasurementEvent(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent)
{
   HRPM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Decode the Heart Rate Event data into a local event.              */
   if(_HRPM_Decode_Heart_Rate_Measurement_Event_To_Event(HeartRateMeasurementEvent, &(EventData.EventData.HeartRateMeasurementEventData)) == 0)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                                   = hetHRPHeartRateMeasurement;
      EventData.EventLength                                                 = HRPM_HEART_RATE_MEASUREMENT_EVENT_DATA_SIZE;
      EventData.EventData.HeartRateMeasurementEventData.RemoteDeviceAddress = HeartRateMeasurementEvent->RemoteSensor;

      /* Dispatch the event to the registered callback.                 */
      DispatchLocalHRPMEvent(&EventData);

      /* Free the Heart Rate Measurement Event that was allocated by the*/
      /* decode function.                                               */
      _HRPM_Free_Heart_Rate_Measurement_Event_RR_Interval(&(EventData.EventData.HeartRateMeasurementEventData));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Heart Rate Measurement Message.  It is the caller's    */
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteHeartRateMeasurementEvent(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent)
{
   HRPM_Heart_Rate_Measurement_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Only dispatch the message if there is a registered remote         */
   /* callback.                                                         */
   if(EventCallbackInfo)
   {
      /* Decode the Heart Rate Event data into a remote message.        */
      if((Message = _HRPM_Decode_Heart_Rate_Measurement_Event_To_Message(HeartRateMeasurementEvent)) != NULL)
      {
         /* Format the Message to dispatch.                             */
         Message->MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
         Message->MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_HEART_RATE_MEASUREMENT;
         Message->MessageHeader.MessageLength   = (HRPM_HEART_RATE_MEASUREMENT_MESSAGE_SIZE(Message->NumberOfRRIntervals) - BTPM_MESSAGE_HEADER_SIZE);
         Message->RemoteDeviceAddress           = HeartRateMeasurementEvent->RemoteSensor;

         MSG_SendMessage((BTPM_Message_t *)Message);

         /* Free the message now that it has been sent.                 */
         _HRPM_Free_Heart_Rate_Measurement_Message((void *)Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Get Body Sensor Response Event.  It is the caller's    */
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalGetBodySensorLocationResponse(Get_Body_Sensor_Location_Response_t *GetBodySensorLocationResponse)
{
   HRPM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                = hetHRPGetBodySensorLocationResponse;
   EventData.EventLength                                              = HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE;
   EventData.EventData.BodySensorLocationResponse.RemoteDeviceAddress = GetBodySensorLocationResponse->TransactionData->RemoteSensor;
   EventData.EventData.BodySensorLocationResponse.Status              = GetBodySensorLocationResponse->ResponseStatus;

   /* The Body Sensor Location is only valid upon success.              */
   if((!EventData.EventData.BodySensorLocationResponse.Status) && (GetBodySensorLocationResponse->BodySensorLocation < (sizeof(BodySensorLocationTable)/sizeof(HRPM_Body_Sensor_Location_t))))
      EventData.EventData.BodySensorLocationResponse.Location = BodySensorLocationTable[GetBodySensorLocationResponse->BodySensorLocation];
   else
      EventData.EventData.BodySensorLocationResponse.Location = bslOther;

   /* Dispatch the event to the registered callback.                    */
   DispatchLocalHRPMEvent(&EventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Get Body Sensor Response Message.  It is the caller's  */
   /* responsibility to verify the Response before calling this         */
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteGetBodySensorLocationResponse(Get_Body_Sensor_Location_Response_t *GetBodySensorLocationResponse)
{
   HRPM_Get_Body_Sensor_Location_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EventCallbackInfo)
   {
      if(EventCallbackInfo->ClientID == GetBodySensorLocationResponse->TransactionData->AddressID)
      {
         /* Format the Message to dispatch.                             */
         Message.MessageHeader.AddressID       = GetBodySensorLocationResponse->TransactionData->AddressID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
         Message.MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION_RESPONSE;
         Message.MessageHeader.MessageLength   = (HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
         Message.RemoteDeviceAddress           = GetBodySensorLocationResponse->TransactionData->RemoteSensor;
         Message.Status                        = GetBodySensorLocationResponse->ResponseStatus;

         /* The Body Sensor Location is only valid upon success.        */
         if((!Message.Status) && (GetBodySensorLocationResponse->BodySensorLocation < (sizeof(BodySensorLocationTable)/sizeof(HRPM_Body_Sensor_Location_t))))
            Message.Location = BodySensorLocationTable[GetBodySensorLocationResponse->BodySensorLocation];
         else
            Message.Location = bslOther;

         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Reset Energy Expended Response Event.  It is the       */
   /* caller's responsibility to verify the Response before calling this*/
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalResetEnergyExpendedResponse(Reset_Energy_Expended_Response_t *ResetEnergyExpendedResponse)
{
   HRPM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event that will be dispatched locally.                 */
   EventData.EventType                                                  = hetHRPGetBodySensorLocationResponse;
   EventData.EventLength                                                = HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE;
   EventData.EventData.ResetEnergyExpendedResponse.RemoteDeviceAddress  = ResetEnergyExpendedResponse->TransactionData->RemoteSensor;
   EventData.EventData.ResetEnergyExpendedResponse.Status               = ResetEnergyExpendedResponse->ResponseStatus;

   /* Dispatch the event to the registered callback.                    */
   DispatchLocalHRPMEvent(&EventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Reset Energy Expended Response Message.  It is the     */
   /* caller's responsibility to verify the Response before calling this*/
   /* function.                                                         */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteResetEnergyExpendedResponse(Reset_Energy_Expended_Response_t *ResetEnergyExpendedResponse)
{
   HRPM_Reset_Energy_Expended_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EventCallbackInfo)
   {
      if(EventCallbackInfo->ClientID == ResetEnergyExpendedResponse->TransactionData->AddressID)
      {
         /* Format the Message to dispatch.                             */
         Message.MessageHeader.AddressID       = ResetEnergyExpendedResponse->TransactionData->AddressID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
         Message.MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED_RESPONSE;
         Message.MessageHeader.MessageLength   = (HRPM_RESET_ENERGY_EXPENDED_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
         Message.RemoteDeviceAddress           = ResetEnergyExpendedResponse->TransactionData->RemoteSensor;
         Message.Status                        = ResetEnergyExpendedResponse->ResponseStatus;

         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Event Event resulting from a successful CCD  */
   /* Response.  It is the caller's responsibility to verify the        */
   /* Response before calling this function.                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalConnectedEvent(Write_Measurement_CCD_Response_t *WriteMeasurementCCDResponse)
{
   Device_Entry_t    *DeviceEntry;
   HRPM_Event_Data_t  EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((DeviceEntry = SearchDeviceEntry(&(WriteMeasurementCCDResponse->TransactionData->RemoteSensor))) != NULL)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = hetHRPConnected;
      EventData.EventLength                                      = HRPM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = WriteMeasurementCCDResponse->TransactionData->RemoteSensor;
      EventData.EventData.ConnectedEventData.ConnectionType      = hctCollector;

      if(DeviceEntry->SensorInformationHandles.Body_Sensor_Location)
         EventData.EventData.ConnectedEventData.ConnectedFlags = HRPM_CONNECTED_FLAGS_BODY_SENSOR_LOCATION_SUPPORTED;
      else
         EventData.EventData.ConnectedEventData.ConnectedFlags = 0;

      if(DeviceEntry->SensorInformationHandles.Heart_Rate_Control_Point)
         EventData.EventData.ConnectedEventData.ConnectedFlags |= HRPM_CONNECTED_FLAGS_RESET_ENERGY_EXPENDED_SUPPORTED;

      /* Dispatch the event to the registered callback.                 */
      DispatchLocalHRPMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Event Message resulting from a successful CCD*/
   /* Response.  It is the caller's responsibility to verify the        */
   /* Response before calling this function.                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteConnectedEvent(Write_Measurement_CCD_Response_t *WriteMeasurementCCDResponse)
{
   Device_Entry_t           *DeviceEntry;
   HRPM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((DeviceEntry = SearchDeviceEntry(&(WriteMeasurementCCDResponse->TransactionData->RemoteSensor))) != NULL)
   {
      /* Format the Message to dispatch.                                */
      Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
      Message.MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (HRPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      Message.RemoteDeviceAddress           = WriteMeasurementCCDResponse->TransactionData->RemoteSensor;
      Message.ConnectionType                = hctCollector;

      if(DeviceEntry->SensorInformationHandles.Body_Sensor_Location)
         Message.ConnectedFlags = HRPM_CONNECTED_FLAGS_BODY_SENSOR_LOCATION_SUPPORTED;
      else
         Message.ConnectedFlags = 0;

      if(DeviceEntry->SensorInformationHandles.Heart_Rate_Control_Point)
         Message.ConnectedFlags |= HRPM_CONNECTED_FLAGS_RESET_ENERGY_EXPENDED_SUPPORTED;

      MSG_SendMessage((BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Event Event.  It is the caller's          */
   /* responsibility to verify the Address before calling this function.*/
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchLocalDisconnectedEvent(BD_ADDR_t *RemoteSensor)
{
   HRPM_Event_Data_t EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(SearchDeviceEntry(RemoteSensor))
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                           = hetHRPDisconnected;
      EventData.EventLength                                         = HRPM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = *RemoteSensor;
      EventData.EventData.DisconnectedEventData.ConnectionType      = hctCollector;

      /* Dispatch the event to the registered callback.                 */
      DispatchLocalHRPMEvent(&EventData);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Event Message.  It is the caller's        */
   /* responsibility to verify the Address before calling this function.*/
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DispatchRemoteDisconnectedEvent(BD_ADDR_t *RemoteSensor)
{
   HRPM_Disconnected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(SearchDeviceEntry(RemoteSensor))
   {
      if(EventCallbackInfo)
      {
         /* Format the Message to dispatch.                             */
         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER;
         Message.MessageHeader.MessageFunction = HRPM_MESSAGE_FUNCTION_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HRPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
         Message.RemoteDeviceAddress           = *RemoteSensor;
         Message.ConnectionType                = hctCollector;

         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Notification Events.                   */
static void BTPSAPI BTPSDispatchCallback_GATT(void *CallbackParameter)
{
   Device_Entry_t                  *DeviceEntry;
   Heart_Rate_Measurement_Event_t   HeartRateMeasurementEvent;
   GATT_Server_Notification_Data_t *GATTServerNotificationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, let's check to see if we need to process it.          */
         if(Initialized)
         {
            GATTServerNotificationData = &(((HRPM_Update_Data_t *)CallbackParameter)->UpdateData.GATTServerNotificationData);

            /* Verify that the notification has occurred on the Heart   */
            /* Rate attribute handle.                                   */
            if(((DeviceEntry = SearchDeviceEntry(&(GATTServerNotificationData->RemoteDevice))) != NULL) && (DeviceEntry->SensorInformationHandles.Heart_Rate_Measurement == GATTServerNotificationData->AttributeHandle))
            {
               /* Notification is a Heart Rate notification, go ahead   */
               /* and format up the event and dispatch it.              */
               HeartRateMeasurementEvent.RemoteSensor = GATTServerNotificationData->RemoteDevice;
               HeartRateMeasurementEvent.BufferLength = GATTServerNotificationData->AttributeValueLength;
               HeartRateMeasurementEvent.Buffer       = GATTServerNotificationData->AttributeValue;

               /* Verify that there is measurement data.                */
               if(HeartRateMeasurementEvent.BufferLength)
               {
                  /* Verify that a callback has been registered.        */
                  if(EventCallbackInfo)
                  {
                     /* Check the callback type.                        */
                     if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
                        DispatchLocalHeartRateMeasurementEvent(&HeartRateMeasurementEvent);
                     else
                     {
                        if(EventCallbackInfo->ClientID)
                           DispatchRemoteHeartRateMeasurementEvent(&HeartRateMeasurementEvent);
                     }
                  }
               }
            }
         }
      }

      /* Release the lock because we are finished with it.              */
      DEVM_ReleaseLock();

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HRP Collector Events.                       */
static void BTPSAPI BTPSDispatchCallback_HRP_Collector(void *CallbackParameter)
{
   Device_Entry_t             *DeviceEntry;
   HRP_Collector_Event_Data_t *EventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, let's check to see if we need to process it.          */
         if(Initialized)
         {
            EventData = &(((HRPM_Update_Data_t *)CallbackParameter)->UpdateData.CollectorEventData);

            switch(EventData->EventDataType)
            {
               case cetGetBodySensorLocationResponse:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Processing cetGetBodySensorLocationResponse\n"));

                  if((EventData->EventData.GetBodySensorLocationResponse.TransactionData) && (EventData->EventData.GetBodySensorLocationResponse.TransactionData->AddressID == MSG_GetServerAddressID()))
                     DispatchLocalGetBodySensorLocationResponse(&(EventData->EventData.GetBodySensorLocationResponse));
                  else
                     DispatchRemoteGetBodySensorLocationResponse(&(EventData->EventData.GetBodySensorLocationResponse));

                  /* Free the transaction data allocated for this       */
                  /* response.                                          */
                  if(EventData->EventData.GetBodySensorLocationResponse.TransactionData)
                     BTPS_FreeMemory((void *)EventData->EventData.GetBodySensorLocationResponse.TransactionData);
                  break;
               case cetResetEnergyExpendedResponse:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Processing cetResetEnergyExpendedResponse\n"));

                  if((EventData->EventData.ResetEnergyExpendedResponse.TransactionData) && (EventData->EventData.ResetEnergyExpendedResponse.TransactionData->AddressID == MSG_GetServerAddressID()))
                     DispatchLocalResetEnergyExpendedResponse(&(EventData->EventData.ResetEnergyExpendedResponse));
                  else
                     DispatchRemoteResetEnergyExpendedResponse(&(EventData->EventData.ResetEnergyExpendedResponse));

                  /* Free the transaction data allocated for this       */
                  /* response.                                          */
                  if(EventData->EventData.ResetEnergyExpendedResponse.TransactionData)
                     BTPS_FreeMemory((void *)EventData->EventData.ResetEnergyExpendedResponse.TransactionData);
                  break;
               case cetWriteMeasurementCCDResponse:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Processing cetWriteMeasurementCCDResponse\n"));

                  /* Verify a successful response before notifying the  */
                  /* collector about a connected HRP sensor.            */
                  if(!(EventData->EventData.WriteMeasurementCCDResponse.ResponseStatus))
                  {
                     /* Verify that a callback has been registered.     */
                     if(EventCallbackInfo)
                     {
                        /* Check the callback type.                     */
                        if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
                           DispatchLocalConnectedEvent(&(EventData->EventData.WriteMeasurementCCDResponse));
                        else
                        {
                           if(EventCallbackInfo->ClientID)
                              DispatchRemoteConnectedEvent(&(EventData->EventData.WriteMeasurementCCDResponse));
                        }
                     }
                  }
                  else
                  {
                     /* Writing to the Measurement CCD failed.  This    */
                     /* device should not be considered as supporting   */
                     /* HRS.                                            */
                     if((DeviceEntry = DeleteDeviceEntry(&(EventData->EventData.WriteMeasurementCCDResponse.TransactionData->RemoteSensor))) != NULL)
                        FreeDeviceEntryMemory(DeviceEntry);
                  }

                  /* Free the transaction data allocated for this       */
                  /* response.                                          */
                  if(EventData->EventData.WriteMeasurementCCDResponse.TransactionData)
                     BTPS_FreeMemory((void *)EventData->EventData.WriteMeasurementCCDResponse.TransactionData);
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Unknown collector event type.\n"));
                  break;
            }
         }
      }

      /* Release the lock because we are finished with it.              */
      DEVM_ReleaseLock();

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following is a utility function used to discover a Heart Rate */
   /* Sensor on a remote device.  If a valid Heart Rate Sensor is found,*/
   /* then a Device Entry is added to the module's list and a request to*/
   /* configure the CCD of the sensor will be submitted.  On success, a */
   /* pointer to the new device entry will be returned; otherwise, NULL */
   /* will be returned.                                                 */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static Device_Entry_t *DiscoverHeartRateSensor(BD_ADDR_t *BluetoothAddress)
{
   int                                       Result;
   GATT_UUID_t                               UUID;
   Device_Entry_t                           *DeviceEntry = NULL;
   unsigned int                              ServiceDataSize;
   unsigned int                              idx;
   unsigned int                              idx2;
   unsigned char                            *ServiceData;
   DEVM_Parsed_Services_Data_t               ParsedGATTData;
   GATT_Characteristic_Information_t        *CharacteristicInformation;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Get the buffer size required to parse the service data.           */
   if((Result = DEVM_QueryRemoteDeviceServices(*BluetoothAddress, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &ServiceDataSize)) == 0)
   {
      /* Allocate the buffer required to parse the service data         */
      if((ServiceData = (unsigned char *)BTPS_AllocateMemory(ServiceDataSize)) != NULL)
      {
         /* Query the services for the remote device.                   */
         if((Result = DEVM_QueryRemoteDeviceServices(*BluetoothAddress, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, ServiceDataSize, ServiceData, NULL)) > 0)
         {
            /* Convert the Raw GATT Stream to a Parsed GATT Stream.     */
            Result = DEVM_ConvertRawServicesStreamToParsedServicesData(ServiceDataSize, ServiceData, &ParsedGATTData);
            if(!Result)
            {
               /* Iterate the services on the remote device.            */
               for(idx = ParsedGATTData.NumberServices; idx--;)
               {
                  UUID = ParsedGATTData.GATTServiceDiscoveryIndicationData[idx].ServiceInformation.UUID;

                  /* Check for HRS.                                     */
                  if((UUID.UUID_Type == guUUID_16) && (HRS_COMPARE_HRS_SERVICE_UUID_TO_UUID_16(UUID.UUID.UUID_16)))
                  {
                     /* Add the device to the list.                     */
                     if((DeviceEntry = (Device_Entry_t *)BTPS_AllocateMemory(sizeof(Device_Entry_t))) != NULL)
                     {
                        /* Initalize device information.                */
                        DeviceEntry->BluetoothAddress = *BluetoothAddress;

                        BTPS_MemInitialize(&(DeviceEntry->SensorInformationHandles), 0, sizeof(HRS_Client_Information_t));

                        GATTServiceDiscoveryIndicationData = &(ParsedGATTData.GATTServiceDiscoveryIndicationData[idx]);

                        /* Iterate the HRS characteristics.             */
                        for(idx = GATTServiceDiscoveryIndicationData->NumberOfCharacteristics; idx--;)
                        {
                           CharacteristicInformation = &(GATTServiceDiscoveryIndicationData->CharacteristicInformationList[idx]);
                           UUID                      = CharacteristicInformation->Characteristic_UUID;

                           if(UUID.UUID_Type == guUUID_16)
                           {
                              /* Check for Measurement UUID.            */
                              if((HRS_COMPARE_HRS_HEART_RATE_MEASUREMENT_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
                              {
                                 DeviceEntry->SensorInformationHandles.Heart_Rate_Measurement = CharacteristicInformation->Characteristic_Handle;

                                 for(idx2 = CharacteristicInformation->NumberOfDescriptors; idx2--;)
                                 {
                                    UUID = CharacteristicInformation->DescriptorList[idx2].Characteristic_Descriptor_UUID;

                                    /* Check for Measurement CCD UUID.  */
                                    if((UUID.UUID_Type == guUUID_16) && (GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16)))
                                       DeviceEntry->SensorInformationHandles.Heart_Rate_Measurement_Client_Configuration = CharacteristicInformation->DescriptorList[idx2].Characteristic_Descriptor_Handle;
                                 }
                              }
                              else
                              {
                                 /* Check for Body Sensor Location UUID.*/
                                 if((HRS_COMPARE_HRS_BODY_SENSOR_LOCATION_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                                    DeviceEntry->SensorInformationHandles.Body_Sensor_Location = CharacteristicInformation->Characteristic_Handle;
                                 else
                                 {
                                    /* Check for Control Point UUID.    */
                                    if((HRS_COMPARE_HRS_HEART_RATE_CONTROL_POINT_UUID_TO_UUID_16(UUID.UUID.UUID_16)) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                                       DeviceEntry->SensorInformationHandles.Heart_Rate_Control_Point = CharacteristicInformation->Characteristic_Handle;
                                 }
                              }
                           }
                        }

                        /* At a minimum, HRS must support the           */
                        /* Measurement Characteristic and it's CCD.  If */
                        /* not found, then the service is not supported.*/
                        if(!((DeviceEntry->SensorInformationHandles.Heart_Rate_Measurement) && (DeviceEntry->SensorInformationHandles.Heart_Rate_Measurement_Client_Configuration)))
                        {
                           /* HRS is invalid.                           */
                           BTPS_FreeMemory(DeviceEntry);

                           DeviceEntry = NULL;
                        }
                     }

                     break;
                  }
               }

               /* All finished with the parsed data, so free it.        */
               DEVM_FreeParsedServicesData(&ParsedGATTData);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_ConvertRawServicesStreamToParsedServicesData returned %d.\n", Result));
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
         }

         /* Free the previously allocated buffer holding service data   */
         /* information.                                                */
         BTPS_FreeMemory(ServiceData);
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FAILURE), ("Allocation request for Service Data failed, size = %u.\n", ServiceDataSize));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FAILURE), ("DEVM_QueryRemoteDeviceServices returned %d.\n", Result));
   }

   /* A device entry was created.  This means that a valid HRS has been */
   /* found on the remote device.  Add the device to the list of known  */
   /* services.                                                         */
   if(DeviceEntry)
   {
      /* The last step before notifying the collector is to configure   */
      /* the remote HRS.                                                */
      if((AddDeviceEntryActual(DeviceEntry) == FALSE) || (WriteMeasurementCCD(DeviceEntry)))
      {
         BTPS_FreeMemory(DeviceEntry);

         DeviceEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit %p\n", DeviceEntry));

   return(DeviceEntry);
}

   /* The following is a utility function used to dispatch a disconnect */
   /* event to the BTPM Client (if registered) and to delete the        */
   /* corresponding device entry in the module's list.  If the device   */
   /* entry does not exist, then nothing will be done.                  */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Lock held.                                               */
static void DisconnectDeleteDeviceEntry(BD_ADDR_t *BluetoothAddress)
{
   Device_Entry_t *DeviceEntry;

   if(EventCallbackInfo)
   {
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
         DispatchLocalDisconnectedEvent(BluetoothAddress);
      else
      {
         if(EventCallbackInfo->ClientID)
            DispatchRemoteDisconnectedEvent(BluetoothAddress);
      }
   }

   /* Delete the device if it is known.                                 */
   if((DeviceEntry = DeleteDeviceEntry(BluetoothAddress)) != NULL)
      FreeDeviceEntryMemory(DeviceEntry);
}

   /* The following is a utility function that submits a Get Body Sensor*/
   /* Location request to the lower level HRP implementation.  The      */
   /* Address ID should also be passed in to identify where the response*/
   /* should be sent.                                                   */
static int GetBodySensorLocation(BD_ADDR_t *RemoteSensor, unsigned int AddressID)
{
   int                                          ret_val;
   Device_Entry_t                              *DeviceEntry;
   Get_Body_Sensor_Location_Transaction_Data_t *TransactionData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock that guards access to this module.    */
   if(DEVM_AcquireLock())
   {
      /* Verify that a callback has been registered with this client.   */
      if((EventCallbackInfo) && (EventCallbackInfo->ClientID == AddressID))
      {
         /* Find the remote device.                                     */
         if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
         {
            /* Verify that the Body Sensor Location Characteristic is   */
            /* supported.                                               */
            if(DeviceEntry->SensorInformationHandles.Body_Sensor_Location)
            {
               /* Allocate the transaction data.                        */
               if((TransactionData = BTPS_AllocateMemory(sizeof(Get_Body_Sensor_Location_Transaction_Data_t))) != NULL)
               {
                  TransactionData->AddressID    = AddressID;
                  TransactionData->RemoteSensor = *RemoteSensor;

                  /* Submit the command request.                        */
                  if((ret_val = _HRPM_Get_Body_Sensor_Location(TransactionData, DeviceEntry->SensorInformationHandles.Body_Sensor_Location)) != 0)
                     BTPS_FreeMemory(TransactionData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEART_RATE_BODY_SENSOR_LOCATION_NOT_SUPPORTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_HEART_RATE_SENSOR_NOT_AVAILABLE;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_NOT_REGISTERED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function that submits a Reset Energy   */
   /* Expended request to the lower level HRP implementation.  The      */
   /* Address ID should also be passed in to identify where the response*/
   /* should be sent.                                                   */
static int ResetEnergyExpended(BD_ADDR_t *RemoteSensor, unsigned int AddressID)
{
   int                                       ret_val;
   Device_Entry_t                           *DeviceEntry;
   Reset_Energy_Expended_Transaction_Data_t *TransactionData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Verify that a callback has been registered with this client.   */
      if((EventCallbackInfo) && (EventCallbackInfo->ClientID == AddressID))
      {
         if((DeviceEntry = SearchDeviceEntry(RemoteSensor)) != NULL)
         {
            /* Verify that the control point is supported.  If it is    */
            /* not, then Reset Energy Expended is also not supported.   */
            if(DeviceEntry->SensorInformationHandles.Heart_Rate_Control_Point)
            {
               /* Allocate the transaction data.                        */
               if((TransactionData = BTPS_AllocateMemory(sizeof(Reset_Energy_Expended_Transaction_Data_t))) != NULL)
               {
                  TransactionData->AddressID    = AddressID;
                  TransactionData->RemoteSensor = *RemoteSensor;

                  /* Submit the command request.                        */
                  if((ret_val = _HRPM_Reset_Energy_Expended(TransactionData, DeviceEntry->SensorInformationHandles.Heart_Rate_Control_Point)) != 0)
                     BTPS_FreeMemory(TransactionData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEART_RATE_RESET_ENERGY_EXPENDED_NOT_SUPPORTED;
         }
         else
            ret_val = BTPM_ERROR_CODE_HEART_RATE_SENSOR_NOT_AVAILABLE;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_NOT_REGISTERED;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following is a utility function that submits a Write          */
   /* Measurement CCD request to the lower level HRP implementation.    */
static int WriteMeasurementCCD(Device_Entry_t *DeviceEntry)
{
   int                                       ret_val;
   Write_Measurement_CCD_Transaction_Data_t *TransactionData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Wait for access to the lock for this module.                      */
   if(DEVM_AcquireLock())
   {
      /* Allocate the transaction data.                                 */
      if((TransactionData = BTPS_AllocateMemory(sizeof(Reset_Energy_Expended_Transaction_Data_t))) != NULL)
      {
         TransactionData->RemoteSensor = DeviceEntry->BluetoothAddress;

         /* Submit the command request.                                 */
         if((ret_val = _HRPM_Write_Measurement_CCD(TransactionData, DeviceEntry->SensorInformationHandles.Heart_Rate_Measurement_Client_Configuration, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)) != 0)
            BTPS_FreeMemory(TransactionData);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Module Manager Messages.*/
static void BTPSAPI HRPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("HRP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HRP Manager defined    */
            /* Message.  If it is it will be within the range:          */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* HRP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HRPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

                  MSG_FreeReceivedMessageGroupHandlerMessage(Message);
               }
            }
            else
            {
               /* Dispatch to the main handler that a client has        */
               /* un-registered.                                        */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_MSG, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HRP Manager defined   */
            /* Message.  If it is it will be within the range:          */
            /*                                                          */
            /*    - BTPM_MESSAGE_FUNCTION_MINIMUM                       */
            /*    - BTPM_MESSAGE_FUNCTION_MAXIMUM                       */
            /*                                                          */
            /* See BTPMMSGT.h for more information on message functions */
            /* that are defined outside of this range.                  */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
               MSG_FreeReceivedMessageGroupHandlerMessage(Message);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* this Bluetopia Platform Manager Module.  This function should be  */
   /* registered with the Bluetopia Platform Manager Module Handler and */
   /* will be called when the Platform Manager is initialized (or shut  */
   /* down).                                                            */
void BTPSAPI HRPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HRP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process HRP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER, HRPManagerGroupHandler, NULL))
         {
            /* Initialize the HRP Manager Implementation Module (this is*/
            /* the module that is responsible for implementing the HRP  */
            /* Manager functionality - this module is just the framework*/
            /* shell).                                                  */
            if(!(Result = _HRPM_Initialize()))
            {
               /* Flag that there is no Event Callback handler          */
               /* registered.                                           */
               EventCallbackInfo = NULL;

               /* Flag that this module is initialized.                 */
               Initialized       = TRUE;

               /* Flag success.                                         */
               Result            = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _HRPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("HRP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the HRP Manager Implementation that  */
            /* we are shutting down.                                    */
            _HRPM_Cleanup();

            /* Free the Connection Info list.                           */
            FreeDeviceEntryList();

            /* Free the memory allocated for this event callback.       */
            if(EventCallbackInfo)
            {
               BTPS_FreeMemory(EventCallbackInfo);

               EventCallbackInfo = NULL;
            }

            /* Flag that this module is no longer initialized.          */
            Initialized = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HRPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int             Result;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HRP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Set the Bluetooth ID in the lower level module if     */
               /* available.                                            */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _HRPM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Clear the Bluetooth ID in the lower level module .    */
               _HRPM_SetBluetoothStackID(0);

               /* Free the Connection Info list.                        */
               FreeDeviceEntryList();
               break;
            case detRemoteDeviceFound:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDeviceFound\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_FOUND_EVENT_DATA_SIZE)
               {
                  DisconnectDeleteDeviceEntry(&(EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties.BD_ADDR));

                  /* Check to see if this is an LE device, services are */
                  /* known, and it is connected.                        */
                  if((EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties.RemoteDeviceFlags & (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)) == (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Connected LE Device with Known Services Found\n"));

                     /* Update Device Entry with service discovery      */
                     /* information.                                    */
                     DiscoverHeartRateSensor(&(EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties.BD_ADDR));
                  }
               }
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDevicePropertiesChanged\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_EVENT_DATA_SIZE)
               {
                  /* We only care if the device flags field has changed */
                  if((EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Remote Device Flags: 0x%08lX\n", EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.RemoteDeviceFlags));

                     /* Check to see if this is an LE device, services  */
                     /* are known, and it is connected.                 */
                     if((EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.RemoteDeviceFlags & (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)) == (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE))
                     {
                        /* If the device is known, then only parse the  */
                        /* services if they have changed.               */
                        if(((DeviceEntry = SearchDeviceEntry(&(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR))) != NULL) && (EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
                        {
                           /* Delete the entry before re-discovering    */
                           /* services.                                 */
                           DisconnectDeleteDeviceEntry(&(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR));

//xxx This registers as a disconnect/reconnect. Find a way around this.

                           DeviceEntry = NULL;
                        }

                        /* Parse the services for HRS only if a device  */
                        /* entry does not exist.                        */
                        if(!DeviceEntry)
                        {
                           /* Update Device Entry with service discovery*/
                           /* information.                              */
                           DiscoverHeartRateSensor(&(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR));
                        }
                     }
                     else
                     {
                        /* The entry is not needed anymore.  This will  */
                        /* also dispatch a disconnect if the device is  */
                        /* known.                                       */
                        DisconnectDeleteDeviceEntry(&(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR));
                     }
                  }
               }
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("detRemoteDeviceDeleted\n"));

               if(EventData->EventLength >= DEVM_REMOTE_DEVICE_DELETED_EVENT_DATA_SIZE)
               {
                  /* Delete the device if it is known.                  */
                  DisconnectDeleteDeviceEntry(&(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR));
               }
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Module Manager of a specific Update Event.  The     */
   /* Module Manager can then take the correct action to process the    */
   /* update.                                                           */
Boolean_t HRPM_NotifyUpdate(HRPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utGATTNotificationEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Notification Event\n"));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPSDispatchCallback_GATT, (void *)UpdateData);
            break;
         case utHRPCollectorEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Collector Event: %d\n", UpdateData->UpdateData.CollectorEventData.EventDataType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPSDispatchCallback_HRP_Collector, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Heart Rate (HRP) */
   /* Manager Service.  This Callback will be dispatched by the HRP     */
   /* Manager when various HRP Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a HRP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HRPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI HRPM_Register_Collector_Event_Callback(HRPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            if(!EventCallbackInfo)
            {
               if((EventCallbackInfo = BTPS_AllocateMemory(sizeof(HRPM_Event_Callback_Info_t))) != NULL)
               {
                  /* Initialize the Event Callback.                     */
                  EventCallbackInfo->ClientID          = MSG_GetServerAddressID();
                  EventCallbackInfo->EventCallback     = CallbackFunction;
                  EventCallbackInfo->CallbackParameter = CallbackParameter;

                  ret_val                              = HRPM_COLLECTOR_CALLBACK_ID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_ALREADY_REGISTERED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEART_RATE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HRP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HRPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the HRP Manager Event Callback ID (return value  */
   /* from HRPM_Register_Collector_Event_Callback() function).          */
void BTPSAPI HRPM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(CollectorCallbackID == HRPM_COLLECTOR_CALLBACK_ID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that there is currently a callback registered.    */
            if(EventCallbackInfo)
            {
               /* Free the memory allocated for this event callback.    */
               BTPS_FreeMemory(EventCallbackInfo);

               EventCallbackInfo = NULL;
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Body Sensor Location Request to a remote sensor.  This      */
   /* function accepts as input the Callback ID (return value from      */
   /* HRPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Body Sensor Location from.  This     */
   /* function returns zero on success; otherwise, a negative error     */
   /* value is returned.                                                */
int BTPSAPI HRPM_Get_Body_Sensor_Location(unsigned int CollectorCallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(CollectorCallbackID == HRPM_COLLECTOR_CALLBACK_ID)
         ret_val = GetBodySensorLocation(&RemoteSensor, MSG_GetServerAddressID());
      else
         ret_val = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_NOT_REGISTERED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEART_RATE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to submit */
   /* a Reset Energy Expended Request to a remote sensor.  This function*/
   /* accepts as input the Callback ID (return value from               */
   /* HRPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the execution of the Reset Energy        */
   /* Expended command.  This function returns zero on success;         */
   /* otherwise, a negative error value is returned.                    */
int BTPSAPI HRPM_Reset_Energy_Expended(unsigned int CollectorCallbackID, BD_ADDR_t RemoteSensor)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(CollectorCallbackID == HRPM_COLLECTOR_CALLBACK_ID)
      {
         /* Configure the Body Sensor Location Request.                 */
         ret_val = ResetEnergyExpended(&RemoteSensor, MSG_GetServerAddressID());
      }
      else
         ret_val = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_NOT_REGISTERED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HEART_RATE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


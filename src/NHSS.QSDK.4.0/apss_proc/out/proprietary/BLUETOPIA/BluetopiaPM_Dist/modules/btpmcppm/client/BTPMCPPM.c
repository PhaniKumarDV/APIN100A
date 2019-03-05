/*****< BTPMCPPM.c >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  BTPMCPPM - Cycling Power Platform Manager Client                          */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants        */
#include "CPPMType.h"            /* CPP Manager Type Definitions              */
#include "CPPMAPI.h"             /* CPP Manager Prototypes/Constants          */
#include "CPPMMSG.h"             /* BTPM CPP Manager Message Formats          */
#include "CPPMUTIL.h"            /* CPP Manager Impl. Prototypes/Constants    */
#include "CPPMGR.h"              /* CPP Internal Prototypes                   */

static Callback_Entry_t *CallbackEntryList;

static Boolean_t         Initialized;
static unsigned int      NextCallbackID;
static unsigned int      NextTransactionID;
static Mutex_t           CPPManagerMutex;


   /*********************************************************************/
   /* Registered in the initialization handler:                         */
   /*********************************************************************/
static void BTPSAPI ProcessMessageCPPM(BTPM_Message_t *Message, void *CallbackParameter);


   /*********************************************************************/
   /* Queued by ProcessMessageCPPM:                                     */
   /*********************************************************************/
static void BTPSAPI ProcessServerMessageCPPM(void *CallbackParameter);
static void BTPSAPI ProcessUnregisterMessage(void *CallbackParameter);


   /*********************************************************************/
   /* Called by ProcessServerMessageCPPM:                               */
   /*********************************************************************/
static void ConnectedEventCPPM(CPPM_Connected_Message_t *Message);
static void DisconnectedEventCPPM(CPPM_Disconnected_Message_t *Message);
static void WriteResponseEventCPPM(CPPM_Write_Response_Message_t *Message);
static void MeasurementEventCPPM(CPPM_Measurement_Message_t *Message);
static void VectorEventCPPM(CPPM_Vector_Message_t *Message);
static void ControlPointEventCPPM(CPPM_Control_Point_Message_t *Message);
static void SensorFeaturesEventCPPM(CPPM_Sensor_Features_Message_t *Message);
static void SensorLocationEventCPPM(CPPM_Sensor_Location_Message_t *Message);


   /*********************************************************************/
   /* Called by EventCPPM functions:                                    */
   /*********************************************************************/
static void EventCPPM(Callback_Entry_t *CallbackEntry, CPPM_Event_Data_t *EventDataCPPM);

   /* CPPM_InitializationHandlerFunction is included in the MODC        */
   /* ModuleHandlerList array and is called to initalize the module.    */
void BTPSAPI CPPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing CPP Manager\n"));

         Result = 0;

         if((CPPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Register the Message Group handler                       */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER, ProcessMessageCPPM, NULL))
            {
               /* Initialize a Callback ID.                             */
               NextCallbackID    = 0x000000001;

               /* Initialize a Transaction ID.                          */
               NextTransactionID = 0x000000001;

               Initialized       = TRUE;
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;

         if(Result < 0)
         {
            if(CPPManagerMutex)
            {
               BTPS_CloseMutex(CPPManagerMutex);
               CPPManagerMutex = NULL;
            }

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CPP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* Unregister the Message Group handler.                       */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER);

         /* Wait for access to the mutex.                               */
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Unregister the callbacks.                                */
            CallbackEntry = CallbackEntryList;
            while(CallbackEntry)
            {
               _CPPM_Unregister_Collector_Event_Callback(CallbackEntry->CallbackID);

               CallbackEntry = CallbackEntry->NextCallbackEntry;
            }

            /* Free the Callback Entry List.                            */
            FreeCallbackEntryList(&CallbackEntryList);

            /* Close the CPP Manager Mutex.                             */
            BTPS_CloseMutex(CPPManagerMutex);

            CPPManagerMutex     = NULL;

            Initialized         = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* CPPM_DeviceManagerHandlerFunction is included in the MODC         */
   /* ModuleHandlerList array. It is the DEVM callback function for the */
   /* module.                                                           */
void BTPSAPI CPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Acquire the mutex.                                             */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On\n"));

               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off\n"));

               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Registered in the initialization handler, ProcessMessageCPPM      */
   /* queues both the callback for messages from the PM server and the  */
   /* callback that handles the loss of the connection to the server.   */
static void BTPSAPI ProcessMessageCPPM(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Check the group.                                               */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         if(Initialized)
         {
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Post the message.                                     */
               if(!BTPM_QueueMailboxCallback(ProcessServerMessageCPPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Cycling Power Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

                  MSG_FreeReceivedMessageGroupHandlerMessage(Message);
               }
            }
            else
            {
               /* The server connection has been lost.                  */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Manager Message: lost PM server connection\n"));

                     if(!BTPM_QueueMailboxCallback(ProcessUnregisterMessage, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Cycling Power Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
               MSG_FreeReceivedMessageGroupHandlerMessage(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Queued by the message group handler, ProcessServerMessageCPPM     */
   /* processes messages from the platform manager server.              */
static void BTPSAPI ProcessServerMessageCPPM(void *CallbackParameter)
{
   BTPM_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackParameter)
   {
      if(Initialized)
      {
         /* Acquire the mutex.                                          */
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Process the Message.                                     */
            Message = (BTPM_Message_t *)CallbackParameter;

            if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Message Function: 0x%08X\n", Message->MessageHeader.MessageFunction));

               switch(Message->MessageHeader.MessageFunction)
               {
                  case CPPM_MESSAGE_FUNCTION_CONNECTED:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Connected\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_CONNECTED_MESSAGE_SIZE)
                     {
                        ConnectedEventCPPM((CPPM_Connected_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_DISCONNECTED:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Disconnected\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_DISCONNECTED_MESSAGE_SIZE)
                     {
                        DisconnectedEventCPPM((CPPM_Disconnected_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_WRITE_RESPONSE:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Write Response\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_WRITE_RESPONSE_MESSAGE_SIZE)
                     {
                        WriteResponseEventCPPM((CPPM_Write_Response_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_MEASUREMENT:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Measurement\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_MEASUREMENT_MESSAGE_SIZE)
                     {
                        MeasurementEventCPPM((CPPM_Measurement_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_VECTOR:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Vector\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_VECTOR_MESSAGE_SIZE(((CPPM_Vector_Message_t *)Message)->MagnitudeDataLength))
                     {
                        VectorEventCPPM((CPPM_Vector_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_CONTROL_POINT:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Control Point\n"));

                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("MessageLength: %u CPPM_Length: %lu\n", Message->MessageHeader.MessageLength, CPPM_CONTROL_POINT_MESSAGE_SIZE(((CPPM_Control_Point_Message_t *)Message)->ParameterLength)));
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_CONTROL_POINT_MESSAGE_SIZE(((CPPM_Control_Point_Message_t *)Message)->ParameterLength))
                     {
                        ControlPointEventCPPM((CPPM_Control_Point_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_SENSOR_FEATURES:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Sensor Features\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_SENSOR_FEATURES_MESSAGE_SIZE)
                     {
                        SensorFeaturesEventCPPM((CPPM_Sensor_Features_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  case CPPM_MESSAGE_FUNCTION_SENSOR_LOCATION:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Sensor Location\n"));

                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_SENSOR_LOCATION_MESSAGE_SIZE)
                     {
                        SensorLocationEventCPPM((CPPM_Sensor_Location_Message_t *)Message);
                     }
                     else
                        BTPS_ReleaseMutex(CPPManagerMutex);

                     break;

                  default:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Cycling Power Message\n"));
                     BTPS_ReleaseMutex(CPPManagerMutex);
                     break;
               }
            }
            else
               BTPS_ReleaseMutex(CPPManagerMutex);
         }
      }

      /* Free the message.                                              */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Queued by the message group handler, ProcessUnregisterMessageCPPM */
   /* processes the loss of the connection to the PM server.            */
static void BTPSAPI ProcessUnregisterMessage(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackParameter)
   {
      if(Initialized)
      {
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete callback entries.                                 */
            FreeCallbackEntryList(&CallbackEntryList);

            BTPS_ReleaseMutex(CPPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, ConnectedEventCPPM is run     */
   /* when connected event data is received from the server.            */
   /* ConnectionType will be the local role in the cycling power        */
   /* connection. ConnectionFlags is not currently used.                */
   /* NumberOfInstances equals the number of sensor instances on the    */
   /* remote device.                                                    */
static void ConnectedEventCPPM(CPPM_Connected_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                        = cetConnectedCPP;
      EventDataCPPM.EventLength                                      = CPPM_CONNECTED_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
      EventDataCPPM.EventData.ConnectedEventData.ConnectedFlags      = Message->ConnectedFlags;
      EventDataCPPM.EventData.ConnectedEventData.NumberOfInstances   = Message->NumberOfInstances;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, DisconnectedEventCPPM is run  */
   /* when disconnected event data is received from the PM server. A    */
   /* remote sensor is considered disconnected when the LE connection   */
   /* is lost or when the services are considered unknown.              */
static void DisconnectedEventCPPM(CPPM_Disconnected_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                        = cetDisconnectedCPP;
      EventDataCPPM.EventLength                                      = CPPM_DISCONNECTED_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, WriteResponseEventCPPM        */
   /* GATM write response status information. Write responses will be   */
   /* received after enabling notifications or indications and after    */
   /* writing an opcode to a control point to initiate a procedure.     */
static void WriteResponseEventCPPM(CPPM_Write_Response_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t  EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                             = Message->EventType;
      EventDataCPPM.EventLength                                           = CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE;
      EventDataCPPM.EventCallbackID                                       = CallbackEntry->CallbackID;

      EventDataCPPM.EventData.WriteResponseEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.WriteResponseEventData.InstanceID          = Message->InstanceID;
      EventDataCPPM.EventData.WriteResponseEventData.TransactionID       = Message->TransactionID;
      EventDataCPPM.EventData.WriteResponseEventData.Status              = Message->Status;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, MeasurementEventCPPM is run   */
   /* when measurement notification data is received. The client will   */
   /* only receive measurement notifications if it is registered for    */
   /* them and they have been enabled on the remote device. See         */
   /* CPPMType.h for the definition of CPPM_Measurement_Data_t.         */
 void MeasurementEventCPPM(CPPM_Measurement_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t  EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                          = cetMeasurementCPP;
      EventDataCPPM.EventLength                                        = CPPM_MEASUREMENT_EVENT_DATA_SIZE;
      EventDataCPPM.EventCallbackID                                    = CallbackEntry->CallbackID;

      EventDataCPPM.EventData.MeasurementEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.MeasurementEventData.InstanceID          = Message->InstanceID;
      EventDataCPPM.EventData.MeasurementEventData.Measurement         = Message->Measurement;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, VectorEventCPPM is run when   */
   /* vector notification data is received. The client will only        */
   /* receive vector notifications if it is registered for them and     */
   /* they have been enabled on the remote device. See CPPMType.h for   */
   /* the defintion of CPPM_Vector_Data_t.                              */
static void VectorEventCPPM(CPPM_Vector_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t  EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                                   = cetVectorCPP;
      EventDataCPPM.EventLength                                                 = CPPM_VECTOR_EVENT_DATA_SIZE;
      EventDataCPPM.EventCallbackID                                             = CallbackEntry->CallbackID;

      EventDataCPPM.EventData.VectorEventData.RemoteDeviceAddress               = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.VectorEventData.InstanceID                        = Message->InstanceID;
      EventDataCPPM.EventData.VectorEventData.Vector.Flags                      = Message->VectorFlags;
      EventDataCPPM.EventData.VectorEventData.Vector.CrankRevolutionData        = Message->CrankRevolutionData;
      EventDataCPPM.EventData.VectorEventData.Vector.FirstCrankMeasurementAngle = Message->FirstCrankMeasurementAngle;
      EventDataCPPM.EventData.VectorEventData.Vector.MagnitudeDataLength        = Message->MagnitudeDataLength;
      EventDataCPPM.EventData.VectorEventData.Vector.InstantaneousMagnitude     = Message->InstantaneousMagnitude;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, ControlPointEventCPPM is run  */
   /* when control point indication data is received. The client will   */
   /* only receive control point indications if it is registered for    */
   /* them and they have been enabled on the remote device. A control   */
   /* point indication is received when a procedure is complete or has  */
   /* timed out. See CPPMType.h for the defintion of                    */
   /* CPPM_Control_Point_Data_t.                                        */
static void ControlPointEventCPPM(CPPM_Control_Point_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t  EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                            = cetControlPointCPP;
      EventDataCPPM.EventLength                                          = CPPM_CONTROL_POINT_EVENT_DATA_SIZE;
      EventDataCPPM.EventCallbackID                                      = CallbackEntry->CallbackID;

      EventDataCPPM.EventData.ControlPointEventData.RemoteDeviceAddress  = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.ControlPointEventData.InstanceID           = Message->InstanceID;
      EventDataCPPM.EventData.ControlPointEventData.Timeout              = Message->Timeout;

      if(!(Message->Timeout))
      {
         EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Opcode       = Message->Opcode;
         EventDataCPPM.EventData.ControlPointEventData.ControlPoint.ResponseCode = Message->ResponseCode;

         if(Message->ResponseCode == prcSuccessCPP)
         {
            if(Message->Opcode == pocRequestSupportedSensorLocations)
            {
               EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.NumberOfSensorLocations = Message->Parameter.SupportedSensorLocations.NumberOfSensorLocations;

               if((EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.SensorLocations = (CPPM_Sensor_Location_t *)BTPS_AllocateMemory(Message->Parameter.SupportedSensorLocations.NumberOfSensorLocations * sizeof(CPPM_Sensor_Location_t))) != NULL)
               {
                  BTPS_MemCopy(EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.SensorLocations, Message->Parameter.SupportedSensorLocations.SensorLocations, Message->Parameter.SupportedSensorLocations.NumberOfSensorLocations * sizeof(CPPM_Sensor_Location_t));
               }
            }
            else
               BTPS_MemCopy(&(EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Parameter), &(Message->Parameter), Message->ParameterLength);
         }
      }

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, SensorFeaturesEventCPPM is    */
   /* run when supported feature read response data is received. The    */
   /* read response event can be the result of a call to                */
   /* CPPM_Read_Sensor_Features or when a saved sensor's attributes are */
   /* read automatically after a connection is re-established.          */
   /* See CPPMType.h for #define values to be used when decoding the    */
   /* feature field.                                                    */
static void SensorFeaturesEventCPPM(CPPM_Sensor_Features_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t  EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                             = cetSensorFeaturesCPP;
      EventDataCPPM.EventLength                                           = CPPM_SENSOR_FEATURES_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.SensorFeaturesEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.SensorFeaturesEventData.InstanceID          = Message->InstanceID;
      EventDataCPPM.EventData.SensorFeaturesEventData.Features            = Message->Features;
      EventDataCPPM.EventData.SensorFeaturesEventData.Status              = Message->Status;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessServerMessageCPPM, SensorLocationEventCPPM is    */
   /* run when mount location read response data is received. The read  */
   /* response event can be the result of a call to                     */
   /* CPPM_Read_Sensor_Location or when a saved sensor's attributes are */
   /* read automatically after a connection is re-established.          */
   /* See CPPMType.h for the CPPM_Sensor_Location_t type definition.    */
static void SensorLocationEventCPPM(CPPM_Sensor_Location_Message_t *Message)
{
   Callback_Entry_t  *CallbackEntry;
   CPPM_Event_Data_t EventDataCPPM;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check for a valid callback.                                       */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
   {
      EventDataCPPM.EventType                                             = cetSensorLocationCPP;
      EventDataCPPM.EventLength                                           = CPPM_SENSOR_LOCATION_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.SensorLocationEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
      EventDataCPPM.EventData.SensorLocationEventData.InstanceID          = Message->InstanceID;
      EventDataCPPM.EventData.SensorLocationEventData.Location            = Message->Location;
      EventDataCPPM.EventData.SensorLocationEventData.Status              = Message->Status;

      /* Call the registered function.                                  */
      EventCPPM(CallbackEntry, &EventDataCPPM);
   }
   else
   {
      /* Release the Mutex.                                             */
      BTPS_ReleaseMutex(CPPManagerMutex);

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Callback Entry %u not found\n", Message->CallbackID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* EventCPPM makes the client callback.                              */
static void EventCPPM(Callback_Entry_t *CallbackEntry, CPPM_Event_Data_t *EventDataCPPM)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Release the Mutex because.                                        */
   BTPS_ReleaseMutex(CPPManagerMutex);

   __BTPSTRY
   {
      if(CallbackEntry->EventCallback)
      {
         (*CallbackEntry->EventCallback)(EventDataCPPM, CallbackEntry->CallbackParameter);
      }
   }
   __BTPSEXCEPT(1)
   {
      /* Do Nothing.                                                    */
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* CPPM_Register_Collector_Event_Callback registers a callback       */
   /* function that will be run when a cycling power event occurs. If   */
   /* If successful, the callback ID to be used in other API functions  */
   /* is returned.                                                      */
int BTPSAPI CPPM_Register_Collector_Event_Callback(CPPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              Result;
   Callback_Entry_t CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      if(CallbackFunction)
      {
         /* Wait for the mutex.                                         */
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Register for collector events.                           */
            if((Result = _CPPM_Register_Collector_Event_Callback()) > 0)
            {
               /* Attempt to add an entry into the CPP Entry list.      */
               CallbackEntry.EventCallback     = CallbackFunction;
               CallbackEntry.CallbackParameter = CallbackParameter;
               CallbackEntry.CallbackID        = (unsigned int)Result;

               /* Attempt to add the entry to the local callback list.  */
               if(!AddCallbackEntry(&CallbackEntryList, &CallbackEntry))
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }

            /* Release the mutex.                                       */
            BTPS_ReleaseMutex(CPPManagerMutex);
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* After a successful call to                                        */
   /* CPPM_Unregister_Collector_Event_Callback the previously           */
   /* registered callback function will no longer be run when a cycling */
   /* power event occurs.                                               */
void BTPSAPI CPPM_Unregister_Collector_Event_Callback(unsigned int CallbackID)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Delete the specified callback.                              */
         if((CallbackEntry = DeleteCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            /* Unregister the event callback                            */
            _CPPM_Unregister_Collector_Event_Callback(CallbackEntry->CallbackID);

            /* Free the memory.                                         */
            FreeCallbackEntryMemory(CallbackEntry);
         }

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

  /* CPPM_Register_Measurements registers for measurement notifications */
  /* from the specified sensor instance. If EnableUpdates is TRUE then  */
  /* the client configuration descriptor will be written to enable the  */
  /* notifications and the transaction ID from a successful GATM write  */
  /* will be returned. If EnableUpdates is false then zero indicates    */
  /* success. If the descriptor is written then the response comes in a */
  /* CPPM_Write_Response_Event_Data_t type struct, delivered in a       */
  /* cetMeasurementsSetCPP type event.                                  */
int BTPSAPI CPPM_Register_Measurements(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Register_Updates(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, EnableUpdates, CPPM_MESSAGE_FUNCTION_REGISTER_MEASUREMENTS);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* After a successful call to CPPM_Unregister_Measurements, the      */
   /* caller will no longer receive measurement notifications from the  */
   /* specified sensor. If there are no other registered applications,  */
   /* then the notifications are disabled by writing the configuration  */
   /* descriptor. If the descriptor is written then the response data   */
   /* comes in a CPPM_Write_Response_Event_Data_t type struct delivered */
   /* in a cetMeasurementsSetCPP type event.                            */
int BTPSAPI CPPM_Unregister_Measurements(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_UNREGISTER_MEASUREMENTS);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

  /* CPPM_Register_Vectors registers for vector notifications from the  */
  /* specified sensor instance. If EnableUpdates is TRUE then the       */
  /* client configuration descriptor will be written to enable the      */
  /* notifications and the transaction ID from a successful GATM write  */
  /* will be returned. If there is such a write then the response is    */
  /* delivered via a cetVectorsSetCPP type event. It delivers the data  */
  /* in a CPPM_Write_Response_Event_Data_t type struct. If              */
  /* EnableUpdates is false then zero indicates success.                */
int BTPSAPI CPPM_Register_Vectors(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Register_Updates(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, EnableUpdates, CPPM_MESSAGE_FUNCTION_REGISTER_VECTORS);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* After a successful call to CPPM_Unregister_Vectors, the caller    */
   /* will no longer receive vector notifications from the specified    */
   /* sensor. If no other  applications have registered to receive      */
   /* them, then the notifications are disabled by writing the          */
   /* configuration descriptor. If the descriptor is written then the   */
   /* response is delivered in a CPPM_Write_Response_Event_Data_t       */
   /* struct by a cetVectorsSetCPP type event.                          */
int BTPSAPI CPPM_Unregister_Vectors(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_UNREGISTER_VECTORS);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

  /* CPPM_Register_Procedures registers for control point indications   */
  /* from the specified sensor instance. If EnableUpdates is TRUE then  */
  /* the client configuration descriptor will be written to enable the  */
  /* indications and the transaction ID from a successful GATM write    */
  /* will be returned. If the descriptor is written the write response  */
  /* comes in a cetProceduresSetCPP type event. It delivers the         */
  /* response data in a CPPM_Write_Response_Event_Data_t type struct.   */
  /* If EnableUpdates is false then zero indicates success. Control     */
  /* point indications complete procedures. Procedures are initiated by */
  /* writing an opcode to the control point characteristic.             */
  /* See CPPM_Write_Sensor_Control_Point.                               */
int BTPSAPI CPPM_Register_Procedures(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Register_Updates(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, EnableUpdates, CPPM_MESSAGE_FUNCTION_REGISTER_PROCEDURES);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* After a successful call to CPPM_Unregister_Procedures, the caller */
   /* will no longer receive procedure results via control point        */
   /* indications from the specified sensor. If there are no other      */
   /* registered applications, then the indications are disabled by     */
   /* writing the configuration descriptor. If such a write is          */
   /* triggered then the response is delivered via a                    */
   /* cetProceduresSetCPP type event. The write response data is in a   */
   /* CPPM_Write_Response_Event_Data_t type struct.                     */
int BTPSAPI CPPM_Unregister_Procedures(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_UNREGISTER_PROCEDURES);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Enable_Broadcasts enables measurement broadcasts by writing  */
   /* the server configuration descriptor of the measurement            */
   /* characteristic of the specified sensor. If successful, the        */
   /* transaction ID of the GATM write is returned. The write response  */
   /* triggers a cetBroadcastsSetCPP type event which delivers the      */
   /* response in a CPPM_Write_Response_Event_Data_t type struct.       */
int BTPSAPI CPPM_Enable_Broadcasts(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_ENABLE_BROADCASTS);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Enable_Broadcasts enables measurement broadcasts by writing  */
   /* the server configuration descriptor of the measurement            */
   /* characteristic of the specified sensor. If successful, the        */
   /* transaction ID of the GATM write is returned. The write response  */
   /* triggers a cetBroadcastsSetCPP type event which delivers the      */
   /* response in a CPPM_Write_Response_Event_Data_t type struct.       */
int BTPSAPI CPPM_Disable_Broadcasts(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_DISABLE_BROADCASTS);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Read_Sensor_Features reads the supported features of a       */
   /* sensor. If successful it returns the GATM transaction ID from the */
   /* read. The response is delivered in a cetSensorFeaturesCPP type    */
   /* event with a CPPM_Sensor_Features_Event_Data_t type struct.       */
int BTPSAPI CPPM_Read_Sensor_Features(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_READ_SENSOR_FEATURES);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Read_Sensor_Location reads the mount location of the sensor. */
   /* If successful, it returns the GATM transaction ID from the read.  */
   /* The read response is delivered in a cetSensorLocationCPP type     */
   /* event with a CPPM_Sensor_Location_Event_Data_t type struct.       */
int BTPSAPI CPPM_Read_Sensor_Location(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the mutex.                                            */
      if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
      {
         /* Check for a registered callback.                            */
         if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
         {
            Result = _CPPM_Request(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, CPPM_MESSAGE_FUNCTION_READ_SENSOR_LOCATION);
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

         /* Release the Mutex.                                          */
         BTPS_ReleaseMutex(CPPManagerMutex);
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Write_Sensor_Control_Point writes an opcode and, depending   */
   /* on the procedure type, may also write additional procedure data   */
   /* to the specified sensor instance's control point characteristic   */
   /* thereby initiating a procedure. The write response triggers a     */
   /* cetProcedureBegunCPP type event. The response data is packaged in */
   /* a CPPM_Write_Response_Event_Data_t type struct. A                 */
   /* cetControlPointCPP type event indicates the procedure has         */
   /* completed. The control point indication data is packaged in a     */
   /* CPPM_Control_Point_Event_Data_t type struct.                      */
int BTPSAPI CPPM_Write_Sensor_Control_Point(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, CPPM_Procedure_Data_t ProcedureData)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteSensor)) && (InstanceID))
   {
      if(Initialized)
      {
         /* Wait for the mutex.                                         */
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check for a registered callback.                         */
            if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
            {
               Result = _CPPM_Write_Sensor_Control_Point(CallbackEntry->CallbackID, &RemoteSensor, InstanceID, ProcedureData);
            }
            else
               Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

            /* Release the Mutex.                                       */
            BTPS_ReleaseMutex(CPPManagerMutex);
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Query_Sensors lists the bluetooth address of sensors. The    */
   /* function returns the total number of sensors in the device list.  */
   /* If the caller allocates the RemoteSensors buffer, then the        */
   /* NumberOfSensors parameter should point to a value equaling the    */
   /* number of BD_ADDR_t type addresses that the buffer can hold.      */
   /* The value pointed to by NumberOfSensors will be changed to the    */
   /* number of sensor addresses copied to the buffer.                  */
int BTPSAPI CPPM_Query_Sensors(unsigned int CallbackID, unsigned int *NumberOfSensors, BD_ADDR_t *RemoteSensors)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackID)
   {
      if(Initialized)
      {
         /* Wait for the mutex.                                         */
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check for a registered callback.                         */
            if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
            {
               Result = _CPPM_Query_Sensors(CallbackEntry->CallbackID, NumberOfSensors, RemoteSensors);
            }
            else
               Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

            /* Release the Mutex.                                       */
            BTPS_ReleaseMutex(CPPManagerMutex);
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* CPPM_Query_Sensor_Instances lists the sensor service instance     */
   /* records of the specified sensor. The function returns the total   */
   /* number of service instances. If the caller allocates the          */
   /* Instances buffer, then the NumberOfInstances parameter should     */
   /* point to a value equaling the number of instance records that the */
   /* buffer can hold. The value pointed to by NumberOfInstances will   */
   /* be changed to the number of records copied to the buffer.         */
int BTPSAPI CPPM_Query_Sensor_Instances(unsigned int CallbackID, BD_ADDR_t Sensor, unsigned int *NumberOfInstances, Instance_Record_t *Instances)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackID)
   {
      if(Initialized)
      {
         /* Wait for the mutex.                                         */
         if(BTPS_WaitMutex(CPPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Check for a registered callback.                         */
            if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
            {
               Result = _CPPM_Query_Sensor_Instances(CallbackEntry->CallbackID, &Sensor, NumberOfInstances, Instances);
            }
            else
               Result = BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED;

            /* Release the Mutex.                                       */
            BTPS_ReleaseMutex(CPPManagerMutex);
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         Result = BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

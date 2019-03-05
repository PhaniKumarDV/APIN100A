/*****< BTPMCPPM.c >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  BTPMCPPM - Cycling Power Platform Manager Server                          */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/

#include <limits.h>
#include "SS1BTCPS.h"            /* Bluetopia CP header                       */
#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "CPPMType.h"            /* CPP Manager type defs. and constants      */
#include "CPPMAPI.h"             /* CPP Manager functions and events          */
#include "CPPMUTIL.h"            /* CPP Manager utilities header              */
#include "BTPMCPPM.h"            /* CPP Manager header                        */
#include "CPPMMSG.h"             /* CPP Manager IPC messages                  */
#include "CPPMDEVM.h"            /* CPP Manager DEVM handling header          */


   /*********************************************************************/
   /* Registered in the initialization handler:                         */
   /*********************************************************************/
static void BTPSAPI ProcessMessageCPPM(BTPM_Message_t *Message, void *CallbackParameter);
static void BTPSAPI ProcessGATMEventCPPM(GATM_Event_Data_t *EventData, void *CallbackParameter);

   /*********************************************************************/
   /* Queued by ProcessMessageCPPM:                                     */
   /*********************************************************************/
static void BTPSAPI ProcessClientMessageCPPM(void *CallbackParameter);
static void BTPSAPI ProcessUnregisterMessageCPPM(void *CallbackParameter);

   /*********************************************************************/
   /* Called by ProcessClientMessageCPPM:                               */
   /*********************************************************************/
static void ProcessRegisterCollectorRequestCPPM(CPPM_Register_Collector_Events_Request_t *Message);
static void ProcessUnregisterCollectorRequestCPPM(CPPM_Unregister_Collector_Events_Request_t *Message);
static void ProcessRegisterMeasurementsRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message);
static void ProcessRegisterVectorsRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message);
static void ProcessRegisterProceduresRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message);
static void ProcessEnableBroadcastsRequestCPPM(CPPM_Request_t *Message);
static void ProcessUnregisterMeasurementsRequestCPPM(CPPM_Request_t *Message);
static void ProcessUnregisterVectorsRequestCPPM(CPPM_Request_t *Message);
static void ProcessUnregisterProceduresRequestCPPM(CPPM_Request_t *Message);
static void ProcessDisableBroadcastsRequestCPPM(CPPM_Request_t *Message);
static void ProcessReadSensorFeaturesRequestCPPM(CPPM_Request_t *Message);
static void ProcessReadSensorLocationRequestCPPM(CPPM_Request_t *Message);
static void ProcessWriteSensorControlPointRequestCPPM(CPPM_Write_Sensor_Control_Point_Request_t *Message);
static void ProcessQuerySensorsRequestCPPM(CPPM_Query_Sensors_Request_t *Message);
static void ProcessQuerySensorInstancesRequestCPPM(CPPM_Query_Sensor_Instances_Request_t *Message);

   /*********************************************************************/
   /* Called by register and unregister update functions:               */
   /*********************************************************************/
static void ProcessRegisterUpdatesRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset, Boolean_t Notification);
static int RegisterUpdatesCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset, Boolean_t EnableUpdates, Boolean_t Notification);
static void ProcessUnregisterUpdatesRequestCPPM(CPPM_Request_t *Message, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset);
static int UnregisterUpdatesCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset);

   /*********************************************************************/
   /* Called by enable and disable broadcast functions:                 */
   /*********************************************************************/
static void ProcessToggleBroadcastsRequestCPPM(CPPM_Request_t *Message, Boolean_t Enable);
static int ToggleBroadcastsCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Boolean_t Enable);

   /*********************************************************************/
   /* Called by read attribute functions:                               */
   /*********************************************************************/
static void ProcessReadAttributeRequestCPPM(CPPM_Request_t *Message, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset);
static int ReadAttributeCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset);

   /*********************************************************************/
   /* Called by ProcessWriteSensorControlPointRequestCPPM:              */
   /*********************************************************************/
static int WriteSensorControlPointCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, CPPM_Procedure_Data_t *ProcedureData);
static Byte_t *ConvertProcedureDataCPPM(CPS_Control_Point_Format_Data_t *ProcedureDataCPS, CPPM_Procedure_Data_t *ProcedureDataCPPM);

   /*********************************************************************/
   /* Called by ProcessQuerySensorsRequestCPPM:                         */
   /*********************************************************************/
static int QuerySensorsCPPM(unsigned int CallbackID, unsigned int *NumberOfSensors);
static void ListSensorsCPPM(BD_ADDR_t *Sensors, unsigned int NumberOfSensors);

   /*********************************************************************/
   /* Called by ProcessQuerySensorInstancesRequestCPPM:                 */
   /*********************************************************************/
static int QuerySensorInstancesCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int *NumberOfInstances);
static void ListSensorInstancesCPPM(BD_ADDR_t *Sensors, Instance_Record_t *Instances, unsigned int NumberOfInstances);

   /*********************************************************************/
   /* Called by ProcessGATMEventCPPM:                                   */
   /*********************************************************************/
static void ProcessHandleValueCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData);
static void ProcessReadResponseCPPM(GATM_Read_Response_Event_Data_t *ReadResponseData);
static void ProcessWriteResponseCPPM(GATM_Write_Response_Event_Data_t *WriteResponseData);
static void ProcessErrorResponseCPPM(GATM_Error_Response_Event_Data_t *ErrorResponse);

   /*********************************************************************/
   /* Called by ProcessHandleValueCPPM:                                 */
   /*********************************************************************/
static void MeasurementEventCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData, BD_ADDR_t BluetoothAddress, unsigned int InstanceID);
static void VectorEventCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData, BD_ADDR_t BluetoothAddress, unsigned int InstanceID);
static void ControlPointEventCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, Boolean_t Timeout, unsigned int CallbackID, unsigned int AddressID);

   /*********************************************************************/
   /* Called by notification event routines:                            */
   /*********************************************************************/
static void ConvertMeasurementCPPM(CPS_Measurement_Data_t *MeasurementDataCPS, CPPM_Measurement_Data_t *MeasurementDataCPPM);
static void ConvertVectorCPPM(CPS_Vector_Data_t *VectorDataCPS, CPPM_Vector_Data_t *VectorDataCPPM);

   /*********************************************************************/
   /* Called by ControlPointEventCPPM:                                  */
   /*********************************************************************/
static CPPM_Control_Point_Message_t *ConvertControlPointCPPM(CPS_Control_Point_Response_Data_t *ControlPointDataCPS);

   /*********************************************************************/
   /* Callbacks to queue procedure timeout and the timeout itself:      */
   /*********************************************************************/
static Boolean_t QueueProcedureTimeoutEventCPPM(unsigned int TimerID, void *InstanceEntry);
static void ProcedureTimeoutEventCPPM(void *InstanceEntryParameter);

   /*********************************************************************/
   /* Called by ProcessWriteResponseCPPM:                               */
   /*********************************************************************/
static void WriteResponseEventCPPM(unsigned int CallbackID, unsigned int AddressID, unsigned int TransactionID, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, CPPM_Event_Type_t EventType, int Status);

   /*********************************************************************/
   /* Called by ProcessReadResponseCPPM:                                */
   /*********************************************************************/
static void SensorFeaturesEventCPPM(unsigned int CallbackID, unsigned int AddressID, unsigned int TransactionID, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, unsigned long Features, int Status);
static void SensorLocationEventCPPM(unsigned int CallbackID, unsigned int AddressID, unsigned int TransactionID, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, unsigned int Location, int Status);

static unsigned int NextCallbackID;

Callback_Entry_t *CallbackEntryList = NULL;
Device_Entry_t   *DeviceEntryList   = NULL;
Boolean_t         Initialized       = FALSE;
unsigned int      GATMCallbackID    = 0;

   /* CPPM_InitializationHandlerFunction is included in the MODC        */
   /* ModuleHandlerList array and is called to initalize the module.    */
void BTPSAPI CPPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;
   Callback_Entry_t  CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* Determine if the request is to initialize or shut down.           */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Register the Message Group Handler.                         */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER, ProcessMessageCPPM, NULL))
         {
            /* Register a GATM Event Callback.                          */
            if((Result = GATM_RegisterEventCallback(ProcessGATMEventCPPM, NULL)) > 0)
            {
               /* Save the GATM Event Callback ID.                      */
               GATMCallbackID = (unsigned int)Result;

               /* Initialize a starting CPP Callback ID.                */
               NextCallbackID = 0x000000000;

               /* Initialize a dummy Callback Entry.                    */
               BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

               AddCallbackEntry(&CallbackEntryList, &CallbackEntry);

               Initialized    = TRUE;

               /* Flag success.                                         */
               Result         = 0;
            }
            else
            {
               Initialized = FALSE;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* In case of error, free the resources.                       */
         if(Result)
         {
            Initialized = FALSE;

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("Cycling Power manager already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* Unregister the message group handler.                       */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER);

         if(DEVM_AcquireLock())
         {
            /* Free the Device Entries.                                 */
            FreeDeviceEntryList(&DeviceEntryList);

            /* Free the Callback Entries.                               */
            FreeCallbackEntryList(&CallbackEntryList);

            /* Flag that this module is no longer initialized.          */
            Initialized = FALSE;

            /* Free the lock.                                           */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Registered in the initialization handler, ProcessMessageCPPM      */
   /* queues both the callback for messages from PM clients and the     */
   /* callback that handles clients unregistering.                      */
static void BTPSAPI ProcessMessageCPPM(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Check the group.                                               */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER)
      {
         if(Initialized)
         {
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Manager Client Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

               /* Post the message to the CPP Manager Thread.           */
               if(!BTPM_QueueMailboxCallback(ProcessClientMessageCPPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FAILURE), ("Unable to Queue Cycling Power Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

                  MSG_FreeReceivedMessageGroupHandlerMessage(Message);
               }
            }
            else
            {
               /* A client is unregistering.                            */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Cycling Power Manager Message: client unregistered\n"));

                     if(!BTPM_QueueMailboxCallback(ProcessUnregisterMessageCPPM, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FAILURE), ("Unable to Queue Cycling Power Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
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

   /* Registered in the initialization handler, ProcessGATMEventCPPM    */
   /* handles GATM events.                                              */
static void BTPSAPI ProcessGATMEventCPPM(GATM_Event_Data_t *EventData, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EventData)
   {
      if(Initialized)
      {
         /* Wait for access to the lock.                                */
         if(DEVM_AcquireLock())
         {
            switch(EventData->EventType)
            {
               case getGATTHandleValueData:
                  /* Process the GATT Handle Value Data event.          */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Handle Value Event\n"));
                  ProcessHandleValueCPPM(&(EventData->EventData.HandleValueDataEventData));
                  break;
               case getGATTReadResponse:
                  /* Process the GATT Read Response.                    */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Read Response Event\n"));
                  ProcessReadResponseCPPM(&(EventData->EventData.ReadResponseEventData));
                  break;
               case getGATTWriteResponse:
                  /* Process the GATT Write Response.                   */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Write Response Event\n"));
                  ProcessWriteResponseCPPM(&(EventData->EventData.WriteResponseEventData));
                  break;
               case getGATTErrorResponse:
                  /* Process the GATT Error Response.                   */
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Error Response Event\n"));
                  ProcessErrorResponseCPPM(&(EventData->EventData.ErrorResponseEventData));
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event\n"));
                  break;
            }

            /* Release the lock.                                        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Queued by the message group handler, ProcessClientMessageCPPM     */
   /* processes request messages from platform manager clients.         */
static void BTPSAPI ProcessClientMessageCPPM(void *CallbackParameter)
{
   BTPM_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackParameter)
   {
      if(Initialized)
      {
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            Message = (BTPM_Message_t *)CallbackParameter;

            if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Message Function: 0x%08X\n", Message->MessageHeader.MessageFunction));

               switch(Message->MessageHeader.MessageFunction)
               {
                  case CPPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Cycling Power Events Message\n"));

                     /* A PM client is requesting cycling power events. */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
                     {
                        ProcessRegisterCollectorRequestCPPM((CPPM_Register_Collector_Events_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_UNREGISTER_COLLECTOR_EVENTS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unregister Cycling Power Events Message\n"));

                     /* A PM client no longer needs CPP events.         */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_UNREGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
                     {
                        ProcessUnregisterCollectorRequestCPPM((CPPM_Unregister_Collector_Events_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_REGISTER_MEASUREMENTS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Cycling Power Measurements Message\n"));

                     /* A PM client wants to receive Measurement Notif. */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REGISTER_UNSOLICITED_UPDATES_REQUEST_SIZE)
                     {
                        ProcessRegisterMeasurementsRequestCPPM((CPPM_Register_Unsolicited_Updates_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_UNREGISTER_MEASUREMENTS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unregister Cycling Power Measurements Message\n"));

                     /* A PM client no longer needs Measurement Notif.  */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessUnregisterMeasurementsRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_REGISTER_VECTORS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Cycling Power Vectors Message\n"));

                     /* A PM client wants to receive Vector Notif.      */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REGISTER_UNSOLICITED_UPDATES_REQUEST_SIZE)
                     {
                        ProcessRegisterVectorsRequestCPPM((CPPM_Register_Unsolicited_Updates_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_UNREGISTER_VECTORS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unregister Cycling Power Vectors Message\n"));

                     /* A PM client no longer needs Vector Notif.       */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessUnregisterVectorsRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_REGISTER_PROCEDURES:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Cycling Power Procedures Message\n"));

                     /* A PM client wants to receive Control Point Ind. */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REGISTER_UNSOLICITED_UPDATES_REQUEST_SIZE)
                     {
                        ProcessRegisterProceduresRequestCPPM((CPPM_Register_Unsolicited_Updates_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_UNREGISTER_PROCEDURES:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unregister Cycling Power Procedures Message\n"));

                     /* A PM client no longer needs Control Point Ind.  */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessUnregisterProceduresRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_ENABLE_BROADCASTS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Cycling Power Broadcasts Message\n"));

                     /* A PM client is requesting that broadcasts be    */
                     /* enabled.                                        */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessEnableBroadcastsRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_DISABLE_BROADCASTS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Cycling Power Broadcasts Message\n"));

                     /* A PM client is requesting that broadcasts be    */
                     /* disabled.                                       */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessDisableBroadcastsRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_READ_SENSOR_FEATURES:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Read Cycling Power Sensor Features Message\n"));

                     /* A PM client requests the supported features be  */
                     /* read.                                           */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessReadSensorFeaturesRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_READ_SENSOR_LOCATION:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Read Cycling Power Sensor Location Message\n"));

                     /* A PM client requests the sensor location be     */
                     /* read.                                           */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_REQUEST_SIZE)
                     {
                        ProcessReadSensorLocationRequestCPPM((CPPM_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_WRITE_SENSOR_CONTROL_POINT:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Write Cycling Power Sensor Control Point Message\n"));

                     /* A PM client is attempting to initiate a control */
                     /* point procedure.                                */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_WRITE_SENSOR_CONTROL_POINT_REQUEST_SIZE)
                     {
                        ProcessWriteSensorControlPointRequestCPPM((CPPM_Write_Sensor_Control_Point_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_QUERY_SENSORS:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Cycling Power Sensors Message\n"));

                     /* A PM client is requesting a sensor list.        */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_QUERY_SENSORS_REQUEST_SIZE)
                     {
                        ProcessQuerySensorsRequestCPPM((CPPM_Query_Sensors_Request_t *)Message);
                     }
                     break;

                  case CPPM_MESSAGE_FUNCTION_QUERY_SENSOR_INSTANCES:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Cycling Power Sensor Instances Message\n"));

                     /* A PM client is requesting the instance info     */
                     /* from a sensor.                                  */
                     if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= CPPM_QUERY_SENSOR_INSTANCES_REQUEST_SIZE)
                     {
                        ProcessQuerySensorInstancesRequestCPPM((CPPM_Query_Sensor_Instances_Request_t *)Message);
                     }
                     break;

                  default:
                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Cycling Power Message\n"));
                     break;
               }
            }

            DEVM_ReleaseLock();
         }
      }

      /* Free the message.                                              */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Queued by the message group handler, ProcessUnregisterMessageCPPM */
   /* processes a PM client unregistration.                             */
static void BTPSAPI ProcessUnregisterMessageCPPM(void *CallbackParameter)
{
   unsigned long     AddressID;
   Callback_Entry_t *CallbackEntry;
   Callback_Entry_t *tmpCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackParameter)
   {
      if(Initialized)
      {
         if(DEVM_AcquireLock())
         {
            AddressID = (unsigned long)CallbackParameter;

            /* Find and delete the callback entry.                      */
            for(CallbackEntry = CallbackEntryList; CallbackEntry;)
            {
               if(CallbackEntry->AddressID == AddressID)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Callback Entry with Address ID %lu found\n", AddressID));

                  tmpCallbackEntry = CallbackEntry;
                  CallbackEntry    = CallbackEntry->NextCallbackEntry;

                  /* Delete the entry from the list.                    */
                  if((tmpCallbackEntry = DeleteCallbackEntry(&CallbackEntryList, tmpCallbackEntry->CallbackID)) != NULL)
                     FreeCallbackEntryMemory(tmpCallbackEntry);
               }
               else
               {
                  /* Advance the pointer.                               */
                  CallbackEntry = CallbackEntry->NextCallbackEntry;
               }
            }

            /* Release the lock.                                        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessRegisterCollectorRequestCPPM adds a callback entry to the  */
   /* callback list. The request is initiated by PM clients using       */
   /* using CPPM_Register_Collector_Event_Callback.                     */
static void ProcessRegisterCollectorRequestCPPM(CPPM_Register_Collector_Events_Request_t *Message)
{
   Callback_Entry_t        *CallbackEntryPtr;
   Callback_Entry_t         CallbackEntry;
   CPPM_Response_t          Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Initialize the Callback Entry.                                 */
      BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

      CallbackEntry.AddressID     = Message->MessageHeader.AddressID;
      CallbackEntry.EventCallback = NULL;

      GetNextID(&NextCallbackID);
      CallbackEntry.CallbackID    = NextCallbackID;

      if((CallbackEntryPtr = AddCallbackEntry(&CallbackEntryList, &CallbackEntry)) != NULL)
      {
         Response.Status = CallbackEntryPtr->CallbackID;
      }
      else
         Response.Status = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessUnregisterCollectorRequestCPPM removes a callback entry    */
   /* from the callback list. A PM client initiates the request by      */
   /* using CPPM_Unregister_Collector_Event_Callback.                   */
static void ProcessUnregisterCollectorRequestCPPM(CPPM_Unregister_Collector_Events_Request_t *Message)
{
   Callback_Entry_t *CallbackEntryPtr;
   CPPM_Response_t   Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if((CallbackEntryPtr = DeleteCallbackEntry(&(CallbackEntryList), Message->CallbackID)) != NULL)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Callback Entry with CallbackID ID %u found\n", Message->CallbackID));

         /* Indicate success.                                           */
         Response.Status = 0;

         /* Free the callback entry.                                    */
         FreeCallbackEntryMemory(CallbackEntryPtr);
      }
      else
         Response.Status = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessRegisterMeasurementsRequestCPPM handles a request to       */
   /* receive measurement notifications. The PM client can just         */
   /* register or register and initiate a write to the measurement      */
   /* client configuration descriptor to enable the measurements.       */
   /* The request is made by a PM client using                          */
   /* CPPM_Register_Measurements.                                       */
static void ProcessRegisterMeasurementsRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessRegisterUpdatesRequestCPPM(Message, ttEnableCyclingPowerMeasurement, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration), TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessRegisterVectorsRequestCPPM handles a request to receive    */
   /* vector notifications. The PM client can just register or register */
   /* and initiate a write to the vector client configuration           */
   /* descriptor to enable the vectors.                                 */
   /* The request is made by a PM client using CPPM_Register_Vectors.   */
static void ProcessRegisterVectorsRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessRegisterUpdatesRequestCPPM(Message, ttEnableCyclingPowerVector, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration), TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessRegisterProceduresRequestCPPM handles a request to receive */
   /* control point indications. Control point indications contain the  */
   /* results of control point procedures. The PM client can just       */
   /* register or register and initiate a write to the control point    */
   /* client configuration descriptor to enable the procedures.         */
   /* The request is made by a PM client using CPPM_Register_Procedures.*/
static void ProcessRegisterProceduresRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessRegisterUpdatesRequestCPPM(Message, ttEnableCyclingPowerControlPoint, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration), FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by functions that process requests to register for         */
   /* notifications or indications, ProcessRegisterUpdatesRequestCPPM   */
   /* adds entries to a callback entry's unsolicited update list and,   */
   /* optionally writes the appropiate value to the remote client       */
   /* configuration descriptor.                                         */
static void ProcessRegisterUpdatesRequestCPPM(CPPM_Register_Unsolicited_Updates_Request_t *Message, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset, Boolean_t Notification)
{
   CPPM_Response_t             Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      Response.Status                       = RegisterUpdatesCPPM(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID, TransactionType, CharacteristicOffset, DescriptorOffset, Message->EnableUnsolicitedUpdates, Notification);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessRegisterUpdatesRequestCPPM and server side API   */
   /* functions which register for unsolicited updates,                 */
   /* RegisterUpdatesCPPM adds entries to a callback entry's            */
   /* unsolicited update list and, optionally writes the appropiate     */
   /* value to the remote client configuration descriptor. If           */
   /* successful, it either returns a transaction ID (if a GATM write   */
   /* is attempted) or zero.                                            */
static int RegisterUpdatesCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset, Boolean_t EnableUpdates, Boolean_t Notification)
{
   int                         Result;
   Callback_Entry_t           *CallbackEntry;
   Instance_Entry_t           *InstanceEntry;
   Unsolicited_Update_Entry_t  UnsolicitedUpdateEntry;
   Transaction_Entry_t         TransactionEntry;
   Word_t                      DescriptorHandle;
   NonAlignedWord_t            Value;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackID) && (Sensor) && (!COMPARE_NULL_BD_ADDR(*Sensor)) && (InstanceID))
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Search for the instance.                                    */
         if((InstanceEntry = SearchDeviceInstanceEntry(&(DeviceEntryList), Sensor, InstanceID)) != NULL)
         {
            BTPS_MemInitialize(&(UnsolicitedUpdateEntry), 0, sizeof(Unsolicited_Update_Entry_t));

            /* Set the update entry values.                             */
            UnsolicitedUpdateEntry.BluetoothAddress = *Sensor;
            UnsolicitedUpdateEntry.InstanceID       = InstanceID;
            UnsolicitedUpdateEntry.Handle           = *((Word_t *)(((Byte_t *)(&(InstanceEntry->ServiceHandles))) + CharacteristicOffset));

            /* Add the entry to the list.                               */
            if(AddUnsolicitedUpdateEntry(&(CallbackEntry->UnsolicitedUpdateEntryList), &UnsolicitedUpdateEntry))
            {
               /* If requested to do so, write the remote               */
               /* characteristic descriptor.                            */
               if(EnableUpdates)
               {
                  /* Set the Transaction Entry values.                  */
                  BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

                  TransactionEntry.TransactionType  = TransactionType;

                  DescriptorHandle                  = *((Word_t *)(((Byte_t *)(&(InstanceEntry->ServiceHandles))) + DescriptorOffset));

                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Value, Notification ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE : GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE);

                  /* Perform the write.                                 */
                  if((Result = GATM_WriteValue(GATMCallbackID, *Sensor, DescriptorHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Value)) > 0)
                  {
                     /* Save the GATM Transaction ID.                   */
                     TransactionEntry.TransactionID = (unsigned int)Result;

                     /* Add the Transaction Entry.                      */
                     if(!(AddTransactionEntry(&(CallbackEntry->TransactionEntryList), &TransactionEntry)))
                        Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }

                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Enable Unsolicited Updates - Result: %d\n", Result));
               }
               else
                  Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* ProcessUnregisterMeasurementsRequestCPPM removes an entry from    */
   /* a callback entry's update list so that the PM client making the   */
   /* request will no longer receive measurement notifications.         */
static void ProcessUnregisterMeasurementsRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessUnregisterUpdatesRequestCPPM(Message, ttDisableCyclingPowerMeasurement, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessUnregisterVectorsRequestCPPM removes an entry from a       */
   /* callback entry's update list so that the PM client making the     */
   /* request will no longer receive vector notifications.              */
static void ProcessUnregisterVectorsRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessUnregisterUpdatesRequestCPPM(Message, ttDisableCyclingPowerVector, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessUnregisterProceduresRequestCPPM removes an entry from a    */
   /* callback entry's update list so that the PM client making the     */
   /* request will no longer receive control point indications.         */
static void ProcessUnregisterProceduresRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessUnregisterUpdatesRequestCPPM(Message, ttDisableCyclingPowerControlPoint, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by functions that unregister for unsolicited updates,      */
   /* ProcessUnregisterUpdatesRequestCPPM removes the list entry. It    */
   /* also searches the lists of all the callback entries and if no     */
   /* list contains the the same type of update entry then the remote   */
   /* descriptor is written to disable the updates.                     */
static void ProcessUnregisterUpdatesRequestCPPM(CPPM_Request_t *Message, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset)
{
   CPPM_Response_t             Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      Response.Status = UnregisterUpdatesCPPM(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID, TransactionType, CharacteristicOffset, DescriptorOffset);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessUnregisterUpdatesRequestCPPM and server side API */
   /* functions which unregister for unsolicited updates,               */
   /* UnregisterUpdatesCPPM removes the entry from the update list so   */
   /* the caller will no longer receive those update events. Also, it   */
   /* searches for other registrations of the same type. If none are    */
   /* found then the configuration descriptor is written to disable the */
   /* updates. If no GATM write is attempted then zero indicates        */
   /* success. If a write is attempted successfully then the            */
   /* transaction ID is returned.                                       */
static int UnregisterUpdatesCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset, intptr_t DescriptorOffset)
{
   int                         Result;
   Callback_Entry_t           *CallbackEntry;
   Callback_Entry_t           *CallbackEntrySearch;
   Instance_Entry_t           *InstanceEntry;
   Word_t                      CharacteristicHandle;
   Word_t                      DescriptorHandle;
   Unsolicited_Update_Entry_t *UnsolicitedUpdateEntry;
   Transaction_Entry_t         TransactionEntry;
   NonAlignedWord_t            Value;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackID) && (Sensor) && (!COMPARE_NULL_BD_ADDR(*Sensor)) && (InstanceID))
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Search for the instance.                                    */
         if((InstanceEntry = SearchDeviceInstanceEntry(&(DeviceEntryList), Sensor, InstanceID)) != NULL)
         {
            CharacteristicHandle = *((Word_t *)(((Byte_t *)(&(InstanceEntry->ServiceHandles))) + CharacteristicOffset));
            DescriptorHandle     = *((Word_t *)(((Byte_t *)(&(InstanceEntry->ServiceHandles))) + DescriptorOffset));

            /* Delete the entry.                                        */
            if((UnsolicitedUpdateEntry = DeleteUnsolicitedUpdateEntry(&(CallbackEntry->UnsolicitedUpdateEntryList), Sensor, InstanceID, CharacteristicHandle)) != NULL)
            {
               FreeUnsolicitedUpdateEntryMemory(UnsolicitedUpdateEntry);
               UnsolicitedUpdateEntry = NULL;

               Result = 0;

               /* Search the callbacks.                                 */
               for(CallbackEntrySearch = CallbackEntryList; CallbackEntrySearch;)
               {
                  if((UnsolicitedUpdateEntry = SearchUnsolicitedUpdateEntry(&(CallbackEntrySearch->UnsolicitedUpdateEntryList), Sensor, InstanceID, CharacteristicHandle)) != NULL)
                  {
                     break;
                  }

                  CallbackEntrySearch = CallbackEntrySearch->NextCallbackEntry;
               }

               /* If no similiar entry was found, disable the update.   */
               if(CallbackEntrySearch == NULL)
               {
                  /* Set the Transaction Entry.                         */
                  BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

                  TransactionEntry.TransactionType = TransactionType;

                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Value, 0);

                  /* Perform the write.                                 */
                  if((Result = GATM_WriteValue(GATMCallbackID, *(Sensor), DescriptorHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Value)) > 0)
                  {
                     /* Save the GATM Transaction ID.                   */
                     TransactionEntry.TransactionID = (unsigned int)Result;

                     /* Add the Transaction Entry.                      */
                     if(!(AddTransactionEntry(&(CallbackEntry->TransactionEntryList), &TransactionEntry)))
                        Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }

                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Disable Unsolicited Update - Result %d\n", Result));
               }
            }
            else
               Result = BTPM_ERROR_CODE_CYCLING_POWER_UPDATE_NOT_REGISTERED;
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* ProcessEnableBroadcastsRequestCPPM writes a server configuration  */
   /* descriptor thereby enabling measurement broadcasts.               */
static void ProcessEnableBroadcastsRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessToggleBroadcastsRequestCPPM(Message, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessEnableBroadcastsRequestCPPM writes a server configuration  */
   /* descriptor thereby disabling measurement broadcasts.              */
static void ProcessDisableBroadcastsRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessToggleBroadcastsRequestCPPM(Message, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by the enable and disable functions,                       */
   /* ProcessToggleBroadcastsRequestCPPM writes a remote server         */
   /* configuration descriptor in order to begin or end measurement     */
   /* broadcasting.                                                     */
static void ProcessToggleBroadcastsRequestCPPM(CPPM_Request_t *Message, Boolean_t Enable)
{
   CPPM_Response_t Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      Response.Status = ToggleBroadcastsCPPM(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID, Enable);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessToggleBroadcastsRequestCPPM and the server side  */
   /* API functions that either enable or disable broadcasts,           */
   /* ToggleBroadcastsCPPM enables or disables measurement broadcasts   */
   /* via a GATM write to the server configuration descriptor. If       */
   /* successful, the transaction ID of the GATM write is returned.     */
static int ToggleBroadcastsCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Boolean_t Enable)
{
   int                  Result;
   Callback_Entry_t    *CallbackEntry;
   Instance_Entry_t    *InstanceEntry;
   Transaction_Entry_t  TransactionEntry;
   NonAlignedWord_t     Value;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackID) && (Sensor) && (!COMPARE_NULL_BD_ADDR(*Sensor)) && (InstanceID))
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Search for the instance.                                    */
         if((InstanceEntry = SearchDeviceInstanceEntry(&(DeviceEntryList), Sensor, InstanceID)) != NULL)
         {
            BTPS_MemInitialize(&TransactionEntry, 0, sizeof(Transaction_Entry_t));

            if(Enable)
            {
               TransactionEntry.TransactionType = ttEnableCyclingPowerBroadcast;

               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Value, GATT_SERVER_CONFIGURATION_CHARACTERISTIC_BROADCAST_ENABLE);
            }
            else
            {
               TransactionEntry.TransactionType = ttDisableCyclingPowerBroadcast;

               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Value, 0);
            }

            /* Perform the write.                                       */
            if((Result = GATM_WriteValue(GATMCallbackID, *Sensor, InstanceEntry->ServiceHandles.CP_Measurement_Server_Configuration, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Value)) > 0)
            {
               /* Save the GATM Transaction ID.                         */
               TransactionEntry.TransactionID = (unsigned int)Result;

               /* Add the Transaction Entry.                            */
               if(!(AddTransactionEntry(&(CallbackEntry->TransactionEntryList), &TransactionEntry)))
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* ProcessReadSensorFeaturesRequestCPPM reads the supported features */
   /* of a sensor.                                                      */
static void ProcessReadSensorFeaturesRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessReadAttributeRequestCPPM(Message, ttReadCyclingPowerSensorFeatures, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Feature));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessReadSensorLocationRequestCPPM reads the sensor location.   */
static void ProcessReadSensorLocationRequestCPPM(CPPM_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcessReadAttributeRequestCPPM(Message, ttReadCyclingPowerSensorLocation, STRUCTURE_OFFSET(CPS_Client_Information_t, Sensor_Location));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by handlers of requests to read attributes,                */
   /* ProcessReadAttributeRequestCPPM sets the attribute handle and     */
   /* calls the GATM read.                                              */
static void ProcessReadAttributeRequestCPPM(CPPM_Request_t *Message, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset)
{
   CPPM_Response_t      Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      Response.Status = ReadAttributeCPPM(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID, TransactionType, CharacteristicOffset);

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

  /* Called by ProcessReadAttributeRequestCPPM and server side API      */
  /* functions that read attribute values, ReadAttributeCPPM reads      */
  /* an attribute and returns the GATM transaction ID upon success.     */
static int ReadAttributeCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, Transaction_Type_t TransactionType, intptr_t CharacteristicOffset)
{
   int                  Result;
   Callback_Entry_t    *CallbackEntry;
   Instance_Entry_t    *InstanceEntry;
   Transaction_Entry_t  TransactionEntry;
   Word_t               CharacteristicHandle;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackID) && (Sensor) && (!COMPARE_NULL_BD_ADDR(*Sensor)) && (InstanceID))
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Search for the instance.                                    */
         if((InstanceEntry = SearchDeviceInstanceEntry(&(DeviceEntryList), Sensor, InstanceID)) != NULL)
         {
            /* Configure the Transaction Entry.                         */
            BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

            TransactionEntry.TransactionType  = TransactionType;

            CharacteristicHandle = *((Word_t *)(((Byte_t *)(&(InstanceEntry->ServiceHandles))) + CharacteristicOffset));

            /* Perform the read.                                        */
            if((Result = GATM_ReadValue(GATMCallbackID, *Sensor, CharacteristicHandle, 0, TRUE)) > 0)
            {
               /* Save the GATM Transaction ID.                         */
               TransactionEntry.TransactionID = (unsigned int)Result;

               /* Add the Transaction Entry.                            */
               if(!(AddTransactionEntry(&(CallbackEntry->TransactionEntryList), &TransactionEntry)))
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* ProcessWriteSensorControlPointRequestCPPM writes an opcode to the */
   /* control point thereby initiating a procedure.                     */
static void ProcessWriteSensorControlPointRequestCPPM(CPPM_Write_Sensor_Control_Point_Request_t *Message)
{
   CPPM_Response_t                  Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      /* Format the response message.                                   */
      Response.MessageHeader                = Message->MessageHeader;
      Response.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      Response.MessageHeader.MessageLength  = CPPM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      Response.Status = WriteSensorControlPointCPPM(Message->CallbackID, &(Message->RemoteDeviceAddress), Message->InstanceID, &(Message->ProcedureData));

      /* Send the response message.                                     */
      MSG_SendMessage((BTPM_Message_t *)&Response);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessWriteSensorControlPointRequestCPPM and           */
   /* the server side API function CPPM_Write_Sensor_Control_Point,     */
   /* WriteSensorControlPointCPPM translates procedure data from a PM   */
   /* defined structure to a Bluetopia defined structure for use in the */
   /* Bluetopia API function CPS_Format_Control_Point_Command.          */
   /* CPS_Format_Control_Point_Command converts the data to a buffer    */
   /* that can be used in a GATM write. The opcode and any other        */
   /* procedure data is written to the control point characteristic     */
   /* thereby initiating a procedure.                                   */
static int WriteSensorControlPointCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int InstanceID, CPPM_Procedure_Data_t *ProcedureData)
{
   int                              Result;
   Callback_Entry_t                *CallbackEntry;
   Instance_Entry_t                *InstanceEntry;
   Transaction_Entry_t              TransactionEntry;
   CPS_Control_Point_Format_Data_t  ProcedureDataCPS;
   unsigned int                     BufferLength;
   Byte_t                          *Buffer;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((CallbackID) && (Sensor) && (!COMPARE_NULL_BD_ADDR(*Sensor)) && (InstanceID) && (ProcedureData))
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Search for the instance.                                    */
         if((InstanceEntry = SearchDeviceInstanceEntry(&(DeviceEntryList), Sensor, InstanceID)) != NULL)
         {
            /* Configure the Transaction Entry.                         */
            BTPS_MemInitialize(&(TransactionEntry), 0, sizeof(Transaction_Entry_t));

            TransactionEntry.TransactionType = ttWriteCyclingPowerControlPoint;

            BTPS_MemInitialize(&(ProcedureDataCPS), 0, CPS_CONTROL_POINT_FORMAT_DATA_SIZE);

            /* Convert CPPM procedure data to CPS Bluetopia procedure   */
            /* data and allocate a buffer to be used in the GATM write. */
            if((Buffer = ConvertProcedureDataCPPM(&(ProcedureDataCPS), ProcedureData)) != NULL)
            {
               BufferLength = sizeof(ProcedureDataCPS);

               /* Write the command to a byte buffer.                   */
               if((Result = CPS_Format_Control_Point_Command(&(ProcedureDataCPS), &(BufferLength), Buffer)) == 0)
               {
                  /* Perform the write.                                 */
                  if((Result = GATM_WriteValue(GATMCallbackID, *Sensor, InstanceEntry->ServiceHandles.CP_Control_Point, BufferLength, Buffer)) > 0)
                  {
                     /* Save the GATM Transaction ID.                   */
                     TransactionEntry.TransactionID = (unsigned int)Result;

                     /* Add the Transaction Entry.                      */
                     if(!(AddTransactionEntry(&(CallbackEntry->TransactionEntryList), &TransactionEntry)))
                        Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }
               }

               BTPS_FreeMemory(Buffer);
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* ProcessQuerySensorsRequestCPPM counts the number of sensors in    */
   /* the device list and conditionally, returns a list of bluetooth    */
   /* addresses. The NumberOfSensors element of the request message     */
   /* should equal the number of sensors that can fit into the client   */
   /* provided buffer. If the NumberOfSensors value is positive then at */
   /* most NumberOfSensors addresses will be included in the response.  */
   /* Unless an error occurs, the total number of sensors will be       */
   /* returned in the status field of the response.                     */
static void ProcessQuerySensorsRequestCPPM(CPPM_Query_Sensors_Request_t *Message)
{
   int                            Result;
   CPPM_Query_Sensors_Response_t *Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Response = NULL;

      if((Result = QuerySensorsCPPM(Message->CallbackID, &(Message->NumberOfSensors))) > 0)
      {
         if((Response = (CPPM_Query_Sensors_Response_t *)BTPS_AllocateMemory(CPPM_QUERY_SENSORS_RESPONSE_SIZE(Message->NumberOfSensors))) != NULL)
         {
            if((Response->NumberOfSensors = Message->NumberOfSensors) > 0)
            {
               ListSensorsCPPM(Response->Sensors, Response->NumberOfSensors);
            }
         }
      }
      else
      {
         if((Response = (CPPM_Query_Sensors_Response_t *)BTPS_AllocateMemory(CPPM_QUERY_SENSORS_RESPONSE_SIZE(0))) != NULL)
            Response->NumberOfSensors = 0;
      }

      if(Response)
      {
         /* Format the response message.                                */
         Response->MessageHeader                 = Message->MessageHeader;
         Response->MessageHeader.MessageID      |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
         Response->MessageHeader.MessageLength   = CPPM_QUERY_SENSORS_RESPONSE_SIZE(Response->NumberOfSensors) - BTPM_MESSAGE_HEADER_SIZE;

         Response->Status                        = Result;

         MSG_SendMessage((BTPM_Message_t *)Response);

         BTPS_FreeMemory(Response);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessQuerySensorsRequestCPPM and CPPM_Query_Sensors,  */
   /* QuerySensorsCPPM returns the total number of sensors in the       */
   /* device list. If the value pointed to by NumberOfSensors is        */
   /* greater than the total number of devices it will be set equal to  */
   /* that total.                                                       */
static int QuerySensorsCPPM(unsigned int CallbackID, unsigned int *NumberOfSensors)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;
   Device_Entry_t   *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackID)
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Count the sensors in the device entry list.                 */
         for(Result = 0, DeviceEntry = DeviceEntryList; DeviceEntry;)
         {
            Result++;

            DeviceEntry = DeviceEntry->NextDeviceEntry;
         }

         if(NumberOfSensors)
         {
            if(Result < *NumberOfSensors)
               *NumberOfSensors = Result;
         }
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* Called by ProcessQuerySensorsRequestCPPM and the server side API  */
   /* function CPPM_Query_Sensors, ListSensorsCPPM copies               */
   /* NumberOfSensors bluetooth addresses from the device entry list to */
   /* the Sensors address buffer.                                       */
static void ListSensorsCPPM(BD_ADDR_t *Sensors, unsigned int NumberOfSensors)
{
   unsigned int    Index;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Sensors)
   {
      /* Fill the buffer.                                               */
      for(Index = 0, DeviceEntry = DeviceEntryList; Index < NumberOfSensors && DeviceEntry; Index++)
      {
         Sensors[Index] = DeviceEntry->BluetoothAddress;

         DeviceEntry    = DeviceEntry->NextDeviceEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessQuerySensorInstancesRequestCPPM counts the number of       */
   /* service instances on a specified sensor and may return            */
   /* information about the instances. The total number of instances    */
   /* will be returned in the response status unless an error occurs.   */
static void ProcessQuerySensorInstancesRequestCPPM(CPPM_Query_Sensor_Instances_Request_t *Message)
{
   int                                     Result;
   CPPM_Query_Sensor_Instances_Response_t *Response;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Message)
   {
      Response = NULL;

      if((Result = QuerySensorInstancesCPPM(Message->CallbackID, &(Message->Sensor), &(Message->NumberOfInstances))) > 0)
      {
         if((Response = (CPPM_Query_Sensor_Instances_Response_t *)BTPS_AllocateMemory(CPPM_QUERY_SENSOR_INSTANCES_RESPONSE_SIZE(Message->NumberOfInstances))) != NULL)
         {
            if((Response->NumberOfInstances = Message->NumberOfInstances) > 0)
            {
               ListSensorInstancesCPPM(&(Message->Sensor), Response->Instances, Response->NumberOfInstances);
            }
         }
      }
      else
      {
         if((Response = (CPPM_Query_Sensor_Instances_Response_t *)BTPS_AllocateMemory(CPPM_QUERY_SENSOR_INSTANCES_RESPONSE_SIZE(0))) != NULL)
            Response->NumberOfInstances = 0;
      }

      if(Response)
      {
         /* Format the response message.                                */
         Response->MessageHeader                 = Message->MessageHeader;
         Response->MessageHeader.MessageID      |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
         Response->MessageHeader.MessageLength   = CPPM_QUERY_SENSOR_INSTANCES_RESPONSE_SIZE(Response->NumberOfInstances) - BTPM_MESSAGE_HEADER_SIZE;

         Response->Status                        = Result;

         MSG_SendMessage((BTPM_Message_t *)Response);

         BTPS_FreeMemory(Response);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessQuerySensorInstancesRequestCPPM and              */
   /* the server side API function CPPM_Query_Sensor_Instances,         */
   /* QuerySensorInstancesCPPM returns the total number of sensor       */
   /* service instances in the specified device's instance list. If the */
   /* value pointed to by NumberOfInstances is greater than the total   */
   /* number of instances it will be adjusted to that total.            */
static int QuerySensorInstancesCPPM(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int *NumberOfInstances)
{
   int               Result;
   Callback_Entry_t *CallbackEntry;
   Device_Entry_t   *DeviceEntry;
   Instance_Entry_t *InstanceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(CallbackID)
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
      {
         /* Search for the sensor.                                      */
         if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, Sensor)) != NULL)
         {
            /* Count the sensors in the device entry list.                 */
            for(Result = 0, InstanceEntry = DeviceEntry->InstanceEntryList; InstanceEntry;)
            {
               Result++;

               InstanceEntry = InstanceEntry->NextInstanceEntry;
            }

            if(NumberOfInstances)
            {
               if(Result < *NumberOfInstances)
                  *NumberOfInstances = Result;
            }
         }
         else
            Result = BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* Called by ProcessQuerySensorInstancesRequestCPPM and              */
   /* CPPM_Query_Sensor_Instances, ListInstancesCPPM copies             */
   /* NumberOfInstances sensor instance records from the instance entry */
   /* list to the Instances buffer.                                     */
static void ListSensorInstancesCPPM(BD_ADDR_t *Sensor, Instance_Record_t *Instances, unsigned int NumberOfInstances)
{
   Device_Entry_t   *DeviceEntry;
   unsigned int      Index;
   Instance_Entry_t *InstanceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Instances)
   {
      /* Search for the sensor.                                         */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, Sensor)) != NULL)
      {
         /* Fill the buffer.                                            */
         for(Index = 0, InstanceEntry = DeviceEntry->InstanceEntryList; Index < NumberOfInstances && InstanceEntry; Index++)
         {
            Instances[Index].InstanceID      = InstanceEntry->InstanceID;
            Instances[Index].StateMask       = InstanceEntry->StateMask;
            Instances[Index].FeatureMask     = InstanceEntry->FeatureMask;
            Instances[Index].SensorLocation  = InstanceEntry->SensorLocation;

            InstanceEntry                    = InstanceEntry->NextInstanceEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessHandleValueCPPM, called by ProcessGATMEventCPPM, handles   */
   /* measurement and vector notifications and control point            */
   /* indications.                                                      */
static void ProcessHandleValueCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData)
{
   Device_Entry_t      *DeviceEntry;
   Instance_Entry_t    *InstanceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(HandleValueData)
   {
      /* Search for the the device entry.                               */
      if((DeviceEntry = SearchDeviceEntry(&DeviceEntryList, &(HandleValueData->RemoteDeviceAddress))) != NULL)
      {
         if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement), HandleValueData->AttributeHandle)) != NULL)
         {
            /* Process the Cycling Power Measurement notification.      */
            MeasurementEventCPPM(HandleValueData, DeviceEntry->BluetoothAddress, InstanceEntry->InstanceID);
         }
         else
         {
            if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector), HandleValueData->AttributeHandle)) != NULL)
            {
               /* Process the Cycling Power Vector notification.        */
               VectorEventCPPM(HandleValueData, DeviceEntry->BluetoothAddress, InstanceEntry->InstanceID);
            }
            else
            {
               if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), HandleValueData->AttributeHandle)) != NULL)
               {
                  /* Process the Control Point indication.              */

                  /* Stop the procedure timer.                          */
                  TMR_StopTimer(InstanceEntry->ProcedureRecord.TimerID);

                  ControlPointEventCPPM(HandleValueData, DeviceEntry->BluetoothAddress, InstanceEntry->InstanceID, FALSE, InstanceEntry->ProcedureRecord.CallbackID, InstanceEntry->ProcedureRecord.AddressID);

                  InstanceEntry->ProcedureRecord.CallbackID       = 0;
                  InstanceEntry->ProcedureRecord.AddressID        = 0;
                  InstanceEntry->ProcedureRecord.BluetoothAddress = NULL;
                  InstanceEntry->ProcedureRecord.TimerID          = 0;
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessReadResponseCPPM, called by ProcessGATMEventCPPM, handles  */
   /* responses to reads of supported features, sensor location, the    */
   /* client configuration descriptors for measurements, features and   */
   /* the control point, as well as the measurement server              */
   /* configuration descriptor(broadcasts).                             */
static void ProcessReadResponseCPPM(GATM_Read_Response_Event_Data_t *ReadResponseData)
{
   unsigned int         Index;
   unsigned int         Descriptor;
   Device_Entry_t      *DeviceEntry;
   Instance_Entry_t    *InstanceEntry;
   Transaction_Entry_t *TransactionEntry;
   Callback_Entry_t    *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ReadResponseData)
   {
      /* Search for the device.                                         */
      if((DeviceEntry = SearchDeviceEntry(&(DeviceEntryList), &(ReadResponseData->RemoteDeviceAddress))) != NULL)
      {
         TransactionEntry = NULL;
         CallbackEntry    = NULL;

         /* Search for the callback entry.                              */
         SearchCallbackEntryByTransactionID(&(CallbackEntryList), &(CallbackEntry), &(TransactionEntry), ReadResponseData->TransactionID);

         if((CallbackEntry) && (TransactionEntry))
         {
            /* Search for the instance.                                 */
            if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Feature), ReadResponseData->Handle)) != NULL)
            {
               InstanceEntry->FeatureMask = 0;

               /* Shift the little endian byte array.                   */
               for(Index = 0; Index < ReadResponseData->ValueLength; Index++)
                  InstanceEntry->FeatureMask |= ((unsigned long)(ReadResponseData->Value[Index] << (CHAR_BIT * Index)));

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Feature Read Response - ValueLength: %u Value: %lu\n", ReadResponseData->ValueLength, InstanceEntry->FeatureMask));

               /* Callback ID zero is the ID of the dummy callback      */
               /* entry used when automatically reading sensor values   */
               /* for saved sensor instances. The event information     */
               /* won't be sent.                                        */
               if(CallbackEntry->CallbackID)
                  SensorFeaturesEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ReadResponseData->TransactionID, ReadResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, InstanceEntry->FeatureMask, 0);
            }
            else
            {
               /* Search for the instance.                              */
               if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, Sensor_Location), ReadResponseData->Handle)) != NULL)
               {
                  InstanceEntry->SensorLocation = 0;

                  /* Shift the little endian byte array.                */
                  for(Index = 0; Index < ReadResponseData->ValueLength; Index++)
                     InstanceEntry->SensorLocation |= (ReadResponseData->Value[Index] << (CHAR_BIT * Index));

                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Feature Read Response - ValueLength: %u Value: %lu\n", ReadResponseData->ValueLength, (unsigned long)(InstanceEntry->SensorLocation)));

                  /* Callback ID zero is the ID of the dummy callback   */
                  /* entry used when automatically reading sensor       */
                  /* values for saved sensor instances. The event       */
                  /* information won't be sent.                         */
                  if(CallbackEntry->CallbackID)
                     SensorLocationEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ReadResponseData->TransactionID, ReadResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, InstanceEntry->SensorLocation, 0);
               }
               else
               {
                  /* Search for the instance.                           */
                  if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration), ReadResponseData->Handle)) != NULL)
                  {
                     Descriptor = 0;

                     /* Shift the little endian byte array.             */
                     for(Index = 0; Index < ReadResponseData->ValueLength; Index++)
                        Descriptor |= (ReadResponseData->Value[Index] << (CHAR_BIT * Index));

                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Measurement_Client_Configuration Read Response - ValueLength: %u Value: %u\n", ReadResponseData->ValueLength, Descriptor));

                     /* Update the state mask.                          */
                     if(Descriptor)
                        InstanceEntry->StateMask |=   CPPM_SENSOR_STATE_MEASUREMENT_ENABLED;
                     else
                        InstanceEntry->StateMask &= ~(CPPM_SENSOR_STATE_MEASUREMENT_ENABLED);
                  }
                  else
                  {
                     /* Search for the instance.                        */
                     if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration), ReadResponseData->Handle)) != NULL)
                     {
                        Descriptor = 0;

                        /* Shift the little endian byte array.          */
                        for(Index = 0; Index < ReadResponseData->ValueLength; Index++)
                           Descriptor |= (ReadResponseData->Value[Index] << (CHAR_BIT * Index));

                        DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Vector_Client_Configuration Read Response - ValueLength: %u Value: %u\n", ReadResponseData->ValueLength, Descriptor));

                        /* Update the state mask.                       */
                        if(Descriptor)
                           InstanceEntry->StateMask |=   CPPM_SENSOR_STATE_VECTOR_ENABLED;
                        else
                           InstanceEntry->StateMask &= ~(CPPM_SENSOR_STATE_VECTOR_ENABLED);
                     }
                     else
                     {
                        /* Search for the instance.                     */
                        if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration), ReadResponseData->Handle)) != NULL)
                        {
                           Descriptor = 0;

                           /* Shift the little endian byte array.       */
                           for(Index = 0; Index < ReadResponseData->ValueLength; Index++)
                              Descriptor |= (ReadResponseData->Value[Index] << (CHAR_BIT * Index));

                           DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Control_Point_Client_Configuration Read Response - ValueLength: %u Value: %u\n", ReadResponseData->ValueLength, Descriptor));

                           /* Update the state mask.                    */
                           if(Descriptor)
                              InstanceEntry->StateMask |=   CPPM_SENSOR_STATE_CONTROL_POINT_ENABLED;
                           else
                              InstanceEntry->StateMask &= ~(CPPM_SENSOR_STATE_CONTROL_POINT_ENABLED);
                        }
                        else
                        {
                           /* Search for the instance.                  */
                           if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Server_Configuration), ReadResponseData->Handle)) != NULL)
                           {
                              Descriptor = 0;

                              /* Shift the little endian byte array.    */
                              for(Index = 0; Index < ReadResponseData->ValueLength; Index++)
                                 Descriptor |= (ReadResponseData->Value[Index] << (CHAR_BIT * Index));

                              DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Measurement_Server_Configuration Read Response - ValueLength: %u Value: %u\n", ReadResponseData->ValueLength, Descriptor));

                              /* Update the state mask.                 */
                              if(Descriptor)
                                 InstanceEntry->StateMask |=   CPPM_SENSOR_STATE_BROADCAST_ENABLED;
                              else
                                 InstanceEntry->StateMask &= ~(CPPM_SENSOR_STATE_BROADCAST_ENABLED);
                           }
                        }
                     }
                  }
               }
            }

            FreeTransactionEntryMemory(TransactionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessWriteResponseCPPM, called by ProcessGATMEventCPPM, handles */
   /* responses to writes of the measurement, vector and control point  */
   /* client configuration descriptors, the measurement server          */
   /* configuration descriptor and the control point characteristic for */
   /* procedure initiation.                                             */
static void ProcessWriteResponseCPPM(GATM_Write_Response_Event_Data_t *WriteResponseData)
{
   Device_Entry_t      *DeviceEntry;
   Instance_Entry_t    *InstanceEntry;
   Transaction_Entry_t *TransactionEntry;
   Callback_Entry_t    *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(WriteResponseData)
   {
      /* Search for the the device entry.                               */
      if((DeviceEntry = SearchDeviceEntry(&(DeviceEntryList), &(WriteResponseData->RemoteDeviceAddress))) != NULL)
      {
         TransactionEntry = NULL;
         CallbackEntry    = NULL;

         /* Search for the callback entry.                              */
         SearchCallbackEntryByTransactionID(&(CallbackEntryList), &(CallbackEntry), &(TransactionEntry), WriteResponseData->TransactionID);

         if((CallbackEntry) && (TransactionEntry))
         {
            /* Search for the instance.                                 */
            if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration), WriteResponseData->Handle)) != NULL)
            {
               /* Update the state mask.                                */
               switch(TransactionEntry->TransactionType)
               {
                  case ttEnableCyclingPowerMeasurement:
                     InstanceEntry->StateMask |=   CPPM_SENSOR_STATE_MEASUREMENT_ENABLED;
                     break;
                  case ttDisableCyclingPowerMeasurement:
                     InstanceEntry->StateMask &= ~(CPPM_SENSOR_STATE_MEASUREMENT_ENABLED);
                     break;
                  default:
                     break;
               }

               WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, WriteResponseData->TransactionID, WriteResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetMeasurementsSetCPP, 0);

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Measurement_Client_Configuration Write Response - Handle: %u Transaction ID: %u\n", WriteResponseData->Handle, WriteResponseData->TransactionID));
            }
            else
            {
               if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration), WriteResponseData->Handle)) != NULL)
               {
                  /* Update the state mask.                             */
                  switch(TransactionEntry->TransactionType)
                  {
                     case ttEnableCyclingPowerVector:
                        InstanceEntry->StateMask |=   CPPM_SENSOR_STATE_VECTOR_ENABLED;
                        break;
                     case ttDisableCyclingPowerVector:
                        InstanceEntry->StateMask &= ~(CPPM_SENSOR_STATE_VECTOR_ENABLED);
                        break;
                     default:
                        break;
                  }

                  WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, WriteResponseData->TransactionID, WriteResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetVectorsSetCPP, 0);

                  DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Vector_Client_Configuration Write Response - Handle: %u Transaction ID: %u\n", WriteResponseData->Handle, WriteResponseData->TransactionID));
               }
               else
               {
                  if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration), WriteResponseData->Handle)) != NULL)
                  {
                     /* Update the state mask.                          */
                     switch(TransactionEntry->TransactionType)
                     {
                        case ttEnableCyclingPowerControlPoint:
                           InstanceEntry->StateMask |=  CPPM_SENSOR_STATE_CONTROL_POINT_ENABLED;
                           break;
                        case ttDisableCyclingPowerControlPoint:
                           InstanceEntry->StateMask &= ~CPPM_SENSOR_STATE_CONTROL_POINT_ENABLED;
                           break;
                        default:
                           break;
                     }

                     WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, WriteResponseData->TransactionID, WriteResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetProceduresSetCPP, 0);

                     DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Control_Point_Client_Configuration Write Response - Handle: %u Transaction ID: %u\n", WriteResponseData->Handle, WriteResponseData->TransactionID));
                  }
                  else
                  {
                     if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), WriteResponseData->Handle)) != NULL)
                     {
                        /* Record the procedure information in the      */
                        /* instance entry and start the timer.          */
                        InstanceEntry->ProcedureRecord.CallbackID        = CallbackEntry->CallbackID;
                        InstanceEntry->ProcedureRecord.AddressID         = CallbackEntry->AddressID;
                        InstanceEntry->ProcedureRecord.BluetoothAddress  = &(DeviceEntry->BluetoothAddress);
                        InstanceEntry->ProcedureRecord.TimerID           = TMR_StartTimer((void *)(InstanceEntry), QueueProcedureTimeoutEventCPPM, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));

                        WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, WriteResponseData->TransactionID, WriteResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetProcedureBegunCPP, 0);

                        DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Control_Point Write Response - Handle: %u Transaction ID: %u\n", WriteResponseData->Handle, WriteResponseData->TransactionID));
                     }
                     else
                     {
                        if((InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Server_Configuration), WriteResponseData->Handle)) != NULL)
                        {
                           switch(TransactionEntry->TransactionType)
                           {
                              case ttEnableCyclingPowerBroadcast:
                                 InstanceEntry->StateMask |=  CPPM_SENSOR_STATE_BROADCAST_ENABLED;
                                 break;
                              case ttDisableCyclingPowerBroadcast:
                                 InstanceEntry->StateMask &= ~CPPM_SENSOR_STATE_BROADCAST_ENABLED;
                                 break;
                              default:
                                 break;
                           }

                           WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, WriteResponseData->TransactionID, WriteResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetBroadcastsSetCPP, 0);

                           DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("CP_Measurement_Server_Configuration Write Response - Handle: %u Transaction ID: %u\n", WriteResponseData->Handle, WriteResponseData->TransactionID));
                        }
                     }
                  }
               }
            }

            FreeTransactionEntryMemory(TransactionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ProcessErrorResponseCPPM, called by ProcessGATMEventCPPM, handles */
   /* error responses to all GATM transactions, including reads and     */
   /* writes of the measurement, vector and control point configuration */
   /* descriptors, writes of the control point characteristic and reads */
   /* of sensor features and the sensor location.                       */
static void ProcessErrorResponseCPPM(GATM_Error_Response_Event_Data_t *ErrorResponseData)
{
   Device_Entry_t      *DeviceEntry;
   Instance_Entry_t    *InstanceEntry;
   Transaction_Entry_t *TransactionEntry;
   Callback_Entry_t    *CallbackEntry;
   Boolean_t            Found;
   int                  Status;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ErrorResponseData)
   {
      /* Search for the the device entry.                               */
      if((DeviceEntry = SearchDeviceEntry(&(DeviceEntryList), &(ErrorResponseData->RemoteDeviceAddress))) != NULL)
      {
         TransactionEntry = NULL;
         CallbackEntry    = NULL;

         /* Search for the callback entry.                              */
         SearchCallbackEntryByTransactionID(&(CallbackEntryList), &(CallbackEntry), &(TransactionEntry), ErrorResponseData->TransactionID);

         if((CallbackEntry) && (TransactionEntry))
         {
            Found = FALSE;

            switch(ErrorResponseData->ErrorType)
            {
               case retErrorResponse:
                  Status = ErrorResponseData->AttributeProtocolErrorCode;
                  break;
               case retProtocolTimeout:
                  Status = BTPM_ERROR_CODE_CYCLING_POWER_TRANSACTION_TIMEOUT;
                  break;
               case retPrepareWriteDataMismatch:
               default:
                  Status = BTPM_ERROR_CODE_CYCLING_POWER_UNKNOWN_ERROR;
                  break;
            }

            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("GATM error type: %d\n", Status));

            /* Search for the instance.                                 */
            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttEnableCyclingPowerMeasurement || TransactionEntry->TransactionType == ttDisableCyclingPowerMeasurement)
               {
                  WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetMeasurementsSetCPP, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Measurement_Client_Configuration Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttEnableCyclingPowerVector || TransactionEntry->TransactionType == ttDisableCyclingPowerVector)
               {
                  WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetVectorsSetCPP, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Vector_Client_Configuration Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttEnableCyclingPowerControlPoint || TransactionEntry->TransactionType == ttDisableCyclingPowerControlPoint)
               {
                  WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetProceduresSetCPP, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Control_Point_Client_Configuration Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Server_Configuration), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttEnableCyclingPowerBroadcast || TransactionEntry->TransactionType == ttDisableCyclingPowerBroadcast)
               {
                  WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetBroadcastsSetCPP, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Measurement_Server_Configuration Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttWriteCyclingPowerControlPoint)
               {
                  WriteResponseEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, cetProcedureBegunCPP, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Control_Point Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Feature), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttReadCyclingPowerSensorFeatures)
               {
                  if(CallbackEntry->CallbackID)
                     SensorFeaturesEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, 0, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Feature Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if((!Found) && (InstanceEntry = SearchInstanceEntryByAttributeHandle(&(DeviceEntry->InstanceEntryList), BTPS_STRUCTURE_OFFSET(Instance_Entry_t, ServiceHandles) + BTPS_STRUCTURE_OFFSET(CPS_Client_Information_t, Sensor_Location), ErrorResponseData->Handle)) != NULL)
            {
               if(TransactionEntry->TransactionType == ttReadCyclingPowerSensorLocation)
               {
                  if(CallbackEntry->CallbackID)
                     SensorLocationEventCPPM(CallbackEntry->CallbackID, CallbackEntry->AddressID, ErrorResponseData->TransactionID, ErrorResponseData->RemoteDeviceAddress, InstanceEntry->InstanceID, 0, Status);
               }

               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("CP_Sensor_Location Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

               Found = TRUE;
            }

            if(!Found)
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("Unknown Error Response - Transaction Type: %u Transaction ID: %u\n", TransactionEntry->TransactionType, ErrorResponseData->TransactionID));

            FreeTransactionEntryMemory(TransactionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* MeasurementEventCPPM formats and sends a measurement notification */
   /* message to PM clients or runs a callback function if one has      */
   /* been registered via the server-side API.                          */
static void MeasurementEventCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData, BD_ADDR_t BluetoothAddress, unsigned int InstanceID)
{
   int                         Result;
   CPS_Measurement_Data_t      MeasurementDataCPS;
   CPPM_Measurement_Data_t     MeasurementDataCPPM;
   Callback_Entry_t           *CallbackEntry;
   Unsolicited_Update_Entry_t *UnsolicitedUpdateEntry;
   CPPM_Event_Data_t           EventDataCPPM;
   CPPM_Measurement_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter - AttributeValueLength: %u\n", HandleValueData->AttributeValueLength));

   if(HandleValueData)
   {
      BTPS_MemInitialize(&MeasurementDataCPS, 0, sizeof(MeasurementDataCPS));

      /* Decode the measurement using the Bluetopia API function.       */
      if((Result = CPS_Decode_CP_Measurement((unsigned int)HandleValueData->AttributeValueLength, HandleValueData->AttributeValue, &(MeasurementDataCPS))) == 0)
      {
         BTPS_MemInitialize(&MeasurementDataCPPM, 0, sizeof(MeasurementDataCPPM));

         /* Move the Bluetopia structure data to a PM structure.        */
         ConvertMeasurementCPPM(&(MeasurementDataCPS), &(MeasurementDataCPPM));

         /* Search each callback entry's update list for measurement    */
         /* registrations.                                              */
         for(CallbackEntry = CallbackEntryList; CallbackEntry;)
         {
            /* Search for a measurement notification registration.      */
            if((UnsolicitedUpdateEntry = SearchUnsolicitedUpdateEntry(&(CallbackEntry->UnsolicitedUpdateEntryList), &(HandleValueData->RemoteDeviceAddress), InstanceID, HandleValueData->AttributeHandle)) != NULL)
            {
               /* Check the callback type.                              */
               if(CallbackEntry->AddressID == MSG_GetServerAddressID())
               {
                  /* Format the event data for server side callbacks.   */
                  EventDataCPPM.EventCallbackID                                    = CallbackEntry->CallbackID;
                  EventDataCPPM.EventType                                          = cetMeasurementCPP;
                  EventDataCPPM.EventLength                                        = CPPM_MEASUREMENT_EVENT_DATA_SIZE;

                  EventDataCPPM.EventData.MeasurementEventData.RemoteDeviceAddress = BluetoothAddress;
                  EventDataCPPM.EventData.MeasurementEventData.InstanceID          = InstanceID;
                  EventDataCPPM.EventData.MeasurementEventData.Measurement         = MeasurementDataCPPM;

                  /* Call the registered function.                      */
                  ServerEventCPPM(&EventDataCPPM);
               }
               else
               {
                  /* Format the message for PM clients.                 */
                  Message.MessageHeader.MessageID        = MSG_GetNextMessageID();
                  Message.MessageHeader.AddressID        = CallbackEntry->AddressID;
                  Message.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
                  Message.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_MEASUREMENT;
                  Message.MessageHeader.MessageLength    = (CPPM_MEASUREMENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
                  Message.CallbackID                     = CallbackEntry->CallbackID;
                  Message.RemoteDeviceAddress            = BluetoothAddress;
                  Message.InstanceID                     = InstanceID;
                  Message.Measurement                    = MeasurementDataCPPM;

                  MSG_SendMessage((BTPM_Message_t *)&Message);
               }
            }

            CallbackEntry = CallbackEntry->NextCallbackEntry;
         }
      }
      else
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FAILURE), ("CPS_Decode_CP_Measurement Result: %d\n", Result));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* VectorEventCPPM formats and sends a vector notification message   */
   /* to PM clients or runs a callback function if one has been         */
   /* registered via the server-side API.                               */
static void VectorEventCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData, BD_ADDR_t BluetoothAddress, unsigned int InstanceID)
{
   CPS_Vector_Data_t          *VectorDataCPS;
   CPPM_Vector_Data_t          VectorDataCPPM;
   Callback_Entry_t           *CallbackEntry;
   Unsolicited_Update_Entry_t *UnsolicitedUpdateEntry;
   CPPM_Event_Data_t           EventDataCPPM;
   CPPM_Vector_Message_t      *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter - AttributeValueLength: %u\n", HandleValueData->AttributeValueLength));

   if(HandleValueData)
   {
      VectorDataCPS = NULL;

      /* Decode handle value data into Bluetopia structure.             */
      if((VectorDataCPS = CPS_Decode_CP_Vector((unsigned int)HandleValueData->AttributeValueLength, HandleValueData->AttributeValue)) != NULL)
      {
         BTPS_MemInitialize(&VectorDataCPPM, 0, sizeof(VectorDataCPPM));

         /* Copy the data into a PM structure.                          */
         ConvertVectorCPPM(VectorDataCPS, &(VectorDataCPPM));

         /* Search each callback entry's update list for a vector       */
         /* notification registration.                                  */
         for(CallbackEntry = CallbackEntryList; CallbackEntry;)
         {
            /* Search for the update entry.                             */
            if((UnsolicitedUpdateEntry = SearchUnsolicitedUpdateEntry(&(CallbackEntry->UnsolicitedUpdateEntryList), &(HandleValueData->RemoteDeviceAddress), InstanceID, HandleValueData->AttributeHandle)) != NULL)
            {
               /* Check the callback type.                              */
               if(CallbackEntry->AddressID == MSG_GetServerAddressID())
               {
                  /* Format the event data for server side callbacks.   */
                  EventDataCPPM.EventCallbackID                               = CallbackEntry->CallbackID;
                  EventDataCPPM.EventType                                     = cetVectorCPP;
                  EventDataCPPM.EventLength                                   = CPPM_VECTOR_EVENT_DATA_SIZE;

                  EventDataCPPM.EventData.VectorEventData.RemoteDeviceAddress = BluetoothAddress;
                  EventDataCPPM.EventData.VectorEventData.InstanceID          = InstanceID;
                  EventDataCPPM.EventData.VectorEventData.Vector              = VectorDataCPPM;

                  /* Call the registered function.                      */
                  ServerEventCPPM(&EventDataCPPM);
               }
               else
               {
                  /* Format the message for PM clients.                 */
                  if((Message = (CPPM_Vector_Message_t *)BTPS_AllocateMemory(CPPM_VECTOR_MESSAGE_SIZE(VectorDataCPPM.MagnitudeDataLength))) != NULL)
                  {
                     Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
                     Message->MessageHeader.AddressID        = CallbackEntry->AddressID;
                     Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
                     Message->MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_VECTOR;
                     Message->MessageHeader.MessageLength    = (CPPM_VECTOR_MESSAGE_SIZE(VectorDataCPPM.MagnitudeDataLength) - BTPM_MESSAGE_HEADER_SIZE);
                     Message->CallbackID                     = CallbackEntry->CallbackID;
                     Message->RemoteDeviceAddress            = BluetoothAddress;
                     Message->InstanceID                     = InstanceID;
                     Message->VectorFlags                    = VectorDataCPPM.Flags;
                     Message->CrankRevolutionData            = VectorDataCPPM.CrankRevolutionData;
                     Message->FirstCrankMeasurementAngle     = VectorDataCPPM.FirstCrankMeasurementAngle;
                     Message->MagnitudeDataLength            = VectorDataCPPM.MagnitudeDataLength;

                     BTPS_MemCopy(Message->InstantaneousMagnitude, VectorDataCPPM.InstantaneousMagnitude, ((sizeof(SWord_t)) * VectorDataCPPM.MagnitudeDataLength));

                     MSG_SendMessage((BTPM_Message_t *)Message);

                     BTPS_FreeMemory(Message);
                  }
               }
            }

            CallbackEntry = CallbackEntry->NextCallbackEntry;
         }

         CPS_Free_CP_Vector_Data(VectorDataCPS);
      }
      else
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FAILURE), ("CPS_Decode_CP_Vector Result: NULL\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ContrlPointEventCPPM formats and sends a control point indication */
   /* message to PM clients or runs a callback function if one has been */
   /* registered via the server-side API.                               */
static void ControlPointEventCPPM(GATM_Handle_Value_Data_Event_Data_t *HandleValueData, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, Boolean_t Timeout, unsigned int CallbackID, unsigned int AddressID)
{
   int                                Result;
   CPS_Control_Point_Response_Data_t  ControlPointDataCPS;
   Callback_Entry_t                  *CallbackEntry;
   Unsolicited_Update_Entry_t        *UnsolicitedUpdateEntry;
   CPPM_Event_Data_t                  EventDataCPPM;
   CPPM_Control_Point_Message_t      *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter - AttributeValueLength: %u\n", HandleValueData->AttributeValueLength));

   Message = NULL;

   if(HandleValueData)
   {
      BTPS_MemInitialize(&ControlPointDataCPS, 0, sizeof(CPS_Control_Point_Response_Data_t));

      /* Decode the handle value data into a Bluetopia data structure.  */
      if((Result = CPS_Decode_Control_Point_Response((unsigned int)HandleValueData->AttributeValueLength, HandleValueData->AttributeValue, &(ControlPointDataCPS))) == 0)
      {
         /* Allocate an event message structure and copy the Bluetopia  */
         /* data to it.                                                 */
         Message = ConvertControlPointCPPM(&(ControlPointDataCPS));

         /* If the procedure was a success, then memory may need to be  */
         /* allocated depending on the procedure type.                  */
         if(Message->ResponseCode == prcSuccessCPP)
         {
            /* Search in each callback enty's update list for the       */
            /* control point registration entry.                        */
            for(CallbackEntry = CallbackEntryList; CallbackEntry;)
            {
               if((UnsolicitedUpdateEntry = SearchUnsolicitedUpdateEntry(&(CallbackEntry->UnsolicitedUpdateEntryList), &(HandleValueData->RemoteDeviceAddress), InstanceID, HandleValueData->AttributeHandle)) != NULL)
               {
                  /* Check the callback type.                           */
                  if(CallbackEntry->AddressID == MSG_GetServerAddressID())
                  {
                     /* Format the event data for server side callbacks.*/
                     BTPS_MemInitialize(&(EventDataCPPM), 0, sizeof(CPPM_Event_Data_t));

                     EventDataCPPM.EventCallbackID                                           = CallbackEntry->CallbackID;
                     EventDataCPPM.EventType                                                 = cetControlPointCPP;
                     EventDataCPPM.EventLength                                               = CPPM_CONTROL_POINT_EVENT_DATA_SIZE;

                     EventDataCPPM.EventData.ControlPointEventData.RemoteDeviceAddress       = BluetoothAddress;
                     EventDataCPPM.EventData.ControlPointEventData.InstanceID                = InstanceID;
                     EventDataCPPM.EventData.ControlPointEventData.Timeout                   = FALSE;
                     EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Opcode       = Message->Opcode;
                     EventDataCPPM.EventData.ControlPointEventData.ControlPoint.ResponseCode = Message->ResponseCode;

                     /* If the supported sensor locations were          */
                     /* requested then memory needs to be allocated and */
                     /* the data needs to be copied from the message.   */
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

                     /* Call the registered function.                   */
                     ServerEventCPPM(&EventDataCPPM);

                     BTPS_FreeMemory(EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.SensorLocations);
                  }
                  else
                  {
                     /* Format the message for PM clients.              */
                     Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
                     Message->MessageHeader.AddressID        = CallbackEntry->AddressID;
                     Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
                     Message->MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_CONTROL_POINT;
                     Message->MessageHeader.MessageLength    = (CPPM_CONTROL_POINT_MESSAGE_SIZE(Message->ParameterLength) - BTPM_MESSAGE_HEADER_SIZE);
                     Message->CallbackID                     = CallbackEntry->CallbackID;
                     Message->RemoteDeviceAddress            = BluetoothAddress;
                     Message->InstanceID                     = InstanceID;
                     Message->Timeout                        = FALSE;

                     MSG_SendMessage((BTPM_Message_t *)Message);
                  }
               }

               CallbackEntry = CallbackEntry->NextCallbackEntry;
            }
         }
         else
         {
            /* If the procedure wasn't a success then the message       */
            /* lengths won't vary.                                      */
            if(AddressID == MSG_GetServerAddressID())
            {
               BTPS_MemInitialize(&(EventDataCPPM), 0, sizeof(CPPM_Event_Data_t));

               EventDataCPPM.EventCallbackID                                           = CallbackID;
               EventDataCPPM.EventType                                                 = cetControlPointCPP;
               EventDataCPPM.EventLength                                               = CPPM_CONTROL_POINT_EVENT_DATA_SIZE;

               EventDataCPPM.EventData.ControlPointEventData.RemoteDeviceAddress       = BluetoothAddress;
               EventDataCPPM.EventData.ControlPointEventData.InstanceID                = InstanceID;
               EventDataCPPM.EventData.ControlPointEventData.Timeout                   = FALSE;
               EventDataCPPM.EventData.ControlPointEventData.ControlPoint.Opcode       = Message->Opcode;
               EventDataCPPM.EventData.ControlPointEventData.ControlPoint.ResponseCode = Message->ResponseCode;
            }
            else
            {
               Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
               Message->MessageHeader.AddressID        = AddressID;
               Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
               Message->MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_CONTROL_POINT;
               Message->MessageHeader.MessageLength    = (CPPM_CONTROL_POINT_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE);
               Message->CallbackID                     = CallbackID;
               Message->RemoteDeviceAddress            = BluetoothAddress;
               Message->InstanceID                     = InstanceID;
               Message->Timeout                        = FALSE;

               MSG_SendMessage((BTPM_Message_t *)Message);
            }
         }

         BTPS_FreeMemory(Message);
      }
      else
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FAILURE), ("CPS_Decode_Control_Point_Response Result: %d\n", Result));
   }
   else
   {
      /* Check for a time out.                                          */
      if(Timeout)
      {
         if(AddressID == MSG_GetServerAddressID())
         {
            BTPS_MemInitialize(&(EventDataCPPM), 0, sizeof(CPPM_Event_Data_t));

            EventDataCPPM.EventCallbackID                                     = CallbackID;
            EventDataCPPM.EventType                                           = cetControlPointCPP;
            EventDataCPPM.EventLength                                         = CPPM_CONTROL_POINT_EVENT_DATA_SIZE;

            EventDataCPPM.EventData.ControlPointEventData.RemoteDeviceAddress = BluetoothAddress;
            EventDataCPPM.EventData.ControlPointEventData.InstanceID          = InstanceID;
            EventDataCPPM.EventData.ControlPointEventData.Timeout             = TRUE;
         }
         else
         {
            Message = (CPPM_Control_Point_Message_t *)BTPS_AllocateMemory(CPPM_CONTROL_POINT_MESSAGE_SIZE(0));

            BTPS_MemInitialize(Message, 0, sizeof(CPPM_CONTROL_POINT_MESSAGE_SIZE(0)));

            Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
            Message->MessageHeader.AddressID        = AddressID;
            Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
            Message->MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_CONTROL_POINT;
            Message->MessageHeader.MessageLength    = (CPPM_CONTROL_POINT_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE);
            Message->CallbackID                     = CallbackID;
            Message->RemoteDeviceAddress            = BluetoothAddress;
            Message->InstanceID                     = InstanceID;
            Message->Timeout                        = TRUE;

            MSG_SendMessage((BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }

         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_TIMEOUT), ("ControlPointEventCPPM - Procedure Timeout\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Registered as a callback function by TMR_StartTimer when the      */
   /* control point characteristic has been written successfully        */
   /* (see ProcessWriteResponseCPPM), QueueProcedureTimeoutEventCPPM    */
   /* is called when a time out occurs. It queues                       */
   /* ProcedureTimeoutEventCPPM as a platform manager callback.         */
static Boolean_t BTPSAPI QueueProcedureTimeoutEventCPPM(unsigned int TimerID, void *InstanceEntry)
{
   Boolean_t Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(BTPM_QueueMailboxCallback(ProcedureTimeoutEventCPPM, InstanceEntry))
   {
      Result = FALSE;

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FAILURE), ("QueueProcedureTimeoutEventCPPM - failed to queue timeout event\n"));
   }
   else
   {
      Result = TRUE;

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_TIMEOUT), ("QueueProcedureTimeoutEventCPPM - timeout event queued\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(Result);
}

   /* Queued as a platform manager callback by                          */
   /* QueueProcedureTimeoutEventCPPM, ProcedureTimeoutEventCPPM calls   */
   /* ControlPointEventCPPM with the boolean timeout parameter set to   */
   /* TRUE.                                                             */
static void BTPSAPI ProcedureTimeoutEventCPPM(void *InstanceEntryParameter)
{
   Instance_Entry_t *InstanceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(InstanceEntryParameter)
   {
      InstanceEntry = (Instance_Entry_t *)InstanceEntryParameter;

      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_TIMEOUT), ("ProcedureTimeoutEventCPPM - procedure timeout event\n"));

      ControlPointEventCPPM(NULL, *(InstanceEntry->ProcedureRecord.BluetoothAddress), InstanceEntry->InstanceID, TRUE, InstanceEntry->ProcedureRecord.CallbackID, InstanceEntry->ProcedureRecord.AddressID);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* WriteResponseEventCPPM sends response data from GATM write        */
   /* attempts via IPC messages to PM clients or calls a function       */
   /* registered via the server side API.                               */
static void WriteResponseEventCPPM(unsigned int CallbackID, unsigned int AddressID, unsigned int TransactionID, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, CPPM_Event_Type_t EventType, int Status)
{
   CPPM_Event_Data_t              EventDataCPPM;
   CPPM_Write_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the callback type.                                          */
   if(AddressID == MSG_GetServerAddressID())
   {
      /* Format the event data for server side callbacks.               */
      EventDataCPPM.EventCallbackID                                      = CallbackID;
      EventDataCPPM.EventType                                            = EventType;
      EventDataCPPM.EventLength                                          = CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.WriteResponseEventData.RemoteDeviceAddress = BluetoothAddress;
      EventDataCPPM.EventData.WriteResponseEventData.InstanceID          = InstanceID;
      EventDataCPPM.EventData.WriteResponseEventData.TransactionID       = TransactionID;
      EventDataCPPM.EventData.WriteResponseEventData.Status              = Status;

      /* Call the registered function.                                  */
      ServerEventCPPM(&EventDataCPPM);
   }
   else
   {
      /* Format the message for PM clients.                             */
      Message.MessageHeader.MessageID        = MSG_GetNextMessageID();
      Message.MessageHeader.AddressID        = AddressID;
      Message.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
      Message.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_WRITE_RESPONSE;
      Message.MessageHeader.MessageLength    = (CPPM_WRITE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      Message.CallbackID                     = CallbackID;
      Message.RemoteDeviceAddress            = BluetoothAddress;
      Message.InstanceID                     = InstanceID;
      Message.TransactionID                  = TransactionID;
      Message.EventType                      = EventType;
      Message.Status                         = Status;

      MSG_SendMessage((BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* SensorFeaturesEventCPPM sends the response to supported feature   */
   /* reads via an IPC message to PM clients or calls a function        */
   /* registered using the server side API.                             */
static void SensorFeaturesEventCPPM(unsigned int CallbackID, unsigned int AddressID, unsigned int TransactionID, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, unsigned long Features, int Status)
{
   CPPM_Event_Data_t               EventDataCPPM;
   CPPM_Sensor_Features_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the callback type.                                          */
   if(AddressID == MSG_GetServerAddressID())
   {
      /* Format the event data for server side callbacks.               */
      EventDataCPPM.EventCallbackID                                       = CallbackID;
      EventDataCPPM.EventType                                             = cetSensorFeaturesCPP;
      EventDataCPPM.EventLength                                           = CPPM_SENSOR_FEATURES_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.SensorFeaturesEventData.RemoteDeviceAddress = BluetoothAddress;
      EventDataCPPM.EventData.SensorFeaturesEventData.InstanceID          = InstanceID;
      EventDataCPPM.EventData.SensorFeaturesEventData.TransactionID       = TransactionID;
      EventDataCPPM.EventData.SensorFeaturesEventData.Features            = Features;
      EventDataCPPM.EventData.SensorFeaturesEventData.Status              = Status;

      /* Call the registered function.                                  */
      ServerEventCPPM(&EventDataCPPM);
   }
   else
   {
      /* Format the message for PM clients.                             */
      Message.MessageHeader.MessageID        = MSG_GetNextMessageID();
      Message.MessageHeader.AddressID        = AddressID;
      Message.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
      Message.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_SENSOR_FEATURES;
      Message.MessageHeader.MessageLength    = (CPPM_SENSOR_FEATURES_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      Message.CallbackID                     = CallbackID;
      Message.RemoteDeviceAddress            = BluetoothAddress;
      Message.InstanceID                     = InstanceID;
      Message.TransactionID                  = TransactionID;
      Message.Features                       = Features;
      Message.Status                         = Status;

      MSG_SendMessage((BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* SensorLocationEventCPPM sends the response to sensor location     */
   /* reads via an IPC message to PM clients or calls a function        */
   /* registered using the server side API.                             */
static void SensorLocationEventCPPM(unsigned int CallbackID, unsigned int AddressID, unsigned int TransactionID, BD_ADDR_t BluetoothAddress, unsigned int InstanceID, unsigned int Location, int Status)
{
   CPPM_Event_Data_t               EventDataCPPM;
   CPPM_Sensor_Location_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check the callback type.                                          */
   if(AddressID == MSG_GetServerAddressID())
   {
      /* Format the event data for server side callbacks.               */
      EventDataCPPM.EventCallbackID                                       = CallbackID;
      EventDataCPPM.EventType                                             = cetSensorLocationCPP;
      EventDataCPPM.EventLength                                           = CPPM_SENSOR_LOCATION_EVENT_DATA_SIZE;

      EventDataCPPM.EventData.SensorLocationEventData.RemoteDeviceAddress = BluetoothAddress;
      EventDataCPPM.EventData.SensorLocationEventData.InstanceID          = InstanceID;
      EventDataCPPM.EventData.SensorLocationEventData.TransactionID       = TransactionID;
      EventDataCPPM.EventData.SensorLocationEventData.Location            = Location;
      EventDataCPPM.EventData.SensorLocationEventData.Status              = Status;

      /* Call the registered function.                                  */
      ServerEventCPPM(&EventDataCPPM);
   }
   else
   {
      /* Format the message for PM clients.                             */
      Message.MessageHeader.MessageID        = MSG_GetNextMessageID();
      Message.MessageHeader.AddressID        = AddressID;
      Message.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
      Message.MessageHeader.MessageFunction  = CPPM_MESSAGE_FUNCTION_SENSOR_LOCATION;
      Message.MessageHeader.MessageLength    = (CPPM_SENSOR_LOCATION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
      Message.CallbackID                     = CallbackID;
      Message.RemoteDeviceAddress            = BluetoothAddress;
      Message.InstanceID                     = InstanceID;
      Message.TransactionID                  = TransactionID;
      Message.Location                       = Location;
      Message.Status                         = Status;

      MSG_SendMessage((BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ConvertMeasurementCPPM copies the measurement data from a         */
   /* Bluetopia structure to a PM structure.                            */
static void ConvertMeasurementCPPM(CPS_Measurement_Data_t *MeasurementDataCPS, CPPM_Measurement_Data_t *MeasurementDataCPPM)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((MeasurementDataCPS) && (MeasurementDataCPPM))
   {
      MeasurementDataCPPM->InstantaneousPower = MeasurementDataCPS->InstantaneousPower;

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_PRESENT)
      {
         MeasurementDataCPPM->Flags             |= CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_PRESENT;

         MeasurementDataCPPM->PedalPowerBalance  = MeasurementDataCPS->PedalPowerBalance;

         if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_REFERENCE_LEFT)
            MeasurementDataCPPM->Flags |= CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_REFERENCE_LEFT;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_PRESENT)
      {
         MeasurementDataCPPM->Flags             |= CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_PRESENT;

         MeasurementDataCPPM->AccumulatedTorque  = MeasurementDataCPS->AccumulatedTorque;

         if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_SOURCE_CRANK_BASED)
            MeasurementDataCPPM->Flags |= CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_SOURCE_CRANK_BASED;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT)
      {
         MeasurementDataCPPM->Flags                                          |= CPPM_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT;

         MeasurementDataCPPM->WheelRevolutionData.CumulativeWheelRevolutions  = MeasurementDataCPS->WheelRevolutionData.CumulativeWheelRevolutions;
         MeasurementDataCPPM->WheelRevolutionData.LastWheelEventTime          = MeasurementDataCPS->WheelRevolutionData.LastWheelEventTime;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT)
      {
         MeasurementDataCPPM->Flags                                          |= CPPM_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT;

         MeasurementDataCPPM->CrankRevolutionData.CumulativeCrankRevolutions  = MeasurementDataCPS->CrankRevolutionData.CumulativeCrankRevolutions;
         MeasurementDataCPPM->CrankRevolutionData.LastCrankEventTime          = MeasurementDataCPS->CrankRevolutionData.LastCrankEventTime;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_EXTREME_FORCE_MAGNITUDES_PRESENT)
      {
         MeasurementDataCPPM->Flags                                        |= CPPM_MEASUREMENT_FLAGS_EXTREME_FORCE_MAGNITUDES_PRESENT;

         MeasurementDataCPPM->ExtremeForceMagnitudes.MaximumForceMagnitude  = MeasurementDataCPS->ExtremeForceMagnitudes.MaximumForceMagnitude;
         MeasurementDataCPPM->ExtremeForceMagnitudes.MinimumForceMagnitude  = MeasurementDataCPS->ExtremeForceMagnitudes.MinimumForceMagnitude;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_EXTREME_TORQUE_MAGNITUDES_PRESENT)
      {
         MeasurementDataCPPM->Flags                                          |= CPPM_MEASUREMENT_FLAGS_EXTREME_TORQUE_MAGNITUDES_PRESENT;

         MeasurementDataCPPM->ExtremeTorqueMagnitudes.MaximumTorqueMagnitude  = MeasurementDataCPS->ExtremeTorqueMagnitudes.MaximumTorqueMagnitude;
         MeasurementDataCPPM->ExtremeTorqueMagnitudes.MinimumTorqueMagnitude  = MeasurementDataCPS->ExtremeTorqueMagnitudes.MinimumTorqueMagnitude;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_EXTREME_ANGLES_PRESENT)
      {
         MeasurementDataCPPM->Flags                      |= CPPM_MEASUREMENT_FLAGS_EXTREME_ANGLES_PRESENT;

         MeasurementDataCPPM->ExtremeAngles.MaximumAngle  = MeasurementDataCPS->ExtremeAngles.MaximumAngle;
         MeasurementDataCPPM->ExtremeAngles.MinimumAngle  = MeasurementDataCPS->ExtremeAngles.MinimumAngle;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT)
      {
         MeasurementDataCPPM->Flags            |= CPPM_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT;

         MeasurementDataCPPM->TopDeadSpotAngle  = MeasurementDataCPS->TopDeadSpotAngle;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT)
      {
         MeasurementDataCPPM->Flags               |= CPPM_MEASUREMENT_FLAGS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT;

         MeasurementDataCPPM->BottomDeadSpotAngle  = MeasurementDataCPS->BottomDeadSpotAngle;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_ACCUMULATED_ENERGY_PRESENT)
      {
         MeasurementDataCPPM->Flags             |= CPPM_MEASUREMENT_FLAGS_ACCUMULATED_ENERGY_PRESENT;

         MeasurementDataCPPM->AccumulatedEnergy  = MeasurementDataCPS->AccumulatedEnergy;
      }

      if(MeasurementDataCPS->Flags & CPS_MEASUREMENT_FLAGS_OFFSET_COMPENSATION_INDICATOR)
         MeasurementDataCPPM->Flags |= CPPM_MEASUREMENT_FLAGS_OFFSET_COMPENSATION_INDICATOR;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ConvertVectorCPPM copies vector data from a Bluetopia structure   */
   /* to a PM structure.                                                */
static void ConvertVectorCPPM(CPS_Vector_Data_t *VectorDataCPS, CPPM_Vector_Data_t *VectorDataCPPM)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(VectorDataCPS->Flags & CPS_VECTOR_FLAGS_CRANK_REVOLUTION_DATA_PRESENT)
   {
      VectorDataCPPM->Flags                                          |= CPPM_VECTOR_FLAGS_CRANK_REVOLUTION_DATA_PRESENT;

      VectorDataCPPM->CrankRevolutionData.CumulativeCrankRevolutions  = VectorDataCPS->CrankRevolutionData.CumulativeCrankRevolutions;
      VectorDataCPPM->CrankRevolutionData.LastCrankEventTime          = VectorDataCPS->CrankRevolutionData.LastCrankEventTime;
   }

   if(VectorDataCPS->Flags & CPS_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT)
   {
      VectorDataCPPM->Flags                      |= CPPM_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT;

      VectorDataCPPM->FirstCrankMeasurementAngle  = VectorDataCPS->FirstCrankMeasurementAngle;
   }

   if(VectorDataCPS->Flags & CPS_VECTOR_FLAGS_INSTANTANEOUS_FORCE_MAGNITUDE_ARRAY_PRESENT)
   {
      VectorDataCPPM->Flags                  |= CPPM_VECTOR_FLAGS_INSTANTANEOUS_FORCE_MAGNITUDE_ARRAY_PRESENT;

      VectorDataCPPM->MagnitudeDataLength     = VectorDataCPS->MagnitudeDataLength;
      VectorDataCPPM->InstantaneousMagnitude  = VectorDataCPS->InstantaneousMagnitude;
   }
   else
   {
      if(VectorDataCPS->Flags & CPS_VECTOR_FLAGS_INSTANTANEOUS_TORQUE_MAGNITUDE_ARRAY_PRESENT)
      {
         VectorDataCPPM->Flags                  |= CPPM_VECTOR_FLAGS_INSTANTANEOUS_TORQUE_MAGNITUDE_ARRAY_PRESENT;

         VectorDataCPPM->MagnitudeDataLength     = VectorDataCPS->MagnitudeDataLength;
         VectorDataCPPM->InstantaneousMagnitude  = VectorDataCPS->InstantaneousMagnitude;
      }
   }

   if(VectorDataCPS->Flags & CPS_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_TANGENTIAL_COMPONENT)
      VectorDataCPPM->Flags |= CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_TANGENTIAL_COMPONENT;

   if(VectorDataCPS->Flags & CPS_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_RADIAL_COMPONENT)
      VectorDataCPPM->Flags |= CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_RADIAL_COMPONENT;

   /* The instantaneous measurement direction is lateral if both the    */
   /* above bits are set.                                               */

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* ConvertControlPointCPPM populates and returns the control point   */
   /* IPC message structure using the Bluetopia control point data      */
   /* structure. The message size depends on the procedure type.        */
static CPPM_Control_Point_Message_t *ConvertControlPointCPPM(CPS_Control_Point_Response_Data_t *ControlPointDataCPS)
{
   int                           Index;
   unsigned int                  ParameterLength;
   CPPM_Control_Point_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Message = NULL;

   /* Check first for the a success response code.                      */
   if(ControlPointDataCPS->ResponseCodeValue == prcSuccessCPP)
   {
      /* Calculate the parameter length value.                          */
      switch(ControlPointDataCPS->RequestOpCode)
      {
         case CPS_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS:
            /* The CPPM_Sensor_Location_t structure contains the list   */
            /* locations plus an element set to the number of locations.*/
            ParameterLength = ((ControlPointDataCPS->ResponseParameter.SupportedSensorLocations.NumberOfSensorLocations * sizeof(CPPM_Sensor_Location_t)) + sizeof(ControlPointDataCPS->ResponseParameter.SupportedSensorLocations.NumberOfSensorLocations));
            break;
         case CPS_CONTROL_POINT_OPCODE_REQUEST_FACTORY_CALIBRATION_DATE:
            ParameterLength = sizeof(CPPM_Date_Time_Data_t);
            break;
         case CPS_CONTROL_POINT_OPCODE_REQUEST_CRANK_LENGTH:
         case CPS_CONTROL_POINT_OPCODE_REQUEST_CHAIN_LENGTH:
         case CPS_CONTROL_POINT_OPCODE_REQUEST_CHAIN_WEIGHT:
         case CPS_CONTROL_POINT_OPCODE_REQUEST_SPAN_LENGTH:
            ParameterLength = sizeof(Word_t);
            break;
         case CPS_CONTROL_POINT_OPCODE_START_OFFSET_COMPENSATION:
            ParameterLength = sizeof(SWord_t);
            break;
         case CPS_CONTROL_POINT_OPCODE_REQUEST_SAMPLING_RATE:
            ParameterLength = sizeof(Byte_t);
            break;
         default:
            ParameterLength = 0;
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("Control Point Response Code: %u\n", ControlPointDataCPS->ResponseCodeValue));
      ParameterLength = 0;
   }

   if((Message = (CPPM_Control_Point_Message_t *)BTPS_AllocateMemory(CPPM_CONTROL_POINT_MESSAGE_SIZE(ParameterLength))) != NULL)
   {
      BTPS_MemInitialize(Message, 0, CPPM_CONTROL_POINT_MESSAGE_SIZE(ParameterLength));

      Message->ResponseCode    = ControlPointDataCPS->ResponseCodeValue;
      Message->Opcode          = ControlPointDataCPS->RequestOpCode;
      Message->ParameterLength = ParameterLength;

      if((ParameterLength) && (Message->ResponseCode == prcSuccessCPP))
      {
         switch(Message->Opcode)
         {
            case pocRequestSupportedSensorLocations:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestSupportedSensorLocations Response\n"));
               Message->Parameter.SupportedSensorLocations.NumberOfSensorLocations = ControlPointDataCPS->ResponseParameter.SupportedSensorLocations.NumberOfSensorLocations;

               /* Copy each list element.                              */
               for(Index = 0; Index < Message->Parameter.SupportedSensorLocations.NumberOfSensorLocations; Index++)
                  Message->Parameter.SupportedSensorLocations.SensorLocations[Index] = ControlPointDataCPS->ResponseParameter.SupportedSensorLocations.SensorLocations[Index];

               /* Free the Bluetopia locations list.                    */
               CPS_Free_Supported_Sensor_Locations_Data(ControlPointDataCPS->ResponseParameter.SupportedSensorLocations.SensorLocations);
               break;
            case pocRequestFactoryCalibrationDate:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestFactoryCalibrationDate Response\n"));
               Message->Parameter.FactoryCalibrationDate.Year    = ControlPointDataCPS->ResponseParameter.FactoryCalibrationDate.Year;
               Message->Parameter.FactoryCalibrationDate.Month   = ControlPointDataCPS->ResponseParameter.FactoryCalibrationDate.Month;
               Message->Parameter.FactoryCalibrationDate.Day     = ControlPointDataCPS->ResponseParameter.FactoryCalibrationDate.Day;
               Message->Parameter.FactoryCalibrationDate.Hours   = ControlPointDataCPS->ResponseParameter.FactoryCalibrationDate.Hours;
               Message->Parameter.FactoryCalibrationDate.Minutes = ControlPointDataCPS->ResponseParameter.FactoryCalibrationDate.Minutes;
               Message->Parameter.FactoryCalibrationDate.Seconds = ControlPointDataCPS->ResponseParameter.FactoryCalibrationDate.Seconds;
               break;
            case pocRequestCrankLength:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestCrankLength Response\n"));
               Message->Parameter.CrankLength                    = ControlPointDataCPS->ResponseParameter.CrankLength;
               break;
            case pocRequestChainLength:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestChainLength Response\n"));
               Message->Parameter.ChainLength                    = ControlPointDataCPS->ResponseParameter.ChainLength;
               break;
            case pocRequestChainWeight:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestChainWeight Response\n"));
               Message->Parameter.ChainWeight                    = ControlPointDataCPS->ResponseParameter.ChainWeight;
               break;
            case pocRequestSpanLength:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestSpanLength Response\n"));
               Message->Parameter.SpanLength                     = ControlPointDataCPS->ResponseParameter.SpanLength;
               break;
            case pocStartOffsetCompensation:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocStartOffsetCompensation Response\n"));
               Message->Parameter.OffsetCompensation             = ControlPointDataCPS->ResponseParameter.OffsetCompensation;
               break;
            case pocRequestSamplingRate:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("pocRequestSamplingRate Response\n"));
               Message->Parameter.SamplingRate                   = ControlPointDataCPS->ResponseParameter.SamplingRate;
               break;
            default:
               break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return (Message);
}

   /* ConvertProcedureDataCPPM copies procedure data that is part of    */
   /* the write control point request message to a Bluetopia structure. */
   /* It also allocates and returns the buffer that will be used in the */
   /* GATM write.                                                       */
static Byte_t *ConvertProcedureDataCPPM(CPS_Control_Point_Format_Data_t *ProcedureDataCPS, CPPM_Procedure_Data_t *ProcedureDataCPPM)
{
   Byte_t *Buffer;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ProcedureDataCPS->CommandType = ProcedureDataCPPM->Opcode;

   Buffer = NULL;

   switch(ProcedureDataCPPM->Opcode)
   {
      case pocSetCumulativeValue:
         ProcedureDataCPS->CommandParameter.CumulativeValue = ProcedureDataCPPM->ProcedureParameter.CumulativeValue;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_DWORD_SIZE));
         break;
      case pocUdateSensorLocation:
         ProcedureDataCPS->CommandParameter.SensorLocation  = (Byte_t)ProcedureDataCPPM->ProcedureParameter.SensorLocation;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_BYTE_SIZE));
         break;
      case pocSetCrankLength:
         ProcedureDataCPS->CommandParameter.CrankLength     = ProcedureDataCPPM->ProcedureParameter.CrankLength;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_WORD_SIZE));
         break;
      case pocSetChainLength:
         ProcedureDataCPS->CommandParameter.ChainLength     = ProcedureDataCPPM->ProcedureParameter.ChainLength;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_WORD_SIZE));
         break;
      case pocSetChainWeight:
         ProcedureDataCPS->CommandParameter.ChainWeight     = ProcedureDataCPPM->ProcedureParameter.ChainWeight;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_WORD_SIZE));
         break;
      case pocSetSpanLength:
         ProcedureDataCPS->CommandParameter.SpanLength      = ProcedureDataCPPM->ProcedureParameter.SpanLength;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_WORD_SIZE));
         break;
      case pocMaskMeasurementCharacteristicContent:
         ProcedureDataCPS->CommandParameter.ContentMask     = ProcedureDataCPPM->ProcedureParameter.ContentMask;
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(NON_ALIGNED_WORD_SIZE));
         break;
      default:
         Buffer = (Byte_t *)BTPS_AllocateMemory(CPS_CONTROL_POINT_SIZE(0));
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(Buffer);
}

   /* ServerEventCPPM calls the function registered by                  */
   /* CPPM_Register_Collector_Event_Callback for server side            */
   /* applications.                                                     */
void ServerEventCPPM(CPPM_Event_Data_t *EventData)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Search the event callback entry list for the specified callback.  */
   if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), EventData->EventCallbackID)) != NULL)
   {
      /* Make sure that this is a local client.                         */
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
      {
         /* Release the Lock before dispatching the callback.           */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(CallbackEntry->EventCallback)
               (*CallbackEntry->EventCallback)(EventData, CallbackEntry->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         /* Re-acquire the Lock.                                        */
         DEVM_AcquireLock();
      }
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
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry to the list.                     */
            CallbackEntry.EventCallback     = CallbackFunction;
            CallbackEntry.CallbackParameter = CallbackParameter;

            CallbackEntry.AddressID         = MSG_GetServerAddressID();

            GetNextID(&NextCallbackID);
            CallbackEntry.CallbackID        = NextCallbackID;

            /* Attempt to add the entry to the local callback list.     */
            if(AddCallbackEntry(&CallbackEntryList, &CallbackEntry) != NULL)
            {
               Result = CallbackEntry.CallbackID;
            }
            else
            {
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }

            /* Release the mutex.                                       */
            DEVM_ReleaseLock();
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
      if(CallbackID)
      {
         if(DEVM_AcquireLock())
         {
            /* Search for the callback entry.                           */
            if((CallbackEntry = DeleteCallbackEntry(&(CallbackEntryList), CallbackID)) != NULL)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Callback Entry with CallbackID ID %u found\n", CallbackID));

               /* Free the callback entry.                              */
               FreeCallbackEntryMemory(CallbackEntry);
            }
            else
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("Callback Entry with CallbackID ID %u not found\n", CallbackID));

            DEVM_ReleaseLock();
         }
         else
            DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("The Lock could not be acquired\n"));
      }
      else
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("Callback ID is NULL\n"));
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("Module is not initialized\n"));

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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = RegisterUpdatesCPPM(CallbackID, &(RemoteSensor), InstanceID, ttEnableCyclingPowerMeasurement, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration), EnableUpdates, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = RegisterUpdatesCPPM(CallbackID, &(RemoteSensor), InstanceID, ttEnableCyclingPowerVector, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration), EnableUpdates, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = RegisterUpdatesCPPM(CallbackID, &(RemoteSensor), InstanceID, ttEnableCyclingPowerControlPoint, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration), EnableUpdates, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = UnregisterUpdatesCPPM(CallbackID, &(RemoteSensor), InstanceID, ttDisableCyclingPowerMeasurement, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Measurement_Client_Configuration));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = UnregisterUpdatesCPPM(CallbackID, &(RemoteSensor), InstanceID, ttDisableCyclingPowerVector, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Vector_Client_Configuration));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = UnregisterUpdatesCPPM(CallbackID, &(RemoteSensor), InstanceID, ttDisableCyclingPowerControlPoint, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point), STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Control_Point_Client_Configuration));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* CPPM_Enable_Broadcasts enables measurement broadcasts by writing  */
   /* the server configuration descriptor of the measurement            */
   /* characteristic of the specified sensor. If successful, the        */
   /* transaction ID of the GATM write is returned. The write response  */
   /* triggers a cetBroadcastsSetCPP type event which delivers the      */
   /* response in a CPPM_Write_Response_Event_Data_t type struct.       */
int BTPSAPI CPPM_Enable_Broadcasts(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = ToggleBroadcastsCPPM(CallbackID, &(RemoteSensor), InstanceID, TRUE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* CPPM_Disable_Broadcasts disables measurement broadcasts by        */
   /* writing the server configuration descriptor of the measurement    */
   /* characteristic of the specified sensor. If successful, the        */
   /* transaction ID of the GATM write is returned. The response comes  */
   /* in a cetBroadcastsSetCPP type event. The event data is delivered  */
   /* in a CPPM_Write_Response_Event_Data_t type struct.                */
int BTPSAPI CPPM_Disable_Broadcasts(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = ToggleBroadcastsCPPM(CallbackID, &(RemoteSensor), InstanceID, FALSE);

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* CPPM_Read_Sensor_Features reads the supported features of a       */
   /* sensor. If successful it returns the GATM transaction ID from the */
   /* read. The response is delivered in a cetSensorFeaturesCPP type    */
   /* event with a CPPM_Sensor_Features_Event_Data_t type struct.       */
int BTPSAPI CPPM_Read_Sensor_Features(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = ReadAttributeCPPM(CallbackID, &(RemoteSensor), InstanceID, ttReadCyclingPowerSensorFeatures, STRUCTURE_OFFSET(CPS_Client_Information_t, CP_Feature));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

   /* CPPM_Read_Sensor_Location reads the mount location of the sensor. */
   /* If successful, it returns the GATM transaction ID from the read.  */
   /* The read response is delivered in a cetSensorLocationCPP type     */
   /* event with a CPPM_Sensor_Location_Event_Data_t type struct.       */
int BTPSAPI CPPM_Read_Sensor_Location(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = ReadAttributeCPPM(CallbackID, &(RemoteSensor), InstanceID, ttReadCyclingPowerSensorLocation, STRUCTURE_OFFSET(CPS_Client_Information_t, Sensor_Location));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = WriteSensorControlPointCPPM(CallbackID, &(RemoteSensor), InstanceID, &(ProcedureData));

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((Result = QuerySensorsCPPM(CallbackID, NumberOfSensors)) > 0) && (NumberOfSensors) && (RemoteSensors))
   {
      ListSensorsCPPM(RemoteSensors, *NumberOfSensors);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
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
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((Result = QuerySensorInstancesCPPM(CallbackID, &(Sensor), NumberOfInstances)) > 0) && (NumberOfInstances) && (Instances))
   {
      ListSensorInstancesCPPM(&(Sensor), Instances, *NumberOfInstances);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit - Result: %d\n", Result));

   return (Result);
}

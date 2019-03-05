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
#define HRPM_COLLECTOR_CALLBACK_ID                       1

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHRP_Collector_Callback_Info_t
{
   HRPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} HRP_Collector_Callback_Info_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to protect access to the state information */
   /* in this module.                                                   */
static Mutex_t HRPManagerMutex;

static HRP_Collector_Callback_Info_t *CollectorCallbackInfo;

   /* Internal Function Prototypes.                                     */
static void DispatchHRPEvent(HRPM_Event_Data_t *HRPMEventData);

static void DispatchConnected(HRPM_Connected_Message_t *Message);
static void DispatchDisconnected(HRPM_Disconnected_Message_t *Message);
static void DispatchHeartRateMeasurement(HRPM_Heart_Rate_Measurement_Message_t *Message);
static void DispatchGetBodySensorLocationResponse(HRPM_Get_Body_Sensor_Location_Response_Message_t *Message);
static void DispatchResetEnergyExpendedResponse(HRPM_Reset_Energy_Expended_Response_Message_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);

static void BTPSAPI BTPMDispatchCallback_HRPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HRPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* dispatch the specified module event to the HRP Event Callback (if */
   /* registered).                                                      */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchHRPEvent(HRPM_Event_Data_t *HRPMEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((HRPMEventData) && (CollectorCallbackInfo))
   {
      /* Assign the Callback ID before dispatching.                     */
      HRPMEventData->EventCallbackID = HRPM_COLLECTOR_CALLBACK_ID;

      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HRPManagerMutex);

      __BTPSTRY
      {
         if(CollectorCallbackInfo->EventCallback)
         {
            (*CollectorCallbackInfo->EventCallback)(HRPMEventData, CollectorCallbackInfo->CallbackParameter);
         }
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
   else
   {
      /* Release the Mutex because we are finished with it.             */
      BTPS_ReleaseMutex(HRPManagerMutex);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Connected Asynchronous Event to the local callback.  It*/
   /* is the caller's responsibility to verify the message before       */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchConnected(HRPM_Connected_Message_t *Message)
{
   HRPM_Event_Data_t HRPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HRPMEventData.EventType                                        = hetHRPConnected;
   HRPMEventData.EventLength                                      = HRPM_CONNECTED_EVENT_DATA_SIZE;

   HRPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   HRPMEventData.EventData.ConnectedEventData.ConnectionType      = Message->ConnectionType;
   HRPMEventData.EventData.ConnectedEventData.ConnectedFlags      = Message->ConnectedFlags;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHRPEvent(&HRPMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Disconnected Asynchronous Event to the local callback. */
   /* It is the caller's responsibility to verify the message before    */
   /* calling this function.                                            */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchDisconnected(HRPM_Disconnected_Message_t *Message)
{
   HRPM_Event_Data_t HRPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HRPMEventData.EventType                                           = hetHRPDisconnected;
   HRPMEventData.EventLength                                         = HRPM_DISCONNECTED_EVENT_DATA_SIZE;

   HRPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   HRPMEventData.EventData.DisconnectedEventData.ConnectionType      = Message->ConnectionType;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHRPEvent(&HRPMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Heart Rate Measurement Asynchronous Event to the local */
   /* callback.  It is the caller's responsibility to verify the message*/
   /* before calling this function.                                     */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchHeartRateMeasurement(HRPM_Heart_Rate_Measurement_Message_t *Message)
{
   HRPM_Event_Data_t HRPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HRPMEventData.EventType                                                   = hetHRPHeartRateMeasurement;
   HRPMEventData.EventLength                                                 = HRPM_HEART_RATE_MEASUREMENT_EVENT_DATA_SIZE;

   HRPMEventData.EventData.HeartRateMeasurementEventData.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   HRPMEventData.EventData.HeartRateMeasurementEventData.MeasurementFlags    = Message->MeasurementFlags;
   HRPMEventData.EventData.HeartRateMeasurementEventData.HeartRate           = Message->HeartRate;
   HRPMEventData.EventData.HeartRateMeasurementEventData.EnergyExpended      = Message->EnergyExpended;
   HRPMEventData.EventData.HeartRateMeasurementEventData.NumberOfRRIntervals = Message->NumberOfRRIntervals;
   HRPMEventData.EventData.HeartRateMeasurementEventData.RRIntervals         = Message->RRIntervals;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHRPEvent(&HRPMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Get Body Sensor Location Response Event to the local   */
   /* callback.  It is the caller's responsibility to verify the message*/
   /* before calling this function.                                     */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchGetBodySensorLocationResponse(HRPM_Get_Body_Sensor_Location_Response_Message_t *Message)
{
   HRPM_Event_Data_t HRPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HRPMEventData.EventType                                                = hetHRPGetBodySensorLocationResponse;
   HRPMEventData.EventLength                                              = HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE;

   HRPMEventData.EventData.BodySensorLocationResponse.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   HRPMEventData.EventData.BodySensorLocationResponse.Status              = Message->Status;
   HRPMEventData.EventData.BodySensorLocationResponse.Location            = Message->Location;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHRPEvent(&HRPMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch a Reset Energy Expended Response Event to the local      */
   /* callback.  It is the caller's responsibility to verify the message*/
   /* before calling this function.                                     */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void DispatchResetEnergyExpendedResponse(HRPM_Reset_Energy_Expended_Response_Message_t *Message)
{
   HRPM_Event_Data_t HRPMEventData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format up the Event.                                              */
   HRPMEventData.EventType                                                 = hetHRPResetEnergyExpendedResponse;
   HRPMEventData.EventLength                                               = HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE;

   HRPMEventData.EventData.ResetEnergyExpendedResponse.RemoteDeviceAddress = Message->RemoteDeviceAddress;
   HRPMEventData.EventData.ResetEnergyExpendedResponse.Status              = Message->Status;

   /* Now that the event is formatted, dispatch it.                     */
   DispatchHRPEvent(&HRPMEventData);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function should be called with the Module Manager's */
   /*          Mutex held.  Upon exit from this function it will free   */
   /*          the Module Manager's Mutex.                              */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   Boolean_t ReleaseMutex = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HRPM_MESSAGE_FUNCTION_CONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_CONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               DispatchConnected((HRPM_Connected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_DISCONNECTED:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnected\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_DISCONNECTED_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Incoming Connection Request Event.                    */
               DispatchDisconnected((HRPM_Disconnected_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_HEART_RATE_MEASUREMENT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Heart Rate Measurement\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_HEART_RATE_MEASUREMENT_MESSAGE_SIZE(0))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connected Indication Event.                    */
               DispatchHeartRateMeasurement((HRPM_Heart_Rate_Measurement_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Body Sensor Location Response\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connection Status Event.                       */
               DispatchGetBodySensorLocationResponse((HRPM_Get_Body_Sensor_Location_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("New Alert Category Enabled Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HRPM_RESET_ENERGY_EXPENDED_RESPONSE_MESSAGE_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Device Connection Status Event.                       */
               DispatchResetEnergyExpendedResponse((HRPM_Reset_Energy_Expended_Response_Message_t *)Message);

               /* Flag that we do not need to release the Mutex (it has */
               /* already been released).                               */
               ReleaseMutex = FALSE;
            }
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

   /* If we need to Release the Mutex then we will do it at this time.  */
   if(ReleaseMutex)
      BTPS_ReleaseMutex(HRPManagerMutex);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HRP Manager Asynchronous Events.            */
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
         /* Attempt to wait for access to the HRP state information.    */
         if(BTPS_WaitMutex(HRPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Note we do not have to release the Mutex because         */
            /* ProcessReceivedMessage() is documented that it will be   */
            /* called with the Mutex being held and it will release the */
            /* Mutex when it is finished with it.                       */
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Server un-register asynchronous events.     */
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
         /* Attempt to wait for access to the HRP state information.    */
         if(BTPS_WaitMutex(HRPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Delete any event callbacks.                              */
            if(CollectorCallbackInfo)
            {
               BTPS_FreeMemory(CollectorCallbackInfo);

               CollectorCallbackInfo = NULL;
            }

            /* Release the Mutex because we are finished with it.       */
            BTPS_ReleaseMutex(HRPManagerMutex);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
               /* Module Manager Thread.                                */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HRPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HRP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

                  MSG_FreeReceivedMessageGroupHandlerMessage(Message);
               }
            }
            else
            {
               /* Dispatch to the main handler that the server has      */
               /* un-registered.                                        */
               if(Message->MessageHeader.MessageFunction == BTPM_MESSAGE_FUNCTION_CLIENT_REGISTRATION)
               {
                  if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= BTPM_CLIENT_REGISTRATION_MESSAGE_SIZE) && (!(((BTPM_Client_Registration_Message_t *)Message)->Registered)))
                  {
                     if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_MSG, (void *)(((BTPM_Client_Registration_Message_t *)Message)->AddressID)))
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HRP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HRP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HRP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
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

         /* First, let's create a Mutex to guard access to this module. */
         if((HRPManagerMutex = BTPS_CreateMutex(FALSE)) != NULL)
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process HRP Manager messages.                         */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER, HRPManagerGroupHandler, NULL))
            {
               /* Initialize the HRP Manager Implementation Module (this*/
               /* is the module that is responsible for implementing the*/
               /* HRP Manager functionality - this module is just the   */
               /* framework shell).                                     */
               if((Result = _HRPM_Initialize()) == 0)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Success\n"));
                  Initialized = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result < 0)
         {
            if(HRPManagerMutex)
               BTPS_CloseMutex(HRPManagerMutex);

            /* Flag that none of the resources are allocated.           */
            HRPManagerMutex = NULL;

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

         /* Wait for access to the Mutex that guards access to this     */
         /* module.                                                     */
         if(BTPS_WaitMutex(HRPManagerMutex, BTPS_INFINITE_WAIT))
         {
            /* Un-Register for HRP Events.                              */
            if(CollectorCallbackInfo)
            {
               _HRPM_Un_Register_Collector_Events();
               BTPS_FreeMemory(CollectorCallbackInfo);

               CollectorCallbackInfo = NULL;
            }

            /* Make sure we inform the HRP Manager Implementation that  */
            /* we are shutting down.                                    */
            _HRPM_Cleanup();

            BTPS_CloseMutex(HRPManagerMutex);

            /* Flag that the resources are no longer allocated.         */
            HRPManagerMutex = NULL;

            /* Flag that this module is no longer initialized.          */
            Initialized     = FALSE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Initialized));
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

   /* First, check to make sure the HRP Manager has been initialized.   */
   if(Initialized)
   {
      /* Attempt to wait for access to the HRP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(HRPManagerMutex, BTPS_INFINITE_WAIT))
      {
         if(CollectorCallbackInfo == NULL)
         {
            if((CollectorCallbackInfo = BTPS_AllocateMemory(sizeof(HRP_Collector_Callback_Info_t))) != NULL)
            {
               /* Go ahead and register with the HRP Manager Server.    */
               if((ret_val = _HRPM_Register_Collector_Events()) > 0)
               {
                  CollectorCallbackInfo->EventCallback     = CallbackFunction;
                  CollectorCallbackInfo->CallbackParameter = CallbackParameter;

                  ret_val                                  = HRPM_COLLECTOR_CALLBACK_ID;
               }
               else
               {
                  BTPS_FreeMemory(CollectorCallbackInfo);

                  CollectorCallbackInfo = NULL;
                  ret_val               = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_ALREADY_REGISTERED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_HEART_RATE_CALLBACK_ALREADY_REGISTERED;

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HRPManagerMutex);
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX;
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

   /* First, check to make sure the HRP Manager has been initialized, a */
   /* callback has been registered, and the parameter appears correct.  */
   if((Initialized) && (CollectorCallbackInfo) && (CollectorCallbackID == HRPM_COLLECTOR_CALLBACK_ID))
   {
      /* Attempt to wait for access to the HRP Manager State            */
      /* information.                                                   */
      if(BTPS_WaitMutex(HRPManagerMutex, BTPS_INFINITE_WAIT))
      {
         if(_HRPM_Un_Register_Collector_Events() == 0)
         {
            BTPS_FreeMemory(CollectorCallbackInfo);

            CollectorCallbackInfo = NULL;
         }

         /* Release the Mutex because we are finished with it.          */
         BTPS_ReleaseMutex(HRPManagerMutex);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEART_RATE  | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the module has been initialized and a   */
   /* callback has been registered.                                     */
   if((Initialized) && (CollectorCallbackID == HRPM_COLLECTOR_CALLBACK_ID))
   {
      /* Check for a registered callback.                               */
      if(CollectorCallbackInfo)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* set the alert.                                              */
         ret_val = _HRPM_Get_Body_Sensor_Location(&RemoteSensor);
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

   /* First, check to make sure the module has been initialized and a   */
   /* callback has been registered.                                     */
   if((Initialized) && (CollectorCallbackID == HRPM_COLLECTOR_CALLBACK_ID))
   {
      /* Check for a registered callback.                               */
      if(CollectorCallbackInfo)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* set the alert.                                              */
         ret_val = _HRPM_Reset_Energy_Expended(&RemoteSensor);
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

/*****< CPPMDEVM.c >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMDEVM - Cycling Power Collector Device Management Processing           */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/

#include "SS1BTCPS.h"
#include "SS1BTPM.h"
#include "CPPMType.h"
#include "CPPMAPI.h"
#include "CPPMUTIL.h"
#include "BTPMCPPM.h"
#include "CPPMMSG.h"

#define CPPM_COLLECTOR_LE_CONFIGURATION_FILE_SECTION_NAME  "CPPM-Collector"
#define CPPM_LE_KEY_NAME_SENSOR                            "CPS-%02X%02X%02X%02X%02X%02X"
#define CPPM_LE_KEY_LENGTH                                 (4 + (BD_ADDR_SIZE * 2) + 1)

   /*********************************************************************/
   /* Called by CPPM_DeviceManagerHandlerFunction:                      */
   /*********************************************************************/
static void ProcessRemoteDevicePropertiesChangedEventCPPM(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

   /*********************************************************************/
   /* Called by a DEVM event handler:                                   */
   /*********************************************************************/
static void ProcessSensorConnectionCPPM(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessSensorDisconnectionCPPM(BD_ADDR_t *BluetoothAddress);
static void RemoveSensorReferenceCPPM(BD_ADDR_t *BluetoothAddress);
static void AddSensorReferenceCPPM(BD_ADDR_t *BluetoothAddress);
static Boolean_t SensorReferenceFoundCPPM(BD_ADDR_t *BluetoothAddress);

   /*********************************************************************/
   /* Called by ProcessSensorConnectionCPPM:                            */
   /*********************************************************************/
static Boolean_t SensorServicePresentCPPM(BD_ADDR_t BD_ADDR);
static Device_Entry_t *SensorDiscoveryCPPM(BD_ADDR_t BluetoothAddress);
static void UpdateSensorEntryCPPM(Device_Entry_t *DeviceEntry);

   /*********************************************************************/
   /* Called by UpdateSensorEntryCPPM:                                  */
   /*********************************************************************/
static int ReadSensorAttributeCPPM(Transaction_Type_t TransactionType, BD_ADDR_t *Sensor, Word_t Handle, Transaction_Entry_t **TransactionEntryList);

   /*********************************************************************/
   /* Called by SensorDiscoveryCPPM:                                    */
   /*********************************************************************/
static Boolean_t ParseServiceDataCPPM(BD_ADDR_t BD_ADDR, DEVM_Parsed_Services_Data_t *ParsedGATTData);
static Device_Entry_t *ParseSensorDataCPPM(BD_ADDR_t *BluetoothAddress, DEVM_Parsed_Services_Data_t  *ParsedGATTData);

   /*********************************************************************/
   /* Called by ParseSensorDataCPPM:                                    */
   /*********************************************************************/
static void ParseSensorCharacteristicsCPPM(Instance_Entry_t *InstanceEntry, GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData);

   /*********************************************************************/
   /* Format and make server side callbacks or send client messages:    */
   /*********************************************************************/
static void ConnectedEventCPPM(Callback_Entry_t *CallbackEntryList, BD_ADDR_t BluetoothAddress, unsigned int NumberOfInstances);
static void DisconnectedEventCPPM(Callback_Entry_t *CallbackEntryList, BD_ADDR_t BluetoothAddress);


   /* CPPM_DeviceManagerHandlerFunction is included in the MODC         */
   /* ModuleHandlerList array. It is the DEVM callback function for the */
   /* module.                                                           */
void BTPSAPI CPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(Initialized)
   {
      /* Wait for the lock.                                             */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               FreeDeviceEntryList(&(DeviceEntryList));

               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEventCPPM(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Remove the sensors reference from the config file.    */
               if(SensorReferenceFoundCPPM(&(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress)))
                  RemoveSensorReferenceCPPM(&(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress));

               ProcessSensorDisconnectionCPPM(&(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress));

               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by CPPM_DeviceManagerHandlerFunction,                      */
   /* ProcessRemoteDevicePropertiesChangedEventCPPM checks for          */
   /* connection state changes, pairing state changes and address       */
   /* updates. A sensor connected event can only be triggered when      */
   /* both an LE connection is established and the services are known.  */
   /* When the remote sensor is both paired and the connection is       */
   /* encrypted then a reference to that sensor is saved to a file. If  */
   /* that reference is found when a future connection to that sensor   */
   /* is established the the attributes will be read automatically. See */
   /* AddSensorReferenceCPPM and UpdateSensorEntryCPPM. If the          */
   /* resolvable address changes and then the device entry is updated.  */
static void ProcessRemoteDevicePropertiesChangedEventCPPM(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long   RequiredConnectionFlags;
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

      /* Check for a LE connection state change.                        */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         /* Require a connection and known services.                    */
         RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN);

         /* Check for the required flags.                               */
         if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags)
         {
            ProcessSensorConnectionCPPM(RemoteDeviceProperties);
         }
         else
         {
            /* Remove the sensors reference from the config file.       */
            if((!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)) && (SensorReferenceFoundCPPM(&(RemoteDeviceProperties->BD_ADDR))))
               RemoveSensorReferenceCPPM(&(RemoteDeviceProperties->BD_ADDR));

            ProcessSensorDisconnectionCPPM(&(RemoteDeviceProperties->BD_ADDR));
         }
      }

      /* Process a LE Pairing State Change Event.                       */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE))
      {
         /* Require currently paired or encrypted.                      */
         RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED);

         /* Check for the required flags.                               */
         if(((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags) && (!(SensorReferenceFoundCPPM(&(RemoteDeviceProperties->BD_ADDR)))))
         {
            AddSensorReferenceCPPM(&(RemoteDeviceProperties->BD_ADDR));
         }
         else
         {
            if((!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)) && (SensorReferenceFoundCPPM(&(RemoteDeviceProperties->BD_ADDR))))
               RemoveSensorReferenceCPPM(&(RemoteDeviceProperties->BD_ADDR));
         }
      }

      /* Process the Address Updated event.                             */
      if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
      {
         if(SensorReferenceFoundCPPM(&(RemoteDeviceProperties->PriorResolvableBD_ADDR)))
         {
            AddSensorReferenceCPPM(&(RemoteDeviceProperties->BD_ADDR));
            RemoveSensorReferenceCPPM(&(RemoteDeviceProperties->PriorResolvableBD_ADDR));
         }

         if((DeviceEntry = SearchDeviceEntry(&(DeviceEntryList), &(RemoteDeviceProperties->PriorResolvableBD_ADDR))) != NULL)
            DeviceEntry->BluetoothAddress = RemoteDeviceProperties->BD_ADDR;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessRemoteDevicePropertiesChangedEventCPPM,          */
   /* ProcessSensorConnectionCPPM is run when both an LE connection has */
   /* been established and the services are known. It searches for      */
   /* a sensor instance in the services and if one is found, adds a     */
   /* device entry to the list. If a saved reference is found, then the */
   /* the attributes of the instance are read.                          */
static void ProcessSensorConnectionCPPM(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(RemoteDeviceProperties)
   {
      /* Check for a Cycling Power service instance.                    */
      if(SensorServicePresentCPPM(RemoteDeviceProperties->BD_ADDR))
      {
         DeviceEntry = NULL;

         /* Load the Cycling Power services from the service table.     */
         if((DeviceEntry = SensorDiscoveryCPPM(RemoteDeviceProperties->BD_ADDR)) != NULL)
         {
            if(AddDeviceEntryActual(&(DeviceEntryList), DeviceEntry))
            {
               ConnectedEventCPPM(CallbackEntryList, DeviceEntry->BluetoothAddress, CalculateNumberOfInstances(DeviceEntry->InstanceEntryList));

               if(SensorReferenceFoundCPPM(&(DeviceEntry->BluetoothAddress)))
                  UpdateSensorEntryCPPM(DeviceEntry);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called when either the connection is lost, the services are no    */
   /* longer known or the device is deleted,                            */
   /* ProcessSensorDisconnectionCPPM deletes the device entry and calls */
   /* the disconnect event routine.                                     */
static void ProcessSensorDisconnectionCPPM(BD_ADDR_t *BluetoothAddress)
{
   Device_Entry_t *DeviceEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(BluetoothAddress)
   {
      if((DeviceEntry = DeleteDeviceEntry(&(DeviceEntryList), BluetoothAddress)) != NULL)
      {
         DisconnectedEventCPPM(CallbackEntryList, DeviceEntry->BluetoothAddress);
         FreeDeviceEntryMemory(DeviceEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessSensorConnectionCPPM, SensorServicePresentCPPM   */
   /* determines if an instance of the cycling power service is present */
   /* in the service data through a call to                             */
   /* DEVM_QueryRemoteDeviceServiceSupported.                           */
static Boolean_t SensorServicePresentCPPM(BD_ADDR_t BD_ADDR)
{
   UUID_16_t        UUID;
   Boolean_t        Result;;
   SDP_UUID_Entry_t ServiceUUID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ServiceUUID.SDP_Data_Element_Type = deUUID_16;
   CPS_ASSIGN_SERVICE_UUID_16(&UUID);
   CONVERT_BLUETOOTH_UUID_16_TO_SDP_UUID_16(ServiceUUID.UUID_Value.UUID_16, UUID);

   Result = FALSE;

   if(DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR, ServiceUUID) > 0)
   {
      Result = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* Called by ProcessSensorConnectionCPPM, SensorDiscoveryCPPM parses */
   /* the service data and returns a device entry if a sensor is found. */
static Device_Entry_t *SensorDiscoveryCPPM(BD_ADDR_t BluetoothAddress)
{
   Device_Entry_t              *DeviceEntry;
   DEVM_Parsed_Services_Data_t  ParsedGATTData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   DeviceEntry = NULL;

   if(!COMPARE_NULL_BD_ADDR(BluetoothAddress))
   {
      /* Parsed the GATT data.                                          */
      if(ParseServiceDataCPPM(BluetoothAddress, &ParsedGATTData))
      {
         DeviceEntry = ParseSensorDataCPPM(&(BluetoothAddress), &ParsedGATTData);

         /* Free the parsed service data.                               */
         DEVM_FreeParsedServicesData(&ParsedGATTData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(DeviceEntry);
}

   /* Called by SensorDiscoveryCPPM, ParseServiceDataCPPM queries the   */
   /* service data and parses it. If successful it will return TRUE,    */
   /* with the parsed service data in the provided ParsedGATTData       */
   /* parameter.                                                        */
static Boolean_t ParseServiceDataCPPM(BD_ADDR_t BD_ADDR, DEVM_Parsed_Services_Data_t *ParsedGATTData)
{
   Byte_t       *ServiceData;
   Boolean_t     Result;
   int           ReturnedServiceSize;
   unsigned int  TotalServiceSize;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   Result = FALSE;

   if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ParsedGATTData))
   {
      /* Get the service data size.                                     */
      if(!DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &TotalServiceSize))
      {
         /* Allocate a service data buffer.                             */
         if((ServiceData = BTPS_AllocateMemory(TotalServiceSize)) != NULL)
         {
            /* Get the data.                                            */
            if((ReturnedServiceSize = DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, TotalServiceSize, ServiceData, NULL)) >= 0)
            {
               /* Parse the raw service data.                           */
               if(!DEVM_ConvertRawServicesStreamToParsedServicesData((unsigned int)ReturnedServiceSize, ServiceData, ParsedGATTData))
               {
                  /* Return success to the caller.                      */
                  Result = TRUE;
               }
            }

            /* Free the memory.                                         */
            BTPS_FreeMemory(ServiceData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* Called by SensorDiscoveryCPPM, ParseSensorDataCPPM allocates and  */
   /* populates a device entry and its instance entries from the parsed */
   /* service data in ParsedGATTData. The device entry is returned.     */
static Device_Entry_t *ParseSensorDataCPPM(BD_ADDR_t *BluetoothAddress, DEVM_Parsed_Services_Data_t  *ParsedGATTData)
{
   unsigned int                              IndexService;
   GATT_UUID_t                               UUID;
   unsigned int                              NextInstanceID;
   Device_Entry_t                           *DeviceEntry;
   Instance_Entry_t                         *InstanceEntry;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   DeviceEntry = NULL;

   if(ParsedGATTData)
   {
      NextInstanceID = 0;

      /* Iterate the services on the remote device.                     */
      for(IndexService = 0; IndexService < ParsedGATTData->NumberServices; IndexService++)
      {
         UUID = ParsedGATTData->GATTServiceDiscoveryIndicationData[IndexService].ServiceInformation.UUID;

         /* Check for CPS.                                              */
         if((UUID.UUID_Type == guUUID_16) && (CPS_COMPARE_SERVICE_UUID_TO_UUID_16(UUID.UUID.UUID_16)))
         {
            /* A Cycling Power instance was found.                      */
            if(!DeviceEntry)
            {
               if((DeviceEntry = (Device_Entry_t *)BTPS_AllocateMemory(sizeof(Device_Entry_t))) != NULL)
               {
                  /* Initalize the device entry.                        */
                  BTPS_MemInitialize(DeviceEntry, 0, sizeof(Device_Entry_t));
                  DeviceEntry->BluetoothAddress = *BluetoothAddress;
               }
            }

            if(DeviceEntry)
            {
               /* Create an instance entry.                             */
               if((InstanceEntry = (Instance_Entry_t *)BTPS_AllocateMemory(sizeof(Instance_Entry_t))) != NULL)
               {
                  /* Initalize the instance entry.                      */
                  BTPS_MemInitialize(InstanceEntry, 0, sizeof(Instance_Entry_t));

                  GATTServiceDiscoveryIndicationData = &(ParsedGATTData->GATTServiceDiscoveryIndicationData[IndexService]);

                  ParseSensorCharacteristicsCPPM(InstanceEntry, GATTServiceDiscoveryIndicationData);

                  /* Set the Instance ID.                               */
                  InstanceEntry->InstanceID = ++NextInstanceID;

                  AddInstanceEntryActual(&(DeviceEntry->InstanceEntryList), InstanceEntry);
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(DeviceEntry);
}

   /* Called by ParseSensorDataCPPM, ParseSensorCharacteristicsCPPM     */
   /* loads the sensor attribute handle values into the provided        */
   /* instance entry. The function looks for the measurement, vector,   */
   /* features, location and control point characteristics, the client  */
   /* configuration descriptors for measurements, vectors and control   */
   /* points and the server configuration descriptor for measurements.  */
static void ParseSensorCharacteristicsCPPM(Instance_Entry_t *InstanceEntry, GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData)
{
   int                                IndexCharacteristic;
   int                                IndexDescriptor;
   GATT_UUID_t                        UUID;
   GATT_Characteristic_Information_t *CharacteristicInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(GATTServiceDiscoveryIndicationData)
   {
      /* Iterate through the CPS characteristics.                       */
      for(IndexCharacteristic = 0; IndexCharacteristic < GATTServiceDiscoveryIndicationData->NumberOfCharacteristics; IndexCharacteristic++)
      {
         CharacteristicInformation = &(GATTServiceDiscoveryIndicationData->CharacteristicInformationList[IndexCharacteristic]);
         UUID                      = CharacteristicInformation->Characteristic_UUID;

         if(UUID.UUID_Type == guUUID_16)
         {
            /* Check for the Measurement UUID.                          */
            if(CPS_COMPARE_MEASUREMENT_UUID_TO_UUID_16(UUID.UUID.UUID_16))
            {
               InstanceEntry->ServiceHandles.CP_Measurement = CharacteristicInformation->Characteristic_Handle;

               for(IndexDescriptor = 0; IndexDescriptor < CharacteristicInformation->NumberOfDescriptors; IndexDescriptor++)
               {
                  UUID = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_UUID;

                  if(UUID.UUID_Type == guUUID_16)
                  {
                     /* Check for the Measurement CCD.                  */
                     if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                     {
                        InstanceEntry->ServiceHandles.CP_Measurement_Client_Configuration = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_Handle;
                     }
                     else
                     {
                        /* Check for the Measurement SCD.               */
                        if(GATT_COMPARE_SERVER_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                        {
                           InstanceEntry->ServiceHandles.CP_Measurement_Server_Configuration = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_Handle;
                        }
                     }
                  }
               }
            }
            else
            {
               /* Check for the supported features UUID.                */
               if(CPS_COMPARE_FEATURE_UUID_TO_UUID_16(UUID.UUID.UUID_16))
               {
                  InstanceEntry->ServiceHandles.CP_Feature = CharacteristicInformation->Characteristic_Handle;
               }
               else
               {
                  /* Check for the mount location UUID.                 */
                  if(CPS_COMPARE_SENSOR_LOCATION_UUID_TO_UUID_16(UUID.UUID.UUID_16))
                  {
                     InstanceEntry->ServiceHandles.Sensor_Location = CharacteristicInformation->Characteristic_Handle;
                  }
                  else
                  {
                     /* Check for the vector UUID.                      */
                     if(CPS_COMPARE_VECTOR_UUID_TO_UUID_16(UUID.UUID.UUID_16))
                     {
                        InstanceEntry->ServiceHandles.CP_Vector = CharacteristicInformation->Characteristic_Handle;

                        for(IndexDescriptor = 0; IndexDescriptor < CharacteristicInformation->NumberOfDescriptors; IndexDescriptor++)
                        {
                           UUID = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_UUID;

                           if(UUID.UUID_Type == guUUID_16)
                           {
                              /* Check for the Vector CCD.              */
                              if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                              {
                                 InstanceEntry->ServiceHandles.CP_Vector_Client_Configuration = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_Handle;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Check for the control point UUID.            */
                        if(CPS_COMPARE_CONTROL_POINT_UUID_TO_UUID_16(UUID.UUID.UUID_16))
                        {
                           InstanceEntry->ServiceHandles.CP_Control_Point = CharacteristicInformation->Characteristic_Handle;

                           for(IndexDescriptor = 0; IndexDescriptor < CharacteristicInformation->NumberOfDescriptors; IndexDescriptor++)
                           {
                              UUID = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_UUID;

                              if(UUID.UUID_Type == guUUID_16)
                              {
                                 /* Check for the Control Point CCD.    */
                                 if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(UUID.UUID.UUID_16))
                                 {
                                    InstanceEntry->ServiceHandles.CP_Control_Point_Client_Configuration = CharacteristicInformation->DescriptorList[IndexDescriptor].Characteristic_Descriptor_Handle;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessSensorConnectionCPPM, ConnectedEventCPPM formats */
   /* connected event data and sends an IPC messages to every           */
   /* registered PM client or makes a callback for a server side        */
   /* application.                                                      */
static void ConnectedEventCPPM(Callback_Entry_t *CallbackEntryList, BD_ADDR_t BluetoothAddress, unsigned int NumberOfInstances)
{
   Callback_Entry_t         *CallbackEntry;
   CPPM_Event_Data_t         EventData;
   CPPM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event data for server side callbacks.                  */
   EventData.EventType                                        = cetConnectedCPP;
   EventData.EventLength                                      = CPPM_CONNECTED_EVENT_DATA_SIZE;
   EventData.EventData.ConnectedEventData.RemoteDeviceAddress = BluetoothAddress;

   EventData.EventData.ConnectedEventData.ConnectionType      = cctCollector;
   EventData.EventData.ConnectedEventData.ConnectedFlags      = 0;
   EventData.EventData.ConnectedEventData.NumberOfInstances   = NumberOfInstances;

   /* Format the message that will be dispatched to PM clients.         */
   Message.MessageHeader.MessageGroup                         = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
   Message.MessageHeader.MessageFunction                      = CPPM_MESSAGE_FUNCTION_CONNECTED;
   Message.MessageHeader.MessageLength                        = (CPPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                = BluetoothAddress;
   Message.ConnectionType                                     = cctCollector;
   Message.ConnectedFlags                                     = 0;
   Message.NumberOfInstances                                  = NumberOfInstances;

   /* Iterate through each callback entry.                              */
   for(CallbackEntry=CallbackEntryList;CallbackEntry;CallbackEntry=CallbackEntry->NextCallbackEntry)
   {
      /* Check the callback type.                                       */
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
      {
         /* Populate the Callback ID.                                   */
         EventData.EventCallbackID = CallbackEntry->CallbackID;

         /* Dispatch the event to the registered callback.              */
         ServerEventCPPM(&EventData);
      }
      else
      {
         if(CallbackEntry->AddressID)
         {
            /* Populate the Address, Message, and Callback ID.          */
            Message.MessageHeader.MessageID = MSG_GetNextMessageID();
            Message.MessageHeader.AddressID = CallbackEntry->AddressID;
            Message.CallbackID              = CallbackEntry->CallbackID;

            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessSensorDisconnectionCPPM, DisconnectedEventCPPM   */
   /* formats disconnected event data and sends an IPC messages to      */
   /* every registered PM client or makes a callback for a server side  */
   /* application.                                                      */
static void DisconnectedEventCPPM(Callback_Entry_t *CallbackEntryList, BD_ADDR_t BluetoothAddress)
{
   Callback_Entry_t         *CallbackEntry;
   CPPM_Event_Data_t         EventData;
   CPPM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the event data for server side callbacks.                  */
   EventData.EventType                                           = cetDisconnectedCPP;
   EventData.EventLength                                         = CPPM_DISCONNECTED_EVENT_DATA_SIZE;
   EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = BluetoothAddress;

   EventData.EventData.DisconnectedEventData.ConnectionType      = cctCollector;

   /* Format the message that will be dispatched to PM clients.         */
   Message.MessageHeader.MessageGroup                            = BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER;
   Message.MessageHeader.MessageFunction                         = CPPM_MESSAGE_FUNCTION_DISCONNECTED;
   Message.MessageHeader.MessageLength                           = (CPPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);
   Message.RemoteDeviceAddress                                   = BluetoothAddress;
   Message.ConnectionType                                        = cctCollector;

   /* Iterate through each callback entry.                              */
   for(CallbackEntry=CallbackEntryList;CallbackEntry;CallbackEntry=CallbackEntry->NextCallbackEntry)
   {
      /* Check the callback type.                                       */
      if(CallbackEntry->AddressID == MSG_GetServerAddressID())
      {
         /* Populate the Callback ID.                                   */
         EventData.EventCallbackID = CallbackEntry->CallbackID;

         /* Dispatch the event to the registered callback.              */
         ServerEventCPPM(&EventData);
      }
      else
      {
         if(CallbackEntry->AddressID)
         {
            /* Populate the Address, Message, and Callback ID.          */
            Message.MessageHeader.MessageID = MSG_GetNextMessageID();
            Message.MessageHeader.AddressID = CallbackEntry->AddressID;
            Message.CallbackID              = CallbackEntry->CallbackID;

            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* SensorReferenceFoundCPPM checks the configuration file for a      */
   /* reference to a sensor with the specified bluetooth address.       */
static Boolean_t SensorReferenceFoundCPPM(BD_ADDR_t *BluetoothAddress)
{
   char      KeyName[CPPM_LE_KEY_LENGTH];
   Boolean_t Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(BluetoothAddress)
   {
      /* Build the Key Name.                                            */
      snprintf(KeyName, CPPM_LE_KEY_LENGTH, CPPM_LE_KEY_NAME_SENSOR, BluetoothAddress->BD_ADDR5, BluetoothAddress->BD_ADDR4, BluetoothAddress->BD_ADDR3, BluetoothAddress->BD_ADDR2, BluetoothAddress->BD_ADDR1, BluetoothAddress->BD_ADDR0);

      if((SET_ReadInteger(CPPM_COLLECTOR_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME)) > 0)
         Result = TRUE;
      else
         Result = FALSE;
   }
   else
      Result = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return (Result);
}

   /* AddSensorReferenceCPPM records a reference to a sensor with the   */
   /* specified bluetooth address. Paired devices are recorded in the   */
   /* configuration file. Upon reconnection the attributes will be read */
   /* automatically.                                                    */
static void AddSensorReferenceCPPM(BD_ADDR_t *BluetoothAddress)
{
   char KeyName[CPPM_LE_KEY_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(BluetoothAddress)
   {
      /* Build the Key Name.                                            */
      snprintf(KeyName, CPPM_LE_KEY_LENGTH, CPPM_LE_KEY_NAME_SENSOR, BluetoothAddress->BD_ADDR5, BluetoothAddress->BD_ADDR4, BluetoothAddress->BD_ADDR3, BluetoothAddress->BD_ADDR2, BluetoothAddress->BD_ADDR1, BluetoothAddress->BD_ADDR0);

      if(!(SET_WriteInteger(CPPM_COLLECTOR_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, 1, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME)))
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("adding sensor reference failed\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* RemoveSensorReferenceCPPM deletes a sensor reference from the     */
   /* configuration file. The reference is removed if services become   */
   /* unknown, the device is unpaired or if the resolvable address      */
   /* changes.                                                          */
static void RemoveSensorReferenceCPPM(BD_ADDR_t *BluetoothAddress)
{
   char KeyName[CPPM_LE_KEY_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(BluetoothAddress)
   {
      /* Build the Key Name.                                            */
      snprintf(KeyName, CPPM_LE_KEY_LENGTH, CPPM_LE_KEY_NAME_SENSOR, BluetoothAddress->BD_ADDR5, BluetoothAddress->BD_ADDR4, BluetoothAddress->BD_ADDR3, BluetoothAddress->BD_ADDR2, BluetoothAddress->BD_ADDR1, BluetoothAddress->BD_ADDR0);

      if(!(SET_WriteBinaryData(CPPM_COLLECTOR_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME)))
         DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("removing sensor reference failed\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* Called by ProcessSensorConnectionCPPM, UpdateSensorEntryCPPM      */
   /* reads the attributes of all sensor instances in the specified     */
   /* device entry. When a sensor is connected, the configuration file  */
   /* is checked for a reference to that sensor. If found, the          */
   /* attribute values are read so that sensor instance data is updated */
   /* automatically .                                                   */
static void UpdateSensorEntryCPPM(Device_Entry_t *DeviceEntry)
{
   int               Result;
   BD_ADDR_t        *BluetoothAddress;
   Instance_Entry_t *InstanceEntry;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(DeviceEntry)
   {
      /* Search for the callback.                                       */
      if((CallbackEntry = SearchCallbackEntry(&(CallbackEntryList), 0)) != NULL)
      {
         BluetoothAddress = &(DeviceEntry->BluetoothAddress);

         for(InstanceEntry = DeviceEntry->InstanceEntryList; InstanceEntry;)
         {
            Result = 0;

            if((InstanceEntry->ServiceHandles.Sensor_Location) && (Result = ReadSensorAttributeCPPM(ttReadCyclingPowerSensorLocation, BluetoothAddress, InstanceEntry->ServiceHandles.Sensor_Location, &(CallbackEntry->TransactionEntryList))) < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("ttReadCyclingPowerSensorLocation Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.Sensor_Location));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("ttReadCyclingPowerSensorLocation Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.Sensor_Location));
            }

            Result = 0;

            if((InstanceEntry->ServiceHandles.CP_Feature) && (Result = ReadSensorAttributeCPPM(ttReadCyclingPowerSensorFeatures, BluetoothAddress, InstanceEntry->ServiceHandles.CP_Feature, &(CallbackEntry->TransactionEntryList))) < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("ttReadCyclingPowerSensorFeatures Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Feature));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("ttReadCyclingPowerSensorFeatures Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Feature));
            }

            Result = 0;

            if((InstanceEntry->ServiceHandles.CP_Measurement_Client_Configuration) && (Result = ReadSensorAttributeCPPM(ttReadCyclingPowerMeasurementClientDescriptor, BluetoothAddress, InstanceEntry->ServiceHandles.CP_Measurement_Client_Configuration, &(CallbackEntry->TransactionEntryList))) < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("ttReadCyclingPowerMeasurementClientDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Measurement_Client_Configuration));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("ttReadCyclingPowerMeasurementClientDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Measurement_Client_Configuration));
            }

            Result = 0;

            if((InstanceEntry->ServiceHandles.CP_Vector_Client_Configuration) && (Result = ReadSensorAttributeCPPM(ttReadCyclingPowerVectorClientDescriptor, BluetoothAddress, InstanceEntry->ServiceHandles.CP_Vector_Client_Configuration, &(CallbackEntry->TransactionEntryList))) < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("ttReadCyclingPowerVectorClientDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Vector_Client_Configuration));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("ttReadCyclingPowerVectorClientDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Vector_Client_Configuration));
            }

            Result = 0;

            if((InstanceEntry->ServiceHandles.CP_Control_Point_Client_Configuration) && (Result = ReadSensorAttributeCPPM(ttReadCyclingPowerControlPointClientDescriptor, BluetoothAddress, InstanceEntry->ServiceHandles.CP_Control_Point_Client_Configuration, &(CallbackEntry->TransactionEntryList))) < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("ttReadCyclingPowerControlPointClientDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Control_Point_Client_Configuration));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("ttReadCyclingPowerControlPointClientDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Control_Point_Client_Configuration));
            }

            Result = 0;

            if((InstanceEntry->ServiceHandles.CP_Measurement_Server_Configuration) && (Result = ReadSensorAttributeCPPM(ttReadCyclingPowerMeasurementServerDescriptor, BluetoothAddress, InstanceEntry->ServiceHandles.CP_Measurement_Server_Configuration, &(CallbackEntry->TransactionEntryList))) < 0)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_WARNING), ("ttReadCyclingPowerMeasurementServerDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Measurement_Server_Configuration));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_VERBOSE), ("ttReadCyclingPowerMeasurementServerDescriptor Result: %d Handle: %u\n", Result, InstanceEntry->ServiceHandles.CP_Measurement_Server_Configuration));
            }

            InstanceEntry = InstanceEntry->NextInstanceEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}
   /* Called by UpdateSensorEntryCPPM, ReadSensorAttributeCPPM makes    */
   /* the GATM read of the attribute with the specified handle on the   */
   /* specified device. The transaction is added to the devices         */
   /* transaction list.                                                 */
static int ReadSensorAttributeCPPM(Transaction_Type_t TransactionType, BD_ADDR_t *Sensor, Word_t Handle, Transaction_Entry_t **TransactionEntryList)
{
   int                 Result;
   Transaction_Entry_t TransactionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Configure the Transaction Entry.                                  */
   BTPS_MemInitialize(&TransactionEntry, 0, sizeof(TransactionEntry));

   TransactionEntry.TransactionType = TransactionType;

   /* Perform the read.                                                 */
   if((Result = GATM_ReadValue(GATMCallbackID, *Sensor, Handle, 0, TRUE)) > 0)
   {
      TransactionEntry.TransactionID = (unsigned int)Result;

      /* Add the Transaction Entry.                                     */
      AddTransactionEntry(TransactionEntryList, &TransactionEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_CYCLING_POWER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return (Result);
}


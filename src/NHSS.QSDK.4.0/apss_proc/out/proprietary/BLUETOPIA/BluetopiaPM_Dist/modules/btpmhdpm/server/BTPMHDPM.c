/*****< btpmhdpm.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDPM - Health Device Profile Manager for Stonestreet One Bluetooth    */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHDPM.h"            /* BTPM HDP Manager Prototypes/Constants.    */
#include "HDPMAPI.h"             /* HDP Manager Prototypes/Constants.         */
#include "HDPMMSG.h"             /* BTPM HDP Manager Message Formats.         */
#include "HDPMGR.h"              /* HDP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

typedef struct _tagCallback_Registraion_t
{
   unsigned int           ClientID;
   HDPM_Event_Callback_t  CallbackFunction;
   void                  *CallbackParameter;
} Callback_Registration_t;

typedef struct _tagEndpoint_Entry_t
{
   /* XXX                                                               */
   /* If supporting multiple endpoints of the same type+role between    */
   /* clients, to multiplex based on Control Connection ownership, maybe*/
   /* add a new EndpointID field and supply this new ID to clients to   */
   /* distinguish between registrations.                                */
   Byte_t                       MDEP_ID;
   Word_t                       MDEP_DataType;
   HDP_Device_Role_t            MDEP_Role;
   Callback_Registration_t      EventCallback;
   struct _tagEndpoint_Entry_t *NextEntry;
} Endpoint_Entry_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking connections.                          */
typedef enum _tagConnection_State_t
{
   csIdle,
   csAuthorizing,
   csConnectingDevice,
   csConnecting,
   csConnected
} Connection_State_t;

typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                      BD_ADDR;
   DWord_t                        Instance;
   unsigned int                   MCLID;
   Boolean_t                      Server;
   Connection_State_t             ConnectionState;
   unsigned int                   ConnectionStatus;
   Event_t                        ConnectionEvent;
   Callback_Registration_t        EventCallback;
   struct _tagConnection_Entry_t *NextEntry;
} Connection_Entry_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking data channel connections.             */
typedef enum
{
   /* Incoming connection: Data Channel Request received.               */
   /* Outgoing connection: Data Channel Request sent.                   */
   dcsAuthorizing,
   /* Incoming connection: Data Channel Request response sent.          */
   /* Outgoing connection: Data Channel Request response received.      */
   dcsConnecting,
   /* Incoming connection: Data PSM connected.                          */
   /* Incoming connection: Data PSM connected.                          */
   dcsConnected
} Data_Channel_State_t;

typedef struct _tagData_Channel_Entry_t
{
   unsigned int                     MCLID;
   unsigned int                     DataLinkID;
   Byte_t                           MDEP_ID;
   HDP_Channel_Mode_t               ChannelMode;
   HDP_Device_Role_t                LocalRole;
   Data_Channel_State_t             ConnectionState;
   unsigned int                     ConnectionStatus;
   Event_t                          ConnectionEvent;
   Callback_Registration_t          EventCallback;
   struct _tagData_Channel_Entry_t *NextEntry;
} Data_Channel_Entry_t;

   /* Private structure for tracking parsed SDP data.                   */
typedef struct _tag_ParsedSDPData_t
{
   DEVM_Parsed_SDP_Data_t  ParsedSDPData;
   unsigned int            RESERVED;
   unsigned char          *RawSDPData;
} _ParsedSDPData_t;

#define HDPM_PARSED_SDP_DATA_MAGIC                             ((unsigned int)0x2FA58CC8)

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection,
   dstDisconnected
} DEVM_Status_Type_t;

   /* The following structure is a container structure that is used to  */
   /* hold all information regarding a Device Manager (DEVM)            */
   /* Asynchronous Status Result.                                       */
typedef struct _tagDEVM_Status_t
{
   DEVM_Status_Type_t StatusType;
   BD_ADDR_t          BD_ADDR;
   int                Status;
} DEVM_Status_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which tracks the state of registered endpoints.          */
static Endpoint_Entry_t *EndpointList;

   /* Variable which tracks the state of connections.                   */
static Connection_Entry_t *ConnectionList;

   /* Variable which tracks the state of data channel connections.      */
static Data_Channel_Entry_t *DataChannelList;

   /* Internal Function Prototypes.                                     */
static Boolean_t CallbackRegistrationValid(Callback_Registration_t *Registration);

static Endpoint_Entry_t *AddEndpointEntry(Endpoint_Entry_t **ListHead, Endpoint_Entry_t *EntryToAdd);
static Endpoint_Entry_t *SearchEndpointEntryMDEP(Endpoint_Entry_t **ListHead, Byte_t MDEP_ID);
static Endpoint_Entry_t *SearchEndpointEntryTypeRole(Endpoint_Entry_t **ListHead, Word_t DataType, HDP_Device_Role_t Role);
static Endpoint_Entry_t *DeleteEndpointEntryMDEP(Endpoint_Entry_t **ListHead, Byte_t MDEP_ID);
static Endpoint_Entry_t *DeleteEndpointEntryClientID(Endpoint_Entry_t **ListHead, unsigned int ClientID);
static void FreeEndpointEntryMemory(Endpoint_Entry_t *EntryToFree);
static void FreeEndpointEntryList(Endpoint_Entry_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, DWord_t Instance);
static Connection_Entry_t *SearchConnectionEntryMCLID(Connection_Entry_t **ListHead, unsigned int MCLID);
static Connection_Entry_t *SearchConnectionEntryBDADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *DeleteConnectionEntryClientID(Connection_Entry_t **ListHead, unsigned int ClientID);
static Connection_Entry_t *DeleteConnectionEntryExact(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToDelete);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static Data_Channel_Entry_t *AddDataChannelEntry(Data_Channel_Entry_t **ListHead, Data_Channel_Entry_t *EntryToAdd);
static Data_Channel_Entry_t *SearchDataChannelEntry(Data_Channel_Entry_t **ListHead, unsigned int DataLinkID);
static Data_Channel_Entry_t *SearchDataChannelEntryMCLID_MDEP(Data_Channel_Entry_t **ListHead, unsigned int MCLID, Byte_t MDEP_ID);
static Data_Channel_Entry_t *DeleteDataChannelEntry(Data_Channel_Entry_t **ListHead, unsigned int DataLinkID);
static Data_Channel_Entry_t *DeleteDataChannelEntryClientID(Data_Channel_Entry_t **ListHead, unsigned int ClientID);
static Data_Channel_Entry_t *DeleteDataChannelEntryExact(Data_Channel_Entry_t **ListHead, Data_Channel_Entry_t *EntryToDelete);
static void FreeDataChannelEntryMemory(Data_Channel_Entry_t *EntryToFree);
static void FreeDataChannelEntryList(Data_Channel_Entry_t **ListHead);

static int CompareSDPElementUUID(SDP_Data_Element_t *Element, UUID_128_t *UUID);
static int FindSDPNextHDPRecord(DEVM_Parsed_SDP_Data_t *ParsedSDPData, unsigned int *StartingRecordIndex, SDP_Service_Attribute_Response_Data_t **InstanceRecord);
static int FindSDPInstance(DEVM_Parsed_SDP_Data_t *ParsedSDPData, DWord_t InstanceNumber, SDP_Service_Attribute_Response_Data_t **InstanceRecord);
static int ParseSDPInstanceNumber(SDP_Service_Attribute_Response_Data_t *InstanceRecord, DWord_t *InstanceNumber);
static int FindSDPNumberOfEndpoints(SDP_Service_Attribute_Response_Data_t *InstanceRecord);
static int GetSDPEndpointInfo(SDP_Service_Attribute_Response_Data_t *InstanceRecord, unsigned int Index, HDP_MDEP_Info_t *EndpointInfo);
static int ParseSDPInstances(DEVM_Parsed_SDP_Data_t *ParsedSDPData, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances);
static int ParseSDPEndpoints(DEVM_Parsed_SDP_Data_t *ParsedSDPData, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints);
static int FindSDPEndpointInfo(DEVM_Parsed_SDP_Data_t *ParsedSDPData, DWord_t Instance, unsigned int *StartingIndex, Byte_t MDEP_ID, HDP_MDEP_Info_t *MDEPInfo, unsigned int *DescriptionLength);
static int GetParsedSDPData(BD_ADDR_t RemoteDeviceAddress, DEVM_Parsed_SDP_Data_t **ParsedSDPData);
static void FreeParsedSDPData(DEVM_Parsed_SDP_Data_t *ParsedSDPData);
static void DispatchControlConnectConfirmationEvent(Connection_Entry_t *ConnectionPtr, int ConnectionStatus);

static void ProcessRegisterEndpointMessage(HDPM_Register_Endpoint_Request_t *Message);
static void ProcessUnRegisterEndpointMessage(HDPM_Un_Register_Endpoint_Request_t *Message);
static void ProcessDataConnectionRequestResponseMessage(HDPM_Data_Connection_Request_Response_Request_t *Message);
static void ProcessQueryRemoteDeviceInstancesMessage(HDPM_Query_Remote_Device_Instances_Request_t *Message);
static void ProcessQueryRemoteDeviceEndpointsMessage(HDPM_Query_Remote_Device_Endpoints_Request_t *Message);
static void ProcessQueryEndpointDescriptionMessage(HDPM_Query_Endpoint_Description_Request_t *Message);
static void ProcessConnectRemoteDeviceMessage(HDPM_Connect_Remote_Device_Request_t *Message);
static void ProcessDisconnectRemoteDeviceMessage(HDPM_Disconnect_Remote_Device_Request_t *Message);
static void ProcessConnectRemoteDeviceEndpointMessage(HDPM_Connect_Remote_Device_Endpoint_Request_t *Message);
static void ProcessDisconnectRemoteDeviceEndpointMessage(HDPM_Disconnect_Remote_Device_Endpoint_Request_t *Message);
static void ProcessWriteDataMessage(HDPM_Write_Data_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessControlConnectIndicationEvent(HDP_Control_Connect_Indication_Data_t *ControlConnectIndicationData);
static void ProcessControlConnectConfirmationEvent(HDP_Control_Connect_Confirmation_Data_t *ControlConnectConfirmationData);
static void ProcessControlDisconnectIndicationEvent(HDP_Control_Disconnect_Indication_Data_t *ControlDisconnectIndicationData);
static void ProcessControlCreateDataLinkIndicationEvent(HDP_Control_Create_Data_Link_Indication_t *ControlCreateDataLinkIndicationData);
static void ProcessControlCreateDataLinkConfirmationEvent(HDP_Control_Create_Data_Link_Confirmation_t *ControlCreateDataLinkConfirmationData);
static void ProcessControlAbortDataLinkIndicationEvent(HDP_Control_Abort_Data_Link_Indication_t *ControlAbortDataLinkIndicationData);
static void ProcessControlAbortDataLinkConfirmationEvent(HDP_Control_Abort_Data_Link_Confirmation_t *ControlAbortDataLinkConfirmationData);
static void ProcessControlDeleteDataLinkIndicationEvent(HDP_Control_Delete_Data_Link_Indication_t *ControlDeleteDataLinkIndicationData);
static void ProcessControlDeleteDataLinkConfirmationEvent(HDP_Control_Delete_Data_Link_Confirmation_t *ControlDeleteDataLinkConfirmationData);
static void ProcessDataLinkConnectIndicationEvent(HDP_Data_Link_Connect_Indication_Data_t *DataLinkConnectIndicationData);
static void ProcessDataLinkConnectConfirmationEvent(HDP_Data_Link_Connect_Confirmation_Data_t *DataLinkConnectConfirmationData);
static void ProcessDataLinkDisconnectIndicationEvent(HDP_Data_Link_Disconnect_Indication_Data_t *DataLinkDisconnectIndicationData);
static void ProcessDataLinkDataIndicationEvent(HDP_Data_Link_Data_Indication_Data_t *DataLinkDataIndicationData);
static void ProcessSyncCapabilitiesIndicationEvent(HDP_Sync_Capabilities_Indication_t *SyncCapabilitiesIndicationData);
static void ProcessSyncCapabilitiesConfirmationEvent(HDP_Sync_Capabilities_Confirmation_t *SyncCapabilitiesConfirmationData);
static void ProcessSyncSetIndicationEvent(HDP_Sync_Set_Indication_t *SyncSetIndicationData);
static void ProcessSyncSetConfirmationEvent(HDP_Sync_Set_Confirmation_t *SyncSetConfirmationData);
static void ProcessSyncInfoIndicationEvent(HDP_Sync_Info_Indication_t *SyncInfoIndicationData);

static void ProcessHealthDeviceEvent(HDPM_HDP_Event_Data_t *HealthDeviceEventData);

static void BTPSAPI BTPMDispatchCallback_DEVM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_HDPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_HDP(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HealthDeviceManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

static Boolean_t CallbackRegistrationValid(Callback_Registration_t *Registration)
{
   Boolean_t ret_val;

   if(Registration)
   {
      /* A direct function callback should have the ClientID set to the */
      /* local server. A valid remote callback simply requires a valid, */
      /* non-server, non-broadcast ClientID.                            */
      if(Registration->ClientID == MSG_GetServerAddressID())
         ret_val = (Boolean_t)(Registration->CallbackFunction != NULL);
      else
         ret_val = (Boolean_t)(Registration->ClientID != MSG_GetBroadcastAddressID());
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

static Endpoint_Entry_t *AddEndpointEntry(Endpoint_Entry_t **ListHead, Endpoint_Entry_t *EntryToAdd)
{
   Endpoint_Entry_t *AddedEntry = NULL;
   Endpoint_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (Endpoint_Entry_t *)BTPS_AllocateMemory(sizeof(Endpoint_Entry_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                      = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextEntry = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               if(tmpEntry->MDEP_ID == AddedEntry->MDEP_ID)
               {
                  /* Entry was already added, so free the memory and    */
                  /* flag an error to the caller.                       */
                  FreeEndpointEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* OK, we need to see if we are at the last element   */
                  /* of the List. If we are, we simply break out of     */
                  /* the list traversal because we know there are NO    */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextEntry)
                     tmpEntry = tmpEntry->NextEntry;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextEntry = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

static Endpoint_Entry_t *SearchEndpointEntryMDEP(Endpoint_Entry_t **ListHead, Byte_t MDEP_ID)
{
   Endpoint_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", MDEP_ID));

   /* Let's make sure the list appears to be valid.                     */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->MDEP_ID != MDEP_ID))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Endpoint_Entry_t *SearchEndpointEntryTypeRole(Endpoint_Entry_t **ListHead, Word_t DataType, HDP_Device_Role_t Role)
{
   Endpoint_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%04X, %d\n", DataType, Role));

   /* Let's make sure the list appears to be valid.                     */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((FoundEntry->MDEP_DataType != DataType) || (FoundEntry->MDEP_Role != Role)))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Endpoint_Entry_t *DeleteEndpointEntryMDEP(Endpoint_Entry_t **ListHead, Byte_t MDEP_ID)
{
   Endpoint_Entry_t *FoundEntry = NULL;
   Endpoint_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%02X\n", MDEP_ID));

   /* Let's make sure the list and MDEP ID to search for appear to be   */
   /* valid.                                                            */
   if((ListHead) && (MDEP_ID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->MDEP_ID != MDEP_ID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Endpoint_Entry_t *DeleteEndpointEntryClientID(Endpoint_Entry_t **ListHead, unsigned int ClientID)
{
   Endpoint_Entry_t *FoundEntry = NULL;
   Endpoint_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", ClientID));

   /* Let's make sure the list and MDEP ID to search for appear to be   */
   /* valid.                                                            */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallback.ClientID != ClientID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static void FreeEndpointEntryMemory(Endpoint_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void FreeEndpointEntryList(Endpoint_Entry_t **ListHead)
{
   Endpoint_Entry_t *EntryToFree;
   Endpoint_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextEntry;

         FreeEndpointEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd)
{
   Connection_Entry_t *AddedEntry = NULL;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Connection_Entry_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                      = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextEntry = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               if((COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)) && (tmpEntry->Instance == AddedEntry->Instance))
               {
                  /* Entry was already added, so free the memory and    */
                  /* flag an error to the caller.                       */
                  FreeConnectionEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* OK, we need to see if we are at the last element   */
                  /* of the List. If we are, we simply break out of     */
                  /* the list traversal because we know there are NO    */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextEntry)
                     tmpEntry = tmpEntry->NextEntry;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextEntry = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, DWord_t Instance)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0, Instance));

   /* Let's make sure the list and search keys appear to be valid.      */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (VALID_INSTANCE(Instance)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->Instance != Instance)))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Connection_Entry_t *SearchConnectionEntryMCLID(Connection_Entry_t **ListHead, unsigned int MCLID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", MCLID));

   /* Let's make sure the list and MCLID to search for appear to be     */
   /* valid.                                                            */
   if((ListHead) && (MCLID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->MCLID != MCLID))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Connection_Entry_t *SearchConnectionEntryBDADDR(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X\n", BD_ADDR.BD_ADDR5, BD_ADDR.BD_ADDR4, BD_ADDR.BD_ADDR3, BD_ADDR.BD_ADDR2, BD_ADDR.BD_ADDR1, BD_ADDR.BD_ADDR0));

   /* Let's make sure the list appears to be valid.                     */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Connection_Entry_t *DeleteConnectionEntryClientID(Connection_Entry_t **ListHead, unsigned int ClientID)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", ClientID));

   /* Let's make sure the list appears to be valid.                     */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallback.ClientID != ClientID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Connection_Entry_t *DeleteConnectionEntryExact(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToDelete)
{
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %p\n", EntryToDelete));

   /* Let's make sure the list and entry to search for appear to be     */
   /* valid.                                                            */
   if((ListHead) && (EntryToDelete))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry != EntryToDelete))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
   {
      if(EntryToFree->ConnectionEvent)
         BTPS_CloseEvent(EntryToFree->ConnectionEvent);

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextEntry;

         FreeConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static Data_Channel_Entry_t *AddDataChannelEntry(Data_Channel_Entry_t **ListHead, Data_Channel_Entry_t *EntryToAdd)
{
   Data_Channel_Entry_t *AddedEntry = NULL;
   Data_Channel_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's allocate a new data structure */
      /* to add to the list.                                            */
      AddedEntry = (Data_Channel_Entry_t *)BTPS_AllocateMemory(sizeof(Data_Channel_Entry_t));

      if(AddedEntry)
      {
         /* Copy All Data over.                                         */
         *AddedEntry                      = *EntryToAdd;

         /* Now Add it to the end of the list.                          */
         AddedEntry->NextEntry = NULL;

         /* First, let's check to see if there are any elements already */
         /* present in the List that was passed in.                     */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Head Pointer was not NULL, so we will traverse the list  */
            /* until we reach the last element.                         */
            while(tmpEntry)
            {
               if((tmpEntry->MCLID == AddedEntry->MCLID) && (tmpEntry->DataLinkID == AddedEntry->DataLinkID))
               {
                  /* Entry was already added, so free the memory and    */
                  /* flag an error to the caller.                       */
                  FreeDataChannelEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the Search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* OK, we need to see if we are at the last element   */
                  /* of the List. If we are, we simply break out of     */
                  /* the list traversal because we know there are NO    */
                  /* duplicates AND we are at the end of the list.      */
                  if(tmpEntry->NextEntry)
                     tmpEntry = tmpEntry->NextEntry;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Last element found, simply Add the entry.             */
               tmpEntry->NextEntry = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

static Data_Channel_Entry_t *SearchDataChannelEntry(Data_Channel_Entry_t **ListHead, unsigned int DataLinkID)
{
   Data_Channel_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", DataLinkID));

   /* Let's make sure the list and DataLink ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (DataLinkID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->DataLinkID != DataLinkID))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Data_Channel_Entry_t *SearchDataChannelEntryMCLID_MDEP(Data_Channel_Entry_t **ListHead, unsigned int MCLID, Byte_t MDEP_ID)
{
   Data_Channel_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d, 0x%02X\n", MCLID, MDEP_ID));

   /* Let's make sure the list and DataLink ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (MCLID) && (MDEP_ID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((FoundEntry->MCLID != MCLID) || (FoundEntry->MDEP_ID != MDEP_ID)))
         FoundEntry = FoundEntry->NextEntry;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Data_Channel_Entry_t *DeleteDataChannelEntry(Data_Channel_Entry_t **ListHead, unsigned int DataLinkID)
{
   Data_Channel_Entry_t *FoundEntry = NULL;
   Data_Channel_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", DataLinkID));

   /* Let's make sure the list and DataLink ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (DataLinkID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->DataLinkID != DataLinkID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Data_Channel_Entry_t *DeleteDataChannelEntryClientID(Data_Channel_Entry_t **ListHead, unsigned int ClientID)
{
   Data_Channel_Entry_t *FoundEntry = NULL;
   Data_Channel_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", ClientID));

   /* Let's make sure the list appears to be valid.                     */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallback.ClientID != ClientID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static Data_Channel_Entry_t *DeleteDataChannelEntryExact(Data_Channel_Entry_t **ListHead, Data_Channel_Entry_t *EntryToDelete)
{
   Data_Channel_Entry_t *FoundEntry = NULL;
   Data_Channel_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %p\n", EntryToDelete));

   /* Let's make sure the list and entry to search for appear to be     */
   /* valid.                                                            */
   if((ListHead) && (EntryToDelete))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry != EntryToDelete))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextEntry = FoundEntry->NextEntry;
         }
         else
            *ListHead = FoundEntry->NextEntry;

         FoundEntry->NextEntry = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

static void FreeDataChannelEntryMemory(Data_Channel_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
   {
      if(EntryToFree->ConnectionEvent)
         BTPS_CloseEvent(EntryToFree->ConnectionEvent);

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void FreeDataChannelEntryList(Data_Channel_Entry_t **ListHead)
{
   Data_Channel_Entry_t *EntryToFree;
   Data_Channel_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextEntry;

         FreeDataChannelEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which compares an    */
   /* SDP Data Element to a UUID. This function returns 0 if the UUIDs  */
   /* match, or a negative error code.                                  */
static int CompareSDPElementUUID(SDP_Data_Element_t *Element, UUID_128_t *UUID)
{
   int        ret_val;
   UUID_128_t TmpUUID;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((Element) && (UUID))
   {
      ASSIGN_SDP_UUID_128(TmpUUID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

      ret_val = 0;

      switch(Element->SDP_Data_Element_Type)
      {
         case deUUID_16:
            if(Element->SDP_Data_Element_Length == sizeof(UUID_16_t))
            {
               SDP_ASSIGN_SDP_UUID_128(TmpUUID);
               ASSIGN_SDP_UUID_16_TO_SDP_UUID_128(TmpUUID, Element->SDP_Data_Element.UUID_16);
            }
            else
               ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
            break;
         case deUUID_32:
            if(Element->SDP_Data_Element_Length == sizeof(UUID_32_t))
            {
               SDP_ASSIGN_SDP_UUID_128(TmpUUID);
               ASSIGN_SDP_UUID_32_TO_SDP_UUID_128(TmpUUID, Element->SDP_Data_Element.UUID_32);
            }
            else
               ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
            break;
         case deUUID_128:
            if(Element->SDP_Data_Element_Length == sizeof(UUID_128_t))
               TmpUUID = Element->SDP_Data_Element.UUID_128;
            else
               ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
            break;
         default:
            /* Unexpected element type. Return an error.                */
            ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
            break;
      }

      if(!ret_val)
         ret_val = !COMPARE_UUID_128(TmpUUID, *UUID);
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which locates the    */
   /* next HDP SDP record, starting from a given record number. On      */
   /* success, this function returns the index of the located record,   */
   /* and InstanceRecord (if provided) will point to the Attribute      */
   /* Response Data for that record. Regardless of the result of the    */
   /* search, StartingRecord will be set to the index of the record     */
   /* *following* the last searched index, so it can be reused in the   */
   /* next call to this function in order to continue the search from   */
   /* where it left off.                                                */
static int FindSDPNextHDPRecord(DEVM_Parsed_SDP_Data_t *ParsedSDPData, unsigned int *StartingRecordIndex, SDP_Service_Attribute_Response_Data_t **InstanceRecord)
{
   int                                    ret_val;
   UUID_128_t                             SinkUUID;
   UUID_128_t                             SourceUUID;
   unsigned int                           AttributeIndex;
   unsigned int                           ServiceClassIndex;
   SDP_Data_Element_t                    *ServiceClassElement;
   SDP_Data_Element_t                    *ServiceClassIDList;
   SDP_Service_Attribute_Response_Data_t *SDPRecord;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if((ParsedSDPData) && (StartingRecordIndex))
   {
      SDP_ASSIGN_HEALTH_DEVICE_SINK_SERVICE_CLASS_UUID_128(SinkUUID);
      SDP_ASSIGN_HEALTH_DEVICE_SOURCE_SERVICE_CLASS_UUID_128(SourceUUID);

      ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND;

      /* Begin searching SDP records for HDP service classes.           */
      for( ;((ret_val == BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND) && (*StartingRecordIndex < ParsedSDPData->NumberServiceRecords));*StartingRecordIndex += 1)
      {
         SDPRecord          = &(ParsedSDPData->SDPServiceAttributeResponseData[*StartingRecordIndex]);
         ServiceClassIDList = NULL;

         /* Locate the ServiceClassIDList attribute.                    */
         for(AttributeIndex=0;((AttributeIndex < (unsigned int)SDPRecord->Number_Attribute_Values) && (!ServiceClassIDList));AttributeIndex++)
         {
            if(SDPRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST)
               ServiceClassIDList = SDPRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;
         }

         /* Check whether the ServiceClassIDList attribute was located  */
         /* before the end of the attribute list was reached.           */
         if(ServiceClassIDList)
         {
            /* The ServiceClassIDList attribute was found. First,       */
            /* confirm that the attribute is a Sequence.                */
            if(ServiceClassIDList->SDP_Data_Element_Type == deSequence)
            {
               /* The attribute appears valid, so scan the attribute for*/
               /* either the HDP Source or HDP Sink Service Class ID.   */
               for(ServiceClassIndex = 0;((ret_val == BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND) && (ServiceClassIndex < ServiceClassIDList->SDP_Data_Element_Length));ServiceClassIndex++)
               {
                  ServiceClassElement = &(ServiceClassIDList->SDP_Data_Element.SDP_Data_Element_Sequence[ServiceClassIndex]);

                  /* First, check if this is a HDP Source UUID.         */
                  ret_val = CompareSDPElementUUID(ServiceClassElement, &SourceUUID);

                  /* If not, then check if this is a HDP Sink UUID.     */
                  if(ret_val)
                     ret_val = CompareSDPElementUUID(ServiceClassElement, &SinkUUID);

                  if(!ret_val)
                  {
                     /* HDP Record found. Return this record index and  */
                     /* update the StartingRecordIndex state variable   */
                     /* for use in the next call to this function.      */
                     ret_val = *StartingRecordIndex;

                     /* If the caller requested it, provide a reference */
                     /* to the discovered HDP record.                   */
                     if(InstanceRecord)
                        *InstanceRecord = SDPRecord;
                  }
                  else
                  {
                     if(ret_val != BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND)
                     {
                        /* The SDP Data Element was not a UUID. This is */
                        /* an invalid service record so skip it.        */
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("ServiceClassIDList element %d non-UUID\n", ServiceClassIndex));
                        break;
                     }
                  }
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("ServiceClassIDList attribute type invalid\n"));

               ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("ServiceClassIDList attribute not found\n"));

            ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which locates the HDP*/
   /* SDP record associated with the given Instance. On success, this   */
   /* function returns zero and points InstanceRecord (if provided) to  */
   /* the Attribute Response Data for that record.                      */
static int FindSDPInstance(DEVM_Parsed_SDP_Data_t *ParsedSDPData, DWord_t InstanceNumber, SDP_Service_Attribute_Response_Data_t **InstanceRecord)
{
   int                                    ret_val;
   DWord_t                                Instance;
   unsigned int                           NextRecordIndex;
   SDP_Service_Attribute_Response_Data_t *InstancePtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((ParsedSDPData) && (VALID_INSTANCE(InstanceNumber)))
   {
      ret_val         = 0;
      InstancePtr     = NULL;
      NextRecordIndex = 0;

      /* Search for the interesting SDP record.                         */
      while((!ret_val) && (!InstancePtr))
      {
         /* Locate the next HDP SDP record.                             */
         if((ret_val = FindSDPNextHDPRecord(ParsedSDPData, &NextRecordIndex, &InstancePtr)) >= 0)
         {
            /* An HDP Instance record was found.                        */
            ret_val = 0;

            /* Extract the Instance ID from the record.                 */
            if(!ParseSDPInstanceNumber(InstancePtr, &Instance))
            {
               /* Check whether this is the record we are seeking.      */
               if(InstanceNumber == Instance)
               {
                  /* The record has been found. If the caller requested */
                  /* a reference to this record, set it now.            */
                  if(InstanceRecord)
                     *InstanceRecord = InstancePtr;
               }
               else
               {
                  /* This is not the target record, so throw away the   */
                  /* reference and keep searching.                      */
                  InstancePtr = NULL;
               }
            }
            else
            {
               /* Locating the Instace ID of the record failed, so note */
               /* that this is NOT the record we are seeking.           */
               InstancePtr = NULL;
            }
         }
         else
         {
            /* Check whether we reached the end of the set of records.  */
            if(ret_val != BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND)
            {
               /* Record parsing failed before reaching the end of the  */
               /* set of records. Ignore the current record and move to */
               /* the next. This may involve re-processing some non-HDP */
               /* records, but eventually we will move past the last    */
               /* record and end the loop.                              */
               ret_val = 0;
            }
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which extracts the   */
   /* Instance ID from an HDP SDP record. On success, this function     */
   /* returns zero and sets InstanceNumber to the Instance's Instance   */
   /* ID.                                                               */
static int ParseSDPInstanceNumber(SDP_Service_Attribute_Response_Data_t *InstanceRecord, DWord_t *InstanceNumber)
{
   int                 ret_val;
   Word_t              DataPSM;
   Word_t              ControlPSM;
   UUID_128_t          L2CAP_UUID;
   UUID_128_t          MCAPDataUUID;
   UUID_128_t          MCAPControlUUID;
   unsigned int        AttributeIndex;
   SDP_Data_Element_t *MCAPDescriptor;
   SDP_Data_Element_t *L2CAPDescriptor;
   SDP_Data_Element_t *ProtocolDescriptorList;
   SDP_Data_Element_t *AdditionalDescriptorList;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if((InstanceRecord) && (InstanceNumber))
   {
      SDP_ASSIGN_L2CAP_UUID_128(L2CAP_UUID);
      SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_128(MCAPDataUUID);
      SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_128(MCAPControlUUID);

      ret_val    = 0;
      DataPSM    = 0;
      ControlPSM = 0;

      /* Locate the Protocol Descriptor List attribute.                 */
      for(AttributeIndex=0;AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values;AttributeIndex++)
      {
         if(InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST)
            break;
      }

      /* Check whether the Protocol Descriptor List attribute was       */
      /* located before the end of the attribute list was reached.      */
      if(AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values)
      {
         /* The Protocol Descriptor List attribute was found.           */
         ProtocolDescriptorList = InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;

         /* Begin validating that the contents of this attribute match  */
         /* was is expected for HDP. First, confirm that the Protocol   */
         /* Descriptor List is a sequence of at least two elements.     */
         if((ProtocolDescriptorList->SDP_Data_Element_Type == deSequence) && (ProtocolDescriptorList->SDP_Data_Element_Length >= 2))
         {
            /* The first Protocol Descriptor should be for L2CAP and    */
            /* will contain the MCAP Control Channel PSM.               */
            L2CAPDescriptor = &(ProtocolDescriptorList->SDP_Data_Element.SDP_Data_Element_Sequence[0]);

            /* The second Protocol Descriptor should be for MCAP.       */
            MCAPDescriptor  = &(ProtocolDescriptorList->SDP_Data_Element.SDP_Data_Element_Sequence[1]);

            /* The L2CAP Descriptor should be a sequence of exactly two */
            /* elements.                                                */
            if((L2CAPDescriptor->SDP_Data_Element_Type == deSequence) && (L2CAPDescriptor->SDP_Data_Element_Length == 2))
            {
               /* Confirm that the first element of the L2CAP Descriptor*/
               /* is the L2CAP Protocol UUID.                           */
               if(!CompareSDPElementUUID(&(L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[0]), &L2CAP_UUID))
               {
                  /* Confirm that the second element of the L2CAP       */
                  /* protocol identifier is a 16-bit unsigned integer.  */
                  if((L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type == deUnsignedInteger2Bytes) && (L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Length == sizeof(Word_t)))
                  {
                     /* For extra validation, confirm that the MCAP     */
                     /* Protocol Descriptor is a sequence of exactly two*/
                     /* elements.                                       */
                     if((MCAPDescriptor->SDP_Data_Element_Type == deSequence) && (MCAPDescriptor->SDP_Data_Element_Length == 2))
                     {
                        /* And that the first element of the MCAP       */
                        /* Protocol Descriptor is the MCAP Control      */
                        /* Channel UUID.                                */
                        if(!CompareSDPElementUUID(&(MCAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[0]), &MCAPControlUUID))
                        {
                           /* The Protocol Descriptor List seems valid. */
                           /* Copy the Control Channel PSM from the     */
                           /* L2CAP Descriptor.                         */
                           ControlPSM = L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element.UnsignedInteger2Bytes;
                        }
                        else
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("First element of primary MCAP descriptor does not contain the MCAP Control UUID.\n"));

                           ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                        }
                     }
                     else
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The primary MCAP descriptor is not a two-element sequence.\n"));

                        ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Second element of primary L2CAP descriptor does not contain a UINT_16 value.\n"));

                     ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("First element of primary L2CAP descriptor does not contain the L2CAP UUID.\n"));

                  ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The primary L2CAP descriptor is not a two-element sequence.\n"));

               ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The primary Protocol Descriptor List is not a sequence of at least two elements.\n"));

            ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Protocol Descriptor List attribute was not found.\n"));

         ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
      }

      /* Continue only if no errors have occurred and the ControlPSM was*/
      /* located.                                                       */
      if((!ret_val) && (ControlPSM))
      {
         /* Locate the Additional Protocol Descriptor Lists attribute.  */
         for(AttributeIndex=0;AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values;AttributeIndex++)
         {
            if(InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == SDP_ATTRIBUTE_ID_ADDITIONAL_PROTOCOL_DESCRIPTOR_LISTS)
               break;
         }

         /* Check whether the Additional Protocol Descriptor Lists      */
         /* attribute was located before the end of the attribute list  */
         /* was reached.                                                */
         if(AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values)
         {
            /* The Additional Protocol Descriptor List attribute was    */
            /* found.                                                   */
            AdditionalDescriptorList = InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;

            /* Begin validating that the contents of this attribute     */
            /* match was is expected for HDP. First, confirm that the   */
            /* Additional Protocol Descriptor Lists attribute is a      */
            /* sequence of exactly one element.                         */
            if((AdditionalDescriptorList->SDP_Data_Element_Type == deSequence) && (AdditionalDescriptorList->SDP_Data_Element_Length == 1))
            {
               /* The only element of the Additional Protocol Descriptor*/
               /* Lists sequence is the secondary Protocol Descriptor   */
               /* List.                                                 */
               ProtocolDescriptorList = &(AdditionalDescriptorList->SDP_Data_Element.SDP_Data_Element_Sequence[0]);

               /* Confirm that the Protocol Descriptor List is a        */
               /* sequence of at least two elements.                    */
               if((ProtocolDescriptorList->SDP_Data_Element_Type == deSequence) && (ProtocolDescriptorList->SDP_Data_Element_Length >= 2))
               {
                  /* The first Protocol Descriptor should be for L2CAP  */
                  /* and will contain the MCAP Data Channel PSM.        */
                  L2CAPDescriptor = &(ProtocolDescriptorList->SDP_Data_Element.SDP_Data_Element_Sequence[0]);

                  /* The second Protocol Descriptor should be for MCAP. */
                  MCAPDescriptor  = &(ProtocolDescriptorList->SDP_Data_Element.SDP_Data_Element_Sequence[1]);

                  /* The L2CAP Descriptor should be a sequence of       */
                  /* exactly two elements.                              */
                  if((L2CAPDescriptor->SDP_Data_Element_Type == deSequence) && (L2CAPDescriptor->SDP_Data_Element_Length == 2))
                  {
                     /* Confirm that the first element of the L2CAP     */
                     /* Descriptor is the L2CAP Protocol UUID.          */
                     if(!CompareSDPElementUUID(&(L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[0]), &L2CAP_UUID))
                     {
                        /* Confirm that the second element of the L2CAP */
                        /* protocol identifier is a 16-bit unsigned     */
                        /* integer.                                     */
                        if((L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type == deUnsignedInteger2Bytes) && (L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Length == sizeof(Word_t)))
                        {
                           /* For extra validation, confirm that the    */
                           /* MCAP Protocol Descriptor is a sequence of */
                           /* exactly one element.                      */
                           if((MCAPDescriptor->SDP_Data_Element_Type == deSequence) && (MCAPDescriptor->SDP_Data_Element_Length == 1))
                           {
                              /* And that the single element of the MCAP*/
                              /* Protocol Descriptor is the MCAP Data   */
                              /* Channel UUID.                          */
                              if(!CompareSDPElementUUID(&(MCAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[0]), &MCAPDataUUID))
                              {
                                 /* The Protocol Descriptor List seems  */
                                 /* valid. Copy the Data Channel PSM    */
                                 /* from the L2CAP Descriptor.          */
                                 DataPSM = L2CAPDescriptor->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element.UnsignedInteger2Bytes;
                              }
                              else
                              {
                                 DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("First element of secondary MCAP descriptor does not contain the MCAP Data UUID.\n"));

                                 ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                              }
                           }
                           else
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The secondary MCAP descriptor is not a one-element sequence.\n"));

                              ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                           }
                        }
                        else
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Second element of secondary L2CAP descriptor does not contain a UINT_16 value.\n"));

                           ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                        }
                     }
                     else
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("First element of secondary L2CAP descriptor does not contain the L2CAP UUID.\n"));

                        ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The secondary L2CAP descriptor is not a two-element sequence.\n"));

                     ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The secondary Protocol Descriptor List is not a sequence of at least two elements.\n"));

                  ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Additional Protocol Descriptor Lists attribute was not a one-element sequence.\n"));

               ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Additional Protocol Descriptor Lists attribute was not found\n"));

            ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
         }

         /* If no errors have occurred, create the Instance ID and      */
         /* return it to the user.                                      */
         if((!ret_val) && (DataPSM))
            *InstanceNumber = SET_INSTANCE(ControlPSM, DataPSM);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which determines the */
   /* number of Endpoints advertised for a particular Instance in an HDP*/
   /* SDP record. On success, this function returns the number of valid */
   /* endpoints that were found.                                        */
static int FindSDPNumberOfEndpoints(SDP_Service_Attribute_Response_Data_t *InstanceRecord)
{
   int                 ret_val;
   unsigned int        EndpointCount;
   unsigned int        MDEPIndex;
   unsigned int        AttributeIndex;
   SDP_Data_Element_t *SupportedFeaturesList;
   SDP_Data_Element_t *MDEPDefinition;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if(InstanceRecord)
   {
      ret_val = 0;

      /* Locate the Protocol Descriptor List attribute.                 */
      for(AttributeIndex=0;AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values;AttributeIndex++)
      {
         if(InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == SDP_ATTRIBUTE_ID_HDP_SUPPORTED_FEATURES)
            break;
      }

      /* Check whether the Supported Features attribute was located     */
      /* before the end of the attribute list was reached.              */
      if(AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values)
      {
         /* The Protocol Descriptor List attribute was found.           */
         SupportedFeaturesList = InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;

         /* Begin validating that the contents of this attribute match  */
         /* was is expected for HDP. First, confirm that the Supported  */
         /* Features List is a sequence. The spec requires that this    */
         /* attribute contain at least one element, but we will be      */
         /* tolerant of implementations that do not adhere to this      */
         /* restriction.                                                */
         if(SupportedFeaturesList->SDP_Data_Element_Type == deSequence)
         {
            EndpointCount = 0;

            /* Validate each MCAP Definition before returning the count.*/
            for(MDEPIndex = 0;((!ret_val) && (MDEPIndex < SupportedFeaturesList->SDP_Data_Element_Length));MDEPIndex++)
            {
               MDEPDefinition = &(SupportedFeaturesList->SDP_Data_Element.SDP_Data_Element_Sequence[MDEPIndex]);

               /* Each MDEP Definition is a sequence of exactly three or*/
               /* four elements.                                        */
               if((MDEPDefinition->SDP_Data_Element_Type == deSequence) && ((MDEPDefinition->SDP_Data_Element_Length == 3) || (MDEPDefinition->SDP_Data_Element_Length == 4)))
               {
                  /* The first element is the MDEP ID, an 8-bit unsigned*/
                  /* integer.                                           */
                  if((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUnsignedInteger1Byte) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Length == sizeof(Byte_t)))
                  {
                     /* Confirm that the MDEP ID is valid.              */
                     if(VALID_MDEP_ID(MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UnsignedInteger1Byte))
                     {
                        /* The second element is the MDEP Data Type, a  */
                        /* 16-bit unsigned integer.                     */
                        if((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type == deUnsignedInteger2Bytes) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Length == sizeof(Word_t)))
                        {
                           /* The third element is the MDEP Role, an    */
                           /* 8-bit unsigned integer.                   */
                           if((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element_Type == deUnsignedInteger1Byte) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element_Length == sizeof(Byte_t)))
                           {
                              /* The MDEP Role may only have a value of */
                              /* 0x00 (Source) or 0x01 (Sink).          */
                              if(MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element.UnsignedInteger1Byte <= 0x01)
                              {
                                 /* The optional fourth element is the  */
                                 /* MDEP Description. If it exists, it  */
                                 /* is a string of variable length.     */
                                 if(((MDEPDefinition->SDP_Data_Element_Length == 4) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[3].SDP_Data_Element_Type == deTextString)) || (MDEPDefinition->SDP_Data_Element_Length == 3))
                                 {
                                    /* This MDEP Definition appears     */
                                    /* valid, so count it.              */
                                    EndpointCount++;
                                 }
                                 else
                                 {
                                    DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The fourth element of MDEP Definition #%d is present but is not a String.\n", MDEPIndex));

                                    ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                                 }
                              }
                              else
                              {
                                 DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("MDEP Definition #%d contains an invalid Role.\n", MDEPIndex));

                                 ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                              }
                           }
                           else
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The third element of MDEP Definition #%d is not a UINT_8.\n", MDEPIndex));

                              ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                           }
                        }
                        else
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The second element of MDEP Definition #%d is not a UINT_16.\n", MDEPIndex));

                           ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                        }
                     }
                     else
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("MDEP Definition #%d contains an invalid MDEP ID.\n", MDEPIndex));

                        ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The first element of MDEP Definition #%d is not a UINT_8.\n", MDEPIndex));

                     ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("MDEP Definition #%d is not a sequence or contains an invalid number of elements.\n", MDEPIndex));

                  ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
               }
            }

            /* If at least one valid endpoint was found, return the     */
            /* number of endpoints. Otherwise, either zero or an error  */
            /* code will be returned.                                   */
            if(EndpointCount)
               ret_val = EndpointCount;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Supported Features attribute is not a sequence.\n"));

            ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Supported Features attribute was not found.\n"));

         ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which extracts the   */
   /* details of a particular endpoint from an Instance's SDP record.   */
   /* On success, this function returns zero or (if present) the length */
   /* of the MDEP Description field, and populates MDEPInfo with the    */
   /* details of the Endpoint.  Note that MDEPInfo may reference data   */
   /* within InstanceRecord, so InstanceRecord must remain valid as long*/
   /* as MDEPInfo is within scope.                                      */
static int GetSDPEndpointInfo(SDP_Service_Attribute_Response_Data_t *InstanceRecord, unsigned int Index, HDP_MDEP_Info_t *MDEPInfo)
{
   int                 ret_val;
   unsigned int        AttributeIndex;
   SDP_Data_Element_t *SupportedFeaturesList;
   SDP_Data_Element_t *MDEPDefinition;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if((InstanceRecord) && (MDEPInfo))
   {
      ret_val = 0;

      /* Locate the Protocol Descriptor List attribute.                 */
      for(AttributeIndex=0;AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values;AttributeIndex++)
      {
         if(InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].Attribute_ID == SDP_ATTRIBUTE_ID_HDP_SUPPORTED_FEATURES)
            break;
      }

      /* Check whether the Supported Features attribute was located     */
      /* before the end of the attribute list was reached.              */
      if(AttributeIndex < (unsigned int)InstanceRecord->Number_Attribute_Values)
      {
         /* The Protocol Descriptor List attribute was found.           */
         SupportedFeaturesList = InstanceRecord->SDP_Service_Attribute_Value_Data[AttributeIndex].SDP_Data_Element;

         /* Begin validating that the contents of this attribute match  */
         /* was is expected for HDP. First, confirm that the Supported  */
         /* Features List is a sequence. The spec requires that this    */
         /* attribute contain at least one element, but we will be      */
         /* tolerant of implementations that do not adhere to this      */
         /* restriction.                                                */
         if(SupportedFeaturesList->SDP_Data_Element_Type == deSequence)
         {
            /* Make sure the requested Index is in range.               */
            if(Index < SupportedFeaturesList->SDP_Data_Element_Length)
            {
               MDEPDefinition = &(SupportedFeaturesList->SDP_Data_Element.SDP_Data_Element_Sequence[Index]);

               /* Validate the MCAP Definition before returning the     */
               /* count.                                                */

               /* Each MDEP Definition is a sequence of exactly three or*/
               /* four elements.                                        */
               if((MDEPDefinition->SDP_Data_Element_Type == deSequence) && ((MDEPDefinition->SDP_Data_Element_Length == 3) || (MDEPDefinition->SDP_Data_Element_Length == 4)))
               {
                  /* The first element is the MDEP ID, an 8-bit unsigned*/
                  /* integer.                                           */
                  if((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Type == deUnsignedInteger1Byte) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element_Length == sizeof(Byte_t)))
                  {
                     /* Confirm that the MDEP ID is valid.              */
                     if(VALID_MDEP_ID(MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UnsignedInteger1Byte))
                     {
                        /* The second element is the MDEP Data Type, a  */
                        /* 16-bit unsigned integer.                     */
                        if((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Type == deUnsignedInteger2Bytes) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element_Length == sizeof(Word_t)))
                        {
                           /* The third element is the MDEP Role, an    */
                           /* 8-bit unsigned integer.                   */
                           if((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element_Type == deUnsignedInteger1Byte) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element_Length == sizeof(Byte_t)))
                           {
                              /* The MDEP Role may only have a value of */
                              /* 0x00 (Source) or 0x01 (Sink).          */
                              if(MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element.UnsignedInteger1Byte <= 0x01)
                              {
                                 /* The optional fourth element is the  */
                                 /* MDEP Description. If it exists, it  */
                                 /* is a string of variable length.     */
                                 if(((MDEPDefinition->SDP_Data_Element_Length == 4) && (MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[3].SDP_Data_Element_Type == deTextString)) || (MDEPDefinition->SDP_Data_Element_Length == 3))
                                 {
                                    /* This MDEP Definition appears     */
                                    /* valid, so return it.             */
                                    MDEPInfo->MDEP_ID       = MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[0].SDP_Data_Element.UnsignedInteger1Byte;
                                    MDEPInfo->MDEP_DataType = MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[1].SDP_Data_Element.UnsignedInteger2Bytes;
                                    MDEPInfo->MDEP_Role     = ((MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[2].SDP_Data_Element.UnsignedInteger1Byte == 0x00) ? drSource : drSink);

                                    if(MDEPDefinition->SDP_Data_Element_Length == 4)
                                    {
                                       MDEPInfo->MDEP_Description = (char *)(MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[3].SDP_Data_Element.TextString);
                                       ret_val                    = MDEPDefinition->SDP_Data_Element.SDP_Data_Element_Sequence[3].SDP_Data_Element_Length;
                                    }
                                    else
                                       MDEPInfo->MDEP_Description = NULL;
                                 }
                                 else
                                 {
                                    DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The fourth element of MDEP Definition #%u is present but is not a String.\n", Index));

                                    ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                                 }
                              }
                              else
                              {
                                 DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("MDEP Definition #%u contains an invalid Role.\n", Index));

                                 ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                              }
                           }
                           else
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The third element of MDEP Definition #%u is not a UINT_8.\n", Index));

                              ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                           }
                        }
                        else
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The second element of MDEP Definition #%u is not a UINT_16.\n", Index));

                           ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                        }
                     }
                     else
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("MDEP Definition #%u contains an invalid MDEP ID.\n", Index));

                        ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The first element of MDEP Definition #%u is not a UINT_8.\n", Index));

                     ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("MDEP Definition #%u is not a sequence or contains an invalid number of elements.\n", Index));

                  ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Requested MDEP Index %u is out of range.\n", Index));

               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Supported Features attribute is not a sequence.\n"));

            ret_val = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The Supported Features attribute was not found.\n"));

         ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which determines     */
   /* the number of HDP Instance SDP records exist in the parsed SDP    */
   /* data and, optionally, extract the Instance ID from each record.   */
   /* If InstanceList is supplied and MaximumInstanceListEntries is     */
   /* non-zero, this function returns the number of Instance IDs loaded */
   /* into the List. Otherwise, this function returns zero on success.  */
   /* On error, a negative error code is returned and the contents of   */
   /* InstanceList is undefined.                                        */
static int ParseSDPInstances(DEVM_Parsed_SDP_Data_t *ParsedSDPData, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances)
{
   int                                    ret_val;
   DWord_t                                InstanceNumber;
   unsigned int                           InstanceCount;
   unsigned int                           NextRecordIndex;
   SDP_Service_Attribute_Response_Data_t *InstanceRecord;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if((ParsedSDPData) && ((MaximumInstanceListEntries == 0) || (InstanceList)))
   {
      ret_val         = 0;
      InstanceCount   = 0;
      InstanceRecord  = NULL;
      NextRecordIndex = 0;

      while((!ret_val) && (!InstanceRecord))
      {
         /* Locate the next HDP SDP record.                             */
         if((ret_val = FindSDPNextHDPRecord(ParsedSDPData, &NextRecordIndex, &InstanceRecord)) >= 0)
         {
            /* An HDP Instance record was found. Extract the Instance ID*/
            /* from the record.                                         */
            if(!ParseSDPInstanceNumber(InstanceRecord, &InstanceNumber))
            {
               /* The Instance ID was successfully extracted from the   */
               /* SDP record. If the user provided a list, record it.   */
               if((InstanceList) && (InstanceCount < MaximumInstanceListEntries))
                  InstanceList[InstanceCount] = InstanceNumber;

               /* Keep track of how many valid Instance records we have */
               /* found.                                                */
               InstanceCount++;
            }

            /* Continue the search.                                     */
            ret_val        = 0;
            InstanceRecord = NULL;
         }
         else
         {
            /* Check whether we reached the end of the set of records.  */
            if(ret_val != BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND)
            {
               /* Record parsing failed before reaching the end of the  */
               /* set of records. Ignore the current record and move to */
               /* the next. This may involve re-processing some non-HDP */
               /* records, but eventually we will move past the last    */
               /* record and end the loop.                              */
               ret_val = 0;
            }
         }
      }

      /* Check whether we reached the end of the set of records.        */
      if(ret_val == BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND)
      {
         /* We have reached the end of the set of records and no        */
         /* additional HDP records were found.  Return the number of    */
         /* records added to the caller's List and, if requested, record*/
         /* the total number of HDP records found.                      */
         ret_val = (int)((InstanceCount < MaximumInstanceListEntries) ? InstanceCount : MaximumInstanceListEntries);

         if(TotalNumberInstances)
            *TotalNumberInstances = InstanceCount;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which determines the */
   /* number of Endpoints in an HDP Instance SDP record and, optionally,*/
   /* extracts the MDEP details from each record. If EndpointInfoList is*/
   /* supplied and MaximumEndpointListEntries is non-zero, this function*/
   /* returns the number of Endpoints loaded into the List. Otherwise,  */
   /* this function returns zero on success. On error, a negative       */
   /* error code is returned and the contents of EndpointInfoList is    */
   /* undefined.                                                        */
static int ParseSDPEndpoints(DEVM_Parsed_SDP_Data_t *ParsedSDPData, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints)
{
   int                                    ret_val;
   unsigned int                           EndpointCount;
   unsigned int                           EndpointIndex;
   unsigned int                           TotalEndpoints;
   HDP_MDEP_Info_t                        MDEPInfo;
   SDP_Service_Attribute_Response_Data_t *InstanceRecord;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if((ParsedSDPData) && (((MaximumEndpointListEntries) && (EndpointInfoList)) || ((!MaximumEndpointListEntries) && (TotalNumberEndpoints))))
   {
      /* Locate the interesting SDP record.                             */
      if(!FindSDPInstance(ParsedSDPData, Instance, &InstanceRecord))
      {
         /* The record was found. Next, determine the number of         */
         /* endpoints in the record.                                    */
         if((ret_val = FindSDPNumberOfEndpoints(InstanceRecord)) >= 0)
         {
            EndpointCount  = 0;
            TotalEndpoints = (unsigned int)ret_val;

            /* Walk the set of endpoints.                               */
            for(EndpointIndex = 0;EndpointIndex < TotalEndpoints;EndpointIndex++)
            {
               /* Extract the endpoint details.                         */
               if((ret_val = GetSDPEndpointInfo(InstanceRecord, EndpointIndex, &MDEPInfo)) >= 0)
               {
                  /* Extraction was successful. If the caller requested */
                  /* it, record the details.                            */
                  if((EndpointInfoList) && (EndpointCount < MaximumEndpointListEntries))
                  {
                     EndpointInfoList[EndpointCount].EndpointID = MDEPInfo.MDEP_ID;
                     EndpointInfoList[EndpointCount].DataType   = MDEPInfo.MDEP_DataType;
                     EndpointInfoList[EndpointCount].Role       = MDEPInfo.MDEP_Role;
                  }

                  EndpointCount++;
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Endpoint #%d failed to parse: %d", EndpointIndex, ret_val));
               }
            }

            /* If at least one endpoint was correctly read, report the  */
            /* number of endpoints.                                     */
            if(EndpointCount > 0)
               ret_val = (int)((EndpointCount < MaximumEndpointListEntries) ? EndpointCount : MaximumEndpointListEntries);

            if(TotalNumberEndpoints)
               *TotalNumberEndpoints = EndpointCount;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which locates an     */
   /* endpoint in an HDP Instance SDP record based on the MDEP_ID and,  */
   /* optionally, extracts the MDEP details from the record. Because    */
   /* more than one Endpoint declaration can have the same MDEP_ID,     */
   /* the caller can supply the index into the list of endpoints at     */
   /* which to begin the search. On success, this function returns      */
   /* the index of the located Endpoint and, if provided, populates     */
   /* the EndpointInfo parameter with the details of the endpoint. If   */
   /* provided, DescriptionLength will be set to the length of the      */
   /* MDEP Description field or zero, if no description is available.   */
   /* On error, a negative error code is returned. Regardless of the    */
   /* result, StartingIndex will be set to the index *following* the    */
   /* last searched position so that it may be reused in a future call  */
   /* to this function in order to continue the search from where it    */
   /* left off.                                                         */
static int FindSDPEndpointInfo(DEVM_Parsed_SDP_Data_t *ParsedSDPData, DWord_t Instance, unsigned int *StartingIndex, Byte_t MDEP_ID, HDP_MDEP_Info_t *MDEPInfo, unsigned int *DescriptionLength)
{
   int                                    ret_val;
   unsigned int                           TotalEndpoints;
   HDP_MDEP_Info_t                        CurrentMDEPInfo;
   SDP_Service_Attribute_Response_Data_t *InstanceRecord;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Validate parameters.                                              */
   if((ParsedSDPData) && (StartingIndex))
   {
      /* Locate the interesting SDP record.                             */
      if(!FindSDPInstance(ParsedSDPData, Instance, &InstanceRecord))
      {
         /* We found the target record. Next, determine the number of   */
         /* endpoints in the record.                                    */
         if((ret_val = FindSDPNumberOfEndpoints(InstanceRecord)) >= 0)
         {
            TotalEndpoints  = (unsigned int)ret_val;

            /* Walk the set of endpoints.                               */
            for(;(*StartingIndex < TotalEndpoints);*StartingIndex += 1)
            {
               /* Extract the endpoint details.                         */
               if((ret_val = GetSDPEndpointInfo(InstanceRecord, *StartingIndex, &CurrentMDEPInfo)) >= 0)
               {
                  /* Check if this is the endpoint the caller asked for.*/
                  if(CurrentMDEPInfo.MDEP_ID == MDEP_ID)
                  {
                     /* The target endpoint was found, so drop out of   */
                     /* the loop.                                       */
                     break;
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Endpoint #%d failed to parse: %d", *StartingIndex, ret_val));
               }
            }

            /* If the target endpoint was found, record the endpoint    */
            /* details and return success.                              */
            if(*StartingIndex < TotalEndpoints)
            {
               if(MDEPInfo)
               {
                  *MDEPInfo = CurrentMDEPInfo;

                  if(DescriptionLength)
                     *DescriptionLength = (unsigned int)ret_val;
               }

               ret_val         = *StartingIndex;

               /* Increment Starting Index if the target was found. The */
               /* index is set to 1 greater than the target's.          */
               *StartingIndex += 1;
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The endpoint was not found in the Instance record.\n"));

               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND;
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("The endpoint list could not be located or was invalid.\n"));

            ret_val = BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which retrieves the  */
   /* parsed SDP service details for a remote device from the local     */
   /* cache. On success, this function returns zero and ParsedSDPData   */
   /* will point to the parsed SDP structure tree. This pointer must be */
   /* passed to FreeParsedSDPData when the caller is finished with it.  */
static int GetParsedSDPData(BD_ADDR_t RemoteDeviceAddress, DEVM_Parsed_SDP_Data_t **ParsedSDPData)
{
   int               Result;
   unsigned int      TotalBytesSDPData;
   _ParsedSDPData_t *DataPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ParsedSDPData))
   {
      if((DataPtr = (_ParsedSDPData_t *)BTPS_AllocateMemory(sizeof(_ParsedSDPData_t))) != NULL)
      {
         BTPS_MemInitialize(DataPtr, 0, sizeof(_ParsedSDPData_t));

         DataPtr->RESERVED = HDPM_PARSED_SDP_DATA_MAGIC;

         if((Result = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, 0, 0, NULL, &TotalBytesSDPData)) >= 0)
         {
            /* Allocate storage for the raw SDP data.                   */
            if((DataPtr->RawSDPData = (unsigned char *)BTPS_AllocateMemory(TotalBytesSDPData)) != NULL)
            {
               /* Load cached SDP data into the buffer.                 */
               if((Result = DEVM_QueryRemoteDeviceServices(RemoteDeviceAddress, 0, TotalBytesSDPData, DataPtr->RawSDPData, &TotalBytesSDPData)) >= 0)
               {
                  /* Double-check that we received all the available    */
                  /* data.                                              */
                  if((unsigned int)Result == TotalBytesSDPData)
                  {
                     /* Parse the SDP data into a structured form.      */
                     if((Result = DEVM_ConvertRawSDPStreamToParsedSDPData(TotalBytesSDPData, DataPtr->RawSDPData, &(DataPtr->ParsedSDPData))) == 0)
                        *ParsedSDPData = &(DataPtr->ParsedSDPData);
                  }
                  else
                     Result = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }

         if(Result < 0)
         {
            if(DataPtr->RawSDPData)
               BTPS_FreeMemory(DataPtr->RawSDPData);

            BTPS_FreeMemory(DataPtr);
         }
      }
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      Result = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", Result));

   return(Result);
}

   /* The following function is a utility function which cleans         */
   /* up parsed SDP data and supporting data structures for those       */
   /* DEVM_Parsed_SDP_Data_t structures allocated by GetParsedSDPData.  */
static void FreeParsedSDPData(DEVM_Parsed_SDP_Data_t *ParsedSDPData)
{
   _ParsedSDPData_t *DataPtr;

   if(ParsedSDPData)
   {
      DataPtr = (_ParsedSDPData_t *)ParsedSDPData;

      if(DataPtr->RESERVED == HDPM_PARSED_SDP_DATA_MAGIC)
      {
         if(DataPtr->RawSDPData)
         {
            DEVM_FreeParsedSDPData(&(DataPtr->ParsedSDPData));

            BTPS_FreeMemory(DataPtr->RawSDPData);
         }

         BTPS_MemInitialize(DataPtr, 0, sizeof(_ParsedSDPData_t));

         BTPS_FreeMemory(DataPtr);
      }
   }
}

   /* The following function dispatches an HDP Instance Connection      */
   /* Status event.  This is an internal function and assumes that the  */
   /* caller checked and has verified the input parameters before this  */
   /* function was called.                                              */
static void DispatchControlConnectConfirmationEvent(Connection_Entry_t *ConnectionPtr, int ConnectionStatus)
{
   HDPM_Event_Data_t                Event;
   HDPM_Connection_Status_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Connection Status: %d\n", ConnectionStatus));

   /* Determine whether the notification is going to a local callback or*/
   /* remote client.                                                    */
   if(ConnectionPtr->EventCallback.CallbackFunction)
   {
      /* A local callback is registered.                                */
      BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

      Event.EventType                                               = hetHDPConnectionStatus;
      Event.EventLength                                             = HDPM_CONNECTION_STATUS_EVENT_DATA_SIZE;

      Event.EventData.ConnectionStatusEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
      Event.EventData.ConnectionStatusEventData.Instance            = ConnectionPtr->Instance;
      Event.EventData.ConnectionStatusEventData.Status              = ConnectionStatus;

      /* Release the Lock so we can make the callback.                  */
      DEVM_ReleaseLock();

      __BTPSTRY
      {
         if(ConnectionPtr->EventCallback.CallbackFunction)
            (ConnectionPtr->EventCallback.CallbackFunction)(&Event, ConnectionPtr->EventCallback.CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }

      /* Re-acquire the Lock.                                           */
      DEVM_AcquireLock();
   }
   else
   {
      /* This connection is owned by a remote client.                   */
      BTPS_MemInitialize(&Message, 0, HDPM_CONNECTION_STATUS_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = ConnectionPtr->EventCallback.ClientID;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
      Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_CONNECTION_STATUS;
      Message.MessageHeader.MessageLength   = (HDPM_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
      Message.Instance                      = ConnectionPtr->Instance;
      Message.Status                        = ConnectionStatus;

      /* Finally dispatch the Message.                                  */
      MSG_SendMessage((BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRegisterEndpointMessage(HDPM_Register_Endpoint_Request_t *Message)
{
   int                                Result;
   Endpoint_Entry_t                   EndpointEntry;
   Endpoint_Entry_t                  *EndpointEntryPtr;
   HDPM_Register_Endpoint_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_REGISTER_ENDPOINT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* First, register the handler locally.                           */
      BTPS_MemInitialize(&EndpointEntry, 0, sizeof(Endpoint_Entry_t));

      EndpointEntry.EventCallback.ClientID         = Message->MessageHeader.AddressID;

      EndpointEntry.MDEP_DataType                  = Message->DataType;
      EndpointEntry.MDEP_Role                      = Message->LocalRole;

      /* XXX                                                            */
      /* Currently, we support only one endpoint for each possible      */
      /* combination of Data Type and Role. This could be enhanced by   */
      /* either:                                                        */
      /*     - allowing only one Type/Role pair per _client_ and        */
      /*       multiplexing incoming data connections according to which*/
      /*       client claimed the Control Channel when the remote device*/
      /*       initially connected to our local instance.               */
      /*     - allowing clients to register separate instance and       */
      /*       restricting them to one Type/Role pair per instance.     */

      /* Check whether an endpoint with this Type and Role has already  */
      /* been registered on this local Instance.                        */
      if(SearchEndpointEntryTypeRole(&EndpointList, Message->DataType, Message->LocalRole) == NULL)
      {
         /* Force the description string to be terminated properly.     */
         Message->Description[Message->DescriptionLength - 1] = '\0';

         /* Attempt to register the new endpoint with the HDP framework.*/
         if((Result = _HDPM_Register_Endpoint(EndpointEntry.MDEP_DataType, EndpointEntry.MDEP_Role, Message->Description)) > 0)
         {
            EndpointEntry.MDEP_ID = (Byte_t)Result;

            /* Successfully registered. Now attempt to add it to the    */
            /* registered endpoint list. This should always succeed     */
            /* because the framework always assigns unique MDEP_IDs.    */
            if((EndpointEntryPtr = AddEndpointEntry(&EndpointList, &EndpointEntry)) == NULL)
            {
               /* Registration unexpectedly failed.                     */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to add Endpoint Registration entry to the list for MDEP: %d\n", EndpointEntry.MDEP_ID));

               /* Remove the new endpoint registration.                 */
               _HDPM_Un_Register_Endpoint(EndpointEntry.MDEP_ID, EndpointEntry.MDEP_DataType, EndpointEntry.MDEP_Role);

               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
         }
         else
            Result = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_ENDPOINT;
      }
      else
         Result = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_ENDPOINT;

      ResponseMessage.Status = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessUnRegisterEndpointMessage(HDPM_Un_Register_Endpoint_Request_t *Message)
{
   int                                Result;
   Endpoint_Entry_t                  *EndpointEntryPtr;
   HDPM_Register_Endpoint_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* XXX                                                            */
      /* A more careful search may be required here if we eventually    */
      /* support hosting multiple data-types on one MDEP.               */

      /* First, confirm that the client requesting to be un-registered  */
      /* is the same that created this registration.                    */
      EndpointEntryPtr = SearchEndpointEntryMDEP(&EndpointList, (Byte_t)Message->EndpointID);

      if((EndpointEntryPtr) && (EndpointEntryPtr->EventCallback.ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */

         /* Go ahead and initialize the Response.                       */
         ResponseMessage.MessageHeader                = Message->MessageHeader;
         ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
         ResponseMessage.MessageHeader.MessageLength  = HDPM_UN_REGISTER_ENDPOINT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         /* Remove the endpoint registration from the list.             */
         if((EndpointEntryPtr = DeleteEndpointEntryMDEP(&EndpointList, (Byte_t)Message->EndpointID)) != NULL)
         {
            /* XXX                                                      */
            /* If support for multiplexing endpoints between multiple PM*/
            /* clients is added, we must check here whether the MDEP is */
            /* still in use before unregistering it.                    */

            /* This HDP endpoint is no longer in use, so it is safe to  */
            /* unregister it.                                           */
            Result = _HDPM_Un_Register_Endpoint(EndpointEntryPtr->MDEP_ID, EndpointEntryPtr->MDEP_DataType, EndpointEntryPtr->MDEP_Role);

            if(Result)
            {
               /* If an error occurs in HDP while un-registering the    */
               /* endpoint, log the error but ignore it since the       */
               /* client's HDPM registration has already been removed   */
               /* successfully.                                         */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to unregister MDEP from HDP (%d)\n", Result));
            }

            /* The endpoint registration can now be deleted.            */
            FreeEndpointEntryMemory(EndpointEntryPtr);

            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_REGISTERED;

      ResponseMessage.Status = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDataConnectionRequestResponseMessage(HDPM_Data_Connection_Request_Response_Request_t *Message)
{
   int                                               Result;
   Data_Channel_Entry_t                             *DataEntryPtr;
   HDP_Channel_Config_Info_t                         ConfigInfo;
   HDPM_Data_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if((Message->DataLinkID) && (((Message->ResponseCode == MCAP_RESPONSE_CODE_SUCCESS) && ((Message->ChannelMode == cmReliable) || (Message->ChannelMode == cmStreaming))) || ((Message->ResponseCode > MCAP_RESPONSE_CODE_SUCCESS)  && (Message->ResponseCode <= MCAP_RESPONSE_CODE_CONFIGURATION_REJECTED))))
         {
            /* Next, verify that we are already tracking the referenced */
            /* data channel and that the caller has permission to accept*/
            /* this connection.                                         */
            if(((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, Message->DataLinkID)) != NULL) && (DataEntryPtr->EventCallback.ClientID == Message->MessageHeader.AddressID))
            {
               /* Verify that the data channel is pending authorization */
               /* and that this client is currently permitted to        */
               /* authorize the connection.                             */
               if(DataEntryPtr->ConnectionState == dcsAuthorizing)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Data Channel Request: %d, %d\n", Message->DataLinkID, Message->ResponseCode));

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(Message->ResponseCode == MCAP_RESPONSE_CODE_SUCCESS)
                  {
                     /* Map this Data Channel back to the Control       */
                     /* Connection.                                     */
                     if(SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID))
                     {
                        /* Enable FCS if we are the Source.             */
                        ConfigInfo.FCSMode                  = ((DataEntryPtr->LocalRole == drSource) ? fcsEnabled : fcsNoPreference);
                        ConfigInfo.MaxTxPacketSize          = 2048;
                        ConfigInfo.TxSegmentSize            = 256;
                        ConfigInfo.NumberOfTxSegmentBuffers = 10;

                        /* Accept the data connection.                  */
                        if((Result = _HDPM_Data_Connection_Request_Response(DataEntryPtr->DataLinkID, MCAP_RESPONSE_CODE_SUCCESS, Message->ChannelMode, &ConfigInfo)) == 0)
                        {
                           /* XXX                                       */
                           /* For client multiplexing: If timers are    */
                           /* used to protect against the case where no */
                           /* client accepts the connection, clean them */
                           /* up, here.                                 */

                           /* Update the current data channel state.    */
                           DataEntryPtr->ConnectionState = dcsConnecting;
                        }
                     }
                     else
                     {
                        /* No associated control channel was found. This*/
                        /* should never happen because Bluetopia should */
                        /* have already rejected the connection attempt.*/
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Received data channel connection request without associated control channel.\n"));

                        /* Reject the request.                          */
                        Result = _HDPM_Data_Connection_Request_Response(Message->DataLinkID, MCAP_RESPONSE_CODE_INVALID_OPERATION, cmNoPreference, NULL);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        /* XXX                                          */
                        /* For client multiplexing on endpoints, the    */
                        /* data channel object should _not_ be deleted  */
                        /* until all clients have had a chance to claim */
                        /* the control connection.                      */
                        if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                           FreeDataChannelEntryMemory(DataEntryPtr);
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     Result = _HDPM_Data_Connection_Request_Response(Message->DataLinkID, Message->ResponseCode, cmNoPreference, NULL);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                        FreeDataChannelEntryMemory(DataEntryPtr);
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
            }
            else
               Result = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_DATALINK_ID;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_DATA_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessQueryRemoteDeviceInstancesMessage(HDPM_Query_Remote_Device_Instances_Request_t *Message)
{
   int                                            Result;
   unsigned int                                   TotalNumberInstances;
   DEVM_Parsed_SDP_Data_t                        *ParsedSDPData;
   HDPM_Query_Remote_Device_Instances_Response_t  ErrorResponseMessage;
   HDPM_Query_Remote_Device_Instances_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;
      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ErrorResponseMessage.MessageHeader.MessageLength  = HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
      ErrorResponseMessage.Status                       = 0;
      ErrorResponseMessage.NumberInstances              = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Load cached SDP data into the buffer.                    */
            if((Result = GetParsedSDPData(Message->RemoteDeviceAddress, &ParsedSDPData)) == 0)
            {
               /* Determine the number of published HDP instances.      */
               if((Result = ParseSDPInstances(ParsedSDPData, 0, NULL, &TotalNumberInstances)) >= 0)
               {
                  /* Now let's attempt to allocate memory to hold the   */
                  /* entire list of instances.                          */
                  if((ResponseMessage = (HDPM_Query_Remote_Device_Instances_Response_t *)BTPS_AllocateMemory(HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(TotalNumberInstances))) != NULL)
                  {
                     BTPS_MemInitialize(ResponseMessage, 0, HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(TotalNumberInstances));

                     /* Memory allocated, now let's build the response  */
                     /* message.                                        */
                     /* * NOTE * Error Response has initialized all     */
                     /*          values to known values (i.e. zero      */
                     /*          instances and success).                */
                     BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(0));

                     /* Extract the Instance IDs, storing them in the   */
                     /* response message.                               */
                     if((Result = ParseSDPInstances(ParsedSDPData, TotalNumberInstances, ResponseMessage->InstanceList, &TotalNumberInstances)) >= 0)
                     {
                        ResponseMessage->NumberInstances = (unsigned int)Result;

                        /* Now that we are finished we need to update   */
                        /* the length.                                  */
                        ResponseMessage->MessageHeader.MessageLength  = HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(ResponseMessage->NumberInstances) - BTPM_MESSAGE_HEADER_SIZE;

                        /* Response Message built, go ahead and send it.*/
                        MSG_SendMessage((BTPM_Message_t *)ResponseMessage);
                     }

                     /* Free the response message memory because we are */
                     /* finished with it.                               */
                     BTPS_FreeMemory(ResponseMessage);
                  }
                  else
                  {
                     ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

                     MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                  }
               }
               else
               {
                  ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_PARSE_SERVICE_DATA;

                  MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
               }

               /* Release the parsed SDP data structures.               */
               FreeParsedSDPData(ParsedSDPData);
            }
            else
            {
               ErrorResponseMessage.Status = Result;

               MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
            }
         }
         else
         {
            ErrorResponseMessage.Status = BTPM_ERROR_CODE_INVALID_PARAMETER;

            MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
         }
      }
      else
      {
         ErrorResponseMessage.Status = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessQueryRemoteDeviceEndpointsMessage(HDPM_Query_Remote_Device_Endpoints_Request_t *Message)
{
   int                                            Result;
   unsigned int                                   TotalNumberEndpoints;
   DEVM_Parsed_SDP_Data_t                        *ParsedSDPData;
   HDPM_Query_Remote_Device_Endpoints_Response_t  ErrorResponseMessage;
   HDPM_Query_Remote_Device_Endpoints_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;
      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ErrorResponseMessage.MessageHeader.MessageLength  = HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
      ErrorResponseMessage.Status                       = 0;
      ErrorResponseMessage.NumberEndpoints              = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if(!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress))
         {
            /* Load cached SDP data into the buffer.                    */
            if((Result = GetParsedSDPData(Message->RemoteDeviceAddress, &ParsedSDPData)) == 0)
            {
               /* Determine the number of published HDP Endpoints for   */
               /* this instance.                                        */
               if((Result = ParseSDPEndpoints(ParsedSDPData, Message->Instance, 0, NULL, &TotalNumberEndpoints)) >= 0)
               {
                  /* Now let's attempt to allocate memory to hold the   */
                  /* entire list of endpoints.                          */
                  if((ResponseMessage = (HDPM_Query_Remote_Device_Endpoints_Response_t *)BTPS_AllocateMemory(HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(TotalNumberEndpoints))) != NULL)
                  {
                     BTPS_MemInitialize(ResponseMessage, 0, HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(TotalNumberEndpoints));

                     /* Memory allocated, now let's build the response  */
                     /* message.                                        */
                     /* * NOTE * Error Response has initialized all     */
                     /*          values to known values (i.e. zero      */
                     /*          endpoints and success).                */
                     BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(0));

                     /* Extract the Endpoint IDs, storing them in the   */
                     /* response message.                               */
                     if((Result = ParseSDPEndpoints(ParsedSDPData, Message->Instance, TotalNumberEndpoints, ResponseMessage->EndpointList, &TotalNumberEndpoints)) >= 0)
                     {
                        ResponseMessage->NumberEndpoints = (unsigned int)Result;

                        /* Now that we are finished we need to update   */
                        /* the length.                                  */
                        ResponseMessage->MessageHeader.MessageLength  = HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(ResponseMessage->NumberEndpoints) - BTPM_MESSAGE_HEADER_SIZE;

                        /* Response Message built, go ahead and send it.*/
                        MSG_SendMessage((BTPM_Message_t *)ResponseMessage);
                     }
                     else
                     {
                        ErrorResponseMessage.Status = Result;

                        MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                     }

                     /* Free the response message memory because we are */
                     /* finished with it.                               */
                     BTPS_FreeMemory(ResponseMessage);
                  }
                  else
                  {
                     ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

                     MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                  }
               }
               else
               {
                  ErrorResponseMessage.Status = Result;

                  MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
               }

               /* Release the parsed SDP data structures.               */
               FreeParsedSDPData(ParsedSDPData);
            }
            else
            {
               ErrorResponseMessage.Status = Result;

               MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
            }
         }
         else
         {
            ErrorResponseMessage.Status = BTPM_ERROR_CODE_INVALID_PARAMETER;

            MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
         }
      }
      else
      {
         ErrorResponseMessage.Status = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessQueryEndpointDescriptionMessage(HDPM_Query_Endpoint_Description_Request_t *Message)
{
   int                                         Result;
   unsigned int                                DescriptionLength;
   unsigned int                                NextEndpointIndex;
   unsigned int                                TotalBytesSDPData;
   unsigned char                              *RawSDPData;
   HDP_MDEP_Info_t                             MDEPInfo;
   DEVM_Parsed_SDP_Data_t                      ParsedSDPData;
   HDPM_Query_Endpoint_Description_Response_t  ErrorResponseMessage;
   HDPM_Query_Endpoint_Description_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;
      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ErrorResponseMessage.MessageHeader.MessageLength  = HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
      ErrorResponseMessage.Status                       = 0;
      ErrorResponseMessage.DescriptionLength            = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (VALID_INSTANCE(Message->Instance)) && (VALID_MDEP_ID(Message->EndpointInfo.EndpointID)) && (Message->EndpointInfo.DataType))
         {
            /* Parameters appear valid, so now determine the size of the*/
            /* cached SDP data for this device.                         */
            if((Result = DEVM_QueryRemoteDeviceServices(Message->RemoteDeviceAddress, 0, 0, NULL, &TotalBytesSDPData)) >= 0)
            {
               /* Allocate storage for the raw SDP data.                */
               if((RawSDPData = (unsigned char *)BTPS_AllocateMemory(TotalBytesSDPData)) != NULL)
               {
                  /* Load cached SDP data into the buffer.              */
                  if((Result = DEVM_QueryRemoteDeviceServices(Message->RemoteDeviceAddress, 0, TotalBytesSDPData, RawSDPData, &TotalBytesSDPData)) >= 0)
                  {
                     /* Double-check that we received all the available */
                     /* data.                                           */
                     if((unsigned int)Result == TotalBytesSDPData)
                     {
                        /* Parse the SDP data into a structured form.   */
                        if((Result = DEVM_ConvertRawSDPStreamToParsedSDPData(TotalBytesSDPData, RawSDPData, &ParsedSDPData)) == 0)
                        {
                           Result            = 0;
                           NextEndpointIndex = 0;
                           DescriptionLength = 0;

                           /* Walk the list of endpoints to find the    */
                           /* requested MDEP definition.                */
                           while(Result >= 0)
                           {
                              /* Locate the next Endpoint declaration.  */
                              if((Result = FindSDPEndpointInfo(&ParsedSDPData, Message->Instance, &NextEndpointIndex, Message->EndpointInfo.EndpointID, &MDEPInfo, &DescriptionLength)) >= 0)
                              {
                                 /* An endpoint declaration was found.  */
                                 /* Determine whether this is the       */
                                 /* requested endpoint.                 */
                                 if(MDEPInfo.MDEP_DataType == Message->EndpointInfo.DataType)
                                 {
                                    /* It is the requested endpoint.    */
                                    /* End the search.                  */
                                    break;
                                 }
                              }
                           }

                           if(Result >= 0)
                           {
                              /* The target endpoint was found, so      */
                              /* allocate the response message.         */
                              if((ResponseMessage = (HDPM_Query_Endpoint_Description_Response_t *)BTPS_AllocateMemory(HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(DescriptionLength))) != NULL)
                              {
                                 BTPS_MemInitialize(ResponseMessage, 0, HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(DescriptionLength));

                                 /* Memory allocated, now let's build   */
                                 /* the response message.               */
                                 /* * NOTE * Error Response has         */
                                 /*          initialized all values to  */
                                 /*          known values (i.e. zero    */
                                 /*          endpoints and success).    */
                                 BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(0));

                                 ResponseMessage->DescriptionLength = DescriptionLength;

                                 if(DescriptionLength > 0)
                                    BTPS_MemCopy(ResponseMessage->DescriptionBuffer, MDEPInfo.MDEP_Description, DescriptionLength);

                                 ResponseMessage->Status            = 0;

                                 /* Now that we are finished we need to */
                                 /* update the length.                  */
                                 ResponseMessage->MessageHeader.MessageLength = HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(ResponseMessage->DescriptionLength) - BTPM_MESSAGE_HEADER_SIZE;

                                 /* Response Message built, go ahead and*/
                                 /* send it.                            */
                                 MSG_SendMessage((BTPM_Message_t *)ResponseMessage);
                              }
                              else
                              {
                                 ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

                                 MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                              }
                           }
                           else
                           {
                              ErrorResponseMessage.Status = Result;

                              MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                           }

                           /* Release the parsed SDP data structures.   */
                           DEVM_FreeParsedSDPData(&ParsedSDPData);
                        }
                        else
                        {
                           ErrorResponseMessage.Status = Result;

                           MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                        }
                     }
                     else
                     {
                        ErrorResponseMessage.Status = BTPM_ERROR_CODE_SERVICE_DATA_INVALID;

                        MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                     }
                  }
                  else
                  {
                     ErrorResponseMessage.Status = Result;

                     MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
                  }

                  /* Free the response message memory because we are    */
                  /* finished with it.                                  */
                  BTPS_FreeMemory(RawSDPData);
               }
               else
               {
                  ErrorResponseMessage.Status = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

                  MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
               }
            }
            else
            {
               ErrorResponseMessage.Status = Result;

               MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
            }
         }
         else
         {
            ErrorResponseMessage.Status = BTPM_ERROR_CODE_INVALID_PARAMETER;

            MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
         }
      }
      else
      {
         ErrorResponseMessage.Status = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectRemoteDeviceMessage(HDPM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   Connection_Entry_t                     ConnectionEntry;
   Connection_Entry_t                    *ConnectionEntryPtr;
   HDPM_Connect_Remote_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (VALID_INSTANCE(Message->Instance)))
         {
            /* Next, verify that we are not already tracking a          */
            /* connection to the specified device.                      */
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionList, Message->RemoteDeviceAddress, Message->Instance)) == NULL)
            {
               /* Entry is not present, go ahead and create a new entry.*/
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.BD_ADDR                = Message->RemoteDeviceAddress;
               ConnectionEntry.Instance               = Message->Instance;
               ConnectionEntry.MCLID                  = 0;
               ConnectionEntry.Server                 = FALSE;
               ConnectionEntry.EventCallback.ClientID = Message->MessageHeader.AddressID;
               ConnectionEntry.ConnectionState        = csIdle;

               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionList, &ConnectionEntry)) != NULL)
               {
                  /* If we are not currently connected to the remote    */
                  /* device, do so now.                                 */
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08x\n", ConnectionEntry.BD_ADDR.BD_ADDR5, ConnectionEntry.BD_ADDR.BD_ADDR4, ConnectionEntry.BD_ADDR.BD_ADDR3, ConnectionEntry.BD_ADDR.BD_ADDR2, ConnectionEntry.BD_ADDR.BD_ADDR1, ConnectionEntry.BD_ADDR.BD_ADDR0, ConnectionEntry.Instance));

                  /* Next, attempt to open the remote device            */
                  ConnectionEntryPtr->ConnectionState = csConnectingDevice;

                  Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT));

                  if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     /* Check to see if we need to actually issue the   */
                     /* Remote connection.                              */
                     if(Result < 0)
                     {
                        /* Set the state to connecting the profile.     */
                        ConnectionEntryPtr->ConnectionState = csConnecting;

                        if((Result = _HDPM_Connect_Remote_Instance(ConnectionEntryPtr->BD_ADDR, ConnectionEntryPtr->Instance)) <= 0)
                        {
                           Result = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;

                           /* Error opening device, go ahead and delete */
                           /* the entry that was added.                 */
                           if((ConnectionEntryPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionEntryPtr)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntryPtr);
                        }
                        else
                        {
                           /* Note the MCLID.                           */
                           ConnectionEntryPtr->MCLID = (unsigned int)Result;

                           /* Flag success.                             */
                           Result                    = 0;
                        }
                     }
                  }
                  else
                  {
                     /* Error connecting to the device, go ahead and    */
                     /* delete the entry that was added.                */
                     if((ConnectionEntryPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionEntryPtr)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
            {
               if(ConnectionEntryPtr->EventCallback.ClientID == Message->MessageHeader.AddressID)
               {
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                     Result = BTPM_ERROR_CODE_HEALTH_DEVICE_INSTANCE_ALREADY_CONNECTED;
                  else
                     Result = BTPM_ERROR_CODE_HEALTH_DEVICE_CONNECTION_IN_PROGRESS;
               }
               else
                  Result = BTPM_ERROR_CODE_HEALTH_DEVICE_REMOTE_INSTANCE_IN_USE;
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisconnectRemoteDeviceMessage(HDPM_Disconnect_Remote_Device_Request_t *Message)
{
   int                                       Result;
   Connection_Entry_t                       *ConnectionPtr;
   HDPM_Disconnect_Remote_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Go ahead and process the message request.                   */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to disconnect remote device: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08X\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0, Message->Instance));

         /* Verify that we are tracking the connection and that the     */
         /* client who requested the disconnection has permission to do */
         /* so (owns the connection).                                   */
         if(((ConnectionPtr = SearchConnectionEntry(&ConnectionList, Message->RemoteDeviceAddress, Message->Instance)) != NULL) && (ConnectionPtr->EventCallback.ClientID == Message->MessageHeader.AddressID))
         {
            /* Nothing really to do other than to disconnect the device */
            /* (if it is connected, a disconnect will be dispatched from*/
            /* the framework).                                          */
            Result = _HDPM_Disconnect_Remote_Instance(ConnectionPtr->MCLID);

            /* No need to clean up associated data channels. The        */
            /* framework will automatically close them and issue        */
            /* disconnect events before closing the control channel.    */

            /* XXX Do we need to fake a "Disconnected" message for      */
            /* the control channel? Based on a quick code review, the   */
            /* framework appears to issue events for the data channels, */
            /* but possibly not the control channel.                    */
         }
         else
            Result = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_DISCONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectRemoteDeviceEndpointMessage(HDPM_Connect_Remote_Device_Endpoint_Request_t *Message)
{
   int                                             Result;
   unsigned int                                    Index;
   unsigned int                                    TotalNumberEndpoints;
   Connection_Entry_t                             *ConnectionEntryPtr;
   Data_Channel_Entry_t                            DataEntry;
   Data_Channel_Entry_t                           *DataEntryPtr;
   HDPM_Endpoint_Info_t                           *EndpointInfoList;
   DEVM_Parsed_SDP_Data_t                         *ParsedSDPData;
   HDP_Channel_Config_Info_t                       ConfigInfo;
   HDPM_Connect_Remote_Device_Endpoint_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if((!COMPARE_NULL_BD_ADDR(Message->RemoteDeviceAddress)) && (VALID_INSTANCE(Message->Instance)) && (VALID_MDEP_ID(Message->EndpointID)) && ((Message->ChannelMode == cmNoPreference) || (Message->ChannelMode == cmReliable) || (Message->ChannelMode == cmStreaming)))
         {
            /* Next, verify that we are not already tracking a          */
            /* connection to the specified device.                      */
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionList, Message->RemoteDeviceAddress, Message->Instance)) != NULL)
            {
               /* A client may manage Data Channel connections only on  */
               /* Instance connections which the client owns.           */
               if(ConnectionEntryPtr->EventCallback.ClientID == Message->MessageHeader.AddressID)
               {
                  /* Check that the instance connection is in a state   */
                  /* which support endpoint connections.                */
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                  {
                     /* Build the Data Channel Entry for this request.  */
                     BTPS_MemInitialize(&DataEntry, 0, sizeof(Data_Channel_Entry_t));

                     DataEntry.MCLID                  = ConnectionEntryPtr->MCLID;
                     DataEntry.DataLinkID             = 0;
                     DataEntry.MDEP_ID                = Message->EndpointID;
                     DataEntry.ChannelMode            = Message->ChannelMode;
                     DataEntry.ConnectionState        = dcsAuthorizing;
                     DataEntry.EventCallback.ClientID = Message->MessageHeader.AddressID;

                     /* Resolve whether we are connecting to a Sink or a*/
                     /* Source.  First, just try to find another        */
                     /* connection to that same MDEP and copy that --   */
                     /* the spec requires that each MDEP support one and*/
                     /* only one role.                                  */
                     if((DataEntryPtr = SearchDataChannelEntryMCLID_MDEP(&DataChannelList, DataEntry.MCLID, DataEntry.MDEP_ID)) != NULL)
                     {
                        /* Success - another data channel to this same  */
                        /* MDEP over the same connection was found.     */
                        DataEntry.LocalRole = DataEntryPtr->LocalRole;

                        Result              = 0;
                     }
                     else
                     {
                        /* This is the first data channel opened to this*/
                        /* MDEP for this connection. We'll have to parse*/
                        /* SDP records.                                 */

                        /* Load cached SDP data into the buffer.        */
                        if((Result = GetParsedSDPData(ConnectionEntryPtr->BD_ADDR, &ParsedSDPData)) == 0)
                        {
                           /* Determine the number of published HDP     */
                           /* Endpoints for this instance.              */
                           if((Result = ParseSDPEndpoints(ParsedSDPData, ConnectionEntryPtr->Instance, 0, NULL, &TotalNumberEndpoints)) >= 0)
                           {
                              /* Allocate space for the endpoint list.  */
                              if((EndpointInfoList = (HDPM_Endpoint_Info_t *)BTPS_AllocateMemory(HDPM_ENDPOINT_INFO_DATA_SIZE * TotalNumberEndpoints)) != NULL)
                              {
                                 /* Now get the actual endpoint data.   */
                                 if((Result = ParseSDPEndpoints(ParsedSDPData, ConnectionEntryPtr->Instance, TotalNumberEndpoints, EndpointInfoList, NULL)) >= 0)
                                 {
                                    /* Scan the endpoint list for our   */
                                    /* target MDEP.                     */
                                    for(Index=0;Index < (unsigned int)Result;Index++)
                                    {
                                       if(EndpointInfoList[Index].EndpointID == DataEntry.MDEP_ID)
                                       {
                                          /* Found an entry for this    */
                                          /* MDEP, so we now know the   */
                                          /* remote role. The local role*/
                                          /* will be the opposite.      */
                                          DataEntry.LocalRole = ((EndpointInfoList[Index].Role == drSink) ? drSource : drSink);
                                          break;
                                       }
                                    }

                                    /* Check whether a role was found.  */
                                    if(Index < (unsigned int)Result)
                                    {
                                       /* Found.                        */
                                       Result = 0;
                                    }
                                    else
                                    {
                                       /* Not found. Report the error.  */
                                       Result = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;
                                    }
                                 }

                                 /* Clean up the allocated space.       */
                                 BTPS_FreeMemory(EndpointInfoList);
                              }
                              else
                                 Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                           }

                           /* Release the parsed SDP data structures.   */
                           FreeParsedSDPData(ParsedSDPData);
                        }
                     }


                     if(Result >= 0)
                     {
                        if((DataEntryPtr = AddDataChannelEntry(&DataChannelList, &DataEntry)) != NULL)
                        {
                           /* Enable FCS if we are the Source.          */
                           ConfigInfo.FCSMode                  = (HDP_FCS_Mode_t)((DataEntryPtr->LocalRole == drSource)?fcsEnabled:fcsNoPreference);
                           ConfigInfo.MaxTxPacketSize          = 2048;
                           ConfigInfo.TxSegmentSize            = 256;
                           ConfigInfo.NumberOfTxSegmentBuffers = 10;

                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connecting data channel: %d, %d, %d\n", DataEntryPtr->MCLID, DataEntryPtr->LocalRole, DataEntryPtr->ChannelMode));

                           if((Result = _HDPM_Connect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->MDEP_ID, DataEntryPtr->LocalRole, DataEntryPtr->ChannelMode, &ConfigInfo)) > 0)
                           {
                              /* Note the Health Device DataLinkID.     */
                              DataEntryPtr->DataLinkID = (unsigned int)Result;
                           }
                           else
                           {
                              /* An error occurred, so clean up the Data*/
                              /* Channel Entry that was added to the    */
                              /* list.                                  */
                              DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                              FreeDataChannelEntryMemory(DataEntryPtr);
                           }
                        }
                        else
                           Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_HEALTH_DEVICE_CONNECTION_IN_PROGRESS;
               }
               else
                  Result = BTPM_ERROR_CODE_HEALTH_DEVICE_REMOTE_INSTANCE_IN_USE;
            }
            else
               Result = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection attempt result: %d\n", Result));

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisconnectRemoteDeviceEndpointMessage(HDPM_Disconnect_Remote_Device_Endpoint_Request_t *Message)
{
   int                                                Result;
   Connection_Entry_t                                *ConnectionEntryPtr;
   Data_Channel_Entry_t                              *DataEntryPtr;
   HDPM_Disconnect_Remote_Device_Endpoint_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if(Message->DataLinkID)
         {
            if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, Message->DataLinkID)) != NULL)
            {
               /* Obtain the connection entry for this data link.       */
               if((ConnectionEntryPtr = SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID)) != NULL)
               {
                  /* A client may manage Data Channel connections only  */
                  /* on Instance connections which the client owns.     */
                  if(ConnectionEntryPtr->EventCallback.ClientID == Message->MessageHeader.AddressID)
                  {
                     switch(DataEntryPtr->ConnectionState)
                     {
                        case dcsAuthorizing:
                           if(ConnectionEntryPtr->Server)
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Rejecting Incoming Connection: %d\n", DataEntryPtr->DataLinkID));

                              //FIXME Deny authorization; What's the best response code?
                              Result = _HDPM_Data_Connection_Request_Response(DataEntryPtr->DataLinkID, MCAP_RESPONSE_CODE_DATA_LINK_BUSY, cmNoPreference, NULL);
                              //XXX Do we need to trigger an event?
                           }
                           else
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Data Channel: %d, %d\n", DataEntryPtr->MCLID, DataEntryPtr->DataLinkID));

                              Result = _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
                           }
                           break;
                        case dcsConnecting:
                        case dcsConnected:
                        default:
                           /* Just issue the disconnect request. The    */
                           /* lower layer will perform a Disconnect     */
                           /* or Abort as appropriate. Events will      */
                           /* be dispatched to clients when the         */
                           /* disconnection or abort confirmation is    */
                           /* received from the framework.              */
                           Result = _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
                           break;
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
               }
               else
               {
                  Result = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;

                  /* The endpoint data channel was found, but not the   */
                  /* connection it claims to be associated with. Just   */
                  /* clean up the data channel entry.                   */
                  if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                  {
                     _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
                     FreeDataChannelEntryMemory(DataEntryPtr);
                  }
               }
            }
            else
               Result = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessWriteDataMessage(HDPM_Write_Data_Request_t *Message)
{
   int                         Result;
   Connection_Entry_t         *ConnectionEntryPtr;
   Data_Channel_Entry_t       *DataEntryPtr;
   HDPM_Write_Data_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on. Next, verify that the input parameters*/
         /* appear to be semi-valid.                                    */
         if((Message->DataLinkID) && (Message->DataLength))
         {
            if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, Message->DataLinkID)) != NULL)
            {
               /* Obtain the connection entry for this data link.       */
               if((ConnectionEntryPtr = SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID)) != NULL)
               {
                  /* A client may manage Data Channel connections only  */
                  /* on Instance connections which the client owns.     */
                  if(ConnectionEntryPtr->EventCallback.ClientID == Message->MessageHeader.AddressID)
                  {
                     if(DataEntryPtr->ConnectionState == dcsConnected)
                        Result = _HDPM_Write_Data(Message->DataLinkID, Message->DataLength, &(Message->DataBuffer[0]));
                     else
                        Result = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
                  }
                  else
                     Result = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
               }
               else
               {
                  Result = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;

                  /* The endpoint data channel was found, but not the   */
                  /* connection it claims to be associated with. Just   */
                  /* clean up the data channel entry.                   */
                  if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                     FreeDataChannelEntryMemory(DataEntryPtr);
               }
            }
            else
               Result = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDPM_WRITE_DATA_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HDPM_MESSAGE_FUNCTION_REGISTER_ENDPOINT:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Endpoint Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_REGISTER_ENDPOINT_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_REGISTER_ENDPOINT_REQUEST_SIZE(((HDPM_Register_Endpoint_Request_t *)Message)->DescriptionLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessRegisterEndpointMessage((HDPM_Register_Endpoint_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_UN_REGISTER_ENDPOINT:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("UnRegister Endpoint Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_UN_REGISTER_ENDPOINT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessUnRegisterEndpointMessage((HDPM_Un_Register_Endpoint_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DATA_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessDataConnectionRequestResponseMessage((HDPM_Data_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_INSTANCES:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Remote Device Instances Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_QUERY_REMOTE_DEVICE_INSTANCES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessQueryRemoteDeviceInstancesMessage((HDPM_Query_Remote_Device_Instances_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ENDPOINTS:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Remote Device Endpoints Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessQueryRemoteDeviceEndpointsMessage((HDPM_Query_Remote_Device_Endpoints_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_QUERY_ENDPOINT_DESCRIPTION:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Remote Device Endpoint Description Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_QUERY_ENDPOINT_DESCRIPTION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessQueryEndpointDescriptionMessage((HDPM_Query_Endpoint_Description_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessConnectRemoteDeviceMessage((HDPM_Connect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Remote Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DISCONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessDisconnectRemoteDeviceMessage((HDPM_Disconnect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE_ENDPOINT:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device Endpoint Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessConnectRemoteDeviceEndpointMessage((HDPM_Connect_Remote_Device_Endpoint_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE_ENDPOINT:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Remote Device Endpoint Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessDisconnectRemoteDeviceEndpointMessage((HDPM_Disconnect_Remote_Device_Endpoint_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDPM_MESSAGE_FUNCTION_WRITE_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Write Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_WRITE_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDPM_WRITE_DATA_REQUEST_SIZE(((HDPM_Write_Data_Request_t *)Message)->DataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Health Device message.                                */
               ProcessWriteDataMessage((HDPM_Write_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process  */
   /* a client Un-Registration event. This function will perform any    */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Endpoint_Entry_t     *EndpointPtr;
   Connection_Entry_t   *ConnectionPtr;
   Data_Channel_Entry_t *DataEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   /* Unregister all endpoints owned by the client.                     */
   EndpointPtr = EndpointList;
   while((EndpointPtr = DeleteEndpointEntryClientID(&EndpointList, ClientID)) != NULL)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unregistering Endpoint: %d, 0x%04X, %s\n", EndpointPtr->MDEP_ID, EndpointPtr->MDEP_DataType, ((EndpointPtr->MDEP_Role == drSink) ? "Sink" : "Source")));

      _HDPM_Un_Register_Endpoint(EndpointPtr->MDEP_ID, EndpointPtr->MDEP_DataType, EndpointPtr->MDEP_Role);

      FreeEndpointEntryMemory(EndpointPtr);
   }

   /* Close all outbound data channels owned by the client.             */
   while((DataEntryPtr = DeleteDataChannelEntryClientID(&DataChannelList, ClientID)) != NULL)
   {
      switch(DataEntryPtr->ConnectionState)
      {
         case dcsAuthorizing:
            if(((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID)) != NULL) && (ConnectionPtr->Server))
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Rejecting Incoming Connection: %d\n", DataEntryPtr->DataLinkID));

               _HDPM_Data_Connection_Request_Response(DataEntryPtr->DataLinkID, MCAP_RESPONSE_CODE_INVALID_DATA_ENDPOINT, cmNoPreference, NULL);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Data Channel: %d, %d\n", DataEntryPtr->MCLID, DataEntryPtr->DataLinkID));

               _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
            }
            break;
         case dcsConnecting:
         case dcsConnected:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Data Channel: %d, %d\n", DataEntryPtr->MCLID, DataEntryPtr->DataLinkID));

            _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
            break;
      }

      FreeDataChannelEntryMemory(DataEntryPtr);
   }

   /* Close all control channels owned by the client.                   */
   while((ConnectionPtr = DeleteConnectionEntryClientID(&ConnectionList, ClientID)) != NULL)
   {
      switch(ConnectionPtr->ConnectionState)
      {
         case csIdle:
            /* XXX Unused                                               */
            break;
         case csAuthorizing:
            /* XXX Once multiple local instances are supported, incoming*/
            /* connections in the authorizing state will have to be     */
            /* rejected, here. Currently, this state is never used.     */
            break;
         case csConnectingDevice:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Remote Device: %02X:%02X:%02X:%02X:%02X:%02X\n", ConnectionPtr->BD_ADDR.BD_ADDR5, ConnectionPtr->BD_ADDR.BD_ADDR4, ConnectionPtr->BD_ADDR.BD_ADDR3, ConnectionPtr->BD_ADDR.BD_ADDR2, ConnectionPtr->BD_ADDR.BD_ADDR1, ConnectionPtr->BD_ADDR.BD_ADDR0));

            DEVM_DisconnectRemoteDevice(ConnectionPtr->BD_ADDR, 0);
            break;
         case csConnecting:
         case csConnected:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Remote Instance: %d\n", ConnectionPtr->MCLID));

            _HDPM_Disconnect_Remote_Instance(ConnectionPtr->MCLID);
            break;
      }

      FreeConnectionEntryMemory(ConnectionPtr);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlConnectIndicationEvent(HDP_Control_Connect_Indication_Data_t *ControlConnectIndicationData)
{
   Connection_Entry_t  Connection;
   Connection_Entry_t *ConnectionPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlConnectIndicationData) && (ControlConnectIndicationData->HDPInstanceID) && (ControlConnectIndicationData->MCLID) && (!COMPARE_NULL_BD_ADDR(ControlConnectIndicationData->BD_ADDR)))
   {
      /* Make sure this connection does not already exist.              */
      if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlConnectIndicationData->MCLID)) == NULL)
      {
         BTPS_MemInitialize(&Connection, 0, sizeof(Connection_Entry_t));

         Connection.BD_ADDR         = ControlConnectIndicationData->BD_ADDR;
         Connection.Instance        = ControlConnectIndicationData->HDPInstanceID;
         Connection.MCLID           = ControlConnectIndicationData->MCLID;
         Connection.Server          = TRUE;
         Connection.ConnectionState = csConnected;

         if((ConnectionPtr = AddConnectionEntry(&ConnectionList, &Connection)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control connection established for local instance 0x%08X: %d\n", ConnectionPtr->Instance, ConnectionPtr->MCLID));
         }
         else
         {
            /* Adding the connection object failed. Close the           */
            /* connection, since it cannot be tracked.                  */
            _HDPM_Disconnect_Remote_Instance(ControlConnectIndicationData->MCLID);
         }
      }
      else
      {
         /* No connection is known for this MCLID. All connections must */
         /* be authenticated, so this is an invalid event. Close the    */
         /* connection immediately.                                     */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Received a connection indication for an already-existing link.\n"));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlConnectConfirmationEvent(HDP_Control_Connect_Confirmation_Data_t *ControlConnectConfirmationData)
{
   int                 ConnectionStatus;
   Connection_Entry_t *ConnectionPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlConnectConfirmationData) && (ControlConnectConfirmationData->MCLID))
   {
      /* Retrieve the associated connection object.                     */
      if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlConnectConfirmationData->MCLID)) != NULL)
      {
         /* Confirm that the connection was in the appropriate state to */
         /* expect a connection confirmation.                           */
         if(ConnectionPtr->ConnectionState == csConnecting)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control connection confirmed for remote instance 0x%08X: %d (%d)\n", ConnectionPtr->Instance, ConnectionPtr->MCLID, ControlConnectConfirmationData->Status));

            /* Map the connect confirmation error to the correct Health */
            /* Device Manager error status.                             */
            switch(ControlConnectConfirmationData->Status)
            {
               case HDP_CONNECTION_STATUS_SUCCESS:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_SUCCESS;
                  break;
               case HDP_CONNECTION_STATUS_CONNECTION_TIMEOUT:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_TIMEOUT;
                  break;
               case HDP_CONNECTION_STATUS_CONNECTION_REFUSED:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_REFUSED;
                  break;
               case HDP_CONNECTION_STATUS_CONNECTION_TERMINATED:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;
                  break;
               case HDP_CONNECTION_STATUS_CONFIGURATION_ERROR:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONFIGURATION;
                  break;
               case HDP_CONNECTION_STATUS_INSTANCE_NOT_REGISTERED:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_INVALID_INSTANCE;
                  break;
               case HDP_CONNECTION_STATUS_UNKNOWN_ERROR:
               default:
                  ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                  break;
            }

            /* If the connection was successful, record the state       */
            /* change.                                                  */
            if(ControlConnectConfirmationData->Status == HDP_CONNECTION_STATUS_SUCCESS)
               ConnectionPtr->ConnectionState = csConnected;

            /* Check to see if we need to dispatch the event or set an  */
            /* internal event.                                          */
            if(ConnectionPtr->ConnectionEvent)
            {
               /* Note the Status.                                      */
               ConnectionPtr->ConnectionStatus = ControlConnectConfirmationData->Status;

               BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
            }
            else
            {
               if(CallbackRegistrationValid(&(ConnectionPtr->EventCallback)))
               {
                  /* Event needs to be dispatched.                      */
                  if(ControlConnectConfirmationData->Status != HDP_CONNECTION_STATUS_SUCCESS)
                  {
                     /* The connection request was rejected.  Remove    */
                     /* the connection from the list now, so it is not  */
                     /* accessible during the client callback.  It will */
                     /* be cleaned up after the client notification.    */
                     DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);
                  }

                  DispatchControlConnectConfirmationEvent(ConnectionPtr, ConnectionStatus);

                  if(ControlConnectConfirmationData->Status != HDP_CONNECTION_STATUS_SUCCESS)
                  {
                     /* Connection failed, free the connection pointer  */
                     /* that was deleted just above.                    */
                     FreeConnectionEntryMemory(ConnectionPtr);
                  }
               }
               else
               {
                  /* No client is registered to own this                */
                  /* connection. This shouldn't happen, because any     */
                  /* outgoing connection must have been initiated by a  */
                  /* client. Close the connection immediately.          */
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("No client registered to receive notification of the new connection.\n"));

                  if(ControlConnectConfirmationData->Status == HDP_CONNECTION_STATUS_SUCCESS)
                     _HDPM_Disconnect_Remote_Instance(ControlConnectConfirmationData->MCLID);

                  /* An unexpected error occurred, remove the connection*/
                  /* from the tracking list (if present), and clean it  */
                  /* up.                                                */
                  DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);

                  FreeConnectionEntryMemory(ConnectionPtr);
               }
            }
         }
         else
         {
            /* A connection confirmation was not expected for this      */
            /* control channel. This shouldn't happen, because any      */
            /* outgoing connection must have been initiated by a client.*/
            /* Close the connection immediately.                        */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Connection confirmation unexpected: incorrect state.\n"));

            if(ControlConnectConfirmationData->Status == HDP_CONNECTION_STATUS_SUCCESS)
               _HDPM_Disconnect_Remote_Instance(ControlConnectConfirmationData->MCLID);

            /* An unexpected error occurred, remove the connection from */
            /* the tracking list (if present), and clean it up.         */
            DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);

            FreeConnectionEntryMemory(ConnectionPtr);
         }
      }
      else
      {
         /* No connection is known for this MCLID. Any confirmation must*/
         /* be the result of a previous connection request, so perhaps  */
         /* the connection attempt was cancelled. Close the connection  */
         /* immediately.                                                */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a connection confirmation for a non-existent link.\n"));

         if(ControlConnectConfirmationData->Status == HDP_CONNECTION_STATUS_SUCCESS)
            _HDPM_Disconnect_Remote_Instance(ControlConnectConfirmationData->MCLID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlDisconnectIndicationEvent(HDP_Control_Disconnect_Indication_Data_t *ControlDisconnectIndicationData)
{
   HDPM_Event_Data_t            Event;
   Connection_Entry_t          *ConnectionPtr;
   HDPM_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlDisconnectIndicationData) && (ControlDisconnectIndicationData->MCLID))
   {
      /* Remove the associated connection object from the list.         */
      if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlDisconnectIndicationData->MCLID)) != NULL)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control connection closed for remote instance 0x%08X: %d\n", ConnectionPtr->Instance, ConnectionPtr->MCLID));

         /* Only send notification of this event if it is for a         */
         /* connection to a remote instance.                            */
         if(ConnectionPtr->Server == FALSE)
         {
            /* Check to see if we need to dispatch the event or set an  */
            /* internal event.                                          */
            if(ConnectionPtr->ConnectionEvent)
            {
               /* Note the Status.                                      */
               ConnectionPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;

               BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
            }
            else
            {
               /* Remove the associated connection object from the      */
               /* connection list.                                      */
               if((ConnectionPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr)) != NULL)
               {
                  if(CallbackRegistrationValid(&(ConnectionPtr->EventCallback)))
                  {
                     /* Determine whether the notification is going to a*/
                     /* local callback or remote client.                */
                     if(ConnectionPtr->EventCallback.CallbackFunction)
                     {
                        /* A local callback is registered.              */
                        BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                        Event.EventType                                           = hetHDPDisconnected;
                        Event.EventLength                                         = HDPM_DISCONNECTED_EVENT_DATA_SIZE;

                        Event.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                        Event.EventData.DisconnectedEventData.Instance            = ConnectionPtr->Instance;

                        /* Release the Lock so we can make the callback.*/
                        DEVM_ReleaseLock();

                        __BTPSTRY
                        {
                           if(ConnectionPtr->EventCallback.CallbackFunction)
                              (ConnectionPtr->EventCallback.CallbackFunction)(&Event, ConnectionPtr->EventCallback.CallbackParameter);
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Re-acquire the Lock.                         */
                        DEVM_AcquireLock();
                     }
                     else
                     {
                        /* This connection is owned by a remote client. */
                        BTPS_MemInitialize(&Message, 0, HDPM_DISCONNECTED_MESSAGE_SIZE);

                        Message.MessageHeader.AddressID       = ConnectionPtr->EventCallback.ClientID;
                        Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                        Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                        Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DISCONNECTED;
                        Message.MessageHeader.MessageLength   = (HDPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                        Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                        Message.Instance                      = ConnectionPtr->Instance;

                        /* Finally dispatch the Message.                */
                        MSG_SendMessage((BTPM_Message_t *)&Message);
                     }
                  }

                  /* Free connection object.                            */
                  FreeConnectionEntryMemory(ConnectionPtr);
               }
            }
         }
         else
         {
            /* This connection was created by the remote device to the  */
            /* local server.  Since the local server is internally      */
            /* managed, no notification will be dispatched.  Simply     */
            /* remove the associated connection object from the         */
            /* connection list.                                         */
            DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);

            /* Free connection object.                                  */
            FreeConnectionEntryMemory(ConnectionPtr);
         }
      }
      else
      {
         /* No connection is known for this MCLID. Any confirmation must*/
         /* be the result of a previous connection request, so perhaps  */
         /* the connection attempt was cancelled. Close the connection  */
         /* immediately.                                                */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a disconnection indication for a non-existent link: %d.\n", ControlDisconnectIndicationData->MCLID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlCreateDataLinkIndicationEvent(HDP_Control_Create_Data_Link_Indication_t *ControlCreateDataLinkIndicationData)
{
   Endpoint_Entry_t                        *EndpointPtr;
   HDPM_Event_Data_t                        Event;
   Connection_Entry_t                      *ConnectionPtr;
   Data_Channel_Entry_t                     DataEntry;
   Data_Channel_Entry_t                    *DataEntryPtr;
   HDPM_Incoming_Data_Connection_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlCreateDataLinkIndicationData) && (ControlCreateDataLinkIndicationData->MCLID) && (ControlCreateDataLinkIndicationData->MDEPID) && (ControlCreateDataLinkIndicationData->DataLinkID))
   {
      /* XXX                                                            */
      /* Notes for supporting multiple PM clients per MDEP:             */
      /*************************                                        */
      /*                                                                */
      /* - Build list of all clients who need to be polled for          */
      /*   ownership.                                                   */
      /*                                                                */
      /* - If list is empty {                                           */
      /*     Reject connection request (which reason code?)             */
      /*   } else {                                                     */
      /*     Queue list to message queue.                               */
      /*   }                                                            */
      /*                                                                */
      /* - Message queue handler should send indication to client, and  */
      /*   block on result.                                             */
      /*                                                                */
      /* - After return:                                                */
      /*   If the client sent "request response" during indication {    */
      /*      If connection is accepted {                               */
      /*         Process is finished, clean up client list.             */
      /*      } else {                                                  */
      /*         Note the client's rejection reason,                    */
      /*         Re-queue to msg queue with next client as "current"    */
      /*      }                                                         */
      /*   } else {                                                     */
      /*      Assume rejection, queue msg with next client as "current" */
      /*   }                                                            */
      /*                                                                */
      /* - If no clients remain, reject.                                */
      /*      TODO What rejection reason to use?                        */
      /*                                                                */
      /***** Alternatively *****                                        */
      /*                                                                */
      /* - Broadcast to all clients. Maybe show preference to callbacks */
      /*   registered by the PM server?                                 */
      /*                                                                */
      /* - Note each negative reply.                                    */
      /*                                                                */
      /* - Accept the first positive reply.                             */
      /*                                                                */
      /* - When all replies have been received (or timer expired?),     */
      /*   resolve set of negative replies to use the most reasonable.  */
      /*                                                                */
      /*************************                                        */

      /* Locate the registered endpoint matching the requested MDEP.    */
      if((EndpointPtr = SearchEndpointEntryMDEP(&EndpointList, ControlCreateDataLinkIndicationData->MDEPID)) != NULL)
      {
         /* Identify the connection the request is coming from.         */
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlCreateDataLinkIndicationData->MCLID)) != NULL)
         {
            /* Determine if this connection has an owner, yet.          */
            if(!CallbackRegistrationValid(&(ConnectionPtr->EventCallback)))
            {
               /* The connection has no owner, so assign the owner of   */
               /* the requested endpoint.                               */
               ConnectionPtr->EventCallback = EndpointPtr->EventCallback;
            }

            /* In order to process the request, it must have come from a*/
            /* connection owned by the same client which holds the MDEP */
            /* registration.                                            */
            if(ConnectionPtr->EventCallback.ClientID == EndpointPtr->EventCallback.ClientID)
            {
               /* Build the data connection object.                     */
               BTPS_MemInitialize(&DataEntry, 0, sizeof(Data_Channel_Entry_t));

               DataEntry.MCLID           = ConnectionPtr->MCLID;
               DataEntry.DataLinkID      = ControlCreateDataLinkIndicationData->DataLinkID;
               DataEntry.MDEP_ID         = EndpointPtr->MDEP_ID;
               DataEntry.ChannelMode     = ControlCreateDataLinkIndicationData->ChannelMode;
               DataEntry.LocalRole       = EndpointPtr->MDEP_Role;
               DataEntry.ConnectionState = dcsAuthorizing;
               DataEntry.EventCallback   = EndpointPtr->EventCallback;

               /* Add the new data connection object to the list.       */
               if((DataEntryPtr = AddDataChannelEntry(&DataChannelList, &DataEntry)) != NULL)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming data connection request: %d, %d\n", ControlCreateDataLinkIndicationData->MCLID, ControlCreateDataLinkIndicationData->MDEPID));

                  /* Now notify the owner of the connection request.    */
                  /* First, determine whether the notification is going */
                  /* to a local callback or remote client.              */
                  if(DataEntryPtr->EventCallback.CallbackFunction)
                  {
                     /* A local callback is registered.                 */
                     BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                     Event.EventType                                                            = hetHDPIncomingDataConnectionRequest;
                     Event.EventLength                                                          = HDPM_INCOMING_DATA_CONNECTION_REQUEST_EVENT_DATA_SIZE;

                     Event.EventData.IncomingDataConnectionRequestEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                     Event.EventData.IncomingDataConnectionRequestEventData.EndpointID          = EndpointPtr->MDEP_ID;
                     Event.EventData.IncomingDataConnectionRequestEventData.ChannelMode         = DataEntryPtr->ChannelMode;
                     Event.EventData.IncomingDataConnectionRequestEventData.DataLinkID          = DataEntryPtr->DataLinkID;

                     /* Release the Lock so we can make the callback.   */
                     DEVM_ReleaseLock();

                     __BTPSTRY
                     {
                        if(DataEntryPtr->EventCallback.CallbackFunction)
                           (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Re-acquire the Lock.                            */
                     DEVM_AcquireLock();
                  }
                  else
                  {
                     /* This connection is owned by a remote client.    */
                     BTPS_MemInitialize(&Message, 0, HDPM_INCOMING_DATA_CONNECTION_MESSAGE_SIZE);

                     Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                     Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                     Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                     Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_INCOMING_DATA_CONNECTION_REQUEST;
                     Message.MessageHeader.MessageLength   = (HDPM_INCOMING_DATA_CONNECTION_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                     Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                     Message.EndpointID                    = DataEntryPtr->MDEP_ID;
                     Message.ChannelMode                   = DataEntryPtr->ChannelMode;
                     Message.DataLinkID                    = DataEntryPtr->DataLinkID;

                     /* Finally dispatch the Message.                   */
                     MSG_SendMessage((BTPM_Message_t *)&Message);
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to add data connection object to the list.\n"));

                  _HDPM_Data_Connection_Request_Response(ControlCreateDataLinkIndicationData->DataLinkID, MCAP_RESPONSE_CODE_UNSPECIFIED_ERROR, cmNoPreference, NULL);
               }
            }
            else
            {
               /* MDEP connection request was for an endpoint that is   */
               /* not available from the PM client handling the control */
               /* connection. The spec requires that if we publish an   */
               /* endpoint in SDP, we must support connections to it.   */
               /* To avoid breaking spec, report that the endpoint is   */
               /* busy -- that is, it is currently handling the maximum */
               /* number of concurrent connections (in this case, that  */
               /* max is 0).                                            */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data connection request was for an endpoint not owned by the client handling the control connection: 0x%02x, %d\n", ControlCreateDataLinkIndicationData->MDEPID, ControlCreateDataLinkIndicationData->MCLID));

               _HDPM_Data_Connection_Request_Response(ControlCreateDataLinkIndicationData->DataLinkID, MCAP_RESPONSE_CODE_DATA_ENDPOINT_BUSY, cmNoPreference, NULL);
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Missing control channel connection object associated with incoming data channel request: %d\n", ControlCreateDataLinkIndicationData->MCLID));

            _HDPM_Data_Connection_Request_Response(ControlCreateDataLinkIndicationData->DataLinkID, MCAP_RESPONSE_CODE_UNSPECIFIED_ERROR, cmNoPreference, NULL);
         }
      }
      else
      {
         /* No endpoint exists. This should never happen because the    */
         /* framework should auto-reject connection attempts for MDEPs  */
         /* that don't exist. Reject with "Invalid Endpoint".           */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection request for non-registered endpoint: 0x%02x\n", ControlCreateDataLinkIndicationData->MDEPID));

         _HDPM_Data_Connection_Request_Response(ControlCreateDataLinkIndicationData->DataLinkID, MCAP_RESPONSE_CODE_INVALID_DATA_ENDPOINT, cmNoPreference, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlCreateDataLinkConfirmationEvent(HDP_Control_Create_Data_Link_Confirmation_t *ControlCreateDataLinkConfirmationData)
{
   int                                    ConnectionStatus;
   HDPM_Event_Data_t                      Event;
   Connection_Entry_t                    *ConnectionPtr;
   Data_Channel_Entry_t                  *DataEntryPtr;
   HDPM_Data_Connection_Status_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlCreateDataLinkConfirmationData) && (ControlCreateDataLinkConfirmationData->MCLID) && (ControlCreateDataLinkConfirmationData->DataLinkID))
   {
      /* Retrieve the associated data channel object.                   */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, ControlCreateDataLinkConfirmationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlCreateDataLinkConfirmationData->MCLID)) != NULL)
         {
            /* Confirm that the connection was in the appropriate state */
            /* to expect a connection confirmation.                     */
            if(DataEntryPtr->ConnectionState == dcsAuthorizing)
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection confirmed: %d\n", DataEntryPtr->DataLinkID));

               if(ControlCreateDataLinkConfirmationData->ResponseCode == MCAP_RESPONSE_CODE_SUCCESS)
               {
                  /* Note the successful connection status.             */
                  DataEntryPtr->ConnectionState = dcsConnecting;
               }
               else
               {
                  /* The connection request was rejected.  Map the open */
                  /* confirmation Response to the correct Health Device */
                  /* Manager error status.                              */
                  switch(ControlCreateDataLinkConfirmationData->ResponseCode)
                  {
                     case MCAP_RESPONSE_CODE_SUCCESS:
                        ConnectionStatus = HDPM_CONNECTION_STATUS_SUCCESS;
                        break;

                     case MCAP_RESPONSE_CODE_DATA_ENDPOINT_BUSY:
                     case MCAP_RESPONSE_CODE_DATA_LINK_BUSY:
                     case MCAP_RESPONSE_CODE_RESOURCE_UNAVAILABLE:
                        ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;

                     case MCAP_RESPONSE_CODE_CONFIGURATION_REJECTED:
                        ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONFIGURATION;
                        break;

                     case MCAP_RESPONSE_CODE_INVALID_DATA_ENDPOINT:
                     case MCAP_RESPONSE_CODE_INVALID_DATA_LINK_ID:
                        ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_INVALID_INSTANCE;
                        break;

                     case MCAP_RESPONSE_CODE_INVALID_OPCODE:
                     case MCAP_RESPONSE_CODE_INVALID_OPERATION:
                     case MCAP_RESPONSE_CODE_INVALID_PARAMETER_VALUE:
                     case MCAP_RESPONSE_CODE_REQUEST_NOT_SUPPORTED:
                     case MCAP_RESPONSE_CODE_UNSPECIFIED_ERROR:
                     default:
                        ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* Check to see if we need to dispatch the event or   */
                  /* set an internal event.                             */
                  if(DataEntryPtr->ConnectionEvent)
                  {
                     /* Note the Status.                                */
                     DataEntryPtr->ConnectionStatus = ConnectionStatus;

                     BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
                  }
                  else
                  {
                     /* Remove the data channel from the list now,      */
                     /* so it is not accessible during the client       */
                     /* callback.  It will be cleaned up after the      */
                     /* client notification.                            */
                     DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);

                     /* Notify the client of the rejection.             */
                     if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
                     {
                        /* Determine whether the notification is going  */
                        /* to a local callback or remote client.        */
                        if(DataEntryPtr->EventCallback.CallbackFunction)
                        {
                           /* A local callback is registered.           */
                           BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                           Event.EventType                                                   = hetHDPDataConnectionStatus;
                           Event.EventLength                                                 = HDPM_DATA_CONNECTION_STATUS_EVENT_DATA_SIZE;

                           Event.EventData.DataConnectionStatusEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                           Event.EventData.DataConnectionStatusEventData.Instance            = ConnectionPtr->Instance;
                           Event.EventData.DataConnectionStatusEventData.EndpointID          = DataEntryPtr->MDEP_ID;
                           Event.EventData.DataConnectionStatusEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                           Event.EventData.DataConnectionStatusEventData.Status              = ConnectionStatus;

                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection result: %d\n", Event.EventData.DataConnectionStatusEventData.Status));

                           /* Release the Lock so we can make the       */
                           /* callback.                                 */
                           DEVM_ReleaseLock();

                           __BTPSTRY
                           {
                              if(DataEntryPtr->EventCallback.CallbackFunction)
                                 (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                           }
                           __BTPSEXCEPT(1)
                           {
                              /* Do Nothing.                            */
                           }

                           /* Re-acquire the Lock.                      */
                           DEVM_AcquireLock();
                        }
                        else
                        {
                           /* This connection is owned by a remote      */
                           /* client.                                   */
                           BTPS_MemInitialize(&Message, 0, HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE);

                           Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                           Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                           Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                           Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_STATUS;
                           Message.MessageHeader.MessageLength   = (HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                           Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                           Message.Instance                      = ConnectionPtr->Instance;
                           Message.EndpointID                    = DataEntryPtr->MDEP_ID;
                           Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                           Message.Status                        = ConnectionStatus;

                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection result: %d\n", Message.Status));

                           /* Finally dispatch the Message.             */
                           MSG_SendMessage((BTPM_Message_t *)&Message);
                        }
                     }
                     else
                     {
                        /* No client is registered to own this          */
                        /* connection.  This shouldn't happen, because  */
                        /* any outgoing connection must have been       */
                        /* initiated by a client.  Close the connection */
                        /* immediately.                                 */
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("No client registered to receive notification of the failed connection attempt.\n"));
                     }

                     /* The data channel entry is no longer needed.     */
                     FreeDataChannelEntryMemory(DataEntryPtr);
                  }
               }
            }
            else
            {
               /* A connection confirmation was not expected for this   */
               /* data channel.  This shouldn't happen, because any     */
               /* outgoing connection request must have been initiated  */
               /* by a client.  Close the connection immediately.       */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Connection confirmation unexpected: incorrect state.\n"));

               if(ControlCreateDataLinkConfirmationData->ResponseCode == MCAP_RESPONSE_CODE_SUCCESS)
               {
                  _HDPM_Disconnect_Data_Channel(ControlCreateDataLinkConfirmationData->MCLID, ControlCreateDataLinkConfirmationData->DataLinkID);
               }
               else
               {
                  if(DataEntryPtr->ConnectionEvent)
                  {
                     DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

                     BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
                  }
                  else
                  {
                     DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                     FreeDataChannelEntryMemory(DataEntryPtr);
                  }
               }
            }
         }
         else
         {
            /* No control channel connection is associated with this    */
            /* Data Link ID.  Close the connection immediately.         */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection confirmation with no associated control channel: %d, %d\n", ControlCreateDataLinkConfirmationData->DataLinkID, ControlCreateDataLinkConfirmationData->MCLID));

            if(ControlCreateDataLinkConfirmationData->ResponseCode == MCAP_RESPONSE_CODE_SUCCESS)
               _HDPM_Disconnect_Data_Channel(ControlCreateDataLinkConfirmationData->MCLID, ControlCreateDataLinkConfirmationData->DataLinkID);
            else
            {
               if(DataEntryPtr->ConnectionEvent)
               {
                  DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

                  BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
               }
               else
               {
                  DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
            }
         }
      }
      else
      {
         /* No data connection is known for this Data Link ID. Any      */
         /* confirmation must be the result of a previous connection    */
         /* request, so perhaps the connection attempt was cancelled.   */
         /* Close the connection immediately.                           */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a connection confirmation for a non-existent link: %d\n", ControlCreateDataLinkConfirmationData->DataLinkID));

         if(ControlCreateDataLinkConfirmationData->ResponseCode == MCAP_RESPONSE_CODE_SUCCESS)
            _HDPM_Disconnect_Data_Channel(ControlCreateDataLinkConfirmationData->MCLID, ControlCreateDataLinkConfirmationData->DataLinkID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlAbortDataLinkIndicationEvent(HDP_Control_Abort_Data_Link_Indication_t *ControlAbortDataLinkIndicationData)
{
   HDPM_Event_Data_t                 Event;
   Connection_Entry_t               *ConnectionPtr;
   Data_Channel_Entry_t             *DataEntryPtr;
   HDPM_Data_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlAbortDataLinkIndicationData) && (ControlAbortDataLinkIndicationData->MCLID) && (ControlAbortDataLinkIndicationData->DataLinkID))
   {
      /* Remove the associated connection object from the list.         */
      if((DataEntryPtr = DeleteDataChannelEntry(&DataChannelList, ControlAbortDataLinkIndicationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlAbortDataLinkIndicationData->MCLID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection aborted: %d\n", DataEntryPtr->MCLID));

            /* Confirm that this data connection has a registered owner */
            /* to receive the event.                                    */
            if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
            {
               /* Determine whether the notification is going to a      */
               /* local callback or remote client.                      */
               if(DataEntryPtr->EventCallback.CallbackFunction)
               {
                  /* A local callback is registered.                    */
                  BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                  Event.EventType                                               = hetHDPDataDisconnected;
                  Event.EventLength                                             = HDPM_DATA_DISCONNECTED_EVENT_DATA_SIZE;

                  Event.EventData.DataDisconnectedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                  Event.EventData.DataDisconnectedEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                  Event.EventData.DataDisconnectedEventData.Reason              = HDPM_DATA_DISCONNECT_REASON_ABORTED;

                  /* Release the Lock so we can make the callback.      */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                        (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* This connection is owned by a remote client.       */
                  BTPS_MemInitialize(&Message, 0, HDPM_DATA_DISCONNECTED_MESSAGE_SIZE);

                  Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                  Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED;
                  Message.MessageHeader.MessageLength   = (HDPM_DATA_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                  Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                  Message.Reason                        = HDPM_DATA_DISCONNECT_REASON_ABORTED;

                  /* Finally dispatch the Message.                      */
                  MSG_SendMessage((BTPM_Message_t *)&Message);
               }
            }
         }
         else
         {
            /* No control channel connection is associated with this    */
            /* Data Link ID.                                            */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection abort with no associated control channel: %d, %d\n", ControlAbortDataLinkIndicationData->DataLinkID, ControlAbortDataLinkIndicationData->MCLID));
         }

         /* Free connection object.                                     */
         FreeDataChannelEntryMemory(DataEntryPtr);
      }
      else
      {
         /* No connection is known for this MCLID. Any confirmation must*/
         /* be the result of a previous connection request, so perhaps  */
         /* the connection attempt was cancelled. Close the connection  */
         /* immediately.                                                */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Data channel was already cleaned up: %d\n", ControlAbortDataLinkIndicationData->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlAbortDataLinkConfirmationEvent(HDP_Control_Abort_Data_Link_Confirmation_t *ControlAbortDataLinkConfirmationData)
{
   HDPM_Event_Data_t                      Event;
   Connection_Entry_t                    *ConnectionPtr;
   Data_Channel_Entry_t                  *DataEntryPtr;
   HDPM_Data_Connection_Status_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlAbortDataLinkConfirmationData) && (ControlAbortDataLinkConfirmationData->MCLID) && (ControlAbortDataLinkConfirmationData->DataLinkID))
   {
      /* Retrieve the associated data channel object.                   */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, ControlAbortDataLinkConfirmationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlAbortDataLinkConfirmationData->MCLID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection aborted: %d\n", DataEntryPtr->DataLinkID));

            /* Check to see if we need to dispatch the event or set an  */
            /* internal event.                                          */
            if(DataEntryPtr->ConnectionEvent)
            {
               /* Note the Status.                                      */
               DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_ABORTED;

               BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
            }
            else
            {
               /* Remove the associated data channel object from the    */
               /* data channel list.                                    */
               if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
               {
                  /* Confirm that this data connection has a registered */
                  /* owner to receive the event.                        */
                  if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
                  {
                     /* Determine whether the notification is going to a*/
                     /* local callback or remote client.                */
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                     {
                        /* A local callback is registered.              */
                        BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                        Event.EventType                                                   = hetHDPDataConnectionStatus;
                        Event.EventLength                                                 = HDPM_DATA_CONNECTION_STATUS_EVENT_DATA_SIZE;

                        Event.EventData.DataConnectionStatusEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                        Event.EventData.DataConnectionStatusEventData.Instance            = ConnectionPtr->Instance;
                        Event.EventData.DataConnectionStatusEventData.EndpointID          = DataEntryPtr->MDEP_ID;
                        Event.EventData.DataConnectionStatusEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                        Event.EventData.DataConnectionStatusEventData.Status              = HDPM_CONNECTION_STATUS_FAILURE_ABORTED;

                        /* Release the Lock so we can make the callback.*/
                        DEVM_ReleaseLock();

                        __BTPSTRY
                        {
                           if(DataEntryPtr->EventCallback.CallbackFunction)
                              (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Re-acquire the Lock.                         */
                        DEVM_AcquireLock();
                     }
                     else
                     {
                        /* This connection is owned by a remote client. */
                        BTPS_MemInitialize(&Message, 0, HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE);

                        Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                        Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                        Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                        Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_STATUS;
                        Message.MessageHeader.MessageLength   = (HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                        Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                        Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                        Message.Status                        = HDPM_CONNECTION_STATUS_FAILURE_ABORTED;

                        /* Finally dispatch the Message.                */
                        MSG_SendMessage((BTPM_Message_t *)&Message);
                     }
                  }
                  else
                  {
                     /* No registered owner/callback.                   */
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection abort for a known data channel with no owner: %d, %d\n", ControlAbortDataLinkConfirmationData->DataLinkID, ControlAbortDataLinkConfirmationData->MCLID));
                  }

                  /* Free connection object.                            */
                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Error removing Data Link entry from list: %d, %d\n", ControlAbortDataLinkConfirmationData->DataLinkID, ControlAbortDataLinkConfirmationData->MCLID));
            }
         }
         else
         {
            /* No control channel connection is associated with this    */
            /* Data Link ID.                                            */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection abort with no associated control channel: %d, %d\n", ControlAbortDataLinkConfirmationData->DataLinkID, ControlAbortDataLinkConfirmationData->MCLID));
         }
      }
      else
      {
         /* No connection is known for this DataLinkID. Any confirmation*/
         /* must be the result of a previous connection request, so     */
         /* perhaps the connection attempt was cancelled.               */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Data channel connection already aborted: %d.\n", ControlAbortDataLinkConfirmationData->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlDeleteDataLinkIndicationEvent(HDP_Control_Delete_Data_Link_Indication_t *ControlDeleteDataLinkIndicationData)
{
   HDPM_Event_Data_t                 Event;
   Connection_Entry_t               *ConnectionPtr;
   Data_Channel_Entry_t             *DataEntryPtr;
   HDPM_Data_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlDeleteDataLinkIndicationData) && (ControlDeleteDataLinkIndicationData->MCLID) && (ControlDeleteDataLinkIndicationData->DataLinkID))
   {
      /* Retrieve the associated connection object from the list.       */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, ControlDeleteDataLinkIndicationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlDeleteDataLinkIndicationData->MCLID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection deleted: %d\n", DataEntryPtr->DataLinkID));

            /* Check to see if we need to dispatch the event or set an  */
            /* internal event.                                          */
            if(DataEntryPtr->ConnectionEvent)
            {
               /* Note the Status.                                      */
               DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;

               BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
            }
            else
            {
               /* Remove the associated data channel object from the    */
               /* data channel list.                                    */
               if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
               {
                  /* Confirm that this data connection has a registered */
                  /* owner to receive the event.                        */
                  if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
                  {
                     /* Determine whether the notification is going to a*/
                     /* local callback or remote client.                */
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                     {
                        /* A local callback is registered.              */
                        BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                        Event.EventType                                               = hetHDPDataDisconnected;
                        Event.EventLength                                             = HDPM_DATA_DISCONNECTED_EVENT_DATA_SIZE;

                        Event.EventData.DataDisconnectedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                        Event.EventData.DataDisconnectedEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                        Event.EventData.DataDisconnectedEventData.Reason              = HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT;

                        /* Release the Lock so we can make the callback.*/
                        DEVM_ReleaseLock();

                        __BTPSTRY
                        {
                           if(DataEntryPtr->EventCallback.CallbackFunction)
                              (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Re-acquire the Lock.                         */
                        DEVM_AcquireLock();
                     }
                     else
                     {
                        /* This connection is owned by a remote client. */
                        BTPS_MemInitialize(&Message, 0, HDPM_DATA_DISCONNECTED_MESSAGE_SIZE);

                        Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                        Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                        Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                        Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED;
                        Message.MessageHeader.MessageLength   = (HDPM_DATA_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                        Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                        Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                        Message.Reason                        = HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT;

                        /* Finally dispatch the Message.                */
                        MSG_SendMessage((BTPM_Message_t *)&Message);
                     }
                  }
                  else
                  {
                     /* No registered owner/callback.                   */
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection deletion for a known data channel with no owner: %d, %d\n", ControlDeleteDataLinkIndicationData->DataLinkID, ControlDeleteDataLinkIndicationData->MCLID));
                  }

                  /* Free connection object.                            */
                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Error removing Data Link entry from list: %d, %d\n", ControlDeleteDataLinkIndicationData->DataLinkID, ControlDeleteDataLinkIndicationData->MCLID));
            }
         }
         else
         {
            /* No control channel connection is associated with this    */
            /* Data Link ID.                                            */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection deletion with no associated control channel: %d, %d\n", ControlDeleteDataLinkIndicationData->DataLinkID, ControlDeleteDataLinkIndicationData->MCLID));
         }
      }
      else
      {
         /* No connection is known for this DataLinkID. Any confirmation*/
         /* must be the result of a previous connection request, so     */
         /* perhaps the connection attempt was cancelled. Close the     */
         /* connection immediately.                                     */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel already cleaned up: %d.\n", ControlDeleteDataLinkIndicationData->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlDeleteDataLinkConfirmationEvent(HDP_Control_Delete_Data_Link_Confirmation_t *ControlDeleteDataLinkConfirmationData)
{
   HDPM_Event_Data_t                 Event;
   Connection_Entry_t               *ConnectionPtr;
   Data_Channel_Entry_t             *DataEntryPtr;
   HDPM_Data_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((ControlDeleteDataLinkConfirmationData) && (ControlDeleteDataLinkConfirmationData->MCLID) && (ControlDeleteDataLinkConfirmationData->DataLinkID))
   {
      /* Retrieve the associated connection object from the list.       */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, ControlDeleteDataLinkConfirmationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, ControlDeleteDataLinkConfirmationData->MCLID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection deleted: %d\n", DataEntryPtr->DataLinkID));

            /* Check to see if we need to dispatch the event or set an  */
            /* internal event.                                          */
            if(DataEntryPtr->ConnectionEvent)
            {
               /* Note the Status.                                      */
               DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;

               BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
            }
            else
            {
               /* Remove the associated data channel object from the    */
               /* data channel list.                                    */
               if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
               {
                  /* Confirm that this data connection has a registered */
                  /* owner to receive the event.                        */
                  if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
                  {
                     /* Determine whether the notification is going to a*/
                     /* local callback or remote client.                */
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                     {
                        /* A local callback is registered.              */
                        BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                        Event.EventType                                               = hetHDPDataDisconnected;
                        Event.EventLength                                             = HDPM_DATA_DISCONNECTED_EVENT_DATA_SIZE;

                        Event.EventData.DataDisconnectedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                        Event.EventData.DataDisconnectedEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                        Event.EventData.DataDisconnectedEventData.Reason              = HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT;

                        /* Release the Lock so we can make the callback.*/
                        DEVM_ReleaseLock();

                        __BTPSTRY
                        {
                           if(DataEntryPtr->EventCallback.CallbackFunction)
                              (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Re-acquire the Lock.                         */
                        DEVM_AcquireLock();
                     }
                     else
                     {
                        /* This connection is owned by a remote client. */
                        BTPS_MemInitialize(&Message, 0, HDPM_DATA_DISCONNECTED_MESSAGE_SIZE);

                        Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                        Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                        Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                        Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED;
                        Message.MessageHeader.MessageLength   = (HDPM_DATA_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                        Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                        Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                        Message.Reason                        = HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT;

                        /* Finally dispatch the Message.                */
                        MSG_SendMessage((BTPM_Message_t *)&Message);
                     }
                  }
                  else
                  {
                     /* No registered owner/callback.                   */
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Deleted a known data channel but it has no owner: %d, %d\n", ControlDeleteDataLinkConfirmationData->DataLinkID, ControlDeleteDataLinkConfirmationData->MCLID));
                  }

                  /* Free connection object.                            */
                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
               else
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Error removing Data Link entry from list: %d, %d\n", ControlDeleteDataLinkConfirmationData->DataLinkID, ControlDeleteDataLinkConfirmationData->MCLID));
            }
         }
         else
         {
            /* No control channel connection is associated with this    */
            /* Data Link ID.                                            */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received a data connection deletion with no associated control channel: %d, %d\n", ControlDeleteDataLinkConfirmationData->DataLinkID, ControlDeleteDataLinkConfirmationData->MCLID));

            /* Remove and free connection object.                       */
            DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
            FreeDataChannelEntryMemory(DataEntryPtr);
         }
      }
      else
      {
         /* No connection is known for this DataLinkID. Any confirmation*/
         /* must be the result of a previous connection request, so     */
         /* perhaps the connection attempt was cancelled. Close the     */
         /* connection immediately.                                     */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel already cleaned up: %d.\n", ControlDeleteDataLinkConfirmationData->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDataLinkConnectIndicationEvent(HDP_Data_Link_Connect_Indication_Data_t *DataLinkConnectIndicationData)
{
   HDPM_Event_Data_t              Event;
   Connection_Entry_t            *ConnectionPtr;
   Data_Channel_Entry_t          *DataEntryPtr;
   HDPM_Data_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((DataLinkConnectIndicationData) && (DataLinkConnectIndicationData->MCLID) && (DataLinkConnectIndicationData->DataLinkID))
   {
      /* There should already be a connection object for tracking this  */
      /* connection from when the data link was registered.             */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkConnectIndicationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, DataLinkConnectIndicationData->MCLID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connected: %d\n", DataEntryPtr->DataLinkID));

            /* Note that the connection is established.                 */
            DataEntryPtr->ConnectionState = dcsConnected;

            /* Confirm that this data connection has a registered owner */
            /* to receive the event.                                    */
            if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
            {
               /* Determine whether the notification is going to a local*/
               /* callback or remote client.                            */
               if(DataEntryPtr->EventCallback.CallbackFunction)
               {
                  /* A local callback is registered.                    */
                  BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                  Event.EventType                                            = hetHDPDataConnected;
                  Event.EventLength                                          = HDPM_DATA_CONNECTED_EVENT_DATA_SIZE;

                  Event.EventData.DataConnectedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                  Event.EventData.DataConnectedEventData.EndpointID          = DataEntryPtr->MDEP_ID;
                  Event.EventData.DataConnectedEventData.DataLinkID          = DataEntryPtr->DataLinkID;

                  /* Release the Lock so we can make the callback.      */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                        (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* This connection is owned by a remote client.       */
                  BTPS_MemInitialize(&Message, 0, HDPM_DATA_DISCONNECTED_MESSAGE_SIZE);

                  Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                  Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                  Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_CONNECTED;
                  Message.MessageHeader.MessageLength   = (HDPM_DATA_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                  Message.EndpointID                    = DataEntryPtr->MDEP_ID;
                  Message.DataLinkID                    = DataEntryPtr->DataLinkID;

                  /* Finally dispatch the Message.                      */
                  MSG_SendMessage((BTPM_Message_t *)&Message);
               }
            }
            else
            {
               /* This data channel is not owned by any client.  Since  */
               /* no one is available to exchange data, close the       */
               /* connection.                                           */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected data channel is not owned by any client: %d.\n", DataLinkConnectIndicationData->DataLinkID));

               _HDPM_Disconnect_Data_Channel(DataLinkConnectIndicationData->MCLID, DataLinkConnectIndicationData->DataLinkID);
            }
         }
         else
         {
            /* No control connection is known for the MCLID of this data*/
            /* connection. Close the connection immediately.            */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel references an invalid control channel connection: %d, %d.\n", DataLinkConnectIndicationData->DataLinkID, DataLinkConnectIndicationData->MCLID));

            _HDPM_Disconnect_Data_Channel(DataLinkConnectIndicationData->MCLID, DataLinkConnectIndicationData->DataLinkID);

            /* Remove and free the data channel entry.                  */
            DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
            FreeDataChannelEntryMemory(DataEntryPtr);
         }
      }
      else
      {
         /* No data connection is known for this DataLinkID. Any        */
         /* confirmation must be the result of a previous connection    */
         /* request, so perhaps the connection attempt was cancelled.   */
         /* Close the connection immediately.                           */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel already cleaned up before incoming data connection: %d.\n", DataLinkConnectIndicationData->DataLinkID));

         _HDPM_Disconnect_Data_Channel(DataLinkConnectIndicationData->MCLID, DataLinkConnectIndicationData->DataLinkID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDataLinkConnectConfirmationEvent(HDP_Data_Link_Connect_Confirmation_Data_t *DataLinkConnectConfirmationData)
{
   int                                    ConnectionStatus;
   HDPM_Event_Data_t                      Event;
   Connection_Entry_t                    *ConnectionPtr;
   Data_Channel_Entry_t                  *DataEntryPtr;
   HDPM_Data_Connection_Status_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((DataLinkConnectConfirmationData) && (DataLinkConnectConfirmationData->MCLID) && (DataLinkConnectConfirmationData->DataLinkID))
   {
      /* Locate the associated data connection object from the list.    */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkConnectConfirmationData->DataLinkID)) != NULL)
      {
         /* Locate the control channel connection entry associated with */
         /* this data channel.                                          */
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, DataLinkConnectConfirmationData->MCLID)) != NULL)
         {
            if(DataEntryPtr->MCLID == ConnectionPtr->MCLID)
            {
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection request confirmed: %d, %d\n", DataEntryPtr->DataLinkID, DataLinkConnectConfirmationData->Status));

               /* Map the connect confirmation error to the correct     */
               /* Health Device Manager error status.                   */
               switch(DataLinkConnectConfirmationData->Status)
               {
                  case HDP_CONNECTION_STATUS_SUCCESS:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_SUCCESS;
                     break;
                  case HDP_CONNECTION_STATUS_CONNECTION_TIMEOUT:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_TIMEOUT;
                     break;
                  case HDP_CONNECTION_STATUS_CONNECTION_REFUSED:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_REFUSED;
                     break;
                  case HDP_CONNECTION_STATUS_CONNECTION_TERMINATED:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;
                     break;
                  case HDP_CONNECTION_STATUS_CONFIGURATION_ERROR:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONFIGURATION;
                     break;
                  case HDP_CONNECTION_STATUS_INSTANCE_NOT_REGISTERED:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_INVALID_INSTANCE;
                     break;
                  case HDP_CONNECTION_STATUS_UNKNOWN_ERROR:
                  default:
                     ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                     break;
               }

               /* If the connection was successful, record the state    */
               /* change.                                               */
               if(DataLinkConnectConfirmationData->Status == MCAP_RESPONSE_CODE_SUCCESS)
                  DataEntryPtr->ConnectionState = dcsConnected;

               /* Check to see if we need to dispatch the event or set  */
               /* an internal event.                                    */
               if(DataEntryPtr->ConnectionEvent)
               {
                  /* Note the Status.                                   */
                  DataEntryPtr->ConnectionStatus = ConnectionStatus;

                  BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
               }
               else
               {
                  if(ConnectionStatus != HDPM_CONNECTION_STATUS_SUCCESS)
                  {
                     /* The connection failed, so remove the associated */
                     /* data channel object from the data channel list  */
                     /* now, so it is not accessible during any client  */
                     /* callback.                                       */
                     DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                  }

                  /* Confirm that this data connection has a registered */
                  /* owner to receive the event.                        */
                  if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
                  {
                     /* Determine whether the notification is going to a*/
                     /* local callback or remote client.                */
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                     {
                        /* A local callback is registered.              */
                        BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                        Event.EventType                                                   = hetHDPDataConnectionStatus;
                        Event.EventLength                                                 = HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE;

                        Event.EventData.DataConnectionStatusEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                        Event.EventData.DataConnectionStatusEventData.Instance            = ConnectionPtr->Instance;
                        Event.EventData.DataConnectionStatusEventData.EndpointID          = DataEntryPtr->MDEP_ID;
                        Event.EventData.DataConnectionStatusEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                        Event.EventData.DataConnectionStatusEventData.Status              = ConnectionStatus;

                        /* Release the Lock so we can make the callback.*/
                        DEVM_ReleaseLock();

                        __BTPSTRY
                        {
                           if(DataEntryPtr->EventCallback.CallbackFunction)
                              (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                        }
                        __BTPSEXCEPT(1)
                        {
                           /* Do Nothing.                               */
                        }

                        /* Re-acquire the Lock.                         */
                        DEVM_AcquireLock();
                     }
                     else
                     {
                        /* This connection is owned by a remote client. */
                        BTPS_MemInitialize(&Message, 0, HDPM_DATA_DISCONNECTED_MESSAGE_SIZE);

                        Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                        Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                        Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                        Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_STATUS;
                        Message.MessageHeader.MessageLength   = (HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                        Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                        Message.Instance                      = ConnectionPtr->Instance;
                        Message.EndpointID                    = DataEntryPtr->MDEP_ID;
                        Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                        Message.Status                        = ConnectionStatus;

                        /* Finally dispatch the Message.                */
                        MSG_SendMessage((BTPM_Message_t *)&Message);
                     }
                  }
                  else
                  {
                     /* This data channel is not owned by any client.   */
                     /* Since no one is available to exchange data,     */
                     /* close the connection.                           */
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connected data channel is not owned by any client: %d.\n", DataLinkConnectConfirmationData->DataLinkID));

                     if(DataLinkConnectConfirmationData->Status == HDPM_CONNECTION_STATUS_SUCCESS)
                     {
                        _HDPM_Disconnect_Data_Channel(DataLinkConnectConfirmationData->MCLID, DataLinkConnectConfirmationData->DataLinkID);
                     }
                     else
                     {
                        DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                        FreeDataChannelEntryMemory(DataEntryPtr);
                     }
                  }

                  /* If the connection was unsuccessful (so the Data    */
                  /* Link entry was removed from the list, above), free */
                  /* any unneeded resources.                            */
                  if(ConnectionStatus != HDPM_CONNECTION_STATUS_SUCCESS)
                     FreeDataChannelEntryMemory(DataEntryPtr);
               }
            }
            else
            {
               /* Mismatch between control channel and data channel     */
               /* MCLIDs.                                               */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel does not match control channel referenced in response: %d, %d.\n", DataLinkConnectConfirmationData->MCLID, ConnectionPtr->MCLID));

               if(DataLinkConnectConfirmationData->Status == HDPM_CONNECTION_STATUS_SUCCESS)
               {
                  _HDPM_Disconnect_Data_Channel(DataLinkConnectConfirmationData->MCLID, DataLinkConnectConfirmationData->DataLinkID);
               }
               else
               {
                  if(DataEntryPtr->ConnectionEvent)
                  {
                     DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

                     BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
                  }
                  else
                  {
                     DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                     FreeDataChannelEntryMemory(DataEntryPtr);
                  }
               }
            }
         }
         else
         {
            /* No control connection is known for the MCLID of this data*/
            /* connection. Close the connection immediately.            */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel references an invalid control channel connection: %d, %d.\n", DataLinkConnectConfirmationData->DataLinkID, DataLinkConnectConfirmationData->MCLID));

            if(DataLinkConnectConfirmationData->Status == HDPM_CONNECTION_STATUS_SUCCESS)
            {
               _HDPM_Disconnect_Data_Channel(DataLinkConnectConfirmationData->MCLID, DataLinkConnectConfirmationData->DataLinkID);
            }
            else
            {
               if(DataEntryPtr->ConnectionEvent)
               {
                  DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

                  BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
               }
               else
               {
                  DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
            }
         }
      }
      else
      {
         /* No connection is known for this DataLinkID. Any confirmation*/
         /* must be the result of a previous connection request, so     */
         /* perhaps the connection attempt was cancelled. Close the     */
         /* connection immediately.                                     */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel already cleaned up: %d.\n", DataLinkConnectConfirmationData->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDataLinkDisconnectIndicationEvent(HDP_Data_Link_Disconnect_Indication_Data_t *DataLinkDisconnectIndicationData)
{
   HDPM_Event_Data_t                 Event;
   Connection_Entry_t               *ConnectionPtr;
   Data_Channel_Entry_t             *DataEntryPtr;
   HDPM_Data_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameters appear to be semi-valid.           */
   if((DataLinkDisconnectIndicationData) && (DataLinkDisconnectIndicationData->MCLID) && (DataLinkDisconnectIndicationData->DataLinkID))
   {
      /* Retrieve the associated connection object from the list.       */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkDisconnectIndicationData->DataLinkID)) != NULL)
      {
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel connection closed: %d\n", DataEntryPtr->DataLinkID));

            /* Check to see if we need to dispatch the event or set an  */
            /* internal event.                                          */
            if(DataEntryPtr->ConnectionEvent)
            {
               /* Note the Status.                                      */
               DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;

               BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
            }
            else
            {
               /* Remove the associated data channel object from the    */
               /* data channel list now, so it is not accessible during */
               /* any client callback.                                  */
               DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);

               /* Confirm that this data connection has a registered    */
               /* owner to receive the event.                           */
               if(CallbackRegistrationValid(&(DataEntryPtr->EventCallback)))
               {
                  /* Determine whether the notification is going to a   */
                  /* local callback or remote client.                   */
                  if(DataEntryPtr->EventCallback.CallbackFunction)
                  {
                     /* A local callback is registered.                 */
                     BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                     Event.EventType                                               = hetHDPDataDisconnected;
                     Event.EventLength                                             = HDPM_DATA_DISCONNECTED_EVENT_DATA_SIZE;

                     Event.EventData.DataDisconnectedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                     Event.EventData.DataDisconnectedEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                     Event.EventData.DataDisconnectedEventData.Reason              = HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT;

                     /* Release the Lock so we can make the callback.   */
                     DEVM_ReleaseLock();

                     __BTPSTRY
                     {
                        if(DataEntryPtr->EventCallback.CallbackFunction)
                           (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Re-acquire the Lock.                            */
                     DEVM_AcquireLock();
                  }
                  else
                  {
                     /* This connection is owned by a remote client.    */
                     BTPS_MemInitialize(&Message, 0, HDPM_DATA_DISCONNECTED_MESSAGE_SIZE);

                     Message.MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                     Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
                     Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                     Message.MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED;
                     Message.MessageHeader.MessageLength   = (HDPM_DATA_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                     Message.RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                     Message.DataLinkID                    = DataEntryPtr->DataLinkID;
                     Message.Reason                        = HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT;

                     /* Finally dispatch the Message.                   */
                     MSG_SendMessage((BTPM_Message_t *)&Message);
                  }
               }

               /* Free the data channel object.                         */
               FreeDataChannelEntryMemory(DataEntryPtr);
            }
         }
         else
         {
            /* No control connection is known for the MCLID of this data*/
            /* connection. Log this event, then clean up.               */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel disconnection references an unknown control channel connection: %d, %d.\n", DataLinkDisconnectIndicationData->DataLinkID, DataLinkDisconnectIndicationData->MCLID));

            if(DataEntryPtr->ConnectionEvent)
            {
               DataEntryPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

               BTPS_SetEvent(DataEntryPtr->ConnectionEvent);
            }
            else
            {
               DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
               FreeDataChannelEntryMemory(DataEntryPtr);
            }
         }
      }
      else
      {
         /* No connection is known for this DataLinkID. Any confirmation*/
         /* must be the result of a previous connection request, so     */
         /* perhaps the connection attempt was cancelled. Close the     */
         /* connection immediately.                                     */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data channel already cleaned up: %d.\n", DataLinkDisconnectIndicationData->DataLinkID));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


static void ProcessDataLinkDataIndicationEvent(HDP_Data_Link_Data_Indication_Data_t *DataLinkDataIndicationData)
{
   HDPM_Event_Data_t             Event;
   Connection_Entry_t           *ConnectionPtr;
   Data_Channel_Entry_t         *DataEntryPtr;
   HDPM_Data_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((DataLinkDataIndicationData) && (DataLinkDataIndicationData->DataLinkID) && (DataLinkDataIndicationData->DataLength) && (DataLinkDataIndicationData->DataPtr))
   {
      /* Locate the data channel object for this link.                  */
      if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkDataIndicationData->DataLinkID)) != NULL)
      {
         /* Next map the Health Device MCLID to a connection entry we   */
         /* are tracking.                                               */
         if((ConnectionPtr = SearchConnectionEntryMCLID(&ConnectionList, DataLinkDataIndicationData->MCLID)) != NULL)
         {
            if(DataEntryPtr->MCLID == ConnectionPtr->MCLID)
            {
               /* Determine whether the notification is going to a local*/
               /* callback or remote client.                            */
               if(DataEntryPtr->EventCallback.CallbackFunction)
               {
                  /* A local callback is registered.                    */
                  BTPS_MemInitialize(&Event, 0, HDPM_EVENT_DATA_SIZE);

                  Event.EventType                                           = hetHDPDataReceived;
                  Event.EventLength                                         = HDPM_DATA_RECEIVED_EVENT_DATA_SIZE;

                  Event.EventData.DataReceivedEventData.RemoteDeviceAddress = ConnectionPtr->BD_ADDR;
                  Event.EventData.DataReceivedEventData.DataLinkID          = DataEntryPtr->DataLinkID;
                  Event.EventData.DataReceivedEventData.DataLength          = DataLinkDataIndicationData->DataLength;
                  Event.EventData.DataReceivedEventData.Data                = DataLinkDataIndicationData->DataPtr;

                  /* Release the Lock so we can make the callback.      */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(DataEntryPtr->EventCallback.CallbackFunction)
                        (DataEntryPtr->EventCallback.CallbackFunction)(&Event, DataEntryPtr->EventCallback.CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
               else
               {
                  /* This connection is owned by a remote client.       */
                  if((Message = (HDPM_Data_Received_Message_t *)BTPS_AllocateMemory(HDPM_DATA_RECEIVED_MESSAGE_SIZE(DataLinkDataIndicationData->DataLength))) != NULL)
                  {
                     BTPS_MemInitialize(Message, 0, HDPM_DATA_RECEIVED_MESSAGE_SIZE(DataLinkDataIndicationData->DataLength));

                     Message->MessageHeader.AddressID       = DataEntryPtr->EventCallback.ClientID;
                     Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
                     Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER;
                     Message->MessageHeader.MessageFunction = HDPM_MESSAGE_FUNCTION_DATA_RECEIVED;
                     Message->MessageHeader.MessageLength   = (HDPM_DATA_RECEIVED_MESSAGE_SIZE(DataLinkDataIndicationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE);

                     Message->RemoteDeviceAddress           = ConnectionPtr->BD_ADDR;
                     Message->DataLinkID                    = DataEntryPtr->DataLinkID;
                     Message->DataLength                    = DataLinkDataIndicationData->DataLength;

                     BTPS_MemCopy(Message->Data, DataLinkDataIndicationData->DataPtr, DataLinkDataIndicationData->DataLength);

                     /* Finally dispatch the Message.                   */
                     MSG_SendMessage((BTPM_Message_t *)Message);

                     BTPS_FreeMemory(Message);
                  }
               }
            }
            else
            {
               /* The data has arrived citing a DataLinkID and MCLID    */
               /* that are not associated. Close the data link.         */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming data claims to be from a data link and connection that do not match: %d, %d\n", DataLinkDataIndicationData->DataLinkID, DataLinkDataIndicationData->MCLID));

               _HDPM_Disconnect_Data_Channel(DataLinkDataIndicationData->MCLID, DataLinkDataIndicationData->DataLinkID);
            }
         }
         else
         {
            /* No control channel object exists for this connection.    */
            /* Close the data link.                                     */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming data is on a data link for an unknown control channel: %d, %d\n", DataLinkDataIndicationData->DataLinkID, DataLinkDataIndicationData->MCLID));

            _HDPM_Disconnect_Data_Channel(DataLinkDataIndicationData->MCLID, DataLinkDataIndicationData->DataLinkID);
         }
      }
      else
      {
         /* No data channel object exists for this connection. Close the*/
         /* data link.                                                  */
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming data is on an unknown data link: %d, %d\n", DataLinkDataIndicationData->DataLinkID, DataLinkDataIndicationData->MCLID));

         _HDPM_Disconnect_Data_Channel(DataLinkDataIndicationData->MCLID, DataLinkDataIndicationData->DataLinkID);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSyncCapabilitiesIndicationEvent(HDP_Sync_Capabilities_Indication_t *SyncCapabilitiesIndicationData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* XXX Not Supported                                                 */

   /* At a minimum, respond that this operation is unsupported in case a*/
   /* remote HDP instance requests our Sync Capabilities.               */
   _HDPM_Sync_Capabilities_Response(SyncCapabilitiesIndicationData->MCLID, 0, 0, 0, 0, HDP_RESPONSE_CODE_INVALID_OPERATION);

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSyncCapabilitiesConfirmationEvent(HDP_Sync_Capabilities_Confirmation_t *SyncCapabilitiesConfirmationData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* XXX Not Supported                                                 */

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSyncSetIndicationEvent(HDP_Sync_Set_Indication_t *SyncSetIndicationData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* XXX Not Supported                                                 */

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSyncSetConfirmationEvent(HDP_Sync_Set_Confirmation_t *SyncSetConfirmationData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* XXX Not Supported                                                 */

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSyncInfoIndicationEvent(HDP_Sync_Info_Indication_t *SyncInfoIndicationData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* XXX Not Supported                                                 */

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessHealthDeviceEvent(HDPM_HDP_Event_Data_t *HealthDeviceEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(HealthDeviceEventData)
   {
      /* Process the event based on the event type.                     */
      switch(HealthDeviceEventData->EventType)
      {
         case etHDP_Connect_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Request Indication\n"));

            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Received unexpected connection request, indicating possible misconfiguration (HDPM automatically accepts control connections).\n"));
            break;
         case etHDP_Control_Connect_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Connect Indication\n"));

            ProcessControlConnectIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Connect_Indication_Data));
            break;
         case etHDP_Control_Connect_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Connect Confirmation\n"));

            ProcessControlConnectConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Connect_Confirmation_Data));
            break;
         case etHDP_Control_Disconnect_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Disconnect Indication\n"));

            ProcessControlDisconnectIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Disconnect_Indication_Data));
            break;
         case etHDP_Control_Create_Data_Link_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Create Data Link Indication\n"));

            ProcessControlCreateDataLinkIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Create_Data_Link_Indication_Data));
            break;
         case etHDP_Control_Create_Data_Link_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Data Link Confirmation\n"));

            ProcessControlCreateDataLinkConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Create_Data_Link_Confirmation_Data));
            break;
         case etHDP_Control_Abort_Data_Link_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Abort Data Link Indication\n"));

            ProcessControlAbortDataLinkIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Abort_Data_Link_Indication_Data));
            break;
         case etHDP_Control_Abort_Data_Link_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Abort Data Link Confirmation\n"));

            ProcessControlAbortDataLinkConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Abort_Data_Link_Confirmation_Data));
            break;
         case etHDP_Control_Delete_Data_Link_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Delete Data Link Indication\n"));

            ProcessControlDeleteDataLinkIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Delete_Data_Link_Indication_Data));
            break;
         case etHDP_Control_Delete_Data_Link_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Delete Data Link Confirmation\n"));

            ProcessControlDeleteDataLinkConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Control_Delete_Data_Link_Confirmation_Data));
            break;
         case etHDP_Data_Link_Connect_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Link Connect Indication\n"));

            ProcessDataLinkConnectIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Data_Link_Connect_Indication_Data));
            break;
         case etHDP_Data_Link_Connect_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Link Connect Confirmation\n"));

            ProcessDataLinkConnectConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Data_Link_Connect_Confirmation_Data));
            break;
         case etHDP_Data_Link_Disconnect_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Link Disconnect Indication\n"));

            ProcessDataLinkDisconnectIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Data_Link_Disconnect_Indication_Data));
            break;
         case etHDP_Data_Link_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Link Data Indication\n"));

            ProcessDataLinkDataIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Data_Link_Data_Indication_Data));
            break;
         case etHDP_Sync_Capabilities_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Sync Capabilities Indication\n"));

            ProcessSyncCapabilitiesIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Sync_Capabilities_Indication_Data));
            break;
         case etHDP_Sync_Capabilities_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Sync Capabilities Confirmation\n"));

            ProcessSyncCapabilitiesConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Sync_Capabilities_Confirmation_Data));
            break;
         case etHDP_Sync_Set_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Sync Set Indication\n"));

            ProcessSyncSetIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Sync_Set_Indication_Data));
            break;
         case etHDP_Sync_Set_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Sync Set Confirmation\n"));

            ProcessSyncSetConfirmationEvent(&(HealthDeviceEventData->EventData.HDP_Sync_Set_Confirmation_Data));
            break;
         case etHDP_Sync_Info_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Sync Info Indication\n"));

            ProcessSyncInfoIndicationEvent(&(HealthDeviceEventData->EventData.HDP_Sync_Info_Indication_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown Health Device Event Type: %d\n", HealthDeviceEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Health Device Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI BTPMDispatchCallback_DEVM(void *CallbackParameter)
{
   int                                       Result;
   int                                       ConnectionStatus;
   Connection_Entry_t                       *TmpPtr;
   Connection_Entry_t                       *ConnectionPtr;
   HDP_Control_Disconnect_Indication_Data_t  ControlDisconnectIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HDPM): 0x%08X, %d\n", ((DEVM_Status_t *)CallbackParameter)->StatusType, ((DEVM_Status_t *)CallbackParameter)->Status));

   /* Device Manager Dispatch Callback.                                 */

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Verify that the input parameter appears to be semi-valid.   */
         if(CallbackParameter)
         {
            /* Cycle through all pending connections to this device.    */
            ConnectionPtr = ConnectionList;

            while(ConnectionPtr)
            {
               if(COMPARE_BD_ADDR(ConnectionPtr->BD_ADDR, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR))
               {
                  switch(((DEVM_Status_t *)CallbackParameter)->StatusType)
                  {
                     case dstConnection:
                     case dstAuthentication:
                     case dstEncryption:
                        if(ConnectionPtr->Server == FALSE)
                        {
                           /* This is an outgoing connection.           */
                           if(ConnectionPtr->ConnectionState == csConnectingDevice)
                           {
                              DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Found pending outgoing connection for this device.\n"));

                              /* Map the DEVM status to an HDPM         */
                              /* connection status.                     */
                              switch(((DEVM_Status_t *)CallbackParameter)->Status)
                              {
                                 case 0:
                                    ConnectionStatus = HDPM_CONNECTION_STATUS_SUCCESS;
                                    break;
                                 case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                                 case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                                    ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_REFUSED;
                                    break;
                                 case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
                                 case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                                    ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_TIMEOUT;
                                    break;
                                 default:
                                    ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                                    break;
                              }

                              /* We were waiting for a connection to the*/
                              /* device.                                */
                              if(ConnectionStatus != HDPM_CONNECTION_STATUS_SUCCESS)
                              {
                                 /* Connection attempt failed.  First,  */
                                 /* go ahead and disconnect the device. */
                                 DEVM_DisconnectRemoteDevice(ConnectionPtr->BD_ADDR, FALSE);

                                 /* Save the next connection object.    */
                                 TmpPtr = ConnectionPtr->NextEntry;

                                 /* Check to see if there is a          */
                                 /* synchronous open operation.         */
                                 if(ConnectionPtr->ConnectionEvent)
                                 {
                                    /* Interrupt the synchronous        */
                                    /* connection attempt.              */
                                    ConnectionPtr->ConnectionStatus = ConnectionStatus;

                                    BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
                                 }
                                 else
                                 {
                                    /* Remove the current connection    */
                                    /* object from the list.            */
                                    DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);

                                    /* Notify the client that was       */
                                    /* waiting on this connection, if   */
                                    /* possible.                        */
                                    if(CallbackRegistrationValid(&(ConnectionPtr->EventCallback)))
                                       DispatchControlConnectConfirmationEvent(ConnectionPtr, ConnectionStatus);

                                    /* And now clean up the connection  */
                                    /* object.                          */
                                    FreeConnectionEntryMemory(ConnectionPtr);
                                 }

                                 /* Move to the next connection object. */
                                 ConnectionPtr = TmpPtr;
                              }
                              else
                              {
                                 /* The device is connected             */
                                 /* successfully, so proceed with the   */
                                 /* HDP profile connection.             */

                                 /* Set the state to connecting the     */
                                 /* profile.                            */
                                 ConnectionPtr->ConnectionState = csConnecting;

                                 /* Initiate the HDP connection to the  */
                                 /* remote instance.                    */
                                 if((Result = _HDPM_Connect_Remote_Instance(ConnectionPtr->BD_ADDR, ConnectionPtr->Instance)) <= 0)
                                 {
                                    DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Profile connection attempt failed: %d\n", Result));

                                    /* Error, go ahead and disconnect   */
                                    /* the device.                      */
                                    DEVM_DisconnectRemoteDevice(ConnectionPtr->BD_ADDR, FALSE);

                                    /* Connection attempt failed.       */
                                    /* First, save the next connection  */
                                    /* object.                          */
                                    TmpPtr = ConnectionPtr->NextEntry;

                                    /* Check to see if there is a       */
                                    /* synchronous open operation.      */
                                    if(ConnectionPtr->ConnectionEvent)
                                    {
                                       /* Interrupt the synchronous     */
                                       /* connection attempt.           */
                                       ConnectionPtr->ConnectionStatus = ConnectionStatus;

                                       BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
                                    }
                                    else
                                    {
                                       /* Remove the newly added        */
                                       /* connection entry.             */
                                       DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);

                                       /* Notify the client that was    */
                                       /* waiting on this connection, if*/
                                       /* possible.                     */
                                       if(CallbackRegistrationValid(&(ConnectionPtr->EventCallback)))
                                          DispatchControlConnectConfirmationEvent(ConnectionPtr, HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN);

                                       /* Clean up the connection       */
                                       /* object.                       */
                                       FreeConnectionEntryMemory(ConnectionPtr);
                                    }

                                    /* Move to the next connection      */
                                    /* object.                          */
                                    ConnectionPtr = TmpPtr;
                                 }
                                 else
                                 {
                                    DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Profile connection started successfully: %d\n", Result));

                                    /* Note the MCLID.                  */
                                    ConnectionPtr->MCLID = (unsigned int)Result;

                                    /* Flag success.                    */
                                    Result               = 0;

                                    ConnectionPtr = ConnectionPtr->NextEntry;
                                 }
                              }
                           }
                           else
                           {
                              /* Skip this connection object since it is*/
                              /* not in the expected state.             */
                              ConnectionPtr = ConnectionPtr->NextEntry;
                           }
                        }
                        else
                        {
                           /* Incoming connection.                      */

                           /* XXX If we force encryption on incoming    */
                           /* connections, handle the result of the     */
                           /* encryption request here.                  */
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming connection DEVM event: %d, %02x:%02x:%02x:%02x:%02x:%02x, %d\n", ((DEVM_Status_t *)CallbackParameter)->StatusType, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR.BD_ADDR5, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR.BD_ADDR4, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR.BD_ADDR3, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR.BD_ADDR2, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR.BD_ADDR1, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR.BD_ADDR0, ((DEVM_Status_t *)CallbackParameter)->Status));

                           ConnectionPtr = ConnectionPtr->NextEntry;
                        }
                        break;
                     case dstDisconnected:
                        /* The ACL was dropped. HDP will send us        */
                        /* indications of the data channels going down, */
                        /* but not the control channel. Simulate the    */
                        /* control channel event.                       */
                        while((ConnectionPtr = SearchConnectionEntryBDADDR(&ConnectionList, ((DEVM_Status_t *)CallbackParameter)->BD_ADDR)) != NULL)
                        {
                           if((ConnectionPtr->MCLID == 0) || (ConnectionPtr->ConnectionState == csConnecting))
                           {
                              /* Check to see if there is a synchronous */
                              /* open operation.                        */
                              if(ConnectionPtr->ConnectionEvent)
                              {
                                 /* Interrupt the synchronous connection*/
                                 /* attempt.                            */
                                 ConnectionPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED;

                                 BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
                              }
                              else
                              {
                                 /* Remove the connection from the      */
                                 /* list now, so it is not accessible   */
                                 /* during the client callback.  It     */
                                 /* will be cleaned up after the client */
                                 /* notification.                       */
                                 DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);

                                 DispatchControlConnectConfirmationEvent(ConnectionPtr, HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED);

                                 /* Free the connection pointer that was*/
                                 /* deleted just above.                 */
                                 FreeConnectionEntryMemory(ConnectionPtr);
                              }
                           }
                           else
                           {
                              if(ConnectionPtr->ConnectionState == csConnected)
                              {
                                 ControlDisconnectIndicationData.MCLID = ConnectionPtr->MCLID;

                                 /* Each call to this function will     */
                                 /* remove the corresponding connection */
                                 /* entry from the list, invalidating   */
                                 /* ConnectionPtr, so we restart the    */
                                 /* search from the beginning of the    */
                                 /* list on each pass.                  */
                                 ProcessControlDisconnectIndicationEvent(&ControlDisconnectIndicationData);
                              }
                              else
                              {
                                 DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("HDPM Unexpected Error: Received ACL disconnect for device that is not connecting or connected.\n"));

                                 /* Remove the connection from the list */
                                 /* and free its memory.                */
                                 DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr);
                                 FreeConnectionEntryMemory(ConnectionPtr);
                              }
                           }
                        }
                        break;
                     default:
                        break;
                  }
               }
               else
               {
                  ConnectionPtr = ConnectionPtr->NextEntry;
               }
            }
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   if(CallbackParameter)
      BTPS_FreeMemory(CallbackParameter);

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HDPM)\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HDP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HDPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Health Device Profile Notification Events.  */
static void BTPSAPI BTPMDispatchCallback_HDP(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is a Health Device Event Update.  */
            if(((HDPM_Update_Data_t *)CallbackParameter)->UpdateType == utHDPEvent)
            {
               /* Process the Notification.                             */
               ProcessHealthDeviceEvent(&(((HDPM_Update_Data_t *)CallbackParameter)->UpdateData.HDPEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI HealthDeviceManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Health Device Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a Health Device Manager  */
            /* defined Message.  If it is it will be within the range: -*/
            /* BTPM_MESSAGE_FUNCTION_MINIMUM -                          */
            /* BTPM_MESSAGE_FUNCTION_MAXIMUM See BTPMMSGT.h for more    */
            /* information on message functions that are defined outside*/
            /* of this range.                                           */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
            {
               /* Still processing, go ahead and post the message to the*/
               /* Health Device Manager Thread.                         */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Health Device Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Health Device Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Health Device Manager */
            /* defined Message.  If it is it will be within the range: -*/
            /* BTPM_MESSAGE_FUNCTION_MINIMUM -                          */
            /* BTPM_MESSAGE_FUNCTION_MAXIMUM See BTPMMSGT.h for more    */
            /* information on message functions that are defined outside*/
            /* of this range.                                           */
            if((Message->MessageHeader.MessageFunction >= BTPM_MESSAGE_FUNCTION_MINIMUM) && (Message->MessageHeader.MessageFunction <= BTPM_MESSAGE_FUNCTION_MAXIMUM))
               MSG_FreeReceivedMessageGroupHandlerMessage(Message);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Health Device Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HDP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HDPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                   Result;
   Connection_Entry_t   *ConnectionPtr;
   Data_Channel_Entry_t *DataChannelPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      /* Check to see if this module has already been initialized.      */
      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Health Device\n"));

            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Health Device Manager messages.               */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER, HealthDeviceManagerGroupHandler, NULL))
            {
               /* Initialize the actual Health Device Manager           */
               /* Implementation Module (this is the module that is     */
               /* actually responsible for actually implementing the    */
               /* Health Device Manager functionality - this module is  */
               /* just the framework shell).                            */
               if(!(Result = _HDPM_Initialize((HDPM_Initialization_Info_t *)InitializationData)))
               {
                  // XXX Initialize state variables

                  /* Finally determine the current Device Power State.  */
                  CurrentPowerState = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Go ahead and flag that this module is initialized. */
                  Initialized       = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

            /* If an error occurred then we need to free all resources  */
            /* that were allocated.                                     */
            if(Result)
            {
               _HDPM_Cleanup();

               MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER);
            }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Health Device Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the Health Device Manager            */
            /* Implementation that we are shutting down.                */
            _HDPM_Cleanup();

            /* Loop through all outgoing connections to determine       */
            /* if there are any synchronous connection attempts         */
            /* outstanding.                                             */
            ConnectionPtr = ConnectionList;

            while(ConnectionPtr)
            {
               /* Check to see if there is a synchronous open operation.*/
               if(ConnectionPtr->ConnectionEvent)
               {
                  /* Interrupt the synchronous connection attempt.      */
                  ConnectionPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
               }

               ConnectionPtr = ConnectionPtr->NextEntry;
            }

            /* Loop through all outgoing data channels to determine     */
            /* if there are any synchronous connection attempts         */
            /* outstanding.                                             */
            DataChannelPtr = DataChannelList;

            while(DataChannelPtr)
            {
               /* Check to see if there is a synchronous open operation.*/
               if(DataChannelPtr->ConnectionEvent)
               {
                  /* Interrupt the synchronous connection attempt.      */
                  DataChannelPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                  BTPS_SetEvent(DataChannelPtr->ConnectionEvent);
               }

               DataChannelPtr = DataChannelPtr->NextEntry;
            }

            /* Clean up state tracking lists                            */
            FreeEndpointEntryList(&EndpointList);
            FreeConnectionEntryList(&ConnectionList);
            FreeDataChannelEntryList(&DataChannelList);

            /* Flag that the resources are no longer allocated.         */
            CurrentPowerState     = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized           = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                   Result;
   DEVM_Status_t        *DEVMStatus;
   Endpoint_Entry_t     *EndpointPtr;
   Connection_Entry_t   *ConnectionPtr;
   Data_Channel_Entry_t *DataChannelPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HDP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the Hands Free Manager that it    */
               /* should initialize.                                    */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _HDPM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Powering Off Occurred.\n"));

//XXX Disconnect Instance connections while running through list?

               /* Loop through all outgoing connections to determine    */
               /* if there are any synchronous connection attempts      */
               /* outstanding.                                          */
               ConnectionPtr = ConnectionList;

               while(ConnectionPtr)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(ConnectionPtr->ConnectionEvent)
                  {
                     /* Interrupt the synchronous connection attempt.   */
                     ConnectionPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(ConnectionPtr->ConnectionEvent);
                  }

                  ConnectionPtr = ConnectionPtr->NextEntry;
               }

               /* Loop through all outgoing data channels to determine  */
               /* if there are any synchronous connection attempts      */
               /* outstanding.                                          */
               DataChannelPtr = DataChannelList;

               while(DataChannelPtr)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(DataChannelPtr->ConnectionEvent)
                  {
                     /* Interrupt the synchronous connection attempt.   */
                     DataChannelPtr->ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(DataChannelPtr->ConnectionEvent);
                  }

                  DataChannelPtr = DataChannelPtr->NextEntry;
               }

               /* Cleanup the endpoint list.                            */
               EndpointPtr = EndpointList;

               while(EndpointPtr)
               {
                  _HDPM_Un_Register_Endpoint(EndpointPtr->MDEP_ID, EndpointPtr->MDEP_DataType, EndpointPtr->MDEP_Role);

                  EndpointPtr = EndpointPtr->NextEntry;
               }

               FreeEndpointEntryList(&EndpointList);

               /* Inform the Hands Free Manager that the Stack has been */
               /* closed.                                               */
               _HDPM_SetBluetoothStackID(0);

               /* Finally free all incoming/outgoing connection entries */
               /* (as there cannot be any active connections).          */
               FreeConnectionEntryList(&ConnectionList);

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               break;
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               break;
            case detRemoteDeviceAuthenticationStatus:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Authentication Status.\n"));

               if((DEVMStatus = (DEVM_Status_t *)BTPS_AllocateMemory(sizeof(DEVM_Status_t))) != NULL)
               {
                  DEVMStatus->StatusType = dstAuthentication;
                  DEVMStatus->BD_ADDR    = EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress;
                  DEVMStatus->Status     = EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status;

                  if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)DEVMStatus))
                     BTPS_FreeMemory(DEVMStatus);
               }
               break;
            case detRemoteDeviceEncryptionStatus:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Encryption Status.\n"));

               if((DEVMStatus = (DEVM_Status_t *)BTPS_AllocateMemory(sizeof(DEVM_Status_t))) != NULL)
               {
                  DEVMStatus->StatusType = dstEncryption;
                  DEVMStatus->BD_ADDR    = EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress;
                  DEVMStatus->Status     = EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status;

                  if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)DEVMStatus))
                     BTPS_FreeMemory(DEVMStatus);
               }
               break;
            case detRemoteDeviceConnectionStatus:
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connection Status.\n"));

               if((DEVMStatus = (DEVM_Status_t *)BTPS_AllocateMemory(sizeof(DEVM_Status_t))) != NULL)
               {
                  DEVMStatus->StatusType = dstConnection;
                  DEVMStatus->BD_ADDR    = EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress;
                  DEVMStatus->Status     = EventData->EventData.RemoteDeviceConnectionStatusEventData.Status;

                  if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)DEVMStatus))
                     BTPS_FreeMemory(DEVMStatus);
               }
               break;
            case detRemoteDevicePropertiesChanged:
               if((EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CONNECTION_STATE) && (!(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED)))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Properties Changed: Device Disconnected.\n"));

                  if((DEVMStatus = (DEVM_Status_t *)BTPS_AllocateMemory(sizeof(DEVM_Status_t))) != NULL)
                  {
                     DEVMStatus->StatusType = dstDisconnected;
                     DEVMStatus->BD_ADDR    = EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR;
                     DEVMStatus->Status     = 0;

                     if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)DEVMStatus))
                        BTPS_FreeMemory(DEVMStatus);
                  }
               }
               break;
            default:
               /* Do nothing.                                           */
               DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("DEVM Event: %d\n", EventData->EventType));
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Health Device Manager of a specific Update Event.   */
   /* The Health Device Manager can then take the correct action to     */
   /* process the update.                                               */
Boolean_t HDPM_NotifyUpdate(HDPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utHDPEvent:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Heath Device Event: %d\n", UpdateData->UpdateData.HDPEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDP, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to register an Endpoint on the Local HDP Server. The first*/
   /* parameter defines the Data Type that will be supported by this    */
   /* endpoint. The second parameter specifies whether the Endpoint     */
   /* will be a data source or sink. The third parameter is optional    */
   /* and can be used to specify a short, human-readable description of */
   /* the Endpoint. The final parameters specify the Event Callback and */
   /* Callback parameter (to receive events related to the registered   */
   /* endpoint). This function returns a positive, non-zero, value if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value represents the Endpoint ID that*/
   /*          can be used with various functions in this module to     */
   /*          refer to this endpoint.                                  */
int BTPSAPI HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t LocalRole, char *Description, HDPM_Event_Callback_t EventCallback, void *CallbackParameter)
{
   int               ret_val;
   Endpoint_Entry_t  EndpointEntry;
   Endpoint_Entry_t *EndpointEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %u\n", DataType, LocalRole));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(EventCallback)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* XXX                                                      */
            /* Currently, we support only one endpoint for each possible*/
            /* combination of Data Type and Role. This could be enhanced*/
            /* by either:                                               */
            /*     - allowing only one Type/Role pair per _client_ and  */
            /*       multiplexing incoming data connections according   */
            /*       to which client claimed the Control Channel when   */
            /*       the remote device initially connected to our local */
            /*       instance.                                          */
            /*     - allowing clients to register separate instance and */
            /*       restricting them to one Type/Role pair per         */
            /*       instance.                                          */

            /* Check whether an endpoint with this Type and Role has    */
            /* already been registered on this local Instance.          */
            if(SearchEndpointEntryTypeRole(&EndpointList, DataType, LocalRole) == NULL)
            {
               /* Attempt to register the new endpoint with the HDP     */
               /* framework.                                            */
               if((ret_val = _HDPM_Register_Endpoint(DataType, LocalRole, Description)) > 0)
               {
                  /* Record the registration locally.                   */
                  BTPS_MemInitialize(&EndpointEntry, 0, sizeof(Endpoint_Entry_t));

                  EndpointEntry.EventCallback.ClientID          = MSG_GetServerAddressID();
                  EndpointEntry.EventCallback.CallbackFunction  = EventCallback;
                  EndpointEntry.EventCallback.CallbackParameter = CallbackParameter;

                  EndpointEntry.MDEP_DataType                   = DataType;
                  EndpointEntry.MDEP_Role                       = LocalRole;

                  EndpointEntry.MDEP_ID = (Byte_t)ret_val;

                  /* Successfully registered. Now attempt to add it to  */
                  /* the registered endpoint list. This should always   */
                  /* succeed because the framework always assigns unique*/
                  /* MDEP_IDs.                                          */
                  if((EndpointEntryPtr = AddEndpointEntry(&EndpointList, &EndpointEntry)) == NULL)
                  {
                     /* Registration unexpectedly failed.               */
                     DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_CRITICAL), ("Unable to add Endpoint Registration entry to the list for MDEP: %d\n", EndpointEntry.MDEP_ID));

                     /* Remove the new endpoint registration.           */
                     _HDPM_Un_Register_Endpoint(EndpointEntry.MDEP_ID, EndpointEntry.MDEP_DataType, EndpointEntry.MDEP_Role);

                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }
               }
               else
               {
                  /* Failed to register the actual endpoint.  Return    */
                  /* failure to the client.                             */
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_ENDPOINT;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_ENDPOINT;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Endpoint. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI HDPM_Un_Register_Endpoint(unsigned int EndpointID)
{
   int               ret_val;
   Endpoint_Entry_t *EndpointEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", EndpointID));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* First, confirm that the client requesting to be             */
         /* un-registered is the same that created this registration.   */
         EndpointEntryPtr = SearchEndpointEntryMDEP(&EndpointList, (Byte_t)EndpointID);

         /* A valid local callback function indicates a local (PM       */
         /* Server) registration.                                       */
         if((EndpointEntryPtr) && (EndpointEntryPtr->EventCallback.CallbackFunction))
         {
            /* Remove the endpoint registration from the list.          */
            if((EndpointEntryPtr = DeleteEndpointEntryMDEP(&EndpointList, (Byte_t)EndpointID)) != NULL)
            {
               /* XXX                                                   */
               /* If support for multiplexing endpoints between multiple*/
               /* PM clients is added, we must check here whether the   */
               /* MDEP is still in use before unregistering it.         */

               /* This HDP endpoint is no longer in use, so it is safe  */
               /* to unregister it.                                     */
               ret_val = _HDPM_Un_Register_Endpoint(EndpointEntryPtr->MDEP_ID, EndpointEntryPtr->MDEP_DataType, EndpointEntryPtr->MDEP_Role);

               if(ret_val)
               {
                  /* If an error occurs in HDP while un-registering the */
                  /* endpoint, log the error but ignore it because the  */
                  /* client's HDPM registration has already been removed*/
                  /* successfully.                                      */
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Failed to unregister MDEP from HDP (%d)\n", ret_val));
               }

               /* The endpoint registration can now be deleted.         */
               FreeEndpointEntryMemory(EndpointEntryPtr);

               ret_val = 0;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_REGISTERED;
         }
         else
            ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_REGISTERED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to establish a data connection to a*/
   /* local endpoint. The first parameter is the DataLinkID associated  */
   /* with the connection request. The second parameter is one of       */
   /* the MCAP_RESPONSE_CODE_* constants which indicates either that    */
   /* the request should be accepted (MCAP_RESPONSE_CODE_SUCCESS) or    */
   /* provides a reason for rejecting the request. If the request is to */
   /* be accepted, and the request is for a local Data Source, the final*/
   /* parameter indicates whether the connection shall use the Reliable */
   /* or Streaming communication mode. This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Data Connected  */
   /*          event will be dispatched to signify the actual result.   */
   /* * NOTE * If the connection is accepted, and the connection request*/
   /*          is for a local Data Sink, then ChannelMode must be set to*/
   /*          the Mode indicated in the request.  If the connection is */
   /*          accepted for a local Data Source, ChannelMode must be set*/
   /*          to either cmReliable or cmStreaming. If the connection   */
   /*          request is rejected, ChannelMode is ignored.             */
int BTPSAPI HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode)
{
   int                        ret_val;
   Data_Channel_Entry_t      *DataEntryPtr;
   HDP_Channel_Config_Info_t  ConfigInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %u, %u\n", DataLinkID, ResponseCode, ChannelMode));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((DataLinkID) && (((ResponseCode == MCAP_RESPONSE_CODE_SUCCESS) && ((ChannelMode == cmReliable) || (ChannelMode == cmStreaming))) || ((ResponseCode > MCAP_RESPONSE_CODE_SUCCESS)  && (ResponseCode <= MCAP_RESPONSE_CODE_CONFIGURATION_REJECTED))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, verify that we are already tracking the referenced */
            /* data channel and that the caller has permission to accept*/
            /* this connection.                                         */
            if(((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkID)) != NULL) && (DataEntryPtr->EventCallback.CallbackFunction))
            {
               /* Verify that the data channel is pending authorization */
               /* and that this client is currently permitted to        */
               /* authorize the connection.                             */
               if(DataEntryPtr->ConnectionState == dcsAuthorizing)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Data Channel Request: %d, %d\n", DataLinkID, ResponseCode));

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(ResponseCode == MCAP_RESPONSE_CODE_SUCCESS)
                  {
                     /* Map this Data Channel back to the Control       */
                     /* Connection.                                     */
                     if(SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID))
                     {
                        /* Enable FCS if we are the Source.             */
                        ConfigInfo.FCSMode                  = ((DataEntryPtr->LocalRole == drSource) ? fcsEnabled : fcsNoPreference);
                        ConfigInfo.MaxTxPacketSize          = 2048;
                        ConfigInfo.TxSegmentSize            = 256;
                        ConfigInfo.NumberOfTxSegmentBuffers = 10;

                        /* Accept the data connection.                  */
                        if((ret_val = _HDPM_Data_Connection_Request_Response(DataLinkID, MCAP_RESPONSE_CODE_SUCCESS, ChannelMode, &ConfigInfo)) == 0)
                        {
                           /* XXX                                       */
                           /* For client multiplexing: If timers are    */
                           /* used to protect against the case where no */
                           /* client accepts the connection, clean them */
                           /* up, here.                                 */

                           /* Update the current data channel state.    */
                           DataEntryPtr->ConnectionState = dcsConnecting;
                        }
                     }
                     else
                     {
                        /* No associated control channel was found. This*/
                        /* should never happen because Bluetopia should */
                        /* have already rejected the connection attempt.*/
                        DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FAILURE), ("Received data channel connection request without associated control channel.\n"));

                        /* Reject the request.                          */
                        ret_val = _HDPM_Data_Connection_Request_Response(DataLinkID, MCAP_RESPONSE_CODE_INVALID_OPERATION, cmNoPreference, NULL);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */

                        /* XXX                                          */
                        /* For client multiplexing on endpoints, the    */
                        /* data channel object should _not_ be deleted  */
                        /* until all clients have had a chance to claim */
                        /* the control connection.                      */
                        if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                           FreeDataChannelEntryMemory(DataEntryPtr);
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     ret_val = _HDPM_Data_Connection_Request_Response(DataLinkID, ResponseCode, cmNoPreference, NULL);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                        FreeDataChannelEntryMemory(DataEntryPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
            }
            else
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_DATALINK_ID;

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
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to query the available HDP Instances on a remote    */
   /* device. The first parameter specifies the Address of the Remote   */
   /* Device to query. The second parameter specifies the maximum       */
   /* number of Instances that the buffer will support (i.e. can be     */
   /* copied into the buffer). The next parameter is optional and,      */
   /* if specified, will be populated with up to the total number of    */
   /* Instances advertised by the remote device, if the function is     */
   /* successful. The final parameter is optional and can be used to    */
   /* retrieve the total number of available Instances (regardless of   */
   /* the size of the list specified by the first two parameters).      */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Instances that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Instance  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int BTPSAPI HDPM_Query_Remote_Device_Instances(BD_ADDR_t RemoteDeviceAddress, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances)
{
   int                     ret_val;
   DEVM_Parsed_SDP_Data_t *ParsedSDPData;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, MaximumInstanceListEntries));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((!MaximumInstanceListEntries) || (InstanceList)))
      {
         /* Load cached SDP data into the buffer.                       */
         if((ret_val = GetParsedSDPData(RemoteDeviceAddress, &ParsedSDPData)) == 0)
         {
            /* Extract the Instance IDs.                                */
            ret_val = ParseSDPInstances(ParsedSDPData, MaximumInstanceListEntries, InstanceList, TotalNumberInstances);

            /* Release the parsed SDP data structures.                  */
            FreeParsedSDPData(ParsedSDPData);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the available Endpoints published for a specific */
   /* HDP Instances on a remote device. The first parameter specifies   */
   /* the Address of the Remote Device to query. The second parameter   */
   /* specifies Instance on the Remote Device. The third parameter      */
   /* specifies the maximum number of Endpoints that the buffer will    */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with up to the   */
   /* total number of Endpoints published by the remote device, if the  */
   /* function is successful. The final parameter is optional and can   */
   /* be used to retrieve the total number of Endpoints (regardless     */
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Endpoints that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Endpoint  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
int BTPSAPI HDPM_Query_Remote_Device_Instance_Endpoints(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints)
{
   int                     ret_val;
   DEVM_Parsed_SDP_Data_t *ParsedSDPData;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance, MaximumEndpointListEntries));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (Instance) && (((MaximumEndpointListEntries) && (EndpointInfoList)) || ((!MaximumEndpointListEntries) && (TotalNumberEndpoints))))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Load cached SDP data into the buffer.                    */
            if((ret_val = GetParsedSDPData(RemoteDeviceAddress, &ParsedSDPData)) == 0)
            {
               /* Determine the number of published HDP Endpoints for   */
               /* this instance.                                        */
               ret_val = ParseSDPEndpoints(ParsedSDPData, Instance, MaximumEndpointListEntries, EndpointInfoList, TotalNumberEndpoints);

               /* Release the parsed SDP data structures.               */
               FreeParsedSDPData(ParsedSDPData);
            }

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
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the description of a known Endpoint published in */
   /* a specific HDP Instance by a remote device. The first parameter   */
   /* specifies the Address of the Remote Device to query. The second   */
   /* parameter specifies Instance on the Remote Device. The third      */
   /* parameter identifies the Endpoint to query. The fourth and fifth  */
   /* parameters specific the size of the buffer and the buffer to hold */
   /* the description string, respectively. The final parameter is      */
   /* optional and, if specified, will be set to the total size of the  */
   /* description string for the given Endpoint, if the function is     */
   /* successful (regardless of the size of the list specified by the   */
   /* first two parameters). This function returns a non-negative value */
   /* if successful which represents the number of bytes copied into the*/
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum           */
   /*          Description Length, in which case the final parameter    */
   /*          *MUST* be specified.                                     */
int BTPSAPI HDPM_Query_Remote_Device_Endpoint_Description(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Endpoint_Info_t *EndpointInfo, unsigned int MaximumDescriptionLength, char *DescriptionBuffer, unsigned int *TotalDescriptionLength)
{
   int                     ret_val;
   unsigned int            DescriptionLength;
   unsigned int            NextEndpointIndex;
   HDP_MDEP_Info_t         MDEPInfo;
   DEVM_Parsed_SDP_Data_t *ParsedSDPData;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, %u, (%u,%u), %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance, (EndpointInfo ? EndpointInfo->EndpointID : (unsigned int)(-1)), (EndpointInfo ? EndpointInfo->DataType : (unsigned int)(-1)), MaximumDescriptionLength));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (VALID_INSTANCE(Instance)) && (EndpointInfo) && (VALID_MDEP_ID(EndpointInfo->EndpointID)) && (EndpointInfo->DataType) && (((MaximumDescriptionLength) && (DescriptionBuffer)) || ((!MaximumDescriptionLength) && (!DescriptionBuffer) && (TotalDescriptionLength))))
      {
         /* Load cached SDP data into the buffer.                       */
         if((ret_val = GetParsedSDPData(RemoteDeviceAddress, &ParsedSDPData)) == 0)
         {
            NextEndpointIndex = 0;
            DescriptionLength = 0;

            /* Walk the list of endpoints to find the requested MDEP    */
            /* definition.                                              */
            while(ret_val >= 0)
            {
               /* Locate the next Endpoint declaration.                 */
               if((ret_val = FindSDPEndpointInfo(ParsedSDPData, Instance, &NextEndpointIndex, EndpointInfo->EndpointID, &MDEPInfo, &DescriptionLength)) >= 0)
               {
                  /* An endpoint declaration was found.  Determine      */
                  /* whether this is the requested endpoint.            */
                  if(MDEPInfo.MDEP_DataType == EndpointInfo->DataType)
                  {
                     /* It is the requested endpoint, so stop the       */
                     /* search.                                         */
                     break;
                  }
               }
            }

            if(ret_val >= 0)
            {
               /* If requested, report the total length of the endpoint */
               /* description.                                          */
               if(TotalDescriptionLength)
                  *TotalDescriptionLength = DescriptionLength;

               /* If requested, copy the description into the provided  */
               /* buffer.                                               */
               if((MaximumDescriptionLength) && (DescriptionBuffer))
               {
                  /* Cap the amount of the description to be copied,    */
                  /* leaving room for a terminating NULL character.     */
                  if(DescriptionLength >= MaximumDescriptionLength)
                     DescriptionLength = (MaximumDescriptionLength - 1);

                  if(DescriptionLength > 0)
                  {
                     BTPS_MemCopy(DescriptionBuffer, MDEPInfo.MDEP_Description, DescriptionLength);

                     DescriptionBuffer[DescriptionLength] = '\0';
                  }

                  /* Return the number of bytes copied (not counting    */
                  /* NULL terminating character).                       */
                  ret_val = DescriptionLength;
               }
               else
               {
                  /* Return success, no bytes copied.                   */
                  ret_val = 0;
               }
            }

            /* Release the parsed SDP data structures.                  */
            FreeParsedSDPData(ParsedSDPData);
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a connection to a specific HDP Instance on a */
   /* Remote Device. The first parameter specifies the Remote Device to */
   /* connect to. The second parameter specifies the HDP Instance on the*/
   /* remote device. The next two parameters specify the (optional)     */
   /* Event Callback and Callback parameter (to receive events related  */
   /* to the connection attempt). This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int BTPSAPI HDPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (VALID_INSTANCE(Instance)) && (EventCallback))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, verify that we are not already tracking a          */
            /* connection to the specified device.                      */
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionList, RemoteDeviceAddress, Instance)) == NULL)
            {
               /* Entry is not present, go ahead and create a new entry.*/
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.BD_ADDR                         = RemoteDeviceAddress;
               ConnectionEntry.Instance                        = Instance;
               ConnectionEntry.MCLID                           = 0;
               ConnectionEntry.Server                          = FALSE;
               ConnectionEntry.EventCallback.ClientID          = MSG_GetServerAddressID();
               ConnectionEntry.EventCallback.CallbackFunction  = EventCallback;
               ConnectionEntry.EventCallback.CallbackParameter = CallbackParameter;
               ConnectionEntry.ConnectionState                 = csIdle;

               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionList, &ConnectionEntry)) != NULL)
               {
                  /* If we are not currently connected to the remote    */
                  /* device, do so now.                                 */
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device: %02X:%02X:%02X:%02X:%02X:%02X, 0x%08x\n", ConnectionEntry.BD_ADDR.BD_ADDR5, ConnectionEntry.BD_ADDR.BD_ADDR4, ConnectionEntry.BD_ADDR.BD_ADDR3, ConnectionEntry.BD_ADDR.BD_ADDR2, ConnectionEntry.BD_ADDR.BD_ADDR1, ConnectionEntry.BD_ADDR.BD_ADDR0, ConnectionEntry.Instance));

                  /* Next, attempt to open the remote device            */
                  ConnectionEntryPtr->ConnectionState = csConnectingDevice;

                  ret_val = DEVM_ConnectWithRemoteDevice(ConnectionEntryPtr->BD_ADDR, (DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT));

                  if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     /* Check to see if we need to actually issue the   */
                     /* Remote connection.                              */
                     if(ret_val < 0)
                     {
                        /* Set the state to connecting the profile.     */
                        ConnectionEntryPtr->ConnectionState = csConnecting;

                        if((ret_val = _HDPM_Connect_Remote_Instance(ConnectionEntryPtr->BD_ADDR, ConnectionEntryPtr->Instance)) <= 0)
                        {
                           ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;

                           /* Error opening device, go ahead and delete */
                           /* the entry that was added.                 */
                           if((ConnectionEntryPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionEntryPtr)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntryPtr);
                        }
                        else
                        {
                           /* Note the MCLID.                           */
                           ConnectionEntryPtr->MCLID = (unsigned int)ret_val;

                           /* Flag success.                             */
                           ret_val                   = 0;
                        }
                     }

                     /* Next, determine if the caller has requested a   */
                     /* blocking open.                                  */
                     if(ConnectionStatus)
                     {
                        /* Only wait for the connection if the          */
                        /* connection process was successfully started. */
                        if(!ret_val)
                        {
                           /* Allocate an event to signal success or    */
                           /* failure.                                  */
                           if((ConnectionEvent = BTPS_CreateEvent(TRUE)) != NULL)
                           {
                              ConnectionEntryPtr->ConnectionEvent = ConnectionEvent;

                              /* Release the Lock to avoid deadlocks    */
                              /* while waiting for the connection.      */
                              DEVM_ReleaseLock();

                              /* Wait for the connection to complete.   */
                              BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                              /* Re-acquire the Mutex.                  */
                              if(DEVM_AcquireLock())
                              {
                                 /* Re-acquire the connection entry.    */
                                 if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionList, RemoteDeviceAddress, Instance)) != NULL)
                                 {
                                    *ConnectionStatus = ConnectionEntryPtr->ConnectionStatus;

                                    /* Remove access to the connection  */
                                    /* event from the connection entry  */
                                    /* object.                          */
                                    ConnectionEntryPtr->ConnectionEvent = NULL;

                                    if(ConnectionEntryPtr->ConnectionStatus == HDPM_CONNECTION_STATUS_SUCCESS)
                                    {
                                       /* Connection succeeded.         */
                                       ret_val = 0;
                                    }
                                    else
                                    {
                                       /* Open failed, so note the      */
                                       /* status and remove the         */
                                       /* connection entry from the     */
                                       /* list.                         */
                                       ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;

                                       if((ConnectionEntryPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionEntryPtr)) != NULL)
                                          FreeConnectionEntryMemory(ConnectionEntryPtr);
                                    }
                                 }
                                 else
                                 {
                                    ret_val           = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;
                                    *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                                 }

                                 /* Release the Lock because we are     */
                                 /* finished with it.                   */
                                 DEVM_ReleaseLock();
                              }
                              else
                              {
                                 ret_val           = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                                 *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                              }

                              /* Free the event object allocated        */
                              /* previously.                            */
                              BTPS_CloseEvent(ConnectionEvent);
                           }
                           else
                           {
                              ret_val           = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
                              *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

                              /* Release the Lock because we are        */
                              /* finished with it.                      */
                              DEVM_ReleaseLock();
                           }
                        }
                        else
                        {
                           /* The connection process was not started.   */
                           *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        }
                     }
                     else
                     {
                        /* Release the Lock because we are finished with*/
                        /* it.                                          */
                        DEVM_ReleaseLock();
                     }
                  }
                  else
                  {
                     /* Error connecting to the device, go ahead and    */
                     /* delete the entry that was added.                */
                     if((ConnectionEntryPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionEntryPtr)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);

                     /* Release the Lock because we are finished with   */
                     /* it.                                             */
                     DEVM_ReleaseLock();
                  }
               }
               else
               {
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

                  /* Release the Lock because we are finished with it.  */
                  DEVM_ReleaseLock();
               }
            }
            else
            {
               if(ConnectionEntryPtr->EventCallback.ClientID == MSG_GetServerAddressID())
               {
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                     ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INSTANCE_ALREADY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_CONNECTION_IN_PROGRESS;
               }
               else
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_REMOTE_INSTANCE_IN_USE;

               /* Release the Lock because we are finished with it.     */
               DEVM_ReleaseLock();
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an existing connection to a specific HDP Instance*/
   /* on a Remote Device. The first parameter specifies the Remote      */
   /* Device. The second parameter specifies the HDP Instance on the    */
   /* remote device from which to disconnect. This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
int BTPSAPI HDPM_Disconnect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance)
{
   int                 ret_val;
   Connection_Entry_t *ConnectionPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (VALID_INSTANCE(Instance)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that we are tracking the connection and that the  */
            /* client who requested the disconnection has permission to */
            /* do so (owns the connection).                             */
            if(((ConnectionPtr = SearchConnectionEntry(&ConnectionList, RemoteDeviceAddress, Instance)) != NULL) && (ConnectionPtr->EventCallback.ClientID == MSG_GetServerAddressID()))
            {
               switch(ConnectionPtr->ConnectionState)
               {
                  case csConnected:
                  case csConnecting:
                     /* Nothing really to do other than to disconnect   */
                     /* the device (if it is connected, a disconnect    */
                     /* will be dispatched from the framework).         */
                     /* * NOTE * No need to clean up associated data    */
                     /*          channels. The framework will           */
                     /*          automatically close them and issue     */
                     /*          disconnect events before closing the   */
                     /*          control channel.                       */
                     ret_val = _HDPM_Disconnect_Remote_Instance(ConnectionPtr->MCLID);

                     /* XXX Do we need to fake a "Disconnected" message */
                     /* for the control channel? Based on a quick code  */
                     /* review, the framework appears to issue events   */
                     /* for the data channels, but possibly not the     */
                     /* control channel.                                */

                     /* XXX Could we see problems with disconnections   */
                     /* if we are currently in the "connecting" state?  */
                     /* That is, will HDP_Close_Connection abort an     */
                     /* in-progress connection attempt or will it throw */
                     /* a "not connected" error?                        */
                     break;

                  case csConnectingDevice:
                     /* Cancel the connection process.                  */
                     ret_val = DEVM_DisconnectRemoteDevice(RemoteDeviceAddress, 0);

                     break;

                  case csAuthorizing:
                     /* XXX Once multiple local instances are supported,*/
                     /* incoming connections in the authorizing state   */
                     /* will have to be rejected, here. Currently, this */
                     /* state is never used.                            */
                  case csIdle:
                     /* XXX Unused.  This is a (short-lived) placeholder*/
                     /* state when creating new connection entries.     */
                  default:
                     /* Do nothing in these cases.                      */
                     ret_val = 0;
               }

               if(!ret_val)
               {
                  /* The connection was successfully closed.  Issue     */
                  /* disconnection events and remove the connection     */
                  /* entry.                                             */

                  /* FIXME Simulate disconnection events here.          */

                  if((ConnectionPtr = DeleteConnectionEntryExact(&ConnectionList, ConnectionPtr)) != NULL)
                     FreeConnectionEntryMemory(ConnectionPtr);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;

            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to establish an HDP connection to an Endpoint of    */
   /* a specific HDP Instance on a Remote Device. The first parameter   */
   /* specifies the Remote Device to connect to. The second parameter   */
   /* specifies the HDP Instance on the remote device. The third        */
   /* parameter specifies the Endpoint of that Instance to which the    */
   /* connection will be attempted. The fourth parameter specifies      */
   /* the type of connection that will be established. The next two     */
   /* parameters specify the Event Callback and Callback parameter (to  */
   /* receive events related to the connection). This function returns a*/
   /* positive value if successful, or a negative return value if there */
   /* was an error.                                                     */
   /* * NOTE * A successful return value represents the Data Link ID    */
   /*          shall be used with various functions and by various      */
   /*          events in this module to reference this data connection. */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int BTPSAPI HDPM_Connect_Remote_Device_Endpoint(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, Byte_t EndpointID, HDP_Channel_Mode_t ChannelMode, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                        ret_val;
   Event_t                    ConnectionEvent;
   unsigned int               Index;
   unsigned int               TotalNumberEndpoints;
   Connection_Entry_t        *ConnectionEntryPtr;
   Data_Channel_Entry_t       DataEntry;
   Data_Channel_Entry_t      *DataEntryPtr;
   HDPM_Endpoint_Info_t      *EndpointInfoList;
   DEVM_Parsed_SDP_Data_t    *ParsedSDPData;
   HDP_Channel_Config_Info_t  ConfigInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X, %u, %u, %u\n", RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0, Instance, EndpointID, ChannelMode));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (VALID_INSTANCE(Instance)) && (VALID_MDEP_ID(EndpointID)) && ((ChannelMode == cmNoPreference) || (ChannelMode == cmReliable) || (ChannelMode == cmStreaming)) && ((EventCallback) || (ConnectionStatus)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, verify that we are not already tracking a          */
            /* connection to the specified device.                      */
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionList, RemoteDeviceAddress, Instance)) != NULL)
            {
               /* A client may manage Data Channel connections only on  */
               /* Instance connections which the client owns.           */
               if(ConnectionEntryPtr->EventCallback.ClientID == MSG_GetServerAddressID())
               {
                  /* Check that the instance connection is in a state   */
                  /* which support endpoint connections.                */
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                  {
                     /* Build the Data Channel Entry for this request.  */
                     BTPS_MemInitialize(&DataEntry, 0, sizeof(Data_Channel_Entry_t));

                     DataEntry.MCLID                           = ConnectionEntryPtr->MCLID;
                     DataEntry.DataLinkID                      = 0;
                     DataEntry.MDEP_ID                         = EndpointID;
                     DataEntry.ChannelMode                     = ChannelMode;
                     DataEntry.ConnectionState                 = dcsAuthorizing;
                     DataEntry.EventCallback.ClientID          = MSG_GetServerAddressID();
                     DataEntry.EventCallback.CallbackFunction  = EventCallback;
                     DataEntry.EventCallback.CallbackParameter = CallbackParameter;

                     /* Resolve whether we are connecting to a Sink     */
                     /* or a Source.  First, just try to find another   */
                     /* connection to that same MDEP and copy that --   */
                     /* the spec requires that each MDEP support one and*/
                     /* only one role.                                  */
                     if((DataEntryPtr = SearchDataChannelEntryMCLID_MDEP(&DataChannelList, DataEntry.MCLID, DataEntry.MDEP_ID)) != NULL)
                     {
                        /* Success - another data channel to this same  */
                        /* MDEP over the same connection was found.     */
                        DataEntry.LocalRole = DataEntryPtr->LocalRole;

                        ret_val             = 0;
                     }
                     else
                     {
                        /* This is the first data channel opened to this*/
                        /* MDEP for this connection. We'll have to parse*/
                        /* SDP records.                                 */

                        /* Load cached SDP data into the buffer.        */
                        if((ret_val = GetParsedSDPData(ConnectionEntryPtr->BD_ADDR, &ParsedSDPData)) == 0)
                        {
                           /* Determine the number of published HDP     */
                           /* Endpoints for this instance.              */
                           if((ret_val = ParseSDPEndpoints(ParsedSDPData, ConnectionEntryPtr->Instance, 0, NULL, &TotalNumberEndpoints)) >= 0)
                           {
                              /* Allocate space for the endpoint list.  */
                              if((EndpointInfoList = (HDPM_Endpoint_Info_t *)BTPS_AllocateMemory(HDPM_ENDPOINT_INFO_DATA_SIZE * TotalNumberEndpoints)) != NULL)
                              {
                                 /* Now get the actual endpoint data.   */
                                 if((ret_val = ParseSDPEndpoints(ParsedSDPData, ConnectionEntryPtr->Instance, TotalNumberEndpoints, EndpointInfoList, NULL)) >= 0)
                                 {
                                    /* Scan the endpoint list for our   */
                                    /* target MDEP.                     */
                                    for(Index=0;Index < (unsigned int)ret_val;Index++)
                                    {
                                       if(EndpointInfoList[Index].EndpointID == DataEntry.MDEP_ID)
                                       {
                                          /* Found an entry for this    */
                                          /* MDEP, so we now know the   */
                                          /* remote role. The local role*/
                                          /* will be the opposite.      */
                                          DataEntry.LocalRole = ((EndpointInfoList[Index].Role == drSink) ? drSource : drSink);
                                          break;
                                       }
                                    }

                                    /* Check whether a role was found.  */
                                    if(Index < (unsigned int)ret_val)
                                    {
                                       /* Found.                        */
                                       ret_val = 0;
                                    }
                                    else
                                    {
                                       /* Not found. Report the error.  */
                                       ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;
                                    }
                                 }

                                 /* Clean up the allocated space.       */
                                 BTPS_FreeMemory(EndpointInfoList);
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                           }

                           /* Release the parsed SDP data structures.   */
                           FreeParsedSDPData(ParsedSDPData);
                        }
                     }


                     if(ret_val >= 0)
                     {
                        if((DataEntryPtr = AddDataChannelEntry(&DataChannelList, &DataEntry)) != NULL)
                        {
                           /* Enable FCS if we are the Source.          */
                           ConfigInfo.FCSMode                  = (HDP_FCS_Mode_t)((DataEntryPtr->LocalRole == drSource)?fcsEnabled:fcsNoPreference);
                           ConfigInfo.MaxTxPacketSize          = 2048;
                           ConfigInfo.TxSegmentSize            = 256;
                           ConfigInfo.NumberOfTxSegmentBuffers = 10;

                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connecting data channel: %d, %d, %d\n", DataEntryPtr->MCLID, DataEntryPtr->LocalRole, DataEntryPtr->ChannelMode));

                           if((ret_val = _HDPM_Connect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->MDEP_ID, DataEntryPtr->LocalRole, DataEntryPtr->ChannelMode, &ConfigInfo)) > 0)
                           {
                              /* Note the Health Device DataLinkID.     */
                              DataEntryPtr->DataLinkID = (unsigned int)ret_val;
                           }
                           else
                           {
                              /* An error occurred, so clean up the Data*/
                              /* Channel Entry that was added to the    */
                              /* list.                                  */
                              DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr);
                              FreeDataChannelEntryMemory(DataEntryPtr);

                              ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT;
                           }

                           /* Next, determine if the caller has         */
                           /* requested a blocking open.                */
                           if(ConnectionStatus)
                           {
                              /* Only wait for the connection if the    */
                              /* connection process was successfully    */
                              /* started.                               */
                              if(!ret_val)
                              {
                                 /* Allocate an event to signal success */
                                 /* or failure.                         */
                                 if((ConnectionEvent = BTPS_CreateEvent(TRUE)) != NULL)
                                 {
                                    DataEntryPtr->ConnectionEvent = ConnectionEvent;

                                    /* Release the Lock to avoid        */
                                    /* deadlocks while waiting for the  */
                                    /* connection.                      */
                                    DEVM_ReleaseLock();

                                    /* Wait for the connection to       */
                                    /* complete.                        */
                                    BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                                    /* Re-acquire the Mutex.            */
                                    if(DEVM_AcquireLock())
                                    {
                                       /* Re-acquire the connection     */
                                       /* entry.                        */
                                       if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, (unsigned int)ret_val)) != NULL)
                                       {
                                          *ConnectionStatus = DataEntryPtr->ConnectionStatus;

                                          /* Remove access to the       */
                                          /* connection event from the  */
                                          /* connection entry object.   */
                                          DataEntryPtr->ConnectionEvent = NULL;

                                          if(DataEntryPtr->ConnectionStatus == HDPM_CONNECTION_STATUS_SUCCESS)
                                          {
                                             /* Connection succeeded.   */
                                             ret_val = 0;
                                          }
                                          else
                                          {
                                             /* Open failed, so note the*/
                                             /* status and remove the   */
                                             /* connection entry from   */
                                             /* the list.               */
                                             ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE;

                                             if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
                                                FreeDataChannelEntryMemory(DataEntryPtr);
                                          }
                                       }
                                       else
                                       {
                                          ret_val           = BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT;
                                          *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                                       }

                                       /* Release the Lock because we   */
                                       /* are finished with it.         */
                                       DEVM_ReleaseLock();
                                    }
                                    else
                                    {
                                       ret_val           = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                                       *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                                    }

                                    /* Free the event object allocated  */
                                    /* previously.                      */
                                    BTPS_CloseEvent(ConnectionEvent);
                                 }
                                 else
                                 {
                                    ret_val           = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
                                    *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;

                                    /* Release the Lock because we are  */
                                    /* finished with it.                */
                                    DEVM_ReleaseLock();
                                 }
                              }
                              else
                              {
                                 /* The connection process was not      */
                                 /* started.                            */
                                 *ConnectionStatus = HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                              }
                           }
                           else
                           {
                              /* Release the Lock because we are        */
                              /* finished with it.                      */
                              DEVM_ReleaseLock();
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }
                  }
                  else
                  {
                     ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_CONNECTION_IN_PROGRESS;

                     /* Release the Lock because we are finished with   */
                     /* it.                                             */
                     DEVM_ReleaseLock();
                  }
               }
               else
               {
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_REMOTE_INSTANCE_IN_USE;

                  /* Release the Lock because we are finished with it.  */
                  DEVM_ReleaseLock();
               }
            }
            else
            {
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;

               /* Release the Lock because we are finished with it.     */
               DEVM_ReleaseLock();
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect an established HDP data connection.   */
   /* This function accepts the Data Link ID of the data connection     */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
int BTPSAPI HDPM_Disconnect_Remote_Device_Endpoint(unsigned int DataLinkID)
{
   int                   ret_val;
   Connection_Entry_t   *ConnectionEntryPtr;
   Data_Channel_Entry_t *DataEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", DataLinkID));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if(DataLinkID)
      {
         if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkID)) != NULL)
         {
            /* Obtain the connection entry for this data link.          */
            if((ConnectionEntryPtr = SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID)) != NULL)
            {
               /* A client may manage Data Channel connections only on  */
               /* Instance connections which the client owns.           */
               if(ConnectionEntryPtr->EventCallback.ClientID == MSG_GetServerAddressID())
               {
                  switch(DataEntryPtr->ConnectionState)
                  {
//FIXME Generic disconnect function for use here, in ProcessDisconnect* and in the shutdown/client-disconnect code
                     case dcsAuthorizing:
                        if(ConnectionEntryPtr->Server)
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Rejecting Incoming Connection: %d\n", DataEntryPtr->DataLinkID));

                           //FIXME Deny authorization; What's the best response code?
                           ret_val = _HDPM_Data_Connection_Request_Response(DataEntryPtr->DataLinkID, MCAP_RESPONSE_CODE_DATA_LINK_BUSY, cmNoPreference, NULL);
                           //XXX Do we need to trigger an event?
                        }
                        else
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnecting Data Channel: %d, %d\n", DataEntryPtr->MCLID, DataEntryPtr->DataLinkID));

                           ret_val = _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
                        }
                        break;
                     case dcsConnecting:
                     case dcsConnected:
                     default:
                        /* Just issue the disconnect request. The lower */
                        /* layer will perform a Disconnect or Abort     */
                        /* as appropriate. Events will be dispatched    */
                        /* to clients when the disconnection or abort   */
                        /* confirmation is received from the framework. */
                        ret_val = _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
                        break;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
            }
            else
            {
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;

               /* The endpoint data channel was found, but not the      */
               /* connection it claims to be associated with. Just clean*/
               /* up the data channel entry.                            */
               if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
               {
                  ret_val = _HDPM_Disconnect_Data_Channel(DataEntryPtr->MCLID, DataEntryPtr->DataLinkID);
                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to send data over an established HDP data connection.  The*/
   /* first parameter is the Data Link ID which represents the data     */
   /* connection to use.  The final parameters specify the data (and    */
   /* amount) to be sent.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * This function will either send all of the data or none of*/
   /*          the data.                                                */
int BTPSAPI HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer)
{
   int                   ret_val;
   Connection_Entry_t   *ConnectionEntryPtr;
   Data_Channel_Entry_t *DataEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u, %u\n", DataLinkID, DataLength));

   /* Next, check to see if we are powered up.                          */
   if(CurrentPowerState)
   {
      /* Device is powered on. Next, verify that the input parameters   */
      /* appear to be semi-valid.                                       */
      if((DataLinkID) && (DataLength) && (DataBuffer))
      {
         if((DataEntryPtr = SearchDataChannelEntry(&DataChannelList, DataLinkID)) != NULL)
         {
            /* Obtain the connection entry for this data link.          */
            if((ConnectionEntryPtr = SearchConnectionEntryMCLID(&ConnectionList, DataEntryPtr->MCLID)) != NULL)
            {
               /* A client may manage Data Channel connections only on  */
               /* Instance connections which the client owns.           */
               if(ConnectionEntryPtr->EventCallback.ClientID == MSG_GetServerAddressID())
               {
                  if(DataEntryPtr->ConnectionState == dcsConnected)
                     ret_val = _HDPM_Write_Data(DataLinkID, DataLength, &(DataBuffer[0]));
                  else
                     ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED;
            }
            else
            {
               ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID;

               /* The endpoint data channel was found, but not the      */
               /* connection it claims to be associated with. Just clean*/
               /* up the data channel entry.                            */
               if((DataEntryPtr = DeleteDataChannelEntryExact(&DataChannelList, DataEntryPtr)) != NULL)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_WARNING), ("Data channel for Link ID %d known, but no associated connection", DataLinkID));

                  FreeDataChannelEntryMemory(DataEntryPtr);
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

   DebugPrint((BTPM_DEBUG_ZONE_HEALTH_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


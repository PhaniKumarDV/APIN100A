/*****< btpmhtpm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHTPM - Health Thermometer Manager for Stonestreet One Bluetooth       */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"            /* BTPS Protocol Stack Prototypes/Constants.  */
#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */

#include "BTPMHTPM.h"           /* BTPM HTP Manager Prototypes/Constants.     */
#include "HTPMAPI.h"            /* HTP Manager Prototypes/Constants.          */
#include "HTPMMSG.h"            /* BTPM HTP Manager Message Formats.          */
#include "HTPMGR.h"             /* HTP Manager Impl. Prototypes/Constants.    */

#include "SS1BTHTS.h"           /* Bluetooth HTS Service Prototypes/Constants.*/

#include "SS1BTPM.h"            /* BTPM Main Prototypes and Constants.        */
#include "BTPMERR.h"            /* BTPM Error Prototypes/Constants.           */
#include "BTPMCFG.h"            /* BTPM Configuration Settings/Constants.     */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHTP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ClientID;
   unsigned long                Flags;
   HTPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHTP_Entry_Info_t *NextHTPEntryInfoPtr;
} HTP_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HTP_Entry_Info_t structure to denote various state information.   */
#define HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   HTPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the type of       */
   /* outstanding transactions that are possible.                       */
typedef enum
{
   ttConfigureCCCD,
   ttGetMeasurementInterval,
   ttSetMeasurementInterval,
   ttGetValidRange,
   ttGetTemperatureType
} Transaction_Type_t;

   /* The following structure is a container structure that contains all*/
   /* of the information on an outstanding transaction.                 */
typedef struct _tagTransaction_Info_t
{
   unsigned int                   TransactionID;
   unsigned int                   GATMTransactionID;
   unsigned int                   HTPManagerDataCallbackID;
   Transaction_Type_t             TransactionType;
   Word_t                         CharacteristicValueHandle;
   struct _tagTransaction_Info_t *NextTransactionInfoPtr;
} Transaction_Info_t;

   /* The following structure is a container structure which contains   */
   /* all of the information on a HTP Device.                           */
typedef struct _tagHTP_Information_t
{
   Word_t        TemperatureCharacteristicHandle;
   Word_t        IntermediateTemperatureCharacteristicHandle;
   Word_t        TemperatureTypeCharacteristicHandle;
   Word_t        MeasurementIntervalCharacteristicHandle;
   Word_t        MeasurementIntervalCCCDHandle;
   Word_t        MeasurementIntervalValidRangeHandle;
   unsigned long SupportedFeatures;
} HTP_Information_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking incoming connections.                 */
typedef enum
{
   csConfiguring,
   csConnected
} Connection_State_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                      BD_ADDR;
   Connection_State_t             ConnectionState;
   HTP_Information_t              HTPInformation;
   unsigned long                  OperationFlags;
   Transaction_Info_t            *TransactionInfoList;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

#define CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_MEASUREMENT_INTERVAL   0x00000001
#define CONNECTION_ENTRY_OPERATION_FLAGS_SETTING_MEASUREMENT_INTERVAL   0x00000002
#define CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_VALID_RANGE            0x00000004
#define CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_TEMPERATURE_TYPE       0x00000008
#define CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD      0x00000010
#define CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD_DONE 0x00000020

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int TransactionID;

   /* Variable which holds a pointer to the first element in the HTP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HTP_Entry_Info_t *HTPEntryInfoList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the GATM Event Callback ID.                  */
static unsigned int GATMEventCallbackID;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);
static unsigned int GetNextTransactionID(void);

static HTP_Entry_Info_t *AddHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, HTP_Entry_Info_t *EntryToAdd);
static HTP_Entry_Info_t *SearchHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID);
static HTP_Entry_Info_t *DeleteHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHTPEntryInfoEntryMemory(HTP_Entry_Info_t *EntryToFree);
static void FreeHTPEntryInfoList(HTP_Entry_Info_t **ListHead);

static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd);
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int TransactionID);
static Transaction_Info_t *DeleteTransactionInfoByGATMTransactionID(Transaction_Info_t **ListHead, unsigned int GATMTransactionID);
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree);
static void FreeTransactionInfoList(Transaction_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static void DispatchHTPEvent(HTPM_Event_Data_t *HTPMEventData, BTPM_Message_t *Message);

static void DispatchHTPDeviceConnection(Connection_Entry_t *ConnectionEntry);
static void DispatchHTPGetTemperatureTypeResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode, HTPM_Temperature_Type_t TemperatureType);
static void DispatchHTPGetMeasurementIntervalResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode, unsigned int MeasurementInterval);
static void DispatchHTPSetMeasurementIntervalResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode);
static void DispatchHTPGetMeasurementIntervalValidRangeResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode, unsigned int LowerBounds, unsigned int UpperBounds);
static void DispatchHTPTemperatureMeasurementEvent(Connection_Entry_t *ConnectionEntry, HTPM_Temperature_Measurement_Event_Data_t *TemperatureMeasurement);

static Boolean_t CheckHTPServicesSupported(BD_ADDR_t BD_ADDR);
static Boolean_t QueryParsedServicesData(BD_ADDR_t BD_ADDR, DEVM_Parsed_Services_Data_t *ParsedGATTData);

static Boolean_t ConvertHTSTempTypeToHTPM(Byte_t HTSTemperatureType, HTPM_Temperature_Type_t *TemperatureType);

static int HTPSetMeasurementIntervalRequest(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int MeasurementInterval);
static int HTPGetCharacteristicRequest(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, Transaction_Type_t TransactionType);

static Transaction_Info_t *ConfigureClientConfiguration(Connection_Entry_t *ConnectionEntry, Word_t CharacteristicHandle, Word_t CCCDHandle, Boolean_t Indicate);

static Boolean_t ProcessHTPTemperatureMeasurementCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation, HTPM_Temperature_Measurement_Type_t TemperatureMeasurementType);
static Boolean_t ProcessHTPTemperatureTypeCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessHTPMeasurementIntervalCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessConfigureHTPService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *HTPService);
static void ProcessConfigureHTPConnection(BD_ADDR_t BD_ADDR);

static void ProcessHTPDeviceConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessHTPDeviceDisconnection(BD_ADDR_t RemoteDeviceAddress);

static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRegisterHTPCollectorEventsMessage(HTPM_Register_Collector_Events_Request_t *Message);
static void ProcessUnRegisterHTPCollectorEventsMessage(HTPM_Un_Register_Collector_Events_Request_t *Message);
static void ProcessHTPGetTemperatureTypeRequestMessage(HTPM_Get_Temperature_Type_Request_t *Message);
static void ProcessHTPGetMeasurementIntervalRequestMessage(HTPM_Get_Measurement_Interval_Request_t *Message);
static void ProcessHTPSetMeasurementIntervalRequestMessage(HTPM_Set_Measurement_Interval_Request_t *Message);
static void ProcessHTPGetMeasurementIntervalValidRangeRequestMessage(HTPM_Get_Measurement_Interval_Valid_Range_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessGATTTransactionResponse(Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, Boolean_t ErrorResponse, void *EventData);

static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessGATTReadResponse(GATM_Read_Response_Event_Data_t *ReadResponseData);
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse);
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse);

static void BTPSAPI BTPMDispatchCallback_HTPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter);
static void BTPSAPI HTPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HTP Entry Information List.                              */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Transaction ID that can be used to add an entry */
   /* into the Transaction Information List.                            */
static unsigned int GetNextTransactionID(void)
{
   unsigned int ret_val;

   ret_val = TransactionID++;

   if((!TransactionID) || (TransactionID & 0x80000000))
      TransactionID = 0x00000001;

   return(ret_val);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            CallbackID field is the same as an entry already in    */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static HTP_Entry_Info_t *AddHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, HTP_Entry_Info_t *EntryToAdd)
{
   HTP_Entry_Info_t *AddedEntry = NULL;
   HTP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HTP_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HTP_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHTPEntryInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->CallbackID == AddedEntry->CallbackID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeHTPEntryInfoEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextHTPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHTPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHTPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HTP_Entry_Info_t *SearchHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HTP_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHTPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HTP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HTP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHTPEntryInfoEntryMemory().                   */
static HTP_Entry_Info_t *DeleteHTPEntryInfoEntry(HTP_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HTP_Entry_Info_t *FoundEntry = NULL;
   HTP_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHTPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHTPEntryInfoPtr = FoundEntry->NextHTPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHTPEntryInfoPtr;

         FoundEntry->NextHTPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HTP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHTPEntryInfoEntryMemory(HTP_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HTP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHTPEntryInfoList(HTP_Entry_Info_t **ListHead)
{
   HTP_Entry_Info_t *EntryToFree;
   HTP_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHTPEntryInfoPtr;

         FreeHTPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the TransactionID field is the same as an entry already*/
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd)
{
   Transaction_Info_t *AddedEntry = NULL;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->TransactionID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Transaction_Info_t *)BTPS_AllocateMemory(sizeof(Transaction_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                        = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextTransactionInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->TransactionID == AddedEntry->TransactionID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeTransactionInfoMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextTransactionInfoPtr)
                        tmpEntry = tmpEntry->NextTransactionInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextTransactionInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified TransactionID and*/
   /* removes it from the List.  This function returns NULL if either   */
   /* the Transaction Info List Head is invalid, the Transaction ID is  */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionInfoMemory().                     */
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int TransactionID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Transaction ID to search for appear  */
   /* to be valid.                                                      */
   if((ListHead) && (TransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TransactionID != TransactionID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextTransactionInfoPtr = FoundEntry->NextTransactionInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextTransactionInfoPtr;

         FoundEntry->NextTransactionInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified GATM             */
   /* TransactionID and removes it from the List.  This function returns*/
   /* NULL if either the Transaction Info List Head is invalid, the GATM*/
   /* Transaction ID is invalid, or the specified Entry was NOT present */
   /* in the list.  The entry returned will have the Next Entry field   */
   /* set to NULL, and the caller is responsible for deleting the memory*/
   /* associated with this entry by calling FreeTransactionInfoMemory().*/
static Transaction_Info_t *DeleteTransactionInfoByGATMTransactionID(Transaction_Info_t **ListHead, unsigned int GATMTransactionID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and GATM Transaction ID to search for    */
   /* appear to be valid.                                               */
   if((ListHead) && (GATMTransactionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GATMTransactionID != GATMTransactionID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextTransactionInfoPtr = FoundEntry->NextTransactionInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextTransactionInfoPtr;

         FoundEntry->NextTransactionInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionInfoList(Transaction_Info_t **ListHead)
{
   Transaction_Info_t *EntryToFree;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextTransactionInfoPtr;

         FreeTransactionInfoMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the BD_ADDR field is the same as an entry already in   */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *AddedEntry = NULL;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Format a NULL BD_ADDR to test against.                         */
      ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

      /* Make sure that the element that we are adding seems semi-valid.*/
      if(!COMPARE_BD_ADDR(NULL_BD_ADDR, EntryToAdd->BD_ADDR))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Connection_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                        = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextConnectionEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeConnectionEntryMemory(AddedEntry);
                     AddedEntry = NULL;

                     /* Abort the Search.                               */
                     tmpEntry   = NULL;
                  }
                  else
                  {
                     /* OK, we need to see if we are at the last element*/
                     /* of the List.  If we are, we simply break out of */
                     /* the list traversal because we know there are NO */
                     /* duplicates AND we are at the end of the list.   */
                     if(tmpEntry->NextConnectionEntryPtr)
                        tmpEntry = tmpEntry->NextConnectionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextConnectionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR))))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the Connection Entry with the specified Bluetooth Device */
   /* Address and removes it from the List.  This function returns NULL */
   /* if either the Connection Entry List Head is invalid, the Bluetooth*/
   /* Device Address is invalid, or the specified Entry was NOT present */
   /* in the list.  The entry returned will have the Next Entry field   */
   /* set to NULL, and the caller is responsible for deleting the memory*/
   /* associated with this entry by calling FreeConnectionEntryMemory().*/
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t           NULL_BD_ADDR;
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR))))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextConnectionEntryPtr = FoundEntry->NextConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextConnectionEntryPtr;

         FoundEntry->NextConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
   {
      /* Make sure the transaction list is empty for this connection.   */
      if(EntryToFree->TransactionInfoList)
         FreeTransactionInfoList(&(EntryToFree->TransactionInfoList));

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextConnectionEntryPtr;

         FreeConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HTP event to every registered HTP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HTP Manager Lock */
   /*          held.  Upon exit from this function it will free the HTP */
   /*          Manager Lock.                                            */
static void DispatchHTPEvent(HTPM_Event_Data_t *HTPMEventData, BTPM_Message_t *Message)
{
   unsigned int      Index;
   unsigned int      Index1;
   unsigned int      ServerID;
   unsigned int      NumberCallbacks;
   Callback_Info_t   CallbackInfoArray[16];
   Callback_Info_t  *CallbackInfoArrayPtr;
   HTP_Entry_Info_t *HTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HTPEntryInfoList) && (HTPMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      HTPEntryInfo    = HTPEntryInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(HTPEntryInfo)
      {
         if(((HTPEntryInfo->EventCallback) || (HTPEntryInfo->ClientID != ServerID)) && (HTPEntryInfo->Flags & HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;
      }

      if(NumberCallbacks)
      {
         if(NumberCallbacks <= (sizeof(CallbackInfoArray)/sizeof(Callback_Info_t)))
            CallbackInfoArrayPtr = CallbackInfoArray;
         else
            CallbackInfoArrayPtr = BTPS_AllocateMemory((NumberCallbacks*sizeof(Callback_Info_t)));

         /* Make sure that we have memory to copy the Callback List     */
         /* into.                                                       */
         if(CallbackInfoArrayPtr)
         {
            HTPEntryInfo    = HTPEntryInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(HTPEntryInfo)
            {
               if(((HTPEntryInfo->EventCallback) || (HTPEntryInfo->ClientID != ServerID)) && (HTPEntryInfo->Flags & HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HTPEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HTPEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HTPEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;
            }

            /* Release the Lock because we have already built the       */
            /* Callback Array.                                          */
            DEVM_ReleaseLock();

            /* Now we are ready to dispatch the callbacks.              */
            Index = 0;

            while((Index < NumberCallbacks) && (Initialized))
            {
               /* * NOTE * It is possible that we have already          */
               /*          dispatched the event to the client (case     */
               /*          would occur if a single client has registered*/
               /*          for HTP events and Data Events.              */
               /*          To avoid this case we need to walk the       */
               /*          list of previously dispatched events to check*/
               /*          to see if it has already been dispatched     */
               /*          (we need to do this with Client Address ID's */
               /*          for messages - Event Callbacks are local     */
               /*          and therefore unique so we don't have to do  */
               /*          this filtering.                              */

               /* Determine the type of event that needs to be          */
               /* dispatched.                                           */
               if(CallbackInfoArrayPtr[Index].ClientID == ServerID)
               {
                  /* Go ahead and make the callback.                    */
                  /* * NOTE * If the callback was deleted (or new ones  */
                  /*          were added, they will not be caught for   */
                  /*          this message dispatch).  Under normal     */
                  /*          operating circumstances this case         */
                  /*          shouldn't matter because these groups     */
                  /*          aren't really dynamic and are only        */
                  /*          registered at initialization time.        */
                  __BTPSTRY
                  {
                     if(CallbackInfoArrayPtr[Index].EventCallback)
                     {
                        (*CallbackInfoArrayPtr[Index].EventCallback)(HTPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Walk the proceding list and see if we have already */
                  /* dispatched this event to this client.              */
                  for(Index1=0;Index1<Index;Index1++)
                  {
                     if((CallbackInfoArrayPtr[Index1].ClientID != ServerID) && (CallbackInfoArrayPtr[Index1].ClientID == CallbackInfoArrayPtr[Index].ClientID))
                        break;
                  }

                  if(Index1 == Index)
                  {
                     /* Dispatch the Message.                           */
                     Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                     MSG_SendMessage(Message);
                  }
               }

               Index++;
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HTP Device Connection Event.                           */
static void DispatchHTPDeviceConnection(Connection_Entry_t *ConnectionEntry)
{
   HTPM_Event_Data_t        HTPMEventData;
   HTPM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Make sure that we flag that this device is connected.          */
      ConnectionEntry->ConnectionState = csConnected;

      /* Next, format up the Event to dispatch.                         */
      HTPMEventData.EventType                                           = hetHTPConnected;
      HTPMEventData.EventLength                                         = HTPM_CONNECTED_EVENT_DATA_SIZE;

      HTPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress    = ConnectionEntry->BD_ADDR;
      HTPMEventData.EventData.ConnectedEventData.ConnectionType         = httCollector;
      HTPMEventData.EventData.ConnectedEventData.ConnectedFlags         = ConnectionEntry->HTPInformation.SupportedFeatures;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, HTPM_CONNECTED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (HTPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.ConnectionType                = httCollector;
      Message.ConnectedFlags                = ConnectionEntry->HTPInformation.SupportedFeatures;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchHTPEvent(&HTPMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HTP Get Temperature Type Response event.               */
static void DispatchHTPGetTemperatureTypeResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode, HTPM_Temperature_Type_t TemperatureType)
{
   void                                         *CallbackParameter;
   HTPM_Event_Data_t                             HTPMEventData;
   HTPM_Event_Callback_t                         EventCallback;
   HTPM_Get_Temperature_Type_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo))
   {
      /* Check to see if this should be dispatched locally or to a      */
      /* client process.                                                */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         HTPMEventData.EventType                                                         = hetHTPGetTemperatureTypeResponse;
         HTPMEventData.EventLength                                                       = HTPM_GET_TEMPERATURE_TYPE_RESPONSE_EVENT_DATA_SIZE;

         HTPMEventData.EventData.GetTemperatureTypeResponseEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HTPMEventData.EventData.GetTemperatureTypeResponseEventData.TransactionID       = TransactionID;
         HTPMEventData.EventData.GetTemperatureTypeResponseEventData.TemperatureType     = TemperatureType;

         if(!ErrorCode)
         {
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.Success            = TRUE;
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.AttributeErrorCode = 0;
         }
         else
         {
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.Success            = FALSE;
            HTPMEventData.EventData.GetTemperatureTypeResponseEventData.AttributeErrorCode = ErrorCode;
         }

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&HTPMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         /* Re-acquire the Lock.                                        */
         DEVM_AcquireLock();
      }
      else
      {
         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, HTPM_GET_TEMPERATURE_TYPE_RESPONSE_MESSAGE_SIZE);

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
         Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE_RESPONSE;
         Message.MessageHeader.MessageLength   = (HTPM_GET_TEMPERATURE_TYPE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.TransactionID                 = TransactionID;
         Message.TemperatureType               = TemperatureType;

         if(!ErrorCode)
         {
            Message.Success            = TRUE;
            Message.AttributeErrorCode = 0;
         }
         else
         {
            Message.Success            = FALSE;
            Message.AttributeErrorCode = ErrorCode;
         }

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HTP Get Measurement Interval Response event.           */
static void DispatchHTPGetMeasurementIntervalResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode, unsigned int MeasurementInterval)
{
   void                                             *CallbackParameter;
   HTPM_Event_Data_t                                 HTPMEventData;
   HTPM_Event_Callback_t                             EventCallback;
   HTPM_Get_Measurement_Interval_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && ((!TransactionID) || (EventCallbackInfo)))
   {
      /* Format the event to dispatch.                                  */
      HTPMEventData.EventType                                                             = hetHTPGetMeasurementIntervalResponse;
      HTPMEventData.EventLength                                                           = HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_EVENT_DATA_SIZE;

      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
      HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.TransactionID       = TransactionID;

      if(!ErrorCode)
      {
         HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.Success             = TRUE;
         HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.AttributeErrorCode  = 0;
         HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.MeasurementInterval = MeasurementInterval;
      }
      else
      {
         HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.Success             = FALSE;
         HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.AttributeErrorCode  = ErrorCode;
         HTPMEventData.EventData.GetMeasurementIntervalResponseEventData.MeasurementInterval = 0;
      }

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = (EventCallbackInfo?EventCallbackInfo->ClientID:0);
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_RESPONSE;
      Message.MessageHeader.MessageLength   = (HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.TransactionID                 = TransactionID;

      if(!ErrorCode)
      {
         Message.Success             = TRUE;
         Message.AttributeErrorCode  = 0;
         Message.MeasurementInterval = MeasurementInterval;
      }
      else
      {
         Message.Success             = FALSE;
         Message.AttributeErrorCode  = ErrorCode;
         Message.MeasurementInterval = 0;
      }

      /* Check to see who should get the event.                         */
      if(TransactionID)
      {
         /* Check to see if this should be dispatched locally or to a   */
         /* client process.                                             */
         if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Save the Event Callback Information.                     */
            EventCallback     = EventCallbackInfo->EventCallback;
            CallbackParameter = EventCallbackInfo->CallbackParameter;

            /* Release the Lock so we can make the callback.            */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&HTPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
         else
         {
            /* Go ahead and send the message to the client.             */
            MSG_SendMessage((BTPM_Message_t *)&Message);
         }
      }
      else
      {
         /* Just dispatch globally.                                     */
         DispatchHTPEvent(&HTPMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HTP Set Measurement Interval Response event.           */
static void DispatchHTPSetMeasurementIntervalResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode)
{
   void                                             *CallbackParameter;
   HTPM_Event_Data_t                                 HTPMEventData;
   HTPM_Event_Callback_t                             EventCallback;
   HTPM_Set_Measurement_Interval_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo))
   {
      /* Check to see if this should be dispatched locally or to a      */
      /* client process.                                                */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         HTPMEventData.EventType                                                             = hetHTPSetMeasurementIntervalResponse;
         HTPMEventData.EventLength                                                           = HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_EVENT_DATA_SIZE;

         HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.TransactionID       = TransactionID;

         if(!ErrorCode)
         {
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.Success            = TRUE;
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.AttributeErrorCode = 0;
         }
         else
         {
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.Success            = FALSE;
            HTPMEventData.EventData.SetMeasurementIntervalResponseEventData.AttributeErrorCode = ErrorCode;
         }

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&HTPMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         /* Re-acquire the Lock.                                        */
         DEVM_AcquireLock();
      }
      else
      {
         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE);

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
         Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL_RESPONSE;
         Message.MessageHeader.MessageLength   = (HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.TransactionID                 = TransactionID;

         if(!ErrorCode)
         {
            Message.Success            = TRUE;
            Message.AttributeErrorCode = 0;
         }
         else
         {
            Message.Success            = FALSE;
            Message.AttributeErrorCode = ErrorCode;
         }

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HTP Get Measurement Interval Valid Range Response      */
   /* event.                                                            */
static void DispatchHTPGetMeasurementIntervalValidRangeResponse(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int TransactionID, Byte_t ErrorCode, unsigned int LowerBounds, unsigned int UpperBounds)
{
   void                                                         *CallbackParameter;
   HTPM_Event_Data_t                                             HTPMEventData;
   HTPM_Event_Callback_t                                         EventCallback;
   HTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo))
   {
      /* Check to see if this should be dispatched locally or to a      */
      /* client process.                                                */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         HTPMEventData.EventType                                                                       = hetHTPGetMeasurementIntervalValidRangeResponse;
         HTPMEventData.EventLength                                                                     = HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_EVENT_DATA_SIZE;

         HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.TransactionID       = TransactionID;

         if(!ErrorCode)
         {
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.Success             = TRUE;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.AttributeErrorCode  = 0;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.LowerBounds         = LowerBounds;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.UpperBounds         = UpperBounds;
         }
         else
         {
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.Success             = FALSE;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.AttributeErrorCode  = ErrorCode;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.LowerBounds         = 0;
            HTPMEventData.EventData.GetMeasurementIntervalValidRangeResponseEventData.UpperBounds         = 0;
         }

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&HTPMEventData, CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         /* Re-acquire the Lock.                                        */
         DEVM_AcquireLock();
      }
      else
      {
         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_MESSAGE_SIZE);

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
         Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE;
         Message.MessageHeader.MessageLength   = (HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.TransactionID                 = TransactionID;

         if(!ErrorCode)
         {
            Message.Success             = TRUE;
            Message.AttributeErrorCode  = 0;
            Message.LowerBounds         = LowerBounds;
            Message.UpperBounds         = UpperBounds;
         }
         else
         {
            Message.Success             = FALSE;
            Message.AttributeErrorCode  = ErrorCode;
            Message.LowerBounds         = 0;
            Message.UpperBounds         = 0;
         }

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HTP Temperature Measurement event.                     */
static void DispatchHTPTemperatureMeasurementEvent(Connection_Entry_t *ConnectionEntry, HTPM_Temperature_Measurement_Event_Data_t *TemperatureMeasurement)
{
   HTPM_Event_Data_t                      HTPMEventData;
   HTPM_Temperature_Measurement_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (TemperatureMeasurement))
   {
      /* Format the event to dispatch.                                  */
      HTPMEventData.EventType                                 = hetHTPTemperatureMeasurement;
      HTPMEventData.EventLength                               = HTPM_TEMPERATURE_MEASUREMENT_EVENT_DATA_SIZE;

      HTPMEventData.EventData.TemperatureMeasurementEventData = *TemperatureMeasurement;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, HTPM_TEMPERATURE_MEASUREMENT_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
      Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_TEMPERATURE_MEASUREMENT;
      Message.MessageHeader.MessageLength   = (HTPM_TEMPERATURE_MEASUREMENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = TemperatureMeasurement->RemoteDeviceAddress;
      Message.MeasurementType               = TemperatureMeasurement->MeasurementType;
      Message.MeasurementFlags              = TemperatureMeasurement->MeasurementFlags;
      Message.TemperatureType               = TemperatureMeasurement->TemperatureType;
      Message.TimeStamp                     = TemperatureMeasurement->TimeStamp;
      Message.TemperatureMantissa           = TemperatureMeasurement->TemperatureMantissa;
      Message.TemperatureExponent           = TemperatureMeasurement->TemperatureExponent;

      /* Just dispatch globally.                                        */
      DispatchHTPEvent(&HTPMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* determine if a device supports the necessary services to support  */
   /* the HTP Sensor role.                                              */
static Boolean_t CheckHTPServicesSupported(BD_ADDR_t BD_ADDR)
{
   UUID_16_t        UUID;
   Boolean_t        ret_val = FALSE;
   SDP_UUID_Entry_t ServiceUUID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the HTP Service is supported.                     */
   ServiceUUID.SDP_Data_Element_Type = deUUID_16;
   HTS_ASSIGN_HTS_SERVICE_UUID_16(&UUID);
   CONVERT_BLUETOOTH_UUID_16_TO_SDP_UUID_16(ServiceUUID.UUID_Value.UUID_16, UUID);

   if(DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR, ServiceUUID) > 0)
   {
      /* Flag that this device supports the HTP Device Role.            */
      ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to query */
   /* the parsed GATT Services data for the specified device.           */
   /* * NOTE * The caller must call DEVM_FreeParsedServicesData when    */
   /*          finished with the parsed GATT Data.                      */
static Boolean_t QueryParsedServicesData(BD_ADDR_t BD_ADDR, DEVM_Parsed_Services_Data_t *ParsedGATTData)
{
   int           Result;
   Byte_t       *ServiceData;
   Boolean_t     ret_val = FALSE;
   unsigned int  TotalServiceSize;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ParsedGATTData))
   {
      /* Query the Remote device services to determine how much memory  */
      /* to allocate.                                                   */
      if(!DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 0, NULL, &TotalServiceSize))
      {
         /* Allocate a buffer to hold the service data.                 */
         if((ServiceData = BTPS_AllocateMemory(TotalServiceSize)) != NULL)
         {
            /* Query the actual service data.                           */
            if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, TotalServiceSize, ServiceData, NULL)) >= 0)
            {
               /* Save the number of bytes quered.                      */
               TotalServiceSize = (unsigned int)Result;

               /* Convert the Raw Service data to Parsed Service Data.  */
               if(!DEVM_ConvertRawServicesStreamToParsedServicesData(TotalServiceSize, ServiceData, ParsedGATTData))
               {
                  /* Return success to the caller.                      */
                  ret_val = TRUE;
               }
            }

            /* Free the memory that was allocated previously.           */
            BTPS_FreeMemory(ServiceData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* convert a HTS Temperature Type to a HTPM Type.                    */
static Boolean_t ConvertHTSTempTypeToHTPM(Byte_t HTSTemperatureType, HTPM_Temperature_Type_t *TemperatureType)
{
   Boolean_t ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(TemperatureType)
   {
      /* Do the conversion.                                             */
      switch(HTSTemperatureType)
      {
         case HTS_TEMPERATURE_TYPE_ARMPIT:
            *TemperatureType = httArmpit;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_BODY:
            *TemperatureType = httBody;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_EAR:
            *TemperatureType = httEar;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_FINGER:
            *TemperatureType = httFinger;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_GASTRO_INTESTINAL_TRACT:
            *TemperatureType = httGastroIntestinalTract;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_MOUTH:
            *TemperatureType = httMouth;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_RECTUM:
            *TemperatureType = httRectum;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_TOE:
            *TemperatureType = httToe;
            ret_val          = TRUE;
            break;
         case HTS_TEMPERATURE_TYPE_TYMPANUM:
            *TemperatureType = httTympanum;
            ret_val          = TRUE;
            break;
         default:
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* perform a Set Measurement Interval Request for the specified HTP  */
   /* Connection.                                                       */
static int HTPSetMeasurementIntervalRequest(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, unsigned int MeasurementInterval)
{
   int                 ret_val;
   NonAlignedWord_t    FormattedMeasurementInterval;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo))
   {
      /* Verify that the measurement interval write feature is          */
      /* supported.                                                     */
      if((ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle) && (ConnectionEntry->HTPInformation.SupportedFeatures & HTPM_CONNECTED_FLAGS_SET_MEASUREMENT_INTERVAL_SUPPORTED))
      {
         /* Verify that there is not another operation in progress.     */
         if(!(ConnectionEntry->OperationFlags & CONNECTION_ENTRY_OPERATION_FLAGS_SETTING_MEASUREMENT_INTERVAL))
         {
            /* Configure the Transaction Information for this           */
            /* transaction.                                             */
            BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

            TransactionInfo.TransactionID             = GetNextTransactionID();
            TransactionInfo.HTPManagerDataCallbackID  = EventCallbackInfo->CallbackID;
            TransactionInfo.CharacteristicValueHandle = ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle;
            TransactionInfo.TransactionType           = ttSetMeasurementInterval;

            /* Add the Transaction Info to the transaction list.        */
            if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
            {
               /* Format the value to write.                            */
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&FormattedMeasurementInterval, MeasurementInterval);

               /* Attempt to write the value to the device.             */
               ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, TransactionInfoPtr->CharacteristicValueHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&FormattedMeasurementInterval);
               if(ret_val > 0)
               {
                  /* Save the GATM Transaction ID.                      */
                  TransactionInfoPtr->GATMTransactionID  = (unsigned int)ret_val;

                  /* Flag that the operation is in progress.            */
                  ConnectionEntry->OperationFlags       |= CONNECTION_ENTRY_OPERATION_FLAGS_SETTING_MEASUREMENT_INTERVAL;

                  /* Return the local Transaction ID.                   */
                  ret_val                                = (int)TransactionInfoPtr->TransactionID;

                  /* Check to see if it is necessary to configure the   */
                  /* CCCD.                                              */
                  if(!(ConnectionEntry->OperationFlags & (CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD | CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD_DONE)))
                  {
                     /* Next check to see if a CCCD is present.         */
                     if(ConnectionEntry->HTPInformation.MeasurementIntervalCCCDHandle)
                     {
                        /* Attempt to configure the CCCD for            */
                        /* indications.                                 */
                        if(ConfigureClientConfiguration(ConnectionEntry, ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle, ConnectionEntry->HTPInformation.MeasurementIntervalCCCDHandle, TRUE))
                        {
                           /* Flag that the CCCD configuration is in    */
                           /* progress for this connection.             */
                           ConnectionEntry->OperationFlags |= CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD;
                        }
                     }
                  }
               }
               else
               {
                  /* Delete the transaction information memory and free */
                  /* the memory.                                        */
                  if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
                  {
                     BTPS_FreeMemory(TransactionInfoPtr);

                     TransactionInfoPtr = NULL;
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_OPERATION_IN_PROGRESS;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_FEATURE_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* perform a Get Request for the specified HTP Characteristic.       */
static int HTPGetCharacteristicRequest(Connection_Entry_t *ConnectionEntry, HTP_Entry_Info_t *EventCallbackInfo, Transaction_Type_t TransactionType)
{
   int                 ret_val;
   Word_t              ValueHandle;
   unsigned long       FlagMask;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo))
   {
      /* Determine the correct handle.                                  */
      switch(TransactionType)
      {
         case ttGetMeasurementInterval:
            ValueHandle = ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle;
            FlagMask    = CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_MEASUREMENT_INTERVAL;
            break;
         case ttGetTemperatureType:
            ValueHandle = ConnectionEntry->HTPInformation.TemperatureTypeCharacteristicHandle;
            FlagMask    = CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_TEMPERATURE_TYPE;
            break;
         case ttGetValidRange:
            ValueHandle = ConnectionEntry->HTPInformation.MeasurementIntervalValidRangeHandle;
            FlagMask    = CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_VALID_RANGE;
            break;
         default:
            ValueHandle = 0;
            FlagMask    = 0;
            break;
      }

      /* Verify that we have a Attribute Handle to read.                */
      if(ValueHandle)
      {
         /* Make sure that the operation is not already in progress.    */
         if(!(ConnectionEntry->OperationFlags & FlagMask))
         {
            /* Configure the Transaction Information for this           */
            /* transaction.                                             */
            BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

            TransactionInfo.TransactionID             = GetNextTransactionID();
            TransactionInfo.HTPManagerDataCallbackID  = EventCallbackInfo->CallbackID;
            TransactionInfo.CharacteristicValueHandle = ValueHandle;
            TransactionInfo.TransactionType           = TransactionType;

            /* Add the Transaction Info to the transaction list.        */
            if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
            {
               /* Write the Protocol Mode characteristic.               */
               ret_val = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ValueHandle, 0, TRUE);
               if(ret_val > 0)
               {
                  /* Save the GATM Transaction ID.                      */
                  TransactionInfoPtr->GATMTransactionID  = (unsigned int)ret_val;

                  /* Flag that the operation is in progress.            */
                  ConnectionEntry->OperationFlags       |= FlagMask;

                  /* Return the local Transaction ID.                   */
                  ret_val                                = (int)TransactionInfoPtr->TransactionID;

                  /* If this is a Read of either the Measurement        */
                  /* Interval or the Measurement Interval Valid Range   */
                  /* then we will go ahead and configure the CCCD (if   */
                  /* present).                                          */
                  if((TransactionType == ttGetMeasurementInterval) || (TransactionType == ttGetValidRange))
                  {
                     /* Check to see if it is necessary to configure the*/
                     /* CCCD.                                           */
                     if(!(ConnectionEntry->OperationFlags & (CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD | CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD_DONE)))
                     {
                        /* Next check to see if a CCCD is present.      */
                        if(ConnectionEntry->HTPInformation.MeasurementIntervalCCCDHandle)
                        {
                           /* Attempt to configure the CCCD for         */
                           /* indications.                              */
                           if(ConfigureClientConfiguration(ConnectionEntry, ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle, ConnectionEntry->HTPInformation.MeasurementIntervalCCCDHandle, TRUE))
                           {
                              /* Flag that the CCCD configuration is in */
                              /* progress for this connection.          */
                              ConnectionEntry->OperationFlags |= CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD;
                           }
                        }
                     }
                  }
               }
               else
               {
                  /* Delete the transaction information memory and free */
                  /* the memory.                                        */
                  if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
                  {
                     BTPS_FreeMemory(TransactionInfoPtr);

                     TransactionInfoPtr = NULL;
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_OPERATION_IN_PROGRESS;
      }
      else
         ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_FEATURE_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* configure a Client Characteristic Configuration Descriptor for the*/
   /* specified operation.                                              */
static Transaction_Info_t *ConfigureClientConfiguration(Connection_Entry_t *ConnectionEntry, Word_t CharacteristicHandle, Word_t CCCDHandle, Boolean_t Indicate)
{
   int                 Result;
   NonAlignedWord_t    CCCDValue;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicHandle) && (CCCDHandle))
   {
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

      TransactionInfo.TransactionID             = GetNextTransactionID();
      TransactionInfo.CharacteristicValueHandle = CharacteristicHandle;
      TransactionInfo.TransactionType           = ttConfigureCCCD;

      /* Add the Transaction Info to the transaction list.              */
      if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
      {
         /* Configure the CCCD for notifications.                       */
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, (Indicate?GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE:GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE));

         /* Perform the write to configure the CCCD.                    */
         Result = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CCCDHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue);
         if(Result > 0)
         {
            /* Save the GATM Transaction ID.                            */
            TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;
         }
         else
         {
            /* Delete the transaction information memory and free the   */
            /* memory.                                                  */
            if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
            {
               BTPS_FreeMemory(TransactionInfoPtr);

               TransactionInfoPtr = NULL;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));

   return(TransactionInfoPtr);
}

   /* The following function is a utility function that is used to      */
   /* process a HTP Temperature Measurement Characteristic.             */
static Boolean_t ProcessHTPTemperatureMeasurementCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation, HTPM_Temperature_Measurement_Type_t TemperatureMeasurementType)
{
   Word_t       CCCD;
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Determine the CCCD and RRD Handles for the Report (if any).    */
      for(Index=0,CCCD=0;Index<CharacteristicInformation->NumberOfDescriptors;Index++)
      {
         if(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
               CCCD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
         }
      }

      /* Determine what type of report this is.                         */
      switch(TemperatureMeasurementType)
      {
         case tmtIntermediateMeasurement:
            /* Verify that a Client Characteristic Configuration        */
            /* Descriptor is present and that the notify property is set*/
            /* is present.                                              */
            if((CCCD) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
            {
               /* Attempt to configure the CCCD for notifications.      */
               if(ConfigureClientConfiguration(ConnectionEntry, CharacteristicInformation->Characteristic_Handle, CCCD, FALSE) != NULL)
               {
                  /* Flag tha the Intermediate Temperature is supported.*/
                  ConnectionEntry->HTPInformation.SupportedFeatures                            |= HTPM_CONNECTED_FLAGS_INTERMEDIATE_TEMPERATURE_SUPPORTED;

                  /* Save the Attribute Handle.                         */
                  ConnectionEntry->HTPInformation.IntermediateTemperatureCharacteristicHandle   = CharacteristicInformation->Characteristic_Handle;

                  /* Return success to the caller.                      */
                  ret_val                                                                       = TRUE;
               }
            }
            break;
         case tmtTemperatureMeasurement:
            /* Verify that a Client Characteristic Configuration        */
            /* Descriptor is present and that the indicate property is  */
            /* set is present.                                          */
            if((CCCD) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
            {
               /* Attempt to configure the CCCD for indications.        */
               if(ConfigureClientConfiguration(ConnectionEntry, CharacteristicInformation->Characteristic_Handle, CCCD, TRUE) != NULL)
               {
                  /* Save the Attribute Handle.                         */
                  ConnectionEntry->HTPInformation.TemperatureCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;

                  /* Return success to the caller.                      */
                  ret_val                                                         = TRUE;
               }
            }
            break;
         default:
            /* Default case do nothing.                                 */
            break;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a HTP Temperature Type Characteristic.                    */
static Boolean_t ProcessHTPTemperatureTypeCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Boolean_t ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
   {
      /* Simply cache the required information.                         */
      ConnectionEntry->HTPInformation.TemperatureTypeCharacteristicHandle  = CharacteristicInformation->Characteristic_Handle;

      ConnectionEntry->HTPInformation.SupportedFeatures                   |= HTPM_CONNECTED_FLAGS_GET_TEMPERATURE_TYPE_SUPPORTED;

      ret_val                                                              = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a HTP Measurement Interval Characteristic.                */
static Boolean_t ProcessHTPMeasurementIntervalCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Word_t       CCCD;
   Word_t       VRD;
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Determine the CCCD and RRD Handles for the Report (if any).    */
      for(Index=0,CCCD=0,VRD=0;Index<CharacteristicInformation->NumberOfDescriptors;Index++)
      {
         if(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
               CCCD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
            else
            {
               if(HTS_COMPARE_HTS_VALID_RANGE_DESCRIPTOR_UUID_TO_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
                  VRD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
            }
         }
      }

      /* Next verify that the mandatory property is set (Readable).     */
      if(CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
      {
         /* Next verify that if the descriptors are present then the    */
         /* optional properties are set.                                */
         if((!CCCD) || (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
         {
            if((!VRD) || (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
            {
               /* Next save the required information.                   */
               ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle  = CharacteristicInformation->Characteristic_Handle;
               ConnectionEntry->HTPInformation.MeasurementIntervalCCCDHandle            = CCCD;
               ConnectionEntry->HTPInformation.MeasurementIntervalValidRangeHandle      = VRD;
               ConnectionEntry->HTPInformation.SupportedFeatures                       |= HTPM_CONNECTED_FLAGS_GET_MEASUREMENT_INTERVAL_SUPPORTED;

               if(VRD)
                  ConnectionEntry->HTPInformation.SupportedFeatures                    |= HTPM_CONNECTED_FLAGS_SET_MEASUREMENT_INTERVAL_SUPPORTED;

               /* Return success to the caller.                         */
               ret_val                                                                  = TRUE;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process and configure the specified HTP Service.                  */
static Boolean_t ProcessConfigureHTPService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *HTPService)
{
   Boolean_t    ret_val = FALSE;
   Boolean_t    NoError;
   unsigned int Index;
   unsigned int UUIDNumber;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (HTPService))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure HTP Service: 0x%04X - 0x%04X.\n", HTPService->ServiceInformation.Service_Handle, HTPService->ServiceInformation.End_Group_Handle));

      /* Loop through all of the characteristics of the service.        */
      for(Index=0,NoError=TRUE;(Index<HTPService->NumberOfCharacteristics)&&(NoError);Index++)
      {
         /* Verify that this characteristic has a 16-bit UUID.          */
         if(HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Determine what this characteristic is.                   */
            if(!HTS_COMPARE_HTS_TEMPERATURE_MEASUREMENT_UUID_TO_UUID_16(HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
            {
               if(!HTS_COMPARE_HTS_INTERMEDIATE_TEMPERATURE_UUID_TO_UUID_16(HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  if(!HTS_COMPARE_HTS_TEMPERATURE_TYPE_UUID_TO_UUID_16(HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                  {
                     if(!HTS_COMPARE_HTS_MEASUREMENT_INTERVAL_UUID_TO_UUID_16(HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                        UUIDNumber = 0;
                     else
                        UUIDNumber = 4;
                  }
                  else
                     UUIDNumber = 3;
               }
               else
                  UUIDNumber = 2;
            }
            else
               UUIDNumber = 1;

            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("HTP Characteristic UUID: 0x%02X%02X (UUIDNumber=%u).\n", HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16.UUID_Byte1, HTPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16.UUID_Byte0, UUIDNumber));

            /* Handle this characteristic.                              */
            switch(UUIDNumber)
            {
               case 1:
               case 2:
                  /* Process the HTP Temperature Measurement            */
                  /* Characteristic.                                    */
                  if(!ProcessHTPTemperatureMeasurementCharacteristic(ConnectionEntry, &(HTPService->CharacteristicInformationList[Index]), ((UUIDNumber == 1)?tmtTemperatureMeasurement:tmtIntermediateMeasurement)))
                     NoError = FALSE;
                  break;
               case 3:
                  /* Process the HTP Temperature Type Characteristic.   */
                  if(!ProcessHTPTemperatureTypeCharacteristic(ConnectionEntry, &(HTPService->CharacteristicInformationList[Index])))
                     NoError = FALSE;
                  break;
               case 4:
                  /* Process the HTP Measurement Interval               */
                  /* Characteristic.                                    */
                  if(!ProcessHTPMeasurementIntervalCharacteristic(ConnectionEntry, &(HTPService->CharacteristicInformationList[Index])))
                     NoError = FALSE;
                  break;
            }
         }
      }

      /* If no error occurred return success.                           */
      if(NoError)
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* Process and Configure a HTP Device Connection.                    */
static void ProcessConfigureHTPConnection(BD_ADDR_t BD_ADDR)
{
   Boolean_t                    ConfiguredSuccessfully;
   unsigned int                 Index;
   unsigned int                 NumberHTServices;
   Connection_Entry_t           ConnectionEntry;
   Connection_Entry_t          *ConnectionEntryPtr;
   DEVM_Parsed_Services_Data_t  ParsedGATTData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Verify that we are not already connected to this device.       */
      if(SearchConnectionEntry(&ConnectionEntryList, BD_ADDR) == NULL)
      {
         /* Before we go to the trouble of checking the service table,  */
         /* verify that this device supports the HTP Device Role.       */
         if(CheckHTPServicesSupported(BD_ADDR))
         {
            /* Query the parsed GATT data.                              */
            if(QueryParsedServicesData(BD_ADDR, &ParsedGATTData))
            {
               /* Initialize the Connection Entry to add.               */
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.ConnectionState     = csConfiguring;
               ConnectionEntry.BD_ADDR             = BD_ADDR;

               /* Add the Connection Info to the Connection List.       */
               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
               {
                  /* Process all of the required services.              */
                  for(Index=0,NumberHTServices=0,ConfiguredSuccessfully=TRUE;(Index<ParsedGATTData.NumberServices)&&(ConfiguredSuccessfully);Index++)
                  {
                     /* Verify that this service has a 16bit UUID.      */
                     if(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID_Type == guUUID_16)
                     {
                        /* Check to see if this is a HTP Service.       */
                        if(HTS_COMPARE_HTS_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16))
                        {
                           /* Attempt to process and configure the HTP  */
                           /* Service.                                  */
                           if(ProcessConfigureHTPService(ConnectionEntryPtr, &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index])))
                           {
                              ++NumberHTServices;
                              break;
                           }
                           else
                           {
                              ConfiguredSuccessfully = FALSE;
                              break;
                           }
                        }
                     }
                  }

                  /* If we did not configure the services correctly then*/
                  /* delete the connection entry.                       */
                  if((NumberHTServices == 0) || (!ConfiguredSuccessfully))
                  {
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }

               /* Free the parsed service data.                         */
               DEVM_FreeParsedServicesData(&ParsedGATTData);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a potential HTP Device connection.                        */
static void ProcessHTPDeviceConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(RemoteDeviceProperties)
   {
      /* Attempt to Process/Configure the HTP Device connection.        */
      ProcessConfigureHTPConnection(RemoteDeviceProperties->BD_ADDR);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* process a HTP Device Disconnection.                               */
static void ProcessHTPDeviceDisconnection(BD_ADDR_t RemoteDeviceAddress)
{
   HTPM_Event_Data_t            HTPMEventData;
   Connection_Entry_t          *ConnectionEntry;
   HTPM_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
   {
      /* Only dispatch the disconnection if we are in the connection    */
      /* state.                                                         */
      if(ConnectionEntry->ConnectionState == csConnected)
      {
         /* Next, format up the Event to dispatch.                      */
         HTPMEventData.EventType                                           = hetHTPDisconnected;
         HTPMEventData.EventLength                                         = HTPM_DISCONNECTED_EVENT_DATA_SIZE;

         HTPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         HTPMEventData.EventData.DisconnectedEventData.ConnectionType      = httCollector;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER;
         Message.MessageHeader.MessageFunction = HTPM_MESSAGE_FUNCTION_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HTPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = RemoteDeviceAddress;
         Message.ConnectionType                = httCollector;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHTPEvent(&HTPMEventData, (BTPM_Message_t *)&Message);
      }

      /* Free the memory allocated for the connection entry.            */
      FreeConnectionEntryMemory(ConnectionEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the HTPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(RemoteDeviceProperties)
   {
      /* Walk the Connection List and update any BD_ADDRs as needed.    */
      ConnectionEntryPtr = ConnectionEntryList;
      while(ConnectionEntryPtr)
      {
         /* Check to see if this entry needs to be updated.             */
         if(COMPARE_BD_ADDR(ConnectionEntryPtr->BD_ADDR, RemoteDeviceProperties->PriorResolvableBD_ADDR))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("HTPM Address Updated\n"));

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the HTPM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long RequiredConnectionFlags;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Address is updated or the LE Pairing State changes.            */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Process the LE Pairing State Change Event if necessary.     */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
         {
            /* Set the required flags.  We must be currently connected  */
            /* over LE and must know all of the remote device's services*/
            /* before processing the connection.                        */
            RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN);

            /* Check to see that the required flags are set for a       */
            /* connection.                                              */
            if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags)
            {
               /* Process this as a potential HTP device connection.    */
               ProcessHTPDeviceConnection(RemoteDeviceProperties);
            }
            else
            {
               /* Process this as a potential HTP device disconnection. */
               ProcessHTPDeviceDisconnection(RemoteDeviceProperties->BD_ADDR);
            }
         }

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register HTP       */
   /* Collector Events Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessRegisterHTPCollectorEventsMessage(HTPM_Register_Collector_Events_Request_t *Message)
{
   int                                       Result;
   HTP_Entry_Info_t                          HTPEntryInfo;
   HTPM_Register_Collector_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* For now we will only allow 1 Collector Event Callback to be    */
      /* registered at a time.  Whoever registers this will be the      */
      /* "Owner" of all Health Thermometer Monitor connections.  In the */
      /* future this could change and this check would have to be       */
      /* removed.                                                       */
      if(HTPEntryInfoList == NULL)
      {
         /* Attempt to add an entry into the HTP Entry list.            */
         BTPS_MemInitialize(&HTPEntryInfo, 0, sizeof(HTP_Entry_Info_t));

         HTPEntryInfo.CallbackID         = GetNextCallbackID();
         HTPEntryInfo.ClientID           = Message->MessageHeader.AddressID;
         HTPEntryInfo.Flags              = HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         if(AddHTPEntryInfoEntry(&HTPEntryInfoList, &HTPEntryInfo))
            Result = HTPEntryInfo.CallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HTPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.HTPCollectorEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status                      = 0;
      }
      else
      {
         ResponseMessage.HTPCollectorEventsHandlerID = 0;

         ResponseMessage.Status                      = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register HTP    */
   /* Collector Events Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessUnRegisterHTPCollectorEventsMessage(HTPM_Un_Register_Collector_Events_Request_t *Message)
{
   int                                           Result;
   HTP_Entry_Info_t                             *HTPEntryInfo;
   HTPM_Un_Register_Collector_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, Message->HTPCollectorEventsHandlerID)) != NULL) && (HTPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((HTPEntryInfo = DeleteHTPEntryInfoEntry(&HTPEntryInfoList, Message->HTPCollectorEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeHTPEntryInfoEntryMemory(HTPEntryInfo);

            /* Flag success.                                            */
            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HTPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HTP Get Temperature*/
   /* Type Request Message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessHTPGetTemperatureTypeRequestMessage(HTPM_Get_Temperature_Type_Request_t *Message)
{
   int                                   Result;
   HTP_Entry_Info_t                     *HTPEntryInfo;
   Connection_Entry_t                   *ConnectionEntry;
   HTPM_Get_Temperature_Type_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, Message->HTPCollectorEventsHandlerID)) != NULL) && (HTPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Call the internal function to perform the request.       */
            Result = HTPGetCharacteristicRequest(ConnectionEntry, HTPEntryInfo, ttGetTemperatureType);
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HTPM_GET_TEMPERATURE_TYPE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.TransactionID             = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.TransactionID             = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HTP Get Measurement*/
   /* Interval Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessHTPGetMeasurementIntervalRequestMessage(HTPM_Get_Measurement_Interval_Request_t *Message)
{
   int                                       Result;
   HTP_Entry_Info_t                         *HTPEntryInfo;
   Connection_Entry_t                       *ConnectionEntry;
   HTPM_Get_Measurement_Interval_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, Message->HTPCollectorEventsHandlerID)) != NULL) && (HTPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Call the internal function to perform the request.       */
            Result = HTPGetCharacteristicRequest(ConnectionEntry, HTPEntryInfo, ttGetMeasurementInterval);
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.TransactionID             = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.TransactionID             = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HTP Set Measurement*/
   /* Interval Request Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessHTPSetMeasurementIntervalRequestMessage(HTPM_Set_Measurement_Interval_Request_t *Message)
{
   int                                       Result;
   HTP_Entry_Info_t                         *HTPEntryInfo;
   Connection_Entry_t                       *ConnectionEntry;
   HTPM_Set_Measurement_Interval_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, Message->HTPCollectorEventsHandlerID)) != NULL) && (HTPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Call the internal function to perform the request.       */
            Result = HTPSetMeasurementIntervalRequest(ConnectionEntry, HTPEntryInfo, Message->MeasurementInterval);
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.TransactionID             = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.TransactionID             = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HTP Get Measurement*/
   /* Interval Valid Range Request Message and responds to the message  */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
static void ProcessHTPGetMeasurementIntervalValidRangeRequestMessage(HTPM_Get_Measurement_Interval_Valid_Range_Request_t *Message)
{
   int                                                   Result;
   HTP_Entry_Info_t                                     *HTPEntryInfo;
   Connection_Entry_t                                   *ConnectionEntry;
   HTPM_Get_Measurement_Interval_Valid_Range_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, Message->HTPCollectorEventsHandlerID)) != NULL) && (HTPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Call the internal function to perform the request.       */
            Result = HTPGetCharacteristicRequest(ConnectionEntry, HTPEntryInfo, ttGetValidRange);
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.TransactionID             = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.TransactionID             = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the HTP Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HTPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Register HTP Collector Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register HTP Collector Events Request.                */
               ProcessRegisterHTPCollectorEventsMessage((HTPM_Register_Collector_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register HTP Collectors Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register HTP Collector Events Request.             */
               ProcessUnRegisterHTPCollectorEventsMessage((HTPM_Un_Register_Collector_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Temperature Type Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_GET_TEMPERATURE_TYPE_REQUEST_SIZE)
            {
               /* Size seems valid, go ahead and process the request.   */
               ProcessHTPGetTemperatureTypeRequestMessage((HTPM_Get_Temperature_Type_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Measurement Interval Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_GET_MEASUREMENT_INTERVAL_REQUEST_SIZE)
            {
               /* Size seems valid, go ahead and process the request.   */
               ProcessHTPGetMeasurementIntervalRequestMessage((HTPM_Get_Measurement_Interval_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Measurement Interval Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_SET_MEASUREMENT_INTERVAL_REQUEST_SIZE)
            {
               /* Size seems valid, go ahead and process the request.   */
               ProcessHTPSetMeasurementIntervalRequestMessage((HTPM_Set_Measurement_Interval_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Measurement Interval Valid Range Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_REQUEST_SIZE)
            {
               /* Size seems valid, go ahead and process the request.   */
               ProcessHTPGetMeasurementIntervalValidRangeRequestMessage((HTPM_Get_Measurement_Interval_Valid_Range_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   HTP_Entry_Info_t  *HTPEntryInfo;
   HTP_Entry_Info_t **_HTPEntryInfoList;
   HTP_Entry_Info_t  *tmpHTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      HTPEntryInfo      = HTPEntryInfoList;
      _HTPEntryInfoList = &HTPEntryInfoList;
      while(HTPEntryInfo)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(HTPEntryInfo->ClientID == ClientID)
         {
            /* Note the next HTP Entry in the list (we are about to     */
            /* delete the current entry).                               */
            tmpHTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;

            /* Go ahead and delete the HTP Information Entry and clean  */
            /* up the resources.                                        */
            if((HTPEntryInfo = DeleteHTPEntryInfoEntry(_HTPEntryInfoList, HTPEntryInfo->CallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeHTPEntryInfoEntryMemory(HTPEntryInfo);
            }

            /* Go ahead and set the next HTP Information Entry (past the*/
            /* one we just deleted).                                    */
            HTPEntryInfo = tmpHTPEntryInfo;
         }
         else
            HTPEntryInfo = HTPEntryInfo->NextHTPEntryInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Transaction Response.                              */
static void ProcessGATTTransactionResponse(Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, Boolean_t ErrorResponse, void *EventData)
{
   Byte_t                   ErrorCode;
   Byte_t                   HTSTemperatureType;
   unsigned int             MeasurementInterval;
   unsigned int             LowerBounds;
   unsigned int             UpperBounds;
   HTP_Entry_Info_t        *EventCallbackInfo;
   HTPM_Temperature_Type_t  TemperatureType;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (TransactionInfo) && (EventData))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: Status = 0x%02X\n", (ErrorResponse?((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode:0)));

      /* Determine the correct error code.                              */
      if(ErrorResponse)
      {
         /* Get the proper error code.                                  */
         if(((GATM_Error_Response_Event_Data_t *)EventData)->ErrorType == retErrorResponse)
            ErrorCode = ((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode;
         else
            ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;
      }
      else
         ErrorCode = 0;

      /* Process the correct transaction.                               */
      switch(TransactionInfo->TransactionType)
      {
         case ttConfigureCCCD:
            /* Check to see if we have just finished configuring the    */
            /* measurement interval CCCD.                               */
            if(TransactionInfo->CharacteristicValueHandle == ConnectionEntry->HTPInformation.MeasurementIntervalCCCDHandle)
            {
               /* Clear the operation in progress flag.                 */
               ConnectionEntry->OperationFlags &= ~((unsigned long)CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD);

               /* If the operation was success flag that it was.        */
               if(!ErrorResponse)
                  ConnectionEntry->OperationFlags |= (unsigned long)CONNECTION_ENTRY_OPERATION_FLAGS_MEASUREMENT_INTERVAL_CCCD_DONE;
            }
            break;
         case ttGetMeasurementInterval:
            /* Clear the operation in progress flag.                    */
            ConnectionEntry->OperationFlags &= ~((unsigned long)CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_MEASUREMENT_INTERVAL);

            /* If this isn't a error response then get the requested    */
            /* value out of the response.                               */
            if(!ErrorResponse)
            {
               /* Verify that the value length is valid.                */
               if(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength == NON_ALIGNED_WORD_SIZE)
                  MeasurementInterval = READ_UNALIGNED_WORD_LITTLE_ENDIAN(((GATM_Read_Response_Event_Data_t *)EventData)->Value);
               else
               {
                  ErrorCode           = ATT_PROTOCOL_ERROR_CODE_INVALID_PDU;
                  MeasurementInterval = 0;
               }
            }
            else
               MeasurementInterval = 0;

            /* Dispatch the event to the caller.                        */
            if((EventCallbackInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
               DispatchHTPGetMeasurementIntervalResponse(ConnectionEntry, EventCallbackInfo, TransactionInfo->TransactionID, ErrorCode, MeasurementInterval);
            break;
         case ttSetMeasurementInterval:
            /* Clear the operation in progress flag.                    */
            ConnectionEntry->OperationFlags &= ~((unsigned long)CONNECTION_ENTRY_OPERATION_FLAGS_SETTING_MEASUREMENT_INTERVAL);

            /* Dispatch the event to the caller.                        */
            if((EventCallbackInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
               DispatchHTPSetMeasurementIntervalResponse(ConnectionEntry, EventCallbackInfo, TransactionInfo->TransactionID, ErrorCode);
            break;
         case ttGetValidRange:
            /* Clear the operation in progress flag.                    */
            ConnectionEntry->OperationFlags &= ~((unsigned long)CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_VALID_RANGE);

            /* If this isn't a error response then get the requested    */
            /* value out of the response.                               */
            if(!ErrorResponse)
            {
               /* Verify that the value length is valid.                */
               if(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength == HTS_VALID_RANGE_SIZE)
               {
                  LowerBounds = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((HTS_Valid_Range_t *)(((GATM_Read_Response_Event_Data_t *)EventData)->Value))->Lower_Bounds));
                  UpperBounds = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((HTS_Valid_Range_t *)(((GATM_Read_Response_Event_Data_t *)EventData)->Value))->Upper_Bounds));
               }
               else
               {
                  ErrorCode   = ATT_PROTOCOL_ERROR_CODE_INVALID_PDU;
                  LowerBounds = 0;
                  UpperBounds = 0;
               }
            }
            else
            {
               LowerBounds = 0;
               UpperBounds = 0;
            }

            /* Dispatch the event to the caller.                        */
            if((EventCallbackInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
               DispatchHTPGetMeasurementIntervalValidRangeResponse(ConnectionEntry, EventCallbackInfo, TransactionInfo->TransactionID, ErrorCode, LowerBounds, UpperBounds);
            break;
         case ttGetTemperatureType:
            /* Clear the operation in progress flag.                    */
            ConnectionEntry->OperationFlags &= ~((unsigned long)CONNECTION_ENTRY_OPERATION_FLAGS_GETTING_TEMPERATURE_TYPE);

            /* If this isn't a error response then get the requested    */
            /* value out of the response.                               */
            if(!ErrorResponse)
            {
               /* Verify that the value length is valid.                */
               if(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength == HTS_TEMPERATURE_TYPE_VALUE_LENGTH)
                  HTSTemperatureType = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(((GATM_Read_Response_Event_Data_t *)EventData)->Value);
               else
               {
                  ErrorCode          = ATT_PROTOCOL_ERROR_CODE_INVALID_PDU;
                  HTSTemperatureType = HTS_TEMPERATURE_TYPE_ARMPIT;
               }
            }
            else
               HTSTemperatureType = HTS_TEMPERATURE_TYPE_ARMPIT;

            /* Convert the temperature type.                            */
            if(!ConvertHTSTempTypeToHTPM(HTSTemperatureType, &TemperatureType))
            {
               ErrorCode       = ATT_PROTOCOL_ERROR_CODE_INVALID_PDU;
               TemperatureType = httArmpit;
            }

            /* Dispatch the event to the caller.                        */
            if((EventCallbackInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, TransactionInfo->HTPManagerDataCallbackID)) != NULL)
               DispatchHTPGetTemperatureTypeResponse(ConnectionEntry, EventCallbackInfo, TransactionInfo->TransactionID, ErrorCode, TemperatureType);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled response type\n"));
            break;
      }

      /* If an error occurs when configuring the HTP Service we will    */
      /* just delete the connection entry.                              */
      if((ErrorResponse) && (ConnectionEntry->ConnectionState == csConfiguring))
      {
         if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntry->BD_ADDR)) != NULL)
            FreeConnectionEntryMemory(ConnectionEntry);
      }
      else
      {
         /* If we are in the configuration stage we will check to see if*/
         /* all of the configuration is done.                           */
         if(ConnectionEntry->ConnectionState == csConfiguring)
         {
            /* If the Transaction Info list is NULL it means that we    */
            /* have received responses for every configuration          */
            /* transaction and can therefore dispatch a HTP Device      */
            /* Connection event.                                        */
            if(ConnectionEntry->TransactionInfoList == NULL)
            {
               /* Dispatch the HTP Device Connection event.             */
               DispatchHTPDeviceConnection(ConnectionEntry);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Handle Value notification.                         */
   /* * NOTE * This function *MUST* be called with the HTP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   SByte_t                                    MeasurementExponent;
   DWord_t                                    MeasurementValue;
   SDWord_t                                   Mantissa;
   unsigned int                               MeasurementInterval;
   Connection_Entry_t                        *ConnectionEntry;
   HTS_Temperature_Measurement_Data_t         HTSTemperatureMeasurement;
   HTPM_Temperature_Measurement_Event_Data_t  TemperatureMeasurement;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(HandleValueEventData)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, HandleValueEventData->RemoteDeviceAddress)) != NULL)
      {
         /* Determine what is being indicated/notified.                 */
         if((HandleValueEventData->AttributeHandle == ConnectionEntry->HTPInformation.IntermediateTemperatureCharacteristicHandle) || (HandleValueEventData->AttributeHandle == ConnectionEntry->HTPInformation.TemperatureCharacteristicHandle))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Temperature Measurement Indication/Notification\n"));

            /* Attempt to decode the temperature measurement.           */
            if(!HTS_Decode_Temperature_Measurement(HandleValueEventData->AttributeValueLength, HandleValueEventData->AttributeValue, &HTSTemperatureMeasurement))
            {
               /* Initialize the Temperature Measurement.               */
               BTPS_MemInitialize(&TemperatureMeasurement, 0, sizeof(TemperatureMeasurement));

               TemperatureMeasurement.MeasurementType     = (HandleValueEventData->AttributeHandle == ConnectionEntry->HTPInformation.IntermediateTemperatureCharacteristicHandle)?tmtIntermediateMeasurement:tmtTemperatureMeasurement;
               TemperatureMeasurement.RemoteDeviceAddress = HandleValueEventData->RemoteDeviceAddress;

               /* Check to see what the units are.                      */
               if(HTSTemperatureMeasurement.Flags & HTS_TEMPERATURE_MEASUREMENT_FLAGS_FAHRENHEIT)
                  TemperatureMeasurement.MeasurementFlags |= HTPM_MEASUREMENT_FLAGS_TEMPERATURE_FAHRENHEIT;

               /* Check to see if the Temperature Type is specified in  */
               /* the measurement.                                      */
               if(HTSTemperatureMeasurement.Flags & HTS_TEMPERATURE_MEASUREMENT_FLAGS_TEMPERATURE_TYPE)
               {
                  /* Attempt to convert the temperature type.           */
                  if(ConvertHTSTempTypeToHTPM(HTSTemperatureMeasurement.Temperature_Type, &(TemperatureMeasurement.TemperatureType)))
                     TemperatureMeasurement.MeasurementFlags |= HTPM_MEASUREMENT_FLAGS_TEMPERATURE_TYPE_VALID;
               }

               /* Check to see if a Time Stamp is specified in the      */
               /* measurement.                                          */
               if(HTSTemperatureMeasurement.Flags & HTS_TEMPERATURE_MEASUREMENT_FLAGS_TIME_STAMP)
               {
                  /* Convert the time stamp.                            */
                  TemperatureMeasurement.TimeStamp.Month    = (unsigned int)HTSTemperatureMeasurement.Time_Stamp.Month;
                  TemperatureMeasurement.TimeStamp.Day      = (unsigned int)HTSTemperatureMeasurement.Time_Stamp.Day;
                  TemperatureMeasurement.TimeStamp.Year     = (unsigned int)HTSTemperatureMeasurement.Time_Stamp.Year;
                  TemperatureMeasurement.TimeStamp.Hours    = (unsigned int)HTSTemperatureMeasurement.Time_Stamp.Hours;
                  TemperatureMeasurement.TimeStamp.Minutes  = (unsigned int)HTSTemperatureMeasurement.Time_Stamp.Minutes;
                  TemperatureMeasurement.TimeStamp.Seconds  = (unsigned int)HTSTemperatureMeasurement.Time_Stamp.Seconds;

                  /* Flag that the Time Stamp is valid.                 */
                  TemperatureMeasurement.MeasurementFlags  |= HTPM_MEASUREMENT_FLAGS_TIME_STAMP_VALID;
               }

               /* Convert the Temperature Value.                        */
               MeasurementValue = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&(HTSTemperatureMeasurement.Temperature));

               /* Check to see if NaN is specified.                     */
               if(MeasurementValue == 0x007FFFFF)
                  TemperatureMeasurement.MeasurementFlags |= HTPM_MEASUREMENT_FLAGS_TEMPERATURE_VALUE_NAN;
               else
               {
                  /* Get the mantissa and exponent.                     */
                  Mantissa            = (SDWord_t)(MeasurementValue & 0x00FFFFFF);
                  MeasurementExponent = (SByte_t)((MeasurementValue & 0xFF000000)>>24);

                  /* Sign extend the Temperature Mantissa as neccessary.*/
                  if(Mantissa & 0x00800000)
                     Mantissa |= 0xFF000000;

                  /* Assign the mantissa.                               */
                  TemperatureMeasurement.TemperatureMantissa = (long)Mantissa;

                  /* Assign the exponent.                               */
                  TemperatureMeasurement.TemperatureExponent = (int)MeasurementExponent;
               }

               /* Dispatch the event.                                   */
               DispatchHTPTemperatureMeasurementEvent(ConnectionEntry, &TemperatureMeasurement);
            }
         }
         else
         {
            /* Check to see if this is a Measurement Interval           */
            /* indication.                                              */
            if(HandleValueEventData->AttributeHandle == ConnectionEntry->HTPInformation.MeasurementIntervalCharacteristicHandle)
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Measurement Interval Indication\n"));

               /* Verify that the length of the data is correct for the */
               /* measurement interval.                                 */
               if(HandleValueEventData->AttributeValueLength == NON_ALIGNED_WORD_SIZE)
               {
                  /* Read the measurement interval.                     */
                  MeasurementInterval = READ_UNALIGNED_WORD_LITTLE_ENDIAN(HandleValueEventData->AttributeValue);

                  /* Dispatch the Measurement Interval event.           */
                  DispatchHTPGetMeasurementIntervalResponse(ConnectionEntry, NULL, 0, 0, MeasurementInterval);
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Read Response.                                     */
   /* * NOTE * This function *MUST* be called with the HTP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTReadResponse(GATM_Read_Response_Event_Data_t *ReadResponseData)
{
   Transaction_Info_t *TransactionInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ReadResponseData)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ReadResponseData->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionInfo = DeleteTransactionInfoByGATMTransactionID(&(ConnectionEntry->TransactionInfoList), ReadResponseData->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionInfo, FALSE, (void *)ReadResponseData);

            /* Free the memory allocated for this transaction.          */
            FreeTransactionInfoMemory(TransactionInfo);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Write Response.                                    */
   /* * NOTE * This function *MUST* be called with the HTP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse)
{
   Transaction_Info_t *TransactionInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(WriteResponse)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, WriteResponse->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionInfo = DeleteTransactionInfoByGATMTransactionID(&(ConnectionEntry->TransactionInfoList), WriteResponse->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionInfo, FALSE, (void *)WriteResponse);

            /* Free the memory allocated for this transaction.          */
            FreeTransactionInfoMemory(TransactionInfo);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Error Response.                                    */
   /* * NOTE * This function *MUST* be called with the HTP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse)
{
   Transaction_Info_t *TransactionInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ErrorResponse)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ErrorResponse->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionInfo = DeleteTransactionInfoByGATMTransactionID(&(ConnectionEntry->TransactionInfoList), ErrorResponse->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionInfo, TRUE, (void *)ErrorResponse);

            /* Free the memory allocated for this transaction.          */
            FreeTransactionInfoMemory(TransactionInfo);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HTP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HTPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the GATM Event Callback that is */
   /* installed to process all GATM Events.                             */
static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter)
{
   /* Verify that the input parameters are semi-valid.                  */
   if(EventData)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            switch(EventData->EventType)
            {
               case getGATTHandleValueData:
                  /* Process the GATT Handle Value Data event.          */
                  ProcessGATTHandleValueData(&(EventData->EventData.HandleValueDataEventData));
                  break;
               case getGATTReadResponse:
                  /* Process the GATT Read Response.                    */
                  ProcessGATTReadResponse(&(EventData->EventData.ReadResponseEventData));
                  break;
               case getGATTWriteResponse:
                  /* Process the GATT Write Response.                   */
                  ProcessGATTWriteResponse(&(EventData->EventData.WriteResponseEventData));
                  break;
               case getGATTErrorResponse:
                  /* Process the GATT Error Response.                   */
                  ProcessGATTErrorResponse(&(EventData->EventData.ErrorResponseEventData));
                  break;
               default:
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event\n"));
                  break;
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all HTP Manager Messages.   */
static void BTPSAPI HTPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("HTP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HTP Manager defined    */
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
               /* HTP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HTPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HTP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HTP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HTP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HTP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HTP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HTPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HTP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process HTP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER, HTPManagerGroupHandler, NULL))
         {
            /* Initialize the actual HTP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the HTP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _HTPM_Initialize()))
            {
               /* Register a GATM Event Callback.                       */
               if((Result = GATM_RegisterEventCallback(GATM_Event_Callback, NULL)) > 0)
               {
                  /* Save the GATM Event Callback ID.                   */
                  GATMEventCallbackID     = (unsigned int)Result;

                  /* Determine the current Device Power State.          */
                  CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting HTP Callback ID.     */
                  NextCallbackID          = 0x000000001;

                  /* Initialize a unique, starting Tansaction ID.       */
                  TransactionID           = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized             = TRUE;

                  /* Flag success.                                      */
                  Result                  = 0;
               }
               else
               {
                  /* Failed to register callback so cleanup the HTP     */
                  /* Manager Implementation Module.                     */
                  _HTPM_Cleanup();
               }
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _HTPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("HTP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the HTP Manager Implementation that  */
            /* we are shutting down.                                    */
            _HTPM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the HTP Entry Information List is empty.  */
            FreeHTPEntryInfoList(&HTPEntryInfoList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HTPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the HTP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _HTPM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the HTP Manager that the Stack has been closed.*/
               _HTPM_SetBluetoothStackID(0);

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Process this as a potential HTP Device Disconnection. */
               ProcessHTPDeviceDisconnection(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Health           */
   /* Thermometer (HTP) Manager Service.  This Callback will be         */
   /* dispatched by the HTP Manager when various HTP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a HTP Manager Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HTPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI HTPM_Register_Collector_Event_Callback(HTPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HTP_Entry_Info_t HTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* For now we will only allow 1 Collector Event Callback to */
            /* be registered at a time.  Whoever registers this will be */
            /* the "Owner" of all Health Thermometer Monitor            */
            /* connections.  In the future this could change and this   */
            /* check would have to be removed.                          */
            if(HTPEntryInfoList == NULL)
            {
               /* Attempt to add an entry into the HTP Entry list.      */
               BTPS_MemInitialize(&HTPEntryInfo, 0, sizeof(HTP_Entry_Info_t));

               HTPEntryInfo.CallbackID         = GetNextCallbackID();
               HTPEntryInfo.ClientID           = MSG_GetServerAddressID();
               HTPEntryInfo.EventCallback      = CallbackFunction;
               HTPEntryInfo.CallbackParameter  = CallbackParameter;
               HTPEntryInfo.Flags              = HTP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               if(AddHTPEntryInfoEntry(&HTPEntryInfoList, &HTPEntryInfo))
                  ret_val = HTPEntryInfo.CallbackID;
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

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
      ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HTP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HTPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the HTP Manager Event Callback ID (return value  */
   /* from HTPM_Register_Collector_Event_Callback() function).          */
void BTPSAPI HTPM_Un_Register_Collector_Event_Callback(unsigned int HTPMCollectorCallbackID)
{
   HTP_Entry_Info_t *HTPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HTPMCollectorCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL) && (HTPEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Delete the Callback Entry.                            */
               if((HTPEntryInfo = DeleteHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL)
               {
                  /* Free the memory because we are finished with it.   */
                  FreeHTPEntryInfoEntryMemory(HTPEntryInfo);
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Temperature Type procedure to a remote HTP   */
   /* Sensor.  This function accepts as input the HTP Collector Callback*/
   /* ID (return value from HTPM_Register_Collector_Event_Callback()    */
   /* function), and the BD_ADDR of the remote HTP Sensor.  This        */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
   /* * NOTE * The hetHTPGetTemperatureTypeResponse event will be       */
   /*          generated when the remote HTP Sensor responds to the Get */
   /*          Temperature Type Request.                                */
int BTPSAPI HTPM_Get_Temperature_Type_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((HTPMCollectorCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL) && (HTPEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Determine the connection entry for the specified      */
               /* connection and make sure we are done configuring the  */
               /* device.                                               */
               if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
               {
                  /* Call the internal function to perform the request. */
                  ret_val = HTPGetCharacteristicRequest(ConnectionEntry, HTPEntryInfo, ttGetTemperatureType);
               }
               else
                  ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

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
      ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Callback ID (return value from                                    */
   /* HTPM_Register_Collector_Event_Callback() function), and the       */
   /* BD_ADDR of the remote HTP Sensor.  This function returns the      */
   /* positive, non-zero, Transaction ID of the request on success or a */
   /* negative error code.                                              */
   /* * NOTE * The hetHTPGetMeasurementIntervalResponse event will be   */
   /*          generated when the remote HTP Sensor responds to the Get */
   /*          Measurement Interval Request.                            */
int BTPSAPI HTPM_Get_Measurement_Interval_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((HTPMCollectorCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL) && (HTPEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Determine the connection entry for the specified      */
               /* connection and make sure we are done configuring the  */
               /* device.                                               */
               if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
               {
                  /* Call the internal function to perform the request. */
                  ret_val = HTPGetCharacteristicRequest(ConnectionEntry, HTPEntryInfo, ttGetMeasurementInterval);
               }
               else
                  ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

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
      ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Set Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Callback ID (return value from                                    */
   /* HTPM_Register_Collector_Event_Callback() function), the BD_ADDR of*/
   /* the remote HTP Sensor, and the Measurement Interval to attempt to */
   /* set.  This function returns the positive, non-zero, Transaction ID*/
   /* of the request on success or a negative error code.               */
   /* * NOTE * The hetHTPSetMeasurementIntervalResponse event will be   */
   /*          generated when the remote HTP Sensor responds to the Set */
   /*          Measurement Interval Request.                            */
int BTPSAPI HTPM_Set_Measurement_Interval_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MeasurementInterval)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((HTPMCollectorCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL) && (HTPEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Determine the connection entry for the specified      */
               /* connection and make sure we are done configuring the  */
               /* device.                                               */
               if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
               {
                  /* Call the internal function to do the work.         */
                  ret_val = HTPSetMeasurementIntervalRequest(ConnectionEntry, HTPEntryInfo, MeasurementInterval);
               }
               else
                  ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

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
      ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval Valid Range procedure to*/
   /* a remote HTP Sensor.  This function accepts as input the HTP      */
   /* Collector Callback ID (return value from                          */
   /* HTPM_Register_Collector_Event_Callback() function), and the       */
   /* BD_ADDR of the remote HTP Sensor.  This function returns the      */
   /* positive, non-zero, Transaction ID of the request on success or a */
   /* negative error code.                                              */
   /* * NOTE * The hetHTPGetMeasurementIntervalValidRangeResponse event */
   /*          will be generated when the remote HTP Sensor responds to */
   /*          the Get Measurement Interval Request.                    */
int BTPSAPI HTPM_Get_Measurement_Interval_Valid_Range_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int                 ret_val;
   HTP_Entry_Info_t   *HTPEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HTP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if((HTPMCollectorCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HTPEntryInfo = SearchHTPEntryInfoEntry(&HTPEntryInfoList, HTPMCollectorCallbackID)) != NULL) && (HTPEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Determine the connection entry for the specified      */
               /* connection and make sure we are done configuring the  */
               /* device.                                               */
               if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
               {
                  /* Call the internal function to perform the request. */
                  ret_val = HTPGetCharacteristicRequest(ConnectionEntry, HTPEntryInfo, ttGetValidRange);
               }
               else
                  ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

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
      ret_val = BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HEALTH_THERMOMETER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


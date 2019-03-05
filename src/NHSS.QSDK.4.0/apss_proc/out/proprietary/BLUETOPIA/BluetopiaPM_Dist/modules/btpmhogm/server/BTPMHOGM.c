/*****< btpmhogm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHOGM - HID over GATT Manager for Stonestreet One Bluetooth Protocol   */
/*             Stack Platform Manager.                                        */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"            /* BTPS Protocol Stack Prototypes/Constants.  */
#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */

#include "BTPMHOGM.h"           /* BTPM HOG Manager Prototypes/Constants.     */
#include "HOGMAPI.h"            /* HOG Manager Prototypes/Constants.          */
#include "HOGMMSG.h"            /* BTPM HOG Manager Message Formats.          */
#include "HOGMGR.h"             /* HOG Manager Impl. Prototypes/Constants.    */

#include "SS1BTHIDS.h"          /* Bluetooth HID Service Prototypes/Constants.*/
#include "SS1BTDIS.h"           /* Bluetooth DIS Service Prototypes/Constants.*/
#include "SS1BTSCP.h"           /* Bluetooth ScP Service Prototypes/Constants.*/

#include "SS1BTPM.h"            /* BTPM Main Prototypes and Constants.        */
#include "BTPMERR.h"            /* BTPM Error Prototypes/Constants.           */
#include "BTPMCFG.h"            /* BTPM Configuration Settings/Constants.     */

   /* The following defines the HOGM LE Configuration File Section Name.*/
#define HOGM_LE_CONFIGURATION_FILE_SECTION_NAME                   "HOGM-Host"

   /* The following defines the Maximum Key Size that is used in the    */
   /* HOGM LE Configuration File.                                       */
#define HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH             (4+(BD_ADDR_SIZE*2)+WORD_SIZE+8)

   /* The following defines the HOGM LE Configuration File Maximum Line */
   /* Length.                                                           */
#define HOGM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH            ((BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_LINE_LENGTH/2)-HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH)

   /* The following define the Key Names that are used with the HOGM LE */
   /* Configuration File.                                               */
#define HOGM_LE_KEY_NAME_REPORT_DESCRIPTOR_PREFIX                 "RD-%02X%02X%02X%02X%02X%02X-%04u"
#define HOGM_LE_KEY_NAME_REPORT_INFORMATION_PREFIX                "RI-%02X%02X%02X%02X%02X%02X-%04u"
#define HOGM_LE_KEY_NAME_HID_INFORMATION_PREFIX                   "HI-%02X%02X%02X%02X%02X%02X"

   /* The following define special values used to indicate HID Boot     */
   /* Reports.*                                                         */
#define HOGM_HID_REPORT_TYPE_INPUT_REPORT                         0x00000002
#define HOGM_HID_REPORT_TYPE_OUTPUT_REPORT                        0x00000003
#define HOGM_HID_REPORT_TYPE_FEATURE_REPORT                       0x00000004
#define HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT           0x00000005
#define HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_OUTPUT_REPORT          0x00000006
#define HOGM_HID_REPORT_TYPE_BOOT_MOUSE_INPUT_REPORT              0x00000007

   /* Structure which is used to hold all of the HID binary information */
   /* that is stored to file for a paired HID Device.                   */
typedef struct _tagHID_Information_Entry_t
{
   NonAlignedDWord_t SupportedFeatures;
   NonAlignedWord_t  HIDVersion;
   NonAlignedWord_t  CountryCode;
   NonAlignedWord_t  VendorID_Source;
   NonAlignedWord_t  VendorID;
   NonAlignedWord_t  ProductID;
   NonAlignedWord_t  ProductVersion;
} HID_Information_Entry_t;

#define HID_INFORMATION_ENTRY_DATA_SIZE                           (sizeof(HID_Information_Entry_t))

   /* Structure which is used to hold of all the HID Report binary      */
   /* information that is stored for each report supported for a paired */
   /* HID Device.                                                       */
typedef struct _tagHID_Report_Entry_t
{
   NonAlignedWord_t CharacteristicValueHandle;
   NonAlignedWord_t CCCD;
   NonAlignedByte_t ReportType;
   NonAlignedByte_t ReportID;
   NonAlignedByte_t ReportProperties;
} HID_Report_Entry_t;

#define HID_REPORT_ENTRY_DATA_SIZE                                (sizeof(HID_Report_Entry_t))

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHOG_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ClientID;
   unsigned long                Flags;
   HOGM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHOG_Entry_Info_t *NextHOGEntryInfoPtr;
} HOG_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HOG_Entry_Info_t structure to denote various state information.   */
#define HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   HOGM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the type of       */
   /* outstanding transactions that are possible.                       */
typedef enum
{
   ttReadHIDInformation,
   ttReadReportMap,
   ttReadReportReference,
   ttReadExternalReportReference,
   ttConfigureCCCD,
   ttSetReport,
   ttGetReport,
   ttReadPnPID
} Transaction_Type_t;

   /* The following structure is a container structure that contains all*/
   /* of the information on an outstanding transaction.                 */
typedef struct _tagTransaction_Info_t
{
   unsigned int                   TransactionID;
   unsigned int                   GATMTransactionID;
   unsigned int                   HOGManagerDataCallbackID;
   Transaction_Type_t             TransactionType;
   Word_t                         CharacteristicValueHandle;
   HOGM_HID_Protocol_Mode_t       ProtocolMode;
   struct _tagTransaction_Info_t *NextTransactionInfoPtr;
} Transaction_Info_t;

   /* The following structure is a container structure which contains   */
   /* all of the information on a HID Report.                           */
typedef struct _tagHID_Report_Info_t
{
   HOGM_HID_Report_Information_t  HIDReportInformation;
   Word_t                         CharacteristicValueHandle;
   Word_t                         CharacterisicConfigurationHandle;
   Byte_t                         ReportProperties;
   struct _tagHID_Report_Info_t  *NextReportInfoPtr;
} HID_Report_Info_t;

   /* The following structure is a container structure which contains   */
   /* all of the information on a HID Device.                           */
typedef struct _tagHID_Information_t
{
   HOGM_HID_Information_t  HIDDeviceInformation;
   Word_t                  ProtocolModeCharacteristicHandle;
   Word_t                  ControlPointCharacteristicHandle;
   Word_t                  ScanIntervalWindowCharacteristicHandle;
   Word_t                  ScanRefreshCharacteristicHandle;
   unsigned long           SupportedFeatures;
   unsigned int            ReportDescriptorLength;
   Byte_t                 *ReportDescriptor;
} HID_Information_t;

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
   HOGM_HID_Protocol_Mode_t       CurrentProtocolMode;
   HID_Information_t              HIDInformation;
   HID_Report_Info_t             *ReportInfoList;
   Transaction_Info_t            *TransactionInfoList;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

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

   /* Variable which holds a pointer to the first element in the HOG    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HOG_Entry_Info_t *HOGEntryInfoList;

   /* Variable which holds a pointer to the first element of the HOG    */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static HOG_Entry_Info_t *HOGEntryInfoDataList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which supported features.                                */
static unsigned long SupportedFeatures;

   /* Variable which holds the GATM Event Callback ID.                  */
static unsigned int GATMEventCallbackID;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);
static unsigned int GetNextTransactionID(void);

static HOG_Entry_Info_t *AddHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, HOG_Entry_Info_t *EntryToAdd);
static HOG_Entry_Info_t *SearchHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID);
static HOG_Entry_Info_t *DeleteHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHOGEntryInfoEntryMemory(HOG_Entry_Info_t *EntryToFree);
static void FreeHOGEntryInfoList(HOG_Entry_Info_t **ListHead);

static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd);
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int TransactionID);
static Transaction_Info_t *DeleteTransactionInfoByGATMTransactionID(Transaction_Info_t **ListHead, unsigned int GATMTransactionID);
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree);
static void FreeTransactionInfoList(Transaction_Info_t **ListHead);

static HID_Report_Info_t *AddHIDReportInfo(HID_Report_Info_t **ListHead, HID_Report_Info_t *EntryToAdd);
static HID_Report_Info_t *SearchHIDReportInfo(HID_Report_Info_t **ListHead, Word_t CharacteristicValueHandle);
static HID_Report_Info_t *SearchHIDReportInfoByReportInfo(HID_Report_Info_t **ListHead, HOGM_HID_Report_Information_t *HIDReportInformation);
static HID_Report_Info_t *DeleteHIDReportInfo(HID_Report_Info_t **ListHead, Word_t CharacteristicValueHandle);
static void FreeHIDReportInfoMemory(HID_Report_Info_t *EntryToFree);
static void FreeHIDReportInfoList(HID_Report_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static void DispatchHOGEvent(HOGM_Event_Data_t *HOGMEventData, BTPM_Message_t *Message);
static void DispatchHOGDataEvent(HOGM_Event_Data_t *HOGMEventData, BTPM_Message_t *Message);

static void DispatchHIDDeviceConnection(Connection_Entry_t *ConnectionEntry);
static void DispatchHIDDataIndication(Connection_Entry_t *ConnectionEntry, HID_Report_Info_t *ReportInfo, unsigned int DataLength, Byte_t *Data);
static void DispatchHIDSetReportResponse(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *HIDReportInfo, unsigned int TransactionID, Byte_t ErrorCode);
static void DispatchHIDGetReportResponse(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *HIDReportInfo, unsigned int TransactionID, Byte_t ErrorCode, unsigned int DataLength, Byte_t *Data);

static void ReloadHIDReportDescriptors(Connection_Entry_t *ConnectionEntry);
static void ReloadHIDReportInformation(Connection_Entry_t *ConnectionEntry);
static Boolean_t ReloadHIDInformation(Connection_Entry_t *ConnectionEntry, BD_ADDR_t RemoteDeviceAddress);
static Connection_Entry_t *LoadHIDDeviceFromFile(BD_ADDR_t RemoteDeviceAddress);
static void StoreHIDReportDescriptors(Connection_Entry_t *ConnectionEntry, Boolean_t Store);
static void StoreHIDInformation(Connection_Entry_t *ConnectionEntry, Boolean_t Store);
static void StoreHIDDeviceToFile(Connection_Entry_t *ConnectionEntry, Boolean_t Store);

static Boolean_t CheckHIDServicesSupported(BD_ADDR_t BD_ADDR);
static Boolean_t QueryParsedServicesData(BD_ADDR_t BD_ADDR, DEVM_Parsed_Services_Data_t *ParsedGATTData);

static int HIDSetProtocolMode(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HIDS_Protocol_Mode_t ProtocolMode);
static int HIDGetReportRequest(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *ReportInfo);
static int HIDSetReportRequest(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *ReportInfo, Boolean_t ResponseExpected, unsigned int DataLength, Byte_t *Data);
static int HIDSetSuspendMode(Connection_Entry_t *ConnectionEntry, Boolean_t Suspend);

static Boolean_t ProcessHIDProtocolModeCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessHIDReportMapCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessHIDReportCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation, unsigned int ReportType);
static Boolean_t ProcessHIDInformationCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessConfigureHIDService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *HIDService);
static Boolean_t ProcessDISPnPIDCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessConfigureDIService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *DISService);
static Boolean_t RefereshScanIntervalWindowCharacteristic(Connection_Entry_t *ConnectionEntry, Word_t Handle, Word_t ScanInterval, Word_t ScanWindow);
static Boolean_t ProcessSCPSScanRefreshCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessSCPSScanIntervalWindowCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessConfigureSCPService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *DISService);
static Boolean_t ProcessConfigureService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *DISService);
static void ProcessConfigureHIDConnection(BD_ADDR_t BD_ADDR);

static void ParseHOGAttributeHandles(Connection_Entry_t *ConnectionEntry);
static void ProcessHIDDeviceConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessHIDDeviceDisconnection(BD_ADDR_t RemoteDeviceAddress);

static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRegisterHOGEventsMessage(HOGM_Register_HID_Events_Request_t *Message);
static void ProcessUnRegisterHOGEventsMessage(HOGM_Un_Register_HID_Events_Request_t *Message);
static void ProcessRegisterHOGDataEventsMessage(HOGM_Register_HID_Data_Events_Response_t *Message);
static void ProcessUnRegisterHOGDataEventsMessage(HOGM_Un_Register_HID_Data_Events_Request_t *Message);
static void ProcessSetProtocolModeMessage(HOGM_HID_Set_Protocol_Mode_Request_t *Message);
static void ProcessSetSuspendModeMessage(HOGM_HID_Set_Suspend_Mode_Request_t *Message);
static void ProcessGetReportMessage(HOGM_HID_Get_Report_Request_t *Message);
static void ProcessSetReportMessage(HOGM_HID_Set_Report_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessGATTTransactionResponse(Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, Boolean_t ErrorResponse, void *EventData);

static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessGATTReadResponse(GATM_Read_Response_Event_Data_t *ReadResponseData);
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse);
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse);

static void BTPSAPI BTPMDispatchCallback_HOGM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter);
static void BTPSAPI HOGManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HOG Entry Information List.                              */
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
static HOG_Entry_Info_t *AddHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, HOG_Entry_Info_t *EntryToAdd)
{
   HOG_Entry_Info_t *AddedEntry = NULL;
   HOG_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HOG_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HOG_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHOGEntryInfoPtr = NULL;

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
                     FreeHOGEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHOGEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHOGEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHOGEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HOG_Entry_Info_t *SearchHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HOG_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHOGEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HOG Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HOG Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHOGEntryInfoEntryMemory().                   */
static HOG_Entry_Info_t *DeleteHOGEntryInfoEntry(HOG_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HOG_Entry_Info_t *FoundEntry = NULL;
   HOG_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHOGEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHOGEntryInfoPtr = FoundEntry->NextHOGEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHOGEntryInfoPtr;

         FoundEntry->NextHOGEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HOG Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHOGEntryInfoEntryMemory(HOG_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HOG Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHOGEntryInfoList(HOG_Entry_Info_t **ListHead)
{
   HOG_Entry_Info_t *EntryToFree;
   HOG_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHOGEntryInfoPtr;

         FreeHOGEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionInfoList(Transaction_Info_t **ListHead)
{
   Transaction_Info_t *EntryToFree;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the Characteristic Value Handle field is the same as an*/
   /*            entry already in the list.  When this occurs, this     */
   /*            function returns NULL.                                 */
static HID_Report_Info_t *AddHIDReportInfo(HID_Report_Info_t **ListHead, HID_Report_Info_t *EntryToAdd)
{
   HID_Report_Info_t *AddedEntry = NULL;
   HID_Report_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CharacteristicValueHandle)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HID_Report_Info_t *)BTPS_AllocateMemory(sizeof(HID_Report_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                   = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextReportInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->CharacteristicValueHandle == AddedEntry->CharacteristicValueHandle)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeHIDReportInfoMemory(AddedEntry);
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
                     if(tmpEntry->NextReportInfoPtr)
                        tmpEntry = tmpEntry->NextReportInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextReportInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Report Info List for*/
   /* the specified Report Info based on the specified Characteristic   */
   /* Value Handle.  This function returns NULL if either the Report    */
   /* Info List Head is invalid, the Characteristic Value Handle is     */
   /* invalid, or the specified Entry was NOT present in the list.      */
static HID_Report_Info_t *SearchHIDReportInfo(HID_Report_Info_t **ListHead, Word_t CharacteristicValueHandle)
{
   HID_Report_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (CharacteristicValueHandle))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CharacteristicValueHandle != CharacteristicValueHandle))
         FoundEntry = FoundEntry->NextReportInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HID Report Info List*/
   /* for the specified Report Info based on the specified HID Report   */
   /* Information.  This function returns NULL if either the Report Info*/
   /* List Head is invalid, the HID Report Information is invalid, or   */
   /* the specified Entry was NOT present in the list.                  */
static HID_Report_Info_t *SearchHIDReportInfoByReportInfo(HID_Report_Info_t **ListHead, HOGM_HID_Report_Information_t *HIDReportInformation)
{
   HID_Report_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (HIDReportInformation))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (((FoundEntry->HIDReportInformation.ReportID != 0) && (FoundEntry->HIDReportInformation.ReportID != HIDReportInformation->ReportID)) || (FoundEntry->HIDReportInformation.ReportType != HIDReportInformation->ReportType)))
         FoundEntry = FoundEntry->NextReportInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Report Info List for*/
   /* the Report Info with the specified TransactionID and removes it   */
   /* from the List.  This function returns NULL if either the Report   */
   /* Info List Head is invalid, the Characteristic Value Handle is     */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHIDReportInfoMemory().                       */
static HID_Report_Info_t *DeleteHIDReportInfo(HID_Report_Info_t **ListHead, Word_t CharacteristicValueHandle)
{
   HID_Report_Info_t *FoundEntry = NULL;
   HID_Report_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (CharacteristicValueHandle))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CharacteristicValueHandle != CharacteristicValueHandle))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextReportInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextReportInfoPtr = FoundEntry->NextReportInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextReportInfoPtr;

         FoundEntry->NextReportInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Report Info member.  No check is*/
   /* done on this entry other than making sure it NOT NULL.            */
static void FreeHIDReportInfoMemory(HID_Report_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Report Info List.  Upon return of this   */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeHIDReportInfoList(HID_Report_Info_t **ListHead)
{
   HID_Report_Info_t *EntryToFree;
   HID_Report_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextReportInfoPtr;

         FreeHIDReportInfoMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
   {
      /* Make sure the transaction list is empty for this connection.   */
      if(EntryToFree->TransactionInfoList)
         FreeTransactionInfoList(&(EntryToFree->TransactionInfoList));

      /* Make sure the HID Report Info List is empty for this           */
      /* connection.                                                    */
      if(EntryToFree->ReportInfoList)
         FreeHIDReportInfoList(&(EntryToFree->ReportInfoList));

      /* Free any memory that may be allocated to hold the HID Report   */
      /* Descriptor.                                                    */
      if(EntryToFree->HIDInformation.ReportDescriptor)
         BTPS_FreeMemory(EntryToFree->HIDInformation.ReportDescriptor);

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HOG event to every registered HOG Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HOG Manager Lock */
   /*          held.  Upon exit from this function it will free the HOG */
   /*          Manager Lock.                                            */
static void DispatchHOGEvent(HOGM_Event_Data_t *HOGMEventData, BTPM_Message_t *Message)
{
   unsigned int      Index;
   unsigned int      Index1;
   unsigned int      ServerID;
   unsigned int      NumberCallbacks;
   Callback_Info_t   CallbackInfoArray[16];
   Callback_Info_t  *CallbackInfoArrayPtr;
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HOGEntryInfoList) || (HOGEntryInfoDataList)) && (HOGMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      HOGEntryInfo    = HOGEntryInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(HOGEntryInfo)
      {
         if(((HOGEntryInfo->EventCallback) || (HOGEntryInfo->ClientID != ServerID)) && (HOGEntryInfo->Flags & HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
      }

      /* We need to add the HOG Data Entry Information List as well.    */
      HOGEntryInfo = HOGEntryInfoDataList;
      while(HOGEntryInfo)
      {
         NumberCallbacks++;

         HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
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
            HOGEntryInfo    = HOGEntryInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(HOGEntryInfo)
            {
               if(((HOGEntryInfo->EventCallback) || (HOGEntryInfo->ClientID != ServerID)) && (HOGEntryInfo->Flags & HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HOGEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HOGEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HOGEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
            }

            /* We need to add the HOG Data Entry Information List as    */
            /* well.                                                    */
            HOGEntryInfo = HOGEntryInfoDataList;
            while(HOGEntryInfo)
            {
               CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HOGEntryInfo->ClientID;
               CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HOGEntryInfo->EventCallback;
               CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HOGEntryInfo->CallbackParameter;

               NumberCallbacks++;

               HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
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
               /*          for HOG events and Data Events.              */
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(HOGMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HOG Data event to the correct registered   */
   /* HOG Data Event Callback.                                          */
   /* * NOTE * This function should be called with the HOG Manager Lock */
   /*          held.  Upon exit from this function it will free the HOG */
   /*          Manager Lock.                                            */
static void DispatchHOGDataEvent(HOGM_Event_Data_t *HOGMEventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   HOG_Entry_Info_t      *HOGEntryInfo;
   HOGM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HOGMEventData) && (Message))
   {
      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if((HOGEntryInfo = HOGEntryInfoDataList) != NULL)
      {
         /* Format up the Data.                                         */
         if(HOGEntryInfo->ClientID != MSG_GetServerAddressID())
         {
            /* Dispatch a Message Callback.                             */

            /* Note the Client (destination) address.                   */
            Message->MessageHeader.AddressID = HOGEntryInfo->ClientID;

            /* All that is left to do is to dispatch the Event.         */
            MSG_SendMessage(Message);
         }
         else
         {
            /* Dispatch Local Event Callback.                           */
            if(HOGEntryInfo->EventCallback)
            {
               /* Note the Callback Information.                        */
               EventCallback     = HOGEntryInfo->EventCallback;
               CallbackParameter = HOGEntryInfo->CallbackParameter;

               /* Release the Lock because we have already noted the    */
               /* callback information.                                 */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  (*EventCallback)(HOGMEventData, CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }

               /* Re-acquire the Lock.                                  */
               DEVM_AcquireLock();
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HID Device Connection Event.                           */
static void DispatchHIDDeviceConnection(Connection_Entry_t *ConnectionEntry)
{
   HOGM_Event_Data_t                    HIDMEventData;
   HOGM_HID_Device_Connected_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Successfully loaded the HID Device Information from file so we */
      /* must have already configured this HID Device.  Go ahead and    */
      /* dispatch the connection event.                                 */
      if((Message = BTPS_AllocateMemory(HOGM_HID_DEVICE_CONNECTED_MESSAGE_SIZE(ConnectionEntry->HIDInformation.ReportDescriptorLength))) != NULL)
      {
         /* Make sure that we flag that this device is connected.       */
         ConnectionEntry->ConnectionState = csConnected;

         /* Next, format up the Event to dispatch.                      */
         HIDMEventData.EventType                                                 = hetHOGMHIDDeviceConnected;
         HIDMEventData.EventLength                                               = HOGM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE;

         HIDMEventData.EventData.DeviceConnectedEventData.RemoteDeviceAddress    = ConnectionEntry->BD_ADDR;
         HIDMEventData.EventData.DeviceConnectedEventData.SupportedFeatures      = ConnectionEntry->HIDInformation.SupportedFeatures;
         HIDMEventData.EventData.DeviceConnectedEventData.HIDInformation         = ConnectionEntry->HIDInformation.HIDDeviceInformation;
         HIDMEventData.EventData.DeviceConnectedEventData.ReportDescriptorLength = ConnectionEntry->HIDInformation.ReportDescriptorLength;
         HIDMEventData.EventData.DeviceConnectedEventData.ReportDescriptor       = ConnectionEntry->HIDInformation.ReportDescriptor;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(Message, 0, HOGM_HID_DEVICE_CONNECTED_MESSAGE_SIZE(0));

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         Message->MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED;
         Message->MessageHeader.MessageLength   = (HOGM_HID_DEVICE_CONNECTED_MESSAGE_SIZE(ConnectionEntry->HIDInformation.ReportDescriptorLength) - BTPM_MESSAGE_HEADER_SIZE);

         Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message->SupportedFeatures             = ConnectionEntry->HIDInformation.SupportedFeatures;
         Message->HIDInformation                = ConnectionEntry->HIDInformation.HIDDeviceInformation;
         Message->ReportDescriptorLength        = ConnectionEntry->HIDInformation.ReportDescriptorLength;

         BTPS_MemCopy(Message->ReportDescriptor, ConnectionEntry->HIDInformation.ReportDescriptor, ConnectionEntry->HIDInformation.ReportDescriptorLength);

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHOGEvent(&HIDMEventData, (BTPM_Message_t *)Message);

         /* Free the memory allocated for the message.                  */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HID Data Indication Event.                             */
static void DispatchHIDDataIndication(Connection_Entry_t *ConnectionEntry, HID_Report_Info_t *ReportInfo, unsigned int DataLength, Byte_t *Data)
{
   unsigned int                        Index;
   HOGM_Event_Data_t                   HIDMEventData;
   HOGM_HID_Data_Indication_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (ReportInfo) && (DataLength) && (Data))
   {
      /* Allocate a buffer to hold the Data Indication Event.           */
      if((Message = BTPS_AllocateMemory(HOGM_HID_DATA_INDICATION_MESSAGE_SIZE(DataLength+NON_ALIGNED_BYTE_SIZE))) != NULL)
      {
         /* Format up the Message to dispatch.                          */
         BTPS_MemInitialize(Message, 0, HOGM_HID_DATA_INDICATION_MESSAGE_SIZE(0));

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         Message->MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_DATA_INDICATION_EVENT;
         Message->MessageHeader.MessageLength   = (HOGM_HID_DATA_INDICATION_MESSAGE_SIZE(DataLength+(ReportInfo->HIDReportInformation.ReportID?NON_ALIGNED_BYTE_SIZE:0)) - BTPM_MESSAGE_HEADER_SIZE);

         Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message->ReportInformation             = ReportInfo->HIDReportInformation;
         Message->ReportDataLength              = DataLength;

         /* If necessary pre-pended the Report ID to the data.          */
         if(ReportInfo->HIDReportInformation.ReportID)
         {
            /* Save the Report ID at the beginning of the data.         */
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(Message->ReportData, ReportInfo->HIDReportInformation.ReportID);

            /* Increment the Report Data length by the size of the      */
            /* Report ID that was added to the data.                    */
            Message->ReportDataLength += NON_ALIGNED_BYTE_SIZE;

            /* Note that the Report ID was pre-pended to the data.      */
            Index = NON_ALIGNED_BYTE_SIZE;
         }
         else
            Index = 0;

         /* Copy the report data.                                       */
         BTPS_MemCopy(&(Message->ReportData[Index]), Data, DataLength);

         /* Next, format up the Event to dispatch.                      */
         HIDMEventData.EventType                                              = hetHOGMHIDDataIndication;
         HIDMEventData.EventLength                                            = HOGM_HID_DATA_INDICATION_EVENT_DATA_SIZE;

         HIDMEventData.EventData.DataIndicationEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HIDMEventData.EventData.DataIndicationEventData.ReportInformation   = ReportInfo->HIDReportInformation;
         HIDMEventData.EventData.DataIndicationEventData.ReportDataLength    = Message->ReportDataLength;
         HIDMEventData.EventData.DataIndicationEventData.ReportData          = Message->ReportData;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHOGDataEvent(&HIDMEventData, (BTPM_Message_t *)Message);

         /* Free the memory allocated for the message.                  */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HID Set Report Response event.                         */
static void DispatchHIDSetReportResponse(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *HIDReportInfo, unsigned int TransactionID, Byte_t ErrorCode)
{
   void                                   *CallbackParameter;
   HOGM_Event_Data_t                       HIDMEventData;
   HOGM_Event_Callback_t                   EventCallback;
   HOGM_HID_Set_Report_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (HIDReportInfo) && (TransactionID))
   {
      /* Check to see if this should be dispatched locally or to a      */
      /* client process.                                                */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         HIDMEventData.EventType                                                = hetHOGMHIDSetReportResponse;
         HIDMEventData.EventLength                                              = HOGM_HID_SET_REPORT_RESPONSE_EVENT_DATA_SIZE;

         HIDMEventData.EventData.SetReportResponseEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HIDMEventData.EventData.SetReportResponseEventData.TransactionID       = TransactionID;
         HIDMEventData.EventData.SetReportResponseEventData.Success             = (Boolean_t)(ErrorCode?FALSE:TRUE);
         HIDMEventData.EventData.SetReportResponseEventData.AttributeErrorCode  = ErrorCode;
         HIDMEventData.EventData.SetReportResponseEventData.ReportInformation   = HIDReportInfo->HIDReportInformation;

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&HIDMEventData, CallbackParameter);
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
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         Message.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT;
         Message.MessageHeader.MessageLength   = (HOGM_HID_SET_REPORT_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.TransactionID                 = TransactionID;
         Message.Success                       = (Boolean_t)(ErrorCode?FALSE:TRUE);
         Message.AttributeErrorCode            = ErrorCode;
         Message.ReportInformation             = HIDReportInfo->HIDReportInformation;

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a HID Get Report Response event.                         */
static void DispatchHIDGetReportResponse(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *HIDReportInfo, unsigned int TransactionID, Byte_t ErrorCode, unsigned int DataLength, Byte_t *Data)
{
   void                                   *CallbackParameter;
   Byte_t                                 *TempBuffer;
   HOGM_Event_Data_t                       HIDMEventData;
   HOGM_Event_Callback_t                   EventCallback;
   HOGM_HID_Get_Report_Response_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (HIDReportInfo) && (TransactionID))
   {
      /* If this is not an error event we need to check to see if the   */
      /* Report ID needs to be prepended to the data.                   */
      if(!ErrorCode)
      {
         /* We will check to see if the Report ID is valid and if so we */
         /* will append it to the data that we receive over the air.    */
         if((DataLength) && (Data) && (HIDReportInfo->HIDReportInformation.ReportID))
         {
            /* Allocate a buffer to hold the data plus the Report ID.   */
            if((TempBuffer = BTPS_AllocateMemory(DataLength+NON_ALIGNED_BYTE_SIZE)) != NULL)
            {
               /* Copy the data into the buffer.                        */
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(TempBuffer, HIDReportInfo->HIDReportInformation.ReportID);

               /* Copy the report data into the report buffer.          */
               BTPS_MemCopy(&(TempBuffer[NON_ALIGNED_BYTE_SIZE]), Data, DataLength);

               /* Increment the data length.                            */
               DataLength += NON_ALIGNED_BYTE_SIZE;
            }
         }
         else
         {
            if(DataLength)
               TempBuffer = Data;
            else
               TempBuffer = NULL;
         }
      }
      else
         TempBuffer = NULL;

      /* Only continue if everything is kosher.                         */
      if((ErrorCode) || ((!ErrorCode) && (((DataLength) && (TempBuffer)) || (!DataLength))))
      {
         /* Check to see if this should be dispatched locally or to a   */
         /* client process.                                             */
         if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Format the event to dispatch.                            */
            HIDMEventData.EventType                                                = hetHOGMHIDGetReportResponse;
            HIDMEventData.EventLength                                              = HOGM_HID_GET_REPORT_RESPONSE_EVENT_DATA_SIZE;

            HIDMEventData.EventData.GetReportResponseEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
            HIDMEventData.EventData.GetReportResponseEventData.TransactionID       = TransactionID;
            HIDMEventData.EventData.GetReportResponseEventData.ReportInformation   = HIDReportInfo->HIDReportInformation;

            if(!ErrorCode)
            {
               HIDMEventData.EventData.GetReportResponseEventData.Success            = TRUE;
               HIDMEventData.EventData.GetReportResponseEventData.AttributeErrorCode = 0;
               HIDMEventData.EventData.GetReportResponseEventData.ReportDataLength   = DataLength;
               HIDMEventData.EventData.GetReportResponseEventData.ReportData         = TempBuffer;
            }
            else
            {
               HIDMEventData.EventData.GetReportResponseEventData.Success            = FALSE;
               HIDMEventData.EventData.GetReportResponseEventData.AttributeErrorCode = ErrorCode;
               HIDMEventData.EventData.GetReportResponseEventData.ReportDataLength   = 0;
               HIDMEventData.EventData.GetReportResponseEventData.ReportData         = NULL;
            }

            /* Save the Event Callback Information.                     */
            EventCallback     = EventCallbackInfo->EventCallback;
            CallbackParameter = EventCallbackInfo->CallbackParameter;

            /* Release the Lock so we can make the callback.            */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&HIDMEventData, CallbackParameter);
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
            /* Allocate memory for the message.                         */
            if((Message = BTPS_AllocateMemory(HOGM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(DataLength))) != NULL)
            {
               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(Message, 0, HOGM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(0));

               Message->MessageHeader.AddressID       = EventCallbackInfo->ClientID;
               Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
               Message->MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT;
               Message->MessageHeader.MessageLength   = (HOGM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(DataLength) - BTPM_MESSAGE_HEADER_SIZE);

               Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
               Message->TransactionID                 = TransactionID;
               Message->ReportInformation             = HIDReportInfo->HIDReportInformation;

               /* Format the rest of the message based on whether it was*/
               /* successful.                                           */
               if(!ErrorCode)
               {
                  Message->Success                    = TRUE;
                  Message->AttributeErrorCode         = 0;
                  Message->ReportDataLength           = DataLength;

                  if(DataLength)
                     BTPS_MemCopy(Message->ReportData, TempBuffer, DataLength);
               }
               else
               {
                  Message->Success                    = FALSE;
                  Message->AttributeErrorCode         = ErrorCode;
                  Message->ReportDataLength           = 0;
               }

               /* Go ahead and send the message to the client.          */
               MSG_SendMessage((BTPM_Message_t *)Message);

               /* Free the memory allocated for this message.           */
               BTPS_FreeMemory(Message);
            }
         }

         /* If the temp buffer was allocated then go ahead and free it. */
         if((!ErrorCode) && (TempBuffer) && (TempBuffer != Data))
            BTPS_FreeMemory(TempBuffer);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the HID Report Descriptors stored for a specified device.  */
static void ReloadHIDReportDescriptors(Connection_Entry_t *ConnectionEntry)
{
   char           KeyName[HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH+1];
   Byte_t        *ReportDescriptor;
   Boolean_t      Done;
   unsigned int   LineNumber;
   unsigned int   ReportDescriptorLength;
   unsigned int   Result;
   unsigned int   MemorySize;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* First lets determine how many lines are present and how many   */
      /* descriptor bytes are present.                                  */
      Done                   = FALSE;
      LineNumber             = 0;
      ReportDescriptorLength = 0;
      while(!Done)
      {
         /* Build the Key Name.                                         */
         sprintf(KeyName, HOGM_LE_KEY_NAME_REPORT_DESCRIPTOR_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0, LineNumber);

         /* Check to see if any Report Descriptor data is stored for    */
         /* this key.                                                   */
         if((Result = SET_ReadBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME)) > 0)
         {
            /* Increment the Report Descriptor Length.                  */
            ReportDescriptorLength += Result;
         }
         else
            Done = TRUE;

         /* Increment the Line Number.                                  */
         ++LineNumber;
      }

      /* If there is any report descriptor data, allocate a buffer to   */
      /* hold the data.                                                 */
      if(ReportDescriptorLength)
      {
         /* Allocate a buffer to hold all of the Report Descriptor Data.*/
         if((ReportDescriptor = BTPS_AllocateMemory(ReportDescriptorLength)) != NULL)
         {
            /* Loop through and read all of the Report Descriptor data. */
            Done                   = FALSE;
            LineNumber             = 0;
            MemorySize             = ReportDescriptorLength;
            ReportDescriptorLength = 0;
            while(!Done)
            {
               /* Build the Key Name.                                   */
               sprintf(KeyName, HOGM_LE_KEY_NAME_REPORT_DESCRIPTOR_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0, LineNumber);

               /* Check to see if any Report Descriptor data is stored  */
               /* for this key.                                         */
               if((Result = SET_ReadBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, &ReportDescriptor[ReportDescriptorLength], (MemorySize-ReportDescriptorLength), BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME)) > 0)
               {
                  /* Increment the Report Descriptor Length.            */
                  ReportDescriptorLength += Result;
               }
               else
                  Done = TRUE;

               /* Increment the Line Number.                            */
               ++LineNumber;
            }

            ConnectionEntry->HIDInformation.ReportDescriptorLength = ReportDescriptorLength;
            ConnectionEntry->HIDInformation.ReportDescriptor       = ReportDescriptor;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the HID Report Information stored for a specified device.  */
static void ReloadHIDReportInformation(Connection_Entry_t *ConnectionEntry)
{
   char               KeyName[HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH+5];
   Byte_t             ReportType;
   Boolean_t          Done;
   unsigned int       LineNumber;
   HID_Report_Info_t  ReportInfo;
   HID_Report_Entry_t ReportInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Loop through and reload all stored Report Information          */
      /* structures.                                                    */
      Done       = FALSE;
      LineNumber = 0;
      while(!Done)
      {
         /* Build the Key Name.                                         */
         sprintf(KeyName, HOGM_LE_KEY_NAME_REPORT_INFORMATION_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0, LineNumber);

         /* Check to see if the HID Information is present for this     */
         /* device.                                                     */
         if(SET_ReadBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (Byte_t *)&ReportInformation, HID_REPORT_ENTRY_DATA_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == HID_REPORT_ENTRY_DATA_SIZE)
         {
            /* Configure the structure that we will add to the report   */
            /* list.                                                    */
            BTPS_MemInitialize(&ReportInfo, 0, sizeof(HID_Report_Info_t));

            ReportInfo.CharacteristicValueHandle        = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(ReportInformation.CharacteristicValueHandle));
            ReportInfo.CharacterisicConfigurationHandle = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(ReportInformation.CCCD));
            ReportType                                  = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(ReportInformation.ReportType));
            ReportInfo.HIDReportInformation.ReportID    = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(ReportInformation.ReportID));
            ReportInfo.ReportProperties                 = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(ReportInformation.ReportProperties));

            /* Convert the ReportType to an enumerated value.           */
            switch(ReportType)
            {
               case HOGM_HID_REPORT_TYPE_INPUT_REPORT:
                  ReportInfo.HIDReportInformation.ReportType = hrtInput;
                  break;
               case HOGM_HID_REPORT_TYPE_OUTPUT_REPORT:
                  ReportInfo.HIDReportInformation.ReportType = hrtOutput;
                  break;
               case HOGM_HID_REPORT_TYPE_FEATURE_REPORT:
                  ReportInfo.HIDReportInformation.ReportType = hrtFeature;
                  break;
               case HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT:
                  ReportInfo.HIDReportInformation.ReportType = hrtBootKeyboardInput;
                  break;
               case HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_OUTPUT_REPORT:
                  ReportInfo.HIDReportInformation.ReportType = hrtBootKeyboardOutput;
                  break;
               default:
               case HOGM_HID_REPORT_TYPE_BOOT_MOUSE_INPUT_REPORT:
                  ReportInfo.HIDReportInformation.ReportType = hrtBootMouseInput;
                  break;
            }

            /* Add the Report Info to the list for this connection.     */
            AddHIDReportInfo(&(ConnectionEntry->ReportInfoList), &ReportInfo);
         }
         else
            Done = TRUE;

         /* Increment the line number.                                  */
         ++LineNumber;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the HID Information for the specified device.  This device */
   /* will load the HID Information into ConnectionEntry (if specified) */
   /* and return TRUE if the HID Information was present for the device.*/
static Boolean_t ReloadHIDInformation(Connection_Entry_t *ConnectionEntry, BD_ADDR_t RemoteDeviceAddress)
{
   char                    KeyName[HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Boolean_t               ret_val = FALSE;
   HID_Information_Entry_t HIDInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Build the Key Name.                                            */
      sprintf(KeyName, HOGM_LE_KEY_NAME_HID_INFORMATION_PREFIX, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0);

      /* Check to see if the HID Information is present for this device.*/
      if(SET_ReadBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (Byte_t *)&HIDInformation, HID_INFORMATION_ENTRY_DATA_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == HID_INFORMATION_ENTRY_DATA_SIZE)
      {
         /* Return TRUE since the HID Information key is present for    */
         /* this device.                                                */
         ret_val = TRUE;

         /* Check to see if we need to actually reload the HID          */
         /* Information.                                                */
         if(ConnectionEntry)
         {
            /* Reload the HID Information.                              */
            ConnectionEntry->HIDInformation.SupportedFeatures                    = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(&(HIDInformation.SupportedFeatures));
            ConnectionEntry->HIDInformation.HIDDeviceInformation.HIDVersion      = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(HIDInformation.HIDVersion));
            ConnectionEntry->HIDInformation.HIDDeviceInformation.CountryCode     = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(HIDInformation.CountryCode));
            ConnectionEntry->HIDInformation.HIDDeviceInformation.ProductVersion  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(HIDInformation.ProductVersion));
            ConnectionEntry->HIDInformation.HIDDeviceInformation.ProductID       = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(HIDInformation.ProductID));
            ConnectionEntry->HIDInformation.HIDDeviceInformation.VendorID_Source = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(HIDInformation.VendorID_Source));
            ConnectionEntry->HIDInformation.HIDDeviceInformation.VendorID        = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(HIDInformation.VendorID));
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that attempts to load*/
   /* a HID Device from the LE configuration file.                      */
static Connection_Entry_t *LoadHIDDeviceFromFile(BD_ADDR_t RemoteDeviceAddress)
{
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Verify that the correct services are supported by this to be a */
      /* potential HID Device.                                          */
      if(CheckHIDServicesSupported(RemoteDeviceAddress))
      {
         /* First lets verify that the HID Information is present in the*/
         /* configuration file.                                         */
         if(ReloadHIDInformation(NULL, RemoteDeviceAddress))
         {
            /* HID Information is present so lets go ahead and create a */
            /* connection entry and add it to the list.                 */
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

            ConnectionEntry.ConnectionState     = csConnected;
            ConnectionEntry.CurrentProtocolMode = hpmReport;
            ConnectionEntry.BD_ADDR             = RemoteDeviceAddress;

            /* Add the Connection Info to the Connection List.          */
            if((ret_val = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
            {
               /* Attempt to reload the HID Information.                */
               if(ReloadHIDInformation(ret_val, ret_val->BD_ADDR))
               {
                  /* Attempt to reload any reports.                     */
                  ReloadHIDReportInformation(ret_val);

                  /* Attempt to reload the Report Descriptor.           */
                  ReloadHIDReportDescriptors(ret_val);
               }
               else
               {
                  /* Failed to reload the HID Information so just delete*/
                  /* it from the list.                                  */
                  if((ret_val = DeleteConnectionEntry(&ConnectionEntryList, ret_val->BD_ADDR)) != NULL)
                  {
                     FreeConnectionEntryMemory(ret_val);

                     ret_val = NULL;
                  }
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* store/delete HID Report Descriptors for the specified device to   */
   /* file.                                                             */
static void StoreHIDReportDescriptors(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char           KeyName[HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Byte_t        *ReportData;
   Word_t         LineNumber;
   Boolean_t      Done;
   unsigned int   ReportDataLength;
   unsigned int   CopyLength;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Loop and store the correct number of keys needed to store the  */
      /* entire report descriptor.                                      */
      Done             = FALSE;
      LineNumber       = 0;
      ReportDataLength = ConnectionEntry->HIDInformation.ReportDescriptorLength;
      ReportData       = ConnectionEntry->HIDInformation.ReportDescriptor;
      while(!Done)
      {
         /* Build the Key Name.                                         */
         sprintf(KeyName, HOGM_LE_KEY_NAME_REPORT_DESCRIPTOR_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0, LineNumber);

         /* Check to see if we are store or deleting the Report         */
         /* Descriptor from the configuration file.                     */
         if(Store)
         {
            /* Only continue if we have more data to write.             */
            if((ReportDataLength) && (ReportData))
            {
               /* Check to see if the line needs to be truncated.       */
               CopyLength = (ReportDataLength > HOGM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH)?HOGM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH:ReportDataLength;

               /* Write the requested line.                             */
               if(SET_WriteBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, ReportData, CopyLength, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) > 0)
               {
                  /* Adjust the counts.                                 */
                  ReportDataLength -= CopyLength;
                  ReportData       += CopyLength;
               }
               else
                  Done = TRUE;
            }
            else
               Done = TRUE;
         }
         else
         {
            /* Check to see if any Report Descriptor data is stored for */
            /* this key.                                                */
            if(SET_ReadBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) > 0)
            {
               /* Delete this Report Descriptor Line.                   */
               SET_WriteBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            }
            else
               Done = TRUE;
         }

         /* Increment the Key Name Line Number.                         */
         LineNumber++;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that exists to       */
   /* store/delete HID Information for the specified device to file.    */
static void StoreHIDInformation(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char                    KeyName[HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   HID_Information_Entry_t HIDInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Build the Key Name.                                            */
      sprintf(KeyName, HOGM_LE_KEY_NAME_HID_INFORMATION_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

      /* See if we are deleting or storing the HID Information.         */
      if(Store)
      {
         /* Format the HID Information.                                 */
         ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(HIDInformation.SupportedFeatures), ConnectionEntry->HIDInformation.SupportedFeatures);
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(HIDInformation.HIDVersion), ConnectionEntry->HIDInformation.HIDDeviceInformation.HIDVersion);
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(HIDInformation.CountryCode), ConnectionEntry->HIDInformation.HIDDeviceInformation.CountryCode);
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(HIDInformation.VendorID_Source), ConnectionEntry->HIDInformation.HIDDeviceInformation.VendorID_Source);
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(HIDInformation.VendorID), ConnectionEntry->HIDInformation.HIDDeviceInformation.VendorID);
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(HIDInformation.ProductID), ConnectionEntry->HIDInformation.HIDDeviceInformation.ProductID);
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(HIDInformation.ProductVersion), ConnectionEntry->HIDInformation.HIDDeviceInformation.ProductVersion);

         /* Write the Key.                                              */
         SET_WriteBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (Byte_t *)&HIDInformation, HID_INFORMATION_ENTRY_DATA_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
      else
      {
         /* Delete this Report Descriptor Line.                         */
         SET_WriteBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* store/delete HID Report Information for the specified device to   */
   /* file.                                                             */
static void StoreHIDReportInformation(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char                KeyName[HOGM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Byte_t              ReportType;
   Word_t              LineNumber;
   Boolean_t           Done;
   HID_Report_Info_t  *ReportInfo;
   HID_Report_Entry_t  ReportInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Loop and store the correct number of keys needed to store the  */
      /* entire HID Report Information list.                            */
      Done       = FALSE;
      LineNumber = 0;
      ReportInfo = ConnectionEntry->ReportInfoList;
      while(!Done)
      {
         /* Build the Key Name.                                         */
         sprintf(KeyName, HOGM_LE_KEY_NAME_REPORT_INFORMATION_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0, LineNumber);

         /* Check to see if we are store or deleting the Report         */
         /* Descriptor from the configuration file.                     */
         if(Store)
         {
            /* Verify that we have anything to write to the line.       */
            if(ReportInfo)
            {
               /* Format a report type that we can save to file.        */
               switch(ReportInfo->HIDReportInformation.ReportType)
               {
                  default:
                  case hrtInput:
                     ReportType = HOGM_HID_REPORT_TYPE_INPUT_REPORT;
                     break;
                  case hrtOutput:
                     ReportType = HOGM_HID_REPORT_TYPE_OUTPUT_REPORT;
                     break;
                  case hrtFeature:
                     ReportType = HOGM_HID_REPORT_TYPE_FEATURE_REPORT;
                     break;
                  case hrtBootKeyboardInput:
                     ReportType = HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT;
                     break;
                  case hrtBootKeyboardOutput:
                     ReportType = HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_OUTPUT_REPORT;
                     break;
                  case hrtBootMouseInput:
                     ReportType = HOGM_HID_REPORT_TYPE_BOOT_MOUSE_INPUT_REPORT;
                     break;
               }

               /* Format the entry.                                     */
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(ReportInformation.CharacteristicValueHandle), ReportInfo->CharacteristicValueHandle);
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(ReportInformation.CCCD), ReportInfo->CharacterisicConfigurationHandle);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(ReportInformation.ReportType), ReportType);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(ReportInformation.ReportID), ReportInfo->HIDReportInformation.ReportID);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(ReportInformation.ReportProperties), ReportInfo->ReportProperties);

               /* Write the Key.                                        */
               SET_WriteBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (Byte_t *)&ReportInformation, HID_REPORT_ENTRY_DATA_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);

               /* Move to the next report to write to the line.         */
               ReportInfo = ReportInfo->NextReportInfoPtr;
            }
            else
               Done = TRUE;
         }
         else
         {
            /* Check to see if any Report Information data is stored for*/
            /* this key.                                                */
            if(SET_ReadBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) > 0)
            {
               /* Delete this Report Information Line.                  */
               SET_WriteBinaryData(HOGM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
            }
            else
               Done = TRUE;
         }

         /* Increment the Line Number since we cannot put any more      */
         /* entries on this line.                                       */
         ++LineNumber;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to store */
   /* a HID Device to the LE Configuration File.                        */
static void StoreHIDDeviceToFile(Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Store/Delete the HID Information.                              */
      StoreHIDInformation(ConnectionEntry, Store);

      /* Store/Delete the HID Report Information.                       */
      StoreHIDReportInformation(ConnectionEntry, Store);

      /* First attempt to store/delete the report descriptors.          */
      StoreHIDReportDescriptors(ConnectionEntry, Store);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* determine if a device supports the necessary services to support  */
   /* the HID Device role.                                              */
static Boolean_t CheckHIDServicesSupported(BD_ADDR_t BD_ADDR)
{
   UUID_16_t        UUID;
   Boolean_t        ret_val = FALSE;
   SDP_UUID_Entry_t ServiceUUID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see that the Battery Service is supported.               */
   ServiceUUID.SDP_Data_Element_Type = deUUID_16;
   BAS_ASSIGN_BAS_SERVICE_UUID_16(&UUID);
   CONVERT_BLUETOOTH_UUID_16_TO_SDP_UUID_16(ServiceUUID.UUID_Value.UUID_16, UUID);

   if(DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR, ServiceUUID) > 0)
   {
      /* Check to see if the HID Service is supported.                  */
      ServiceUUID.SDP_Data_Element_Type = deUUID_16;
      HIDS_ASSIGN_HIDS_SERVICE_UUID_16(&UUID);
      CONVERT_BLUETOOTH_UUID_16_TO_SDP_UUID_16(ServiceUUID.UUID_Value.UUID_16, UUID);

      if(DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR, ServiceUUID) > 0)
      {
         /* Flag that this device supports the HID Device Role.         */
         ret_val = TRUE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to set  */
   /* the protocol mode for the specified connection.                   */
static int HIDSetProtocolMode(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HIDS_Protocol_Mode_t ProtocolMode)
{
   int    ret_val;
   Byte_t Buffer[HIDS_PROTOCOL_MODE_VALUE_LENGTH];

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Verify that the protocol mode is supported.                    */
      if(ConnectionEntry->HIDInformation.ProtocolModeCharacteristicHandle)
      {
         /* Format the Protocol Mode message.                           */
         if(!HIDS_Format_Protocol_Mode(pmBoot, sizeof(Buffer), Buffer))
         {
            /* Write the Protocol Mode characteristic.                  */
            ret_val = GATM_WriteValueWithoutResponse(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->HIDInformation.ProtocolModeCharacteristicHandle, FALSE, HIDS_PROTOCOL_MODE_VALUE_LENGTH, Buffer);
            if(ret_val > 0)
            {
               /* Set the current protocol mode.                        */
               ConnectionEntry->CurrentProtocolMode = ProtocolMode;

               /* Return success to the caller.                         */
               ret_val                               = 0;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_FEATURE_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* perform a Set Report Request for the specified HID Report.        */
static int HIDGetReportRequest(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *ReportInfo)
{
   int                 ret_val;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (ReportInfo))
   {
      /* Verify that the Set Report Feature is supported.               */
      if(ReportInfo->ReportProperties & GATT_CHARACTERISTIC_PROPERTIES_READ)
      {
         /* Configure the Transaction Information for this transaction. */
         BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

         TransactionInfo.TransactionID             = GetNextTransactionID();
         TransactionInfo.HOGManagerDataCallbackID  = EventCallbackInfo->CallbackID;
         TransactionInfo.CharacteristicValueHandle = ReportInfo->CharacteristicValueHandle;
         TransactionInfo.TransactionType           = ttGetReport;

         /* Add the Transaction Info to the transaction list.           */
         if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
         {
            /* Write the Protocol Mode characteristic.                  */
            ret_val = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, TransactionInfoPtr->CharacteristicValueHandle, 0, TRUE);
            if(ret_val > 0)
            {
               /* Save the GATM Transaction ID.                         */
               TransactionInfoPtr->GATMTransactionID = (unsigned int)ret_val;

               /* Return the local Transaction ID.                      */
               ret_val                               = (int)TransactionInfoPtr->TransactionID;
            }
            else
            {
               /* Delete the transaction information memory and free the*/
               /* memory.                                               */
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
         ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_REQUEST_NOT_VALID;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* perform a Set Report Request for the specified HID Report.        */
static int HIDSetReportRequest(Connection_Entry_t *ConnectionEntry, HOG_Entry_Info_t *EventCallbackInfo, HID_Report_Info_t *ReportInfo, Boolean_t ResponseExpected, unsigned int DataLength, Byte_t *Data)
{
   int                 ret_val;
   Byte_t              ReportID;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (ReportInfo) && (DataLength) && (Data))
   {
      /* Verify that the Set Report Feature is supported.               */
      if(((!ResponseExpected) && (ReportInfo->ReportProperties & GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE)) || ((ResponseExpected) && (ReportInfo->ReportProperties & GATT_CHARACTERISTIC_PROPERTIES_WRITE)))
      {
         /* Check to see if the Report ID is valid for the Report that  */
         /* this data is being written to.                              */
         if(ReportInfo->HIDReportInformation.ReportID)
         {
            /* The Report ID is valid for this report.  Therefore we    */
            /* state in the API that the Report ID MUST be pre-pended to*/
            /* the Report Data that is passed into this function.       */
            /* However we will check to see if the first byte of the    */
            /* Report Data matches the Report ID of this report.  If it */
            /* doesn't match we will assume that the Report ID is not   */
            /* pre-pended and just pass it through.                     */
            ReportID = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Data);
            if(ReportID == ReportInfo->HIDReportInformation.ReportID)
            {
               /* The Report ID is present in the Report data so just   */
               /* remove the Report ID from the data that we will write.*/
               DataLength -= NON_ALIGNED_BYTE_SIZE;
               Data       += NON_ALIGNED_BYTE_SIZE;
            }
         }

         /* Configure the Transaction Information for this transaction. */
         BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

         TransactionInfo.TransactionID             = GetNextTransactionID();
         TransactionInfo.HOGManagerDataCallbackID  = EventCallbackInfo->CallbackID;
         TransactionInfo.CharacteristicValueHandle = ReportInfo->CharacteristicValueHandle;
         TransactionInfo.TransactionType           = ttSetReport;

         /* Add the Transaction Info to the transaction list.           */
         if((!ResponseExpected) || ((ResponseExpected) && ((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)))
         {
            /* Write the Report characteristic using the correct        */
            /* procedure.                                               */
            if(ResponseExpected)
               ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ReportInfo->CharacteristicValueHandle, DataLength, Data);
            else
               ret_val = GATM_WriteValueWithoutResponse(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ReportInfo->CharacteristicValueHandle, FALSE, DataLength, Data);

            if(ret_val > 0)
            {
               /* Check to see if a Response is expected and return the */
               /* correct value to indicate success.                    */
               if(ResponseExpected)
               {
                  /* Save the GATM Transaction ID.                      */
                  TransactionInfoPtr->GATMTransactionID = (unsigned int)ret_val;

                  /* Return the local Transaction ID.                   */
                  ret_val                               = (int)TransactionInfoPtr->TransactionID;
               }
               else
               {
                  /* If no response is expected simply return 0 to      */
                  /* indicate success.                                  */
                  ret_val                               = 0;
               }
            }
            else
            {
               /* Delete the transaction information memory and free the*/
               /* memory.                                               */
               if(ResponseExpected)
               {
                  if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
                  {
                     BTPS_FreeMemory(TransactionInfoPtr);

                     TransactionInfoPtr = NULL;
                  }
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
      }
      else
         ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_REQUEST_NOT_VALID;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* perform a Set Suspend Mode procedure to the specified remote      */
   /* device.                                                           */
static int HIDSetSuspendMode(Connection_Entry_t *ConnectionEntry, Boolean_t Suspend)
{
   int                          ret_val;
   Byte_t                       Buffer[HIDS_CONTROL_POINT_VALUE_LENGTH];
   HIDS_Control_Point_Command_t Command;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Verify that the device supports the feature.                   */
      if(ConnectionEntry->HIDInformation.ControlPointCharacteristicHandle)
      {
         /* Format the Set Suspend Mode command.                        */
         Command = (Suspend?pcSuspend:pcExitSuspend);

         if(!HIDS_Format_Control_Point_Command(Command, sizeof(Buffer), Buffer))
         {
            /* Perform the Write Without Response procedure.            */
            ret_val = GATM_WriteValueWithoutResponse(GATMEventCallbackID, ConnectionEntry->BD_ADDR, ConnectionEntry->HIDInformation.ControlPointCharacteristicHandle, FALSE, sizeof(Buffer), Buffer);
            if(ret_val > 0)
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_FEATURE_NOT_SUPPORTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process the HID Protocol Mode Characteristic.                     */
static Boolean_t ProcessHIDProtocolModeCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Boolean_t ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Save the Protocol Mode Characteristic Handle.                  */
      ConnectionEntry->HIDInformation.ProtocolModeCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;

      /* If Report Mode is not supported go ahead and enter into Boot   */
      /* Mode.                                                          */
      if(!(SupportedFeatures & HOGM_SUPPORTED_FEATURES_FLAGS_REPORT_MODE))
      {
         /* Report Mode is not supported locally so enter boot mode.    */
         if(!HIDSetProtocolMode(ConnectionEntry, NULL, pmBoot))
            ret_val = TRUE;
         else
            ret_val = FALSE;
      }
      else
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process the HID Report Map Characteristic.                        */
static Boolean_t ProcessHIDReportMapCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   int                 Result;
   Boolean_t           ret_val = FALSE;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

      TransactionInfo.TransactionID             = GetNextTransactionID();
      TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
      TransactionInfo.TransactionType           = ttReadReportMap;

      /* Add the Transaction Info to the transaction list.              */
      if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
      {
         /* Attempt to read the entire Report Map characteristic.       */
         Result = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CharacteristicInformation->Characteristic_Handle, 0, TRUE);
         if(Result > 0)
         {
            /* Save the GATM Transaction ID.                            */
            TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;

            /* Return success to the caller.                            */
            ret_val                               = TRUE;
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a HID Report Characteristic.                              */
static Boolean_t ProcessHIDReportCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation, unsigned int ReportType)
{
   int                 Result;
   Word_t              CCCD;
   Word_t              RRD;
   Boolean_t           ret_val = FALSE;
   Boolean_t           AddReportInfo;
   unsigned int        Index;
   NonAlignedWord_t    CCCDValue;
   HID_Report_Info_t   HIDReportInfo;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Flag that the Report Info doesn't need to be added just yet.   */
      AddReportInfo = FALSE;

      /* Determine the CCCD and RRD Handles for the Report (if any).    */
      for(Index=0,CCCD=0,RRD=0;Index<CharacteristicInformation->NumberOfDescriptors;Index++)
      {
         if(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
               CCCD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
            else
            {
               if(HIDS_COMPARE_REPORT_REFERENCE_DESCRIPTOR_UUID_TO_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
                  RRD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
            }
         }
      }

      /* Determine what type of report this is.                         */
      switch(ReportType)
      {
         case 0:
            /* Generic report, process.                                 */
            if(SupportedFeatures & HOGM_SUPPORTED_FEATURES_FLAGS_REPORT_MODE)
            {
               /* Verify that a Report Reference Descriptor is present. */
               if(RRD)
               {
                  /* Configure the HID Report Info structure.           */
                  BTPS_MemInitialize(&HIDReportInfo, 0, sizeof(HIDReportInfo));

                  HIDReportInfo.CharacteristicValueHandle        = CharacteristicInformation->Characteristic_Handle;
                  HIDReportInfo.CharacterisicConfigurationHandle = CCCD;
                  HIDReportInfo.ReportProperties                 = CharacteristicInformation->Characteristic_Properties;

                  /* Add the Report Info to the List.                   */
                  AddReportInfo = TRUE;

                  /* Configure the Transaction Information for this     */
                  /* transaction.                                       */
                  BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

                  TransactionInfo.TransactionID             = GetNextTransactionID();
                  TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
                  TransactionInfo.TransactionType           = ttReadReportReference;

                  /* Add the Transaction Info to the transaction list.  */
                  if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
                  {
                     /* Attempt to read the Report Reference Descriptor.*/
                     Result = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, RRD, 0, TRUE);
                     if(Result > 0)
                     {
                        /* Save the GATM Transaction ID.                */
                        TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;

                        /* If A CCCD was found then go ahead and        */
                        /* configure it.                                */
                        if(CCCD)
                        {
                           /* Configure the Transaction Information for */
                           /* this transaction.                         */
                           BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

                           TransactionInfo.TransactionID             = GetNextTransactionID();
                           TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
                           TransactionInfo.TransactionType           = ttConfigureCCCD;

                           /* Add the Transaction Info to the           */
                           /* transaction list.                         */
                           if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
                           {
                              /* Configure the CCCD for notifications.  */
                              ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

                              /* Perform the write to configure the     */
                              /* CCCD.                                  */
                              Result = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue);
                              if(Result > 0)
                              {
                                 /* Save the GATM Transaction ID.       */
                                 TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;
                              }
                              else
                              {
                                 /* Error occurred so don't add report  */
                                 /* to list and return an error.        */
                                 AddReportInfo = FALSE;

                                 /* Delete the transaction information  */
                                 /* memory and free the memory.         */
                                 if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
                                 {
                                    BTPS_FreeMemory(TransactionInfoPtr);

                                    TransactionInfoPtr = NULL;
                                 }
                              }
                           }
                           else
                           {
                              /* Error occurred so don't add report to  */
                              /* list and return an error.              */
                              AddReportInfo = FALSE;
                           }
                        }
                     }
                     else
                     {
                        /* Error occurred so don't add report to list   */
                        /* and return an error.                         */
                        AddReportInfo = FALSE;

                        /* Delete the transaction information memory and*/
                        /* free the memory.                             */
                        if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
                        {
                           BTPS_FreeMemory(TransactionInfoPtr);

                           TransactionInfoPtr = NULL;
                        }
                     }
                  }
                  else
                  {
                     /* Error occurred so don't add report to list and  */
                     /* return an error.                                */
                     AddReportInfo = FALSE;
                  }
               }
            }
            else
               ret_val = TRUE;
            break;
         case HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT:
         case HOGM_HID_REPORT_TYPE_BOOT_MOUSE_INPUT_REPORT:
            /* Verify that we found the required CCCD.                  */
            if(CCCD)
            {
               /* Flag that Boot Mode is supported.                     */
               if(ReportType == HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT)
                  ConnectionEntry->HIDInformation.SupportedFeatures |= HOGM_SUPPORTED_FEATURES_FLAGS_BOOT_KEYBOARD;
               else
                  ConnectionEntry->HIDInformation.SupportedFeatures |= HOGM_SUPPORTED_FEATURES_FLAGS_BOOT_MOUSE;

               /* Configure the HID Report Info structure.              */
               BTPS_MemInitialize(&HIDReportInfo, 0, sizeof(HIDReportInfo));

               HIDReportInfo.CharacteristicValueHandle        = CharacteristicInformation->Characteristic_Handle;
               HIDReportInfo.CharacterisicConfigurationHandle = CCCD;
               HIDReportInfo.HIDReportInformation.ReportType  = (ReportType == HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT)?hrtBootKeyboardInput:hrtBootMouseInput;
               HIDReportInfo.ReportProperties                 = CharacteristicInformation->Characteristic_Properties;

               /* Add the Report Info to the List.                      */
               AddReportInfo = TRUE;

               /* Configure the Transaction Information for this        */
               /* transaction.                                          */
               BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

               TransactionInfo.TransactionID             = GetNextTransactionID();
               TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
               TransactionInfo.TransactionType           = ttConfigureCCCD;

               /* Add the Transaction Info to the Transaction           */
               /* Information list for this connection.                 */
               if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
               {
                  /* Configure the CCCD for notifications.              */
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

                  /* Perform the write to configure the CCCD.           */
                  Result = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue);
                  if(Result > 0)
                  {
                     /* Save the GATM Transaction ID for this write     */
                     /* request.                                        */
                     TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;
                  }
                  else
                  {
                     /* Error occurred so don't add report to list and  */
                     /* return an error.                                */
                     AddReportInfo = FALSE;

                     /* Delete the transaction information memory and   */
                     /* free the memory.                                */
                     if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->TransactionID)) != NULL)
                     {
                        BTPS_FreeMemory(TransactionInfoPtr);

                        TransactionInfoPtr = NULL;
                     }
                  }
               }
               else
               {
                  /* Error occurred so don't add report to list and     */
                  /* return an error.                                   */
                  AddReportInfo = FALSE;
               }
            }
            break;
         case HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_OUTPUT_REPORT:
            /* Configure the HID Report Info structure.                 */
            BTPS_MemInitialize(&HIDReportInfo, 0, sizeof(HIDReportInfo));

            HIDReportInfo.CharacteristicValueHandle        = CharacteristicInformation->Characteristic_Handle;
            HIDReportInfo.HIDReportInformation.ReportType  = hrtBootKeyboardOutput;
            HIDReportInfo.ReportProperties                 = CharacteristicInformation->Characteristic_Properties;

            /* Flag that Boot Mode is supported.                        */
            ConnectionEntry->HIDInformation.SupportedFeatures |= HOGM_SUPPORTED_FEATURES_FLAGS_BOOT_KEYBOARD;

            /* Add the Report Info to the List.                         */
            AddReportInfo = TRUE;
            break;
      }

      /* Add the HID Report Info structure to the list if requested.    */
      if(AddReportInfo)
      {
         if(AddHIDReportInfo(&(ConnectionEntry->ReportInfoList), &HIDReportInfo))
            ret_val = TRUE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a HID Information Characteristic.                         */
static Boolean_t ProcessHIDInformationCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   int                 Result;
   Boolean_t           ret_val = FALSE;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

      TransactionInfo.TransactionID             = GetNextTransactionID();
      TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
      TransactionInfo.TransactionType           = ttReadHIDInformation;

      /* Add the Transaction Info to the transaction list.              */
      if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
      {
         /* Attempt to read the Report Reference Descriptor.            */
         Result = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CharacteristicInformation->Characteristic_Handle, 0, TRUE);
         if(Result > 0)
         {
            /* Save the GATM Transaction ID.                            */
            TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;

            /* Return success to the caller.                            */
            ret_val                               = TRUE;
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process and configure the specified HID Service.                  */
static Boolean_t ProcessConfigureHIDService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *HIDService)
{
   Boolean_t    ret_val = FALSE;
   Boolean_t    NoError;
   unsigned int Index;
   unsigned int LoopIndex;
   unsigned int UUIDNumber;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (HIDService))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure HID Service: 0x%04X - 0x%04X.\n", HIDService->ServiceInformation.Service_Handle, HIDService->ServiceInformation.End_Group_Handle));

      /* Loop through the characteristics twice.  We will do this so    */
      /* that we don't potentially read/configure a Report              */
      /* Characteristic before the report map (which could lead to us   */
      /* receiving data before we know what it is).  Therefore we will  */
      /* loop through the characteristic list twice.  We will process   */
      /* everything EXCEPT for Report Characteristics (including the    */
      /* Boot Report Characteristics) on the first loop iteration and   */
      /* everything else on the second loop iteration.                  */
      LoopIndex = 2;
      while(LoopIndex)
      {
         /* Loop through all of the characteristics of the service.     */
         for(Index=0,NoError=TRUE;(Index<HIDService->NumberOfCharacteristics)&&(NoError);Index++)
         {
            /* Verify that this characteristic has a 16-bit UUID.       */
            if(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine what this characteristic is.                */
               if(!HIDS_COMPARE_HIDS_PROTOCOL_MODE_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  if(!HIDS_COMPARE_HIDS_REPORT_MAP_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                  {
                     if(!HIDS_COMPARE_HIDS_REPORT_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                     {
                        if(!HIDS_COMPARE_HIDS_BOOT_KEYBOARD_INPUT_REPORT_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                        {
                           if(!HIDS_COMPARE_HIDS_BOOT_KEYBOARD_OUTPUT_REPORT_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                           {
                              if(!HIDS_COMPARE_HIDS_BOOT_MOUSE_INPUT_REPORT_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                              {
                                 if(!HIDS_COMPARE_HIDS_HID_INFORMATION_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                                 {
                                    if(!HIDS_COMPARE_HIDS_HID_CONTROL_POINT_UUID_TO_UUID_16(HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                                       UUIDNumber = 0;
                                    else
                                       UUIDNumber = 8;
                                 }
                                 else
                                    UUIDNumber = 7;
                              }
                              else
                                 UUIDNumber = 6;
                           }
                           else
                              UUIDNumber = 5;
                        }
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

               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Characteristic UUID: 0x%02X%02X (UUIDNumber=%u).\n", HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16.UUID_Byte1, HIDService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16.UUID_Byte0, UUIDNumber));

               /* Handle this characteristic.                           */
               switch(UUIDNumber)
               {
                  case 1:
                     /* Protocol Mode Characteristic.                   */

                     /* Only process this on the first loop iteration.  */
                     if(LoopIndex == 2)
                     {
                        /* Process the HID Protocol Mode Characteristic.*/
                        if(!ProcessHIDProtocolModeCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index])))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 2:
                     /* Report Map Characteristic.                      */

                     /* Only process this on the first loop iteration.  */
                     if(LoopIndex == 2)
                     {
                        /* Processs the HID Report Map Characteristic.  */
                        if(!ProcessHIDReportMapCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index])))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 3:
                     /* Report Characteristic.                          */

                     /* Only process a Report Characteristic on the     */
                     /* second loop iteration.                          */
                     if(LoopIndex == 1)
                     {
                        /* Process the HID Report Characteristic.       */
                        if(!ProcessHIDReportCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index]), 0))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 4:
                     /* Boot Keyboard Input Report Characteristic.      */

                     /* Only process a Report Characteristic on the     */
                     /* second loop iteration.                          */
                     if(LoopIndex == 1)
                     {
                        /* Process the HID Boot Keyboard Input Report   */
                        /* Characteristic.                              */
                        if(!ProcessHIDReportCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index]), HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_INPUT_REPORT))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 5:
                     /* Boot Keyboard Output Report Characteristic.     */

                     /* Only process a Report Characteristic on the     */
                     /* second loop iteration.                          */
                     if(LoopIndex == 1)
                     {
                        /* Process the HID Boot Keyboard Output Report  */
                        /* Characteristic.                              */
                        if(!ProcessHIDReportCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index]), HOGM_HID_REPORT_TYPE_BOOT_KEYBOARD_OUTPUT_REPORT))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 6:
                     /* Boot Mouse Input Report Characteristic.         */

                     /* Only process a Report Characteristic on the     */
                     /* second loop iteration.                          */
                     if(LoopIndex == 1)
                     {
                        /* Process the HID Boot Mouse Input Report      */
                        /* Characteristic.                              */
                        if(!ProcessHIDReportCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index]), HOGM_HID_REPORT_TYPE_BOOT_MOUSE_INPUT_REPORT))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 7:
                     /* HID Information Characteristic.                 */

                     /* Only process this on the first loop iteration.  */
                     if(LoopIndex == 2)
                     {
                        /* Process the HID Information Characteristic.  */
                        if(!ProcessHIDInformationCharacteristic(ConnectionEntry, &(HIDService->CharacteristicInformationList[Index])))
                        {
                           /* Failed to process the characteristic so   */
                           /* return an error.                          */
                           NoError = FALSE;
                           break;
                        }
                     }
                     break;
                  case 8:
                     /* HID Control Point Characteristic.               */

                     /* Simply save the handle of the Control Point.    */
                     ConnectionEntry->HIDInformation.ControlPointCharacteristicHandle = HIDService->CharacteristicInformationList[Index].Characteristic_Handle;
                     break;
               }
            }
         }

         /* If an error occurred exit the loop.                         */
         if(!NoError)
            break;

         /* Decrement the loop index.                                   */
         LoopIndex--;
      }

      /* If no error occurred return success.                           */
      if(NoError)
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a PnP ID characteristic.                                  */
static Boolean_t ProcessDISPnPIDCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   int                 Result;
   Boolean_t           ret_val = FALSE;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation))
   {
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

      TransactionInfo.TransactionID             = GetNextTransactionID();
      TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
      TransactionInfo.TransactionType           = ttReadPnPID;

      /* Add the Transaction Info to the transaction list.              */
      if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
      {
         /* Attempt to read the Report Reference Descriptor.            */
         Result = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CharacteristicInformation->Characteristic_Handle, 0, TRUE);
         if(Result > 0)
         {
            /* Save the GATM Transaction ID.                            */
            TransactionInfoPtr->GATMTransactionID = (unsigned int)Result;

            /* Return success to the caller.                            */
            ret_val                               = TRUE;

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process and configure the specified DIS Service.                  */
static Boolean_t ProcessConfigureDIService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *DISService)
{
   Boolean_t    ret_val = FALSE;
   Boolean_t    NoError;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (DISService))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure DIS Service: 0x%04X - 0x%04X.\n", DISService->ServiceInformation.Service_Handle, DISService->ServiceInformation.End_Group_Handle));

      /* Loop through all of the characteristics of the service.        */
      for(Index=0,NoError=TRUE;(Index<DISService->NumberOfCharacteristics)&&(NoError);Index++)
      {
         /* Verify that this characteristic has a 16-bit UUID.          */
         if(DISService->CharacteristicInformationList[Index].Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Check to see if this is the PnP ID Characteristic which  */
            /* is the only DIS characteristic we care about.            */
            if(DIS_COMPARE_DIS_PNP_ID_UUID_TO_UUID_16(DISService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
            {
               /* Attempt to process the DIS PnP ID.                    */
               if(!ProcessDISPnPIDCharacteristic(ConnectionEntry, &(DISService->CharacteristicInformationList[Index])))
               {
                  /* Failed to process the characteristic so return an  */
                  /* error.                                             */
                  NoError = FALSE;
               }
            }
         }
      }

      /* If no error occurred return success.                           */
      if(NoError)
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to do a */
   /* Scan Interval/Scan Window Refresh.                                */
static Boolean_t RefereshScanIntervalWindowCharacteristic(Connection_Entry_t *ConnectionEntry, Word_t Handle, Word_t ScanInterval, Word_t ScanWindow)
{
   Byte_t                           Buffer[SCPS_SCAN_INTERVAL_WINDOW_SIZE];
   Boolean_t                        ret_val = FALSE;
   SCPS_Scan_Interval_Window_Data_t ScanIntervalWindow;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (Handle))
   {
      /* Format the Scan Interval/Window value to write.                */
      ScanIntervalWindow.LE_Scan_Interval = ScanInterval;
      ScanIntervalWindow.LE_Scan_Window   = ScanWindow;

      if(!SCPS_Format_Scan_Interval_Window(&ScanIntervalWindow, SCPS_SCAN_INTERVAL_WINDOW_SIZE, Buffer))
      {
         /* Simply do a write without response to the characteristic.   */
         if(GATM_WriteValueWithoutResponse(GATMEventCallbackID, ConnectionEntry->BD_ADDR, Handle, FALSE, SCPS_SCAN_INTERVAL_WINDOW_SIZE, Buffer) > 0)
         {
            /* Return success to the caller.                            */
            ret_val = TRUE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a SCPS Scan Refresh characteristic.                       */
static Boolean_t ProcessSCPSScanRefreshCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   int              Result;
   Word_t           CCCD;
   Boolean_t        ret_val = FALSE;
   unsigned int     Index;
   NonAlignedWord_t CCCDValue;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
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

      /* Make sure that we found the CCCD for the Scan Refresh          */
      /* Characteristic.                                                */
      if(CCCD)
      {
         /* Configure the CCCD for notifications.                       */
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCDValue, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

         /* Perform the write to configure the CCCD.                    */
         Result = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CCCD, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCDValue);
         if(Result > 0)
         {
            /* Save the Handle of the Scan Refresh Characteristic       */
            /* Handle.                                                  */
            ConnectionEntry->HIDInformation.ScanRefreshCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;

            /* We don't care about the result so just return success.   */
            ret_val                                                         = TRUE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a SCPS Scan Interval/Scan Window characteristic.          */
static Boolean_t ProcessSCPSScanIntervalWindowCharacteristic(Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Boolean_t ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE))
   {
      /* Simply refresh the Scan Interval/Scan Window characteristic.   */
      ret_val = RefereshScanIntervalWindowCharacteristic(ConnectionEntry, CharacteristicInformation->Characteristic_Handle, BTPM_CONFIGURATION_DEVICE_MANAGER_DEFAULT_SCAN_INTERVAL, BTPM_CONFIGURATION_DEVICE_MANAGER_DEFAULT_SCAN_WINDOW);
      if(ret_val)
      {
         /* Save the Handle of the Scan Interval Window Characteristic  */
         /* Handle.                                                     */
         ConnectionEntry->HIDInformation.ScanIntervalWindowCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process and configure the specified SCPS Service.                 */
static Boolean_t ProcessConfigureSCPService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *SCPService)
{
   Boolean_t    ret_val = FALSE;
   Boolean_t    NoError;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (SCPService))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure SCPS Service: 0x%04X - 0x%04X.\n", SCPService->ServiceInformation.Service_Handle, SCPService->ServiceInformation.End_Group_Handle));

      /* Loop through all of the characteristics of the service.        */
      for(Index=0,NoError=TRUE;(Index<SCPService->NumberOfCharacteristics)&&(NoError);Index++)
      {
         /* Verify that this characteristic has a 16-bit UUID.          */
         if(SCPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Check to see if this is the Scan Interval/Scan Window    */
            /* Characteristic.                                          */
            if(SCPS_COMPARE_SCAN_INTERVAL_WINDOW_UUID_TO_UUID_16(SCPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
            {
               /* Process and Configure the Scan Interval/Window        */
               /* Characteristic.                                       */
               if(!ProcessSCPSScanIntervalWindowCharacteristic(ConnectionEntry, &(SCPService->CharacteristicInformationList[Index])))
               {
                  /* Failed to process the characteristic so return an  */
                  /* error.                                             */
                  NoError = FALSE;
               }
            }
            else
            {
               /* Check to see if this is the Scan Refresh              */
               /* Characteristic.                                       */
               if(SCPS_COMPARE_SCAN_REFRESH_UUID_TO_UUID_16(SCPService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  /* Process and Configure the Scan Refresh             */
                  /* Characteristic.                                    */
                  if(!ProcessSCPSScanRefreshCharacteristic(ConnectionEntry, &(SCPService->CharacteristicInformationList[Index])))
                  {
                     /* Failed to process the characteristic so return  */
                     /* an error.                                       */
                     NoError = FALSE;
                  }
               }
            }
         }
      }

      /* If no error occurred return success.                           */
      if(NoError)
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process and configure the specified Service (which will only      */
   /* happen if a characteristic in this service has a Report Reference */
   /* Descriptor).                                                      */
static Boolean_t ProcessConfigureService(Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *Service)
{
   Boolean_t    ret_val = FALSE;
   Boolean_t    NoError;
   unsigned int Index1;
   unsigned int Index2;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (Service))
   {
      /* Loop through all of the characteristics of the service.        */
      for(Index1=0,NoError=TRUE;(Index1<Service->NumberOfCharacteristics)&&(NoError);Index1++)
      {
         /* Loop through the descriptor list for this characteristic.   */
         for(Index2=0;(Index2<Service->CharacteristicInformationList[Index1].NumberOfDescriptors)&&(NoError);Index2++)
         {
            /* Check to see if this is a Report Reference Descriptor.   */
            if((Service->CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16) && (HIDS_COMPARE_REPORT_REFERENCE_DESCRIPTOR_UUID_TO_UUID_16(Service->CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID.UUID_16)))
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure Service due to RRD: 0x%04X - 0x%04X, UUID = 0x%02X%02X.\n", Service->ServiceInformation.Service_Handle, Service->ServiceInformation.End_Group_Handle, Service->ServiceInformation.UUID.UUID.UUID_16.UUID_Byte1, Service->ServiceInformation.UUID.UUID.UUID_16.UUID_Byte0));

               /* Since the characteristic has a Report Reference       */
               /* Descriptor, it must be a part of the HID Report.      */
               /* Therefore process and configure the HID Report.       */
               if(!ProcessHIDReportCharacteristic(ConnectionEntry, &(Service->CharacteristicInformationList[Index1]), 0))
               {
                  /* An error occurred so return an error.              */
                  NoError = FALSE;
               }

               /* Exit the descriptor loop.                             */
               break;
            }
         }
      }

      /* If no error occurred return success.                           */
      if(NoError)
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* Process and Configure a HID Device Connection.                    */
static void ProcessConfigureHIDConnection(BD_ADDR_t BD_ADDR)
{
   Boolean_t                    ConfiguredSuccessfully;
   unsigned int                 Index;
   unsigned int                 NumberHIDServices;
   Connection_Entry_t           ConnectionEntry;
   Connection_Entry_t          *ConnectionEntryPtr;
   DEVM_Parsed_Services_Data_t  ParsedGATTData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Before we go to the trouble of checking the service table,     */
      /* verify that this device supports the HID Device Role.          */
      if(CheckHIDServicesSupported(BD_ADDR))
      {
         /* Query the parsed GATT data.                                 */
         if(QueryParsedServicesData(BD_ADDR, &ParsedGATTData))
         {
            /* Initialize the Connection Entry to add.                  */
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

            ConnectionEntry.ConnectionState     = csConfiguring;
            ConnectionEntry.CurrentProtocolMode = hpmReport;
            ConnectionEntry.BD_ADDR             = BD_ADDR;

            /* Add the Connection Info to the Connection List.          */
            if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
            {
               /* Process all of the required services.                 */
               for(Index=0,NumberHIDServices=0,ConfiguredSuccessfully=TRUE;(Index<ParsedGATTData.NumberServices)&&(ConfiguredSuccessfully);Index++)
               {
                  /* Verify that this service has a 16bit UUID.         */
                  if(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID_Type == guUUID_16)
                  {
                     /* Check to see if this is a HID Service.          */
                     if(HIDS_COMPARE_HIDS_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16))
                     {
                        /* Attempt to process and configure the HID     */
                        /* Service.                                     */
                        if(ProcessConfigureHIDService(ConnectionEntryPtr, &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index])))
                           ++NumberHIDServices;
                        else
                        {
                           ConfiguredSuccessfully = FALSE;
                           break;
                        }
                     }
                     else
                     {
                        /* Check to see if this is a DIS Service.       */
                        if(DIS_COMPARE_DIS_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16))
                        {
                           /* Attempt to process the DIS Service.       */
                           if(!ProcessConfigureDIService(ConnectionEntryPtr, &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index])))
                           {
                              /* Failed to process the DIS Service so   */
                              /* return an error.                       */
                              ConfiguredSuccessfully = FALSE;
                           }
                        }
                        else
                        {
                           /* Check to see if this is a ScP Service.    */
                           if(SCPS_COMPARE_SCPS_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16))
                           {
                              /* Process and configure the Scan         */
                              /* Parameter Service.                     */
                              if(!ProcessConfigureSCPService(ConnectionEntryPtr, &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index])))
                              {
                                 /* Failed to process the SCPS Service  */
                                 /* so return an error.                 */
                                 ConfiguredSuccessfully = FALSE;
                              }
                           }
                           else
                           {
                              /* Process this other service and look for*/
                              /* report reference descriptors.          */
                              if(!ProcessConfigureService(ConnectionEntryPtr, &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index])))
                              {
                                 /* Failed to process the Service so    */
                                 /* return an error.                    */
                                 ConfiguredSuccessfully = FALSE;
                              }
                           }
                        }
                     }
                  }
               }

               /* If we did not configure the services correctly then   */
               /* delete the connection entry.                          */
               if((NumberHIDServices == 0) || (!ConfiguredSuccessfully))
               {
                  if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                     FreeConnectionEntryMemory(ConnectionEntryPtr);
               }
            }

            /* Free the parsed service data.                            */
            DEVM_FreeParsedServicesData(&ParsedGATTData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to parse*/
   /* the correct HID (and other Service) Handles for a device that has */
   /* been reloaded from file.                                          */
static void ParseHOGAttributeHandles(Connection_Entry_t *ConnectionEntry)
{
   unsigned int                       Index1;
   unsigned int                       Index2;
   DEVM_Parsed_Services_Data_t        ParsedGATTData;
   GATT_Characteristic_Information_t *CharacteristicInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Query the parsed GATT data.                                    */
      if(QueryParsedServicesData(ConnectionEntry->BD_ADDR, &ParsedGATTData))
      {
         /* Process all of the required services.                       */
         for(Index1=0;Index1<ParsedGATTData.NumberServices;Index1++)
         {
            /* Verify that this service has a 16bit UUID.               */
            if(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].ServiceInformation.UUID.UUID_Type == guUUID_16)
            {
               /* Check to see if this is a HID Service.                */
               if(HIDS_COMPARE_HIDS_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].ServiceInformation.UUID.UUID.UUID_16))
               {
                  /* Loop through the characteristic information list.  */
                  CharacteristicInformation = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].CharacteristicInformationList;

                  for(Index2=0;Index2<ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].NumberOfCharacteristics;Index2++,CharacteristicInformation++)
                  {
                     /* Verify that this characteristic has a 16-bit    */
                     /* UUID.                                           */
                     if(CharacteristicInformation->Characteristic_UUID.UUID_Type == guUUID_16)
                     {
                        /* Check to see if this is either the Protocol  */
                        /* Mode or HID Control Point characteristic as  */
                        /* these are the only two handles that are      */
                        /* stored after the first connection.           */
                        if(HIDS_COMPARE_HIDS_PROTOCOL_MODE_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                           ConnectionEntry->HIDInformation.ProtocolModeCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;
                        else
                        {
                           if(HIDS_COMPARE_HIDS_HID_CONTROL_POINT_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                              ConnectionEntry->HIDInformation.ControlPointCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;
                        }
                     }
                  }
               }
               else
               {
                  /* Check to see if this is the Scan Parameters        */
                  /* Service.                                           */
                  if(SCPS_COMPARE_SCPS_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].ServiceInformation.UUID.UUID.UUID_16))
                  {
                     /* Loop through the characteristic information     */
                     /* list.                                           */
                     CharacteristicInformation = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].CharacteristicInformationList;

                     for(Index2=0;Index2<ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].NumberOfCharacteristics;Index2++,CharacteristicInformation++)
                     {
                        /* Verify that this characteristic has a 16-bit */
                        /* UUID.                                        */
                        if(CharacteristicInformation->Characteristic_UUID.UUID_Type == guUUID_16)
                        {
                           /* Check to see if this is the Scan          */
                           /* Interval/Scan Window Characteristic.      */
                           if(SCPS_COMPARE_SCAN_INTERVAL_WINDOW_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                              ConnectionEntry->HIDInformation.ScanIntervalWindowCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;
                           else
                           {
                              /* Check to see if this is the Scan       */
                              /* Refresh Characteristic.                */
                              if(SCPS_COMPARE_SCAN_REFRESH_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                                 ConnectionEntry->HIDInformation.ScanRefreshCharacteristicHandle = CharacteristicInformation->Characteristic_Handle;
                           }
                        }
                     }
                  }
               }
            }
         }

         /* Free the parsed service data.                               */
         DEVM_FreeParsedServicesData(&ParsedGATTData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a potential HID Device connection.                        */
static void ProcessHIDDeviceConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Boolean_t           DispatchConnection = FALSE;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(RemoteDeviceProperties)
   {
      /* First attempt to reload the connection entry from the LE       */
      /* Configuration File.                                            */
      if((ConnectionEntry = LoadHIDDeviceFromFile(RemoteDeviceProperties->BD_ADDR)) != NULL)
      {
         /* Attempt to parse the HID Attribute Handles that are not     */
         /* stored in the configuration file.                           */
         ParseHOGAttributeHandles(ConnectionEntry);

         /* Check to see if we need to put the device in Boot Mode.     */
         if(ConnectionEntry->HIDInformation.ProtocolModeCharacteristicHandle)
         {
            /* If Report Mode is not supported go ahead and enter into  */
            /* Boot Mode.                                               */
            if(!(SupportedFeatures & HOGM_SUPPORTED_FEATURES_FLAGS_REPORT_MODE))
            {
               /* Put the connection into Boot Mode.                    */
               HIDSetProtocolMode(ConnectionEntry, NULL, pmBoot);
            }
            else
               DispatchConnection = TRUE;
         }
         else
            DispatchConnection = TRUE;

         /* Dispatch the HID Device Connection Event if requested.      */
         if(DispatchConnection)
            DispatchHIDDeviceConnection(ConnectionEntry);
      }
      else
      {
         /* Attempt to Process/Configure the HID Device connection.     */
         ProcessConfigureHIDConnection(RemoteDeviceProperties->BD_ADDR);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* process a HID Device Disconnection.                               */
static void ProcessHIDDeviceDisconnection(BD_ADDR_t RemoteDeviceAddress)
{
   HOGM_Event_Data_t                       HIDMEventData;
   Connection_Entry_t                     *ConnectionEntry;
   HOGM_HID_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
   {
      /* Only dispatch the disconnection if we are in the connection    */
      /* state.                                                         */
      if(ConnectionEntry->ConnectionState == csConnected)
      {
         /* Next, format up the Event to dispatch.                      */
         HIDMEventData.EventType                                                 = hetHOGMHIDDeviceDisconnected;
         HIDMEventData.EventLength                                               = HOGM_HID_DEVICE_DISCONNECTED_EVENT_DATA_SIZE;

         HIDMEventData.EventData.DeviceDisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HOGP_MANAGER;
         Message.MessageHeader.MessageFunction = HOGM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HOGM_HID_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = RemoteDeviceAddress;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHOGEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }

      /* Free the memory allocated for the connection entry.            */
      FreeConnectionEntryMemory(ConnectionEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the HOGM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("HOGM Address Updated\n"));

            /* Delete any information stored with the old BD_ADDR.      */
            StoreHIDDeviceToFile(ConnectionEntryPtr, FALSE);

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;

            /* Store information to the file with the new BD_ADDR.      */
            StoreHIDDeviceToFile(ConnectionEntryPtr, TRUE);
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the HOGM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long      RequiredConnectionFlags;
   Connection_Entry_t ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Address is updated or the LE Pairing State changes.            */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

         /* Process the LE Pairing State Change Event if necessary.     */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
         {
            /* Set the required flags.  We must be currently            */
            /* connected/paired over LE, with an encrypted link and must*/
            /* know all of the remote device's services before          */
            /* processing the connection.                               */
            RequiredConnectionFlags = (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED);

            /* Check to see that the required flags are set for a       */
            /* connection.                                              */
            if((RemoteDeviceProperties->RemoteDeviceFlags & RequiredConnectionFlags) == RequiredConnectionFlags)
            {
               /* Process this as a potential HID device connection.    */
               ProcessHIDDeviceConnection(RemoteDeviceProperties);
            }
            else
            {
               /* Check to see if we unpaired from a device OR if the   */
               /* service state is no longer known.  In either case we  */
               /* need to delete any information stored for this device.*/
               if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
               {
                  /* Check to see if we un-paired or no longer know the */
                  /* services.                                          */
                  if((RemoteDeviceProperties->RemoteDeviceFlags & (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)) != (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN))
                  {
                     /* Check to see if we stored anything for this     */
                     /* device.                                         */
                     if(ReloadHIDInformation(NULL, RemoteDeviceProperties->BD_ADDR))
                     {
                        /* Delete the information that is stored for    */
                        /* this device in the file.                     */
                        BTPS_MemInitialize(&ConnectionEntry, 0, (sizeof(ConnectionEntry)));
                        ConnectionEntry.BD_ADDR = RemoteDeviceProperties->BD_ADDR;

                        StoreHIDDeviceToFile(&ConnectionEntry, FALSE);
                     }
                  }
               }

               /* Process this as a potential HID device disconnection. */
               ProcessHIDDeviceDisconnection(RemoteDeviceProperties->BD_ADDR);
            }
         }

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register HOG Events*/
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessRegisterHOGEventsMessage(HOGM_Register_HID_Events_Request_t *Message)
{
   int                                 Result;
   HOG_Entry_Info_t                    HOGEntryInfo;
   HOGM_Register_HID_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Attempt to add an entry into the HOG Entry list.               */
      BTPS_MemInitialize(&HOGEntryInfo, 0, sizeof(HOG_Entry_Info_t));

      HOGEntryInfo.CallbackID         = GetNextCallbackID();
      HOGEntryInfo.ClientID           = Message->MessageHeader.AddressID;
      HOGEntryInfo.Flags              = HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

      if(AddHOGEntryInfoEntry(&HOGEntryInfoList, &HOGEntryInfo))
         Result = HOGEntryInfo.CallbackID;
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HOGM_REGISTER_HID_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.HOGEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status             = 0;
      }
      else
      {
         ResponseMessage.HOGEventsHandlerID = 0;

         ResponseMessage.Status             = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register HOG    */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterHOGEventsMessage(HOGM_Un_Register_HID_Events_Request_t *Message)
{
   int                                     Result;
   HOG_Entry_Info_t                       *HOGEntryInfo;
   HOGM_Un_Register_HID_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoList, Message->HOGEventsHandlerID)) != NULL) && (HOGEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((HOGEntryInfo = DeleteHOGEntryInfoEntry(&HOGEntryInfoList, Message->HOGEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeHOGEntryInfoEntryMemory(HOGEntryInfo);

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

      ResponseMessage.MessageHeader.MessageLength  = HOGM_UN_REGISTER_HID_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register HOG Data  */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessRegisterHOGDataEventsMessage(HOGM_Register_HID_Data_Events_Response_t *Message)
{
   int                                      Result;
   HOG_Entry_Info_t                         HOGEntryInfo;
   HOGM_Register_HID_Data_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HOGM_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Before proceding any further, make sure that there is not      */
      /* already a Data Event Handler registered.                       */
      if(!HOGEntryInfoDataList)
      {
         /* Attempt to add an entry into the HOG Entry list.            */
         BTPS_MemInitialize(&HOGEntryInfo, 0, sizeof(HOG_Entry_Info_t));

         HOGEntryInfo.CallbackID         = GetNextCallbackID();
         HOGEntryInfo.ClientID           = Message->MessageHeader.AddressID;
         HOGEntryInfo.Flags              = HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         if(AddHOGEntryInfoEntry(&HOGEntryInfoDataList, &HOGEntryInfo))
            Result = HOGEntryInfo.CallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_HID_OVER_GATT_DATA_EVENTS_ALREADY_REGISTERED;

      if(Result > 0)
      {
         ResponseMessage.HOGDataEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status                 = 0;
      }
      else
      {
         ResponseMessage.HOGDataEventsHandlerID = 0;

         ResponseMessage.Status                 = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register HOG    */
   /* Data Events Message and responds to the message accordingly.  This*/
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterHOGDataEventsMessage(HOGM_Un_Register_HID_Data_Events_Request_t *Message)
{
   int                                          Result;
   HOG_Entry_Info_t                            *HOGEntryInfo;
   HOGM_Un_Register_HID_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, Message->HOGDataEventsHandlerID)) != NULL) && (HOGEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((HOGEntryInfo = DeleteHOGEntryInfoEntry(&HOGEntryInfoDataList, Message->HOGDataEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeHOGEntryInfoEntryMemory(HOGEntryInfo);

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

      ResponseMessage.MessageHeader.MessageLength  = HOGM_UN_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Protocol Mode  */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessSetProtocolModeMessage(HOGM_HID_Set_Protocol_Mode_Request_t *Message)
{
   int                                    Result;
   HOG_Entry_Info_t                      *HOGEntryInfo;
   Connection_Entry_t                    *ConnectionEntry;
   HOGM_HID_Set_Protocol_Mode_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, Message->HOGManagerDataHandlerID)) != NULL) && (HOGEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Check to see if we actually need to do anything.         */
            if(ConnectionEntry->CurrentProtocolMode != Message->ProtocolMode)
            {
               /* Call the internal function to actually set the        */
               /* protocol mode.                                        */
               Result = HIDSetProtocolMode(ConnectionEntry, HOGEntryInfo, Message->ProtocolMode);
            }
            else
               Result = BTPM_ERROR_CODE_HID_OVER_GATT_PROTOCOL_MODE_CURRENT;
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HOGM_HID_SET_PROTOCOL_MODE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Suspend Mode   */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessSetSuspendModeMessage(HOGM_HID_Set_Suspend_Mode_Request_t *Message)
{
   int                                   Result;
   HOG_Entry_Info_t                     *HOGEntryInfo;
   Connection_Entry_t                   *ConnectionEntry;
   HOGM_HID_Set_Suspend_Mode_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, Message->HOGManagerDataHandlerID)) != NULL) && (HOGEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Call the internal function to set the suspend mode.      */
            Result = HIDSetSuspendMode(ConnectionEntry, Message->Suspend);
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HOGM_HID_SET_SUSPEND_MODE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Get Report Message */
   /* and responds to the message accordingly.  This function does not  */
   /* verify the integrity of the Message (i.e.  the length) because it */
   /* is the caller's responsibility to verify the Message before       */
   /* calling this function.                                            */
static void ProcessGetReportMessage(HOGM_HID_Get_Report_Request_t *Message)
{
   int                             Result;
   HOG_Entry_Info_t               *HOGEntryInfo;
   HID_Report_Info_t              *ReportInfo;
   Connection_Entry_t             *ConnectionEntry;
   HOGM_HID_Get_Report_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, Message->HOGManagerDataHandlerID)) != NULL) && (HOGEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Search for the HID Report Info.                          */
            if((ReportInfo = SearchHIDReportInfoByReportInfo(&(ConnectionEntry->ReportInfoList), &(Message->ReportInformation))) != NULL)
            {
               /* Simply call the internal function to do the Get Report*/
               /* Request.                                              */
               Result = HIDGetReportRequest(ConnectionEntry, HOGEntryInfo, ReportInfo);
            }
            else
               Result = BTPM_ERROR_CODE_HID_OVER_GATT_INVALID_REPORT;
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HOGM_HID_GET_REPORT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result >= 0)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Report Message */
   /* and responds to the message accordingly.  This function does not  */
   /* verify the integrity of the Message (i.e.  the length) because it */
   /* is the caller's responsibility to verify the Message before       */
   /* calling this function.                                            */
static void ProcessSetReportMessage(HOGM_HID_Set_Report_Request_t *Message)
{
   int                             Result;
   HOG_Entry_Info_t               *HOGEntryInfo;
   HID_Report_Info_t              *ReportInfo;
   Connection_Entry_t             *ConnectionEntry;
   HOGM_HID_Set_Report_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, Message->HOGManagerDataHandlerID)) != NULL) && (HOGEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Search for the HID Report Info.                          */
            if((ReportInfo = SearchHIDReportInfoByReportInfo(&(ConnectionEntry->ReportInfoList), &(Message->ReportInformation))) != NULL)
            {
               /* Simply call the internal function to do the Set Report*/
               /* Request.                                              */
               Result = HIDSetReportRequest(ConnectionEntry, HOGEntryInfo, ReportInfo, Message->ResponseExpected, Message->ReportDataLength, Message->ReportData);
            }
            else
               Result = BTPM_ERROR_CODE_HID_OVER_GATT_INVALID_REPORT;
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HOGM_HID_SET_REPORT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result >= 0)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the HOG Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HOGM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Register HOG Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_REGISTER_HID_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register HOG Events Request.                          */
               ProcessRegisterHOGEventsMessage((HOGM_Register_HID_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register HOG Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_UN_REGISTER_HID_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register HOG Events Request.                       */
               ProcessUnRegisterHOGEventsMessage((HOGM_Un_Register_HID_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_REGISTER_HID_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Register HOG Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register HOG Data Events Request.                     */
               ProcessRegisterHOGDataEventsMessage((HOGM_Register_HID_Data_Events_Response_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register HOG Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_UN_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register HOG Data Events Request.                  */
               ProcessUnRegisterHOGDataEventsMessage((HOGM_Un_Register_HID_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_SET_PROTOCOL_MODE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Protocol Mode Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_SET_PROTOCOL_MODE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead process the Set      */
               /* Protocol Mode Request.                                */
               ProcessSetProtocolModeMessage((HOGM_HID_Set_Protocol_Mode_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_SET_SUSPEND_MODE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Suspend Mode Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_SET_SUSPEND_MODE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead process the Set      */
               /* Suspend Mode Request.                                 */
               ProcessSetSuspendModeMessage((HOGM_HID_Set_Suspend_Mode_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_GET_REPORT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Report Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_GET_REPORT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead process the Get      */
               /* Report Request.                                       */
               ProcessGetReportMessage((HOGM_HID_Get_Report_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HOGM_MESSAGE_FUNCTION_SET_REPORT:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Report Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_SET_REPORT_REQUEST_SIZE(0))
            {
               if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HOGM_HID_SET_REPORT_REQUEST_SIZE(((HOGM_HID_Set_Report_Request_t *)Message)->ReportDataLength))
               {
                  /* Size seems to be valid, go ahead process the Set   */
                  /* Report Request.                                    */
                  ProcessSetReportMessage((HOGM_HID_Set_Report_Request_t *)Message);
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;

         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Boolean_t          LoopCount;
   HOG_Entry_Info_t  *HOGEntryInfo;
   HOG_Entry_Info_t **_HOGEntryInfoList;
   HOG_Entry_Info_t  *tmpHOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      HOGEntryInfo      = HOGEntryInfoList;
      _HOGEntryInfoList = &HOGEntryInfoList;

      /* We need to loop through both lists as there could be client    */
      /* registrations in any of the lists.                             */
      LoopCount = 2;
      while(LoopCount)
      {
         while(HOGEntryInfo)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(HOGEntryInfo->ClientID == ClientID)
            {
               /* Note the next HOG Entry in the list (we are about to  */
               /* delete the current entry).                            */
               tmpHOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;

               /* Go ahead and delete the HOG Information Entry and     */
               /* clean up the resources.                               */
               if((HOGEntryInfo = DeleteHOGEntryInfoEntry(_HOGEntryInfoList, HOGEntryInfo->CallbackID)) != NULL)
               {
                  /* All finished with the memory so free the entry.    */
                  FreeHOGEntryInfoEntryMemory(HOGEntryInfo);
               }

               /* Go ahead and set the next HOG Information Entry (past */
               /* the one we just deleted).                             */
               HOGEntryInfo = tmpHOGEntryInfo;
            }
            else
               HOGEntryInfo = HOGEntryInfo->NextHOGEntryInfoPtr;
         }

         /* Decrement the loop count so that we can make another pass   */
         /* through the loop.                                           */
         LoopCount--;

         /* We have processed the HOG Information List, now process the */
         /* HOG Information Data List.                                  */
         HOGEntryInfo      = HOGEntryInfoDataList;
         _HOGEntryInfoList = &HOGEntryInfoDataList;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Transaction Response.                              */
static void ProcessGATTTransactionResponse(Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, Boolean_t ErrorResponse, void *EventData)
{
   Byte_t                       *ReportDescriptorBuffer;
   Byte_t                        ErrorCode;
   unsigned int                  ReportDescriptorLength;
   HOG_Entry_Info_t             *EventCallbackInfo;
   DIS_PNP_ID_Data_t             PNPIDData;
   HID_Report_Info_t            *ReportInfo;
   HIDS_HID_Information_Data_t   HIDInformation;
   HIDS_Report_Reference_Data_t  ReportReference;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (TransactionInfo) && (EventData))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: Status = 0x%02X\n", (ErrorResponse?((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode:0)));

      /* Process the correct transaction.                               */
      switch(TransactionInfo->TransactionType)
      {
         case ttReadHIDInformation:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttReadHIDInformation\n"));

            /* Process this if no error has occurred.                   */
            if(!ErrorResponse)
            {
               /* Attempt to decode the HID Information.                */
               if(!HIDS_Decode_HID_Information(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength, ((GATM_Read_Response_Event_Data_t *)EventData)->Value, &HIDInformation))
               {
                  /* Update the HID Device Information.                 */
                  ConnectionEntry->HIDInformation.HIDDeviceInformation.HIDVersion  = HIDInformation.Version;
                  ConnectionEntry->HIDInformation.HIDDeviceInformation.CountryCode = HIDInformation.CountryCode;

                  if(HIDInformation.Flags & HIDS_HID_INFORMATION_FLAGS_REMOTE_WAKE)
                     ConnectionEntry->HIDInformation.SupportedFeatures |= HOGM_SUPPORTED_FEATURES_FLAGS_REMOTE_WAKEUP_CAPABLE;

                  if(HIDInformation.Flags & HIDS_HID_INFORMATION_FLAGS_NORMALLY_CONNECTABLE)
                     ConnectionEntry->HIDInformation.SupportedFeatures |= HOGM_SUPPORTED_FEATURES_FLAGS_NORMALLY_CONNECTABLE;
               }
               else
               {
                  /* Flag that an error occurred.                       */
                  ErrorResponse = TRUE;
               }
            }
            break;
         case ttReadReportMap:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttReadReportMap\n"));

            /* Process this if no error has occurred.                   */
            if(!ErrorResponse)
            {
               /* Calculate the size of the Memory Buffer that is needed*/
               /* to hold the Report Map.                               */
               ReportDescriptorLength = ConnectionEntry->HIDInformation.ReportDescriptorLength + ((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength;

               /* Allocate a buffer to hold the Report Descriptor.      */
               if((ReportDescriptorBuffer = BTPS_AllocateMemory(ReportDescriptorLength)) != NULL)
               {
                  /* Copy any existing descriptor data into the buffer. */
                  if((ConnectionEntry->HIDInformation.ReportDescriptorLength) && (ConnectionEntry->HIDInformation.ReportDescriptor))
                  {
                     /* Copy the data into the new buffer.              */
                     BTPS_MemCopy(ReportDescriptorBuffer, ConnectionEntry->HIDInformation.ReportDescriptor, ConnectionEntry->HIDInformation.ReportDescriptorLength);

                     /* Free the previously allocated buffer.           */
                     BTPS_FreeMemory(ReportDescriptorBuffer);

                     /* Set the length of the descriptor.               */
                     ReportDescriptorLength = ConnectionEntry->HIDInformation.ReportDescriptorLength;
                  }
                  else
                     ReportDescriptorLength = 0;

                  /* Copy the data into the new buffer.                 */
                  BTPS_MemCopy(&ReportDescriptorBuffer[ReportDescriptorLength], ((GATM_Read_Response_Event_Data_t *)EventData)->Value, ((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength);

                  ReportDescriptorLength += ((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength;

                  /* Save the new report descriptor buffer and length.  */
                  ConnectionEntry->HIDInformation.ReportDescriptorLength = ReportDescriptorLength;
                  ConnectionEntry->HIDInformation.ReportDescriptor       = ReportDescriptorBuffer;
               }
               else
               {
                  /* Flag that an error occurred.                       */
                  ErrorResponse = TRUE;
               }
            }
            break;
         case ttReadReportReference:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttReadReportReference\n"));

            /* Process this if no error has occurred.                   */
            if(!ErrorResponse)
            {
               /* Attempt to decode the Report Reference Characteristic.*/
               if(!HIDS_Decode_Report_Reference(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength, ((GATM_Read_Response_Event_Data_t *)EventData)->Value, &ReportReference))
               {
                  /* Find the HID Report Info structure for this Report.*/
                  if((ReportInfo = SearchHIDReportInfo(&(ConnectionEntry->ReportInfoList), TransactionInfo->CharacteristicValueHandle)) != NULL)
                  {
                     /* Save the Report Reference Information for this  */
                     /* device.                                         */
                     ReportInfo->HIDReportInformation.ReportID = ReportReference.ReportID;

                     /* Determine what type of report this is.          */
                     switch(ReportReference.ReportType)
                     {
                        case HIDS_REPORT_REFERENCE_REPORT_TYPE_INPUT_REPORT:
                           ReportInfo->HIDReportInformation.ReportType = hrtInput;
                           break;
                        case HIDS_REPORT_REFERENCE_REPORT_TYPE_OUTPUT_REPORT:
                           ReportInfo->HIDReportInformation.ReportType = hrtOutput;
                           break;
                        case HIDS_REPORT_REFERENCE_REPORT_TYPE_FEATURE_REPORT:
                           ReportInfo->HIDReportInformation.ReportType = hrtFeature;
                           break;
                        default:
                           /* Flag that an error occurred.              */
                           ErrorResponse = TRUE;
                           break;
                     }
                  }
                  else
                  {
                     /* Flag that an error occurred.                    */
                     ErrorResponse = TRUE;
                  }
               }
               else
               {
                  /* Flag that an error occurred.                       */
                  ErrorResponse = TRUE;
               }
            }
            else
            {
               /* Delete the HID Report Info since we failed to read the*/
               /* RRD.                                                  */
               if((ReportInfo = DeleteHIDReportInfo(&(ConnectionEntry->ReportInfoList), TransactionInfo->CharacteristicValueHandle)) != NULL)
               {
                  FreeHIDReportInfoMemory(ReportInfo);

                  ReportInfo = NULL;
               }
            }
            break;
         case ttReadPnPID:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttReadPnPID\n"));

            /* Process this if no error has occurred.                   */
            if(!ErrorResponse)
            {
               /* Attempt to decode the PNP ID Characteristic.          */
               if(!DIS_Decode_PNP_ID(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength, ((GATM_Read_Response_Event_Data_t *)EventData)->Value, &PNPIDData))
               {
                  /* Save the PNP ID Information.                       */
                  ConnectionEntry->HIDInformation.HIDDeviceInformation.ProductID       = PNPIDData.ProductID;
                  ConnectionEntry->HIDInformation.HIDDeviceInformation.VendorID        = PNPIDData.VendorID;
                  ConnectionEntry->HIDInformation.HIDDeviceInformation.ProductVersion  = PNPIDData.ProductVersion;
                  ConnectionEntry->HIDInformation.HIDDeviceInformation.VendorID_Source = PNPIDData.VendorID_Source;
               }
               else
               {
                  /* Flag that an error occurred.                       */
                  ErrorResponse = TRUE;
               }
            }
            break;
         case ttConfigureCCCD:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttConfigureCCCD\n"));

            /* There is nothing to do here other than let the logic     */
            /* determine if we can move to the connection state.        */
            break;
         case ttSetReport:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttSetReport\n"));

            /* Search for the Callback Entry for this response.         */
            if((EventCallbackInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, TransactionInfo->HOGManagerDataCallbackID)) != NULL)
            {
               /* Search for the Report Info for this response.         */
               if((ReportInfo = SearchHIDReportInfo(&(ConnectionEntry->ReportInfoList), TransactionInfo->CharacteristicValueHandle)) != NULL)
               {
                  /* Determine the correct error code.                  */
                  if(ErrorResponse)
                  {
                     if(((GATM_Error_Response_Event_Data_t *)EventData)->ErrorType == retErrorResponse)
                        ErrorCode = ((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode;
                     else
                        ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;
                  }
                  else
                     ErrorCode = 0;

                  /* Dispatch the HID Set Report Response Event.        */
                  DispatchHIDSetReportResponse(ConnectionEntry, EventCallbackInfo, ReportInfo, TransactionInfo->TransactionID, ErrorCode);
               }
            }
            break;
         case ttGetReport:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Transaction Response: ttGetReport\n"));

            /* Search for the Callback Entry for this response.         */
            if((EventCallbackInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, TransactionInfo->HOGManagerDataCallbackID)) != NULL)
            {
               /* Search for the Report Info for this response.         */
               if((ReportInfo = SearchHIDReportInfo(&(ConnectionEntry->ReportInfoList), TransactionInfo->CharacteristicValueHandle)) != NULL)
               {
                  /* Determine the correct error code.                  */
                  if(ErrorResponse)
                  {
                     if(((GATM_Error_Response_Event_Data_t *)EventData)->ErrorType == retErrorResponse)
                        ErrorCode = (((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode);
                     else
                        ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;

                     ReportDescriptorLength = 0;
                     ReportDescriptorBuffer = NULL;
                  }
                  else
                  {
                     ErrorCode              = 0;
                     ReportDescriptorLength = ((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength;
                     ReportDescriptorBuffer = ((GATM_Read_Response_Event_Data_t *)EventData)->Value;
                  }

                  /* Dispatch the Get Report Response event.            */
                  DispatchHIDGetReportResponse(ConnectionEntry, EventCallbackInfo, ReportInfo, TransactionInfo->TransactionID, ErrorCode, ReportDescriptorLength, ReportDescriptorBuffer);
               }
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled response type\n"));
            break;
      }

      /* If an error occurs when configuring the HID Service we will    */
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
            /* transaction and can therefore dispatch a HID Device      */
            /* Connection event.                                        */
            if(ConnectionEntry->TransactionInfoList == NULL)
            {
               /* Store the connection information to the LE            */
               /* Configuration file.                                   */
               StoreHIDDeviceToFile(ConnectionEntry, TRUE);

               /* Dispatch the HID Device Connection event.             */
               DispatchHIDDeviceConnection(ConnectionEntry);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Handle Value notification.                         */
   /* * NOTE * This function *MUST* be called with the HOG Manager Lock */
   /*          held.                                                    */
static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   HID_Report_Info_t  *ReportInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(HandleValueEventData)
   {
      /* Verify that this is a notification as that is all we are       */
      /* concerned about.                                               */
      if(HandleValueEventData->HandleValueIndication == FALSE)
      {
         /* Search for the connection entry for this device.            */
         if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, HandleValueEventData->RemoteDeviceAddress)) != NULL)
         {
            /* Search for the Report Info structure for this            */
            /* notification.  If none exists this must be a notification*/
            /* for something I don't care about.                        */
            if((ReportInfo = SearchHIDReportInfo(&(ConnectionEntry->ReportInfoList), HandleValueEventData->AttributeHandle)) != NULL)
            {
               /* Since we have a Report Info this must be an Input     */
               /* Report Data Indication.  Therefore go ahead and       */
               /* dispatch the event.                                   */
               DispatchHIDDataIndication(ConnectionEntry, ReportInfo, HandleValueEventData->AttributeValueLength, HandleValueEventData->AttributeValue);
            }
            else
            {
               /* Check to see if this is a Scan Refresh Notification.  */
               if(ConnectionEntry->HIDInformation.ScanRefreshCharacteristicHandle == HandleValueEventData->AttributeHandle)
               {
                  /* Check to see if we need to update the Scan         */
                  /* Interval/Window characteristic.                    */
                  if((HandleValueEventData->AttributeValueLength == 1) && (HandleValueEventData->AttributeValue[0] == SCPS_SCAN_REFRESH_VALUE_SERVER_REQUIRES_REFRESH))
                     RefereshScanIntervalWindowCharacteristic(ConnectionEntry, ConnectionEntry->HIDInformation.ScanRefreshCharacteristicHandle, BTPM_CONFIGURATION_DEVICE_MANAGER_DEFAULT_SCAN_INTERVAL, BTPM_CONFIGURATION_DEVICE_MANAGER_DEFAULT_SCAN_WINDOW);
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Read Response.                                     */
   /* * NOTE * This function *MUST* be called with the HOG Manager Lock */
   /*          held.                                                    */
static void ProcessGATTReadResponse(GATM_Read_Response_Event_Data_t *ReadResponseData)
{
   Transaction_Info_t *TransactionInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Write Response.                                    */
   /* * NOTE * This function *MUST* be called with the HOG Manager Lock */
   /*          held.                                                    */
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse)
{
   Transaction_Info_t *TransactionInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Error Response.                                    */
   /* * NOTE * This function *MUST* be called with the HOG Manager Lock */
   /*          held.                                                    */
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse)
{
   Transaction_Info_t *TransactionInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HOG Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HOGM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event\n"));
                  break;
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all HOG Manager Messages.   */
static void BTPSAPI HOGManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HOGP_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("HOG Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HOG Manager defined    */
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
               /* HOG Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HOGM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HOG Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HOG Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HOG Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HOG Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HOG Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HOGM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HOG Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process HOG Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HOGP_MANAGER, HOGManagerGroupHandler, NULL))
         {
            /* Initialize the actual HOG Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the HOG Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _HOGM_Initialize()))
            {
               /* Check to see if any initialization data was specified.*/
               if(InitializationData)
                  SupportedFeatures = ((HOGM_Initialization_Data_t *)InitializationData)->SupportedFeaturesFlags;
               else
                  SupportedFeatures = 0;

               /* Register a GATM Event Callback.                       */
               if((Result = GATM_RegisterEventCallback(GATM_Event_Callback, NULL)) > 0)
               {
                  /* Save the GATM Event Callback ID.                   */
                  GATMEventCallbackID     = (unsigned int)Result;

                  /* Determine the current Device Power State.          */
                  CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting HOG Callback ID.     */
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
                  /* Failed to register callback so cleanup the HOG     */
                  /* Manager Implementation Module.                     */
                  _HOGM_Cleanup();
               }
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _HOGM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HOGP_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("HOG Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HOGP_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the HOG Manager Implementation that  */
            /* we are shutting down.                                    */
            _HOGM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the HOG Entry Information List is empty.  */
            FreeHOGEntryInfoList(&HOGEntryInfoList);

            /* Make sure that the HOG Entry Data Information List is    */
            /* empty.                                                   */
            FreeHOGEntryInfoList(&HOGEntryInfoDataList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HOGM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                Result;
   Connection_Entry_t ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the HOG Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _HOGM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the HOG Manager that the Stack has been closed.*/
               _HOGM_SetBluetoothStackID(0);

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Check to see if we stored anything for this device.   */
               if(ReloadHIDInformation(NULL, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress))
               {
                  /* Delete the information that is stored for this     */
                  /* device in the file.                                */
                  BTPS_MemInitialize(&ConnectionEntry, 0, (sizeof(ConnectionEntry)));
                  ConnectionEntry.BD_ADDR = EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress;

                  StoreHIDDeviceToFile(&ConnectionEntry, FALSE);
               }

               /* Process this as a potential HID Device Disconnection. */
               ProcessHIDDeviceDisconnection(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HOG) Manager Service.  This Callback will be dispatched by*/
   /* the HOG Manager when various HOG Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HOG Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HOGM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int BTPSAPI HOGM_Register_Event_Callback(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HOG_Entry_Info_t HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the HOG Entry list.         */
            BTPS_MemInitialize(&HOGEntryInfo, 0, sizeof(HOG_Entry_Info_t));

            HOGEntryInfo.CallbackID         = GetNextCallbackID();
            HOGEntryInfo.ClientID           = MSG_GetServerAddressID();
            HOGEntryInfo.EventCallback      = CallbackFunction;
            HOGEntryInfo.CallbackParameter  = CallbackParameter;
            HOGEntryInfo.Flags              = HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if(AddHOGEntryInfoEntry(&HOGEntryInfoList, &HOGEntryInfo))
               ret_val = HOGEntryInfo.CallbackID;
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

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
      ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HOG Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HOGM_RegisterEventCallback() function).  This function accepts as */
   /* input the HOG Manager Event Callback ID (return value from        */
   /* HOGM_RegisterEventCallback() function).                           */
void BTPSAPI HOGM_Un_Register_Event_Callback(unsigned int HOGManagerCallbackID)
{
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HOGManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoList, HOGManagerCallbackID)) != NULL) && (HOGEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Delete the Callback Entry.                            */
               if((HOGEntryInfo = DeleteHOGEntryInfoEntry(&HOGEntryInfoList, HOGManagerCallbackID)) != NULL)
               {
                  /* Free the memory because we are finished with it.   */
                  FreeHOGEntryInfoEntryMemory(HOGEntryInfo);
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HOG) Manager Service to explicitly process HOG report     */
   /* data.  This Callback will be dispatched by the HOG Manager when   */
   /* various HOG Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HOG Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HOGM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HOGM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HOGM_Register_Data_Event_Callback(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HOG_Entry_Info_t HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a Data Event Handler registered.                 */
            if(!HOGEntryInfoDataList)
            {
               /* First, Register the handler locally.                  */
               BTPS_MemInitialize(&HOGEntryInfo, 0, sizeof(HOG_Entry_Info_t));

               HOGEntryInfo.CallbackID         = GetNextCallbackID();
               HOGEntryInfo.ClientID           = MSG_GetServerAddressID();
               HOGEntryInfo.EventCallback      = CallbackFunction;
               HOGEntryInfo.CallbackParameter  = CallbackParameter;
               HOGEntryInfo.Flags              = HOG_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               if(AddHOGEntryInfoEntry(&HOGEntryInfoDataList, &HOGEntryInfo))
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  ret_val = HOGEntryInfo.CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_DATA_EVENTS_ALREADY_REGISTERED;

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HOG Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HOGM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HOG Manager Data Event Callback ID (return   */
   /* value from HOGM_Register_Data_Event_Callback() function).         */
void BTPSAPI HOGM_Un_Register_Data_Event_Callback(unsigned int HOGManagerDataCallbackID)
{
   HOG_Entry_Info_t *HOGEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* HOG Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HOGManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL) && (HOGEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Delete the local handler.                             */
               if((HOGEntryInfo = DeleteHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL)
               {
                  /* All finished with the entry, delete it.            */
                  FreeHOGEntryInfoEntryMemory(HOGEntryInfo);
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism of setting*/
   /* the HID Protocol Mode on a remote HID Device.  This function      */
   /* accepts as input the HOG Manager Data Event Callback ID (return   */
   /* value from HOGM_Register_Data_Event_Callback() function), the     */
   /* BD_ADDR of the remote HID Device and the Protocol Mode to set.    */
   /* This function returns zero on success or a negative error code.   */
   /* * NOTE * On each connection to a HID Device the Protocol Mode     */
   /*          defaults to Report Mode.                                 */
int BTPSAPI HOGM_Set_Protocol_Mode(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Protocol_Mode_t ProtocolMode)
{
   int                 ret_val;
   HOG_Entry_Info_t   *HOGEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if((HOGManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && ((ProtocolMode == hpmBoot) || (ProtocolMode == hpmReport)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Determine the HOG Callback Information and make sure  */
               /* that it is valid.                                     */
               if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL) && (HOGEntryInfo->ClientID == MSG_GetServerAddressID()))
               {
                  /* Determine the connection entry for the specified   */
                  /* connection and make sure we are done configuring   */
                  /* the device.                                        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
                  {
                     /* Check to see if we actually need to do anything.*/
                     if(ConnectionEntry->CurrentProtocolMode != ProtocolMode)
                     {
                        /* Call the internal function to actually set   */
                        /* the protocol mode.                           */
                        ret_val = HIDSetProtocolMode(ConnectionEntry, HOGEntryInfo, ProtocolMode);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_PROTOCOL_MODE_CURRENT;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

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
      ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* informing the specified remote HID Device that the local HID Host */
   /* is entering/exiting the Suspend State.  This function accepts as  */
   /* input the HOG Manager Data Event Callback ID (return value from   */
   /* HOGM_Register_Data_Event_Callback() function), the BD_ADDR of the */
   /* remote HID Device and the a Boolean that indicates if the Host is */
   /* entering suspend state (TRUE) or exiting suspend state (FALSE).   */
   /* This function returns zero on success or a negative error code.   */
int BTPSAPI HOGM_Set_Suspend_Mode(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Suspend)
{
   int                 ret_val;
   HOG_Entry_Info_t   *HOGEntryInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if((HOGManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Determine the HOG Callback Information and make sure  */
               /* that it is valid.                                     */
               if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL) && (HOGEntryInfo->ClientID == MSG_GetServerAddressID()))
               {
                  /* Determine the connection entry for the specified   */
                  /* connection and make sure we are done configuring   */
                  /* the device.                                        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
                  {
                     /* Call the internal function to set the suspend   */
                     /* mode.                                           */
                     ret_val = HIDSetSuspendMode(ConnectionEntry, Suspend);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

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
      ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Get Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from HOGM_Register_Data_Event_Callback()         */
   /* function), the BD_ADDR of the remote HID Device and a pointer to a*/
   /* structure containing information on the Report to set.  This      */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
   /* * NOTE * The hetHOGMHIDGetReportResponse event will be generated  */
   /*          when the remote HID Device responds to the Get Report    */
   /*          Request.                                                 */
int BTPSAPI HOGM_Get_Report_Request(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation)
{
   int                 ret_val;
   HOG_Entry_Info_t   *HOGEntryInfo;
   HID_Report_Info_t  *ReportInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if((HOGManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ReportInformation) && (ReportInformation->ReportType >= hrtInput) && (ReportInformation->ReportType <= hrtBootMouseInput))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Determine the HOG Callback Information and make sure  */
               /* that it is valid.                                     */
               if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL) && (HOGEntryInfo->ClientID == MSG_GetServerAddressID()))
               {
                  /* Determine the connection entry for the specified   */
                  /* connection and make sure we are done configuring   */
                  /* the device.                                        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
                  {
                     /* Search for the HID Report Info.                 */
                     if((ReportInfo = SearchHIDReportInfoByReportInfo(&(ConnectionEntry->ReportInfoList), ReportInformation)) != NULL)
                     {
                        /* Simply call the internal function to do the  */
                        /* Get Report Request.                          */
                        ret_val = HIDGetReportRequest(ConnectionEntry, HOGEntryInfo, ReportInfo);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_INVALID_REPORT;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

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
      ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Set Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from HOGM_Register_Data_Event_Callback()         */
   /* function), the BD_ADDR of the remote HID Device, a pointer to a   */
   /* structure containing information on the Report to set, a Boolean  */
   /* that indicates if a response is expected, and the Report Data to  */
   /* set.  This function returns the positive, non-zero, Transaction ID*/
   /* of the request (if a Response is expected, ZERO if no response is */
   /* expected) on success or a negative error code.                    */
   /* * NOTE * If a response is expected the hetHOGMHIDSetReportResponse*/
   /*          event will be generated when the remote HID Device       */
   /*          responds to the Set Report Request.                      */
   /* * NOTE * The ResponseExpected parameter can be set to TRUE to     */
   /*          indicate that no response is expected.  This can only be */
   /*          set to TRUE if the Report to set is an Output Report and */
   /*          corresponds to a HID Data Output procedure.              */
   /* * NOTE * The ReportID, if valid for the specified Report, MUST be */
   /*          appended to the data that is passed to this function.    */
int BTPSAPI HOGM_Set_Report_Request(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation, Boolean_t ResponseExpected, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                 ret_val;
   HOG_Entry_Info_t   *HOGEntryInfo;
   HID_Report_Info_t  *ReportInfo;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HOG Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if((HOGManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ReportInformation) && (ReportInformation->ReportType >= hrtInput) && (ReportInformation->ReportType <= hrtBootMouseInput) && (ReportDataLength) && (ReportData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Determine the HOG Callback Information and make sure  */
               /* that it is valid.                                     */
               if(((HOGEntryInfo = SearchHOGEntryInfoEntry(&HOGEntryInfoDataList, HOGManagerDataCallbackID)) != NULL) && (HOGEntryInfo->ClientID == MSG_GetServerAddressID()))
               {
                  /* Determine the connection entry for the specified   */
                  /* connection and make sure we are done configuring   */
                  /* the device.                                        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
                  {
                     /* Search for the HID Report Info.                 */
                     if((ReportInfo = SearchHIDReportInfoByReportInfo(&(ConnectionEntry->ReportInfoList), ReportInformation)) != NULL)
                     {
                        /* Simply call the internal function to do the  */
                        /* Set Report Request.                          */
                        ret_val = HIDSetReportRequest(ConnectionEntry, HOGEntryInfo, ReportInfo, ResponseExpected, ReportDataLength, ReportData);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_INVALID_REPORT;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

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
      ret_val = BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_HID_OVER_GATT | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


/*****< btpmglpm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMGLPM - Glucose Manager for Stonestreet One Bluetooth Protocol Stack   */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"            /* BTPS Protocol Stack Prototypes/Constants.  */
#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */

#include "BTPMGLPM.h"           /* BTPM GLP Manager Prototypes/Constants.     */
#include "GLPMAPI.h"            /* GLP Manager Prototypes/Constants.          */
#include "GLPMMSG.h"            /* BTPM GLP Manager Message Formats.          */
#include "GLPMGR.h"             /* GLP Manager Impl. Prototypes/Constants.    */

#include "SS1BTGLS.h"           /* Bluetooth GLS Service Prototypes/Constants.*/

#include "SS1BTPM.h"            /* BTPM Main Prototypes and Constants.        */
#include "BTPMERR.h"            /* BTPM Error Prototypes/Constants.           */
#include "BTPMCFG.h"            /* BTPM Configuration Settings/Constants.     */

   /* The following defines the GLPM LE Configuration File Section Name.*/
#define GLPM_LE_CONFIGURATION_FILE_SECTION_NAME                "GLPM-Collector"

   /* The following defines the Maximum Key Size that is used in the    */
   /* GLPM LE Configuration File.                                       */
#define GLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH          (4+(BD_ADDR_SIZE*2)+WORD_SIZE+8)

   /* The following defines the GLPM LE Configuration File Maximum Line */
   /* Length.                                                           */
#define GLPM_LE_CONFIGURATION_FILE_MAXIMUM_LINE_LENGTH         ((BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_LINE_LENGTH/2)-GLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH)

   /* The following define the Key Names that are used with the GLPM LE */
   /* Configuration File.                                               */
#define GLPM_LE_KEY_NAME_GLUCOSE_FEATURES_PREFIX               "GF-%02X%02X%02X%02X%02X%02X"

   /* Structure which is used to hold all of the Glucose binary         */
   /* information that is stored to file for a paired Glucose Device.   */
typedef struct _tagGlucose_Information_Entry_t
{
   NonAlignedWord_t SupportedFeatures;
} Glucose_Information_Entry_t;

#define GLUCOSE_INFORMATION_ENTRY_DATA_SIZE                    (sizeof(Glucose_Information_Entry_t))

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagGLP_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ClientID;
   unsigned long                Flags;
   GLPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagGLP_Entry_Info_t *NextGLPEntryInfoPtr;
} GLPM_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* GLP_Entry_Info_t structure to denote various state information.   */
#define GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   GLPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the type of       */
   /* outstanding transactions that are possible.                       */
typedef enum
{
   ttReadGlucoseFeature,
   ttConfigureCCCD,
   ttRACPStartProcedure,
   ttRACPStopProcedure
} Transaction_Type_t;

   /* The following structure is a container structure that contains all*/
   /* of the information on an outstanding transaction.                 */
typedef struct _tagTransaction_Info_t
{
   unsigned int                     ProcedureID;
   unsigned int                     GATMProcedureID;
   unsigned int                     GLPManagerDataCallbackID;
   unsigned int                     TimerID;
   Boolean_t                        AbortActive;
   GLPM_Procedure_Type_t            ProcedureType;
   Transaction_Type_t               TransactionType;
   Word_t                           CharacteristicValueHandle;
   Boolean_t                        MeasurementDataValid;
   GLPM_Glucose_Measurement_Data_t  MeasurementData;
   struct _tagTransaction_Info_t   *NextTransactionInfoPtr;
} Transaction_Info_t;

   /* The following structure is a container structure which contains   */
   /* all of the information on a HID Device.                           */
typedef struct _tagGlucose_Information_t
{
   unsigned long SupportedFeatures;
   Word_t        GlucoseMeasurementHandle;
   Word_t        GlucoseMeasurementCCCDHandle;
   Word_t        GlucoseMeasurementContextHandle;
   Word_t        GlucoseMeasurementContextCCCDHandle;
   Word_t        GlucoseFeaturesHandle;
   Word_t        RACPHandle;
   Word_t        RACPCCCDHandle;
} Glucose_Information_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking incoming connections.                 */
typedef enum
{
   csConfiguring,
   csConnected
} Connection_State_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagGLPM_Connection_Entry_t
{
   BD_ADDR_t                           BD_ADDR;
   Connection_State_t                  ConnectionState;
   unsigned long                       ConnectionFlags;
   Glucose_Information_t               GlucoseInformation;
   Transaction_Info_t                 *TransactionInfoList;
   struct _tagGLPM_Connection_Entry_t *NextConnectionEntryPtr;
} GLPM_Connection_Entry_t;

#define GLPM_CONNECTION_ENTRY_FLAGS_RACP_PROCEDURE_IN_PROGRESS          0x00000001
#define GLPM_CONNECTION_ENTRY_FLAGS_ABORT_RACP_PROCEDURE_IN_PROGRESS    0x00000002

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which is used to hold the next (unique) Transaction ID.  */
static unsigned int ProcedureID;

   /* Variable which holds a pointer to the first element in the GLP    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static GLPM_Entry_Info_t *GLPEntryInfoList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds the GATM Event Callback ID.                  */
static unsigned int GATMEventCallbackID;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static GLPM_Connection_Entry_t *ConnectionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);
static unsigned int GetNextProcedureID(void);

static GLPM_Entry_Info_t *AddGLPEntryInfoEntry(GLPM_Entry_Info_t **ListHead, GLPM_Entry_Info_t *EntryToAdd);
static GLPM_Entry_Info_t *SearchGLPEntryInfoEntry(GLPM_Entry_Info_t **ListHead, unsigned int CallbackID);
static GLPM_Entry_Info_t *DeleteGLPEntryInfoEntry(GLPM_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeGLPEntryInfoEntryMemory(GLPM_Entry_Info_t *EntryToFree);
static void FreeGLPEntryInfoList(GLPM_Entry_Info_t **ListHead);

static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd);
static Transaction_Info_t *SearchTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID);
static Transaction_Info_t *SearchTransactionInfoByGATMProcedureID(Transaction_Info_t **ListHead, unsigned int GATMProcedureID);
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID);
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree);
static void FreeTransactionInfoList(Transaction_Info_t **ListHead);

static GLPM_Connection_Entry_t *AddConnectionEntry(GLPM_Connection_Entry_t **ListHead, GLPM_Connection_Entry_t *EntryToAdd);
static GLPM_Connection_Entry_t *SearchConnectionEntry(GLPM_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static GLPM_Connection_Entry_t *DeleteConnectionEntry(GLPM_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(GLPM_Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(GLPM_Connection_Entry_t **ListHead);

static void DispatchGLPEvent(GLPM_Event_Data_t *GLPMEventData, BTPM_Message_t *Message);

static void DispatchGlucoseConnection(GLPM_Connection_Entry_t *ConnectionEntry);
static void DispatchProcedureStartedEvent(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, Transaction_Info_t *TransactionInfo, Byte_t ErrorCode);
static void DispatchProcedureStoppedEvent(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, Transaction_Info_t *TransactionInfo, GLPM_Procedure_Response_Code_Type_t ResponseCode, unsigned int NumberStoredRecords);
static void DispatchGlucoseMeasurementEvent(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, Transaction_Info_t *TransactionInfo, GLPM_Measurement_Error_Type_t ErrorType, GLPM_Glucose_Measurement_Data_t *MeasurementData, GLPM_Glucose_Measurement_Context_Data_t *MeasurementContextData);

static Boolean_t ReloadGlucoseInformation(GLPM_Connection_Entry_t *ConnectionEntry, BD_ADDR_t RemoteDeviceAddress);
static GLPM_Connection_Entry_t *LoadGlucoseDeviceFromFile(BD_ADDR_t RemoteDeviceAddress);
static void StoreGlucoseInformation(GLPM_Connection_Entry_t *ConnectionEntry, Boolean_t Store);
static void StoreGlucoseDeviceToFile(GLPM_Connection_Entry_t *ConnectionEntry, Boolean_t Store);

static Transaction_Info_t *ConfigureClientConfiguration(GLPM_Connection_Entry_t *ConnectionEntry, Word_t CharacteristicHandle, Word_t CCCDHandle, Boolean_t Indicate);

static int ConvertTimeToGLS(GLPM_Date_Time_Data_t *GLPMTime, GLS_Date_Time_Data_t *GLSTime);
static int ConvertProcedureDataToGLS(GLPM_Procedure_Data_t *ProcedureData, GLS_Record_Access_Control_Point_Format_Data_t *GLSProcedureData);

static void ConvertSFloatToGLPM(Word_t SFLOAT, GLPM_Floating_Point_Data_t *FloatData);

static void ConvertGlucoseMeasurementToGLPM(GLS_Glucose_Measurement_Data_t *GLSMeasurementData, GLPM_Glucose_Measurement_Data_t *GLPMMeasurementData);
static void ConvertGlucoseMeasurementContextToGLPM(GLS_Glucose_Measurement_Context_Data_t *GLSMeasurementContextData, GLPM_Glucose_Measurement_Context_Data_t *GLPMMeasurementContextData);

static int StartProcedureRequest(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, GLPM_Procedure_Data_t *ProcedureData);
static int StopProcedureRequest(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, unsigned int ProcedureID);

static Boolean_t CheckGlucoseServicesSupported(BD_ADDR_t BD_ADDR);
static Boolean_t QueryParsedServicesData(BD_ADDR_t BD_ADDR, DEVM_Parsed_Services_Data_t *ParsedGATTData);

static Boolean_t ProcessGlucoseMeasurementCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessGlucoseMeasurementContextCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessGlucoseFeatureCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessRecordAccessControlPointCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation);
static Boolean_t ProcessConfigureGlucoseService(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *HIDService);
static void ProcessConfigureGlucoseConnection(BD_ADDR_t BD_ADDR);

static void ParseGLPAttributeHandles(GLPM_Connection_Entry_t *ConnectionEntry);
static void ProcessGlucoseSensorConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessGlucoseSensorDisconnection(BD_ADDR_t RemoteDeviceAddress);

static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRegisterCollectorEventsMessage(GLPM_Register_Collector_Events_Request_t *Message);
static void ProcessUnRegisterCollectorEventsMessage(GLPM_Un_Register_Collector_Events_Request_t *Message);
static void ProcessStartProcedureMessage(GLPM_Start_Procedure_Request_t *Message);
static void ProcessStopProcedureMessage(GLPM_Stop_Procedure_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessGATTTransactionResponse(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, Boolean_t ErrorResponse, void *EventData);

static void ProcessGlucoseMeasurementNotification(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessGlucoseMeasurementContextNotification(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessRACPIndication(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);

static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData);
static void ProcessGATTReadResponse(GATM_Read_Response_Event_Data_t *ReadResponseData);
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse);
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse);

static void ProcessProcedureTimeout(unsigned int ProcedureID);

static void BTPSAPI BTPMDistpatchCallback_TMR(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_GLPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static Boolean_t BTPSAPI TimerCallback(unsigned int TimerID, void *CallbackParameter);
static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter);
static void BTPSAPI GLPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the GLP Entry Information List.                              */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Procedure ID that can be used to add an entry   */
   /* into the Procedure Information List.                              */
static unsigned int GetNextProcedureID(void)
{
   unsigned int ret_val;

   ret_val = ProcedureID++;

   if((!ProcedureID) || (ProcedureID & 0x80000000))
      ProcedureID = 0x00000001;

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
static GLPM_Entry_Info_t *AddGLPEntryInfoEntry(GLPM_Entry_Info_t **ListHead, GLPM_Entry_Info_t *EntryToAdd)
{
   GLPM_Entry_Info_t *AddedEntry = NULL;
   GLPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (GLPM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(GLPM_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextGLPEntryInfoPtr = NULL;

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
                     FreeGLPEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextGLPEntryInfoPtr)
                        tmpEntry = tmpEntry->NextGLPEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextGLPEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static GLPM_Entry_Info_t *SearchGLPEntryInfoEntry(GLPM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   GLPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextGLPEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified GLP Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the GLP Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeGLPEntryInfoEntryMemory().                   */
static GLPM_Entry_Info_t *DeleteGLPEntryInfoEntry(GLPM_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   GLPM_Entry_Info_t *FoundEntry = NULL;
   GLPM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextGLPEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextGLPEntryInfoPtr = FoundEntry->NextGLPEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextGLPEntryInfoPtr;

         FoundEntry->NextGLPEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified GLP Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeGLPEntryInfoEntryMemory(GLPM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified GLP Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeGLPEntryInfoList(GLPM_Entry_Info_t **ListHead)
{
   GLPM_Entry_Info_t *EntryToFree;
   GLPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextGLPEntryInfoPtr;

         FreeGLPEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            the ProcedureID field is the same as an entry already  */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static Transaction_Info_t *AddTransactionInfo(Transaction_Info_t **ListHead, Transaction_Info_t *EntryToAdd)
{
   Transaction_Info_t *AddedEntry = NULL;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->ProcedureID)
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
                  if(tmpEntry->ProcedureID == AddedEntry->ProcedureID)
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified ProcedureID.     */
   /* This function returns NULL if either the Transaction Info List    */
   /* Head is invalid, the Procedure ID is invalid, or the specified    */
   /* Entry was NOT present in the list.                                */
static Transaction_Info_t *SearchTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID)
{
   Transaction_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ProcedureID));

   /* Let's make sure the list and GATM Transaction ID to search for    */
   /* appear to be valid.                                               */
   if((ListHead) && (ProcedureID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ProcedureID != ProcedureID))
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified GATM ProcedureID.*/
   /* This function returns NULL if either the Transaction Info List    */
   /* Head is invalid, the GATM Transaction ID is invalid, or the       */
   /* specified Entry was NOT present in the list.                      */
static Transaction_Info_t *SearchTransactionInfoByGATMProcedureID(Transaction_Info_t **ListHead, unsigned int GATMProcedureID)
{
   Transaction_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and GATM Transaction ID to search for    */
   /* appear to be valid.                                               */
   if((ListHead) && (GATMProcedureID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GATMProcedureID != GATMProcedureID))
         FoundEntry = FoundEntry->NextTransactionInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Transaction Info    */
   /* List for the Transaction Info with the specified ProcedureID and  */
   /* removes it from the List.  This function returns NULL if either   */
   /* the Transaction Info List Head is invalid, the Transaction ID is  */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and the*/
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeTransactionInfoMemory().                     */
static Transaction_Info_t *DeleteTransactionInfo(Transaction_Info_t **ListHead, unsigned int ProcedureID)
{
   Transaction_Info_t *FoundEntry = NULL;
   Transaction_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Transaction ID to search for appear  */
   /* to be valid.                                                      */
   if((ListHead) && (ProcedureID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ProcedureID != ProcedureID))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Transaction Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeTransactionInfoMemory(Transaction_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Transaction Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeTransactionInfoList(Transaction_Info_t **ListHead)
{
   Transaction_Info_t *EntryToFree;
   Transaction_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
static GLPM_Connection_Entry_t *AddConnectionEntry(GLPM_Connection_Entry_t **ListHead, GLPM_Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t           NULL_BD_ADDR;
   GLPM_Connection_Entry_t *AddedEntry = NULL;
   GLPM_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
         AddedEntry = (GLPM_Connection_Entry_t *)BTPS_AllocateMemory(sizeof(GLPM_Connection_Entry_t));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* Bluetooth Device Address.  This function returns NULL if either   */
   /* the Connection Entry List Head is invalid, the Bluetooth Device   */
   /* Address is invalid, or the specified Entry was NOT present in the */
   /* list.                                                             */
static GLPM_Connection_Entry_t *SearchConnectionEntry(GLPM_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t           NULL_BD_ADDR;
   GLPM_Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
static GLPM_Connection_Entry_t *DeleteConnectionEntry(GLPM_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t           NULL_BD_ADDR;
   GLPM_Connection_Entry_t *FoundEntry = NULL;
   GLPM_Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(GLPM_Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
   {
      /* Make sure the transaction list is empty for this connection.   */
      if(EntryToFree->TransactionInfoList)
         FreeTransactionInfoList(&(EntryToFree->TransactionInfoList));

      BTPS_FreeMemory(EntryToFree);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(GLPM_Connection_Entry_t **ListHead)
{
   GLPM_Connection_Entry_t *EntryToFree;
   GLPM_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified GLP event to every registered GLP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the GLP Manager Lock */
   /*          held.  Upon exit from this function it will free the GLP */
   /*          Manager Lock.                                            */
static void DispatchGLPEvent(GLPM_Event_Data_t *GLPMEventData, BTPM_Message_t *Message)
{
   unsigned int       Index;
   unsigned int       Index1;
   unsigned int       ServerID;
   unsigned int       NumberCallbacks;
   Callback_Info_t    CallbackInfoArray[16];
   Callback_Info_t   *CallbackInfoArrayPtr;
   GLPM_Entry_Info_t *GLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((GLPEntryInfoList) && (GLPMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      GLPEntryInfo    = GLPEntryInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(GLPEntryInfo)
      {
         if(((GLPEntryInfo->EventCallback) || (GLPEntryInfo->ClientID != ServerID)) && (GLPEntryInfo->Flags & GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         GLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;
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
            GLPEntryInfo    = GLPEntryInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(GLPEntryInfo)
            {
               if(((GLPEntryInfo->EventCallback) || (GLPEntryInfo->ClientID != ServerID)) && (GLPEntryInfo->Flags & GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = GLPEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = GLPEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = GLPEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               GLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;
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
               /*          for GLP events and Data Events.              */
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(GLPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Glucose Connection Event.                              */
static void DispatchGlucoseConnection(GLPM_Connection_Entry_t *ConnectionEntry)
{
   GLPM_Event_Data_t        GLPMEventData;
   GLPM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(ConnectionEntry)
   {
      /* Make sure that we flag that this device is connected.          */
      ConnectionEntry->ConnectionState = csConnected;

      /* Next, format up the Event to dispatch.                         */
      GLPMEventData.EventType                                        = getGLPMConnected;
      GLPMEventData.EventLength                                      = GLPM_CONNECTED_EVENT_DATA_SIZE;

      GLPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
      GLPMEventData.EventData.ConnectedEventData.ConnectionType      = gctSensor;
      GLPMEventData.EventData.ConnectedEventData.SupportedFeatures   = ConnectionEntry->GlucoseInformation.SupportedFeatures;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, GLPM_CONNECTED_MESSAGE_SIZE);

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
      Message.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (GLPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.ConnectionType                = gctSensor;
      Message.SupportedFeatures             = ConnectionEntry->GlucoseInformation.SupportedFeatures;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchGLPEvent(&GLPMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Procedure Started Event.                               */
static void DispatchProcedureStartedEvent(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, Transaction_Info_t *TransactionInfo, Byte_t ErrorCode)
{
   void                             *CallbackParameter;
   GLPM_Event_Data_t                 GLPMEventData;
   GLPM_Event_Callback_t             EventCallback;
   GLPM_Procedure_Started_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (TransactionInfo))
   {
      /* Check to see where the event should be dispatched to.          */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         GLPMEventData.EventType                                                      = getGLPMProcedureStarted;
         GLPMEventData.EventLength                                                    = GLPM_PROCEDURE_STARTED_EVENT_DATA_SIZE;

         GLPMEventData.EventData.ProcedureStartedEventData.RemoteDeviceAddress        = ConnectionEntry->BD_ADDR;
         GLPMEventData.EventData.ProcedureStartedEventData.ProcedureID                = TransactionInfo->ProcedureID;
         GLPMEventData.EventData.ProcedureStartedEventData.Success                    = (Boolean_t)(ErrorCode?FALSE:TRUE);
         GLPMEventData.EventData.ProcedureStartedEventData.AttributeProtocolErrorCode = ErrorCode;

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&GLPMEventData, CallbackParameter);
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
         BTPS_MemInitialize(&Message, 0, (sizeof(Message)));

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
         Message.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_PROCEDURE_STARTED_EVENT;
         Message.MessageHeader.MessageLength   = (GLPM_PROCEDURE_STARTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ProcedureID                   = TransactionInfo->ProcedureID;
         Message.Success                       = (Boolean_t)(ErrorCode?FALSE:TRUE);
         Message.AttributeProtocolErrorCode    = ErrorCode;

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Procedure Started Event.                               */
static void DispatchProcedureStoppedEvent(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, Transaction_Info_t *TransactionInfo, GLPM_Procedure_Response_Code_Type_t ResponseCode, unsigned int NumberStoredRecords)
{
   void                             *CallbackParameter;
   GLPM_Event_Data_t                 GLPMEventData;
   GLPM_Event_Callback_t             EventCallback;
   GLPM_Procedure_Stopped_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (TransactionInfo))
   {
      /* Check to see where the event should be dispatched to.          */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         GLPMEventData.EventType                                               = getGLPMProcedureStopped;
         GLPMEventData.EventLength                                             = GLPM_PROCEDURE_STOPPED_EVENT_DATA_SIZE;

         GLPMEventData.EventData.ProcedureStoppedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         GLPMEventData.EventData.ProcedureStoppedEventData.ProcedureID         = TransactionInfo->ProcedureID;
         GLPMEventData.EventData.ProcedureStoppedEventData.ResponseCode        = ResponseCode;
         GLPMEventData.EventData.ProcedureStoppedEventData.NumberStoredRecords = NumberStoredRecords;

         if(TransactionInfo->AbortActive)
            GLPMEventData.EventData.ProcedureStoppedEventData.ProcedureType    = gptAbortProcedure;
         else
            GLPMEventData.EventData.ProcedureStoppedEventData.ProcedureType    = TransactionInfo->ProcedureType;

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&GLPMEventData, CallbackParameter);
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
         BTPS_MemInitialize(&Message, 0, (sizeof(Message)));

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
         Message.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_PROCEDURE_STOPPED_EVENT;
         Message.MessageHeader.MessageLength   = (GLPM_PROCEDURE_STOPPED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ProcedureID                   = TransactionInfo->ProcedureID;
         Message.ResponseCode                  = ResponseCode;
         Message.NumberStoredRecords           = NumberStoredRecords;

         if(TransactionInfo->AbortActive)
            Message.ProcedureType              = gptAbortProcedure;
         else
            Message.ProcedureType              = TransactionInfo->ProcedureType;

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Glucose Measurement Event.                             */
static void DispatchGlucoseMeasurementEvent(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, Transaction_Info_t *TransactionInfo, GLPM_Measurement_Error_Type_t ErrorType, GLPM_Glucose_Measurement_Data_t *MeasurementData, GLPM_Glucose_Measurement_Context_Data_t *MeasurementContextData)
{
   void                               *CallbackParameter;
   unsigned long                       Flags;
   GLPM_Event_Data_t                   GLPMEventData;
   GLPM_Event_Callback_t               EventCallback;
   GLPM_Glucose_Measurement_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (TransactionInfo))
   {
      /* Determine the flags.                                           */
      Flags = 0;

      if(MeasurementData)
         Flags |= GLPM_MEASUREMENT_FLAGS_GLUCOSE_MEASUREMENT_DATA_PRESENT;

      if(MeasurementContextData)
         Flags |= GLPM_MEASUREMENT_FLAGS_GLUCOSE_MEASUREMENT_CONTEXT_DATA_PRESENT;

      /* Check to see where the event should be dispatched to.          */
      if(EventCallbackInfo->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         BTPS_MemInitialize(&GLPMEventData, 0, sizeof(GLPMEventData));

         GLPMEventData.EventType                                                  = getGLPMGlucoseMeasurement;
         GLPMEventData.EventLength                                                = GLPM_GLUCOSE_MEASUREMENT_EVENT_DATA_SIZE;

         GLPMEventData.EventData.GlucoseMeasurementEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
         GLPMEventData.EventData.GlucoseMeasurementEventData.ProcedureID          = TransactionInfo->ProcedureID;
         GLPMEventData.EventData.GlucoseMeasurementEventData.MeasurementFlags     = Flags;
         GLPMEventData.EventData.GlucoseMeasurementEventData.MeasurementErrorType = ErrorType;

         if(MeasurementData)
            GLPMEventData.EventData.GlucoseMeasurementEventData.GlucoseMeasurementData = *MeasurementData;

         if(MeasurementContextData)
            GLPMEventData.EventData.GlucoseMeasurementEventData.GlucoseMeasurementContextData = *MeasurementContextData;

         /* Save the Event Callback Information.                        */
         EventCallback     = EventCallbackInfo->EventCallback;
         CallbackParameter = EventCallbackInfo->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&GLPMEventData, CallbackParameter);
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
         BTPS_MemInitialize(&Message, 0, (sizeof(Message)));

         Message.MessageHeader.AddressID       = EventCallbackInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
         Message.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_GLUCOSE_MEASUREMENT_EVENT;
         Message.MessageHeader.MessageLength   = (GLPM_GLUCOSE_MEASUREMENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ProcedureID                   = TransactionInfo->ProcedureID;
         Message.MeasurementFlags              = Flags;
         Message.MeasurementErrorType          = ErrorType;

         if(MeasurementData)
            Message.GlucoseMeasurementData = *MeasurementData;

         if(MeasurementContextData)
            Message.GlucoseMeasurementContextData = *MeasurementContextData;

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* reload the Glucose Information for the specified device.  This    */
   /* device will load the Glucose Information into ConnectionEntry (if */
   /* specified) and return TRUE if the Glucose Information was present */
   /* for the device.                                                   */
static Boolean_t ReloadGlucoseInformation(GLPM_Connection_Entry_t *ConnectionEntry, BD_ADDR_t RemoteDeviceAddress)
{
   char                        KeyName[GLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Boolean_t                   ret_val = FALSE;
   Glucose_Information_Entry_t GlucoseInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Build the Key Name.                                            */
      sprintf(KeyName, GLPM_LE_KEY_NAME_GLUCOSE_FEATURES_PREFIX, RemoteDeviceAddress.BD_ADDR5, RemoteDeviceAddress.BD_ADDR4, RemoteDeviceAddress.BD_ADDR3, RemoteDeviceAddress.BD_ADDR2, RemoteDeviceAddress.BD_ADDR1, RemoteDeviceAddress.BD_ADDR0);

      /* Check to see if the Glucose Information is present for this    */
      /* device.                                                        */
      if(SET_ReadBinaryData(GLPM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (Byte_t *)&GlucoseInformation, GLUCOSE_INFORMATION_ENTRY_DATA_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME) == GLUCOSE_INFORMATION_ENTRY_DATA_SIZE)
      {
         /* Return TRUE since the Glucose Features key is present for   */
         /* this device.                                                */
         ret_val = TRUE;

         /* Check to see if we need to actually reload the Glucose      */
         /* Information.                                                */
         if(ConnectionEntry)
         {
            /* Reload the Glucose Information.                          */
            ConnectionEntry->GlucoseInformation.SupportedFeatures = (unsigned long)READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(GlucoseInformation.SupportedFeatures));
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that attempts to load*/
   /* a Glucose Device from the LE configuration file.                  */
static GLPM_Connection_Entry_t *LoadGlucoseDeviceFromFile(BD_ADDR_t RemoteDeviceAddress)
{
   GLPM_Connection_Entry_t  ConnectionEntry;
   GLPM_Connection_Entry_t *ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
   {
      /* Verify that the correct services are supported by this to be a */
      /* potential Glucose Device.                                      */
      if(CheckGlucoseServicesSupported(RemoteDeviceAddress))
      {
         /* First lets verify that the Glucose Information is present in*/
         /* the configuration file.                                     */
         if(ReloadGlucoseInformation(NULL, RemoteDeviceAddress))
         {
            /* Glucose Information is present so lets go ahead and      */
            /* create a connection entry and add it to the list.        */
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(GLPM_Connection_Entry_t));

            ConnectionEntry.ConnectionState     = csConnected;
            ConnectionEntry.BD_ADDR             = RemoteDeviceAddress;

            /* Add the Connection Info to the Connection List.          */
            if((ret_val = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
            {
               /* Attempt to reload the Glucose Information.            */
               if(!ReloadGlucoseInformation(ret_val, ret_val->BD_ADDR))
               {
                  /* Failed to reload the Glucose Information so just   */
                  /* delete it from the list.                           */
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* store/delete Glucose Information for the specified device to file.*/
static void StoreGlucoseInformation(GLPM_Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   char                        KeyName[GLPM_LE_CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];
   Glucose_Information_Entry_t GlucoseInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Build the Key Name.                                            */
      sprintf(KeyName, GLPM_LE_KEY_NAME_GLUCOSE_FEATURES_PREFIX, ConnectionEntry->BD_ADDR.BD_ADDR5, ConnectionEntry->BD_ADDR.BD_ADDR4, ConnectionEntry->BD_ADDR.BD_ADDR3, ConnectionEntry->BD_ADDR.BD_ADDR2, ConnectionEntry->BD_ADDR.BD_ADDR1, ConnectionEntry->BD_ADDR.BD_ADDR0);

      /* See if we are deleting or storing the Glucose Information.     */
      if(Store)
      {
         /* Format the Glucose Information.                             */
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(GlucoseInformation.SupportedFeatures), ConnectionEntry->GlucoseInformation.SupportedFeatures);

         /* Write the Key.                                              */
         SET_WriteBinaryData(GLPM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, (Byte_t *)&GlucoseInformation, GLUCOSE_INFORMATION_ENTRY_DATA_SIZE, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
      else
      {
         /* Delete this Report Descriptor Line.                         */
         SET_WriteBinaryData(GLPM_LE_CONFIGURATION_FILE_SECTION_NAME, KeyName, NULL, 0, BTPM_CONFIGURATION_LOW_ENERGY_CONFIGURATION_FILE_NAME);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to store */
   /* a Glucose Device to the LE Configuration File.                    */
static void StoreGlucoseDeviceToFile(GLPM_Connection_Entry_t *ConnectionEntry, Boolean_t Store)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ConnectionEntry)
   {
      /* Store/Delete the Glucose Features.                             */
      StoreGlucoseInformation(ConnectionEntry, Store);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* configure a Client Characteristic Configuration Descriptor for the*/
   /* specified operation.                                              */
static Transaction_Info_t *ConfigureClientConfiguration(GLPM_Connection_Entry_t *ConnectionEntry, Word_t CharacteristicHandle, Word_t CCCDHandle, Boolean_t Indicate)
{
   int                 Result;
   NonAlignedWord_t    CCCDValue;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicHandle) && (CCCDHandle))
   {
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

      TransactionInfo.ProcedureID               = GetNextProcedureID();
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
            TransactionInfoPtr->GATMProcedureID = (unsigned int)Result;
         }
         else
         {
            /* Delete the transaction information memory and free the   */
            /* memory.                                                  */
            if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->ProcedureID)) != NULL)
            {
               FreeTransactionInfoMemory(TransactionInfoPtr);

               TransactionInfoPtr = NULL;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));

   return(TransactionInfoPtr);
}

   /* The following function is a utility function that is used to      */
   /* convert a GLPM Time Structure to a GLS Time Structure.            */
static int ConvertTimeToGLS(GLPM_Date_Time_Data_t *GLPMTime, GLS_Date_Time_Data_t *GLSTime)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((GLPMTime) && (GLSTime) && ((GLPMTime->Year == 0) || ((GLPMTime->Year >= 1582) && (GLPMTime->Year <= 9999))))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Date Time: %u/%u/%u %u:%u:%u\n", GLPMTime->Month,
                                                                                                             GLPMTime->Day,
                                                                                                             GLPMTime->Year,
                                                                                                             GLPMTime->Hours,
                                                                                                             GLPMTime->Minutes,
                                                                                                             GLPMTime->Seconds));

      /* Do the necessary conversions.                                  */
      GLSTime->Year = (Word_t)GLPMTime->Year;

      /* Next attempt to convert the month.                             */
      if(GLPMTime->Month <= 12)
      {
         GLSTime->Month = (Byte_t)GLPMTime->Month;

         /* Next attempt to convert the day.                            */
         if(GLPMTime->Day <= 31)
         {
            GLSTime->Day = (Byte_t)GLPMTime->Day;

            /* Next attempt to convert the Hours.                       */
            if(GLPMTime->Hours <= 23)
            {
               GLSTime->Hours = (Byte_t)GLPMTime->Hours;

               /* Next attempt to convert the Minutes.                  */
               if(GLPMTime->Minutes <= 59)
               {
                  GLSTime->Minutes = (Byte_t)GLPMTime->Minutes;

                  /* Next attempt to convert the Seconds.               */
                  if(GLPMTime->Seconds <= 59)
                  {
                     GLSTime->Seconds = (Byte_t)GLPMTime->Seconds;

                     ret_val          = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert GLPM Procedure Data to the GLS Procedure Data.            */
static int ConvertProcedureDataToGLS(GLPM_Procedure_Data_t *ProcedureData, GLS_Record_Access_Control_Point_Format_Data_t *GLSProcedureData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ProcedureData) && (GLSProcedureData))
   {
      /* Initialize success for returning to the caller.                */
      ret_val = 0;

      /* First convert the command type.                                */
      switch(ProcedureData->CommandType)
      {
         case gptReportStoredRecords:
            GLSProcedureData->CommandType = racReportStoredRecordsRequest;
            break;
         case gptDeleteStoredRecords:
            GLSProcedureData->CommandType = racDeleteStoredRecordsRequest;
            break;
         case gptAbortProcedure:
            GLSProcedureData->CommandType = racAbortOperationRequest;
            break;
         case gptReportNumberStoredRecords:
            GLSProcedureData->CommandType = racNumberOfStoredRecordsRequest;
            break;
         default:
            ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
            break;
      }

      /* Continue only if no error occurred.                            */
      if(!ret_val)
      {
         /* Next convert the operator type.                             */
         switch(ProcedureData->OperatorType)
         {
            case gotAllRecords:
               GLSProcedureData->OperatorType = raoAllRecords;
               break;
            case gotLessThanOrEqualTo:
               GLSProcedureData->OperatorType = raoLessThanOrEqualTo;
               break;
            case gotGreaterThanOrEqualTo:
               GLSProcedureData->OperatorType = raoGreaterThanOrEqualTo;
               break;
            case gotWithinRangeOf:
               GLSProcedureData->OperatorType = raoWithinRangeOf;
               break;
            case gotFirstRecord:
               GLSProcedureData->OperatorType = raoFirstRecord;
               break;
            case gotLastRecord:
               GLSProcedureData->OperatorType = raoLastRecord;
               break;
            default:
               ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
               break;
         }

         /* Continue only if no error occurred.                         */
         if((!ret_val) && (ProcedureData->OperatorType != gotAllRecords) && (ProcedureData->OperatorType != gotFirstRecord) && (ProcedureData->OperatorType != gotLastRecord))
         {
            /* Next convert the filter type.                            */
            switch(ProcedureData->FilterType)
            {
               case gftSequenceNumber:
                  GLSProcedureData->FilterType = rafSequenceNumber;
                  break;
               case gftUserFacingTime:
                  GLSProcedureData->FilterType = rafUserFacingTime;
                  break;
               default:
                  ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
                  break;
            }

            /* Continue only if no error occurred.                      */
            if(!ret_val)
            {
               /* Now convert the filter operands.                      */
               if(ProcedureData->FilterType == gftSequenceNumber)
               {
                  /* Convert the sequence number range.                 */
                  switch(ProcedureData->OperatorType)
                  {
                     case gotLessThanOrEqualTo:
                     case gotGreaterThanOrEqualTo:
                        if(ProcedureData->FilterParameters.SequenceNumber <= GLPM_MAXIMUM_SEQUENCE_NUMBER)
                           GLSProcedureData->FilterParameters.SequenceNumber = (Word_t)ProcedureData->FilterParameters.SequenceNumber;
                        else
                           ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
                        break;
                     case gotWithinRangeOf:
                        if((ProcedureData->FilterParameters.SequenceNumberRange.MinimumSequenceNumber <= GLPM_MAXIMUM_SEQUENCE_NUMBER) && (ProcedureData->FilterParameters.SequenceNumberRange.MaximumSequenceNumber <= GLPM_MAXIMUM_SEQUENCE_NUMBER) && (ProcedureData->FilterParameters.SequenceNumberRange.MinimumSequenceNumber <= ProcedureData->FilterParameters.SequenceNumberRange.MaximumSequenceNumber))
                        {
                           GLSProcedureData->FilterParameters.SequenceNumberRange.Minimum = (Word_t)ProcedureData->FilterParameters.SequenceNumberRange.MinimumSequenceNumber;
                           GLSProcedureData->FilterParameters.SequenceNumberRange.Maximum = (Word_t)ProcedureData->FilterParameters.SequenceNumberRange.MaximumSequenceNumber;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
                        break;
                     default:
                        break;
                  }
               }
               else
               {
                  /* Convert the sequence number range.                 */
                  switch(ProcedureData->OperatorType)
                  {
                     case gotLessThanOrEqualTo:
                     case gotGreaterThanOrEqualTo:
                        if(ConvertTimeToGLS(&(ProcedureData->FilterParameters.UserFacingTime), &(GLSProcedureData->FilterParameters.UserFacingTime)))
                           ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
                        break;
                     case gotWithinRangeOf:
                        if(!ConvertTimeToGLS(&(ProcedureData->FilterParameters.UserFacingTimeRange.MinimumDateTime), &(GLSProcedureData->FilterParameters.UserFacingTimeRange.Minimum)))
                        {
                           if(ConvertTimeToGLS(&(ProcedureData->FilterParameters.UserFacingTimeRange.MaximumDateTime), &(GLSProcedureData->FilterParameters.UserFacingTimeRange.Maximum)))
                              ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;
                        break;
                     default:
                        break;
                  }
               }
            }
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* convert a SFLOAT to the GLPM structure.                           */
static void ConvertSFloatToGLPM(Word_t SFLOAT, GLPM_Floating_Point_Data_t *FloatData)
{
   SByte_t Exponent;
   SWord_t Mantissa;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if(FloatData)
   {
      /* Check for any special values.                                  */
      if(SFLOAT == 0x07FF)
         FloatData->ValueType = gftNotANumber;
      else
      {
         if(SFLOAT == 0x0800)
            FloatData->ValueType = gftNotAtThisResolution;
         else
         {
            if(SFLOAT == 0x07FE)
               FloatData->ValueType = gftPositiveInfinity;
            else
            {
               if(SFLOAT == 0x0802)
                  FloatData->ValueType = gftNegativeInfinity;
               else
               {
                  if(SFLOAT == 0x0801)
                     FloatData->ValueType = gftRFU;
                  else
                     FloatData->ValueType = gftValid;
               }
            }
         }
      }

      /* If the value is not a special type convert it.                 */
      if(FloatData->ValueType == gftValid)
      {
         /* Get the components.                                         */
         Exponent = (SByte_t)((SFLOAT & 0xF000) >> 12);
         Mantissa = (SWord_t)(SFLOAT & 0x0FFF);

         /* Sign extend the values as necessary.                        */
         if(Exponent & 0x08)
            Exponent |= 0xF0;

         if(Mantissa & 0x0800)
            Mantissa |= 0xF000;

         /* Assign the values into the structure.                       */
         FloatData->Mantissa = (int)Mantissa;
         FloatData->Exponent = (int)Exponent;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* convert GLS Measurement Data to GLPM Measurement Data.            */
static void ConvertGlucoseMeasurementToGLPM(GLS_Glucose_Measurement_Data_t *GLSMeasurementData, GLPM_Glucose_Measurement_Data_t *GLPMMeasurementData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((GLSMeasurementData) && (GLPMMeasurementData))
   {
      /* Initialize the GLPM Measurement Data.                          */
      BTPS_MemInitialize(GLPMMeasurementData, 0, sizeof(GLPMMeasurementData));

      /* Convert the sequence number and date time which are mandatory. */
      GLPMMeasurementData->SequenceNumber   = (unsigned int)GLSMeasurementData->SequenceNumber;
      GLPMMeasurementData->BaseTime.Month   = (unsigned int)GLSMeasurementData->BaseTime.Month;
      GLPMMeasurementData->BaseTime.Day     = (unsigned int)GLSMeasurementData->BaseTime.Day;
      GLPMMeasurementData->BaseTime.Year    = (unsigned int)GLSMeasurementData->BaseTime.Year;
      GLPMMeasurementData->BaseTime.Hours   = (unsigned int)GLSMeasurementData->BaseTime.Hours;
      GLPMMeasurementData->BaseTime.Minutes = (unsigned int)GLSMeasurementData->BaseTime.Minutes;
      GLPMMeasurementData->BaseTime.Seconds = (unsigned int)GLSMeasurementData->BaseTime.Seconds;

      /* Now Convert the optional data.                                 */

      /* Check to see if the Time Offset is present.                    */
      if(GLSMeasurementData->OptionFlags & GLS_MEASUREMENT_FLAGS_TIME_OFFSET_PRESENT)
      {
         GLPMMeasurementData->MeasurementFlags |= GLPM_MEASUREMENT_FLAGS_TIME_OFFSET_PRESENT;

         GLPMMeasurementData->TimeOffset        = (unsigned int)GLSMeasurementData->TimeOffset;

         if(GLPMMeasurementData->TimeOffset == GLS_TIME_OFFSET_VALUE_OVERRUN)
            GLPMMeasurementData->MeasurementFlags |= GLPM_MEASUREMENT_FLAGS_TIME_OFFSET_OVERRUN;
         else
         {
            if(GLPMMeasurementData->TimeOffset == GLS_TIME_OFFSET_VALUE_UNDERRUN)
               GLPMMeasurementData->MeasurementFlags |= GLPM_MEASUREMENT_FLAGS_TIME_OFFSET_UNDERRUN;
         }
      }

      /* Check to see if the Sensor Status Annunciation is present.     */
      if(GLSMeasurementData->OptionFlags & GLS_MEASUREMENT_FLAGS_SENSOR_STATUS_ANNUNCIATION_PRESENT)
      {
         GLPMMeasurementData->MeasurementFlags  |= GLPM_MEASUREMENT_FLAGS_SENSOR_STATUS_PRESENT;

         GLPMMeasurementData->SensorStatusFlags  = GLSMeasurementData->SensorStatus;
      }

      /* Check to see if the Glucose Concentration is present.          */
      if(GLSMeasurementData->OptionFlags & GLS_MEASUREMENT_FLAGS_CONCENTRATION_AND_TYPE_SAMPLE_LOCATION_PRESENT)
      {
         GLPMMeasurementData->MeasurementFlags |= GLPM_MEASUREMENT_FLAGS_CONCENTRATION_PRESENT;

         /* Check to see what the units of Glucose Concentration are.   */
         if(GLSMeasurementData->OptionFlags & GLS_MEASUREMENT_FLAGS_CONCENTRATION_IN_MOL_PER_LITER)
            GLPMMeasurementData->MeasurementFlags |= GLPM_MEASUREMENT_FLAGS_CONCENTRATION_IN_MOL_PER_LITER;

         /* Convert the SFLOAT Concentration to a                       */
         /* GLPM_Floating_Point_Data_t structure.                       */
         ConvertSFloatToGLPM(GLSMeasurementData->GlucoseConcentration.Value, &(GLPMMeasurementData->Concentration.Value));

         /* Convert the Concentration Type.                             */
         switch(GLSMeasurementData->GlucoseConcentration.Type)
         {
            case GLS_TYPE_CAPILLARY_WHOLE_BLOOD:
               GLPMMeasurementData->Concentration.MeasurementType = gmtCapillaryWholeBlood;
               break;
            case GLS_TYPE_CAPILLARY_PLASMA:
               GLPMMeasurementData->Concentration.MeasurementType = gmtCapillaryPlasma;
               break;
            case GLS_TYPE_VENOUS_WHOLE_BLOOD:
               GLPMMeasurementData->Concentration.MeasurementType = gmtVenousWholeBlood;
               break;
            case GLS_TYPE_VENOUS_PLASMA:
               GLPMMeasurementData->Concentration.MeasurementType = gmtVenousPlasma;
               break;
            case GLS_TYPE_ARTERIAL_WHOLE_BLOOD:
               GLPMMeasurementData->Concentration.MeasurementType = gmtArterialWholeBlood;
               break;
            case GLS_TYPE_ARTERIAL_PLASMA:
               GLPMMeasurementData->Concentration.MeasurementType = gmtArterialPlasma;
               break;
            case GLS_TYPE_UNDETERMINED_WHOLE_BLOOD:
               GLPMMeasurementData->Concentration.MeasurementType = gmtUndeterminedWholeBlood;
               break;
            case GLS_TYPE_UNDETERMINED_PLASMA:
               GLPMMeasurementData->Concentration.MeasurementType = gmtUndeterminedPlasma;
               break;
            case GLS_TYPE_INTERSTITIAL_FLUID:
               GLPMMeasurementData->Concentration.MeasurementType = gmtInterstitialFluid;
               break;
            case GLS_TYPE_CONTROL_SOLUTION:
               GLPMMeasurementData->Concentration.MeasurementType = gmtControlSolution;
               break;
            default:
               GLPMMeasurementData->Concentration.MeasurementType = gmtUnknown;
               break;
         }

         /* Convert the Sample Location.                                */
         switch(GLSMeasurementData->GlucoseConcentration.SampleLocation)
         {
            case GLS_SAMPLE_LOCATION_FINGER:
               GLPMMeasurementData->Concentration.SampleLocationType = gstFinger;
               break;
            case GLS_SAMPLE_LOCATION_ALTERNATE_SITE_TEST:
               GLPMMeasurementData->Concentration.SampleLocationType = gstAlternateSiteTest;
               break;
            case GLS_SAMPLE_LOCATION_EARLOBE:
               GLPMMeasurementData->Concentration.SampleLocationType = gstEarlobe;
               break;
            case GLS_SAMPLE_LOCATION_CONTROL_SOLUTION:
               GLPMMeasurementData->Concentration.SampleLocationType = gstControlSolution;
               break;
            case GLS_SAMPLE_LOCATION_NOT_AVAILABLE:
               GLPMMeasurementData->Concentration.SampleLocationType = gstLocationNotAvailable;
               break;
            default:
               GLPMMeasurementData->Concentration.SampleLocationType = gstUnknown;
               break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* convert GLS Measurement Context Data to GLPM Measurement Context  */
   /* Data.                                                             */
static void ConvertGlucoseMeasurementContextToGLPM(GLS_Glucose_Measurement_Context_Data_t *GLSMeasurementContextData, GLPM_Glucose_Measurement_Context_Data_t *GLPMMeasurementContextData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((GLSMeasurementContextData) && (GLPMMeasurementContextData))
   {
      /* Initialize the GLPM Measurement Context Data.                  */
      BTPS_MemInitialize(GLPMMeasurementContextData, 0, sizeof(GLPMMeasurementContextData));

      /* Convert the mandatory sequence number field first.             */
      GLPMMeasurementContextData->SequenceNumber = GLSMeasurementContextData->SequenceNumber;

      /* Next check to see if the extended flags are present.           */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_EXTENDED_FLAGS_PRESENT)
      {
         /* Flag that the extended flags is present.                    */
         GLPMMeasurementContextData->ContextFlags  |= GLPM_CONTEXT_FLAGS_EXTENDED_FLAGS_PRESENT;

         /* Simply do a conversion of the extended flags.               */
         GLPMMeasurementContextData->ExtendedFlags  = (unsigned int)GLSMeasurementContextData->ExtendedFlags;
      }

      /* Next check to see if the optional Carbohydrate information is  */
      /* present.                                                       */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_CARBOHYDRATE_PRESENT)
      {
         /* Flag that the carbohydrate information is present.          */
         GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_CARBOHYDRATE_DATA_PRESENT;

         /* Convert the Carbohydrate SFLOAT value.                      */
         ConvertSFloatToGLPM(GLSMeasurementContextData->Carbohydrate.Value, &(GLPMMeasurementContextData->CarbohydrateData.CarbohydrateValue));

         /* Convert the Carbohydrate ID.                                */
         switch(GLSMeasurementContextData->Carbohydrate.ID)
         {
            case GLS_CARBOHYDRATE_FIELD_BREAKFAST:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciBreakfast;
               break;
            case GLS_CARBOHYDRATE_FIELD_LUNCH:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciLunch;
               break;
            case GLS_CARBOHYDRATE_FIELD_DINNER:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciDinner;
               break;
            case GLS_CARBOHYDRATE_FIELD_SNACK:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciSnack;
               break;
            case GLS_CARBOHYDRATE_FIELD_DRINK:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciDrink;
               break;
            case GLS_CARBOHYDRATE_FIELD_SUPPER:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciSupper;
               break;
            case GLS_CARBOHYDRATE_FIELD_BRUNCH:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciBrunch;
               break;
            default:
               GLPMMeasurementContextData->CarbohydrateData.CarbohydrateID = gciUnknown;
               break;
         }
      }

      /* Next check to see if the optional Meal type information is     */
      /* present.                                                       */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEAL_PRESENT)
      {
         /* Flag that the meal information is present.                  */
         GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_MEAL_TYPE_PRESENT;

         /* Convert the Meal Type.                                      */
         switch(GLSMeasurementContextData->Meal)
         {
            case GLS_MEAL_FIELD_PREPRANDIAL:
               GLPMMeasurementContextData->MealType = gmtPreprandial;
               break;
            case GLS_MEAL_FIELD_POSTPRANDIAL:
               GLPMMeasurementContextData->MealType = gmtPostprandial;
               break;
            case GLS_MEAL_FIELD_FASTING:
               GLPMMeasurementContextData->MealType = gmtFasting;
               break;
            case GLS_MEAL_FIELD_CASUAL:
               GLPMMeasurementContextData->MealType = gmtCasual;
               break;
            case GLS_MEAL_FIELD_BEDTIME:
               GLPMMeasurementContextData->MealType = gmtBedtime;
               break;
            default:
               GLPMMeasurementContextData->MealType = gmtUnknownMeal;
               break;
         }
      }

      /* Next check to see if the optional Tester Type/Health Type      */
      /* information is present.                                        */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_TESTER_HEALTH_PRESENT)
      {
         /* Flag that the tester type/health type information is        */
         /* present.                                                    */
         GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_TESTER_TYPE_AND_HEALTH_TYPE_PRESENT;

         /* Convert the Tester Type.                                    */
         switch(GLSMeasurementContextData->Tester)
         {
            case GLS_TESTER_SELF:
               GLPMMeasurementContextData->TesterType = gttSelf;
               break;
            case GLS_TESTER_HEALTH_CARE_PROFESSIONAL:
               GLPMMeasurementContextData->TesterType = gttHealthCareProfessional;
               break;
            case GLS_TESTER_LAB_TEST:
               GLPMMeasurementContextData->TesterType = gttLabTest;
               break;
            case GLS_TESTER_NOT_AVAILABLE:
               GLPMMeasurementContextData->TesterType = gttNotAvailable;
               break;
            default:
               GLPMMeasurementContextData->TesterType = gttUnknown;
               break;
         }

         /* Convert the Health Type.                                    */
         switch(GLSMeasurementContextData->Health)
         {
            case GLS_HEALTH_MINOR_HEALTH_ISSUES:
               GLPMMeasurementContextData->HealthType = ghtMinorHealthIssues;
               break;
            case GLS_HEALTH_MAJOR_HEALTH_ISSUES:
               GLPMMeasurementContextData->HealthType = ghtMajorHealthIssues;
               break;
            case GLS_HEALTH_DURING_MENSES:
               GLPMMeasurementContextData->HealthType = ghtDuringMenses;
               break;
            case GLS_HEALTH_UNDER_STRESS:
               GLPMMeasurementContextData->HealthType = ghtUnderStress;
               break;
            case GLS_HEALTH_NO_HEALTH_ISSUES:
               GLPMMeasurementContextData->HealthType = ghtNoHealthIssues;
               break;
            case GLS_HEALTH_NOT_AVAILABLE:
               GLPMMeasurementContextData->HealthType = ghtNotAvailable;
               break;
            default:
               GLPMMeasurementContextData->HealthType = ghtUnknown;
               break;
         }
      }

      /* Next check to see if the optional Exercise information is      */
      /* present.                                                       */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_EXERCISE_PRESENT)
      {
         /* Flag that the exercise information is present.              */
         GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_EXERCISE_DATA_PRESENT;

         /* Convert the values.                                         */
         GLPMMeasurementContextData->ExerciseData.Duration  = (unsigned int)GLSMeasurementContextData->ExerciseData.Duration;
         GLPMMeasurementContextData->ExerciseData.Intensity = (unsigned int)GLSMeasurementContextData->ExerciseData.Intensity;

         /* Check to see if an overrun occurred for the duration.       */
         if(GLSMeasurementContextData->ExerciseData.Duration == GLS_EXERSISE_DURATION_IN_SECONDS_OVERRUN)
            GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_EXERCISE_DATA_DURATION_OVERRUN;
      }

      /* Next check to see if the medication information is present.    */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEDICATION_PRESENT)
      {
         /* Flag that the exercise information is present.              */
         GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_MEDICATION_DATA_PRESENT;

         /* Check to see what the units of the value are.               */
         if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEDICATION_UNITS_LITERS)
            GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_MEDICATION_DATA_UNITS_LITERS;

         /* Next convert the SFLOAT Medication Value.                   */
         ConvertSFloatToGLPM(GLSMeasurementContextData->Medication.Value, &(GLPMMeasurementContextData->MedicationData.MedicationValue));

         /* Next convert the Medication ID.                             */
         switch(GLSMeasurementContextData->Medication.ID)
         {
            case GLS_MEDICATION_RAPID_ACTING_INSULIN:
               GLPMMeasurementContextData->MedicationData.MedicationID = gmiRapidActingInsulin;
               break;
            case GLS_MEDICATION_SHORT_ACTING_INSULIN:
               GLPMMeasurementContextData->MedicationData.MedicationID = gmiShortActingInsulin;
               break;
            case GLS_MEDICATION_INTERMEDIATE_ACTING_INSULIN:
               GLPMMeasurementContextData->MedicationData.MedicationID = gmiIntermediateActingInsulin;
               break;
            case GLS_MEDICATION_LONG_ACTING_INSULIN:
               GLPMMeasurementContextData->MedicationData.MedicationID = gmiLongActingInsulin;
               break;
            case GLS_MEDICATION_PRE_MIXED_INSULIN:
               GLPMMeasurementContextData->MedicationData.MedicationID = gmiPremixedInsulin;
               break;
            default:
               GLPMMeasurementContextData->MedicationData.MedicationID = gmiUnknown;
               break;
         }
      }

      /* Next check to see if the HbA1c information is present.         */
      if(GLSMeasurementContextData->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_HBA1C_PRESENT)
      {
         /* Flag that the exercise information is present.              */
         GLPMMeasurementContextData->ContextFlags |= GLPM_CONTEXT_FLAGS_HBA1C_PRESENT;

         /* Convert the SFLOAT HbA1c data.                              */
         ConvertSFloatToGLPM(GLSMeasurementContextData->HbA1c, &(GLPMMeasurementContextData->HbA1cData));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that is used to      */
   /* perform a Start Procedure Request.                                */
static int StartProcedureRequest(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, GLPM_Procedure_Data_t *ProcedureData)
{
   int                                            ret_val;
   Byte_t                                        *Buffer;
   unsigned int                                   BufferLength;
   Transaction_Info_t                             TransactionInfo;
   Transaction_Info_t                            *TransactionInfoPtr = NULL;
   GLS_Record_Access_Control_Point_Format_Data_t  GLSProcedureData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (ConnectionEntry->GlucoseInformation.RACPHandle) && (EventCallbackInfo) && (ProcedureData) && (ProcedureData->CommandType != gptAbortProcedure))
   {
      /* Verify that no procedure is in progress.                       */
      if(!(ConnectionEntry->ConnectionFlags & GLPM_CONNECTION_ENTRY_FLAGS_RACP_PROCEDURE_IN_PROGRESS))
      {
         /* Next attempt to convert the GLPM Procedure Data to GLS      */
         /* Procedure Data.                                             */
         if((ret_val = ConvertProcedureDataToGLS(ProcedureData, &GLSProcedureData)) == 0)
         {
            /* Allocate a buffer big enough to hold the biggest packet. */
            BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(GLS_DATE_TIME_RANGE_SIZE + NON_ALIGNED_BYTE_SIZE);

            if((Buffer = BTPS_AllocateMemory(BufferLength)) != NULL)
            {
               /* Next attempt to format the packet.                    */
               if(!GLS_Format_Record_Access_Control_Point_Command(&GLSProcedureData, &BufferLength, Buffer))
               {
                  /* Configure the Transaction Information for this     */
                  /* transaction.                                       */
                  BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

                  TransactionInfo.ProcedureID               = GetNextProcedureID();
                  TransactionInfo.GLPManagerDataCallbackID  = EventCallbackInfo->CallbackID;
                  TransactionInfo.ProcedureType             = ProcedureData->CommandType;
                  TransactionInfo.CharacteristicValueHandle = ConnectionEntry->GlucoseInformation.RACPHandle;
                  TransactionInfo.TransactionType           = ttRACPStartProcedure;

                  /* Add the Transaction Info to the transaction list.  */
                  if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
                  {
                     /* Perform the write to configure the RACP         */
                     /* Characteristic.                                 */
                     ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, TransactionInfoPtr->CharacteristicValueHandle, BufferLength, Buffer);
                     if(ret_val > 0)
                     {
                        /* Flag that a RACP Procedure is in progress.   */
                        ConnectionEntry->ConnectionFlags    |= GLPM_CONNECTION_ENTRY_FLAGS_RACP_PROCEDURE_IN_PROGRESS;

                        /* Save the GATM Transaction ID.                */
                        TransactionInfoPtr->GATMProcedureID  = (unsigned int)ret_val;

                        /* Return the Procedure ID to the caller.       */
                        ret_val                              = TransactionInfoPtr->ProcedureID;
                     }
                     else
                     {
                        /* Delete the transaction information memory and*/
                        /* free the memory.                             */
                        if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->ProcedureID)) != NULL)
                        {
                           FreeTransactionInfoMemory(TransactionInfoPtr);

                           TransactionInfoPtr = NULL;
                        }
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }
               else
                  ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;

               /* Free the memory that was allocated for this entry.    */
               BTPS_FreeMemory(Buffer);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_GLUOCSE_PROCEDURE_IN_PROGRESS;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* perform a Start Procedure Request.                                */
static int StopProcedureRequest(GLPM_Connection_Entry_t *ConnectionEntry, GLPM_Entry_Info_t *EventCallbackInfo, unsigned int ProcedureID)
{
   int                                            ret_val;
   Byte_t                                        *Buffer;
   unsigned int                                   BufferLength;
   Transaction_Info_t                            *TransactionInfoPtr;
   GLS_Record_Access_Control_Point_Format_Data_t  GLSProcedureData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (EventCallbackInfo) && (ProcedureID))
   {
      /* Verify that no procedure is in progress.                       */
      if(!(ConnectionEntry->ConnectionFlags & GLPM_CONNECTION_ENTRY_FLAGS_ABORT_RACP_PROCEDURE_IN_PROGRESS))
      {
         /* Get the Procedure Information for the procedure to be       */
         /* stopped.                                                    */
         if(((TransactionInfoPtr = SearchTransactionInfo(&(ConnectionEntry->TransactionInfoList), ProcedureID)) != NULL) && (TransactionInfoPtr->TransactionType == ttRACPStartProcedure))
         {
            /* Allocate a buffer big enough to hold the biggest packet. */
            BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(GLS_DATE_TIME_RANGE_SIZE + NON_ALIGNED_BYTE_SIZE);

            if((Buffer = BTPS_AllocateMemory(BufferLength)) != NULL)
            {
               /* Format the GLS Procedure Data for the abort.          */
               GLSProcedureData.CommandType  = racAbortOperationRequest;
               GLSProcedureData.OperatorType = raoNull;

               /* Next attempt to format the packet.                    */
               if(!GLS_Format_Record_Access_Control_Point_Command(&GLSProcedureData, &BufferLength, Buffer))
               {
                  /* Perform the write to configure the RACP            */
                  /* Characteristic.                                    */
                  ret_val = GATM_WriteValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, TransactionInfoPtr->CharacteristicValueHandle, BufferLength, Buffer);
                  if(ret_val > 0)
                  {
                     /* Stop any active timer for this transaction.     */
                     TMR_StopTimer(TransactionInfoPtr->TimerID);

                     TransactionInfoPtr->TimerID = 0;

                     /* Flag that a RACP Abort Procedure is in progress.*/
                     ConnectionEntry->ConnectionFlags    |= GLPM_CONNECTION_ENTRY_FLAGS_ABORT_RACP_PROCEDURE_IN_PROGRESS;

                     /* Save the new Transaction Type.                  */
                     TransactionInfoPtr->TransactionType  = ttRACPStopProcedure;

                     /* Save the GATM Transaction ID.                   */
                     TransactionInfoPtr->GATMProcedureID  = (unsigned int)ret_val;

                     /* Flag that an abort is active.                   */
                     TransactionInfoPtr->AbortActive      = TRUE;

                     /* Return the success to the caller.               */
                     ret_val                              = 0;
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA;

               /* Free the memory that was allocated for this entry.    */
               BTPS_FreeMemory(Buffer);
            }
            else
               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_GLUOCSE_PROCEDURE_IN_PROGRESS;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));

   return(ret_val);

}

   /* The following function is a utility function which is used to     */
   /* determine if a device supports the necessary services to support  */
   /* the Glucose Sensor role.                                          */
static Boolean_t CheckGlucoseServicesSupported(BD_ADDR_t BD_ADDR)
{
   UUID_16_t        UUID;
   Boolean_t        ret_val = FALSE;
   SDP_UUID_Entry_t ServiceUUID;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Glucose Service is supported.                 */
   ServiceUUID.SDP_Data_Element_Type = deUUID_16;
   GLS_ASSIGN_SERVICE_UUID_16(&UUID);
   CONVERT_BLUETOOTH_UUID_16_TO_SDP_UUID_16(ServiceUUID.UUID_Value.UUID_16, UUID);

   if(DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR, ServiceUUID) > 0)
   {
      /* Flag that this device supports the Glucose Device Role.        */
      ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Glucose Measurement Characteristic.                     */
static Boolean_t ProcessGlucoseMeasurementCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Word_t       CCCD;
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
   {
      /* Attempt to locate the CCCD for this characteristic.            */
      for(Index=0,CCCD=0;Index<CharacteristicInformation->NumberOfDescriptors;Index++)
      {
         if(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
            {
               CCCD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
               break;
            }
         }
      }

      /* Continue only if we have the required CCCD Handle.             */
      if(CCCD)
      {
         /* Attempt to configure the characteristic for notifications.  */
         if(ConfigureClientConfiguration(ConnectionEntry, CharacteristicInformation->Characteristic_Handle, CCCD, FALSE))
         {
            /* Save the required information.                           */
            ConnectionEntry->GlucoseInformation.GlucoseMeasurementHandle     = CharacteristicInformation->Characteristic_Handle;
            ConnectionEntry->GlucoseInformation.GlucoseMeasurementCCCDHandle = CCCD;

            /* Return success to the caller.                            */
            ret_val                                                          = TRUE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Glucose Measurement Context Characteristic.             */
static Boolean_t ProcessGlucoseMeasurementContextCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Word_t       CCCD;
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
   {
      /* Attempt to locate the CCCD for this characteristic.            */
      for(Index=0,CCCD=0;Index<CharacteristicInformation->NumberOfDescriptors;Index++)
      {
         if(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
            {
               CCCD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
               break;
            }
         }
      }

      /* Continue only if we have the required CCCD Handle.             */
      if(CCCD)
      {
         /* Attempt to configure the characteristic for notifications.  */
         if(ConfigureClientConfiguration(ConnectionEntry, CharacteristicInformation->Characteristic_Handle, CCCD, FALSE))
         {
            /* Flag that the measurement context is supported.          */
            ConnectionEntry->GlucoseInformation.SupportedFeatures                   |= GLPM_SUPPORTED_FEATURES_GLUCOSE_MEASUREMENT_CONTEXT;

            /* Save the required information.                           */
            ConnectionEntry->GlucoseInformation.GlucoseMeasurementContextHandle      = CharacteristicInformation->Characteristic_Handle;
            ConnectionEntry->GlucoseInformation.GlucoseMeasurementContextCCCDHandle  = CCCD;

            /* Return success to the caller.                            */
            ret_val                                                                  = TRUE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Glucose Feature Characteristic.                         */
static Boolean_t ProcessGlucoseFeatureCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   int                 Result;
   Boolean_t           ret_val = FALSE;
   Transaction_Info_t  TransactionInfo;
   Transaction_Info_t *TransactionInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
   {
      /* Configure the Transaction Information for this transaction.    */
      BTPS_MemInitialize(&TransactionInfo, 0, sizeof(TransactionInfo));

      TransactionInfo.ProcedureID               = GetNextProcedureID();
      TransactionInfo.CharacteristicValueHandle = CharacteristicInformation->Characteristic_Handle;
      TransactionInfo.TransactionType           = ttReadGlucoseFeature;

      /* Add the Transaction Info to the transaction list.              */
      if((TransactionInfoPtr = AddTransactionInfo(&(ConnectionEntry->TransactionInfoList), &TransactionInfo)) != NULL)
      {
         /* Attempt to read the Report Reference Descriptor.            */
         Result = GATM_ReadValue(GATMEventCallbackID, ConnectionEntry->BD_ADDR, CharacteristicInformation->Characteristic_Handle, 0, TRUE);
         if(Result > 0)
         {
            /* Save the GATM Transaction ID.                            */
            TransactionInfoPtr->GATMProcedureID                       = (unsigned int)Result;

            /* Save the Glucose Feature Handle.                         */
            ConnectionEntry->GlucoseInformation.GlucoseFeaturesHandle = CharacteristicInformation->Characteristic_Handle;

            /* Return success to the caller.                            */
            ret_val                                                   = TRUE;
         }
         else
         {
            /* Delete the transaction information memory and free the   */
            /* memory.                                                  */
            if((TransactionInfoPtr = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfoPtr->ProcedureID)) != NULL)
            {
               FreeTransactionInfoMemory(TransactionInfoPtr);

               TransactionInfoPtr = NULL;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process a Glucose Record Access Control Point Characteristic.     */
static Boolean_t ProcessRecordAccessControlPointCharacteristic(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Characteristic_Information_t *CharacteristicInformation)
{
   Word_t       CCCD;
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (CharacteristicInformation) && (CharacteristicInformation->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
   {
      /* Attempt to locate the CCCD for this characteristic.            */
      for(Index=0,CCCD=0;Index<CharacteristicInformation->NumberOfDescriptors;Index++)
      {
         if(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
            {
               CCCD = CharacteristicInformation->DescriptorList[Index].Characteristic_Descriptor_Handle;
               break;
            }
         }
      }

      /* Continue only if we have the required CCCD Handle.             */
      if(CCCD)
      {
         /* Attempt to configure the characteristic for indications.    */
         if(ConfigureClientConfiguration(ConnectionEntry, CharacteristicInformation->Characteristic_Handle, CCCD, TRUE))
         {
            /* Save the required information.                           */
            ConnectionEntry->GlucoseInformation.RACPHandle     = CharacteristicInformation->Characteristic_Handle;
            ConnectionEntry->GlucoseInformation.RACPCCCDHandle = CCCD;

            /* Return success to the caller.                            */
            ret_val                                            = TRUE;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* process and configure the specified Glucose Service.              */
static Boolean_t ProcessConfigureGlucoseService(GLPM_Connection_Entry_t *ConnectionEntry, GATT_Service_Discovery_Indication_Data_t *GlucoseService)
{
   Boolean_t    ret_val = FALSE;
   Boolean_t    NoError;
   unsigned int Index;
   unsigned int UUIDNumber;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (GlucoseService))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Configure Glucose Service: 0x%04X - 0x%04X.\n", GlucoseService->ServiceInformation.Service_Handle, GlucoseService->ServiceInformation.End_Group_Handle));

      /* Loop through all of the characteristics of the service.        */
      for(Index=0,NoError=TRUE;(Index<GlucoseService->NumberOfCharacteristics)&&(NoError);Index++)
      {
         /* Verify that this characteristic has a 16-bit UUID.          */
         if(GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Determine what this characteristic is.                   */
            if(!GLS_COMPARE_MEASUREMENT_UUID_TO_UUID_16(GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
            {
               if(!GLS_COMPARE_MEASUREMENT_CONTEXT_UUID_TO_UUID_16(GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
               {
                  if(!GLS_COMPARE_FEATURE_TYPE_UUID_TO_UUID_16(GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
                  {
                     if(!GLS_COMPARE_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_TO_UUID_16(GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16))
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

            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Glucose Characteristic UUID: 0x%02X%02X (UUIDNumber=%u).\n", GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16.UUID_Byte1, GlucoseService->CharacteristicInformationList[Index].Characteristic_UUID.UUID.UUID_16.UUID_Byte0, UUIDNumber));

            /* Handle this characteristic.                              */
            switch(UUIDNumber)
            {
               case 1:
                  /* Glucose Measurement Characteristic.                */

                  /* Attempt to process and Configure the Glucose       */
                  /* Measurement Characteristic                         */
                  if(!ProcessGlucoseMeasurementCharacteristic(ConnectionEntry, &(GlucoseService->CharacteristicInformationList[Index])))
                  {
                     /* Failed to process the characteristic so return  */
                     /* an error.                                       */
                     NoError = FALSE;
                  }
                  break;
               case 2:
                  /* Glucode Measurement Context Characteristic.        */

                  /* Attempt to process and Configure the Glucose       */
                  /* Measurement Context Characteristic.                */
                  if(!ProcessGlucoseMeasurementContextCharacteristic(ConnectionEntry, &(GlucoseService->CharacteristicInformationList[Index])))
                  {
                     /* Failed to process the characteristic so return  */
                     /* an error.                                       */
                     NoError = FALSE;
                  }
                  break;
               case 3:
                  /* Glucose Feature Characteristic.                    */

                  /* Attempt to process and Configure the Glucose       */
                  /* Feature Characteristic.                            */
                  if(!ProcessGlucoseFeatureCharacteristic(ConnectionEntry, &(GlucoseService->CharacteristicInformationList[Index])))
                  {
                     /* Failed to process the characteristic so return  */
                     /* an error.                                       */
                     NoError = FALSE;
                  }
                  break;
               case 4:
                  /* Record Access Control Point Characteristic.        */

                  /* Attempt to process and Configure the Record Access */
                  /* Control Point Characteristic.                      */
                  if(!ProcessRecordAccessControlPointCharacteristic(ConnectionEntry, &(GlucoseService->CharacteristicInformationList[Index])))
                  {
                     /* Failed to process the characteristic so return  */
                     /* an error.                                       */
                     NoError = FALSE;
                  }
                  break;
            }
         }
      }

      /* If no error occurred return success.                           */
      if(NoError)
         ret_val = TRUE;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* Process and Configure a Glucose Device Connection.                */
static void ProcessConfigureGlucoseConnection(BD_ADDR_t BD_ADDR)
{
   Boolean_t                    ConfiguredSuccessfully;
   unsigned int                 Index;
   unsigned int                 NumberGlucoseServices;
   GLPM_Connection_Entry_t      ConnectionEntry;
   GLPM_Connection_Entry_t     *ConnectionEntryPtr;
   DEVM_Parsed_Services_Data_t  ParsedGATTData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Before we go to the trouble of checking the service table,     */
      /* verify that this device supports the Glucose Device Role.      */
      if(CheckGlucoseServicesSupported(BD_ADDR))
      {
         /* Query the parsed GATT data.                                 */
         if(QueryParsedServicesData(BD_ADDR, &ParsedGATTData))
         {
            /* Initialize the Connection Entry to add.                  */
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(GLPM_Connection_Entry_t));

            ConnectionEntry.ConnectionState = csConfiguring;
            ConnectionEntry.BD_ADDR         = BD_ADDR;

            /* Add the Connection Info to the Connection List.          */
            if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
            {
               /* Process all of the required services.                 */
               for(Index=0,NumberGlucoseServices=0,ConfiguredSuccessfully=TRUE;(Index<ParsedGATTData.NumberServices)&&(ConfiguredSuccessfully);Index++)
               {
                  /* Verify that this service has a 16 bit UUID.        */
                  if(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID_Type == guUUID_16)
                  {
                     /* Check to see if this is a Glucose Service.      */
                     if(GLS_COMPARE_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16))
                     {
                        /* Attempt to process and configure the Glucose */
                        /* Service.                                     */
                        if(ProcessConfigureGlucoseService(ConnectionEntryPtr, &(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index])))
                           ++NumberGlucoseServices;
                        else
                           ConfiguredSuccessfully = FALSE;
                        break;
                     }
                  }
               }

               /* If we did not configure the services correctly then   */
               /* delete the connection entry.                          */
               if((NumberGlucoseServices == 0) || (!ConfiguredSuccessfully))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to parse*/
   /* the correct Glucose Handles for a device that has been reloaded   */
   /* from file.                                                        */
static void ParseGLPAttributeHandles(GLPM_Connection_Entry_t *ConnectionEntry)
{
   Word_t                                       *CCCD;
   unsigned int                                  Index1;
   unsigned int                                  Index2;
   unsigned int                                  Index3;
   DEVM_Parsed_Services_Data_t                   ParsedGATTData;
   GATT_Characteristic_Information_t            *CharacteristicInformation;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInformation;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
               /* Check to see if this is a Glucose Service.            */
               if(GLS_COMPARE_SERVICE_UUID_TO_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].ServiceInformation.UUID.UUID.UUID_16))
               {
                  /* Loop through the characteristic information list.  */
                  CharacteristicInformation = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].CharacteristicInformationList;

                  for(Index2=0;Index2<ParsedGATTData.GATTServiceDiscoveryIndicationData[Index1].NumberOfCharacteristics;Index2++,CharacteristicInformation++)
                  {
                     /* Verify that this characteristic has a 16-bit    */
                     /* UUID.                                           */
                     if(CharacteristicInformation->Characteristic_UUID.UUID_Type == guUUID_16)
                     {
                        /* Set the CCCD Handle to NULL.                 */
                        CCCD = NULL;

                        /* Now determine which characteristic this is.  */
                        if(GLS_COMPARE_MEASUREMENT_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                        {
                           ConnectionEntry->GlucoseInformation.GlucoseMeasurementHandle = CharacteristicInformation->Characteristic_Handle;
                           CCCD                                                         = &(ConnectionEntry->GlucoseInformation.GlucoseMeasurementCCCDHandle);
                        }
                        else
                        {
                           if(GLS_COMPARE_MEASUREMENT_CONTEXT_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                           {
                              ConnectionEntry->GlucoseInformation.SupportedFeatures               |= GLPM_SUPPORTED_FEATURES_GLUCOSE_MEASUREMENT_CONTEXT;
                              ConnectionEntry->GlucoseInformation.GlucoseMeasurementContextHandle  = CharacteristicInformation->Characteristic_Handle;
                              CCCD                                                                 = &(ConnectionEntry->GlucoseInformation.GlucoseMeasurementContextCCCDHandle);
                           }
                           else
                           {
                              if(GLS_COMPARE_FEATURE_TYPE_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                                 ConnectionEntry->GlucoseInformation.GlucoseFeaturesHandle = CharacteristicInformation->Characteristic_Handle;
                              else
                              {
                                 if(GLS_COMPARE_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_TO_UUID_16(CharacteristicInformation->Characteristic_UUID.UUID.UUID_16))
                                 {
                                    ConnectionEntry->GlucoseInformation.RACPHandle = CharacteristicInformation->Characteristic_Handle;
                                    CCCD                                           = &(ConnectionEntry->GlucoseInformation.RACPCCCDHandle);
                                 }
                              }
                           }
                        }

                        /* If requested loop through the characteristic */
                        /* descriptor list to determine the CCCD handle.*/
                        if(CCCD)
                        {
                           DescriptorInformation = CharacteristicInformation->DescriptorList;
                           for(Index3=0;Index3<CharacteristicInformation->NumberOfDescriptors;Index3++,DescriptorInformation++)
                           {
                              if(DescriptorInformation->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                              {
                                 if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInformation->Characteristic_Descriptor_UUID.UUID.UUID_16))
                                    *CCCD = DescriptorInformation->Characteristic_Descriptor_Handle;
                              }
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a potential Glucose Sensor connection.                    */
static void ProcessGlucoseSensorConnection(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(RemoteDeviceProperties)
   {
      /* First attempt to reload the connection entry from the LE       */
      /* Configuration File.                                            */
      if((ConnectionEntry = LoadGlucoseDeviceFromFile(RemoteDeviceProperties->BD_ADDR)) != NULL)
      {
         /* Attempt to parse the Glucose Attribute Handles that are not */
         /* stored in the configuration file.                           */
         ParseGLPAttributeHandles(ConnectionEntry);

         /* Dispatch the Glucose Sensor Connection event.               */
         DispatchGlucoseConnection(ConnectionEntry);
      }
      else
      {
         /* Attempt to Process/Configure the Glucose Device connection. */
         ProcessConfigureGlucoseConnection(RemoteDeviceProperties->BD_ADDR);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* process a Glucose Sensor Disconnection.                           */
static void ProcessGlucoseSensorDisconnection(BD_ADDR_t RemoteDeviceAddress)
{
   GLPM_Event_Data_t             GLPMEventData;
   GLPM_Connection_Entry_t      *ConnectionEntry;
   GLPM_Disconnected_Message_t   Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
   {
      /* Only dispatch the disconnection if we are in the connection    */
      /* state.                                                         */
      if(ConnectionEntry->ConnectionState == csConnected)
      {
         /* Next, format up the Event to dispatch.                      */
         GLPMEventData.EventType                                           = getGLPMDisconnected;
         GLPMEventData.EventLength                                         = GLPM_DISCONNECTED_EVENT_DATA_SIZE;

         GLPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = RemoteDeviceAddress;
         GLPMEventData.EventData.DisconnectedEventData.ConnectionType      = gctSensor;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER;
         Message.MessageHeader.MessageFunction = GLPM_MESSAGE_FUNCTION_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (GLPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = RemoteDeviceAddress;
         Message.ConnectionType                = gctSensor;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchGLPEvent(&GLPMEventData, (BTPM_Message_t *)&Message);
      }

      /* Free the memory allocated for the connection entry.            */
      FreeConnectionEntryMemory(ConnectionEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* LE Address Change Event.                                          */
   /* * NOTE * This function *MUST* be called with the GLPM Manager Lock*/
   /*          held.                                                    */
static void ProcessLowEnergyAddressChangeEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   GLPM_Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLPM Address Updated\n"));

            /* Delete any information stored with the old BD_ADDR.      */
            StoreGlucoseDeviceToFile(ConnectionEntryPtr, FALSE);

            /* Save the new Base Address.                               */
            ConnectionEntryPtr->BD_ADDR = RemoteDeviceProperties->BD_ADDR;

            /* Store information to the file with the new BD_ADDR.      */
            StoreGlucoseDeviceToFile(ConnectionEntryPtr, FALSE);
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
   /* * NOTE * This function *MUST* be called with the GLPM Manager Lock*/
   /*          held.                                                    */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   unsigned long      RequiredConnectionFlags;
   GLPM_Connection_Entry_t ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Address is updated or the LE Pairing State changes.            */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

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
               /* Process this as a potential Glucose device connection.*/
               ProcessGlucoseSensorConnection(RemoteDeviceProperties);
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
                     if(ReloadGlucoseInformation(NULL, RemoteDeviceProperties->BD_ADDR))
                     {
                        /* Delete the information that is stored for    */
                        /* this device in the file.                     */
                        BTPS_MemInitialize(&ConnectionEntry, 0, (sizeof(ConnectionEntry)));
                        ConnectionEntry.BD_ADDR = RemoteDeviceProperties->BD_ADDR;

                        StoreGlucoseDeviceToFile(&ConnectionEntry, FALSE);
                     }
                  }
               }

               /* Process this as a potential Glucose device            */
               /* disconnection.                                        */
               ProcessGlucoseSensorDisconnection(RemoteDeviceProperties->BD_ADDR);
            }
         }

         /* Process the Address Updated event if necessary.             */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressChangeEvent(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register GLP Events*/
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessRegisterCollectorEventsMessage(GLPM_Register_Collector_Events_Request_t *Message)
{
   int                                       Result;
   GLPM_Entry_Info_t                         GLPEntryInfo;
   GLPM_Register_Collector_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* For now we will only allow 1 Collector Event Callback to be    */
      /* registered at a time.  Whoever registers this will be the      */
      /* "Owner" of all Glucose Collector connections.  In the future   */
      /* this could change and this check would have to be removed.     */
      if(GLPEntryInfoList == NULL)
      {
         /* Attempt to add an entry into the GLP Entry list.            */
         BTPS_MemInitialize(&GLPEntryInfo, 0, sizeof(GLPM_Entry_Info_t));

         GLPEntryInfo.CallbackID         = GetNextCallbackID();
         GLPEntryInfo.ClientID           = Message->MessageHeader.AddressID;
         GLPEntryInfo.Flags              = GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         if(AddGLPEntryInfoEntry(&GLPEntryInfoList, &GLPEntryInfo))
            Result = GLPEntryInfo.CallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = GLPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.GLPEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status             = 0;
      }
      else
      {
         ResponseMessage.GLPEventsHandlerID = 0;

         ResponseMessage.Status             = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register GLP    */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterCollectorEventsMessage(GLPM_Un_Register_Collector_Events_Request_t *Message)
{
   int                                           Result;
   GLPM_Entry_Info_t                            *GLPEntryInfo;
   GLPM_Un_Register_Collector_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, Message->GLPEventsHandlerID)) != NULL) && (GLPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((GLPEntryInfo = DeleteGLPEntryInfoEntry(&GLPEntryInfoList, Message->GLPEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeGLPEntryInfoEntryMemory(GLPEntryInfo);

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

      ResponseMessage.MessageHeader.MessageLength  = GLPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Start Procedure    */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessStartProcedureMessage(GLPM_Start_Procedure_Request_t *Message)
{
   int                              Result;
   GLPM_Entry_Info_t               *GLPEntryInfo;
   GLPM_Connection_Entry_t         *ConnectionEntry;
   GLPM_Start_Procedure_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Determine the GLP Callback Information and make sure that it is*/
      /* valid.                                                         */
      if(((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, Message->GLPCollectorEventsHandlerID)) != NULL) && (GLPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Simply call the internal function to attempt to start the*/
            /* procedure.                                               */
            Result = StartProcedureRequest(ConnectionEntry, GLPEntryInfo, &(Message->ProcedureData));
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = GLPM_START_PROCEDURE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status                    = 0;

         ResponseMessage.ProcedureID               = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status                    = Result;

         ResponseMessage.ProcedureID               = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Stop Procedure     */
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e.  the length)   */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessStopProcedureMessage(GLPM_Stop_Procedure_Request_t *Message)
{
   int                             Result;
   GLPM_Entry_Info_t              *GLPEntryInfo;
   GLPM_Connection_Entry_t        *ConnectionEntry;
   GLPM_Stop_Procedure_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Determine the GLP Callback Information and make sure that it is*/
      /* valid.                                                         */
      if(((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, Message->GLPCollectorEventsHandlerID)) != NULL) && (GLPEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Determine the connection entry for the specified connection */
         /* and make sure we are done configuring the device.           */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
         {
            /* Simply call the internal function to attempt to start the*/
            /* procedure.                                               */
            Result = StopProcedureRequest(ConnectionEntry, GLPEntryInfo, Message->ProcedureID);
         }
         else
            Result = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = GLPM_STOP_PROCEDURE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the GLP Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case GLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register GLP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register GLP Events Request.                          */
               ProcessRegisterCollectorEventsMessage((GLPM_Register_Collector_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register GLP Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register GLP Events Request.                       */
               ProcessUnRegisterCollectorEventsMessage((GLPM_Un_Register_Collector_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_START_PROCEDURE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Start Procedure Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_START_PROCEDURE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Start Procedure Request.                              */
               ProcessStartProcedureMessage((GLPM_Start_Procedure_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case GLPM_MESSAGE_FUNCTION_STOP_PROCEDURE:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Stop Procedure Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= GLPM_STOP_PROCEDURE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Stop*/
               /* Procedure Request.                                    */
               ProcessStopProcedureMessage((GLPM_Stop_Procedure_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   GLPM_Entry_Info_t *GLPEntryInfo;
   GLPM_Entry_Info_t **_GLPEntryInfoList;
   GLPM_Entry_Info_t *tmpGLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      GLPEntryInfo      = GLPEntryInfoList;
      _GLPEntryInfoList = &GLPEntryInfoList;

      while(GLPEntryInfo)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(GLPEntryInfo->ClientID == ClientID)
         {
            /* Note the next GLP Entry in the list (we are about to     */
            /* delete the current entry).                               */
            tmpGLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;

            /* Go ahead and delete the GLP Information Entry and clean  */
            /* up the resources.                                        */
            if((GLPEntryInfo = DeleteGLPEntryInfoEntry(_GLPEntryInfoList, GLPEntryInfo->CallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeGLPEntryInfoEntryMemory(GLPEntryInfo);
            }

            /* Go ahead and set the next GLP Information Entry (past the*/
            /* one we just deleted).                                    */
            GLPEntryInfo = tmpGLPEntryInfo;
         }
         else
            GLPEntryInfo = GLPEntryInfo->NextGLPEntryInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Transaction Response.                              */
static void ProcessGATTTransactionResponse(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, Boolean_t ErrorResponse, void *EventData)
{
   Byte_t             ErrorCode;
   GLPM_Entry_Info_t *EventCallbackInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters are semi-valid.                  */
   if((ConnectionEntry) && (TransactionInfo) && (EventData))
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: Status = 0x%02X\n", (ErrorResponse?((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode:0)));

      /* Process the correct transaction.                               */
      switch(TransactionInfo->TransactionType)
      {
         case ttReadGlucoseFeature:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttReadGlucoseFeature\n"));

            /* Process this if no error has occurred.                   */
            if(!ErrorResponse)
            {
               /* Verify that the length is valid.                      */
               if(((GATM_Read_Response_Event_Data_t *)EventData)->ValueLength == NON_ALIGNED_WORD_SIZE)
               {
                  /* Update the Glucose Information.                    */
                  ConnectionEntry->GlucoseInformation.SupportedFeatures |= (unsigned long)READ_UNALIGNED_WORD_LITTLE_ENDIAN(((GATM_Read_Response_Event_Data_t *)EventData)->Value);
               }
               else
               {
                  /* Flag that an error occurred.                       */
                  ErrorResponse = TRUE;
               }
            }

            /* Delete the Transaction Info.                             */
            if((TransactionInfo = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfo->ProcedureID)) != NULL)
               FreeTransactionInfoMemory(TransactionInfo);
            break;
         case ttConfigureCCCD:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttConfigureCCCD\n"));

            /* There is nothing to do here other than let the logic     */
            /* determine if we can move to the connection state.        */

            /* Delete the Transaction Info.                             */
            if((TransactionInfo = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfo->ProcedureID)) != NULL)
               FreeTransactionInfoMemory(TransactionInfo);
            break;
         case ttRACPStartProcedure:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttRACPStartProcedure\n"));

            /* Get the transaction info for this operation.             */
            if(ErrorResponse)
               TransactionInfo = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfo->ProcedureID);

            /* Continue only if we found the correct transaction        */
            /* information.                                             */
            if(TransactionInfo)
            {
               /* Clear the GATM Procedure ID since the GATM Procedure  */
               /* is no longer outstanding.                             */
               TransactionInfo->GATMProcedureID = 0;

               /* If this is an error response do the necessary cleanup.*/
               if(ErrorResponse)
               {
                  /* Flag that the RACP Procedure is no longer in       */
                  /* progress.                                          */
                  ConnectionEntry->ConnectionFlags &= ~((unsigned long)GLPM_CONNECTION_ENTRY_FLAGS_RACP_PROCEDURE_IN_PROGRESS);

                  /* Get the error code.                                */
                  if(((GATM_Error_Response_Event_Data_t *)EventData)->ErrorType == retErrorResponse)
                     ErrorCode = ((GATM_Error_Response_Event_Data_t *)EventData)->AttributeProtocolErrorCode;
                  else
                     ErrorCode = ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR;
               }
               else
                  ErrorCode = 0;

               /* Search for the callback entry for this event.         */
               if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
               {
                  /* Dispatch the Procedure Started Event.              */
                  DispatchProcedureStartedEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, ErrorCode);
               }

               /* Check to see if we should start a timer waiting on the*/
               /* next packet.                                          */
               if(!ErrorResponse)
               {
                  /* Start a timer for the operation.                   */
                  TransactionInfo->TimerID = TMR_StartTimer((void *)TransactionInfo->ProcedureID, TimerCallback, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));
               }
               else
               {
                  /* If we deleted the entry then free the memory.      */
                  FreeTransactionInfoMemory(TransactionInfo);
               }
            }
            break;
         case ttRACPStopProcedure:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Transaction Response: ttRACPStopProcedure\n"));

            /* Clear the GATM Procedure ID since the GATM Procedure is  */
            /* no longer outstanding.                                   */
            TransactionInfo->GATMProcedureID = 0;

            /* Start a timer for the operation.                         */
            TransactionInfo->TimerID         = TMR_StartTimer((void *)TransactionInfo->ProcedureID, TimerCallback, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));

            /* Continue only if this is an error response.  Otherwise we*/
            /* will wait for the RACP Indication to dispatch the        */
            /* Procedure Stopped event.                                 */
            if(ErrorResponse)
            {
               /* Search for the callback entry for this event.         */
               if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
               {
                  /* Dispatch the Procedure Started Event.              */
                  DispatchProcedureStoppedEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, grcUnknown, 0);
               }

               /* Flag that a RACP Abort Procedure is in progress.      */
               ConnectionEntry->ConnectionFlags &= ~((unsigned long)GLPM_CONNECTION_ENTRY_FLAGS_ABORT_RACP_PROCEDURE_IN_PROGRESS);

               /* Flag that the abort is no longer in progress.         */
               TransactionInfo->AbortActive      = FALSE;
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled response type\n"));

            /* Delete the Transaction Info.                             */
            if((TransactionInfo = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfo->ProcedureID)) != NULL)
               FreeTransactionInfoMemory(TransactionInfo);
            break;
      }

      /* If an error occurs when configuring the Glucose Service we will*/
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
            /* transaction and can therefore dispatch a Glucose Device  */
            /* Connection event.                                        */
            if(ConnectionEntry->TransactionInfoList == NULL)
            {
               /* Store the connection information to the LE            */
               /* Configuration file.                                   */
               StoreGlucoseDeviceToFile(ConnectionEntry, TRUE);

               /* Dispatch the Glucose Device Connection event.         */
               DispatchGlucoseConnection(ConnectionEntry);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Glucose Measurement Notification.                       */
static void ProcessGlucoseMeasurementNotification(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   Boolean_t                        DispatchEvent;
   GLPM_Entry_Info_t               *EventCallbackInfo;
   GLPM_Measurement_Error_Type_t    ErrorType;
   GLS_Glucose_Measurement_Data_t   GLSMeasurementData;
   GLPM_Glucose_Measurement_Data_t *MeasurementData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (TransactionInfo) && (HandleValueEventData))
   {
      /* Initialize some local variables.                               */
      MeasurementData = NULL;
      DispatchEvent   = FALSE;
      ErrorType       = meNoError;

      /* Stop any active timer for this transaction.                    */
      TMR_StopTimer(TransactionInfo->TimerID);

      /* Attempt to convert the raw data to parsed GLS Data.            */
      if(!GLS_Decode_Glucose_Measurement(HandleValueEventData->AttributeValueLength, HandleValueEventData->AttributeValue, &GLSMeasurementData))
      {
         /* Check to see if we stored a Measurement Data structure in   */
         /* the transaction info.  If so this indicates that we are     */
         /* expecting a Glucose Measurement Context next and therefore  */
         /* should NOT have received this packet.                       */
         if(TransactionInfo->MeasurementDataValid)
         {
            /* Flag that the data is no longer valid.                   */
            TransactionInfo->MeasurementDataValid = FALSE;

            /* Search for the callback for this event and dispatch the  */
            /* event.                                                   */
            if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
               DispatchGlucoseMeasurementEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, meNoGlucoseContext, &(TransactionInfo->MeasurementData), NULL);
         }

         /* Convert the GLS Data to GLPM Data.                          */
         ConvertGlucoseMeasurementToGLPM(&GLSMeasurementData, &(TransactionInfo->MeasurementData));

         /* Check to see if we should queue this data waiting on the    */
         /* context or just dispatch the event.                         */
         if(GLSMeasurementData.OptionFlags & GLS_MEASUREMENT_FLAGS_CONTEXT_INFORMATION_PRESENT)
         {
            /* Flag that the measurement data is valid.                 */
            TransactionInfo->MeasurementDataValid = TRUE;
         }
         else
         {
            /* Just go ahead and dispatch the event with this           */
            /* measurement.                                             */
            MeasurementData = &(TransactionInfo->MeasurementData);
            DispatchEvent   = TRUE;
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLS_Decode_Glucose_Measurement() failed to parse glucose data.\n"));

         /* Failed to parse the data so return an error for this event. */
         DispatchEvent = TRUE;
         ErrorType     = meInvalidData;
      }

      /* If requested dispatch the Glucose Measurement Event.           */
      if(DispatchEvent)
      {
         /* Search for the callback for this event and dispatch the     */
         /* event.                                                      */
         if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
            DispatchGlucoseMeasurementEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, ErrorType, MeasurementData, NULL);
      }

      /* Start a new timer for the operation.                           */
      TransactionInfo->TimerID = TMR_StartTimer((void *)TransactionInfo->ProcedureID, TimerCallback, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Glucose Measurement Context Notification.               */
static void ProcessGlucoseMeasurementContextNotification(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   GLPM_Entry_Info_t                       *EventCallbackInfo;
   GLPM_Measurement_Error_Type_t            ErrorType;
   GLPM_Glucose_Measurement_Data_t         *MeasurementData;
   GLS_Glucose_Measurement_Context_Data_t   GLSMeasurementContextData;
   GLPM_Glucose_Measurement_Context_Data_t  MeasurementContextData;
   GLPM_Glucose_Measurement_Context_Data_t *MeasurementContextDataPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (TransactionInfo) && (HandleValueEventData))
   {
      /* Stop any active timer for this transaction.                    */
      TMR_StopTimer(TransactionInfo->TimerID);

      /* Initialize some local variables.                               */
      ErrorType = meNoError;

      /* Verify that we have a corresponding Glucose Measurement        */
      /* structure that is queued waiting for this context.             */
      if(TransactionInfo->MeasurementDataValid)
      {
         /* Flag that the data is no longer valid.                      */
         TransactionInfo->MeasurementDataValid = FALSE;

         /* Attempt to convert the raw data into parsed GLS data.       */
         if(!GLS_Decode_Glucose_Measurement_Context(HandleValueEventData->AttributeValueLength, HandleValueEventData->AttributeValue, &GLSMeasurementContextData))
         {
            /* Convert the GLS structure into a GLPM structure.         */
            ConvertGlucoseMeasurementContextToGLPM(&GLSMeasurementContextData, &MeasurementContextData);

            /* Save pointers to the data we will dispatch.              */
            MeasurementData           = &(TransactionInfo->MeasurementData);
            MeasurementContextDataPtr = &(MeasurementContextData);

            /* Verify that the sequence numbers match.                  */
            if(MeasurementData->SequenceNumber != MeasurementContextDataPtr->SequenceNumber)
               ErrorType = meSequenceNumberMismatch;
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLS_Decode_Glucose_Measurement_Context() failed to parse glucose data.\n"));

            ErrorType                 = meInvalidData;
            MeasurementData           = &(TransactionInfo->MeasurementData);
            MeasurementContextDataPtr = NULL;
         }
      }
      else
      {
         ErrorType                 = meUnexpectedPacket;
         MeasurementData           = NULL;
         MeasurementContextDataPtr = NULL;
      }

      /* Search for the callback for this event and dispatch the event. */
      if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
         DispatchGlucoseMeasurementEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, ErrorType, MeasurementData, MeasurementContextDataPtr);

      /* Start a new timer for the operation.                           */
      TransactionInfo->TimerID = TMR_StartTimer((void *)TransactionInfo->ProcedureID, TimerCallback, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Record Access Control Point Indication.                 */
static void ProcessRACPIndication(GLPM_Connection_Entry_t *ConnectionEntry, Transaction_Info_t *TransactionInfo, GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   Byte_t                                           Opcode;
   Byte_t                                           ExpectedOpcode;
   Boolean_t                                        DeletedTransactionInfo;
   unsigned int                                     NumberStoredRecords;
   GLPM_Entry_Info_t                               *EventCallbackInfo;
   GLPM_Procedure_Response_Code_Type_t              ResponseCode;
   GLS_Record_Access_Control_Point_Response_Data_t  RACPResponseData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if((ConnectionEntry) && (TransactionInfo) && (HandleValueEventData))
   {
      /* Stop any active timer for this transaction.                    */
      TMR_StopTimer(TransactionInfo->TimerID);

      /* Initialize some local variables.                               */
      DeletedTransactionInfo = TRUE;
      ResponseCode           = grcSuccess;
      NumberStoredRecords    = 0;
      Opcode                 = 0;

      /* Attempt to parse the RACP Response.                            */
      if(!GLS_Decode_Record_Access_Control_Point_Response(HandleValueEventData->AttributeValueLength, HandleValueEventData->AttributeValue, &RACPResponseData))
      {
         /* Convert the response code.                                  */
         switch(RACPResponseData.ResponseType)
         {
            case rarNumberOfStoredRecords:
               /* Save the relevant information into local variables.   */
               Opcode              = GLS_RECORD_ACCESS_OPCODE_REPORT_NUM_STORED_RECORDS;
               NumberStoredRecords = RACPResponseData.ResponseData.NumberOfStoredRecordsResult;
               break;
            case rarResponseCode:
               /* Save the opcode into a local variable.                */
               Opcode = RACPResponseData.ResponseData.ResponseCodeValue.RequestOpCode;

               /* Convert the response code.                            */
               switch(RACPResponseData.ResponseData.ResponseCodeValue.ResponseCodeValue)
               {
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_SUCCESS:
                     ResponseCode = grcSuccess;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_OPCODE_NOT_SUPPORTED:
                     ResponseCode = grcOpcodeNotSupported;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_INVALID_OPERATOR:
                     ResponseCode = grcInvalidOperator;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_OPERATOR_NOT_SUPPORTED:
                     ResponseCode = grcOperatorNotSupported;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_INVALID_OPERAND:
                     ResponseCode = grcInvalidOperand;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_NO_RECORDS_FOUND:
                     ResponseCode = grcNoRecordsFound;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_ABORT_UNSUCCESSFUL:
                     ResponseCode = grcAbortUnsuccessful;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED:
                     ResponseCode = grcProcedureNotCompleted;
                     break;
                  case GLS_RECORD_ACCESS_RESPONSE_CODE_OPERAND_NOT_SUPPORTED:
                     ResponseCode = grcOperandNotSupported;
                     break;
                  default:
                     ResponseCode = grcUnknown;
                     break;
               }
               break;
         }

         /* Check to see if this is an RACP indication that indicates an*/
         /* abort was unsuccessful.                                     */
         if((TransactionInfo->AbortActive) && (Opcode == GLS_RECORD_ACCESS_OPCODE_ABORT_OPERATION))
         {
            /* Flag that the abort is not active.                       */
            ConnectionEntry->ConnectionFlags &= ~((unsigned long)GLPM_CONNECTION_ENTRY_FLAGS_ABORT_RACP_PROCEDURE_IN_PROGRESS);

            /* Check to see if the abort was successfull or not.        */
            if(ResponseCode != grcSuccess)
            {
               /* The abort was not successful so do NOT delete the     */
               /* transaction info.                                     */
               DeletedTransactionInfo = FALSE;
            }
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Failed to parse RACP response\n"));

         ResponseCode = grcUnknown;
      }

      /* Determine the expected opcode.                                 */
      switch(TransactionInfo->ProcedureType)
      {
         case gptReportStoredRecords:
            ExpectedOpcode = GLS_RECORD_ACCESS_OPCODE_REPORT_STORED_RECORDS;
            break;
         case gptDeleteStoredRecords:
            ExpectedOpcode = GLS_RECORD_ACCESS_OPCODE_DELETE_STORED_RECORDS;
            break;
         case gptReportNumberStoredRecords:
            ExpectedOpcode = GLS_RECORD_ACCESS_OPCODE_REPORT_NUM_STORED_RECORDS;
            break;
         default:
            ExpectedOpcode = 0;
            break;
      }

      /* Verify that the opcode matches the response of what we are     */
      /* expecting.                                                     */
      if((Opcode == GLS_RECORD_ACCESS_OPCODE_ABORT_OPERATION) || (Opcode == ExpectedOpcode))
      {
         /* Check to see if we need to delete the transaction info.     */
         if(DeletedTransactionInfo)
         {
            /* Flag that the an operation is not in progress if we are  */
            /* deleting the transaction information.                    */
            ConnectionEntry->ConnectionFlags &= ~((unsigned long)GLPM_CONNECTION_ENTRY_FLAGS_RACP_PROCEDURE_IN_PROGRESS);

            /* Delete the transaction information from the list.        */
            TransactionInfo = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfo->ProcedureID);
         }

         if(TransactionInfo)
         {
            /* Search for the callback for this event and dispatch the  */
            /* event.                                                   */
            if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
               DispatchProcedureStoppedEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, ResponseCode, NumberStoredRecords);

            /* Verify that we flag that no abort operation is active in */
            /* the connection entry.                                    */
            TransactionInfo->AbortActive = FALSE;

            /* If we delete the transaction info then free the memory.  */
            if(DeletedTransactionInfo)
               FreeTransactionInfoMemory(TransactionInfo);
            else
            {
               /* Start a new timer for the operation.                  */
               TransactionInfo->TimerID = TMR_StartTimer((void *)TransactionInfo->ProcedureID, TimerCallback, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));
            }
         }
      }
      else
      {
         /* Start a new timer for the operation.                        */
         TransactionInfo->TimerID = TMR_StartTimer((void *)TransactionInfo->ProcedureID, TimerCallback, (ATT_PROTOCOL_TRANSACTION_TIMEOUT_VALUE * 1000));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Handle Value notification.                         */
   /* * NOTE * This function *MUST* be called with the GLP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTHandleValueData(GATM_Handle_Value_Data_Event_Data_t *HandleValueEventData)
{
   Transaction_Info_t      *TransactionInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(HandleValueEventData)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, HandleValueEventData->RemoteDeviceAddress)) != NULL)
      {
         /* Next check to see if this is a notification or indication of*/
         /* a Glucose characteristic.                                   */
         if(HandleValueEventData->AttributeHandle == ConnectionEntry->GlucoseInformation.GlucoseMeasurementHandle)
         {
            /* Verify that this is a notification.                      */
            if(HandleValueEventData->HandleValueIndication == FALSE)
            {
               /* Note the transaction info that started this must be   */
               /* the first in the list.                                */
               if((TransactionInfo = ConnectionEntry->TransactionInfoList) != NULL)
               {
                  /* Attempt to process the Glucose Measurement         */
                  /* Notification.                                      */
                  ProcessGlucoseMeasurementNotification(ConnectionEntry, TransactionInfo, HandleValueEventData);
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Expected notification of Glucose Measurement Characteristic\n"));
            }
         }
         else
         {
            if(HandleValueEventData->AttributeHandle == ConnectionEntry->GlucoseInformation.GlucoseMeasurementContextHandle)
            {
               /* Verify that this is a notification.                   */
               if(HandleValueEventData->HandleValueIndication == FALSE)
               {
                  /* Note the transaction info that started this must be*/
                  /* the first in the list.                             */
                  if((TransactionInfo = ConnectionEntry->TransactionInfoList) != NULL)
                  {
                     /* Attempt to process the Glucose Measurement      */
                     /* Context Notification.                           */
                     ProcessGlucoseMeasurementContextNotification(ConnectionEntry, TransactionInfo, HandleValueEventData);
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Expected notification of Glucose Measurement Context Characteristic\n"));
               }
            }
            else
            {
               if(HandleValueEventData->AttributeHandle == ConnectionEntry->GlucoseInformation.RACPHandle)
               {
                  /* Verify that this is a indication.                  */
                  if(HandleValueEventData->HandleValueIndication == TRUE)
                  {
                     /* Note the transaction info that started this must*/
                     /* be the first in the list.                       */
                     if((TransactionInfo = ConnectionEntry->TransactionInfoList) != NULL)
                     {
                        /* Attempt to process the Record Access Control */
                        /* Point indication.                            */
                        ProcessRACPIndication(ConnectionEntry, TransactionInfo, HandleValueEventData);
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Expected indication of RACP Characteristic\n"));
                  }
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Read Response.                                     */
   /* * NOTE * This function *MUST* be called with the GLP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTReadResponse(GATM_Read_Response_Event_Data_t *ReadResponseData)
{
   Transaction_Info_t *TransactionInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ReadResponseData)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ReadResponseData->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionInfo = SearchTransactionInfoByGATMProcedureID(&(ConnectionEntry->TransactionInfoList), ReadResponseData->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionInfo, FALSE, (void *)ReadResponseData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Write Response.                                    */
   /* * NOTE * This function *MUST* be called with the GLP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTWriteResponse(GATM_Write_Response_Event_Data_t *WriteResponse)
{
   Transaction_Info_t *TransactionInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(WriteResponse)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, WriteResponse->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionInfo = SearchTransactionInfoByGATMProcedureID(&(ConnectionEntry->TransactionInfoList), WriteResponse->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionInfo, FALSE, (void *)WriteResponse);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GATT Error Response.                                    */
   /* * NOTE * This function *MUST* be called with the GLP Manager Lock */
   /*          held.                                                    */
static void ProcessGATTErrorResponse(GATM_Error_Response_Event_Data_t *ErrorResponse)
{
   Transaction_Info_t      *TransactionInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameter is semi-valid.                    */
   if(ErrorResponse)
   {
      /* Search for the connection entry for this device.               */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, ErrorResponse->RemoteDeviceAddress)) != NULL)
      {
         /* Delete the Transaction Info for this response.              */
         if((TransactionInfo = SearchTransactionInfoByGATMProcedureID(&(ConnectionEntry->TransactionInfoList), ErrorResponse->TransactionID)) != NULL)
         {
            /* Process the GATT Transaction Response.                   */
            ProcessGATTTransactionResponse(ConnectionEntry, TransactionInfo, TRUE, (void *)ErrorResponse);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a GLPM RACP Procedure Timeout.                            */
   /* * NOTE * This function *MUST* be called with the GLP Manager Lock */
   /*          held.                                                    */
static void ProcessProcedureTimeout(unsigned int ProcedureID)
{
   GLPM_Entry_Info_t       *EventCallbackInfo;
   Transaction_Info_t      *TransactionInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", ProcedureID));

   /* Verify that the input parameter is semi-valid.                    */
   if(ProcedureID)
   {
      /* Search the connect list for the connection who this transaction*/
      /* belongs to.                                                    */
      ConnectionEntry = ConnectionEntryList;
      TransactionInfo = NULL;
      while(ConnectionEntry)
      {
         if((TransactionInfo = SearchTransactionInfo(&(ConnectionEntry->TransactionInfoList), ProcedureID)) != NULL)
            break;

         ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
      }

      /* Continue only if we found the correct information.             */
      if((ConnectionEntry) && (TransactionInfo))
      {
         /* Search for the callback entry for this event.               */
         if((EventCallbackInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, TransactionInfo->GLPManagerDataCallbackID)) != NULL)
            DispatchProcedureStoppedEvent(ConnectionEntry, EventCallbackInfo, TransactionInfo, grcProcedureTimeout, 0);

         /* Perform the necessary cleanup based on the operation type.  */
         if(TransactionInfo->AbortActive == FALSE)
         {
            /* Flag that no operation is currently active.              */
            ConnectionEntry->ConnectionFlags &= ~((unsigned long)GLPM_CONNECTION_ENTRY_FLAGS_RACP_PROCEDURE_IN_PROGRESS);

            /* If this was not an abort then we need to delete the      */
            /* connection entry.                                        */
            if((TransactionInfo = DeleteTransactionInfo(&(ConnectionEntry->TransactionInfoList), TransactionInfo->ProcedureID)) != NULL)
               FreeTransactionInfoMemory(TransactionInfo);
         }
         else
         {
            /* Flag that no abort is currently active.                  */
            TransactionInfo->AbortActive      = FALSE;
            ConnectionEntry->ConnectionFlags &= ~((unsigned long)GLPM_CONNECTION_ENTRY_FLAGS_ABORT_RACP_PROCEDURE_IN_PROGRESS);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process TMR events.                                 */
static void BTPSAPI BTPMDistpatchCallback_TMR(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, let's check to see if we need to process it.                */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Process the procedure timeout.                              */
         ProcessProcedureTimeout((unsigned int)CallbackParameter);

         /* Release the lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GLP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_GLPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Timer Callback function which is    */
   /* used to verify that a response is received in the correct amount  */
   /* of time.                                                          */
static Boolean_t BTPSAPI TimerCallback(unsigned int TimerID, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, let's check to see if we need to process it.                */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Timer Callback...\n"));

         /* Queue a callback.                                           */
         BTPM_QueueMailboxCallback(BTPMDistpatchCallback_TMR, (void *)CallbackParameter);

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));

   return(FALSE);
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
                  DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled GATM Event\n"));
                  break;
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all GLP Manager Messages.   */
static void BTPSAPI GLPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a GLP Manager defined    */
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
               /* GLP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_GLPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue GLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue GLP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an GLP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non GLP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager GLP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI GLPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing GLP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process GLP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER, GLPManagerGroupHandler, NULL))
         {
            /* Initialize the actual GLP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the GLP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _GLPM_Initialize()))
            {
               /* Register a GATM Event Callback.                       */
               if((Result = GATM_RegisterEventCallback(GATM_Event_Callback, NULL)) > 0)
               {
                  /* Save the GATM Event Callback ID.                   */
                  GATMEventCallbackID     = (unsigned int)Result;

                  /* Determine the current Device Power State.          */
                  CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting GLP Callback ID.     */
                  NextCallbackID          = 0x000000001;

                  /* Initialize a unique, starting Tansaction ID.       */
                  ProcedureID           = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized             = TRUE;

                  /* Flag success.                                      */
                  Result                  = 0;
               }
               else
               {
                  /* Failed to register callback so cleanup the GLP     */
                  /* Manager Implementation Module.                     */
                  _GLPM_Cleanup();
               }
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _GLPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("GLP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the GLP Manager Implementation that  */
            /* we are shutting down.                                    */
            _GLPM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the GLP Entry Information List is empty.  */
            FreeGLPEntryInfoList(&GLPEntryInfoList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI GLPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                Result;
   GLPM_Connection_Entry_t ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the GLP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _GLPM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the GLP Manager that the Stack has been closed.*/
               _GLPM_SetBluetoothStackID(0);

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Check to see if we stored anything for this device.   */
               if(ReloadGlucoseInformation(NULL, EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress))
               {
                  /* Delete the information that is stored for this     */
                  /* device in the file.                                */
                  BTPS_MemInitialize(&ConnectionEntry, 0, (sizeof(ConnectionEntry)));
                  ConnectionEntry.BD_ADDR = EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress;

                  StoreGlucoseDeviceToFile(&ConnectionEntry, FALSE);
               }

               /* Process this as a potential Glucose Device            */
               /* Disconnection.                                        */
               ProcessGlucoseSensorDisconnection(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Glucose Profile  */
   /* (GLPM) Manager Service.  This Callback will be dispatched by the  */
   /* GLP Manager when various GLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a GLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          GLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int BTPSAPI GLPM_Register_Collector_Event_Callback(GLPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   GLPM_Entry_Info_t GLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
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
            /* the "Owner" of all Glucose Collector connections.  In the*/
            /* future this could change and this check would have to be */
            /* removed.                                                 */
            if(GLPEntryInfoList == NULL)
            {
               /* Attempt to add an entry into the GLP Entry list.      */
               BTPS_MemInitialize(&GLPEntryInfo, 0, sizeof(GLPM_Entry_Info_t));

               GLPEntryInfo.CallbackID         = GetNextCallbackID();
               GLPEntryInfo.ClientID           = MSG_GetServerAddressID();
               GLPEntryInfo.EventCallback      = CallbackFunction;
               GLPEntryInfo.CallbackParameter  = CallbackParameter;
               GLPEntryInfo.Flags              = GLP_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               if(AddGLPEntryInfoEntry(&GLPEntryInfoList, &GLPEntryInfo))
                  ret_val = GLPEntryInfo.CallbackID;
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
      ret_val = BTPM_ERROR_CODE_GLUCOSE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered GLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* GLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the GLP Manager Event Callback ID (return value  */
   /* from GLPM_Register_Collector_Event_Callback() function).          */
void BTPSAPI GLPM_Un_Register_Collector_Event_Callback(unsigned int GLPMCollectorCallbackID)
{
   GLPM_Entry_Info_t *GLPEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(GLPMCollectorCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure that this callback was registered by the Server*/
            /* Process.                                                 */
            if(((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL) && (GLPEntryInfo->ClientID == MSG_GetServerAddressID()))
            {
               /* Delete the Callback Entry.                            */
               if((GLPEntryInfo = DeleteGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL)
               {
                  /* Free the memory because we are finished with it.   */
                  FreeGLPEntryInfoEntryMemory(GLPEntryInfo);
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is provided to allow a mechanism of        */
   /* starting a Glucose Procedure to a remote Glucose Device.  This    */
   /* function accepts as input the GLP Manager Data Event Callback ID  */
   /* (return value from GLPM_Register_Collector_Event_Callback()       */
   /* function), the BD_ADDR of the remote Glucose Device and a pointer */
   /* to a structure containing the procedure data.  This function      */
   /* returns the positive, non-zero, Procedure ID of the request on    */
   /* success or a negative error code.                                 */
   /* * NOTE * The getGLPMProcedureStarted event will be generated when */
   /*          the remote Glucose Device responds to the Start Procedure*/
   /*          Request.                                                 */
   /* * NOTE * Only 1 Glucose procedure can be outstanding at a time for*/
   /*          each remote Glucose device.  A procedure is completed    */
   /*          when either the getGLPMProcedureStarted event is received*/
   /*          with a error code or if the getGLPMProcedureStopped event*/
   /*          is received for a procedure that started successfully.   */
int BTPSAPI GLPM_Start_Procedure_Request(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, GLPM_Procedure_Data_t *ProcedureData)
{
   int                      ret_val;
   GLPM_Entry_Info_t       *GLPEntryInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if((GLPMCollectorCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ProcedureData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Determine the GLP Callback Information and make sure  */
               /* that it is valid.                                     */
               if(((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL) && (GLPEntryInfo->ClientID == MSG_GetServerAddressID()))
               {
                  /* Determine the connection entry for the specified   */
                  /* connection and make sure we are done configuring   */
                  /* the device.                                        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
                  {
                     /* Simply call the internal function to attempt to */
                     /* start the procedure.                            */
                     ret_val = StartProcedureRequest(ConnectionEntry, GLPEntryInfo, ProcedureData);
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
      ret_val = BTPM_ERROR_CODE_GLUCOSE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* stopping a previouly started Glucose Procedure to a remote Glucose*/
   /* Device.  This function accepts as input the GLP Manager Data Event*/
   /* Callback ID (return value from                                    */
   /* GLPM_Register_Collector_Event_Callback() function), the BD_ADDR of*/
   /* the remote Glucose Device and the Procedure ID that was returned  */
   /* via a successfull call to GLPM_Start_Procedure_Request().  This   */
   /* function returns zero on success or a negative error code.        */
   /* * NOTE * The getGLPMProcedureStoped event will be generated when  */
   /*          the remote Glucse Device responds to the Stop Procedure  */
   /*          Request.                                                 */
int BTPSAPI GLPM_Stop_Procedure_Request(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ProcedureID)
{
   int                      ret_val;
   GLPM_Entry_Info_t       *GLPEntryInfo;
   GLPM_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the GLP Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if((GLPMCollectorCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (ProcedureID))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Determine the GLP Callback Information and make sure  */
               /* that it is valid.                                     */
               if(((GLPEntryInfo = SearchGLPEntryInfoEntry(&GLPEntryInfoList, GLPMCollectorCallbackID)) != NULL) && (GLPEntryInfo->ClientID == MSG_GetServerAddressID()))
               {
                  /* Determine the connection entry for the specified   */
                  /* connection and make sure we are done configuring   */
                  /* the device.                                        */
                  if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csConnected))
                  {
                     /* Simply call the internal function to attempt to */
                     /* start the procedure.                            */
                     ret_val = StopProcedureRequest(ConnectionEntry, GLPEntryInfo, ProcedureID);
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
      ret_val = BTPM_ERROR_CODE_GLUCOSE_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_GLUCOSE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}



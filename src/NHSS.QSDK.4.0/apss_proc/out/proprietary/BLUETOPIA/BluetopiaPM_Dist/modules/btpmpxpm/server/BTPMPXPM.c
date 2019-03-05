/*****< btpmPXPm.c >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMPXPM - PXP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "SS1BTLLS.h"            /* LLS Prototypes/Constants.                 */
#include "SS1BTIAS.h"            /* IAS Prototypes/Constants.                 */
#include "SS1BTTPS.h"            /* TPS Prototypes/Constants.                 */

#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMPXPM.h"            /* BTPM PXP Manager Prototypes/Constants.    */
#include "PXPMAPI.h"             /* PXP Manager Prototypes/Constants.         */
#include "PXPMMSG.h"             /* BTPM PXP Manager Message Formats.         */
#include "PXPMGR.h"              /* PXP Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagPXPM_Event_Callback_Info_t
{
   unsigned int                           EventCallbackID;
   unsigned int                           ClientID;
   PXPM_Event_Callback_t                  EventCallback;
   void                                  *CallbackParameter;
   struct _tagPXPM_Event_Callback_Info_t *NextPXPMEventCallbackInfoPtr;
} PXPM_Event_Callback_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           CallbackID;
   unsigned int           ClientID;
   PXPM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                      BD_ADDR;
   BD_ADDR_t                      PriorBD_ADDR;
   PXPM_Connection_Type_t         ConnectionType;
   unsigned int                   ConnectionID;
   Word_t                         LLS_Alert_Handle;
   Word_t                         IAS_Alert_Handle;
   Word_t                         Transmit_Power_Handle;
   PXPM_Alert_Level_t             LinkLossAlertLevel;
   PXPM_Alert_Level_t             PathLossAlertLevel;
   int                            PathLossThreshold;
   int                            TxPower;
   unsigned int                   LLS_Transaction_ID;
   unsigned int                   TPS_Transaction_ID;
   unsigned long                  ConnectionFlags;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

#define CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID               0x00000001
#define CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_VALID              0x00000002
#define CONNECTION_ENTRY_FLAGS_MONITOR_PATH_LOSS_ALERT_ACTIVE       0x00000004
#define CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE       0x00000008
#define CONNECTION_ENTRY_FLAGS_MONITOR_AWAITING_ENCRYPTION          0x00000010

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Monitor Callback */
   /* ID.                                                               */
static unsigned int NextMonitorEventCallbackID;

   /* Variable which holds the current Link Monitoring state of the     */
   /* device.                                                           */
static Boolean_t LinkMonitoringStarted;

   /* Variable which holds the Link Monitoring Timer ID.                */
static unsigned int LinkMonitoringTimerID;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer to the first element in the Generic*/
   /* Attribute Profile Callback Info List (which holds all PXPM Event  */
   /* Callbacks registered with this module).                           */
static PXPM_Event_Callback_Info_t *MonitorEventCallbackInfoList;
static PXPM_Event_Callback_Info_t *ReporterEventCallbackInfoList;

   /* Variable which holds the default refresh time for the Path Loss   */
   /* Threshold Check.                                                  */
static unsigned int DefaultRefreshTime;

   /* Variable which holds the default Path Loss and Link Loss Alert    */
   /* Level.                                                            */
static PXPM_Alert_Level_t DefaultAlertLevel;

   /* Variable which holds the default Path Loss Threshold.             */
static int DefaultPathLossThreshold;

   /* Variable which holds a pointer to the first element of the Monitor*/
   /* Connection Information List (which holds all currently active     */
   /* Monitor, connections where the local device is the Monitor,       */
   /* connections).                                                     */
static Connection_Entry_t *MonitorConnectionEntryList;

   /* Variable which holds a pointer to the first element of the Report */
   /* Connection Information List (which holds all currently active     */
   /* Report, connections where the local device is the Report,         */
   /* connections).                                                     */
static Connection_Entry_t *ReporterConnectionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextMonitorEventCallbackID(void);

static PXPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(PXPM_Event_Callback_Info_t **ListHead, PXPM_Event_Callback_Info_t *EntryToAdd);
static PXPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(PXPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static PXPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(PXPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID);
static void FreeEventCallbackInfoEntryMemory(PXPM_Event_Callback_Info_t *EntryToFree);
static void FreeEventCallbackInfoList(PXPM_Event_Callback_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *SearchConnectionEntryByConnectionID(Connection_Entry_t **ListHead, unsigned int ConnectionID);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static Byte_t ConvertAlertLevel(PXPM_Alert_Level_t PXPMAlertLevel);

static Boolean_t PopulateLinkLossService(DEVM_Parsed_Services_Data_t *ParsedServiceData, Word_t *LLS_Alert_Handle);
static Boolean_t PopulateImmediateAlertService(DEVM_Parsed_Services_Data_t *ParsedServiceData, Word_t *IAS_Alert_Handle);
static Boolean_t PopulateTransmitPowerService(DEVM_Parsed_Services_Data_t *ParsedServiceData, Word_t *Transmit_Power_Handle);

static Boolean_t ProximityProfileReporterSupported(BD_ADDR_t BD_ADDR, Word_t *LLS_Alert_Handle, Word_t *IAS_Alert_Handle, Word_t *Transmit_Power_Handle);

static int GetPathLoss(Connection_Entry_t *ConnectionEntryPtr, int *PathLossResult);

static void ProcessStartLinkMonitoring(void);

static void DispatchPXPMEvent(PXPM_Event_Data_t *PXPMEventData, BTPM_Message_t *Message, unsigned int *EventCallbackID, unsigned int *MessageCallbackID);

static void DispatchPXPMonitorConnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchPXPMonitorDisconnectionEvent(Connection_Entry_t *ConnectionEntry);
static void DispatchPXPLinkLossAlert(Connection_Entry_t *ConnectionEntry, PXPM_Alert_Level_t AlertLevel);
static void DispatchPXPPathLossAlert(Connection_Entry_t *ConnectionEntry, PXPM_Alert_Level_t AlertLevel);

static void ProcessRegisterMonitorEventsRequestMessage(PXPM_Register_Monitor_Events_Request_t *Message);
static void ProcessUnRegisterMonitorEventsRequestMessage(PXPM_Un_Register_Monitor_Events_Request_t *Message);
static void ProcessSetPathLossRefreshTimeRequestMessage(PXPM_Set_Path_Loss_Refresh_Time_Request_t *Message);
static void ProcessSetPathLossThresholdRequestMessage(PXPM_Set_Path_Loss_Threshold_Request_t *Message);
static void ProcessQueryCurrentPathLossRequestMessage(PXPM_Query_Current_Path_Loss_Request_t *Message);
static void ProcessSetPathLossAlertLevelRequestMessage(PXPM_Set_Path_Loss_Alert_Level_Request_t *Message);
static void ProcessSetLinkLossAlertLevelRequestMessage(PXPM_Set_Link_Loss_Alert_Level_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessGATTErrorResponse(GATT_Request_Error_Data_t *ErrorResponseData);
static void ProcessGATTReadResponse(GATT_Read_Response_Data_t *ReadResponseData);
static void ProcessGATTWriteResponse(GATT_Write_Response_Data_t *WriteResponseData);

static void ProcessGATTClientEvent(GATM_Client_Event_Data_t *GATTClientEventData);

static void ProcessLowEnergyConnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyDisconnectionEvent(BD_ADDR_t BD_ADDR, Boolean_t ForceDown);
static void ProcessLowEnergyAddressUpdateEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
static void ProcessLowEnergyEncryptionChange(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void ProcessLinkMonitorEvent(void);

static void BTPSAPI BTPMDispatchCallback_PXPM(void *CallbackParameter);
static void BTPSAPI BTPMDistpatchCallback_GATT(void *CallbackParameter);
static void BTPSAPI BTPMDistpatchCallback_TMR(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static Boolean_t BTPSAPI LinkMonitorTimerCallback(unsigned int TimerID, void *CallbackParameter);
static void BTPSAPI PXPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the PXP Event Callback List.                                 */
static unsigned int GetNextMonitorEventCallbackID(void)
{
   ++NextMonitorEventCallbackID;

   if(NextMonitorEventCallbackID & 0x80000000)
      NextMonitorEventCallbackID = 1;

   return(NextMonitorEventCallbackID);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            EventCallbackID field is the same as an entry already  */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static PXPM_Event_Callback_Info_t *AddEventCallbackInfoEntry(PXPM_Event_Callback_Info_t **ListHead, PXPM_Event_Callback_Info_t *EntryToAdd)
{
   PXPM_Event_Callback_Info_t *AddedEntry = NULL;
   PXPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->EventCallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (PXPM_Event_Callback_Info_t *)BTPS_AllocateMemory(sizeof(PXPM_Event_Callback_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                              = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextPXPMEventCallbackInfoPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->EventCallbackID == AddedEntry->EventCallbackID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeEventCallbackInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextPXPMEventCallbackInfoPtr)
                        tmpEntry = tmpEntry->NextPXPMEventCallbackInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextPXPMEventCallbackInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Event Callback ID.  This function returns NULL if either*/
   /* the List Head is invalid, the Event Callback ID is invalid, or the*/
   /* specified Event Callback ID was NOT found.                        */
static PXPM_Event_Callback_Info_t *SearchEventCallbackInfoEntry(PXPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   PXPM_Event_Callback_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the list and Event Callback ID to search for      */
   /* appear to be valid.                                               */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
         FoundEntry = FoundEntry->NextPXPMEventCallbackInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the Callback ID  */
   /* is invalid, or the specified Callback ID was NOT present in the   */
   /* list.  The entry returned will have the Next Entry field set to   */
   /* NULL, and the caller is responsible for deleting the memory       */
   /* associated with this entry by calling                             */
   /* FreeEventCallbackInfoEntryMemory().                               */
static PXPM_Event_Callback_Info_t *DeleteEventCallbackInfoEntry(PXPM_Event_Callback_Info_t **ListHead, unsigned int EventCallbackID)
{
   PXPM_Event_Callback_Info_t *FoundEntry = NULL;
   PXPM_Event_Callback_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", EventCallbackID));

   /* Let's make sure the List and Event Callback ID to search for      */
   /* appear to be semi-valid.                                          */
   if((ListHead) && (EventCallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->EventCallbackID != EventCallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextPXPMEventCallbackInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextPXPMEventCallbackInfoPtr = FoundEntry->NextPXPMEventCallbackInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextPXPMEventCallbackInfoPtr;

         FoundEntry->NextPXPMEventCallbackInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Event Callback member.  No check*/
   /* is done on this entry other than making sure it NOT NULL.         */
static void FreeEventCallbackInfoEntryMemory(PXPM_Event_Callback_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Event Callback List.  Upon return of this*/
   /* function, the Head Pointer is set to NULL.                        */
static void FreeEventCallbackInfoList(PXPM_Event_Callback_Info_t **ListHead)
{
   PXPM_Event_Callback_Info_t *EntryToFree;
   PXPM_Event_Callback_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Deleting Event Callback ID: 0x%08X\n", EntryToFree->EventCallbackID));

         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextPXPMEventCallbackInfoPtr;

         FreeEventCallbackInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

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
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) && (!COMPARE_BD_ADDR(FoundEntry->PriorBD_ADDR, BD_ADDR)))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Connection Entry    */
   /* List for the specified Connection Entry based on the specified    */
   /* GATT Connection ID.  This function returns NULL if either the     */
   /* Connection Entry List Head is invalid, the GATT Connection ID is  */
   /* invalid, or the specified Entry was NOT present in the list.      */
static Connection_Entry_t *SearchConnectionEntryByConnectionID(Connection_Entry_t **ListHead, unsigned int ConnectionID)
{
   Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: ConnectionID = %u\n", ConnectionID));

   /* Let's make sure the list and Connection ID to search for appear to*/
   /* be valid.                                                         */
   if((ListHead) && (ConnectionID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->ConnectionID != ConnectionID))
         FoundEntry = FoundEntry->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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
   Connection_Entry_t *FoundEntry = NULL;
   Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) && (!COMPARE_BD_ADDR(FoundEntry->PriorBD_ADDR, BD_ADDR)))
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* convert a PXPM Alert Level to the Alert Level that is sent over   */
   /* the air.                                                          */
static Byte_t ConvertAlertLevel(PXPM_Alert_Level_t PXPMAlertLevel)
{
   Byte_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)PXPMAlertLevel));

   switch(PXPMAlertLevel)
   {
      case alNoAlert:
         ret_val = LLS_ALERT_LEVEL_NO_ALERT;
         break;
      case alMildAlert:
         ret_val = LLS_ALERT_LEVEL_MILD_ALERT;
         break;
      case alHighAlert:
      default:
         ret_val = LLS_ALERT_LEVEL_HIGH_ALERT;
         break;
   }


   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: 0x%02X.\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* populate the Link Loss Alert Handle from the given parsed service */
   /* data.  This function returns TRUE if successful or FALSE          */
   /* otherwise.                                                        */
static Boolean_t PopulateLinkLossService(DEVM_Parsed_Services_Data_t *ParsedServiceData, Word_t *LLS_Alert_Handle)
{
   Boolean_t    ret_val = FALSE;
   unsigned int Index;
   unsigned int Index1;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ParsedServiceData)
   {
      /* Reset the requested handle.                                    */
      if(LLS_Alert_Handle)
         *LLS_Alert_Handle = 0;

      /* Loop through the service list and check to see if the Link Loss*/
      /* Service is present.                                            */
      for(Index=0;Index<ParsedServiceData->NumberServices;Index++)
      {
         /* Check to see if this is the LLS Service.                    */
         if((ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID_Type == guUUID_16) && (LLS_COMPARE_LLS_SERVICE_UUID_TO_UUID_16(ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16)))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("LLS Service Found: 0x%04X - 0x%04X.\n", ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle, ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle));

            /* LLS service found so return success.                     */
            ret_val = TRUE;

            /* If the caller requests the handle we will search for it  */
            /* and save it.                                             */
            if(LLS_Alert_Handle)
            {
               /* Walk the characteristic list and search for the Alert */
               /* Level characteristic.                                 */
               for(Index1=0;Index1<ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics;Index1++)
               {
                  if((ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID_Type == guUUID_16) && (LLS_COMPARE_LLS_ALERT_LEVEL_UUID_TO_UUID_16(ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID.UUID_16)))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("LLS Alert Level Characteristic Found: 0x%04X.\n", ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle));

                     /* Return the Alert Level Characteristic Value     */
                     /* Handle.                                         */
                     *LLS_Alert_Handle = ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle;

                     /* Exit the loop.                                  */
                     break;
                  }
               }

               /* If we did NOT find the MANDATORY characteristic value */
               /* for LLS then we will return FALSE.                    */
               if((LLS_Alert_Handle) && (*LLS_Alert_Handle == 0))
                  ret_val = FALSE;
            }

            /* Nothing more to do so exit the loop.                     */
            break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* populate the Immediate Alert Service Handle from the given parsed */
   /* service data.  This function returns TRUE if successful or FALSE  */
   /* otherwise.                                                        */
static Boolean_t PopulateImmediateAlertService(DEVM_Parsed_Services_Data_t *ParsedServiceData, Word_t *IAS_Alert_Handle)
{
   Boolean_t    ret_val = FALSE;
   unsigned int Index;
   unsigned int Index1;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ParsedServiceData)
   {
      /* Reset the requested handle.                                    */
      if(IAS_Alert_Handle)
         *IAS_Alert_Handle = 0;

      /* Loop through the service list and check to see if the Immediate*/
      /* Alert Service is present.                                      */
      for(Index=0;Index<ParsedServiceData->NumberServices;Index++)
      {
         /* Check to see if this is the IAS Service.                    */
         if((ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID_Type == guUUID_16) && (IAS_COMPARE_IAS_SERVICE_UUID_TO_UUID_16(ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16)))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("IAS Service Found: 0x%04X - 0x%04X.\n", ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle, ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle));

            /* IAS service found so return success.                     */
            ret_val = TRUE;

            /* If the caller requests the handle we will search for it  */
            /* and save it.                                             */
            if(IAS_Alert_Handle)
            {
               /* Walk the characteristic list and search for the Alert */
               /* Level characteristic.                                 */
               for(Index1=0;Index1<ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics;Index1++)
               {
                  if((ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID_Type == guUUID_16) && (IAS_COMPARE_ALERT_LEVEL_UUID_TO_UUID_16(ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID.UUID_16)))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("IAS Alert Level Characteristic Found: 0x%04X.\n", ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle));

                     /* Return the Alert Level Characteristic Value     */
                     /* Handle.                                         */
                     *IAS_Alert_Handle = ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle;

                     /* Exit the loop.                                  */
                     break;
                  }
               }

               /* If we did NOT find the MANDATORY characteristic value */
               /* for IAS then we will return FALSE.                    */
               if((IAS_Alert_Handle) && (*IAS_Alert_Handle == 0))
                  ret_val = FALSE;
            }

            /* Nothing more to do so exit the loop.                     */
            break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* populate the Transmit Power Service Handle from the given parsed  */
   /* service data.  This function returns TRUE if successful or FALSE  */
   /* otherwise.                                                        */
static Boolean_t PopulateTransmitPowerService(DEVM_Parsed_Services_Data_t *ParsedServiceData, Word_t *Transmit_Power_Handle)
{
   Boolean_t    ret_val = FALSE;
   unsigned int Index;
   unsigned int Index1;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(ParsedServiceData)
   {
      /* Reset the requested handle.                                    */
      if(Transmit_Power_Handle)
         *Transmit_Power_Handle = 0;

      /* Loop through the service list and check to see if the Transmit */
      /* Power is present.                                              */
      for(Index=0;Index<ParsedServiceData->NumberServices;Index++)
      {
         /* Check to see if this is the TPS Service.                    */
         if((ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID_Type == guUUID_16) && (TPS_COMPARE_TPS_SERVICE_UUID_TO_UUID_16(ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_16)))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("TPS Service Found: 0x%04X - 0x%04X.\n", ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle, ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle));

            /* TPS service found so return success.                     */
            ret_val = TRUE;

            /* If the caller requests the handle we will search for it  */
            /* and save it.                                             */
            if(Transmit_Power_Handle)
            {
               /* Walk the characteristic list and search for the       */
               /* Transmit Power characteristic.                        */
               for(Index1=0;Index1<ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics;Index1++)
               {
                  if((ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID_Type == guUUID_16) && (TPS_COMPARE_TPS_TX_POWER_LEVEL_UUID_TO_UUID_16(ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID.UUID_16)))
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Transmit Power Characteristic Found: 0x%04X.\n", ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle));

                     /* Return the Transmit Power Characteristic Value  */
                     /* Handle.                                         */
                     *Transmit_Power_Handle = ParsedServiceData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle;

                     /* Exit the loop.                                  */
                     break;
                  }
               }

               /* If we did NOT find the MANDATORY characteristic value */
               /* for TPS then we will return FALSE.                    */
               if((Transmit_Power_Handle) && (*Transmit_Power_Handle == 0))
                  ret_val = FALSE;
            }

            /* Nothing more to do so exit the loop.                     */
            break;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to query */
   /* if a remote device supports the Proximity Profile Report Role.    */
   /* This functions returns TRUE if the remote device supports         */
   /* Proximity or FALSE otherwise.                                     */
static Boolean_t ProximityProfileReporterSupported(BD_ADDR_t BD_ADDR, Word_t *LLS_Alert_Handle, Word_t *IAS_Alert_Handle, Word_t *Transmit_Power_Handle)
{
   int                          Result;
   Byte_t                      *ServiceData;
   Boolean_t                    ret_val = FALSE;
   unsigned int                 TotalServiceSize;
   DEVM_Parsed_Services_Data_t  ParsedGATTData;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Clear any handles that are requested.                          */
      if(LLS_Alert_Handle)
         *LLS_Alert_Handle = 0;

      if(IAS_Alert_Handle)
         *IAS_Alert_Handle = 0;

      if(Transmit_Power_Handle)
         *Transmit_Power_Handle = 0;

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
               if(!DEVM_ConvertRawServicesStreamToParsedServicesData(TotalServiceSize, ServiceData, &ParsedGATTData))
               {
                  /* First determine if the mandatory Link Loss Service */
                  /* is supported.  If the Link Loss Service is         */
                  /* supported we will return TRUE to indicate that     */
                  /* Proximity Report Mode is supported by the remote   */
                  /* device.                                            */
                  ret_val = PopulateLinkLossService(&ParsedGATTData, LLS_Alert_Handle);

                  /* We will attempt to populate the IAS and TPS        */
                  /* handles.  However we will not check to see if they */
                  /* are supported here as these are optional services  */
                  /* for Proximity.                                     */
                  PopulateImmediateAlertService(&ParsedGATTData, IAS_Alert_Handle);
                  PopulateTransmitPowerService(&ParsedGATTData, Transmit_Power_Handle);

                  /* Free the parsed service data.                      */
                  DEVM_FreeParsedServicesData(&ParsedGATTData);
               }
            }

            /* Free the memory that was allocated previously.           */
            BTPS_FreeMemory(ServiceData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %u\n", (unsigned int)ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* calculate the path loss for a specified connection.               */
static int GetPathLoss(Connection_Entry_t *ConnectionEntryPtr, int *PathLossResult)
{
   int ret_val;
   int RSSI;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Verify that the input parameters appear to be semi-valid.         */
   if((ConnectionEntryPtr) && (PathLossResult))
   {
      /* Verify that the Tx Power Value is valid.                       */
      if(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID)
      {
         /* We are monitoring the link.  Therefore get the current RSSI */
         /* of the connection.                                          */
         if(!(ret_val = _PXPM_Get_Link_RSSI(ConnectionEntryPtr->BD_ADDR, &RSSI)))
         {
            /* Now calculate the Path Loss since we have the RSSI.      */
            if(RSSI > ConnectionEntryPtr->TxPower)
               *PathLossResult = 0;
            else
               *PathLossResult = ConnectionEntryPtr->TxPower - RSSI;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_PROXIMITY_TX_POWER_NOT_VALID;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that is used to start*/
   /* (if not already started) the monitoring of the Link Quality.      */
static void ProcessStartLinkMonitoring(void)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Link Monitor Timer is currently active.  If   */
   /* not we need to start the timer.                                   */
   if(!LinkMonitoringStarted)
   {
      /* Start the timer.                                               */
      if((LinkMonitoringTimerID = TMR_StartTimer(NULL, LinkMonitorTimerCallback, DefaultRefreshTime)) > 0)
      {
         /* Flag that the timer is active.                              */
         LinkMonitoringStarted = TRUE;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified PXP event to every registered PXP Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the PXP Manager Lock */
   /*          held.  Upon exit from this function it will free the PXP */
   /*          Manager Lock.                                            */
static void DispatchPXPMEvent(PXPM_Event_Data_t *PXPMEventData, BTPM_Message_t *Message, unsigned int *EventCallbackID, unsigned int *MessageCallbackID)
{
   unsigned int                Index;
   unsigned int                ServerID;
   unsigned int                NumberCallbacks;
   Callback_Info_t             CallbackInfoArray[16];
   Callback_Info_t            *CallbackInfoArrayPtr;
   PXPM_Event_Callback_Info_t *CallbackInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((MonitorEventCallbackInfoList) && (PXPMEventData) && (Message) && (EventCallbackID) && (MessageCallbackID))
   {
      /* Next, let's determine how many callbacks are registered.       */
      CallbackInfoPtr = MonitorEventCallbackInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackInfoPtr)
      {
         if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
            NumberCallbacks++;

         CallbackInfoPtr = CallbackInfoPtr->NextPXPMEventCallbackInfoPtr;
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
            CallbackInfoPtr = MonitorEventCallbackInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackInfoPtr)
            {
               if((CallbackInfoPtr->EventCallback) || (CallbackInfoPtr->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackID        = CallbackInfoPtr->EventCallbackID;
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackInfoPtr->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackInfoPtr->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackInfoPtr->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackInfoPtr = CallbackInfoPtr->NextPXPMEventCallbackInfoPtr;
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
               /*          for PXP events and Data Events.              */
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
                     /* Adjust the Callback ID for this event to the    */
                     /* callback ID for this callback.                  */
                     *EventCallbackID = CallbackInfoArrayPtr[Index].CallbackID;

                     if(CallbackInfoArrayPtr[Index].EventCallback)
                     {
                        (*CallbackInfoArrayPtr[Index].EventCallback)(PXPMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
                     }
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Adjust the Callback ID for this message to the     */
                  /* callback ID for this callback.                     */
                  *MessageCallbackID = CallbackInfoArrayPtr[Index].CallbackID;

                  /* Dispatch the Message.                              */
                  Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

                  MSG_SendMessage(Message);
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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Monitor Connection Event to all registered callbacks.  */
static void DispatchPXPMonitorConnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   unsigned long            SupportedFeatures;
   PXPM_Event_Data_t        EventData;
   PXPM_Connected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Configure the Supported Features for the Connection.           */
      SupportedFeatures  = PXPM_SUPPORTED_FEATURES_FLAGS_LINK_LOSS_ALERT;
      SupportedFeatures |= (unsigned long)(((ConnectionEntry->IAS_Alert_Handle) && (ConnectionEntry->Transmit_Power_Handle))?PXPM_SUPPORTED_FEATURES_FLAGS_PATH_LOSS_ALERT:0);

      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                        = etPXPConnected;
      EventData.EventLength                                      = PXPM_CONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.ConnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
      EventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
      EventData.EventData.ConnectedEventData.SupportedFeatures   = SupportedFeatures;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      Message.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_CONNECTED;
      Message.MessageHeader.MessageLength   = (PXPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = ConnectionEntry->ConnectionType;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
      Message.SupportedFeatures             = SupportedFeatures;

      /* Dispatch the event to all registered callbacks.                */
      DispatchPXPMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.ConnectedEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Monitor Disconnection Event to all registered          */
   /* callbacks.                                                        */
static void DispatchPXPMonitorDisconnectionEvent(Connection_Entry_t *ConnectionEntry)
{
   PXPM_Event_Data_t           EventData;
   PXPM_Disconnected_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ConnectionEntry)
   {
      /* Format the event that will be dispatched locally.              */
      EventData.EventType                                           = etPXPDisconnected;
      EventData.EventLength                                         = PXPM_DISCONNECTED_EVENT_DATA_SIZE;
      EventData.EventData.DisconnectedEventData.ConnectionType      = ConnectionEntry->ConnectionType;
      EventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
      Message.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (PXPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.ConnectionType                = ConnectionEntry->ConnectionType;
      Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

      /* Dispatch the event to all registered callbacks.                */
      DispatchPXPMEvent(&EventData, (BTPM_Message_t *)&Message, &(EventData.EventData.DisconnectedEventData.CallbackID), &(Message.EventHandlerID));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Link Loss Alert for the specified connection.          */
static void DispatchPXPLinkLossAlert(Connection_Entry_t *ConnectionEntry, PXPM_Alert_Level_t AlertLevel)
{
   void                           *CallbackParameter;
   PXPM_Event_Data_t               EventData;
   PXPM_Event_Callback_t           EventCallback;
   PXPM_Event_Callback_Info_t     *CallbackInfoPtr;
   PXPM_Link_Loss_Alert_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* For now we will only allow 1 Monitor Event Callback to be         */
   /* registered at a time.  Whoever registers this will be the "Owner" */
   /* of all Proximity Monitor connections.  In the future this could   */
   /* change and this would have to search the list to figure who to    */
   /* dispatch to.                                                      */
   if((CallbackInfoPtr = MonitorEventCallbackInfoList) != NULL)
   {
      /* Determine if this is a local callback or a callback (via       */
      /* message) to the client.                                        */
      if(CallbackInfoPtr->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         EventData.EventType                                             = etPXPLinkLossAlert;
         EventData.EventLength                                           = PXPM_LINK_LOSS_ALERT_EVENT_DATA_SIZE;

         EventData.EventData.LinkLossAlertEventData.CallbackID           = CallbackInfoPtr->EventCallbackID;
         EventData.EventData.LinkLossAlertEventData.ConnectionType       = ConnectionEntry->ConnectionType;
         EventData.EventData.LinkLossAlertEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
         EventData.EventData.LinkLossAlertEventData.AlertLevel           = AlertLevel;

         /* Save the Event Callback Information.                        */
         EventCallback     = CallbackInfoPtr->EventCallback;
         CallbackParameter = CallbackInfoPtr->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
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
         BTPS_MemInitialize(&Message, 0, (sizeof(PXPM_Link_Loss_Alert_Message_t)));

         Message.MessageHeader.AddressID       = CallbackInfoPtr->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
         Message.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_LINK_LOSS_ALERT;
         Message.MessageHeader.MessageLength   = (PXPM_LINK_LOSS_ALERT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.EventHandlerID                = CallbackInfoPtr->EventCallbackID;
         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.AlertLevel                    = AlertLevel;

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* dispatch a Link Loss Alert for the specified connection.          */
static void DispatchPXPPathLossAlert(Connection_Entry_t *ConnectionEntry, PXPM_Alert_Level_t AlertLevel)
{
   void                           *CallbackParameter;
   PXPM_Event_Data_t               EventData;
   PXPM_Event_Callback_t           EventCallback;
   PXPM_Event_Callback_Info_t     *CallbackInfoPtr;
   PXPM_Path_Loss_Alert_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* For now we will only allow 1 Monitor Event Callback to be         */
   /* registered at a time.  Whoever registers this will be the "Owner" */
   /* of all Proximity Monitor connections.  In the future this could   */
   /* change and this would have to search the list to figure who to    */
   /* dispatch to.                                                      */
   if((CallbackInfoPtr = MonitorEventCallbackInfoList) != NULL)
   {
      /* Determine if this is a local callback or a callback (via       */
      /* message) to the client.                                        */
      if(CallbackInfoPtr->ClientID == MSG_GetServerAddressID())
      {
         /* Format the event to dispatch.                               */
         EventData.EventType                                             = etPXPPathLossAlert;
         EventData.EventLength                                           = PXPM_PATH_LOSS_ALERT_EVENT_DATA_SIZE;

         EventData.EventData.PathLossAlertEventData.CallbackID           = CallbackInfoPtr->EventCallbackID;
         EventData.EventData.PathLossAlertEventData.ConnectionType       = ConnectionEntry->ConnectionType;
         EventData.EventData.PathLossAlertEventData.RemoteDeviceAddress  = ConnectionEntry->BD_ADDR;
         EventData.EventData.PathLossAlertEventData.AlertLevel           = AlertLevel;

         /* Save the Event Callback Information.                        */
         EventCallback     = CallbackInfoPtr->EventCallback;
         CallbackParameter = CallbackInfoPtr->CallbackParameter;

         /* Release the Lock so we can make the callback.               */
         DEVM_ReleaseLock();

         __BTPSTRY
         {
            if(EventCallback)
               (*EventCallback)(&EventData, CallbackParameter);
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
         BTPS_MemInitialize(&Message, 0, (sizeof(PXPM_Link_Loss_Alert_Message_t)));

         Message.MessageHeader.AddressID       = CallbackInfoPtr->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER;
         Message.MessageHeader.MessageFunction = PXPM_MESSAGE_FUNCTION_PATH_LOSS_ALERT;
         Message.MessageHeader.MessageLength   = (PXPM_PATH_LOSS_ALERT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.EventHandlerID                = CallbackInfoPtr->EventCallbackID;
         Message.ConnectionType                = ConnectionEntry->ConnectionType;
         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.AlertLevel                    = AlertLevel;

         /* Go ahead and send the message to the client.                */
         MSG_SendMessage((BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Monitors  */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessRegisterMonitorEventsRequestMessage(PXPM_Register_Monitor_Events_Request_t *Message)
{
   int                                     Result;
   PXPM_Event_Callback_Info_t              EventCallbackEntry;
   PXPM_Register_Monitor_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* For now we will only allow 1 Monitor Event Callback to be      */
      /* registered at a time.  Whoever registers this will be the      */
      /* "Owner" of all Proximity Monitor connections.  In the future   */
      /* this could change and this check would have to be removed.     */
      if(MonitorEventCallbackInfoList == NULL)
      {
         /* Attempt to add an entry into the Event Callback Entry list. */
         BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(PXPM_Event_Callback_Info_t));

         EventCallbackEntry.EventCallbackID   = GetNextMonitorEventCallbackID();
         EventCallbackEntry.ClientID          = Message->MessageHeader.AddressID;
         EventCallbackEntry.EventCallback     = NULL;
         EventCallbackEntry.CallbackParameter = 0;

         if(AddEventCallbackInfoEntry(&MonitorEventCallbackInfoList, &EventCallbackEntry))
            Result = EventCallbackEntry.EventCallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_PROXIMITY_EVENT_ALREADY_REGISTERED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_REGISTER_MONITOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.MonitorEventHandlerID    = (unsigned int)Result;

         ResponseMessage.Status                   = 0;
      }
      else
      {
         ResponseMessage.MonitorEventHandlerID    = 0;

         ResponseMessage.Status                   = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register PXP    */
   /* Events Request Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessUnRegisterMonitorEventsRequestMessage(PXPM_Un_Register_Monitor_Events_Request_t *Message)
{
   int                                         Result;
   PXPM_Event_Callback_Info_t                 *EventCallbackPtr;
   PXPM_Un_Register_Monitor_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the person who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Client Process that is     */
         /* attempting to un-register the callback.                     */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Attempt to delete the callback specified for this device.*/
            if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
            {
               /* Free the memory allocated for this event callback.    */
               FreeEventCallbackInfoEntryMemory(EventCallbackPtr);

               /* Return success.                                       */
               Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_UN_REGISTER_MONITOR_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Path Loss      */
   /* Refresh Time Request Message and responds to the message          */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessSetPathLossRefreshTimeRequestMessage(PXPM_Set_Path_Loss_Refresh_Time_Request_t *Message)
{
   int                                         Result;
   PXPM_Event_Callback_Info_t                 *EventCallbackPtr;
   PXPM_Set_Path_Loss_Refresh_Time_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the person who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Client Process that is     */
         /* attempting to un-register the callback.                     */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Verify that the refresh time is valid.                   */
            if((Message->RefreshTime >= PXPM_CONFIGURATION_MINIMUM_REFRESH_TIME) && (Message->RefreshTime <= PXPM_CONFIGURATION_MAXIMUM_REFRESH_TIME))
            {
               /* Update the Refresh Timer.                             */
               DefaultRefreshTime = Message->RefreshTime;

               /* If the Link Monitor timer is currently active we need */
               /* to change the time-out.                               */
               if(LinkMonitoringStarted)
                  TMR_ChangeTimer(LinkMonitoringTimerID, DefaultRefreshTime);

               /* Return success to the caller.                         */
               Result = 0;
            }
            else
               Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_SET_PATH_LOSS_REFRESH_TIME_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Path Loss      */
   /* Threshold Request Message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessSetPathLossThresholdRequestMessage(PXPM_Set_Path_Loss_Threshold_Request_t *Message)
{
   int                                      Result;
   Connection_Entry_t                      *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t              *EventCallbackPtr;
   PXPM_Set_Path_Loss_Threshold_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the person who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Client Process that is     */
         /* attempting to un-register the callback.                     */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Search for the Connection Entry for this device.         */
            if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, Message->RemoteDevice)) != NULL)
            {
               /* Save the new Path Loss Threshold.                     */
               ConnectionEntryPtr->PathLossThreshold = Message->PathLossThreshold;

               /* Return success to the caller.                         */
               Result                                = 0;
            }
            else
               Result = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_SET_PATH_LOSS_THRESHOLD_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Current Path */
   /* Loss Request Message and responds to the message accordingly.     */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessQueryCurrentPathLossRequestMessage(PXPM_Query_Current_Path_Loss_Request_t *Message)
{
   int                                      PathLossResult;
   int                                      Result;
   Connection_Entry_t                      *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t              *EventCallbackPtr;
   PXPM_Query_Current_Path_Loss_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Initialize the path loss to 0.                                 */
      PathLossResult = 0;

      /* Verify that the owner of this callback is the person who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Client Process that is     */
         /* attempting to un-register the callback.                     */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Search for the Connection Entry for this device.         */
            if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, Message->RemoteDevice)) != NULL)
            {
               /* Simply call the internal function to calculate the    */
               /* path loss.                                            */
               Result = GetPathLoss(ConnectionEntryPtr, &PathLossResult);
            }
            else
               Result = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_QUERY_CURRENT_PATH_LOSS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(!Result)
      {
         ResponseMessage.Status                       = Result;

         ResponseMessage.CurrentPathLoss              = PathLossResult;
      }
      else
      {
         ResponseMessage.Status                       = Result;

         ResponseMessage.CurrentPathLoss              = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Path Loss Alert*/
   /* Level Request Message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessSetPathLossAlertLevelRequestMessage(PXPM_Set_Path_Loss_Alert_Level_Request_t *Message)
{
   int                                        Result;
   Connection_Entry_t                        *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t                *EventCallbackPtr;
   PXPM_Set_Path_Loss_Alert_Level_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the person who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Client Process that is     */
         /* attempting to un-register the callback.                     */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Search for the Connection Entry for this device.         */
            if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, Message->RemoteDevice)) != NULL)
            {
               /* Verify that the alert level is valid.                 */
               if((Message->AlertLevel == alNoAlert) || (Message->AlertLevel == alMildAlert) || (Message->AlertLevel == alHighAlert))
               {
                  /* Save the new Path Loss Threshold.                  */
                  ConnectionEntryPtr->PathLossAlertLevel = Message->AlertLevel;

                  /* Return success to the caller.                      */
                  Result                                 = 0;
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_SET_PATH_LOSS_ALERT_LEVEL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Link Loss Alert*/
   /* Level Request Message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
   /* * NOTE * This function *MUST* be called with the PXP Manager Lock */
   /*          held.                                                    */
static void ProcessSetLinkLossAlertLevelRequestMessage(PXPM_Set_Link_Loss_Alert_Level_Request_t *Message)
{
   int                                        Result;
   Byte_t                                     AlertLevel;
   Connection_Entry_t                        *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t                *EventCallbackPtr;
   PXPM_Set_Link_Loss_Alert_Level_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Verify that the owner of this callback is the person who is    */
      /* doing the un-registering.                                      */
      if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, Message->MonitorEventHandlerID)) != NULL)
      {
         /* Verify that this is owned by the Client Process that is     */
         /* attempting to un-register the callback.                     */
         if(EventCallbackPtr->ClientID == Message->MessageHeader.AddressID)
         {
            /* Search for the Connection Entry for this device.         */
            if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, Message->RemoteDevice)) != NULL)
            {
               /* Verify that the alert level is valid.                 */
               if((Message->AlertLevel == alNoAlert) || (Message->AlertLevel == alMildAlert) || (Message->AlertLevel == alHighAlert))
               {
                  /* If the new Link Loss alert level does not match the*/
                  /* current alert level inform the remote device of the*/
                  /* new value.                                         */
                  if(ConnectionEntryPtr->LinkLossAlertLevel != Message->AlertLevel)
                  {
                     /* Convert the Link Loss Alert Level to one that   */
                     /* can be used over the air.                       */
                     AlertLevel = ConvertAlertLevel(Message->AlertLevel);

                     /* Now Attempt to set the LLS Alert Level (this    */
                     /* MUST be supported by the remote device if it    */
                     /* supports the Reporter Role.                     */
                     _PXPM_Write_Value(ConnectionEntryPtr->ConnectionID, ConnectionEntryPtr->LLS_Alert_Handle, BYTE_SIZE, &AlertLevel);
                  }

                  /* Save the new Link Loss Threshold.                  */
                  ConnectionEntryPtr->LinkLossAlertLevel = Message->AlertLevel;

                  /* Return success to the caller.                      */
                  Result                                 = 0;
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               Result = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

      /* Format and send the response message.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = PXPM_SET_LINK_LOSS_ALERT_LEVEL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the PXP Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case PXPM_MESSAGE_FUNCTION_REGISTER_MONITOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Monitor Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_REGISTER_MONITOR_EVENTS_REQUEST_SIZE)
               ProcessRegisterMonitorEventsRequestMessage((PXPM_Register_Monitor_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_UN_REGISTER_MONITOR_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Monitor Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_UN_REGISTER_MONITOR_EVENTS_REQUEST_SIZE)
               ProcessUnRegisterMonitorEventsRequestMessage((PXPM_Un_Register_Monitor_Events_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_REFRESH_TIME:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Path Loss Refresh Time Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_SET_PATH_LOSS_REFRESH_TIME_REQUEST_SIZE)
               ProcessSetPathLossRefreshTimeRequestMessage((PXPM_Set_Path_Loss_Refresh_Time_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_THRESHOLD:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Path Loss Threshold Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_SET_PATH_LOSS_THRESHOLD_REQUEST_SIZE)
               ProcessSetPathLossThresholdRequestMessage((PXPM_Set_Path_Loss_Threshold_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_QUERY_CURRENT_PATH_LOSS:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Current Path Loss Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_QUERY_CURRENT_PATH_LOSS_REQUEST_SIZE)
               ProcessQueryCurrentPathLossRequestMessage((PXPM_Query_Current_Path_Loss_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_SET_PATH_LOSS_ALERT_LEVEL:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Path Loss Alert Level Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_SET_PATH_LOSS_ALERT_LEVEL_REQUEST_SIZE)
               ProcessSetPathLossAlertLevelRequestMessage((PXPM_Set_Path_Loss_Alert_Level_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case PXPM_MESSAGE_FUNCTION_SET_LINK_LOSS_ALERT_LEVEL:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Link Loss Alert Level Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= PXPM_SET_LINK_LOSS_ALERT_LEVEL_RESPONSE_SIZE)
               ProcessSetLinkLossAlertLevelRequestMessage((PXPM_Set_Link_Loss_Alert_Level_Request_t *)Message);
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   PXPM_Event_Callback_Info_t *EventCallback;
   PXPM_Event_Callback_Info_t *tmpEventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* Loop through the event callback list and delete all callbacks  */
      /* registered for this callback.                                  */
      EventCallback = MonitorEventCallbackInfoList;
      while(EventCallback)
      {
         /* Check to see if the current Client Information is the one   */
         /* that is being un-registered.                                */
         if(EventCallback->ClientID == ClientID)
         {
            /* Note the next Event Callback Entry in the list (we are   */
            /* about to delete the current entry).                      */
            tmpEventCallback = EventCallback->NextPXPMEventCallbackInfoPtr;

            /* Go ahead and delete the Event Callback Entry and clean up*/
            /* the resources.                                           */
            if((EventCallback = DeleteEventCallbackInfoEntry(&MonitorEventCallbackInfoList, EventCallback->EventCallbackID)) != NULL)
            {
               /* All finished with the memory so free the entry.       */
               FreeEventCallbackInfoEntryMemory(EventCallback);
            }

            /* Go ahead and set the next Event Callback Entry (past the */
            /* one we just deleted).                                    */
            EventCallback = tmpEventCallback;
         }
         else
            EventCallback = EventCallback->NextPXPMEventCallbackInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a GATT Error */
   /* Response Event that has been received with the specified          */
   /* information.  This function should be called with the Lock        */
   /* protecting the PXP Manager Information held.                      */
static void ProcessGATTErrorResponse(GATT_Request_Error_Data_t *ErrorResponseData)
{
   Connection_Entry_t              *ConnectionEntry;
   DEVM_Remote_Device_Properties_t  RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(ErrorResponseData)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXPM GATT Error Type: %u.\n", (unsigned int)ErrorResponseData->ErrorType));
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXPM GATT Error Code: 0x%02X.\n", ErrorResponseData->ErrorCode));

      /* Search for the connection entry for this connection.           */
      if((ConnectionEntry = SearchConnectionEntryByConnectionID(&MonitorConnectionEntryList, ErrorResponseData->ConnectionID)) != NULL)
      {
         /* Cancel any outstanding transactions and clear the save      */
         /* Transaction IDs.                                            */
         if(ConnectionEntry->LLS_Transaction_ID != ErrorResponseData->TransactionID)
            _PXPM_Cancel_Transaction(ConnectionEntry->LLS_Transaction_ID);

         if(ConnectionEntry->TPS_Transaction_ID != ErrorResponseData->TransactionID)
            _PXPM_Cancel_Transaction(ConnectionEntry->TPS_Transaction_ID);

         ConnectionEntry->LLS_Transaction_ID = 0;
         ConnectionEntry->TPS_Transaction_ID = 0;

         /* Check to see if this is a security requirement error code.  */
         /* In this case we won't disconnect the link but will just     */
         /* re-submit the request later (as long is this error did not  */
         /* occur after encryption took place).                         */
         if((!(ConnectionEntry->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_AWAITING_ENCRYPTION)) && (ErrorResponseData->ErrorType == retErrorResponse) && ((ErrorResponseData->ErrorCode == ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION) || (ErrorResponseData->ErrorCode == ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION)))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Awaiting encryption for Monitor Connection...\n"));

            /* Flag that the Transmit Power and Link Loss values are NOT*/
            /* valid.                                                   */
            ConnectionEntry->ConnectionFlags &= ~((unsigned long)(CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID | CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_VALID));

            /* Flag that we are awaiting encryption for the device.     */
            ConnectionEntry->ConnectionFlags |= CONNECTION_ENTRY_FLAGS_MONITOR_AWAITING_ENCRYPTION;

            /* There is a race condition.  By the time we get an error  */
            /* to a request submited when the LE link was not encrypted */
            /* the link could have become encrypted.  Therefore we will */
            /* check here if the link is encrypted and if so resubmit   */
            /* the request.                                             */
            if(!DEVM_QueryRemoteDeviceProperties(ConnectionEntry->BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
            {
               /* Check to see if the LE Link is currently encrypted.   */
               if(RemoteDeviceProperties.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED)
               {
                  /* Process the Encryption Change so that the requests */
                  /* will be resubmitted.                               */
                  ProcessLowEnergyEncryptionChange(&RemoteDeviceProperties);
               }
            }
         }
         else
         {
            /* This is some unknown error so just disconnect the link.  */
            ProcessLowEnergyDisconnectionEvent(ConnectionEntry->BD_ADDR, TRUE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a GATT Read  */
   /* Response Event that has been received with the specified          */
   /* information.  This function should be called with the Lock        */
   /* protecting the PXP Manager Information held.                      */
static void ProcessGATTReadResponse(GATT_Read_Response_Data_t *ReadResponseData)
{
   Connection_Entry_t              *ConnectionEntryPtr;
   DEVM_Remote_Device_Properties_t  RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(ReadResponseData)
   {
      /* Query the remote device properties so that we can get the      */
      /* BD_ADDR that we use internally.                                */
      if(!DEVM_QueryRemoteDeviceProperties(ReadResponseData->RemoteDevice, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
      {
         /* Verify that the length is valid for the Tx Power            */
         /* characteristic.                                             */
         if(ReadResponseData->AttributeValueLength == TPS_TX_POWER_LEVEL_LENGTH)
         {
            /* Search for the connection entry for this device.         */
            if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, RemoteDeviceProperties.BD_ADDR)) != NULL)
            {
               /* Reset the TPS Transaction ID.                         */
               ConnectionEntryPtr->TPS_Transaction_ID = 0;

               /* Read the transmit power from the packet.              */
               ConnectionEntryPtr->TxPower            = (int)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(ReadResponseData->AttributeValue);

               /* Verify that the transmit power value is valid.        */
               if((ConnectionEntryPtr->TxPower >= TPS_TX_POWER_MIN_VALUE) && (ConnectionEntryPtr->TxPower <= TPS_TX_POWER_MAX_VALUE))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Tx Power for device = %d dBm\n", ConnectionEntryPtr->TxPower));

                  /* Flag that the Transmit Power is valid.             */
                  ConnectionEntryPtr->ConnectionFlags |= (unsigned long)CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID;

                  /* This is the last characteristic that is read or    */
                  /* written.  So if we are here we can clear the       */
                  /* waiting for encryption flag as it must have been   */
                  /* resolved.                                          */
                  ConnectionEntryPtr->ConnectionFlags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_MONITOR_AWAITING_ENCRYPTION);

                  /* Start the Link Monitoring process.                 */
                  ProcessStartLinkMonitoring();
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Incorrect Tx Power Level Value: %d\n", ConnectionEntryPtr->TxPower));

                  ProcessLowEnergyDisconnectionEvent(RemoteDeviceProperties.BD_ADDR, TRUE);
               }
            }
         }
         else
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Incorrect Tx Power Level Length: %u\n", (unsigned int)ReadResponseData->AttributeValueLength));

            ProcessLowEnergyDisconnectionEvent(RemoteDeviceProperties.BD_ADDR, TRUE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a GATT Write */
   /* Response Event that has been received with the specified          */
   /* information.  This function should be called with the Lock        */
   /* protecting the PXP Manager Information held.                      */
static void ProcessGATTWriteResponse(GATT_Write_Response_Data_t *WriteResponseData)
{
   Connection_Entry_t              *ConnectionEntryPtr;
   DEVM_Remote_Device_Properties_t  RemoteDeviceProperties;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(WriteResponseData)
   {
      /* Query the remote device properties so that we can get the      */
      /* BD_ADDR that we use internally.                                */
      if(!DEVM_QueryRemoteDeviceProperties(WriteResponseData->RemoteDevice, DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY, &RemoteDeviceProperties))
      {
         /* Search for the connection entry for this device.            */
         if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, RemoteDeviceProperties.BD_ADDR)) != NULL)
         {
            /* If either IAS or TPS Services is not supported we can    */
            /* clear the waiting for encryption flag here.              */
            if((!ConnectionEntryPtr->IAS_Alert_Handle) || (!ConnectionEntryPtr->Transmit_Power_Handle))
               ConnectionEntryPtr->ConnectionFlags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_MONITOR_AWAITING_ENCRYPTION);

            /* Flag that the Link Loss value was successfully written to*/
            /* the remote device.                                       */
            ConnectionEntryPtr->ConnectionFlags |= CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_VALID;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing GATT Client Events that have been received.  This      */
   /* function should ONLY be called with the Context locked AND ONLY in*/
   /* the context of an arbitrary processing thread.                    */
static void ProcessGATTClientEvent(GATM_Client_Event_Data_t *GATTClientEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(GATTClientEventData)
   {
      /* Process the event based on the event type.                     */
      switch(GATTClientEventData->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXPM GATT Client Event: Error Response\n"));

            ProcessGATTErrorResponse(&(GATTClientEventData->Event_Data.GATT_Request_Error_Data));
            break;
         case etGATT_Client_Read_Response:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXPM GATT Client Event: Read Response (must be Tx Power)\n"));

            ProcessGATTReadResponse(&(GATTClientEventData->Event_Data.GATT_Read_Response_Data));
            break;
         case etGATT_Client_Write_Response:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXPM GATT Client Event: Write Response (must be Link Loss)\n"));

            ProcessGATTWriteResponse(&(GATTClientEventData->Event_Data.GATT_Write_Response_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown GATT Client Event Type: %d\n", GATTClientEventData->Event_Data_Type));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid GATT Client Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Low Energy Connection Events.                           */
   /* * NOTE * Low Energy Connection here means either a LE physical    */
   /*          connection for a device whose services are known OR when */
   /*          the LE services become known for a previously connected  */
   /*          LE connection.                                           */
static void ProcessLowEnergyConnectionEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   int                 Result;
   Byte_t              AlertLevel;
   Word_t              LLS_Alert_Handle;
   Word_t              IAS_Alert_Handle;
   Word_t              Transmit_Power_Handle;
   BD_ADDR_t           BD_ADDR;
   unsigned int        WriteTransactionID;
   unsigned int        ReadTransactionID;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Clear the Transaction ID.                                      */
      WriteTransactionID    = 0;
      ReadTransactionID     = 0;

      /* Initialize the characteristic handles.                         */
      LLS_Alert_Handle      = 0;
      IAS_Alert_Handle      = 0;
      Transmit_Power_Handle = 0;

      /* Check to see if this device supports Proximity.                */
      if(ProximityProfileReporterSupported(RemoteDeviceProperties->BD_ADDR, &LLS_Alert_Handle, &IAS_Alert_Handle, &Transmit_Power_Handle))
      {
         /* Proximity Reporter Role is supported by device so go ahead  */
         /* and add a connection entry for the device.                  */
         BTPS_MemInitialize(&ConnectionEntry, 0, (sizeof(ConnectionEntry)));

         ConnectionEntry.BD_ADDR               = RemoteDeviceProperties->BD_ADDR;
         ConnectionEntry.ConnectionType        = pctPXPMonitor;
         ConnectionEntry.LLS_Alert_Handle      = LLS_Alert_Handle;
         ConnectionEntry.IAS_Alert_Handle      = IAS_Alert_Handle;
         ConnectionEntry.Transmit_Power_Handle = Transmit_Power_Handle;
         ConnectionEntry.PathLossThreshold     = DefaultPathLossThreshold;
         ConnectionEntry.LinkLossAlertLevel    = DefaultAlertLevel;
         ConnectionEntry.PathLossAlertLevel    = DefaultAlertLevel;

         /* Query the Connection ID for the connection.                 */
         BD_ADDR = RemoteDeviceProperties->BD_ADDR;

         /* If the Prior Resolvable BD_ADDR is valid we will use that   */
         /* instead.                                                    */
         if(!COMPARE_NULL_BD_ADDR(RemoteDeviceProperties->PriorResolvableBD_ADDR))
            BD_ADDR = RemoteDeviceProperties->PriorResolvableBD_ADDR;

         /* Finally query and save the GATT Connection ID for the       */
         /* connection.                                                 */
         if(!_PXPM_Query_Connection_ID(BD_ADDR, &(ConnectionEntry.ConnectionID)))
         {
            /* Convert the Link Loss Alert Level to one that can be used*/
            /* over the air.                                            */
            AlertLevel = ConvertAlertLevel(ConnectionEntry.LinkLossAlertLevel);

            /* Now Attempt to set the LLS Alert Level (this MUST be     */
            /* supported by the remote device if it supports the        */
            /* Reporter Role.                                           */
            if((Result = _PXPM_Write_Value(ConnectionEntry.ConnectionID, ConnectionEntry.LLS_Alert_Handle, BYTE_SIZE, &AlertLevel)) > 0)
            {
               /* Save the Transaction ID for the write in case it needs*/
               /* to be canceled.                                       */
               WriteTransactionID = (unsigned int)Result;

               /* Read the Transmit Power if necessary.                 */
               if((!ConnectionEntry.IAS_Alert_Handle) || (!ConnectionEntry.Transmit_Power_Handle) || ((Result = _PXPM_Read_Value(ConnectionEntry.ConnectionID, ConnectionEntry.Transmit_Power_Handle)) >= 0))
               {
                  /* Save the Transaction ID if necessary.              */
                  if((ConnectionEntry.IAS_Alert_Handle) && (ConnectionEntry.Transmit_Power_Handle))
                     ReadTransactionID = (unsigned int)Result;

                  /* Check to see if we already have a connection entry */
                  /* for this connection.                               */
                  if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, RemoteDeviceProperties->BD_ADDR)) != NULL)
                  {
                     /* Save the Connection ID of the connection to the */
                     /* device.                                         */
                     ConnectionEntryPtr->ConnectionID          = ConnectionEntry.ConnectionID;

                     /* Save the Handles of the services.               */
                     ConnectionEntryPtr->LLS_Alert_Handle      = LLS_Alert_Handle;
                     ConnectionEntryPtr->IAS_Alert_Handle      = IAS_Alert_Handle;
                     ConnectionEntryPtr->Transmit_Power_Handle = Transmit_Power_Handle;
                  }
                  else
                  {
                     /* We do not have an already existing entry so go  */
                     /* ahead and add it to the list.                   */
                     ConnectionEntryPtr = AddConnectionEntry(&MonitorConnectionEntryList, &ConnectionEntry);
                  }

                  /* Attempt to add the connection to the connection    */
                  /* list.                                              */
                  if(ConnectionEntryPtr)
                  {
                     /* Save the Transaction IDs for the currently      */
                     /* outstanding transactions.                       */
                     ConnectionEntryPtr->LLS_Transaction_ID = WriteTransactionID;
                     ConnectionEntryPtr->TPS_Transaction_ID = ReadTransactionID;

                     /* If a Link Loss Alert is active go ahead and     */
                     /* dispatch an event to cancel the alert (since we */
                     /* are re-connected to the device).                */
                     if(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE)
                     {
                        /* Dispatch the No Alert-Alert.                 */
                        DispatchPXPLinkLossAlert(ConnectionEntryPtr, alNoAlert);

                        /* Flag that the alert is no longer active.     */
                        ConnectionEntryPtr->ConnectionFlags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE);
                     }

                     /* Dispatch the Proximity Monitor Connected Event. */
                     DispatchPXPMonitorConnectionEvent(ConnectionEntryPtr);
                  }
                  else
                  {
                     /* If we failed to add the connection entry we     */
                     /* should cancel the outstanding transactions.     */
                     _PXPM_Cancel_Transaction(WriteTransactionID);

                     if(ReadTransactionID)
                        _PXPM_Cancel_Transaction(ReadTransactionID);
                  }
               }
               else
                  _PXPM_Cancel_Transaction(WriteTransactionID);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Low Energy Disconnection Events.                        */
   /* * NOTE * Low Energy Disconnection here means either a LE physical */
   /*          connection for a device whose services are known has     */
   /*          disconnected OR when the LE services that are known      */
   /*          become invalidated.                                      */
static void ProcessLowEnergyDisconnectionEvent(BD_ADDR_t BD_ADDR, Boolean_t ForceDown)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", (unsigned int)ForceDown));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
   {
      /* Delete the Monitor (Local) Connection entry if any exists for  */
      /* this device.                                                   */
      if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, BD_ADDR)) != NULL)
      {
         /* Clear the Connection ID since we are no longer connected.   */
         ConnectionEntryPtr->ConnectionID     = 0;

         /* Cancel an outstanding transactions.                         */
         if(ConnectionEntryPtr->LLS_Transaction_ID)
         {
            _PXPM_Cancel_Transaction(ConnectionEntryPtr->LLS_Transaction_ID);

            ConnectionEntryPtr->LLS_Transaction_ID = 0;
         }

         if(ConnectionEntryPtr->TPS_Transaction_ID)
         {
            _PXPM_Cancel_Transaction(ConnectionEntryPtr->TPS_Transaction_ID);

            ConnectionEntryPtr->TPS_Transaction_ID = 0;
         }

         /* If the Link Loss characteristic was configured properly we  */
         /* should dispatch a Link Loss Alert here.                     */
         if((ForceDown == FALSE) && (ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_VALID))
         {
            /* If an alert level is specified go ahead and issue the    */
            /* alert.                                                   */
            if(ConnectionEntryPtr->LinkLossAlertLevel != alNoAlert)
            {
               /* Dispatch the Link Loss Alert.                         */
               DispatchPXPLinkLossAlert(ConnectionEntryPtr, ConnectionEntryPtr->LinkLossAlertLevel);

               /* Flag that the Link Loss Alert is active.              */
               ConnectionEntryPtr->ConnectionFlags |= CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE;
            }
         }

         /* Flag that the Link Loss and Transmit Power values are no    */
         /* longer valid.                                               */
         ConnectionEntryPtr->ConnectionFlags &= ~((unsigned long)(CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_VALID | CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID));

         /* Dispatch a Proximity Monitor Disconnection.                 */
         DispatchPXPMonitorDisconnectionEvent(ConnectionEntryPtr);

         /* If we did not issue the link loss alert then we will go     */
         /* ahead and delete the connection entry.                      */
         if(!(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE))
         {
            /* Delete the entry and free the memory allocated for this  */
            /* entry.                                                   */
            if((ConnectionEntryPtr = DeleteConnectionEntry(&MonitorConnectionEntryList, BD_ADDR)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntryPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Low Energy Address Update Events.                       */
static void ProcessLowEnergyAddressUpdateEvent(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Delete the Monitor (Local) Connection entry if any exists for  */
      /* this device.                                                   */
      if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, RemoteDeviceProperties->PriorResolvableBD_ADDR)) != NULL)
      {
         /* Save the prior BD_ADDR that was being used.                 */
         ConnectionEntryPtr->PriorBD_ADDR = ConnectionEntryPtr->BD_ADDR;

         /* Save the new "Base" address for the connection.             */
         ConnectionEntryPtr->BD_ADDR      = RemoteDeviceProperties->BD_ADDR;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Low Energy Encryption Change event.                     */
static void ProcessLowEnergyEncryptionChange(DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   int                 Result;
   Byte_t              AlertLevel;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED))
   {
      /* Walk the monitor list and see if any connections are awaiting  */
      /* encryption.                                                    */
      ConnectionEntryPtr = MonitorConnectionEntryList;
      while(ConnectionEntryPtr)
      {
         /* Check to see if this device is awaiting encryption.         */
         if(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_AWAITING_ENCRYPTION)
         {
            /* Convert the Link Loss Alert Level to one that can be used*/
            /* over the air.                                            */
            AlertLevel = ConvertAlertLevel(ConnectionEntryPtr->LinkLossAlertLevel);

            /* Now Attempt to set the LLS Alert Level (this MUST be     */
            /* supported by the remote device if it supports the        */
            /* Reporter Role.                                           */
            if((Result = _PXPM_Write_Value(ConnectionEntryPtr->ConnectionID, ConnectionEntryPtr->LLS_Alert_Handle, BYTE_SIZE, &AlertLevel)) > 0)
            {
               /* Save the Transaction ID for the write in case it needs*/
               /* to be canceled.                                       */
               ConnectionEntryPtr->LLS_Transaction_ID = (unsigned int)Result;

               /* Read the Transmit Power if necessary.                 */
               if((!ConnectionEntryPtr->IAS_Alert_Handle) || (!ConnectionEntryPtr->Transmit_Power_Handle) || ((Result = _PXPM_Read_Value(ConnectionEntryPtr->ConnectionID, ConnectionEntryPtr->Transmit_Power_Handle)) >= 0))
               {
                  /* Save the Transaction ID if necessary.              */
                  if((ConnectionEntryPtr->IAS_Alert_Handle) && (ConnectionEntryPtr->Transmit_Power_Handle))
                     ConnectionEntryPtr->TPS_Transaction_ID = (unsigned int)Result;
               }
               else
               {
                  /* Error occurred so just disconnect the monitor      */
                  /* connection.                                        */
                  ProcessLowEnergyDisconnectionEvent(ConnectionEntryPtr->BD_ADDR, TRUE);
               }
            }
            else
            {
               /* Error occurred so just disconnect the monitor         */
               /* connection.                                           */
               ProcessLowEnergyDisconnectionEvent(ConnectionEntryPtr->BD_ADDR, TRUE);
            }
         }

         ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Link is not currently encrypted...\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* process a Remote Device Properties Changed Event.                 */
static void ProcessRemoteDevicePropertiesChangedEvent(unsigned long ChangedMemberMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08lX\n", ChangedMemberMask));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if((ChangedMemberMask) && (RemoteDeviceProperties) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY))
   {
      /* Check to see what changed.  We are only interested if the LE   */
      /* Connection State or the LE Service state or the Address is     */
      /* updated.                                                       */
      if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE))
      {
         /* Handle Connections/Disconnections.                          */
         if(ChangedMemberMask & (DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE | DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Flags: 0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags));

            /* Check to see if we are currently connected AND know the  */
            /* services of an LE device.                                */
            if((RemoteDeviceProperties->RemoteDeviceFlags & (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)) == (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN))
            {
               /* Process the Low Energy Connection.                    */
               ProcessLowEnergyConnectionEvent(RemoteDeviceProperties);
            }
            else
            {
               /* If the LE Services are no longer known we will force  */
               /* the link down.                                        */
               if(!(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN))
                  ProcessLowEnergyDisconnectionEvent(RemoteDeviceProperties->BD_ADDR, TRUE);
               else
                  ProcessLowEnergyDisconnectionEvent(RemoteDeviceProperties->BD_ADDR, FALSE);
            }
         }

         /* Process the Address Updated event.                          */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS)
            ProcessLowEnergyAddressUpdateEvent(RemoteDeviceProperties);

         /* Process the LE Encryption Change event.                     */
         if(ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE)
            ProcessLowEnergyEncryptionChange(RemoteDeviceProperties);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that is used to      */
   /* monitor the link quality for all connected Monitor connections.   */
static void ProcessLinkMonitorEvent(void)
{
   int                 PathLoss;
   Byte_t              AlertLevel;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Walk the monitor connection list.                                 */
   ConnectionEntryPtr = MonitorConnectionEntryList;
   while(ConnectionEntryPtr)
   {
      /* Verify that there is no Link Loss Alert active (indicating we  */
      /* are not connected).                                            */
      if(!(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE))
      {
         /* Check to see if we are monitoring the link for this         */
         /* connection.                                                 */
         if((ConnectionEntryPtr->IAS_Alert_Handle) && (ConnectionEntryPtr->Transmit_Power_Handle) && (ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID))
         {
            /* We are monitoring the link.  Therefore get the current   */
            /* Path Loss of the connection.                             */
            PathLoss = 0;
            if(!GetPathLoss(ConnectionEntryPtr, &PathLoss))
            {
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Path Loss: Loss = %d, Threshold = %d.\n", PathLoss, ConnectionEntryPtr->PathLossThreshold));

               /* Check to see if the path loss exceeds the threshold.  */
               if(PathLoss >= ConnectionEntryPtr->PathLossThreshold)
               {
                  /* Verify that the specified alert level is not no    */
                  /* alert.                                             */
                  if(ConnectionEntryPtr->PathLossAlertLevel != alNoAlert)
                  {
                     /* Make sure that we don't dispatch duplicate      */
                     /* events.                                         */
                     if(!(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_PATH_LOSS_ALERT_ACTIVE))
                     {
                        /* Convert the Link Loss Alert Level to one that*/
                        /* can be used over the air.                    */
                        AlertLevel = ConvertAlertLevel(ConnectionEntryPtr->PathLossAlertLevel);

                        /* Write to the remote device to inform it of   */
                        /* the Path Loss Alert.                         */
                        _PXPM_Write_Value_Without_Response(ConnectionEntryPtr->ConnectionID, ConnectionEntryPtr->IAS_Alert_Handle, BYTE_SIZE, &AlertLevel);

                        /* Dispatch the Path Loss Alert.                */
                        DispatchPXPPathLossAlert(ConnectionEntryPtr, ConnectionEntryPtr->PathLossAlertLevel);

                        /* Flag that the alert is active.               */
                        ConnectionEntryPtr->ConnectionFlags |= CONNECTION_ENTRY_FLAGS_MONITOR_PATH_LOSS_ALERT_ACTIVE;
                     }
                  }
               }
               else
               {
                  /* The Path Loss is NOT greater than the threshold.   */
                  /* Therefore check to see if a Path Loss Alert is     */
                  /* active and if so cancel it.                        */
                  if(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_PATH_LOSS_ALERT_ACTIVE)
                  {
                     /* Convert the Link Loss Alert Level to one that   */
                     /* can be used over the air.                       */
                     AlertLevel = ConvertAlertLevel(alNoAlert);

                     /* Write to the remote device to inform it of the  */
                     /* Path Loss Alert that is no longer alerting.     */
                     _PXPM_Write_Value_Without_Response(ConnectionEntryPtr->ConnectionID, ConnectionEntryPtr->IAS_Alert_Handle, BYTE_SIZE, &AlertLevel);

                     /* Dispatch the Path Loss Alert to cancel the      */
                     /* previous alert.                                 */
                     DispatchPXPPathLossAlert(ConnectionEntryPtr, alNoAlert);

                     /* Flag that the alert is no loner active.         */
                     ConnectionEntryPtr->ConnectionFlags &= ~((unsigned long)CONNECTION_ENTRY_FLAGS_MONITOR_PATH_LOSS_ALERT_ACTIVE);
                  }
               }
            }
         }
      }

      ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process PXP Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_PXPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process GATT Connection Events.                     */
static void BTPSAPI BTPMDistpatchCallback_GATT(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an GATT Client Event Update.   */
            if(((PXPM_Update_Data_t *)CallbackParameter)->UpdateType == utGATTClientEvent)
               ProcessGATTClientEvent(&(((PXPM_Update_Data_t *)CallbackParameter)->UpdateData.ClientEventData));

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process TMR events (for monitor link quality).      */
static void BTPSAPI BTPMDistpatchCallback_TMR(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, let's check to see if we need to process it.                */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Process the Link Monitor event.                             */
         ProcessLinkMonitorEvent();

         /* Release the lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Link Monitoring Timer Callback      */
   /* function which is used to process monitor the links of connected  */
   /* devices.                                                          */
static Boolean_t BTPSAPI LinkMonitorTimerCallback(unsigned int TimerID, void *CallbackParameter)
{
   Boolean_t           ret_val = FALSE;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, let's check to see if we need to process it.                */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Walk the monitor list as needed.                            */
         ConnectionEntryPtr = MonitorConnectionEntryList;
         while(ConnectionEntryPtr)
         {
            /* Verify that there is no Link Loss Alert active           */
            /* (indicating we are not connected).                       */
            if(!(ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_LINK_LOSS_ALERT_ACTIVE))
            {
               /* Check to see if we should monitor the link for this   */
               /* connection.                                           */
               if((ConnectionEntryPtr->IAS_Alert_Handle) && (ConnectionEntryPtr->Transmit_Power_Handle) && (ConnectionEntryPtr->ConnectionFlags & CONNECTION_ENTRY_FLAGS_MONITOR_TX_POWER_VALID))
               {
                  /* Flag that the timer should continue.               */
                  ret_val = TRUE;

                  /* Nothing further to do here since we have at least 1*/
                  /* Monitor connection to monitor.  We should exit the */
                  /* loop here.                                         */
                  break;
               }
            }

            ConnectionEntryPtr = ConnectionEntryPtr->NextConnectionEntryPtr;
         }

         /* If we are not going to continue the timer after this call   */
         /* then we need to flag that the Link Monitor Timer is no      */
         /* longer active.                                              */
         if(ret_val)
         {
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Timer Callback to monitor link quality...\n"));

            /* Queue a callback to monitor the links.                   */
            ret_val = BTPM_QueueMailboxCallback(BTPMDistpatchCallback_TMR, (void *)NULL);
            if(!ret_val)
            {
               LinkMonitoringStarted = FALSE;
               LinkMonitoringTimerID = 0;
            }
         }
         else
         {
            LinkMonitoringStarted = FALSE;
            LinkMonitoringTimerID = 0;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all PXP Manager Messages.   */
static void BTPSAPI PXPManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a PXP Manager defined    */
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
               /* PXP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_PXPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PXP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue PXP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an PXP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Non PXP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager PXP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI PXPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing PXP Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process PXP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER, PXPManagerGroupHandler, NULL))
         {
            /* Configure the initialization data.                       */
            if(InitializationData)
            {
               DefaultRefreshTime       = ((PXPM_Initialization_Info_t *)InitializationData)->DefaultRefreshTime;
               DefaultAlertLevel        = ((PXPM_Initialization_Info_t *)InitializationData)->DefaultAlertLevel;
               DefaultPathLossThreshold = ((PXPM_Initialization_Info_t *)InitializationData)->DefaultPathLossThreshold;
            }
            else
            {
               DefaultRefreshTime       = PXPM_CONFIGURATION_MINIMUM_REFRESH_TIME;
               DefaultAlertLevel        = PXPM_CONFIGURATION_MAXIMUM_REFRESH_TIME;
               DefaultPathLossThreshold = PXPM_CONFIGURATION_DEFAULT_PATH_LOSS_THRESHOLD;
            }

            /* Initialize the actual PXP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the PXP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _PXPM_Initialize()))
            {
               /* Determine the current Device Power State.             */
               CurrentPowerState          = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Initialize a unique, starting PXP Callback ID.        */
               NextMonitorEventCallbackID = 0;

               /* Go ahead and flag that this module is initialized.    */
               Initialized                = TRUE;

               /* Clear the link monitoring variables.                  */
               LinkMonitoringStarted      = FALSE;
               LinkMonitoringTimerID      = 0;

               /* Flag success.                                         */
               Result                     = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _PXPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("PXP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_PROXIMITY_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the PXP Manager Implementation that  */
            /* we are shutting down.                                    */
            _PXPM_Cleanup();

            /* Free the Connection Info lists.                          */
            FreeConnectionEntryList(&MonitorConnectionEntryList);
            FreeConnectionEntryList(&ReporterConnectionEntryList);

            /* Free the Event Callback Info List.                       */
            FreeEventCallbackInfoList(&MonitorEventCallbackInfoList);
            FreeEventCallbackInfoList(&ReporterEventCallbackInfoList);

            /* Flag that the device is not powered on.                  */
            CurrentPowerState       = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized             = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PXPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the PXP Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the PXP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _PXPM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Cancel the Link Monitor timer if necessary.           */
               if(LinkMonitoringStarted)
               {
                  TMR_StopTimer(LinkMonitoringTimerID);

                  LinkMonitoringStarted = FALSE;
                  LinkMonitoringTimerID = 0;
               }

               /* Inform the PXP Manager that the Stack has been closed.*/
               _PXPM_SetBluetoothStackID(0);

               /* Free the Connection Info lists.                       */
               FreeConnectionEntryList(&MonitorConnectionEntryList);
               FreeConnectionEntryList(&ReporterConnectionEntryList);
               break;
            case detRemoteDevicePropertiesChanged:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Properties Changed.\n"));

               /* Process the Remote Device Properties Changed Event.   */
               ProcessRemoteDevicePropertiesChangedEvent(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
               break;
            case detRemoteDeviceDeleted:
               DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Device Deleted.\n"));

               /* Simply attempt to process this as a Low Energy        */
               /* Disconnection.                                        */
               ProcessLowEnergyDisconnectionEvent(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, TRUE);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the PXP Manager of a specific Update Event.  The PXP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t PXPM_NotifyUpdate(PXPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utGATTClientEvent:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing GATT Client Event: %d\n", UpdateData->UpdateData.ClientEventData.Event_Data_Type));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Message.                             */
            ret_val = BTPM_QueueMailboxCallback(BTPMDistpatchCallback_GATT, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Monitor callback function with the Proximity*/
   /* Profile (PXP) Manager Service.  This Callback will be dispatched  */
   /* by the PXP Manager when various PXP Manager Monitor Events occur. */
   /* This function accepts the Callback Function and Callback Parameter*/
   /* (respectively) to call when a PXP Manager Monitor Event needs to  */
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          PXPM_Un_Register_Monitor_Event_Callback() function to    */
   /*          un-register the callback from this module.               */
   /* * NOTE * Only 1 Monitor Event Callback can be registered in the   */
   /*          system at a time.                                        */
int BTPSAPI PXPM_Register_Monitor_Event_Callback(PXPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   PXPM_Event_Callback_Info_t  EventCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* For now we will only allow 1 Monitor Event Callback to be*/
            /* registered at a time.  Whoever registers this will be the*/
            /* "Owner" of all Proximity Monitor connections.  In the    */
            /* future this could change and this check would have to be */
            /* removed.                                                 */
            if(MonitorEventCallbackInfoList == NULL)
            {
               /* Attempt to add an entry into the Event Callback Entry */
               /* list.                                                 */
               BTPS_MemInitialize(&EventCallbackEntry, 0, sizeof(PXPM_Event_Callback_Info_t));

               EventCallbackEntry.EventCallbackID   = GetNextMonitorEventCallbackID();
               EventCallbackEntry.ClientID          = MSG_GetServerAddressID();
               EventCallbackEntry.EventCallback     = CallbackFunction;
               EventCallbackEntry.CallbackParameter = CallbackParameter;

               if(AddEventCallbackInfoEntry(&MonitorEventCallbackInfoList, &EventCallbackEntry))
                  ret_val = EventCallbackEntry.EventCallbackID;
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_PROXIMITY_EVENT_ALREADY_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PXP Manager Monitor Event     */
   /* Callback (registered via a successful call to the                 */
   /* PXPM_Register_Monitor_Event_Callback() function).  This function  */
   /* accepts as input the PXP Manager Event Callback ID (return value  */
   /* from PXPM_Register_Monitor_Event_Callback() function).            */
void BTPSAPI PXPM_Un_Register_Monitor_Event_Callback(unsigned int PMonitorCallbackID)
{
   PXPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Check to see if the input parameters appear to be semi-valid.  */
      if(PMonitorCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Verify that the owner of this callback is the person who */
            /* is doing the un-registering.                             */
            if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, PMonitorCallbackID)) != NULL)
            {
               /* Verify that this is owned by the Server Process.      */
               if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
               {
                  /* Delete the callback back from the list.            */
                  if((EventCallbackPtr = DeleteEventCallbackInfoEntry(&MonitorEventCallbackInfoList, PMonitorCallbackID)) != NULL)
                  {
                     /* Free the memory because we are finished with it.*/
                     FreeEventCallbackInfoEntryMemory(EventCallbackPtr);
                  }
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the refresh time for checking the Path Loss (the time    */
   /* between checking the path loss for a given link).  This function  */
   /* accepts as it's parameter the MonitorCallbackID that was returned */
   /* from a successful call to PXPM_Register_Monitor_Event_Callback()  */
   /* and the Refresh Time (in milliseconds).  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * Decreasing the refresh rate will increase the power      */
   /*          consumption of the local Bluetooth device as it will     */
   /*          involve reading the RSSI at a faster rate.               */
int BTPSAPI PXPM_Set_Path_Loss_Refresh_Time(unsigned int MonitorCallbackID, unsigned int RefreshTime)
{
   int                         ret_val;
   PXPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: Monitor Callback ID = %u, Refresh Time = %u.\n", MonitorCallbackID, RefreshTime));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Verify that the the caller is the "Owner" of PXP Monitor    */
         /* Connections.                                                */
         if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, MonitorCallbackID)) != NULL)
         {
            /* Verify that this is owned by the Server Process.         */
            if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
            {
               /* Verify that the refresh time is valid.                */
               if((RefreshTime >= PXPM_CONFIGURATION_MINIMUM_REFRESH_TIME) && (RefreshTime <= PXPM_CONFIGURATION_MAXIMUM_REFRESH_TIME))
               {
                  /* Update the Refresh Timer.                          */
                  DefaultRefreshTime = RefreshTime;

                  /* If the Link Monitor timer is currently active we   */
                  /* need to change the time-out.                       */
                  if(LinkMonitoringStarted)
                     TMR_ChangeTimer(LinkMonitoringTimerID, DefaultRefreshTime);

                  /* Return success to the caller.                      */
                  ret_val = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Threshold for a specified PXP Client       */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert.  This   */
   /* function accepts as it's parameter the MonitorCallbackID that was */
   /* returned from a successful call to                                */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Client */
   /* Connection to set the path loss for, and the Path Loss Threshold  */
   /* to set.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The Path Loss Threshold should be specified in units of  */
   /*          dBm.                                                     */
int BTPSAPI PXPM_Set_Path_Loss_Threshold(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int PathLossThreshold)
{
   int                         ret_val;
   Connection_Entry_t         *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Verify that the the caller is the "Owner" of PXP Monitor    */
         /* Connections.                                                */
         if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, MonitorCallbackID)) != NULL)
         {
            /* Verify that this is owned by the Server Process.         */
            if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
            {
               /* Search for the Connection Entry for this device.      */
               if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, BD_ADDR)) != NULL)
               {
                  /* Save the new Path Loss Threshold.                  */
                  ConnectionEntryPtr->PathLossThreshold = PathLossThreshold;

                  /* Return success to the caller.                      */
                  ret_val                               = 0;
               }
               else
                  ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* querying the current Path Loss for a specified PXP Monitor        */
   /* Connection.  This function accepts as it's parameter the          */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss for, and a pointer to a buffer to */
   /* return the current Path Loss in (if successfull).  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The Path Loss Threshold will be specified in units of    */
   /*          dBm.                                                     */
int BTPSAPI PXPM_Query_Current_Path_Loss(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int *PathLossResult)
{
   int                         ret_val;
   Connection_Entry_t         *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Verify that the the caller is the "Owner" of PXP Monitor    */
         /* Connections.                                                */
         if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, MonitorCallbackID)) != NULL)
         {
            /* Verify that this is owned by the Server Process.         */
            if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
            {
               /* Search for the Connection Entry for this device.      */
               if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, BD_ADDR)) != NULL)
               {
                  /* Simply return the result of the internal utility   */
                  /* function.                                          */
                  ret_val = GetPathLoss(ConnectionEntryPtr, PathLossResult);
               }
               else
                  ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Alert Level for a specified PXP Client     */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Client */
   /* Connection to set the path loss alert level for, and the Path Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int BTPSAPI PXPM_Set_Path_Loss_Alert_Level(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel)
{
   int                         ret_val;
   Connection_Entry_t         *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Verify that the the caller is the "Owner" of PXP Monitor    */
         /* Connections.                                                */
         if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, MonitorCallbackID)) != NULL)
         {
            /* Verify that this is owned by the Server Process.         */
            if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
            {
               /* Search for the Connection Entry for this device.      */
               if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, BD_ADDR)) != NULL)
               {
                  /* Verify that the alert level is valid.              */
                  if((AlertLevel == alNoAlert) || (AlertLevel == alMildAlert) || (AlertLevel == alHighAlert))
                  {
                     /* Save the new Path Loss Alert Level.             */
                     ConnectionEntryPtr->PathLossAlertLevel = AlertLevel;

                     /* Return success to the caller.                   */
                     ret_val                                = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* changing the Link Loss Alert Level for a specified PXP Client     */
   /* Connection.  If the link to the proximity device is lost then an  */
   /* event will be generated to inform the caller to sound an alert at */
   /* the specified level.  This function accepts as it's parameter the */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Client */
   /* Connection to set the link loss alert level for, and the Link Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int BTPSAPI PXPM_Set_Link_Loss_Alert_Level(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel)
{
   int                         ret_val;
   Byte_t                      FormattedAlertLevel;
   Connection_Entry_t         *ConnectionEntryPtr;
   PXPM_Event_Callback_Info_t *EventCallbackPtr;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Serial Port Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Verify that the the caller is the "Owner" of PXP Monitor    */
         /* Connections.                                                */
         if((EventCallbackPtr = SearchEventCallbackInfoEntry(&MonitorEventCallbackInfoList, MonitorCallbackID)) != NULL)
         {
            /* Verify that this is owned by the Server Process.         */
            if(EventCallbackPtr->ClientID == MSG_GetServerAddressID())
            {
               /* Search for the Connection Entry for this device.      */
               if((ConnectionEntryPtr = SearchConnectionEntry(&MonitorConnectionEntryList, BD_ADDR)) != NULL)
               {
                  /* Verify that the alert level is valid.              */
                  if((AlertLevel == alNoAlert) || (AlertLevel == alMildAlert) || (AlertLevel == alHighAlert))
                  {
                     /* If the new Link Loss alert level does not match */
                     /* the current alert level inform the remote device*/
                     /* of the new value.                               */
                     if(ConnectionEntryPtr->LinkLossAlertLevel != AlertLevel)
                     {
                        /* Convert the Link Loss Alert Level to one that*/
                        /* can be used over the air.                    */
                        FormattedAlertLevel = ConvertAlertLevel(AlertLevel);

                        /* Now Attempt to set the LLS Alert Level (this */
                        /* MUST be supported by the remote device if it */
                        /* supports the Reporter Role so we aren't      */
                        /* worried about the return value).             */
                        _PXPM_Write_Value(ConnectionEntryPtr->ConnectionID, ConnectionEntryPtr->LLS_Alert_Handle, BYTE_SIZE, &FormattedAlertLevel);
                     }

                     /* Save the new Link Loss Alert Level.             */
                     ConnectionEntryPtr->LinkLossAlertLevel = AlertLevel;

                     /* Return success to the caller.                   */
                     ret_val                                = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_LE_PROXIMITY | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


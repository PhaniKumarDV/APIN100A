/*****< btpmhidm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHIDM - HID Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/18/11  D. Lange       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHIDM.h"            /* BTPM HID Manager Prototypes/Constants.    */
#include "HIDMAPI.h"             /* HID Manager Prototypes/Constants.         */
#include "HIDMMSG.h"             /* BTPM HID Manager Message Formats.         */
#include "HIDMGR.h"              /* HID Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagHID_Entry_Info_t
{
   unsigned int                 CallbackID;
   unsigned int                 ClientID;
   unsigned int                 ConnectionStatus;
   BD_ADDR_t                    ConnectionBD_ADDR;
   Event_t                      ConnectionEvent;
   unsigned long                Flags;
   HIDM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagHID_Entry_Info_t *NextHIDEntryInfoPtr;
} HID_Entry_Info_t;

   /* The following functions are used with the Flags member of the     */
   /* HID_Entry_Info_t structure to denote various state information.   */
#define HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY           0x40000000

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   HIDM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking incoming connections.                 */
typedef enum
{
   csIdle,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnecting,
   csConnected
} Connection_State_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagConnection_Entry_t
{
   BD_ADDR_t                      BD_ADDR;
   Connection_State_t             ConnectionState;
   unsigned long                  ConnectionFlags;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection
} DEVM_Status_Type_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds a pointer to the first element in the HID    */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static HID_Entry_Info_t *HIDEntryInfoList;

   /* Variable which holds a pointer to the first element of the HID    */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static HID_Entry_Info_t *HIDEntryInfoDataList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variables which control incoming connection requests              */
   /* (Authorization/Authentication/Encryption).                        */
static unsigned long IncomingConnectionFlags;

   /* Variable which holds a pointer to the first element of the        */
   /* Connection Information List (which holds all currently active     */
   /* connections).                                                     */
static Connection_Entry_t *ConnectionEntryList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);
static Boolean_t ConvertHIDResultType(HID_Result_Type_t HIDResultType, HIDM_Result_t *HIDMResult);
static Boolean_t ConvertHIDReportType(HID_Report_Type_Type_t HIDReportType, HIDM_Report_Type_t *HIDMReportType);

static HID_Entry_Info_t *AddHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, HID_Entry_Info_t *EntryToAdd);
static HID_Entry_Info_t *SearchHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID);
static HID_Entry_Info_t *DeleteHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeHIDEntryInfoEntryMemory(HID_Entry_Info_t *EntryToFree);
static void FreeHIDEntryInfoList(HID_Entry_Info_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static void DispatchHIDEvent(HIDM_Event_Data_t *HIDMEventData, BTPM_Message_t *Message);
static void DispatchHIDDataEvent(HIDM_Event_Data_t *HIDMEventData, BTPM_Message_t *Message);

static void ProcessConnectionResponseMessage(HIDM_Connection_Request_Response_Request_t *Message);
static void ProcessConnectHIDDeviceMessage(HIDM_Connect_HID_Device_Request_t *Message);
static void ProcessDisconnectHIDDeviceMessage(HIDM_Disconnect_HID_Device_Request_t *Message);
static void ProcessQueryConnectedHIDDevicesMessage(HIDM_Query_Connected_HID_Devices_Request_t *Message);
static void ProcessChangeIncomingConnectionFlagsMessage(HIDM_Change_Incoming_Connection_Flags_Request_t *Message);
static void ProcessSetKeyboardRepeatRateMessage(HIDM_Set_Keyboard_Repeat_Rate_Request_t *Message);
static void ProcessSendReportDataMessage(HIDM_Send_Report_Data_Request_t *Message);
static void ProcessSendGetReportRequest(HIDM_Send_Get_Report_Request_Request_t *Message);
static void ProcessSendSetReportRequest(HIDM_Send_Set_Report_Request_Request_t *Message);
static void ProcessSendGetProtocolRequest(HIDM_Send_Get_Protocol_Request_Request_t *Message);
static void ProcessSendSetProtocolRequest(HIDM_Send_Set_Protocol_Request_Request_t *Message);
static void ProcessSendGetIdleRequest(HIDM_Send_Get_Idle_Request_Request_t *Message);
static void ProcessSendSetIdleRequest(HIDM_Send_Set_Idle_Request_Request_t *Message);
static void ProcessRegisterHIDEventsMessage(HIDM_Register_HID_Events_Request_t *Message);
static void ProcessUnRegisterHIDEventsMessage(HIDM_Un_Register_HID_Events_Request_t *Message);
static void ProcessRegisterHIDDataEventsMessage(HIDM_Register_HID_Data_Events_Response_t *Message);
static void ProcessUnRegisterHIDDataEventsMessage(HIDM_Un_Register_HID_Data_Events_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessOpenRequestIndicationEvent(HID_Host_Open_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessOpenIndicationEvent(HID_Host_Open_Indication_Data_t *OpenIndicationData);
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HID_Host_Open_Confirmation_Data_t *OpenConfirmationData);
static void ProcessCloseIndicationEvent(HID_Host_Close_Indication_Data_t *CloseIndicationData);
static void ProcessBootKeyboardDataIndicationEvent(HID_Host_Boot_Keyboard_Data_t *BootKeyboardData);
static void ProcessBootKeyboardRepeatIndicationEvent(HID_Host_Boot_Keyboard_Repeat_Data_t *KeyboardRepeatData);
static void ProcessBootMouseDataIndicationEvent(HID_Host_Boot_Mouse_Data_t *BootMouseData);
static void ProcessDataIndicationEvent(HID_Host_Data_Indication_Data_t *DataIndicationData);
static void ProcessGetReportConfirmationEvent(HID_Host_Get_Report_Confirmation_Data_t *GetReportConfirmationData);
static void ProcessSetReportConfirmationEvent(HID_Host_Set_Report_Confirmation_Data_t *SetReportConfirmationData);
static void ProcessGetProtocolConfirmationEvent(HID_Host_Get_Protocol_Confirmation_Data_t *GetProtocolConfirmationData);
static void ProcessSetProtocolConfirmationEvent(HID_Host_Set_Protocol_Confirmation_Data_t *SetProtocolConfirmationData);
static void ProcessGetIdleConfirmationEvent(HID_Host_Get_Idle_Confirmation_Data_t *GetIdleConfirmationData);
static void ProcessSetIdleConfirmationEvent(HID_Host_Set_Idle_Confirmation_Data_t *SetIdleConfirmationData);

static void ProcessHIDHEvent(HIDM_HIDH_Event_Data_t *HIDHEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static void BTPSAPI BTPMDispatchCallback_HIDM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_HIDH(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HIDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HID Entry Information List.                              */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert a HID_Host Result Type to a HIDM Result Type.             */
static Boolean_t ConvertHIDResultType(HID_Result_Type_t HIDResultType, HIDM_Result_t *HIDMResult)
{
   Boolean_t ret_val = TRUE;

   if(HIDMResult)
   {
      switch(HIDResultType)
      {
         case rtSuccessful:
            *HIDMResult = hmrSuccessful;
            break;
         case rtNotReady:
            *HIDMResult = hmrNotReady;
            break;
         case rtErrInvalidReportID:
            *HIDMResult = hmrErrInvalidReportID;
            break;
         case rtErrUnsupportedRequest:
            *HIDMResult = hmrErrUnsupportedRequest;
            break;
         case rtErrInvalidParameter:
            *HIDMResult = hmrErrInvalidParameter;
            break;
         case rtErrUnknown:
            *HIDMResult = hmrErrUnknown;
            break;
         case rtErrFatal:
            *HIDMResult = hmrErrFatal;
            break;
         case rtData:
            *HIDMResult = hmrData;
            break;
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert a HID_Host Report Type to a HIDM Report Type.             */
static Boolean_t ConvertHIDReportType(HID_Report_Type_Type_t HIDReportType, HIDM_Report_Type_t *HIDMReportType)
{
   Boolean_t ret_val = TRUE;

   if(HIDMReportType)
   {
      switch(HIDReportType)
      {
         case rtInput:
            *HIDMReportType = hmrtInput;
            break;
         case rtOutput:
            *HIDMReportType = hmrtOutput;
            break;
         case rtFeature:
            *HIDMReportType = hmrtFeature;
            break;
         case rtOther:
         default:
            /* These cases are not used in HIDM.                        */
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

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
static HID_Entry_Info_t *AddHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, HID_Entry_Info_t *EntryToAdd)
{
   HID_Entry_Info_t *AddedEntry = NULL;
   HID_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (HID_Entry_Info_t *)BTPS_AllocateMemory(sizeof(HID_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                     = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextHIDEntryInfoPtr = NULL;

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
                     FreeHIDEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextHIDEntryInfoPtr)
                        tmpEntry = tmpEntry->NextHIDEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextHIDEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static HID_Entry_Info_t *SearchHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HID_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextHIDEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified HID Entry           */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HID Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeHIDEntryInfoEntryMemory().                   */
static HID_Entry_Info_t *DeleteHIDEntryInfoEntry(HID_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   HID_Entry_Info_t *FoundEntry = NULL;
   HID_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextHIDEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextHIDEntryInfoPtr = FoundEntry->NextHIDEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextHIDEntryInfoPtr;

         FoundEntry->NextHIDEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified HID Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeHIDEntryInfoEntryMemory(HID_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified HID Entry Information List.  Upon return */
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeHIDEntryInfoList(HID_Entry_Info_t **ListHead)
{
   HID_Entry_Info_t *EntryToFree;
   HID_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextHIDEntryInfoPtr;

         if(tmpEntry->ConnectionEvent)
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeHIDEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Connection Information member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Connection Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   Connection_Entry_t *EntryToFree;
   Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID event to every registered HID Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HID Manager Lock */
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Lock.                                            */
static void DispatchHIDEvent(HIDM_Event_Data_t *HIDMEventData, BTPM_Message_t *Message)
{
   unsigned int      Index;
   unsigned int      Index1;
   unsigned int      ServerID;
   unsigned int      NumberCallbacks;
   Callback_Info_t   CallbackInfoArray[16];
   Callback_Info_t  *CallbackInfoArrayPtr;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((HIDEntryInfoList) || (HIDEntryInfoDataList)) && (HIDMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      HIDEntryInfo    = HIDEntryInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(HIDEntryInfo)
      {
         if(((HIDEntryInfo->EventCallback) || (HIDEntryInfo->ClientID != ServerID)) && (HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
            NumberCallbacks++;

         HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
      }

      /* We need to add the HID Data Entry Information List as well.    */
      HIDEntryInfo = HIDEntryInfoDataList;
      while(HIDEntryInfo)
      {
         NumberCallbacks++;

         HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
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
            HIDEntryInfo    = HIDEntryInfoList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(HIDEntryInfo)
            {
               if(((HIDEntryInfo->EventCallback) || (HIDEntryInfo->ClientID != ServerID)) && (HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HIDEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HIDEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HIDEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
            }

            /* We need to add the HID Data Entry Information List as    */
            /* well.                                                    */
            HIDEntryInfo = HIDEntryInfoDataList;
            while(HIDEntryInfo)
            {
               CallbackInfoArrayPtr[NumberCallbacks].ClientID          = HIDEntryInfo->ClientID;
               CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = HIDEntryInfo->EventCallback;
               CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = HIDEntryInfo->CallbackParameter;

               NumberCallbacks++;

               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
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
               /*          for HID events and Data Events.              */
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(HIDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID Data event to the correct registered   */
   /* HID Data Event Callback.                                          */
   /* * NOTE * This function should be called with the HID Manager Lock */
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Lock.                                            */
static void DispatchHIDDataEvent(HIDM_Event_Data_t *HIDMEventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   HID_Entry_Info_t      *HIDEntryInfo;
   HIDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HIDMEventData) && (Message))
   {
      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if((HIDEntryInfo = HIDEntryInfoDataList) != NULL)
      {
         /* Format up the Data.                                         */
         if(HIDEntryInfo->ClientID != MSG_GetServerAddressID())
         {
            /* Dispatch a Message Callback.                             */

            /* Note the Client (destination) address.                   */
            Message->MessageHeader.AddressID                                             = HIDEntryInfo->ClientID;

            /* Note the HID Manager Data Event Callback ID.             */
            /* * NOTE * All Messages have this member in the same       */
            /*          location.                                       */
            ((HIDM_HID_Report_Data_Received_Message_t *)Message)->HIDDataEventsHandlerID = HIDEntryInfo->CallbackID;

            /* All that is left to do is to dispatch the Event.         */
            MSG_SendMessage(Message);
         }
         else
         {
            /* Dispatch Local Event Callback.                           */
            if(HIDEntryInfo->EventCallback)
            {
               /* Note the HID Manager Data Event Callback ID.          */
               /* * NOTE * All Events have this member in the same      */
               /*          location.                                    */
               HIDMEventData->EventData.BootKeyboardKeyPressEventData.HIDManagerDataCallbackID = HIDEntryInfo->CallbackID;

               /* Note the Callback Information.                        */
               EventCallback                                                                   = HIDEntryInfo->EventCallback;
               CallbackParameter                                                               = HIDEntryInfo->CallbackParameter;

               /* Release the Lock because we have already noted the    */
               /* callback information.                                 */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  (*EventCallback)(HIDMEventData, CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Connect        */
   /* Response Message and responds to the message accordingly.  This   */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessConnectionResponseMessage(HIDM_Connection_Request_Response_Request_t *Message)
{
   int                                          Result;
   Boolean_t                                    Authenticate;
   Boolean_t                                    Encrypt;
   Connection_Entry_t                          *ConnectionEntry;
   HIDM_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are already      */
         /* tracking a connection for the specified connection type.    */
         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
         {
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Message->Accept));

            /* If the caller has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Message->Accept)
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(ConnectionEntry->BD_ADDR, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               /* We need to map the Request flags from the Bluetopia PM*/
               /* HID Host request flags to the correct Bluetopia HID   */
               /* Host Request flags.                                   */
               if(Message->ConnectionFlags & HIDM_CONNECTION_REQUEST_CONNECTION_FLAGS_PARSE_BOOT)
                  ConnectionEntry->ConnectionFlags = HID_HOST_CONNECTION_FLAGS_PARSE_BOOT;
               else
                  ConnectionEntry->ConnectionFlags = 0;

               if(Message->ConnectionFlags & HIDM_CONNECTION_REQUEST_CONNECTION_FLAGS_REPORT_MODE)
                  ConnectionEntry->ConnectionFlags |= HID_HOST_CONNECTION_FLAGS_REPORT_MODE;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Authorization not required, and we are already in  */
                  /* the correct state.                                 */
                  Result = _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, TRUE, ConnectionEntry->ConnectionFlags);

                  if(Result)
                  {
                     _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, FALSE, 0);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
                  else
                  {
                     /* Update the current connection state.            */
                     ConnectionEntry->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!Result)
                  {
                     if(Encrypt)
                        ConnectionEntry->ConnectionState = csEncrypting;
                     else
                        ConnectionEntry->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, FALSE, 0);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
               }
            }
            else
            {
               /* Rejection - Simply respond to the request.            */
               Result = _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, FALSE, 0);

               /* Go ahead and delete the entry because we are finished */
               /* with tracking it.                                     */
               if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntry);
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Connect Remote */
   /* Device Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessConnectHIDDeviceMessage(HIDM_Connect_HID_Device_Request_t *Message)
{
   int                                 Result;
   BD_ADDR_t                           NULL_BD_ADDR;
   HID_Entry_Info_t                    HIDEntryInfo;
   HID_Entry_Info_t                   *HIDEntryInfoPtr;
   Connection_Entry_t                  ConnectionEntry;
   Connection_Entry_t                 *ConnectionEntryPtr;
   HIDM_Connect_HID_Device_Response_t  ResponseMessage;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are not already  */
         /* tracking a connection to the specified device.              */

         /* Next, verify that the input parameters appear to be         */
         /* semi-valid.                                                 */
         if(!COMPARE_BD_ADDR(Message->RemoteDeviceAddress, NULL_BD_ADDR))
         {
            if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) == NULL)
            {
               /* Entry is not present, go ahead and create a new entry.*/
               BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

               ConnectionEntry.BD_ADDR         = Message->RemoteDeviceAddress;
               ConnectionEntry.ConnectionState = csIdle;

               if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
               {
                  /* Attempt to add an entry into the HID Entry list.   */
                  BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

                  HIDEntryInfo.CallbackID        = GetNextCallbackID();
                  HIDEntryInfo.ClientID          = Message->MessageHeader.AddressID;
                  HIDEntryInfo.ConnectionBD_ADDR = Message->RemoteDeviceAddress;

                  if((HIDEntryInfoPtr = AddHIDEntryInfoEntry(&HIDEntryInfoList, &HIDEntryInfo)) != NULL)
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                     /* Next, attempt to open the remote device         */
                     if(Message->ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                        ConnectionEntryPtr->ConnectionState = csEncrypting;
                     else
                     {
                        if(Message->ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                           ConnectionEntryPtr->ConnectionState = csAuthenticating;
                        else
                           ConnectionEntryPtr->ConnectionState = csIdle;
                     }

                     /* Note that we need to map the Bluetopia PM HID   */
                     /* Host Connection Flags to the Bluetopia HID Host */
                     /* Connection Flags.                               */
                     if(Message->ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_PARSE_BOOT)
                        ConnectionEntryPtr->ConnectionFlags = HID_HOST_CONNECTION_FLAGS_PARSE_BOOT;
                     else
                        ConnectionEntryPtr->ConnectionFlags = 0;

                     if(Message->ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_REPORT_MODE)
                        ConnectionEntryPtr->ConnectionFlags |= HID_HOST_CONNECTION_FLAGS_REPORT_MODE;

                     Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csIdle)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                     if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                     {
                        /* Check to see if we need to actually issue the*/
                        /* Remote connection.                           */
                        if(Result < 0)
                        {
                           /* Set the state to connecting remote device.*/
                           ConnectionEntryPtr->ConnectionState = csConnecting;

                           if((Result = _HIDM_Connect_Remote_Device(Message->RemoteDeviceAddress, ConnectionEntryPtr->ConnectionFlags)) != 0)
                           {
                              if(Result == BTHID_HOST_ERROR_ALREADY_CONNECTED)
                                 Result = BTPM_ERROR_CODE_HID_DEVICE_ALREADY_CONNECTED;
                              else
                                 Result = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_HID_DEVICE;

                              /* Error opening device, go ahead and     */
                              /* delete the entry that was added.       */
                              if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfoPtr->CallbackID)) != NULL)
                                 FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
                           }
                        }
                     }
                     else
                     {
                        /* If we are not tracking this connection OR    */
                        /* there was an error, go ahead and delete the  */
                        /* entry that was added.                        */
                        if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfo.CallbackID)) != NULL)
                           FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
                     }
                  }
                  else
                     Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

                  /* If an error occurred, go ahead and delete the      */
                  /* Connection Information that was added.             */
                  if(Result)
                  {
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
            {
               if(ConnectionEntryPtr->ConnectionState == csConnected)
                  Result = BTPM_ERROR_CODE_HID_DEVICE_ALREADY_CONNECTED;
               else
                  Result = BTPM_ERROR_CODE_HID_DEVICE_CONNECTION_IN_PROGRESS;
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_CONNECT_HID_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Disconnect     */
   /* Device Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessDisconnectHIDDeviceMessage(HIDM_Disconnect_HID_Device_Request_t *Message)
{
   int                                    Result;
   Connection_Entry_t                    *ConnectionEntry;
   HID_Host_Close_Indication_Data_t       CloseIndicationData;
   HIDM_Disconnect_HID_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Go ahead and process the message request.                   */
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to disconnect HID Device\n"));

         if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, Message->RemoteDeviceAddress)) == NULL) || ((ConnectionEntry) && (ConnectionEntry->ConnectionState == csConnected)))
         {
            /* Nothing really to do other than to Disconnect the HID    */
            /* Device (if it is connected, a disconnect will be         */
            /* dispatched from the framework).                          */
            Result = _HIDM_Disconnect_Device(Message->RemoteDeviceAddress, Message->SendVirtualCableDisconnect);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DEVICE_CONNECTION_IN_PROGRESS;

         ResponseMessage.MessageHeader                = Message->MessageHeader;

         ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

         ResponseMessage.MessageHeader.MessageLength  = HIDM_DISCONNECT_HID_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

         ResponseMessage.Status                       = Result;

         MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);

         /* If the result was successful, we need to make sure we clean */
         /* up everything and dispatch the event to all registered      */
         /* clients.                                                    */
         if(!Result)
         {
            /* Fake a HID Close Event to dispatch to all registered     */
            /* clients that the device is no longer connected.          */
            CloseIndicationData.BD_ADDR = Message->RemoteDeviceAddress;

            ProcessCloseIndicationEvent(&CloseIndicationData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Query Connected*/
   /* HID Devices Message and responds to the message accordingly.  This*/
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessQueryConnectedHIDDevicesMessage(HIDM_Query_Connected_HID_Devices_Request_t *Message)
{
   unsigned int                                 NumberConnected;
   Connection_Entry_t                          *ConnectionEntry;
   HIDM_Query_Connected_HID_Devices_Response_t  ErrorResponseMessage;
   HIDM_Query_Connected_HID_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Initialize the Error Response Message.                         */
      ErrorResponseMessage.MessageHeader                = Message->MessageHeader;

      ErrorResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ErrorResponseMessage.MessageHeader.MessageLength  = HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

      ErrorResponseMessage.Status                       = 0;

      ErrorResponseMessage.NumberDevicesConnected       = 0;

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered up, so now let's determine how many       */
         /* devices are connected.                                      */
         NumberConnected = 0;
         ConnectionEntry = ConnectionEntryList;

         while(ConnectionEntry)
         {
            /* Note that we are only counting devices that are counting */
            /* devices that are either in the connected state or the    */
            /* connecting state (i.e. have been authorized OR passed    */
            /* authentication).                                         */
            if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
               NumberConnected++;

            ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
         }

         /* Now let's attempt to allocate memory to hold the entire     */
         /* list.                                                       */
         if((ResponseMessage = (HIDM_Query_Connected_HID_Devices_Response_t *)BTPS_AllocateMemory(HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(NumberConnected))) != NULL)
         {
            /* Memory allocated, now let's build the response message.  */
            /* * NOTE * Error Response has initialized all values to    */
            /*          known values (i.e. zero devices and success).   */
            BTPS_MemCopy(ResponseMessage, &ErrorResponseMessage, HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(0));

            ConnectionEntry = ConnectionEntryList;

            while((ConnectionEntry) && (ResponseMessage->NumberDevicesConnected < NumberConnected))
            {
               /* Note that we are only counting devices that are       */
               /* counting devices that either in the connected state or*/
               /* the connecting state (i.e. have been authorized OR    */
               /* passed authentication).                               */
               if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
                  ResponseMessage->DeviceConnectedList[ResponseMessage->NumberDevicesConnected++] = ConnectionEntry->BD_ADDR;

               ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
            }

            /* Now that we are finsished we need to update the length.  */
            ResponseMessage->MessageHeader.MessageLength  = HIDM_QUERY_CONNECTED_HID_DEVICES_RESPONSE_SIZE(ResponseMessage->NumberDevicesConnected) - BTPM_MESSAGE_HEADER_SIZE;

            /* Response Message built, go ahead and send it.            */
            MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

            /* Free the memory that was allocated because are finished  */
            /* with it.                                                 */
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
         ErrorResponseMessage.Status = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         MSG_SendMessage((BTPM_Message_t *)&ErrorResponseMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Change Incoming    */
   /* Connection Flags Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessChangeIncomingConnectionFlagsMessage(HIDM_Change_Incoming_Connection_Flags_Request_t *Message)
{
   int                                              Result;
   HIDM_Change_Incoming_Connection_Flags_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* All that remains is to note the specified Flags.            */
         IncomingConnectionFlags = Message->ConnectionFlags;

         /* Flag success.                                               */
         Result                  = 0;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Set Keyboard Repeat*/
   /* Rate Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSetKeyboardRepeatRateMessage(HIDM_Set_Keyboard_Repeat_Rate_Request_t *Message)
{
   int                                      Result;
   HIDM_Set_Keyboard_Repeat_Rate_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* All that remains is to attempt to set the Keyboard Repeat   */
         /* Rate.                                                       */
         Result = _HIDM_Set_Keyboard_Repeat_Rate(Message->RepeatDelay, Message->RepeatRate);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendReportDataMessage(HIDM_Send_Report_Data_Request_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* First, find the local handler.                              */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID Data.                                    */
            _HIDM_Send_Report_Data(Message->RemoteDeviceAddress, Message->ReportLength, Message->ReportData);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendGetReportRequest(HIDM_Send_Get_Report_Request_Request_t *Message)
{
   int                                     Result;
   HIDM_Send_Get_Report_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Confirm that the caller is allowed to call this API.        */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID request.                                 */
            Result = _HIDM_Send_Get_Report_Request(Message->RemoteDeviceAddress, Message->ReportType, Message->ReportID);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_SEND_GET_REPORT_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendSetReportRequest(HIDM_Send_Set_Report_Request_Request_t *Message)
{
   int                                     Result;
   HIDM_Send_Set_Report_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Confirm that the caller is allowed to call this API.        */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID request.                                 */
            Result = _HIDM_Send_Set_Report_Request(Message->RemoteDeviceAddress, Message->ReportType, Message->ReportDataLength, Message->ReportData);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_SEND_SET_REPORT_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendGetProtocolRequest(HIDM_Send_Get_Protocol_Request_Request_t *Message)
{
   int                                       Result;
   HIDM_Send_Get_Protocol_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Confirm that the caller is allowed to call this API.        */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID request.                                 */
            Result = _HIDM_Send_Get_Protocol_Request(Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_SEND_GET_PROTOCOL_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendSetProtocolRequest(HIDM_Send_Set_Protocol_Request_Request_t *Message)
{
   int                                       Result;
   HIDM_Send_Set_Protocol_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Confirm that the caller is allowed to call this API.        */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID request.                                 */
            Result = _HIDM_Send_Set_Protocol_Request(Message->RemoteDeviceAddress, Message->Protocol);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_SEND_SET_PROTOCOL_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendGetIdleRequest(HIDM_Send_Get_Idle_Request_Request_t *Message)
{
   int                                   Result;
   HIDM_Send_Get_Idle_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Confirm that the caller is allowed to call this API.        */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID request.                                 */
            Result = _HIDM_Send_Get_Idle_Request(Message->RemoteDeviceAddress);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_SEND_GET_IDLE_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified HID Send Report    */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendSetIdleRequest(HIDM_Send_Set_Idle_Request_Request_t *Message)
{
   int                                   Result;
   HIDM_Send_Set_Idle_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Confirm that the caller is allowed to call this API.        */
         if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID))
         {
            /* Nothing to do here other than to call the actual function*/
            /* to send the HID request.                                 */
            Result = _HIDM_Send_Set_Idle_Request(Message->RemoteDeviceAddress, Message->IdleRate);
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_SEND_SET_IDLE_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register HID Events*/
   /* Message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the Message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the Message   */
   /* before calling this function.                                     */
static void ProcessRegisterHIDEventsMessage(HIDM_Register_HID_Events_Request_t *Message)
{
   int                                 Result;
   HID_Entry_Info_t                    HIDEntryInfo;
   HIDM_Register_HID_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Attempt to add an entry into the HID Entry list.               */
      BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

      HIDEntryInfo.CallbackID         = GetNextCallbackID();
      HIDEntryInfo.ClientID           = Message->MessageHeader.AddressID;
      HIDEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

      if(AddHIDEntryInfoEntry(&HIDEntryInfoList, &HIDEntryInfo))
         Result = HIDEntryInfo.CallbackID;
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_REGISTER_HID_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.HIDEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status             = 0;
      }
      else
      {
         ResponseMessage.HIDEventsHandlerID = 0;

         ResponseMessage.Status             = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register HID    */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterHIDEventsMessage(HIDM_Un_Register_HID_Events_Request_t *Message)
{
   int                                     Result;
   HID_Entry_Info_t                       *HIDEntryInfo;
   HIDM_Un_Register_HID_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoList, Message->HIDEventsHandlerID)) != NULL) && (HIDEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, Message->HIDEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeHIDEntryInfoEntryMemory(HIDEntryInfo);

            /* Flag success.                                            */
            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_HID_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_HID_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_UN_REGISTER_HID_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register HID Data  */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessRegisterHIDDataEventsMessage(HIDM_Register_HID_Data_Events_Response_t *Message)
{
   int                                      Result;
   HID_Entry_Info_t                         HIDEntryInfo;
   HIDM_Register_HID_Data_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Go ahead and initialize the Response.                          */
      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      /* Before proceding any further, make sure that there is not      */
      /* already a Data Event Handler registered.                       */
      if(!HIDEntryInfoDataList)
      {
         /* Attempt to add an entry into the HID Entry list.            */
         BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

         HIDEntryInfo.CallbackID         = GetNextCallbackID();
         HIDEntryInfo.ClientID           = Message->MessageHeader.AddressID;
         HIDEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

         if(AddHIDEntryInfoEntry(&HIDEntryInfoDataList, &HIDEntryInfo))
            Result = HIDEntryInfo.CallbackID;
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_HID_DATA_EVENTS_ALREADY_REGISTERED;

      if(Result > 0)
      {
         ResponseMessage.HIDDataEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status                 = 0;
      }
      else
      {
         ResponseMessage.HIDDataEventsHandlerID = 0;

         ResponseMessage.Status                 = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-register HID    */
   /* Data Events Message and responds to the message accordingly.  This*/
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterHIDDataEventsMessage(HIDM_Un_Register_HID_Data_Events_Request_t *Message)
{
   int                                          Result;
   HID_Entry_Info_t                            *HIDEntryInfo;
   HIDM_Un_Register_HID_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((HIDEntryInfo = SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID)) != NULL) && (HIDEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoDataList, Message->HIDDataEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeHIDEntryInfoEntryMemory(HIDEntryInfo);

            /* Flag success.                                            */
            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = HIDM_UN_REGISTER_HID_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the HID Manager      */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the HID */
               /* Connect Response Request.                             */
               ProcessConnectionResponseMessage((HIDM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_CONNECT_HID_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect HID Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_CONNECT_HID_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connect HID Device Request.                           */
               ProcessConnectHIDDeviceMessage((HIDM_Connect_HID_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_DISCONNECT_HID_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect HID Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_DISCONNECT_HID_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnect HID Device Request.                        */
               ProcessDisconnectHIDDeviceMessage((HIDM_Disconnect_HID_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HID_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected HID Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_QUERY_CONNECTED_HID_DEVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Connected HID Devices Request.                  */
               ProcessQueryConnectedHIDDevicesMessage((HIDM_Query_Connected_HID_Devices_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Incoming Connection Flags Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnect HID Device Request.                        */
               ProcessChangeIncomingConnectionFlagsMessage((HIDM_Change_Incoming_Connection_Flags_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SET_KEYBOARD_REPEAT_RATE:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Keyboard Repeat Rate Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SET_KEYBOARD_REPEAT_RATE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Set */
               /* Keyboard Repeat Rate Request.                         */
               ProcessSetKeyboardRepeatRateMessage((HIDM_Set_Keyboard_Repeat_Rate_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_REPORT_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Report Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_REPORT_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_REPORT_DATA_REQUEST_SIZE(((HIDM_Send_Report_Data_Request_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Report Data Request.                                  */
               ProcessSendReportDataMessage((HIDM_Send_Report_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_GET_REPORT_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Get Report Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_GET_REPORT_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Get Report Request.                                   */
               ProcessSendGetReportRequest((HIDM_Send_Get_Report_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_SET_REPORT_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Set Report Request Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_SET_REPORT_REQUEST_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_SET_REPORT_REQUEST_REQUEST_SIZE(((HIDM_Send_Set_Report_Request_Request_t *)Message)->ReportDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Set Report Request.                                   */
               ProcessSendSetReportRequest((HIDM_Send_Set_Report_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Get Protocol Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_GET_PROTOCOL_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Get Protocol Request.                                 */
               ProcessSendGetProtocolRequest((HIDM_Send_Get_Protocol_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Set Protocol Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_SET_PROTOCOL_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Set Protocol Request.                                 */
               ProcessSendSetProtocolRequest((HIDM_Send_Set_Protocol_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_GET_IDLE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Get Idle Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_GET_IDLE_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Get Idle Request.                                     */
               ProcessSendGetIdleRequest((HIDM_Send_Get_Idle_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_SEND_SET_IDLE_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Set Idle Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_SEND_SET_IDLE_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Set Idle Request.                                     */
               ProcessSendSetIdleRequest((HIDM_Send_Set_Idle_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_REGISTER_HID_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Register HID Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_REGISTER_HID_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register HID Events Request.                          */
               ProcessRegisterHIDEventsMessage((HIDM_Register_HID_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register HID Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_UN_REGISTER_HID_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register HID Events Request.                       */
               ProcessUnRegisterHIDEventsMessage((HIDM_Un_Register_HID_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_REGISTER_HID_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Register HID Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register HID Data Events Request.                     */
               ProcessRegisterHIDDataEventsMessage((HIDM_Register_HID_Data_Events_Response_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HIDM_MESSAGE_FUNCTION_UN_REGISTER_HID_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register HID Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HIDM_UN_REGISTER_HID_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register HID Data Events Request.                  */
               ProcessUnRegisterHIDDataEventsMessage((HIDM_Un_Register_HID_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Boolean_t          LoopCount;
   HID_Entry_Info_t  *HIDEntryInfo;
   HID_Entry_Info_t **_HIDEntryInfoList;
   HID_Entry_Info_t  *tmpHIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      HIDEntryInfo      = HIDEntryInfoList;
      _HIDEntryInfoList = &HIDEntryInfoList;

      /* We need to loop through both lists as there could be client    */
      /* registrations in any of the lists.                             */
      LoopCount = 2;
      while(LoopCount)
      {
         while(HIDEntryInfo)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(HIDEntryInfo->ClientID == ClientID)
            {
               /* Note the next HID Entry in the list (we are about to  */
               /* delete the current entry).                            */
               tmpHIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;

               /* Go ahead and delete the HID Information Entry and     */
               /* clean up the resources.                               */
               if((HIDEntryInfo = DeleteHIDEntryInfoEntry(_HIDEntryInfoList, HIDEntryInfo->CallbackID)) != NULL)
               {
                  /* Close any events that were allocated.              */
                  if(HIDEntryInfo->ConnectionEvent)
                     BTPS_CloseEvent(HIDEntryInfo->ConnectionEvent);

                  /* All finished with the memory so free the entry.    */
                  FreeHIDEntryInfoEntryMemory(HIDEntryInfo);
               }

               /* Go ahead and set the next HID Information Entry (past */
               /* the one we just deleted).                             */
               HIDEntryInfo = tmpHIDEntryInfo;
            }
            else
               HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
         }

         /* Decrement the loop count so that we can make another pass   */
         /* through the loop.                                           */
         LoopCount--;

         /* We have processed the HID Information List, now process the */
         /* HID Information Data List.                                  */
         HIDEntryInfo      = HIDEntryInfoDataList;
         _HIDEntryInfoList = &HIDEntryInfoDataList;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Open Request Indication Event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the HID Manager Information held.                 */
static void ProcessOpenRequestIndicationEvent(HID_Host_Open_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   HIDM_Event_Data_t                  HIDMEventData;
   Connection_Entry_t                 ConnectionEntry;
   Connection_Entry_t                *ConnectionEntryPtr;
   HIDM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenRequestIndicationData)
   {
      /* First, let's see if we actually need to do anything, other than*/
      /* simply accept the connection.                                  */
      if(!IncomingConnectionFlags)
      {
         /* Simply Accept the connection.                               */
         /* * NOTE * that since we did not call back we do not know what*/
         /*          to use for the Connection Flags.  We will simply   */
         /*          use BOOT Mode Parsing processing for the device.   */
         _HIDM_Connection_Request_Response(OpenRequestIndicationData->BD_ADDR, TRUE, HID_HOST_CONNECTION_FLAGS_PARSE_BOOT);
      }
      else
      {
         /* Before proceding any further, let's make sure that there    */
         /* doesn't already exist an entry for this device.             */
         if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenRequestIndicationData->BD_ADDR)) == NULL)
         {
            /* Entry does not exist, go ahead and format a new entry.   */
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

            ConnectionEntry.BD_ADDR                = OpenRequestIndicationData->BD_ADDR;
            ConnectionEntry.ConnectionState        = csAuthorizing;
            ConnectionEntry.NextConnectionEntryPtr = NULL;

            ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
         }

         /* Check to see if we are tracking this connection.            */
         if(ConnectionEntryPtr)
         {
            if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
            {
               /* Authorization (at least) required, go ahead and       */
               /* dispatch the request.                                 */
               ConnectionEntryPtr->ConnectionState = csAuthorizing;

               /* Next, format up the Event to dispatch.                */
               HIDMEventData.EventType                                                 = hetHIDDeviceConnectionRequest;
               HIDMEventData.EventLength                                               = HIDM_HID_DEVICE_CONNECTION_REQUEST_EVENT_DATA_SIZE;

               HIDMEventData.EventData.DeviceConnectionRequestData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;

               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = 0;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
               Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
               Message.MessageHeader.MessageLength   = (HIDM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;

               /* Finally dispatch the formatted Event and Message.     */
               DispatchHIDEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
            }
            else
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               /* Since Authorization wasn't specified, we need to      */
               /* default the special HID Host processing flags.        */
               if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REPORT_MODE)
                 ConnectionEntryPtr->ConnectionFlags |= HID_HOST_CONNECTION_FLAGS_REPORT_MODE;

               if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_PARSE_BOOT)
                 ConnectionEntryPtr->ConnectionFlags |= HID_HOST_CONNECTION_FLAGS_PARSE_BOOT;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(ConnectionEntryPtr->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(ConnectionEntryPtr->BD_ADDR, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  Result = _HIDM_Connection_Request_Response(ConnectionEntryPtr->BD_ADDR, TRUE, ConnectionEntryPtr->ConnectionFlags);

                  if(Result)
                  {
                     _HIDM_Connection_Request_Response(ConnectionEntryPtr->BD_ADDR, FALSE, 0);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
                  else
                  {
                     /* Update the current connection state.            */
                     ConnectionEntryPtr->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!Result)
                  {
                     if(Encrypt)
                        ConnectionEntryPtr->ConnectionState = csEncrypting;
                     else
                        ConnectionEntryPtr->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     _HIDM_Connection_Request_Response(ConnectionEntryPtr->BD_ADDR, FALSE, 0);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                  }
               }
            }
         }
         else
         {
            /* Unable to add entry, go ahead and reject the request.    */
            _HIDM_Connection_Request_Response(OpenRequestIndicationData->BD_ADDR, FALSE, 0);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Open Indication Event that has been received with the specified   */
   /* information.  This function should be called with the Lock        */
   /* protecting the HID Manager Information held.                      */
static void ProcessOpenIndicationEvent(HID_Host_Open_Indication_Data_t *OpenIndicationData)
{
   HIDM_Event_Data_t                       HIDMEventData;
   Connection_Entry_t                      ConnectionEntry;
   Connection_Entry_t                     *ConnectionEntryPtr;
   HIDM_Connection_Request_Message_t       Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenIndicationData)
   {
      if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenIndicationData->BD_ADDR)) == NULL)
      {
         /* Entry does not exist, go ahead and format a new entry.      */
         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

         ConnectionEntry.BD_ADDR                = OpenIndicationData->BD_ADDR;
         ConnectionEntry.ConnectionState        = csConnected;
         ConnectionEntry.NextConnectionEntryPtr = NULL;

         ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
      }
      else
         ConnectionEntryPtr->ConnectionState = csConnected;

      if(ConnectionEntryPtr)
      {
         /* Next, format up the Event to dispatch.                      */
         HIDMEventData.EventType                                              = hetHIDDeviceConnected;
         HIDMEventData.EventLength                                            = HIDM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE;

         HIDMEventData.EventData.DeviceConnectedEventData.RemoteDeviceAddress = OpenIndicationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED;
         Message.MessageHeader.MessageLength   = (HIDM_HID_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = OpenIndicationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHIDEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
      else
      {
         /* Error, go ahead and disconnect the device.                  */
         _HIDM_Disconnect_Device(OpenIndicationData->BD_ADDR, TRUE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Open Confirmation Event that has been received with the specified */
   /* information.  This function should be called with the Lock        */
   /* protecting the HID Manager Information held.                      */
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HID_Host_Open_Confirmation_Data_t *OpenConfirmationData)
{
   void                                        *CallbackParameter;
   unsigned int                                 ClientID;
   HIDM_Event_Data_t                            HIDMEventData;
   HID_Entry_Info_t                            *HIDEntryInfo;
   HIDM_Event_Callback_t                        EventCallback;
   Connection_Entry_t                           ConnectionEntry;
   Connection_Entry_t                          *ConnectionEntryPtr;
   HIDM_HID_Device_Connected_Message_t          Message;
   HIDM_HID_Device_Connection_Status_Message_t  ConnectionStatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenConfirmationData)
   {
      /* First, flag the connected state.                               */
      if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenConfirmationData->BD_ADDR)) == NULL)
      {
         /* Entry does not exist, go ahead and format a new entry.      */
         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

         ConnectionEntry.BD_ADDR                = OpenConfirmationData->BD_ADDR;
         ConnectionEntry.ConnectionState        = OpenConfirmationData->OpenStatus?csIdle:csConnected;
         ConnectionEntry.NextConnectionEntryPtr = NULL;

         /* Do not add the entry if the confirmation was a failure.     */
         if(!OpenConfirmationData->OpenStatus)
            ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
      }
      else
      {
         if((OpenConfirmationData->OpenStatus) && (ConnectionEntryPtr->ConnectionState == csConnecting))
         {
            if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, OpenConfirmationData->BD_ADDR)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntryPtr);
         }
         else
            ConnectionEntryPtr->ConnectionState = csConnected;
      }

      /* Dispatch any registered Connection Status Message/Event.       */
      HIDEntryInfo = HIDEntryInfoList;
      while(HIDEntryInfo)
      {
         if((!(HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(OpenConfirmationData->BD_ADDR, HIDEntryInfo->ConnectionBD_ADDR)))
         {
            /* Connection status registered, now see if we need to issue*/
            /* a Callack or an event.                                   */

            /* Determine if we need to dispatch the event locally or    */
            /* remotely.                                                */
            if(HIDEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Callback.                                             */
               BTPS_MemInitialize(&HIDMEventData, 0, sizeof(HIDM_Event_Data_t));

               HIDMEventData.EventType                                                     = hetHIDDeviceConnectionStatus;
               HIDMEventData.EventLength                                                   = HIDM_HID_DEVICE_CONNECTION_STATUS_EVENT_DATA_SIZE;

               HIDMEventData.EventData.DeviceConnectionStatusEventData.RemoteDeviceAddress = OpenConfirmationData->BD_ADDR;

               /* Map the Open Confirmation Error to the correct HID    */
               /* Manager Error Status.                                 */
               switch(OpenConfirmationData->OpenStatus)
               {
                  case HID_OPEN_PORT_STATUS_SUCCESS:
                     HIDMEventData.EventData.DeviceConnectionStatusEventData.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_SUCCESS;
                     break;
                  case HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
                     HIDMEventData.EventData.DeviceConnectionStatusEventData.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                     break;
                  case HID_OPEN_PORT_STATUS_CONNECTION_REFUSED:
                     HIDMEventData.EventData.DeviceConnectionStatusEventData.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                     break;
                  default:
                     HIDMEventData.EventData.DeviceConnectionStatusEventData.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                     break;
               }

               /* If this was a synchronous event we need to set the    */
               /* status and the event.                                 */
               if(HIDEntryInfo->ConnectionEvent)
               {
                  /* Synchronous event, go ahead and set the correct    */
                  /* status, then set the event.                        */
                  HIDEntryInfo->ConnectionStatus = HIDMEventData.EventData.DeviceConnectionStatusEventData.ConnectionStatus;

                  BTPS_SetEvent(HIDEntryInfo->ConnectionEvent);
               }
               else
               {
                  /* Note the Callback information.                     */
                  EventCallback     = HIDEntryInfo->EventCallback;
                  CallbackParameter = HIDEntryInfo->CallbackParameter;

                  /* Go ahead and delete the entry (since we are        */
                  /* dispatching the callback).                         */
                  if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfo->CallbackID)) != NULL)
                     FreeHIDEntryInfoEntryMemory(HIDEntryInfo);

                  /* Release the Lock so we can make the callback.      */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&HIDMEventData, CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  /* Re-acquire the Lock.                               */
                  DEVM_AcquireLock();
               }
            }
            else
            {
               /* Remote Event.                                         */

               /* Note the Client ID.                                   */
               ClientID = HIDEntryInfo->ClientID;

               /* Go ahead and delete the entry (since we are           */
               /* dispatching the event).                               */
               if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfo->CallbackID)) != NULL)
                  FreeHIDEntryInfoEntryMemory(HIDEntryInfo);

               BTPS_MemInitialize(&ConnectionStatusMessage, 0, sizeof(ConnectionStatusMessage));

               ConnectionStatusMessage.MessageHeader.AddressID       = ClientID;
               ConnectionStatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               ConnectionStatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
               ConnectionStatusMessage.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTION_STATUS;
               ConnectionStatusMessage.MessageHeader.MessageLength   = (HIDM_HID_DEVICE_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               ConnectionStatusMessage.RemoteDeviceAddress           = OpenConfirmationData->BD_ADDR;

               /* Map the Open Confirmation Error to the correct HID    */
               /* Manager Error Status.                                 */
               switch(OpenConfirmationData->OpenStatus)
               {
                  case HID_OPEN_PORT_STATUS_SUCCESS:
                     ConnectionStatusMessage.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_SUCCESS;
                     break;
                  case HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
                     ConnectionStatusMessage.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_TIMEOUT;
                     break;
                  case HID_OPEN_PORT_STATUS_CONNECTION_REFUSED:
                     ConnectionStatusMessage.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_REFUSED;
                     break;
                  default:
                     ConnectionStatusMessage.ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_UNKNOWN;
                     break;
               }

               /* Finally dispatch the Message.                         */
               MSG_SendMessage((BTPM_Message_t *)&ConnectionStatusMessage);
            }

            /* Break out of the loop.                                   */
            HIDEntryInfo = NULL;
         }
         else
            HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
      }

      /* Next, format up the Event to dispatch - ONLY if we need to     */
      /* dispatch a Connected Event.                                    */
      if((!OpenConfirmationData->OpenStatus) && (DispatchOpen))
      {
         HIDMEventData.EventType                                              = hetHIDDeviceConnected;
         HIDMEventData.EventLength                                            = HIDM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE;

         HIDMEventData.EventData.DeviceConnectedEventData.RemoteDeviceAddress = OpenConfirmationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_HID_DEVICE_CONNECTED;
         Message.MessageHeader.MessageLength   = (HIDM_HID_DEVICE_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = OpenConfirmationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHIDEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Close Indication Event that has been received with the specified  */
   /* information.  This function should be called with the Lock        */
   /* protecting the HID Manager Information held.                      */
static void ProcessCloseIndicationEvent(HID_Host_Close_Indication_Data_t *CloseIndicationData)
{
   HIDM_Event_Data_t                       HIDMEventData;
   Connection_Entry_t                     *ConnectionEntry;
   HIDM_HID_Device_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CloseIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, CloseIndicationData->BD_ADDR)) != NULL)
         FreeConnectionEntryMemory(ConnectionEntry);

      /* Next, format up the Event to dispatch.                         */
      HIDMEventData.EventType                                                 = hetHIDDeviceDisconnected;
      HIDMEventData.EventLength                                               = HIDM_HID_DEVICE_DISCONNECTED_EVENT_DATA_SIZE;

      HIDMEventData.EventData.DeviceDisconnectedEventData.RemoteDeviceAddress = CloseIndicationData->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_HID_DEVICE_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (HIDM_HID_DEVICE_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = CloseIndicationData->BD_ADDR;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchHIDEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Boot Keyboard Data Indication Event that has been received with   */
   /* the specified information.  This function should be called with   */
   /* the Lock protecting the HID Manager Information held.             */
static void ProcessBootKeyboardDataIndicationEvent(HID_Host_Boot_Keyboard_Data_t *BootKeyboardData)
{
   HIDM_Event_Data_t                                HIDMEventData;
   HIDM_HID_Boot_Keyboard_Key_Press_Event_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(BootKeyboardData)
   {
      /* Format the message.                                            */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_PRESS_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_BOOT_KEYBOARD_KEY_PRESS_EVENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = BootKeyboardData->BD_ADDR;
      Message.KeyDown                       = BootKeyboardData->KeyDown;
      Message.KeyModifiers                  = BootKeyboardData->KeyModifiers;
      Message.Key                           = BootKeyboardData->Key;

      /* Build the Event Data.                                          */
      HIDMEventData.EventType                                                        = hetHIDBootKeyboardKeyPress;
      HIDMEventData.EventLength                                                      = HIDM_HID_BOOT_KEYBOARD_KEY_PRESS_EVENT_DATA_SIZE;

      HIDMEventData.EventData.BootKeyboardKeyPressEventData.HIDManagerDataCallbackID = 0;
      HIDMEventData.EventData.BootKeyboardKeyPressEventData.RemoteDeviceAddress      = BootKeyboardData->BD_ADDR;
      HIDMEventData.EventData.BootKeyboardKeyPressEventData.KeyDown                  = BootKeyboardData->KeyDown;
      HIDMEventData.EventData.BootKeyboardKeyPressEventData.KeyModifiers             = BootKeyboardData->KeyModifiers;
      HIDMEventData.EventData.BootKeyboardKeyPressEventData.Key                      = BootKeyboardData->Key;

      /* Now Dispatch the Events.                                       */
      DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Keyboard Repeat Indication Event that has been received with the  */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the HID Manager Information held.                 */
static void ProcessBootKeyboardRepeatIndicationEvent(HID_Host_Boot_Keyboard_Repeat_Data_t *KeyboardRepeatData)
{
   HIDM_Event_Data_t                                 HIDMEventData;
   HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(KeyboardRepeatData)
   {
      /* Format the message.                                            */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_BOOT_KEYBOARD_KEY_REPEAT_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_BOOT_KEYBOARD_KEY_REPEAT_EVENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = KeyboardRepeatData->BD_ADDR;
      Message.KeyModifiers                  = KeyboardRepeatData->KeyModifiers;
      Message.Key                           = KeyboardRepeatData->Key;

      /* Build the Event Data.                                          */
      HIDMEventData.EventType                                                         = hetHIDBootKeyboardKeyRepeat;
      HIDMEventData.EventLength                                                       = HIDM_HID_BOOT_KEYBOARD_KEY_REPEAT_EVENT_DATA_SIZE;

      HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.HIDManagerDataCallbackID = 0;
      HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.RemoteDeviceAddress      = KeyboardRepeatData->BD_ADDR;
      HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.KeyModifiers             = KeyboardRepeatData->KeyModifiers;
      HIDMEventData.EventData.BootKeyboardKeyRepeatEventData.Key                      = KeyboardRepeatData->Key;

      /* Now Dispatch the Events.                                       */
      DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Boot Mouse Data Indication Event that has been received with the  */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the HID Manager Information held.                 */
static void ProcessBootMouseDataIndicationEvent(HID_Host_Boot_Mouse_Data_t *BootMouseData)
{
   HIDM_Event_Data_t                         HIDMEventData;
   HIDM_HID_Boot_Mouse_Mouse_Event_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(BootMouseData)
   {
      /* Format the message.                                            */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_BOOT_MOUSE_MOUSE_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_BOOT_MOUSE_MOUSE_EVENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = BootMouseData->BD_ADDR;
      Message.CX                            = BootMouseData->CX;
      Message.CY                            = BootMouseData->CY;
      Message.ButtonState                   = BootMouseData->ButtonState;
      Message.CZ                            = BootMouseData->CZ;

      /* Build the Event Data.                                          */
      HIDMEventData.EventType                                                  = hetHIDBootMouseEvent;
      HIDMEventData.EventLength                                                = HIDM_HID_BOOT_MOUSE_EVENT_EVENT_DATA_SIZE;

      HIDMEventData.EventData.BootMouseEventEventData.HIDManagerDataCallbackID = 0;
      HIDMEventData.EventData.BootMouseEventEventData.RemoteDeviceAddress      = BootMouseData->BD_ADDR;
      HIDMEventData.EventData.BootMouseEventEventData.CX                       = BootMouseData->CX;
      HIDMEventData.EventData.BootMouseEventEventData.CY                       = BootMouseData->CY;
      HIDMEventData.EventData.BootMouseEventEventData.ButtonState              = BootMouseData->ButtonState;
      HIDMEventData.EventData.BootMouseEventEventData.CZ                       = BootMouseData->CZ;

      /* Now Dispatch the Events.                                       */
      DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Data Indication Event that has been received with the specified   */
   /* information.  This function should be called with the Lock        */
   /* protecting the HID Manager Information held.                      */
static void ProcessDataIndicationEvent(HID_Host_Data_Indication_Data_t *DataIndicationData)
{
   HIDM_Event_Data_t                        HIDMEventData;
   HIDM_HID_Report_Data_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(DataIndicationData)
   {
      /* First, allocate enough memory to hold the HID Data Message.    */
      if((Message = (HIDM_HID_Report_Data_Received_Message_t *)BTPS_AllocateMemory(HIDM_HID_REPORT_DATA_RECEIVED_MESSAGE_SIZE(DataIndicationData->ReportLength))) != NULL)
      {
         /* Memory allocated, format the message.                       */
         BTPS_MemInitialize(Message, 0, HIDM_HID_REPORT_DATA_RECEIVED_MESSAGE_SIZE(0));

         Message->MessageHeader.AddressID        = 0;
         Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_HID_MANAGER;
         Message->MessageHeader.MessageFunction  = HIDM_MESSAGE_FUNCTION_HID_REPORT_DATA_RECEIVED;
         Message->MessageHeader.MessageLength    = (HIDM_HID_REPORT_DATA_RECEIVED_MESSAGE_SIZE(DataIndicationData->ReportLength) - BTPM_MESSAGE_HEADER_SIZE);

         Message->HIDDataEventsHandlerID         = 0;
         Message->RemoteDeviceAddress            = DataIndicationData->BD_ADDR;
         Message->ReportLength                   = DataIndicationData->ReportLength;

         if(Message->ReportLength)
            BTPS_MemCopy(Message->ReportData, DataIndicationData->ReportDataPayload, Message->ReportLength);

         /* Build the Event Data.                                       */
         HIDMEventData.EventType                                                      = hetHIDReportDataReceived;
         HIDMEventData.EventLength                                                    = HIDM_HID_REPORT_DATA_RECEIVED_EVENT_DATA_SIZE;

         HIDMEventData.EventData.ReportDataReceivedEventData.HIDManagerDataCallbackID = 0;
         HIDMEventData.EventData.ReportDataReceivedEventData.RemoteDeviceAddress      = DataIndicationData->BD_ADDR;
         HIDMEventData.EventData.ReportDataReceivedEventData.ReportLength             = DataIndicationData->ReportLength;
         HIDMEventData.EventData.ReportDataReceivedEventData.ReportData               = DataIndicationData->ReportDataPayload;

         /* Now Dispatch the Events.                                    */
         DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)Message);

         /* Finished with the Message, so go ahead and delete the       */
         /* memory.                                                     */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a HID Host   */
   /* Get Report Confirmation Event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the HID Manager Information held.                 */
static void ProcessGetReportConfirmationEvent(HID_Host_Get_Report_Confirmation_Data_t *GetReportConfirmationData)
{
   HIDM_Event_Data_t                       HIDMEventData;
   HIDM_HID_Get_Report_Response_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GetReportConfirmationData)
   {
      /* First, allocate enough memory to hold the HID Data Message.    */
      if((Message = (HIDM_HID_Get_Report_Response_Message_t *)BTPS_AllocateMemory(HIDM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(GetReportConfirmationData->ReportLength))) != NULL)
      {
         /* Memory allocated, format the message.                       */
         BTPS_MemInitialize(Message, 0, HIDM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(0));

         Message->MessageHeader.AddressID       = 0;
         Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
         Message->MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_GET_REPORT_RESPONSE_EVENT;
         Message->MessageHeader.MessageLength   = (HIDM_HID_GET_REPORT_RESPONSE_MESSAGE_SIZE(GetReportConfirmationData->ReportLength) - BTPM_MESSAGE_HEADER_SIZE);

         Message->HIDDataEventsHandlerID        = 0;
         Message->RemoteDeviceAddress           = GetReportConfirmationData->BD_ADDR;

         if(ConvertHIDResultType(GetReportConfirmationData->Status, &(Message->Status)))
         {
            if(ConvertHIDReportType(GetReportConfirmationData->ReportType, &(Message->ReportType)))
            {
               Message->ReportLength            = GetReportConfirmationData->ReportLength;

               if(Message->ReportLength)
                  BTPS_MemCopy(Message->ReportData, GetReportConfirmationData->ReportDataPayload, Message->ReportLength);

               /* Build the Event Data.                                 */
               HIDMEventData.EventType                                                     = hetHIDGetReportResponse;
               HIDMEventData.EventLength                                                   = HIDM_HID_GET_REPORT_RESPONSE_DATA_SIZE;

               HIDMEventData.EventData.GetReportResponseEventData.HIDManagerDataCallbackID = 0;
               HIDMEventData.EventData.GetReportResponseEventData.RemoteDeviceAddress      = Message->RemoteDeviceAddress;
               HIDMEventData.EventData.GetReportResponseEventData.ReportType               = Message->ReportType;
               HIDMEventData.EventData.GetReportResponseEventData.ReportLength             = Message->ReportLength;
               HIDMEventData.EventData.GetReportResponseEventData.ReportData               = GetReportConfirmationData->ReportDataPayload;

               /* Now Dispatch the Events.                              */
               DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)Message);
            }
         }

         /* Finished with the Message, so go ahead and delete the       */
         /* memory.                                                     */
         BTPS_FreeMemory(Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetReportConfirmationEvent(HID_Host_Set_Report_Confirmation_Data_t *SetReportConfirmationData)
{
   HIDM_Event_Data_t                      HIDMEventData;
   HIDM_HID_Set_Report_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetReportConfirmationData)
   {
      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SET_REPORT_RESPONSE_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_SET_REPORT_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = SetReportConfirmationData->BD_ADDR;

      if(ConvertHIDResultType(SetReportConfirmationData->Status, &(Message.Status)))
      {
         /* Build the Event Data.                                       */
         HIDMEventData.EventType                                                     = hetHIDSetReportResponse;
         HIDMEventData.EventLength                                                   = HIDM_HID_SET_REPORT_RESPONSE_DATA_SIZE;

         HIDMEventData.EventData.SetReportResponseEventData.HIDManagerDataCallbackID = 0;
         HIDMEventData.EventData.SetReportResponseEventData.RemoteDeviceAddress      = Message.RemoteDeviceAddress;
         HIDMEventData.EventData.SetReportResponseEventData.Status                   = Message.Status;

         /* Now Dispatch the Events.                                    */
         DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetProtocolConfirmationEvent(HID_Host_Get_Protocol_Confirmation_Data_t *GetProtocolConfirmationData)
{
   HIDM_Event_Data_t                        HIDMEventData;
   HIDM_HID_Get_Protocol_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GetProtocolConfirmationData)
   {
      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_GET_PROTOCOL_RESPONSE_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_GET_PROTOCOL_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = GetProtocolConfirmationData->BD_ADDR;

      switch(GetProtocolConfirmationData->Protocol)
      {
         case ptBoot:
            Message.Protocol = hmpBoot;
            break;
         case ptReport:
            Message.Protocol = hmpReport;
            break;
      }

      if(ConvertHIDResultType(GetProtocolConfirmationData->Status, &(Message.Status)))
      {
         /* Build the Event Data.                                       */
         HIDMEventData.EventType                                                       = hetHIDGetProtocolResponse;
         HIDMEventData.EventLength                                                     = HIDM_HID_GET_PROTOCOL_RESPONSE_DATA_SIZE;

         HIDMEventData.EventData.GetProtocolResponseEventData.HIDManagerDataCallbackID = 0;
         HIDMEventData.EventData.GetProtocolResponseEventData.RemoteDeviceAddress      = Message.RemoteDeviceAddress;
         HIDMEventData.EventData.GetProtocolResponseEventData.Status                   = Message.Status;
         HIDMEventData.EventData.GetProtocolResponseEventData.Protocol                 = Message.Protocol;

         /* Now Dispatch the Events.                                    */
         DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetProtocolConfirmationEvent(HID_Host_Set_Protocol_Confirmation_Data_t *SetProtocolConfirmationData)
{
   HIDM_Event_Data_t                        HIDMEventData;
   HIDM_HID_Set_Protocol_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetProtocolConfirmationData)
   {
      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SET_PROTOCOL_RESPONSE_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_SET_PROTOCOL_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = SetProtocolConfirmationData->BD_ADDR;

      if(ConvertHIDResultType(SetProtocolConfirmationData->Status, &(Message.Status)))
      {
         /* Build the Event Data.                                       */
         HIDMEventData.EventType                                                       = hetHIDSetProtocolResponse;
         HIDMEventData.EventLength                                                     = HIDM_HID_SET_PROTOCOL_RESPONSE_DATA_SIZE;

         HIDMEventData.EventData.SetProtocolResponseEventData.HIDManagerDataCallbackID = 0;
         HIDMEventData.EventData.SetProtocolResponseEventData.RemoteDeviceAddress      = Message.RemoteDeviceAddress;
         HIDMEventData.EventData.SetProtocolResponseEventData.Status                   = Message.Status;

         /* Now Dispatch the Events.                                    */
         DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetIdleConfirmationEvent(HID_Host_Get_Idle_Confirmation_Data_t *GetIdleConfirmationData)
{
   HIDM_Event_Data_t                    HIDMEventData;
   HIDM_HID_Get_Idle_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GetIdleConfirmationData)
   {
      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_GET_IDLE_RESPONSE_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_GET_IDLE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = GetIdleConfirmationData->BD_ADDR;
      Message.IdleRate                      = GetIdleConfirmationData->IdleRate;

      if(ConvertHIDResultType(GetIdleConfirmationData->Status, &(Message.Status)))
      {
         /* Build the Event Data.                                       */
         HIDMEventData.EventType                                                   = hetHIDGetIdleResponse;
         HIDMEventData.EventLength                                                 = HIDM_HID_GET_IDLE_RESPONSE_DATA_SIZE;

         HIDMEventData.EventData.GetIdleResponseEventData.HIDManagerDataCallbackID = 0;
         HIDMEventData.EventData.GetIdleResponseEventData.RemoteDeviceAddress      = Message.RemoteDeviceAddress;
         HIDMEventData.EventData.GetIdleResponseEventData.Status                   = Message.Status;
         HIDMEventData.EventData.GetIdleResponseEventData.IdleRate                 = Message.IdleRate;

         /* Now Dispatch the Events.                                    */
         DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetIdleConfirmationEvent(HID_Host_Set_Idle_Confirmation_Data_t *SetIdleConfirmationData)
{
   HIDM_Event_Data_t                    HIDMEventData;
   HIDM_HID_Set_Idle_Response_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetIdleConfirmationData)
   {
      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HID_MANAGER;
      Message.MessageHeader.MessageFunction = HIDM_MESSAGE_FUNCTION_SET_IDLE_RESPONSE_EVENT;
      Message.MessageHeader.MessageLength   = (HIDM_HID_SET_IDLE_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.HIDDataEventsHandlerID        = 0;
      Message.RemoteDeviceAddress           = SetIdleConfirmationData->BD_ADDR;

      if(ConvertHIDResultType(SetIdleConfirmationData->Status, &(Message.Status)))
      {
         /* Build the Event Data.                                       */
         HIDMEventData.EventType                                                   = hetHIDSetIdleResponse;
         HIDMEventData.EventLength                                                 = HIDM_HID_SET_IDLE_RESPONSE_DATA_SIZE;

         HIDMEventData.EventData.SetIdleResponseEventData.HIDManagerDataCallbackID = 0;
         HIDMEventData.EventData.SetIdleResponseEventData.RemoteDeviceAddress      = Message.RemoteDeviceAddress;
         HIDMEventData.EventData.SetIdleResponseEventData.Status                   = Message.Status;

         /* Now Dispatch the Events.                                    */
         DispatchHIDDataEvent(&HIDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


   /* The following function is the function that is responsible for    */
   /* processing HID Host Events that have been received.  This function*/
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessHIDHEvent(HIDM_HIDH_Event_Data_t *HIDHEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(HIDHEventData)
   {
      /* Process the event based on the event type.                     */
      switch(HIDHEventData->EventType)
      {
         case etHID_Host_Open_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(HIDHEventData->EventData.HID_Host_Open_Request_Indication_Data));
            break;
         case etHID_Host_Open_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Indication\n"));

            ProcessOpenIndicationEvent(&(HIDHEventData->EventData.HID_Host_Open_Indication_Data));
            break;
         case etHID_Host_Open_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Confirmation\n"));

            ProcessOpenConfirmationEvent(TRUE, &(HIDHEventData->EventData.HID_Host_Open_Confirmation_Data));
            break;
         case etHID_Host_Close_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Indication\n"));

            ProcessCloseIndicationEvent(&(HIDHEventData->EventData.HID_Host_Close_Indication_Data));
            break;
         case etHID_Host_Boot_Keyboard_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Boot Keyboard Data Indication\n"));

            ProcessBootKeyboardDataIndicationEvent(&(HIDHEventData->EventData.HID_Host_Boot_Keyboard_Data));
            break;
         case etHID_Host_Boot_Keyboard_Repeat_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Boot Keyboard Repeat Indication\n"));

            ProcessBootKeyboardRepeatIndicationEvent(&(HIDHEventData->EventData.HID_Host_Boot_Keyboard_Repeat_Data));
            break;
         case etHID_Host_Boot_Mouse_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Boot Mouse Data Indication\n"));

            ProcessBootMouseDataIndicationEvent(&(HIDHEventData->EventData.HID_Host_Boot_Mouse_Data));
            break;
         case etHID_Host_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Indication\n"));

            ProcessDataIndicationEvent(&(HIDHEventData->EventData.HID_Host_Data_Indication_Data));
            break;
         case etHID_Host_Get_Report_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Report Confirmation\n"));

            ProcessGetReportConfirmationEvent(&(HIDHEventData->EventData.HID_Host_Get_Report_Confirmation_Data));
            break;
         case etHID_Host_Set_Report_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Report Confirmation\n"));

            ProcessSetReportConfirmationEvent(&(HIDHEventData->EventData.HID_Host_Set_Report_Confirmation_Data));
            break;
         case etHID_Host_Get_Protocol_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Protocol Confirmation\n"));

            ProcessGetProtocolConfirmationEvent(&(HIDHEventData->EventData.HID_Host_Get_Protocol_Confirmation_Data));
            break;
         case etHID_Host_Set_Protocol_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Protocol Confirmation\n"));

            ProcessSetProtocolConfirmationEvent(&(HIDHEventData->EventData.HID_Host_Set_Protocol_Confirmation_Data));
            break;
         case etHID_Host_Get_Idle_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Idle Confirmation\n"));

            ProcessGetIdleConfirmationEvent(&(HIDHEventData->EventData.HID_Host_Get_Idle_Confirmation_Data));
            break;
         case etHID_Host_Set_Idle_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Idle Confirmation\n"));

            ProcessSetIdleConfirmationEvent(&(HIDHEventData->EventData.HID_Host_Set_Idle_Confirmation_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown HID Host Event Type: %d\n", HIDHEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid HID Host Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for out-going        */
   /* connection management).                                           */
static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                                Result;
   HID_Entry_Info_t                  *HIDEntryInfo;
   Connection_Entry_t                *ConnectionEntry;
   HID_Host_Open_Confirmation_Data_t  OpenConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HIDM): 0x%08X, %d\n", StatusType, Status));

   /* First, determine if we are tracking a connection to this device.  */
   if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL) && (ConnectionEntry->ConnectionState != csConnected))
   {
      /* Next, let's loop through the list and see if there is an       */
      /* out-going Event connection being tracked for this event.       */
      HIDEntryInfo = HIDEntryInfoList;

      while(HIDEntryInfo)
      {
         /* Check to see if there is a out-going connection operation.  */
         if((!(HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY)) && (COMPARE_BD_ADDR(BD_ADDR, HIDEntryInfo->ConnectionBD_ADDR)))
         {
            /* Match found.                                             */
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Outgoing Connection Entry found\n"));
            break;
         }
         else
            HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
      }

      /* See if there is any processing that is required (i.e. match    */
      /* found).                                                        */
      if(HIDEntryInfo)
      {
         /* Process the status event.                                   */

         /* Initialize common connection event members.                 */
         BTPS_MemInitialize(&OpenConfirmationData, 0, sizeof(HID_Host_Open_Confirmation_Data_t));

         OpenConfirmationData.BD_ADDR = BD_ADDR;

         if(Status)
         {
            /* Disconnect the device.                                   */
            DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

            /* Connection Failed.                                       */

            /* Go ahead and delete the connection from the list.        */
            if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntry);

            /* Map the status to a known status.                        */
            switch(Status)
            {
               case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                  OpenConfirmationData.OpenStatus = HID_OPEN_PORT_STATUS_CONNECTION_REFUSED;
                  break;
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                  OpenConfirmationData.OpenStatus = HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT;
                  break;
               default:
                  OpenConfirmationData.OpenStatus = HID_OPEN_PORT_STATUS_UNKNOWN_ERROR;
                  break;
            }

            /* * NOTE * This function will delete the HID Info entry    */
            /*          from the list.                                  */
            ProcessOpenConfirmationEvent(TRUE, &OpenConfirmationData);

            /* Flag that the connection has been deleted.               */
            ConnectionEntry = NULL;
         }
         else
         {
            /* Connection succeeded.                                    */

            /* Move the state to the connecting state.                  */
            ConnectionEntry->ConnectionState = csConnecting;

            if(((Result = _HIDM_Connect_Remote_Device(BD_ADDR, ConnectionEntry?ConnectionEntry->ConnectionFlags:0)) != 0) && (Result != BTHID_HOST_ERROR_ALREADY_CONNECTED))
            {
               /* Error opening device.                                 */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               OpenConfirmationData.OpenStatus = HID_OPEN_PORT_STATUS_UNKNOWN_ERROR;

               /* * NOTE * This function will delete the HID Info entry */
               /*          from the list.                               */
               ProcessOpenConfirmationEvent(TRUE, &OpenConfirmationData);

               /* Flag that the connection has been deleted.            */
               ConnectionEntry = NULL;
            }
            else
            {
               /* If the device is already connected, we will dispach   */
               /* the the Status only (note this case shouldn't really  */
               /* occur, but just to be safe we will clean up our state */
               /* machine).                                             */
               if(Result == BTHID_HOST_ERROR_ALREADY_CONNECTED)
               {
                  ConnectionEntry->ConnectionState = csConnected;

                  OpenConfirmationData.OpenStatus  = 0;

                  ProcessOpenConfirmationEvent(FALSE, &OpenConfirmationData);
               }
            }
         }
      }

      /* Next, we will check to see if this event is referencing an     */
      /* incoming connection.                                           */
      if((ConnectionEntry) && ((ConnectionEntry->ConnectionState == csAuthenticating) || (ConnectionEntry->ConnectionState == csEncrypting)))
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Entry found\n"));

         /* Status does not reference an outgoing connection.           */
         if(!Status)
         {
            /* Success, accept the connection.                          */
            /* * NOTE * The Connection Flags have been set at this      */
            /*          point (either the default OR overridden by      */
            /*          authorization having been specified.            */
            _HIDM_Connection_Request_Response(BD_ADDR, TRUE, ConnectionEntry->ConnectionFlags);
         }
         else
         {
            /* Failure, reject the connection.                          */
            _HIDM_Connection_Request_Response(BD_ADDR, FALSE, 0);

            /* First, delete the Connection Entry we are tracking.      */
            if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntry);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HIDM)\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HID Manager Asynchronous Events.            */
static void BTPSAPI BTPMDispatchCallback_HIDM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process HID Manager Notification Events.            */
static void BTPSAPI BTPMDispatchCallback_HIDH(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an SPP Event Update.           */
            if(((HIDM_Update_Data_t *)CallbackParameter)->UpdateType == utHIDHEvent)
            {
               /* Process the Notification.                             */
               ProcessHIDHEvent(&(((HIDM_Update_Data_t *)CallbackParameter)->UpdateData.HIDHEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all HID Manager Messages.   */
static void BTPSAPI HIDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HID_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a HID Manager defined    */
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
               /* HID Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HIDM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HID Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an HID Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HID Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HIDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process HID Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HID_MANAGER, HIDManagerGroupHandler, NULL))
         {
            /* Initialize the actual HID Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the HID Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _HIDM_Initialize()))
            {
               /* Check to see if any initialization data was specified.*/
               if(InitializationData)
                  IncomingConnectionFlags = ((HIDM_Initialization_Data_t *)InitializationData)->IncomingConnectionFlags;
               else
                  IncomingConnectionFlags = 0;

               /* Determine the current Device Power State.             */
               CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

               /* Initialize a unique, starting HID Callback ID.        */
               NextCallbackID          = 0x000000001;

               /* Go ahead and flag that this module is initialized.    */
               Initialized             = TRUE;

               /* Flag success.                                         */
               Result                  = 0;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _HIDM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HID_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HID_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the HID Manager Implementation that  */
            /* we are shutting down.                                    */
            _HIDM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the HID Entry Information List is empty.  */
            FreeHIDEntryInfoList(&HIDEntryInfoList);

            /* Make sure that the HID Entry Data Information List is    */
            /* empty.                                                   */
            FreeHIDEntryInfoList(&HIDEntryInfoDataList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HIDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int               Result;
   HID_Entry_Info_t *tmpHIDEntryInfo;
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the HID Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _HIDM_SetBluetoothStackID((unsigned int)Result);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the HID Manager that the Stack has been closed.*/
               _HIDM_SetBluetoothStackID(0);

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
               HIDEntryInfo = HIDEntryInfoList;

               while(HIDEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(!(HIDEntryInfo->Flags & HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY))
                  {
                     if(HIDEntryInfo->ConnectionEvent)
                     {
                        HIDEntryInfo->ConnectionStatus = HIDM_HID_DEVICE_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                        BTPS_SetEvent(HIDEntryInfo->ConnectionEvent);

                        HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
                     }
                     else
                     {
                        /* Entry was waiting on a response, but it was  */
                        /* registered as either an Event Callback or    */
                        /* Connection Message.  Regardless we need to   */
                        /* delete it.                                   */
                        tmpHIDEntryInfo = HIDEntryInfo;

                        HIDEntryInfo    = HIDEntryInfo->NextHIDEntryInfoPtr;

                        if((tmpHIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, tmpHIDEntryInfo->CallbackID)) != NULL)
                           FreeHIDEntryInfoEntryMemory(tmpHIDEntryInfo);
                     }
                  }
                  else
                     HIDEntryInfo = HIDEntryInfo->NextHIDEntryInfoPtr;
               }

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeConnectionEntryList(&ConnectionEntryList);
               break;
            case detRemoteDeviceAuthenticationStatus:
               /* Authentication Status, process the Status Event.      */
               ProcessDEVMStatusEvent(dstAuthentication, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status);
               break;
            case detRemoteDeviceEncryptionStatus:
               /* Encryption Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstEncryption, EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status);
               break;
            case detRemoteDeviceConnectionStatus:
               /* Connection Status, process the Status Event.          */
               ProcessDEVMStatusEvent(dstConnection, EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status);
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the HID Manager of a specific Update Event.  The HID    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t HIDM_NotifyUpdate(HIDM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utHIDHEvent:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing HID Host Event: %d\n", UpdateData->UpdateData.HIDHEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_HIDH, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming HID connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HID Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI HIDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept, unsigned long ConnectionFlags)
{
   int                 ret_val;
   BD_ADDR_t           NULL_BD_ADDR;
   Boolean_t           Authenticate;
   Boolean_t           Encrypt;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* First, check to make sure the HID Host Manager has been           */
   /* initialized.                                                      */
   if(Initialized)
   {
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Device is powered on, next, verify that we are already*/
               /* tracking a connection for the specified connection    */
               /* type.                                                 */
               if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Accept));

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(Accept)
                  {
                     /* Determine if Authentication and/or Encryption is*/
                     /* required for this link.                         */
                     if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                        Authenticate = TRUE;
                     else
                        Authenticate = FALSE;

                     if(IncomingConnectionFlags & HIDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                        Encrypt = TRUE;
                     else
                        Encrypt = FALSE;

                     if((Authenticate) || (Encrypt))
                     {
                        if(Encrypt)
                           ret_val = DEVM_EncryptRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                        else
                           ret_val = DEVM_AuthenticateRemoteDevice(ConnectionEntry->BD_ADDR, 0);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                     /* We need to map the Request flags from the       */
                     /* Bluetopia PM HID Host request flags to the      */
                     /* correct Bluetopia HID Host Request flags.       */
                     if(ConnectionFlags & HIDM_CONNECTION_REQUEST_CONNECTION_FLAGS_PARSE_BOOT)
                        ConnectionEntry->ConnectionFlags = HID_HOST_CONNECTION_FLAGS_PARSE_BOOT;
                     else
                        ConnectionEntry->ConnectionFlags = 0;

                     if(ConnectionFlags & HIDM_CONNECTION_REQUEST_CONNECTION_FLAGS_REPORT_MODE)
                        ConnectionEntry->ConnectionFlags |= HID_HOST_CONNECTION_FLAGS_REPORT_MODE;

                     if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                     {
                        /* Authorization not required, and we are       */
                        /* already in the correct state.                */
                        ret_val = _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, TRUE, ConnectionEntry->ConnectionFlags);

                        if(ret_val)
                        {
                           _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, FALSE, 0);

                           /* Go ahead and delete the entry because we  */
                           /* are finished with tracking it.            */
                           if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntry);
                        }
                        else
                           ConnectionEntry->ConnectionState = csConnecting;
                     }
                     else
                     {
                        /* If we were successfully able to Authenticate */
                        /* and/or Encrypt, then we need to set the      */
                        /* correct state.                               */
                        if(!ret_val)
                        {
                           if(Encrypt)
                              ConnectionEntry->ConnectionState = csEncrypting;
                           else
                              ConnectionEntry->ConnectionState = csAuthenticating;

                           /* Flag success to the caller.               */
                           ret_val = 0;
                        }
                        else
                        {
                           /* Error, reject the request.                */
                           _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, FALSE, 0);

                           /* Go ahead and delete the entry because we  */
                           /* are finished with tracking it.            */
                           if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                              FreeConnectionEntryMemory(ConnectionEntry);
                        }
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     ret_val = _HIDM_Connection_Request_Response(ConnectionEntry->BD_ADDR, FALSE, 0);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                        FreeConnectionEntryMemory(ConnectionEntry);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
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
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote HID device.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHIDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HIDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   BD_ADDR_t           NULL_BD_ADDR;
   Boolean_t           Delete;
   unsigned int        CallbackID;
   HID_Entry_Info_t    HIDEntryInfo;
   HID_Entry_Info_t   *HIDEntryInfoPtr;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_BD_ADDR(RemoteDeviceAddress, NULL_BD_ADDR))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) == NULL)
               {
                  /* Entry is not present, go ahead and create a new    */
                  /* entry.                                             */
                  BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

                  ConnectionEntry.BD_ADDR         = RemoteDeviceAddress;
                  ConnectionEntry.ConnectionState = csIdle;

                  if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
                  {
                     /* Attempt to add an entry into the HID Entry list.*/
                     BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

                     HIDEntryInfo.CallbackID        = GetNextCallbackID();
                     HIDEntryInfo.ClientID          = MSG_GetServerAddressID();
                     HIDEntryInfo.ConnectionBD_ADDR = RemoteDeviceAddress;
                     HIDEntryInfo.EventCallback     = CallbackFunction;
                     HIDEntryInfo.CallbackParameter = CallbackParameter;

                     if(ConnectionStatus)
                        HIDEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

                     Delete = FALSE;

                     if((!ConnectionStatus) || ((ConnectionStatus) && (HIDEntryInfo.ConnectionEvent)))
                     {
                        if((HIDEntryInfoPtr = AddHIDEntryInfoEntry(&HIDEntryInfoList, &HIDEntryInfo)) != NULL)
                        {
                           DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                           /* Next, attempt to open the remote device   */
                           if(ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                              ConnectionEntryPtr->ConnectionState = csEncrypting;
                           else
                           {
                              if(ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                                 ConnectionEntryPtr->ConnectionState = csAuthenticating;
                              else
                                 ConnectionEntryPtr->ConnectionState = csIdle;
                           }

                           /* Note that we need to map the Bluetopia PM */
                           /* HID Host Connection Flags to the Bluetopia*/
                           /* HID Host Connection Flags.                */
                           if(ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_PARSE_BOOT)
                              ConnectionEntryPtr->ConnectionFlags = HID_HOST_CONNECTION_FLAGS_PARSE_BOOT;
                           else
                              ConnectionEntryPtr->ConnectionFlags = 0;

                           if(ConnectionFlags & HIDM_CONNECT_HID_DEVICE_FLAGS_REPORT_MODE)
                              ConnectionEntryPtr->ConnectionFlags |= HID_HOST_CONNECTION_FLAGS_REPORT_MODE;

                           DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

                           ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csIdle)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                           if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                           {
                              /* Check to see if we need to actually    */
                              /* issue the Remote connection.           */
                              if(ret_val < 0)
                              {
                                 /* Set the state to connecting remote  */
                                 /* device.                             */
                                 ConnectionEntryPtr->ConnectionState = csConnecting;

                                 if((ret_val = _HIDM_Connect_Remote_Device(RemoteDeviceAddress, ConnectionEntryPtr->ConnectionFlags)) != 0)
                                 {
                                    if(ret_val == BTHID_HOST_ERROR_ALREADY_CONNECTED)
                                       ret_val = BTPM_ERROR_CODE_HID_DEVICE_ALREADY_CONNECTED;
                                    else
                                       ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_HID_DEVICE;

                                    /* Error opening device, go ahead   */
                                    /* and delete the entry that was    */
                                    /* added.                           */
                                    if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfoPtr->CallbackID)) != NULL)
                                    {
                                       if(HIDEntryInfoPtr->ConnectionEvent)
                                          BTPS_CloseEvent(HIDEntryInfoPtr->ConnectionEvent);

                                       FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
                                    }
                                 }
                              }
                           }

                           /* Next, determine if the caller has         */
                           /* requested a blocking open.                */
                           if((!ret_val) && (ConnectionStatus))
                           {
                              /* Blocking open, go ahead and wait for   */
                              /* the event.                             */

                              /* Note the Callback ID.                  */
                              CallbackID      = HIDEntryInfoPtr->CallbackID;

                              /* Note the Open Event.                   */
                              ConnectionEvent = HIDEntryInfoPtr->ConnectionEvent;

                              /* Release the lock because we are        */
                              /* finished with it.                      */
                              DEVM_ReleaseLock();

                              BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                              /* Re-acquire the Lock.                   */
                              if(DEVM_AcquireLock())
                              {
                                 if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, CallbackID)) != NULL)
                                 {
                                    /* Note the connection status.      */
                                    *ConnectionStatus = HIDEntryInfoPtr->ConnectionStatus;

                                    BTPS_CloseEvent(HIDEntryInfoPtr->ConnectionEvent);

                                    FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);

                                    /* Flag success to the caller.      */
                                    ret_val = 0;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_HID_DEVICE;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

                              /* Flag that the entry is to be deleted.  */
                              Delete = TRUE;
                           }
                           else
                           {
                              /* If we are not tracking this connection */
                              /* OR there was an error, go ahead and    */
                              /* delete the entry that was added.       */
                              if((!CallbackFunction) || (ret_val))
                              {
                                 if((HIDEntryInfoPtr = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDEntryInfo.CallbackID)) != NULL)
                                 {
                                    if(HIDEntryInfoPtr->ConnectionEvent)
                                       BTPS_CloseEvent(HIDEntryInfoPtr->ConnectionEvent);

                                    FreeHIDEntryInfoEntryMemory(HIDEntryInfoPtr);
                                 }
                              }
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;

                     /* If an error occurred, go ahead and delete the   */
                     /* Connection Information that was added.          */
                     if((ret_val) || (Delete))
                     {
                        if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                           FreeConnectionEntryMemory(ConnectionEntryPtr);
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
               else
               {
                  if(ConnectionEntryPtr->ConnectionState == csConnected)
                     ret_val = BTPM_ERROR_CODE_HID_DEVICE_ALREADY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_HID_DEVICE_CONNECTION_IN_PROGRESS;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

            /* Release the Lock because we are finished with it.        */
            if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
               DEVM_ReleaseLock();
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for the   */
   /* local device to disconnect a currently connected remote device    */
   /* (connected to the local device's HID Host).  This function accepts*/
   /* as input the Remote Device Address of the Remote HID Device to    */
   /* disconnect from the local HID Host followed by a BOOLEAN value    */
   /* that specifies whether or not the device is be disconnected via a */
   /* Virtual Cable Disconnection (TRUE), or merely disconnected at the */
   /* Bluetooth link (FALSE).  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.            */
int BTPSAPI HIDM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect)
{
   int                               ret_val;
   Connection_Entry_t               *ConnectionEntry;
   HID_Host_Close_Indication_Data_t  CloseIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Guard against disconnecting if a connection is in        */
            /* progress.                                                */
            if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) == NULL) || ((ConnectionEntry) && (ConnectionEntry->ConnectionState == csConnected)))
            {
               /* Nothing to do here other than to call the actual      */
               /* function to Disconnect the HID device.                */
               ret_val = _HIDM_Disconnect_Device(RemoteDeviceAddress, SendVirtualCableDisconnect);

               /* If the result was successful, we need to make sure we */
               /* clean up everything and dispatch the event to all     */
               /* registered clients.                                   */
               if(!ret_val)
               {
                  /* Fake a HID Close Event to dispatch to all          */
                  /* registered clients that the device is no longer    */
                  /* connected.                                         */
                  CloseIndicationData.BD_ADDR = RemoteDeviceAddress;

                  ProcessCloseIndicationEvent(&CloseIndicationData);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_HID_DEVICE_CONNECTION_IN_PROGRESS;
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected HID     */
   /* Devices.  This function accepts a pointer to a buffer that will   */
   /* receive any currently connected HID devices.  The first parameter */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated with   */
   /* the total number of connected devices if the function is          */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI HIDM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                 ret_val;
   unsigned int        NumberConnected;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, check to see if the input parameters appear to be        */
      /* semi-valid.                                                    */
      if(((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)) || ((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Check to see if the device is powered on.                */
            if(CurrentPowerState)
            {
               /* Let's determine how many devices are actually         */
               /* connected.                                            */
               NumberConnected = 0;
               ConnectionEntry = ConnectionEntryList;

               while(ConnectionEntry)
               {
                  /* Note that we are only counting devices that are    */
                  /* counting devices that either in the connected state*/
                  /* or the connecting state (i.e. have been authorized */
                  /* OR passed authentication).                         */
                  if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
                     NumberConnected++;

                  ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
               }

               /* We now have the total number of devices that will     */
               /* satisy the query.                                     */
               if(TotalNumberConnectedDevices)
                  *TotalNumberConnectedDevices = NumberConnected;

               /* Now that have the total, we need to build the         */
               /* Connected Device List.                                */

               /* See if the caller would like to copy some (or all) of */
               /* the list.                                             */
               if(MaximumRemoteDeviceListEntries)
               {
                  /* If there are more entries in the returned list than*/
                  /* the buffer specified, we need to truncate it.      */
                  if(MaximumRemoteDeviceListEntries >= NumberConnected)
                     MaximumRemoteDeviceListEntries = NumberConnected;

                  NumberConnected = 0;

                  ConnectionEntry = ConnectionEntryList;

                  while((ConnectionEntry) && (NumberConnected < MaximumRemoteDeviceListEntries))
                  {
                     /* Note that we are only counting devices that are */
                     /* counting devices that either in the connected   */
                     /* state or the connecting state (i.e.  have been  */
                     /* authorized OR passed authentication).           */
                     if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
                        RemoteDeviceAddressList[NumberConnected++] = ConnectionEntry->BD_ADDR;

                     ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
                  }

                  /* Note the total number of devices that were copied  */
                  /* into the array.                                    */
                  ret_val = (int)NumberConnected;
               }
               else
                  ret_val = 0;
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
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int BTPSAPI HIDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* All that remains is to note the specified Flags.            */
         IncomingConnectionFlags = ConnectionFlags;

         /* Flag success to the caller.                                 */
         ret_val                 = 0;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is used to set the HID Host Keyboard Key   */
   /* Repeat behavior.  Some Host operating systems nativity support Key*/
   /* Repeat automatically, but for those Host operating systems that do*/
   /* not - this function will instruct the HID Manager to simulate Key */
   /* Repeat behavior (with the specified parameters).  This function   */
   /* accepts the initial amount to delay (in milliseconds) before      */
   /* starting the repeat functionality.  The final parameter specifies */
   /* the rate of repeat (in milliseconds).  This function returns zero */
   /* if successful or a negative value if there was an error.          */
   /* * NOTE * Specifying zero for the Repeat Delay (first parameter)   */
   /*          will disable HID Manager Key Repeat processing.  This    */
   /*          means that only Key Up/Down events will be dispatched    */
   /*          and No Key Repeat events will be dispatched.             */
   /* * NOTE * The Key Repeat parameters can only be changed when there */
   /*          are no actively connected HID devices.                   */
int BTPSAPI HIDM_Set_Keyboard_Repeat_Rate(unsigned int RepeatDelay, unsigned int RepeatRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Simply call the function to Set the Keyboard Repeat Rate.*/
            ret_val = _HIDM_Set_Keyboard_Repeat_Rate(RepeatDelay, RepeatRate);
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* HID Report Data to a currently connected remote device.  This     */
   /* function accepts as input the HID Manager Data Handler ID         */
   /* (registered via call to the HIDM_Register_Data_Event_Callback()   */
   /* function), followed by the remote device address of the remote HID*/
   /* device to send the report data to, followed by the report data    */
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int BTPSAPI HIDM_Send_Report_Data(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HIDManagerDataCallbackID) && (ReportDataLength) && (ReportData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID Data.                     */
                  ret_val = _HIDM_Send_Report_Data(RemoteDeviceAddress, ReportDataLength, ReportData);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report requested.  The    */
   /* fourth parameter is the Report ID determined by the Device's SDP  */
   /* record.  Passing HIDM_INVALID_REPORT_ID as the value for this     */
   /* parameter will indicate that this parameter is not used and will  */
   /* exclude the appropriate byte from the transaction payload.  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Get_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID Request.                  */
                  ret_val = _HIDM_Send_Get_Report_Request(RemoteDeviceAddress, ReportType, ReportID);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input     */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report being sent.  The   */
   /* final two parameters to this function are the Length of the Report*/
   /* Data to send and a pointer to the Report Data that will be sent.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Set_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HIDManagerDataCallbackID) && (ReportDataLength) && (ReportData))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID request.                  */
                  ret_val = _HIDM_Send_Set_Report_Request(RemoteDeviceAddress, ReportType, ReportDataLength, ReportData);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Get_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID request.                  */
                  ret_val = _HIDM_Send_Get_Protocol_Request(RemoteDeviceAddress);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the protocol to be set.  This     */
   /* function returns a zero if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Set_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID request.                  */
                  ret_val = _HIDM_Send_Set_Protocol_Request(RemoteDeviceAddress, Protocol);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Get_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID request.                  */
                  ret_val = _HIDM_Send_Get_Idle_Request(RemoteDeviceAddress);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the Idle Rate to be set.  The Idle*/
   /* Rate LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms */
   /* with a range from 4ms to 1.020s).  This function returns a zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int BTPSAPI HIDM_Send_Set_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* First, find the local handler.                        */
               if(SearchHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID))
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the HID request.                  */
                  ret_val = _HIDM_Send_Set_Idle_Request(RemoteDeviceAddress, IdleRate);
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service.  This Callback will be dispatched by*/
   /* the HID Manager when various HID Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HID Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HIDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int BTPSAPI HIDM_Register_Event_Callback(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   HID_Entry_Info_t HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the HID Entry list.         */
            BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

            HIDEntryInfo.CallbackID         = GetNextCallbackID();
            HIDEntryInfo.ClientID           = MSG_GetServerAddressID();
            HIDEntryInfo.EventCallback      = CallbackFunction;
            HIDEntryInfo.CallbackParameter  = CallbackParameter;
            HIDEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

            if(AddHIDEntryInfoEntry(&HIDEntryInfoList, &HIDEntryInfo))
               ret_val = HIDEntryInfo.CallbackID;
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
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HIDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HIDM_RegisterEventCallback() function).                           */
void BTPSAPI HIDM_Un_Register_Event_Callback(unsigned int HIDManagerCallbackID)
{
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(HIDManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoList, HIDManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeHIDEntryInfoEntryMemory(HIDEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service to explicitly process HID report     */
   /* data.  This Callback will be dispatched by the HID Manager when   */
   /* various HID Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HID Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HIDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HIDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HIDM_Register_Data_Event_Callback(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int               ret_val;
   HID_Entry_Info_t  HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a Data Event Handler registered.                 */
            if(!HIDEntryInfoDataList)
            {
               /* First, Register the handler locally.                  */
               BTPS_MemInitialize(&HIDEntryInfo, 0, sizeof(HID_Entry_Info_t));

               HIDEntryInfo.CallbackID         = GetNextCallbackID();
               HIDEntryInfo.ClientID           = MSG_GetServerAddressID();
               HIDEntryInfo.EventCallback      = CallbackFunction;
               HIDEntryInfo.CallbackParameter  = CallbackParameter;
               HIDEntryInfo.Flags              = HID_ENTRY_INFO_FLAGS_EVENT_CALLBACK_ENTRY;

               if(AddHIDEntryInfoEntry(&HIDEntryInfoDataList, &HIDEntryInfo))
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  ret_val = HIDEntryInfo.CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_HID_DATA_EVENTS_ALREADY_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_HID_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HIDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HIDM_Register_Data_Event_Callback() function).         */
void BTPSAPI HIDM_Un_Register_Data_Event_Callback(unsigned int HIDManagerDataCallbackID)
{
   HID_Entry_Info_t *HIDEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HIDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Delete the local handler.                                */
            if((HIDEntryInfo = DeleteHIDEntryInfoEntry(&HIDEntryInfoDataList, HIDManagerDataCallbackID)) != NULL)
            {
               /* All finished with the entry, delete it.               */
               FreeHIDEntryInfoEntryMemory(HIDEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


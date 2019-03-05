/*****< btpmoppm.c >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMOPPM - Object Push Manager for Stonestreet One Bluetooth              */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/09/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMOPPM.h"            /* BTPM OPPM Manager Prototypes/Constants.   */
#include "OPPMAPI.h"             /* OPPM Manager Prototypes/Constants.        */
#include "OPPMMSG.h"             /* BTPM OPPM Manager Message Formats.        */
#include "OPPMGR.h"              /* OPPM Manager Impl. Prototypes/Constants.  */
#include "OPPMUTIL.h"            /* OPPM Manager Utility Functions.           */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* to wait for a Serial Port to close when it is closed by the local */
   /* host.                                                             */
#define MAXIMUM_OPP_PORT_DELAY_TIMEOUT_MS                      (BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_CLOSE_DELAY_TIME_MS * BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_CLOSE_DELAY_RETRIES)

   /* The following constant represents the number of times that this   */
   /* module will attempt to retry waiting for the Port to Disconnect   */
   /* (before attempting to connect to a remote port) if it is          */
   /* connected.                                                        */
#define MAXIMUM_OPP_PORT_OPEN_DELAY_RETRY                      (BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_RETRIES)

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking connections.                          */
typedef enum
{
   csIdle,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnectingWaiting,
   csConnectingDevice,
   csConnecting,
   csConnected
} Connection_State_t;

   /* The following enumerated type is used to denote the current OPP   */
   /* operation that is currently on-going.                             */
typedef enum
{
   coNone,
   coAbort,
   coPushObject,
   coPullBusinessCard
} Current_Operation_t;

   /* Structure which is used to track connection and port info for this*/
   /* module.                                                           */
typedef struct _tagOPPM_Entry_Info_t
{
   unsigned int                  TrackingID;
   unsigned int                  OPPID;
   unsigned int                  PortNumber;
   unsigned long                 ConnectionFlags;
   unsigned long                 Flags;
   unsigned int                  DataBufferSize;
   unsigned int                  DataBufferSent;
   Byte_t                       *DataBuffer;
   Boolean_t                     DataFinal;
   unsigned int                  ClientID;
   unsigned long                 SupportedObjectTypes;
   BD_ADDR_t                     RemoteDeviceAddress;
   Event_t                       ConnectionEvent;
   unsigned int                  ConnectionStatus;
   unsigned int                  ConnectionTimerID;
   unsigned int                  ConnectionTimerCount;
   DWord_t                       ServiceRecordHandle;
   Connection_State_t            ConnectionState;
   Current_Operation_t           CurrentOperation;
   OPPM_Event_Callback_t         CallbackFunction;
   void                         *CallbackParameter;
   struct _tagOPPM_Entry_Info_t *NextOPPMEntryInfoPtr;
} OPPM_Entry_Info_t;

   /* The following constants are used with the Flags member of the     */
   /* OPPM_Entry_Info_t structure to denote various state information.  */
#define OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT                    0x00000001

#define OPPM_ENTRY_INFO_FLAGS_SERVER                           0x80000000

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection
} DEVM_Status_Type_t;

   /* The following structure represents the information needed to      */
   /* submit an event callback.                                         */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   OPPM_Event_Callback_t  CallbackFunction;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* Internal Variables to this Module (remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which holds the current power state of the device.       */
static Boolean_t CurrentPowerState;

   /* Variable which is used to hold the next (unique) tracking ID.     */
static unsigned int NextTrackingID;

   /* Variable which holds a pointer to the first element in the Object */
   /* Push Info list (which stores information about all open ports and */
   /* connections).                                                     */
static OPPM_Entry_Info_t *OPPMEntryInfoList;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextTrackingID(void);

static OPPM_Entry_Info_t *AddOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, OPPM_Entry_Info_t *EntryToAdd);
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByOPPID(OPPM_Entry_Info_t **ListHead, unsigned int OPPID);
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByTrackingID(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID);
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByConnection(OPPM_Entry_Info_t **ListHead, BD_ADDR_t RemoteDeviceAddress, Boolean_t Server);
static OPPM_Entry_Info_t *DeleteOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID);
static void FreeOPPMEntryInfoEntryMemory(OPPM_Entry_Info_t *EntryToFree);
static void FreeOPPMEntryInfoList(OPPM_Entry_Info_t **ListHead);

static void CleanupOPPMEntryInfo(OPPM_Entry_Info_t *EntryToCleanup);

static Word_t *ConvertUTF8ToUnicode(char *UTFString);
static char *ConvertUnicodeToUTF8(Word_t *UnicodeString);

static Boolean_t MapResponseStatusCodeToResponseCode(unsigned int ResponseStatusCode, Byte_t *ResponseCode);
static unsigned int MapResponseCodeToResponseStatusCode(Byte_t ResponseCode);

static void IssuePendingAbort(OPPM_Entry_Info_t *OPPMEntryInfo);

static int ProcessConnectionRequestResponse(unsigned int ClientID, unsigned int TrackingID, Boolean_t Accept);
static int ProcessRegisterServer(unsigned int ClientID, unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
static int ProcessUnRegisterServer(unsigned int ClientID, unsigned int TrackingID);
static int ProcessConnectRemoteDevice(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
static int ProcessDisconnect(unsigned int ClientID, unsigned int TrackingID);
static int ProcessAbort(unsigned int ClientID, unsigned int TrackingID);
static int ProcessPushObjectRequest(unsigned int ClientID, unsigned int TrackingID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);
static int ProcessPushObjectResponse(unsigned int ClientID, unsigned int TrackingID, unsigned int ResponseCode);
static int ProcessPullBusinessCardRequest(unsigned int ClientID, unsigned int TrackingID);
static int ProcessPullBusinessCardResponse(unsigned int ClientID, unsigned int TrackingID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

static void ProcessConnectionRequestResponseMessage(OPPM_Connection_Request_Response_Request_t *Message);
static void ProcessRegisterServerMessage(OPPM_Register_Server_Request_t *Message);
static void ProcessUnRegisterServerMessage(OPPM_Un_Register_Server_Request_t *Message);
static void ProcessConnectRemoteDeviceMessage(OPPM_Connect_Remote_Device_Request_t *Message);
static void ProcessDisconnectMessage(OPPM_Disconnect_Request_t *Message);
static void ProcessAbortMessage(OPPM_Abort_Request_t *Message);
static void ProcessPushObjectRequestMessage(OPPM_Push_Object_Request_Request_t *Message);
static void ProcessPushObjectResponseMessage(OPPM_Push_Object_Response_Request_t *Message);
static void ProcessPullBusinessCardRequestMessage(OPPM_Pull_Business_Card_Request_Request_t *Message);
static void ProcessPullBusinessCardResponseMessage(OPPM_Pull_Business_Card_Response_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void DispatchOPPMEvent(OPPM_Entry_Info_t *OPPMEntryInfo, OPPM_Event_Data_t *EventData, BTPM_Message_t *Message);

static void ProcessOpenRequestIndicationEvent(OPP_Open_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessOpenPortIndicationEvent(OPP_Open_Port_Indication_Data_t *OpenPortIndicationData);
static void ProcessOpenPortConfirmationEvent(OPP_Open_Port_Confirmation_Data_t *OpenPortConfirmationData);
static void ProcessClosePortIndicationEvent(OPP_Close_Port_Indication_Data_t *ClosePortIndicationData);
static void ProcessPushObjectIndicationEvent(OPP_Push_Object_Indication_Data_t *PushObjectIndicationData);
static void ProcessPushObjectConfirmationEvent(OPP_Push_Object_Confirmation_Data_t *PushObjectConfirmationData);
static void ProcessPullBusinessCardIndicationEvent(OPP_Pull_Business_Card_Indication_Data_t *PullBusinessCardIndicationData);
static void ProcessPullBusinessCardConfirmationEvent(OPP_Pull_Business_Card_Confirmation_Data_t *PullBusinessCardConfirmationData);
static void ProcessAbortIndicationEvent(OPP_Abort_Indication_Data_t *AbortIndicationData);
static void ProcessAbortConfirmationEvent(OPP_Abort_Confirmation_Data_t *AbortConfirmationData);

static void ProcessOPPEvent(OPPM_OPP_Event_Data_t *OPPEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter);

static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_OPPM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_OPP(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI ObjectPushManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique, Tracking ID that can be used to track a OPP    */
   /* Entry (client or server).                                         */
static unsigned int GetNextTrackingID(void)
{
   unsigned int ret_val;

   ret_val = ++NextTrackingID;

   if(NextTrackingID & 0x80000000)
      NextTrackingID = 1;

   return(ret_val);
}

   /* The following function adds the specified entry to the specified  */
   /* list.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the entry passed into this function.   */
   /* This function will return NULL if NO entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* list head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Tracking ID is the same as an entry already in the     */
   /*            list.  When this occurs, this function returns NULL.   */
static OPPM_Entry_Info_t *AddOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, OPPM_Entry_Info_t *EntryToAdd)
{
   OPPM_Entry_Info_t *AddedEntry = NULL;
   OPPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the parameters are not NULL.                           */
   if((ListHead) && (EntryToAdd))
   {
      /* Allocate memory for the entry data structure.                  */
      AddedEntry = (OPPM_Entry_Info_t *)BTPS_AllocateMemory(sizeof(OPPM_Entry_Info_t));

      if(AddedEntry)
      {
         /* Copy the entry data to the newly allocated memory.          */
         *AddedEntry = *EntryToAdd;

         AddedEntry->NextOPPMEntryInfoPtr = NULL;

         /* Check if the list is empty.                                 */
         if((tmpEntry = *ListHead) != NULL)
         {
            /* Find the last element.                                   */
            while(tmpEntry)
            {
               if(tmpEntry->TrackingID == AddedEntry->TrackingID)
               {
                  /* The entry is already in the list.  Free the memory */
                  /* and and set added entry to NULL.                   */
                  FreeOPPMEntryInfoEntryMemory(AddedEntry);
                  AddedEntry = NULL;

                  /* Abort the search.                                  */
                  tmpEntry   = NULL;
               }
               else
               {
                  /* If there is another entry, point to that entry.  If*/
                  /* not, end the search.                               */
                  if(tmpEntry->NextOPPMEntryInfoPtr)
                     tmpEntry = tmpEntry->NextOPPMEntryInfoPtr;
                  else
                     break;
               }
            }

            if(AddedEntry)
            {
               /* Add the entry to the end of the list.                 */
               tmpEntry->NextOPPMEntryInfoPtr = AddedEntry;
            }
         }
         else
            *ListHead = AddedEntry;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified OPP Entry (based on Tracking ID).  This function returns*/
   /* NULL if either the list head is invalid, the Tracking ID is       */
   /* invalid or the specified entry was NOT found.                     */
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByTrackingID(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID)
{
   OPPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", TrackingID));

   /* Check that the parameters appear to be semi-valid.                */
   if((ListHead) && (TrackingID))
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TrackingID != TrackingID))
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified OPP ID.  This function returns NULL if either the list  */
   /* head is invalid, the OPP ID is invalid, or the specified OPP ID   */
   /* was NOT found.                                                    */
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByOPPID(OPPM_Entry_Info_t **ListHead, unsigned int OPPID)
{
   OPPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", OPPID));

   /* Check that the parameters appear to be semi-valid.                */
   if((ListHead) && (OPPID))
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->OPPID != OPPID))
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified connection.  This function returns NULL if either the   */
   /* list head is invalid, the Remote Device is invalid, the Instance  */
   /* ID is invalid, or the specified entry was NOT found.              */
static OPPM_Entry_Info_t *SearchOPPMEntryInfoByConnection(OPPM_Entry_Info_t **ListHead, BD_ADDR_t RemoteDeviceAddress, Boolean_t Server)
{
   unsigned long      ServerFlags;
   OPPM_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter"));

   /* Check if ListHead is not NULL and the remote address appears to be*/
   /* semi-valid.                                                       */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
   {
      /* Note the Server Flags to test against when searching.          */
      ServerFlags = Server?OPPM_ENTRY_INFO_FLAGS_SERVER:0;

      /* Search the list for the specified entry.                       */
      FoundEntry  = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->RemoteDeviceAddress, RemoteDeviceAddress)) || ((FoundEntry->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER) != ServerFlags)))
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* The following function searches the specified OPP entry           */
   /* information list for the specified entry and removes it from the  */
   /* List.  This function returns NULL if either the OPP entry         */
   /* information list head is invalid, the entry is invalid, or the    */
   /* specified entry was NOT present in the list.  The entry returned  */
   /* will have the next entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeOPPMEntryInfoEntryMemory().                           */
static OPPM_Entry_Info_t *DeleteOPPMEntryInfoEntry(OPPM_Entry_Info_t **ListHead, unsigned int TrackingID)
{
   OPPM_Entry_Info_t *FoundEntry = NULL;
   OPPM_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that ListHead is not NULL.                                  */
   if(ListHead)
   {
      /* Search the list for the specified entry.                       */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->TrackingID != TrackingID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextOPPMEntryInfoPtr;
      }

      /* Check if the specified entry was found.                        */
      if(FoundEntry)
      {
         /* Check if the entry was the first entry in the list.         */
         if(LastEntry)
            LastEntry->NextOPPMEntryInfoPtr = FoundEntry->NextOPPMEntryInfoPtr;
         else
            *ListHead = FoundEntry->NextOPPMEntryInfoPtr;

         FoundEntry->NextOPPMEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   /* Return the specified entry or a NULL value.                       */
   return(FoundEntry);
}

   /* This function frees the specified Object Push entry information   */
   /* member.  No check is done on this entry other than making sure it */
   /* not NULL.                                                         */
static void FreeOPPMEntryInfoEntryMemory(OPPM_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and frees all memory of) every    */
   /* element of the specified Object Push entry information list.  The */
   /* list head pointer is set to NULL.                                 */
static void FreeOPPMEntryInfoList(OPPM_Entry_Info_t **ListHead)
{
   OPPM_Entry_Info_t *EntryToFree;
   OPPM_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Traverse the list and free every element.                      */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextOPPMEntryInfoPtr;

         /* Clean up any resources that might have been allocated.      */
         CleanupOPPMEntryInfo(tmpEntry);

         FreeOPPMEntryInfoEntryMemory(tmpEntry);
      }

      /* Set ListHead to NULL.                                          */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to free  */
   /* any resources that might have been allocated for the specified OPP*/
   /* Entry.  This Entry *DOES NOT* free the Entry itself, just any     */
   /* resources that have not been freed by the Entry.                  */
static void CleanupOPPMEntryInfo(OPPM_Entry_Info_t *EntryToCleanup)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToCleanup)
   {
      /* Close any currently executing timer.                           */
      if(EntryToCleanup->ConnectionTimerID)
         TMR_StopTimer(EntryToCleanup->ConnectionTimerID);

      /* Free any Events that might have been allocated.                */
      if(EntryToCleanup->ConnectionEvent)
         BTPS_CloseEvent(EntryToCleanup->ConnectionEvent);

      /* If there was an on-going transfer we need to clean this up as  */
      /* well.                                                          */
      if(EntryToCleanup->DataBufferSent)
         BTPS_FreeMemory(EntryToCleanup->DataBuffer);

      /* Flag the resources that were freed (above) as not being        */
      /* present.                                                       */
      EntryToCleanup->CurrentOperation  = coNone;
      EntryToCleanup->ConnectionEvent   = NULL;
      EntryToCleanup->DataBuffer        = NULL;
      EntryToCleanup->ConnectionTimerID = 0;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* convert the specified string from UTF-8 to Unicode.  This function*/
   /* allocates an array of Word_t's to hold the string and returns the */
   /* string (if successful).  If un-successful this function returns   */
   /* NULL.  If this function is successful (i.e. returns non-NULL, it  */
   /* is the callers responsiblity to to free the memory allocated using*/
   /* the BTPS_FreeMemory() function.                                   */
   /* * NOTE * The returned string is NULL terminated (i.e. ends with   */
   /*          0x0000).                                                 */
static Word_t *ConvertUTF8ToUnicode(char *UTFString)
{
   Word_t       *ret_val;
   unsigned int  StringLength;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((UTFString) && ((StringLength = (BTPS_StringLength(UTFString) + 1)) != 1))
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Converting: %s\n", UTFString));

      if((ret_val = (Word_t *)BTPS_AllocateMemory(StringLength*sizeof(Word_t))) != NULL)
      {
         /* Build the Unicode string.                                   */
         /* * NOTE * We will currently just convert the string directly */
         /*          from ASCII to Unicode (as ASCII).                  */
         while(StringLength--)
            ret_val[StringLength] = (Word_t)UTFString[StringLength];
      }
   }
   else
      ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the specified string from Unicode to UTF-8.  This function*/
   /* allocates an array of char's to hold the string and returns the   */
   /* string (if successful).  If un-successful this function returns   */
   /* NULL.  If this function is successful (i.e. returns non-NULL, it  */
   /* is the callers responsiblity to to free the memory allocated using*/
   /* the BTPS_FreeMemory() function.                                   */
   /* * NOTE * The returned string is NULL terminated (i.e.  ends with  */
   /*          '\0').                                                   */
static char *ConvertUnicodeToUTF8(Word_t *UnicodeString)
{
   char         *ret_val;
   unsigned int  StringLength;
   unsigned int  Index;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(UnicodeString)
   {
      /* Determine the string length.                                   */
      StringLength = 0;
      while(UnicodeString[StringLength])
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unicode String: 0x%04X (%c)\n", UnicodeString[StringLength], (char)(UnicodeString[StringLength])));

         StringLength++;
      }

      if(StringLength)
      {
         if((ret_val = (char *)BTPS_AllocateMemory(StringLength+1)) != NULL)
         {
            /* Build the UTF-8 string.                                  */
            /* * NOTE * We will currently just convert the string       */
            /*          directly from Unicode to ASCII (as ASCII).      */
            for(Index=0;Index<StringLength;Index++)
               ret_val[Index] = (char)(UnicodeString[Index]);

            ret_val[Index] = '\0';

            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Converted: %s\n", ret_val));
         }
      }
      else
         ret_val = NULL;
   }
   else
      ret_val = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the Bluetopia PM Response Status Codes to the correct OBEX*/
   /* Response Code.  This function returns TRUE if the mapping was able*/
   /* to made successfully or FALSE if there was invalid parameter.     */
static Boolean_t MapResponseStatusCodeToResponseCode(unsigned int ResponseStatusCode, Byte_t *ResponseCode)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d \n", ResponseStatusCode));

   /* Check to make sure that we have a buffer to store the result into.*/
   if(ResponseCode)
   {
      /* Initialize success.                                            */
      ret_val = TRUE;

      /* Next, map the Status to the correct code.                      */
      switch(ResponseStatusCode)
      {
         case OPPM_RESPONSE_STATUS_CODE_SUCCESS:
            *ResponseCode = OPP_OBEX_RESPONSE_OK;
            break;
         case OPPM_RESPONSE_STATUS_CODE_NOT_FOUND:
            *ResponseCode = OPP_OBEX_RESPONSE_NOT_FOUND;
            break;
         case OPPM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE:
            *ResponseCode = OPP_OBEX_RESPONSE_SERVICE_UNAVAILABLE;
            break;
         case OPPM_RESPONSE_STATUS_CODE_BAD_REQUEST:
            *ResponseCode = OPP_OBEX_RESPONSE_BAD_REQUEST;
            break;
         case OPPM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED:
            *ResponseCode = OPP_OBEX_RESPONSE_NOT_IMPLEMENTED;
            break;
         case OPPM_RESPONSE_STATUS_CODE_UNAUTHORIZED:
            *ResponseCode = OPP_OBEX_RESPONSE_UNAUTHORIZED;
            break;
         case OPPM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED:
            *ResponseCode = OPP_OBEX_RESPONSE_PRECONDITION_FAILED;
            break;
         case OPPM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE:
            *ResponseCode = OPP_OBEX_RESPONSE_NOT_ACCEPTABLE;
            break;
         case OPPM_RESPONSE_STATUS_CODE_FORBIDDEN:
            *ResponseCode = OPP_OBEX_RESPONSE_FORBIDDEN;
            break;
         case OPPM_RESPONSE_STATUS_CODE_SERVER_ERROR:
            *ResponseCode = OPP_OBEX_RESPONSE_SERVER_ERROR;
            break;
         case OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED:
         case OPPM_RESPONSE_STATUS_CODE_DEVICE_POWER_OFF:
         case OPPM_RESPONSE_STATUS_CODE_UNKNOWN:
         default:
            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert the OBEX Response to the correct Bluetopia PM Response    */
   /* Status Codes.                                                     */
static unsigned int MapResponseCodeToResponseStatusCode(Byte_t ResponseCode)
{
   unsigned int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d (0x%02X) \n", (int)ResponseCode, (int)ResponseCode));

   /* Next, map the OBEX Response to the correct Status Code.           */
   switch(ResponseCode)
   {
      case OPP_OBEX_RESPONSE_OK:
      case OPP_OBEX_RESPONSE_CONTINUE:
         ret_val = OPPM_RESPONSE_STATUS_CODE_SUCCESS;
         break;
      case OPP_OBEX_RESPONSE_NOT_FOUND:
         ret_val = OPPM_RESPONSE_STATUS_CODE_NOT_FOUND;
         break;
      case OPP_OBEX_RESPONSE_SERVICE_UNAVAILABLE:
         ret_val = OPPM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE;
         break;
      case OPP_OBEX_RESPONSE_BAD_REQUEST:
         ret_val = OPPM_RESPONSE_STATUS_CODE_BAD_REQUEST;
         break;
      case OPP_OBEX_RESPONSE_NOT_IMPLEMENTED:
         ret_val = OPPM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED;
         break;
      case OPP_OBEX_RESPONSE_UNAUTHORIZED:
         ret_val = OPPM_RESPONSE_STATUS_CODE_UNAUTHORIZED;
         break;
      case OPP_OBEX_RESPONSE_PRECONDITION_FAILED:
         ret_val = OPPM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED;
         break;
      case OPP_OBEX_RESPONSE_NOT_ACCEPTABLE:
         ret_val = OPPM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE;
         break;
      case OPP_OBEX_RESPONSE_FORBIDDEN:
         ret_val = OPPM_RESPONSE_STATUS_CODE_FORBIDDEN;
         break;
      case OPP_OBEX_RESPONSE_SERVER_ERROR:
         ret_val = OPPM_RESPONSE_STATUS_CODE_SERVER_ERROR;
         break;
      default:
         ret_val = OPPM_RESPONSE_STATUS_CODE_UNKNOWN;
         break;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* actually format and send an Abort Request to the remote device    */
   /* specified by the Object Push Manager Entry.                       */
static void IssuePendingAbort(OPPM_Entry_Info_t *OPPMEntryInfo)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure the input the parameters appear to be semi-valid.*/
   if((OPPMEntryInfo) && (OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
   {
      /* Simply issue the Abort.                                        */
      _OPPM_Abort_Request(OPPMEntryInfo->OPPID);

      /* Clear the Pending Abort flag.                                  */
      OPPMEntryInfo->Flags            &= ~((unsigned long)OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT);

      /* Clear any current operation.                                   */
      OPPMEntryInfo->CurrentOperation  = coNone;

      /* Finally, clear any queued data (if present).                   */
      if(OPPMEntryInfo->DataBuffer)
      {
         BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

         OPPMEntryInfo->DataBuffer = NULL;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("No OPPM Entry or no Pending Abort queued.\n"));

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function to process an Open   */
   /* Request Response.                                                 */
static int ProcessConnectionRequestResponse(unsigned int ClientID, unsigned int TrackingID, Boolean_t Accept)
{
   int                ret_val;
   Boolean_t          Encrypt;
   Boolean_t          Authenticate;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         if(OPPMEntryInfo->ConnectionState == csAuthorizing)
         {
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Accept));

            /* If the client has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Accept)
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     ret_val = DEVM_EncryptRemoteDevice(OPPMEntryInfo->RemoteDeviceAddress, 0);
                  else
                     ret_val = DEVM_AuthenticateRemoteDevice(OPPMEntryInfo->RemoteDeviceAddress, 0);
               }
               else
                  ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Authorization not required, and we are already in  */
                  /* the correct state.                                 */
                  if((ret_val = _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, TRUE)) != 0)
                  {
                     /* Failure, go ahead and try to disconnect it (will*/
                     /* probably fail as well).                         */
                     OPPMEntryInfo->ConnectionState = csIdle;

                     _OPPM_Close_Connection(OPPMEntryInfo->OPPID);
                  }
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!ret_val)
                  {
                     if(Encrypt)
                        OPPMEntryInfo->ConnectionState = csEncrypting;
                     else
                        OPPMEntryInfo->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     ret_val = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     if(_OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE))
                     {
                        /* Failure, go ahead and try to disconnect it   */
                        /* (will probably fail as well).                */
                        OPPMEntryInfo->ConnectionState = csIdle;

                        _OPPM_Close_Connection(OPPMEntryInfo->OPPID);
                     }
                  }
               }
            }
            else
            {
               /* Rejection - Simply respond to the request.            */
               _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE);

               OPPMEntryInfo->ConnectionState = csIdle;

               _OPPM_Close_Connection(OPPMEntryInfo->OPPID);

               /* Flag success.                                         */
               ret_val = 0;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a Register*/
   /* Server request.                                                   */
static int ProcessRegisterServer(unsigned int ClientID, unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val = 0;
   DWord_t            ServiceRecordHandle;
   Boolean_t          ServerPresent;
   OPPM_Entry_Info_t  OPPMEntryInfo;
   OPPM_Entry_Info_t *OPPMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check whether a server port is supplied or if we need to assign   */
   /* one.                                                              */
   if(!ServerPort)
   {
      /* Assign an open server port.                                    */
      if((ret_val = SPPM_FindFreeServerPort()) > 0)
      {
         ServerPort = ret_val;
         ret_val    = 0;
      }
   }

   if(!ret_val)
   {
      /* Verify that the supplied port is valid.                        */
      if((ServerPort >= OPP_PORT_NUMBER_MINIMUM) && (ServerPort <= OPP_PORT_NUMBER_MAXIMUM) && (!SPPM_QueryServerPresent(ServerPort, &ServerPresent)) && (!ServerPresent))
      {
         BTPS_MemInitialize(&OPPMEntryInfo, 0, sizeof(OPPM_Entry_Info_t));

         OPPMEntryInfo.TrackingID           = GetNextTrackingID();
         OPPMEntryInfo.ClientID             = ClientID;
         OPPMEntryInfo.Flags                = OPPM_ENTRY_INFO_FLAGS_SERVER;
         OPPMEntryInfo.ConnectionState      = csIdle;
         OPPMEntryInfo.ConnectionFlags      = IncomingConnectionFlags;
         OPPMEntryInfo.CurrentOperation     = coNone;
         OPPMEntryInfo.PortNumber           = ServerPort;
         OPPMEntryInfo.SupportedObjectTypes = SupportedObjectTypes;
         OPPMEntryInfo.CallbackFunction     = CallbackFunction;
         OPPMEntryInfo.CallbackParameter    = CallbackParameter;

         if((OPPMEntryInfoPtr = AddOPPMEntryInfoEntry(&OPPMEntryInfoList, &OPPMEntryInfo)) != NULL)
         {
            if((ret_val = _OPPM_Open_Object_Push_Server(ServerPort)) > 0)
            {
               /* Note the returned OPP ID.                             */
               OPPMEntryInfoPtr->OPPID = (unsigned int)ret_val;

               /* Now try to register the service record for this port. */
               if(!(ret_val = _OPPM_Register_Object_Push_Server_SDP_Record(OPPMEntryInfoPtr->OPPID, ServiceName, (DWord_t)SupportedObjectTypes, &ServiceRecordHandle)))
               {
                  /* Note the record handle.                            */
                  OPPMEntryInfoPtr->ServiceRecordHandle = ServiceRecordHandle;

                  /* Return the server tracking ID to the caller.       */
                  ret_val = OPPMEntryInfoPtr->TrackingID;
               }
               else
               {
                  /* Service record failed to register. We need to clean*/
                  /* up the port.                                       */
                  _OPPM_Close_Server(OPPMEntryInfoPtr->OPPID);
               }
            }

            if(ret_val < 0)
            {
               /* An error occurred, go ahead and delete the entry that */
               /* was added.                                            */
               if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfoPtr->TrackingID)) != NULL)
               {
                  CleanupOPPMEntryInfo(OPPMEntryInfoPtr);

                  FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process an Un     */
   /* Register Server request.                                          */
static int ProcessUnRegisterServer(unsigned int ClientID, unsigned int TrackingID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         /* Delete the OPP Server Entry from the OPP Entry List.        */
         if((OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID)) != NULL)
         {
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Delete Server: %d\n", TrackingID));

            /* Reject any incoming connection that might be in progress.*/
            if((OPPMEntryInfo->ConnectionState == csAuthenticating) || (OPPMEntryInfo->ConnectionState == csAuthorizing) || (OPPMEntryInfo->ConnectionState == csEncrypting))
               _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE);

            /* If there was a Service Record Registered, go ahead and   */
            /* make sure it is freed.                                   */
            if(OPPMEntryInfo->ServiceRecordHandle)
               _OPPM_Un_Register_SDP_Record(OPPMEntryInfo->OPPID, OPPMEntryInfo->ServiceRecordHandle);

            /* Next, go ahead and Un-Register the Server.               */
            _OPPM_Close_Server(OPPMEntryInfo->OPPID);

            /* Clean up any resources that were allocated for this      */
            /* entry.                                                   */
            CleanupOPPMEntryInfo(OPPMEntryInfo);

            /* All finished, free any memory that was allocated for the */
            /* server.                                                  */
            FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
            ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a Connect */
   /* Remote Device request.                                            */
static int ProcessConnectRemoteDevice(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                ret_val;
   Event_t            ConnectionEvent;
   unsigned int       TrackingID;
   OPPM_Entry_Info_t  OPPMEntryInfo;
   OPPM_Entry_Info_t *OPPMEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize success.                                               */
   ret_val = 0;

   /* Next, make sure that we do not already have a connection to the   */
   /* specified device.                                                 */
   if((OPPMEntryInfoPtr = SearchOPPMEntryInfoByConnection(&OPPMEntryInfoList, RemoteDeviceAddress, FALSE)) == NULL)
   {
      /* Entry is not present, go ahead and create a new entry.         */
      BTPS_MemInitialize(&OPPMEntryInfo, 0, sizeof(OPPM_Entry_Info_t));

      OPPMEntryInfo.TrackingID          = GetNextTrackingID();
      OPPMEntryInfo.ClientID            = ClientID;
      OPPMEntryInfo.RemoteDeviceAddress = RemoteDeviceAddress;
      OPPMEntryInfo.PortNumber          = RemoteServerPort;
      OPPMEntryInfo.ConnectionState     = csIdle;
      OPPMEntryInfo.ConnectionFlags     = ConnectionFlags;
      OPPMEntryInfo.CurrentOperation    = coNone;
      OPPMEntryInfo.CallbackFunction    = CallbackFunction;
      OPPMEntryInfo.CallbackParameter   = CallbackParameter;

      if((OPPMEntryInfoPtr = AddOPPMEntryInfoEntry(&OPPMEntryInfoList, &OPPMEntryInfo)) != NULL)
      {
         if(ConnectionStatus)
            OPPMEntryInfoPtr->ConnectionEvent = BTPS_CreateEvent(FALSE);

         if((!ConnectionStatus) || ((ConnectionStatus) && (OPPMEntryInfoPtr->ConnectionEvent)))
         {
            /* First, let's wait for the Port to disconnect.            */
            if(!SPPM_WaitForPortDisconnection(RemoteServerPort, FALSE, RemoteDeviceAddress, MAXIMUM_OPP_PORT_DELAY_TIMEOUT_MS))
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

               /* Next, attempt to open the remote device.              */
               if(ConnectionFlags & OPPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_ENCRYPTION)
                  OPPMEntryInfoPtr->ConnectionState = csEncrypting;
               else
               {
                  if(ConnectionFlags & OPPM_CONNECT_REMOTE_DEVICE_FLAGS_REQUIRE_AUTHENTICATION)
                     OPPMEntryInfoPtr->ConnectionState = csAuthenticating;
                  else
                     OPPMEntryInfoPtr->ConnectionState = csConnectingDevice;
               }

               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

               ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (OPPMEntryInfoPtr->ConnectionState == csConnectingDevice)?0:((OPPMEntryInfoPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

               if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Check to see if we need to actually issue the      */
                  /* Remote connection.                                 */
                  if(ret_val < 0)
                  {
                     /* Set the state to connecting remote device.      */
                     OPPMEntryInfoPtr->ConnectionState = csConnecting;

                     ret_val = _OPPM_Open_Remote_Object_Push_Server(RemoteDeviceAddress, RemoteServerPort);

                     if(ret_val < 0)
                     {
                        ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE;

                        /* Error opening device, go ahead and delete the*/
                        /* entry that was added.                        */
                        if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfoPtr->TrackingID)) != NULL)
                        {
                           CleanupOPPMEntryInfo(OPPMEntryInfoPtr);

                           FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
                        }
                     }
                     else
                     {
                        /* Note the OPP ID.                             */
                        OPPMEntryInfoPtr->OPPID = (unsigned int)ret_val;

                        /* Flag success.                                */
                        ret_val                = 0;
                     }
                  }
               }
            }
            else
            {
               /* Move the state to the connecting Waiting state.       */
               OPPMEntryInfoPtr->ConnectionState = csConnectingWaiting;

               if((BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS) && (MAXIMUM_OPP_PORT_OPEN_DELAY_RETRY))
               {
                  /* Port is NOT disconnected, go ahead and start a     */
                  /* timer so that we can continue to check for the Port*/
                  /* Disconnection.                                     */
                  ret_val = TMR_StartTimer((void *)OPPMEntryInfoPtr->TrackingID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                  /* If the timer was started, go ahead and note the    */
                  /* Timer ID.                                          */
                  if(ret_val > 0)
                  {
                     OPPMEntryInfoPtr->ConnectionTimerID = (unsigned int)ret_val;

                     ret_val                             = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
               }
               else
                  ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_ALREADY_CONNECTED;
            }

            /* Next, determine if the caller has requested a blocking   */
            /* open.                                                    */
            if((!ret_val) && (ConnectionStatus))
            {
               /* Blocking open, go ahead and wait for the event.       */

               /* Note the Tracking ID.                                 */
               TrackingID      = OPPMEntryInfoPtr->TrackingID;

               /* Note the Open Event.                                  */
               ConnectionEvent = OPPMEntryInfoPtr->ConnectionEvent;

               /* Release the lock because we are finished with it.     */
               DEVM_ReleaseLock();

               BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

               /* Re-acquire the Lock.                                  */
               if(DEVM_AcquireLock())
               {
                  if((OPPMEntryInfoPtr = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
                  {
                     /* Note the connection status.                     */
                     *ConnectionStatus = OPPMEntryInfoPtr->ConnectionStatus;

                     BTPS_CloseEvent(OPPMEntryInfoPtr->ConnectionEvent);

                     /* Flag that the Connection Event is no longer     */
                     /* valid.                                          */
                     OPPMEntryInfoPtr->ConnectionEvent = NULL;

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
            }
            else
            {
               /* If there was an error, go ahead and delete the entry  */
               /* that was added.                                       */
               if(ret_val)
               {
                  if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo.TrackingID)) != NULL)
                  {
                     CleanupOPPMEntryInfo(OPPMEntryInfoPtr);

                     FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
                  }
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;

         /* If an error occurred, go ahead and delete the Connection    */
         /* Information that was added.                                 */
         if(ret_val)
         {
            if((OPPMEntryInfoPtr = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo.TrackingID)) != NULL)
            {
               CleanupOPPMEntryInfo(OPPMEntryInfoPtr);

               FreeOPPMEntryInfoEntryMemory(OPPMEntryInfoPtr);
            }
         }
         else
            ret_val = OPPMEntryInfoPtr->TrackingID;
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
   }
   else
   {
      if(OPPMEntryInfoPtr->ConnectionState == csConnected)
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_ALREADY_CONNECTED;
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_CONNECTION_IN_PROGRESS;
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a         */
   /* Disconnect request.                                               */
static int ProcessDisconnect(unsigned int ClientID, unsigned int TrackingID)
{
   int                ret_val;
   BD_ADDR_t          RemoteDeviceAddress;
   Boolean_t          PerformDisconnect;
   Boolean_t          Server;
   unsigned int       ServerPort;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Close Connection"));

         /* Determine the type of connection.                           */
         Server = (OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER)?TRUE:FALSE;

         /* Next, go ahead and close the connection.                    */
         if((Server) && (OPPMEntryInfo->ConnectionState == csIdle))
            ret_val = 0;
         else
         {
            if(Server)
            {
               CleanupOPPMEntryInfo(OPPMEntryInfo);

               OPPMEntryInfo->ConnectionState = csIdle;

               ret_val = _OPPM_Close_Connection(OPPMEntryInfo->OPPID);
            }
            else
            {
               switch(OPPMEntryInfo->ConnectionState)
               {
                  case csAuthorizing:
                  case csAuthenticating:
                  case csEncrypting:
                     /* Should not occur.                               */
                     PerformDisconnect = FALSE;
                     break;
                  case csConnectingWaiting:
                     if(OPPMEntryInfo->ConnectionTimerID)
                        TMR_StopTimer(OPPMEntryInfo->ConnectionTimerID);

                     PerformDisconnect = FALSE;
                     break;
                  case csConnectingDevice:
                     PerformDisconnect = FALSE;
                     break;
                  default:
                  case csConnecting:
                  case csConnected:
                     PerformDisconnect = TRUE;
                     break;
               }

               if(PerformDisconnect)
               {
                  /* Nothing really to do other than to disconnect the  */
                  /* device.                                            */
                  ret_val = _OPPM_Close_Connection(OPPMEntryInfo->OPPID);
               }
               else
                  ret_val = 0;

               /* If this is a client, we need to go ahead and delete   */
               /* the entry.                                            */
               if(!ret_val)
               {
                  /* Note the Port Number before we delete the OPP Entry*/
                  /* (we will use it below after we free the entry).    */
                  ServerPort          = OPPMEntryInfo->PortNumber;
                  RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;

                  if((OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID)) != NULL)
                  {
                     /* All finished, free any memory that was allocated*/
                     /* for the server.                                 */
                     CleanupOPPMEntryInfo(OPPMEntryInfo);

                     FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);
                  }

                  /* Go ahead and give the port some time to disconnect */
                  /* (since it was initiated locally).                  */
                  SPPM_WaitForPortDisconnection(ServerPort, FALSE, RemoteDeviceAddress, MAXIMUM_OPP_PORT_DELAY_TIMEOUT_MS);
               }
            }
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process an abort  */
   /* request.                                                          */
static int ProcessAbort(unsigned int ClientID, unsigned int TrackingID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         /* Entry found, make sure that there is an on-going operation. */
         if((OPPMEntryInfo->CurrentOperation != coNone) && (!(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)))
         {
            /* Operation in progress, go ahead and send the Abort.      */
            if((ret_val = _OPPM_Abort_Request(OPPMEntryInfo->OPPID)) == 0)
               OPPMEntryInfo->Flags |= OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT;
         }
         else
         {
            if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_ABORT_OPERATION_IN_PROGRESS;
            else
               ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NO_OPERATION_IN_PROGRESS;
         }
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a Push    */
   /* Object Request.                                                   */
static int ProcessPushObjectRequest(unsigned int ClientID, unsigned int TrackingID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                ret_val;
   Word_t            *ObjectNameUnicode;
   OPP_Object_Type_t  OPPObjectType;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         /* Entry found, make sure that there is NO on-going operation. */
         if((OPPMEntryInfo->CurrentOperation == coNone) || (OPPMEntryInfo->CurrentOperation == coPushObject))
         {
            /* No operation in progress, go ahead and convert the Object*/
            /* Name from UTF-8 to Unicode.                              */

            /* Initialize success.                                      */
            ret_val = 0;

            if(!BTPS_StringLength(ObjectName))
               ObjectNameUnicode = NULL;
            else
            {
               if((ObjectNameUnicode = ConvertUTF8ToUnicode(ObjectName)) == NULL)
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }

            /* Object name converted. Go ahead and attempt to push the  */
            /* object.                                                  */
            if(!ret_val)
            {
               /* Determine if we need to back up the data we are       */
               /* sending.                                              */
               if((OPPMEntryInfo->DataBufferSize = DataLength) != 0)
               {
                  /* Free any current data we have buffered (should be  */
                  /* none).                                             */
                  if(OPPMEntryInfo->DataBuffer)
                  {
                     BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                     OPPMEntryInfo->DataBuffer = NULL;
                  }

                  /* Go ahead and allocate the buffer (we will not copy */
                  /* it yet, but we will allocate it so that we don't   */
                  /* get an error *AFTER* we send the first part of the */
                  /* data.                                              */
                  if((OPPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
               }

               /* Flag that we have not sent any data at this point.    */
               OPPMEntryInfo->DataBufferSent = 0;

               switch(ObjectType)
               {
                  case obtvCard:
                     OPPObjectType = oppvCard;
                     break;
                  case obtvCalendar:
                     OPPObjectType = oppvCalendar;
                     break;
                  case obtiCalendar:
                     OPPObjectType = oppiCalendar;
                     break;
                  case obtvNote:
                     OPPObjectType = oppvNote;
                     break;
                  case obtvMessage:
                     OPPObjectType = oppvMessage;
                     break;
                  case obtUnknownObject:
                  default:
                     OPPObjectType = oppUnknownObject;
                     break;
               }

               if(!ret_val)
               {
                  if((ret_val = _OPPM_Push_Object_Request(OPPMEntryInfo->OPPID, OPPObjectType, ObjectNameUnicode, ObjectTotalLength, DataLength, DataBuffer, &OPPMEntryInfo->DataBufferSent, Final)) == 0)
                  {
                     /* Flag that a Push Object Operation is in         */
                     /* progress.                                       */
                     OPPMEntryInfo->CurrentOperation = coPushObject;

                     /* Copy any remaining data into the buffer for     */
                     /* future operations.                              */
                     if(OPPMEntryInfo->DataBufferSent != DataLength)
                        BTPS_MemCopy(OPPMEntryInfo->DataBuffer, DataBuffer, DataLength);

                     OPPMEntryInfo->DataFinal = Final;
                  }

                  /* If there was an error or we sent all of the data,  */
                  /* then we need to free any buffer that was allocated.*/
                  if((ret_val) || (OPPMEntryInfo->DataBufferSent == DataLength))
                  {
                     if(OPPMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                        OPPMEntryInfo->DataBuffer = NULL;
                     }
                  }
               }
            }

            /* Free any memory that was allocated for the Folder Name.  */
            if(ObjectNameUnicode)
               BTPS_FreeMemory(ObjectNameUnicode);
         }
         else
            ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a Push    */
   /* Object Response.                                                  */
static int ProcessPushObjectResponse(unsigned int ClientID, unsigned int TrackingID, unsigned int ResponseCode)
{
   int                ret_val;
   Byte_t             OBEXResponseCode;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         /* Entry found, make sure that there is the correct on-going   */
         /* operation.                                                  */
         if(OPPMEntryInfo->CurrentOperation == coPushObject)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseCode, &OBEXResponseCode))
            {
               /* Operation in progress, go ahead and attempt to submit */
               /* the Send Push Object Response command.                */
               if((ret_val = _OPPM_Push_Object_Response(OPPMEntryInfo->OPPID, OBEXResponseCode)) == 0)
                  OPPMEntryInfo->CurrentOperation = coNone;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a Pull    */
   /* Business Card Request.                                            */
static int ProcessPullBusinessCardRequest(unsigned int ClientID, unsigned int TrackingID)
{
   int                ret_val;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         /* Entry found, make sure that there is NO on-going operation. */
         if(OPPMEntryInfo->CurrentOperation == coNone)
         {
            /* No operation in progress, go ahead and attempt to submit */
            /* the Pull Business Card command.                          */
            if((ret_val = _OPPM_Pull_Business_Card_Request(OPPMEntryInfo->OPPID)) == 0)
               OPPMEntryInfo->CurrentOperation = coPullBusinessCard;
         }
         else
            ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function to process a Pull    */
   /* Business Card Response.                                           */
static int ProcessPullBusinessCardResponse(unsigned int ClientID, unsigned int TrackingID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int                ret_val;
   Byte_t             OBEXResponseCode;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID)) != NULL)
   {
      if(ClientID == OPPMEntryInfo->ClientID)
      {
         /* Entry found, make sure that the correct on-going operation  */
         /* is in progress.                                             */
         if(OPPMEntryInfo->CurrentOperation == coPullBusinessCard)
         {
            if(MapResponseStatusCodeToResponseCode(ResponseCode, &OBEXResponseCode))
            {
               /* Determine if we need to back up the data we are       */
               /* sending.                                              */
               /* * NOTE * There is no reason to worry about sending    */
               /*          any data (or backing any data up if this is  */
               /*          not a successful response).                  */
               if(ResponseCode == OPPM_RESPONSE_STATUS_CODE_SUCCESS)
               {
                  /* Check to see if we need to OPP the final bit into a*/
                  /* continue (the lack of it).                         */
                  /* * NOTE * This is required because there is No Final*/
                  /*          flag for responses (it is inherant with   */
                  /*          either an OK or or CONTINUE being sent as */
                  /*          the code).                                */
                  if(!Final)
                     OBEXResponseCode = OPP_OBEX_RESPONSE_CONTINUE;

                  if((OPPMEntryInfo->DataBufferSize = DataLength) != 0)
                  {
                     /* Free any current data we have buffered (should  */
                     /* be none).                                       */
                     if(OPPMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                        OPPMEntryInfo->DataBuffer = NULL;
                     }

                     /* Go ahead and allocate the buffer (we will not   */
                     /* copy it yet, but we will allocate it so that we */
                     /* don't get an error *AFTER* we send the first    */
                     /* part of the data.                               */
                     if((OPPMEntryInfo->DataBuffer = (Byte_t *)BTPS_AllocateMemory(DataLength)) == NULL)
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
                     else
                        ret_val = 0;
                  }
                  else
                     ret_val = 0;

                  /* Flag that we have not sent any data at this point. */
                  OPPMEntryInfo->DataBufferSent = 0;
               }
               else
               {
                  /* There is no reason to send any data because this is*/
                  /* an error response.                                 */
                  DataLength = 0;

                  ret_val    = 0;
               }

               if(!ret_val)
               {
                  if((ret_val = _OPPM_Pull_Business_Card_Response(OPPMEntryInfo->OPPID, OBEXResponseCode, ObjectTotalLength, DataLength, DataLength?DataBuffer:NULL, &(OPPMEntryInfo->DataBufferSent))) == 0)
                  {
                     /* Copy any remaining data into the buffer for     */
                     /* future operations.                              */
                     if((ResponseCode == OPPM_RESPONSE_STATUS_CODE_SUCCESS) && (OPPMEntryInfo->DataBufferSent != DataLength))
                     {
                        BTPS_MemCopy(OPPMEntryInfo->DataBuffer, DataBuffer, DataLength);

                        OPPMEntryInfo->DataFinal = Final;
                     }
                     else
                        OPPMEntryInfo->CurrentOperation = coNone;
                  }

                  /* If there was an error or we sent all of the data,  */
                  /* then we need to free any buffer that was allocated.*/
                  if((ret_val) || ((ResponseCode == OPPM_RESPONSE_STATUS_CODE_SUCCESS) && (OPPMEntryInfo->DataBufferSent == DataLength)))
                  {
                     if(OPPMEntryInfo->DataBuffer)
                     {
                        BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                        OPPMEntryInfo->DataBuffer = NULL;
                     }
                  }
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION;
      }
      else
         ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function processes the specified Object Push        */
   /* connection request response message and responds to the message   */
   /* accordingly.  This function does not verify the integrity         */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessConnectionRequestResponseMessage(OPPM_Connection_Request_Response_Request_t *Message)
{
   int                                         Result;
   OPPM_Connection_Request_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", Message->MessageHeader.AddressID));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessConnectionRequestResponse(Message->MessageHeader.AddressID, Message->ServerID, Message->Accept);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push        */
   /* register server message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessRegisterServerMessage(OPPM_Register_Server_Request_t *Message)
{
   int                             Result;
   OPPM_Register_Server_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", Message->MessageHeader.AddressID));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessRegisterServer(Message->MessageHeader.AddressID, Message->ServerPort, Message->SupportedObjectTypes, Message->IncomingConnectionFlags, Message->ServiceName, NULL, NULL);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_REGISTER_SERVER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push        */
   /* unregister server message and responds to the message accordingly.*/
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessUnRegisterServerMessage(OPPM_Un_Register_Server_Request_t *Message)
{
   int                                Result;
   OPPM_Un_Register_Server_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessUnRegisterServer(Message->MessageHeader.AddressID, Message->ServerID);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_UN_REGISTER_SERVER_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push connect*/
   /* remote device message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessConnectRemoteDeviceMessage(OPPM_Connect_Remote_Device_Request_t *Message)
{
   int                                    Result;
   OPPM_Connect_Remote_Device_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessConnectRemoteDevice(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->RemoteServerPort, Message->ConnectionFlags, NULL, NULL, NULL);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push        */
   /* disconnect message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* message before calling this function.                             */
static void ProcessDisconnectMessage(OPPM_Disconnect_Request_t *Message)
{
   int                        Result;
   OPPM_Disconnect_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessDisconnect(Message->MessageHeader.AddressID, Message->PortID);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_DISCONNECT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push abort  */
   /* message and responds to the message accordingly.  This function   */
   /* does not verify the integrity of the message (i.e. the length)    */
   /* because it is the caller's responsibility to verify the message   */
   /* before calling this function.                                     */
static void ProcessAbortMessage(OPPM_Abort_Request_t *Message)
{
   int                   Result;
   OPPM_Abort_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessAbort(Message->MessageHeader.AddressID, Message->ClientID);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_ABORT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push push   */
   /* object request message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessPushObjectRequestMessage(OPPM_Push_Object_Request_Request_t *Message)
{
   int                                  Result;
   char                                *ObjectName;
   Byte_t                              *DataBuffer;
   OPPM_Push_Object_Request_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         /* Note the object name in the data buffer.                    */
         ObjectName = (char *)Message->VariableData;

         /* Make sure that the object name is null-terminated.          */
         ObjectName[Message->ObjectNameLength-1] = '\0';

         /* Note the start of the object data.                          */
         DataBuffer = &(Message->VariableData[Message->ObjectNameLength]);

         /* Go ahead and submit the request.                            */
         Result = ProcessPushObjectRequest(Message->MessageHeader.AddressID, Message->ClientID, Message->ObjectType, ObjectName, Message->ObjectTotalLength, Message->DataLength, DataBuffer, Message->Final);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_PUSH_OBJECT_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push push   */
   /* object response message and responds to the message accordingly.  */
   /* This function does not verify the integrity of the message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the message before calling this function.                         */
static void ProcessPushObjectResponseMessage(OPPM_Push_Object_Response_Request_t *Message)
{
   int                                  Result;
   OPPM_Push_Object_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessPushObjectResponse(Message->MessageHeader.AddressID, Message->ServerID, Message->ResponseCode);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_PUSH_OBJECT_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push        */
   /* pull business card request message and responds to the message    */
   /* accordingly.  This function does not verify the integrity         */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessPullBusinessCardRequestMessage(OPPM_Pull_Business_Card_Request_Request_t *Message)
{
   int                                        Result;
   OPPM_Pull_Business_Card_Request_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessPullBusinessCardRequest(Message->MessageHeader.AddressID, Message->ClientID);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_PULL_BUSINESS_CARD_REQUEST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Object Push pull   */
   /* business card response message and responds to the message        */
   /* accordingly.  This function does not verify the integrity         */
   /* of the message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the message before calling this function.*/
static void ProcessPullBusinessCardResponseMessage(OPPM_Pull_Business_Card_Response_Request_t *Message)
{
   int                                         Result;
   OPPM_Pull_Business_Card_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the message is semi-valid.                            */
   if(Message)
   {
      /* Check to see if we are powered up.                             */
      if(CurrentPowerState)
      {
         Result = ProcessPullBusinessCardResponse(Message->MessageHeader.AddressID, Message->ServerID, Message->ResponseCode, Message->ObjectTotalLength, Message->DataLength, Message->DataBuffer, Message->Final);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = OPPM_PULL_BUSINESS_CARD_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the OPP Manager lock */
   /*          held.  This function will release the lock before it     */
   /*          exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).     */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access connect response request.              */
               ProcessConnectionRequestResponseMessage((OPPM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_REGISTER_SERVER:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Server Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_REGISTER_SERVER_REQUEST_SIZE(((OPPM_Register_Server_Request_t *)Message)->ServiceNameLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access register server request.               */
               ProcessRegisterServerMessage((OPPM_Register_Server_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Server Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_UN_REGISTER_SERVER_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access un-register server request.            */
               ProcessUnRegisterServerMessage((OPPM_Un_Register_Server_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Device Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access connect remote device request.         */
               ProcessConnectRemoteDeviceMessage((OPPM_Connect_Remote_Device_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_DISCONNECT:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_DISCONNECT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access disconnect request.                    */
               ProcessDisconnectMessage((OPPM_Disconnect_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_ABORT:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_ABORT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access abort request.                         */
               ProcessAbortMessage((OPPM_Abort_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Object Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PUSH_OBJECT_REQUEST_REQUEST_SIZE(((OPPM_Push_Object_Request_Request_t *)Message)->ObjectNameLength, ((OPPM_Push_Object_Request_Request_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access PushObjectRequest request.             */
               ProcessPushObjectRequestMessage((OPPM_Push_Object_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_SEND_PUSH_OBJECT_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Object Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PUSH_OBJECT_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access PushObjectRequest request.             */
               ProcessPushObjectResponseMessage((OPPM_Push_Object_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_REQUEST:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Business Card Request Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PULL_BUSINESS_CARD_REQUEST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access PullBusinessCardRequest request.       */
               ProcessPullBusinessCardRequestMessage((OPPM_Pull_Business_Card_Request_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case OPPM_MESSAGE_FUNCTION_SEND_PULL_BUSINESS_CARD_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Business Card Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= OPPM_PULL_BUSINESS_CARD_RESPONSE_REQUEST_SIZE(((OPPM_Pull_Business_Card_Response_Request_t *)Message)->DataLength))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Message Access PullBusinessCardRequest request.       */
               ProcessPullBusinessCardResponseMessage((OPPM_Pull_Business_Card_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   OPPM_Entry_Info_t  *OPPMEntryInfo;
   OPPM_Entry_Info_t  *tmpOPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      /* We need to check for any entries from the un-registered client */
      OPPMEntryInfo = OPPMEntryInfoList;

      /* Loop through the list.                                         */
      while(OPPMEntryInfo)
      {
         /* Check whether this entry belongs to the client.             */
         if(OPPMEntryInfo->ClientID == ClientID)
         {
            /* Note the next OPP Entry in the list, since we are about  */
            /* to delete the current entry.                             */
            tmpOPPMEntryInfo = OPPMEntryInfo->NextOPPMEntryInfoPtr;

            if((OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID)) != NULL)
            {
               /* Check if this is a client or server entry.            */
               if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER)
               {
                  /* Un-register the server's SDP record.               */
                  _OPPM_Un_Register_SDP_Record(OPPMEntryInfo->OPPID, OPPMEntryInfo->ServiceRecordHandle);

                  /* Close the server port.                             */
                  _OPPM_Close_Server(OPPMEntryInfo->OPPID);
               }
               else
               {
                  /* If we have a client connection, close it.          */
                  if(OPPMEntryInfo->OPPID)
                     _OPPM_Close_Connection(OPPMEntryInfo->OPPID);
               }

               /* Cleanup any state information in the entry.           */
               CleanupOPPMEntryInfo(OPPMEntryInfo);

               /* Free the entry.                                       */
               FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);
            }

            /* Move on to the next entry.                               */
            OPPMEntryInfo = tmpOPPMEntryInfo;
         }
         else
            OPPMEntryInfo = OPPMEntryInfo->NextOPPMEntryInfoPtr;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified  Object Push event to the specified client.*/
   /* * NOTE * This function should be called with the Object Push      */
   /*          Manager Lock held.  Upon exit from this function it will */
   /*          free the Object Push Manager Lock.                       */
static void DispatchOPPMEvent(OPPM_Entry_Info_t *OPPMEntryInfo, OPPM_Event_Data_t *EventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   unsigned int           ClientID;
   unsigned int           ServerID;
   OPPM_Event_Callback_t  CallbackFunction;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the the parameters are semi-valid.                         */
   if((OPPMEntryInfo) && (EventData) && (Message))
   {
      /* Note the callback information.                                 */
      ServerID          = MSG_GetServerAddressID();
      ClientID          = OPPMEntryInfo->ClientID;
      CallbackFunction  = OPPMEntryInfo->CallbackFunction;
      CallbackParameter = OPPMEntryInfo->CallbackParameter;

      /* Release the lock to make the callback.                         */
      DEVM_ReleaseLock();

      /* Check whether this is a local callback.                        */
      if(ClientID == ServerID)
      {
         /* Callback is local. Go ahead and make the callback.          */
         __BTPSTRY
         {
            if(CallbackFunction)
            {
               (*CallbackFunction)(EventData, CallbackParameter);
            }
         }
         __BTPSEXCEPT(1)
         {
            /* Do nothing.                                              */
         }
      }
      else
      {
         /* Callback is a remote client.                                */

         /* Sent the ClientID of the Event Message.                     */
         Message->MessageHeader.AddressID = ClientID;

         /* Send the message to the appropriate client.                 */
         MSG_SendMessage(Message);
      }

      /* Re-acquire the lock.                                           */
      DEVM_AcquireLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

}

   /* The following function is responsible for processing an incoming  */
   /* open request indication event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Object Push Manager information held.         */
static void ProcessOpenRequestIndicationEvent(OPP_Open_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   OPPM_Event_Data_t                  OPPMEventData;
   OPPM_Entry_Info_t                 *OPPMEntryInfo;
   OPPM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check that the pointer to the event data is not NULL.             */
   if(OpenRequestIndicationData)
   {
      /* Check to see if we can find the indicated server instance.     */
      /* * NOTE * This entry could be either a OPP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if(((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, OpenRequestIndicationData->OPPID)) != NULL) && (OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER))
      {
         /* Record the address of the remote device.                    */
         OPPMEntryInfo->RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;

         /* Check whether any connection flags are set.                 */
         if(!OPPMEntryInfo->ConnectionFlags)
         {
            /* Simply Accept the connection.                            */
            _OPPM_Open_Request_Response(OpenRequestIndicationData->OPPID, TRUE);
         }
         else
         {
            /* Check if authorization is required.                      */
            if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHORIZATION)
            {
               OPPMEntryInfo->ConnectionState = csAuthorizing;

               /* Go ahead and format the event.                        */
               BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

               /* Format the event data.                                */
               OPPMEventData.EventType                                                        = oetIncomingConnectionRequest;
               OPPMEventData.EventLength                                                      = OPPM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

               OPPMEventData.EventData.IncomingConnectionRequestEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
               OPPMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;

               /* Format the message.                                   */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
               Message.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
               Message.MessageHeader.MessageLength   = (OPPM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;
               Message.ServerID                      = OPPMEntryInfo->TrackingID;

               /* Dispatch the event.                                   */
               DispatchOPPMEvent(OPPMEntryInfo, &OPPMEventData, (BTPM_Message_t *)&Message);
            }
            else
            {
               /* Determine if authentication or encryption is required.*/
               if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(OPPMEntryInfo->RemoteDeviceAddress, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(OPPMEntryInfo->RemoteDeviceAddress, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Accept the connection.                             */
                  Result = _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, TRUE);

                  if(Result)
                     _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE);
                  else
                  {
                     /* Update the current connection state.            */
                     OPPMEntryInfo->ConnectionState = csConnecting;
                  }
               }
               else
               {
                  /* Set the connection state.                          */
                  if(!Result)
                  {
                     if(Encrypt)
                        OPPMEntryInfo->ConnectionState = csEncrypting;
                     else
                        OPPMEntryInfo->ConnectionState = csAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Reject the request.                             */
                     _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE);
                  }
               }
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* open port indication event that has been received with the        */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Object Push Manager information held.         */
static void ProcessOpenPortIndicationEvent(OPP_Open_Port_Indication_Data_t *OpenPortIndicationData)
{
   OPPM_Event_Data_t         OPPMEventData;
   OPPM_Entry_Info_t        *OPPMEntryInfo;
   OPPM_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(OpenPortIndicationData)
   {
      /* Find the list entry by the OPPID.                              */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, OpenPortIndicationData->OPPID)) != NULL)
      {
         /* Flag that this Server Entry is now connected.               */
         OPPMEntryInfo->ConnectionState = csConnected;

         /* Go ahead and format the event.                              */
         BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

         /* Format the event data.                                      */
         OPPMEventData.EventType                                        = oetConnected;
         OPPMEventData.EventLength                                      = OPPM_CONNECTED_EVENT_DATA_SIZE;

         OPPMEventData.EventData.ConnectedEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
         OPPMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;

         /* Format the message.                                         */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
         Message.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_CONNECTED;
         Message.MessageHeader.MessageLength   = (OPPM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
         Message.ServerID                      = OPPMEntryInfo->TrackingID;

         /* Dispatch the event.                                         */
         DispatchOPPMEvent(OPPMEntryInfo, &OPPMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* open port confirmation event that has been received with the      */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Object Push Manager information held.         */
static void ProcessOpenPortConfirmationEvent(OPP_Open_Port_Confirmation_Data_t *OpenPortConfirmationData)
{
   void                             *CallbackParameter;
   unsigned int                      ConnectionStatus;
   OPPM_Event_Data_t                 OPPMEventData;
   OPPM_Entry_Info_t                *OPPMEntryInfo;
   OPPM_Event_Callback_t             EventCallback;
   OPPM_Connection_Status_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(OpenPortConfirmationData)
   {
      /***************************** NOTE *******************************/
      /* * This function can be called internally to clean up failed  * */
      /* * connections (or connections that need to be closed).  It   * */
      /* * is possible that a OPP ID has not been assigned to the     * */
      /* * Entry during some portions of out-going connections.  To   * */
      /* * handle this case and allow the use of this function for    * */
      /* * cleanup, the Tracking ID will be used in place of the OPP  * */
      /* * ID.  This will be signified by the Most Significant bit of * */
      /* * of the OPP ID being set (this ID cannot occur as a normal  * */
      /* * OPP ID so there will be no conflicts).                     * */
      /***************************** NOTE *******************************/
      if(OpenPortConfirmationData->OPPID & 0x80000000)
      {
         OpenPortConfirmationData->OPPID &= 0x7FFFFFFF;

         OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, OpenPortConfirmationData->OPPID);
      }
      else
      {
         /* Find the list entry by the OPPID.                           */
         /* * NOTE * This entry could be either a OPP Server Entry or a */
         /*          Notification Server Entry.                         */
         OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, OpenPortConfirmationData->OPPID);
      }

      if(OPPMEntryInfo)
      {
         /* First, let's map the status to the correct Object Push      */
         /* Manager status.                                             */
         switch(OpenPortConfirmationData->OpenStatus)
         {
            case OPP_OPEN_STATUS_SUCCESS:
               ConnectionStatus = OPPM_CONNECTION_STATUS_SUCCESS;
               break;
            case OPP_OPEN_STATUS_CONNECTION_TIMEOUT:
               ConnectionStatus = OPPM_CONNECTION_STATUS_FAILURE_TIMEOUT;
               break;
            case OPP_OPEN_STATUS_CONNECTION_REFUSED:
               ConnectionStatus = OPPM_CONNECTION_STATUS_FAILURE_REFUSED;
               break;
            case OPP_OPEN_STATUS_UNKNOWN_ERROR:
            default:
               ConnectionStatus = OPPM_CONNECTION_STATUS_FAILURE_UNKNOWN;
               break;
         }

         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Status, %d %d\n", OpenPortConfirmationData->OpenStatus, ConnectionStatus));

         /* Check the connection status.                                */
         if(ConnectionStatus == OPPM_CONNECTION_STATUS_SUCCESS)
            OPPMEntryInfo->ConnectionState = csConnected;
         else
         {
            /* If this was not a synchronous connection, go ahead and   */
            /* delete it from the list (we will free the resources at   */
            /* the end).                                                */
            if(!OPPMEntryInfo->ConnectionEvent)
               OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID);
         }

         if(OPPMEntryInfo)
         {
            /* Dispatch the event based upon the client registration    */
            /* type.                                                    */
            if(OPPMEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Dispatch the event locally.                           */

               /* If this was a synchronous event we need to set the    */
               /* status and the event.                                 */
               if(OPPMEntryInfo->ConnectionEvent)
               {
                  /* Synchronous event, go ahead and set the correct    */
                  /* status, then set the event.                        */
                  OPPMEntryInfo->ConnectionStatus = ConnectionStatus;

                  BTPS_SetEvent(OPPMEntryInfo->ConnectionEvent);
               }
               else
               {
                  /* Event needs to be dispatched.  Go ahead and format */
                  /* the event.                                         */
                  BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

                  /* Format the event data.                             */
                  OPPMEventData.EventType                                               = oetConnectionStatus;
                  OPPMEventData.EventLength                                             = OPPM_CONNECTION_STATUS_EVENT_DATA_SIZE;

                  OPPMEventData.EventData.ConnectionStatusEventData.ClientPortID        = OPPMEntryInfo->TrackingID;
                  OPPMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;
                  OPPMEventData.EventData.ConnectionStatusEventData.Status              = ConnectionStatus;

                  /* Note the Callback information.                     */
                  EventCallback                                                         = OPPMEntryInfo->CallbackFunction;
                  CallbackParameter                                                     = OPPMEntryInfo->CallbackParameter;

                  /* Release the Lock so we can dispatch the event.     */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&OPPMEventData, CallbackParameter);
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
               /* Dispatch the event remotely.                          */

               /* Format the message.                                   */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
               Message.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_CONNECTION_STATUS;
               Message.MessageHeader.MessageLength   = (OPPM_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
               Message.Status                        = ConnectionStatus;
               Message.ClientID                      = OPPMEntryInfo->TrackingID;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&Message);
            }

            /* If the entry was deleted, we need to free any resources  */
            /* that were allocated.                                     */
            if(ConnectionStatus != OPPM_CONNECTION_STATUS_SUCCESS)
            {
               CleanupOPPMEntryInfo(OPPMEntryInfo);

               FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);
            }
         }
      }

   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* close port indication event that has been received with the       */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Object Push Manager information held.         */
static void ProcessClosePortIndicationEvent(OPP_Close_Port_Indication_Data_t *ClosePortIndicationData)
{
   OPPM_Event_Data_t            OPPMEventData;
   OPPM_Entry_Info_t           *OPPMEntryInfo;
   OPPM_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to ensure the event data pointer parameter is not NULL.     */
   if(ClosePortIndicationData)
   {
      /* Search by OPPID.                                               */
      /* Find the list entry by the OPPID.                              */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, ClosePortIndicationData->OPPID)) != NULL)
      {
         /* Set the state to idle.                                      */
         OPPMEntryInfo->ConnectionState = csIdle;

         if((OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER) || ((OPPMEntryInfo = DeleteOPPMEntryInfoEntry(&OPPMEntryInfoList, OPPMEntryInfo->TrackingID)) != NULL))
         {
            /* Entry has been deleted.                                  */

            /* Go ahead and format the event.                           */
            BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

            /* Format the event data.                                   */
            OPPMEventData.EventType                                           = oetDisconnected;
            OPPMEventData.EventLength                                         = OPPM_DISCONNECTED_EVENT_DATA_SIZE;

            OPPMEventData.EventData.DisconnectedEventData.PortID              = OPPMEntryInfo->TrackingID;
            OPPMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            Message.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_DISCONNECTED;
            Message.MessageHeader.MessageLength   = (OPPM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
            Message.Server                        = (OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER)?TRUE:FALSE;
            Message.PortID                        = OPPMEntryInfo->TrackingID;

            /* All finished with the OPP Entry, go ahead and clean it   */
            /* up.                                                      */
            CleanupOPPMEntryInfo(OPPMEntryInfo);

            /* Dispatch the event.                                      */
            /* * NOTE * This function release the lock, so if the entry */
            /*          referenced is still in the list (i.e. it is a   */
            /*          server) it may no longer be valid.              */
            DispatchOPPMEvent(OPPMEntryInfo, &OPPMEventData, (BTPM_Message_t *)&Message);

            /* If this is a Client entry it has been deleted and we     */
            /* should free the list entry memory. For servers, reset    */
            /* the Bluetooth Address.                                   */
            if(!(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER))
               FreeOPPMEntryInfoEntryMemory(OPPMEntryInfo);
            else
               ASSIGN_BD_ADDR(OPPMEntryInfo->RemoteDeviceAddress, 0, 0, 0, 0, 0, 0);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* push object indication event that has been received with the      */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Object Push Manager information held.         */
static void ProcessPushObjectIndicationEvent(OPP_Push_Object_Indication_Data_t *PushObjectIndicationData)
{
   char                               *ObjectName;
   Byte_t                              ResponseCode;
   unsigned int                        ObjectNameLength;
   unsigned int                        TrackingID;
   OPPM_Event_Data_t                   OPPMEventData;
   OPPM_Entry_Info_t                  *OPPMEntryInfo;
   OPPM_Object_Type_t                  ObjectType;
   OPPM_Push_Object_Request_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(PushObjectIndicationData)
   {
      /* Search for the OPPID in the entry list.                        */
      if(((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, PushObjectIndicationData->OPPID)) != NULL) && ((OPPMEntryInfo->CurrentOperation == coNone) || (OPPMEntryInfo->CurrentOperation == coPushObject)))
      {
         /* Go ahead and initialize the response (either OK or a        */
         /* continue).                                                  */
         if(PushObjectIndicationData->Final)
         {
            ResponseCode                    = OPP_OBEX_RESPONSE_OK;
            OPPMEntryInfo->CurrentOperation = coPushObject;
         }
         else
         {
            ResponseCode                    = OPP_OBEX_RESPONSE_CONTINUE;
            OPPMEntryInfo->CurrentOperation = coNone;
         }

         /* Initialize some defaults.                                   */
         ObjectName   = NULL;
         ObjectNameLength = 0;

         /* We need to determine how much space is required for the     */
         /* Folder Name (this is a Unicode string, so we need to convert*/
         /* it to ASCII).                                               */
         if(PushObjectIndicationData->ObjectName)
         {
            if((ObjectName = ConvertUnicodeToUTF8(PushObjectIndicationData->ObjectName)) != NULL)
               ObjectNameLength = BTPS_StringLength(ObjectName) + 1;
            else
               ObjectNameLength = 0;
         }
         else
            ObjectNameLength = 0;

         switch(PushObjectIndicationData->ObjectType)
         {
            case oppvCard:
               ObjectType = obtvCard;
               break;
            case oppvCalendar:
               ObjectType = obtvCalendar;
               break;
            case oppiCalendar:
               ObjectType = obtiCalendar;
               break;
            case oppvNote:
               ObjectType = obtvNote;
               break;
            case oppvMessage:
               ObjectType = obtvMessage;
               break;
            case oppUnknownObject:
            default:
               ObjectType = obtUnknownObject;
               break;
         }

         /* Event needs to be dispatched.  Go ahead and format the      */
         /* event.                                                      */
         BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

         /* Format the event data.                                      */
         OPPMEventData.EventType                                                = oetPushObjectRequest;
         OPPMEventData.EventLength                                              = OPPM_PUSH_OBJECT_REQUEST_EVENT_DATA_SIZE;

         OPPMEventData.EventData.PushObjectRequestEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
         OPPMEventData.EventData.PushObjectRequestEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;
         OPPMEventData.EventData.PushObjectRequestEventData.ObjectType          = ObjectType;
         OPPMEventData.EventData.PushObjectRequestEventData.ObjectTotalLength   = PushObjectIndicationData->ObjectTotalLength;
         OPPMEventData.EventData.PushObjectRequestEventData.Final               = PushObjectIndicationData->Final;
         OPPMEventData.EventData.PushObjectRequestEventData.ObjectName          = ObjectName;

         if((OPPMEventData.EventData.PushObjectRequestEventData.DataLength = PushObjectIndicationData->DataLength) != 0)
            OPPMEventData.EventData.PushObjectRequestEventData.DataBuffer = PushObjectIndicationData->DataBuffer;

         /* Format the message.                                         */

         /* Next, allocate a message of the correct size.               */
         if((Message = (OPPM_Push_Object_Request_Message_t *)BTPS_AllocateMemory(OPPM_PUSH_OBJECT_REQUEST_MESSAGE_SIZE(ObjectNameLength, PushObjectIndicationData->DataLength))) != NULL)
         {
            BTPS_MemInitialize(Message, 0, OPPM_PUSH_OBJECT_REQUEST_MESSAGE_SIZE(ObjectNameLength, PushObjectIndicationData->DataLength));

            Message->MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            Message->MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_REQUEST;
            Message->MessageHeader.MessageLength   = OPPM_PUSH_OBJECT_REQUEST_MESSAGE_SIZE(ObjectNameLength, PushObjectIndicationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

            Message->RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
            Message->Final                         = PushObjectIndicationData->Final;
            Message->ObjectNameLength              = ObjectNameLength;
            Message->ObjectType                    = ObjectType;
            Message->ObjectTotalLength             = PushObjectIndicationData->ObjectTotalLength;
            Message->ServerID                      = OPPMEntryInfo->TrackingID;

            if(ObjectNameLength)
               BTPS_StringCopy((char *)(Message->VariableData), ObjectName);

            if((Message->DataLength = PushObjectIndicationData->DataLength) != 0)
               BTPS_MemCopy(&(Message->VariableData[ObjectNameLength]), PushObjectIndicationData->DataBuffer, PushObjectIndicationData->DataLength);

            /* Note the Tracking ID before we release the lock for the  */
            /* callback.                                                */
            TrackingID = OPPMEntryInfo->TrackingID;

            /* Dispatch the event.                                      */
            DispatchOPPMEvent(OPPMEntryInfo, &OPPMEventData, (BTPM_Message_t *)Message);

            /* Make sure that the Entry wasn't deleted during the       */
            /* callback.                                                */
            OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID);

            /* Go ahead and free the memory because we are finished with*/
            /* it.                                                      */
            BTPS_FreeMemory(Message);
         }
         else
         {
            /* Error allocating memory, flag an error.                  */
            ResponseCode                     = OPP_OBEX_RESPONSE_SERVER_ERROR;

            OPPMEntryInfo->CurrentOperation = coNone;
         }

         /* Free any memory that was allocated to hold the converted    */
         /* Folder Name.                                                */
         if((ObjectNameLength) && (ObjectName))
            BTPS_FreeMemory(ObjectName);

         /* Respond to the request.                                     */
         /* * NOTE * We do not respond to the last request because the  */
         /*          server will need to respond with the Message       */
         /*          Handle.                                            */
         if(OPPMEntryInfo)
         {
            if(PushObjectIndicationData->Final == FALSE)
            {
               if(_OPPM_Push_Object_Response(PushObjectIndicationData->OPPID, ResponseCode))
                  OPPMEntryInfo->CurrentOperation = coNone;
            }
         }
      }
      else
      {
         if(!OPPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find OPPID: %d\n", PushObjectIndicationData->OPPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)OPPMEntryInfo->CurrentOperation));

         _OPPM_Push_Object_Response(PushObjectIndicationData->OPPID, (Byte_t)(OPPMEntryInfo?OPP_OBEX_RESPONSE_NOT_ACCEPTABLE:OPP_OBEX_RESPONSE_BAD_REQUEST));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* push object confirmation event that has been received with the    */
   /* specified information.  This function should be called with the   */
   /* lock protecting the Object Push Manager information held.         */
static void ProcessPushObjectConfirmationEvent(OPP_Push_Object_Confirmation_Data_t *PushObjectConfirmationData)
{
   Boolean_t                            DispatchEvent;
   Boolean_t                            Error;
   unsigned int                         DataLength;
   OPPM_Event_Data_t                    OPPMEventData;
   OPPM_Entry_Info_t                   *OPPMEntryInfo;
   OPPM_Push_Object_Response_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(PushObjectConfirmationData)
   {
      /* Search for the OPPID in the entry list.                        */
      if(((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, PushObjectConfirmationData->OPPID)) != NULL) && (OPPMEntryInfo->CurrentOperation == coPushObject))
      {
         /* Flag that we do not need to dispatch the event confirmation */
         /* (this might change below).                                  */
         DispatchEvent = FALSE;

         /* Flag that no error occurred.                                */
         Error         = FALSE;

         /* Determine if we need to send more data.                     */
         if(((PushObjectConfirmationData->ResponseCode == OPP_OBEX_RESPONSE_OK) || (PushObjectConfirmationData->ResponseCode == OPP_OBEX_RESPONSE_CONTINUE)) && (OPPMEntryInfo->DataBufferSize) && (OPPMEntryInfo->DataBuffer) && (!(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)))
         {
            /* Calculate the remaining data to send.                    */
            DataLength = OPPMEntryInfo->DataBufferSize - OPPMEntryInfo->DataBufferSent;

            if(_OPPM_Push_Object_Request(OPPMEntryInfo->OPPID, oppUnknownObject, NULL, 0, DataLength, &(OPPMEntryInfo->DataBuffer[OPPMEntryInfo->DataBufferSent]), &DataLength, OPPMEntryInfo->DataFinal) == 0)
            {
               OPPMEntryInfo->DataBufferSent += DataLength;

               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)OPPMEntryInfo->OPPID, OPPMEntryInfo->DataBufferSent));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)OPPMEntryInfo->OPPID, OPPMEntryInfo->DataBufferSent));

               /* Flag that we have sent all the data (so it can be     */
               /* freed below).                                         */
               OPPMEntryInfo->DataBufferSent = OPPMEntryInfo->DataBufferSize;

               DispatchEvent                 = TRUE;

               Error                         = TRUE;
            }

            /* Free any memory that was allocated (if we have sent all  */
            /* the data).                                               */
            if((OPPMEntryInfo) && (OPPMEntryInfo->DataBufferSent == OPPMEntryInfo->DataBufferSize))
            {
               if(OPPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                  OPPMEntryInfo->DataBuffer = NULL;
               }
            }
         }
         else
         {
            DispatchEvent = TRUE;

            /* If we have a pending abort, let's go ahead and clean up  */
            /* everything.                                              */
            if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
            {
               if(OPPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                  OPPMEntryInfo->DataBuffer = NULL;
               }
            }
         }

         if(DispatchEvent)
         {
            /* Determine if we need to clear the state.                 */
            if((OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT) || (PushObjectConfirmationData->ResponseCode != OPP_OBEX_RESPONSE_CONTINUE) || (Error))
               OPPMEntryInfo->CurrentOperation = coNone;

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

            /* Format the event data.                                   */
            OPPMEventData.EventType                                                  = oetPushObjectResponse;
            OPPMEventData.EventLength                                                = OPPM_PUSH_OBJECT_RESPONSE_EVENT_DATA_SIZE;

            OPPMEventData.EventData.PushObjectResponseEventData.ClientPortID         = OPPMEntryInfo->TrackingID;
            OPPMEventData.EventData.PushObjectResponseEventData.RemoteDeviceAddress  = OPPMEntryInfo->RemoteDeviceAddress;

            if(!(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
               OPPMEventData.EventData.PushObjectResponseEventData.ResponseCode = MapResponseCodeToResponseStatusCode(PushObjectConfirmationData->ResponseCode);
            else
               OPPMEventData.EventData.PushObjectResponseEventData.ResponseCode = OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            Message.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_PUSH_OBJECT_RESPONSE;
            Message.MessageHeader.MessageLength   = OPPM_PUSH_OBJECT_RESPONSE_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
            Message.ClientID                      = OPPMEntryInfo->TrackingID;

            if(!(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT))
               Message.ResponseCode = MapResponseCodeToResponseStatusCode(PushObjectConfirmationData->ResponseCode);
            else
               Message.ResponseCode = OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;

            /* If there is a pending abort, go ahead and issue the      */
            /* abort.                                                   */
            if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               IssuePendingAbort(OPPMEntryInfo);

            /* Dispatch the event.                                      */
            DispatchOPPMEvent(OPPMEntryInfo, &OPPMEventData, (BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!OPPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find OPPID: %d\n", PushObjectConfirmationData->OPPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)OPPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* pull business card indication event that has been received with   */
   /* the specified information.  This function should be called with   */
   /* the lock protecting the Object Push Manager information held.     */
static void ProcessPullBusinessCardIndicationEvent(OPP_Pull_Business_Card_Indication_Data_t *PullBusinessCardIndicationData)
{
   unsigned int                               DataLength;
   OPPM_Event_Data_t                          OPPMEventData;
   OPPM_Entry_Info_t                         *OPPMEntryInfo;
   OPPM_Pull_Business_Card_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(PullBusinessCardIndicationData)
   {
      /* Search for the OPPID in the entry list.                        */
      if(((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, PullBusinessCardIndicationData->OPPID)) != NULL) && ((OPPMEntryInfo->CurrentOperation == coNone) || (OPPMEntryInfo->CurrentOperation == coPullBusinessCard)))
      {
         /* Determine if this a continuation to a prior request.        */

         /* Determine if we need to send more response data.            */
         if((OPPMEntryInfo->CurrentOperation == coPullBusinessCard) && (OPPMEntryInfo->DataBufferSize) && (OPPMEntryInfo->DataBuffer))
         {
            /* Calculate the remaining data to send.                    */
            DataLength = OPPMEntryInfo->DataBufferSize - OPPMEntryInfo->DataBufferSent;

            if(_OPPM_Pull_Business_Card_Response(OPPMEntryInfo->OPPID, (Byte_t)(OPPMEntryInfo->DataFinal?OPP_OBEX_RESPONSE_OK:OPP_OBEX_RESPONSE_CONTINUE), 0, DataLength, &(OPPMEntryInfo->DataBuffer[OPPMEntryInfo->DataBufferSent]), &DataLength) == 0)
            {
               OPPMEntryInfo->DataBufferSent += DataLength;

               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Submitting continuation data: %d, %d\n", (int)OPPMEntryInfo->OPPID, OPPMEntryInfo->DataBufferSent));
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Error submitting continuation data: %d, %d\n", (int)OPPMEntryInfo->OPPID, OPPMEntryInfo->DataBufferSent));

               /* Error submitting response.  Not sure what we can do   */
               /* here.                                                 */
               _OPPM_Pull_Business_Card_Response(OPPMEntryInfo->OPPID, (Byte_t)(OPP_OBEX_RESPONSE_SERVER_ERROR), 0, 0, NULL, NULL);

               /* Flag that we have sent all the data (so it can be     */
               /* freed below).                                         */
               OPPMEntryInfo->DataBufferSent   = OPPMEntryInfo->DataBufferSize;

               /* Flag that there is no longer an operation in progress.*/
               OPPMEntryInfo->CurrentOperation = coNone;
            }

            /* Free any memory that was allocated (if we have sent all  */
            /* the data).                                               */
            if(OPPMEntryInfo->DataBufferSent == OPPMEntryInfo->DataBufferSize)
            {
               if(OPPMEntryInfo->DataBuffer)
               {
                  BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

                  OPPMEntryInfo->DataBuffer = NULL;
               }

               OPPMEntryInfo->CurrentOperation = coNone;
            }
         }
         else
         {
            /* Flag the new state we are entering.                      */
            OPPMEntryInfo->CurrentOperation = coPullBusinessCard;

            /* Free any left-over data (just to be safe).               */
            if(OPPMEntryInfo->DataBuffer)
            {
               BTPS_FreeMemory(OPPMEntryInfo->DataBuffer);

               OPPMEntryInfo->DataBuffer = NULL;
            }

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

            /* Format the event data.                                   */
            OPPMEventData.EventType                                                      = oetPullBusinessCardRequest;
            OPPMEventData.EventLength                                                    = OPPM_PULL_BUSINESS_CARD_REQUEST_EVENT_DATA_SIZE;

            OPPMEventData.EventData.PullBusinessCardRequestEventData.ServerPortID        = OPPMEntryInfo->TrackingID;
            OPPMEventData.EventData.PullBusinessCardRequestEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;

            /* Format the message.                                      */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
            Message.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_REQUEST;
            Message.MessageHeader.MessageLength   = OPPM_PULL_BUSINESS_CARD_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

            Message.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
            Message.ServerID                      = OPPMEntryInfo->TrackingID;

            /* Dispatch the event.                                      */
            DispatchOPPMEvent(OPPMEntryInfo, &OPPMEventData, (BTPM_Message_t *)&Message);
         }
      }
      else
      {
         if(!OPPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find OPPID: %d\n", PullBusinessCardIndicationData->OPPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)OPPMEntryInfo->CurrentOperation));

         _OPPM_Pull_Business_Card_Response(PullBusinessCardIndicationData->OPPID, (Byte_t)(OPPMEntryInfo?OPP_OBEX_RESPONSE_NOT_ACCEPTABLE:OPP_OBEX_RESPONSE_BAD_REQUEST), 0, 0, NULL, NULL);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* pull business card confirmation event that has been received with */
   /* the specified information.  This function should be called with   */
   /* the lock protecting the Object Push Manager information held.     */
static void ProcessPullBusinessCardConfirmationEvent(OPP_Pull_Business_Card_Confirmation_Data_t *PullBusinessCardConfirmationData)
{
   void                                       *CallbackParameter;
   unsigned int                                TrackingID;
   OPPM_Event_Data_t                           OPPMEventData;
   OPPM_Entry_Info_t                          *OPPMEntryInfo;
   OPPM_Event_Callback_t                       EventCallback;
   OPPM_Pull_Business_Card_Response_Message_t  ErrorPullBusinessCardMessage;
   OPPM_Pull_Business_Card_Response_Message_t *PullBusinessCardMessage = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify that the input parameter is not NULL.                      */
   if(PullBusinessCardConfirmationData)
   {
      /* Search for the OPPID in the entry list.                        */
      if(((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, PullBusinessCardConfirmationData->OPPID)) != NULL) && (OPPMEntryInfo->CurrentOperation == coPullBusinessCard))
      {
         /* Get Message Response.                                       */
         if(PullBusinessCardConfirmationData->ResponseCode != OPP_OBEX_RESPONSE_CONTINUE)
            OPPMEntryInfo->CurrentOperation = coNone;

         if(OPPMEntryInfo->ClientID == MSG_GetServerAddressID())
         {
            /* Dispatch the event locally.                              */

            /* Event needs to be dispatched.  Go ahead and format the   */
            /* event.                                                   */
            BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

            /* Format the event data.                                   */
            OPPMEventData.EventType                                                       = oetPullBusinessCardResponse;
            OPPMEventData.EventLength                                                     = OPPM_PULL_BUSINESS_CARD_RESPONSE_EVENT_DATA_SIZE;

            OPPMEventData.EventData.PullBusinessCardResponseEventData.ClientPortID        = OPPMEntryInfo->TrackingID;
            OPPMEventData.EventData.PullBusinessCardResponseEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;
            OPPMEventData.EventData.PullBusinessCardResponseEventData.ResponseCode        = (PullBusinessCardConfirmationData->ResponseCode == 0xFF)?OPPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST:MapResponseCodeToResponseStatusCode(PullBusinessCardConfirmationData->ResponseCode);
            OPPMEventData.EventData.PullBusinessCardResponseEventData.Final               = TRUE;
            OPPMEventData.EventData.PullBusinessCardResponseEventData.ObjectTotalLength   = PullBusinessCardConfirmationData->TotalLength;


            if((OPPMEventData.EventData.PullBusinessCardResponseEventData.DataLength = PullBusinessCardConfirmationData->DataLength) != 0)
               OPPMEventData.EventData.PullBusinessCardResponseEventData.DataBuffer = PullBusinessCardConfirmationData->DataBuffer;

            /* Map the response code to special cases.                  */
            if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               OPPMEventData.EventData.PullBusinessCardResponseEventData.ResponseCode = OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
            else
            {
               if(PullBusinessCardConfirmationData->ResponseCode == OPP_OBEX_RESPONSE_CONTINUE)
                  OPPMEventData.EventData.PullBusinessCardResponseEventData.Final = FALSE;
            }

            /* Note the Callback information.                           */
            TrackingID        = OPPMEntryInfo->TrackingID;
            EventCallback     = OPPMEntryInfo->CallbackFunction;
            CallbackParameter = OPPMEntryInfo->CallbackParameter;

            /* Release the Lock so we can dispatch the event.           */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                   (*EventCallback)(&OPPMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();

            /* Make sure that the Entry wasn't deleted during the       */
            /* callback.                                                */
            OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID);
         }
         else
         {
            /* Dispatch the event remotely.                             */

            /* Format the message.                                      */

            /* First, allocate a message of the correct size.           */
            if((PullBusinessCardMessage = (OPPM_Pull_Business_Card_Response_Message_t *)BTPS_AllocateMemory(OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(PullBusinessCardConfirmationData->DataLength))) != NULL)
            {
               BTPS_MemInitialize(PullBusinessCardMessage, 0, OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(PullBusinessCardConfirmationData->DataLength));

               PullBusinessCardMessage->MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
               PullBusinessCardMessage->MessageHeader.MessageID       = MSG_GetNextMessageID();
               PullBusinessCardMessage->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
               PullBusinessCardMessage->MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_RESPONSE;
               PullBusinessCardMessage->MessageHeader.MessageLength   = OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(PullBusinessCardConfirmationData->DataLength) - BTPM_MESSAGE_HEADER_SIZE;

               PullBusinessCardMessage->RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
               PullBusinessCardMessage->ResponseCode                  = (PullBusinessCardConfirmationData->ResponseCode == 0xFF)?OPPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST:MapResponseCodeToResponseStatusCode(PullBusinessCardConfirmationData->ResponseCode);
               PullBusinessCardMessage->Final                         = TRUE;
               PullBusinessCardMessage->ObjectTotalLength             = PullBusinessCardConfirmationData->TotalLength;
               PullBusinessCardMessage->ClientID                      = OPPMEntryInfo->TrackingID;

               if((PullBusinessCardMessage->DataLength = PullBusinessCardConfirmationData->DataLength) != 0)
                  BTPS_MemCopy(PullBusinessCardMessage->DataBuffer, PullBusinessCardConfirmationData->DataBuffer, PullBusinessCardConfirmationData->DataLength);

               /* Map the response code to special cases.               */
               if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
                  PullBusinessCardMessage->ResponseCode       = OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED;
               else
               {
                  if(PullBusinessCardConfirmationData->ResponseCode == OPP_OBEX_RESPONSE_CONTINUE)
                     PullBusinessCardMessage->Final = FALSE;
               }

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)PullBusinessCardMessage);

               /* Go ahead and free the memory because we are finished  */
               /* with it.                                              */
               BTPS_FreeMemory(PullBusinessCardMessage);
            }
            else
            {
               /* Error allocating the message, go ahead and issue an   */
               /* error response.                                       */
               BTPS_MemInitialize(&ErrorPullBusinessCardMessage, 0, sizeof(ErrorPullBusinessCardMessage));

               ErrorPullBusinessCardMessage.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
               ErrorPullBusinessCardMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               ErrorPullBusinessCardMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
               ErrorPullBusinessCardMessage.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_RESPONSE;
               ErrorPullBusinessCardMessage.MessageHeader.MessageLength   = OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

               ErrorPullBusinessCardMessage.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
               ErrorPullBusinessCardMessage.ResponseCode                  = OPPM_RESPONSE_STATUS_CODE_OPERATION_ABORTED_RESOURCES;
               ErrorPullBusinessCardMessage.Final                         = TRUE;
               ErrorPullBusinessCardMessage.ClientID                      = OPPMEntryInfo->TrackingID;

               /* Message has been formatted, go ahead and dispatch it. */
               MSG_SendMessage((BTPM_Message_t *)&ErrorPullBusinessCardMessage);
            }
         }

         /* Determine if we have an Abort pending.  If so, we need to   */
         /* issue it.                                                   */
         if(OPPMEntryInfo)
         {
            if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT)
               IssuePendingAbort(OPPMEntryInfo);
            else
            {
               /* If there was an error, we might need to issue an      */
               /* Abort.                                                */
               if((!PullBusinessCardMessage) && (PullBusinessCardConfirmationData->ResponseCode == OPP_OBEX_RESPONSE_CONTINUE))
               {
                  OPPMEntryInfo->Flags |= OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                  IssuePendingAbort(OPPMEntryInfo);
               }
               else
               {
                  /* Check to see if we need to re-submit.              */
                  if(PullBusinessCardConfirmationData->ResponseCode == OPP_OBEX_RESPONSE_CONTINUE)
                  {
                     /* Resubmit.                                       */
                     if(_OPPM_Pull_Business_Card_Request(OPPMEntryInfo->OPPID) < 0)
                     {
                        /* Error submitting request.                    */

                        /* Flag that we do not have a current operation */
                        /* in progress.                                 */
                        OPPMEntryInfo->CurrentOperation = coNone;

                        if(OPPMEntryInfo->ClientID == MSG_GetServerAddressID())
                        {
                           /* Dispatch the event locally.               */

                           /* Note the Tracking ID.                     */
                           TrackingID = OPPMEntryInfo->TrackingID;

                           /* Event needs to be dispatched.  Go ahead   */
                           /* and format the event.                     */
                           BTPS_MemInitialize(&OPPMEventData, 0, sizeof(OPPM_Event_Data_t));

                           /* Format the event data.                    */
                           OPPMEventData.EventType                                                       = oetPullBusinessCardResponse;
                           OPPMEventData.EventLength                                                     = OPPM_PULL_BUSINESS_CARD_RESPONSE_EVENT_DATA_SIZE;

                           OPPMEventData.EventData.PullBusinessCardResponseEventData.ClientPortID        = OPPMEntryInfo->TrackingID;
                           OPPMEventData.EventData.PullBusinessCardResponseEventData.RemoteDeviceAddress = OPPMEntryInfo->RemoteDeviceAddress;
                           OPPMEventData.EventData.PullBusinessCardResponseEventData.ResponseCode        = OPPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                           OPPMEventData.EventData.PullBusinessCardResponseEventData.Final               = TRUE;

                           /* Note the Callback information.            */
                           EventCallback                                                                 = OPPMEntryInfo->CallbackFunction;
                           CallbackParameter                                                             = OPPMEntryInfo->CallbackParameter;

                           /* Release the Lock so we can dispatch the   */
                           /* event.                                    */
                           DEVM_ReleaseLock();

                           __BTPSTRY
                           {
                              if(EventCallback)
                                  (*EventCallback)(&OPPMEventData, CallbackParameter);
                           }
                           __BTPSEXCEPT(1)
                           {
                              /* Do Nothing.                            */
                           }

                           /* Re-acquire the Lock.                      */
                           DEVM_AcquireLock();

                           /* Make sure that the Entry wasn't deleted   */
                           /* during the callback.                      */
                           OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, TrackingID);
                        }
                        else
                        {
                           /* Dispatch the event remotely.              */

                           /* Format the message.                       */
                           BTPS_MemInitialize(&ErrorPullBusinessCardMessage, 0, sizeof(ErrorPullBusinessCardMessage));

                           ErrorPullBusinessCardMessage.MessageHeader.AddressID       = OPPMEntryInfo->ClientID;
                           ErrorPullBusinessCardMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                           ErrorPullBusinessCardMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER;
                           ErrorPullBusinessCardMessage.MessageHeader.MessageFunction = OPPM_MESSAGE_FUNCTION_PULL_BUSINESS_CARD_RESPONSE;
                           ErrorPullBusinessCardMessage.MessageHeader.MessageLength   = OPPM_PULL_BUSINESS_CARD_RESPONSE_MESSAGE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;

                           ErrorPullBusinessCardMessage.RemoteDeviceAddress           = OPPMEntryInfo->RemoteDeviceAddress;
                           ErrorPullBusinessCardMessage.ResponseCode                  = OPPM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST;
                           ErrorPullBusinessCardMessage.Final                         = TRUE;
                           ErrorPullBusinessCardMessage.ClientID                      = OPPMEntryInfo->TrackingID;

                           /* Message has been formatted, go ahead and  */
                           /* dispatch it.                              */
                           MSG_SendMessage((BTPM_Message_t *)&PullBusinessCardMessage);
                        }

                        /* Go ahead and issue an Abort to clean things  */
                        /* up.                                          */
                        if(OPPMEntryInfo)
                        {
                           OPPMEntryInfo->Flags |= OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT;

                           IssuePendingAbort(OPPMEntryInfo);
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {
         if(!OPPMEntryInfo)
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to find OPPID: %d\n", PullBusinessCardConfirmationData->OPPID));
         else
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Prior Operation in Progress: %d\n", (int)OPPMEntryInfo->CurrentOperation));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* abort indication event that has been received with the specified  */
   /* information.  This function should be called with the lock        */
   /* protecting the Object Push Manager information held.              */
static void ProcessAbortIndicationEvent(OPP_Abort_Indication_Data_t *AbortIndicationData)
{
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(AbortIndicationData)
   {
      /* Find the list entry by the OPPID.                              */
      /* * NOTE * This entry could be either a OPP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, AbortIndicationData->OPPID)) != NULL)
      {
         /* Flag that there is no longer an operation in progress.      */
         OPPMEntryInfo->CurrentOperation = coNone;

         CleanupOPPMEntryInfo(OPPMEntryInfo);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an incoming  */
   /* abort confirmation event that has been received with the specified*/
   /* information.  This function should be called with the lock        */
   /* protecting the Object Push Manager information held.              */
static void ProcessAbortConfirmationEvent(OPP_Abort_Confirmation_Data_t *AbortConfirmationData)
{
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check if the event data parameter is not NULL.                    */
   if(AbortConfirmationData)
   {
      /* Find the list entry by the OPPID.                              */
      /* * NOTE * This entry could be either a OPP Server Entry or a    */
      /*          Notification Server Entry.                            */
      if((OPPMEntryInfo = SearchOPPMEntryInfoByOPPID(&OPPMEntryInfoList, AbortConfirmationData->OPPID)) != NULL)
      {
         /* Nothing really to do other than to clear the Pending Abort  */
         /* flag.                                                       */
         OPPMEntryInfo->Flags &= ~((unsigned long)OPPM_ENTRY_INFO_FLAGS_PENDING_ABORT);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing OPP Events that have been received.  This function     */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessOPPEvent(OPPM_OPP_Event_Data_t *OPPEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be valid.     */
   if(OPPEventData)
   {
      /* Process the event based on the event type.                     */
      switch(OPPEventData->EventType)
      {
         case etOPP_Open_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(OPPEventData->EventData.OpenRequestIndicationData));
            break;
         case etOPP_Open_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Port Indication\n"));

            ProcessOpenPortIndicationEvent(&(OPPEventData->EventData.OpenPortIndicationData));
            break;
         case etOPP_Open_Port_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Port Confirmation\n"));

            ProcessOpenPortConfirmationEvent(&(OPPEventData->EventData.OpenPortConfirmationData));
            break;
         case etOPP_Close_Port_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Port Indication\n"));

            ProcessClosePortIndicationEvent(&(OPPEventData->EventData.ClosePortIndicationData));
            break;
         case etOPP_Push_Object_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Object Indication\n"));

            ProcessPushObjectIndicationEvent(&(OPPEventData->EventData.PushObjectIndicationData));
            break;
         case etOPP_Push_Object_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Push Object Confirmation\n"));

            ProcessPushObjectConfirmationEvent(&(OPPEventData->EventData.PushObjectConfirmationData));
            break;
         case etOPP_Pull_Business_Card_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Business Card Indication\n"));

            ProcessPullBusinessCardIndicationEvent(&(OPPEventData->EventData.PullBusinessCardIndicationData));
            break;
         case etOPP_Pull_Business_Card_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Pull Business Card Confirmation\n"));

            ProcessPullBusinessCardConfirmationEvent(&(OPPEventData->EventData.PullBusinessCardConfirmationData));
            break;
         case etOPP_Abort_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Indication\n"));

            ProcessAbortIndicationEvent(&(OPPEventData->EventData.AbortIndicationData));
            break;
         case etOPP_Abort_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Abort Confirmation\n"));

            ProcessAbortConfirmationEvent(&(OPPEventData->EventData.AbortConfirmationData));
            break;
      }
   }
   else
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid OPP Event Data\n"));

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* process Device Manager (DEVM) Status Events (for out-going        */
   /* connection management).                                           */
static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                                Result;
   OPPM_Entry_Info_t                 *OPPMEntryInfo;
   OPP_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (OPPM): 0x%08X, %d\n", StatusType, Status));

   /* Determine if we are tracking an outgoing connection to this       */
   /* device.                                                           */
   if((OPPMEntryInfo = SearchOPPMEntryInfoByConnection(&OPPMEntryInfoList, BD_ADDR, FALSE)) != NULL)
   {
      /* Ensure that we are actually waiting for this event.            */
      if((OPPMEntryInfo->ConnectionState == csAuthenticating) || (OPPMEntryInfo->ConnectionState == csEncrypting) || (OPPMEntryInfo->ConnectionState == csConnectingDevice))
      {
         /* Initialize common connection event members.                 */
         BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(OPP_Open_Port_Confirmation_Data_t));

         /**************************** NOTE *****************************/
         /* * We do not have a OPP ID because we were unable to make  * */
         /* * a connection.  To allow re-use of the disconnect event  * */
         /* * dispatching we will use the Tracking ID AND logical OR  * */
         /* * the Most Significant bit (this is an ID that cannot     * */
         /* * occur as a OPP ID).  There is special code added to     * */
         /* * process Open Port Confirmation function to handle this  * */
         /* * case.                                                   * */
         /**************************** NOTE *****************************/
         if(!OPPMEntryInfo->OPPID)
            OpenPortConfirmationData.OPPID = OPPMEntryInfo->TrackingID | 0x80000000;
         else
            OpenPortConfirmationData.OPPID = OPPMEntryInfo->OPPID;

         if(Status)
         {
            /* Disconnect the device.                                   */
            DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

            /* Connection Failed.                                       */

            /* Map the status to a known status.                        */
            switch(Status)
            {
               case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                  OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_CONNECTION_REFUSED;
                  break;
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                  OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_CONNECTION_TIMEOUT;
                  break;
               default:
                  OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_UNKNOWN_ERROR;
                  break;
            }

            /* * NOTE * This function will delete the OPP entry from the*/
            /* list.                                                    */
            ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
         }
         else
         {
            /* Connection succeeded.                                    */

            /* Move the state to the connecting state.                  */
            OPPMEntryInfo->ConnectionState = csConnecting;

            /* Go ahead and submit the connection.                      */
            Result = _OPPM_Open_Remote_Object_Push_Server(BD_ADDR, OPPMEntryInfo->PortNumber);

            if((Result <= 0) && (Result != BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_ALREADY_CONNECTED))
            {
               /* Error opening device.                                 */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_UNKNOWN_ERROR;

               /* * NOTE * This function will delete the Message Access */
               /* entry from the list.                                  */
               ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
            }
            else
            {
               /* If the device is already connected, we will dispatch  */
               /* the the Status only (note this case shouldn't really  */
               /* occur, but just to be safe we will clean up our state */
               /* machine).                                             */
               if(Result == BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_ALREADY_CONNECTED)
               {
                  OPPMEntryInfo->ConnectionState      = csConnected;

                  OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_SUCCESS;

                  ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
               }
               else
                  OPPMEntryInfo->OPPID = (unsigned int)Result;
            }
         }
      }
   }

   /* We may also have an incoming connection from the same device.     */
   if((OPPMEntryInfo = SearchOPPMEntryInfoByConnection(&OPPMEntryInfoList, BD_ADDR, TRUE)) != NULL)
   {
      /* Ensure that we are actually waiting for this event.            */
      if((OPPMEntryInfo->ConnectionState == csAuthenticating) || (OPPMEntryInfo->ConnectionState == csEncrypting) || (OPPMEntryInfo->ConnectionState == csConnectingDevice))
      {
         /* Check the DEVM status.                                      */
         if(!Status)
         {
            /* Success, simply accept the connection.                   */
            _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, TRUE);
         }
         else
         {
            /* An error occurred, reject the connection.                */
            _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE);

            /* Set the state back to Idle.                              */
            OPPMEntryInfo->ConnectionState = csIdle;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (OPPM)\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Object Push Profile Manager Timer Events.   */
static void BTPSAPI BTPMDispatchCallback_TMR(void *CallbackParameter)
{
   int                                Result;
   OPPM_Entry_Info_t                 *OPPMEntryInfo;
   OPP_Open_Port_Confirmation_Data_t  OpenPortConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (OPPM)\n"));

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
            if((OPPMEntryInfo = SearchOPPMEntryInfoByTrackingID(&OPPMEntryInfoList, (unsigned int)CallbackParameter)) != NULL)
            {
               /* Check to see if the Timer is still active.            */
               if(OPPMEntryInfo->ConnectionTimerID)
               {
                  /* Flag that the Timer is no longer valid (it has been*/
                  /* processed).                                        */
                  OPPMEntryInfo->ConnectionTimerID = 0;

                  /* Finally make sure that we are still in the correct */
                  /* state.                                             */
                  if(OPPMEntryInfo->ConnectionState == csConnectingWaiting)
                  {
                     /* Everything appears to be valid, go ahead and    */
                     /* attempt to check to see if a connection is      */
                     /* possible (if so, attempt it).                   */
                     if(!SPPM_WaitForPortDisconnection(OPPMEntryInfo->PortNumber, FALSE, OPPMEntryInfo->RemoteDeviceAddress, MAXIMUM_OPP_PORT_DELAY_TIMEOUT_MS))
                     {
                        /* Port is disconnected, let's attempt to make  */
                        /* the connection.                              */
                        DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

                        /* Next, attempt to open the remote device      */
                        if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_ENCRYPTION)
                           OPPMEntryInfo->ConnectionState = csEncrypting;
                        else
                        {
                           if(OPPMEntryInfo->ConnectionFlags & OPPM_REGISTER_SERVER_FLAGS_REQUIRE_AUTHENTICATION)
                              OPPMEntryInfo->ConnectionState = csAuthenticating;
                           else
                              OPPMEntryInfo->ConnectionState = csConnectingDevice;
                        }

                        Result = DEVM_ConnectWithRemoteDevice(OPPMEntryInfo->RemoteDeviceAddress, (OPPMEntryInfo->ConnectionState == csConnectingDevice)?0:((OPPMEntryInfo->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                        if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* Check to see if we need to actually issue */
                           /* the Remote connection.                    */
                           if(Result < 0)
                           {
                              /* Set the state to connecting remote     */
                              /* device.                                */
                              OPPMEntryInfo->ConnectionState = csConnecting;


                              Result = _OPPM_Open_Remote_Object_Push_Server(OPPMEntryInfo->RemoteDeviceAddress, OPPMEntryInfo->PortNumber);

                              if(Result < 0)
                                 Result = BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE;
                              else
                              {
                                 /* Note the OPP ID.                    */
                                 OPPMEntryInfo->OPPID = (unsigned int)Result;

                                 /* Flag success.                       */
                                 Result               = 0;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Port is not disconnected, check to see if the*/
                        /* count exceedes the maximum count.            */
                        OPPMEntryInfo->ConnectionTimerCount++;

                        if(OPPMEntryInfo->ConnectionTimerCount >= MAXIMUM_OPP_PORT_OPEN_DELAY_RETRY)
                           Result = BTPM_ERROR_CODE_OBJECT_PUSH_CONNECTION_RETRIES_EXCEEDED;
                        else
                        {
                           /* Port is NOT disconnected, go ahead and    */
                           /* start a timer so that we can continue to  */
                           /* check for the Port Disconnection.         */
                           Result = TMR_StartTimer((void *)OPPMEntryInfo->TrackingID, TMRCallback, BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_PORT_OPEN_DELAY_TIME_MS);

                           /* If the timer was started, go ahead and    */
                           /* note the Timer ID.                        */
                           if(Result > 0)
                           {
                              OPPMEntryInfo->ConnectionTimerID = (unsigned int)Result;

                              Result                           = 0;
                           }
                           else
                              Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER;
                        }
                     }

                     /* Error occurred, go ahead and dispatch an error  */
                     /* (as well as to delete the connection entry).    */
                     if(Result < 0)
                     {
                        /* Initialize common connection event members.  */
                        BTPS_MemInitialize(&OpenPortConfirmationData, 0, sizeof(OPP_Open_Port_Confirmation_Data_t));

                        /********************* NOTE *********************/
                        /* * We do not have a OPP ID because we were  * */
                        /* * unable to make a connection.  To allow   * */
                        /* * re-use of the disconnect event           * */
                        /* * dispatching we will use the Tracking ID  * */
                        /* * AND logical OR the Most Significant bit  * */
                        /* * (this is an ID that cannot occur as a    * */
                        /* * OPP ID).  There is special code added to * */
                        /* * process Open Port Confirmation function  * */
                        /* * to handle this case.                     * */
                        /********************* NOTE *********************/
                        OpenPortConfirmationData.OPPID = OPPMEntryInfo->TrackingID | 0x80000000;

                        if(Result)
                        {
                           /* Error, go ahead and disconnect the device.*/
                           DEVM_DisconnectRemoteDevice(OPPMEntryInfo->RemoteDeviceAddress, 0);

                           /* Connection Failed.                        */

                           /* Map the status to a known status.         */
                           switch(Result)
                           {
                              case BTPM_ERROR_CODE_OBJECT_PUSH_CONNECTION_RETRIES_EXCEEDED:
                              case BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER:
                                 OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_CONNECTION_TIMEOUT;
                                 break;
                              case BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE:
                                 OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_UNKNOWN_ERROR;
                                 break;
                              case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                              case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                              default:
                                 OpenPortConfirmationData.OpenStatus = OPP_OPEN_STATUS_CONNECTION_REFUSED;
                                 break;
                           }

                           /* * NOTE * This function will delete the    */
                           /*          Object Push entry from the list. */
                           ProcessOpenPortConfirmationEvent(&OpenPortConfirmationData);
                        }
                     }
                  }
                  else
                  {
                     DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Connection is no longer in the correct state: 0x%08X (%d)\n", (unsigned int)CallbackParameter, OPPMEntryInfo->ConnectionState));
                  }
               }
               else
               {
                  DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Close Timer is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
               }
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Connection is no longer valid: 0x%08X\n", (unsigned int)CallbackParameter));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (OPPM)\n"));
}

   /* The following function is the Timer Callback function that is     */
   /* registered to process Serial Port Disconnection Events (to        */
   /* determine when it is safe to connect to a remote device).         */
static Boolean_t BTPSAPI TMRCallback(unsigned int TimerID, void *CallbackParameter)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (OPPM)\n"));

   /* Simply queue a Timer Callback Event to process.                   */
   if(BTPM_QueueMailboxCallback(BTPMDispatchCallback_TMR, CallbackParameter))
      ret_val = FALSE;
   else
      ret_val = TRUE;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (OPPM): %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Message Access Manager asynchronous events. */
static void BTPSAPI BTPMDispatchCallback_OPPM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine if the specified parameter appears to be          */
   /* semi-valid.                                                       */
   if(CallbackParameter)
   {
      /* Next, let's check to see if we need to process it.             */
      if(Initialized)
      {
         /* Attempt to wait for access to the Message Access state      */
         /* information.                                                */
         if(DEVM_AcquireLock())
         {
            /* Process the Message.                                     */
            ProcessReceivedMessage((BTPM_Message_t *)CallbackParameter);

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* All finished with the Message, so go ahead and free it.        */
      MSG_FreeReceivedMessageGroupHandlerMessage((BTPM_Message_t *)CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Message Access Manager update events.       */
static void BTPSAPI BTPMDispatchCallback_OPP(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to ensure the function parameter is not NULL.               */
   if(CallbackParameter)
   {
      /* Check to make sure the module is initialized.                  */
      if(Initialized)
      {
         /* Wait for access to the Message Access state information.    */
         if(DEVM_AcquireLock())
         {
            if(((OPPM_Update_Data_t *)CallbackParameter)->UpdateType == utOPPEvent)
            {
               /* Process the Message.                                  */
               ProcessOPPEvent(&(((OPPM_Update_Data_t *)CallbackParameter)->UpdateData.OPPEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all OPP Manager Messages.   */
static void BTPSAPI ObjectPushManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is a OPP Manager defined    */
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
               /* OPP Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_OPPM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue OPP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue OPP Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an OPP Manager defined   */
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
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Non OPP Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning   */
   /* up the Bluetopia Platform Manager Opbject Push Profile (OPP)      */
   /* Manager (OPPM) module.  This function should be registered with   */
   /* the Bluetopia Platform Manager module handler and will be called  */
   /* when the Platform Manager is initialized (or shut down).          */
void BTPSAPI OPPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing OPP Manager\n"));

         /* Initialize success.                                         */
         Result = 0;

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process OPP Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER, ObjectPushManagerGroupHandler, NULL))
         {
            /* Initialize the actual OPP Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the OPP Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _OPPM_Initialize()))
            {
               /* Go ahead and flag that this module is initialized.    */
               Initialized = TRUE;
            }
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

         /* If an error occurred then we need to free all resources that*/
         /* were allocated.                                             */
         if(Result)
         {
            _OPPM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("OPP Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_OBJECT_PUSH_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the OPP Manager Implementation that  */
            /* we are shutting down.                                    */
            _OPPM_Cleanup();

            /* Make sure that the OPP Entry Information List is empty.  */
            FreeOPPMEntryInfoList(&OPPMEntryInfoList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI OPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                Result;
   OPPM_Entry_Info_t *OPPMEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the OPP Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
               {
                  /* Set the Bluetooth Stack ID in the OPPMGR.          */
                  _OPPM_SetBluetoothStackID((unsigned int)Result);
               }
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
               OPPMEntryInfo = OPPMEntryInfoList;

               while(OPPMEntryInfo)
               {
                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(OPPMEntryInfo->ConnectionEvent)
                  {
                     OPPMEntryInfo->ConnectionStatus = OPPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(OPPMEntryInfo->ConnectionEvent);
                  }

                  /* Next, determine if we need to close down anything. */
                  if(OPPMEntryInfo->OPPID)
                  {
                     if(OPPMEntryInfo->Flags & OPPM_ENTRY_INFO_FLAGS_SERVER)
                     {
                        if(OPPMEntryInfo->ConnectionState == csAuthorizing)
                           _OPPM_Open_Request_Response(OPPMEntryInfo->OPPID, FALSE);

                        _OPPM_Close_Server(OPPMEntryInfo->OPPID);

                        if(OPPMEntryInfo->ServiceRecordHandle)
                           _OPPM_Un_Register_SDP_Record(OPPMEntryInfo->OPPID, OPPMEntryInfo->ServiceRecordHandle);
                     }
                     else
                        _OPPM_Close_Connection(OPPMEntryInfo->OPPID);
                  }

                  OPPMEntryInfo = OPPMEntryInfo->NextOPPMEntryInfoPtr;
               }

               /* Finally free all Connection Entries (as there cannot  */
               /* be any active connections).                           */
               FreeOPPMEntryInfoList(&OPPMEntryInfoList);

               /* Inform the OPP Manager that the Stack has been closed.*/
               _OPPM_SetBluetoothStackID(0);
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

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the OPP Manager of a specific Update Event.  The OPP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t OPPM_NotifyUpdate(OPPM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utOPPEvent:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing OPP Event: %d\n", UpdateData->UpdateData.OPPEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_OPP, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming OPP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A OPP Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI OPPM_Connection_Request_Response(unsigned int ServerID, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ServerID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessConnectionRequestResponse(MSG_GetServerAddressID(), ServerID, Accept);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allows a mechanism for local*/
   /* modules to register an Object Push Server. This first parameter   */
   /* is the RFCOMM server port. If this parameter is zero, the server  */
   /* will be opened on an available port. The SupportedObjectTypes     */
   /* parameter is a bitmask representing the types of objects supported*/
   /* by this server. The IncomingConnectionFlags parameter is a        */
   /* bitmask which indicates whether incoming connections should       */
   /* be authorized, autenticated, or encrypted. The ServiceName        */
   /* parameter is a null-terminate string represting the name of the   */
   /* service to be placed in the Service Record. The EventCallback     */
   /* is the function which will receive events related to this         */
   /* server. The CallbackParameter will be included in each call to    */
   /* the CallbackFunction. This function returns a positive integer    */
   /* representing the ServerID of the created server if successful and */
   /* a negative error code if there was an error.                      */
int BTPSAPI OPPM_Register_Server(unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if((ServerPort >= OPP_PORT_NUMBER_MINIMUM) && (ServerPort <= OPP_PORT_NUMBER_MAXIMUM) && (CallbackFunction))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessRegisterServer(MSG_GetServerAddressID(), ServerPort, SupportedObjectTypes, IncomingConnectionFlags, ServiceName, CallbackFunction, CallbackParameter);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allows a mechanism for      */
   /* local modules to register an Object Push Server registered by a   */
   /* successful call to OPPM_Register_Server(). This function accepts  */
   /* as a parameter the ServerID returned from a successful call to    */
   /* OPPM_Register_Server(). This function returns zero if successful  */
   /* and a negative error code if there was an error.                  */
int BTPSAPI OPPM_Un_Register_Server(unsigned int ServerID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ServerID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessUnRegisterServer(MSG_GetServerAddressID(), ServerID);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the details of OPP services offered by a remote  */
   /* Object Push Server device. This function accepts the remote device*/
   /* address of the device whose SDP records will be parsed and the    */
   /* buffer which will hold the parsed service details. This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
   /* * NOTE * When this function is successful, the provided buffer    */
   /*          will be populated with the parsed service                */
   /*          details. This buffer MUST be passed to                   */
   /*          OPPM_Free_Object_Push_Service_Info() in order to release */
   /*          any resources that were allocated during the query       */
   /*          process.                                                 */
int BTPSAPI OPPM_Parse_Remote_Object_Push_Services(BD_ADDR_t RemoteDeviceAddress, OPPM_Parsed_Service_Info_t *ServiceInfo)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress) && (ServiceInfo))
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ParseObjectPushServicesFromSDP(RemoteDeviceAddress, ServiceInfo);

               /* Release the lock.                                     */
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to free all resources that were allocated to        */
   /* query the service details of a Object Push Server device. See     */
   /* the OPPM_Query_Remote_Object_Push_Services() function for more    */
   /* information.                                                      */
void BTPSAPI OPPM_Free_Parsed_Object_Push_Service_Info(OPPM_Parsed_Service_Info_t *ServiceInfo)
{
   unsigned int Index;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Next, verfiy that the input parameters appear to be semi-valid    */
   if((ServiceInfo) && (ServiceInfo->ServiceDetails) && (ServiceInfo->RESERVED))
   {
      for(Index=0;Index<ServiceInfo->NumberServices;Index++)
      {
         BTPS_FreeMemory(&(ServiceInfo->ServiceDetails[Index]));
      }

      BTPS_FreeMemory(ServiceInfo->RESERVED);

      BTPS_MemInitialize(ServiceInfo, 0, sizeof(OPPM_Parsed_Service_Info_t));
   }

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Object Push Server device.  The    */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The ConnectionFlags*/
   /* parameter specifies whether authentication or encryption should   */
   /* be used to create this connection.  The CallbackFunction is the   */
   /* function that will be registered to receive events for this       */
   /* connection.  The CallbackParameter is a parameter which will be   */
   /* included in the status callback.  This function returns a positive*/
   /* value representing the ClientID of this connection if successful, */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Message Access Manager Event Callback supplied.          */
int BTPSAPI OPPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if((!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)) && (RemoteServerPort >= OPP_PORT_NUMBER_MINIMUM) && (RemoteServerPort <= OPP_PORT_NUMBER_MAXIMUM))
      {
         /* Verify that the Event Callback appears to be semi-valid.    */
         if(CallbackFunction)
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               /* Wait for access to the lock that guards access to this*/
               /* module.                                               */
               if(DEVM_AcquireLock())
               {
                  /* Simply call the helper function to handle this     */
                  /* request.                                           */
                  ret_val = ProcessConnectRemoteDevice(MSG_GetServerAddressID(), RemoteDeviceAddress, RemoteServerPort, ConnectionFlags, CallbackFunction, CallbackParameter, ConnectionStatus);

                  /* Release the Lock because we are finished with it.  */
                  if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
                     DEVM_ReleaseLock();
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
            }
            else
               ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function exists to close an active Object Push      */
   /* connection that was previously opened by a successful call to     */
   /* OPPM_Connect_Remote_Device() function or by a oetConnected        */
   /* event. This function accpets the either the ClientID or ServerID  */
   /* of the connection as a parameter. This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
int BTPSAPI OPPM_Disconnect(unsigned int PortID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(PortID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessDisconnect(MSG_GetServerAddressID(), PortID);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding OPPM profile client request.  This function accepts as*/
   /* input the ClientID of the device specifying which connection is to*/
   /* have the Abort issued.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int BTPSAPI OPPM_Abort(unsigned int ClientID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ClientID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessAbort(MSG_GetServerAddressID(), ClientID);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter    */
   /* is the ClientID of the remote device connection. The ObjectType   */
   /* parameter specifies the type of object being pushed. The Object   */
   /* Name parameter is a UNICODE encoded string representing the name  */
   /* of the object to push. The DataLength and DataBuffer specify the  */
   /* length and contents of the object. This function returns zero if  */
   /* successful and a negative error code if there is an error.        */
   /* * NOTE * The Object Name is a pointer to a NULL Terminated        */
   /*          UNICODE String.                                          */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int BTPSAPI OPPM_Push_Object_Request(unsigned int ClientID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ClientID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessPushObjectRequest(MSG_GetServerAddressID(), ClientID, ObjectType, ObjectName, ObjectTotalLength, DataLength, DataBuffer, Final);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an Object       */
   /* Push Response to the remote Client.  The first parameter is       */
   /* the ServerID of the local Object Push server. The ResponseCode    */
   /* parameter is the OBEX response code associated with this          */
   /* response. The function returns zero if successful and a negative  */
   /* error code if there is an error.                                  */
int BTPSAPI OPPM_Push_Object_Response(unsigned int ServerID, unsigned int ResponseCode)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ServerID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessPushObjectResponse(MSG_GetServerAddressID(), ServerID, ResponseCode);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The Client        */
   /* parameter is the ClientID of the remote Object Push server        */
   /* connection. This function returns zero if successful and a        */
   /* negative error code if there was an error.                        */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
int BTPSAPI OPPM_Pull_Business_Card_Request(unsigned int ClientID)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ClientID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessPullBusinessCardRequest(MSG_GetServerAddressID(), ClientID);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter is the   */
   /* ServerID of the remote Object Push client. The ResponseCode       */
   /* parameter is the OBEX response code associated with the           */
   /* response. The DataLength and DataBuffer parameters contain the    */
   /* business card data to be sent. This function returns zero if      */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
int BTPSAPI OPPM_Pull_Business_Card_Response(unsigned int ServerID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Object Push Manager has been        */
   /* initialized.                                                      */
   if(Initialized)
   {
      /* Next, verfiy that the input parameters appear to be semi-valid */
      if(ServerID)
      {
         /* Next check to make sure we are powered up.                  */
         if(CurrentPowerState)
         {
            /* Wait for access to the lock that guards access to this   */
            /* module.                                                  */
            if(DEVM_AcquireLock())
            {
               /* Simply call the helper function to handle this        */
               /* request.                                              */
               ret_val = ProcessPullBusinessCardResponse(MSG_GetServerAddressID(), ServerID, ResponseCode, ObjectTotalLength, DataLength, DataBuffer, Final);

               /* Release the Lock, only if the processing call did not */
               /* release it already.                                   */
               if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
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
   }
   else
      ret_val = BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_OBJECT_PUSH | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


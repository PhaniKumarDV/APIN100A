/*****< btpmaudm.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMAUDM - Audio Manager for Stonestreet One Bluetooth Protocol Stack     */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMAUDM.h"            /* BTPM Audio Manager Prototypes/Constants.  */
#include "AUDMMSG.h"             /* BTPM Audio Manager Message Formats.       */
#include "AUDMGR.h"              /* Audio Manager Impl. Prototypes/Constants. */
#include "AUDMUTIL.h"            /* Audio Manager Util. Prototypes/Constants. */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* Structure which is used to track Callback Information related to  */
   /* this module.                                                      */
typedef struct _tagAudio_Entry_Info_t
{
   unsigned int                   CallbackID;
   unsigned int                   ClientID;
   unsigned int                   ConnectionStatus;
   Event_t                        ConnectionEvent;
   BD_ADDR_t                      RemoteControlDevice;
   Boolean_t                      EventCallbackEntry;
   AUD_Stream_Type_t              StreamType;
   AUDM_Event_Callback_t          EventCallback;
   void                          *CallbackParameter;
   struct _tagAudio_Entry_Info_t *NextAudioEntryInfoPtr;
} Audio_Entry_Info_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   AUDM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking incoming connections.                 */
typedef enum
{
   icsAuthorizing,
   icsAuthenticating,
   icsEncrypting
} Incoming_Connection_State_t;

   /* Structure which is used to track information pertaining to        */
   /* incoming connection requests.                                     */
typedef struct _tagIncoming_Connection_Entry_t
{
   BD_ADDR_t                               BD_ADDR;
   Incoming_Connection_State_t             IncomingConnectionState;
   AUD_Connection_Request_Type_t           RequestType;
   struct _tagIncoming_Connection_Entry_t *NextIncomingConnectionEntryPtr;
} Incoming_Connection_Entry_t;

   /* The following enumerated type is used with the DEVM_Status_t      */
   /* structure to denote actual type of the status type information.   */
typedef enum
{
   dstAuthentication,
   dstEncryption,
   dstConnection
} DEVM_Status_Type_t;

   /* The following stucture is a container structure that is used to   */
   /* hold all information regarding a Device Manager (DEVM)            */
   /* Asynchronous Status Result.                                       */
typedef struct _tagDEVM_Status_t
{
   DEVM_Status_Type_t StatusType;
   BD_ADDR_t          BD_ADDR;
   int                Status;
} DEVM_Status_t;

   /* The following enumerated type represents all the states that an   */
   /* Audio Stream or Remote Control connection can be operating in     */
   /* (regarding connections).                                          */
typedef enum
{
   scsConnecting,
   scsConnectingAuthenticating,
   scsConnectingEncrypting,
   scsConnectionRequest,
   scsConnectingStream,
   scsConnected,
   scsConnectingBrowsing,
   scsConnectedBrowsing
} Connection_State_t;

   /* Structure which is used to track information pertaining to Remote */
   /* Control Connections.                                              */
typedef struct _tagControl_Connection_Entry_t
{
   Connection_State_t                     ConnectionState;
   BD_ADDR_t                              BD_ADDR;
   struct _tagControl_Connection_Entry_t *NextControlConnectionEntryPtr;
} Control_Connection_Entry_t;

   /* Structure which is used to track information pertaining to Audio  */
   /* Connections                                                       */
typedef struct _tagAudio_Connection_Entry_t
{
   Connection_State_t                   ConnectionState;
   BD_ADDR_t                            BD_ADDR;
   AUD_Stream_Type_t                    StreamType;
   struct _tagAudio_Connection_Entry_t *NextAudioConnectionEntryPtr;
} Audio_Connection_Entry_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* Variable which is used to hold the next (unique) Callback ID.     */
static unsigned int NextCallbackID;

   /* Variable which holds a pointer to the first element in the Audio  */
   /* Entry Information List (which holds all Callbacks tracked by this */
   /* module).                                                          */
static Audio_Entry_Info_t *AudioEntryInfoList;

   /* Variable which holds a pointer to the first element of the Audio  */
   /* Entry Information List for Data Event Callbacks (which holds all  */
   /* Data Event Callbacks tracked by this module).                     */
static Audio_Entry_Info_t *AudioEntryInfoDataList;

   /* Variable which holds a pointer to the first element of the Audio  */
   /* Entry Information List for Remote Control Event Callbacks         */
   /* registered for outgoing connections (which holds all Remote       */
   /* Control Event Callbacks tracked by this module).                  */
static Audio_Entry_Info_t *OutgoingRemoteControlList;

   /* Variable which holds a pointer to the first element of the Audio  */
   /* Entry Information List for Remote Control Event Callbacks (which  */
   /* holds all Remote Control Event Callbacks tracked by this module). */
static Audio_Entry_Info_t *AudioEntryInfoRemoteControlList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable which holds a pointer for the first element of the Remote*/
   /* Control Connection List.                                          */
static Control_Connection_Entry_t *RemoteControlConnectionList;

   /* Variable which holds a pointer for the first element of the Remote*/
   /* Control Connection List.                                          */
static Audio_Connection_Entry_t *AudioConnectionList;

   /* Variables which control incoming connection requests              */
   /* (Authorization/Authentication/Encryption).                        */
static unsigned long                IncomingConnectionFlags;
static Incoming_Connection_Entry_t *IncomingConnectionEntryList;

   /* Variables which hold the current connection states of the SRC and */
   /* SNK Streams (respectively).                                       */
//static Connection_State_t SRCConnectionState;
//static BD_ADDR_t          SRCConnectedBD_ADDR;
//static Boolean_t          SRCChangeFormatOutstanding;
//static Boolean_t          SRCChangeStateOutstanding;
//
//static Connection_State_t SNKConnectionState;
//static BD_ADDR_t          SNKConnectedBD_ADDR;
//static Boolean_t          SNKChangeFormatOutstanding;
//static Boolean_t          SNKChangeStateOutstanding;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static Audio_Entry_Info_t *AddAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, Audio_Entry_Info_t *EntryToAdd);
static Audio_Entry_Info_t *SearchAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID);
static Audio_Entry_Info_t *DeleteAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID);
static void FreeAudioEntryInfoEntryMemory(Audio_Entry_Info_t *EntryToFree);
static void FreeAudioEntryInfoList(Audio_Entry_Info_t **ListHead);

static Incoming_Connection_Entry_t *AddIncomingConnectionEntry(Incoming_Connection_Entry_t **ListHead, Incoming_Connection_Entry_t *EntryToAdd);
static Incoming_Connection_Entry_t *SearchIncomingConnectionEntry(Incoming_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Connection_Request_Type_t RequestType);
static Incoming_Connection_Entry_t *DeleteIncomingConnectionEntry(Incoming_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Connection_Request_Type_t RequestType);
static void FreeIncomingConnectionEntryMemory(Incoming_Connection_Entry_t *EntryToFree);
static void FreeIncomingConnectionEntryList(Incoming_Connection_Entry_t **ListHead);

static Control_Connection_Entry_t *AddControlConnectionEntry(Control_Connection_Entry_t **ListHead, Control_Connection_Entry_t *EntryToAdd);
static Control_Connection_Entry_t *SearchControlConnectionEntry(Control_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Control_Connection_Entry_t *DeleteControlConnectionEntry(Control_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeControlConnectionEntryMemory(Control_Connection_Entry_t *EntryToFree);
static void FreeControlConnectionEntryList(Control_Connection_Entry_t **ListHead);

static Audio_Connection_Entry_t *AddAudioConnectionEntry(Audio_Connection_Entry_t **ListHead, Audio_Connection_Entry_t *EntryToAdd);
static Audio_Connection_Entry_t *SearchAudioConnectionEntry(Audio_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Stream_Type_t StreamType);
static Audio_Connection_Entry_t *DeleteAudioConnectionEntry(Audio_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Stream_Type_t StreamType);
static void FreeAudioConnectionEntryMemory(Audio_Connection_Entry_t *EntryToFree);
static void FreeAudioConnectionEntryList(Audio_Connection_Entry_t **ListHead);

static void DispatchAudioEvent(AUDM_Event_Data_t *AUDMEventData, BTPM_Message_t *Message);
static void DispatchRemoteControlEvent(AUDM_Event_Data_t *AUDMEventData, BTPM_Message_t *Message, unsigned int TypeMask);

static void DispatchEventList(AUDM_Event_Data_t *AUDMEventData, BTPM_Message_t *Message, unsigned int NumberCallbacks, Callback_Info_t *CallbackInfoArrayPtr);

static int ProcessQueryAudioConnections(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

static int ConnectRemoteControlDevice(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, unsigned int *ConnectionStatus, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
static int ProcessQueryRemoteControlConnections(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

static void ProcessConnectionResponseMessage(AUDM_Connection_Request_Response_Request_t *Message);
static void ProcessConnectAudioStreamMessage(AUDM_Connect_Audio_Stream_Request_t *Message);
static void ProcessDisconnectAudioStreamMessage(AUDM_Disconnect_Audio_Stream_Request_t *Message);
static void ProcessQueryAudioConnectedDevicesMessage(AUDM_Query_Audio_Connected_Devices_Request_t *Message);
static void ProcessQueryAudioStreamStateMessage(AUDM_Query_Audio_Stream_State_Request_t *Message);
static void ProcessQueryAudioStreamFormatMessage(AUDM_Query_Audio_Stream_Format_Request_t *Message);
static void ProcessChangeAudioStreamStateMessage(AUDM_Change_Audio_Stream_State_Request_t *Message);
static void ProcessChangeAudioStreamFormatMessage(AUDM_Change_Audio_Stream_Format_Request_t *Message);
static void ProcessQueryAudioStreamConfigurationMessage(AUDM_Query_Audio_Stream_Configuration_Request_t *Message);
static void ProcessChangeIncomingConnectionFlagsMessage(AUDM_Change_Incoming_Connection_Flags_Request_t *Message);
static void ProcessSendEncodedAudioDataMessage(AUDM_Send_Encoded_Audio_Data_Request_t *Message);
static void ProcessSendRTPEncodedAudioDataMessage(AUDM_Send_RTP_Encoded_Audio_Data_Request_t *Message);
static void ProcessConnectRemoteControlMessage(AUDM_Connect_Remote_Control_Request_t *Message);
static void ProcessDisconnectRemoteControlMessage(AUDM_Disconnect_Remote_Control_Request_t *Message);
static void ProcessQueryRemoteControlConnectedDevicesMessage(AUDM_Query_Remote_Control_Connected_Devices_Request_t *Message);
static void ProcessSendRemoteControlCommandMessage(AUDM_Send_Remote_Control_Command_Request_t *Message);
static void ProcessSendRemoteControlResponseMessage(AUDM_Send_Remote_Control_Response_Request_t *Message);
static void ProcessRegisterAudioStreamEventsMessage(AUDM_Register_Audio_Stream_Events_Request_t *Message);
static void ProcessUnRegisterAudioStreamEventsMessage(AUDM_Un_Register_Audio_Stream_Events_Request_t *Message);
static void ProcessRegisterAudioStreamDataEventsMessage(AUDM_Register_Audio_Stream_Data_Events_Request_t *Message);
static void ProcessUnRegisterAudioStreamDataEventsMessage(AUDM_Un_Register_Audio_Stream_Data_Events_Request_t *Message);
static void ProcessRegisterRemoteControlDataEventsMessage(AUDM_Register_Remote_Control_Data_Events_Request_t *Message);
static void ProcessUnRegisterRemoteControlDataEventsMessage(AUDM_Un_Register_Remote_Control_Data_Events_Request_t *Message);
static void ProcessConnectRemoteControlBrowsingMessage(AUDM_Connect_Remote_Control_Browsing_Request_t *Message);
static void ProcessDisconnectRemoteControlBrowsingMessage(AUDM_Disconnect_Remote_Control_Browsing_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void ProcessOpenRequestIndicationEvent(AUD_Open_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessStreamOpenIndicationEvent(AUD_Stream_Open_Indication_Data_t *StreamOpenIndicationData);
static void ProcessStreamOpenConfirmationEvent(AUD_Stream_Open_Confirmation_Data_t *StreamOpenConfirmationData);
static void ProcessStreamCloseIndicationEvent(AUD_Stream_Close_Indication_Data_t *StreamCloseIndicationData);
static void ProcessStreamStateChangeIndicationEvent(AUD_Stream_State_Change_Indication_Data_t *StreamStateChangeIndicationData);
static void ProcessStreamStateChangeConfirmationEvent(AUD_Stream_State_Change_Confirmation_Data_t *StreamStateChangeConfirmationData);
static void ProcessStreamFormatChangeIndicationEvent(AUD_Stream_Format_Change_Indication_Data_t *StreamFormatChangeIndicationData);
static void ProcessStreamFormatChangeConfirmationEvent(AUD_Stream_Format_Change_Confirmation_Data_t *StreamFormatChangeConfirmationData);
static void ProcessRemoteControlCommandIndicationEvent(AUDM_AUD_Remote_Control_Command_Indication_Data_t *AUDMRemoteControlCommandIndicationData);
static void ProcessRemoteControlCommandConfirmationEvent(AUDM_AUD_Remote_Control_Command_Confirmation_Data_t *AUDMRemoteControlCommandConfirmationData);
static void ProcessRemoteControlOpenIndicationEvent(AUD_Remote_Control_Open_Indication_Data_t *RemoteControlOpenIndicationData);
static void ProcessRemoteControlOpenConfirmationEvent(AUD_Remote_Control_Open_Confirmation_Data_t *RemoteControlOpenConfirmationData);
static void ProcessRemoteControlCloseIndicationEvent(AUD_Remote_Control_Close_Indication_Data_t *RemoteControlCloseIndicationData);
static void ProcessEncodedAudioDataIndicationEvent(AUD_Encoded_Audio_Data_Indication_Data_t *EncodedAudioDataIndicatioData);
static void ProcessBrowsingChannelOpenIndicationEvent(AUD_Browsing_Channel_Open_Indication_Data_t *BrowsingChannelOpenIndication);
static void ProcessBrowsingChannelOpenConfirmationEvent(AUD_Browsing_Channel_Open_Confirmation_Data_t *BrowsingChannelOpenConfirmation);
static void ProcessBrowsingChannelCloseIndicationEvent(AUD_Browsing_Channel_Close_Indication_Data_t *BrowsingChannelCloseIndication);

static void ProcessAudioEvent(AUDM_AUD_Event_Data_t *AUDEventData);

static void ProcessPowerEvent(Boolean_t Power);

static void BTPSAPI BTPMDispatchCallback_DEVM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_AUDM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_AUD(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI AudioManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the Audio Entry Information List.                            */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

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
static Audio_Entry_Info_t *AddAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, Audio_Entry_Info_t *EntryToAdd)
{
   Audio_Entry_Info_t *AddedEntry = NULL;
   Audio_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->CallbackID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Audio_Entry_Info_t *)BTPS_AllocateMemory(sizeof(Audio_Entry_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                       = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextAudioEntryInfoPtr = NULL;

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
                     FreeAudioEntryInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextAudioEntryInfoPtr)
                        tmpEntry = tmpEntry->NextAudioEntryInfoPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextAudioEntryInfoPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static Audio_Entry_Info_t *SearchAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   Audio_Entry_Info_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the list and Callback ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
         FoundEntry = FoundEntry->NextAudioEntryInfoPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Audio Entry         */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the Audio Entry   */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeAudioEntryInfoEntryMemory().                 */
static Audio_Entry_Info_t *DeleteAudioEntryInfoEntry(Audio_Entry_Info_t **ListHead, unsigned int CallbackID)
{
   Audio_Entry_Info_t *FoundEntry = NULL;
   Audio_Entry_Info_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", CallbackID));

   /* Let's make sure the List and Callback ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (CallbackID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->CallbackID != CallbackID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextAudioEntryInfoPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextAudioEntryInfoPtr = FoundEntry->NextAudioEntryInfoPtr;
         }
         else
            *ListHead = FoundEntry->NextAudioEntryInfoPtr;

         FoundEntry->NextAudioEntryInfoPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Audio Entry Information member. */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeAudioEntryInfoEntryMemory(Audio_Entry_Info_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Audio Entry Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeAudioEntryInfoList(Audio_Entry_Info_t **ListHead)
{
   Audio_Entry_Info_t *EntryToFree;
   Audio_Entry_Info_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextAudioEntryInfoPtr;

         if((!tmpEntry->EventCallbackEntry) && (tmpEntry->ConnectionEvent))
            BTPS_CloseEvent(tmpEntry->ConnectionEvent);

         FreeAudioEntryInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            RequestType field AND the BD_ADDR field is the same as */
   /*            an entry already in the list.  When this occurs, this  */
   /*            function returns NULL.                                 */
static Incoming_Connection_Entry_t *AddIncomingConnectionEntry(Incoming_Connection_Entry_t **ListHead, Incoming_Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t                    NULL_BD_ADDR;
   Incoming_Connection_Entry_t *AddedEntry = NULL;
   Incoming_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
         AddedEntry = (Incoming_Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Incoming_Connection_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                                = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextIncomingConnectionEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if((COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)) && (tmpEntry->RequestType == AddedEntry->RequestType))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeIncomingConnectionEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextIncomingConnectionEntryPtr)
                        tmpEntry = tmpEntry->NextIncomingConnectionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextIncomingConnectionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Incoming Connection */
   /* Entry List for the specified Request Type AND Bluetooth Device    */
   /* Address.  This function returns NULL if either the Incoming       */
   /* Connection Entry List Head is invalid, the Request Type or        */
   /* Bluetooth Device Address is invalid, or the specified Entry was   */
   /* NOT present in the list.                                          */
static Incoming_Connection_Entry_t *SearchIncomingConnectionEntry(Incoming_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Connection_Request_Type_t RequestType)
{
   BD_ADDR_t                    NULL_BD_ADDR;
   Incoming_Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %s\n", (RequestType == acrStream)?"Audio Stream":"Remote Control"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->RequestType != RequestType)))
         FoundEntry = FoundEntry->NextIncomingConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Incoming Connection */
   /* Entry List for the specified Request Type AND Bluetooth Device    */
   /* Address and removes it from the List.  This function returns NULL */
   /* if either the Incoming Connection Entry List Head is invalid, the */
   /* Request Type or Bluetooth Device Address is invalid, or the       */
   /* specified Entry was NOT present in the list.  The entry returned  */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeIncomingConnectionEntryMemory().                      */
static Incoming_Connection_Entry_t *DeleteIncomingConnectionEntry(Incoming_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Connection_Request_Type_t RequestType)
{
   BD_ADDR_t                    NULL_BD_ADDR;
   Incoming_Connection_Entry_t *FoundEntry = NULL;
   Incoming_Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %s\n", (RequestType == acrStream)?"Audio Stream":"Remote Control"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->RequestType != RequestType)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextIncomingConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextIncomingConnectionEntryPtr = FoundEntry->NextIncomingConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextIncomingConnectionEntryPtr;

         FoundEntry->NextIncomingConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Incoming Connection Information */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeIncomingConnectionEntryMemory(Incoming_Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Incoming Connection Information List.    */
   /* Upon return of this function, the Head Pointer is set to NULL.    */
static void FreeIncomingConnectionEntryList(Incoming_Connection_Entry_t **ListHead)
{
   Incoming_Connection_Entry_t *EntryToFree;
   Incoming_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextIncomingConnectionEntryPtr;

         FreeIncomingConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            RequestType field AND the BD_ADDR field is the same as */
   /*            an entry already in the list.  When this occurs, this  */
   /*            function returns NULL.                                 */
static Control_Connection_Entry_t *AddControlConnectionEntry(Control_Connection_Entry_t **ListHead, Control_Connection_Entry_t *EntryToAdd)
{
   BD_ADDR_t                    NULL_BD_ADDR;
   Control_Connection_Entry_t *AddedEntry = NULL;
   Control_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
         AddedEntry = (Control_Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Control_Connection_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                                = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextControlConnectionEntryPtr = NULL;

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
                     FreeControlConnectionEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextControlConnectionEntryPtr)
                        tmpEntry = tmpEntry->NextControlConnectionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextControlConnectionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Control Connection  */
   /* Entry List for the specified Request Type AND Bluetooth Device    */
   /* Address.  This function returns NULL if either the Control        */
   /* Connection Entry List Head is invalid, the Request Type or        */
   /* Bluetooth Device Address is invalid, or the specified Entry was   */
   /* NOT present in the list.                                          */
static Control_Connection_Entry_t *SearchControlConnectionEntry(Control_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t                   NULL_BD_ADDR;
   Control_Connection_Entry_t *FoundEntry = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)))
         FoundEntry = FoundEntry->NextControlConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Control Connection  */
   /* Entry List for the specified Request Type AND Bluetooth Device    */
   /* Address and removes it from the List.  This function returns NULL */
   /* if either the Control Connection Entry List Head is invalid, the  */
   /* Request Type or Bluetooth Device Address is invalid, or the       */
   /* specified Entry was NOT present in the list.  The entry returned  */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeControlConnectionEntryMemory().                       */
static Control_Connection_Entry_t *DeleteControlConnectionEntry(Control_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   BD_ADDR_t                   NULL_BD_ADDR;
   Control_Connection_Entry_t *FoundEntry = NULL;
   Control_Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_BD_ADDR(NULL_BD_ADDR, BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextControlConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextControlConnectionEntryPtr = FoundEntry->NextControlConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextControlConnectionEntryPtr;

         FoundEntry->NextControlConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Control Connection Information  */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeControlConnectionEntryMemory(Control_Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Control Connection Information List.     */
   /* Upon return of this function, the Head Pointer is set to NULL.    */
static void FreeControlConnectionEntryList(Control_Connection_Entry_t **ListHead)
{
   Control_Connection_Entry_t *EntryToFree;
   Control_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextControlConnectionEntryPtr;

         FreeControlConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            StreamType field AND the BD_ADDR field is the same as  */
   /*            an entry already in the list.  When this occurs, this  */
   /*            function returns NULL.                                 */
static Audio_Connection_Entry_t *AddAudioConnectionEntry(Audio_Connection_Entry_t **ListHead, Audio_Connection_Entry_t *EntryToAdd)
{
   Audio_Connection_Entry_t *AddedEntry   = NULL;
   Audio_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {

      /* Make sure that the element that we are adding seems semi-valid.*/
      if(!COMPARE_NULL_BD_ADDR(EntryToAdd->BD_ADDR))
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (Audio_Connection_Entry_t *)BTPS_AllocateMemory(sizeof(Audio_Connection_Entry_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                                = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextAudioConnectionEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if((COMPARE_BD_ADDR(tmpEntry->BD_ADDR, AddedEntry->BD_ADDR)) && (tmpEntry->StreamType == EntryToAdd->StreamType))
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeAudioConnectionEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextAudioConnectionEntryPtr)
                        tmpEntry = tmpEntry->NextAudioConnectionEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextAudioConnectionEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", AddedEntry));

   return(AddedEntry);
}

   /* The following function searches the specified Audio Connection    */
   /* Entry List for the specified Stream Type AND Bluetooth Device     */
   /* Address.  This function returns NULL if either the Audio          */
   /* Connection Entry List Head is invalid, the Stream Type or         */
   /* Bluetooth Device Address is invalid, or the specified Entry was   */
   /* NOT present in the list.                                          */
static Audio_Connection_Entry_t *SearchAudioConnectionEntry(Audio_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Stream_Type_t StreamType)
{
   Audio_Connection_Entry_t *FoundEntry   = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->StreamType != StreamType)))
         FoundEntry = FoundEntry->NextAudioConnectionEntryPtr;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* The following function searches the specified Audio Connection    */
   /* Entry List for the specified Stream Type AND Bluetooth Device     */
   /* Address and removes it from the List.  This function returns      */
   /* NULL if either the Audio Connection Entry List Head is invalid,   */
   /* the Stream Type or Bluetooth Device Address is invalid, or the    */
   /* specified Entry was NOT present in the list.  The entry returned  */
   /* will have the Next Entry field set to NULL, and the caller is     */
   /* responsible for deleting the memory associated with this entry by */
   /* calling FreeAudioConnectionEntryMemory().                         */
static Audio_Connection_Entry_t *DeleteAudioConnectionEntry(Audio_Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR, AUD_Stream_Type_t StreamType)
{
   Audio_Connection_Entry_t *FoundEntry = NULL;
   Audio_Connection_Entry_t *LastEntry  = NULL;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's make sure the list and Bluetooth Device Address to search   */
   /* for appear to be valid.                                           */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && ((!COMPARE_BD_ADDR(FoundEntry->BD_ADDR, BD_ADDR)) || (FoundEntry->StreamType != StreamType)))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextAudioConnectionEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextAudioConnectionEntryPtr = FoundEntry->NextAudioConnectionEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextAudioConnectionEntryPtr;

         FoundEntry->NextAudioConnectionEntryPtr = NULL;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", FoundEntry));

   return(FoundEntry);
}

   /* This function frees the specified Audio Connection Information    */
   /* member.  No check is done on this entry other than making sure it */
   /* NOT NULL.                                                         */
static void FreeAudioConnectionEntryMemory(Audio_Connection_Entry_t *EntryToFree)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(EntryToFree)
      BTPS_FreeMemory(EntryToFree);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Audio Connection Information List.  Upon */
   /* return of this function, the Head Pointer is set to NULL.         */
static void FreeAudioConnectionEntryList(Audio_Connection_Entry_t **ListHead)
{
   Audio_Connection_Entry_t *EntryToFree;
   Audio_Connection_Entry_t *tmpEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextAudioConnectionEntryPtr;

         FreeAudioConnectionEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified Audio event to every registered Audio Event*/
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the Audio Manager    */
   /*          Lock held.  Upon exit from this function it will free    */
   /*          the Audio Manager Lock.                                  */
static void DispatchAudioEvent(AUDM_Event_Data_t *AUDMEventData, BTPM_Message_t *Message)
{
   unsigned int        ServerID;
   unsigned int        NumberCallbacks;
   Callback_Info_t     CallbackInfoArray[16];
   Callback_Info_t    *CallbackInfoArrayPtr;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((AudioEntryInfoList) || (AudioEntryInfoDataList) || (OutgoingRemoteControlList) || (AudioEntryInfoRemoteControlList)) && (AUDMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      AudioEntryInfo  = AudioEntryInfoList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(AudioEntryInfo)
      {
         if((AudioEntryInfo->EventCallback) || (AudioEntryInfo->ClientID != ServerID))
            NumberCallbacks++;

         AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* We need to add the Audio Data Entry Information List as well.  */
      AudioEntryInfo = AudioEntryInfoDataList;
      while(AudioEntryInfo)
      {
         NumberCallbacks++;

         AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
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
            AudioEntryInfo  = AudioEntryInfoList;
            NumberCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(AudioEntryInfo)
            {
               if((AudioEntryInfo->EventCallback) || (AudioEntryInfo->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = AudioEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = AudioEntryInfo->CallbackParameter;

                  NumberCallbacks++;
               }

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            /* We need to add the Audio Data Entry Information List as  */
            /* well.                                                    */
            AudioEntryInfo = AudioEntryInfoDataList;
            while(AudioEntryInfo)
            {
               CallbackInfoArrayPtr[NumberCallbacks].ClientID          = AudioEntryInfo->ClientID;
               CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = AudioEntryInfo->EventCallback;
               CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = AudioEntryInfo->CallbackParameter;

               NumberCallbacks++;

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            DispatchEventList(AUDMEventData, Message, NumberCallbacks, CallbackInfoArrayPtr);

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void DispatchRemoteControlEvent(AUDM_Event_Data_t *AUDMEventData, BTPM_Message_t *Message, unsigned int TypeMask)
{
   unsigned int                      ServerID;
   unsigned int                      NumberCallbacks;
   unsigned int                      NumberServerCallbacks;
   Boolean_t                         Error;
   Callback_Info_t                   CallbackInfoArray[16];
   Callback_Info_t                  *CallbackInfoArrayPtr;
   Audio_Entry_Info_t               *AudioEntryInfo;
   RemoteControlDecodeInformation_t  DecodeInformation;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", TypeMask));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((AudioEntryInfoRemoteControlList) && (AUDMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      AudioEntryInfo  = AudioEntryInfoRemoteControlList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(AudioEntryInfo)
      {
         if(((AudioEntryInfo->EventCallback) || (AudioEntryInfo->ClientID != ServerID)) && (AudioEntryInfo->ConnectionStatus & TypeMask))
            NumberCallbacks++;

         AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
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
            AudioEntryInfo        = AudioEntryInfoRemoteControlList;
            NumberCallbacks       = 0;
            NumberServerCallbacks = 0;

            /* First, add the default event handlers.                   */
            while(AudioEntryInfo)
            {
               if(((AudioEntryInfo->EventCallback) || (AudioEntryInfo->ClientID != ServerID)) && (AudioEntryInfo->ConnectionStatus & TypeMask))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = AudioEntryInfo->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = AudioEntryInfo->CallbackParameter;

                  NumberCallbacks++;

                  /* Check if this is a server callback.                */
                  if(AudioEntryInfo->EventCallback)
                  {
                     /* This is a sever callback, increment the count of*/
                     /* server callbacks.                               */
                     NumberServerCallbacks++;
                  }
               }

               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            Error = FALSE;

            /* Check if there were any server callbacks and if this is a*/
            /* remote control command or a remote control response.     */
            if((NumberServerCallbacks) && ((AUDMEventData->EventType == aetRemoteControlCommandIndication) || (AUDMEventData->EventType == aetRemoteControlCommandConfirmation)))
            {
               /* There were server callbacks and it is a remote control*/
               /* command or a response, check if this a remote control */
               /* command.                                              */
               if(AUDMEventData->EventType == aetRemoteControlCommandIndication)
               {
                  /* This is a remote control command, convert the      */
                  /* stream to a decoded Remote Control Command         */
                  /* Structure.                                         */
                  if(ConvertStreamAVRCPCommandToDecoded(AUDMEventData->EventData.RemoteControlCommandIndicationEventData.RemoteControlCommandData.MessageType, ((AUDM_Remote_Control_Command_Received_Message_t *)Message)->MessageDataLength, ((AUDM_Remote_Control_Command_Received_Message_t *)Message)->MessageData, &DecodeInformation, &(AUDMEventData->EventData.RemoteControlCommandIndicationEventData.RemoteControlCommandData)))
                  {
                     /* An error occurred while decoding the stream,    */
                     /* flag the error.                                 */
                     Error = TRUE;
                  }
               }
               else
               {
                  /* Check if this is a successful command confirmation.*/
                  if(!(AUDMEventData->EventData.RemoteControlCommandConfirmationEventData.Status))
                  {
                     /* This is a successful command confirmation and   */
                     /* there were server callbacks, convert the stream */
                     /* to a decoded Remote Control Response Structure. */
                     if(ConvertStreamAVRCPResponseToDecoded(AUDMEventData->EventData.RemoteControlCommandConfirmationEventData.RemoteControlResponseData.MessageType, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->MessageDataLength, ((AUDM_Remote_Control_Command_Status_Message_t *)Message)->MessageData, &DecodeInformation, &(AUDMEventData->EventData.RemoteControlCommandConfirmationEventData.RemoteControlResponseData)))
                     {
                        /* An error occurred while decoding the stream, */
                        /* flag the error.                              */
                        Error = TRUE;
                     }
                  }
               }
            }

            /* Check if any errors occurred above.                      */
            if(!Error)
            {
               /* No errors occurred, dispatch the event.               */
               DispatchEventList(AUDMEventData, Message, NumberCallbacks, CallbackInfoArrayPtr);

               /* Check if there were any server callbacks and if this  */
               /* is a remote control command or a remote control       */
               /* response.                                             */
               if((NumberServerCallbacks) && ((AUDMEventData->EventType == aetRemoteControlCommandIndication) || (AUDMEventData->EventType == aetRemoteControlCommandConfirmation)))
               {
                  /* There were server callbacks and it is a remote     */
                  /* control command or a response, check if this a     */
                  /* remote control command.                            */
                  if(AUDMEventData->EventType == aetRemoteControlCommandIndication)
                  {
                     /* It is a remote control command, free the decoded*/
                     /* command.                                        */
                     FreeAVRCPDecodedCommand(&DecodeInformation);
                  }
                  else
                  {
                     if(!(AUDMEventData->EventData.RemoteControlCommandConfirmationEventData.Status))
                     {
                        /* It is a remote control response, free the    */
                        /* decoded response.                            */
                        FreeAVRCPDecodedResponse(&DecodeInformation);
                     }
                  }
               }
            }

            /* Free any memory that was allocated.                      */
            if(CallbackInfoArrayPtr != CallbackInfoArray)
               BTPS_FreeMemory(CallbackInfoArrayPtr);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void DispatchEventList(AUDM_Event_Data_t *AUDMEventData, BTPM_Message_t *Message, unsigned int NumberCallbacks, Callback_Info_t *CallbackInfoArrayPtr)
{
   unsigned int Index;
   unsigned int Index1;
   unsigned int ServerID;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the parameters are valid.                               */
   if((NumberCallbacks) && (CallbackInfoArrayPtr))
   {
      ServerID = MSG_GetServerAddressID();

      /* Release the Lock because we have already built the Callback    */
      /* Array.                                                         */
      DEVM_ReleaseLock();

      /* Now we are ready to dispatch the callbacks.                    */
      Index = 0;

      while((Index < NumberCallbacks) && (Initialized))
      {
         /* * NOTE * It is possible that we have already                */
         /*          dispatched the event to the client (case would     */
         /*          occur if a single client has registered for Audio  */
         /*          events and Data Events.  To avoid this case we need*/
         /*          to walk the list of previously dispatched events   */
         /*          to check to see if it has already been dispatched  */
         /*          (we need to do this with Client Address ID's for   */
         /*          messages - Event Callbacks are local and therefore */
         /*          unique so we don't have to do this filtering.      */

         /* Determine the type of event that needs to be dispatched.    */
         if(CallbackInfoArrayPtr[Index].ClientID == ServerID)
         {
            /* Go ahead and make the callback.                          */
            /* * NOTE * If the callback was deleted (or new ones        */
            /*          were added, they will not be caught for this    */
            /*          message dispatch).  Under normal operating      */
            /*          circumstances this case shouldn't matter because*/
            /*          these groups aren't really dynamic and are only */
            /*          registered at initialization time.              */
            __BTPSTRY
            {
               if(CallbackInfoArrayPtr[Index].EventCallback)
               {
                  (*CallbackInfoArrayPtr[Index].EventCallback)(AUDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
               }
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Walk the proceeding list and see if we have already      */
            /* dispatched this event to this client.                    */
            for(Index1=0;Index1<Index;Index1++)
            {
               if((CallbackInfoArrayPtr[Index1].ClientID != ServerID) && (CallbackInfoArrayPtr[Index1].ClientID == CallbackInfoArrayPtr[Index].ClientID))
                  break;
            }

            if(Index1 == Index)
            {
               /* Dispatch the Message.                                 */
               Message->MessageHeader.AddressID = CallbackInfoArrayPtr[Index].ClientID;

               MSG_SendMessage(Message);
            }
         }

         Index++;
      }

      /* Re-acquire the Lock.                                           */
      DEVM_AcquireLock();
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function which is used to     */
   /* handle a query of the connected Remote Control devices.  This     */
   /* function returns the number of connected devices formatted into   */
   /* the buffer on success or a negative error code.                   */
static int ProcessQueryAudioConnections(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                       ret_val;
   Audio_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || (TotalNumberConnectedDevices))
   {
      if(TotalNumberConnectedDevices)
         *TotalNumberConnectedDevices = 0;

      /* Initialize the return value to 0.                              */
      ret_val         = 0;

      /* Walk the connection information list.                          */
      ConnectionEntry = AudioConnectionList;
      while(ConnectionEntry)
      {
         /* Check if this is the correct stream and is connected.       */
         if((ConnectionEntry->StreamType == StreamType) && (ConnectionEntry->ConnectionState == scsConnected))
         {
            /* Increment the total count if requested.                  */
            if(TotalNumberConnectedDevices)
               ++(*TotalNumberConnectedDevices);

            if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
            {
                  /* Format the connection into the list.               */
                  *RemoteDeviceAddressList = ConnectionEntry->BD_ADDR;

                  /* Adjust the list counts.                            */
                  --MaximumRemoteDeviceListEntries;
                  ++RemoteDeviceAddressList;
                  ++ret_val;
            }
            else
            {
               /* We have formatted as many entries into buffer as      */
               /* possible so unless the caller wants the total number  */
               /* of connected devices we will exit the loop here.      */
               if(TotalNumberConnectedDevices == NULL)
                  break;
            }
         }

         ConnectionEntry = ConnectionEntry->NextAudioConnectionEntryPtr;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* attempt to create a Remote Control connection to the specified    */
   /* remote device.                                                    */
static int ConnectRemoteControlDevice(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, unsigned int *ConnectionStatus, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   Event_t                     ConnectionEvent;
   unsigned int                CallbackID;
   unsigned long               Flags;
   Audio_Entry_Info_t          AudioEntryInfo;
   Audio_Entry_Info_t         *AudioEntryInfoPtr;
   Control_Connection_Entry_t  ConnectionEntry;
   Control_Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Device is powered on, next, verify that we are not       */
            /* already tracking a connection for the specified          */
            /* connection type.                                         */
            if((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, RemoteDeviceAddress)) == NULL)
            {
               /* Next make sure that there are no incoming Remote      */
               /* Control Connection Requests for this device.          */
               if(!SearchIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteDeviceAddress, acrRemoteControl))
               {
                  /* Initialize the connection entry that we will add   */
                  /* later.                                             */
                  BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Control_Connection_Entry_t));

                  ConnectionEntry.BD_ADDR = RemoteDeviceAddress;

                  /* Attempt to add an entry into the Audio Entry list. */
                  BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

                  AudioEntryInfo.CallbackID          = GetNextCallbackID();
                  AudioEntryInfo.ClientID            = ClientID;
                  AudioEntryInfo.EventCallback       = CallbackFunction;
                  AudioEntryInfo.CallbackParameter   = CallbackParameter;
                  AudioEntryInfo.RemoteControlDevice = RemoteDeviceAddress;

                  if(ConnectionStatus)
                     AudioEntryInfo.ConnectionEvent  = BTPS_CreateEvent(FALSE);

                  /* Make sure that we allocated the Connection Event   */
                  /* successfully if requested.                         */
                  if((!ConnectionStatus) || ((ConnectionStatus) && (AudioEntryInfo.ConnectionEvent)))
                  {
                     /* Attempt to add the tracking entry for this      */
                     /* connection attempt.                             */
                     if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&OutgoingRemoteControlList, &AudioEntryInfo)) != NULL)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Control 0x%08lX\n", ConnectionFlags));

                        /* Next, attempt to open the remote stream.     */
                        if(ConnectionFlags & AUDM_CONNECT_REMOTE_CONTROL_FLAGS_REQUIRE_ENCRYPTION)
                        {
                           ConnectionEntry.ConnectionState = scsConnectingEncrypting;
                           Flags                           = (DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT);
                        }
                        else
                        {
                           if(ConnectionFlags & AUDM_CONNECT_REMOTE_CONTROL_FLAGS_REQUIRE_AUTHENTICATION)
                           {
                              ConnectionEntry.ConnectionState = scsConnectingAuthenticating;
                              Flags                           = DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE;
                           }
                           else
                           {
                              ConnectionEntry.ConnectionState = scsConnecting;
                              Flags                           = 0;
                           }
                        }

                        /* Attempt to bring up the ACL connection for   */
                        /* this device with the specified flags.        */
                        ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, Flags);

                        /* Check to see the status of the connection    */
                        /* attempt.                                     */
                        if((ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                        {
                           /* Set the state to connecting remote stream.*/
                           ConnectionEntry.ConnectionState = scsConnectingStream;

                           /* Attempt to connect the Remote Control     */
                           /* connection.                               */
                           ret_val = _AUDM_Connect_Remote_Control(RemoteDeviceAddress);
                        }

                        /* Go ahead and attempt to add the connection   */
                        /* entry for this device.                       */
                        if(!AddControlConnectionEntry(&RemoteControlConnectionList, &ConnectionEntry))
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

                        /* Next, determine if the caller has requested a*/
                        /* blocking open.                               */
                        if((!ret_val) && (ConnectionStatus))
                        {
                           /* Blocking open, go ahead and wait for the  */
                           /* event.                                    */

                           /* Note the Callback ID.                     */
                           CallbackID      = AudioEntryInfoPtr->CallbackID;

                           /* Note the Open Event.                      */
                           ConnectionEvent = AudioEntryInfoPtr->ConnectionEvent;

                           /* Release the lock because we are finished  */
                           /* with it.                                  */
                           DEVM_ReleaseLock();

                           /* Wait for the connection process to        */
                           /* complete.                                 */
                           BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                           /* Re-acquire the Lock.                      */
                           if(DEVM_AcquireLock())
                           {
                              /* Delete the connection tracking entry   */
                              /* for this device.                       */
                              if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, CallbackID)) != NULL)
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = AudioEntryInfoPtr->ConnectionStatus;

                                 /* Close the event that was used to    */
                                 /* wait on the connection to complete. */
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                 /* Free the memory that was allocated  */
                                 /* for the connection.                 */
                                 FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);

                                 /* Flag success to the caller.         */
                                 ret_val = 0;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_CONTROL;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                        }

                        /* If an error occurred and that error does not */
                        /* indicate that we could lock the required     */
                        /* locks go ahead and delete the entry that was */
                        /* added.                                       */
                        if((ret_val) && (ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT))
                        {
                           /* Delete the connection entry for this      */
                           /* device.                                   */
                           if((ConnectionEntryPtr = DeleteControlConnectionEntry(&RemoteControlConnectionList, ConnectionEntry.BD_ADDR)) != NULL)
                              FreeControlConnectionEntryMemory(ConnectionEntryPtr);

                           /* Delete the connection tracking entry for  */
                           /* this device.                              */
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo.CallbackID)) != NULL)
                           {
                              /* If we created an event to block on free*/
                              /* the resources allocated for this.      */
                              if(AudioEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              /* Free the memory allocated for this     */
                              /* connection.                            */
                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                           }
                        }
                        else
                        {
                           /* If we are not tracking this connection    */
                           /* (and we are running in the server) go     */
                           /* ahead and delete the entry that was added.*/
                           if((ClientID == MSG_GetServerAddressID()) && (!CallbackFunction) && (!ConnectionStatus) && (!ret_val))
                           {
                              /* Delete the connection tracking entry   */
                              /* for this device.                       */
                              if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo.CallbackID)) != NULL)
                              {
                                 if(AudioEntryInfoPtr->ConnectionEvent)
                                    BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                 FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                              }
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
               }
               else
                  ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_CONNECTION_IN_PROGRESS;
            }
            else
            {
               if(ConnectionEntryPtr->ConnectionState == scsConnected)
                  ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_ALREADY_CONNECTED;
               else
                  ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_CONNECTION_IN_PROGRESS;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* attempt to create a Remote Control connection to the specified    */
   /* remote device.                                                    */
static int ConnectRemoteControlBrowsing(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, unsigned int *ConnectionStatus, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                         ret_val;
   Event_t                     ConnectionEvent;
   unsigned int                CallbackID;
   Audio_Entry_Info_t          AudioEntryInfo;
   Audio_Entry_Info_t         *AudioEntryInfoPtr;
   Control_Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, verify that the input parameters appear to be semi-valid.*/
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Next, check to see if we are powered up.                    */
         if(CurrentPowerState)
         {
            /* Device is powered on, next, verify that we are currently */
            /* connected to this device.                                */
            if(((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, RemoteDeviceAddress)) != NULL) && (ConnectionEntryPtr->ConnectionState == scsConnected))
            {
                  /* Initialize the connection entry that we will add   */
                  /* later.                                             */
                  /* Attempt to add an entry into the Audio Entry list. */
                  BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

                  AudioEntryInfo.CallbackID          = GetNextCallbackID();
                  AudioEntryInfo.ClientID            = ClientID;
                  AudioEntryInfo.EventCallback       = CallbackFunction;
                  AudioEntryInfo.CallbackParameter   = CallbackParameter;
                  AudioEntryInfo.RemoteControlDevice = RemoteDeviceAddress;

                  if(ConnectionStatus)
                     AudioEntryInfo.ConnectionEvent  = BTPS_CreateEvent(FALSE);

                  /* Make sure that we allocated the Connection Event   */
                  /* successfully if requested.                         */
                  if((!ConnectionStatus) || ((ConnectionStatus) && (AudioEntryInfo.ConnectionEvent)))
                  {
                     /* Attempt to add the tracking entry for this      */
                     /* connection attempt.                             */
                     if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&OutgoingRemoteControlList, &AudioEntryInfo)) != NULL)
                     {
                        DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Control 0x%08lX\n", ConnectionFlags));

                        /* Attempt to connect the Remote Control     */
                        /* connection.                               */
                        ret_val = _AUDM_Connect_Remote_Control_Browsing(RemoteDeviceAddress);

                        if(!ret_val)
                        {
                           /* Set the state to connecting remote stream.*/
                           ConnectionEntryPtr->ConnectionState = scsConnectingBrowsing;
                        }

                        /* Next, determine if the caller has requested a*/
                        /* blocking open.                               */
                        if((!ret_val) && (ConnectionStatus))
                        {
                           /* Blocking open, go ahead and wait for the  */
                           /* event.                                    */

                           /* Note the Callback ID.                     */
                           CallbackID      = AudioEntryInfoPtr->CallbackID;

                           /* Note the Open Event.                      */
                           ConnectionEvent = AudioEntryInfoPtr->ConnectionEvent;

                           /* Release the lock because we are finished  */
                           /* with it.                                  */
                           DEVM_ReleaseLock();

                           /* Wait for the connection process to        */
                           /* complete.                                 */
                           BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                           /* Re-acquire the Lock.                      */
                           if(DEVM_AcquireLock())
                           {
                              /* Delete the connection tracking entry   */
                              /* for this device.                       */
                              if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, CallbackID)) != NULL)
                              {
                                 /* Note the connection status.         */
                                 *ConnectionStatus = AudioEntryInfoPtr->ConnectionStatus;

                                 /* Close the event that was used to    */
                                 /* wait on the connection to complete. */
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                 /* Free the memory that was allocated  */
                                 /* for the connection.                 */
                                 FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);

                                 /* Flag success to the caller.         */
                                 ret_val = 0;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_CONTROL;
                           }
                           else
                              ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                        }

                        /* If an error occurred and that error does not */
                        /* indicate that we could lock the required     */
                        /* locks go ahead and delete the entry that was */
                        /* added.                                       */
                        if((ret_val) && (ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT))
                        {
                           /* Delete the connection tracking entry for  */
                           /* this device.                              */
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo.CallbackID)) != NULL)
                           {
                              /* If we created an event to block on free*/
                              /* the resources allocated for this.      */
                              if(AudioEntryInfoPtr->ConnectionEvent)
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              /* Free the memory allocated for this     */
                              /* connection.                            */
                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                           }
                        }
                        else
                        {
                           /* If we are not tracking this connection    */
                           /* (and we are running in the server) go     */
                           /* ahead and delete the entry that was added.*/
                           if((ClientID == MSG_GetServerAddressID()) && (!CallbackFunction) && (!ConnectionStatus) && (!ret_val))
                           {
                              /* Delete the connection tracking entry   */
                              /* for this device.                       */
                              if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo.CallbackID)) != NULL)
                              {
                                 if(AudioEntryInfoPtr->ConnectionEvent)
                                    BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                 FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                              }
                           }
                        }
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
            }
            else
            {
               if(ConnectionEntryPtr)
               {
                  if(ConnectionEntryPtr->ConnectionState == scsConnected)
                     ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_ALREADY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_CONNECTION_IN_PROGRESS;
               }
               else
                  ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_NOT_CONNECTED;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;
      }
      else
         ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* handle a query of the connected Remote Control devices.  This     */
   /* function returns the number of connected devices formatted into   */
   /* the buffer on success or a negative error code.                   */
static int ProcessQueryRemoteControlConnections(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                         ret_val;
   Control_Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   if(((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)) || (TotalNumberConnectedDevices))
   {
      if(TotalNumberConnectedDevices)
         *TotalNumberConnectedDevices = 0;

      /* Initialize the return value to 0.                              */
      ret_val         = 0;

      /* Walk the connection information list.                          */
      ConnectionEntry = RemoteControlConnectionList;
      while(ConnectionEntry)
      {
         /* Increment the total count if requested.                     */
         if(TotalNumberConnectedDevices)
            ++(*TotalNumberConnectedDevices);

         if((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList))
         {
            /* Check to see if we are fully connected to this device.   */
            if(ConnectionEntry->ConnectionState == scsConnected)
            {
               /* Format the connection into the list.                  */
               *RemoteDeviceAddressList = ConnectionEntry->BD_ADDR;

               /* Adjust the list counts.                               */
               --MaximumRemoteDeviceListEntries;
               ++RemoteDeviceAddressList;
               ++ret_val;
            }
         }
         else
         {
            /* We have formatted as many entries into buffer as possible*/
            /* so unless the caller wants the total number of connected */
            /* devices we will exit the loop here.                      */
            if(TotalNumberConnectedDevices == NULL)
               break;
         }

         ConnectionEntry = ConnectionEntry->NextControlConnectionEntryPtr;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function processes the specified Audio Stream or    */
   /* Remote Control Connect Response Message and responds to the       */
   /* message accordingly.  This function does not verify the integrity */
   /* of the Message (i.e. the length) because it is the caller's       */
   /* responsibility to verify the Message before calling this function.*/
static void ProcessConnectionResponseMessage(AUDM_Connection_Request_Response_Request_t *Message)
{
   int                                          Result;
   Boolean_t                                    Authenticate;
   Boolean_t                                    Encrypt;
   Incoming_Connection_Entry_t                 *IncomingConnectionEntry;
   AUDM_Connection_Request_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are already      */
         /* tracking a connection for the specified connection type.    */
         if(((IncomingConnectionEntry = SearchIncomingConnectionEntry(&IncomingConnectionEntryList, Message->RemoteDeviceAddress, Message->RequestType)) != NULL) && (IncomingConnectionEntry->IncomingConnectionState == icsAuthorizing))
         {
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %s, %d\n", ((Message->RequestType == acrStream)?"Audio Stream":"Remote Control"), Message->Accept));

            /* If the caller has accepted the request then we need to   */
            /* process it differently.                                  */
            if(Message->Accept)
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(IncomingConnectionEntry->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(IncomingConnectionEntry->BD_ADDR, 0);
               }
               else
                  Result = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

               if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
               {
                  /* Authorization not required, and we are already in  */
                  /* the correct state.                                 */
                  Result = _AUDM_Connection_Request_Response(Message->RequestType, IncomingConnectionEntry->BD_ADDR, TRUE);

                  /* Go ahead and delete the entry because we are       */
                  /* finished with tracking it.                         */
                  if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, Message->RemoteDeviceAddress, Message->RequestType)) != NULL)
                     FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
               }
               else
               {
                  /* If we were successfully able to Authenticate and/or*/
                  /* Encrypt, then we need to set the correct state.    */
                  if(!Result)
                  {
                     if(Encrypt)
                        IncomingConnectionEntry->IncomingConnectionState = icsEncrypting;
                     else
                        IncomingConnectionEntry->IncomingConnectionState = icsAuthenticating;

                     /* Flag success.                                   */
                     Result = 0;
                  }
                  else
                  {
                     /* Error, reject the request.                      */
                     _AUDM_Connection_Request_Response(Message->RequestType, IncomingConnectionEntry->BD_ADDR, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, Message->RemoteDeviceAddress, Message->RequestType)) != NULL)
                        FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
                  }
               }
            }
            else
            {
               /* Rejection - Simply respond to the request.            */
               Result = _AUDM_Connection_Request_Response(Message->RequestType, IncomingConnectionEntry->BD_ADDR, FALSE);

               /* Go ahead and delete the entry because we are finished */
               /* with tracking it.                                     */
               if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, Message->RemoteDeviceAddress, Message->RequestType)) != NULL)
                  FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
            }
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CONNECTION_STATE;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Connect Audio      */
   /* Stream Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessConnectAudioStreamMessage(AUDM_Connect_Audio_Stream_Request_t *Message)
{
   int                                   Result;
   Audio_Entry_Info_t                    AudioEntryInfo;
   Audio_Entry_Info_t                   *AudioEntryInfoPtr;
   Audio_Connection_Entry_t              AudioConnectionEntry;
   Audio_Connection_Entry_t             *AudioConnectionEntryPtr;
   AUDM_Connect_Audio_Stream_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Next, check to see if we are powered up.                       */
      if(CurrentPowerState)
      {
         /* Device is powered on, next, verify that we are not already  */
         /* tracking a connection for the specified connection type.    */
         if((AudioConnectionEntryPtr = SearchAudioConnectionEntry(&AudioConnectionList, Message->RemoteDeviceAddress, Message->StreamType)) == NULL)
         {
            /* Attempt to add an entry into the Audio Entry list.       */
            BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

            AudioEntryInfo.CallbackID = GetNextCallbackID();
            AudioEntryInfo.ClientID   = Message->MessageHeader.AddressID;
            AudioEntryInfo.StreamType = Message->StreamType;

            if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&AudioEntryInfoList, &AudioEntryInfo)) != NULL)
            {
               /* Add the connection entry.                             */
               BTPS_MemInitialize(&AudioConnectionEntry, 0, sizeof(Audio_Connection_Entry_t));

               AudioConnectionEntry.BD_ADDR    = Message->RemoteDeviceAddress;
               AudioConnectionEntry.StreamType = Message->StreamType;

               if((AudioConnectionEntryPtr = AddAudioConnectionEntry(&AudioConnectionList, &AudioConnectionEntry)) != NULL)
               {
                  DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Stream %s, 0x%08lX\n", (Message->StreamType == astSRC)?"Sink":"Source", Message->StreamFlags));

                  /* Next, attempt to open the remote stream.           */
                  if(Message->StreamFlags & AUDM_CONNECT_AUDIO_STREAM_FLAGS_REQUIRE_ENCRYPTION)
                     AudioConnectionEntryPtr->ConnectionState = scsConnectingEncrypting;
                  else
                  {
                     if(Message->StreamFlags & AUDM_CONNECT_AUDIO_STREAM_FLAGS_REQUIRE_AUTHENTICATION)
                        AudioConnectionEntryPtr->ConnectionState = scsConnectingAuthenticating;
                     else
                        AudioConnectionEntryPtr->ConnectionState = scsConnecting;
                  }

                  Result = DEVM_ConnectWithRemoteDevice(Message->RemoteDeviceAddress, (AudioConnectionEntryPtr->ConnectionState == scsConnecting)?0:((AudioConnectionEntryPtr->ConnectionState == scsConnectingEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                  if((Result >= 0) || (Result == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     /* Check to see if we need to actually issue the   */
                     /* Remote Stream connection.                       */
                     if(Result < 0)
                     {
                        /* Set the state to connecting remote stream.   */
                        AudioConnectionEntryPtr->ConnectionState = scsConnectingStream;

                        if((Result = _AUDM_Connect_Audio_Stream(Message->RemoteDeviceAddress, Message->StreamType)) != 0)
                        {
                           if((Result != BTPM_ERROR_CODE_AUDIO_STREAM_ALREADY_CONNECTED) && (Result != BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS))
                              Result = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_AUDIO_STREAM;

                           /* Delete the connection entry.              */
                           if((AudioConnectionEntryPtr = DeleteAudioConnectionEntry(&AudioConnectionList, Message->RemoteDeviceAddress, Message->StreamType)) != NULL)
                              FreeAudioConnectionEntryMemory(AudioConnectionEntryPtr);

                           /* Error opening stream, go ahead and delete */
                           /* the entry that was added.                 */
                           if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfoPtr->CallbackID)) != NULL)
                           {
                              if((!AudioEntryInfoPtr->EventCallbackEntry) && (AudioEntryInfoPtr->ConnectionEvent))
                                 BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                              FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                           }
                        }
                     }
                  }
                  else
                  {
                     if(Result < 0)
                     {
                        /* Delete the connection entry.                 */
                        if((AudioConnectionEntryPtr = DeleteAudioConnectionEntry(&AudioConnectionList, Message->RemoteDeviceAddress, Message->StreamType)) != NULL)
                           FreeAudioConnectionEntryMemory(AudioConnectionEntryPtr);
                     }
                  }

               }
               else
                  Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
         }
         else
         {
            if(AudioConnectionEntryPtr->ConnectionState == scsConnected)
               Result = BTPM_ERROR_CODE_AUDIO_STREAM_ALREADY_CONNECTED;
            else
               Result = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CONNECT_AUDIO_STREAM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Disconnect Audio   */
   /* Stream Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessDisconnectAudioStreamMessage(AUDM_Disconnect_Audio_Stream_Request_t *Message)
{
   int                                     Result;
   AUD_Stream_Close_Indication_Data_t      StreamCloseIndicationData;
   AUDM_Disconnect_Audio_Stream_Response_t ResponseMessage;

   //DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %02X:%02X:%02X:%02X:%02X:%02X\n", Message->RemoteDeviceAddress.BD_ADDR5, Message->RemoteDeviceAddress.BD_ADDR4, Message->RemoteDeviceAddress.BD_ADDR3, Message->RemoteDeviceAddress.BD_ADDR2, Message->RemoteDeviceAddress.BD_ADDR1, Message->RemoteDeviceAddress.BD_ADDR0));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to close Stream %d\n", (unsigned int)Message->StreamType));

      /* Nothing really to do other than to Disconnect the Audio Stream */
      /* (if it is connected, a disconnect will be dispatched from the  */
      /* framework).                                                    */
      Result                                       = _AUDM_Disconnect_Audio_Stream(Message->RemoteDeviceAddress, Message->StreamType);

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_DISCONNECT_AUDIO_STREAM_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);

      /* If the result was successful, we need to make sure we clean up */
      /* everything and dispatch the event to all registered clients.   */
      if(!Result)
      {
         /* Fake a Stream Close Event to dispatch to all registered     */
         /* clients that the Stream has been closed.                    */
         StreamCloseIndicationData.StreamType       = Message->StreamType;
         StreamCloseIndicationData.DisconnectReason = adrRemoteDeviceDisconnect;
         StreamCloseIndicationData.BD_ADDR          = Message->RemoteDeviceAddress;

         ProcessStreamCloseIndicationEvent(&StreamCloseIndicationData);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Connected    */
   /* Remote Control Devices Message and responds to the message        */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
static void ProcessQueryAudioConnectedDevicesMessage(AUDM_Query_Audio_Connected_Devices_Request_t *Message)
{
   int                                            Result;
   unsigned int                                   TotalConnectedDevices;
   AUDM_Query_Audio_Connected_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First determine how many active connections there are.         */
      Result = ProcessQueryAudioConnections(Message->StreamType, 0, NULL, &TotalConnectedDevices);

      if(Result >= 0)
      {
         /* Allocate memory to hold the response.                       */
         if((ResponseMessage = (AUDM_Query_Audio_Connected_Devices_Response_t *)BTPS_AllocateMemory(AUDM_QUERY_AUDIO_CONNECTED_DEVICES_RESPONSE_SIZE(TotalConnectedDevices))) != NULL)
         {
            /* Query the list of active connections.                    */
            if(TotalConnectedDevices)
               Result = ProcessQueryAudioConnections(Message->StreamType, TotalConnectedDevices, ResponseMessage->DeviceConnectedList, NULL);
            else
               Result = 0;

            if(Result >= 0)
            {
               TotalConnectedDevices = (unsigned int)Result;

               Result                = 0;
            }
            else
            {
               /* Error occurred so just so we returned 0.              */
               TotalConnectedDevices = 0;
            }

            /* Format and send the response.                            */
            ResponseMessage->MessageHeader                = Message->MessageHeader;
            ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
            ResponseMessage->MessageHeader.MessageLength  = (AUDM_QUERY_AUDIO_CONNECTED_DEVICES_RESPONSE_SIZE(TotalConnectedDevices) - BTPM_MESSAGE_HEADER_SIZE);

            ResponseMessage->Status                       = Result;

            ResponseMessage->NumberDevicesConnected       = TotalConnectedDevices;

            MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

            /* Since we have already sent the response message we may go*/
            /* ahead and free the memory allocated to hold the Response.*/
            BTPS_FreeMemory(ResponseMessage);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Audio Stream */
   /* State Message and responds to the message accordingly.  This      */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessQueryAudioStreamStateMessage(AUDM_Query_Audio_Stream_State_Request_t *Message)
{
   int                                      Result;
   AUDM_Query_Audio_Stream_State_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Query the Audio Stream State.                               */
         Result = _AUDM_Query_Audio_Stream_State(Message->RemoteDeviceAddress, Message->StreamType, &(ResponseMessage.StreamState));
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_QUERY_AUDIO_STREAM_STATE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Audio Stream */
   /* Format Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessQueryAudioStreamFormatMessage(AUDM_Query_Audio_Stream_Format_Request_t *Message)
{
   int                                       Result;
   AUDM_Query_Audio_Stream_Format_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Query the Audio Stream Format.                              */
         Result = _AUDM_Query_Audio_Stream_Format(Message->RemoteDeviceAddress, Message->StreamType, &(ResponseMessage.StreamFormat));
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_QUERY_AUDIO_STREAM_FORMAT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Change Audio Stream*/
   /* State Message and responds to the message accordingly.  This      */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessChangeAudioStreamStateMessage(AUDM_Change_Audio_Stream_State_Request_t *Message)
{
   int                                       Result;
   AUDM_Change_Audio_Stream_State_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Start the Audio Stream.                                     */
         Result = _AUDM_Change_Audio_Stream_State(Message->RemoteDeviceAddress, Message->StreamType, Message->StreamState);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CHANGE_AUDIO_STREAM_STATE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Change Audio Stream*/
   /* Format Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessChangeAudioStreamFormatMessage(AUDM_Change_Audio_Stream_Format_Request_t *Message)
{
   int                                        Result;
   AUDM_Change_Audio_Stream_Format_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Start the Audio Stream.                                     */
         Result = _AUDM_Change_Audio_Stream_Format(Message->RemoteDeviceAddress, Message->StreamType, &(Message->StreamFormat));
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CHANGE_AUDIO_STREAM_FORMAT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Audio Stream */
   /* Configuration Message and responds to the message accordingly.    */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessQueryAudioStreamConfigurationMessage(AUDM_Query_Audio_Stream_Configuration_Request_t *Message)
{
   int                                              Result;
   AUDM_Query_Audio_Stream_Configuration_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Query the Audio Stream Format.                              */
         Result = _AUDM_Query_Audio_Stream_Configuration(Message->RemoteDeviceAddress, Message->StreamType, &(ResponseMessage.StreamConfiguration));
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_QUERY_AUDIO_STREAM_CONFIGURATION_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Change Incoming    */
   /* Connection Flags Message and responds to the message accordingly. */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessChangeIncomingConnectionFlagsMessage(AUDM_Change_Incoming_Connection_Flags_Request_t *Message)
{
   int                                              Result;
   AUDM_Change_Incoming_Connection_Flags_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Encoded Audio */
   /* Data Message and responds to the message accordingly.  This       */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendEncodedAudioDataMessage(AUDM_Send_Encoded_Audio_Data_Request_t *Message)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* First, find the local handler.                              */
         if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, Message->StreamEventsHandlerID)) != NULL)
         {
            /* Double check that the type is an Audio Source.           */
            if(AudioEntryInfo->StreamType == astSRC)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the Encoded Audio Data.              */
               /* * NOTE * There is no response to this message.        */
               _AUDM_Send_Encoded_Audio_Data(Message->RemoteDeviceAddress, Message->RawAudioDataFrameLength, Message->RawAudioDataFrame);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send RTP Encoded   */
   /* Audio Data Message and responds to the message accordingly.  This */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendRTPEncodedAudioDataMessage(AUDM_Send_RTP_Encoded_Audio_Data_Request_t *Message)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* First, find the local handler.                              */
         if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, Message->StreamEventsHandlerID)) != NULL)
         {
            /* Double check that the type is an Audio Source.           */
            if(AudioEntryInfo->StreamType == astSRC)
            {
               /* Nothing to do here other than to call the actual      */
               /* function to send the Encoded Audio Data.              */
               /* * NOTE * There is no response to this message.        */
               _AUDM_Send_RTP_Encoded_Audio_Data(Message->RemoteDeviceAddress, Message->RawAudioDataFrameLength, Message->RawAudioDataFrame, Message->Flags, &Message->RTPHeaderInfo);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Connect Remote     */
   /* Control request and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessConnectRemoteControlMessage(AUDM_Connect_Remote_Control_Request_t *Message)
{
   int                                    Result;
   AUDM_Connect_Remote_Control_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Simply wrap the internal function to do all of the work.       */
      Result = ConnectRemoteControlDevice(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->ConnectionFlags, NULL, NULL, NULL);

      /* Format up the response message and send it.                    */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CONNECT_REMOTE_CONTROL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));

}

   /* The following function processes the specified Disconnect Remote  */
   /* Control request and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e.  the  */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessDisconnectRemoteControlMessage(AUDM_Disconnect_Remote_Control_Request_t *Message)
{
   int                                        Result;
   AUDM_Disconnect_Remote_Control_Response_t  ResponseMessage;
   AUD_Remote_Control_Close_Indication_Data_t CloseIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Disconnect the Remote Control.                              */
         Result = _AUDM_Disconnect_Remote_Control(Message->RemoteDeviceAddress);

         /* If the result was successful, we need to make sure we clean */
         /* up everything and dispatch the event to all registered      */
         /* clients.                                                    */
         if(!Result)
         {
            /* Fake a Close Event to dispatch to all registered clients */
            /* that the Stream has been closed.                         */
            CloseIndicationData.BD_ADDR          = Message->RemoteDeviceAddress;
            CloseIndicationData.DisconnectReason = adrRemoteDeviceDisconnect;

            ProcessRemoteControlCloseIndicationEvent(&CloseIndicationData);
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;


      /* Format up the response message and send it.                    */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_DISCONNECT_REMOTE_CONTROL_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Query Connected    */
   /* Remote Control Devices Message and responds to the message        */
   /* accordingly.  This function does not verify the integrity of the  */
   /* Message (i.e.  the length) because it is the caller's             */
   /* responsibility to verify the Message before calling this function.*/
static void ProcessQueryRemoteControlConnectedDevicesMessage(AUDM_Query_Remote_Control_Connected_Devices_Request_t *Message)
{
   int                                                     Result;
   unsigned int                                            TotalConnectedDevices;
   AUDM_Query_Remote_Control_Connected_Devices_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First determine how many active connections there are.         */
      Result = ProcessQueryRemoteControlConnections(0, NULL, &TotalConnectedDevices);
      if(Result >= 0)
      {
         /* Allocate memory to hold the response.                       */
         if((ResponseMessage = BTPS_AllocateMemory(AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_RESPONSE_SIZE(TotalConnectedDevices))) != NULL)
         {
            /* Query the list of active connections.                    */
            if(TotalConnectedDevices)
               Result = ProcessQueryRemoteControlConnections(TotalConnectedDevices, ResponseMessage->DeviceConnectedList, NULL);
            else
               Result = 0;

            if(Result >= 0)
            {
               TotalConnectedDevices = (unsigned int)Result;

               Result                = 0;
            }
            else
            {
               /* Error occurred so just so we returned 0.              */
               TotalConnectedDevices = 0;
            }

            /* Format and send the response.                            */
            ResponseMessage->MessageHeader                = Message->MessageHeader;
            ResponseMessage->MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
            ResponseMessage->MessageHeader.MessageLength  = (AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_RESPONSE_SIZE(TotalConnectedDevices) - BTPM_MESSAGE_HEADER_SIZE);

            ResponseMessage->Status                       = Result;

            ResponseMessage->NumberDevicesConnected       = TotalConnectedDevices;

            MSG_SendMessage((BTPM_Message_t *)ResponseMessage);

            /* Since we have already sent the response message we may go*/
            /* ahead and free the memory allocated to hold the Response.*/
            BTPS_FreeMemory(ResponseMessage);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Remote Control*/
   /* Command Message and responds to the message accordingly.  This    */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendRemoteControlCommandMessage(AUDM_Send_Remote_Control_Command_Request_t *Message)
{
   int                                          Result;
   Audio_Entry_Info_t                          *AudioEntryInfo;
   AUD_Remote_Control_Command_Data_t            CommandData;
   RemoteControlDecodeInformation_t             DecodeInformation;
   AUDM_Send_Remote_Control_Command_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* First, find the local handler.                              */
         if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, Message->RemoteControlEventsHandlerID)) != NULL)
         {
            /* Double check that the type is supported.                 */
            if(AudioEntryInfo->ConnectionStatus & AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER)
            {
               /* Convert the message from it's stream format to the    */
               /* format required for the real Audio Manager (AVRCP     */
               /* Parsed).                                              */
               if(!ConvertStreamAVRCPCommandToDecoded(Message->MessageType, Message->MessageDataLength, Message->MessageData, &DecodeInformation, &CommandData))
               {
                  /* Call the actual function to send the Remote Control*/
                  /* Data.                                              */
                  Result = _AUDM_Send_Remote_Control_Command(Message->RemoteDeviceAddress, Message->ResponseTimeout, &CommandData);

                  // todo: comments.
                  FreeAVRCPDecodedCommand(&DecodeInformation);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
            }
            else
               Result = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_SEND_REMOTE_CONTROL_COMMAND_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.Status        = 0;
         ResponseMessage.TransactionID = (unsigned int)Result;
      }
      else
      {
         ResponseMessage.Status        = Result;
         ResponseMessage.TransactionID = 0;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Send Remote Control*/
   /* Response Message and responds to the message accordingly.  This   */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessSendRemoteControlResponseMessage(AUDM_Send_Remote_Control_Response_Request_t *Message)
{
   int                                           Result;
   Audio_Entry_Info_t                           *AudioEntryInfo;
   RemoteControlDecodeInformation_t              DecodeInformation;
   AUD_Remote_Control_Response_Data_t            ResponseData;
   AUDM_Send_Remote_Control_Response_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* First, find the local handler.                              */
         if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, Message->RemoteControlEventsHandlerID)) != NULL)
         {
            /* Double check that the type is supported.                 */
            if(AudioEntryInfo->ConnectionStatus & AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET)
            {
               /* Convert the message from it's stream format to the    */
               /* format required for the real Audio Manager (AVRCP     */
               /* Parsed).                                              */
               if(!ConvertStreamAVRCPResponseToDecoded(Message->MessageType, Message->MessageDataLength, Message->MessageData, &DecodeInformation, &ResponseData))
               {
                  /* Call the actual function to send the Remote Control*/
                  /* Data.                                              */
                  Result = _AUDM_Send_Remote_Control_Response(Message->RemoteDeviceAddress, Message->TransactionID, &ResponseData);

                  FreeAVRCPDecodedResponse(&DecodeInformation);
               }
               else
                  Result = BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA;
            }
            else
               Result = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
         }
         else
            Result = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_SEND_REMOTE_CONTROL_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Audio     */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessRegisterAudioStreamEventsMessage(AUDM_Register_Audio_Stream_Events_Request_t *Message)
{
   int                                           Result;
   Audio_Entry_Info_t                            AudioEntryInfo;
   AUDM_Register_Audio_Stream_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Attempt to add an entry into the Audio Entry list.             */
      BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

      AudioEntryInfo.CallbackID         = GetNextCallbackID();
      AudioEntryInfo.ClientID           = Message->MessageHeader.AddressID;
      AudioEntryInfo.EventCallbackEntry = TRUE;

      if(AddAudioEntryInfoEntry(&AudioEntryInfoList, &AudioEntryInfo))
         Result = AudioEntryInfo.CallbackID;
      else
         Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_REGISTER_AUDIO_STREAM_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.StreamEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status                = 0;
      }
      else
      {
         ResponseMessage.StreamEventsHandlerID = 0;

         ResponseMessage.Status                = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register Audio  */
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterAudioStreamEventsMessage(AUDM_Un_Register_Audio_Stream_Events_Request_t *Message)
{
   int                                              Result;
   Audio_Entry_Info_t                              *AudioEntryInfo;
   AUDM_Un_Register_Audio_Stream_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoList, Message->StreamEventsHandlerID)) != NULL) && (AudioEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Go ahead and process the message request.                   */
         if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, Message->StreamEventsHandlerID)) != NULL)
         {
            /* Free the memory because we are finished with it.         */
            FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

            /* Flag success.                                            */
            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_UN_REGISTER_AUDIO_STREAM_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Audio Data*/
   /* Events Message and responds to the message accordingly.  This     */
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessRegisterAudioStreamDataEventsMessage(AUDM_Register_Audio_Stream_Data_Events_Request_t *Message)
{
   int                                                Result;
   Audio_Entry_Info_t                                 AudioEntryInfo;
   Audio_Entry_Info_t                                *AudioEntryInfoPtr;
   AUDM_Register_Audio_Stream_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Before proceding any further, make sure that there is not      */
      /* already a Data Event Handler for the specified Stream Type.    */
      AudioEntryInfoPtr = AudioEntryInfoDataList;

      while(AudioEntryInfoPtr)
      {
         if(AudioEntryInfoPtr->StreamType == Message->StreamType)
            break;
         else
            AudioEntryInfoPtr = AudioEntryInfoPtr->NextAudioEntryInfoPtr;
      }

      if(!AudioEntryInfoPtr)
      {
         /* First, Register the handler locally.                        */
         BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

         AudioEntryInfo.CallbackID         = GetNextCallbackID();
         AudioEntryInfo.ClientID           = Message->MessageHeader.AddressID;
         AudioEntryInfo.StreamType         = Message->StreamType;
         AudioEntryInfo.EventCallbackEntry = TRUE;

         if(AddAudioEntryInfoEntry(&AudioEntryInfoDataList, &AudioEntryInfo))
         {
            /* Data Handler registered, go ahead and flag success.      */
            Result = AudioEntryInfo.CallbackID;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_AUDIO_STREAM_DATA_ALREADY_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_REGISTER_AUDIO_STREAM_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.StreamDataEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status                    = 0;
      }
      else
      {
         ResponseMessage.StreamDataEventsHandlerID = 0;

         ResponseMessage.Status                    = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register Audio  */
   /* Data Events Message and responds to the message accordingly.  This*/
   /* function does not verify the integrity of the Message (i.e. the   */
   /* length) because it is the caller's responsibility to verify the   */
   /* Message before calling this function.                             */
static void ProcessUnRegisterAudioStreamDataEventsMessage(AUDM_Un_Register_Audio_Stream_Data_Events_Request_t *Message)
{
   int                                                   Result;
   Audio_Entry_Info_t                                   *AudioEntryInfo;
   AUDM_Un_Register_Audio_Stream_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, Message->StreamDataEventsHandlerID)) != NULL) && (AudioEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Delete the Handler.                                         */
         if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoDataList, Message->StreamDataEventsHandlerID)) != NULL)
         {
            /* All finished with the entry, delete it.                  */
            FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_UN_REGISTER_AUDIO_STREAM_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Register Remote    */
   /* Control Events Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessRegisterRemoteControlDataEventsMessage(AUDM_Register_Remote_Control_Data_Events_Request_t *Message)
{
   int                                                  Result;
   Audio_Entry_Info_t                                   AudioEntryInfo;
   Audio_Entry_Info_t                                  *AudioEntryInfoPtr;
   AUDM_Register_Remote_Control_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Before proceding any further, make sure that there is not      */
      /* already an Event Handler for the specified Service Type.       */
      /* * NOTE * We are using the ConnectionStatus member to denote the*/
      /*          Service Type.                                         */
      AudioEntryInfoPtr = AudioEntryInfoRemoteControlList;

      while(AudioEntryInfoPtr)
      {
         if(AudioEntryInfoPtr->ConnectionStatus & Message->ServiceType)
            break;
         else
            AudioEntryInfoPtr = AudioEntryInfoPtr->NextAudioEntryInfoPtr;
      }

      if(!AudioEntryInfoPtr)
      {
         /* First, Register the handler locally.                        */
         BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

         /* We will use the ConnectionStatus member to keep track of the*/
         /* Service Type.                                               */
         AudioEntryInfo.CallbackID         = GetNextCallbackID();
         AudioEntryInfo.ClientID           = Message->MessageHeader.AddressID;
         AudioEntryInfo.ConnectionStatus   = Message->ServiceType;
         AudioEntryInfo.EventCallbackEntry = TRUE;

         if(AddAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, &AudioEntryInfo))
         {
            /* Data Handler registered, go ahead and flag success to the*/
            /* caller.                                                  */
            Result = AudioEntryInfo.CallbackID;
         }
         else
            Result = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
      }
      else
         Result = BTPM_ERROR_CODE_REMOTE_CONTROL_EVENT_ALREADY_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_REGISTER_REMOTE_CONTROL_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      if(Result > 0)
      {
         ResponseMessage.RemoteControlEventsHandlerID = (unsigned int)Result;

         ResponseMessage.Status                       = 0;
      }
      else
      {
         ResponseMessage.RemoteControlEventsHandlerID = 0;

         ResponseMessage.Status                       = Result;
      }

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function processes the specified Un-Register Remote */
   /* Control Events Message and responds to the message accordingly.   */
   /* This function does not verify the integrity of the Message (i.e.  */
   /* the length) because it is the caller's responsibility to verify   */
   /* the Message before calling this function.                         */
static void ProcessUnRegisterRemoteControlDataEventsMessage(AUDM_Un_Register_Remote_Control_Data_Events_Request_t *Message)
{
   int                                                     Result;
   Audio_Entry_Info_t                                     *AudioEntryInfo;
   AUDM_Un_Register_Remote_Control_Data_Events_Response_t  ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* First, check to make sure the client is the client that        */
      /* actually registered for the events.                            */
      if(((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, Message->RemoteControlEventsHandlerID)) != NULL) && (AudioEntryInfo->ClientID == Message->MessageHeader.AddressID))
      {
         /* Delete the Handler.                                         */
         if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, Message->RemoteControlEventsHandlerID)) != NULL)
         {
            /* All finished with the entry, delete it.                  */
            FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

            Result = 0;
         }
         else
            Result = BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED;
      }
      else
         Result = BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED;

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_UN_REGISTER_REMOTE_CONTROL_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectRemoteControlBrowsingMessage(AUDM_Connect_Remote_Control_Browsing_Request_t *Message)
{
   int                                             Result;
   AUDM_Connect_Remote_Control_Browsing_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Simply wrap the internal function to do all of the work.       */
      Result = ConnectRemoteControlBrowsing(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->ConnectionFlags, NULL, NULL, NULL);

      /* Format up the response message and send it.                    */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_CONNECT_REMOTE_CONTROL_BROWSING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisconnectRemoteControlBrowsingMessage(AUDM_Disconnect_Remote_Control_Browsing_Request_t *Message)
{
   int                                                Result;
   AUD_Browsing_Channel_Close_Indication_Data_t       CloseIndicationData;
   AUDM_Disconnect_Remote_Control_Browsing_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Go ahead and process the message request.                      */

      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Nothing to do here other than to call the actual function to*/
         /* Disconnect the Remote Control.                              */
         Result = _AUDM_Disconnect_Remote_Control_Browsing(Message->RemoteDeviceAddress);

         /* If the result was successful, we need to make sure we clean */
         /* up everything and dispatch the event to all registered      */
         /* clients.                                                    */
         if(!Result)
         {
            /* Fake a Close Event to dispatch to all registered clients */
            /* that the Stream has been closed.                         */
            CloseIndicationData.BD_ADDR          = Message->RemoteDeviceAddress;

            ProcessBrowsingChannelCloseIndicationEvent(&CloseIndicationData);
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;


      /* Format up the response message and send it.                    */
      BTPS_MemInitialize(&ResponseMessage, 0, sizeof(ResponseMessage));

      ResponseMessage.MessageHeader                = Message->MessageHeader;

      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;

      ResponseMessage.MessageHeader.MessageLength  = AUDM_DISCONNECT_REMOTE_CONTROL_BROWSING_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;

      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is called from the Main Bluetopia Dispatch */
   /* thread in response to a Process Received Message.  This function  */
   /* SHOULD ONLY be called in this capacity and NOT under any other    */
   /* circumstances.                                                    */
   /* * NOTE * This function *MUST* be called with the Audio Manager    */
   /*          Lock held.  This function will release the Lock before   */
   /*          it exits (i.e. the caller SHOULD NOT RELEASE THE LOCK).  */
static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Audio Stream Connect Response Request.                */
               ProcessConnectionResponseMessage((AUDM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CONNECT_AUDIO_STREAM:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Audio Stream Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CONNECT_AUDIO_STREAM_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connect Audio Stream Request.                         */
               ProcessConnectAudioStreamMessage((AUDM_Connect_Audio_Stream_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_DISCONNECT_AUDIO_STREAM:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Audio Stream Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_DISCONNECT_AUDIO_STREAM_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnect Audio Stream Request.                      */
               ProcessDisconnectAudioStreamMessage((AUDM_Disconnect_Audio_Stream_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_CONNECTED_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Audio Connected Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_CONNECTED_DEVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Audio Connected Devices Request.                */
               ProcessQueryAudioConnectedDevicesMessage((AUDM_Query_Audio_Connected_Devices_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_STATE:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Audio Stream State Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_STREAM_STATE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Audio Stream State Request.                     */
               ProcessQueryAudioStreamStateMessage((AUDM_Query_Audio_Stream_State_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_FORMAT:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Audio Stream Format Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_STREAM_FORMAT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Audio Stream Format Request.                    */
               ProcessQueryAudioStreamFormatMessage((AUDM_Query_Audio_Stream_Format_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Audio Stream State Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CHANGE_AUDIO_STREAM_STATE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Start/Stop Audio Stream Request.                      */
               ProcessChangeAudioStreamStateMessage((AUDM_Change_Audio_Stream_State_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Audio Stream Format Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CHANGE_AUDIO_STREAM_FORMAT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Start/Stop Audio Stream Request.                      */
               ProcessChangeAudioStreamFormatMessage((AUDM_Change_Audio_Stream_Format_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_QUERY_AUDIO_STREAM_CONFIGURATION:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Audio Stream Configuration Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_QUERY_AUDIO_STREAM_CONFIGURATION_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Audio Stream Configuration Request.             */
               ProcessQueryAudioStreamConfigurationMessage((AUDM_Query_Audio_Stream_Configuration_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Incoming Connection Flags Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Change Incoming Connection Flags Request.             */
               ProcessChangeIncomingConnectionFlagsMessage((AUDM_Change_Incoming_Connection_Flags_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_SEND_ENCODED_AUDIO_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Encoded Audio Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_ENCODED_AUDIO_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_ENCODED_AUDIO_DATA_REQUEST_SIZE(((AUDM_Send_Encoded_Audio_Data_Request_t *)Message)->RawAudioDataFrameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Encoded Audio Data Request.                           */
               ProcessSendEncodedAudioDataMessage((AUDM_Send_Encoded_Audio_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_SEND_RTP_ENCODED_AUDIO_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Send RTP Encoded Audio Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_RTP_ENCODED_AUDIO_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_RTP_ENCODED_AUDIO_DATA_REQUEST_SIZE(((AUDM_Send_RTP_Encoded_Audio_Data_Request_t *)Message)->RawAudioDataFrameLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Encoded Audio Data Request.                           */
               ProcessSendRTPEncodedAudioDataMessage((AUDM_Send_RTP_Encoded_Audio_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Control Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CONNECT_REMOTE_CONTROL_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connect Remote Control Request.                       */
               ProcessConnectRemoteControlMessage((AUDM_Connect_Remote_Control_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Remote Control Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_DISCONNECT_REMOTE_CONTROL_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnect Remote Control Request.                    */
               ProcessDisconnectRemoteControlMessage((AUDM_Disconnect_Remote_Control_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Remote Control Connected Devices Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_QUERY_REMOTE_CONTROL_CONNECTED_DEVICES_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Query Remote Control Connected Devices Request.       */
               ProcessQueryRemoteControlConnectedDevicesMessage((AUDM_Query_Remote_Control_Connected_Devices_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_COMMAND:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Remote Control Command Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_REMOTE_CONTROL_COMMAND_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_REMOTE_CONTROL_COMMAND_REQUEST_SIZE(((AUDM_Send_Remote_Control_Command_Request_t *)Message)->MessageDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Remote Control Command Request.                       */
               ProcessSendRemoteControlCommandMessage((AUDM_Send_Remote_Control_Command_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_SEND_REMOTE_CONTROL_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Remote Control Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_REMOTE_CONTROL_RESPONSE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_SEND_REMOTE_CONTROL_RESPONSE_REQUEST_SIZE(((AUDM_Send_Remote_Control_Response_Request_t *)Message)->MessageDataLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Send Remote Control Response Request.                 */
               ProcessSendRemoteControlResponseMessage((AUDM_Send_Remote_Control_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Audio Stream Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REGISTER_AUDIO_STREAM_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register Audio Stream Events Request.                 */
               ProcessRegisterAudioStreamEventsMessage((AUDM_Register_Audio_Stream_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Audio Stream Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_UN_REGISTER_AUDIO_STREAM_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register Audio Stream Events Request.              */
               ProcessUnRegisterAudioStreamEventsMessage((AUDM_Un_Register_Audio_Stream_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REGISTER_AUDIO_STREAM_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Audio Stream Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REGISTER_AUDIO_STREAM_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register Audio Stream Data Events Request.            */
               ProcessRegisterAudioStreamDataEventsMessage((AUDM_Register_Audio_Stream_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_UN_REGISTER_AUDIO_STREAM_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Audio Stream Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_UN_REGISTER_AUDIO_STREAM_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register Audio Stream Data Events Request.         */
               ProcessUnRegisterAudioStreamDataEventsMessage((AUDM_Un_Register_Audio_Stream_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_REGISTER_REMOTE_CONTROL_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Remote Control Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_REGISTER_REMOTE_CONTROL_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Register Remote Control Data Events Request.          */
               ProcessRegisterRemoteControlDataEventsMessage((AUDM_Register_Remote_Control_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_UN_REGISTER_REMOTE_CONTROL_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Un-Register Remote Control Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_UN_REGISTER_REMOTE_CONTROL_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Un-Register Remote Control Data Events Request.       */
               ProcessUnRegisterRemoteControlDataEventsMessage((AUDM_Un_Register_Remote_Control_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_CONNECT_REMOTE_CONTROL_BROWSING:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Control Browsing Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_CONNECT_REMOTE_CONTROL_BROWSING_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Connect Remote Control Browsing Request.              */
               ProcessConnectRemoteControlBrowsingMessage((AUDM_Connect_Remote_Control_Browsing_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case AUDM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_CONTROL_BROWSING:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Remote Control Browsing Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= AUDM_DISCONNECT_REMOTE_CONTROL_BROWSING_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the     */
               /* Disconnect Remote Control Browsing Request.           */
               ProcessDisconnectRemoteControlBrowsingMessage((AUDM_Disconnect_Remote_Control_Browsing_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is called to process a*/
   /* client Un-Registration event.  This function will perform any     */
   /* cleanup required for the specified Client.                        */
static void ProcessClientUnRegister(unsigned int ClientID)
{
   Boolean_t            LoopCount;
   Audio_Entry_Info_t  *AudioEntryInfo;
   Audio_Entry_Info_t **_AudioEntryInfoList;
   Audio_Entry_Info_t  *tmpAudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      AudioEntryInfo      = AudioEntryInfoList;
      _AudioEntryInfoList = &AudioEntryInfoList;

      /* We need to loop through the all three lists as there could be  */
      /* client registrations in any of the lists.                      */
      LoopCount = 4;
      while(LoopCount)
      {
         while(AudioEntryInfo)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(AudioEntryInfo->ClientID == ClientID)
            {
               /* Note the next Audio Entry in the list (we are about to*/
               /* delete the current entry).                            */
               tmpAudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;

               /* Go ahead and delete the Audio Information Entry and   */
               /* clean up the resources.                               */
               if((AudioEntryInfo = DeleteAudioEntryInfoEntry(_AudioEntryInfoList, AudioEntryInfo->CallbackID)) != NULL)
               {
                  /* Close any events that were allocated.              */
                  if((!AudioEntryInfo->EventCallbackEntry) && (AudioEntryInfo->ConnectionEvent))
                     BTPS_CloseEvent(AudioEntryInfo->ConnectionEvent);

                  /* All finished with the memory so free the entry.    */
                  FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
               }

               /* Go ahead and set the next Audio Information Entry     */
               /* (past the one we just deleted).                       */
               AudioEntryInfo = tmpAudioEntryInfo;
            }
            else
               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
         }

         /* Decrement the loop count so that we can make another pass   */
         /* through the loop.                                           */
         LoopCount--;

         /* We have processed the Audio Information List, now process   */
         /* the Audio Information Data List or the Remote Control Event */
         /* List.                                                       */
         if(LoopCount == 3)
         {
            AudioEntryInfo      = AudioEntryInfoDataList;
            _AudioEntryInfoList = &AudioEntryInfoDataList;
         }
         else
         {
            if(LoopCount == 2)
            {
               AudioEntryInfo      = OutgoingRemoteControlList;
               _AudioEntryInfoList = &OutgoingRemoteControlList;
            }
            else
            {
               AudioEntryInfo      = AudioEntryInfoRemoteControlList;
               _AudioEntryInfoList = &AudioEntryInfoRemoteControlList;
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Open       */
   /* Request Indication Event (Stream or Remote Control) that has been */
   /* received with the specified information.  This function should be */
   /* called with the Lock protecting the Audio Manager Information     */
   /* held.                                                             */
static void ProcessOpenRequestIndicationEvent(AUD_Open_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   AUDM_Event_Data_t                  AUDMEventData;
   Incoming_Connection_Entry_t        IncomingConnectionEntry;
   Incoming_Connection_Entry_t       *IncomingConnectionEntryPtr;
   AUDM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenRequestIndicationData)
   {
      /* First, let's see if we actually need to do anything, other than*/
      /* simply accept the connection.                                  */
      if(!IncomingConnectionFlags)
      {
         /* Simply Accept the connection.                               */
         _AUDM_Connection_Request_Response(OpenRequestIndicationData->ConnectionRequestType, OpenRequestIndicationData->BD_ADDR, TRUE);
      }
      else
      {
         /* Before proceding any further, let's make sure that there    */
         /* doesn't already exist an entry for this type.               */
         if((IncomingConnectionEntryPtr = SearchIncomingConnectionEntry(&IncomingConnectionEntryList, OpenRequestIndicationData->BD_ADDR, OpenRequestIndicationData->ConnectionRequestType)) == NULL)
         {
            /* Entry does not exist, go ahead and format a new entry.   */
            IncomingConnectionEntry.BD_ADDR                        = OpenRequestIndicationData->BD_ADDR;
            IncomingConnectionEntry.IncomingConnectionState        = icsAuthorizing;
            IncomingConnectionEntry.RequestType                    = OpenRequestIndicationData->ConnectionRequestType;
            IncomingConnectionEntry.NextIncomingConnectionEntryPtr = NULL;

            IncomingConnectionEntryPtr = AddIncomingConnectionEntry(&IncomingConnectionEntryList, &IncomingConnectionEntry);
         }

         /* Check to see if we are tracking this connection.            */
         if(IncomingConnectionEntryPtr)
         {
            /* Connection is being tracked.  Now determine the correct  */
            /* state.                                                   */
            if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
            {
               /* Authorization required, go ahead and dispatch the     */
               /* request.                                              */
               IncomingConnectionEntryPtr->IncomingConnectionState = icsAuthorizing;

               /* Next, format up the Event to dispatch.                */
               AUDMEventData.EventType                                                        = aetIncomingConnectionRequest;
               AUDMEventData.EventLength                                                      = AUDM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE;

               AUDMEventData.EventData.IncomingConnectionRequestEventData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;
               AUDMEventData.EventData.IncomingConnectionRequestEventData.RequestType         = OpenRequestIndicationData->ConnectionRequestType;

               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = 0;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
               Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
               Message.MessageHeader.MessageLength   = (AUDM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;
               Message.RequestType                   = OpenRequestIndicationData->ConnectionRequestType;

               /* Finally dispatch the formatted Event and Message.     */
               /* If the connection request is for a Remote Control     */
               /* connection, issue the request to registered Remote    */
               /* Control callbacks, first.                             */
               if(OpenRequestIndicationData->ConnectionRequestType == acrRemoteControl)
                  DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, (AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER));

               /* For legacy reasons, always dispatch connection        */
               /* requests (of any type) to registered Audio event      */
               /* callbacks.                                            */
               DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
            }
            else
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

               if((Authenticate) || (Encrypt))
               {
                  if(Encrypt)
                     Result = DEVM_EncryptRemoteDevice(IncomingConnectionEntryPtr->BD_ADDR, 0);
                  else
                     Result = DEVM_AuthenticateRemoteDevice(IncomingConnectionEntryPtr->BD_ADDR, 0);

                  if((Result == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (Result == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                  {
                     /* Authorization not required, and we are already  */
                     /* in the correct state.                           */
                     _AUDM_Connection_Request_Response(IncomingConnectionEntryPtr->RequestType, IncomingConnectionEntryPtr->BD_ADDR, TRUE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((IncomingConnectionEntryPtr = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, IncomingConnectionEntryPtr->BD_ADDR, IncomingConnectionEntryPtr->RequestType)) != NULL)
                        FreeIncomingConnectionEntryMemory(IncomingConnectionEntryPtr);
                  }
                  else
                  {
                     if(Result < 0)
                     {
                        _AUDM_Connection_Request_Response(OpenRequestIndicationData->ConnectionRequestType, OpenRequestIndicationData->BD_ADDR, FALSE);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        if((IncomingConnectionEntryPtr = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, IncomingConnectionEntryPtr->BD_ADDR, IncomingConnectionEntryPtr->RequestType)) != NULL)
                           FreeIncomingConnectionEntryMemory(IncomingConnectionEntryPtr);
                     }
                     else
                     {
                        if(Encrypt)
                           IncomingConnectionEntryPtr->IncomingConnectionState = icsEncrypting;
                        else
                           IncomingConnectionEntryPtr->IncomingConnectionState = icsAuthenticating;
                     }
                  }
               }
               else
               {
                  /* This case should never occur.                      */
                  _AUDM_Connection_Request_Response(OpenRequestIndicationData->ConnectionRequestType, OpenRequestIndicationData->BD_ADDR, FALSE);
               }
            }
         }
         else
         {
            /* Unable to add entry, go ahead and reject the request.    */
            _AUDM_Connection_Request_Response(OpenRequestIndicationData->ConnectionRequestType, OpenRequestIndicationData->BD_ADDR, FALSE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream Open*/
   /* Indication Event that has been received with the specified        */
   /* information.  This function should be called with the Lock        */
   /* protecting the Audio Manager Information held.                    */
static void ProcessStreamOpenIndicationEvent(AUD_Stream_Open_Indication_Data_t *StreamOpenIndicationData)
{
   AUDM_Event_Data_t                      AUDMEventData;
   Incoming_Connection_Entry_t           *IncomingConnectionEntry;
   Audio_Connection_Entry_t               AudioConnectionEntry;
   AUDM_Audio_Stream_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamOpenIndicationData)
   {
      /* Add the connection to the list.                                */
      BTPS_MemInitialize(&AudioConnectionEntry, 0, sizeof(Audio_Connection_Entry_t));

      AudioConnectionEntry.BD_ADDR         = StreamOpenIndicationData->BD_ADDR;
      AudioConnectionEntry.StreamType      = StreamOpenIndicationData->StreamType;
      AudioConnectionEntry.ConnectionState = scsConnected;

      if(AddAudioConnectionEntry(&AudioConnectionList, &AudioConnectionEntry) != NULL)
      {
         /* Since we are connected, if there was a request for this     */
         /* Device and for the Stream type, we will make sure we delete */
         /* the entry from the list (as there is no longer an reason to */
         /* track it).                                                  */
         if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, StreamOpenIndicationData->BD_ADDR, acrStream)) != NULL)
            FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);

         /* Next, format up the Event to dispatch.                      */
         AUDMEventData.EventType                                                   = aetAudioStreamConnected;
         AUDMEventData.EventLength                                                 = AUDM_AUDIO_STREAM_CONNECTED_EVENT_DATA_SIZE;

         AUDMEventData.EventData.AudioStreamConnectedEventData.StreamType          = StreamOpenIndicationData->StreamType;
         AUDMEventData.EventData.AudioStreamConnectedEventData.RemoteDeviceAddress = StreamOpenIndicationData->BD_ADDR;
         AUDMEventData.EventData.AudioStreamConnectedEventData.MediaMTU            = StreamOpenIndicationData->MediaMTU;
         AUDMEventData.EventData.AudioStreamConnectedEventData.StreamFormat        = StreamOpenIndicationData->StreamFormat;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTED;
         Message.MessageHeader.MessageLength   = (AUDM_AUDIO_STREAM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = StreamOpenIndicationData->BD_ADDR;
         Message.MediaMTU                      = StreamOpenIndicationData->MediaMTU;
         Message.StreamType                    = StreamOpenIndicationData->StreamType;
         Message.StreamFormat                  = StreamOpenIndicationData->StreamFormat;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream Open*/
   /* Confirmation Event that has been received with the specified      */
   /* information.  This function should be called with the Lock        */
   /* protecting the Audio Manager Information held.                    */
static void ProcessStreamOpenConfirmationEvent(AUD_Stream_Open_Confirmation_Data_t *StreamOpenConfirmationData)
{
   void                                          *CallbackParameter;
   unsigned int                                   ClientID;
   AUDM_Event_Data_t                              AUDMEventData;
   Audio_Entry_Info_t                            *AudioEntryInfo;
   AUDM_Event_Callback_t                          EventCallback;
   Audio_Connection_Entry_t                      *AudioConnectionEntry;
   Incoming_Connection_Entry_t                   *IncomingConnectionEntry;
   AUDM_Audio_Stream_Connected_Message_t          Message;
   AUDM_Audio_Stream_Connection_Status_Message_t  StatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamOpenConfirmationData)
   {
      if((AudioConnectionEntry = SearchAudioConnectionEntry(&AudioConnectionList, StreamOpenConfirmationData->BD_ADDR, StreamOpenConfirmationData->StreamType)) != NULL)
      {
         if(StreamOpenConfirmationData->OpenStatus == AUD_STREAM_OPEN_CONFIRMATION_STATUS_SUCCESS)
            AudioConnectionEntry->ConnectionState = scsConnected;
         else
         {
            /* Connection failed. Removed the tracking structure.       */
            if((AudioConnectionEntry = DeleteAudioConnectionEntry(&AudioConnectionList, StreamOpenConfirmationData->BD_ADDR, StreamOpenConfirmationData->StreamType)) != NULL)
               FreeAudioConnectionEntryMemory(AudioConnectionEntry);
         }

         /* Dispatch any registered Connection Status Message/Event.    */
         AudioEntryInfo = AudioEntryInfoList;
         while(AudioEntryInfo)
         {
            if((!AudioEntryInfo->EventCallbackEntry) && (AudioEntryInfo->StreamType == StreamOpenConfirmationData->StreamType))
            {
               /* Connection status registered, now see if we need to   */
               /* issue a Callack or an event.                          */

               /* Since we have received a connect confirmation, if     */
               /* there was a request for this Device and for the Stream*/
               /* type, we will make sure we delete the entry from the  */
               /* list (as there is no longer an reason to track it).   */
               if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, StreamOpenConfirmationData->BD_ADDR, acrStream)) != NULL)
                  FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);

               /* Determine if we need to dispatch the event locally or */
               /* remotely.                                             */
               if(AudioEntryInfo->ClientID == MSG_GetServerAddressID())
               {
                  /* Local Callback.                                    */

                  /* Format up the Event.                               */
                  AUDMEventData.EventType                                                          = aetAudioStreamConnectionStatus;
                  AUDMEventData.EventLength                                                        = AUDM_AUDIO_STREAM_CONNECTION_STATUS_EVENT_DATA_SIZE;

                  AUDMEventData.EventData.AudioStreamConnectionStatusEventData.RemoteDeviceAddress = StreamOpenConfirmationData->BD_ADDR;
                  AUDMEventData.EventData.AudioStreamConnectionStatusEventData.StreamType          = StreamOpenConfirmationData->StreamType;
                  AUDMEventData.EventData.AudioStreamConnectionStatusEventData.MediaMTU            = StreamOpenConfirmationData->MediaMTU;
                  AUDMEventData.EventData.AudioStreamConnectionStatusEventData.StreamFormat        = StreamOpenConfirmationData->StreamFormat;

                  /* Map the Stream Open Confirmation Error to the      */
                  /* correct Audio Manager Error Status.                */
                  switch(StreamOpenConfirmationData->OpenStatus)
                  {
                     case AUD_STREAM_OPEN_CONFIRMATION_STATUS_SUCCESS:
                        AUDMEventData.EventData.AudioStreamConnectionStatusEventData.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_SUCCESS;
                        break;
                     case AUD_STREAM_OPEN_CONFIRMATION_STATUS_CONNECTION_TIMEOUT:
                        AUDMEventData.EventData.AudioStreamConnectionStatusEventData.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_TIMEOUT;
                        break;
                     case AUD_STREAM_OPEN_CONFIRMATION_STATUS_CONNECTION_REFUSED:
                        AUDMEventData.EventData.AudioStreamConnectionStatusEventData.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;
                     default:
                        AUDMEventData.EventData.AudioStreamConnectionStatusEventData.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* If this was a synchronous event we need to set the */
                  /* status and the event.                              */
                  if(AudioEntryInfo->ConnectionEvent)
                  {
                     /* Synchronous event, go ahead and set the correct */
                     /* status, then set the event.                     */
                     AudioEntryInfo->ConnectionStatus = AUDMEventData.EventData.AudioStreamConnectionStatusEventData.ConnectionStatus;

                     BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);
                  }
                  else
                  {
                     /* Note the Callback information.                  */
                     EventCallback     = AudioEntryInfo->EventCallback;
                     CallbackParameter = AudioEntryInfo->CallbackParameter;

                     /* Go ahead and delete the entry (since we are     */
                     /* dispatching the event).                         */
                     if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfo->CallbackID)) != NULL)
                        FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

                     /* Release the Lock so we can make the callback.   */
                     DEVM_ReleaseLock();

                     __BTPSTRY
                     {
                        if(EventCallback)
                           (*EventCallback)(&AUDMEventData, CallbackParameter);
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* Re-acquire the Lock.                            */
                     DEVM_AcquireLock();
                  }
               }
               else
               {
                  /* Remote Event.                                      */

                  /* Note the Client ID.                                */
                  ClientID = AudioEntryInfo->ClientID;

                  /* Go ahead and delete the entry (since we are        */
                  /* dispatching the event).                            */
                  if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfo->CallbackID)) != NULL)
                     FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

                  BTPS_MemInitialize(&StatusMessage, 0, sizeof(StatusMessage));

                  StatusMessage.MessageHeader.AddressID       = ClientID;
                  StatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
                  StatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
                  StatusMessage.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTION_STATUS;
                  StatusMessage.MessageHeader.MessageLength   = (AUDM_AUDIO_STREAM_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

                  StatusMessage.RemoteDeviceAddress           = StreamOpenConfirmationData->BD_ADDR;
                  StatusMessage.MediaMTU                      = StreamOpenConfirmationData->MediaMTU;
                  StatusMessage.StreamType                    = StreamOpenConfirmationData->StreamType;
                  StatusMessage.StreamFormat                  = StreamOpenConfirmationData->StreamFormat;

                  /* Map the Stream Open Confirmation Error to the      */
                  /* correct Audio Manager Error Status.                */
                  switch(StreamOpenConfirmationData->OpenStatus)
                  {
                     case AUD_STREAM_OPEN_CONFIRMATION_STATUS_SUCCESS:
                        StatusMessage.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_SUCCESS;
                        break;
                     case AUD_STREAM_OPEN_CONFIRMATION_STATUS_CONNECTION_TIMEOUT:
                        StatusMessage.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_TIMEOUT;
                        break;
                     case AUD_STREAM_OPEN_CONFIRMATION_STATUS_CONNECTION_REFUSED:
                        StatusMessage.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_REFUSED;
                        break;
                     default:
                        StatusMessage.ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_UNKNOWN;
                        break;
                  }

                  /* Finally dispatch the Message.                      */
                  MSG_SendMessage((BTPM_Message_t *)&StatusMessage);
               }

               /* Break out of the loop.                                */
               AudioEntryInfo = NULL;
            }
            else
               AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
         }

         /* Next, format up the Event to dispatch - ONLY if we need to  */
         /* dispatch a Connected Event.                                 */
         if(!StreamOpenConfirmationData->OpenStatus)
         {
            AUDMEventData.EventType                                                   = aetAudioStreamConnected;
            AUDMEventData.EventLength                                                 = AUDM_AUDIO_STREAM_CONNECTED_EVENT_DATA_SIZE;

            AUDMEventData.EventData.AudioStreamConnectedEventData.StreamType          = StreamOpenConfirmationData->StreamType;
            AUDMEventData.EventData.AudioStreamConnectedEventData.RemoteDeviceAddress = StreamOpenConfirmationData->BD_ADDR;
            AUDMEventData.EventData.AudioStreamConnectedEventData.MediaMTU            = StreamOpenConfirmationData->MediaMTU;
            AUDMEventData.EventData.AudioStreamConnectedEventData.StreamFormat        = StreamOpenConfirmationData->StreamFormat;

            /* Next, format up the Message to dispatch.                 */
            BTPS_MemInitialize(&Message, 0, sizeof(Message));

            Message.MessageHeader.AddressID       = 0;
            Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
            Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_CONNECTED;
            Message.MessageHeader.MessageLength   = (AUDM_AUDIO_STREAM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

            Message.RemoteDeviceAddress           = StreamOpenConfirmationData->BD_ADDR;
            Message.MediaMTU                      = StreamOpenConfirmationData->MediaMTU;
            Message.StreamType                    = StreamOpenConfirmationData->StreamType;
            Message.StreamFormat                  = StreamOpenConfirmationData->StreamFormat;

            /* Finally dispatch the formatted Event and Message.        */
            DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream     */
   /* Close Indication Event that has been received with the specified  */
   /* information.  This function should be called with the Lock        */
   /* protecting the Audio Manager Information held.                    */
static void ProcessStreamCloseIndicationEvent(AUD_Stream_Close_Indication_Data_t *StreamCloseIndicationData)
{
   AUDM_Event_Data_t                         AUDMEventData;
   Audio_Connection_Entry_t                 *AudioConnectionEntry;
   AUDM_Audio_Stream_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamCloseIndicationData)
   {
      if((AudioConnectionEntry = DeleteAudioConnectionEntry(&AudioConnectionList, StreamCloseIndicationData->BD_ADDR, StreamCloseIndicationData->StreamType)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         AUDMEventData.EventType                                                      = aetAudioStreamDisconnected;
         AUDMEventData.EventLength                                                    = AUDM_AUDIO_STREAM_DISCONNECTED_EVENT_DATA_SIZE;

         AUDMEventData.EventData.AudioStreamDisconnectedEventData.RemoteDeviceAddress = StreamCloseIndicationData->BD_ADDR;
         AUDMEventData.EventData.AudioStreamDisconnectedEventData.StreamType          = StreamCloseIndicationData->StreamType;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (AUDM_AUDIO_STREAM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = StreamCloseIndicationData->BD_ADDR;
         Message.StreamType                    = StreamCloseIndicationData->StreamType;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);

         FreeAudioConnectionEntryMemory(AudioConnectionEntry);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream     */
   /* State Change Indication Event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessStreamStateChangeIndicationEvent(AUD_Stream_State_Change_Indication_Data_t *StreamStateChangeIndicationData)
{
   AUDM_Event_Data_t                         AUDMEventData;
   AUDM_Audio_Stream_State_Changed_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamStateChangeIndicationData)
   {
      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                      = aetAudioStreamStateChanged;
      AUDMEventData.EventLength                                                    = AUDM_AUDIO_STREAM_STATE_CHANGED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.AudioStreamStateChangedEventData.RemoteDeviceAddress = StreamStateChangeIndicationData->BD_ADDR;
      AUDMEventData.EventData.AudioStreamStateChangedEventData.StreamType          = StreamStateChangeIndicationData->StreamType;
      AUDMEventData.EventData.AudioStreamStateChangedEventData.StreamState         = StreamStateChangeIndicationData->StreamState;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_STATE_CHANGED;
      Message.MessageHeader.MessageLength   = (AUDM_AUDIO_STREAM_STATE_CHANGED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = StreamStateChangeIndicationData->BD_ADDR;
      Message.StreamType                    = StreamStateChangeIndicationData->StreamType;
      Message.StreamState                   = StreamStateChangeIndicationData->StreamState;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream     */
   /* State Changed Confirmation Event that has been received with the  */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessStreamStateChangeConfirmationEvent(AUD_Stream_State_Change_Confirmation_Data_t *StreamStateChangeConfirmationData)
{
   AUDM_Event_Data_t                               AUDMEventData;
   AUDM_Change_Audio_Stream_State_Status_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamStateChangeConfirmationData)
   {
      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                           = aetChangeAudioStreamStateStatus;
      AUDMEventData.EventLength                                                         = AUDM_CHANGE_AUDIO_STREAM_STATE_STATUS_EVENT_DATA_SIZE;

      AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.RemoteDeviceAddress = StreamStateChangeConfirmationData->BD_ADDR;
      AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.Successful          = StreamStateChangeConfirmationData->Successful;
      AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.StreamType          = StreamStateChangeConfirmationData->StreamType;
      AUDMEventData.EventData.ChangeAudioStreamStateStatusEventData.StreamState         = StreamStateChangeConfirmationData->StreamState;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_STATE_STATUS;
      Message.MessageHeader.MessageLength   = (AUDM_CHANGE_AUDIO_STREAM_STATE_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = StreamStateChangeConfirmationData->BD_ADDR;
      Message.Successful                    = StreamStateChangeConfirmationData->Successful;
      Message.StreamType                    = StreamStateChangeConfirmationData->StreamType;
      Message.StreamState                   = StreamStateChangeConfirmationData->StreamState;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream     */
   /* State Format Indication Event that has been received with the     */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessStreamFormatChangeIndicationEvent(AUD_Stream_Format_Change_Indication_Data_t *StreamFormatChangeIndicationData)
{
   AUDM_Event_Data_t                          AUDMEventData;
   AUDM_Audio_Stream_Format_Changed_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamFormatChangeIndicationData)
   {
      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                       = aetAudioStreamFormatChanged;
      AUDMEventData.EventLength                                                     = AUDM_AUDIO_STREAM_FORMAT_CHANGED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.AudioStreamFormatChangedEventData.RemoteDeviceAddress = StreamFormatChangeIndicationData->BD_ADDR;
      AUDMEventData.EventData.AudioStreamFormatChangedEventData.StreamType          = StreamFormatChangeIndicationData->StreamType;
      AUDMEventData.EventData.AudioStreamFormatChangedEventData.StreamFormat        = StreamFormatChangeIndicationData->StreamFormat;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_AUDIO_STREAM_FORMAT_CHANGED;
      Message.MessageHeader.MessageLength   = (AUDM_AUDIO_STREAM_FORMAT_CHANGED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = StreamFormatChangeIndicationData->BD_ADDR;
      Message.StreamType                    = StreamFormatChangeIndicationData->StreamType;
      Message.StreamFormat                  = StreamFormatChangeIndicationData->StreamFormat;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Stream     */
   /* Format Changed Confirmation Event that has been received with the */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessStreamFormatChangeConfirmationEvent(AUD_Stream_Format_Change_Confirmation_Data_t *StreamFormatChangeConfirmationData)
{
   AUDM_Event_Data_t                                AUDMEventData;
   AUDM_Change_Audio_Stream_Format_Status_Message_t Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(StreamFormatChangeConfirmationData)
   {
      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                            = aetChangeAudioStreamFormatStatus;
      AUDMEventData.EventLength                                                          = AUDM_CHANGE_AUDIO_STREAM_FORMAT_STATUS_EVENT_DATA_SIZE;

      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.RemoteDeviceAddress = StreamFormatChangeConfirmationData->BD_ADDR;
      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.Successful          = StreamFormatChangeConfirmationData->Successful;
      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.StreamType          = StreamFormatChangeConfirmationData->StreamType;
      AUDMEventData.EventData.ChangeAudioStreamFormatStatusEventData.StreamFormat        = StreamFormatChangeConfirmationData->StreamFormat;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID        = 0;
      Message.MessageHeader.MessageID        = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction  = AUDM_MESSAGE_FUNCTION_CHANGE_AUDIO_STREAM_FORMAT_STAT;
      Message.MessageHeader.MessageLength    = (AUDM_CHANGE_AUDIO_STREAM_FORMAT_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress            = StreamFormatChangeConfirmationData->BD_ADDR;
      Message.Successful                     = StreamFormatChangeConfirmationData->Successful;
      Message.StreamType                     = StreamFormatChangeConfirmationData->StreamType;
      Message.StreamFormat                   = StreamFormatChangeConfirmationData->StreamFormat;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchAudioEvent(&AUDMEventData, (BTPM_Message_t *)&Message);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Remote     */
   /* Control Indication Event that has been received with the specified*/
   /* information.  This function should be called with the Lock        */
   /* protecting the Audio Manager Information held.                    */
static void ProcessRemoteControlCommandIndicationEvent(AUDM_AUD_Remote_Control_Command_Indication_Data_t *AUDMRemoteControlCommandIndicationData)
{
   AUDM_Event_Data_t                               AUDMEventData;
   AUD_Remote_Control_Command_Indication_Data_t   *RemoteControlCommandIndicationData;
   AUDM_Remote_Control_Command_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if((AUDMRemoteControlCommandIndicationData) && (AUDMRemoteControlCommandIndicationData->Message.MessageDataLength))
   {
      RemoteControlCommandIndicationData = &AUDMRemoteControlCommandIndicationData->RemoteControlCommandIndicationData;
      Message                            = &AUDMRemoteControlCommandIndicationData->Message;

      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                                  = aetRemoteControlCommandIndication;
      AUDMEventData.EventLength                                                                = AUDM_REMOTE_CONTROL_COMMAND_INDICATION_EVENT_DATA_SIZE;

      AUDMEventData.EventData.RemoteControlCommandIndicationEventData.RemoteDeviceAddress      = RemoteControlCommandIndicationData->BD_ADDR;
      AUDMEventData.EventData.RemoteControlCommandIndicationEventData.TransactionID            = RemoteControlCommandIndicationData->TransactionID;
      AUDMEventData.EventData.RemoteControlCommandIndicationEventData.RemoteControlCommandData = RemoteControlCommandIndicationData->RemoteControlCommandData;

      /* We need to build the correct Remote Control Data.              */
      Message->MessageHeader.AddressID       = 0;
      Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message->MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_RECEIVED;
      Message->MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_COMMAND_RECEIVED_MESSAGE_SIZE(Message->MessageDataLength) - BTPM_MESSAGE_HEADER_SIZE);

      Message->RemoteDeviceAddress           = RemoteControlCommandIndicationData->BD_ADDR;
      Message->TransactionID                 = RemoteControlCommandIndicationData->TransactionID;
      Message->MessageType                   = RemoteControlCommandIndicationData->RemoteControlCommandData.MessageType;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Remote     */
   /* Control Confirmation Event that has been received with the        */
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessRemoteControlCommandConfirmationEvent(AUDM_AUD_Remote_Control_Command_Confirmation_Data_t *AUDMRemoteControlCommandConfirmationData)
{
   AUDM_Event_Data_t                               AUDMEventData;
   AUD_Remote_Control_Command_Confirmation_Data_t *RemoteControlCommandConfirmationData;
   AUDM_Remote_Control_Command_Status_Message_t   *Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(AUDMRemoteControlCommandConfirmationData)
   {
      RemoteControlCommandConfirmationData = &AUDMRemoteControlCommandConfirmationData->RemoteControlCommandConfirmationData;
      Message                              = &AUDMRemoteControlCommandConfirmationData->Message;

      if(((RemoteControlCommandConfirmationData->ConfirmationStatus == AUD_REMOTE_CONTROL_COMMAND_CONFIRMATION_STATUS_SUCCESS) && (AUDMRemoteControlCommandConfirmationData->Message.MessageDataLength)) || (RemoteControlCommandConfirmationData->ConfirmationStatus != AUD_REMOTE_CONTROL_COMMAND_CONFIRMATION_STATUS_SUCCESS))
      {
         /* Next, format up the Event to dispatch.                      */
         AUDMEventData.EventType                                                                        = aetRemoteControlCommandConfirmation;
         AUDMEventData.EventLength                                                                      = AUDM_REMOTE_CONTROL_COMMAND_CONFIRMATION_EVENT_DATA_SIZE;

         AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.RemoteDeviceAddress          = RemoteControlCommandConfirmationData->BD_ADDR;
         AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.TransactionID                = RemoteControlCommandConfirmationData->TransactionID;
         AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.RemoteControlResponseData    = RemoteControlCommandConfirmationData->RemoteControlResponseData;

         /* Map the correct status value.                               */
         if(RemoteControlCommandConfirmationData->ConfirmationStatus == AUD_REMOTE_CONTROL_COMMAND_CONFIRMATION_STATUS_SUCCESS)
            AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.Status = 0;
         else
         {
            if(RemoteControlCommandConfirmationData->ConfirmationStatus == AUD_REMOTE_CONTROL_COMMAND_CONFIRMATION_STATUS_TIMEOUT)
               AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.Status = BTPM_ERROR_CODE_REMOTE_CONTROL_RESPONSE_TIMEOUT_ERROR;
            else
               AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.Status = BTPM_ERROR_CODE_REMOTE_CONTROL_RESPONSE_UNKNOWN_ERROR;
         }

         /* We need to build the correct Remote Control Data.           */
         Message->MessageHeader.AddressID        = 0;
         Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
         Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         Message->MessageHeader.MessageFunction  = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_COMMAND_STATUS;
         Message->MessageHeader.MessageLength    = (AUDM_REMOTE_CONTROL_COMMAND_STATUS_MESSAGE_SIZE(Message->MessageDataLength) - BTPM_MESSAGE_HEADER_SIZE);

         Message->RemoteDeviceAddress            = RemoteControlCommandConfirmationData->BD_ADDR;
         Message->TransactionID                  = RemoteControlCommandConfirmationData->TransactionID;
         Message->Status                         = AUDMEventData.EventData.RemoteControlCommandConfirmationEventData.Status;
         Message->MessageType                    = RemoteControlCommandConfirmationData->RemoteControlResponseData.MessageType;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Remote     */
   /* Control Open Indication Event that has been received with the     */
   /* specified information. This function should be called with the    */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessRemoteControlOpenIndicationEvent(AUD_Remote_Control_Open_Indication_Data_t *RemoteControlOpenIndicationData)
{
   AUDM_Event_Data_t                        AUDMEventData;
   Control_Connection_Entry_t               ConnectionEntry;
   Control_Connection_Entry_t              *ConnectionEntryPtr;
   Incoming_Connection_Entry_t             *IncomingConnectionEntry;
   AUDM_Remote_Control_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(RemoteControlOpenIndicationData)
   {
      /* If a Control Connection Entry does not exist for this device go*/
      /* ahead and add one.                                             */
      if((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, RemoteControlOpenIndicationData->BD_ADDR)) == NULL)
      {
         /* Initialize the connection entry that we will add later.     */
         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Control_Connection_Entry_t));

         ConnectionEntry.ConnectionState     = scsConnected;
         ConnectionEntry.BD_ADDR             = RemoteControlOpenIndicationData->BD_ADDR;

         AddControlConnectionEntry(&RemoteControlConnectionList, &ConnectionEntry);
      }
      else
         ConnectionEntryPtr->ConnectionState = scsConnected;

      /* Since we have received a connect indication, if there was a    */
      /* request for this Device and for the Stream type, we will make  */
      /* sure we delete the entry from the list (as there is no longer  */
      /* an reason to track it).                                        */
      if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteControlOpenIndicationData->BD_ADDR, acrRemoteControl)) != NULL)
         FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);

      /* Format up the Event to dispatch.                               */
      AUDMEventData.EventType                                                     = aetRemoteControlConnected;
      AUDMEventData.EventLength                                                   = AUDM_REMOTE_CONTROL_CONNECTED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.RemoteControlConnectedEventData.RemoteDeviceAddress = RemoteControlOpenIndicationData->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTED;
      Message.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = RemoteControlOpenIndicationData->BD_ADDR;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Remote     */
   /* Control Open Confirmation Event that has been received with the   */
   /* specified information. This function should be called with the    */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessRemoteControlOpenConfirmationEvent(AUD_Remote_Control_Open_Confirmation_Data_t *RemoteControlOpenConfirmationData)
{
   void                                            *CallbackParameter;
   unsigned int                                     ClientID;
   unsigned int                                     ConnectionStatus;
   AUDM_Event_Data_t                                AUDMEventData;
   Audio_Entry_Info_t                              *AudioEntryInfo;
   AUDM_Event_Callback_t                            EventCallback;
   Control_Connection_Entry_t                      *ConnectionEntryPtr;
   Incoming_Connection_Entry_t                     *IncomingConnectionEntry;
   AUDM_Remote_Control_Connected_Message_t          Message;
   AUDM_Remote_Control_Connection_Status_Message_t  StatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(RemoteControlOpenConfirmationData)
   {
      /* Depending on the status go ahead and update the connection     */
      /* enty.                                                          */
      if(RemoteControlOpenConfirmationData->OpenStatus == AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_SUCCESS)
      {
         /* We successfully connected to the remote device so update the*/
         /* connection entry for this entry.                            */
         if((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, RemoteControlOpenConfirmationData->BD_ADDR)) != NULL)
            ConnectionEntryPtr->ConnectionState = scsConnected;
      }
      else
      {
         /* Failed to connect to device so delete the connection entry. */
         if((ConnectionEntryPtr = DeleteControlConnectionEntry(&RemoteControlConnectionList, RemoteControlOpenConfirmationData->BD_ADDR)) != NULL)
            FreeControlConnectionEntryMemory(ConnectionEntryPtr);
      }

      /* Since we have received a connect confirmation, if there was a  */
      /* request for this Device and for the Stream type, we will make  */
      /* sure we delete the entry from the list (as there is no longer  */
      /* an reason to track it).                                        */
      if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteControlOpenConfirmationData->BD_ADDR, acrRemoteControl)) != NULL)
         FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);

      /* Map the Stream Open Confirmation Error to the correct Audio    */
      /* Manager Error Status.                                          */
      switch(RemoteControlOpenConfirmationData->OpenStatus)
      {
         case AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_SUCCESS:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_SUCCESS;
            break;
         case AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_CONNECTION_TIMEOUT:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_TIMEOUT;
            break;
         case AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_CONNECTION_REFUSED:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_REFUSED;
            break;
         default:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_UNKNOWN;
            break;
      }

      /* Dispatch any registered Connection Status Message/Event.       */
      AudioEntryInfo = OutgoingRemoteControlList;
      while(AudioEntryInfo)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Checking Audio Entry: %d, 0x%02X%02X%02X%02X%02X%02X\n", AudioEntryInfo->EventCallbackEntry, AudioEntryInfo->RemoteControlDevice.BD_ADDR5, AudioEntryInfo->RemoteControlDevice.BD_ADDR4, AudioEntryInfo->RemoteControlDevice.BD_ADDR3, AudioEntryInfo->RemoteControlDevice.BD_ADDR2, AudioEntryInfo->RemoteControlDevice.BD_ADDR1, AudioEntryInfo->RemoteControlDevice.BD_ADDR0));

         if((!AudioEntryInfo->EventCallbackEntry) && (COMPARE_BD_ADDR(AudioEntryInfo->RemoteControlDevice, RemoteControlOpenConfirmationData->BD_ADDR)))
         {
            /* Connection status registered, now see if we need to issue*/
            /* a Callack or an event.                                   */

            /* Determine if we need to dispatch the event locally or    */
            /* remotely.                                                */
            if(AudioEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Local Callback.                                       */

               /* Format up the Event.                                  */
               AUDMEventData.EventType                                                            = aetRemoteControlConnectionStatus;
               AUDMEventData.EventLength                                                          = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_EVENT_DATA_SIZE;

               AUDMEventData.EventData.RemoteControlConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;
               AUDMEventData.EventData.RemoteControlConnectionStatusEventData.RemoteDeviceAddress = RemoteControlOpenConfirmationData->BD_ADDR;

               /* If this was a synchronous event we need to set the    */
               /* status and the event.                                 */
               if(AudioEntryInfo->ConnectionEvent)
               {
                  /* Synchronous event, go ahead and set the correct    */
                  /* status, then set the event.                        */
                  AudioEntryInfo->ConnectionStatus = AUDMEventData.EventData.RemoteControlConnectionStatusEventData.ConnectionStatus;

                  BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);
               }
               else
               {
                  /* Note the Callback information.                     */
                  EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackParameter = AudioEntryInfo->CallbackParameter;

                  /* Go ahead and delete the entry (since we are        */
                  /* dispatching the event).                            */
                  if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo->CallbackID)) != NULL)
                     FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

                  /* Release the Lock so we can make the callback.      */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&AUDMEventData, CallbackParameter);
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
               ClientID = AudioEntryInfo->ClientID;

               /* Go ahead and delete the entry (since we are           */
               /* dispatching the event).                               */
               if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo->CallbackID)) != NULL)
                  FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

               BTPS_MemInitialize(&StatusMessage, 0, sizeof(StatusMessage));

               StatusMessage.MessageHeader.AddressID       = ClientID;
               StatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               StatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
               StatusMessage.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTION_STATUS;
               StatusMessage.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               StatusMessage.ConnectionStatus              = ConnectionStatus;
               StatusMessage.RemoteDeviceAddress           = RemoteControlOpenConfirmationData->BD_ADDR;

               /* Finally dispatch the Message.                         */
               MSG_SendMessage((BTPM_Message_t *)&StatusMessage);
            }

            /* Break out of the loop.                                   */
            AudioEntryInfo = NULL;
         }
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* Next, format up the Event to dispatch - ONLY if we need to     */
      /* dispatch a Connected Event.                                    */
      if(RemoteControlOpenConfirmationData->OpenStatus == AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_SUCCESS)
      {
         AUDMEventData.EventType                                                     = aetRemoteControlConnected;
         AUDMEventData.EventLength                                                   = AUDM_REMOTE_CONTROL_CONNECTED_EVENT_DATA_SIZE;

         AUDMEventData.EventData.RemoteControlConnectedEventData.RemoteDeviceAddress = RemoteControlOpenConfirmationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_CONNECTED;
         Message.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = RemoteControlOpenConfirmationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing a Remote     */
   /* Control Close Indication Event that has been received with the    */
   /* specified information. This function should be called with the    */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessRemoteControlCloseIndicationEvent(AUD_Remote_Control_Close_Indication_Data_t *RemoteControlCloseIndicationData)
{
   AUDM_Event_Data_t                           AUDMEventData;
   Control_Connection_Entry_t                 *ConnectionEntryPtr;
   AUDM_Remote_Control_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(RemoteControlCloseIndicationData)
   {
      /* We are disconnected so go ahea and delete the connection entry */
      /* for this connection.                                           */
      if((ConnectionEntryPtr = DeleteControlConnectionEntry(&RemoteControlConnectionList, RemoteControlCloseIndicationData->BD_ADDR)) != NULL)
         FreeControlConnectionEntryMemory(ConnectionEntryPtr);

      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                        = aetRemoteControlDisconnected;
      AUDMEventData.EventLength                                                      = AUDM_REMOTE_CONTROL_DISCONNECTED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.RemoteControlDisconnectedEventData.RemoteDeviceAddress = RemoteControlCloseIndicationData->BD_ADDR;
      AUDMEventData.EventData.RemoteControlDisconnectedEventData.DisconnectReason    = RemoteControlCloseIndicationData->DisconnectReason;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = RemoteControlCloseIndicationData->BD_ADDR;
      Message.DisconnectReason              = RemoteControlCloseIndicationData->DisconnectReason;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing an Encoded   */
   /* Stream Audio Data Indication Event that has been received with the*/
   /* specified information.  This function should be called with the   */
   /* Lock protecting the Audio Manager Information held.               */
static void ProcessEncodedAudioDataIndicationEvent(AUD_Encoded_Audio_Data_Indication_Data_t *EncodedAudioDataIndicatioData)
{
   void                                           *CallbackParameter;
   AUDM_Event_Data_t                               AUDMEventData;
   Audio_Entry_Info_t                             *AudioEntryInfo;
   AUDM_Event_Callback_t                           EventCallback;
   AUDM_RTP_Encoded_Audio_Data_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(EncodedAudioDataIndicatioData)
   {
      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      AudioEntryInfo = AudioEntryInfoDataList;
      while(AudioEntryInfo)
      {
         if(AudioEntryInfo->StreamType == astSNK)
            break;
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* Determine if we found an Audio Entry Event Handler.            */
      if(AudioEntryInfo)
      {
         /* Format up the Data.                                         */
         if(AudioEntryInfo->ClientID != MSG_GetServerAddressID())
         {
            /* Dispatch a Message Callback.                             */

            /* First, allocate enough memory to hold the Audio Data     */
            /* Message.                                                 */
            if((Message = (AUDM_RTP_Encoded_Audio_Data_Received_Message_t *)BTPS_AllocateMemory(AUDM_RTP_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(EncodedAudioDataIndicatioData->RawAudioDataFrameLength))) != NULL)
            {
               /* Memory allocated, format the message.                 */
               BTPS_MemInitialize(Message, 0, AUDM_RTP_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(0));

               Message->MessageHeader.AddressID        = AudioEntryInfo->ClientID;
               Message->MessageHeader.MessageID        = MSG_GetNextMessageID();
               Message->MessageHeader.MessageGroup     = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
               Message->MessageHeader.MessageFunction  = AUDM_MESSAGE_FUNCTION_RTP_ENCODED_AUDIO_DATA_RECEIVED;
               Message->MessageHeader.MessageLength    = AUDM_RTP_ENCODED_AUDIO_DATA_RECEIVED_MESSAGE_SIZE(EncodedAudioDataIndicatioData->RawAudioDataFrameLength) - BTPM_MESSAGE_HEADER_SIZE;

               Message->RemoteDeviceAddress            = EncodedAudioDataIndicatioData->BD_ADDR;
               Message->StreamDataEventsHandlerID      = AudioEntryInfo->CallbackID;
               Message->DataLength                     = EncodedAudioDataIndicatioData->RawAudioDataFrameLength;

               /* Check if RTP Header Information was specified.        */
               if(EncodedAudioDataIndicatioData->RTPHeaderInfo)
               {
                  /* RTP Header Information was specified, copy the RTP */
                  /* Header Information to the message.                 */
                  Message->RTPHeaderInfo = *(EncodedAudioDataIndicatioData->RTPHeaderInfo);
               }

               if(Message->DataLength)
                  BTPS_MemCopy(Message->Data, EncodedAudioDataIndicatioData->RawAudioDataFrame, Message->DataLength);

               /* All that is left to do is to dispatch the Event.      */
               MSG_SendMessage((BTPM_Message_t *)Message);

               /* Finished with the Message, so go ahead and delete     */
               /* the memory.                                           */
               BTPS_FreeMemory(Message);
            }
         }
         else
         {
            /* Dispatch Local Event Callback.                           */
            if(AudioEntryInfo->EventCallback)
            {
               AUDMEventData.EventType                                                           = aetEncodedAudioStreamData;
               AUDMEventData.EventLength                                                         = AUDM_ENCODED_AUDIO_STREAM_DATA_EVENT_DATA_SIZE;

               AUDMEventData.EventData.EncodedAudioStreamDataEventData.RemoteDeviceAddress       = EncodedAudioDataIndicatioData->BD_ADDR;
               AUDMEventData.EventData.EncodedAudioStreamDataEventData.StreamDataEventsHandlerID = AudioEntryInfo->CallbackID;
               AUDMEventData.EventData.EncodedAudioStreamDataEventData.RTPHeaderInfo             = EncodedAudioDataIndicatioData->RTPHeaderInfo;
               AUDMEventData.EventData.EncodedAudioStreamDataEventData.RawAudioDataFrameLength   = EncodedAudioDataIndicatioData->RawAudioDataFrameLength;
               AUDMEventData.EventData.EncodedAudioStreamDataEventData.RawAudioDataFrame         = EncodedAudioDataIndicatioData->RawAudioDataFrame;

               /* Event built, now dispatch it.                         */

               /* Note the Callback Information.                        */
               EventCallback     = AudioEntryInfo->EventCallback;
               CallbackParameter = AudioEntryInfo->CallbackParameter;

               /* Release the Lock because we have already noted the    */
               /* callback information.                                 */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  (*EventCallback)(&AUDMEventData, CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessBrowsingChannelOpenIndicationEvent(AUD_Browsing_Channel_Open_Indication_Data_t *BrowsingChannelOpenIndicationData)
{
   AUDM_Event_Data_t                                 AUDMEventData;
   Control_Connection_Entry_t                       *ConnectionEntryPtr;
   AUDM_Remote_Control_Browsing_Connected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(BrowsingChannelOpenIndicationData)
   {
      /* If a Control Connection Entry does not exist for this device go*/
      /* ahead and add one.                                             */
      if((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, BrowsingChannelOpenIndicationData->BD_ADDR)) != NULL)
         ConnectionEntryPtr->ConnectionState = scsConnectedBrowsing;

      /* Format up the Event to dispatch.                               */
      AUDMEventData.EventType                                                             = aetRemoteControlBrowsingConnected;
      AUDMEventData.EventLength                                                           = AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.RemoteControlBrowsingConnectedEventData.RemoteDeviceAddress = BrowsingChannelOpenIndicationData->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTED;
      Message.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = BrowsingChannelOpenIndicationData->BD_ADDR;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessBrowsingChannelOpenConfirmationEvent(AUD_Browsing_Channel_Open_Confirmation_Data_t *BrowsingChannelOpenConfirmationData)
{
   void                                                     *CallbackParameter;
   unsigned int                                              ClientID;
   unsigned int                                              ConnectionStatus;
   AUDM_Event_Data_t                                         AUDMEventData;
   Audio_Entry_Info_t                                       *AudioEntryInfo;
   AUDM_Event_Callback_t                                     EventCallback;
   Control_Connection_Entry_t                               *ConnectionEntryPtr;
   AUDM_Remote_Control_Browsing_Connected_Message_t          Message;
   AUDM_Remote_Control_Browsing_Connection_Status_Message_t  StatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(BrowsingChannelOpenConfirmationData)
   {
      /* Find the tracking structure for the remote control connection. */
      if((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, BrowsingChannelOpenConfirmationData->BD_ADDR)) != NULL)
      {
         /* Depending on the status go ahead and update the connection     */
         /* enty.                                                          */
         if(BrowsingChannelOpenConfirmationData->OpenStatus == AUD_BROWSE_CHANNEL_OPEN_CONFIRMATION_STATUS_SUCCESS)
            ConnectionEntryPtr->ConnectionState = scsConnectedBrowsing;
         else
            ConnectionEntryPtr->ConnectionState = scsConnected;
      }

      /* Map the Stream Open Confirmation Error to the correct Audio    */
      /* Manager Error Status.                                          */
      switch(BrowsingChannelOpenConfirmationData->OpenStatus)
      {
         case AUD_BROWSE_CHANNEL_OPEN_CONFIRMATION_STATUS_SUCCESS:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_SUCCESS;
            break;
         case AUD_BROWSE_CHANNEL_OPEN_CONFIRMATION_STATUS_CONNECTION_TIMEOUT:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_TIMEOUT;
            break;
         case AUD_BROWSE_CHANNEL_OPEN_CONFIRMATION_STATUS_CONNECTION_REFUSED:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_REFUSED;
            break;
         default:
            ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_UNKNOWN;
            break;
      }

      /* Dispatch any registered Connection Status Message/Event.       */
      AudioEntryInfo = OutgoingRemoteControlList;
      while(AudioEntryInfo)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Checking Audio Entry: %d, 0x%02X%02X%02X%02X%02X%02X\n", AudioEntryInfo->EventCallbackEntry, AudioEntryInfo->RemoteControlDevice.BD_ADDR5, AudioEntryInfo->RemoteControlDevice.BD_ADDR4, AudioEntryInfo->RemoteControlDevice.BD_ADDR3, AudioEntryInfo->RemoteControlDevice.BD_ADDR2, AudioEntryInfo->RemoteControlDevice.BD_ADDR1, AudioEntryInfo->RemoteControlDevice.BD_ADDR0));

         if((!AudioEntryInfo->EventCallbackEntry) && (COMPARE_BD_ADDR(AudioEntryInfo->RemoteControlDevice, BrowsingChannelOpenConfirmationData->BD_ADDR)))
         {
            /* Connection status registered, now see if we need to issue*/
            /* a Callack or an event.                                   */

            /* Determine if we need to dispatch the event locally or    */
            /* remotely.                                                */
            if(AudioEntryInfo->ClientID == MSG_GetServerAddressID())
            {
               /* Local Callback.                                       */

               /* Format up the Event.                                  */
               AUDMEventData.EventType                                                                    = aetRemoteControlBrowsingConnectionStatus;
               AUDMEventData.EventLength                                                                  = AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_EVENT_DATA_SIZE;

               AUDMEventData.EventData.RemoteControlBrowsingConnectionStatusEventData.ConnectionStatus    = ConnectionStatus;
               AUDMEventData.EventData.RemoteControlBrowsingConnectionStatusEventData.RemoteDeviceAddress = BrowsingChannelOpenConfirmationData->BD_ADDR;

               /* If this was a synchronous event we need to set the    */
               /* status and the event.                                 */
               if(AudioEntryInfo->ConnectionEvent)
               {
                  /* Synchronous event, go ahead and set the correct    */
                  /* status, then set the event.                        */
                  AudioEntryInfo->ConnectionStatus = AUDMEventData.EventData.RemoteControlBrowsingConnectionStatusEventData.ConnectionStatus;

                  BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);
               }
               else
               {
                  /* Note the Callback information.                     */
                  EventCallback     = AudioEntryInfo->EventCallback;
                  CallbackParameter = AudioEntryInfo->CallbackParameter;

                  /* Go ahead and delete the entry (since we are        */
                  /* dispatching the event).                            */
                  if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo->CallbackID)) != NULL)
                     FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

                  /* Release the Lock so we can make the callback.      */
                  DEVM_ReleaseLock();

                  __BTPSTRY
                  {
                     if(EventCallback)
                        (*EventCallback)(&AUDMEventData, CallbackParameter);
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
               ClientID = AudioEntryInfo->ClientID;

               /* Go ahead and delete the entry (since we are           */
               /* dispatching the event).                               */
               if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&OutgoingRemoteControlList, AudioEntryInfo->CallbackID)) != NULL)
                  FreeAudioEntryInfoEntryMemory(AudioEntryInfo);

               BTPS_MemInitialize(&StatusMessage, 0, sizeof(StatusMessage));

               StatusMessage.MessageHeader.AddressID       = ClientID;
               StatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
               StatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
               StatusMessage.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS;
               StatusMessage.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_BROWSING_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               StatusMessage.ConnectionStatus              = ConnectionStatus;
               StatusMessage.RemoteDeviceAddress           = BrowsingChannelOpenConfirmationData->BD_ADDR;

               /* Finally dispatch the Message.                         */
               MSG_SendMessage((BTPM_Message_t *)&StatusMessage);
            }

            /* Break out of the loop.                                   */
            AudioEntryInfo = NULL;
         }
         else
            AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
      }

      /* Next, format up the Event to dispatch - ONLY if we need to     */
      /* dispatch a Connected Event.                                    */
      if(BrowsingChannelOpenConfirmationData->OpenStatus == AUD_BROWSE_CHANNEL_OPEN_CONFIRMATION_STATUS_SUCCESS)
      {
         AUDMEventData.EventType                                                             = aetRemoteControlBrowsingConnected;
         AUDMEventData.EventLength                                                           = AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_EVENT_DATA_SIZE;

         AUDMEventData.EventData.RemoteControlBrowsingConnectedEventData.RemoteDeviceAddress = BrowsingChannelOpenConfirmationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
         Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_CONNECTED;
         Message.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_BROWSING_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = BrowsingChannelOpenConfirmationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessBrowsingChannelCloseIndicationEvent(AUD_Browsing_Channel_Close_Indication_Data_t *BrowsingChannelCloseIndicationData)
{
   AUDM_Event_Data_t                                    AUDMEventData;
   Control_Connection_Entry_t                          *ConnectionEntryPtr;
   AUDM_Remote_Control_Browsing_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(BrowsingChannelCloseIndicationData)
   {
      /* Browsing is disconnected. Move the state back to AV/C          */
      /* connection only.                                               */
      if((ConnectionEntryPtr = SearchControlConnectionEntry(&RemoteControlConnectionList, BrowsingChannelCloseIndicationData->BD_ADDR)) != NULL)
         ConnectionEntryPtr->ConnectionState = scsConnected;

      /* Next, format up the Event to dispatch.                         */
      AUDMEventData.EventType                                                                = aetRemoteControlBrowsingDisconnected;
      AUDMEventData.EventLength                                                              = AUDM_REMOTE_CONTROL_BROWSING_DISCONNECTED_EVENT_DATA_SIZE;

      AUDMEventData.EventData.RemoteControlBrowsingDisconnectedEventData.RemoteDeviceAddress = BrowsingChannelCloseIndicationData->BD_ADDR;

      /* Next, format up the Message to dispatch.                       */
      BTPS_MemInitialize(&Message, 0, sizeof(Message));

      Message.MessageHeader.AddressID       = 0;
      Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
      Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_AUDIO_MANAGER;
      Message.MessageHeader.MessageFunction = AUDM_MESSAGE_FUNCTION_REMOTE_CONTROL_BROWSING_DISCONNECTED;
      Message.MessageHeader.MessageLength   = (AUDM_REMOTE_CONTROL_BROWSING_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

      Message.RemoteDeviceAddress           = BrowsingChannelCloseIndicationData->BD_ADDR;

      /* Finally dispatch the formatted Event and Message.              */
      DispatchRemoteControlEvent(&AUDMEventData, (BTPM_Message_t *)&Message, AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the function that is responsible for    */
   /* processing Audio Events that have been received.  This function   */
   /* should ONLY be called with the Context locked AND ONLY in the     */
   /* context of an arbitrary processing thread.                        */
static void ProcessAudioEvent(AUDM_AUD_Event_Data_t *AUDEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(AUDEventData)
   {
      /* Process the event based on the event type.                     */
      switch(AUDEventData->EventType)
      {
         case etAUD_Open_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication: %s\n", (AUDEventData->EventData.OpenRequestIndicationData.ConnectionRequestType == acrStream)?"Audio Stream":"Remote Control"));

            ProcessOpenRequestIndicationEvent(&(AUDEventData->EventData.OpenRequestIndicationData));
            break;
         case etAUD_Stream_Open_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Stream Indication: %s\n", (AUDEventData->EventData.StreamOpenIndicationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamOpenIndicationEvent(&(AUDEventData->EventData.StreamOpenIndicationData));
            break;
         case etAUD_Stream_Open_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Stream Confirmation: %s\n", (AUDEventData->EventData.StreamOpenConfirmationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamOpenConfirmationEvent(&(AUDEventData->EventData.StreamOpenConfirmationData));
            break;
         case etAUD_Stream_Close_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Stream Indication: %s\n", (AUDEventData->EventData.StreamCloseIndicationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamCloseIndicationEvent(&(AUDEventData->EventData.StreamCloseIndicationData));
            break;
         case etAUD_Stream_State_Change_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Stream State Change Indication: %s\n", (AUDEventData->EventData.StreamStateChangeIndicationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamStateChangeIndicationEvent(&(AUDEventData->EventData.StreamStateChangeIndicationData));
            break;
         case etAUD_Stream_State_Change_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Stream State Change Confirmation: %s\n", (AUDEventData->EventData.StreamStateChangeConfirmationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamStateChangeConfirmationEvent(&(AUDEventData->EventData.StreamStateChangeConfirmationData));
            break;
         case etAUD_Stream_Format_Change_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Stream Format Change Indication: %s\n", (AUDEventData->EventData.StreamFormatChangeIndicationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamFormatChangeIndicationEvent(&(AUDEventData->EventData.StreamFormatChangeIndicationData));
            break;
         case etAUD_Stream_Format_Change_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Stream Format Change Confirmation: %s\n", (AUDEventData->EventData.StreamFormatChangeConfirmationData.StreamType == astSRC)?"Audio Source":"Audio Sink"));

            ProcessStreamFormatChangeConfirmationEvent(&(AUDEventData->EventData.StreamFormatChangeConfirmationData));
            break;
         case etAUD_Encoded_Audio_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Encoded Stream Audio Data Indication\n"));

            ProcessEncodedAudioDataIndicationEvent(&(AUDEventData->EventData.EncodedAudioDataIndicationData));
            break;
         case etAUD_Remote_Control_Command_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Command Indication\n"));

            ProcessRemoteControlCommandIndicationEvent(&(AUDEventData->EventData.AUDMRemoteControlCommandIndicationData));
            break;
         case etAUD_Remote_Control_Command_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Command Confirmation\n"));

            ProcessRemoteControlCommandConfirmationEvent(&(AUDEventData->EventData.AUDMRemoteControlCommandConfirmationData));
            break;
         case etAUD_Remote_Control_Open_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Open Indication\n"));

            ProcessRemoteControlOpenIndicationEvent(&(AUDEventData->EventData.RemoteControlOpenIndicationData));
            break;
         case etAUD_Remote_Control_Open_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Open Confirmation\n"));

            ProcessRemoteControlOpenConfirmationEvent(&(AUDEventData->EventData.RemoteControlOpenConfirmationData));
            break;
         case etAUD_Remote_Control_Close_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Remote Control Close Indication\n"));

            ProcessRemoteControlCloseIndicationEvent(&(AUDEventData->EventData.RemoteControlCloseIndicationData));
            break;
         case etAUD_Browsing_Channel_Open_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Browsing Channel Open Indication\n"));

            ProcessBrowsingChannelOpenIndicationEvent(&(AUDEventData->EventData.BrowsingChannelOpenIndicationData));
            break;
         case etAUD_Browsing_Channel_Open_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Browsing Channel Open Confirmation\n"));

            ProcessBrowsingChannelOpenConfirmationEvent(&(AUDEventData->EventData.BrowsingChannelOpenConfirmationData));
            break;
         case etAUD_Browsing_Channel_Close_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Browsing Channel Close Indication\n"));

            ProcessBrowsingChannelCloseIndicationEvent(&(AUDEventData->EventData.BrowsingChannelCloseIndicationData));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown Audio Event Type: %d\n", AUDEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid SPP Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Local Device Manager Power Asynchronous     */
   /* Events.                                                           */
static void ProcessPowerEvent(Boolean_t Power)
{
   unsigned int        LoopCount;
   Audio_Entry_Info_t *tmpAudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (AUDM)\n"));

   /* Power Dispatch Callback.                                          */

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Verify that this is flagging a power off event.                */
      if(!Power)
      {
         /* Power off event, let's loop through ALL the registered Audio*/
         /* Entries and set any events that have synchronous operations */
         /* pending.                                                    */
         AudioEntryInfo = AudioEntryInfoList;
         LoopCount      = 2;
         while(LoopCount)
         {
            /* Loop through the conneciton list.                        */
            while(AudioEntryInfo)
            {
               /* Check to see if there is a synchronous open operation.*/
               if(!AudioEntryInfo->EventCallbackEntry)
               {
                  if(AudioEntryInfo->ConnectionEvent)
                  {
                     if(LoopCount == 2)
                        AudioEntryInfo->ConnectionStatus = AUDM_STREAM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;
                     else
                        AudioEntryInfo->ConnectionStatus = AUDM_REMOTE_CONTROL_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(AudioEntryInfo->ConnectionEvent);

                     AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
                  }
                  else
                  {
                     /* Entry was waiting on a response, but it was     */
                     /* registered as either an Event Callback or       */
                     /* Connection Message.  Regardless we need to      */
                     /* delete it.                                      */
                     tmpAudioEntryInfo = AudioEntryInfo;

                     AudioEntryInfo    = AudioEntryInfo->NextAudioEntryInfoPtr;

                     if((tmpAudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, tmpAudioEntryInfo->CallbackID)) != NULL)
                        FreeAudioEntryInfoEntryMemory(tmpAudioEntryInfo);
                  }
               }
               else
                  AudioEntryInfo = AudioEntryInfo->NextAudioEntryInfoPtr;
            }

            /* Decrement the loop count.                                */
            --LoopCount;

            if(LoopCount == 1)
               AudioEntryInfo = OutgoingRemoteControlList;
         }

         /* Make sure the incoming Connection List is empty.            */
         FreeIncomingConnectionEntryList(&IncomingConnectionEntryList);

         /* Make sure the remote control connection list is empty.      */
         FreeControlConnectionEntryList(&RemoteControlConnectionList);

         /* Make sure the audio connection list is empty.               */
         FreeAudioConnectionEntryList(&AudioConnectionList);

         /* Inform the Audio Manager that the Stack has been closed.    */
         _AUDM_SetBluetoothStackID(0);
      }
      else
      {
         /* Power on Event.                                             */

         /* Go ahead and inform the Audio Manager that it should        */
         /* initialize.                                                 */
         _AUDM_SetBluetoothStackID((unsigned int)DEVM_QueryDeviceBluetoothStackID());
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (AUDM)\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Local Device Manager Asynchronous Events.   */
static void BTPSAPI BTPMDispatchCallback_DEVM(void *CallbackParameter)
{
   int                                          Result;
   Boolean_t                                    Done;
   Boolean_t                                    ProcessedSRC;
   Boolean_t                                    ProcessedSNK;
   Boolean_t                                    ProcessedRMC;
   BD_ADDR_t                                   *StreamBD_ADDR;
   AUD_Stream_Type_t                            StreamType;
   DEVM_Status_t                               *DEVMStatus;
   Connection_State_t                          *StreamConnectionState;
   Control_Connection_Entry_t                  *ControlConnectionEntry;
   AUD_Stream_Configuration_t                   StreamConfiguration;
   Audio_Connection_Entry_t                    *AudioConnectionEntry;
   Incoming_Connection_Entry_t                 *IncomingConnectionEntry;
   AUD_Stream_Open_Confirmation_Data_t          StreamOpenConfirmationData;
   AUD_Remote_Control_Open_Confirmation_Data_t  RemoteOpenConfirmationData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Status Event.  Check to see if there are outgoing        */
            /* connections.                                             */
            Done         = FALSE;
            ProcessedSRC = FALSE;
            ProcessedSNK = FALSE;
            ProcessedRMC = FALSE;
            StreamType   = astSNK;

            DEVMStatus = (DEVM_Status_t *)CallbackParameter;

            while(!Done)
            {
               /* Reset the Control Connection pointer to NULL.         */
               ControlConnectionEntry = NULL;
               AudioConnectionEntry = NULL;

               /* Check to see if a SRC is connecting (out-going).      */
               if((!ProcessedSRC) && ((AudioConnectionEntry = SearchAudioConnectionEntry(&AudioConnectionList, DEVMStatus->BD_ADDR, astSRC)) != NULL) && ((AudioConnectionEntry->ConnectionState == scsConnecting) || (AudioConnectionEntry->ConnectionState == scsConnectingAuthenticating) || (AudioConnectionEntry->ConnectionState == scsConnectingEncrypting)))
               {
                  StreamType            = astSRC;
                  StreamBD_ADDR         = &AudioConnectionEntry->BD_ADDR;
                  StreamConnectionState = &AudioConnectionEntry->ConnectionState;

                  ProcessedSRC          = TRUE;
               }
               else
               {
                  ProcessedSRC = TRUE;

                  /* Check to see if a SNK is connecting (out-going).   */
                  if((!ProcessedSNK) && ((AudioConnectionEntry = SearchAudioConnectionEntry(&AudioConnectionList, DEVMStatus->BD_ADDR, astSNK)) != NULL) && ((AudioConnectionEntry->ConnectionState == scsConnecting) || (AudioConnectionEntry->ConnectionState == scsConnectingAuthenticating) || (AudioConnectionEntry->ConnectionState == scsConnectingEncrypting)))
                  {
                     StreamType            = astSNK;
                     StreamBD_ADDR         = &AudioConnectionEntry->BD_ADDR;
                     StreamConnectionState = &AudioConnectionEntry->ConnectionState;

                     ProcessedSNK          = TRUE;
                  }
                  else
                  {
                     ProcessedSNK          = TRUE;

                     /* Check to see if this is an outgoing Remote      */
                     /* Control connection process.                     */
                     if((!ProcessedRMC) && ((ControlConnectionEntry = SearchControlConnectionEntry(&RemoteControlConnectionList, DEVMStatus->BD_ADDR)) != NULL))
                     {
                        ProcessedRMC          = TRUE;
                        StreamBD_ADDR         = &(ControlConnectionEntry->BD_ADDR);
                        StreamConnectionState = &(ControlConnectionEntry->ConnectionState);
                     }
                     else
                     {
                        ProcessedRMC          = TRUE;
                        Done                  = TRUE;
                     }
                  }
               }

               /* Only continue if there is something to process.       */
               if(!Done)
               {
                  /* Initialize common connection event members.        */
                  BTPS_MemInitialize(&StreamOpenConfirmationData, 0, sizeof(StreamOpenConfirmationData));

                  StreamOpenConfirmationData.StreamType = StreamType;
                  StreamOpenConfirmationData.BD_ADDR    = *StreamBD_ADDR;

                  BTPS_MemInitialize(&RemoteOpenConfirmationData, 0, sizeof(RemoteOpenConfirmationData));

                  RemoteOpenConfirmationData.BD_ADDR    = *StreamBD_ADDR;

                  /* Connecting, let's process based on the status.     */
                  if(DEVMStatus->Status)
                  {
                     /* Error, go ahead and disconnect the device.      */
                     DEVM_DisconnectRemoteDevice(*StreamBD_ADDR, FALSE);

                     /* Connection Failed.                              */

                     /* Map the status to a known status.               */
                     switch(DEVMStatus->Status)
                     {
                        case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
                        case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
                           StreamOpenConfirmationData.OpenStatus = AUD_STREAM_OPEN_CONFIRMATION_STATUS_CONNECTION_REFUSED;
                           RemoteOpenConfirmationData.OpenStatus = AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_CONNECTION_REFUSED;
                           break;
                        case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
                        case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                           StreamOpenConfirmationData.OpenStatus = AUD_STREAM_OPEN_CONFIRMATION_STATUS_CONNECTION_TIMEOUT;
                           RemoteOpenConfirmationData.OpenStatus = AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_CONNECTION_TIMEOUT;
                           break;
                        default:
                           StreamOpenConfirmationData.OpenStatus = AUD_STREAM_OPEN_CONFIRMATION_STATUS_UNKNOWN_ERROR;
                           RemoteOpenConfirmationData.OpenStatus = AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_UNKNOWN_ERROR;
                           break;
                     }

                     /* Call the correct open confirmation function to  */
                     /* perform the rest of the processing.             */
                     if(ControlConnectionEntry == NULL)
                     {
                        /* * NOTE * This function will delete the Audio */
                        /*          Info entry from the list.           */
                        ProcessStreamOpenConfirmationEvent(&StreamOpenConfirmationData);
                     }
                     else
                     {
                        /* * NOTE * This function will delete the Audio */
                        /*          Info entry from the list.           */
                        ProcessRemoteControlOpenConfirmationEvent(&RemoteOpenConfirmationData);
                     }
                  }
                  else
                  {
                     /* Connection succeeded.                           */

                     /* Move the state to the connecting state.         */
                     *StreamConnectionState = scsConnectingStream;

                     /* Call the correct function to open the stream.   */
                     if(ControlConnectionEntry == NULL)
                        Result = _AUDM_Connect_Audio_Stream(*StreamBD_ADDR, StreamType);
                     else
                        Result = _AUDM_Connect_Remote_Control(*StreamBD_ADDR);

                     if(Result != 0)
                     {
                        if(Result == BTAUD_ERROR_STREAM_ALREADY_CONNECTED)
                        {
                           /* Note that this case should not occur.     */
                           *StreamConnectionState = scsConnected;

                           /* Connection Success.                       */
                           StreamOpenConfirmationData.OpenStatus = AUD_STREAM_OPEN_CONFIRMATION_STATUS_SUCCESS;
                           RemoteOpenConfirmationData.OpenStatus = AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_SUCCESS;

                           /* Handle this based on the type of AUD      */
                           /* connection that was just made.            */
                           if((ControlConnectionEntry == NULL) && (AudioConnectionEntry != NULL))
                           {
                              /* We need to fill in the remaining       */
                              /* information.                           */
                              _AUDM_Query_Audio_Stream_Configuration(AudioConnectionEntry->BD_ADDR, StreamType, &StreamConfiguration);

                              StreamOpenConfirmationData.MediaMTU     = StreamConfiguration.MediaMTU;
                              StreamOpenConfirmationData.StreamFormat = StreamConfiguration.StreamFormat;

                              ProcessStreamOpenConfirmationEvent(&StreamOpenConfirmationData);
                           }
                           else
                              ProcessRemoteControlOpenConfirmationEvent(&RemoteOpenConfirmationData);
                        }
                        else
                        {
                           /* Error, go ahead and disconnect the device.*/
                           DEVM_DisconnectRemoteDevice(*StreamBD_ADDR, FALSE);

                           /* Connection Failure.                       */
                           StreamOpenConfirmationData.OpenStatus = AUD_STREAM_OPEN_CONFIRMATION_STATUS_UNKNOWN_ERROR;
                           RemoteOpenConfirmationData.OpenStatus = AUD_REMOTE_CONTROL_OPEN_CONFIRMATION_STATUS_UNKNOWN_ERROR;

                           /* Call the correct open confirmation        */
                           /* function to perform the rest of the       */
                           /* processing.                               */
                           if(ControlConnectionEntry == NULL)
                           {
                              /* * NOTE * This function will delete the */
                              /*          Audio Info entry from the     */
                              /*          list.                         */
                              ProcessStreamOpenConfirmationEvent(&StreamOpenConfirmationData);
                           }
                           else
                           {
                              /* * NOTE * This function will delete the */
                              /*          Audio Info entry from the     */
                              /*          list.                         */
                              ProcessRemoteControlOpenConfirmationEvent(&RemoteOpenConfirmationData);
                           }
                        }
                     }
                  }
               }
            }

            /* Check to see if this event referenced an incoming        */
            /* connection request.                                      */
            if(DEVMStatus->StatusType != dstConnection)
            {
               Done = FALSE;
               while(!Done)
               {
                  /* Check to see if there is an incoming stream        */
                  /* request.                                           */
                  if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, DEVMStatus->BD_ADDR, acrStream)) != NULL)
                  {
                     /* If successful, accept, otherwise, reject.       */
                     DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("%s", DEVMStatus->Status?"Reject":"Accept"));
                     _AUDM_Connection_Request_Response(IncomingConnectionEntry->RequestType, IncomingConnectionEntry->BD_ADDR, (Boolean_t)(DEVMStatus->Status?FALSE:TRUE));

                     /* Finished with this entry, free the memory.      */
                     FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
                  }
                  else
                  {
                     if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, DEVMStatus->BD_ADDR, acrRemoteControl)) != NULL)
                     {
                        /* If successful, accept, otherwise, reject.    */
                        _AUDM_Connection_Request_Response(IncomingConnectionEntry->RequestType, IncomingConnectionEntry->BD_ADDR, (Boolean_t)(DEVMStatus->Status?FALSE:TRUE));

                        /* Finished with this entry, free the memory.   */
                        FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
                     }
                     else
                        Done = TRUE;
                  }
               }
            }
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Audio Manager Asynchronous Events.          */
static void BTPSAPI BTPMDispatchCallback_AUDM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Audio Manager Notification Events.          */
static void BTPSAPI BTPMDispatchCallback_AUD(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            /* Double check that this is an Audio Event Update.         */
            if(((AUDM_Update_Data_t *)CallbackParameter)->UpdateType == utAUDEvent)
            {
               /* Process the Notification.                             */
               ProcessAudioEvent(&(((AUDM_Update_Data_t *)CallbackParameter)->UpdateData.AUDEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is the Dispatch Callback function that is  */
   /* registered to process Client Un-Register Asynchronous Events.     */
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function represents the Message Group Handler       */
   /* function that is installed to process all Audio Manager Messages. */
static void BTPSAPI AudioManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_AUDIO_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

         /* Message is destined for us, make sure we are still          */
         /* processing messages.                                        */
         if(Initialized)
         {
            /* Check to see if this message is an Audio Manager defined */
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
               /* Audio Manager Thread.                                 */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_AUDM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Audio Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue Audio Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
                     }
                  }
               }
            }
         }
         else
         {
            /* Check to see if this message is an Audio Manager defined */
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
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Non Audio Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Audio Manager Module.  This        */
   /* function should be registered with the Bluetopia Platform Manager */
   /* Module Handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI AUDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int                         Result;
   AUDM_Initialization_Data_t *InitializationInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      /* Check to see if this module has already been initialized.      */
      if(!Initialized)
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing Audio Manager\n"));

         /* Make sure that there is actual SRC/SNK data specified.      */
         if(((InitializationInfo = (AUDM_Initialization_Data_t *)InitializationData) != NULL) && ((InitializationInfo->SRCInitializationInfo) || (InitializationInfo->SNKInitializationInfo) || (InitializationInfo->RemoteControlInitializationInfo)))
         {
            /* Next, let's attempt to register our Message Group Handler*/
            /* to process Audio Manager messages.                       */
            if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_AUDIO_MANAGER, AudioManagerGroupHandler, NULL))
            {
               /* Initialize the actual Audio Manager Implementation    */
               /* Module (this is the module that is actually           */
               /* responsible for actually implementing the Audio       */
               /* Manager functionality - this module is just the       */
               /* framework shell).                                     */
               if(!(Result = _AUDM_Initialize(InitializationInfo)))
               {
                  /* Note the incoming Stream Flags.                    */
                  IncomingConnectionFlags = InitializationInfo->IncomingConnectionFlags;

                  /* Finally determine the current Device Power State.  */
                  CurrentPowerState       = (Boolean_t)((DEVM_QueryDevicePowerState() > 0)?TRUE:FALSE);

                  /* Initialize a unique, starting Audio Callback ID.   */
                  NextCallbackID          = 0x000000001;

                  /* Go ahead and flag that this module is initialized. */
                  Initialized             = TRUE;
               }
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER;

            /* If an error occurred then we need to free all resources  */
            /* that were allocated.                                     */
            if(Result)
            {
               _AUDM_Cleanup();

               MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_AUDIO_MANAGER);
            }
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Audio Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_AUDIO_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the Audio Manager Implementation that*/
            /* we are shutting down.                                    */
            _AUDM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeIncomingConnectionEntryList(&IncomingConnectionEntryList);

            /* Make sure the remote control connection list is empty.   */
            FreeControlConnectionEntryList(&RemoteControlConnectionList);

            /* Make sure the audio connection list is empty.            */
            FreeAudioConnectionEntryList(&AudioConnectionList);

            /* Make sure that the Audio Entry Information List is empty.*/
            FreeAudioEntryInfoList(&AudioEntryInfoList);

            /* Make sure that the Audio Entry Data Information List is  */
            /* empty.                                                   */
            FreeAudioEntryInfoList(&AudioEntryInfoDataList);

            /* Make sure that the Outgoing Audio Entry Remote Control   */
            /* Information List is empty.                               */
            FreeAudioEntryInfoList(&OutgoingRemoteControlList);

            /* Make sure that the Audio Entry Remote Control Information*/
            /* List is empty.                                           */
            FreeAudioEntryInfoList(&AudioEntryInfoRemoteControlList);

            /* Flag that the resources are no longer allocated.         */
            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI AUDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   DEVM_Status_t *DEVMStatus;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               ProcessPowerEvent(TRUE);
               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               ProcessPowerEvent(FALSE);
               break;
            case detRemoteDeviceAuthenticationStatus:
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Authentication Status.\n"));

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
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Encryption Status.\n"));

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
               DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Connection Status.\n"));

               if((DEVMStatus = (DEVM_Status_t *)BTPS_AllocateMemory(sizeof(DEVM_Status_t))) != NULL)
               {
                  DEVMStatus->StatusType = dstConnection;
                  DEVMStatus->BD_ADDR    = EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress;
                  DEVMStatus->Status     = EventData->EventData.RemoteDeviceConnectionStatusEventData.Status;

                  if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_DEVM, (void *)DEVMStatus))
                     BTPS_FreeMemory(DEVMStatus);
               }
               break;
            default:
               /* Do nothing.                                           */
               break;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Audio Manager of a specific Update Event.  The Audio*/
   /* Manager can then take the correct action to process the update.   */
Boolean_t AUDM_NotifyUpdate(AUDM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utAUDEvent:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing Audio Event: %d\n", UpdateData->UpdateData.AUDEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_AUD, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  An Audio Stream   */
   /*          Connected event will be dispatched to signify the actual */
   /*          result.                                                  */
int BTPSAPI AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int                          ret_val;
   BD_ADDR_t                    NULL_BD_ADDR;
   Boolean_t                    Authenticate;
   Boolean_t                    Encrypt;
   Incoming_Connection_Entry_t *IncomingConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   /* First, check to make sure the Audio Manager has been initialized. */
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
               if(((IncomingConnectionEntry = SearchIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteDeviceAddress, RequestType)) != NULL) && (IncomingConnectionEntry->IncomingConnectionState == icsAuthorizing))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %s, %d\n", ((RequestType == acrStream)?"Audio Stream":"Remote Control"), Accept));

                  /* If the caller has accepted the request then we need*/
                  /* to process it differently.                         */
                  if(Accept)
                  {
                     /* Determine if Authentication and/or Encryption is*/
                     /* required for this link.                         */
                     if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                        Authenticate = TRUE;
                     else
                        Authenticate = FALSE;

                     if(IncomingConnectionFlags & AUDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                        Encrypt = TRUE;
                     else
                        Encrypt = FALSE;

                     if((Authenticate) || (Encrypt))
                     {
                        if(Encrypt)
                           ret_val = DEVM_EncryptRemoteDevice(IncomingConnectionEntry->BD_ADDR, 0);
                        else
                           ret_val = DEVM_AuthenticateRemoteDevice(IncomingConnectionEntry->BD_ADDR, 0);
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED;

                     if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                     {
                        /* Authorization not required, and we are       */
                        /* already in the correct state.                */
                        ret_val = _AUDM_Connection_Request_Response(RequestType, IncomingConnectionEntry->BD_ADDR, TRUE);

                        /* Go ahead and delete the entry because we are */
                        /* finished with tracking it.                   */
                        if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteDeviceAddress, RequestType)) != NULL)
                           FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
                     }
                     else
                     {
                        /* If we were successfully able to Authenticate */
                        /* and/or Encrypt, then we need to set the      */
                        /* correct state.                               */
                        if(!ret_val)
                        {
                           if(Encrypt)
                              IncomingConnectionEntry->IncomingConnectionState = icsEncrypting;
                           else
                              IncomingConnectionEntry->IncomingConnectionState = icsAuthenticating;

                           /* Flag success to the caller.               */
                           ret_val = 0;
                        }
                        else
                        {
                           /* Error, reject the request.                */
                           _AUDM_Connection_Request_Response(RequestType, IncomingConnectionEntry->BD_ADDR, FALSE);

                           /* Go ahead and delete the entry because we  */
                           /* are finished with tracking it.            */
                           if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteDeviceAddress, RequestType)) != NULL)
                              FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
                        }
                     }
                  }
                  else
                  {
                     /* Rejection - Simply respond to the request.      */
                     ret_val = _AUDM_Connection_Request_Response(RequestType, IncomingConnectionEntry->BD_ADDR, FALSE);

                     /* Go ahead and delete the entry because we are    */
                     /* finished with tracking it.                      */
                     if((IncomingConnectionEntry = DeleteIncomingConnectionEntry(&IncomingConnectionEntryList, RemoteDeviceAddress, RequestType)) != NULL)
                        FreeIncomingConnectionEntryMemory(IncomingConnectionEntry);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect an Audio Stream to a remote device.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Audio Stream Connection Status Event (if specified).     */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          aetAudioStreamConnectionStatus event will be dispatched  */
   /*          to to denote the status of the connection.  This is the  */
   /*          ONLY way to receive this event, as an event callack      */
   /*          registered with the AUDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned long StreamFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                       ret_val;
   Event_t                   ConnectionEvent;
   BD_ADDR_t                 NULL_BD_ADDR;
   unsigned int              CallbackID;
   Audio_Entry_Info_t        AudioEntryInfo;
   Audio_Entry_Info_t       *AudioEntryInfoPtr;
   Audio_Connection_Entry_t  AudioConnectionEntry;
   Audio_Connection_Entry_t *AudioConnectionEntryPtr;

   /* Format a NULL BD_ADDR to test against.                            */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0, 0, 0, 0, 0, 0);

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
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
               /* Device is powered on, next, verify that we are not    */
               /* already tracking a connection for the specified       */
               /* connection type.                                      */
               if((AudioConnectionEntryPtr = SearchAudioConnectionEntry(&AudioConnectionList, RemoteDeviceAddress, StreamType)) == NULL)
               {
                  /* Attempt to add an entry into the Audio Entry list. */
                  BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

                  AudioEntryInfo.CallbackID        = GetNextCallbackID();
                  AudioEntryInfo.ClientID          = MSG_GetServerAddressID();
                  AudioEntryInfo.EventCallback     = CallbackFunction;
                  AudioEntryInfo.CallbackParameter = CallbackParameter;
                  AudioEntryInfo.StreamType        = StreamType;

                  if(ConnectionStatus)
                     AudioEntryInfo.ConnectionEvent = BTPS_CreateEvent(FALSE);

                  if((!ConnectionStatus) || ((ConnectionStatus) && (AudioEntryInfo.ConnectionEvent)))
                  {
                     if((AudioEntryInfoPtr = AddAudioEntryInfoEntry(&AudioEntryInfoList, &AudioEntryInfo)) != NULL)
                     {
                        /* Add the connection entry.                    */
                        BTPS_MemInitialize(&AudioConnectionEntry, 0, sizeof(Audio_Connection_Entry_t));

                        AudioConnectionEntry.BD_ADDR    = RemoteDeviceAddress;
                        AudioConnectionEntry.StreamType = StreamType;

                        if((AudioConnectionEntryPtr = AddAudioConnectionEntry(&AudioConnectionList, &AudioConnectionEntry)) != NULL)
                        {

                           DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Stream %d, 0x%08lX\n", (unsigned int)StreamType, StreamFlags));

                           /* Next, attempt to open the remote stream.  */
                           if(StreamFlags & AUDM_CONNECT_AUDIO_STREAM_FLAGS_REQUIRE_ENCRYPTION)
                              AudioConnectionEntryPtr->ConnectionState = scsConnectingEncrypting;
                           else
                           {
                              if(StreamFlags & AUDM_CONNECT_AUDIO_STREAM_FLAGS_REQUIRE_AUTHENTICATION)
                                 AudioConnectionEntryPtr->ConnectionState = scsConnectingAuthenticating;
                              else
                                 AudioConnectionEntryPtr->ConnectionState = scsConnecting;
                           }

                           DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Stream %s, 0x%08lX\n", (StreamType == astSRC)?"Sink":"Source", StreamFlags));

                           ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (AudioConnectionEntryPtr->ConnectionState == scsConnecting)?0:((AudioConnectionEntryPtr->ConnectionState == scsConnectingEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

                           if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
                           {
                              /* Check to see if we need to actually    */
                              /* issue the Remote Stream connection.    */
                              if(ret_val < 0)
                              {
                                 /* Set the state to connecting remote  */
                                 /* stream.                             */
                                 AudioConnectionEntryPtr->ConnectionState = scsConnectingStream;

                                 if((ret_val = _AUDM_Connect_Audio_Stream(RemoteDeviceAddress, StreamType)) != 0)
                                 {
                                    if((ret_val != BTPM_ERROR_CODE_AUDIO_STREAM_ALREADY_CONNECTED) && (ret_val != BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS))
                                       ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_AUDIO_STREAM;

                                    /* Delete the connection entry.     */
                                    if((AudioConnectionEntryPtr = DeleteAudioConnectionEntry(&AudioConnectionList, RemoteDeviceAddress, StreamType)) != NULL)
                                       FreeAudioConnectionEntryMemory(AudioConnectionEntryPtr);

                                    /* Error opening stream, go ahead   */
                                    /* and delete the entry that was    */
                                    /* added.                           */
                                    if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfoPtr->CallbackID)) != NULL)
                                    {
                                       if(AudioEntryInfoPtr->ConnectionEvent)
                                          BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                       FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                                    }
                                 }
                              }
                           }
                           else
                           {
                              if(ret_val < 0)
                              {
                                 /* Delete the connection entry.        */
                                 if((AudioConnectionEntryPtr = DeleteAudioConnectionEntry(&AudioConnectionList, RemoteDeviceAddress, StreamType)) != NULL)
                                    FreeAudioConnectionEntryMemory(AudioConnectionEntryPtr);
                              }
                           }

                           /* Next, determine if the caller has         */
                           /* requested a blocking open.                */
                           if((!ret_val) && (ConnectionStatus))
                           {
                              /* Blocking open, go ahead and wait for   */
                              /* the event.                             */

                              /* Note the Callback ID.                  */
                              CallbackID      = AudioEntryInfoPtr->CallbackID;

                              /* Note the Open Event.                   */
                              ConnectionEvent = AudioEntryInfoPtr->ConnectionEvent;

                              /* Release the lock because we are        */
                              /* finished with it.                      */
                              DEVM_ReleaseLock();

                              /* * NOTE * If the connection is not      */
                              /*          The bluetopia callback handler*/
                              /*          will remove the connection    */
                              /*          entry from the list.          */
                              BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

                              /* Re-acquire the Lock.                   */
                              if(DEVM_AcquireLock())
                              {
                                 if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, CallbackID)) != NULL)
                                 {
                                    /* Note the connection status.      */
                                    *ConnectionStatus = AudioEntryInfoPtr->ConnectionStatus;

                                    BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                    FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);

                                    /* Flag success to the caller.      */
                                    ret_val = 0;
                                 }
                                 else
                                    ret_val = BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_AUDIO_STREAM;
                              }
                              else
                                 ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
                           }
                           else
                           {
                              /* If we are not tracking this connection */
                              /* OR there was an error, go ahead and    */
                              /* delete the entry that was added.       */
                              if((!CallbackFunction) || (ret_val))
                              {
                                 if((AudioEntryInfoPtr = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioEntryInfo.CallbackID)) != NULL)
                                 {
                                    if(AudioEntryInfoPtr->ConnectionEvent)
                                       BTPS_CloseEvent(AudioEntryInfoPtr->ConnectionEvent);

                                    FreeAudioEntryInfoEntryMemory(AudioEntryInfoPtr);
                                 }
                              }
                           }
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;
               }
               else
               {
                  if(AudioConnectionEntryPtr->ConnectionState == scsConnected)
                     ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ALREADY_CONNECTED;
                  else
                  {
                     /* We should only be tracking the connection if it */
                     /* is connected or connecting.                     */
                     ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS;
                  }
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType)
{
   int                                 ret_val;
   AUD_Stream_Close_Indication_Data_t  StreamCloseIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Disconnect the Audio Stream.                          */
            ret_val = _AUDM_Disconnect_Audio_Stream(RemoteDeviceAddress, StreamType);

            /* If the result was successful, we need to make sure       */
            /* we clean up everything and dispatch the event to all     */
            /* registered clients.                                      */
            if(!ret_val)
            {
               /* Fake a Stream Close Event to dispatch to all          */
               /* registered clients that the Stream has been closed.   */
               StreamCloseIndicationData.StreamType       = StreamType;
               StreamCloseIndicationData.DisconnectReason = adrRemoteDeviceDisconnect;
               StreamCloseIndicationData.BD_ADDR          = RemoteDeviceAddress;

               ProcessStreamCloseIndicationEvent(&StreamCloseIndicationData);
            }

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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to determine if there are currently any connected   */
   /* Audio sessions of the specified role (specified by the first      */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of device address entries that the buffer will     */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI AUDM_Query_Audio_Connected_Devices(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply call the internal function to do all of the work. */
            ret_val = ProcessQueryAudioConnections(StreamType, MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream state of the     */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream State of the Audio Stream (if*/
   /* this function is successful).                                     */
int BTPSAPI AUDM_Query_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Query the Audio Stream State.                         */
            ret_val = _AUDM_Query_Audio_Stream_State(RemoteDeviceAddress, StreamType, StreamState);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream format of the    */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream Format of the Audio Stream   */
   /* (if this function is successful).                                 */
int BTPSAPI AUDM_Query_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Query the Audio Stream Format.                        */
            ret_val = _AUDM_Query_Audio_Stream_Format(RemoteDeviceAddress, StreamType, StreamFormat);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to start/suspend the specified Audio Stream.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int BTPSAPI AUDM_Change_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Start the Audio Stream.                               */
            ret_val = _AUDM_Change_Audio_Stream_State(RemoteDeviceAddress, StreamType, StreamState);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the stream format of a currently connected (but */
   /* suspended) Audio Stream.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The stream format can ONLY be changed when the stream    */
   /*          state is stopped.                                        */
int BTPSAPI AUDM_Change_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Start the Audio Stream.                               */
            ret_val = _AUDM_Change_Audio_Stream_Format(RemoteDeviceAddress, StreamType, StreamFormat);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream configuration of */
   /* the specified Audio Stream.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* The final parameter will hold the Audio Stream Configuration of   */
   /* the Audio Stream (if this function is successful).                */
int BTPSAPI AUDM_Query_Audio_Stream_Configuration(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Query the Audio Stream Configuration.                 */
            ret_val = _AUDM_Query_Audio_Stream_Configuration(RemoteDeviceAddress, StreamType, StreamConfiguration);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for Audio Manager */
   /* Connections (Audio Streams and Remote Control).  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI AUDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          AUDM_Query_Audio_Stream_Configuration() function.        */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int BTPSAPI AUDM_Send_Encoded_Audio_Data(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerDataEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioManagerDataEventCallbackID)) != NULL)
            {
               /* Double check that the type is an Audio Source.        */
               if(AudioEntryInfo->StreamType == astSRC)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Encoded Audio Data.           */
                  ret_val = _AUDM_Send_Encoded_Audio_Data(RemoteDeviceAddress, RawAudioDataFrameLength, RawAudioDataFrame);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send, followed by flags which  */
   /* specify the format of the data (currently not used, this parameter*/
   /* is reserved for future additions), followed by the RTP Header     */
   /* Information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          AUDM_Query_Audio_Stream_Configuration() function.        */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
   /* * NOTE * This is a low level function and allows the user to      */
   /*          specify the RTP Header Information for the outgoing data */
   /*          packet.  To use the default values for the RTP Header    */
   /*          Information use AUDM_Send_Encoded_Audio_Data() instead.  */
int BTPSAPI AUDM_Send_RTP_Encoded_Audio_Data(unsigned int AudioManagerDataEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerDataEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioManagerDataEventCallbackID)) != NULL)
            {
               /* Double check that the type is an Audio Source.        */
               if(AudioEntryInfo->StreamType == astSRC)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Encoded Audio Data.           */
                  ret_val = _AUDM_Send_RTP_Encoded_Audio_Data(RemoteDeviceAddress, RawAudioDataFrameLength, RawAudioDataFrame, Flags, RTPHeaderInfo);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a Remote Control connection to a remote      */
   /* device. This function returns zero if successful, or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Remote Control Connection Status Event (if specified).   */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          aetRemoteControlConnectionStatus event will be dispatched*/
   /*          to to denote the status of the connection.  This is the  */
   /*          ONLY way to receive this event, as an event callback     */
   /*          registered with the AUDM_Register_Event_Callback() or    */
   /*          AUDM_Register_Remote_Control_Event_Callback() functions  */
   /*          will NOT receive connection status events.               */
int BTPSAPI AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Simply wrap the internal function to do all of the work.    */
         ret_val = ConnectRemoteControlDevice(MSG_GetServerAddressID(), RemoteDeviceAddress, ConnectionFlags, ConnectionStatus, CallbackFunction, CallbackParameter);

         /* Release the Lock because we are finished with it if the     */
         /* internal function did not flag that it had trouble          */
         /* re-acquiring the lock.                                      */
         if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
            DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Remote Control        */
   /* session.  This function returns zero if successful, or a negative */
   /* return error code if there was an error.                          */
int BTPSAPI AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress)
{
   int                                        ret_val;
   AUD_Remote_Control_Close_Indication_Data_t CloseIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Disconnect the Remote Control.                        */
            ret_val = _AUDM_Disconnect_Remote_Control(RemoteDeviceAddress);

            /* If the result was successful, we need to make sure we    */
            /* clean up everything and dispatch the event to all        */
            /* registered clients.                                      */
            if(!ret_val)
            {
               /* Fake a Close Event to dispatch to all registered      */
               /* clients that the Stream has been closed.              */
               CloseIndicationData.BD_ADDR          = RemoteDeviceAddress;
               CloseIndicationData.DisconnectReason = adrRemoteDeviceDisconnect;

               ProcessRemoteControlCloseIndicationEvent(&CloseIndicationData);
            }
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Remote  */
   /* Control sessions (specified by the first parameter). This function*/
   /* accepts a the local service type to query, followed by buffer     */
   /* information to receive any currently connected device addresses   */
   /* of the specified connection type. The first parameter specifies   */
   /* the local service type to query the connection information for.   */
   /* The second parameter specifies the maximum number of BD_ADDR      */
   /* entries that the buffer will support (i.e. can be copied into     */
   /* the buffer). The next parameter is optional and, if specified,    */
   /* will be populated with the total number of connected devices if   */
   /* the function is successful. The final parameter can be used to    */
   /* retrieve the total number of connected devices (regardless of     */
   /* the size of the list specified by the first two parameters).      */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of connected devices that were copied into  */
   /* the specified input buffer. This function returns a negative      */
   /* return error code if there was an error.                          */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI AUDM_Query_Remote_Control_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Simply call the internal function to do all of the work. */
            ret_val = ProcessQueryRemoteControlConnections(MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Remote Control Command to the remote Device.  This function       */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* AUDM_Register_Remote_Control_Event_Callback() function), followed */
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Response Timeout (in milliseconds), followed by a */
   /* pointer to the actual Remote Control Message to send.  This       */
   /* function returns a positive, value if successful or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Transaction ID of the Remote Control Event that was  */
   /*          submitted.                                               */
int BTPSAPI AUDM_Send_Remote_Control_Command(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerRemoteControlEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioManagerRemoteControlEventCallbackID)) != NULL)
            {
               /* Double check that the type is supported.              */
               if(AudioEntryInfo->ConnectionStatus & AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Remote Control Data.          */
                  ret_val = _AUDM_Send_Remote_Control_Command(RemoteDeviceAddress, ResponseTimeout, CommandData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is responsible for sending the specified   */
   /* Remote Control Response to the remote Device.  This function      */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* AUDM_Register_Remote_Control_Event_Callback() function), followed */
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Transaction ID of the Remote Control Event,       */
   /* followed by a pointer to the actual Remote Control Response       */
   /* Message to send.  This function returns zero if successful or a   */
   /* negative return error code if there was an error.                 */
int BTPSAPI AUDM_Send_Remote_Control_Response(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData)
{
   int                 ret_val;
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerRemoteControlEventCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* First, find the local handler.                           */
            if((AudioEntryInfo = SearchAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioManagerRemoteControlEventCallbackID)) != NULL)
            {
               /* Double check that the type is supported.              */
               if(AudioEntryInfo->ConnectionStatus & AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET)
               {
                  /* Nothing to do here other than to call the actual   */
                  /* function to send the Remote Control Data.          */
                  ret_val = _AUDM_Send_Remote_Control_Response(RemoteDeviceAddress, TransactionID, ResponseData);
               }
               else
                  ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED;
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
         ret_val = BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Audio Manager    */
   /* Service.  This Callback will be dispatched by the Audio Manager   */
   /* when various Audio Manager Events occur.  This function accepts   */
   /* the Callback Function and Callback Parameter (respectively) to    */
   /* call when an Audio Manager Event needs to be dispatched.  This    */
   /* function returns a positive (non-zero) value if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          AUDM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
int BTPSAPI AUDM_Register_Event_Callback(AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                ret_val;
   Audio_Entry_Info_t AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Attempt to add an entry into the Audio Entry list.       */
            BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

            AudioEntryInfo.CallbackID         = GetNextCallbackID();
            AudioEntryInfo.ClientID           = MSG_GetServerAddressID();
            AudioEntryInfo.EventCallback      = CallbackFunction;
            AudioEntryInfo.CallbackParameter  = CallbackParameter;
            AudioEntryInfo.EventCallbackEntry = TRUE;

            if(AddAudioEntryInfoEntry(&AudioEntryInfoList, &AudioEntryInfo))
               ret_val = AudioEntryInfo.CallbackID;
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Callback  */
   /* (registered via a successful call to the                          */
   /* AUDM_Register_Event_Callback() function).  This function accepts  */
   /* as input the Audio Manager Event Callback ID (return value from   */
   /* AUDM_Register_Event_Callback() function).                         */
void BTPSAPI AUDM_Un_Register_Event_Callback(unsigned int AudioManagerCallbackID)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Next, make sure that the input parameters appear to semi-valid.*/
      if(AudioManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoList, AudioManagerCallbackID)) != NULL)
            {
               /* Free the memory because we are finished with it.      */
               FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Audio     */
   /* Manager Service to explicitly process Data (either Source or      */
   /* Sink).  This Callback will be dispatched by the Audio Manager when*/
   /* various Audio Manager Events occur.  This function accepts Audio  */
   /* Stream Type and the Callback Function and Callback Parameter      */
   /* (respectively) to call when an Audio Manager Event needs to be    */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          AUDM_Send_Encoded_Audio_Data() function to send data (for*/
   /*          Audio Source).                                           */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Audio Stream Type.                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          AUDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI AUDM_Register_Data_Event_Callback(AUD_Stream_Type_t StreamType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                 ret_val;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a Data Event Handler for the specified Stream    */
            /* Type.                                                    */
            AudioEntryInfoPtr = AudioEntryInfoDataList;

            while(AudioEntryInfoPtr)
            {
               if(AudioEntryInfoPtr->StreamType == StreamType)
                  break;
               else
                  AudioEntryInfoPtr = AudioEntryInfoPtr->NextAudioEntryInfoPtr;
            }

            if(!AudioEntryInfoPtr)
            {
               /* First, Register the handler locally.                  */
               BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

               AudioEntryInfo.CallbackID         = GetNextCallbackID();
               AudioEntryInfo.ClientID           = MSG_GetServerAddressID();
               AudioEntryInfo.StreamType         = StreamType;
               AudioEntryInfo.EventCallback      = CallbackFunction;
               AudioEntryInfo.CallbackParameter  = CallbackParameter;
               AudioEntryInfo.EventCallbackEntry = TRUE;

               if(AddAudioEntryInfoEntry(&AudioEntryInfoDataList, &AudioEntryInfo))
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  ret_val = AudioEntryInfo.CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_AUDIO_STREAM_DATA_ALREADY_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Callback  */
   /* (registered via a successful call to the                          */
   /* AUDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the Audio Manager Data Event Callback ID (return */
   /* value from AUDM_Register_Data_Event_Callback() function).         */
void BTPSAPI AUDM_Un_Register_Data_Event_Callback(unsigned int AudioManagerDataCallbackID)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Delete the local handler.                                */
            if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoDataList, AudioManagerDataCallbackID)) != NULL)
            {
               /* All finished with the entry, delete it.               */
               FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Audio     */
   /* Manager Service to explicitly process Remote Control Data (either */
   /* Controller or Target).  This Callback will be dispatched by the   */
   /* Audio Manager when various Audio Manager Events occur.  This      */
   /* function accepts the Service Type (Target or Controller) and the  */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when an Audio Manager Remote Control Event needs to be dispatched.*/
   /* This function returns a positive (non-zero) value if successful,  */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          AUDM_Send_Remote_Control_Command() or                    */
   /*          AUDM_Send_Remote_Control_Response() functions to send    */
   /*          Remote Control Events.                                   */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Service Type.                                   */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          AUDM_Un_Register_Remote_Control_Event_Callback() function*/
   /*          to un-register the callback from this module.            */
int BTPSAPI AUDM_Register_Remote_Control_Event_Callback(unsigned int ServiceType, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int                 ret_val;
   Audio_Entry_Info_t  AudioEntryInfo;
   Audio_Entry_Info_t *AudioEntryInfoPtr;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if((Initialized) && (ServiceType & (AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_CONTROLLER | AUDM_REGISTER_REMOTE_CONTROL_DATA_SERVICE_TYPE_TARGET)))
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(CallbackFunction)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Before proceding any further, make sure that there is not*/
            /* already a Remote Control Event Handler for the specified */
            /* Service Type.                                            */
            /* * NOTE * We are using the ConnectionStatus member to     */
            /*          denote the Service Type.                        */
            AudioEntryInfoPtr = AudioEntryInfoRemoteControlList;

            while(AudioEntryInfoPtr)
            {
               if((AudioEntryInfoPtr->EventCallbackEntry) && (AudioEntryInfoPtr->ConnectionStatus & ServiceType))
                  break;
               else
                  AudioEntryInfoPtr = AudioEntryInfoPtr->NextAudioEntryInfoPtr;
            }

            if(!AudioEntryInfoPtr)
            {
               /* First, Register the handler locally.                  */
               BTPS_MemInitialize(&AudioEntryInfo, 0, sizeof(Audio_Entry_Info_t));

               /* We will use the ConnectionStatus member to keep track */
               /* of the Service Type.                                  */
               AudioEntryInfo.CallbackID         = GetNextCallbackID();
               AudioEntryInfo.ClientID           = MSG_GetServerAddressID();
               AudioEntryInfo.ConnectionStatus   = ServiceType;
               AudioEntryInfo.EventCallback      = CallbackFunction;
               AudioEntryInfo.CallbackParameter  = CallbackParameter;
               AudioEntryInfo.EventCallbackEntry = TRUE;

               if(AddAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, &AudioEntryInfo))
               {
                  /* Data Handler registered, go ahead and flag success */
                  /* to the caller.                                     */
                  ret_val = AudioEntryInfo.CallbackID;
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
            }
            else
               ret_val = BTPM_ERROR_CODE_REMOTE_CONTROL_EVENT_ALREADY_REGISTERED;

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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Remote Control  */
   /* Event Callback (registered via a successful call to the           */
   /* AUDM_Register_Remote_Control_Event_Callback() function).  This    */
   /* function accepts as input the Audio Manager Remote Control Event  */
   /* Callback ID (return value from                                    */
   /* AUDM_Register_Remote_Control_Event_Callback() function).          */
void BTPSAPI AUDM_Un_Register_Remote_Control_Event_Callback(unsigned int AudioManagerRemoteControlCallbackID)
{
   Audio_Entry_Info_t *AudioEntryInfo;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Audio Manager has been initialized, let's check the input      */
      /* parameters to see if they are semi-valid.                      */
      if(AudioManagerRemoteControlCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Delete the local handler.                                */
            if((AudioEntryInfo = DeleteAudioEntryInfoEntry(&AudioEntryInfoRemoteControlList, AudioManagerRemoteControlCallbackID)) != NULL)
            {
               /* All finished with the entry, delete it.               */
               FreeAudioEntryInfoEntryMemory(AudioEntryInfo);
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a Remote Control Browsing Channel connection */
   /* to a remote device. This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function requires that a standard Remote Control    */
   /*          Connection be setup before attempting to connect         */
   /*          browsing.                                                */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in     */
   /*          the Remote Control Browsing Connection Status Event (if  */
   /*          specified).                                              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          aetRemoteControlBrowsingConnectionStatus event           */
   /*          will be dispatched to to denote the status of            */
   /*          the connection.  This is the ONLY way to receive         */
   /*          this event, as an event callack registered               */
   /*          with the AUDM_Register_Event_Callback() or               */
   /*          AUDM_Register_Remote_Control_Event_Callback() functions  */
   /*          will NOT receive connection status events.               */
int BTPSAPI AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, AUDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Simply wrap the internal function to do all of the work.    */
         ret_val = ConnectRemoteControlBrowsing(MSG_GetServerAddressID(), RemoteDeviceAddress, ConnectionFlags, ConnectionStatus, CallbackFunction, CallbackParameter);

         /* Release the Lock because we are finished with it if the     */
         /* internal function did not flag that it had trouble          */
         /* re-acquiring the lock.                                      */
         if(ret_val != BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT)
            DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* Browsing session.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
int BTPSAPI AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress)
{
   int                                          ret_val;
   AUD_Browsing_Channel_Close_Indication_Data_t CloseIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the Audio Manager has been initialized. */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Check to see if the device is powered on.                   */
         if(CurrentPowerState)
         {
            /* Nothing to do here other than to call the actual function*/
            /* to Disconnect the Remote Control.                        */
            ret_val = _AUDM_Disconnect_Remote_Control_Browsing(RemoteDeviceAddress);

            /* If the result was successful, we need to make sure we    */
            /* clean up everything and dispatch the event to all        */
            /* registered clients.                                      */
            if(!ret_val)
            {
               /* Fake a Close Event to dispatch to all registered      */
               /* clients that the Stream has been closed.              */
               CloseIndicationData.BD_ADDR = RemoteDeviceAddress;

               ProcessBrowsingChannelCloseIndicationEvent(&CloseIndicationData);
            }
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
      ret_val = BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_AUDIO | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

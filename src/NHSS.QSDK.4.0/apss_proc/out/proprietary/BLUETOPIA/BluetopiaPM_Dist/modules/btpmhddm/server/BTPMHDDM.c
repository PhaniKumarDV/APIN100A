/*****< btpmhddm.c >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHDDM - HDD Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/12/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMHDDM.h"            /* BTPM HDD Manager Prototypes/Constants.    */
#include "HDDMAPI.h"             /* HDD Manager Prototypes/Constants.         */
#include "HDDMMSG.h"             /* BTPM HDD Manager Message Formats.         */
#include "HDDMGR.h"              /* HDD Manager Impl. Prototypes/Constants.   */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following structure is used to track registered callbacks.    */
typedef struct _tagCallback_Entry_t
{
   unsigned int                 CallbackID;
   unsigned int                 ClientID;
   HDDM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   struct _tagCallback_Entry_t *NextCallbackEntryPtr;
} Callback_Entry_t;

   /* Container structure used when dispatching registered callbacks.   */
typedef struct _tagCallback_Info_t
{
   unsigned int           ClientID;
   HDDM_Event_Callback_t  EventCallback;
   void                  *CallbackParameter;
} Callback_Info_t;

   /* The following enumerated type is used to denote the various states*/
   /* that are used when tracking connections.                          */
typedef enum
{
   csIdle,
   csConnectingDevice,
   csAuthorizing,
   csAuthenticating,
   csEncrypting,
   csConnecting,
   csConnected
} Connection_State_t;

   /* The following structure is used to track information regarding HID*/
   /* Connections.                                                      */
typedef struct _tagConnection_Entry_t
{
   unsigned int                   ClientID;
   unsigned int                   HIDID;
   HDDM_Event_Callback_t          EventCallback;
   void                          *CallbackParameter;
   BD_ADDR_t                      BD_ADDR;
   unsigned int                   ConnectionStatus;
   Event_t                        ConnectionEvent;
   Connection_State_t             ConnectionState;
   unsigned long                  Flags;
   struct _tagConnection_Entry_t *NextConnectionEntryPtr;
} Connection_Entry_t;

#define HDDM_CONNECTION_ENTRY_FLAGS_SERVER_CONNECTION          0x00000001
#define HDDM_CONNECTION_ENTRY_FLAGS_OUTGOING_PENDING           0x00000002

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

   /* Variables which hold pointers to the first element in the callback*/
   /* and connection lists, respectively.                               */
static Callback_Entry_t   *CallbackEntryList;
static Callback_Entry_t   *DataCallbackEntryList;
static Connection_Entry_t *ConnectionEntryList;

   /* Variable which holds the current Power State of the Device.       */
static Boolean_t CurrentPowerState;

   /* Variable to store the Initialization Data passed to the module.   */
static HDDM_Initialization_Data_t HDDMInitializationData;

   /* Internal Function Prototypes.                                     */
static unsigned int GetNextCallbackID(void);

static Boolean_t ConvertHIDResultType(HDDM_Result_t HDDMResult, HID_Result_Type_t *HIDResultType);

static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **ListHead, Callback_Entry_t *EntryToAdd);
static Callback_Entry_t *SearchCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID);
static Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID);
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);
static void FreeCallbackEntryList(Callback_Entry_t **ListHead);

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd);
static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static Connection_Entry_t *SearchConnectionEntryHIDID(Connection_Entry_t **ListHead, unsigned int HIDID);
static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree);
static void FreeConnectionEntryList(Connection_Entry_t **ListHead);

static int ProcessConnectionRequestResponse(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);
static int ProcessConnectRemoteHost(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
static int ProcessDisconnect(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug);
static int ProcessQueryConnectedHosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
static int ProcessChangeIncomingConnectionFlags(unsigned long ConnectionFlags);
static int ProcessSendReportData(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);
static int ProcessGetReportResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData);
static int ProcessSetReportResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);
static int ProcessGetProtocolResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol);
static int ProcessSetProtocolResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);
static int ProcessGetIdleResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate);
static int ProcessSetIdleResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);
static int ProcessRegisterEventCallback(unsigned int ClientID, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
static void ProcessUnRegisterEventCallback(unsigned int ClientID, unsigned int HDDManagerCallbackID);
static int ProcessRegisterDataEventCallback(unsigned int ClientID, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
static void ProcessUnRegisterDataEventCallback(unsigned int ClientID, unsigned int HDDManagerDataCallbackID);

static void ProcessConnectionRequestResponseMessage(HDDM_Connection_Request_Response_Request_t *Message);
static void ProcessConnectRemoteHostMessage(HDDM_Connect_Remote_Host_Request_t *Message);
static void ProcessDisconnectMessage(HDDM_Disconnect_Request_t *Message);
static void ProcessQueryConnectedHostsMessage(HDDM_Query_Connected_Hosts_Request_t *Message);
static void ProcessChangeIncomingConnectionFlagsMessage(HDDM_Change_Incoming_Connection_Flags_Request_t *Message);
static void ProcessSendReportDataMessage(HDDM_Send_Report_Data_Request_t *Message);
static void ProcessGetReportResponseMessage(HDDM_Get_Report_Response_Request_t *Message);
static void ProcessSetReportResponseMessage(HDDM_Set_Report_Response_Request_t *Message);
static void ProcessGetProtocolResponseMessage(HDDM_Get_Protocol_Response_Request_t *Message);
static void ProcessSetProtocolResponseMessage(HDDM_Set_Protocol_Response_Request_t *Message);
static void ProcessGetIdleResponseMessage(HDDM_Get_Idle_Response_Request_t *Message);
static void ProcessSetIdleResponseMessage(HDDM_Set_Idle_Response_Request_t *Message);
static void ProcessRegisterEventsMessage(HDDM_Register_Events_Request_t *Message);
static void ProcessUnRegisterEventsMessage(HDDM_Un_Register_Events_Request_t *Message);
static void ProcessRegisterDataEventsMessage(HDDM_Register_Data_Events_Request_t *Message);
static void ProcessUnRegisterDataEventsMessage(HDDM_Un_Register_Data_Events_Request_t *Message);

static void ProcessReceivedMessage(BTPM_Message_t *Message);
static void ProcessClientUnRegister(unsigned int ClientID);

static void DispatchHDDEvent(HDDM_Event_Data_t *HDDMEventData, BTPM_Message_t *Message);
static void DispatchHDDDataEvent(HDDM_Event_Data_t *HDDMEventData, BTPM_Message_t *Message);
static void DispatchConnectionStatusEvent(Boolean_t DispatchOpen, Connection_Entry_t *ConnectionEntry, unsigned int Status);

static void ProcessOpenRequestIndicationEvent(HID_Open_Request_Indication_Data_t *OpenRequestIndicationData);
static void ProcessOpenIndicationEvent(HID_Open_Indication_Data_t *OpenIndicationData);
static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HID_Open_Confirmation_Data_t *OpenConfirmationData);
static void ProcessCloseIndicationEvent(HID_Close_Indication_Data_t *CloseIndicationData);
static void ProcessControlIndicationEvent(HID_Control_Indication_Data_t *ControlIndicationData);
static void ProcessGetReportIndicationEvent(HID_Get_Report_Indication_Data_t *GetReportIndicationData);
static void ProcessSetReportIndicationEvent(HID_Set_Report_Indication_Data_t *SetReportIndicationData);
static void ProcessGetProtocolIndicationEvent(HID_Get_Protocol_Indication_Data_t *GetProtocolIndicationData);
static void ProcessSetProtocolIndicationEvent(HID_Set_Protocol_Indication_Data_t *SetProtocolIndicationData);
static void ProcessGetIdleIndicationEvent(HID_Get_Idle_Indication_Data_t *GetIdleIndicationData);
static void ProcessSetIdleIndicationEvent(HID_Set_Idle_Indication_Data_t *SetIdleIndicationData);
static void ProcessDataIndicationEvent(HID_Data_Indication_Data_t *DataIndicationData);

static void ProcessHIDEvent(HDDM_HID_Event_Data_t *HIDEventData);

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status);

static void BTPSAPI BTPMDispatchCallback_HDDM(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_HID(void *CallbackParameter);
static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter);

static void BTPSAPI HDDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve a unique Callback ID that can be used to add an entry    */
   /* into the HDD Callback List.                                       */
static unsigned int GetNextCallbackID(void)
{
   unsigned int ret_val;

   ret_val = NextCallbackID++;

   if((!NextCallbackID) || (NextCallbackID & 0x80000000))
      NextCallbackID = 0x00000001;

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* convert a HID_Host Result Type from a HDDM Result Type.           */
static Boolean_t ConvertHIDResultType(HDDM_Result_t HDDMResult, HID_Result_Type_t *HIDResultType)
{
   Boolean_t ret_val = TRUE;

   if(HIDResultType)
   {
      switch(HDDMResult)
      {
         case hdrSuccessful:
            *HIDResultType = rtSuccessful;
            break;
         case hdrNotReady:
            *HIDResultType = rtNotReady;
            break;
         case hdrErrInvalidReportID:
            *HIDResultType = rtErrInvalidReportID;
            break;
         case hdrErrUnsupportedRequest:
            *HIDResultType = rtErrUnsupportedRequest;
            break;
         case hdrErrInvalidParameter:
            *HIDResultType = rtErrInvalidParameter;
            break;
         case hdrErrUnknown:
            *HIDResultType = rtErrUnknown;
            break;
         case hdrErrFatal:
            *HIDResultType = rtErrFatal;
            break;
         case hdrData:
            *HIDResultType = rtData;
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
static Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **ListHead, Callback_Entry_t *EntryToAdd)
{
   return((Callback_Entry_t *)BSC_AddGenericListEntry(sizeof(Callback_Entry_t), ekUnsignedInteger, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), sizeof(Callback_Entry_t), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr), (void **)ListHead, (void *)EntryToAdd));
}

   /* The following function searches the specified List for the        */
   /* specified Callback ID.  This function returns NULL if either the  */
   /* List Head is invalid, the Callback ID is invalid, or the specified*/
   /* Callback ID was NOT found.                                        */
static Callback_Entry_t *SearchCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr), (void **)ListHead));
}

   /* The following function searches the specified HID Callback        */
   /* Information List for the specified Callback ID and removes it from*/
   /* the List.  This function returns NULL if either the HID Entry     */
   /* Information List Head is invalid, the Callback ID is invalid, or  */
   /* the specified Callback ID was NOT present in the list.  The entry */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeCallbackEntryMemory().                       */
static Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **ListHead, unsigned int CallbackID)
{
   return((Callback_Entry_t *)BSC_DeleteGenericListEntry(ekUnsignedInteger, (void *)&CallbackID, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, CallbackID), BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr), (void **)ListHead));
}

   /* This function frees the specified HID Entry Information member.   */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Callback List.  Upon return of this      */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeCallbackEntryList(Callback_Entry_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)ListHead, BTPS_STRUCTURE_OFFSET(Callback_Entry_t, NextCallbackEntryPtr));
}

static Connection_Entry_t *AddConnectionEntry(Connection_Entry_t **ListHead, Connection_Entry_t *EntryToAdd)
{
   return((Connection_Entry_t *)BSC_AddGenericListEntry(sizeof(Connection_Entry_t), ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(Connection_Entry_t, BD_ADDR), sizeof(Connection_Entry_t), BTPS_STRUCTURE_OFFSET(Connection_Entry_t, NextConnectionEntryPtr), (void **)ListHead, (void *)EntryToAdd));
}

static Connection_Entry_t *SearchConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return((Connection_Entry_t *)BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)&BD_ADDR, BTPS_STRUCTURE_OFFSET(Connection_Entry_t, BD_ADDR), BTPS_STRUCTURE_OFFSET(Connection_Entry_t, NextConnectionEntryPtr), (void **)ListHead));
}

static Connection_Entry_t *SearchConnectionEntryHIDID(Connection_Entry_t **ListHead, unsigned int HIDID)
{
   return((Connection_Entry_t *)BSC_SearchGenericListEntry(ekUnsignedInteger, (void *)&HIDID, BTPS_STRUCTURE_OFFSET(Connection_Entry_t, HIDID), BTPS_STRUCTURE_OFFSET(Connection_Entry_t, NextConnectionEntryPtr), (void **)ListHead));
}

static Connection_Entry_t *DeleteConnectionEntry(Connection_Entry_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return((Connection_Entry_t *)BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)&BD_ADDR, BTPS_STRUCTURE_OFFSET(Connection_Entry_t, BD_ADDR), BTPS_STRUCTURE_OFFSET(Connection_Entry_t, NextConnectionEntryPtr), (void **)ListHead));
}

static void FreeConnectionEntryMemory(Connection_Entry_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)EntryToFree);
}

static void FreeConnectionEntryList(Connection_Entry_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)ListHead, BTPS_STRUCTURE_OFFSET(Connection_Entry_t, NextConnectionEntryPtr));
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Connection Request Response    */
   /* command.                                                          */
static int ProcessConnectionRequestResponse(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int                 ret_val;
   Boolean_t           Authenticate;
   Boolean_t           Encrypt;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Device is powered on, next, verify that we are already tracking a */
   /* connection for the specified connection type.                     */
   if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && (ConnectionEntry->ConnectionState == csAuthorizing))
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to Respond to Request: %d\n", Accept));

      /* If the caller has accepted the request then we need to process */
      /* it differently.                                                */
      if(Accept)
      {
         /* Determine if Authentication and/or Encryption is required   */
         /* for this link.                                              */
         if(HDDMInitializationData.IncomingConnectionFlags & HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
            Authenticate = TRUE;
         else
            Authenticate = FALSE;

         if(HDDMInitializationData.IncomingConnectionFlags & HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
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

         if((ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
         {
            /* Authorization not required, and we are already in the    */
            /* correct state.                                           */
            ret_val = _HDDM_Open_Request_Response(ConnectionEntry->HIDID, TRUE);

            if(ret_val)
            {
               _HDDM_Open_Request_Response(ConnectionEntry->HIDID, FALSE);

               /* Go ahead and delete the entry because we are finished */
               /* with tracking it.                                     */
               if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntry);
            }
            else
            {
               /* Update the current connection state.                  */
               ConnectionEntry->ConnectionState = csConnecting;
            }
         }
         else
         {
            /* If we were successfully able to Authenticate and/or      */
            /* Encrypt, then we need to set the correct state.          */
            if(!ret_val)
            {
               if(Encrypt)
                  ConnectionEntry->ConnectionState = csEncrypting;
               else
                  ConnectionEntry->ConnectionState = csAuthenticating;

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error, reject the request.                            */
               _HDDM_Open_Request_Response(ConnectionEntry->HIDID, FALSE);

               /* Go ahead and delete the entry because we are finished */
               /* with tracking it.                                     */
               if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntry);
            }
         }
      }
      else
      {
         /* Rejection - Simply respond to the request.                  */
         ret_val = _HDDM_Open_Request_Response(ConnectionEntry->HIDID, FALSE);

         /* Go ahead and delete the entry because we are finished with  */
         /* tracking it.                                                */
         if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
            FreeConnectionEntryMemory(ConnectionEntry);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Connect Remote Host command.   */
static int ProcessConnectRemoteHost(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int                 ret_val;
   Event_t             ConnectionEvent;
   Connection_Entry_t  ConnectionEntry;
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize success.                                               */
   ret_val = 0;

   /* Next, make sure that we do not already have a connection to the   */
   /* specified device.                                                 */
   if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) == NULL)
   {
      /* Entry is not present, go ahead and create a new entry.         */
      BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

      ConnectionEntry.ClientID          = ClientID;
      ConnectionEntry.BD_ADDR           = RemoteDeviceAddress;
      ConnectionEntry.ConnectionState   = csIdle;
      ConnectionEntry.EventCallback     = CallbackFunction;
      ConnectionEntry.CallbackParameter = CallbackParameter;

      if((ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry)) != NULL)
      {
         if(ConnectionStatus)
            ConnectionEntryPtr->ConnectionEvent = BTPS_CreateEvent(FALSE);

         if((!ConnectionStatus) || ((ConnectionStatus) && (ConnectionEntryPtr->ConnectionEvent)))
         {
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open remote device\n"));

            /* Next, attempt to open the remote device.                 */
            if(ConnectionFlags & HDDM_CONNECT_REMOTE_HOST_FLAGS_REQUIRE_ENCRYPTION)
               ConnectionEntryPtr->ConnectionState = csEncrypting;
            else
            {
               if(ConnectionFlags & HDDM_CONNECT_REMOTE_HOST_FLAGS_REQUIRE_AUTHENTICATION)
                  ConnectionEntryPtr->ConnectionState = csAuthenticating;
               else
                  ConnectionEntryPtr->ConnectionState = csConnectingDevice;
            }

            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Attempting to open Remote Device\n"));

            ret_val = DEVM_ConnectWithRemoteDevice(RemoteDeviceAddress, (ConnectionEntryPtr->ConnectionState == csConnectingDevice)?0:((ConnectionEntryPtr->ConnectionState == csEncrypting)?(DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE | DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_ENCRYPT):DEVM_CONNECT_WITH_REMOTE_DEVICE_FLAGS_AUTHENTICATE));

            if((ret_val >= 0) || (ret_val == BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED) || (ret_val == BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED))
            {
               /* Check to see if we need to actually issue the Remote  */
               /* connection.                                           */
               if(ret_val < 0)
               {
                  /* Set the state to connecting remote device.         */
                  ConnectionEntryPtr->ConnectionState = csConnecting;

                  ret_val = _HDDM_Connect_Remote_Host(RemoteDeviceAddress);

                  if(ret_val < 0)
                  {
                     ret_val = BTPM_ERROR_CODE_HDD_UNABLE_TO_CONNECT_REMOTE_HOST;

                     /* Error opening device, go ahead and delete the   */
                     /* entry that was added.                           */
                     if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
                     {
                        FreeConnectionEntryMemory(ConnectionEntryPtr);
                     }
                  }
                  else
                  {
                     /* Note the HID ID.                                */
                     ConnectionEntryPtr->HIDID = (unsigned int)ret_val;

                     /* Flag success.                                   */
                     ret_val                = 0;
                  }
               }
            }

            /* Next, determine if the caller has requested a blocking   */
            /* open.                                                    */
            if((!ret_val) && (ConnectionStatus))
            {
               /* Blocking open, go ahead and wait for the event.       */

               /* Note the Open Event.                                  */
               ConnectionEvent = ConnectionEntryPtr->ConnectionEvent;

               /* Release the lock because we are finished with it.     */
               DEVM_ReleaseLock();

               BTPS_WaitEvent(ConnectionEvent, BTPS_INFINITE_WAIT);

               /* Re-acquire the Lock.                                  */
               if(DEVM_AcquireLock())
               {
                  if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                  {
                     /* Note the connection status.                     */
                     *ConnectionStatus = ConnectionEntryPtr->ConnectionStatus;

                     BTPS_CloseEvent(ConnectionEntryPtr->ConnectionEvent);

                     /* Flag that the Connection Event is no longer     */
                     /* valid.                                          */
                     ConnectionEntryPtr->ConnectionEvent = NULL;

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_HDD_UNABLE_TO_CONNECT_REMOTE_HOST;
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
                  if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
                     FreeConnectionEntryMemory(ConnectionEntryPtr);
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT;

         /* If an error occurred, go ahead and delete the Connection    */
         /* Information that was added.                                 */
         if(ret_val)
         {
            if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
               FreeConnectionEntryMemory(ConnectionEntryPtr);
         }
         else
            ret_val = 0;
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
   }
   else
   {
      if(ConnectionEntryPtr->ConnectionState == csConnected)
         ret_val = BTPM_ERROR_CODE_HDD_HOST_ALREADY_CONNECTED;
      else
         ret_val = BTPM_ERROR_CODE_HDD_CONNECTION_IN_PROGRESS;
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Disconnect command.            */
static int ProcessDisconnect(unsigned int ClientID, BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug)
{
   int                          ret_val;
   Connection_Entry_t          *ConnectionEntryPtr;
   HID_Close_Indication_Data_t  CloseIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Initialize success.                                               */
   ret_val = 0;

   /* Next, make sure that we do not already have a connection to the   */
   /* specified device.                                                 */
   if(((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL) && ((ConnectionEntryPtr->ConnectionState == csConnecting || ConnectionEntryPtr->ConnectionState == csConnected)))
   {
      ret_val = _HDDM_Close_Connection(ConnectionEntryPtr->HIDID, SendVirtualCableUnplug);

      /* Fake a close indication to inform all registered clients of the*/
      /* disconnect.                                                    */
      if(!ret_val)
      {
         CloseIndicationData.HIDID = ConnectionEntryPtr->HIDID;
         ProcessCloseIndicationEvent(&CloseIndicationData);
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Query Connected Hosts command. */
static int ProcessQueryConnectedHosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int                 ret_val;
   unsigned int        NumberConnected;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Let's determine how many devices are actually connected.          */
   NumberConnected = 0;
   ConnectionEntry = ConnectionEntryList;

   while(ConnectionEntry)
   {
      /* Note that we are only counting devices that are counting       */
      /* devices that either in the connected state or the connecting   */
      /* state (i.e. have been authorized OR passed authentication).    */
      if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
         NumberConnected++;

      ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
   }

   /* We now have the total number of devices that will satisy the      */
   /* query.                                                            */
   if(TotalNumberConnectedDevices)
      *TotalNumberConnectedDevices = NumberConnected;

   /* Now that have the total, we need to build the Connected Device    */
   /* List.                                                             */

   /* See if the caller would like to copy some (or all) of the list.   */
   if(MaximumRemoteDeviceListEntries)
   {
      /* If there are more entries in the returned list than the buffer */
      /* specified, we need to truncate it.                             */
      if(MaximumRemoteDeviceListEntries >= NumberConnected)
         MaximumRemoteDeviceListEntries = NumberConnected;

      NumberConnected = 0;

      ConnectionEntry = ConnectionEntryList;

      while((ConnectionEntry) && (NumberConnected < MaximumRemoteDeviceListEntries))
      {
         /* Note that we are only counting devices that are counting    */
         /* devices that either in the connected state or the connecting*/
         /* state (i.e. have been authorized OR passed authentication). */
         if((ConnectionEntry->ConnectionState == csConnected) || (ConnectionEntry->ConnectionState == csConnecting))
            RemoteDeviceAddressList[NumberConnected++] = ConnectionEntry->BD_ADDR;

         ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
      }

      /* Note the total number of devices that were copied into the     */
      /* array.                                                         */
      ret_val = (int)NumberConnected;
   }
   else
      ret_val = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Change Incoming Connection     */
   /* Flags command.                                                    */
static int ProcessChangeIncomingConnectionFlags(unsigned long ConnectionFlags)
{
   int                 ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Simply note the new flags and return success.                     */
   HDDMInitializationData.IncomingConnectionFlags = ConnectionFlags;
   ret_val                                        = 0;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Send Report Data command.      */
static int ProcessSendReportData(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                 ret_val;
   Callback_Entry_t   *CallbackEntry;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         ret_val = _HDDM_Data_Write(ConnectionEntry->HIDID, (Word_t)ReportDataLength, ReportData);
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Get Report Response command.   */
static int ProcessGetReportResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int                     ret_val;
   Callback_Entry_t       *CallbackEntry;
   HID_Result_Type_t       HIDResultType;
   Connection_Entry_t     *ConnectionEntry;
   HID_Report_Type_Type_t  HIDReportType;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         if(ConvertHIDResultType(Result, &HIDResultType))
         {
            switch(ReportType)
            {
               case hdrtInput:
                  HIDReportType = rtInput;
                  break;
               case hdrtOutput:
                  HIDReportType = rtOutput;
                  break;
               case hdrtFeature:
                  HIDReportType = rtFeature;
                  break;
               case hdrtOther:
               default:
                  HIDReportType = rtOther;
                  break;
            }

            ret_val = _HDDM_Get_Report_Response(ConnectionEntry->HIDID, HIDResultType, HIDReportType, ReportDataLength, ReportData);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Set Report Response command.   */
static int ProcessSetReportResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int                 ret_val;
   Callback_Entry_t   *CallbackEntry;
   HID_Result_Type_t   HIDResultType;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         if(ConvertHIDResultType(Result, &HIDResultType))
         {
            ret_val = _HDDM_Set_Report_Response(ConnectionEntry->HIDID, HIDResultType);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Get Protocol Response command. */
static int ProcessGetProtocolResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol)
{
   int                  ret_val;
   Callback_Entry_t    *CallbackEntry;
   HID_Result_Type_t    HIDResultType;
   Connection_Entry_t  *ConnectionEntry;
   HID_Protocol_Type_t  HIDProtocolType;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         if(ConvertHIDResultType(Result, &HIDResultType))
         {
            switch(Protocol)
            {
               case hdpReport:
                  HIDProtocolType = ptReport;
                  break;
               case hdpBoot:
               default:
                  HIDProtocolType = ptBoot;
                  break;
            }

            ret_val = _HDDM_Get_Protocol_Response(ConnectionEntry->HIDID, HIDResultType, HIDProtocolType); 
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Set Protocol Response command. */
static int ProcessSetProtocolResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int                 ret_val;
   Callback_Entry_t   *CallbackEntry;
   HID_Result_Type_t   HIDResultType;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         if(ConvertHIDResultType(Result, &HIDResultType))
         {
            ret_val = _HDDM_Set_Protocol_Response(ConnectionEntry->HIDID, HIDResultType);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Get Idle Response command.     */
static int ProcessGetIdleResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate)
{
   int                 ret_val;
   Callback_Entry_t   *CallbackEntry;
   HID_Result_Type_t   HIDResultType;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         if(ConvertHIDResultType(Result, &HIDResultType))
         {
            ret_val = _HDDM_Get_Idle_Response(ConnectionEntry->HIDID, HIDResultType, (Byte_t)IdleRate);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Set Idle Response command.     */
static int ProcessSetIdleResponse(unsigned int ClientID, unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int                 ret_val;
   Callback_Entry_t   *CallbackEntry;
   HID_Result_Type_t   HIDResultType;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the caller is registered to manager data.               */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Make sure were are tracking this connection.                   */
      if((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, RemoteDeviceAddress)) != NULL)
      {
         if(ConvertHIDResultType(Result, &HIDResultType))
         {
            ret_val = _HDDM_Set_Idle_Response(ConnectionEntry->HIDID, HIDResultType);
         }
         else
            ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
      }
      else
         ret_val = BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Register Event Callback        */
   /* command.                                                          */
static int ProcessRegisterEventCallback(unsigned int ClientID, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   Callback_Entry_t CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Format the Callback Entry.                                        */
   BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

   CallbackEntry.CallbackID        = GetNextCallbackID();
   CallbackEntry.ClientID          = ClientID;
   CallbackEntry.EventCallback     = CallbackFunction;
   CallbackEntry.CallbackParameter = CallbackParameter;

   if(AddCallbackEntry(&CallbackEntryList, &CallbackEntry) != NULL)
      ret_val = CallbackEntry.CallbackID;
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Un Register Event Callback     */
   /* command.                                                          */
static void ProcessUnRegisterEventCallback(unsigned int ClientID, unsigned int HDDManagerCallbackID)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the Callback ID is valid and is registered to the client.  */
   if(((CallbackEntry = SearchCallbackEntry(&CallbackEntryList, HDDManagerCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Simply delete the Callback from the list.                      */
      if((CallbackEntry = DeleteCallbackEntry(&CallbackEntryList, HDDManagerCallbackID)) != NULL)
         FreeCallbackEntryMemory(CallbackEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Register Data Event Callback   */
   /* command.                                                          */
static int ProcessRegisterDataEventCallback(unsigned int ClientID, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int              ret_val;
   Callback_Entry_t CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure there is not already a register data callback.          */
   if(!DataCallbackEntryList)
   {
      /* Format the Callback Entry.                                     */
      BTPS_MemInitialize(&CallbackEntry, 0, sizeof(Callback_Entry_t));

      CallbackEntry.CallbackID        = GetNextCallbackID();
      CallbackEntry.ClientID          = ClientID;
      CallbackEntry.EventCallback     = CallbackFunction;
      CallbackEntry.CallbackParameter = CallbackParameter;

      /* Add the callback.                                              */
      if(AddCallbackEntry(&DataCallbackEntryList, &CallbackEntry) != NULL)
         ret_val = CallbackEntry.CallbackID;
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_DATA_EVENTS_ALREADY_REGISTERED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* This function is a utility function to manage shared code between */
   /* the API and remote IPC calls for a Un Register Data Event Callback*/
   /* command.                                                          */
static void ProcessUnRegisterDataEventCallback(unsigned int ClientID, unsigned int HDDManagerDataCallbackID)
{
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Verify the Callback ID is valid and is registered to the client.  */
   if(((CallbackEntry = SearchCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL) && (CallbackEntry->ClientID == ClientID))
   {
      /* Simply delete the Callback from the list.                      */
      if((CallbackEntry = DeleteCallbackEntry(&DataCallbackEntryList, HDDManagerDataCallbackID)) != NULL)
         FreeCallbackEntryMemory(CallbackEntry);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectionRequestResponseMessage(HDDM_Connection_Request_Response_Request_t *Message)
{
   int                                         Result;
   HDDM_Connection_Request_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessConnectionRequestResponse(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->Accept);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_CONNECT_REMOTE_HOST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessConnectRemoteHostMessage(HDDM_Connect_Remote_Host_Request_t *Message)
{
   int                                 Result;
   HDDM_Connect_Remote_Host_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessConnectRemoteHost(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->ConnectionFlags, NULL, NULL, NULL);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_CONNECT_REMOTE_HOST_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDisconnectMessage(HDDM_Disconnect_Request_t *Message)
{
   int                        Result;
   HDDM_Disconnect_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessDisconnect(Message->MessageHeader.AddressID, Message->RemoteDeviceAddress, Message->SendVirtualCableUnplug);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_DISCONNECT_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessQueryConnectedHostsMessage(HDDM_Query_Connected_Hosts_Request_t *Message)
{
   int                                    Result;
   unsigned int                           TotalDevices;
   unsigned int                           NumberDevices;
   unsigned int                           MessageSize;
   HDDM_Query_Connected_Hosts_Response_t  ErrorMessage;
   HDDM_Query_Connected_Hosts_Response_t *ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Determine how many devices are connected.                   */
         if((Result = ProcessQueryConnectedHosts(0, NULL, &TotalDevices)) == 0)
         {
            NumberDevices = (TotalDevices < Message->MaximumNumberDevices)?TotalDevices:Message->MaximumNumberDevices;
            MessageSize   = HDDM_QUERY_CONNECTED_HOSTS_RESPONSE_SIZE(NumberDevices);

            /* Attempt to allocate the response.                        */
            if((ResponseMessage = (HDDM_Query_Connected_Hosts_Response_t *)BTPS_AllocateMemory(MessageSize)) != NULL)
            {
               /* Initialize the message.                               */
               BTPS_MemInitialize(ResponseMessage, 0, MessageSize);

               /* Call the handler to obtain the connected devices.     */
               if((Result = ProcessQueryConnectedHosts(NumberDevices, ResponseMessage->DeviceConnectedList, NULL)) >= 0)
               {
                  /* Send the response.                                 */
                  ResponseMessage->MessageHeader               = Message->MessageHeader;
                  ResponseMessage->MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
                  ResponseMessage->MessageHeader.MessageLength = MessageSize - BTPM_MESSAGE_HEADER_SIZE;
                  ResponseMessage->Status                      = 0;

                  ResponseMessage->NumberDevicesConnected      = NumberDevices;
                  ResponseMessage->TotalNumberDevices          = TotalDevices;

                  MSG_SendMessage((BTPM_Message_t *)ResponseMessage);
               }

               BTPS_FreeMemory(ResponseMessage);
            }
            else
               Result = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
         }
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      if(Result < 0)
      {
         /* An error occurred, we need to send an error response.       */
         BTPS_MemInitialize(&ErrorMessage, 0, HDDM_QUERY_CONNECTED_HOSTS_RESPONSE_SIZE(0));

         ErrorMessage.MessageHeader               = Message->MessageHeader;
         ErrorMessage.MessageHeader.MessageID    |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
         ErrorMessage.MessageHeader.MessageLength = HDDM_QUERY_CONNECTED_HOSTS_RESPONSE_SIZE(0) - BTPM_MESSAGE_HEADER_SIZE;
         ErrorMessage.Status                      = Result;

         MSG_SendMessage((BTPM_Message_t *)&ErrorMessage);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessChangeIncomingConnectionFlagsMessage(HDDM_Change_Incoming_Connection_Flags_Request_t *Message)
{
   int                                              Result;
   HDDM_Change_Incoming_Connection_Flags_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessChangeIncomingConnectionFlags(Message->ConnectionFlags);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_CHANGE_INCOMING_CONNECTION_FLAGS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSendReportDataMessage(HDDM_Send_Report_Data_Request_t *Message)
{
   int                              Result;
   HDDM_Send_Report_Data_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         for(Result = 0; Result < Message->ReportLength; Result++)
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Report[%d]: %02X\n", Result, Message->ReportData[Result]));
            
         /* Call the helper function to handle the request.             */
         Result = ProcessSendReportData(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->ReportLength, Message->ReportData);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_SEND_REPORT_DATA_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetReportResponseMessage(HDDM_Get_Report_Response_Request_t *Message)
{
   int                                 Result;
   HDDM_Get_Report_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessGetReportResponse(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->Result, Message->ReportType, Message->ReportLength, Message->ReportData);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_GET_REPORT_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetReportResponseMessage(HDDM_Set_Report_Response_Request_t *Message)
{
   int                                 Result;
   HDDM_Set_Report_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessSetReportResponse(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->Result);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_SET_REPORT_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetProtocolResponseMessage(HDDM_Get_Protocol_Response_Request_t *Message)
{
   int                                   Result;
   HDDM_Get_Protocol_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessGetProtocolResponse(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->Result, Message->Protocol);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_GET_PROTOCOL_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetProtocolResponseMessage(HDDM_Set_Protocol_Response_Request_t *Message)
{
   int                                   Result;
   HDDM_Set_Protocol_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessSetProtocolResponse(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->Result);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_SET_PROTOCOL_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetIdleResponseMessage(HDDM_Get_Idle_Response_Request_t *Message)
{
   int                               Result;
   HDDM_Get_Idle_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessGetIdleResponse(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->Result, Message->IdleRate);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_GET_IDLE_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetIdleResponseMessage(HDDM_Set_Idle_Response_Request_t *Message)
{
   int                               Result;
   HDDM_Set_Idle_Response_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Check to see if the device is powered on.                      */
      if(CurrentPowerState)
      {
         /* Call the helper function to handle the request.             */
         Result = ProcessSetIdleResponse(Message->MessageHeader.AddressID, Message->DataCallbackID, Message->RemoteDeviceAddress, Message->Result);
      }
      else
         Result = BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN;

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_SET_IDLE_RESPONSE_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRegisterEventsMessage(HDDM_Register_Events_Request_t *Message)
{
   int                             Result;
   HDDM_Register_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Call the helper function to handle the request.                */
      Result = ProcessRegisterEventCallback(Message->MessageHeader.AddressID, NULL, NULL);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_REGISTER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessUnRegisterEventsMessage(HDDM_Un_Register_Events_Request_t *Message)
{
   HDDM_Un_Register_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Call the helper function to handle the request.                */
      ProcessUnRegisterEventCallback(Message->MessageHeader.AddressID, Message->CallbackID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_UN_REGISTER_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = 0;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessRegisterDataEventsMessage(HDDM_Register_Data_Events_Request_t *Message)
{
   int                                  Result;
   HDDM_Register_Data_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Call the helper function to handle the request.                */
      Result = ProcessRegisterDataEventCallback(Message->MessageHeader.AddressID, NULL, NULL);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_REGISTER_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = Result;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessUnRegisterDataEventsMessage(HDDM_Un_Register_Data_Events_Request_t *Message)
{
   HDDM_Un_Register_Data_Events_Response_t ResponseMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure the input parameter was specified.                      */
   if(Message)
   {
      /* Call the helper function to handle the request.                */
      ProcessUnRegisterDataEventCallback(Message->MessageHeader.AddressID, Message->CallbackID);

      ResponseMessage.MessageHeader                = Message->MessageHeader;
      ResponseMessage.MessageHeader.MessageID     |= BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK;
      ResponseMessage.MessageHeader.MessageLength  = HDDM_UN_REGISTER_DATA_EVENTS_RESPONSE_SIZE - BTPM_MESSAGE_HEADER_SIZE;
      ResponseMessage.Status                       = 0;

      MSG_SendMessage((BTPM_Message_t *)&ResponseMessage);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessReceivedMessage(BTPM_Message_t *Message)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", Message->MessageHeader.MessageFunction));

   /* Check to make sure this isn't some kind of spoofed response (we do*/
   /* not process responses).                                           */
   if((Message) && (!(Message->MessageHeader.MessageID & BTPM_MESSAGE_MESSAGE_ID_RESPONSE_MASK)))
   {
      /* Request received, go ahead and process it.                     */
      switch(Message->MessageHeader.MessageFunction)
      {
         case HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connection Request Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and dispatch the HID */
               /* Connect Response Request.                             */
               ProcessConnectionRequestResponseMessage((HDDM_Connection_Request_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_CONNECT_REMOTE_HOST:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Connect Remote Host Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CONNECT_REMOTE_HOST_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessConnectRemoteHostMessage((HDDM_Connect_Remote_Host_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_DISCONNECT:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Disconnect Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_DISCONNECT_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessDisconnectMessage((HDDM_Disconnect_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_QUERY_CONNECTED_HOSTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Query Connected Hosts Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_QUERY_CONNECTED_HOSTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessQueryConnectedHostsMessage((HDDM_Query_Connected_Hosts_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_CHANGE_INCOMING_CONNECTION_FLAGS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Change Incoming Connection Flags Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_CHANGE_INCOMING_CONNECTION_FLAGS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessChangeIncomingConnectionFlagsMessage((HDDM_Change_Incoming_Connection_Flags_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_REPORT_DATA:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Send Report Data Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SEND_REPORT_DATA_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SEND_REPORT_DATA_REQUEST_SIZE(((HDDM_Send_Report_Data_Request_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Report Data Request.                                  */
               ProcessSendReportDataMessage((HDDM_Send_Report_Data_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_GET_REPORT_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Report Response Message\n"));

            if((BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_REPORT_RESPONSE_REQUEST_SIZE(0)) && (BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_REPORT_RESPONSE_REQUEST_SIZE(((HDDM_Get_Report_Response_Request_t *)Message)->ReportLength)))
            {
               /* Size seems to be valid, go ahead and dispatch the Send*/
               /* Report Data Request.                                  */
               ProcessGetReportResponseMessage((HDDM_Get_Report_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_SET_REPORT_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Report Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_REPORT_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessSetReportResponseMessage((HDDM_Set_Report_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_GET_PROTOCOL_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Protocol Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_PROTOCOL_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessGetProtocolResponseMessage((HDDM_Get_Protocol_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_SET_PROTOCOL_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Protocol Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_PROTOCOL_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessSetProtocolResponseMessage((HDDM_Set_Protocol_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_GET_IDLE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Idle Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_GET_IDLE_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessGetIdleResponseMessage((HDDM_Get_Idle_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_SEND_SET_IDLE_RESPONSE:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Idle Response Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_SET_IDLE_RESPONSE_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessSetIdleResponseMessage((HDDM_Set_Idle_Response_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_REGISTER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_REGISTER_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessRegisterEventsMessage((HDDM_Register_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_UN_REGISTER_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Register Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_UN_REGISTER_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessUnRegisterEventsMessage((HDDM_Un_Register_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_REGISTER_DATA_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Register Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_REGISTER_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessRegisterDataEventsMessage((HDDM_Register_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         case HDDM_MESSAGE_FUNCTION_UN_REGISTER_DATA_EVENTS:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Un Register Data Events Message\n"));

            if(BTPM_MESSAGE_SIZE(Message->MessageHeader.MessageLength) >= HDDM_UN_REGISTER_DATA_EVENTS_REQUEST_SIZE)
            {
               /* Size seems to be valid, go ahead and process the      */
               /* message.                                              */
               ProcessUnRegisterDataEventsMessage((HDDM_Un_Register_Data_Events_Request_t *)Message);
            }
            else
            {
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid Message Length\n"));
            }
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Message\n"));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid or Response Message Received\n"));
   }

   /* Release the Lock because we are finished with it.                 */
   DEVM_ReleaseLock();

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessClientUnRegister(unsigned int ClientID)
{
   Boolean_t          LoopCount;
   Callback_Entry_t  *CallbackEntry;
   Callback_Entry_t **_CallbackEntryList;
   Callback_Entry_t  *tmpCallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: 0x%08X\n", ClientID));

   if((ClientID) && (ClientID != MSG_GetServerAddressID()))
   {
      CallbackEntry      = CallbackEntryList;
      _CallbackEntryList = &CallbackEntryList;

      /* We need to loop through both lists as there could be client    */
      /* registrations in any of the lists.                             */
      LoopCount = 2;
      while(LoopCount)
      {
         while(CallbackEntry)
         {
            /* Check to see if the current Client Information is the one*/
            /* that is being un-registered.                             */
            if(CallbackEntry->ClientID == ClientID)
            {
               /* Note the next HID Entry in the list (we are about to  */
               /* delete the current entry).                            */
               tmpCallbackEntry = CallbackEntry->NextCallbackEntryPtr;

               /* Go ahead and delete the HID Information Entry and     */
               /* clean up the resources.                               */
               if((CallbackEntry = DeleteCallbackEntry(_CallbackEntryList, CallbackEntry->CallbackID)) != NULL)
               {
                  /* All finished with the memory so free the entry.    */
                  FreeCallbackEntryMemory(CallbackEntry);
               }

               /* Go ahead and set the next HID Information Entry (past */
               /* the one we just deleted).                             */
               CallbackEntry = tmpCallbackEntry;
            }
            else
               CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
         }

         /* Decrement the loop count so that we can make another pass   */
         /* through the loop.                                           */
         LoopCount--;

         /* We have processed the HID Information List, now process the */
         /* HID Information Data List.                                  */
         CallbackEntry      = DataCallbackEntryList;
         _CallbackEntryList = &DataCallbackEntryList;
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID event to every registered HID Event    */
   /* Callback.                                                         */
   /* * NOTE * This function should be called with the HID Manager Lock */
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Lock.                                            */
static void DispatchHDDEvent(HDDM_Event_Data_t *HDDMEventData, BTPM_Message_t *Message)
{
   unsigned int      Index;
   unsigned int      Index1;
   unsigned int      ServerID;
   unsigned int      NumberCallbacks;
   Callback_Info_t   CallbackInfoArray[16];
   Callback_Info_t  *CallbackInfoArrayPtr;
   Callback_Entry_t *CallbackEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if(((CallbackEntryList) || (DataCallbackEntryList)) && (HDDMEventData) && (Message))
   {
      /* Next, let's determine how many callbacks are registered.       */
      CallbackEntry    = CallbackEntryList;
      ServerID        = MSG_GetServerAddressID();
      NumberCallbacks = 0;

      /* First, add the default event handlers.                         */
      while(CallbackEntry)
      {
         if((CallbackEntry->EventCallback) || (CallbackEntry->ClientID != ServerID))
            NumberCallbacks++;

         CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
      }

      /* We need to add the HID Data Entry Information List as well.    */
      CallbackEntry = DataCallbackEntryList;
      while(CallbackEntry)
      {
         if((CallbackEntry->EventCallback) || (CallbackEntry->ClientID != ServerID))
            NumberCallbacks++;

         CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
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
            CallbackEntry    = CallbackEntryList;
            NumberCallbacks = 0;

            /* Next add the default event handlers.                     */
            while(CallbackEntry)
            {
               if((CallbackEntry->EventCallback) || (CallbackEntry->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackEntry->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackEntry->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackEntry->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
            }

            /* We need to add the HID Data Entry Information List as    */
            /* well.                                                    */
            CallbackEntry = DataCallbackEntryList;
            while(CallbackEntry)
            {
               if((CallbackEntry->EventCallback) || (CallbackEntry->ClientID != ServerID))
               {
                  CallbackInfoArrayPtr[NumberCallbacks].ClientID          = CallbackEntry->ClientID;
                  CallbackInfoArrayPtr[NumberCallbacks].EventCallback     = CallbackEntry->EventCallback;
                  CallbackInfoArrayPtr[NumberCallbacks].CallbackParameter = CallbackEntry->CallbackParameter;

                  NumberCallbacks++;
               }

               CallbackEntry = CallbackEntry->NextCallbackEntryPtr;
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
                        (*CallbackInfoArrayPtr[Index].EventCallback)(HDDMEventData, CallbackInfoArrayPtr[Index].CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is a utility function that exists to       */
   /* dispatch the specified HID Data event to the correct registered   */
   /* HID Data Event Callback.                                          */
   /* * NOTE * This function should be called with the HID Manager Lock */
   /*          held.  Upon exit from this function it will free the HID */
   /*          Manager Lock.                                            */
static void DispatchHDDDataEvent(HDDM_Event_Data_t *HDDMEventData, BTPM_Message_t *Message)
{
   void                  *CallbackParameter;
   Callback_Entry_t      *CallbackEntry;
   HDDM_Event_Callback_t  EventCallback;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Make sure that the input parameters appear to be semi-valid.      */
   if((HDDMEventData) && (Message))
   {
      /* Before going any further, check to see if someone has          */
      /* registered to process the data.                                */
      if((CallbackEntry = DataCallbackEntryList) != NULL)
      {
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Got callback\n"));
         /* Format up the Data.                                         */
         if(CallbackEntry->ClientID != MSG_GetServerAddressID())
         {
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Client App\n"));
            /* Dispatch a Message Callback.                             */

            /* Note the Client (destination) address.                   */
            Message->MessageHeader.AddressID = CallbackEntry->ClientID;

            /* All that is left to do is to dispatch the Event.         */
            MSG_SendMessage(Message);
         }
         else
         {
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Server Callback\n"));
            /* Dispatch Local Event Callback.                           */
            if(CallbackEntry->EventCallback)
            {
               /* Note the Callback Information.                        */
               EventCallback     = CallbackEntry->EventCallback;
               CallbackParameter = CallbackEntry->CallbackParameter;

               /* Release the Lock because we have already noted the    */
               /* callback information.                                 */
               DEVM_ReleaseLock();

               __BTPSTRY
               {
                  (*EventCallback)(HDDMEventData, CallbackParameter);
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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* This function holds the shared code for dispatching a Connection  */
   /* Status event. Since it is possible to initiate a connection status*/
   /* either through a bluetopia OpenConfirmation event or within the   */
   /* DEVM callback before we have received a HIDID, this code can be   */
   /* shared by the two situations.                                     */
static void DispatchConnectionStatusEvent(Boolean_t DispatchOpen, Connection_Entry_t *ConnectionEntryPtr, unsigned int Status)
{
   void                             *CallbackParameter;
   unsigned int                      ClientID;
   HDDM_Event_Data_t                 HDDMEventData;
   Connection_Entry_t                ConnectionEntry;
   HDDM_Event_Callback_t             EventCallback;
   HDDM_Connected_Message_t          Message;
   HDDM_Connection_Status_Message_t  ConnectionStatusMessage;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ConnectionEntryPtr)
   {
      if(Status)
      {
         /* Save the connection information before we delete it, since  */
         /* we need it to dispatch the event.                           */
         ConnectionEntry = *ConnectionEntryPtr;

         /* Delete the entry so no incoming calls can function on it    */
         /* once we release the lock.                                   */
         if((ConnectionEntryPtr = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntryPtr->BD_ADDR)) != NULL)
            FreeConnectionEntryMemory(ConnectionEntryPtr);

         /* Use our local copy of the connection to dispatch the event. */
         ConnectionEntryPtr = &ConnectionEntry;
      }
      else
         ConnectionEntryPtr->ConnectionState = csConnected;

      /* Connection status registered, now see if we need to issue a    */
      /* Callack or an event.                                           */

      /* Determine if we need to dispatch the event locally or remotely.*/
      if(ConnectionEntryPtr->ClientID == MSG_GetServerAddressID())
      {
         /* Callback.                                                   */
         BTPS_MemInitialize(&HDDMEventData, 0, sizeof(HDDM_Event_Data_t));

         HDDMEventData.EventType                                               = hetHDDConnectionStatus;
         HDDMEventData.EventLength                                             = HDDM_CONNECTION_STATUS_EVENT_DATA_SIZE;

         HDDMEventData.EventData.ConnectionStatusEventData.RemoteDeviceAddress = ConnectionEntryPtr->BD_ADDR;

         /* Map the Open Confirmation Error to the correct HID Manager  */
         /* Error Status.                                               */
         switch(Status)
         {
            case HID_OPEN_PORT_STATUS_SUCCESS:
               HDDMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDDM_CONNECTION_STATUS_SUCCESS;
               break;
            case HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
               HDDMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_TIMEOUT;
               break;
            case HID_OPEN_PORT_STATUS_CONNECTION_REFUSED:
               HDDMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_REFUSED;
               break;
            default:
               HDDMEventData.EventData.ConnectionStatusEventData.ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_UNKNOWN;
               break;
         }

         /* If this was a synchronous event we need to set the status   */
         /* and the event.                                              */
         if(ConnectionEntryPtr->ConnectionEvent)
         {
            /* Synchronous event, go ahead and set the correct status,  */
            /* then set the event.                                      */
            ConnectionEntryPtr->ConnectionStatus = HDDMEventData.EventData.ConnectionStatusEventData.ConnectionStatus;

            BTPS_SetEvent(ConnectionEntryPtr->ConnectionEvent);
         }
         else
         {
            /* Note the Callback information.                           */
            EventCallback     = ConnectionEntryPtr->EventCallback;
            CallbackParameter = ConnectionEntryPtr->CallbackParameter;

            /* Clear out the callback information in the list since it  */
            /* should no longer be used.                                */
            ConnectionEntryPtr->EventCallback     = NULL;
            ConnectionEntryPtr->CallbackParameter = NULL;

            /* Release the Lock so we can make the callback.            */
            DEVM_ReleaseLock();

            __BTPSTRY
            {
               if(EventCallback)
                  (*EventCallback)(&HDDMEventData, CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }

            /* Re-acquire the Lock.                                     */
            DEVM_AcquireLock();
         }
      }
      else
      {
         /* Remote Event.                                               */

         /* Note the Client ID.                                         */
         ClientID = ConnectionEntryPtr->ClientID;

         BTPS_MemInitialize(&ConnectionStatusMessage, 0, sizeof(ConnectionStatusMessage));

         ConnectionStatusMessage.MessageHeader.AddressID       = ClientID;
         ConnectionStatusMessage.MessageHeader.MessageID       = MSG_GetNextMessageID();
         ConnectionStatusMessage.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         ConnectionStatusMessage.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONNECTION_STATUS;
         ConnectionStatusMessage.MessageHeader.MessageLength   = (HDDM_CONNECTION_STATUS_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         ConnectionStatusMessage.RemoteDeviceAddress           = ConnectionEntryPtr->BD_ADDR;

         /* Map the Open Confirmation Error to the correct HID Manager  */
         /* Error Status.                                               */
         switch(Status)
         {
            case HID_OPEN_PORT_STATUS_SUCCESS:
               ConnectionStatusMessage.ConnectionStatus = HDDM_CONNECTION_STATUS_SUCCESS;
               break;
            case HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT:
               ConnectionStatusMessage.ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_TIMEOUT;
               break;
            case HID_OPEN_PORT_STATUS_CONNECTION_REFUSED:
               ConnectionStatusMessage.ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_REFUSED;
               break;
            default:
               ConnectionStatusMessage.ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_UNKNOWN;
               break;
         }

         /* Finally dispatch the Message.                               */
         MSG_SendMessage((BTPM_Message_t *)&ConnectionStatusMessage);
      }

      /* Next, format up the Event to dispatch - ONLY if we need to     */
      /* dispatch a Connected Event.                                    */
      if((!Status) && (DispatchOpen))
      {
         HDDMEventData.EventType                                        = hetHDDConnected;
         HDDMEventData.EventLength                                      = HDDM_CONNECTED_EVENT_DATA_SIZE;

         HDDMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = ConnectionEntryPtr->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONNECTED;
         Message.MessageHeader.MessageLength   = (HDDM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntryPtr->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessOpenRequestIndicationEvent(HID_Open_Request_Indication_Data_t *OpenRequestIndicationData)
{
   int                                Result;
   Boolean_t                          Authenticate;
   Boolean_t                          Encrypt;
   HDDM_Event_Data_t                  HDDMEventData;
   Connection_Entry_t                 ConnectionEntry;
   Connection_Entry_t                *ConnectionEntryPtr;
   HDDM_Connection_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %u\n", OpenRequestIndicationData->HIDID));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenRequestIndicationData)
   {
      /* First, let's see if we actually need to do anything, other than*/
      /* simply accept the connection.                                  */
      if(!HDDMInitializationData.IncomingConnectionFlags)
      {
         /* Simply Accept the connection.                               */
         _HDDM_Open_Request_Response(OpenRequestIndicationData->HIDID, TRUE);
      }
      else
      {
         /* Before proceding any further, let's make sure that there    */
         /* doesn't already exist an entry for this device.             */
         if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenRequestIndicationData->BD_ADDR)) == NULL)
         {
            /* Entry does not exist, go ahead and format a new entry.   */
            BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

            ConnectionEntry.BD_ADDR         = OpenRequestIndicationData->BD_ADDR;
            ConnectionEntry.HIDID           = OpenRequestIndicationData->HIDID;
            ConnectionEntry.ConnectionState = csAuthorizing;
            ConnectionEntry.Flags           = HDDM_CONNECTION_ENTRY_FLAGS_SERVER_CONNECTION;

            ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
         }

         /* Check to see if we are tracking this connection.            */
         if(ConnectionEntryPtr)
         {
            if(HDDMInitializationData.IncomingConnectionFlags & HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHORIZATION)
            {
               /* Authorization (at least) required, go ahead and       */
               /* dispatch the request.                                 */
               ConnectionEntryPtr->ConnectionState = csAuthorizing;

               /* Next, format up the Event to dispatch.                */
               HDDMEventData.EventType                                                = hetHDDConnectionRequest;
               HDDMEventData.EventLength                                              = HDDM_CONNECTION_REQUEST_EVENT_DATA_SIZE;

               HDDMEventData.EventData.ConnectionRequestEventData.RemoteDeviceAddress = OpenRequestIndicationData->BD_ADDR;

               /* Next, format up the Message to dispatch.              */
               BTPS_MemInitialize(&Message, 0, sizeof(Message));

               Message.MessageHeader.AddressID       = 0;
               Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
               Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
               Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONNECTION_REQUEST;
               Message.MessageHeader.MessageLength   = (HDDM_CONNECTION_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

               Message.RemoteDeviceAddress           = OpenRequestIndicationData->BD_ADDR;

               /* Finally dispatch the formatted Event and Message.     */
               DispatchHDDEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
            }
            else
            {
               /* Determine if Authentication and/or Encryption is      */
               /* required for this link.                               */
               if(HDDMInitializationData.IncomingConnectionFlags & HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_AUTHENTICATION)
                  Authenticate = TRUE;
               else
                  Authenticate = FALSE;

               if(HDDMInitializationData.IncomingConnectionFlags & HDDM_INCOMING_CONNECTION_FLAGS_REQUIRE_ENCRYPTION)
                  Encrypt = TRUE;
               else
                  Encrypt = FALSE;

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
                  Result = _HDDM_Open_Request_Response(OpenRequestIndicationData->HIDID, TRUE);

                  if(Result)
                  {
                     _HDDM_Open_Request_Response(OpenRequestIndicationData->HIDID, FALSE);

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
                     _HDDM_Open_Request_Response(OpenRequestIndicationData->HIDID, FALSE);

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
            _HDDM_Open_Request_Response(OpenRequestIndicationData->HIDID, FALSE);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessOpenIndicationEvent(HID_Open_Indication_Data_t *OpenIndicationData)
{
   HDDM_Event_Data_t                       HDDMEventData;
   Connection_Entry_t                      ConnectionEntry;
   Connection_Entry_t                     *ConnectionEntryPtr;
   HDDM_Connection_Request_Message_t       Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenIndicationData)
   {
      if((ConnectionEntryPtr = SearchConnectionEntry(&ConnectionEntryList, OpenIndicationData->BD_ADDR)) == NULL)
      {
         /* Entry does not exist, go ahead and format a new entry.      */
         BTPS_MemInitialize(&ConnectionEntry, 0, sizeof(Connection_Entry_t));

         ConnectionEntry.BD_ADDR                = OpenIndicationData->BD_ADDR;
         ConnectionEntry.HIDID                  = OpenIndicationData->HIDID;
         ConnectionEntry.ConnectionState        = csConnected;
         ConnectionEntry.NextConnectionEntryPtr = NULL;

         ConnectionEntryPtr = AddConnectionEntry(&ConnectionEntryList, &ConnectionEntry);
      }
      else
         ConnectionEntryPtr->ConnectionState = csConnected;

      if(ConnectionEntryPtr)
      {
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                        = hetHDDConnected;
         HDDMEventData.EventLength                                      = HDDM_CONNECTED_EVENT_DATA_SIZE;

         HDDMEventData.EventData.ConnectedEventData.RemoteDeviceAddress = OpenIndicationData->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONNECTED;
         Message.MessageHeader.MessageLength   = (HDDM_CONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = OpenIndicationData->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
      else
      {
         /* Error, go ahead and disconnect the device.                  */
         _HDDM_Close_Connection(OpenIndicationData->HIDID, TRUE);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessOpenConfirmationEvent(Boolean_t DispatchOpen, HID_Open_Confirmation_Data_t *OpenConfirmationData)
{
   Connection_Entry_t *ConnectionEntryPtr;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(OpenConfirmationData)
   {
      /* First, flag the connected state.                               */
      if((ConnectionEntryPtr = SearchConnectionEntryHIDID(&ConnectionEntryList, OpenConfirmationData->HIDID)) != NULL)
      {
         /* Simply call the shared handler function.                    */
         DispatchConnectionStatusEvent(DispatchOpen, ConnectionEntryPtr, OpenConfirmationData->OpenStatus);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessCloseIndicationEvent(HID_Close_Indication_Data_t *CloseIndicationData)
{
   HDDM_Event_Data_t            HDDMEventData;
   Connection_Entry_t          *ConnectionEntry;
   HDDM_Disconnected_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(CloseIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, CloseIndicationData->HIDID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                           = hetHDDDisconnected;
         HDDMEventData.EventLength                                         = HDDM_DISCONNECTED_EVENT_DATA_SIZE;

         HDDMEventData.EventData.DisconnectedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_DISCONNECTED;
         Message.MessageHeader.MessageLength   = (HDDM_DISCONNECTED_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Delete the entry now that we are finished.                  */
         if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, ConnectionEntry->BD_ADDR)) != NULL)
            FreeConnectionEntryMemory(ConnectionEntry);

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessControlIndicationEvent(HID_Control_Indication_Data_t *ControlIndicationData)
{
   HDDM_Event_Data_t             HDDMEventData;
   Connection_Entry_t           *ConnectionEntry;
   HDDM_Control_Operation_t      ControlOperation;
   HDDM_Control_Event_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(ControlIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, ControlIndicationData->HIDID)) != NULL)
      {
         switch(ControlIndicationData->ControlOperation)
         {
            case hcNop:
            default:
               ControlOperation = hdcNop;
               break;
            case hcHardReset:
               ControlOperation = hdcHardReset;
               break;
            case hcSoftReset:
               ControlOperation = hdcSoftReset;
               break;
            case hcSuspend:
               ControlOperation = hdcSuspend;
               break;
            case hcExitSuspend:
               ControlOperation = hdcExitSuspend;
               break;
            case hcVirtualCableUnplug:
               ControlOperation = hdcVirtualCableUnplug;
               break;
         }

         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                      = hetHDDControlEvent;
         HDDMEventData.EventLength                                    = HDDM_CONTROL_EVENT_DATA_SIZE;

         HDDMEventData.EventData.ControlEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDDMEventData.EventData.ControlEventData.ControlOperation    = ControlOperation;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_CONTROL_EVENT;
         Message.MessageHeader.MessageLength   = (HDDM_CONTROL_EVENT_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ControlOperation              = ControlOperation;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetReportIndicationEvent(HID_Get_Report_Indication_Data_t *GetReportIndicationData)
{
   HDDM_Event_Data_t                  HDDMEventData;
   Connection_Entry_t                *ConnectionEntry;
   HDDM_Report_Type_t                 ReportType;
   HDDM_Get_Report_Size_Type_t        GetReportSizeType;
   HDDM_Get_Report_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GetReportIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, GetReportIndicationData->HIDID)) != NULL)
      {
         switch(GetReportIndicationData->ReportType)
         {
            case rtOther:
            default:
               ReportType = hdrtOther;
               break;
            case rtInput:
               ReportType = hdrtInput;
               break;
            case rtOutput:
               ReportType = hdrtOutput;
               break;
            case rtFeature:
               ReportType = hdrtFeature;
               break;
         }

         switch(GetReportIndicationData->Size)
         {
            case grSizeOfReport:
            default:
               GetReportSizeType = hgrSizeOfReport;
               break;
            case grUseBufferSize:
               GetReportSizeType = grUseBufferSize;
         }
   
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                               = hetHDDGetReportRequest;
         HDDMEventData.EventLength                                             = HDDM_GET_REPORT_REQUEST_EVENT_DATA_SIZE;

         HDDMEventData.EventData.GetReportRequestEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDDMEventData.EventData.GetReportRequestEventData.ReportType          = ReportType;
         HDDMEventData.EventData.GetReportRequestEventData.SizeType            = GetReportSizeType;
         HDDMEventData.EventData.GetReportRequestEventData.ReportID            = GetReportIndicationData->ReportID;
         HDDMEventData.EventData.GetReportRequestEventData.BufferSize          = GetReportIndicationData->BufferSize;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_GET_REPORT_REQUEST;
         Message.MessageHeader.MessageLength   = (HDDM_GET_REPORT_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.ReportType                    = ReportType;
         Message.SizeType                      = GetReportSizeType;
         Message.ReportID                      = GetReportIndicationData->ReportID;
         Message.BufferSize                    = GetReportIndicationData->BufferSize;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetReportIndicationEvent(HID_Set_Report_Indication_Data_t *SetReportIndicationData)
{
   unsigned int                       MessageSize;
   HDDM_Event_Data_t                  HDDMEventData;
   HDDM_Report_Type_t                 ReportType;
   Connection_Entry_t                *ConnectionEntry;
   HDDM_Set_Report_Request_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetReportIndicationData)
   {
      MessageSize = HDDM_SET_REPORT_REQUEST_MESSAGE_SIZE(SetReportIndicationData->ReportLength);

      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, SetReportIndicationData->HIDID)) != NULL)
      {
         /* We need to allocate the IPC Message.                        */
         if((Message = (HDDM_Set_Report_Request_Message_t *)BTPS_AllocateMemory(MessageSize)) != NULL)
         {
            switch(SetReportIndicationData->ReportType)
            {
               case rtOther:
               default:
                  ReportType = hdrtOther;
                  break;
               case rtInput:
                  ReportType = hdrtInput;
                  break;
               case rtOutput:
                  ReportType = hdrtOutput;
                  break;
               case rtFeature:
                  ReportType = hdrtFeature;
                  break;
            }

            /* Next, format up the Event to dispatch.                   */
            HDDMEventData.EventType                                               = hetHDDSetReportRequest;
            HDDMEventData.EventLength                                             = HDDM_SET_REPORT_REQUEST_EVENT_DATA_SIZE;

            HDDMEventData.EventData.SetReportRequestEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
            HDDMEventData.EventData.SetReportRequestEventData.ReportType          = ReportType;
            HDDMEventData.EventData.SetReportRequestEventData.ReportLength        = SetReportIndicationData->ReportLength;
            HDDMEventData.EventData.SetReportRequestEventData.ReportData          = SetReportIndicationData->ReportDataPayload;

            /* Next, format up the Message to dispatch.                 */
            BTPS_MemInitialize(Message, 0, MessageSize);

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
            Message->MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SET_REPORT_REQUEST;
            Message->MessageHeader.MessageLength   = (MessageSize - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
            Message->ReportType                    = ReportType;
            Message->ReportLength                  = SetReportIndicationData->ReportLength;

            if((SetReportIndicationData->ReportLength) && (SetReportIndicationData->ReportDataPayload))
               BTPS_MemCopy(Message->ReportData, SetReportIndicationData->ReportDataPayload, SetReportIndicationData->ReportLength);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetProtocolIndicationEvent(HID_Get_Protocol_Indication_Data_t *GetProtocolIndicationData)
{
   HDDM_Event_Data_t                  HDDMEventData;
   Connection_Entry_t                *ConnectionEntry;
   HDDM_Get_Report_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GetProtocolIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, GetProtocolIndicationData->HIDID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                                 = hetHDDGetProtocolRequest;
         HDDMEventData.EventLength                                               = HDDM_GET_PROTOCOL_REQUEST_EVENT_DATA_SIZE;

         HDDMEventData.EventData.GetProtocolRequestEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_GET_PROTOCOL_REQUEST;
         Message.MessageHeader.MessageLength   = (HDDM_GET_REPORT_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetProtocolIndicationEvent(HID_Set_Protocol_Indication_Data_t *SetProtocolIndicationData)
{
   HDDM_Event_Data_t                    HDDMEventData;
   Connection_Entry_t                  *ConnectionEntry;
   HDDM_Protocol_t                      Protocol;
   HDDM_Set_Protocol_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetProtocolIndicationData)
   {
      switch(SetProtocolIndicationData->Protocol)
      {
         case ptBoot:
         default:
            Protocol = hdpBoot;
            break;
         case ptReport:
            Protocol = hdpReport;
            break;
      }

      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, SetProtocolIndicationData->HIDID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                                 = hetHDDSetProtocolRequest;
         HDDMEventData.EventLength                                               = HDDM_SET_PROTOCOL_REQUEST_EVENT_DATA_SIZE;

         HDDMEventData.EventData.SetProtocolRequestEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDDMEventData.EventData.SetProtocolRequestEventData.Protocol            = Protocol;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SET_PROTOCOL_REQUEST;
         Message.MessageHeader.MessageLength   = (HDDM_SET_PROTOCOL_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.Protocol                      = Protocol;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessGetIdleIndicationEvent(HID_Get_Idle_Indication_Data_t *GetIdleIndicationData)
{
   HDDM_Event_Data_t                HDDMEventData;
   Connection_Entry_t              *ConnectionEntry;
   HDDM_Get_Idle_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(GetIdleIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, GetIdleIndicationData->HIDID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                             = hetHDDGetIdleRequest;
         HDDMEventData.EventLength                                           = HDDM_GET_IDLE_REQUEST_EVENT_DATA_SIZE;

         HDDMEventData.EventData.GetIdleRequestEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_GET_IDLE_REQUEST;
         Message.MessageHeader.MessageLength   = (HDDM_GET_IDLE_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessSetIdleIndicationEvent(HID_Set_Idle_Indication_Data_t *SetIdleIndicationData)
{
   HDDM_Event_Data_t                HDDMEventData;
   Connection_Entry_t              *ConnectionEntry;
   HDDM_Set_Idle_Request_Message_t  Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(SetIdleIndicationData)
   {
      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, SetIdleIndicationData->HIDID)) != NULL)
      {
         /* Next, format up the Event to dispatch.                      */
         HDDMEventData.EventType                                             = hetHDDSetIdleRequest;
         HDDMEventData.EventLength                                           = HDDM_SET_IDLE_REQUEST_EVENT_DATA_SIZE;

         HDDMEventData.EventData.SetIdleRequestEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
         HDDMEventData.EventData.SetIdleRequestEventData.IdleRate            = (unsigned int)SetIdleIndicationData->IdleRate;

         /* Next, format up the Message to dispatch.                    */
         BTPS_MemInitialize(&Message, 0, sizeof(Message));

         Message.MessageHeader.AddressID       = 0;
         Message.MessageHeader.MessageID       = MSG_GetNextMessageID();
         Message.MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
         Message.MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_SET_IDLE_REQUEST;
         Message.MessageHeader.MessageLength   = (HDDM_SET_IDLE_REQUEST_MESSAGE_SIZE - BTPM_MESSAGE_HEADER_SIZE);

         Message.RemoteDeviceAddress           = ConnectionEntry->BD_ADDR;
         Message.IdleRate                      = (unsigned int)SetIdleIndicationData->IdleRate;

         /* Finally dispatch the formatted Event and Message.           */
         DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)&Message);
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDataIndicationEvent(HID_Data_Indication_Data_t *DataIndicationData)
{
   unsigned int                         MessageSize;
   HDDM_Event_Data_t                    HDDMEventData;
   Connection_Entry_t                  *ConnectionEntry;
   HDDM_Report_Data_Received_Message_t *Message;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, verify that the input parameter appears to be semi-valid.  */
   if(DataIndicationData)
   {
      MessageSize = HDDM_REPORT_DATA_RECEIVED_MESSAGE_SIZE(DataIndicationData->ReportLength);

      /* First, flag that we are now disconnected.                      */
      if((ConnectionEntry = SearchConnectionEntryHIDID(&ConnectionEntryList, DataIndicationData->HIDID)) != NULL)
      {
         /* We need to allocate the IPC Message.                        */
         if((Message = (HDDM_Report_Data_Received_Message_t *)BTPS_AllocateMemory(MessageSize)) != NULL)
         {
            /* Next, format up the Event to dispatch.                   */
            HDDMEventData.EventType                                           = hetHDDReportDataReceived;
            HDDMEventData.EventLength                                         = HDDM_REPORT_DATA_RECEIVED_EVENT_DATA_SIZE;

            HDDMEventData.EventData.ReportDataReceivedEventData.RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
            HDDMEventData.EventData.ReportDataReceivedEventData.ReportLength        = DataIndicationData->ReportLength;
            HDDMEventData.EventData.ReportDataReceivedEventData.ReportData          = DataIndicationData->ReportDataPayload;

            /* Next, format up the Message to dispatch.                 */
            BTPS_MemInitialize(Message, 0, MessageSize);

            Message->MessageHeader.AddressID       = 0;
            Message->MessageHeader.MessageID       = MSG_GetNextMessageID();
            Message->MessageHeader.MessageGroup    = BTPM_MESSAGE_GROUP_HDD_MANAGER;
            Message->MessageHeader.MessageFunction = HDDM_MESSAGE_FUNCTION_REPORT_DATA_RECEIVED;
            Message->MessageHeader.MessageLength   = (MessageSize - BTPM_MESSAGE_HEADER_SIZE);

            Message->RemoteDeviceAddress = ConnectionEntry->BD_ADDR;
            Message->ReportLength        = DataIndicationData->ReportLength;

            if((DataIndicationData->ReportLength) && (DataIndicationData->ReportDataPayload))
               BTPS_MemCopy(Message->ReportData, DataIndicationData->ReportDataPayload, DataIndicationData->ReportLength);

            /* Finally dispatch the formatted Event and Message.        */
            DispatchHDDDataEvent(&HDDMEventData, (BTPM_Message_t *)Message);

            BTPS_FreeMemory(Message);
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


static void ProcessHIDEvent(HDDM_HID_Event_Data_t *HIDEventData)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First make sure that the input parameter appears to be semi-valid.*/
   if(HIDEventData)
   {
      /* Process the event based on the event type.                     */
      switch(HIDEventData->EventType)
      {
         case etHID_Open_Request_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Request Indication\n"));

            ProcessOpenRequestIndicationEvent(&(HIDEventData->EventData.HID_Open_Request_Indication_Data));
            break;
         case etHID_Open_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Indication\n"));

            ProcessOpenIndicationEvent(&(HIDEventData->EventData.HID_Open_Indication_Data));
            break;
         case etHID_Open_Confirmation:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Open Confirmation\n"));

            ProcessOpenConfirmationEvent(TRUE, &(HIDEventData->EventData.HID_Open_Confirmation_Data));
            break;
         case etHID_Close_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Close Indication\n"));

            ProcessCloseIndicationEvent(&(HIDEventData->EventData.HID_Close_Indication_Data));
            break;
         case etHID_Control_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Control Indication\n"));

            ProcessControlIndicationEvent(&(HIDEventData->EventData.HID_Control_Indication_Data));
            break;
         case etHID_Get_Report_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Report Indication\n"));

            ProcessGetReportIndicationEvent(&(HIDEventData->EventData.HID_Get_Report_Indication_Data));
            break;
         case etHID_Set_Report_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Report Indication\n"));

            ProcessSetReportIndicationEvent(&(HIDEventData->EventData.HID_Set_Report_Indication_Data));
            break;
         case etHID_Get_Protocol_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Protocol Indication\n"));

            ProcessGetProtocolIndicationEvent(&(HIDEventData->EventData.HID_Get_Protocol_Indication_Data));
            break;
         case etHID_Set_Protocol_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Protocol Indication\n"));

            ProcessSetProtocolIndicationEvent(&(HIDEventData->EventData.HID_Set_Protocol_Indication_Data));
            break;
         case etHID_Get_Idle_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Get Idle Indication\n"));

            ProcessGetIdleIndicationEvent(&(HIDEventData->EventData.HID_Get_Idle_Indication_Data));
            break;
         case etHID_Set_Idle_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Set Idle Indication\n"));

            ProcessSetIdleIndicationEvent(&(HIDEventData->EventData.HID_Set_Idle_Indication_Data));
            break;
         case etHID_Data_Indication:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Data Indication\n"));

            ProcessDataIndicationEvent(&(HIDEventData->EventData.HID_Data_Indication_Data));
            break;
         default:
            /* Invalid/Unknown Event.                                   */
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid/Unknown HID Event Type: %d\n", HIDEventData->EventType));
            break;
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Invalid HID Event Data\n"));
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void ProcessDEVMStatusEvent(DEVM_Status_Type_t StatusType, BD_ADDR_t BD_ADDR, int Status)
{
   int                 Result;
   unsigned int        OpenStatus;
   Connection_Entry_t *ConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (HDDM): 0x%08X, %d\n", StatusType, Status));

   /* First, determine if we are tracking a connection to this device.  */
   if(((ConnectionEntry = SearchConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL) && (ConnectionEntry->ConnectionState != csConnected))
   {
      /* Check if this is an outgoing connection attempt.               */
      if(!(ConnectionEntry->Flags & HDDM_CONNECTION_ENTRY_FLAGS_SERVER_CONNECTION))
      {
         /* Process the status event.                                   */

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
                  OpenStatus = HID_OPEN_PORT_STATUS_CONNECTION_REFUSED;
                  break;
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
               case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
                  OpenStatus = HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT;
                  break;
               default:
                  OpenStatus = HID_OPEN_PORT_STATUS_UNKNOWN_ERROR;
                  break;
            }

            /* * NOTE * This function will delete the HID Info entry    */
            /*          from the list.                                  */
            DispatchConnectionStatusEvent(TRUE, ConnectionEntry, OpenStatus);

            /* Flag that the connection has been deleted.               */
            ConnectionEntry = NULL;
         }
         else
         {
            /* Connection succeeded.                                    */

            /* Move the state to the connecting state.                  */
            ConnectionEntry->ConnectionState = csConnecting;

            if((Result = _HDDM_Connect_Remote_Host(ConnectionEntry->BD_ADDR)) > 0)
            {
               /* Note the HIDID of the new connection.                 */
               ConnectionEntry->HIDID = (unsigned int)Result;
            }
            else
            {
               /* Error opening device.                                 */
               DEVM_DisconnectRemoteDevice(BD_ADDR, 0);

               OpenStatus = HID_OPEN_PORT_STATUS_UNKNOWN_ERROR;

               /* * NOTE * This function will delete the HID Info entry */
               /* from the list.                                        */
               DispatchConnectionStatusEvent(TRUE, ConnectionEntry, OpenStatus);

               /* Flag that the connection has been deleted.            */
               ConnectionEntry = NULL;
            }
         }
      }
      else
      {
         /* Incoming connection, double check the state.                */
         if((ConnectionEntry) && ((ConnectionEntry->ConnectionState == csAuthenticating) || (ConnectionEntry->ConnectionState == csEncrypting)))
         {
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Incoming Connection Entry found\n"));

            /* Status does not reference an outgoing connection.        */
            if(!Status)
            {
               /* Success, accept the connection.                       */
               _HDDM_Open_Request_Response(ConnectionEntry->HIDID, TRUE);
            }
            else
            {
               /* Failure, reject the connection.                       */
               _HDDM_Open_Request_Response(ConnectionEntry->HIDID, FALSE);

               /* First, delete the Connection Entry we are tracking.   */
               if((ConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, BD_ADDR)) != NULL)
                  FreeConnectionEntryMemory(ConnectionEntry);
            }
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (HIDM)\n"));
}

static void BTPSAPI BTPMDispatchCallback_HDDM(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI BTPMDispatchCallback_HID(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            if(((HDDM_Update_Data_t *)CallbackParameter)->UpdateType == utHIDEvent)
            {
               /* Process the Notification.                             */
               ProcessHIDEvent(&(((HDDM_Update_Data_t *)CallbackParameter)->UpdateData.HIDEventData));
            }

            /* Release the lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }

      /* Go ahead and free the memory because we are finished with it.  */
      BTPS_FreeMemory(CallbackParameter);
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI BTPMDispatchCallback_MSG(void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

static void BTPSAPI HDDManagerGroupHandler(BTPM_Message_t *Message, void *CallbackParameter)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* Check to see if the Message specified appears to be semi-valid.   */
   if(Message)
   {
      /* Next, check to see if this message is specified for our        */
      /* specific Message Group.                                        */
      if(Message->MessageHeader.MessageGroup == BTPM_MESSAGE_GROUP_HDD_MANAGER)
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HDD Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
               /* HDD Manager Thread.                                   */
               if(!BTPM_QueueMailboxCallback(BTPMDispatchCallback_HDDM, (void *)Message))
               {
                  DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HDD Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));

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
                        DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unable to Queue HDD Manager Message: 0x%08X\n", Message->MessageHeader.MessageFunction));
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
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Non HID Manager Message: 0x%08X, 0x%08X\n", Message->MessageHeader.MessageGroup, Message->MessageHeader.MessageFunction));
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HDDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData)
{
   int Result;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter: %d\n", Initialize));

   /* First, determine if the request is to initialize or shut down.    */
   if(Initialize)
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initialization Request\n"));

      if(!Initialized)
      {
         /* Note the new initialization state.                          */
         Initialized = TRUE;

         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Initializing HID Manager\n"));

         /* Next, let's attempt to register our Message Group Handler to*/
         /* process HID Manager messages.                               */
         if(!MSG_RegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HDD_MANAGER, HDDManagerGroupHandler, NULL))
         {
            /* Initialize the actual HID Manager Implementation Module  */
            /* (this is the module that is actually responsible for     */
            /* actually implementing the HID Manager functionality -    */
            /* this module is just the framework shell).                */
            if(!(Result = _HDDM_Initialize((HDDM_Initialization_Data_t *)InitializationData)))
            {
               HDDMInitializationData = *((HDDM_Initialization_Data_t *)InitializationData);

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
            _HDDM_Cleanup();

            MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HDD_MANAGER);
         }
      }
      else
      {
         DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("HID Manager Already initialized\n"));
      }
   }
   else
   {
      DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Shutdown Request\n"));

      if(Initialized)
      {
         /* First, let's make sure that we un-register our Message Group*/
         /* Handler (so that we do not process any more incoming        */
         /* messages).                                                  */
         MSG_UnRegisterMessageGroupHandler(BTPM_MESSAGE_GROUP_HDD_MANAGER);

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Make sure we inform the HID Manager Implementation that  */
            /* we are shutting down.                                    */
            _HDDM_Cleanup();

            /* Make sure the incoming Connection List is empty.         */
            FreeConnectionEntryList(&ConnectionEntryList);

            /* Make sure that the Callback List is empty.               */
            FreeCallbackEntryList(&CallbackEntryList);

            /* Make sure that the Data Callback List is empty.          */
            FreeCallbackEntryList(&DataCallbackEntryList);

            CurrentPowerState = FALSE;

            /* Flag that this module is no longer initialized.          */
            Initialized       = FALSE;

            /* Free the Lock we acquired earlier.                       */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData)
{
   int                 Result;
   Connection_Entry_t *ConnectionEntry;
   Connection_Entry_t *tmpConnectionEntry;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         switch(EventData->EventType)
         {
            case detDevicePoweredOn:
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power On Occurred.\n"));

               /* Power on Event.                                       */

               /* Note the new Power State.                             */
               CurrentPowerState = TRUE;

               /* Go ahead and inform the HID Manager that it should    */
               /* initialize.                                           */
               if((Result = DEVM_QueryDeviceBluetoothStackID()) > 0)
                  _HDDM_SetBluetoothStackID((unsigned int)Result);

               break;
            case detDevicePoweringOff:
            case detDevicePoweredOff:
               DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Device Power Off Occurred.\n"));

               /* Note the new Power State.                             */
               CurrentPowerState = FALSE;

               /* Inform the HID Manager that the Stack has been closed.*/
               _HDDM_SetBluetoothStackID(0);

               /* Loop through all outgoing connections to determine if */
               /* there are any synchronous connections outstanding.    */
               ConnectionEntry = ConnectionEntryList;

               while(ConnectionEntry)
               {
                  /* Attempt to close any active connection.            */
                  if(ConnectionEntry->ConnectionState == csAuthorizing)
                     _HDDM_Open_Request_Response(ConnectionEntry->HIDID, FALSE);
                  else
                     _HDDM_Close_Connection(ConnectionEntry->HIDID, FALSE);

                  /* Check to see if there is a synchronous open        */
                  /* operation.                                         */
                  if(ConnectionEntry->ConnectionEvent)
                  {
                     /* NOTE: We are intentionally not deleting the     */
                     /*       connection here. The function waiting on  */
                     /*       the event will re-acquire the connection  */
                     /*       entry to obtain the ConnectionStatus. It  */
                     /*       will then delete it.                      */
                     ConnectionEntry->ConnectionStatus = HDDM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF;

                     BTPS_SetEvent(ConnectionEntry->ConnectionEvent);

                     ConnectionEntry = ConnectionEntry->NextConnectionEntryPtr;
                  }
                  else
                  {
                     /* Entry was waiting on a response, but it was     */
                     /* registered as either an Event Callback or       */
                     /* Connection Message.  Regardless we need to      */
                     /* delete it.                                      */
                     tmpConnectionEntry = ConnectionEntry;

                     ConnectionEntry    = ConnectionEntry->NextConnectionEntryPtr;

                     if((tmpConnectionEntry = DeleteConnectionEntry(&ConnectionEntryList, tmpConnectionEntry->BD_ADDR)) != NULL)
                        FreeConnectionEntryMemory(tmpConnectionEntry);
                  }
               }

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

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}

   /* The following function is provided to allow the caller the ability*/
   /* to notify the HID Manager of a specific Update Event.  The HID    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t HDDM_NotifyUpdate(HDDM_Update_Data_t *UpdateData)
{
   Boolean_t ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, make sure that the input parameters appear to be           */
   /* semi-valid.                                                       */
   if(UpdateData)
   {
      switch(UpdateData->UpdateType)
      {
         case utHIDEvent:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Queueing HID Event: %d\n", UpdateData->UpdateData.HIDEventData.EventType));

            /* Simply Queue the Dispatch Data.                          */
            /* * NOTE * To avoid copying again, we will simply submit   */
            /*          the entire Notification Message.                */
            ret_val = BTPM_QueueMailboxCallback(BTPMDispatchCallback_HID, (void *)UpdateData);
            break;
         default:
            DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_VERBOSE), ("Unhandled Notification: %d\n", UpdateData->UpdateType));

            ret_val = FALSE;
            break;
      }
   }
   else
      ret_val = FALSE;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* Accept or reject (authorize) an incoming HID connection from a    */
   /* remote HID Host.  This function returns zero if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HDD Connected   */
   /*          event will be dispatched to signify the actual result.   */
int BTPSAPI HDDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessConnectionRequestResponse(MSG_GetServerAddressID(), RemoteDeviceAddress, Accept);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Connect to a remote HID Host device.  The RemoteDeviceAddress is  */
   /* the Bluetooth Address of the remote HID Host.  The ConnectionFlags*/
   /* specifiy whay security, if any, is required for the connection.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HDDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int BTPSAPI HDDM_Connect_Remote_Host(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessConnectRemoteHost(MSG_GetServerAddressID(), RemoteDeviceAddress, ConnectionFlags, CallbackFunction, CallbackParameter, ConnectionStatus);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Disconnect from a remote HID Host.  The RemoteDeviceAddress       */
   /* is the Bluetooth Address of the remote HID Host.  The             */
   /* SendVirtualCableUnplug parameter indicates whether the device     */
   /* should be disconnected with a Virtual Cable Unplug (TRUE) or      */
   /* simply at the Bluetooth Link (FALSE).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int BTPSAPI HDDM_Disconnect(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessDisconnect(MSG_GetServerAddressID(), RemoteDeviceAddress, SendVirtualCableUnplug);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Determine if there are currently any connected HID Hosts.  This   */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently connected HID Hosts.  The first parameter specifies the */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer).  The next parameter is      */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI HDDM_Query_Connected_Hosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(((!MaximumRemoteDeviceListEntries) && (TotalNumberConnectedDevices)) || ((MaximumRemoteDeviceListEntries) && (RemoteDeviceAddressList)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessQueryConnectedHosts(MaximumRemoteDeviceListEntries, RemoteDeviceAddressList, TotalNumberConnectedDevices);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int BTPSAPI HDDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         ret_val = ProcessChangeIncomingConnectionFlags(ConnectionFlags);

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Send the specified HID Report Data to a currently connected       */
   /* remote device.  This function accepts as input the HDD            */
   /* Manager Report Data Handler ID (registered via call to the        */
   /* HDDM_Register_Data_Event_Callback() function), followed by the    */
   /* remote device address of the remote HID Host to send the report   */
   /* data to, followed by the report data itself.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int BTPSAPI HDDM_Send_Report_Data(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (ReportDataLength) && (ReportData) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessSendReportData(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, ReportDataLength, ReportData);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Respond to a GetReportRequest.  The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* ReportType indicates the type of report being sent as the         */
   /* response.  The ReportDataLength indicates the size of the report  */
   /* data.  ReportData is a pointer to the report data buffer.  This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Get_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (ReportDataLength) && (ReportData) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessGetReportResponse(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, Result, ReportType, ReportDataLength, ReportData);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Responsd to a SetReportRequest. The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Set_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessSetReportResponse(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, Result);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Respond to a GetProtocolRequest.  The HDDManagerDataCallback      */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* Protocol indicates the current HID Protocol.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int BTPSAPI HDDM_Get_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessGetProtocolResponse(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, Result, Protocol);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Respond to a SetProtocolResponse.  The HDDManagerDataCallback     */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Set_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessSetProtocolResponse(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, Result);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Respond to a GetIdleResponse.  The HDDManagerDataCallback         */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* IdleRate is the current Idle Rate.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
int BTPSAPI HDDM_Get_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessGetIdleResponse(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, Result, IdleRate);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* Respond to a SetIdleRequest.  The HDDManagerDataCallback          */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int BTPSAPI HDDM_Set_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if((HDDManagerDataCallbackID) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceAddress)))
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* Next, check to see if we are powered up.                 */
            if(CurrentPowerState)
            {
               ret_val = ProcessSetIdleResponse(MSG_GetServerAddressID(), HDDManagerDataCallbackID, RemoteDeviceAddress, Result);
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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

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
   /*          HDDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
int BTPSAPI HDDM_Register_Event_Callback(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            ret_val = ProcessRegisterEventCallback(MSG_GetServerAddressID(), CallbackFunction, CallbackParameter);

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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HDDM_RegisterEventCallback() function).                           */
void BTPSAPI HDDM_Un_Register_Event_Callback(unsigned int HDDManagerCallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HDDManagerCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            ProcessUnRegisterEventCallback(MSG_GetServerAddressID(), HDDManagerCallbackID);

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
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
   /*          HDDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
int BTPSAPI HDDM_Register_Data_Event_Callback(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter)
{
   int ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

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
            ret_val = ProcessRegisterDataEventCallback(MSG_GetServerAddressID(), CallbackFunction, CallbackParameter);

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
      ret_val = BTPM_ERROR_CODE_HDD_NOT_INITIALIZED;

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %d\n", ret_val));

   return(ret_val);
}

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HDDM_Register_Data_Event_Callback() function).         */
void BTPSAPI HDDM_Un_Register_Data_Event_Callback(unsigned int HDDManagerDataCallbackID)
{
   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   /* First, check to make sure the HID Manager has been initialized.   */
   if(Initialized)
   {
      /* HID Manager has been initialized, let's check the input        */
      /* parameters to see if they are semi-valid.                      */
      if(HDDManagerDataCallbackID)
      {
         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            ProcessUnRegisterDataEventCallback(MSG_GetServerAddressID(), HDDManagerDataCallbackID);

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_HID_DEVICE | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit\n"));
}


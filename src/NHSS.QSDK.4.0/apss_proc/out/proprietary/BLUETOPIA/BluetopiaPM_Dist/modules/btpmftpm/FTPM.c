/*****< ftpm.c >***************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FTPM - Bluetooth Stack FTP Manager Implementation for Stonestreet One     */
/*         Bluetooth Protocol Stack Platform Manager.                         */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/03/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */
#include "SS1BTPS.h"             /* Bluetopia Core Prototypes/Constants.      */
#include "FTPM.h"                /* Bluetooth FTPM Prototypes/Constants.      */

#include "BTPSCFG.h"             /* BTPS Configuration Constants.             */

#include "FTPCHCVT.h"            /* Platform specific UTF-8 <-> UFT-16 Conv.  */

#include "SS1BTPM.h"             /* BTPM Main Prototypes and Constants.       */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following declared type represents the Prototype Function for */
   /* an OBEX Event Callback.  This function will be called whenever a  */
   /* defined OBEX Action occurs.  This function passes to the caller   */
   /* the OBEX Event Data associated with the OBEX Event that occurred, */
   /* and the OBEX Callback Parameter that was specified when this      */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the OBEX Event Data ONLY in the context of this callback.  If the */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  The processing in this     */
   /* function should be as efficient as possible.  It should be noted  */
   /* that this function is called in the Thread Context of a Thread    */
   /* that the User does NOT own.  Therefore, processing in this        */
   /* function should be as small as possible.                          */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving other Bluetooth     */
   /*            Stack Events.  A Deadlock WILL occur because other     */
   /*            Callbacks might not be issued while this function is   */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *GOEPM_Event_Callback_t)(GOEP_Event_Data_t *GOEP_Event_Data, unsigned long CallbackParameter);

   /* The following define various OBEX parameters.                     */
#define OBEX_COMMAND_OFFSET                                             0
#define OBEX_PACKET_LENGTH_OFFSET                                       1
#define OBEX_VERSION_NUMBER_OFFSET                                      3
#define OBEX_SET_PATH_FLAGS_OFFSET                                      3
#define OBEX_SET_PATH_CONSTANTS_OFFSET                                  4
#define OBEX_CONNECT_FLAGS_OFFSET                                       4
#define OBEX_MAX_PACKET_LENGTH_OFFSET                                   5

#define OBEX_CONNECT_PACKET_HEADER_OFFSET                               7
#define OBEX_DISCONNECT_PACKET_HEADER_OFFSET                            3
#define OBEX_PUT_PACKET_HEADER_OFFSET                                   3
#define OBEX_GET_PACKET_HEADER_OFFSET                                   3
#define OBEX_ABORT_PACKET_HEADER_OFFSET                                 3
#define OBEX_SET_PATH_PACKET_HEADER_OFFSET                              5
#define OBEX_SET_PATH_RESPONSE_PACKET_HEADER_OFFSET                     3

#define MINIMUM_OBEX_PACKET_SIZE                                        3
#define MINIMUM_OBEX_CONNECT_PACKET_SIZE                                7
#define MINIMUM_OBEX_DISCONNECT_PACKET_SIZE                             3
#define MINIMUM_OBEX_PUT_PACKET_SIZE                                    3
#define MINIMUM_OBEX_GET_PACKET_SIZE                                    3
#define MINIMUM_OBEX_ABORT_PACKET_SIZE                                  3
#define MINIMUM_OBEX_SET_PATH_PACKET_SIZE                               3

#define OBEX_INVALID_COMMAND                                            0xFF

   /* Define the amount of memory that is required to hold the Response */
   /* Code and Packet Length along with the Body or End of Body header  */
   /* and the data length                                               */
#define OBEX_RESPONSE_HEADER_SIZE_WITH_BODY                        6

   /* The following defines the possible Actions that can be performed  */
   /* using a PUT operation.  The PUT operation can be used to Put,     */
   /* Create or Delete and Object.                                      */
#define OTP_OBJECT_PUT_ACTION_PUT_OBJECT                        0x00
#define OTP_OBJECT_PUT_ACTION_CREATE                            0x01
#define OTP_OBJECT_PUT_ACTION_DELETE                            0x02

#define _NULL_                                          ((char)0x00)
#define _TAB_                                           ((char)0x09)
#define _QUOTE_                                                  '"'
#define _LESS_THAN_                                              '<'
#define _GREATER_THAN_                                           '>'
#define _SPACE_                                                  ' '
#define _CARRIAGE_RETURN_                                       '\r'
#define _LINE_FEED_                                             '\n'
#define _NON_SPACE_                                     ((char)0xFF)
#define MAKE_LOWER_CASE                                         0x20

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* testing of a character to see if it is a White Space Character.   */
   /* White Space characters are considered to be the same as the       */
   /* White Space characters that the C Run Time Library isspace()      */
   /* MACRO/function returns.  This MACRO returns a BOOLEAN TRUE if     */
   /* the input argument is either (or FALSE otherwise):                */
   /*   - TAB                   0x09                                    */
   /*   - New Line              0x0A                                    */
   /*   - Vertical TAB          0x0B                                    */
   /*   - Form Feed             0x0C                                    */
   /*   - Carriage Return       0x0D                                    */
   /*   - Space                 0x20                                    */
#define IS_WHITE_SPACE_CHARACTER(_x)             (((_x) == _SPACE_) || (((_x) >= _TAB_) && ((_x) <= _CARRIAGE_RETURN_)))

   /* Miscellaneous Internal Error Codes that are used within this      */
   /* Module.                                                           */
#define INVALID_CONNECTION_ID                             0xFFFFFFFF
#define INVALID_PARAMETER                                         -2
#define HEADER_NOT_FOUND                                          -3

#define BODY_WEIGHT                                             0x01
#define END_OF_BODY_WEIGHT                                      0x02
#define FINAL_FLAG_WEIGHT                                       0x04
#define IDLE_STATE_WEIGHT                                       0x10
#define PUT_SETUP_STATE_WEIGHT                                  0x20
#define PUT_STATE_WEIGHT                                        0x40

   /* The following defines the possible operations that the FTPM Mgr   */
   /* Client/Server can be in the process of handling.  When the Client */
   /* or Server is idle, the the Operation is set to None.              */
#define OTP_OPERATION_NONE                                      0x00
#define OTP_OPERATION_GET_DIRECTORY                             0x01
#define OTP_OPERATION_GET_FILE                                  0x02
#define OTP_OPERATION_PUT_FILE                                  0x03
#define OTP_OPERATION_GET_OBJECT                                0x04
#define OTP_OPERATION_PUT_OBJECT                                0x05
#define OTP_OPERATION_GET_SYNC_OBJECT                           0x06
#define OTP_OPERATION_PUT_SYNC_OBJECT                           0x07
#define OTP_OPERATION_SET_PATH                                  0x08
#define OTP_OPERATION_CREATE                                    0x09
#define OTP_OPERATION_DELETE                                    0x10
#define OTP_OPERATION_ABORT                                     0x11

   /* The following enumerated type represents the supported Port       */
   /* Connection State types for Transport Port.                        */
typedef enum
{
   pcsNotConnected,
   pcsConnectionPending,
   pcsConnected
} PortConnectionState_t;

   /* The following enumeration defines the type of operation that is   */
   /* currently being performed by the FTPM Mgr layer.  These types are */
   /* used internal to the FTPM Mgr layer.                              */
typedef enum
{
   ssIdle,
   ssGetSetup,
   ssGet,
   ssPutSetup,
   ssPut
} ServerState_t;

   /* The following enumerated states represent the various Segmentation*/
   /* States that the Directory Entry Response can be in.  These States */
   /* are used so that if an entire directory entry (or any other       */
   /* header information) will not fit into an existing packet, it can  */
   /* be segmented over multiple packets.                               */
   /* * NOTE * The order of these States are dependent on the processing*/
   /*          in the function that uses these states.  DO NOT CHANGE   */
   /*          THE ORDER OF THESE ENUMERATIONS UNLESS YOU VERIFY THAT   */
   /*          THE FUNCTIONS THAT USE THESE STATES WILL FUNCTION        */
   /*          CORRECTLY.                                               */
typedef enum
{
   ssStart,
   ssRootDirectory,
   ssDirectoryListingName,
   ssDirectoryListingNameValue,
   ssDirectoryListingSize,
   ssDirectoryListingSizeValue,
   ssDirectoryListingType,
   ssDirectoryListingTypeValue,
   ssDirectoryListingModified,
   ssDirectoryListingModifiedValue,
   ssDirectoryListingCreated,
   ssDirectoryListingCreatedValue,
   ssDirectoryListingAccessed,
   ssDirectoryListingAccessedValue,
   ssDirectoryListingUserPermission,
   ssDirectoryListingUserPermissionValue,
   ssDirectoryListingGroupPermission,
   ssDirectoryListingGroupPermissionValue,
   ssDirectoryListingOtherPermission,
   ssDirectoryListingOtherPermissionValue,
   ssDirectoryListingOwner,
   ssDirectoryListingOwnerValue,
   ssDirectoryListingGroup,
   ssDirectoryListingGroupValue,
   ssLineTerminator,
   ssFooter,
   ssComplete
} SegmentationState_t;

   /* The following structure is used when receiving and building       */
   /* directory information with connected to the File Browser service. */
   /* The field DirIndex is used by the routine that converts the       */
   /* structure information into XML formatted data.  It is used to keep*/
   /* track of the entries that have been processed during situations   */
   /* where the directory will be required to be transferred in multiple*/
   /* OBEX packets.  The ObjectInfo parameter is a pointer to an array  */
   /* of Object_Info_t structures.  There are DirEntries number of      */
   /* elements in the array.                                            */
typedef struct _tagDirectoryInfo_t
{
   unsigned int         NumberEntries;
   unsigned int         DirectoryIndex;
   Boolean_t            ParentDirectory;
   long                 SegmentedLineLength;
   OTP_ObjectInfo_t    *ObjectInfoList;
   SegmentationState_t  SegmentationState;
} DirectoryInfo_t;

   /* The following structure holds All GOEP Port Information that is   */
   /* is to be added for a specified GOEP Port.                         */
typedef struct _tagGOEPM_Info_t
{
   unsigned int              GOEP_ID;
   Boolean_t                 Server;
   Word_t                    QueuedPacketLength;
   PortState_t               PortState;
   unsigned int              PortNumber;
   unsigned int              SPPPortHandle;
   DWord_t                   ServiceRecordHandle;
   Boolean_t                 RxBufferOverrun;
   Byte_t                    OverrunPendingResponse;
   Byte_t                   *RxBuffer;
   int                       RxBufferNdx;
   Byte_t                   *TxBuffer;
   int                       TxBufferNdx;
   Word_t                    PacketLength;
   Byte_t                    PendingCommand;
   Byte_t                    PendingResponse;
   Word_t                    OutMaxPacketLength;
   Word_t                    MaxPacketLength;
   GOEPM_Event_Callback_t    GOEPM_EventCallback;
   unsigned long             CallbackParameter;
   struct _tagGOEPM_Info_t  *NextGOEPInfoEntryPtr;
} GOEPM_Info_t;

   /* The following structure holds All OTPM Port Information that is   */
   /* is to be added for a specified OTPM Port.                         */
typedef struct _tagOTPM_Info_t
{
   unsigned int            OTP_ID;
   unsigned int            GOEP_ID;
   Boolean_t               Server;
   Word_t                  GOEPPacketSize;
   ServerState_t           ServerState;
   Boolean_t               ClientConnected;
   PortConnectionState_t   PortConnectionState;
   Byte_t                  CurrentOperation;
   Word_t                  MaxPacketSize;
   Byte_t                 *ResponseBuffer;
   OTP_Target_t            Target;
   SyncAnchor_t            SyncAnchor;
   DWord_t                 ConnectionID;
   OTP_ObjectInfo_t        ObjectInfo;
   char                   *ExtendedNameBuffer;
   DirectoryInfo_t         DirInfo;
   char                   *DirEntrySegment;
   unsigned long           UserInfo;
   _FTPM_Event_Callback_t  OTP_EventCallback;
   unsigned long           CallbackParameter;
   struct _tagOTPM_Info_t *NextInfoEntry;
} OTPM_Info_t;

   /* Variable which serves as a global flag about whether this module  */
   /* is initialized or not.                                            */
static Boolean_t Initialized;

   /* The following holds a pointer to the list that is maintained for  */
   /* all Port Connections (Server or Client).                          */
static GOEPM_Info_t *PortInfoList;

   /* The following holds a pointer to the list that is maintained for  */
   /* all FTPM Mgr Connections (Server or Client).                      */
static OTPM_Info_t *OTPInfoList;

   /* The following holds the next (unique) Connection ID.              */
static DWord_t NextConnectionID;

   /* Variable which is used to hold the next (unique) GOEP ID.         */
static unsigned int NextGOEPID;

   /* The following holds the next (unique) FTPM Mgr ID.                */
static unsigned int NextOTPID;

static BTPSCONST char IrSyncUUID[]          = { 'I','R','M','C','-','S','Y','N','C' };
static BTPSCONST char FolderBrowseUUID[]    = { (char)0xF9, (char)0xEC, (char)0x7B, (char)0xC4, (char)0x95, (char)0x3C, (char)0x11, (char)0xD2, (char)0x98, (char)0x4E, (char)0x52, (char)0x54, (char)0x00, (char)0xDC, (char)0x9E, (char)0x09};
static BTPSCONST char FolderListingType[]   = "x-obex/folder-listing";
static BTPSCONST char FolderListingHeader[] = "<?xml version=\"1.0\"?>\x0d\x0a"
                                          "<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\">\x0d\x0a"
                                          "<folder-listing version=\"1.0\">\x0d\x0a";
static BTPSCONST char ParentFolderTag[]     = "  <parent-folder/>\x0d\x0a";
static BTPSCONST char FolderNameHeader[]    = "  <folder name=\"";
static BTPSCONST char FileNameHeader[]      = "  <file name=\"";
static BTPSCONST char SizeHeader[]          = "\" size=\"";
static BTPSCONST char TypeHeader[]          = "\" type=\"";
static BTPSCONST char ModifiedHeader[]      = "\" modified=\"";
static BTPSCONST char CreatedHeader[]       = "\" created=\"";
static BTPSCONST char AccessedHeader[]      = "\" accessed=\"";
static BTPSCONST char UserPermHeader[]      = "\" user-perm=\"";
static BTPSCONST char GroupPermHeader[]     = "\" group-perm=\"";
static BTPSCONST char OtherPermHeader[]     = "\" other-perm=\"";
static BTPSCONST char OwnerHeader[]         = "\" owner=\"";
static BTPSCONST char GroupHeader[]         = "\" group=\"";
static BTPSCONST char LineTerminator[]      = "\"/>\x0d\x0a";
static BTPSCONST char FolderListingFooter[] = "</folder-listing>\x0d\x0a";

   /* Following tables are used to Validate a received Obex packet.     */
   /* Only the Commands and Response values in the table are considered */
   /* valid.  Each table is NULL terminated to indicate the end of the  */
   /* string.                                                           */
static Byte_t ServerCmdOpCodeList[] = {0x02,0x03,0x04,0x80,0x81,0x82,0x83,0x84,0x85,0xFF,0x00};
static Byte_t ClientRspOpCodeList[] = {0x10,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x30,0x31,0x32,0x33,0x34,0x35,0x40,0x41,
                                       0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,
                                       0x52,0x53,0x54,0x55,0x60,0x61,0x00};

   /* Internal GOEP Function Prototypes.                                */
static unsigned int GetNextGOEP_ID(void);

static GOEPM_Info_t *AddGOEPInfoEntry(GOEPM_Info_t **ListHead, GOEPM_Info_t *EntryToAdd);
static GOEPM_Info_t *SearchGOEPInfoEntry(GOEPM_Info_t **ListHead, unsigned int GOEP_ID);
static GOEPM_Info_t *DeleteGOEPInfoEntry(GOEPM_Info_t **ListHead, unsigned int GOEP_ID);
static void FreeGOEPInfoEntryMemory(GOEPM_Info_t *EntryToFree);
static void FreeGOEPInfoList(GOEPM_Info_t **ListHead);

static int GetMatchIndex(Byte_t ByteToMatch, Byte_t *ByteList);
static Byte_t GetNumberOfHeaders(Byte_t *Header, Word_t SizeOfHeaderData);
static void HeaderStreamToHeaderList(Byte_t *HeaderData, OBEX_Header_t *Header, Byte_t NumberOfHeaders);
static int HeaderListToHeaderStream(OBEX_Header_List_t *Header_List, GOEPM_Info_t *PortInfo);

static void GOEPServerEventHandler(GOEPM_Info_t *PortInfo);

static void GOEPClientEventHandler(GOEPM_Info_t *PortInfo);

static void ProcessDataFrame(GOEPM_Info_t *PortInfo);

static void GOEPSPPServerEventHandler(GOEPM_Info_t *PortInfo, SPPM_Event_Data_t *SPPMEventData);

static void GOEPSPPClientEventHandler(GOEPM_Info_t *PortInfo, SPPM_Event_Data_t *SPPMEventData);

static void BTPSAPI SPPM_Event_Callback(SPPM_Event_Data_t *SPPM_Event_Data, void *CallbackParameter);

static int _GOEP_Open_Server_Port(unsigned int ServerPort, unsigned long PortFlags, Word_t MaxPacketLength, GOEPM_Event_Callback_t GOEP_Event_Callback, unsigned long CallbackParameter);
static int _GOEP_Close_Server_Port(unsigned int GOEP_ID);
static int _GOEP_Open_Port_Request_Response(unsigned int GOEP_ID, Boolean_t AcceptConnection);
static int _GOEP_Register_SDP_Record(unsigned int GOEP_ID, GOEP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);
static int _GOEP_Open_Remote_Port(BD_ADDR_t BD_ADDR, unsigned int ServerPort, unsigned long OpenFlags, Word_t MaxPacketLength, GOEPM_Event_Callback_t GOEP_Event_Callback, unsigned long CallbackParameter, unsigned int *ConnectionStatus);
static int _GOEP_Close_Port(unsigned int GOEP_ID);
static int _GOEP_Connect_Request(unsigned int GOEP_ID, OBEX_Header_List_t *Header_List);
static int _GOEP_Disconnect_Request(unsigned int GOEP_ID, OBEX_Header_List_t *Header_List);
static int _GOEP_Put_Request(unsigned int GOEP_ID, Boolean_t Final, OBEX_Header_List_t *Header_List);
static int _GOEP_Get_Request(unsigned int GOEP_ID, Boolean_t Final, OBEX_Header_List_t *Header_List);
static int _GOEP_Set_Path_Request(unsigned int GOEP_ID, Byte_t Flags, OBEX_Header_List_t *Header_List);
static int _GOEP_Abort_Request(unsigned int GOEP_ID, OBEX_Header_List_t *Header_List);
static int _GOEP_Command_Response(unsigned int GOEP_ID, Byte_t ResponseCode, OBEX_Header_List_t *Header_List);

   /* Internal Function Prototypes.                                     */
static OTPM_Info_t *AddOTPInfoEntry(OTPM_Info_t **ListHead, OTPM_Info_t *EntryToAdd);
static OTPM_Info_t *SearchOTPInfoEntry(OTPM_Info_t **ListHead, unsigned int PortID);
static OTPM_Info_t *DeleteOTPInfoEntry(OTPM_Info_t **ListHead, unsigned int PortID);
static void FreeOTPInfoEntryMemory(OTPM_Info_t *EntryToFree);
static void FreeOTPInfoList(OTPM_Info_t **ListHead);

static DWord_t GetNextConnection_ID(void);
static unsigned int GetNextOTP_ID(void);
static void UnicodeHeaderValueToByteSequence(Word_t *UNICodePtr, unsigned int UNICodeStringLength, unsigned int UNICodeBufferSize);
static int CharSequenceToUnicodeHeaderValue(char *CharPtr, Byte_t *UnicodeText, int Length);
static int StructureToTagLengthValue(OBEX_Header_ID_t Header_ID, void *Src, OTP_Tag_Length_Value_t *Triplet);
static int TagLengthValueToStructure(OBEX_Header_ID_t Header_ID, int HeaderLength, OTP_Tag_Length_Value_t *Triplet, void *Dest);
static int FindHeader(OBEX_Header_ID_t HeaderID, OBEX_Header_List_t *ListPtr);
static Boolean_t IsFileFolderEntry(char *DataPtr, Boolean_t *ParentFolder);
static int ExtractInt(char *CharPtr, int NumberOfDigits);
static Byte_t ExtractPermissionInfo(char *PremPtr, int num_permissions);
static int ExtractTimeDate(char *TDStringPtr, OTP_TimeDate_t *TDStructPtr);
static void ExtractObjectInfo(OTPM_Info_t *OTPInfoPtr, OTP_ObjectInfo_t *ObjectInfoPtr, OBEX_Header_List_t *ListPtr);
static void ExtractSyncRequestParams(OTP_Sync_Request_Params_t *SyncParamsPtr, OBEX_Header_t *HeaderPtr);
static void ExtractSyncResponseParams(OTP_Sync_Response_Params_t *SyncParamsPtr, OBEX_Header_t *HeaderPtr);
static char *ExtractNameFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr);
static unsigned int ExtractNameLengthFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr, Boolean_t AddNullLength);
static long LoadXMLData(char **Dest, unsigned int *DestSize, char *Data, long Size, int SegmentedSize);
static int FileObjectToXML(unsigned int NumberOfEntries, OTP_ObjectInfo_t FileFolderInfo[], unsigned int SizeOfBuffer, char **Buffer, Boolean_t RootDirectory, SegmentationState_t *SegmentationState, long *SegmentedLineLength);
static int XMLToFileObject(unsigned int *NumberOfEntries, OTP_ObjectInfo_t *ObjectInfo, char **Segment, char *Buffer, Boolean_t *ParentFolder);
static void OTP_PortOpenRequestIndicationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Open_Request_Indication_Data_t *Port_Open_Request_Indication_Data);
static void OTP_PortOpenIndicationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Open_Indication_Data_t *Port_Open_Indication_Data);
static void OTP_PortOpenConfirmationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Open_Confirmation_Data_t *Port_Open_Confirmation_Data);
static void OTP_PortCloseIndicationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Close_Indication_Data_t *Port_Close_Indication_Data);
static void OTP_ConnectRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Connect_Indication_Data_t *Connect_Request);
static void OTP_ConnectResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Connect_Confirmation_Data_t *Connect_Response);
static void OTP_DisconnectRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Disconnect_Indication_Data_t *Disconnect_Request);
static void OTP_DisconnectResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Disconnect_Confirmation_Data_t *Disconnect_Response);
static void OTP_PutRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Put_Indication_Data_t *Put_Request);
static void OTP_PutResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Put_Confirmation_Data_t *Put_Response);
static void OTP_GetRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Get_Indication_Data_t *Get_Request);
static void OTP_GetResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Get_Confirmation_Data_t *Get_Response);
static void OTP_SetPathRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Set_Path_Indication_Data_t *Set_Path_Request);
static void OTP_SetPathResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Set_Path_Confirmation_Data_t *Set_Path_Response);
static void OTP_AbortRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Abort_Indication_Data_t *Abort_Request);
static void OTP_AbortResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Abort_Confirmation_Data_t *Abort_Response);
static void ProcessDirectoryRequest(OTPM_Info_t *ProfileInfoPtr);
static int BuildPutObjectRequest(OTPM_Info_t *OTPInfoPtr, unsigned int OTP_ID, Byte_t Action, unsigned int Length, char *Type, char *Name, unsigned long UserInfo);

static void BTPSAPI GOEP_Event_Callback(GOEP_Event_Data_t *GOEP_Event_Data, unsigned long CallbackParameter);

   /* Internal GOEP Function Prototypes.                                */

   /* The following function is responsible for returning the Next      */
   /* Available GOEP Port ID.  This function will only return GOEP      */
   /* Port ID's that will be interpreted as Positive Integers (i.e. the */
   /* Most Significant Bit will NEVER be set).                          */
static unsigned int GetNextGOEP_ID(void)
{
   /* Increment the Counter to the next number.  Check the new number   */
   /* to see if it has gone negative (when Port ID is viewed as a       */
   /* signed integer).  If so, return to the first valid Number (one).  */
   NextGOEPID++;

   if(((int)NextGOEPID) < 0)
      NextGOEPID = 1;

   /* Simply return the GOEP Port ID to the caller.                     */
   return(NextGOEPID);
}

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function will return NULL if no list entry is added.  */
   /* This can occur if the element passed in was deemed invalid or the */
   /* actual List Head was invalid.                                     */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            GOEP Port ID field is the same as an entry already in  */
   /*            the list.  When this occurs, this function returns     */
   /*            NULL.                                                  */
static GOEPM_Info_t *AddGOEPInfoEntry(GOEPM_Info_t **ListHead, GOEPM_Info_t *EntryToAdd)
{
   GOEPM_Info_t *AddedEntry = NULL;
   GOEPM_Info_t *tmpEntry;

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->GOEP_ID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (GOEPM_Info_t *)BTPS_AllocateMemory(sizeof(GOEPM_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry                      = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextGOEPInfoEntryPtr = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->GOEP_ID == AddedEntry->GOEP_ID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeGOEPInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextGOEPInfoEntryPtr)
                        tmpEntry = tmpEntry->NextGOEPInfoEntryPtr;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextGOEPInfoEntryPtr = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified GOEP_ID.  This function returns NULL if either the List */
   /* Head is invalid, the GOEP_ID is invalid, or the specified GOEP    */
   /* Port (based on the GOEP_ID) was not found.                        */
static GOEPM_Info_t *SearchGOEPInfoEntry(GOEPM_Info_t **ListHead, unsigned int GOEP_ID)
{
   GOEPM_Info_t *FoundEntry = NULL;

   /* Let's make sure the list and GOEP ID to search for appear to be   */
   /* valid.                                                            */
   if((ListHead) && (GOEP_ID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GOEP_ID != GOEP_ID))
         FoundEntry = FoundEntry->NextGOEPInfoEntryPtr;
   }

   return(FoundEntry);
}

   /* The following function searches the specified Port Information    */
   /* List for the specified GOEP Port and removes it from the List.    */
   /* This function returns NULL if either the Port Information List    */
   /* Head is invalid, the Port Number is invalid, or the specified Port*/
   /* Number was NOT present in the list.  The entry returned will have */
   /* the Next Entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeGOEPInfoEntryMemory().                                        */
static GOEPM_Info_t *DeleteGOEPInfoEntry(GOEPM_Info_t **ListHead, unsigned int GOEP_ID)
{
   GOEPM_Info_t *FoundEntry = NULL;
   GOEPM_Info_t *LastEntry  = NULL;

   /* Let's make sure the List and GOEP ID to search for appear to be   */
   /* semi-valid.                                                       */
   if((ListHead) && (GOEP_ID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->GOEP_ID != GOEP_ID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextGOEPInfoEntryPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextGOEPInfoEntryPtr = FoundEntry->NextGOEPInfoEntryPtr;
         }
         else
            *ListHead = FoundEntry->NextGOEPInfoEntryPtr;

         FoundEntry->NextGOEPInfoEntryPtr = NULL;
      }
   }

   return(FoundEntry);
}

   /* This function frees the specified Port Information Entry member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeGOEPInfoEntryMemory(GOEPM_Info_t *EntryToFree)
{
   if(EntryToFree)
   {
      /* Check to see if any Buffers has been created for this record.  */
      /* If so, free the Buffer Memory before freeing the record.       */
      if(EntryToFree->RxBuffer)
         BTPS_FreeMemory(EntryToFree->RxBuffer);

      if(EntryToFree->TxBuffer)
         BTPS_FreeMemory(EntryToFree->TxBuffer);

      BTPS_FreeMemory((void *)(EntryToFree));
   }
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Port Information List.  Upon return of   */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeGOEPInfoList(GOEPM_Info_t **ListHead)
{
   GOEPM_Info_t *EntryToFree;
   GOEPM_Info_t *tmpEntry;

   /* Let's make sure the List Head appears to be semi-valid.           */
   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextGOEPInfoEntryPtr;

         FreeGOEPInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }
}

   /* The following function is used to locate the byte value in a byte */
   /* list.  If the value is in the list the function return the offset */
   /* into the list where the valus was located.  else the function     */
   /* returns a negative number.                                        */
static int GetMatchIndex(Byte_t ByteToMatch, Byte_t *ByteList)
{
   int ret_val = -1;

   /* Verify that the parameters passed in appear valid.                */
   if((ByteToMatch) && (ByteList))
   {
      ret_val = 0;
      while((*ByteList) && (*ByteList != ByteToMatch))
      {
         ret_val++;
         ByteList++;
      }

      /* Check to see if we exited due to a NULL Character.             */
      if(!(*ByteList))
         ret_val = -1;
   }

   return(ret_val);
}

   /* The following function is a utility routine that will scan an OBEX*/
   /* packet and count the number of Optional Headers that are found in */
   /* that packet.  This information is needed to calculate the amount  */
   /* of memory that will be required to build an Event Packet to be    */
   /* dispatched to the upper layers.                                   */
static Byte_t GetNumberOfHeaders(Byte_t *Header, Word_t SizeOfHeaderData)
{
   Byte_t HeaderType;
   Byte_t NumberOfHeaders = 0;
   Word_t SizeOfHeader;

   /* While Header information is present, continue to count the number */
   /* of headers in the Packet.                                         */
   while((SizeOfHeaderData > 0) && (NumberOfHeaders < (Byte_t)(0xFF)))
   {
      /* The 2 MSBs of the Header indicate how the Header information is*/
      /* to be interpreted.  If the data is Null terminated or is a     */
      /* sequence of bytes, the 2 bytes following the Header type       */
      /* indicate the number of Bytes in the header.                    */
      HeaderType = (Byte_t)(*Header & OBEX_HEADER_TYPE_BIT_MASK);
      switch(HeaderType)
      {
         case OBEX_HEADER_TYPE_NULL_TERMINATED_UNICODE_TEXT_VALUE:
         case OBEX_HEADER_TYPE_BYTE_SEQUENCE_VALUE:
            /* Extract the number of bytes in the header which incudes  */
            /* the 3 bytes to account for the Header ID and Header Size */
            /* values.                                                  */
            SizeOfHeader = (Word_t)READ_UNALIGNED_WORD_BIG_ENDIAN(&(Header[1]));
            break;
         case OBEX_HEADER_TYPE_SINGLE_BYTE_VALUE:
            /* Set the Size to include the Header ID and the 1 byte     */
            /* value.                                                   */
            SizeOfHeader = 2;
            break;
         case OBEX_HEADER_TYPE_FOUR_BYTE_VALUE:
            /* Set the Size to include the Header ID and the 4 byte     */
            /* value.                                                   */
            SizeOfHeader = 5;
            break;
         default:
            SizeOfHeader = 0;
            break;
      }

      /* Verify that we have reached a valid header and the Header data */
      /* is within the bounds of the buffer that we are processing.     */
      if((SizeOfHeader) && (SizeOfHeader <= SizeOfHeaderData))
      {
         Header           += SizeOfHeader;
         SizeOfHeaderData -= SizeOfHeader;
         NumberOfHeaders++;
      }
      else
      {
         NumberOfHeaders = 0;
         break;
      }
   }

   return(NumberOfHeaders);
}

   /* The following function is used to convert a stream of header      */
   /* information located in the OBEX packet, into an array of Header   */
   /* structures that will be presented to the user.  The function takes*/
   /* as its parameters a pointer to be beginning of the Header Stream, */
   /* a pointer to a Header structure to receive the list of structures */
   /* and the number of headers that are to be processed.               */
static void HeaderStreamToHeaderList(Byte_t *HeaderData, OBEX_Header_t *Header, Byte_t NumberOfHeaders)
{
   Byte_t HeaderType;

   /* Each Header in the stream of data will need to be converted from  */
   /* the Stream and placed in a list structure.                        */
   while(NumberOfHeaders)
   {
      /* Map the Header ID and Header type to GOEP Parsable formats.    */
      switch(*HeaderData & OBEX_HEADER_ID_BIT_MASK)
      {
         case OBEX_HEADER_ID_COUNT_VALUE:
            Header->OBEX_Header_ID = hidCount;
            break;
         case OBEX_HEADER_ID_NAME_VALUE:
            Header->OBEX_Header_ID = hidName;
            break;
         case OBEX_HEADER_ID_TYPE_VALUE:
            Header->OBEX_Header_ID = hidType;
            break;
         case OBEX_HEADER_ID_LENGTH_VALUE:
            Header->OBEX_Header_ID = hidLength;
            break;
         case OBEX_HEADER_ID_TIME_VALUE:
            Header->OBEX_Header_ID = hidTime;
            break;
         case OBEX_HEADER_ID_DESCRIPTION_VALUE:
            Header->OBEX_Header_ID = hidDescription;
            break;
         case OBEX_HEADER_ID_TARGET_VALUE:
            Header->OBEX_Header_ID = hidTarget;
            break;
         case OBEX_HEADER_ID_HTTP_VALUE:
            Header->OBEX_Header_ID = hidHTTP;
            break;
         case OBEX_HEADER_ID_BODY_VALUE:
            Header->OBEX_Header_ID = hidBody;
            break;
         case OBEX_HEADER_ID_END_OF_BODY_VALUE:
            Header->OBEX_Header_ID = hidEndOfBody;
            break;
         case OBEX_HEADER_ID_WHO_VALUE:
            Header->OBEX_Header_ID = hidWho;
            break;
         case OBEX_HEADER_ID_CONNECTION_ID_VALUE:
            Header->OBEX_Header_ID = hidConnectionID;
            break;
         case OBEX_HEADER_ID_APPLICATION_PARAMETERS_VALUE:
            Header->OBEX_Header_ID = hidApplicationParameters;
            break;
         case OBEX_HEADER_ID_AUTHENTICATION_CHALLENGE_VALUE:
            Header->OBEX_Header_ID = hidAuthenticationChallenge;
            break;
         case OBEX_HEADER_ID_AUTHENTICATION_RESPONSE_VALUE:
            Header->OBEX_Header_ID = hidAuthenticationResponse;
            break;
         case OBEX_HEADER_ID_OBJECT_CLASS_VALUE:
            Header->OBEX_Header_ID = hidObjectClass;
            break;
         default:
            /* Unknown Header value (nothing else to do but to pass it  */
            /* on).                                                     */
            Header->OBEX_Header_ID = (OBEX_Header_ID_t)(*HeaderData & OBEX_HEADER_ID_BIT_MASK);
            break;
      }

      /* The 2 MSBs of the Header indicate how the Header information is*/
      /* to be interpreted.  If the data is Null terminated or is a     */
      /* sequence of bytes, the 2 bytes following the Header type       */
      /* indicate the number of Bytes in the header.                    */
      HeaderType = (Byte_t)(*HeaderData & OBEX_HEADER_TYPE_BIT_MASK);

      switch(HeaderType)
      {
         case OBEX_HEADER_TYPE_NULL_TERMINATED_UNICODE_TEXT_VALUE:
            /* Denote the Data type.                                    */
            Header->OBEX_Header_Type                    = htNullTerminatedUnicodeText;

            /* The length of a Unicode string should be the number of   */
            /* Words that make up the string.  When a Unicode string is */
            /* sent in an OBEX packet, the length indicates the number  */
            /* of Bytes that makes up the string.  When moving to the   */
            /* structures, the length must be adjusted to reflect the   */
            /* number of Words, not the number of bytes.                */
            Header->Header_Value.UnicodeText.DataLength = (Word_t)((READ_UNALIGNED_WORD_BIG_ENDIAN(&(HeaderData[1])) - 3) >> 1);

            /* The data from the byte stream will not be copied to the  */
            /* structure that holds the information, but instead the    */
            /* structure holds a pointer to the original data.  This is */
            /* important for the user of this data to know, so that if  */
            /* the data is not processed during the callback, the       */
            /* information pointed to must be copied to some other      */
            /* storage became the original data is destroyed when       */
            /* control is returned from the callback.                   */
            if(Header->Header_Value.UnicodeText.DataLength)
               Header->Header_Value.UnicodeText.ValuePointer  = (Word_t *)(&(HeaderData[3]));
            else
               Header->Header_Value.UnicodeText.ValuePointer  = NULL;

            /* Adjust the Byte Stream Pointer to the next Header field. */
            HeaderData += ((Header->Header_Value.UnicodeText.DataLength << 1)+3);
            break;
         case OBEX_HEADER_TYPE_BYTE_SEQUENCE_VALUE:
            /* Denote the Data type.                                    */
            Header->OBEX_Header_Type                       = htByteSequence;

            /* Save the Length of the Byte Sequence.                    */
            Header->Header_Value.ByteSequence.DataLength   = (Word_t)(READ_UNALIGNED_WORD_BIG_ENDIAN(&(HeaderData[1])) - 3);

            /* The data from the byte stream will not be copied to the  */
            /* structure that holds the information, but instead the    */
            /* structure holds a pointer to the original data.  This is */
            /* important for the user of this data to know, so that if  */
            /* the data is not processed during the callback, the       */
            /* information pointed to must be copied to some other      */
            /* storage became the original data is destroyed when       */
            /* control is returned from the callback.                   */
            Header->Header_Value.ByteSequence.ValuePointer = &(HeaderData[3]);

            /* Adjust the Byte Stream Pointer to the next Header field. */
            HeaderData                                    += (Header->Header_Value.ByteSequence.DataLength+3);
            break;
         case OBEX_HEADER_TYPE_SINGLE_BYTE_VALUE:
            /* Denote the Data type.                                    */
            Header->OBEX_Header_Type                       = htUnsignedInteger1Byte;

            /* Move the data byte to the structure.                     */
            Header->Header_Value.OneByteValue              = (Byte_t)READ_UNALIGNED_BYTE_BIG_ENDIAN(&(HeaderData[1]));

            /* Adjust the Byte Stream Pointer to the next Header field. */
            HeaderData                                    += 2;
            break;
         case OBEX_HEADER_TYPE_FOUR_BYTE_VALUE:
            /* Denote the Data type.                                    */
            Header->OBEX_Header_Type                       = htUnsignedInteger4Byte;

            /* Move the Quad data to the structure.                     */
            Header->Header_Value.FourByteValue             = (DWord_t)READ_UNALIGNED_DWORD_BIG_ENDIAN(&(HeaderData[1]));

            /* Adjust the Byte Stream Pointer to the next Header field. */
            HeaderData                                    += 5;
            break;
      }

      /* Increment to the next Header Position.                         */
      Header++;

      /* Decrement the count of the Number of Headers that are left to  */
      /* be processed.                                                  */
      NumberOfHeaders--;
   }
}

   /* The following function is used to convert an array of Header      */
   /* information that was received from the user to a stream of Header */
   /* data that is to be sent with an OBEX Request/Response.  The       */
   /* function takes as its parameter a pointer to the Header List and a*/
   /* pointer to a structure that identified a buffer in which the      */
   /* Header stream will be compiled along with information about the   */
   /* maximum number of bytes that can be compiled without violating the*/
   /* Maximum Packet size of the device that is to receive the          */
   /* Request/Response.                                                 */
static int HeaderListToHeaderStream(OBEX_Header_List_t *Header_List, GOEPM_Info_t *PortInfo)
{
   int            ret_val = 0;
   int            DataLength;
   Byte_t        *Buffer;
   Word_t         RemainingBuffer;
   OBEX_Header_t *Header;

   /* Verify that the pointers that were passed into the function appear*/
   /* to be valid pointers.                                             */
   if((Header_List) && (PortInfo))
   {
      /* Assign values to working pointers to aid in readability.       */
      Header          = Header_List->Headers;
      Buffer          = &(PortInfo->TxBuffer[PortInfo->TxBufferNdx]);

      /* The headers are to be built in a static buffer that has been   */
      /* sized for the Max Packet length of the remote device.  When    */
      /* building the header information, it is important not to build  */
      /* the header information that is greater than the data space that*/
      /* is available to hold the information.  Calculate the amount of */
      /* memory that is available for the header information.           */
      RemainingBuffer = (Word_t)(PortInfo->OutMaxPacketLength-PortInfo->TxBufferNdx);
      while(Header_List->NumberOfHeaders)
      {
         /* The Header ID here defines the lower bits of the Header ID  */
         /* that is discussed in the OBEX specification.  The Upper bits*/
         /* of the Header ID, as referenced by the spec, is contained in*/
         /* the Header Type parameter.                                  */
         switch(Header->OBEX_Header_ID)
         {
            case hidCount:
               *Buffer = OBEX_HEADER_ID_COUNT_VALUE;
               break;
            case hidName:
               *Buffer = OBEX_HEADER_ID_NAME_VALUE;
               break;
            case hidType:
               *Buffer = OBEX_HEADER_ID_TYPE_VALUE;
               break;
            case hidLength:
               *Buffer = OBEX_HEADER_ID_LENGTH_VALUE;
               break;
            case hidTime:
               *Buffer = OBEX_HEADER_ID_TIME_VALUE;
               break;
            case hidDescription:
               *Buffer = OBEX_HEADER_ID_DESCRIPTION_VALUE;
               break;
            case hidTarget:
               *Buffer = OBEX_HEADER_ID_TARGET_VALUE;
               break;
            case hidHTTP:
               *Buffer = OBEX_HEADER_ID_HTTP_VALUE;
               break;
            case hidBody:
               *Buffer = OBEX_HEADER_ID_BODY_VALUE;
               break;
            case hidEndOfBody:
               *Buffer = OBEX_HEADER_ID_END_OF_BODY_VALUE;
               break;
            case hidWho:
               *Buffer = OBEX_HEADER_ID_WHO_VALUE;
               break;
            case hidConnectionID:
               *Buffer = OBEX_HEADER_ID_CONNECTION_ID_VALUE;
               break;
            case hidApplicationParameters:
               *Buffer = OBEX_HEADER_ID_APPLICATION_PARAMETERS_VALUE;
               break;
            case hidAuthenticationChallenge:
               *Buffer = OBEX_HEADER_ID_AUTHENTICATION_CHALLENGE_VALUE;
               break;
            case hidAuthenticationResponse:
               *Buffer = OBEX_HEADER_ID_AUTHENTICATION_RESPONSE_VALUE;
               break;
            case hidObjectClass:
               *Buffer = OBEX_HEADER_ID_OBJECT_CLASS_VALUE;
               break;
            default:
               /* Uknown Header ID (nothing left to do but to pass it   */
               /* on).                                                  */
               *Buffer = (Byte_t)Header->OBEX_Header_ID;
               break;
         }

         switch(Header->OBEX_Header_Type)
         {
            case htNullTerminatedUnicodeText:
               /* The data length of a Null Terminated Unicode string   */
               /* will be twice the number of bytes as the length of the*/
               /* string because the string is made up of 16 bit values.*/
               /* When a header that is formatted as a Unicode string is*/
               /* sent in an OBEX packet, the length indicates the      */
               /* number of bytes that the string is made up of, which  */
               /* also should include a Word size Null terminator.      */
               DataLength = (int)(Header->Header_Value.UnicodeText.DataLength << 1) + 3;

               /* Verify that there is enough room to hold the data and */
               /* the header (ID and Length).                           */
               if(DataLength <= RemainingBuffer)
               {
                  /* When converting from the Header structure to a     */
                  /* header stream, the structure Header ID and Type    */
                  /* must be combined to match the Header ID that is    */
                  /* described in the specification.  Add the High order*/
                  /* bits to describe the format of the data to the     */
                  /* Header ID.                                         */
                  *Buffer |= OBEX_HEADER_TYPE_NULL_TERMINATED_UNICODE_TEXT_VALUE;

                  /* Load the Length of the Unicode String in the Word  */
                  /* after the Header ID.                               */
                  ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(Buffer[1]), DataLength);

                  if(DataLength > 3)
                  {
                     /* Copy the Unicode string, including the null     */
                     /* terminator after the length.                    */
                     BTPS_MemCopy(&(Buffer[3]), (Byte_t *)Header->Header_Value.UnicodeText.ValuePointer, (DataLength-3));
                  }
               }
               else
               {
                  /* Flag that there is not enough room to add any more */
                  /* header information to the buffer.                  */
                  DataLength = 0;
               }
               break;
            case htByteSequence:
               /* The length of a Byte Sequence is simply the length of */
               /* the sequence.                                         */
               DataLength = Header->Header_Value.ByteSequence.DataLength + 3;

               /* Verify that there is enough room to hold the data and */
               /* the header (ID and Length).                           */
               if(DataLength <= RemainingBuffer)
               {
                  /* When converting from the Header structure to a     */
                  /* header stream, the structure Header ID and Type    */
                  /* must be combined to match the Header ID that is    */
                  /* described in the specification.  Add the High order*/
                  /* bits to describe the format of the data to the     */
                  /* Header ID.                                         */
                  *Buffer |= OBEX_HEADER_TYPE_BYTE_SEQUENCE_VALUE;

                  /* Load the Length of the Byte Sequence in the Word   */
                  /* after the Header ID.                               */
                  ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(Buffer[1]), DataLength);

                  if(DataLength > 3)
                  {
                     /* Copy the Byte Sequence Data after the length.   */
                     BTPS_MemCopy(&(Buffer[3]), (Byte_t *)Header->Header_Value.ByteSequence.ValuePointer, (DataLength-3));
                  }
               }
               else
               {
                  /* Flag that there is not enough room to add any more */
                  /* header information to the buffer.                  */
                  DataLength = 0;
               }
               break;
            case htUnsignedInteger1Byte:
               /* Verify that there is enough room to hold the data and */
               /* the header (ID and Length).                           */
               if(RemainingBuffer >= 2)
               {
                  /* When converting from the Header structure to a     */
                  /* header stream, the structure Header ID and Type    */
                  /* must be combined to match the Header ID that is    */
                  /* described in the specification.  Add the High order*/
                  /* bits to describe the format of the data to the     */
                  /* Header ID.                                         */
                  *Buffer |= OBEX_HEADER_TYPE_SINGLE_BYTE_VALUE;

                  /* Load the Single Byte values right after the Header */
                  /* ID.                                                */
                  ASSIGN_HOST_BYTE_TO_BIG_ENDIAN_UNALIGNED_BYTE(&(Buffer[1]), Header->Header_Value.OneByteValue);

                  /* Total Header size for a Single Byte value is 2     */
                  /* bytes.                                             */
                  DataLength = 2;
               }
               else
               {
                  /* Flag that there is not enough room to add any more */
                  /* header information to the buffer.                  */
                  DataLength = 0;
               }
               break;
            case htUnsignedInteger4Byte:
               /* Verify that there is enough room to hold the data and */
               /* the header (ID and Length).                           */
               if(RemainingBuffer >= 5)
               {
                  /* When converting from the Header structure to a     */
                  /* header stream, the structure Header ID and Type    */
                  /* must be combined to match the Header ID that is    */
                  /* described in the specification.  Add the High order*/
                  /* bits to describe the format of the data to the     */
                  /* Header ID.                                         */
                  *Buffer |= OBEX_HEADER_TYPE_FOUR_BYTE_VALUE;

                  /* Load the Quad Byte value after the header ID.      */
                  ASSIGN_HOST_DWORD_TO_BIG_ENDIAN_UNALIGNED_DWORD(&(Buffer[1]), Header->Header_Value.FourByteValue);

                  /* Total Header size for a Quad Byte value is 5 bytes.*/
                  DataLength = 5;
               }
               else
               {
                  /* Flag that there is not enough room to add any more */
                  /* header information to the buffer.                  */
                  DataLength = 0;
               }
               break;
            default:
               DataLength = 0;
               break;
         }

         if(DataLength > 0)
         {
            /* Adjust the Buffer Pointer to past the end of the Header  */
            /* that we have just added.  Adjust the Buffer Index to     */
            /* account for the data that was added.                     */
            Buffer                += DataLength;
            PortInfo->TxBufferNdx += DataLength;

            /* Increment to the next Header Position.                   */
            Header++;

            /* Decrement the count of the Number of Headers that are    */
            /* left to be processed.                                    */
            Header_List->NumberOfHeaders--;
         }
         else
         {
            /* If the DataLength is 0, then an error occurred while     */
            /* adding some header information.  Exit the routine with an*/
            /* error.                                                   */
            ret_val = BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE;
            break;
         }
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for processing all Server   */
   /* OBEX commands for the specified GOEP/OBEX port:                   */
   /*    - OBEX_CONNECT_COMMAND                                         */
   /*    - OBEX_DISCONNECT_COMMAND                                      */
   /*    - OBEX_PUT_COMMAND                                             */
   /*    - OBEX_GET_COMMAND                                             */
   /*    - OBEX_SET_PATH_COMMAND                                        */
   /*    - OBEX_ABORT_COMMAND                                           */
static void GOEPServerEventHandler(GOEPM_Info_t *PortInfo)
{
   Byte_t             HeaderID;
   Byte_t             CmdID;
   Byte_t             NumberOfHeaders;
   int                BufferSize;
   Boolean_t          FinalFlag;
   GOEP_Event_Data_t *EventData;

   /* First make sure the input parameters appear to be semi-valid.     */
   if(PortInfo)
   {
      /* This 1st byte of data is the command byte.  The MSB of the     */
      /* command byte indicates whether this is the last packet in the  */
      /* command sequence for some commands.  Extract the Final Flag and*/
      /* mask the flag from the command ID.                             */
      HeaderID  = *PortInfo->RxBuffer;
      FinalFlag = (Boolean_t)((HeaderID & OBEX_FINAL_BIT) != 0);
      CmdID     = (Byte_t)(HeaderID & OBEX_OPCODE_RESPONSE_BIT_MASK);
      switch(CmdID)
      {
         case OBEX_CONNECT_COMMAND:
            /* Check to see that the minimum requirements for the       */
            /* Connect request are met.                                 */
            if((FinalFlag) && (PortInfo->PacketLength >= MINIMUM_OBEX_CONNECT_PACKET_SIZE))
            {
               PortInfo->OutMaxPacketLength = READ_UNALIGNED_WORD_BIG_ENDIAN(&(PortInfo->RxBuffer[OBEX_MAX_PACKET_LENGTH_OFFSET]));

               /* We have reserved buffer space for both sending and    */
               /* receiving to the size specified in MaxPacketLength.   */
               /* If the remote device indicates that it can accept a   */
               /* packet that is bigger than the buffer that we have    */
               /* reserved, adjust the OutMaxPacketLength so it is not  */
               /* greater than our buffer.                              */
               if(PortInfo->OutMaxPacketLength > PortInfo->MaxPacketLength)
                  PortInfo->OutMaxPacketLength = PortInfo->MaxPacketLength;

               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  The amount of memory required may be       */
               /* greater than the Maximum Packet size due to the       */
               /* conversion of the header information from a Byte      */
               /* Stream to a Structure array, so the actual amount must*/
               /* be calculated.  The amount required will include the  */
               /* size of the Event Structure, the EventData Structure  */
               /* and the Optional Headers.  Each Header will require a */
               /* fixed amount of memory, so the number of headers that */
               /* have been received will need to be known.             */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_CONNECT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_CONNECT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Connect */
               /* Indication Structure and the header list that will be */
               /* included.                                             */
               BufferSize = (sizeof(OBEX_Connect_Indication_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData  = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                      = etOBEX_Connect_Indication;
                  EventData->Event_Data_Size                                                      = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Connect_Indication_Data                              = (OBEX_Connect_Indication_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Connect_Indication_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Connect_Indication_Data->Version_Number              = READ_UNALIGNED_BYTE_BIG_ENDIAN(&(PortInfo->RxBuffer[OBEX_VERSION_NUMBER_OFFSET]));
                  EventData->Event_Data.OBEX_Connect_Indication_Data->Max_Packet_Length           = PortInfo->OutMaxPacketLength;
                  EventData->Event_Data.OBEX_Connect_Indication_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Connect_Indication_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Connect_Indication_Data + sizeof(OBEX_Connect_Indication_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_CONNECT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Connect_Indication_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Connect_Indication_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Set the Port State to Connecting.               */
                     PortInfo->PortState = psConnecting;

                     /* Note that a Response for this command will be   */
                     /* Expected.  When the response is made, the Port  */
                     /* state will be set per the response value.       */
                     PortInfo->PendingResponse = OBEX_CONNECT_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_DISCONNECT_COMMAND:
            /* Check to see that the minimum requirements for the       */
            /* Disconnect request are met.                              */
            if((FinalFlag) && (PortInfo->PacketLength >= MINIMUM_OBEX_DISCONNECT_PACKET_SIZE))
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_DISCONNECT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_DISCONNECT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the         */
               /* Disconnect Indication Structure and the header list   */
               /* that will be included.                                */
               BufferSize      = (sizeof(OBEX_Disconnect_Indication_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                         = etOBEX_Disconnect_Indication;
                  EventData->Event_Data_Size                                                         = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Disconnect_Indication_Data                              = (OBEX_Disconnect_Indication_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Disconnect_Indication_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Disconnect_Indication_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Disconnect_Indication_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Disconnect_Indication_Data + sizeof(OBEX_Disconnect_Indication_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_DISCONNECT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Disconnect_Indication_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Disconnect_Indication_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Set the Port State to Disconnecting.            */
                     PortInfo->PortState = psDisconnecting;

                     /* Note that a Response for this command will be   */
                     /* Expected.  When the response is made, the Port  */
                     /* state will be set per the response value.       */
                     PortInfo->PendingResponse = OBEX_DISCONNECT_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_PUT_COMMAND:
            /* Check to see that the minimum requirements for the Put   */
            /* request are met.                                         */
            if((PortInfo->PacketLength >= MINIMUM_OBEX_PUT_PACKET_SIZE))
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_PUT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_PUT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Put     */
               /* Indication Structure and the header list that will be */
               /* included.                                             */
               BufferSize      = (sizeof(OBEX_Put_Indication_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                  = etOBEX_Put_Indication;
                  EventData->Event_Data_Size                                                  = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Put_Indication_Data                              = (OBEX_Put_Indication_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Put_Indication_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Put_Indication_Data->Final_Flag                  = FinalFlag;
                  EventData->Event_Data.OBEX_Put_Indication_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Put_Indication_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Put_Indication_Data + sizeof(OBEX_Put_Indication_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_PUT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Put_Indication_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Put_Indication_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Note that a Response for this command will be   */
                     /* Expected.  When the response is made, the Port  */
                     /* state will be set per the response value.       */
                     PortInfo->PendingResponse = OBEX_PUT_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_GET_COMMAND:
            /* Check to see that the minimum requirements for the Get   */
            /* request are met.                                         */
            if((PortInfo->PacketLength >= MINIMUM_OBEX_GET_PACKET_SIZE))
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_GET_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_GET_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Get     */
               /* Indication Structure and the header list that will be */
               /* included.                                             */
               BufferSize      = (sizeof(OBEX_Get_Indication_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                  = etOBEX_Get_Indication;
                  EventData->Event_Data_Size                                                  = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Get_Indication_Data                              = (OBEX_Get_Indication_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Get_Indication_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Get_Indication_Data->Final_Flag                  = FinalFlag;
                  EventData->Event_Data.OBEX_Get_Indication_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Get_Indication_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Get_Indication_Data + sizeof(OBEX_Get_Indication_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_GET_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Get_Indication_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Get_Indication_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Note that a Response for this command will be   */
                     /* Expected.  When the response is made, the Port  */
                     /* state will be set per the response value.       */
                     PortInfo->PendingResponse = OBEX_GET_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_SET_PATH_COMMAND:
            /* Check to see that the minimum requirements for the Get   */
            /* request are met.                                         */
            if((PortInfo->PacketLength >= MINIMUM_OBEX_SET_PATH_PACKET_SIZE))
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_SET_PATH_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_SET_PATH_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Set Path*/
               /* Indication Structure and the header list that will be */
               /* included.                                             */
               BufferSize      = (sizeof(OBEX_Set_Path_Indication_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                       = etOBEX_Set_Path_Indication;
                  EventData->Event_Data_Size                                                       = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Set_Path_Indication_Data                              = (OBEX_Set_Path_Indication_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Set_Path_Indication_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Set_Path_Indication_Data->Backup                      = (Boolean_t)((PortInfo->RxBuffer[OBEX_SET_PATH_FLAGS_OFFSET] & OBEX_SET_PATH_FLAGS_BACKUP_MASK) != 0);
                  EventData->Event_Data.OBEX_Set_Path_Indication_Data->CreateDirectory             = (Boolean_t)((PortInfo->RxBuffer[OBEX_SET_PATH_FLAGS_OFFSET] & OBEX_SET_PATH_FLAGS_NO_CREATE_MASK) == 0);
                  EventData->Event_Data.OBEX_Set_Path_Indication_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Set_Path_Indication_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Set_Path_Indication_Data + sizeof(OBEX_Set_Path_Indication_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_SET_PATH_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Set_Path_Indication_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Set_Path_Indication_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Note that a Response for this command will be   */
                     /* Expected.  When the response is made, the Port  */
                     /* state will be set per the response value.       */
                     PortInfo->PendingResponse = OBEX_SET_PATH_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_ABORT_COMMAND:
            /* Check to see that the minimum requirements for the Abort */
            /* request are met.                                         */
            if((PortInfo->PacketLength >= MINIMUM_OBEX_ABORT_PACKET_SIZE))
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_ABORT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_ABORT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Abort   */
               /* Indication Structure and the header list that will be */
               /* included.                                             */
               BufferSize      = (sizeof(OBEX_Abort_Indication_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                    = etOBEX_Abort_Indication;
                  EventData->Event_Data_Size                                                    = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Abort_Indication_Data                              = (OBEX_Abort_Indication_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Abort_Indication_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Abort_Indication_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Abort_Indication_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Abort_Indication_Data + sizeof(OBEX_Abort_Indication_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_ABORT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Abort_Indication_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Abort_Indication_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Note that a Response for this command will be   */
                     /* Expected.  When the response is made, the Port  */
                     /* state will be set per the response value.       */
                     PortInfo->PendingResponse = OBEX_ABORT_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         default:
            /* Unknown command.                                         */
            break;
      }
   }
}

   /* The following function is responsible for processing all client   */
   /* OBEX responses for the specified GOEP/OBEX port:                  */
   /*    - OBEX_CONNECT_COMMAND                                         */
   /*    - OBEX_DISCONNECT_COMMAND                                      */
   /*    - OBEX_PUT_COMMAND                                             */
   /*    - OBEX_GET_COMMAND                                             */
   /*    - OBEX_SET_PATH_COMMAND                                        */
   /*    - OBEX_ABORT_COMMAND                                           */
static void GOEPClientEventHandler(GOEPM_Info_t *PortInfo)
{
   Byte_t             HeaderID;
   Byte_t             NumberOfHeaders;
   int                BufferSize;
   GOEP_Event_Data_t *EventData;

   /* First make sure the input parameters appear to be semi-valid.     */
   if(PortInfo)
   {
      /* Note the Header ID (for later use).                            */
      HeaderID  = *PortInfo->RxBuffer;

      /* If the data does not constitute a Command, then it MUST be a   */
      /* Response.  For Each Pending Command, send the Response to the  */
      /* User.                                                          */
      switch(PortInfo->PendingCommand)
      {
         case OBEX_CONNECT_COMMAND:
            if(PortInfo->PacketLength >= MINIMUM_OBEX_CONNECT_PACKET_SIZE)
            {
               /* Since this is a response to a Connect Request, check  */
               /* the response to see if the Request was Accepted.  An  */
               /* 'OK' with the high bit set is the only positive       */
               /* response for the request.  If it is anything else,    */
               /* then it was refused.  Set the Port state to the       */
               /* correct State.                                        */
               PortInfo->PortState = (HeaderID == (Byte_t)(OBEX_OK_RESPONSE | OBEX_FINAL_BIT)?psConnected:psPortOpened);

               /* We have reserved buffer space for both sending and    */
               /* receiving to the size specified in MaxPacketLength.   */
               /* If the remote device indicates that it can accept a   */
               /* packet that is bigger than the buffer that we have    */
               /* reserved, adjust the OutMaxPacketLength so it is not  */
               /* greater than our buffer.                              */
               PortInfo->OutMaxPacketLength = READ_UNALIGNED_WORD_BIG_ENDIAN(&(PortInfo->RxBuffer[OBEX_MAX_PACKET_LENGTH_OFFSET]));
               if(PortInfo->OutMaxPacketLength > PortInfo->MaxPacketLength)
                  PortInfo->OutMaxPacketLength = PortInfo->MaxPacketLength;

               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  The amount of memory required may be       */
               /* greater than the Maximum Packet size due to the       */
               /* conversion of the header information from a Byte      */
               /* Stream to a Structure array, so the actual amount must*/
               /* be calculated.  The amount required will include the  */
               /* size of the Event Structure, the EventData Structure  */
               /* and the Optional Headers.  Each Header will require a */
               /* fixed amount of memory, so the number of headers that */
               /* have been received will need to be known.             */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_CONNECT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_CONNECT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Connect */
               /* Confirmation Structure and the header list that will  */
               /* be included.                                          */
               BufferSize      = (sizeof(OBEX_Connect_Confirmation_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                        = etOBEX_Connect_Confirmation;
                  EventData->Event_Data_Size                                                        = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data                              = (OBEX_Connect_Confirmation_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data->Response_Code               = HeaderID;
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data->Version_Number              = READ_UNALIGNED_BYTE_BIG_ENDIAN(&(PortInfo->RxBuffer[OBEX_VERSION_NUMBER_OFFSET]));
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data->Flags                       = READ_UNALIGNED_BYTE_BIG_ENDIAN(&(PortInfo->RxBuffer[OBEX_CONNECT_FLAGS_OFFSET]));
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data->Max_Packet_Length           = PortInfo->OutMaxPacketLength;
                  EventData->Event_Data.OBEX_Connect_Confirmation_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Connect_Confirmation_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Connect_Confirmation_Data + sizeof(OBEX_Connect_Confirmation_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_CONNECT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Connect_Confirmation_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Connect_Confirmation_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Clear the pending command not that we have      */
                     /* received the response.                          */
                     PortInfo->PendingCommand = OBEX_INVALID_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_DISCONNECT_COMMAND:
            if(PortInfo->PacketLength >= MINIMUM_OBEX_DISCONNECT_PACKET_SIZE)
            {
               /* The Disconnect is complete, so the Port State should  */
               /* be set to Port Connected.                             */
               PortInfo->PortState = psPortOpened;

               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_DISCONNECT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_DISCONNECT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the         */
               /* Disconnect Confirmation Structure and the header list */
               /* that will be included.                                */
               BufferSize      = (sizeof(OBEX_Disconnect_Confirmation_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                           = etOBEX_Disconnect_Confirmation;
                  EventData->Event_Data_Size                                                           = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Disconnect_Confirmation_Data                              = (OBEX_Disconnect_Confirmation_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Disconnect_Confirmation_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Disconnect_Confirmation_Data->Response_Code               = HeaderID;
                  EventData->Event_Data.OBEX_Disconnect_Confirmation_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Disconnect_Confirmation_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Disconnect_Confirmation_Data + sizeof(OBEX_Disconnect_Confirmation_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_DISCONNECT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Disconnect_Confirmation_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Disconnect_Confirmation_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Clear the pending command not that we have      */
                     /* received the response.                          */
                     PortInfo->PendingCommand = OBEX_INVALID_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_PUT_COMMAND:
            if(PortInfo->PacketLength >= MINIMUM_OBEX_PUT_PACKET_SIZE)
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_PUT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_PUT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Put     */
               /* Confirmation Structure and the header list that will  */
               /* be included.                                          */
               BufferSize      = (sizeof(OBEX_Put_Confirmation_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                    = etOBEX_Put_Confirmation;
                  EventData->Event_Data_Size                                                    = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Put_Confirmation_Data                              = (OBEX_Put_Confirmation_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Put_Confirmation_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Put_Confirmation_Data->Response_Code               = HeaderID;
                  EventData->Event_Data.OBEX_Put_Confirmation_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Put_Confirmation_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Put_Confirmation_Data + sizeof(OBEX_Put_Confirmation_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_PUT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Put_Confirmation_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Put_Confirmation_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Clear the pending command not that we have      */
                     /* received the response.                          */
                     PortInfo->PendingCommand = OBEX_INVALID_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_GET_COMMAND:
            if(PortInfo->PacketLength >= MINIMUM_OBEX_GET_PACKET_SIZE)
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_GET_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_GET_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Get     */
               /* Confirmation Structure and the header list that will  */
               /* be included.                                          */
               BufferSize      = (sizeof(OBEX_Get_Confirmation_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                    = etOBEX_Get_Confirmation;
                  EventData->Event_Data_Size                                                    = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Get_Confirmation_Data                              = (OBEX_Get_Confirmation_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Get_Confirmation_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Get_Confirmation_Data->Response_Code               = HeaderID;
                  EventData->Event_Data.OBEX_Get_Confirmation_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Get_Confirmation_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Get_Confirmation_Data + sizeof(OBEX_Get_Confirmation_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_GET_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Get_Confirmation_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Get_Confirmation_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Clear the pending command not that we have      */
                     /* received the response.                          */
                     PortInfo->PendingCommand = OBEX_INVALID_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_SET_PATH_COMMAND:
            if(PortInfo->PacketLength >= MINIMUM_OBEX_SET_PATH_PACKET_SIZE)
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_SET_PATH_RESPONSE_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_SET_PATH_RESPONSE_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Get     */
               /* Confirmation Structure and the header list that will  */
               /* be included.                                          */
               BufferSize      = (sizeof(OBEX_Set_Path_Confirmation_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                         = etOBEX_Set_Path_Confirmation;
                  EventData->Event_Data_Size                                                         = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Set_Path_Confirmation_Data                              = (OBEX_Set_Path_Confirmation_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Set_Path_Confirmation_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Set_Path_Confirmation_Data->Response_Code               = HeaderID;
                  EventData->Event_Data.OBEX_Set_Path_Confirmation_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Set_Path_Confirmation_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Set_Path_Confirmation_Data + sizeof(OBEX_Set_Path_Confirmation_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_SET_PATH_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Set_Path_Confirmation_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Set_Path_Confirmation_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Clear the pending command not that we have      */
                     /* received the response.                          */
                     PortInfo->PendingCommand = OBEX_INVALID_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         case OBEX_ABORT_COMMAND:
            if(PortInfo->PacketLength >= MINIMUM_OBEX_ABORT_PACKET_SIZE)
            {
               /* Memory must be allocated to provide enough memory to  */
               /* contain all of the information that is dispatched to  */
               /* the user.  See OBEX_CONNECT_COMMAND for Information.  */
               NumberOfHeaders = GetNumberOfHeaders(&(PortInfo->RxBuffer[OBEX_ABORT_PACKET_HEADER_OFFSET]), (Word_t)(PortInfo->PacketLength - OBEX_ABORT_PACKET_HEADER_OFFSET));

               /* Calculate the size of memory required for the Get     */
               /* Confirmation Structure and the header list that will  */
               /* be included.                                          */
               BufferSize      = (sizeof(OBEX_Abort_Confirmation_Data_t)+(OBEX_HEADER_SIZE*NumberOfHeaders));

               /* Allocate the memory to hold all of the information.   */
               EventData       = (GOEP_Event_Data_t *)BTPS_AllocateMemory(GOEP_EVENT_DATA_SIZE+BufferSize);
               if(EventData)
               {
                  EventData->Event_Data_Type                                                      = etOBEX_Abort_Confirmation;
                  EventData->Event_Data_Size                                                      = (Word_t)BufferSize;
                  EventData->Event_Data.OBEX_Abort_Confirmation_Data                              = (OBEX_Abort_Confirmation_Data_t *)((Byte_t *)EventData+GOEP_EVENT_DATA_SIZE);
                  EventData->Event_Data.OBEX_Abort_Confirmation_Data->GOEP_ID                     = PortInfo->GOEP_ID;
                  EventData->Event_Data.OBEX_Abort_Confirmation_Data->Response_Code               = HeaderID;
                  EventData->Event_Data.OBEX_Abort_Confirmation_Data->Header_List.NumberOfHeaders = NumberOfHeaders;

                  if(NumberOfHeaders)
                  {
                     EventData->Event_Data.OBEX_Abort_Confirmation_Data->Header_List.Headers      = (OBEX_Header_t *)((Byte_t *)EventData->Event_Data.OBEX_Abort_Confirmation_Data + sizeof(OBEX_Abort_Confirmation_Data_t));
                     HeaderStreamToHeaderList(&(PortInfo->RxBuffer[OBEX_ABORT_PACKET_HEADER_OFFSET]), EventData->Event_Data.OBEX_Abort_Confirmation_Data->Header_List.Headers, NumberOfHeaders);
                  }
                  else
                     EventData->Event_Data.OBEX_Abort_Confirmation_Data->Header_List.Headers      = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     /* Clear the pending command not that we have      */
                     /* received the response.                          */
                     PortInfo->PendingCommand = OBEX_INVALID_COMMAND;

                     (*PortInfo->GOEPM_EventCallback)(EventData, PortInfo->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }

                  BTPS_FreeMemory(EventData);
               }
            }
            break;
         default:
            /* Unknown response.                                        */
            break;
      }
   }
}

   /* The following function is used to process an OBEX packet that has */
   /* been received from the remote device.  The function takes as its  */
   /* parameter a pointer to a structure that contains information about*/
   /* the Port on which the data is received, which also contains the   */
   /* receive buffer.                                                   */
static void ProcessDataFrame(GOEPM_Info_t *PortInfo)
{
   /* Verify the pointer to the data is Valid.                          */
   if((PortInfo) && (PortInfo->GOEPM_EventCallback))
   {
      /* This 1st byte of data is the command byte.  The MSB of the     */
      /* command byte indicates whether this is the last packet in the  */
      /* command sequence for some commands.                            */
      switch(*PortInfo->RxBuffer & OBEX_OPCODE_RESPONSE_BIT_MASK)
      {
         case OBEX_CONNECT_COMMAND:
         case OBEX_DISCONNECT_COMMAND:
         case OBEX_PUT_COMMAND:
         case OBEX_GET_COMMAND:
         case OBEX_SET_PATH_COMMAND:
         case OBEX_ABORT_COMMAND:
            GOEPServerEventHandler(PortInfo);
            break;
         default:
            /* If the data does not constitute a Command, then it MUST  */
            /* be a Response.  For Each Pending Command, send the       */
            /* Response to the User.                                    */
            switch(PortInfo->PendingCommand)
            {
               case OBEX_CONNECT_COMMAND:
               case OBEX_DISCONNECT_COMMAND:
               case OBEX_PUT_COMMAND:
               case OBEX_GET_COMMAND:
               case OBEX_SET_PATH_COMMAND:
               case OBEX_ABORT_COMMAND:
                  GOEPClientEventHandler(PortInfo);
                  break;
               default:
                  /* Unknown command/response.                          */
                  break;
            }
            break;
      }
   }
}

   /* The following function is responsible for processing the SPP      */
   /* Server specific Events.  This function process the following      */
   /* SPP events.                                                       */
   /*    - etPort_Open_Request_Indication                               */
   /*    - etPort_Open_Indication                                       */
static void GOEPSPPServerEventHandler(GOEPM_Info_t *PortInfo, SPPM_Event_Data_t *SPPMEventData)
{
   GOEP_Event_Data_t GOEPEventData;

   union
   {
      OBEX_Port_Open_Request_Indication_Data_t OBEX_Port_Open_Request_Indication_Data;
      OBEX_Port_Open_Indication_Data_t         OBEX_Port_Open_Indication_Data;
   } Event_Data_Buffer;

   /* First, let's make sure the input parameters appear to be          */
   /* semi-valid.                                                       */
   if((PortInfo) && (SPPMEventData))
   {
      /* Process the event based on the event type.                     */
      if(SPPMEventData->EventType == setServerPortOpenRequest)
      {
         /* Set the Port to the Opening State.                          */
         PortInfo->PortState                                                      = psPortOpening;

         GOEPEventData.Event_Data_Type                                            = etOBEX_Port_Open_Request_Indication;
         GOEPEventData.Event_Data_Size                                            = OBEX_PORT_OPEN_REQUEST_INDICATION_DATA_SIZE;
         GOEPEventData.Event_Data.OBEX_Port_Open_Request_Indication_Data          = &(Event_Data_Buffer.OBEX_Port_Open_Request_Indication_Data);

         GOEPEventData.Event_Data.OBEX_Port_Open_Request_Indication_Data->GOEP_ID = PortInfo->GOEP_ID;
         GOEPEventData.Event_Data.OBEX_Port_Open_Request_Indication_Data->BD_ADDR = SPPMEventData->EventData.ServerPortOpenRequestEventData.RemoteDeviceAddress;
      }
      else
      {
         /* Set the Port to the Opened State.                           */
         PortInfo->PortState                                              = psPortOpened;

         GOEPEventData.Event_Data_Type                                    = etOBEX_Port_Open_Indication;
         GOEPEventData.Event_Data_Size                                    = OBEX_PORT_OPEN_INDICATION_DATA_SIZE;
         GOEPEventData.Event_Data.OBEX_Port_Open_Indication_Data          = &(Event_Data_Buffer.OBEX_Port_Open_Indication_Data);

         GOEPEventData.Event_Data.OBEX_Port_Open_Indication_Data->GOEP_ID = PortInfo->GOEP_ID;
         GOEPEventData.Event_Data.OBEX_Port_Open_Indication_Data->BD_ADDR = SPPMEventData->EventData.ServerPortOpenEventData.RemoteDeviceAddress;
      }

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*PortInfo->GOEPM_EventCallback)(&GOEPEventData, PortInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is responsible for processing the SPP      */
   /* Client specific Events.                                           */
static void GOEPSPPClientEventHandler(GOEPM_Info_t *PortInfo, SPPM_Event_Data_t *SPPMEventData)
{
   int                                Result;
   unsigned int                       GOEP_ID;
   GOEP_Event_Data_t                  GOEPEventData;
   OBEX_Port_Open_Confirmation_Data_t OBEX_Port_Open_Confirmation_Data;

   /* First, let's make sure the input parameters appear to be          */
   /* semi-valid.                                                       */
   if((PortInfo) && (SPPMEventData))
   {
      /* Check to see if we have a queued packet change (and the        */
      /* connection was successfull).                                   */
      if((!SPPMEventData->EventData.RemotePortOpenStatusEventData.Status) && (PortInfo->QueuedPacketLength))
      {
         Result = SPPM_ChangeBufferSize(PortInfo->SPPPortHandle, PortInfo->QueuedPacketLength, PortInfo->QueuedPacketLength);

         /* Check to see if the buffer was successful.  If not we will  */
         /* close the port.                                             */
         if(!Result)
            PortInfo->QueuedPacketLength = 0;
         else
            SPPM_ClosePort(PortInfo->SPPPortHandle, SPPM_CLOSE_DATA_FLUSH_TIMEOUT_IMMEDIATE);
      }
      else
         Result = 0;

      /* Note the GOEP_ID (we will need it later).                      */
      GOEP_ID                                                                   = PortInfo->GOEP_ID;

      /* Set the Port to the Opened State.                              */
      PortInfo->PortState                                                       = psPortOpened;

      /* Format the Event Data structure with information about the     */
      /* Connect Result.                                                */
      GOEPEventData.Event_Data_Type                                             = etOBEX_Port_Open_Confirmation;
      GOEPEventData.Event_Data_Size                                             = OBEX_PORT_OPEN_CONFIRMATION_DATA_SIZE;
      GOEPEventData.Event_Data.OBEX_Port_Open_Confirmation_Data                 = &OBEX_Port_Open_Confirmation_Data;

      GOEPEventData.Event_Data.OBEX_Port_Open_Confirmation_Data->GOEP_ID        = GOEP_ID;
      GOEPEventData.Event_Data.OBEX_Port_Open_Confirmation_Data->PortOpenStatus = (Result==0)?SPPMEventData->EventData.RemotePortOpenStatusEventData.Status:SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_UNKNOWN;

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*PortInfo->GOEPM_EventCallback)(&GOEPEventData, PortInfo->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }

      /* In case it was deleted, refetch the Port Information           */
      PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
      if(PortInfo)
      {
         if((Result) || (SPPMEventData->EventData.RemotePortOpenStatusEventData.Status))
         {
            if((PortInfo = DeleteGOEPInfoEntry(&PortInfoList, PortInfo->GOEP_ID)) != NULL)
               FreeGOEPInfoEntryMemory(PortInfo);
         }
      }
   }
}

   /* The following function is used to process Events that occur at the*/
   /* SPP level.  Note that for GOEP, the events that deal with the     */
   /* hardware features of the serial port are not used.  The only      */
   /* events of interest are the Open, Close and Data events.           */
static void BTPSAPI SPPM_Event_Callback(SPPM_Event_Data_t *SPPM_Event_Data, void *CallbackParameter)
{
   int                                Length;
   Word_t                             BytesToRead;
   GOEPM_Info_t                        *PortInfo;
   GOEP_Event_Data_t                  EventData;
   OBEX_Port_Close_Indication_Data_t  ClosePortIndicationData;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter (GOEPM):\n"));

   /* Before proceeding any further, let's make sure that the data that */
   /* was passed to us appears semi-valid.                              */
   if((SPPM_Event_Data) && (CallbackParameter))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            /* Before proceeding any further let's make sure that we    */
            /* know about this connection (either have a server or      */
            /* initiated the client connection).                        */
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, (unsigned int)CallbackParameter);
            if((PortInfo) && (PortInfo->GOEPM_EventCallback))
            {
               /* We do indeed know about this SPP Channel, now we need */
               /* to process the SPP Event.                             */
               Length    = 0;
               switch(SPPM_Event_Data->EventType)
               {
                  case setServerPortOpenRequest:
                     GOEPSPPServerEventHandler(PortInfo, SPPM_Event_Data);
                     break;
                  case setServerPortOpen:
                     GOEPSPPServerEventHandler(PortInfo, SPPM_Event_Data);
                     break;
                  case setRemotePortOpenStatus:
                     GOEPSPPClientEventHandler(PortInfo, SPPM_Event_Data);
                     break;
                  case setPortClose:
                     /* Set the Port to the Idle State.                 */
                     PortInfo->PortState                                            = psIdle;

                     /* Format the Event Data structure with information*/
                     /* about the Connect Result.                       */
                     EventData.Event_Data_Type                                     = etOBEX_Port_Close_Indication;
                     EventData.Event_Data_Size                                     = (Word_t)OBEX_PORT_CLOSE_INDICATION_DATA_SIZE;
                     EventData.Event_Data.OBEX_Port_Close_Indication_Data          = &ClosePortIndicationData;

                     EventData.Event_Data.OBEX_Port_Close_Indication_Data->GOEP_ID = (unsigned int)CallbackParameter;

                     /* Finally make the callback.                      */
                     __BTPSTRY
                     {
                        (*PortInfo->GOEPM_EventCallback)(&EventData, PortInfo->CallbackParameter);
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }

                     /* In case it was deleted.                         */
                     PortInfo = SearchGOEPInfoEntry(&PortInfoList, (unsigned int)CallbackParameter);
                     if(PortInfo)
                     {
                        if(PortInfo->Server)
                        {
                           /* Since we are a server, then the port must */
                           /* be set back to a ready state and wait for */
                           /* another connection.  Clear any information*/
                           /* that pertains specifically to the         */
                           /* connection that was just terminated.      */
                           PortInfo->TxBufferNdx        = 0;
                           PortInfo->RxBufferNdx        = 0;
                           PortInfo->OutMaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;
                           PortInfo->PendingCommand     = 0;
                           PortInfo->PendingResponse    = OBEX_INVALID_COMMAND;
                           PortInfo->PacketLength       = 0;
                           PortInfo->PortState          = psIdle;
                        }
                        else
                        {
                           /* Since this is not a Server Connection,    */
                           /* then remove all information that pertains */
                           /* to the connection that was just terminated*/
                           /* from the port list.                       */
                           DeleteGOEPInfoEntry(&PortInfoList, PortInfo->GOEP_ID);

                           FreeGOEPInfoEntryMemory(PortInfo);
                        }
                     }
                     break;
                  case setDataReceived:
                     /* A number of bytes have been received.  In OBEX, */
                     /* data is received 1 packet at a time, with the   */
                     /* exception of an Abort Packet.  Worst case, would*/
                     /* occur if a response packet and an Abort packet  */
                     /* are received at the same time.  This could mean */
                     /* that we have received twice as much data as we  */
                     /* have allowed via the MaxPacketLength.  We must  */
                     /* ensure that we read and process 1 packet at a   */
                     /* time.  Since we can not rely on the Length      */
                     /* parameter received from this indication to      */
                     /* depict the length of an entire OBEX packet, we  */
                     /* must get the length of the OBEX from the OBEX   */
                     /* packet itself.  The entire length of the OBEX   */
                     /* packet is contained in the 2nd and 3rd bytes of */
                     /* the packet.  Once we determine the length of the*/
                     /* packet, we will compile the entire packet before*/
                     /* processing the data.                            */
                     while(TRUE)
                     {
                        /* We need to compile the Minimum packet data in*/
                        /* order to determine the OBEX Packet Length.   */
                        if((PortInfo->RxBuffer) && (PortInfo->RxBufferNdx < MINIMUM_OBEX_PACKET_SIZE))
                        {
                           /* Calculate the number of bytes that must be*/
                           /* read in order to obtain the Length of the */
                           /* packet.                                   */
                           BytesToRead = (Word_t)(MINIMUM_OBEX_PACKET_SIZE - PortInfo->RxBufferNdx);
                           Length      = SPPM_ReadData(PortInfo->SPPPortHandle, SPPM_READ_DATA_READ_TIMEOUT_IMMEDIATE, BytesToRead, &(PortInfo->RxBuffer[PortInfo->RxBufferNdx]));

                           /* Check to see if there was an error on the */
                           /* read or if there is no data to read.  The */
                           /* only time that and error is expected to   */
                           /* possibly occur, is if the stack is being  */
                           /* closed during the read.                   */
                           if(Length <= 0)
                              break;

                           /* Update the Index by the amount of bytes   */
                           /* that were successfully read.              */
                           PortInfo->RxBufferNdx += Length;

                           /* Check to see if we now have enough to     */
                           /* extract the Packet Size.                  */
                           if(PortInfo->RxBufferNdx == MINIMUM_OBEX_PACKET_SIZE)
                           {
                              /* The Length is in Big Endian format,    */
                              /* whose value depicts the number of bytes*/
                              /* in the packet, including any header    */
                              /* information.                           */
                              PortInfo->PacketLength = READ_UNALIGNED_WORD_BIG_ENDIAN(&(PortInfo->RxBuffer[OBEX_PACKET_LENGTH_OFFSET]));

                              /* Check to see if this packet header     */
                              /* appears valid.  There are only a       */
                              /* specific number of valid               */
                              /* Commands/Response OpCode values.       */
                              if(PortInfo->Server)
                                 Length = GetMatchIndex(PortInfo->RxBuffer[0], ServerCmdOpCodeList);
                              else
                                 Length = GetMatchIndex((PortInfo->RxBuffer[0] & OBEX_OPCODE_RESPONSE_BIT_MASK), ClientRspOpCodeList);

                              /* Check to see if a match was located or */
                              /* the packet length is not an acceptable */
                              /* size.                                  */
                              if((Length < 0) || (PortInfo->PacketLength < MINIMUM_OBEX_PACKET_SIZE) || (PortInfo->PacketLength > PortInfo->MaxPacketLength))
                              {
                                 /* We did not find a match or the      */
                                 /* packet length is not valid.  Since  */
                                 /* this is a reliable channel, the     */
                                 /* cause of this is either an internal */
                                 /* memory error or a possible attack.  */
                                 /* This error is considered            */
                                 /* non-recoverable, so we should       */
                                 /* terminate this connection.          */
                                 SPPM_ClosePort(PortInfo->SPPPortHandle, SPPM_CLOSE_DATA_FLUSH_TIMEOUT_IMMEDIATE);

                                 /* Set the Port to the Idle State.                 */
                                 PortInfo->PortState                                            = psIdle;

                                 /* Format the Event Data structure with information*/
                                 /* about the Connect Result.                       */
                                 EventData.Event_Data_Type                                     = etOBEX_Port_Close_Indication;
                                 EventData.Event_Data_Size                                     = (Word_t)OBEX_PORT_CLOSE_INDICATION_DATA_SIZE;
                                 EventData.Event_Data.OBEX_Port_Close_Indication_Data          = &ClosePortIndicationData;

                                 EventData.Event_Data.OBEX_Port_Close_Indication_Data->GOEP_ID = (unsigned int)CallbackParameter;

                                 /* Finally make the callback.                      */
                                 __BTPSTRY
                                 {
                                    (*PortInfo->GOEPM_EventCallback)(&EventData, PortInfo->CallbackParameter);
                                 }
                                 __BTPSEXCEPT(1)
                                 {
                                    /* Do Nothing.                                  */
                                 }

                                 /* In case it was deleted.                         */
                                 PortInfo = SearchGOEPInfoEntry(&PortInfoList, (unsigned int)CallbackParameter);
                                 if(PortInfo)
                                 {
                                    /* Cleanup the connection and free any */
                                    /* mamort that needs to be released.   */
                                    if(PortInfo->Server)
                                    {
                                       /* Since we are a server, then the  */
                                       /* port must be set back to a ready */
                                       /* state and wait for another       */
                                       /* connection.  Clear any           */
                                       /* information that pertains        */
                                       /* specifically to the connection   */
                                       /* that was just terminated.        */
                                       PortInfo->TxBufferNdx        = 0;
                                       PortInfo->RxBufferNdx        = 0;
                                       PortInfo->OutMaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;
                                       PortInfo->PendingCommand     = 0;
                                       PortInfo->PendingResponse    = OBEX_INVALID_COMMAND;
                                       PortInfo->PacketLength       = 0;
                                       PortInfo->PortState          = psIdle;
                                    }
                                    else
                                    {
                                       /* Since this is not a Server       */
                                       /* Connection, then remove all      */
                                       /* information that pertains to the */
                                       /* connection that was just         */
                                       /* terminated from the port list.   */
                                       DeleteGOEPInfoEntry(&PortInfoList, PortInfo->GOEP_ID);

                                       FreeGOEPInfoEntryMemory(PortInfo);
                                    }
                                 }
                                 break;
                              }
                           }
                           else
                           {
                              /* We still do not have enough data to    */
                              /* obtain the length.  There is a         */
                              /* possability that more data was received*/
                              /* while we were processing this data.  If*/
                              /* the last read returned some data, then */
                              /* continue to read from the buffer until */
                              /* no data is read.                       */
                              continue;
                           }
                        }

                        /* Determine the number of Bytes remaining to   */
                        /* complete the OBEX Packet.  Attempt to read   */
                        /* enough bytes to complete the packet.         */
                        if((int)PortInfo->PacketLength > PortInfo->RxBufferNdx)
                        {
                           /* Calculate the remaining bytes.            */
                           BytesToRead = (Word_t)((int)PortInfo->PacketLength - PortInfo->RxBufferNdx);

                           Length = SPPM_ReadData(PortInfo->SPPPortHandle, SPPM_READ_DATA_READ_TIMEOUT_IMMEDIATE, BytesToRead, &(PortInfo->RxBuffer[(PortInfo->RxBufferOverrun)?0:PortInfo->RxBufferNdx]));

                           /* Check to see if there was an error on the */
                           /* read or if no data was read.              */
                           if(Length <= 0)
                              break;

                           /* Update the Index by the amount of bytes   */
                           /* that were successfully read.              */
                           PortInfo->RxBufferNdx += Length;
                        }

                        /* If we have read an entire Packet, then       */
                        /* process the packet.                          */
                        if(PortInfo->RxBufferNdx == PortInfo->PacketLength)
                        {
                           /* The entire packet has been received, so   */
                           /* process the packet.                       */
                           ProcessDataFrame(PortInfo);

                           /* Since responding to the command may    */
                           /* have allowed a user to close this port,*/
                           /* we need to make sure this pointer is   */
                           /* still valid.                           */
                           PortInfo = SearchGOEPInfoEntry(&PortInfoList, (unsigned int)CallbackParameter);
                           if(PortInfo)
                           {
                              /* Now that this packet has been          */
                              /* processed, reset the Index and Length  */
                              /* information in preparation for the next*/
                              /* Packet.                                */
                              PortInfo->PacketLength = 0;
                              PortInfo->RxBufferNdx  = 0;
                           }
                           else
                           {
                              /* The Port must have been closed on   */
                              /* us.  So, we must exit.              */
                              break;
                           }
                        }
                     }
                     break;
                  default:
                     /* Do nothing                                      */
                     break;
               }
            }
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit (GOEPM):\n"));
}

   /* The following function is responsible for establishing a GOEP Port*/
   /* Server (will wait for a connection to occur on the port           */
   /* established by this function).  This function accepts as input the*/
   /* the Port Number to establish.  The second parameter are the       */
   /* optional PortFlags to use when opening the corresponding SPPM     */
   /* port.  The function also takes as a parameter the Maximum Packet  */
   /* Length that will be accepted for this server.  If the value       */
   /* supplied is outside the valid range, then the value used will be  */
   /* the valid value closest to the value supplied.  The last two      */
   /* parameters specify the GOEP Event Callback function and Callback  */
   /* Parameter, respectively, that will be called with GOEP Events that*/
   /* occur on the specified GOEP Port.  This function returns a        */
   /* non-zero, positive, number on success or a negative return error  */
   /* code if an error occurred (see BTERRORS.H).  A successful return  */
   /* code will be a GOEP Port ID that can be used to reference the     */
   /* Opened GOEP Port in ALL other functions in this module (except the*/
   /* _GOEP_Open_Remote_Port() function).  Once a Server GOEP Port is   */
   /* opened, it can only be Un-Registered via a call to the            */
   /* _GOEP_Close_Server_Port() function (passing the return value from */
   /* this function).  The _GOEP_Close_Port() function can be used to   */
   /* Disconnect a Client from the Server Port (if one is connected, it */
   /* will NOT Un-Register the Server Port however.                     */
static int _GOEP_Open_Server_Port(unsigned int ServerPort, unsigned long PortFlags, Word_t MaxPacketLength, GOEPM_Event_Callback_t GOEPM_Event_Callback, unsigned long CallbackParameter)
{
   int           ret_val;
   GOEPM_Info_t  PortInfo;
   GOEPM_Info_t *PortInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((SPP_VALID_PORT_NUMBER(ServerPort)) && (GOEPM_Event_Callback))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            /* Verify that MaxPacketLength is within the valid range for*/
            /* this value.  If the value supplied is outside the valid  */
            /* range, then set the value to the closest valid value.    */
            if(MaxPacketLength < OBEX_PACKET_LENGTH_MINIMUM)
               MaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;

            if(MaxPacketLength > OBEX_PACKET_LENGTH_MAXIMUM)
               MaxPacketLength = OBEX_PACKET_LENGTH_MAXIMUM;

            /* Initialize the Port Information to a known state.        */
            BTPS_MemInitialize(&PortInfo, 0, sizeof(GOEPM_Info_t));

            /* Allocate a Buffer for Building and Processing OBEX       */
            /* packets that will be received.  Set the Transmit Buffer  */
            /* to Max Packet Length and add one extra byte on the       */
            /* Receiver to allow for a Null terminator to be placed at  */
            /* the end of each packet.  If the data is successfully     */
            /* allocated, then save the Server Parameters.              */
            PortInfo.RxBuffer = (Byte_t *)BTPS_AllocateMemory(MaxPacketLength+1);
            PortInfo.TxBuffer = (Byte_t *)BTPS_AllocateMemory(MaxPacketLength);

            if((PortInfo.RxBuffer) && (PortInfo.TxBuffer))
            {
               /* Save the operating parameters for this server.        */
               PortInfo.GOEP_ID             = GetNextGOEP_ID();
               PortInfo.OutMaxPacketLength  = OBEX_PACKET_LENGTH_MINIMUM;
               PortInfo.PendingResponse     = OBEX_INVALID_COMMAND;
               PortInfo.PortState           = psIdle;
               PortInfo.Server              = TRUE;
               PortInfo.PortNumber          = ServerPort;
               PortInfo.MaxPacketLength     = MaxPacketLength;
               PortInfo.GOEPM_EventCallback = GOEPM_Event_Callback;
               PortInfo.CallbackParameter   = CallbackParameter;

               if((PortInfoPtr = AddGOEPInfoEntry(&PortInfoList, &PortInfo)) != NULL)
               {
                  /* With the entry successfully added to the list,     */
                  /* attempt to Open the SPP Server Port.               */
                  ret_val = SPPM_RegisterServerPort(ServerPort, PortFlags, SPPM_Event_Callback, (void *)PortInfoPtr->GOEP_ID);
                  if(ret_val > 0)
                  {
                     /* If the port was successfully opened, then save  */
                     /* the reference to the SPP port and attempt to set*/
                     /* the Transmit and Receive buffers to the twice   */
                     /* the MaxPacketSize for the port.  This will be   */
                     /* enough to hold a Command Packet and Possible an */
                     /* Abort Packet.  If the SPP Buffers are already at*/
                     /* least twice the size as what we need, skip the  */
                     /* adjustment of the buffers.                      */
                     PortInfoPtr->SPPPortHandle  = (unsigned int)ret_val;

                     /* Calculate the Maximum Packet Length plus the    */
                     /* size of an abort packet.                        */
                     /* * NOTE * We are doing this to handle the worst  */
                     /*          case of an entire queued packet and an */
                     /*          Abort being sent.  We are not taking   */
                     /*          into account any headers that might be */
                     /*          sent, however, no Bluetopia profiles   */
                     /*          specify any headers with Abort requests*/
                     /*          at this time.                          */
                     MaxPacketLength     += (Word_t)MINIMUM_OBEX_ABORT_PACKET_SIZE;

                     if(MaxPacketLength > SPP_BUFFER_SIZE_MINIMUM)
                     {
                        ret_val = SPPM_ChangeBufferSize(PortInfoPtr->SPPPortHandle, MaxPacketLength, MaxPacketLength);

                        /* If the Buffers were successfully changed,    */
                        /* then load the GOEP_ID to reference this Port */
                        /* to be returned to the user.                  */
                        if(ret_val != 0)
                        {
                           /* If we can not chnage the buffer size, then*/
                           /* close the SPP Port.                       */
                           SPPM_UnRegisterServerPort(PortInfoPtr->SPPPortHandle);
                        }
                     }

                     /* If the Buffers were changed, the ret_val will be*/
                     /* zero.  The the buffers did not need to be       */
                     /* changed then ret_val will be the Port ID.       */
                     if(ret_val >= 0)
                     {
                        /* Return the new GOEP ID.                      */
                        ret_val = (int)PortInfoPtr->GOEP_ID;
                     }
                  }

                  /* Check to see of any errors occurred while opening  */
                  /* the Port.  If so, we must remove the information in*/
                  /* the List that references this port.                */
                  if(ret_val < 0)
                  {
                     if((PortInfoPtr = DeleteGOEPInfoEntry(&PortInfoList, PortInfoPtr->GOEP_ID)) != NULL)
                        FreeGOEPInfoEntryMemory(PortInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
            {
               if(PortInfo.TxBuffer)
                  BTPS_FreeMemory(PortInfo.TxBuffer);

               if(PortInfo.TxBuffer)
                  BTPS_FreeMemory(PortInfo.TxBuffer);

               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for Un-Registering an OBEX  */
   /* Port Server (which was Registered by a successful call to the     */
   /* _GOEP_Open_Server_Port() function).  This function accepts as     */
   /* input the GOEP_ID (returned from a successful call to             */
   /* _GOEP_Open_Server_Port()) of the Server Port to close.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.  Note that this function does NOT      */
   /* delete any SDP Service Record Handles.                            */
static int _GOEP_Close_Server_Port(unsigned int GOEP_ID)
{
   int           ret_val;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            /* Now let's search for the Specified GOEP Port and         */
            /* determine if it is a Client or a Server.                 */
            if((PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID)) != NULL)
            {
               /* Only Process this Server Port if it is a Server Port. */
               if(PortInfo->Server)
               {
                  /* OBEX Port specifies a Server Port so let's delete  */
                  /* it from the Port List.                             */
                  if((PortInfo = DeleteGOEPInfoEntry(&PortInfoList, GOEP_ID)) != NULL)
                  {
                     /* If a valid SPPPortID is assigned to the Server, */
                     /* then Close the SPP Server Port.                 */
                     if(PortInfo->SPPPortHandle)
                     {
                        /* Close the SPP Port.                          */
                        SPPM_UnRegisterServerPort(PortInfo->SPPPortHandle);
                        if(PortInfo->ServiceRecordHandle)
                        {
                           /* Delete the Service Record                 */
                           DEVM_DeleteServiceRecord((unsigned long)(PortInfo->ServiceRecordHandle));
                        }
                     }

                     /* Finally free the memory for the Entry itself.   */
                     FreeGOEPInfoEntryMemory(PortInfo);

                     /* Flag Success to the Caller.                     */
                     ret_val = 0;
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
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for responding to requests  */
   /* to connect to an OBEX Port Server.  This function accepts as input*/
   /* the GOEP Port ID (which *MUST* have been obtained by calling the  */
   /* _GOEP_Open_Server_Port() function), and as the final parameter    */
   /* whether to accept the pending connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
static int _GOEP_Open_Port_Request_Response(unsigned int GOEP_ID, Boolean_t AcceptConnection)
{
   int           ret_val;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is a server port and is      */
               /* currently opening.                                    */
               if((PortInfo->Server) && (PortInfo->PortState == psPortOpening))
               {
                  /* The port is a Server Port and is currently in the  */
                  /* Opening State.  Next simply submit the Open Port   */
                  /* Request Response to the lower layer.               */
                  if((ret_val = SPPM_OpenServerPortRequestResponse(PortInfo->SPPPortHandle, AcceptConnection)) == 0)
                  {
                     /* The Open Port Request Response was successfully */
                     /* submitted to the lower layer.  Next check to see*/
                     /* if this was a call to reject the connection.    */
                     if(!AcceptConnection)
                     {
                        /* The connection was reject, flag that we are  */
                        /* no longer connected.                         */
                        PortInfo->PortState = psIdle;
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is provided to allow a means to add a      */
   /* Generic OBEX Service Record to the SDP Database.  This function   */
   /* takes as input the GOEP Port ID (which *MUST* have been obtained  */
   /* by calling the _GOEP_Open_Server_Port() function.  The second     */
   /* parameter (required) specifies any additional SDP Information to  */
   /* add to the record.  The third parameter specifies the Service Name*/
   /* to associate with the SDP Record.  The final parameter is a       */
   /* pointer to a DWord_t which receives the SDP Service Record Handle */
   /* if this function successfully creates an SDP Service Record.  If  */
   /* this function returns zero, then the SDPServiceRecordHandle entry */
   /* will contain the Service Record Handle of the added SDP Service   */
   /* Record.  If this function fails, a negative return error code will*/
   /* be returned and the SDPServiceRecordHandle value will be          */
   /* undefined.                                                        */
   /* * NOTE * This function should only be called with the GOEP Port ID*/
   /*          that was returned from the _GOEP_Open_Server_Port()      */
   /*          function.  This function should NEVER be used with the   */
   /*          GOEP Port ID returned from the _GOEP_Open_Remote_Port()  */
   /*          function.                                                */
   /* * NOTE * There must be UUID Information specified in the          */
   /*          SDPServiceRecord Parameter, however protocol information */
   /*          is completely optional.  Any Protocol Information that is*/
   /*          specified (if any) will be added in the Protocol         */
   /*          Attribute AFTER the default OBEX Protocol List (L2CAP,   */
   /*          RFCOMM, and OBEX).                                       */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
static int _GOEP_Register_SDP_Record(unsigned int GOEP_ID, GOEP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle)
{
   int                 ret_val;
   long                Result;
   GOEPM_Info_t       *PortInformation;
   unsigned int        Index;
   unsigned int        tmpCount;
   SDP_Data_Element_t *SDP_Data_Element;
   SDP_Data_Element_t  SDP_Data_Element_Main;
   SDP_Data_Element_t  SDP_Data_Element_OBEX;
   SDP_Data_Element_t  SDP_Data_Element_L2CAP;
   SDP_Data_Element_t  SDP_Data_Element_RFCOMM[2];
   SDP_Data_Element_t  SDP_Data_Element_Language[4];

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((GOEP_ID) && (ServiceName) && (BTPS_StringLength(ServiceName)) && (SDPServiceRecord) && (SDPServiceRecordHandle))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            /* Now let's find the OBEX Server Port based upon the ID    */
            /* that was specified.                                      */
            if((PortInformation = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID)) != NULL)
            {
               /* Now let's Add a generic OBEX SDP Record to the SDP    */
               /* Database.                                             */

               /* Now check to see if the caller specified information  */
               /* for the Service Class.                                */
               if((SDPServiceRecord) && (SDPServiceRecord->NumberServiceClassUUID))
               {
                  /* UUID Service Record Information was specified, so  */
                  /* let's add the OBEX SDP Record to the SDP Database  */
                  /* letting SPP do most of the grunt work for us.      */

                  /* Calculate how many extra Protocol List Entries the */
                  /* User has requested to be added to the Protocol     */
                  /* List.                                              */
                  if((SDPServiceRecord) && (SDPServiceRecord->ProtocolList))
                  {
                     if(SDPServiceRecord->ProtocolList[0].SDP_Data_Element_Type == deSequence)
                        tmpCount = 3 + SDPServiceRecord->ProtocolList[0].SDP_Data_Element_Length;
                     else
                        tmpCount = 0;
                  }
                  else
                     tmpCount = 3;

                  /* Make sure that an error hasn't occurred.           */
                  if(tmpCount)
                  {
                     /* Go ahead and create the SDP Service Record with */
                     /* the specified Service Class Information.        */
                     Result = DEVM_CreateServiceRecord(TRUE, SDPServiceRecord->NumberServiceClassUUID, SDPServiceRecord->SDPUUIDEntries);
                     if(Result>=0)
                     {
                        /* Save the SDP Record Handle that is returned. */
                        *SDPServiceRecordHandle               = Result;
                         PortInformation->ServiceRecordHandle = Result;

                        /* Now allocate enough space to hold the SDP    */
                        /* Data Element Sequence List.                  */
                        if((SDP_Data_Element = (SDP_Data_Element_t *)BTPS_AllocateMemory(tmpCount*SDP_DATA_ELEMENT_SIZE)) != NULL)
                        {
                           /* Format the L2CAP Data Element Sequence.   */
                           SDP_Data_Element[0].SDP_Data_Element_Type                      = deSequence;
                           SDP_Data_Element[0].SDP_Data_Element_Length                    = 1;
                           SDP_Data_Element[0].SDP_Data_Element.SDP_Data_Element_Sequence = &SDP_Data_Element_L2CAP;

                           SDP_Data_Element_L2CAP.SDP_Data_Element_Type                   = deUUID_16;
                           SDP_Data_Element_L2CAP.SDP_Data_Element_Length                 = sizeof(SDP_Data_Element_L2CAP.SDP_Data_Element.UUID_16);
                           SDP_ASSIGN_L2CAP_UUID_16(SDP_Data_Element_L2CAP.SDP_Data_Element.UUID_16);

                           /* Format the RFCOMM Data Element Sequence.  */
                           SDP_Data_Element[1].SDP_Data_Element_Type                        = deSequence;
                           SDP_Data_Element[1].SDP_Data_Element_Length                      = 2;
                           SDP_Data_Element[1].SDP_Data_Element.SDP_Data_Element_Sequence   = SDP_Data_Element_RFCOMM;

                           SDP_Data_Element_RFCOMM[0].SDP_Data_Element_Type                 = deUUID_16;
                           SDP_Data_Element_RFCOMM[0].SDP_Data_Element_Length               = UUID_16_SIZE;
                           SDP_ASSIGN_RFCOMM_UUID_16(SDP_Data_Element_RFCOMM[0].SDP_Data_Element.UUID_16);

                           SDP_Data_Element_RFCOMM[1].SDP_Data_Element_Type                 = deUnsignedInteger1Byte;
                           SDP_Data_Element_RFCOMM[1].SDP_Data_Element_Length               = sizeof(SDP_Data_Element_RFCOMM[1].SDP_Data_Element.UnsignedInteger1Byte);
                           SDP_Data_Element_RFCOMM[1].SDP_Data_Element.UnsignedInteger1Byte = (Byte_t)PortInformation->PortNumber;

                           /* Format the OBEX Data Element Sequence.    */
                           SDP_Data_Element[2].SDP_Data_Element_Type                      = deSequence;
                           SDP_Data_Element[2].SDP_Data_Element_Length                    = 1;
                           SDP_Data_Element[2].SDP_Data_Element.SDP_Data_Element_Sequence = &SDP_Data_Element_OBEX;

                           SDP_Data_Element_OBEX.SDP_Data_Element_Type                    = deUUID_16;
                           SDP_Data_Element_OBEX.SDP_Data_Element_Length                  = sizeof(SDP_Data_Element_OBEX.SDP_Data_Element.UUID_16);
                           SDP_ASSIGN_OBEX_UUID_16(SDP_Data_Element_OBEX.SDP_Data_Element.UUID_16);

                           /* Now let's add any Information that the    */
                           /* caller has specified.                     */
                           for(Index=0;Index<(tmpCount-3);Index++)
                              SDP_Data_Element[3+Index] = SDPServiceRecord->ProtocolList[0].SDP_Data_Element.SDP_Data_Element_Sequence[Index];

                           /* Format the Main SDP Data Element Sequence.*/
                           SDP_Data_Element_Main.SDP_Data_Element_Type                      = deSequence;
                           SDP_Data_Element_Main.SDP_Data_Element_Length                    = tmpCount;
                           SDP_Data_Element_Main.SDP_Data_Element.SDP_Data_Element_Sequence = SDP_Data_Element;

                           /* Now lets add the necessary Protocol       */
                           /* Descriptor List.                          */
                           ret_val = DEVM_AddServiceRecordAttribute((unsigned long)(*SDPServiceRecordHandle), SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST, &SDP_Data_Element_Main);
                           if(!ret_val)
                           {
                              /* Add a default Language Attribute ID    */
                              /* List Entry for UTF-8 English.          */
                              SDP_Data_Element_Language[0].SDP_Data_Element_Type                      = deSequence;
                              SDP_Data_Element_Language[0].SDP_Data_Element_Length                    = 3;
                              SDP_Data_Element_Language[0].SDP_Data_Element.SDP_Data_Element_Sequence = &(SDP_Data_Element_Language[1]);
                              SDP_Data_Element_Language[1].SDP_Data_Element_Type                      = deUnsignedInteger2Bytes;
                              SDP_Data_Element_Language[1].SDP_Data_Element_Length                    = SDP_UNSIGNED_INTEGER_2_BYTES_SIZE;
                              SDP_Data_Element_Language[1].SDP_Data_Element.UnsignedInteger2Bytes     = SDP_NATURAL_LANGUAGE_ENGLISH_UTF_8;
                              SDP_Data_Element_Language[2].SDP_Data_Element_Type                      = deUnsignedInteger2Bytes;
                              SDP_Data_Element_Language[2].SDP_Data_Element_Length                    = SDP_UNSIGNED_INTEGER_2_BYTES_SIZE;
                              SDP_Data_Element_Language[2].SDP_Data_Element.UnsignedInteger2Bytes     = SDP_UTF_8_CHARACTER_ENCODING;
                              SDP_Data_Element_Language[3].SDP_Data_Element_Type                      = deUnsignedInteger2Bytes;
                              SDP_Data_Element_Language[3].SDP_Data_Element_Length                    = SDP_UNSIGNED_INTEGER_2_BYTES_SIZE;
                              SDP_Data_Element_Language[3].SDP_Data_Element.UnsignedInteger2Bytes     = SDP_DEFAULT_LANGUAGE_BASE_ATTRIBUTE_ID;

                              ret_val = DEVM_AddServiceRecordAttribute((unsigned long)(*SDPServiceRecordHandle), SDP_ATTRIBUTE_ID_LANGUAGE_BASE_ATTRIBUTE_ID_LIST, SDP_Data_Element_Language);
                              if(!ret_val)
                              {
                                 /* Finally Add the Service Name to the */
                                 /* SDP Database.                       */
                                 SDP_Data_Element_Main.SDP_Data_Element_Type       = deTextString;
                                 SDP_Data_Element_Main.SDP_Data_Element_Length     = BTPS_StringLength(ServiceName);
                                 SDP_Data_Element_Main.SDP_Data_Element.TextString = (Byte_t *)ServiceName;

                                 ret_val = DEVM_AddServiceRecordAttribute((unsigned long)(*SDPServiceRecordHandle), (SDP_DEFAULT_LANGUAGE_BASE_ATTRIBUTE_ID + SDP_ATTRIBUTE_OFFSET_ID_SERVICE_NAME), &SDP_Data_Element_Main);
                              }
                           }
                           /* Free any memory that was allocated.       */
                           BTPS_FreeMemory(SDP_Data_Element);
                        }
                        else
                           ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;

                        /* If an error occurred go ahead and un-register*/
                        /* the SDP Record.                              */
                        if(ret_val)
                        {
                           DEVM_DeleteServiceRecord((unsigned long)(*SDPServiceRecordHandle));
                           PortInformation->ServiceRecordHandle = 0;
                        }
                     }
                     else
                        ret_val = (int)Result;
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
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for Opening a Remote GOEP   */
   /* Port on the specified Remote Device.  This function accepts the   */
   /* the Board Address (NON NULL) of the Remote Bluetooth Device to    */
   /* connect with.  The next parameter specifies the Remote Server     */
   /* Channel ID to connect.  The third parameter specifies the optional*/
   /* flags to use when opening the port.  The fourth parameter specfies*/
   /* the Maximum OBEX Packet size that will be used by this client.  If*/
   /* the supplied value is not acceptable to the remote server, then   */
   /* the closest legal value will be used instead.  The final two      */
   /* parameters specify the GOEP Event Callback function, and callback */
   /* parameter, respectively, of the GOEP Event Callback that is to    */
   /* process any further interaction with the specified Remote Port    */
   /* (Opening Status, Data Writes, etc).  This function returns a      */
   /* non-zero, positive, value if successful, or a negative return     */
   /* error code if this function is unsuccessful.  If this function is */
   /* successful, the return value will represent the GOEP Port ID that */
   /* can be passed to all other functions that require it.  Once a GOEP*/
   /* Port is opened, it can only be closed via a call to the           */
   /* _GOEP_Close_Port() function (passing the return value from this   */
   /* function).                                                        */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
static int _GOEP_Open_Remote_Port(BD_ADDR_t BD_ADDR, unsigned int ServerPort, unsigned long OpenFlags, Word_t MaxPacketLength, GOEPM_Event_Callback_t GOEP_Event_Callback, unsigned long CallbackParameter, unsigned int *ConnectionStatus)
{
   int           ret_val;
   GOEPM_Info_t  PortInfo;
   GOEPM_Info_t *PortInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((SPP_VALID_PORT_NUMBER(ServerPort)) && (GOEP_Event_Callback) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            /* Verify that MaxPacketLength is within the valid range for*/
            /* this value.  If the value supplied is outside the valid  */
            /* range, then set the value to the closest valid value.    */
            if(MaxPacketLength < OBEX_PACKET_LENGTH_MINIMUM)
               MaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;

            if(MaxPacketLength > OBEX_PACKET_LENGTH_MAXIMUM)
               MaxPacketLength = OBEX_PACKET_LENGTH_MAXIMUM;

            /* Initialize the Port Information to a known state.        */
            BTPS_MemInitialize(&PortInfo, 0, sizeof(GOEPM_Info_t));

            /* Initialize the Port Information Structure to default     */
            /* values.  Set the Transmit Buffer to Max Packet Length and*/
            /* add one extra byte on the Receiver to allow for a Null   */
            /* terminator to be placed at the end of each packet.       */
            PortInfo.RxBuffer = (Byte_t *)BTPS_AllocateMemory(MaxPacketLength+1);
            PortInfo.TxBuffer = (Byte_t *)BTPS_AllocateMemory(MaxPacketLength);

            if((PortInfo.RxBuffer) && (PortInfo.TxBuffer))
            {
               /* Save the operating parameters for this connection     */
               PortInfo.GOEP_ID            = GetNextGOEP_ID();
               PortInfo.PortState          = psPortOpening;
               PortInfo.PortNumber         = ServerPort;
               PortInfo.OutMaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;
               PortInfo.PendingResponse    = OBEX_INVALID_COMMAND;
               PortInfo.MaxPacketLength    = MaxPacketLength;
               PortInfo.CallbackParameter  = CallbackParameter;
               PortInfo.GOEPM_EventCallback = GOEP_Event_Callback;

               if((PortInfoPtr = AddGOEPInfoEntry(&PortInfoList, &PortInfo)) != NULL)
               {
                  /* With the entry successfully added to the list,     */
                  /* attempt to Open the SPP Port to the Remote Server. */
                  ret_val = SPPM_OpenRemotePort(BD_ADDR, ServerPort, OpenFlags, SPPM_Event_Callback, (void *)PortInfoPtr->GOEP_ID, ConnectionStatus);
                  if(ret_val > 0)
                  {
                     /* If the port was successfully opened, then save  */
                     /* the reference to the SPP port and attempt to set*/
                     /* the Transmit and Receive buffers to the twice   */
                     /* the MaxPacketSize for the port.  This will be   */
                     /* enough to hold a Command Packet and Possible an */
                     /* Abort Packet.  If the SPP Buffers are already at*/
                     /* least twice the size as what we need, skip the  */
                     /* adjustment of the buffers.                      */
                     PortInfoPtr->SPPPortHandle  = (unsigned int)ret_val;

                     /* Calculate the Maximum Packet Length plus the    */
                     /* size of an abort packet.                        */
                     /* * NOTE * We are doing this to handle the worst  */
                     /*          case of an entire queued packet and an */
                     /*          Abort being sent.  We are not taking   */
                     /*          into account any headers that might be */
                     /*          sent, however, no Bluetopia profiles   */
                     /*          specify any headers with Abort requests*/
                     /*          at this time.                          */
                     MaxPacketLength     += (Word_t)MINIMUM_OBEX_ABORT_PACKET_SIZE;

                     if(MaxPacketLength > SPP_BUFFER_SIZE_MINIMUM)
                     {
                        ret_val = SPPM_ChangeBufferSize(PortInfoPtr->SPPPortHandle, MaxPacketLength, MaxPacketLength);

                        /* Check to see if the buffer was succecssfull. */
                        /* If not we will change the buffer when the    */
                        /* open confirmation is received.               */
                        if(!ret_val)
                           PortInfoPtr->QueuedPacketLength = 0;
                        else
                        {
                           PortInfoPtr->QueuedPacketLength = MaxPacketLength;

                           ret_val                         = 0;
                        }
                     }

                     /* If the Buffers were changed, the ret_val will be*/
                     /* zero.  The the buffers did not need to be       */
                     /* changed then ret_val will be the Port ID.       */
                     if(ret_val >= 0)
                     {
                        /* Return the new GOEP ID.                      */
                        ret_val = (int)PortInfoPtr->GOEP_ID;
                     }
                  }

                  /* Check to see of any errors occurred while opening  */
                  /* the Port.  If so, we must remove the information in*/
                  /* the List that references this port.                */
                  if(ret_val < 0)
                  {
                     if((PortInfoPtr = DeleteGOEPInfoEntry(&PortInfoList, PortInfoPtr->GOEP_ID)) != NULL)
                        FreeGOEPInfoEntryMemory(PortInfoPtr);
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
            else
            {
               /* Free the Buffer allocated for OBEX Packets.           */
               if(PortInfo.TxBuffer)
                  BTPS_FreeMemory(PortInfo.TxBuffer);

               if(PortInfo.RxBuffer)
                  BTPS_FreeMemory(PortInfo.RxBuffer);

               ret_val = BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to close an OBEX Port that was      */
   /* previously opened with the _GOEP_Open_Server_Port() function OR   */
   /* the _GOEP_Open_Remote_Port() function.  This function accepts as  */
   /* input the Port ID (return value from one of the above mentioned   */
   /* Open functions) of the Port to Close.  This function returns zero */
   /* if successful, or a negative return value if there was an error.  */
   /* This function does NOT Un-Register a OBEX Server Port from the    */
   /* system, it ONLY disconnects any connection that is currently      */
   /* active on the Server Port.  The _GOEP_Close_Server_Port() function*/
   /* can be used to Un-Register the GOEP Server Port.                  */
static int _GOEP_Close_Port(unsigned int GOEP_ID)
{
   int           ret_val;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               ret_val = SPPM_ClosePort(PortInfo->SPPPortHandle, SPPM_CLOSE_DATA_FLUSH_TIMEOUT_IMMEDIATE);

               if(PortInfo->Server)
               {
                  /* Reset the information for the server to their      */
                  /* default values.                                    */
                  PortInfo->RxBufferNdx        = 0;
                  PortInfo->TxBufferNdx        = 0;
                  PortInfo->OutMaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;
                  PortInfo->PacketLength       = 0;
                  PortInfo->PendingCommand     = 0;
                  PortInfo->PendingResponse    = OBEX_INVALID_COMMAND;
                  PortInfo->PortState          = psIdle;
               }
               else
               {
                  /* Finally remove the Client Information from the List*/
                  /* an free the memory for the Entry itself.           */
                  if((PortInfo = DeleteGOEPInfoEntry(&PortInfoList, GOEP_ID)) != NULL)
                     FreeGOEPInfoEntryMemory(PortInfo);
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Connect Request to  */
   /* the Remote OBEX entity.  The OBEX entity is referenced by the GOEP*/
   /* ID that was returned from an OBEX_Open_Remote_Port or and         */
   /* OBEX_Open_Server_Port.  This function accepts as input the the    */
   /* Port Identifier that was returned from the Open Port function, and*/
   /* a pointer to an array of Optional Headers to be sent with the     */
   /* Connect Request.  This function returns zero if successful, or a  */
   /* negative return value if there was an error.                      */
static int _GOEP_Connect_Request(unsigned int GOEP_ID, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is in the Open State.        */
               if(PortInfo->PortState == psPortOpened)
               {
                  PortInfo->TxBuffer[OBEX_COMMAND_OFFSET]        = (OBEX_CONNECT_COMMAND | OBEX_FINAL_BIT);
                  PortInfo->TxBuffer[OBEX_VERSION_NUMBER_OFFSET] = OBEX_VERSION_1_0;
                  PortInfo->TxBuffer[OBEX_CONNECT_FLAGS_OFFSET]  = 0x00;
                  ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_MAX_PACKET_LENGTH_OFFSET]), PortInfo->MaxPacketLength);
                  PortInfo->TxBufferNdx                          = OBEX_CONNECT_PACKET_HEADER_OFFSET;

                  if(Header_List)
                  {
                     /* Add the Optional Header information to the      */
                     /* buffer.  If there is not enough buffer space to */
                     /* contain all of the header information, the      */
                     /* function will return an error.                  */
                     ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
                  }

                  /* If there was no problem adding the header          */
                  /* information then try to send to data.              */
                  if(ret_val == 0)
                  {
                     /* Add the size of the Packet to the OBEX Connect  */
                     /* Packet.  The number of bytes that is contained  */
                     /* in the Packet is equal to the Buffer Index      */
                     /* value.                                          */
                     ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                     /* Send the data packet to SPP for transmission to */
                     /* the remote device.  If all of the data was not  */
                     /* accepted, return an error.                      */
                     if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) != (Word_t)PortInfo->TxBufferNdx)
                        ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
                     else
                     {
                        /* Set Port state to Connecting.                */
                        PortInfo->PortState = psConnecting;

                        /* Note that the Connect Command is outstanding.*/
                        PortInfo->PendingCommand = OBEX_CONNECT_COMMAND;
                     }
                  }
               }
               else
               {
                  /* The Connect command is only allowed when the port  */
                  /* is in the PortOpened state.                        */
                  ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Disconnect Request  */
   /* to the Remote OBEX entity.  The OBEX entity is referenced by the  */
   /* GOEP_ID that was returned from an GOEP_Open_Remote_Port or and    */
   /* GOEP_Open_Server_Port.  This function accepts as input the Port   */
   /* Identifier that was returned from the Open Port function, and a   */
   /* pointer to an array of Optional Headers to be sent with the       */
   /* Disconnect Request.  This function returns zero if successful, or */
   /* a negative return value if there was an error.                    */
static int _GOEP_Disconnect_Request(unsigned int GOEP_ID, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is in the Connected State.   */
               if(PortInfo->PortState == psConnected)
               {
                  PortInfo->TxBuffer[OBEX_COMMAND_OFFSET] = (OBEX_DISCONNECT_COMMAND | OBEX_FINAL_BIT);
                  PortInfo->TxBufferNdx                   = OBEX_DISCONNECT_PACKET_HEADER_OFFSET;

                  if(Header_List)
                  {
                     /* Add the Optional Header information to the      */
                     /* buffer.  If there is not enough buffer space to */
                     /* contain all of the header information, the      */
                     /* function will return an error.                  */
                     ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
                  }

                  /* If there was no problem adding the header          */
                  /* information then try to send to data.              */
                  if(ret_val == 0)
                  {
                     /* Add the size of the Packet to the OBEX Connect  */
                     /* Packet.  The number of bytes that is contained  */
                     /* in the Packet is equal to the Buffer Index      */
                     /* value.                                          */
                     ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                     /* Send the data packet to SPP for transmission to */
                     /* the remote device.  If all of the data was not  */
                     /* accepted, return an error.                      */
                     if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) == (Word_t)PortInfo->TxBufferNdx)
                     {
                        /* Set Port state to Disconnecting.             */
                        PortInfo->PortState = psDisconnecting;

                        /* Note that the Disconnect Command is          */
                        /* outstanding.                                 */
                        PortInfo->PendingCommand = OBEX_DISCONNECT_COMMAND;
                     }
                     else
                        ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
                  }
               }
               else
               {
                  /* The Disconnect command is only allowed when the    */
                  /* port is in the connected state.                    */
                  ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Put Request to the  */
   /* Remote OBEX entity.  The OBEX entity is referenced by the GOEP_ID */
   /* that was returned from an OBEX_Open_Remote_Port or and            */
   /* OBEX_Open_Server_Port.  This function accepts as input the Port   */
   /* Identifier that was returned from the Open Port function, a Final */
   /* flag to denote if this is the last packet of the Put sequence, and*/
   /* a pointer to an array of Optional Headers to be sent with the Put */
   /* Request.  Note that the body of the object is contained in the    */
   /* Header List.  This function returns zero if successful, or a      */
   /* negative return value if there was an error.                      */
static int _GOEP_Put_Request(unsigned int GOEP_ID, Boolean_t Final, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is in the Connected State.   */
               if(PortInfo->PortState == psConnected)
               {
                  PortInfo->TxBuffer[OBEX_COMMAND_OFFSET] = (Byte_t)(OBEX_PUT_COMMAND | (Final?OBEX_FINAL_BIT:0));
                  PortInfo->TxBufferNdx                   = OBEX_PUT_PACKET_HEADER_OFFSET;

                  if(Header_List)
                  {
                     /* Add the Optional Header information to the      */
                     /* buffer.  If there is not enough buffer space to */
                     /* contain all of the header information, the      */
                     /* function will return an error.                  */
                     ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
                  }

                  /* If there was no problem adding the header          */
                  /* information then try to send to data.              */
                  if(ret_val == 0)
                  {
                     /* Add the size of the Packet to the OBEX Put      */
                     /* Packet.  The number of bytes that is contained  */
                     /* in the Packet is equal to the Buffer Index      */
                     /* value.                                          */
                     ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                     /* Send the data packet to SPP for transmission to */
                     /* the remote device.  If all of the data was not  */
                     /* accepted, return an error.                      */
                     if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) == (Word_t)PortInfo->TxBufferNdx)
                     {
                        /* Note that the Put Command is outstanding.    */
                        PortInfo->PendingCommand = OBEX_PUT_COMMAND;
                     }
                     else
                        ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
                  }
               }
               else
               {
                  /* The Put command is only allowed when the port is in*/
                  /* the connected state.                               */
                  ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Get Request to the  */
   /* Remote OBEX entity.  The OBEX entity is referenced by the GOEP_ID */
   /* that was returned from an GOEP_Open_Remote_Port or and            */
   /* GOEP_Open_Server_Port.  This function accepts as input the Port   */
   /* Identifier that was returned from the Open Port function, a Final */
   /* flag to denote when the Server should begin to send the Object,   */
   /* and a pointer to an array of Optional Headers to be sent with the */
   /* Get Request.  This function returns zero if successful, or a      */
   /* negative return value if there was an error.                      */
static int _GOEP_Get_Request(unsigned int GOEP_ID, Boolean_t Final, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is in the Connected State.   */
               if(PortInfo->PortState == psConnected)
               {
                  PortInfo->TxBuffer[OBEX_COMMAND_OFFSET] = (Byte_t)(OBEX_GET_COMMAND | (Final?OBEX_FINAL_BIT:0));
                  PortInfo->TxBufferNdx                   = OBEX_GET_PACKET_HEADER_OFFSET;

                  if(Header_List)
                  {
                     /* Add the Optional Header information to the      */
                     /* buffer.  If there is not enough buffer space to */
                     /* contain all of the header information, the      */
                     /* function will return an error.                  */
                     ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
                  }

                  /* If there was no problem adding the header          */
                  /* information then try to send to data.              */
                  if(ret_val == 0)
                  {
                     /* Add the size of the Packet to the OBEX Get      */
                     /* Packet.  The number of bytes that is contained  */
                     /* in the Packet is equal to the Buffer Index      */
                     /* value.                                          */
                     ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                     /* Send the data packet to SPP for transmission to */
                     /* the remote device.  If all of the data was not  */
                     /* accepted, return an error.                      */
                     if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) == (Word_t)PortInfo->TxBufferNdx)
                     {
                        /* Note that the Get Command is outstanding.    */
                        PortInfo->PendingCommand = OBEX_GET_COMMAND;
                     }
                     else
                        ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
                  }
               }
               else
               {
                  /* The Get command is only allowed when the port is in*/
                  /* the connected state.                               */
                  ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Set Path Request to */
   /* the Remote OBEX entity.  The OBEX entity is referenced by the     */
   /* GOEP_ID that was returned from an GOEP_Open_Remote_Port or and    */
   /* GOEP_Open_Server_Port.  This function accepts as input the Port   */
   /* Identifier that was returned from the Open Port function, a Flags */
   /* value that contains bit flags that control Directory navigation   */
   /* and Directory Creation, and a pointer to an array of Optional     */
   /* Headers to be sent with the Set Path Request.  This function      */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
static int _GOEP_Set_Path_Request(unsigned int GOEP_ID, Byte_t Flags, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is in the Connected State.   */
               if(PortInfo->PortState == psConnected)
               {
                  PortInfo->TxBuffer[OBEX_COMMAND_OFFSET]            = (Byte_t)(OBEX_SET_PATH_COMMAND | OBEX_FINAL_BIT);
                  PortInfo->TxBuffer[OBEX_SET_PATH_FLAGS_OFFSET]     = Flags;
                  PortInfo->TxBuffer[OBEX_SET_PATH_CONSTANTS_OFFSET] = 0x00;
                  PortInfo->TxBufferNdx                              = OBEX_SET_PATH_PACKET_HEADER_OFFSET;

                  if(Header_List)
                  {
                     /* Add the Optional Header information to the      */
                     /* buffer.  If there is not enough buffer space to */
                     /* contain all of the header information, the      */
                     /* function will return an error.                  */
                     ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
                  }

                  /* If there was no problem adding the header          */
                  /* information then try to send to data.              */
                  if(ret_val == 0)
                  {
                     /* Add the size of the Packet to the OBEX Set Path */
                     /* Packet.  The number of bytes that is contained  */
                     /* in the Packet is equal to the Buffer Index      */
                     /* value.                                          */
                     ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                     /* Send the data packet to SPP for transmission to */
                     /* the remote device.  If all of the data was not  */
                     /* accepted, return an error.                      */
                     if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) == (Word_t)PortInfo->TxBufferNdx)
                     {
                        /* Note that the Set Path is outstanding.       */
                        PortInfo->PendingCommand = OBEX_SET_PATH_COMMAND;
                     }
                     else
                        ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
                  }
               }
               else
               {
                  /* The Set Path command is only allowed when the port */
                  /* is in the connected state.                         */
                  ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Abort Request to the*/
   /* Remote OBEX entity.  The OBEX entity is referenced by the GOEP_ID */
   /* that was returned from an GOEP_Open_Remote_Port or and            */
   /* GOEP_Open_Server_Port.  This function accepts as input the the    */
   /* Port Identifier that was returned from the Open Port function, and*/
   /* a pointer to an array of Optional Headers to be sent with the Get */
   /* Request.  This function returns zero if successful, or a negative */
   /* return value if there was an error.                               */
static int _GOEP_Abort_Request(unsigned int GOEP_ID, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               /* Check to see if the Port is in the Connected State.   */
               if(PortInfo->PortState == psConnected)
               {
                  PortInfo->TxBuffer[OBEX_COMMAND_OFFSET] = (Byte_t)(OBEX_ABORT_COMMAND | OBEX_FINAL_BIT);
                  PortInfo->TxBufferNdx                   = OBEX_ABORT_PACKET_HEADER_OFFSET;

                  if(Header_List)
                  {
                     /* Add the Optional Header information to the      */
                     /* buffer.  If there is not enough buffer space to */
                     /* contain all of the header information, the      */
                     /* function will return an error.                  */
                     ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
                  }

                  /* If there was no problem adding the header          */
                  /* information then try to send to data.              */
                  if(ret_val == 0)
                  {
                     /* Add the size of the Packet to the OBEX Get      */
                     /* Packet.  The number of bytes that is contained  */
                     /* in the Packet is equal to the Buffer Index      */
                     /* value.                                          */
                     ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                     /* Send the data packet to SPP for transmission to */
                     /* the remote device.  If all of the data was not  */
                     /* accepted, return an error.                      */
                     if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) == (Word_t)PortInfo->TxBufferNdx)
                     {
                        /* Note that the Abort Command is outstanding.  */
                        PortInfo->PendingCommand = OBEX_ABORT_COMMAND;
                     }
                     else
                        ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
                  }
               }
               else
               {
                  /* The Abort command is only allowed when the port is */
                  /* in the connected state.                            */
                  ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function exists to Send an OBEX Command Response to */
   /* the Remote OBEX entity.  The OBEX entity is referenced by the GOEP*/
   /* ID that was returned from an GOEP_Open_Remote_Port or and         */
   /* GOEP_Open_Server_Port.  This function accepts as input the the    */
   /* Port Identifier that was supplied with the Command Request, the   */
   /* Response Code for the command that is being responded to and a    */
   /* pointer to an array of Optional Headers to be sent with the       */
   /* Command Response.  This function returns zero if successful, or a */
   /* negative return value if there was an error.                      */
static int _GOEP_Command_Response(unsigned int GOEP_ID, Byte_t ResponseCode, OBEX_Header_List_t *Header_List)
{
   int           ret_val = 0;
   GOEPM_Info_t *PortInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module has been initialized.            */
         if(Initialized)
         {
            PortInfo = SearchGOEPInfoEntry(&PortInfoList, GOEP_ID);
            if(PortInfo)
            {
               switch(PortInfo->PendingResponse)
               {
                  case OBEX_CONNECT_COMMAND:
                     /* Check to see if the Port is in the connecting   */
                     /* state.                                          */
                     if(PortInfo->PortState == psConnecting)
                     {
                        /* If the Response Code is 'OK', then we can    */
                        /* move to the Connected State.  If not, then we*/
                        /* must return to the PortOpened State.         */
                        PortInfo->PortState = (ResponseCode == OBEX_OK_RESPONSE)?psConnected:psPortOpened;

                        PortInfo->TxBuffer[OBEX_COMMAND_OFFSET]        = (Byte_t)(ResponseCode | OBEX_FINAL_BIT);
                        PortInfo->TxBuffer[OBEX_VERSION_NUMBER_OFFSET] = OBEX_VERSION_1_0;
                        PortInfo->TxBuffer[OBEX_CONNECT_FLAGS_OFFSET]  = 0x00;
                        ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_MAX_PACKET_LENGTH_OFFSET]), PortInfo->MaxPacketLength);
                        PortInfo->TxBufferNdx                          = OBEX_CONNECT_PACKET_HEADER_OFFSET;
                     }
                     else
                     {
                        /* The Connect Response is only allowed when the*/
                        /* port is in the Connecting state.             */
                        ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
                     }
                     break;
                  case OBEX_DISCONNECT_COMMAND:
                     /* Check to see if the Port is in the Disconnecting*/
                     /* state.                                          */
                     if(PortInfo->PortState == psDisconnecting)
                     {
                        PortInfo->TxBuffer[OBEX_COMMAND_OFFSET] = (Byte_t)(ResponseCode | OBEX_FINAL_BIT);
                        PortInfo->TxBufferNdx                   = OBEX_DISCONNECT_PACKET_HEADER_OFFSET;
                     }
                     else
                     {
                        /* These Responses is only allowed when the port*/
                        /* is in the connected state.                   */
                        ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
                     }
                     break;
                  case OBEX_PUT_COMMAND:
                  case OBEX_GET_COMMAND:
                  case OBEX_SET_PATH_COMMAND:
                  case OBEX_ABORT_COMMAND:
                     /* Check to see if the Port is in the connected    */
                     /* state.                                          */
                     if(PortInfo->PortState == psConnected)
                     {
                        PortInfo->TxBuffer[OBEX_COMMAND_OFFSET] = ResponseCode;
                        PortInfo->TxBufferNdx                   = OBEX_PUT_PACKET_HEADER_OFFSET;
                     }
                     else
                     {
                        /* These Responses is only allowed when the port*/
                        /* is in the connected state.                   */
                        ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
                     }
                     break;
                  default:
                     /* Unknown Command specified.                      */
                     ret_val = BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED;
                     break;
               }

               /* If there was no problem with the connection state and */
               /* there is Header Information, then format the Header   */
               /* List in a Stream.                                     */
               if((ret_val == 0) && (Header_List))
               {
                  /* Add the Optional Header information to the buffer. */
                  /* If there is not enough buffer space to contain all */
                  /* of the header information, the function will return*/
                  /* an error.                                          */
                  ret_val = HeaderListToHeaderStream(Header_List, PortInfo);
               }

               /* If there was no problem adding the header information */
               /* then try to send to data.                             */
               if(ret_val == 0)
               {
                  /* Add the size of the Packet to the OBEX Connect     */
                  /* Packet.  The number of bytes that is contained in  */
                  /* the Packet is equal to the Buffer Index value.     */
                  ASSIGN_HOST_WORD_TO_BIG_ENDIAN_UNALIGNED_WORD(&(PortInfo->TxBuffer[OBEX_PACKET_LENGTH_OFFSET]), PortInfo->TxBufferNdx);

                  /* Send the data packet to SPP for transmission to the*/
                  /* remote device.  If all of the data was not         */
                  /* accepted, return an error.                         */
                  if(SPPM_WriteData(PortInfo->SPPPortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE, (Word_t)PortInfo->TxBufferNdx, PortInfo->TxBuffer) == (Word_t)PortInfo->TxBufferNdx)
                  {
                     if(PortInfo->PendingResponse == OBEX_DISCONNECT_COMMAND)
                     {
                        /* Reset the information for the connection to  */
                        /* their default values.                        */
                        PortInfo->RxBufferNdx        = 0;
                        PortInfo->TxBufferNdx        = 0;
                        PortInfo->OutMaxPacketLength = OBEX_PACKET_LENGTH_MINIMUM;
                        PortInfo->PacketLength       = 0;
                        PortInfo->PendingCommand     = 0;
                        PortInfo->PendingResponse    = OBEX_INVALID_COMMAND;
                        PortInfo->PortState          = psPortOpened;
                     }

                     /* Set the Pending Response back to an invalid     */
                     /* state.                                          */
                     PortInfo->PendingResponse = OBEX_INVALID_COMMAND;
                  }
                  else
                     ret_val = BTPS_ERROR_WRITING_DATA_TO_DEVICE;
               }
            }
            else
            {
               /* No reference to the Port ID supplied was located.     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* Internal Functions.                                               */

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* * NOTE * This function does not insert duplicate entries into the */
   /*          list.  An element is considered a duplicate if the       */
   /*          Serial Port ID field is the same as an entry already in  */
   /*          the list.  When this occurs, this function returns NULL. */
static OTPM_Info_t *AddOTPInfoEntry(OTPM_Info_t **ListHead, OTPM_Info_t *EntryToAdd)
{
   OTPM_Info_t *AddedEntry = NULL;
   OTPM_Info_t *tmpEntry;

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* Make sure that the element that we are adding seems semi-valid.*/
      if(EntryToAdd->OTP_ID)
      {
         /* OK, data seems semi-valid, let's allocate a new data        */
         /* structure to add to the list.                               */
         AddedEntry = (OTPM_Info_t *)BTPS_AllocateMemory(sizeof(OTPM_Info_t));

         if(AddedEntry)
         {
            /* Copy All Data over.                                      */
            *AddedEntry               = *EntryToAdd;

            /* Now Add it to the end of the list.                       */
            AddedEntry->NextInfoEntry = NULL;

            /* First, let's check to see if there are any elements      */
            /* already present in the List that was passed in.          */
            if((tmpEntry = *ListHead) != NULL)
            {
               /* Head Pointer was not NULL, so we will traverse the    */
               /* list until we reach the last element.                 */
               while(tmpEntry)
               {
                  if(tmpEntry->OTP_ID == AddedEntry->OTP_ID)
                  {
                     /* Entry was already added, so free the memory and */
                     /* flag an error to the caller.                    */
                     FreeOTPInfoEntryMemory(AddedEntry);
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
                     if(tmpEntry->NextInfoEntry)
                        tmpEntry = tmpEntry->NextInfoEntry;
                     else
                        break;
                  }
               }

               if(AddedEntry)
               {
                  /* Last element found, simply Add the entry.          */
                  tmpEntry->NextInfoEntry = AddedEntry;
               }
            }
            else
               *ListHead = AddedEntry;
         }
      }
   }

   return(AddedEntry);
}

   /* The following function searches the specified List for the        */
   /* specified GOEP_ID.  This function returns NULL if either the List */
   /* Head is invalid, the OTP_ID is invalid, or the specified Serial   */
   /* Port (based on the OTP_ID) was not found.                         */
static OTPM_Info_t *SearchOTPInfoEntry(OTPM_Info_t **ListHead, unsigned int OTP_ID)
{
   OTPM_Info_t *FoundEntry = NULL;

   /* Let's make sure the list and FTPM Mgr ID to search for appear to  */
   /* be valid.                                                         */
   if((ListHead) && (OTP_ID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->OTP_ID != OTP_ID))
         FoundEntry = FoundEntry->NextInfoEntry;
   }

   return(FoundEntry);
}

   /* The following function searches the specified Port Information    */
   /* List for the specified Serial Port and removes it from the List.  */
   /* This function returns NULL if either the Port Information List    */
   /* Head is invalid, the Port Number is invalid, or the specified Port*/
   /* Number was NOT present in the list.  The entry returned will have */
   /* the Next Entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeOTPInfoEntryMemory().                                         */
static OTPM_Info_t *DeleteOTPInfoEntry(OTPM_Info_t **ListHead, unsigned int OTP_ID)
{
   OTPM_Info_t *FoundEntry = NULL;
   OTPM_Info_t *LastEntry  = NULL;

   /* Let's make sure the List and FTPM Mgr ID to search for appear to  */
   /* be semi-valid.                                                    */
   if((ListHead) && (OTP_ID))
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->OTP_ID != OTP_ID))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextInfoEntry;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextInfoEntry = FoundEntry->NextInfoEntry;
         }
         else
            *ListHead = FoundEntry->NextInfoEntry;

         FoundEntry->NextInfoEntry = NULL;
      }
   }

   return(FoundEntry);
}

   /* This function frees the specified Port Information Entry member.  */
   /* No check is done on this entry other than making sure it NOT NULL.*/
static void FreeOTPInfoEntryMemory(OTPM_Info_t *EntryToFree)
{
   if(EntryToFree)
   {
      if(EntryToFree->ResponseBuffer)
         BTPS_FreeMemory(EntryToFree->ResponseBuffer);

      if(EntryToFree->DirEntrySegment)
         BTPS_FreeMemory(EntryToFree->DirEntrySegment);

      if(EntryToFree->ExtendedNameBuffer)
         BTPS_FreeMemory(EntryToFree->ExtendedNameBuffer);

      BTPS_FreeMemory((void *)(EntryToFree));
   }
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Port Information List.  Upon return of   */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeOTPInfoList(OTPM_Info_t **ListHead)
{
   OTPM_Info_t *EntryToFree;
   OTPM_Info_t *tmpEntry;

   /* Let's make sure the List Head appears to be semi-valid.           */
   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextInfoEntry;

         FreeOTPInfoEntryMemory(tmpEntry);
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }
}

   /* The following function is responsible for returning the Next      */
   /* Available ConnectionID.  This function will only return values    */
   /* that will be interpreted as Positive Integers (i.e.  the Most     */
   /* Significant Bit will NEVER be set).                               */
static DWord_t GetNextConnection_ID(void)
{
   /* Increment the Counter to the next number.  Check the new number to*/
   /* see if it has gone negative (when Connection ID is viewed as a    */
   /* signed integer).  If so, return to the first valid Number (one).  */
   NextConnectionID++;

   if(NextConnectionID == INVALID_CONNECTION_ID)
      NextConnectionID = 1;

   /* Simply return the Connection ID to the caller.                    */
   return(NextConnectionID);
}

   /* The following function is responsible for returning the Next      */
   /* Available Port ID.  This function will only return values that    */
   /* will be interpreted as Positive Integers (i.e.  the Most          */
   /* Significant Bit will NEVER be set).                               */
static unsigned int GetNextOTP_ID(void)
{
   /* Increment the Counter to the next number.  Check the new number   */
   /* to see if it has gone negative (when Port ID is viewed as a       */
   /* signed integer).  If so, return to the first valid Number (one).  */
   NextOTPID++;

   if(((int)NextOTPID) < 0)
      NextOTPID = 1;

   /* Simply return the FTPM Mgr Port ID to the caller.                 */
   return(NextOTPID);
}

   /* The following function converts a UNICODE character sequence into */
   /* a Byte Sequence.  This assumes that the UNICODE is in English     */
   /* format and the High Byte is always Zero.  Since the UNICODE string*/
   /* is twice what is required for the byte sequence.  no extra memory */
   /* is required.  The original UNICODE string will be replaced with   */
   /* the Byte Sequence.                                                */
   /* * NOTE * Even though the UNICodePtr is a Word_t Pointer, the      */
   /*          Word_t that is actually at this location is in BIG       */
   /*          ENDIAN Format, *** NOT *** the local host format.        */
static void UnicodeHeaderValueToByteSequence(Word_t *UNICodePtr, unsigned int UNICodeStringLength, unsigned int UNICodeBufferSize)
{
   if((UNICodePtr) && (UNICodeBufferSize))
   {
      if(!ConvertUTF16ToUTF8((Byte_t *)UNICodePtr, UNICodeStringLength, UNICodeBufferSize))
         ((Byte_t *)UNICodePtr)[0] = 0;
   }
}

   /* The following function is used to convert a character sequence to */
   /* a Unicode sequence.  The function is passed a pointer to the      */
   /* character sequence and a pointer to a buffer area that will       */
   /* receive the formatted Unicode Text.  The length of the character  */
   /* sting is provided in the Length parameter.                        */
static int CharSequenceToUnicodeHeaderValue(char *CharPtr, Byte_t *UnicodeText, int Length)
{
   int ret_val;

   /* Verify the pointers that have been passed in are not NULL.        */
   if((CharPtr) && (UnicodeText))
   {
      if(ConvertUTF8ToUTF16((Byte_t *)CharPtr, Length, UnicodeText, (Length*2)))
         ret_val = 0;
      else
         ret_val = -2;
   }
   else
      ret_val = -1;

   return(ret_val);
}

   /* The following function is used to convert structure information   */
   /* containing information used for OBEX Authentication to a Tag,     */
   /* Length and Value stream.  The function takes as it parameter a    */
   /* Header ID that identifies the supplied structure information as an*/
   /* Authentication Challenge structure or an Authentication Response  */
   /* structure.  The parameters Src points to either a                 */
   /* OTP_Digest_Challenge_t structure or a Digest_Response_t structure.*/
   /* The parameter Triplet points to an area of memory where the Tag,  */
   /* Length and Value result will be stored.  The function will return */
   /* a negative value if an error occurred while converting the data.  */
   /* If the return value is a positive number, the value represents the*/
   /* number of bytes that are contained in the Triplet value.  If the  */
   /* function is called with Triplet equal to NULL, then this function */
   /* will calculate the number of bytes that are required without      */
   /* actually copying any data.                                        */
static int StructureToTagLengthValue(OBEX_Header_ID_t Header_ID, void *Src, OTP_Tag_Length_Value_t *Triplet)
{
   int ret_val;
   int TripletLength;

   /* Check to make sure the Src pointer is not NULL.  The value for    */
   /* Triplet is allowed to be NULL.                                    */
   if(Src)
   {
      /* Initialize the return value to indicate no error.              */
      ret_val = 0;

      /* Check to see if the structure that was passed to this function */
      /* is a Digest_Challenge_t structure.                             */
      if(Header_ID == hidAuthenticationChallenge)
      {
         /* Determine that amount of memory required to hold the Nonce  */
         /* information.                                                */
         TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(sizeof(((OTP_Digest_Challenge_t *)Src)->Nonce));

         /* If Triplet is defined, then convert the data from the       */
         /* structure to a data stream.                                 */
         if(Triplet)
         {
            /* Save the data in a Tag-Length-Data format.               */
            Triplet->Tag    = OTP_DIGEST_CHALLENGE_TAG_NONCE;
            Triplet->Length = sizeof(((OTP_Digest_Challenge_t *)Src)->Nonce);

            BTPS_MemCopy(Triplet->Value, ((OTP_Digest_Challenge_t *)Src)->Nonce, Triplet->Length);

            /* Adjust the Triplet pointer to the byte just past the     */
            /* information just saved.                                  */
            Triplet = (OTP_Tag_Length_Value_t *)((Byte_t *)Triplet + TripletLength);
         }

         /* Set the return value to the number of bytes that has been   */
         /* used by the Nonce.                                          */
         ret_val = TripletLength;

         /* Check to see if the optional field has been defined.        */
         if(((OTP_Digest_Challenge_t *)Src)->OptionalParametersMask & OTP_DIGEST_CHALLENGE_OPTIONAL_PARAMETERS_MASK_OPTIONS)
         {
            /* Determine the amount of memory required to contain the   */
            /* optional information byte.                               */
            TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(sizeof(((OTP_Digest_Challenge_t *)Src)->Options));

            /* If Triplet is defined, then convert the data from the    */
            /* structure to a data stream.                              */
            if(Triplet)
            {
               /* Save the data in a Tag-Length-Data format.            */
               Triplet->Tag      = OTP_DIGEST_CHALLENGE_TAG_OPTIONS;
               Triplet->Length   = sizeof(((OTP_Digest_Challenge_t *)Src)->Options);
               Triplet->Value[0] = ((OTP_Digest_Challenge_t *)Src)->Options;

               /* Adjust the Triplet pointer to the byte just past the  */
               /* information just saved.                               */
               Triplet           = (OTP_Tag_Length_Value_t *)((Byte_t *)Triplet + TripletLength);
            }

            /* Set the return value to the total number of bytes that   */
            /* has been used so far.                                    */
            ret_val += TripletLength;
         }

         /* Check to see if the Realm information has been supplied.    */
         if(((OTP_Digest_Challenge_t *)Src)->OptionalParametersMask & OTP_DIGEST_CHALLENGE_OPTIONAL_PARAMETERS_MASK_REALM)
         {
            /* Determine the amount of memory required to contain the   */
            /* Realm information.                                       */
            TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(((OTP_Digest_Challenge_t *)Src)->RealmLength+1);

            /* If Triplet is defined, then convert the data from the    */
            /* structure to a data stream.                              */
            if(Triplet)
            {
               /* Save the data in a Tag-Length-Data format.            */
               Triplet->Tag      = OTP_DIGEST_CHALLENGE_TAG_REALM;
               Triplet->Length   = (Byte_t)(((OTP_Digest_Challenge_t *)Src)->RealmLength+1);
               Triplet->Value[0] = ((OTP_Digest_Challenge_t *)Src)->RealmCharacterSet;

               BTPS_MemCopy(&Triplet->Value[1], &((OTP_Digest_Challenge_t *)Src)->Realm, Triplet->Length-1);
            }

            /* Set the return value to the total number of bytes that   */
            /* has been used so far.                                    */
            ret_val += TripletLength;
         }
      }
      else
      {
         /* Check to see if the structure that was passed to this       */
         /* function is a Digest_Response_t structure.                  */
         if(Header_ID == hidAuthenticationResponse)
         {
            /* Determine that amount of memory required to hold the     */
            /* Request Digest information.                              */
            TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(sizeof(((OTP_Digest_Response_t *)Src)->RequestDigest));

            /* If Triplet is defined, then convert the data from the    */
            /* structure to a data stream.                              */
            if(Triplet)
            {
               /* Save the data in a Tag-Length-Data format.            */
               Triplet->Tag    = OTP_DIGEST_RESPONSE_TAG_REQUEST_DIGEST;
               Triplet->Length = sizeof(((OTP_Digest_Response_t *)Src)->RequestDigest);

               BTPS_MemCopy(Triplet->Value, ((OTP_Digest_Response_t *)Src)->RequestDigest, Triplet->Length);

               /* Adjust the Triplet pointer to the byte just past the  */
               /* information just saved.                               */
               Triplet = (OTP_Tag_Length_Value_t *)((Byte_t *)Triplet + TripletLength);
            }

            /* Set the return value to the number of bytes that has been*/
            /* used by the Nonce.                                       */
            ret_val = TripletLength;

            /* Check to see if the User ID information has been         */
            /* supplied.                                                */
            if(((OTP_Digest_Response_t *)Src)->OptionalParametersMask & OTP_DIGEST_RESPONSE_OPTIONAL_PARAMETERS_MASK_USER_ID)
            {
               /* Determine that amount of memory required to hold the  */
               /* User ID information.                                  */
               TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(((OTP_Digest_Response_t *)Src)->UserIDLength);

               /* If Triplet is defined, then convert the data from the */
               /* structure to a data stream.                           */
               if(Triplet)
               {
                  /* Save the data in a Tag-Length-Data format.         */
                  Triplet->Tag      = OTP_DIGEST_RESPONSE_TAG_USER_ID;
                  Triplet->Length   = (Byte_t)((OTP_Digest_Response_t *)Src)->UserIDLength;

                  BTPS_MemCopy(Triplet->Value, ((OTP_Digest_Response_t *)Src)->UserID, Triplet->Length);

                  /* Adjust the Triplet pointer to the byte just past   */
                  /* the information just saved.                        */
                  Triplet           = (OTP_Tag_Length_Value_t *)((Byte_t *)Triplet + TripletLength);
               }

               /* Set the return value to the total number of bytes that*/
               /* has been used so far.                                 */
               ret_val += TripletLength;
            }

            /* Check to see if the User ID information has been Nonce.  */
            if(((OTP_Digest_Response_t *)Src)->OptionalParametersMask & OTP_DIGEST_RESPONSE_OPTIONAL_PARAMETERS_MASK_NONCE)
            {
               /* Determine that amount of memory required to hold the  */
               /* Nonce information.                                    */
               TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(sizeof(((OTP_Digest_Response_t *)Src)->Nonce));

               /* If Triplet is defined, then convert the data from the */
               /* structure to a data stream.                           */
               if(Triplet)
               {
                  /* Save the data in a Tag-Length-Data format.         */
                  Triplet->Tag      = OTP_DIGEST_RESPONSE_TAG_NONCE;
                  Triplet->Length   = (Byte_t)(sizeof(((OTP_Digest_Response_t *)Src)->Nonce));

                  BTPS_MemCopy(Triplet->Value, ((OTP_Digest_Response_t *)Src)->Nonce, Triplet->Length);
               }

               /* Set the return value to the total number of bytes that*/
               /* has been used so far.                                 */
               ret_val += TripletLength;
            }
         }
      }
   }
   else
   {
      /* Flag that an invalid parameter was passed.                     */
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   return(ret_val);
}

   /* The following function is used to convert Tag-Length-Value        */
   /* formatted data to either a OTP_Digest_Challenge_t or              */
   /* OTP_Digest_Response_t structure.  The function takes as its       */
   /* parameters a Header_ID that specifies the type of structure that  */
   /* Dest points to.  The HeaderLength parameter defines the number of */
   /* bytes that are contained in the data pointed to by the Triplet    */
   /* parameter.  The Triplet parameter points to the actual            */
   /* Tag-Length-Value data.  The parameter Dest points to either an    */
   /* OTP_Digest_Challenge_t structure or an OTP_Digest_Response_t      */
   /* structure.  The function returns a negative value if an error     */
   /* occurs while converting the data.  If no errors occur, the        */
   /* function returns Zero.                                            */
static int TagLengthValueToStructure(OBEX_Header_ID_t Header_ID, int HeaderLength, OTP_Tag_Length_Value_t *Triplet, void *Dest)
{
   int ret_val;
   int TripletLength;
   int FieldLength;

   /* Make sure that the Dest parameter was passed to the function.     */
   if(Dest)
   {
      ret_val = 0;

      /* Check to see if Dest points to a Digest_Challenge_t structure. */
      if(Header_ID == hidAuthenticationChallenge)
      {
         /* Initialize the Parameter Mask to indicate no parameters are */
         /* currently present.                                          */
         ((OTP_Digest_Challenge_t *)Dest)->OptionalParametersMask = 0;

         /* While there is data to be converted, process the data.      */
         while(HeaderLength > 0)
         {
            /* Get the number of bytes that make up the Tag-Length-Value*/
            /* information pointed to by Triplet.                       */
            TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(Triplet->Length);

            /* Check to see what information is being pointed to by     */
            /* Triplet.                                                 */
            switch(Triplet->Tag)
            {
               case OTP_DIGEST_CHALLENGE_TAG_NONCE:
                  /* Since we have a maximum length for storing the     */
                  /* Nonce, make sure we do not go outside the bounds of*/
                  /* this parameters storage area.                      */
                  FieldLength = (Triplet->Length < sizeof(((OTP_Digest_Challenge_t *)Dest)->Nonce))?Triplet->Length:sizeof(((OTP_Digest_Challenge_t *)Dest)->Nonce);

                  BTPS_MemCopy(((OTP_Digest_Challenge_t *)Dest)->Nonce, Triplet->Value, FieldLength);
                  break;
               case OTP_DIGEST_CHALLENGE_TAG_OPTIONS:
                  /* Save the Option Byte and Flag that the optional    */
                  /* information bytes is present in the structure.     */
                  ((OTP_Digest_Challenge_t *)Dest)->OptionalParametersMask |= OTP_DIGEST_CHALLENGE_OPTIONAL_PARAMETERS_MASK_OPTIONS;
                  ((OTP_Digest_Challenge_t *)Dest)->Options                 = Triplet->Value[0];
                  break;
               case OTP_DIGEST_CHALLENGE_TAG_REALM:
                  /* Since we have a maximum length for storing the     */
                  /* Realm information, make sure that we do not go     */
                  /* outside the bounds of this parameters storage area.*/
                  FieldLength = ((Triplet->Length-1) < sizeof(((OTP_Digest_Challenge_t *)Dest)->Realm))?Triplet->Length-1:sizeof(((OTP_Digest_Challenge_t *)Dest)->Realm);

                  /* Move the data to the structure and flag that the   */
                  /* Realm information is present in the structure.     */
                  ((OTP_Digest_Challenge_t *)Dest)->OptionalParametersMask |= OTP_DIGEST_CHALLENGE_OPTIONAL_PARAMETERS_MASK_REALM;
                  ((OTP_Digest_Challenge_t *)Dest)->RealmCharacterSet       = Triplet->Value[0];
                  ((OTP_Digest_Challenge_t *)Dest)->RealmLength             = FieldLength;

                  BTPS_MemCopy(((OTP_Digest_Challenge_t *)Dest)->Realm, &Triplet->Value[1], FieldLength);
                  break;
               default:
                  /* If this is an unknown Tag, then we have reached an */
                  /* error condition.  Set the Length to force the      */
                  /* process loop to terminate and set the return value */
                  /* to indicate an error.                              */
                  TripletLength = HeaderLength;

                  ret_val       = BTPS_ERROR_OTP_ERROR_PARSING_DATA;
            }

            /* Advance Triplet to point to the next parameter in the    */
            /* sequence.                                                */
            Triplet       = (OTP_Tag_Length_Value_t *)((Byte_t *)Triplet + TripletLength);

            /* Deduct the number of bytes that was processed in this    */
            /* pass.                                                    */
            HeaderLength -= TripletLength;
         }
      }
      else
      {
         /* Check to see if Dest points to a Digest_Response_t          */
         /* structure.                                                  */
         if(Header_ID == hidAuthenticationResponse)
         {
            /* Initialize the Parameter Mask to indicate no parameters  */
            /* are currently present.                                   */
            ((OTP_Digest_Response_t *)Dest)->OptionalParametersMask = 0;

            /* While there is data to be converted, process the data.   */
            while(HeaderLength > 0)
            {
               /* Get the number of bytes that make up the              */
               /* Tag-Length-Value information pointed to by Triplet.   */
               TripletLength = OTP_TAG_LENGTH_VALUE_SIZE(Triplet->Length);

               /* Check to see what information is being pointed to by  */
               /* Triplet.                                              */
               switch(Triplet->Tag)
               {
                  case OTP_DIGEST_RESPONSE_TAG_REQUEST_DIGEST:
                     /* Since we have a maximum length for storing the  */
                     /* Digest, make sure we do not go outside the      */
                     /* bounds of this parameters storage area.         */
                     FieldLength = (Triplet->Length < sizeof(((OTP_Digest_Response_t *)Dest)->RequestDigest))?Triplet->Length:sizeof(((OTP_Digest_Response_t *)Dest)->RequestDigest);

                     BTPS_MemCopy(((OTP_Digest_Response_t *)Dest)->RequestDigest, Triplet->Value, FieldLength);
                     break;
                  case OTP_DIGEST_RESPONSE_TAG_USER_ID:
                     /* Since we have a maximum length for storing the  */
                     /* User ID, make sure we do not go outside the     */
                     /* bounds of this parameters storage area.         */
                     FieldLength = (Triplet->Length < sizeof(((OTP_Digest_Response_t *)Dest)->UserID))?Triplet->Length:sizeof(((OTP_Digest_Response_t *)Dest)->UserID);

                     ((OTP_Digest_Response_t *)Dest)->OptionalParametersMask |= OTP_DIGEST_RESPONSE_OPTIONAL_PARAMETERS_MASK_USER_ID;
                     ((OTP_Digest_Response_t *)Dest)->UserIDLength            = FieldLength;

                     BTPS_MemCopy(((OTP_Digest_Response_t *)Dest)->UserID, Triplet->Value, FieldLength);
                     break;
                  case OTP_DIGEST_RESPONSE_TAG_NONCE:
                     /* Since we have a maximum length for storing the  */
                     /* Nonce, make sure we do not go outside the bounds*/
                     /* of this parameters storage area.                */
                     FieldLength = (Triplet->Length < sizeof(((OTP_Digest_Response_t *)Dest)->Nonce))?Triplet->Length:sizeof(((OTP_Digest_Response_t *)Dest)->Nonce);

                     ((OTP_Digest_Response_t *)Dest)->OptionalParametersMask |= OTP_DIGEST_RESPONSE_OPTIONAL_PARAMETERS_MASK_NONCE;

                     BTPS_MemCopy(((OTP_Digest_Response_t *)Dest)->Nonce, Triplet->Value, FieldLength);
                     break;
                  default:
                     /* If this is an unknown Tag, then we have reached */
                     /* an error condition.  Set the Length to force the*/
                     /* process loop to terminate and set the return    */
                     /* value to indicate an error.                     */
                     TripletLength = HeaderLength;

                     ret_val       = BTPS_ERROR_OTP_ERROR_PARSING_DATA;
               }

               /* Advance Triplet to point to the next parameter in the */
               /* sequence.                                             */
               Triplet       = (OTP_Tag_Length_Value_t *)((Byte_t *)Triplet + TripletLength);

               /* Deduct the number of bytes that was processed in this */
               /* pass.                                                 */
               HeaderLength -= TripletLength;
            }
         }
      }
   }
   else
   {
      /* Flag that an invalid parameter was passed.                     */
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
   }

   return(ret_val);
}

   /* The following function is used to scan through an array of headers*/
   /* for the header ID type that was specified.  If found, the index of*/
   /* the header in the list is returned.  If the header was not        */
   /* located, the function returns a negative 1.                       */
static int FindHeader(OBEX_Header_ID_t HeaderID, OBEX_Header_List_t *ListPtr)
{
   int ret_val = HEADER_NOT_FOUND;
   int ndx     = 0;
   int num_headers;

   if(ListPtr)
   {
      /* Make a local copy of the number of headers in the list.  We    */
      /* don't want to change the original information.                 */
      num_headers = ListPtr->NumberOfHeaders;
      while(num_headers--)
      {
         /* If a header with the correct ID is found.  Note the index   */
         /* within the array where the header is located and return the */
         /* index to the caller.                                        */
         if(ListPtr->Headers[ndx].OBEX_Header_ID == HeaderID)
         {
            ret_val = ndx;
            break;
         }

         ndx++;
      }
   }

   return(ret_val);
}

   /* The following function is used to scan a sequence of memory in    */
   /* search for a specific character.  The function will return the    */
   /* number of bytes from the current pointer position to the character*/
   /* that was located.  The function will return an error if a NULL    */
   /* character is located before locating the Token character that is  */
   /* being searched for.                                               */
static int DistanceToToken(char Token, char *DataPtr)
{
   int ret_val;

   ret_val  = 0;

   /* While we have not reached a NULL character and we have not seen   */
   /* the Token character, continue the search.                         */
   while((*DataPtr) && (*DataPtr != Token))
   {
      /* If we are searching for a <SPACE> characters, then we need to  */
      /* also treat all whitespace characters as a <SPACE>.             */
      if((Token == _SPACE_) && (IS_WHITE_SPACE_CHARACTER(*DataPtr)))
         break;

      /* If we are searching for any character but a <SPACE> character, */
      /* then we also must include all white space characters.          */
      if((Token == _NON_SPACE_) && (!IS_WHITE_SPACE_CHARACTER(*DataPtr)))
         break;

      /* The ret_val is a count of the number of character that we have */
      /* checked, do increment the count and increment the pointer to   */
      /* the next character.                                            */
      ret_val++;
      DataPtr++;
   }

   /* No that we have broken out of the loop, we need to check to see if*/
   /* we exited the loop because we have found the character that we    */
   /* were looking for or if an error has occurred.  It will not be     */
   /* considered an error if the character that we are currently pointed*/
   /* to is a NULL character and we were looking for the NULL character.*/
   if((!*DataPtr) && (Token != 0))
   {
      ret_val = -1;
   }

   /* Return the count or the error code.                               */
   return(ret_val);
}

   /* The following utility routine is used to extract digits from a    */
   /* string of digits.  This is used when extracting time and date     */
   /* information from a XML formatted directory or file listing.       */
static int ExtractInt(char *CharPtr, int NumberOfDigits)
{
   int value = 0;

   /* Stop if we reach a NULL or the Number of Digits have been         */
   /* extracted.                                                        */
   while((*CharPtr) && (NumberOfDigits))
   {
      /* Check to make sure that the current character is a valid digit */
      /* of 0 through 9.                                                */
      if((*CharPtr >= '0') && (*CharPtr <= '9'))
      {
         value = (value * 10) + (*CharPtr & 0x0F);
      }
      else
      {
         /* Exit on Error.                                              */
         break;
      }

      /* Advance to the next character in the string.                   */
      CharPtr++;
      NumberOfDigits--;
   }

   /* Check to see if an error occurred while parsing the string.  If   */
   /* the Number of Digits is greater then Zero, then we stopped early  */
   /* due to some error condition.                                      */
   if(NumberOfDigits)
   {
      value = -1;
   }

   return(value);
}

   /* The following utility routine is used to extract digits from a    */
   /* string of digits.  This is used when extracting time and date     */
   /* information from a XML formatted directory or file listing.       */
static Byte_t ExtractPermissionInfo(char *PermPtr, int num_permissions)
{
   char   ch;
   Byte_t Perms;

   Perms = 0;
   while(num_permissions--)
   {
      /* Since the values are not case sensitive, we will convert the   */
      /* character to lower case first to make the processing easier.   */
      /* If a match is then found, set the appropriate bit flag in the  */
      /* access rights variable.                                        */
      ch = (char)(*PermPtr++ | MAKE_LOWER_CASE);
      if(ch == 'r')
      {
         Perms |= OTP_PERMISSION_READ_MASK;
      }
      else
      {
         if(ch == 'w')
         {
            Perms |= OTP_PERMISSION_WRITE_MASK;
         }
         else
         {
            if(ch == 'd')
            {
               Perms |= OTP_PERMISSION_DELETE_MASK;
            }
         }
      }
   }

   return(Perms);
}

   /* The following function is used to extract the Date and Time       */
   /* information for a OBEX ISO Time formatted string.  The time/date  */
   /* information is expressed as YYYYMMDDTHHMMSS and is possibly       */
   /* terminated with the character Z.  The Z indicated the time is     */
   /* provided as a UTC time.  The function takes as its parameters a   */
   /* pointer to the OBEX ISO Time string and a pointer to a Time/Date  */
   /* structure where the converted information will be saved.          */
static int ExtractTimeDate(char *TDStringPtr, OTP_TimeDate_t *TDStructPtr)
{
   int ret_val = -1;
   int value;

   /* Ensure the pointer are not NULL.                                  */
   if((TDStringPtr) && (TDStructPtr))
   {
      /* Since we will allow the use to provide a pointer to may not be */
      /* positioned at the beginning of the string, we will start by    */
      /* scanning past all characters that are not a valid ASCII digit. */
      while((*TDStringPtr) && ((*TDStringPtr < '0') || (*TDStringPtr > '9')))
      {
         TDStringPtr++;
      }

      /* The first item to be extracted is the Year.  This is a 4 digit */
      /* value.  If an error occurs trying to extract the value, exit   */
      /* with an error.                                                 */
      if((value = ExtractInt(TDStringPtr, 4)) >= 0)
      {
         /* Adjust the pointer past the 4 characters we have just       */
         /* extracted and save the Year value in the Structure.         */
         TDStringPtr       += 4;
         TDStructPtr->Year  = (Word_t)value;

         /* Extract the 2 digit Month.  If an error occurs, exit with   */
         /* and error code.                                             */
         if((value = ExtractInt(TDStringPtr, 2)) >= 0)
         {
            /* Adjust the pointer past the 2 characters that we have    */
            /* just extracted and save the value in the structure.      */
            TDStringPtr        += 2;
            TDStructPtr->Month  = (Word_t)value;

            /* Extract the 2 digit Day.  If an error occurs, exit with  */
            /* and error code.                                          */
            if((value = ExtractInt(TDStringPtr, 2)) >= 0)
            {
               /* Adjust the pointer past the 2 characters that we have */
               /* just extracted and save the value in the structure.   */
               TDStringPtr      += 2;
               TDStructPtr->Day  = (Word_t)value;

               /* For a consistency check, make sure that the next      */
               /* character is a T.  This is a separator between the    */
               /* Date and Time.                                        */
               if(*TDStringPtr == 'T')
               {
                  /* Adjust the pointer past the separator.             */
                  TDStringPtr += 1;

                  /* Extract the 2 digit Hour.  If an error occurs, exit*/
                  /* with and error code.                               */
                  if((value = ExtractInt(TDStringPtr, 2)) >= 0)
                  {
                     /* Adjust the pointer past the 2 characters that we*/
                     /* have just extracted and save the value in the   */
                     /* structure.                                      */
                     TDStringPtr       += 2;
                     TDStructPtr->Hour  = (Word_t)value;

                     /* Extract the 2 digit Minute.  If an error occurs,*/
                     /* exit with and error code.                       */
                     if((value = ExtractInt(TDStringPtr, 2)) >= 0)
                     {
                        /* Adjust the pointer past the 2 characters that*/
                        /* we have just extracted and save the value in */
                        /* the structure.                               */
                        TDStringPtr         += 2;
                        TDStructPtr->Minute  = (Word_t)value;

                        /* Extract the 2 digit Minute.  If an error     */
                        /* occurs, exit with and error code.            */
                        if((value = ExtractInt(TDStringPtr, 2)) >= 0)
                        {
                           /* Adjust the pointer past the 2 characters  */
                           /* that we have just extracted and save the  */
                           /* value in the structure.                   */
                           TDStringPtr         += 2;
                           TDStructPtr->Second  = (Word_t)value;

                           /* The last character may contain a Z to     */
                           /* denote UTC formatted time.                */
                           TDStructPtr->UTC_Time = (Boolean_t)((*TDStringPtr == 'Z')?TRUE:FALSE);

                           /* No errors were detected, return zero to   */
                           /* denote success.                           */
                           ret_val = 0;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is used to extract information about on    */
   /* OBEX Object from the information that is provide in the optional  */
   /* header information that precedes the object.  The OTPInfoPtr      */
   /* parameter points to an information structure that contains        */
   /* information about the connection that is currently being used.    */
   /* The second parameter is a pointer to a structure that contains    */
   /* fields that correlate to the various header information that may  */
   /* exist.  The third parameter is a pointer to the Header            */
   /* information, the Header information is proceeded by an 1 byte ID  */
   /* that identified the information that is contained in the header   */
   /* and possibly extra information that defines the length of the     */
   /* header data.                                                      */
static void ExtractObjectInfo(OTPM_Info_t *OTPInfoPtr, OTP_ObjectInfo_t *ObjectInfoPtr, OBEX_Header_List_t *ListPtr)
{
   int            i;
   OBEX_Header_t *HeaderPtr;

   /* Verify that the pointers provided are not NULL.                   */
   if((ObjectInfoPtr) && (ListPtr))
   {
      /* Search the header list for information on the Name of the      */
      /* Object.                                                        */
      if((i = FindHeader(hidName, ListPtr)) >= 0)
      {
         /* The header information was located.  Set a pointer to the   */
         /* beginning of the header to aid in the extraction of the     */
         /* information.  Extract the information and place the         */
         /* information in the structure.  The Name and Type field      */
         /* provide a programming problem.  These fields are made up of */
         /* a sequence of characters and they are not limited in any    */
         /* particular size.  To avoid dynamically allocating memory to */
         /* save this information, a choice was made to limit these     */
         /* fields to a certain number of characters.  This seems       */
         /* sufficient to contain a reasonable size message.  To        */
         /* protect against the possibility of exceeding this limit, a  */
         /* check of the size is made before it is copied and will be   */
         /* truncated if needed.                                        */
         HeaderPtr                 = &ListPtr->Headers[i];

         ObjectInfoPtr->NameLength = HeaderPtr->Header_Value.UnicodeText.DataLength;
         ObjectInfoPtr->Name[0]    = 0;

         if(ObjectInfoPtr->NameLength)
         {
            ObjectInfoPtr->FieldMask  |= OTP_OBJECT_INFO_MASK_NAME;

            /* Convert the name from Unicode to a Byte sequence.        */
            UnicodeHeaderValueToByteSequence(HeaderPtr->Header_Value.UnicodeText.ValuePointer, HeaderPtr->Header_Value.UnicodeText.DataLength, HeaderPtr->Header_Value.UnicodeText.DataLength*2);
            if(HeaderPtr->Header_Value.UnicodeText.DataLength > sizeof(ObjectInfoPtr->Name))
            {
               /* Protect against going out of bounds.                  */
               ObjectInfoPtr->NameLength = sizeof(ObjectInfoPtr->Name);
            }

            BTPS_MemCopy(ObjectInfoPtr->Name, (char *)HeaderPtr->Header_Value.UnicodeText.ValuePointer, ObjectInfoPtr->NameLength);

            /* In case the Input String was truncated, we need to make  */
            /* sure that the Name String is NULL terminated.            */
            ObjectInfoPtr->Name[sizeof(ObjectInfoPtr->Name) - 1] = 0;

            /* Check to see if the Name had to be truncated in order to */
            /* get it to fit in the buffer.                             */
            /* ** NOTE ** This is a work around for the fact that the   */
            /*            Name Buffer in the Object Info Structure is   */
            /*            set to '64' bytes may result in the truncation*/
            /*            of file names.                                */
            if((OTPInfoPtr != NULL) && (HeaderPtr->Header_Value.UnicodeText.DataLength > sizeof(ObjectInfoPtr->Name)))
            {
               /* The Name had to be truncated in order to fit into the */
               /* Name Buffer.  Check to see if there is already an     */
               /* Extended Name Buffer.  If so free it before allocating*/
               /* a new one.                                            */
               if(OTPInfoPtr->ExtendedNameBuffer != NULL)
               {
                  BTPS_FreeMemory(OTPInfoPtr->ExtendedNameBuffer);
                  OTPInfoPtr->ExtendedNameBuffer = NULL;
               }

               /* Now attempt to allocate a buffer to hold the entire   */
               /* name.                                                 */
               if((OTPInfoPtr->ExtendedNameBuffer = (char *)BTPS_AllocateMemory(HeaderPtr->Header_Value.UnicodeText.DataLength)) != NULL)
               {
                  /* Now set the FieldMask to indicate that this is an  */
                  /* extended Name.                                     */
                  ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_EXTENDED_NAME;

                  /* Copy the Name into the allocated buffer.           */
                  BTPS_MemCopy(OTPInfoPtr->ExtendedNameBuffer, (char *)HeaderPtr->Header_Value.UnicodeText.ValuePointer, HeaderPtr->Header_Value.UnicodeText.DataLength);

                  /* Next store the pointer to the end of the Object    */
                  /* Info Name buffer.                                  */
                  ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(ObjectInfoPtr->Name[sizeof(ObjectInfoPtr->Name) - sizeof(NonAlignedDWord_t)]), OTPInfoPtr->ExtendedNameBuffer);

                  /* Finally '/0' terminate the Name buffer before the  */
                  /* pointer, and change the name length.               */
                  ObjectInfoPtr->Name[sizeof(ObjectInfoPtr->Name) - sizeof(NonAlignedDWord_t) - 1] = 0;
                  ObjectInfoPtr->NameLength                                                        = (sizeof(ObjectInfoPtr->Name) - sizeof(NonAlignedDWord_t));
               }
            }
         }
      }

      /* Check for the existence of a Length Header.                    */
      if((i = FindHeader(hidLength, ListPtr)) >= 0)
      {
         /* The Length header ID is followed by a 4 byte value that     */
         /* represents the Length of the Object that is to be           */
         /* transferred.                                                */
         ObjectInfoPtr->Size       = (int)(&ListPtr->Headers[i])->Header_Value.FourByteValue;
         ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_SIZE;
      }

      /* Check for the existence of a Type field Header.                */
      if((i = FindHeader(hidType, ListPtr)) >= 0)
      {
         /* The Type field exists and poses the save programming problem*/
         /* as the Name field.                                          */
         HeaderPtr                 = &ListPtr->Headers[i];

         ObjectInfoPtr->TypeLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
         ObjectInfoPtr->Type[0]    = 0;

         if(ObjectInfoPtr->TypeLength)
         {
            ObjectInfoPtr->FieldMask  |= OTP_OBJECT_INFO_MASK_TYPE;
            if(HeaderPtr->Header_Value.ByteSequence.DataLength > sizeof(ObjectInfoPtr->Type))
            {
               /* Protect against going out of bounds.                  */
               ObjectInfoPtr->TypeLength = sizeof(ObjectInfoPtr->Type);
            }

            BTPS_MemCopy(ObjectInfoPtr->Type, (char *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, ObjectInfoPtr->TypeLength);

            /* In case the Input String was truncated, we need to make  */
            /* sure that the Type String is NULL terminated.            */
            ObjectInfoPtr->Type[sizeof(ObjectInfoPtr->Type) - 1] = 0;
         }
      }

      /* Check for the existence of the Time Header.                    */
      if((i = FindHeader(hidTime, ListPtr)) >= 0)
      {
         /* The Time information is a character sequence in an OBEX ISO */
         /* Time format.  The information contained in this header is   */
         /* extracted from the Time string sequence and placed in a     */
         /* Structure that contains fields for each information item    */
         /* that is in the byte sequence.                               */
         if(!ExtractTimeDate((char *)(&ListPtr->Headers[i])->Header_Value.ByteSequence.ValuePointer, &ObjectInfoPtr->Modified))
            ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_MODIFIED;
      }
   }
}

   /* The following function is used to extract Application Parameter   */
   /* header information from an OBEX Request.  The function takes as   */
   /* its parameter a pointer to a structure that contains fields that  */
   /* correlate to the various header information that may exist.  The  */
   /* second parameter is a pointer to the Parameter information.  The  */
   /* Header Information is passed with the Header ID followed by a 2   */
   /* Byte length of the parameter data then followed by the parameter  */
   /* information.  Parameter information is in the Tag-Length-Value    */
   /* format.                                                           */
static void ExtractSyncRequestParams(OTP_Sync_Request_Params_t *SyncParamsPtr, OBEX_Header_t *HeaderPtr)
{
   int     DataLength;
   int     TLV_Size;
   Byte_t *DataPtr;

   /* Verify that the pointers provided are not NULL.                   */
   if((SyncParamsPtr) && (HeaderPtr))
   {
      /* Initialize all fields in the Request Parameter structure.      */
      SyncParamsPtr->HardDelete                 = FALSE;
      SyncParamsPtr->SyncAnchor.ChangeCountUsed = FALSE;
      SyncParamsPtr->SyncAnchor.TimestampUsed   = FALSE;
      SyncParamsPtr->SyncAnchor.ChangeCount     = 0;

      /* Ensure the header information that was passed to us is the     */
      /* header information that we expected.                           */
      if(HeaderPtr->OBEX_Header_ID == hidApplicationParameters)
      {
         /* Initialize the Length and pointer to the Parameter data.    */
         DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
         DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
         while(DataLength > 0)
         {
            switch(*DataPtr)
            {
               case OTP_SYNC_REQUEST_TAG_EXPECTED_CHANGE_COUNTER:
                  /* Convert the string information to and integer.     */
                  SyncParamsPtr->SyncAnchor.ChangeCountUsed = TRUE;
                  SyncParamsPtr->SyncAnchor.ChangeCount     = ExtractInt((char *)(DataPtr+2), (int)(*(Byte_t *)(DataPtr+1)));
                  break;
               case OTP_SYNC_REQUEST_TAG_HARD_DELETE:
                  /* This is a simple flag.  If it exists, then we need */
                  /* to set the parameter to TRUE.                      */
                  SyncParamsPtr->HardDelete = TRUE;
                  break;
            }

            /* Adjust the pointer and count for the next parameter.     */
            TLV_Size    = *(DataPtr+1)+2;
            DataLength -= TLV_Size;
            DataPtr    += TLV_Size;
         }
      }
   }
}

   /* The following function is used to extract Application Parameter   */
   /* header information from an OBEX Request.  The function takes as   */
   /* its parameter a pointer to a structure that contains fields that  */
   /* correlate to the various header information that may exist.  The  */
   /* second parameter is a pointer to the Parameter information.  The  */
   /* Header Information is passed with the Header ID followed by a 2   */
   /* Byte length of the parameter data then followed by the parameter  */
   /* information.  Parameter information is in the Tag-Length-Value    */
   /* format.                                                           */
static void ExtractSyncResponseParams(OTP_Sync_Response_Params_t *SyncParamsPtr, OBEX_Header_t *HeaderPtr)
{
   int     DataLength;
   int     TLV_Size;
   int     UIDLength;
   Byte_t *DataPtr;

   /* Verify that the pointers provided are not NULL.                   */
   if((SyncParamsPtr) && (HeaderPtr))
   {
      /* Initialize all fields in the Request Parameter structure.      */
      SyncParamsPtr->SyncAnchor.ChangeCountUsed = FALSE;
      SyncParamsPtr->SyncAnchor.TimestampUsed   = FALSE;
      SyncParamsPtr->SyncAnchor.ChangeCount     = 0;

      BTPS_MemInitialize(&SyncParamsPtr->SyncAnchor.Timestamp, 0, sizeof(OTP_TimeDate_t));
      BTPS_MemInitialize(SyncParamsPtr->UID, 0, OTP_SYNC_UID_MAXIMUM_LENGTH);

      /* Ensure the header information that was passed to us is the     */
      /* header information that we expected.                           */
      if(HeaderPtr->OBEX_Header_ID == hidApplicationParameters)
      {
         /* Initialize the Length and pointer to the Parameter data.    */
         DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
         DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
         while(DataLength > 0)
         {
            switch(*DataPtr)
            {
               case OTP_SYNC_RESPONSE_TAG_LUID:
                  /* Check the length of the UID parameter.  Since this */
                  /* implementation limits the size of this field, we   */
                  /* need to make sure that we do not overflow the      */
                  /* field.  Also, since this is a string field, we need*/
                  /* to leave room for a NULL terminator.  Therefore,   */
                  /* the length of the UID data must not be greater than*/
                  /* 1 byte less than the memory reserved.              */
                  UIDLength = (int)(*(Byte_t *)(DataPtr+1));
                  if(UIDLength > (OTP_SYNC_UID_MAXIMUM_LENGTH-1))
                     UIDLength = OTP_SYNC_UID_MAXIMUM_LENGTH-1;

                  BTPS_MemCopy(SyncParamsPtr->UID, (DataPtr+2), UIDLength);
                  break;
               case OTP_SYNC_RESPONSE_TAG_CHANGE_COUNTER:
                  /* Convert the string data to an integer.             */
                  SyncParamsPtr->SyncAnchor.ChangeCountUsed = TRUE;
                  SyncParamsPtr->SyncAnchor.ChangeCount     = ExtractInt((char *)(DataPtr+2), (int)(*(Byte_t *)(DataPtr+1)));
                  break;
               case OTP_SYNC_RESPONSE_TAG_TIMESTAMP:
                  /* Convert the string data to the Timestamp Foemat.   */
                  SyncParamsPtr->SyncAnchor.TimestampUsed = TRUE;
                  ExtractTimeDate((char *)(DataPtr+2), &SyncParamsPtr->SyncAnchor.Timestamp);
                  break;
            }

            /* Adjust the pointer and count for the next parameter.     */
            TLV_Size    = *(DataPtr+1)+2;
            DataLength -= TLV_Size;
            DataPtr    += TLV_Size;
         }
      }
   }
}

   /* The following function is used to extract File/Folder information */
   /* from the Body or EndOfBody header information of type             */
   /* 'x-obex/folder-listing'.  The information is in an XML format and */
   /* this function will extract the information from a single XML line */
   /* of data.  The function takes as its parameters a pointer to a     */
   /* ObjectInfo_t structure that contains fields that correlate to the */
   /* data that may be present in the XML data.  The function takes as  */
   /* its second parameter a pointer to the XML formatted line of data. */
   /* The XML data is delimitated by the '<' '>' pair and is terminated */
   /* by a <CR>.                                                        */
static void ExtractFolderInfo(OTP_ObjectInfo_t *ObjectInfoPtr, char *DataPtr)
{
   int   i, j;
   char  ch;
   char *TempCharPtr;

   /* Verify that the pointer are not NULL.                             */
   if((ObjectInfoPtr) && (DataPtr))
   {
      /* Begin by setting the entire structure to NULL values.          */
      BTPS_MemInitialize(ObjectInfoPtr, 0, sizeof(OTP_ObjectInfo_t));

      /* Until we know what type of information is contained in the     */
      /* data, assume the Object Type is Unknown.                       */
      ObjectInfoPtr->ObjectType = otUnknown;

      /* A Mask is used to identify the items in the structure that     */
      /* contain valid information.  Begin by setting the mask to 0 to  */
      /* indicate there is no information in the structure.             */
      ObjectInfoPtr->FieldMask  = 0;

      /* Since the pointer to the XML string may be proceeded by white  */
      /* space or other data, begin by scanning for the occurrence of   */
      /* the beginning delimiter.  Adjust the Data Pointer to the       */
      /* character just after the delimiter.                            */
      DataPtr += (DistanceToToken(_LESS_THAN_, DataPtr) + 1);

      /* If this is a valid XML line that contains information about a  */
      /* file or folder, then we should be pointing to the word File or */
      /* folder.                                                        */
      if(BTPS_MemCompare(DataPtr, "file ", 5) == 0)
      {
         /* Denote this as a File Type Object.                          */
         ObjectInfoPtr->ObjectType = otFile;
      }
      else
      {
         if(BTPS_MemCompare(DataPtr, "folder ", 7) == 0)
         {
            /* Denote this as a Folder Type Object.                     */
            ObjectInfoPtr->ObjectType = otFolder;
         }
      }

      /* If the Object Type is still not known, then this is an error,  */
      /* since we are trying to obtain information about Files and      */
      /* folders only here.                                             */
      if(ObjectInfoPtr->ObjectType != otUnknown)
      {
         /* Scan past this field information to the 1st white space past*/
         /* this data.  Exit if an error occurred.                      */
         i = DistanceToToken(_SPACE_, DataPtr);
         if(i >= 0)
         {
            /* Adjust the pointer to this white space character.        */
            DataPtr += i;

            /* Scan to the 1st non-white space.  This will be the       */
            /* beginning of the next field in the string.  If the end of*/
            /* the string is reached before we reach the non-white space*/
            /* character, then we will exit.  This will occur when we   */
            /* are processing directory information that is received in */
            /* a number of OBEX packets because the amount of directory */
            /* information is greater than the Max OBEX Packet size that*/
            /* is allowed.  In this case, we may have a portion of a    */
            /* line of information an the remaining portion of the line */
            /* will be in the next packet to be received.  This will be */
            /* handled outside of this routine.                         */
            while((i = DistanceToToken(_NON_SPACE_, DataPtr)) >= 0)
            {
               /* Adjust the pointer to this non-white space character. */
               DataPtr += i;

               /* The field can be a number of different fields since   */
               /* there is no assigned order or requirement for the     */
               /* presence of any field.  To assist in determining what */
               /* field we have located, we will check the 1st character*/
               /* of the field name an process accordingly.             */
               ch = *DataPtr;

               /* If the character is a forward slash, then this is     */
               /* found just before the terminating delimiter, so this  */
               /* is considered the end of the line of data.            */
               if(ch != '/')
               {
                  /* We know that we have found the beginning of a field*/
                  /* but we are not guaranteed that the entire field    */
                  /* will be present due to packet segmentation by OBEX.*/
                  /* Before we process any more, check to see of we can */
                  /* locate a Quote character.  All data in the XML     */
                  /* String will be contained in a set of Quotes If a   */
                  /* Quote is not located, we will exit here.           */
                  i = DistanceToToken(_QUOTE_, DataPtr) + 1;
                  if(i <= 0)
                  {
                     break;
                  }

                  /* Adjust the pointer to the character just past the  */
                  /* Quote character.  Again, we need to look for the   */
                  /* terminating Quote to ensure we have all the        */
                  /* information for the field before we process it.    */
                  /* Note that if the terminating Quote is found, then  */
                  /* the variable 'i' defines the length of the field.  */
                  DataPtr += i;

                  i = DistanceToToken(_QUOTE_, DataPtr);
                  if(i <= 0)
                  {
                     break;
                  }

                  switch(ch)
                  {
                     case 'n':
                        /* We have found the Name information.  Since we*/
                        /* have reserved a fixed amount of memory for   */
                        /* the name field, check the length of the data */
                        /* and truncate the field if it is too long.    */
                        /* When testing the length, allow for a NULL    */
                        /* terminator that will be added to the end of  */
                        /* the Name.                                    */
                        ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_NAME;

                        j = (i >= sizeof(ObjectInfoPtr->Name))?(sizeof(ObjectInfoPtr->Name)-1):i;
                        BTPS_MemCopy(ObjectInfoPtr->Name, DataPtr, j);

                        /* Add a NULL terminator for the name and save  */
                        /* the length of the Name, minus the NULL.      */
                        ObjectInfoPtr->Name[j]    = 0;
                        ObjectInfoPtr->NameLength = j;

                        /* Check to see if the Name had to be truncated */
                        /* in order to get it to fit in the buffer.     */
                        /* ** NOTE ** This is a work around for the fact*/
                        /*            that the Name Buffer in the Object*/
                        /*            Info Structure is set to '64'     */
                        /*            bytes may result in the truncation*/
                        /*            of file names.                    */
                        if(i >= sizeof(ObjectInfoPtr->Name))
                        {
                           /* Now attempt to allocate a buffer to hold  */
                           /* the entire name.                          */
                           if((TempCharPtr = (char *)BTPS_AllocateMemory(i+sizeof(char))) != NULL)
                           {
                              /* Now set the FieldMask to indicate that */
                              /* this is an extended Name.              */
                              ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_EXTENDED_NAME;

                              /* Copy the Name into the allocated buffer*/
                              /* and '/0' terminate the string.         */
                              BTPS_MemCopy(TempCharPtr, (char *)DataPtr, i);
                              TempCharPtr[i] = '\0';

                              /* Next store the pointer to the end of   */
                              /* the Object Info Name buffer.           */
                              ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(ObjectInfoPtr->Name[sizeof(ObjectInfoPtr->Name) - sizeof(NonAlignedDWord_t)]), TempCharPtr);

                              /* Finally '/0' terminate the Name buffer */
                              /* before the pointer, and change the name*/
                              /* length.                                */
                              ObjectInfoPtr->Name[sizeof(ObjectInfoPtr->Name) - sizeof(NonAlignedDWord_t) - sizeof(char)] = '\0';
                              ObjectInfoPtr->NameLength                                                                   = (sizeof(ObjectInfoPtr->Name) - sizeof(NonAlignedDWord_t));
                           }
                        }
                        break;
                     case 's':
                        /* We have found the Size field.  The size is   */
                        /* represented as an ASCII string of the decimal*/
                        /* size.  Extract the value from string and save*/
                        /* the information in the structure.            */
                        ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_SIZE;
                        ObjectInfoPtr->Size       = ExtractInt(DataPtr, i);
                        break;
                     case 'm':
                        /* We have located the modified field.  The     */
                        /* modified accesses and created fields are all */
                        /* formatted in the OBEX ISO Time format.       */
                        /* Extract the time and data information and    */
                        /* place the data in the structure.             */
                        ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_MODIFIED;
                        ExtractTimeDate(DataPtr, &ObjectInfoPtr->Modified);
                        break;
                     case 'c':
                        /* We have located the created field.  Extract  */
                        /* the information and save in the structure.   */
                        ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_CREATED;
                        ExtractTimeDate(DataPtr, &ObjectInfoPtr->Created);
                        break;
                     case 'a':
                        /* We have located the accessed field.  Extract */
                        /* the information and save in the structure.   */
                        ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_ACCESSED;
                        ExtractTimeDate(DataPtr, &ObjectInfoPtr->Accessed);
                        break;
                     case 'u':
                        /* We have located the User Permission field.   */
                        /* The permissions can be a R,W or D and are not*/
                        /* case sensitive.  Before we process anything, */
                        /* check the length of the field to make sure it*/
                        /* is not 0.                                    */
                        if(i)
                        {
                           ObjectInfoPtr->FieldMask  |= OTP_OBJECT_INFO_MASK_USER_PERMISSION;
                           ObjectInfoPtr->Permission |= (Word_t)ExtractPermissionInfo(DataPtr, i);
                        }
                        break;
                     case 'g':
                        if(DataPtr[5] == '-')
                        {
                           /* We have located the Group Permission      */
                           /* field.  The permissions can be a R,W or D */
                           /* and are not case sensitive.  Before we    */
                           /* process anything, check the length of the */
                           /* field to make sure it is not 0.           */
                           if(i)
                           {
                              ObjectInfoPtr->FieldMask  |= OTP_OBJECT_INFO_MASK_GROUP_PERMISSION;
                              ObjectInfoPtr->Permission |= (Word_t)(ExtractPermissionInfo(DataPtr, i) << 4);
                           }
                        }
                        else
                        {
                           /* We have found the Group name information. */
                           /* Since we have reserved a fixed amount of  */
                           /* memory for the owner field, check the     */
                           /* length of the data and truncate the field */
                           /* if it is too long.  When testing the      */
                           /* length, allow for a NULL terminator that  */
                           /* will be added to the end of the Group.    */
                           ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_GROUP;

                           j = (i >= sizeof(ObjectInfoPtr->Group))?(sizeof(ObjectInfoPtr->Group)-1):i;
                           BTPS_MemCopy(ObjectInfoPtr->Group, DataPtr, j);

                           /* Add a NULL terminator for the owner and   */
                           /* save the length of the Group, minus the   */
                           /* NULL.                                     */
                           ObjectInfoPtr->Group[j]    = 0;
                           ObjectInfoPtr->GroupLength = j;
                        }
                        break;
                     case 'o':
                        if(DataPtr[1] == 'w')
                        {
                           /* We have located the Owner Permission      */
                           /* field.  The permissions can be a R,W or D */
                           /* and are not case sensitive.  Before we    */
                           /* process anything, check the length of the */
                           /* field to make sure it is not 0.           */
                           if(i)
                           {
                              ObjectInfoPtr->FieldMask  |= OTP_OBJECT_INFO_MASK_OTHER_PERMISSION;
                              ObjectInfoPtr->Permission |= (Word_t)(ExtractPermissionInfo(DataPtr, i) << 8);
                           }
                        }
                        else
                        {
                           /* We have found the Owner information.      */
                           /* Since we have reserved a fixed amount of  */
                           /* memory for the owner field, check the     */
                           /* length of the data and truncate the field */
                           /* if it is too long.  When testing the      */
                           /* length, allow for a NULL terminator that  */
                           /* will be added to the end of the Owner.    */
                           ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_OWNER;

                           j = (i >= sizeof(ObjectInfoPtr->Owner))?(sizeof(ObjectInfoPtr->Owner)-1):i;
                           BTPS_MemCopy(ObjectInfoPtr->Owner, DataPtr, j);

                           /* Add a NULL terminator for the owner and   */
                           /* save the length of the Owner, minus the   */
                           /* NULL.                                     */
                           ObjectInfoPtr->Owner[j]    = 0;
                           ObjectInfoPtr->OwnerLength = j;
                        }
                        break;
                     case 'x':
                        /* The XML Language information is not used in  */
                        /* this implementation.                         */
                        break;
                     case 't':
                        /* The Type field contains a text description of*/
                        /* the file type.  This information will be     */
                        /* truncated if it does not fit in the memory   */
                        /* that was reserved for the field.             */
                        ObjectInfoPtr->FieldMask |= OTP_OBJECT_INFO_MASK_TYPE;

                        j = (i >= sizeof(ObjectInfoPtr->Type))?(sizeof(ObjectInfoPtr->Type)-1):i;
                        BTPS_MemCopy(ObjectInfoPtr->Type, DataPtr, j);

                        ObjectInfoPtr->Type[j]    = 0;
                        ObjectInfoPtr->TypeLength = j;
                        break;
                  }

                  /* Increment the count by one to place the pointer    */
                  /* just past the Quote Character.                     */
                  DataPtr += (i + 1);
               }
               else
               {
                  /* The forward slash is the end of the information    */
                  /* string.                                            */
                  break;
               }
            }
         }
      }
   }
}

   /* The following function is responsible for extracting the Name from*/
   /* the Object Information.  The only parameter to this function is   */
   /* the Object Information structure in which the Name member exists  */
   /* to be extracted.  This function returns a pointer to the Name     */
   /* member upon successful execution or NULL upon all errors.         */
   /* * NOTE * This function exists to work around a limitation with the*/
   /*          size of the Name member in the Object Information        */
   /*          structure.                                               */
static char *ExtractNameFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr)
{
   char *ret_val;

   /* First check to make sure that the parameter passed in appears to  */
   /* be at least semi-valid.                                           */
   if(ObjectInfoPtr != NULL)
   {
      /* The parameter passed in appears to be at least semi-valid.     */
      /* Next check to see if the Extended Name Bit is set in the Field */
      /* Mask.                                                          */
      if(ObjectInfoPtr->FieldMask & OTP_OBJECT_INFO_MASK_EXTENDED_NAME)
      {
         /* The Extended Name Bit is set, extract the pointer to the    */
         /* Name from the end of the Name member in the Object          */
         /* Information Structure.                                      */
         ret_val = (char *)READ_OBJECT_INFO_EXTENDED_NAME(ObjectInfoPtr->Name);
      }
      else
      {
         /* The Extended Name Bit is not set, simply set the return     */
         /* value to the start of the Name member in the Object         */
         /* Information Structure.                                      */
         ret_val = ObjectInfoPtr->Name;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is responsible for extracting the Name     */
   /* Length from the Object Information.  The first parameter to this  */
   /* function is the Object Information structure in which the Name    */
   /* Length member exists to be extracted.  The second parameter to    */
   /* this function is boolean indicating if the length of '/0'         */
   /* character at the end of the string should be included in the legth*/
   /* in the case of an extended name.  This function returns a the Name*/
   /* Length value upon successful execution or zero upon all errors.   */
   /* * NOTE * This function exists to work around a limitation with the*/
   /*          size of the Name member in the Object Information        */
   /*          structure.                                               */
static unsigned int ExtractNameLengthFromObjectInfo(OTP_ObjectInfo_t *ObjectInfoPtr, Boolean_t AddNullLength)
{
   char         *TempCharPtr;
   unsigned int  ret_val;

   /* First check to make sure that the parameter passed in appears to  */
   /* be at least semi-valid.                                           */
   if(ObjectInfoPtr != NULL)
   {
      /* The parameter passed in appears to be at least semi-valid.     */
      /* Next check to see if the Extended Name Bit is set in the Field */
      /* Mask.                                                          */
      if(ObjectInfoPtr->FieldMask & OTP_OBJECT_INFO_MASK_EXTENDED_NAME)
      {
         /* The Extended Name Bit is set, extract the pointer to the    */
         /* Name from the end of the Name member in the Object          */
         /* Information Structure.                                      */
         TempCharPtr = (char *)READ_OBJECT_INFO_EXTENDED_NAME(ObjectInfoPtr->Name);

         /* Set the return value to the length of the name.             */
         ret_val = BTPS_StringLength(TempCharPtr);

         /* Check to see if the Name Length is supposed to include the  */
         /* length of the '/0' character.                               */
         if(AddNullLength)
            ret_val += sizeof(char);
      }
      else
      {
         /* The Extended Name Bit is not set, simply set the return     */
         /* value to the NameLength member in the Object Information    */
         /* Structure.                                                  */
         ret_val = ObjectInfoPtr->NameLength;
      }
   }
   else
      ret_val = 0;

   return(ret_val);
}

   /* The following is a utility function that is used to assist in     */
   /* building a XML File Structure object.  In building the object some*/
   /* operations are performed over and over.  This function performs   */
   /* some of those functions that are required for each entry that is  */
   /* built.  The function copies string data that is passed in the Data*/
   /* parameter to memory that is indirectly pointed to by the Dest     */
   /* parameter.  The DataSize parameter defines the amount of memory   */
   /* that is available in the Destination buffer and the amount of data*/
   /* to be moved is defined in the Size parameter.  The SegmentedSize  */
   /* parameter specifies how many Bytes of the specified string has    */
   /* already been loaded (string has been segmented over OBEX packets).*/
   /* When SegmentedSize is negative, then NO data has previously been  */
   /* loaded into a previous buffer and sent (the same is true when     */
   /* SegmentedSize is zero, however, when SegmentedSize is zero, this  */
   /* means that prior to this, zero Bytes were unable to be sent).     */
   /* ** NOTE ** This function copies as much data as possible into the */
   /*            supplied Buffer.  This function returns zero if ALL of */
   /*            the data was copied.  If part (or none) of the data    */
   /*            was able to be copied, then this function returns the  */
   /*            number of bytes that were able to be copied !!!!!!!!!! */
static long LoadXMLData(char **Dest, unsigned int *DestSize, char *Data, long Size, int SegmentedSize)
{
   long ret_val = 0;
   long tmpSize;

   /* First, let's verify that all Data Parameters are correct.         */
   if((Dest) && (DestSize) && (Data) && (Size) && (SegmentedSize < Size))
   {
      if(SegmentedSize < 0)
         SegmentedSize = 0;

      /* Next, let's calculate how much remaining data we need to copy  */
      /* into the buffer (and adjust the Start Buffer Pointer by the    */
      /* amount that has already been copied).                          */
      tmpSize  = (Size - SegmentedSize);
      Data    += SegmentedSize;

      /* Now, see if the available buffer space is large enough to      */
      /* contain the data that is being moved.  If it is not, then we   */
      /* only need to copy what we can.                                 */
      if(*DestSize < (unsigned int)tmpSize)
         tmpSize = *DestSize;

      /* Now physically copy the calculated memory into the Buffer.     */
      BTPS_MemCopy(*Dest, Data, tmpSize);

      /* Note that the Destination buffer is passed in as a pointer to a*/
      /* pointer.  Also the amount of data that remains in the          */
      /* destination buffer is also passed in as a pointer.  This is    */
      /* done so that when this function returns to that caller, Dest   */
      /* will indirectly point to the next free area of buffer, just    */
      /* passed the information that was just inserted.  This will      */
      /* assist in the insertion of the next field.  The size of        */
      /* available buffer will also be adjusted when this function      */
      /* returns to the caller.  Update the pointer and the Size.       */
      *Dest     += tmpSize;
      *DestSize -= tmpSize;

      /* Flag the number of bytes that have been copied up to this point*/
      /* (taking into account that we might have already sent some data */
      /* which needs to be added to the total count sent).  We need to  */
      /* return a negative number if all Bytes were copied).            */
      ret_val = ((SegmentedSize + tmpSize) == Size)?(-1):(SegmentedSize + tmpSize);
   }
   else
   {
      /* Invalid parameters, flag that NO Parameters were copied.       */
      ret_val = 0;
   }

   return(ret_val);
}

   /* The following function is used to convert information about a     */
   /* directory structure that is passed in as an array of directory    */
   /* entries, into a File Structure Object in XML format.  The number  */
   /* of directory entries that are contained in the FileFolderInfo     */
   /* array is passed in along with a pointer to the Info structure.    */
   /* The FileFolderInfo structure contains information about files and */
   /* sub-subdirectories.  The XML object is built in a Buffer the is   */
   /* indirectly pointed to by Buffer and the amount of available memory*/
   /* in the Buffer is specified in the SizeOfBuffer parameter.  If the */
   /* amount of memory is not large enough to build the entire object,  */
   /* then this function will have to be called again to build the rest */
   /* of the object.  If the directory structure is not the Root        */
   /* directory, then a Parent directory must exist.  Indicating that   */
   /* this is not the Root Directory, will cause a entry to be placed in*/
   /* the Object indicating this.  The Root Directory parameter only has*/
   /* significance if the Segmentation State (sixth parameter) is of    */
   /* a state that is less than ssRootDirectory.  The final two         */
   /* parameters to this function store the current Segmentation State  */
   /* Information.  The sixth parameters specified what portion of the  */
   /* response is currently being processed, and the final parameter    */
   /* (SegmentedLineLength) specifies what portion of the current       */
   /* Segmentation state has been processed.                            */
   /* * NOTE * This function should be called initially with the        */
   /*          Segmentation State set to ssStart, and the Segmented     */
   /*          Line Length parameter set to zero.  After this, the      */
   /*          user should NOT modify these two parameters, and let     */
   /*          this function modify the state information.  This        */
   /*          function needs to be continually called (to send the     */
   /*          next packet segment) until this function returns that    */
   /*          the state is ssComplete.                                 */
static int FileObjectToXML(unsigned int NumberOfEntries, OTP_ObjectInfo_t FileFolderInfo[], unsigned int SizeOfBuffer, char **Buffer, Boolean_t RootDirectory, SegmentationState_t *SegmentationState, long *SegmentedLineLength)
{
   int       NumberEntriesProcessed;
   char      TmpBuffer[20];
   Boolean_t Done;

   /* Check to make sure all parameters appear valid.                   */
   if((SizeOfBuffer) && (Buffer) && (SegmentationState) && (SegmentedLineLength))
   {
      /* Flag that there was no error detected.                         */
      NumberEntriesProcessed = 0;

      /* Check to see what state we are in for building the packet.     */
      /* * NOTE * We are allowing ANY portion of the packet to be       */
      /*          segmented (this includes Header, Directory, Listing,  */
      /*          etc.).                                                */
      /* * NOTE * The general schema is as follows:                     */
      /*             - Send the Header Information for the specified    */
      /*               Item.  If all of the Header Information was sent */
      /*               then change the state to send the next Item.  If */
      /*               Header Information still exists, then leave the  */
      /*               State the same, and next time through this       */
      /*               function we will continue where we left off.     */
      /*             - Since the State only changes when we have not    */
      /*               segmented a line, then we can simply set in a    */
      /*               big if() listing to determine what (if anything  */
      /*               we should send).                                 */
      if(*SegmentationState == ssStart)
      {
         /* It is completely possible that we need to segment the       */
         /* Folder Listing Header because it is so large.  This will    */
         /* be denoted when the Continued flag is FALSE AND the         */
         /* Segmented Line Length is NOT Zero (when it is zero, then    */
         /* we need to send the header).                                */

         /* Since this is the first time for building the Object, begin */
         /* by inserting the Object Header information into the buffer. */
         *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)FolderListingHeader, sizeof(FolderListingHeader)-1, *SegmentedLineLength);

         /* If we have sent ALL of the Folder Listing Header, then we   */
         /* need to flag that the next portion to send is the Root      */
         /* Directory Information.                                      */
         if(*SegmentedLineLength < 0)
            *SegmentationState = ssRootDirectory;
      }

      /* Next, check to see if we are either in the processing of       */
      /* sending (or are sending) the Root Directory Flag.              */
      if(*SegmentationState == ssRootDirectory)
      {
         /* Check to see if the directory is the Root (if it is not     */
         /* then we will skip this processing and simply move to the    */
         /* state where we are processing the Directory Listings        */
         /* themselves.                                                 */
         if(RootDirectory == FALSE)
         {
            /* If this is not the Root directory, then a parent         */
            /* directory must exist.  Insert a tag in the object to     */
            /* indicate the presence of a parent directory.             */
            *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)ParentFolderTag, sizeof(ParentFolderTag)-1, *SegmentedLineLength);

            /* If the entire Directory Listing Name Header and the Root */
            /* Directory Header (and value) was sent, then we need to   */
            /* start sending the Directory Listings themselves.         */
            if(*SegmentedLineLength < 0)
               *SegmentationState = ssDirectoryListingName;
         }
         else
            *SegmentationState = ssDirectoryListingName;
      }

      /* Finally check to see if we are in the Directory Listing State. */
      if(*SegmentationState >= ssDirectoryListingName)
      {
         /* If we are processing and Empty Directory, then the File     */
         /* Folder Info pointer will be NULL.  If this pointer is NULL, */
         /* then the NumberOfEntries must be 0.  Set the NumberOfEntries*/
         /* to Zero if the pointer is NULL to ensure that we do not try */
         /* to use the pointer to access date.                          */
         if(FileFolderInfo == NULL)
            NumberOfEntries = 0;

         /* For large directory structures, there is a possibility that */
         /* the entire object will not fit in the amount of memory that */
         /* is available.  In this situation, we will build as much of  */
         /* the object as we can and keep track of the number of entries*/
         /* of the array that we have formatted.  Loop though the array */
         /* any process each entry one at a time.                       */
         Done = FALSE;
         while((NumberOfEntries) && (!Done))
         {
            if(*SegmentationState == ssDirectoryListingName)
            {
               /* Check to see if a Name was specified.                 */
               /* * NOTE * The Name is required so if it is not         */
               /*          present then we will bail out of this        */
               /*          entry.                                       */
               if((FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_NAME) && (ExtractNameLengthFromObjectInfo(FileFolderInfo, FALSE) > 0))
               {
                  /* If the current entry information is for a Folder,  */
                  /* then insert a tag denoting a Folder entry.  If the */
                  /* entry is a file, then format the file tag.         */
                  if(FileFolderInfo->ObjectType == otFolder)
                     *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)FolderNameHeader, sizeof(FolderNameHeader)-1, *SegmentedLineLength);
                  else
                     *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)FileNameHeader, sizeof(FileNameHeader)-1, *SegmentedLineLength);

                  /* If the entire Directory List Name Header has been  */
                  /* sent then we need to change the state such that    */
                  /* we will send the Directory List Name Value Next.   */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingNameValue;
               }
               else
               {
                  /* Since there is no name specified skip the entry    */
                  /* (and leave the state where it is).                 */
                  NumberOfEntries--;
                  NumberEntriesProcessed++;
               }
            }

            /* If we have written out the Directory Name Header and/or  */
            /* part of the Directory Listing Name value then we need to */
            /* send the Directory List Name (or the next portion of it).*/
            if(*SegmentationState == ssDirectoryListingNameValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, ExtractNameFromObjectInfo(FileFolderInfo), ExtractNameLengthFromObjectInfo(FileFolderInfo, FALSE), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Name was sent, then we need to send the Directory     */
               /* Listing Size Next.                                    */
               if(*SegmentedLineLength < 0)
               {
                  /* If we are formatting a Folder Type, then skip over */
                  /* sending the size.                                  */
                  *SegmentationState = (FileFolderInfo->ObjectType == otFolder)?ssDirectoryListingType:ssDirectoryListingSize;
               }
            }

            /* Check to see if we are in the state where we are to check*/
            /* to see if we need to send the Directory Listing Size.    */
            /* Note if the Size is not specified then we will simply    */
            /* skip to the next Directory Listing Item to send.         */
            if(*SegmentationState == ssDirectoryListingSize)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_SIZE)
               {
                  /* If the entry is a File, then a size is associated  */
                  /* with it.  A folder is not associated with a Size.  */
                  if(FileFolderInfo->ObjectType == otFile)
                  {
                     *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)SizeHeader, sizeof(SizeHeader)-1, *SegmentedLineLength);

                     /* If the remaining portion of the Directory       */
                     /* Listing Size Header was sent, then we need to   */
                     /* send the Directory Listing Size Value Next.     */
                     if(*SegmentedLineLength < 0)
                        *SegmentationState = ssDirectoryListingSizeValue;
                  }
                  else
                     *SegmentationState = ssDirectoryListingType;
               }
               else
                  *SegmentationState = ssDirectoryListingType;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Size Value.                        */
            if(*SegmentationState == ssDirectoryListingSizeValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%lu", FileFolderInfo->Size), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing Size*/
               /* was sent, then we need to send the Directory Listing  */
               /* Type Next.                                            */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingType;
            }

            /* Check to see if we are in the state where we are to check*/
            /* to see if we need to send the Directory Listing Type.    */
            /* Note if the Type is not specified then we will simply    */
            /* skip to the next Directory Listing Item to send.         */
            if(*SegmentationState == ssDirectoryListingType)
            {
               if((FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_TYPE) && (FileFolderInfo->TypeLength <= sizeof(FileFolderInfo->Type)))
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)TypeHeader, sizeof(TypeHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Type Header was sent, then we need to send the     */
                  /* Directory Listing Type Value Next.                 */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingTypeValue;
               }
               else
                  *SegmentationState = ssDirectoryListingModified;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Type Value.                        */
            if(*SegmentationState == ssDirectoryListingTypeValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, FileFolderInfo->Type, FileFolderInfo->TypeLength, *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing Type*/
               /* Value was sent, then we need to send the Directory    */
               /* Listing Modified Date Next.                           */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingModified;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Modified Date.  Note if the Modified Date is not         */
            /* specified then we will simply skip to the next Directory */
            /* Entry Item to send.                                      */
            if(*SegmentationState == ssDirectoryListingModified)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_MODIFIED)
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)ModifiedHeader, sizeof(ModifiedHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Modified Date Header was sent, then we need to send*/
                  /* the Modified Date Value Next.                      */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingModifiedValue;
               }
               else
                  *SegmentationState = ssDirectoryListingCreated;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Modified Date Value.               */
            if(*SegmentationState == ssDirectoryListingModifiedValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%04d%02d%02dT%02d%02d%02d%s", FileFolderInfo->Modified.Year, FileFolderInfo->Modified.Month, FileFolderInfo->Modified.Day, FileFolderInfo->Modified.Hour, FileFolderInfo->Modified.Minute, FileFolderInfo->Modified.Second, ((FileFolderInfo->Modified.UTC_Time)?"Z":"")), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Modified Date Value was sent, then we need to send the*/
               /* Directory Listing Created Date Next.                  */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingCreated;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Created Date.  Note if the Created Date is not specified */
            /* then we will simply skip to the next Directory Entry     */
            /* Item to send.                                            */
            if(*SegmentationState == ssDirectoryListingCreated)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_CREATED)
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)CreatedHeader, sizeof(CreatedHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Created Date Header was sent, then we need to send */
                  /* the Created Date Value Next.                       */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingCreatedValue;
               }
               else
                  *SegmentationState = ssDirectoryListingAccessed;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Created Date Value.                */
            if(*SegmentationState == ssDirectoryListingCreatedValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%04d%02d%02dT%02d%02d%02d%s", FileFolderInfo->Created.Year, FileFolderInfo->Created.Month, FileFolderInfo->Created.Day, FileFolderInfo->Created.Hour, FileFolderInfo->Created.Minute, FileFolderInfo->Created.Second, ((FileFolderInfo->Created.UTC_Time)?"Z":"")), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Created Date Value was sent, then we need to send the */
               /* Directory Listing Accessed Date Next.                 */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingAccessed;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Accessed Date.  Note if the Created Date is not specified*/
            /* then we will simply skip to the next Directory Entry     */
            /* Item to send.                                            */
            if(*SegmentationState == ssDirectoryListingAccessed)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_ACCESSED)
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)AccessedHeader, sizeof(AccessedHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Accessed Date Header was sent, then we need to send*/
                  /* the Accessed Date Value Next.                      */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingAccessedValue;
               }
               else
                  *SegmentationState = ssDirectoryListingUserPermission;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Accessed Date Value.               */
            if(*SegmentationState == ssDirectoryListingAccessedValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%04d%02d%02dT%02d%02d%02d%s", FileFolderInfo->Accessed.Year, FileFolderInfo->Accessed.Month, FileFolderInfo->Accessed.Day, FileFolderInfo->Accessed.Hour, FileFolderInfo->Accessed.Minute, FileFolderInfo->Accessed.Second, ((FileFolderInfo->Accessed.UTC_Time)?"Z":"")), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Accessed Date Value was sent, then we need to send the*/
               /* Directory Listing User Permission Information next.   */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingUserPermission;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Permissions.  Note that if the User Permission Item is   */
            /* not specified then we will skip to the Next Permission   */
            /* Item to check to send.                                   */
            /* * NOTE * There are really three Permission types, so we  */
            /*          will check to send them in the following order  */
            /*          (User, Group, and Other).  The Order is         */
            /*          completely arbitrary and serves NO purpose other*/
            /*          than the logic inside of this function.         */
            if(*SegmentationState == ssDirectoryListingUserPermission)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_USER_PERMISSION)
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)UserPermHeader, sizeof(UserPermHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* User Permission Header was sent, then we need to   */
                  /* send the User Permission Value Next.               */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingUserPermissionValue;
               }
               else
                  *SegmentationState = ssDirectoryListingGroupPermission;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing User Permissions Value.            */
            if(*SegmentationState == ssDirectoryListingUserPermissionValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%s%s%s", ((FileFolderInfo->Permission & OTP_USER_PERMISSION_READ)?"R":""), ((FileFolderInfo->Permission & OTP_USER_PERMISSION_WRITE)?"W":""), ((FileFolderInfo->Permission & OTP_USER_PERMISSION_DELETE)?"D":"")), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing User*/
               /* Permission Value was sent, then we need to send the   */
               /* Directory Listing Group Permission Information next.  */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingGroupPermission;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Permissions (Group Permissions).  Note that if the Group */
            /* Permissions Information is not specified then we will    */
            /* simply skip ahead to the next Permissions to be checked. */
            if(*SegmentationState == ssDirectoryListingGroupPermission)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_GROUP_PERMISSION)
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)GroupPermHeader, sizeof(GroupPermHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Group Permission Header was sent, then we need to  */
                  /* send the Group Permission Value Next.              */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingGroupPermissionValue;
               }
               else
                  *SegmentationState = ssDirectoryListingOtherPermission;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Group Permissions Value.           */
            if(*SegmentationState == ssDirectoryListingGroupPermissionValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%s%s%s", ((FileFolderInfo->Permission & OTP_GROUP_PERMISSION_READ)?"R":""), ((FileFolderInfo->Permission & OTP_GROUP_PERMISSION_WRITE)?"W":""), ((FileFolderInfo->Permission & OTP_GROUP_PERMISSION_DELETE)?"D":"")), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Group Permission Value was sent, then we need to send */
               /* the Directory Listing Other Permission Information    */
               /* next.                                                 */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingOtherPermission;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Permissions (Other Permissions).  Note that if the       */
            /* Other Permissions Infromation is not specified then we   */
            /* will skip the processing of the Other Permissions and    */
            /* move to the next Directory Listing Item to check for.    */
            if(*SegmentationState == ssDirectoryListingOtherPermission)
            {
               if(FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_OTHER_PERMISSION)
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)OtherPermHeader, sizeof(OtherPermHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Other Permission Header was sent, then we need to  */
                  /* send the Other Permission Value Next.              */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingOtherPermissionValue;
               }
               else
                  *SegmentationState = ssDirectoryListingOwner;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Other Permissions Value.           */
            if(*SegmentationState == ssDirectoryListingOtherPermissionValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, TmpBuffer, BTPS_SprintF(TmpBuffer, "%s%s%s", ((FileFolderInfo->Permission & OTP_OTHER_PERMISSION_READ)?"R":""), ((FileFolderInfo->Permission & OTP_OTHER_PERMISSION_WRITE)?"W":""), ((FileFolderInfo->Permission & OTP_OTHER_PERMISSION_DELETE)?"D":"")), *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Other Permission Value was sent, then we need to send */
               /* the Directory Listing Owner Information Next.         */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingOwner;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Owner.  Note if the Owner Information is note specified  */
            /* the we will simply skip ahead to the next Directory      */
            /* Listing Item to check.                                   */
            if(*SegmentationState == ssDirectoryListingOwner)
            {
               if((FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_OWNER) && (FileFolderInfo->OwnerLength <= sizeof(FileFolderInfo->Owner)))
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)OwnerHeader, sizeof(OwnerHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Owner Information Header was sent, then we need to */
                  /* send the Owner Infromation Value Next.             */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingOwnerValue;
               }
               else
                  *SegmentationState = ssDirectoryListingGroup;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Owner Information Value.           */
            if(*SegmentationState == ssDirectoryListingOwnerValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, FileFolderInfo->Owner, FileFolderInfo->OwnerLength, *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Owner Information Value was sent, then we need to send*/
               /* the Directory Listing Group Information Next.         */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssDirectoryListingGroup;
            }

            /* Check to see if we are in the state where we are to      */
            /* check to see if we need to send the Directory Listing    */
            /* Group.  Note that if the Group Information is not        */
            /* specified then we will move on to the termination of     */
            /* the current entry we are processing so that we can move  */
            /* to the next entry.                                       */
            if(*SegmentationState == ssDirectoryListingGroup)
            {
               if((FileFolderInfo->FieldMask & OTP_OBJECT_INFO_MASK_GROUP) && (FileFolderInfo->GroupLength <= sizeof(FileFolderInfo->Group)))
               {
                  *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)GroupHeader, sizeof(GroupHeader)-1, *SegmentedLineLength);

                  /* If the remaining portion of the Directory Listing  */
                  /* Group Information Header was sent, then we need to */
                  /* send the Group Infromation Value Next.             */
                  if(*SegmentedLineLength < 0)
                     *SegmentationState = ssDirectoryListingGroupValue;
               }
               else
                  *SegmentationState = ssLineTerminator;
            }

            /* Check to see if we are in a state where we need to send  */
            /* the Directory Listing Group Information Value.           */
            if(*SegmentationState == ssDirectoryListingGroupValue)
            {
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, FileFolderInfo->Group, FileFolderInfo->GroupLength, *SegmentedLineLength);

               /* If the remaining portion of the Directory Listing     */
               /* Owner Information Value was sent, then we need to     */
               /* move on to the termination of the current entry       */
               /* because we are finished with the current entry.       */
               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssLineTerminator;
            }

            /* Check to see if we have completed the current Entry OR   */
            /* we need to terminate the XML Entry.                      */
            /* * NOTE * If we get this far and there has been NO        */
            /*          Segmentation, then we *MUST* be at the end of   */
            /*          the Directory Entry Listing.                    */
            if((*SegmentedLineLength < 0) || (*SegmentationState == ssLineTerminator))
            {
               /* Flag that we are in the Line Terminator Segmentation  */
               /* State (in case we aren't already in this state).      */
               *SegmentationState = ssLineTerminator;

               /* The entry needs to be terminated when all of the      */
               /* fields have been formatted.                           */
               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)LineTerminator, sizeof(LineTerminator)-1, *SegmentedLineLength);

               /* If we have successfully terminated the current        */
               /* XML Directory Entry then we need to advance to the    */
               /* next Directory Entry so that we can process it.       */
               if(*SegmentedLineLength < 0)
               {
                  /* Advance to the pointer to the next entry.          */
                  FileFolderInfo++;

                  NumberOfEntries--;
                  NumberEntriesProcessed++;

                  *SegmentationState = ssDirectoryListingName;
               }
            }

            /* If we have filled up the existing Buffer then we need    */
            /* to exit out of the Loop.                                 */
            if(*SegmentedLineLength >= 0)
               Done = TRUE;
         }

         /* If the number of entries remaining to be processed is Zero  */
         /* AND we have looped back around to the Directory Listing     */
         /* Name State (this signifies that we have processed ALL       */
         /* entries) then we need to send the Footer.  We will simply   */
         /* enter a new state and start sending the Footer.  When we    */
         /* have sent the entire Footer we simply flag the State as     */
         /* Complete and we are finished.                               */
         /* * NOTE * We could also already be in the Footer State.  This*/
         /*          would occur if have sent part of the Footer and    */
         /*          need to send another portion of the segment.       */
         if(NumberOfEntries == 0)
         {
            if((*SegmentationState == ssDirectoryListingName) || (*SegmentationState == ssFooter))
            {
               *SegmentationState = ssFooter;

               *SegmentedLineLength = LoadXMLData(Buffer, &SizeOfBuffer, (char *)FolderListingFooter, sizeof(FolderListingFooter)-1, *SegmentedLineLength);

               if(*SegmentedLineLength < 0)
                  *SegmentationState = ssComplete;
            }
         }
      }
   }
   else
      NumberEntriesProcessed = INVALID_PARAMETER;

   return(NumberEntriesProcessed);
}

   /* The following function is used to test a line of XML formatted    */
   /* directory information to determine if the line contains           */
   /* information about a File or Folder.  During the process of testing*/
   /* for the File/Folder information the function also checks to see if*/
   /* the line indicates the presence of a Parent folder in the data.   */
   /* This indicates that we are not at the Root directory.             */
static Boolean_t IsFileFolderEntry(char *DataPtr, Boolean_t *ParentFolder)
{
   Boolean_t ret_val;

   /* Assume that this is not File/Folder information.                  */
   ret_val = FALSE;

   /* Check to make sure the pointer to the data is not NULL.           */
   if(DataPtr)
   {
      /* Scan past all white space characters.                          */
      while(IS_WHITE_SPACE_CHARACTER(*DataPtr))
         DataPtr++;

      /* Each line of information will begin with a '<' character.      */
      if(*DataPtr == '<')
      {
         /* If the data contains File information, the line will begin  */
         /* with 'file name' or 'folder name'.  Check for the specific  */
         /* string of characters.                                       */
         DataPtr++;
         if(BTPS_MemCompare(DataPtr, "file ", 5) == 0)
            ret_val = TRUE;
         else
         {
            if(BTPS_MemCompare(DataPtr, "folder ", 7) == 0)
               ret_val = TRUE;
            else
            {
               /* If this is not File or Folder information, then check */
               /* to see if it defines a Parent Directory.              */
               if((ParentFolder) && (!*ParentFolder))
               {
                  if(BTPS_MemCompare(DataPtr, "parent-folder", 13) == 0)
                     *ParentFolder = TRUE;
               }
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is used to extract File/Folder information */
   /* from an XML formatted data stream.  Due to OBEX segmentation,     */
   /* partial lines of information may exist at the end of the buffer,  */
   /* and special handling must be performed to handle this situation.  */
   /* This routine provides 2 functions.  If the routine is called with */
   /* the ObjectInfo pointer set to NULL, the routine totals the number */
   /* of File/Folder entries that are contained in the buffer of data.  */
   /* When the ObjectInfo pointer is not NULL, File/Folder data is      */
   /* extracted from the Data Buffer and placed in the ObjectInfo       */
   /* structure.  The Segment pointer value is used to pass information */
   /* to the routine to handle OBEX segmentation.  if a partial line of */
   /* information remained from the last packet, the Segment pointer    */
   /* will point to the remaining information.  The Parent Folder       */
   /* pointer is used to pass information about the existence of a      */
   /* Parent Folder tag in the data.                                    */
static int XMLToFileObject(unsigned int *NumberOfEntries, OTP_ObjectInfo_t *ObjectInfo, char **Segment, char *Buffer, Boolean_t *ParentFolder)
{
   int   ret_val = 0;
   int   SegmentLength;
   char *TmpBuffer;

   /* Check to make sure the pointers are not NULL.                     */
   if((NumberOfEntries) && (Buffer))
   {
      /* Initialize the Number of Entries found to 0.                   */
      *NumberOfEntries = 0;

      /* Check to see if there was a segment of information left over   */
      /* from the last packet.  If this pointer is not NULL, then there */
      /* was some partial information at the end of the last packet that*/
      /* could not be processed until the remaining information arrived */
      /* with this packet.  The idea here is to combine the data left   */
      /* from the last packet with the data that is at the beginning of */
      /* this packet to form a complete line of information.  Since the */
      /* parsing routine needs the data in a contiguous format, the     */
      /* information pieces will need to be moved to an are of memory   */
      /* that can contain both segments.                                */
      if(*Segment)
      {
         /* Check the Object Info Pointer.  If the pointer is not NULL, */
         /* then this function has been called to extract the data from */
         /* a complete segment.  The segment has been completely built  */
         /* from past calls in the attempt to calculate the number of   */
         /* entries in the segment.                                     */
         if(ObjectInfo)
         {
            /* If the Object Info pointer is defined, extract the data  */
            /* from the XML line.                                       */
            if(IsFileFolderEntry(*Segment, ParentFolder))
            {
               /* Extract the information from the line of data.        */
               ExtractFolderInfo(ObjectInfo, *Segment);

               /* Now that this ObjectInfo struct is used, advance the  */
               /* pointer to the next structure.                        */
               ObjectInfo++;

               /* Increment the count of the number of lines of         */
               /* information we have seen or processed.                */
               (*NumberOfEntries)++;
            }

            /* We are finished with the data segment, so release the    */
            /* memory and NULL the pointer.                             */
            BTPS_FreeMemory(*Segment);
            *Segment = NULL;

            /* Since we are here, we have received the last segment of  */
            /* XML data to create a full line of information and the    */
            /* line of data compiled while counting the number of       */
            /* entries in this XML data block.  This means that the     */
            /* current buffer must contain this final segment of data at*/
            /* the beginning of the buffer.  We need to advance the     */
            /* pointer to the beginning of the next line before we try  */
            /* to process any more lines of data.                       */
         }
         else
         {
            /* If we are here then we are calculating the number of     */
            /* entries and we have a segment left over from the previous*/
            /* scan of XML data.  The task at hand is to continue to    */
            /* build a string in 'Segment' until 1 full line is held in */
            /* the buffer.  Determine the length of the old segment by  */
            /* scanning for the NULL terminator that was placed at the  */
            /* end of the data.                                         */
            SegmentLength = DistanceToToken(_NULL_, *Segment);

            /* The remaining data will be at the beginning of the new   */
            /* buffer and will continue to the '>'.  Determine the      */
            /* amount of data that makes up this segment.  If we fail to*/
            /* find the terminator, we must concatenate what we         */
            /* currently have and wait for the next packet.             */
            ret_val = DistanceToToken(_GREATER_THAN_, (char *)Buffer);
            if(ret_val >= 0)
            {
               /* The value ret_val is the distance to the target       */
               /* character, and is 1 less than the number of characters*/
               /* that make up the string.  Increment the distance to   */
               /* obtain the number of characters in the buffer to      */
               /* complete the full line.                               */
               ret_val++;

               /* Create a temporary buffer that can contain both       */
               /* segments and a NULL terminator.                       */
               TmpBuffer = (char *)BTPS_AllocateMemory(SegmentLength+ret_val+1);
               if(TmpBuffer)
               {
                  /* Copy each segment into the buffer to form a        */
                  /* complete line of XML data.                         */
                  BTPS_MemCopy(TmpBuffer, *Segment, SegmentLength);
                  BTPS_MemCopy(&TmpBuffer[SegmentLength], Buffer, ret_val);

                  /* Place a NULL terminator at the end of the data to  */
                  /* help prevent us from going out of bounds.          */
                  TmpBuffer[SegmentLength+ret_val] = 0;

                  /* Release the data that holds the original segment   */
                  /* and assign the Segment Pointer to the new buffer of*/
                  /* data.                                              */
                  BTPS_FreeMemory(*Segment);
                  *Segment = TmpBuffer;

                  /* Check to see if this information contains any file */
                  /* or folder information.                             */
                  if(IsFileFolderEntry(*Segment, ParentFolder))
                  {
                     /* Increment the count of the number of lines of   */
                     /* information we have located in the XML data.    */
                     /* The Segment now contains a full line of data and*/
                     /* this line will be processed when this function  */
                     /* is called again with a valid ObjectInfo pointer.*/
                     (*NumberOfEntries)++;
                  }
                  else
                  {
                     /* If the data in the segment does not contain a   */
                     /* Directory entry, we can discard the data now.   */
                     BTPS_FreeMemory(*Segment);
                     *Segment = NULL;
                  }
               }
               else
               {
                  /* We were not able to allocate the memory for the    */
                  /* temporary buffer, so we are in trouble.  Release   */
                  /* the data for the segment and set an error result   */
                  /* code.                                              */
                  BTPS_FreeMemory(*Segment);
                  *Segment = NULL;
                  ret_val  = -1;
               }
            }
            else
            {
               /* The new buffer data still does not contain all of the */
               /* data to complete the line.  So, we need to concatenate*/
               /* the new segment with the existing segment and wait for*/
               /* the next buffer of data.                              */
               ret_val = DistanceToToken(_NULL_, (char *)Buffer)+1;

               /* Create a temporary buffer that can contain both       */
               /* segments and a NULL terminator.                       */
               TmpBuffer = (char *)BTPS_AllocateMemory(SegmentLength+ret_val+1);
               if(TmpBuffer)
               {
                  /* Copy each segment into the buffer to form a new    */
                  /* segment of XML data.                               */
                  BTPS_MemCopy(TmpBuffer, *Segment, SegmentLength);
                  BTPS_MemCopy(&TmpBuffer[SegmentLength], Buffer, ret_val);

                  /* Place a NULL terminator at the end of the data to  */
                  /* help prevent us from going out of bounds.          */
                  TmpBuffer[SegmentLength+ret_val] = 0;

                  /* Release the data for the original segment and set  */
                  /* the Segment pointer to the new buffer area.        */
                  BTPS_FreeMemory(*Segment);
                  *Segment = TmpBuffer;
               }
               else
               {
                  /* We were not able to allocate the memory for the    */
                  /* temporary buffer, so we are in trouble.  Release   */
                  /* the data for the segment and set an error result   */
                  /* code.                                              */
                  BTPS_FreeMemory(*Segment);
                  *Segment = NULL;
               }

               /* We still do not have a full line of data, so set the  */
               /* return value to allow the routine to exit.            */
               ret_val  = -1;
            }
         }
      }

      /* Check to see if we were processing a previous segment of XML   */
      /* data and either have not compiled a complete line of XML data  */
      /* or an error occurred.  In either case, there is no further data*/
      /* to process.                                                    */
      if(ret_val != -1)
      {
         /* If we are here, we are ready to process data in the buffer  */
         /* that is not associated with the line in 'Segment'.  At this */
         /* point we should always be trying to locate the starting     */
         /* delimiter of the string, which is a '<'.                    */
         ret_val = DistanceToToken(_LESS_THAN_, (char *)Buffer);
         if(ret_val != -1)
            Buffer += ret_val;

         /* Check to see if the Start of Line character was located.    */
         if(ret_val != -1)
         {
            /* Buffer points to a string segment that begins with a '<' */
            /* character.  This segment may contain a full or partial   */
            /* line.  Process this line accordingly.                    */
            do
            {
               /* Check to see if we have a full line by checking to see*/
               /* if the terminating delimitor can be located in the    */
               /* current buffer.                                       */
               ret_val = DistanceToToken(_GREATER_THAN_, (char *)Buffer);
               if(ret_val >= 0)
               {
                  /* Check to see if the line contains information about*/
                  /* a File or Folder.                                  */
                  if(IsFileFolderEntry((char *)Buffer, ParentFolder))
                  {
                     /* Check to see if we are processing the data or   */
                     /* totalling the number of lines that contain the  */
                     /* File/Folder information.                        */
                     if(ObjectInfo)
                     {
                        /* Extract the information from the line of XML */
                        /* data.                                        */
                        ExtractFolderInfo(ObjectInfo, Buffer);

                        /* Advance to the next free ObjectInfo          */
                        /* Structure.                                   */
                        ObjectInfo++;
                     }

                     /* Total number of lines that were found or        */
                     /* processed.                                      */
                     (*NumberOfEntries)++;
                  }

                  /* We should currently be at the delimiter for the    */
                  /* beginning of the line.  Advanced the pointer to the*/
                  /* end of line delimiter.                             */
                  Buffer += ret_val;

                  /* Check to see if we can find he delimiter of the    */
                  /* next line of data.                                 */
                  ret_val = DistanceToToken(_LESS_THAN_, (char *)Buffer);
                  if(ret_val >= 0)
                  {
                     /* Since we have found the next beginning, set the */
                     /* pointer to the beginning of the next line.      */
                     Buffer  = (char *)(Buffer + ret_val);
                  }
               }
               else
               {
                  /* We were not able to locate the end of the string,  */
                  /* so we must be at the end of the buffer and we will */
                  /* have to wait for the next packet of data to process*/
                  /* continue to process this line.  Since we cannot    */
                  /* find an end to this line, we need to decide what to*/
                  /* do with the partial line that exists at the end of */
                  /* the buffer.  The idea is to have the last call to  */
                  /* this function that processes the current buffer,   */
                  /* save the data in the Segment buffer.  To determine */
                  /* if this is the last time we will be called to      */
                  /* process this data, we need to examine some         */
                  /* information.  This will be the last call if we have*/
                  /* found no entries.  If entries were found, then this*/
                  /* will be the last call if ObjectInfo is a valid     */
                  /* pointer.  It will never be the case where the      */
                  /* Number of Entries is 0 and ObjectInfo is valid.    */
                  if((*NumberOfEntries == 0) || (ObjectInfo))
                  {
                     /* Determine how much data remains to the end of   */
                     /* the buffer, and add 1 for a Null terminator.    */
                     ret_val  = DistanceToToken(_NULL_, (char *)Buffer)+1;

                     /* Allocate memory to hold the segment of data and */
                     /* copy the data to the memory.  This information  */
                     /* will be used when the next packet is received in*/
                     /* order to complete the line of data.             */
                     if(*Segment)
                        BTPS_FreeMemory(*Segment);

                     *Segment = (char *)BTPS_AllocateMemory(ret_val);
                     if(*Segment)
                        BTPS_MemCopy(*Segment, Buffer, ret_val);

                     /* Flag the end has been reached,                  */
                     ret_val = 0;
                  }
               }
            }
            while(ret_val > 0);
         }
      }
   }

   return(ret_val);
}

   /* The following function processes GOEP Event callbacks from the    */
   /* Bluetooth Stack.                                                  */
static void BTPSAPI GOEP_Event_Callback(GOEP_Event_Data_t *GOEP_Event_Data, unsigned long CallbackParameter)
{
   Byte_t              ResponseCode;
   OTPM_Info_t        *OTPInfoPtr;
   unsigned int        OTP_ID;
   OBEX_Header_t       Header;
   OBEX_Header_List_t  HeaderList;

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter:\n"));

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(GOEP_Event_Data)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* When the GOEP Port was opened, the FTPM Mgr ID to        */
            /* reference the connection was provided as the callback    */
            /* parameter.                                               */
            OTP_ID     = (unsigned int)CallbackParameter;

            /* Search the lined list for the information about this     */
            /* connection.                                              */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* Check what event has occurred and process the event.  */
               switch(GOEP_Event_Data->Event_Data_Type)
               {
                  case etOBEX_Port_Open_Request_Indication:
                     /* This is a request to connect to a registered    */
                     /* server.                                         */
                     OTP_PortOpenRequestIndicationEvent(OTPInfoPtr, (OBEX_Port_Open_Request_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Port_Open_Request_Indication_Data);
                     break;
                  case etOBEX_Port_Open_Indication:
                     /* This is a connection to a registered server.    */
                     OTP_PortOpenIndicationEvent(OTPInfoPtr, (OBEX_Port_Open_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Port_Open_Indication_Data);
                     break;
                  case etOBEX_Port_Open_Confirmation:
                     /* This is a Client Connection to a remote server. */
                     OTP_PortOpenConfirmationEvent(OTPInfoPtr, (OBEX_Port_Open_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Port_Open_Confirmation_Data);

                     /* If the Port failed to open, then we need to     */
                     /* remove any reference to the FTPM Mgr Entry.     */
                     if(GOEP_Event_Data->Event_Data.OBEX_Port_Open_Confirmation_Data->PortOpenStatus != GOEP_OPEN_PORT_STATUS_SUCCESS)
                     {
                        if((OTPInfoPtr = DeleteOTPInfoEntry(&OTPInfoList, OTP_ID)) != NULL)
                           FreeOTPInfoEntryMemory(OTPInfoPtr);
                     }
                     break;
                  case etOBEX_Port_Close_Indication:
                     /* An open port have not been closed by the remote */
                     /* entity.                                         */
                     OTP_PortCloseIndicationEvent(OTPInfoPtr, (OBEX_Port_Close_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Port_Close_Indication_Data);

                     /* Search the list again for information about the */
                     /* connection.  If the user closed the port in the */
                     /* callback, this information would not exist now  */
                     /* and would make our previous pointer invalid.    */
                     OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
                     if(OTPInfoPtr)
                     {
                        /* Check to see if this is a Server Port.  If   */
                        /* not, remove the information about this       */
                        /* connection from the list.                    */
                        if(OTPInfoPtr->Server == FALSE)
                        {
                           if((OTPInfoPtr = DeleteOTPInfoEntry(&OTPInfoList, OTP_ID)) != NULL)
                              FreeOTPInfoEntryMemory(OTPInfoPtr);
                        }
                     }
                     break;
                  case etOBEX_Connect_Indication:
                     /* This is an OBEX Connection request from a remote*/
                     /* Client.  The Upper layer user will be required  */
                     /* to accept the connection request.               */
                     OTP_ConnectRequestEvent(OTPInfoPtr, (OBEX_Connect_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Connect_Indication_Data);
                     break;
                  case etOBEX_Connect_Confirmation:
                     /* This is confirmation that a connect request that*/
                     /* was sent to a remote server have been answered. */
                     /* The information in that callback might indicate */
                     /* that the connection was not accepted.           */
                     OTP_ConnectResponseEvent(OTPInfoPtr, (OBEX_Connect_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Connect_Confirmation_Data);
                     break;
                  case etOBEX_Disconnect_Indication:
                     /* This is a request from a remote client to       */
                     /* disconnect the OBEX Connection.                 */
                     OTP_DisconnectRequestEvent(OTPInfoPtr, (OBEX_Disconnect_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Disconnect_Indication_Data);
                     break;
                  case etOBEX_Disconnect_Confirmation:
                     /* This is Response that an OBEX Connection to a   */
                     /* remote server has been disconnected.            */
                     OTP_DisconnectResponseEvent(OTPInfoPtr, (OBEX_Disconnect_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Disconnect_Confirmation_Data);
                     break;
                  case etOBEX_Put_Indication:
                     /* This is a request from a remote Client to Save  */
                     /* and object on our side.  Check to make sure we  */
                     /* are in a correct state to receive this command. */
                     if((OTPInfoPtr->ServerState != ssGet) && (OTPInfoPtr->ServerState != ssGetSetup))
                     {
                        /* Handle the Put Request.                      */
                        if(OTPInfoPtr->ServerState == ssIdle)
                        {
                           /* Since this is the 1st Put request, clear  */
                           /* the Mask that identifies what information */
                           /* is known about the object to be received. */
                           OTPInfoPtr->ObjectInfo.FieldMask = OTP_OBJECT_INFO_MASK_CLEAR;
                        }

                        OTP_PutRequestEvent(OTPInfoPtr, (OBEX_Put_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Put_Indication_Data);
                     }
                     else
                     {
                        /* Service can not be accessed at this time.    */
                        _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_METHOD_NOT_ALLOWED_RESPONSE | OBEX_FINAL_BIT), NULL);
                     }
                     break;
                  case etOBEX_Put_Confirmation:
                     /* This is an Response that the previous data sent */
                     /* to a remote server has been saved.              */
                     OTP_PutResponseEvent(OTPInfoPtr, (OBEX_Put_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Put_Confirmation_Data);
                     break;
                  case etOBEX_Get_Indication:
                     OTP_GetRequestEvent(OTPInfoPtr, (OBEX_Get_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Get_Indication_Data);
                     break;
                  case etOBEX_Get_Confirmation:
                     OTP_GetResponseEvent(OTPInfoPtr, (OBEX_Get_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Get_Confirmation_Data);

                     /* Make sure the port was not closed during the    */
                     /* callback.                                       */
                     OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
                     if(OTPInfoPtr)
                     {
                        /* Check the state of the Client.  If the state */
                        /* is not Idle, then we are continuing with the */
                        /* Get operation.  If the user called Abort     */
                        /* during the callback, the current operation   */
                        /* would be idle.                               */
                        if(OTPInfoPtr->CurrentOperation != OTP_OPERATION_NONE)
                        {
                           ResponseCode = (Byte_t)(GOEP_Event_Data->Event_Data.OBEX_Get_Confirmation_Data->Response_Code & ~OBEX_FINAL_BIT);
                           if(ResponseCode == OBEX_CONTINUE_RESPONSE)
                           {
                              if(OTPInfoPtr->Target == tFileBrowser)
                              {
                                 HeaderList.NumberOfHeaders = 1;
                                 HeaderList.Headers         = &Header;

                                 if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
                                 {
                                    Header.OBEX_Header_ID                         = hidConnectionID;
                                    Header.OBEX_Header_Type                       = htUnsignedInteger4Byte;
                                    Header.Header_Value.FourByteValue             = OTPInfoPtr->ConnectionID;
                                 }
                                 else
                                 {
                                    Header.OBEX_Header_ID                         = hidTarget;
                                    Header.OBEX_Header_Type                       = htByteSequence;
                                    Header.Header_Value.ByteSequence.DataLength   = sizeof(FolderBrowseUUID);
                                    Header.Header_Value.ByteSequence.ValuePointer = (Byte_t *)FolderBrowseUUID;
                                 }

                                 _GOEP_Get_Request(OTPInfoPtr->GOEP_ID, TRUE, &HeaderList);
                              }
                              else
                                 _GOEP_Get_Request(OTPInfoPtr->GOEP_ID, TRUE, NULL);
                           }
                        }
                        else
                           OTPInfoPtr = NULL;
                     }
                     else
                        OTPInfoPtr = NULL;
                     break;
                  case etOBEX_Set_Path_Indication:
                     if(OTPInfoPtr->ServerState == ssIdle)
                        OTP_SetPathRequestEvent(OTPInfoPtr, (OBEX_Set_Path_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Set_Path_Indication_Data);
                     break;
                  case etOBEX_Set_Path_Confirmation:
                     OTP_SetPathResponseEvent(OTPInfoPtr, (OBEX_Set_Path_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Set_Path_Confirmation_Data);
                     break;
                  case etOBEX_Abort_Indication:
                     OTPInfoPtr->ServerState          = ssIdle;
                     OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                     OTPInfoPtr->ObjectInfo.FieldMask = 0;

                     OTP_AbortRequestEvent(OTPInfoPtr, (OBEX_Abort_Indication_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Abort_Indication_Data);
                     break;
                  case etOBEX_Abort_Confirmation:
                     OTPInfoPtr->ServerState          = ssIdle;
                     OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                     OTPInfoPtr->ObjectInfo.FieldMask = 0;

                     OTP_AbortResponseEvent(OTPInfoPtr, (OBEX_Abort_Confirmation_Data_t *)GOEP_Event_Data->Event_Data.OBEX_Abort_Confirmation_Data);
                     break;
               }
            }
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
   }

   DebugPrint((BTPM_DEBUG_ZONE_FTP | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit:\n"));
}

   /* The following function is used to handle an Request of a          */
   /* connection to a local server.  This will allow the upper layer to */
   /* prepare for a future open indication.  The OTPInfoPtr points to   */
   /* information about the local server.  The Port Open Request        */
   /* structure has been received from the GOEP layer and contains      */
   /* information about the connection.                                 */
static void OTP_PortOpenRequestIndicationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Open_Request_Indication_Data_t *Port_Open_Request_Indication_Data)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_PORT_OPEN_INDICATION_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the pointers and information passed in is valid.      */
   if((OTPInfoPtr) && (Port_Open_Request_Indication_Data))
   {
      /* If a callback is registered for this port, call the User.      */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* Change the port connection state to indicate that there is a*/
         /* connection on the port pending.                             */
         OTPInfoPtr->PortConnectionState = pcsConnectionPending;

         /* Call the user to inform them that a remote client has just  */
         /* request to connect to the port.                             */
         EventData                                                            = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                                           = etOTP_Port_Open_Request_Indication;
         EventData->Event_Data_Size                                           = OTP_PORT_OPEN_REQUEST_INDICATION_DATA_SIZE;
         EventData->Event_Data.OTP_Port_Open_Request_Indication_Data          = (OTP_Port_Open_Request_Indication_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Port_Open_Request_Indication_Data->OTP_ID  = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Port_Open_Request_Indication_Data->BD_ADDR = Port_Open_Request_Indication_Data->BD_ADDR;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function is used to handle an opening of a          */
   /* connection to a local server.  This will allow the upper layer to */
   /* prepare for a future connect request.  The OTPInfoPtr points to   */
   /* information about the local server.  The Port Open structure has  */
   /* been received from the GOEP layer and contains information about  */
   /* the connection.                                                   */
static void OTP_PortOpenIndicationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Open_Indication_Data_t *Port_Open_Indication_Data)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_PORT_OPEN_INDICATION_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the pointers and information passed in is valid.      */
   if((OTPInfoPtr) && (Port_Open_Indication_Data))
   {
      /* If a callback is registered for this port, call the User.      */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* Change the port connection state to indicate that the port  */
         /* is now currently connected.                                 */
         OTPInfoPtr->PortConnectionState = pcsConnected;

         /* Call the user to inform them that a remote client has just  */
         /* connect to the port.                                        */
         EventData                                                    = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                                   = etOTP_Port_Open_Indication;
         EventData->Event_Data_Size                                   = OTP_PORT_OPEN_INDICATION_DATA_SIZE;
         EventData->Event_Data.OTP_Port_Open_Indication_Data          = (OTP_Port_Open_Indication_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Port_Open_Indication_Data->OTP_ID  = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Port_Open_Indication_Data->BD_ADDR = Port_Open_Indication_Data->BD_ADDR;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function is used to handle the Confirmation that a  */
   /* previous connect request to a remote Server has been acknowledged.*/
static void OTP_PortOpenConfirmationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Open_Confirmation_Data_t *Port_Open_Confirmation_Data)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_PORT_OPEN_CONFIRMATION_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the pointers and information passed in is valid.      */
   if((OTPInfoPtr) && (Port_Open_Confirmation_Data))
   {
      /* If a callback is registered for this port, call the User.      */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* If the confirmation indicates success, change the port      */
         /* connection state to indicate that the port is now currently */
         /* connected.                                                  */
         if(Port_Open_Confirmation_Data->PortOpenStatus == GOEP_OPEN_PORT_STATUS_SUCCESS)
            OTPInfoPtr->PortConnectionState = pcsConnected;

         /* Inform the user of the Response to the Port Open request.   */
         EventData                                                             = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                                            = etOTP_Port_Open_Confirmation;
         EventData->Event_Data_Size                                            = OTP_PORT_OPEN_CONFIRMATION_DATA_SIZE;
         EventData->Event_Data.OTP_Port_Open_Confirmation_Data                 = (OTP_Port_Open_Confirmation_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Port_Open_Confirmation_Data->OTP_ID         = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Port_Open_Confirmation_Data->PortOpenStatus = Port_Open_Confirmation_Data->PortOpenStatus;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function is used to handle the situation where the  */
   /* remote client has closed the Server Port.                         */
static void OTP_PortCloseIndicationEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Port_Close_Indication_Data_t *Port_Close_Indication_Data)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_PORT_CLOSE_INDICATION_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the pointers and information passed in is valid.      */
   if((OTPInfoPtr) && (Port_Close_Indication_Data))
   {
      /* Since the Port could have been closed at an unopportune time,  */
      /* pass to the user the current state of the server and the User  */
      /* Information that may be present.  This will allow the user to  */
      /* cleanup any resources.                                         */
      EventData                                                              = (OTP_Event_Data_t *)Buffer;
      EventData->Event_Data_Type                                             = etOTP_Port_Close_Indication;
      EventData->Event_Data_Size                                             = OTP_PORT_CLOSE_INDICATION_DATA_SIZE;
      EventData->Event_Data.OTP_Port_Close_Indication_Data                   = (OTP_Port_Close_Indication_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
      EventData->Event_Data.OTP_Port_Close_Indication_Data->OTP_ID           = OTPInfoPtr->OTP_ID;
      EventData->Event_Data.OTP_Port_Close_Indication_Data->UserInfo         = OTPInfoPtr->UserInfo;

      /* Before we call the user, we may have some cleanup to be done.  */
      /* Check to see if a Response Buffer has been allocated.  If so   */
      /* releases the memory for the buffer.                            */
      if(OTPInfoPtr->ResponseBuffer)
      {
         BTPS_FreeMemory(OTPInfoPtr->ResponseBuffer);
         OTPInfoPtr->ResponseBuffer = NULL;
      }

      /* Check to see if we have a segment of a Directory saved for     */
      /* future processing.  If so, release the memory.                 */
      if(OTPInfoPtr->DirEntrySegment)
      {
         BTPS_FreeMemory(OTPInfoPtr->DirEntrySegment);
         OTPInfoPtr->DirEntrySegment = NULL;
      }

      if(OTPInfoPtr->ExtendedNameBuffer)
      {
         BTPS_FreeMemory(OTPInfoPtr->ExtendedNameBuffer);
         OTPInfoPtr->ExtendedNameBuffer = NULL;
      }

      /* If we are a Server, we will need to reset some state           */
      /* information back to their Idle settings.                       */
      if(OTPInfoPtr->Server)
      {
         OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
         OTPInfoPtr->UserInfo             = 0;
         OTPInfoPtr->ConnectionID         = INVALID_CONNECTION_ID;
         OTPInfoPtr->MaxPacketSize        = 0;
         OTPInfoPtr->ServerState          = ssIdle;
         OTPInfoPtr->PortConnectionState  = pcsNotConnected;
         OTPInfoPtr->ClientConnected      = FALSE;
         OTPInfoPtr->ObjectInfo.FieldMask = 0;
      }

      /* If a callback is assigned, call the User with information about*/
      /* the Port Closure.                                              */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function is used to process the request from a      */
   /* remote client to establish an OBEX connection to a local Server.  */
static void OTP_ConnectRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Connect_Indication_Data_t *Connect_Request)
{
   int                     i;
   Byte_t                  Buffer[OTP_EVENT_DATA_SIZE+OTP_CONNECT_REQUEST_DATA_SIZE];
   Boolean_t               Valid;
   OBEX_Header_t          *HeaderPtr;
   OTP_Event_Data_t       *EventData;
   OBEX_Header_List_t      HeaderList;
   OTP_Digest_Challenge_t  Challenge;
   OTP_Digest_Response_t   Response;

   /* Verify that the pointers and information passed in is valid.      */
   if((OTPInfoPtr) && (Connect_Request))
   {
      Valid = FALSE;

      /* Initialize information about this connection.                  */
      OTPInfoPtr->ServerState                 = ssIdle;
      OTPInfoPtr->CurrentOperation            = OTP_OPERATION_NONE;
      OTPInfoPtr->ClientConnected             = FALSE;
      OTPInfoPtr->UserInfo                    = 0;
      OTPInfoPtr->DirInfo.NumberEntries       = 0;
      OTPInfoPtr->DirInfo.DirectoryIndex      = 0;
      OTPInfoPtr->DirInfo.SegmentedLineLength = -1;
      OTPInfoPtr->DirInfo.ObjectInfoList      = NULL;
      OTPInfoPtr->DirInfo.SegmentationState   = ssStart;
      OTPInfoPtr->DirInfo.ParentDirectory     = FALSE;
      OTPInfoPtr->ConnectionID                = INVALID_CONNECTION_ID;
      OTPInfoPtr->ObjectInfo.FieldMask        = 0;
      OTPInfoPtr->ObjectInfo.NameLength       = 0;
      OTPInfoPtr->ObjectInfo.Name[0]          = 0;
      OTPInfoPtr->ObjectInfo.TypeLength       = 0;
      OTPInfoPtr->ObjectInfo.Type[0]          = 0;
      OTPInfoPtr->GOEP_ID                     = Connect_Request->GOEP_ID;
      OTPInfoPtr->MaxPacketSize               = Connect_Request->Max_Packet_Length;
      HeaderList                              = Connect_Request->Header_List;

      /* Allocate a Buffer the size of MaxPacket Size to build response */
      /* packets in.  If the memory can not be created, deny the        */
      /* connection.                                                    */
      if((OTPInfoPtr->GOEPPacketSize) < OTPInfoPtr->MaxPacketSize)
         OTPInfoPtr->MaxPacketSize = (Word_t)(OTPInfoPtr->GOEPPacketSize);

      OTPInfoPtr->ResponseBuffer = (Byte_t *)BTPS_AllocateMemory(OTPInfoPtr->MaxPacketSize);
      if(OTPInfoPtr->ResponseBuffer)
      {
         /* Check the headers that have arrived with the Connect Request*/
         /* for a target.  If this is a directed connection, then the   */
         /* Target Header is mandatory and should be the 1st Header.  If*/
         /* no target header is included or the target specified does   */
         /* not match the Folder Browsing UUID or Sync UUID, then accept*/
         /* the connection returning no headers.  This will indicate to */
         /* the user that the service was not located and lets the      */
         /* client decide if the connection is usable.                  */
         if((i = FindHeader(hidTarget, &HeaderList)) >= 0)
         {
            if(OTPInfoPtr->Target != tInbox)
            {
               /* The target header that we are looking for to perform  */
               /* the folder browsing, is the Folder Browsing UUID.     */
               /* Check to make sure the user supplied the correct UUID.*/
               HeaderPtr = &HeaderList.Headers[i];
               if((OTPInfoPtr->Target == tFileBrowser) && (HeaderPtr->Header_Value.ByteSequence.DataLength == sizeof(FolderBrowseUUID)))
               {
                  if(BTPS_MemCompare((void *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, (void *)FolderBrowseUUID, sizeof(FolderBrowseUUID)) == 0)
                  {
                     /* We have been requested to accept a connection to*/
                     /* the Folder Browsing Server.  Return the Folder  */
                     /* Browsing UUID in the Who header and assign a    */
                     /* Connection ID.                                  */
                     OTPInfoPtr->ConnectionID = GetNextConnection_ID();
                     Valid                    = TRUE;
                  }
               }
               else
               {
                  if((OTPInfoPtr->Target == tIRSync) && (HeaderPtr->Header_Value.ByteSequence.DataLength == sizeof(IrSyncUUID)))
                  {
                     if(BTPS_MemCompare((void *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, (void *)IrSyncUUID, sizeof(IrSyncUUID)) == 0)
                     {
                        /* We have been requested to accept a connection*/
                        /* to the IrSync Server.  Return the IrSync UUID*/
                        /* in the Who header and assign a Connection ID.*/
                        OTPInfoPtr->ConnectionID = GetNextConnection_ID();
                        Valid                    = TRUE;
                     }
                  }
               }
            }
         }
         else
         {
            /* Since there are no Headers, this must be an Inbox        */
            /* Connection.                                              */
            if(OTPInfoPtr->Target == tInbox)
            {
               Valid = TRUE;
            }
         }

         /* If this appears to be a valid request, dispatch a call to   */
         /* the user to notify them of the request.                     */
         if(Valid)
         {
            EventData                                                       = (OTP_Event_Data_t *)Buffer;
            EventData->Event_Data_Type                                      = etOTP_Connect_Request;
            EventData->Event_Data_Size                                      = OTP_CONNECT_REQUEST_DATA_SIZE;
            EventData->Event_Data.OTP_Connect_Request_Data                  = (OTP_Connect_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
            EventData->Event_Data.OTP_Connect_Request_Data->OTP_ID          = OTPInfoPtr->OTP_ID;
            EventData->Event_Data.OTP_Connect_Request_Data->Target          = OTPInfoPtr->Target;
            EventData->Event_Data.OTP_Connect_Request_Data->DigestChallenge = NULL;
            EventData->Event_Data.OTP_Connect_Request_Data->DigestResponse  = NULL;

            /* Check to see if any Authentication Headers are present in*/
            /* the request.                                             */
            if((i = FindHeader(hidAuthenticationChallenge, &HeaderList)) >= 0)
            {
               /* An Authentication Challenge is present, so extract the*/
               /* data from the header and place the information in the */
               /* Connect Request Structure.                            */
               HeaderPtr = &HeaderList.Headers[i];
               TagLengthValueToStructure(hidAuthenticationChallenge, HeaderPtr->Header_Value.ByteSequence.DataLength, (OTP_Tag_Length_Value_t *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, &Challenge);

               EventData->Event_Data.OTP_Connect_Request_Data->DigestChallenge = &Challenge;
            }
            if((i = FindHeader(hidAuthenticationResponse, &HeaderList)) >= 0)
            {
               /* An Authentication Response is present, so extract the */
               /* data from the header and place the information in the */
               /* Connect Request Structure.                            */
               HeaderPtr = &HeaderList.Headers[i];
               TagLengthValueToStructure(hidAuthenticationResponse, HeaderPtr->Header_Value.ByteSequence.DataLength, (OTP_Tag_Length_Value_t *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, &Response);

               EventData->Event_Data.OTP_Connect_Request_Data->DigestResponse = &Response;
            }

            /* Finally make the callback.                               */
            __BTPSTRY
            {
               (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
         else
         {
            /* Connection to a Service that we are not supporting.      */
            _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_BAD_REQUEST_RESPONSE | OBEX_FINAL_BIT), NULL);
         }
      }
      else
      {
         /* We were not able to allocate the memory for the response    */
         /* buffer, so we can not accept the connection request.        */
         _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_INTERNAL_SERVER_ERROR_RESPONSE | OBEX_FINAL_BIT), NULL);
      }
   }
   else
   {
      if(OTPInfoPtr)
      {
         /* We were not able to allocate all of the information that was*/
         /* needed to process the request, so we can not accept the     */
         /* connection request.                                         */
         _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_INTERNAL_SERVER_ERROR_RESPONSE | OBEX_FINAL_BIT), NULL);
      }
   }
}

   /* The following function is used to process the data from a         */
   /* Connection Request Response.  The function takes as its parameters*/
   /* the Bluetooth Stack ID for which the request was received.  The   */
   /* OTPInfoPtr parameter points to an information structure that      */
   /* contains information about the connection that is attempting to be*/
   /* established.  The Connect_Response parameter contains the response*/
   /* information associated with the connect request.                  */
static void OTP_ConnectResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Connect_Confirmation_Data_t *Connect_Response)
{
   int                     i;
   Byte_t                  Buffer[OTP_EVENT_DATA_SIZE+OTP_CONNECT_RESPONSE_DATA_SIZE];
   OBEX_Header_t          *HeaderPtr;
   OTP_Event_Data_t       *EventData;
   OBEX_Header_List_t      HeaderList;
   OTP_Digest_Challenge_t  Challenge;
   OTP_Digest_Response_t   Response;

   /* Verify that the parameters that were passed in as semi-valid.     */
   if((OTPInfoPtr) && (Connect_Response))
   {
      /* Initialize connection state information.                       */
      OTPInfoPtr->ServerState                 = ssIdle;
      OTPInfoPtr->ClientConnected             = TRUE;
      OTPInfoPtr->CurrentOperation            = OTP_OPERATION_NONE;
      OTPInfoPtr->UserInfo                    = 0;
      OTPInfoPtr->ObjectInfo.FieldMask        = 0;
      OTPInfoPtr->DirInfo.NumberEntries       = 0;
      OTPInfoPtr->DirInfo.DirectoryIndex      = 0;
      OTPInfoPtr->DirInfo.SegmentedLineLength = -1;
      OTPInfoPtr->DirInfo.ObjectInfoList      = NULL;
      OTPInfoPtr->DirInfo.ParentDirectory     = FALSE;
      OTPInfoPtr->DirInfo.SegmentationState   = ssStart;
      OTPInfoPtr->ConnectionID                = INVALID_CONNECTION_ID;
      OTPInfoPtr->GOEP_ID                     = Connect_Response->GOEP_ID;
      OTPInfoPtr->MaxPacketSize               = Connect_Response->Max_Packet_Length;
      HeaderList                              = Connect_Response->Header_List;

      /* Check to see if the Max Packet size was reduced by the remote  */
      /* entity.  If so, we need to adjust the size that we can send    */
      /* since this is a symmetric value.                               */
      if((OTPInfoPtr->GOEPPacketSize) < OTPInfoPtr->MaxPacketSize)
         OTPInfoPtr->MaxPacketSize = (Word_t)(OTPInfoPtr->GOEPPacketSize);

      /* Allocate a Buffer the size of MaxPacket Size to build response */
      /* packets in.  If the memory can not be created, deny the        */
      /* connection.                                                    */
      OTPInfoPtr->ResponseBuffer = (Byte_t *)BTPS_AllocateMemory(OTPInfoPtr->MaxPacketSize);
      if(OTPInfoPtr->ResponseBuffer)
      {
         if(OTPInfoPtr->Target != tInbox)
         {
            /* Check the headers that have arrived with the Connect     */
            /* Response for a target.  If this is a directed connection,*/
            /* then the Who and ConnectionID are mandatory and should be*/
            /* in the Header.  If no Who and ConnectionID header is     */
            /* included or the Who specified does not match the Folder  */
            /* Browsing UUID, then start the disconnect process.        */
            if((i = FindHeader(hidConnectionID, &HeaderList)) >= 0)
            {
               HeaderPtr = &HeaderList.Headers[i];
               OTPInfoPtr->ConnectionID  = HeaderPtr->Header_Value.FourByteValue;
               if((i = FindHeader(hidWho, &HeaderList)) >= 0)
               {
                  /* The Who header that we are expecting is the Folder */
                  /* Browsing UUID or IrSync.  Check to make sure the   */
                  /* Who supplied the correct UUID.                     */
                  HeaderPtr          = &HeaderList.Headers[i];
                  OTPInfoPtr->Target = tUnknown;

                  /* Check to see if the connection was targeted to the */
                  /* File Browser Service.                              */
                  if(HeaderPtr->Header_Value.ByteSequence.DataLength == sizeof(FolderBrowseUUID))
                  {
                     /* Check to see that the Who parameter supplied in */
                     /* the response matches the target used in the     */
                     /* request.                                        */
                     if(BTPS_MemCompare((void *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, (void *)FolderBrowseUUID, sizeof(FolderBrowseUUID)) == 0)
                     {
                        /* We have connected to the Folder Browsing     */
                        /* Server.                                      */
                        OTPInfoPtr->Target = tFileBrowser;
                     }
                  }
                  else
                  {
                     /* Check to see if the connection was targeted to  */
                     /* the IR Sync Service.                            */
                     if(HeaderPtr->Header_Value.ByteSequence.DataLength == sizeof(IrSyncUUID))
                     {
                        /* Check to see that the Who parameter supplied */
                        /* in the response matches the target used in   */
                        /* the request.                                 */
                        if(BTPS_MemCompare((void *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, (void *)IrSyncUUID, sizeof(IrSyncUUID)) == 0)
                        {
                           /* We have connected to the IR Sync Server.  */
                           OTPInfoPtr->Target = tIRSync;
                        }
                     }
                  }
               }
            }
            else
            {
               /* No Connection ID was located.  Either the remote      */
               /* server did not assign a Connection ID and thus we will*/
               /* have to supply the target header for each future      */
               /* request or the Target server was not available and the*/
               /* Default Inbox accepted the connection.                */
               OTPInfoPtr->ConnectionID = INVALID_CONNECTION_ID;
            }
         }
      }

      EventData                                                        = (OTP_Event_Data_t *)Buffer;
      EventData->Event_Data_Type                                       = etOTP_Connect_Response;
      EventData->Event_Data_Size                                       = OTP_CONNECT_RESPONSE_DATA_SIZE;
      EventData->Event_Data.OTP_Connect_Response_Data                  = (OTP_Connect_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
      EventData->Event_Data.OTP_Connect_Response_Data->OTP_ID          = OTPInfoPtr->OTP_ID;
      EventData->Event_Data.OTP_Connect_Response_Data->ResponseCode    = Connect_Response->Response_Code;
      EventData->Event_Data.OTP_Connect_Response_Data->Target          = OTPInfoPtr->Target;
      EventData->Event_Data.OTP_Connect_Response_Data->DigestChallenge = NULL;
      EventData->Event_Data.OTP_Connect_Response_Data->DigestResponse  = NULL;

      /* Check the response to see if the connection required           */
      /* authentication.                                                */
      if((Connect_Response->Response_Code & ~OBEX_FINAL_BIT) == OBEX_UNAUTHORIZED_RESPONSE)
      {
         /* Since the connection is being authenticated, we should find */
         /* a challenge information.                                    */
         if((i = FindHeader(hidAuthenticationChallenge, &HeaderList)) >= 0)
         {
            /* Extract the Challenge information and place the          */
            /* information in the callback information structure.       */
            HeaderPtr = &HeaderList.Headers[i];

            TagLengthValueToStructure(hidAuthenticationChallenge, HeaderPtr->Header_Value.ByteSequence.DataLength, (OTP_Tag_Length_Value_t *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, &Challenge);

            EventData->Event_Data.OTP_Connect_Response_Data->DigestChallenge = &Challenge;
         }

         if((i = FindHeader(hidAuthenticationResponse, &HeaderList)) >= 0)
         {
            /* Extract the Challenge Response information and place the */
            /* information in the callback information structure.       */
            HeaderPtr = &HeaderList.Headers[i];

            TagLengthValueToStructure(hidAuthenticationResponse, HeaderPtr->Header_Value.ByteSequence.DataLength, (OTP_Tag_Length_Value_t *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, &Response);

            EventData->Event_Data.OTP_Connect_Response_Data->DigestResponse = &Response;
         }
      }

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is used to process a Disconnect Request    */
   /* from the remote entity.  The parameters passed to this function   */
   /* include the Bluetooth Stack ID on which the request was received. */
   /* The OTPInfoPtr parameter points to an information structure that  */
   /* contains information about the connection that is attempting to be*/
   /* disconnected.  The Disconnect_Request parameter contains the      */
   /* request information associated with the disconnect request.       */
static void OTP_DisconnectRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Disconnect_Indication_Data_t *Disconnect_Request)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_DISCONNECT_REQUEST_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the parameters that were passed in as semi-valid.     */
   if((OTPInfoPtr) && (Disconnect_Request))
   {
      /* We are not allowed to deny a disconnect, so accept it.         */
      _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, OBEX_OK_RESPONSE, NULL);

      /* Since this connection is being disconnected, the Response      */
      /* buffer is no longer needed, so free then memory and invalidate */
      /* the pointer to the memory.                                     */
      if(OTPInfoPtr->ResponseBuffer)
      {
         BTPS_FreeMemory(OTPInfoPtr->ResponseBuffer);
         OTPInfoPtr->ResponseBuffer = NULL;
      }

      /* If we are the server for the connection, then we must set the  */
      /* connection status information back to the default values.      */
      if(OTPInfoPtr->Server)
      {
         OTPInfoPtr->ServerState          = ssIdle;
         OTPInfoPtr->ClientConnected      = FALSE;
         OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
         OTPInfoPtr->ConnectionID         = INVALID_CONNECTION_ID;
         OTPInfoPtr->ObjectInfo.FieldMask = 0;
         OTPInfoPtr->UserInfo             = 0;
         OTPInfoPtr->MaxPacketSize        = 0;
      }

      /* Check to see if we are holding a handle to some Directory      */
      /* information.  If so, call the user to inform them that the     */
      /* memory is no longer needed.                                    */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* Call the user to inform them that the connection has been   */
         /* terminated.                                                 */
         EventData                                                           = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                                          = etOTP_Disconnect_Request;
         EventData->Event_Data_Size                                          = OTP_DISCONNECT_REQUEST_DATA_SIZE;
         EventData->Event_Data.OTP_Disconnect_Request_Data                   = (OTP_Disconnect_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Disconnect_Request_Data->OTP_ID           = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Disconnect_Request_Data->UserInfo         = OTPInfoPtr->UserInfo;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function is used to process a Disconnect Response   */
   /* from the remote entity.  The parameters passed to this function   */
   /* include the Bluetooth Stack ID on which the response was received.*/
   /* The OTPInfoPtr parameter points to an information structure that  */
   /* contains information about the connection that is attempting to be*/
   /* disconnected.  The Disconnect_Response parameter contains the     */
   /* request information associated with the disconnect response.      */
static void OTP_DisconnectResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Disconnect_Confirmation_Data_t *Disconnect_Response)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+((OTP_DISCONNECT_RESPONSE_DATA_SIZE > OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE)?OTP_DISCONNECT_RESPONSE_DATA_SIZE:OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE)];
   unsigned int      OTP_ID;
   OTP_Event_Data_t *EventData;

   /* Verify that the parameters that were passed in as semi-valid.     */
   if((OTPInfoPtr) && (Disconnect_Response))
   {
      /* Since this connection is being disconnected, the Response      */
      /* buffer is no longer needed, so free then memory and invalidate */
      /* the pointer to the memory.                                     */
      if(OTPInfoPtr->ResponseBuffer)
      {
         BTPS_FreeMemory(OTPInfoPtr->ResponseBuffer);
         OTPInfoPtr->ResponseBuffer = NULL;
      }

      /* If we are the server for the connection, then we must set the  */
      /* connection status information back to the default values.      */
      if(OTPInfoPtr->Server)
      {
         OTPInfoPtr->ServerState          = ssIdle;
         OTPInfoPtr->ClientConnected      = FALSE;
         OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
         OTPInfoPtr->ConnectionID         = INVALID_CONNECTION_ID;
         OTPInfoPtr->ObjectInfo.FieldMask = 0;
         OTPInfoPtr->UserInfo             = 0;
         OTPInfoPtr->MaxPacketSize        = 0;
      }

      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* Store off the FTPM Mgr ID to be used later.                 */
         OTP_ID = OTPInfoPtr->OTP_ID;

         /* Check to see if we are holding a handle to some Directory   */
         /* information.  If so, call the user to inform them that the  */
         /* memory is no longer needed.                                 */
         if(OTPInfoPtr->DirInfo.ObjectInfoList)
         {
            EventData                                                                              = (OTP_Event_Data_t *)Buffer;
            EventData->Event_Data_Type                                                             = etOTP_Free_Directory_Information;
            EventData->Event_Data_Size                                                             = OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE;
            EventData->Event_Data.OTP_Free_Directory_Information_Data                              = (OTP_Free_Directory_Information_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
            EventData->Event_Data.OTP_Free_Directory_Information_Data->OTP_ID                      = OTPInfoPtr->OTP_ID;

            EventData->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.NumberEntries = OTPInfoPtr->DirInfo.DirectoryIndex;
            EventData->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo    = OTPInfoPtr->DirInfo.ObjectInfoList;

            /* Flag that there is no memory being held anymore.         */
            OTPInfoPtr->DirInfo.NumberEntries                                                      = 0;
            OTPInfoPtr->DirInfo.ObjectInfoList                                                     = NULL;

            /* Finally make the callback.                               */
            __BTPSTRY
            {
               (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* The FTPM Mgr Context Information was successfully        */
            /* retrieved, next attempt to get the FTPM Mgr Information  */
            /* Entry.                                                   */
            if((Initialized) && (SearchOTPInfoEntry(&OTPInfoList, OTP_ID) != NULL))
            {
               /* The FTPM Mgr Information Entry is still valid, in this*/
               /* case format up the event to make the callback with.   */
               EventData                                                        = (OTP_Event_Data_t *)Buffer;
               EventData->Event_Data_Type                                       = etOTP_Disconnect_Response;
               EventData->Event_Data_Size                                       = OTP_DISCONNECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Disconnect_Response_Data               = (OTP_Disconnect_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Disconnect_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Disconnect_Response_Data->ResponseCode = Disconnect_Response->Response_Code;

               /* Finally make the callback.                            */
               __BTPSTRY
               {
                  (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }
}

   /* The following function is used to process a Put Request from the  */
   /* remote entity.  The parameters passed to this function include the*/
   /* Bluetooth Stack ID on which the request was received.  The        */
   /* OTPInfoPtr parameter points to an information structure that      */
   /* contains information about the connection that is currently being */
   /* used.  The Put Request parameter contains the request information */
   /* associated with the Put Request.                                  */
static void OTP_PutRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Put_Indication_Data_t *Put_Request)
{
   int                        i;
   int                        BodyIndex, EndOfBodyIndex, AppParamIndex;
   Byte_t                     Weight;
   Boolean_t                  Valid;
   Byte_t                     Buffer[OTP_EVENT_DATA_SIZE+((OTP_PUT_SYNC_OBJECT_REQUEST_DATA_SIZE > OTP_DELETE_SYNC_OBJECT_REQUEST_DATA_SIZE)?OTP_PUT_SYNC_OBJECT_REQUEST_DATA_SIZE:OTP_DELETE_SYNC_OBJECT_REQUEST_DATA_SIZE)];
   OTP_Sync_Request_Params_t  SyncParams;
   OBEX_Header_t             *HeaderPtr;
   OTP_Event_Data_t          *EventData;
   OBEX_Header_List_t         HeaderList;

   /* Check to make sure that data that was passed in is semi-valid.    */
   if((OTPInfoPtr) && (Put_Request))
   {
      /* The first step is to determine if the header information that  */
      /* was received appears correct for the state that we are in.     */
      Valid      = TRUE;
      HeaderPtr  = NULL;
      HeaderList = Put_Request->Header_List;

      /* Check to see if a Connection ID was assigned when the original */
      /* connection was established.  If so, then we need to check to   */
      /* see if a Connection ID was provided with the request and if it */
      /* matches the one that we have saved.                            */
      if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
      {
         /* Check for a Connection ID Header.                           */
         i = FindHeader(hidConnectionID, &HeaderList);
         if(i >= 0)
         {
            /* Since a Connection ID does exist, then we need to see if */
            /* the Connection ID specified matches the Connection ID    */
            /* used in the original connect request.                    */
            HeaderPtr = &HeaderList.Headers[i];
            if(HeaderPtr->Header_Value.FourByteValue != OTPInfoPtr->ConnectionID)
            {
               /* If the Connection ID does not match, then this is an  */
               /* invalid request.                                      */
               Valid = FALSE;
            }
         }
         else
         {
            /* No Connection ID was present in the headers and one was  */
            /* assigned for this connection.  The Connection ID must be */
            /* the 1st header when used, and must be included in the 1st*/
            /* packet of the request.  If this packet is a continuation */
            /* packet, then it is not required.                         */
            if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_NONE)
               Valid = FALSE;
         }
      }

      /* If the Connection ID test passed, then continue processing the */
      /* Put request.                                                   */
      if(Valid)
      {
         /* The preliminary requirements have been met.  Initialize the */
         /* information that will be used to assist in determining what */
         /* to do with this data.                                       */
         BodyIndex      = FindHeader(hidBody, &HeaderList);
         EndOfBodyIndex = FindHeader(hidEndOfBody, &HeaderList);
         AppParamIndex  = FindHeader(hidApplicationParameters, &HeaderList);

         /* If we located Application Parameters and we are connected to*/
         /* an IRSync Target, then we need to retreive the parameters   */
         /* from the header.                                            */
         BTPS_MemInitialize(&SyncParams, 0, sizeof(OTP_Sync_Request_Params_t));
         if((AppParamIndex >= 0) && (OTPInfoPtr->Target == tIRSync))
            ExtractSyncRequestParams(&SyncParams, &HeaderList.Headers[AppParamIndex]);

         /* Set the Object Information Type based upon the target       */
         /* connection (it will either be an object or a file).         */
         if(OTPInfoPtr->Target == tFileBrowser)
            OTPInfoPtr->ObjectInfo.ObjectType = otFile;
         else
            OTPInfoPtr->ObjectInfo.ObjectType = otObject;

         /* The validity of the request will depend upon the Server     */
         /* State, Final Bit, presence of a Body header and the presence*/
         /* of an EndOfBody header.  Due to the variety of combination  */
         /* that can make a request valid or invalid, a special approach*/
         /* to identifying invalid requests will be performed.  A truth */
         /* table is made to identify each combination and each         */
         /* dependancy is assigned a weight.  By weighting each item    */
         /* then totalling the weights assigned to each, a unique number*/
         /* will be generated that will represent the current           */
         /* conditions.  This will then be used to identify valid       */
         /* combinations.                                               */
         Weight  = (Byte_t)((EndOfBodyIndex >= 0)?END_OF_BODY_WEIGHT:0);
         Weight += (Byte_t)((BodyIndex >= 0)?BODY_WEIGHT:0);
         Weight += (Byte_t)((Put_Request->Final_Flag)?FINAL_FLAG_WEIGHT:0);
         Weight += (Byte_t)((OTPInfoPtr->ServerState == ssIdle)?IDLE_STATE_WEIGHT:0);
         Weight += (Byte_t)((OTPInfoPtr->ServerState == ssPutSetup)?PUT_SETUP_STATE_WEIGHT:0);
         Weight += (Byte_t)((OTPInfoPtr->ServerState == ssPut)?PUT_STATE_WEIGHT:0);

         /* With the weighting, the Upper Nibble describes the state of */
         /* the server while the lower nibble defines the Body, End of  */
         /* Body and the Final flag.                                    */
         switch(Weight)
         {
            case 0x10: /* Idle Mode                                     */
            case 0x20: /* Put Setup Mode                                */
               /* In Idle or Put Setup state, if we have no Final flag, */
               /* Body header or End of Body header, then the packet    */
               /* should contain header information that describes the  */
               /* object that is being put.  Extract this information   */
               /* and wait for the Body of the object.                  */
               ExtractObjectInfo(OTPInfoPtr, &OTPInfoPtr->ObjectInfo, &HeaderList);

               OTPInfoPtr->ServerState              = ssPutSetup;
               OTPInfoPtr->CurrentOperation         = (Byte_t)OTP_OPERATION_PUT_OBJECT;

               _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_CONTINUE_RESPONSE | OBEX_FINAL_BIT), NULL);
               break;
            case 0x11: /* Idle and Body Header                          */
            case 0x21: /* Put Setup and Body Header                     */
               /* In the Idle and Put Setup state, a Body Header will   */
               /* indicate the start of the Object data.  Call the user */
               /* so the Object can be processed.  Since additional     */
               /* information about the object may be present in the    */
               /* headers, so extract the Header information before     */
               /* calling the user.                                     */
               ExtractObjectInfo(OTPInfoPtr, &OTPInfoPtr->ObjectInfo, &HeaderList);

               OTPInfoPtr->ServerState                                       = ssPut;
               OTPInfoPtr->CurrentOperation                                  = (Byte_t)((OTPInfoPtr->Target == tFileBrowser)?OTP_OPERATION_PUT_FILE:OTP_OPERATION_PUT_OBJECT);
               HeaderPtr                                                     = &HeaderList.Headers[BodyIndex];

               /* Check to see if we are connected to the IRSync server.*/
               /* If we are, then we could have some application        */
               /* parameters for this request.                          */
               if(OTPInfoPtr->Target == tIRSync)
               {
                  EventData                                                          = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                         = etOTP_Put_Sync_Object_Request;
                  EventData->Event_Data_Size                                         = OTP_PUT_SYNC_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data             = (OTP_Put_Sync_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_FIRST;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
               }
               else
               {
                  EventData                                                     = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                    = etOTP_Put_Object_Request;
                  EventData->Event_Data_Size                                    = OTP_PUT_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Put_Object_Request_Data             = (OTP_Put_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Put_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_FIRST;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
               }

               /* Finally make the callback.                            */
               __BTPSTRY
               {
                  (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }
               break;
            case 0x14: /* Idle and Final Flag                           */
            case 0x24: /* Put Setup and Final Flag                      */
               /* In the Idle and Put Setup state, a Final Flag with no */
               /* Body or End of Body Header will indicate the Object   */
               /* specified is to be deleted.  Call the user so the     */
               /* Object can be deleted.  Since additional information  */
               /* about the object may be present in the headers, so    */
               /* extract the Header information before calling the     */
               /* user.                                                 */
               ExtractObjectInfo(OTPInfoPtr, &OTPInfoPtr->ObjectInfo, &HeaderList);

               if(OTPInfoPtr->Target == tIRSync)
               {
                  EventData                                                             = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                            = etOTP_Delete_Sync_Object_Request;
                  EventData->Event_Data_Size                                            = OTP_DELETE_SYNC_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Delete_Sync_Object_Request_Data             = (OTP_Delete_Sync_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Delete_Sync_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Delete_Sync_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Delete_Sync_Object_Request_Data->SyncParams = SyncParams;
               }
               else
               {
                  EventData                                                        = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                       = etOTP_Delete_Object_Request;
                  EventData->Event_Data_Size                                       = OTP_DELETE_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Delete_Object_Request_Data             = (OTP_Delete_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Delete_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Delete_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
               }

               /* Finally make the callback.                            */
               __BTPSTRY
               {
                  (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }
               break;
            case 0x16:
            case 0x26:
               /* In the Idle and Put Setup state, an End of Body Header*/
               /* will indicate the Object is contained in a single     */
               /* packet.  Call the user so the Object can be processed.*/
               /* Since additional information about the object may be  */
               /* present in the headers, extract the Header information*/
               /* before calling the user.                              */
               ExtractObjectInfo(OTPInfoPtr, &OTPInfoPtr->ObjectInfo, &HeaderList);

               OTPInfoPtr->ServerState                                       = ssPut;
               OTPInfoPtr->CurrentOperation                                  = (Byte_t)((OTPInfoPtr->Target == tFileBrowser)?OTP_OPERATION_PUT_FILE:OTP_OPERATION_PUT_OBJECT);
               HeaderPtr                                                     = &HeaderList.Headers[EndOfBodyIndex];

               if(OTPInfoPtr->Target == tIRSync)
               {
                  EventData                                                          = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                         = etOTP_Put_Sync_Object_Request;
                  EventData->Event_Data_Size                                         = OTP_PUT_SYNC_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data             = (OTP_Put_Sync_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_FIRST | OTP_OBJECT_PHASE_LAST;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
               }
               else
               {
                  EventData                                                     = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                    = etOTP_Put_Object_Request;
                  EventData->Event_Data_Size                                    = OTP_PUT_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Put_Object_Request_Data             = (OTP_Put_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Put_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_FIRST | OTP_OBJECT_PHASE_LAST;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
               }

               /* Finally make the callback.                            */
               __BTPSTRY
               {
                  (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }
               break;
            case 0x41: /* Put State and Body Header                     */
            case 0x46: /* Put State, Final Flag and End of Body Header  */
               /* If in the Put State we receive a packet with the more */
               /* object data, then this is a continuation of the Body  */
               /* Data.  If the Final flag is set, this is the last     */
               /* packet.  Call the user to have the data processed.    */
               HeaderPtr                                                     = &HeaderList.Headers[(Weight == 0x41)?BodyIndex:EndOfBodyIndex];
               OTPInfoPtr->CurrentOperation                                  = (Byte_t)((OTPInfoPtr->Target == tFileBrowser)?OTP_OPERATION_PUT_FILE:OTP_OPERATION_PUT_OBJECT);

               if(OTPInfoPtr->Target == tIRSync)
               {
                  EventData                                                          = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                         = etOTP_Put_Sync_Object_Request;
                  EventData->Event_Data_Size                                         = OTP_PUT_SYNC_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data             = (OTP_Put_Sync_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->Phase      = (Byte_t)((Weight == 0x41)?OTP_OBJECT_PHASE_CONTINUE:OTP_OBJECT_PHASE_LAST);
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
                  EventData->Event_Data.OTP_Put_Sync_Object_Request_Data->DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
               }
               else
               {
                  EventData                                                     = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                    = etOTP_Put_Object_Request;
                  EventData->Event_Data_Size                                    = OTP_PUT_OBJECT_REQUEST_DATA_SIZE;
                  EventData->Event_Data.OTP_Put_Object_Request_Data             = (OTP_Put_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Put_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->Phase      = (Byte_t)((Weight == 0x41)?OTP_OBJECT_PHASE_CONTINUE:OTP_OBJECT_PHASE_LAST);
                  EventData->Event_Data.OTP_Put_Object_Request_Data->DataLength = HeaderPtr->Header_Value.ByteSequence.DataLength;
                  EventData->Event_Data.OTP_Put_Object_Request_Data->DataPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
               }

               /* Finally make the callback.                            */
               __BTPSTRY
               {
                  (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }
               break;
            default:
               Valid = FALSE;
               break;
         }
      }

      /* If the request was not properly formatted or is missing data   */
      /* that we expected to be present, notify the caller of the error.*/
      if(Valid == FALSE)
      {
         _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_BAD_REQUEST_RESPONSE | OBEX_FINAL_BIT), NULL);

         OTPInfoPtr->ServerState      = ssIdle;
         OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;
      }
   }
}

   /* The following function is used to process a Put Response from the */
   /* remote entity.  The parameters passed to this function include the*/
   /* Bluetooth Stack ID on which the request was received.  The        */
   /* OTPInfoPtr parameter points to an information structure that      */
   /* contains information about the connection that is currently being */
   /* used.  The Put_Response parameter contains the response           */
   /* information associated with the Put Request.                      */
static void OTP_PutResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Put_Confirmation_Data_t *Put_Response)
{
   int                         HeaderSize;
   Byte_t                      ResponseCode;
   Byte_t                      Buffer[OTP_EVENT_DATA_SIZE+((OTP_PUT_SYNC_OBJECT_RESPONSE_DATA_SIZE > OTP_DELETE_SYNC_OBJECT_RESPONSE_DATA_SIZE)?OTP_PUT_SYNC_OBJECT_RESPONSE_DATA_SIZE:OTP_DELETE_SYNC_OBJECT_RESPONSE_DATA_SIZE)];
   OTP_Sync_Response_Params_t  SyncParams;
   int                         AppParamIndex;
   OBEX_Header_List_t          HeaderList;
   OTP_Event_Data_t           *EventData;

   /* Check to make sure that data that was passed in is semi-valid.    */
   if((OTPInfoPtr) && (Put_Response))
   {
      EventData    = (OTP_Event_Data_t *)Buffer;
      ResponseCode = (Byte_t)(Put_Response->Response_Code & ~OBEX_FINAL_BIT);

      /* This is a response for a Sync Put, Ther may be additional      */
      /* header information to be fetched.                              */
      if(OTPInfoPtr->Target == tIRSync)
      {
         /* Get the list of headers that was received with the response.*/
         HeaderList = Put_Response->Header_List;

         BTPS_MemInitialize(&SyncParams, 0, sizeof(OTP_Sync_Response_Params_t));

         /* Check to see if we have received and Application Parameters */
         /* with the response.  If so, process the header.              */
         AppParamIndex  = FindHeader(hidApplicationParameters, &HeaderList);
         if(AppParamIndex >= 0)
            ExtractSyncResponseParams(&SyncParams, &HeaderList.Headers[AppParamIndex]);
      }

      /* Check the response to see if this is the last packet for the   */
      /* response.  It will be the last if it is any response other than*/
      /* the Continue result.                                           */
      if(ResponseCode != OBEX_CONTINUE_RESPONSE)
      {
         /* Note, is possible that this is a response to a Delete Object*/
         /* Request.  If it is, then we need to map the Response from   */
         /* a Put Response to a Delete Response.                        */
         if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_DELETE)
         {
            /* This is a response to a Delete Object Request.  Build a  */
            /* response for the delete.                                 */
            if(OTPInfoPtr->Target == tIRSync)
            {
               EventData->Event_Data_Type                                               = etOTP_Delete_Sync_Object_Response;
               EventData->Event_Data_Size                                               = OTP_DELETE_SYNC_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Delete_Sync_Object_Response_Data               = (OTP_Delete_Sync_Object_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Delete_Sync_Object_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Delete_Sync_Object_Response_Data->SyncParams   = SyncParams;
               EventData->Event_Data.OTP_Delete_Sync_Object_Response_Data->ResponseCode = ResponseCode;
            }
            else
            {
               EventData->Event_Data_Type                                          = etOTP_Delete_Object_Response;
               EventData->Event_Data_Size                                          = OTP_DELETE_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Delete_Object_Response_Data               = (OTP_Delete_Object_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Delete_Object_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Delete_Object_Response_Data->ResponseCode = ResponseCode;
            }
         }
         else
         {
            if(ResponseCode == OBEX_OK_RESPONSE)
            {
               /* Convert the OBEX success response to our success      */
               /* response value of Zero.                               */
               ResponseCode = 0;
            }

            /* Build the Callback Information to be dispatched to the   */
            /* user to notify them of the event (generic Put Response   */
            /* Event).                                                  */
            if(OTPInfoPtr->Target == tIRSync)
            {
               EventData->Event_Data_Type                                          = etOTP_Put_Sync_Object_Response;
               EventData->Event_Data_Size                                          = OTP_PUT_SYNC_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data             = (OTP_Put_Sync_Object_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->SyncParams = SyncParams;
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->UserInfo   = OTPInfoPtr->UserInfo;

               /* Set the size of the buffer that is available for the  */
               /* next Put data packet to Zero.  No further Puts are    */
               /* required.                                             */
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->BufferSize   = 0;

               /* Pass the result code on to the user.                  */
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->ResponseCode = ResponseCode;
            }
            else
            {
               EventData->Event_Data_Type                                   = etOTP_Put_Object_Response;
               EventData->Event_Data_Size                                   = OTP_PUT_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Put_Object_Response_Data           = (OTP_Put_Object_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Put_Object_Response_Data->OTP_ID   = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Put_Object_Response_Data->UserInfo = OTPInfoPtr->UserInfo;

               /* Set the size of the buffer that is available for the  */
               /* next Put data packet to Zero.  No further Puts are    */
               /* required.                                             */
               EventData->Event_Data.OTP_Put_Object_Response_Data->BufferSize   = 0;

               /* Pass the result code on to the user.                  */
               EventData->Event_Data.OTP_Put_Object_Response_Data->ResponseCode = ResponseCode;
            }
         }

         /* Reset the Status of the connection to an Idle State.        */
         OTPInfoPtr->ServerState          = ssIdle;
         OTPInfoPtr->ObjectInfo.FieldMask = 0;
         OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
      else
      {
         /* Convert the OBEX success response to the Stack success value*/
         /* of Zero.                                                    */
         ResponseCode = 0;

         /* Check to see if we are using a directed connection or not.  */
         if(OTPInfoPtr->Target != tInbox)
         {
            /* Since the response was to Continue, we must call the user*/
            /* for more data.  We must calculate how much data we can   */
            /* receive without exceeding the Max Frame Size.  For every */
            /* packet, 3 bytes will be required for the Put Command and */
            /* the packet length.  It is unclear from the Bluetooth     */
            /* Specification as to whether the Connection ID is a       */
            /* mandatory field for every Put Request or if it is only   */
            /* required for the first request as stated in the IrOBEX   */
            /* specification.  Until this is clear, we shall include the*/
            /* Connection ID or the Target header with each request.    */
            /* Since we will include the Target or Connection ID with   */
            /* each request, we need to reserve memory for the Header ID*/
            /* and Header data.  Since each Put operation will also     */
            /* include a Body or End-Of-Body header, we need to reserve */
            /* 3 bytes for the B/EOB header and 2 byte length.          */
            if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
            {
               /* Reserve 3 bytes for Put Command and Packet Length, 5  */
               /* bytes for the Connection ID Header, Length and Value, */
               /* and 3 bytes for the B/EOB Header and length.          */
               HeaderSize = OBEX_RESPONSE_HEADER_SIZE_WITH_BODY+5;
            }
            else
            {
               /* Reserve 3 bytes for Put Command and Packet Length, 19 */
               /* bytes for the Target ID Header, Length and Value, and */
               /* 3 bytes for the B/EOB Header and length.              */
               HeaderSize = OBEX_RESPONSE_HEADER_SIZE_WITH_BODY+19;
            }
         }
         else
         {
            /* Since we are connected to the Inbox, we need to reserve 3*/
            /* bytes of memory for the Put Command and the Packet Length*/
            /* and 3 bytes for the Body/End-Of-Body header and Length.  */
            HeaderSize = OBEX_RESPONSE_HEADER_SIZE_WITH_BODY;
         }

         if(OTPInfoPtr->CurrentOperation != OTP_OPERATION_DELETE)
         {
            if(OTPInfoPtr->Target == tIRSync)
            {
               EventData->Event_Data_Type                                          = etOTP_Put_Sync_Object_Response;
               EventData->Event_Data_Size                                          = OTP_PUT_SYNC_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data             = (OTP_Put_Sync_Object_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->SyncParams = SyncParams;
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->UserInfo   = OTPInfoPtr->UserInfo;

               /* Calculate the amount of memory that we have remaining */
               /* to fill with data.                                    */
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->BufferSize = OTPInfoPtr->MaxPacketSize-HeaderSize;

               /* Load the Response code that was received.  Note that  */
               /* since the result code was a Continue, it has been     */
               /* changed to a Zero.  Only a failed Result code will be */
               /* passed to the user as a non-Zero value.               */
               EventData->Event_Data.OTP_Put_Sync_Object_Response_Data->ResponseCode = 0;
            }
            else
            {
               EventData->Event_Data_Type                                   = etOTP_Put_Object_Response;
               EventData->Event_Data_Size                                   = OTP_PUT_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Put_Object_Response_Data           = (OTP_Put_Object_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Put_Object_Response_Data->OTP_ID   = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Put_Object_Response_Data->UserInfo = OTPInfoPtr->UserInfo;

               /* Calculate the amount of memory that we have remaining */
               /* to fill with data.                                    */
               EventData->Event_Data.OTP_Put_Object_Response_Data->BufferSize = OTPInfoPtr->MaxPacketSize-HeaderSize;

               /* Load the Response code that was received.  Note that  */
               /* since the result code was a Continue, it has been     */
               /* changed to a Zero.  Only a failed Result code will be */
               /* passed to the user as a non-Zero value.               */
               EventData->Event_Data.OTP_Put_Object_Response_Data->ResponseCode = 0;
            }
         }

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function processes a request from a Remote Client to*/
   /* receive an Object or Directory Information.  The parameters passed*/
   /* to this function include the Bluetooth Stack ID on which the      */
   /* request was received.  The OTPInfoPtr parameter points to an      */
   /* information structure that contains information about the         */
   /* connection that is currently being used.  The Get Request         */
   /* parameter contains the information associated with the Get        */
   /* Request.                                                          */
static void OTP_GetRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Get_Indication_Data_t *Get_Request)
{
   int                 i;
   Byte_t              Buffer[OTP_EVENT_DATA_SIZE+((OTP_GET_OBJECT_REQUEST_DATA_SIZE > OTP_GET_DIRECTORY_REQUEST_DATA_SIZE)?OTP_GET_OBJECT_REQUEST_DATA_SIZE:OTP_GET_DIRECTORY_REQUEST_DATA_SIZE)];
   Boolean_t           Valid;
   OBEX_Header_t      *HeaderPtr;
   OTP_Event_Data_t   *EventData = NULL;
   OBEX_Header_List_t  HeaderList;

   /* Check to make sure that data that was passed in is semi-valid.    */
   if((OTPInfoPtr) && (Get_Request))
   {
      /* The first step is to determine if the header information that  */
      /* was received appears correct for the state that we are in.     */
      Valid      = TRUE;
      HeaderPtr  = NULL;
      HeaderList = Get_Request->Header_List;

      /* Check to see if a Connection ID was assigned when the original */
      /* connection was established.  If so, then we need to check to   */
      /* see if a Connection ID was provided with the request and if it */
      /* matches the one that we have saved.                            */
      if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
      {
         /* Check for a Connection ID Header.                           */
         i = FindHeader(hidConnectionID, &HeaderList);
         if(i >= 0)
         {
            /* Since a Connection ID does exist, then we need to see if */
            /* the Connection ID specified matches the Connection ID    */
            /* used in the original connect request.                    */
            HeaderPtr = &HeaderList.Headers[i];
            if(HeaderPtr->Header_Value.FourByteValue != OTPInfoPtr->ConnectionID)
            {
               /* If the Connection ID does not match, then this is an  */
               /* invalid request.                                      */
               Valid = FALSE;
            }
         }
         else
         {
            /* No Connection ID was present in the headers and one was  */
            /* assigned for this connection.  The Connection ID must be */
            /* the 1st header when used, and must be included in the 1st*/
            /* packet of the request.  If this packet is a continuation */
            /* packet, then it is not required.  Note that it is unclear*/
            /* if the Connection ID is Mandatory for each request,      */
            /* implied by the Bluetooth specification, or if it is only */
            /* required in the first request.  To be compatible with as */
            /* many implementations as possible, we will accept any     */
            /* method.  So, the Connection ID must be present for the   */
            /* 1st request, which is noted by not being assigned to any */
            /* operation at this time.                                  */
            if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_NONE)
            {
               Valid = FALSE;
            }
         }
      }

      /* If the Connection ID test passed, then continue processing the */
      /* Put request.                                                   */
      if(Valid)
      {
         /* Check to make sure we are in a State that can accept the    */
         /* Request.                                                    */
         if((OTPInfoPtr->ServerState == ssIdle) || (OTPInfoPtr->ServerState == ssGetSetup) || (OTPInfoPtr->ServerState == ssGet))
         {
            /* Extract information about the object being transferred   */
            /* from the header information that may exist with the      */
            /* request.                                                 */
            if(HeaderList.NumberOfHeaders)
            {
               ExtractObjectInfo(OTPInfoPtr, &OTPInfoPtr->ObjectInfo, &HeaderList);
            }

            if(Get_Request->Final_Flag == FALSE)
            {
               /* If the Final flag is not present, then there will be  */
               /* more packets to come before we have to process any    */
               /* data.  We will continue to extract the information    */
               /* from the Headers until the Final Flag is set.  Until  */
               /* we have all of the header information and know exactly*/
               /* what we are getting, set the Current Operation to Get */
               /* Object.  Set the Server State to Get Setup to indicate*/
               /* we have only a partial amount of information for the  */
               /* Get request.                                          */
               OTPInfoPtr->ServerState = ssGetSetup;

               _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_CONTINUE_RESPONSE | OBEX_FINAL_BIT), NULL);
            }
            else
            {
               /* We now have all of the header information, so we can  */
               /* set the server state to the Get Mode.                 */
               OTPInfoPtr->ServerState = ssGet;

               /* Check the Type and Target that is being requested.  If*/
               /* the Target is the File Browser and the Type is a      */
               /* Folder Listing, then we need to return a Directory    */
               /* structure to the user.                                */
               if(OTPInfoPtr->Target == tFileBrowser)
               {
                  /* Check to see if the Type field was indicated and   */
                  /* the type is a Directory listing.  If no Type was   */
                  /* requested or a type that was not a Directory       */
                  /* Listing was requested, then this must be for a     */
                  /* File.                                              */
                  if((OTPInfoPtr->ObjectInfo.FieldMask & OTP_OBJECT_INFO_MASK_TYPE) &&
                     (OTPInfoPtr->ObjectInfo.TypeLength == sizeof(FolderListingType)) &&
                     (BTPS_MemCompare((void *)OTPInfoPtr->ObjectInfo.Type, (void *)FolderListingType, sizeof(FolderListingType)) == 0))
                  {
                     /* A request is being made for a folder listing.   */
                     /* Check to see if this is the 1st request.  If so,*/
                     /* set the Current Operation to denote that we are */
                     /* begin requested to return a directory listing.  */
                     /* Set the Object Type to File Folder to denote the*/
                     /* object that is being requested is a FileFolder  */
                     /* listing.                                        */
                     if(OTPInfoPtr->CurrentOperation != OTP_OPERATION_GET_DIRECTORY)
                     {
                        /* This is the first of this request, so we need*/
                        /* to initialize some state information.        */
                        OTPInfoPtr->CurrentOperation      = OTP_OPERATION_GET_DIRECTORY;
                        OTPInfoPtr->DirInfo.NumberEntries = 0;
                        OTPInfoPtr->ObjectInfo.ObjectType = otFileFolder;

                        /* Setup the information needed to dispatch the */
                        /* Request to get the Directory Listing to the  */
                        /* Upper Layer.  It must be noted that          */
                        /* dispatching a request for the Directory      */
                        /* information will only occur once, even if the*/
                        /* information will take multiple packets to    */
                        /* transmit the entire data.  The user will pass*/
                        /* a pointer to the entire directory information*/
                        /* structure which is save.  Request to receive */
                        /* continuing directory information will be     */
                        /* automatically sent.  When all of the         */
                        /* information has been sent, a call to the user*/
                        /* is dispatched to notify the user that the    */
                        /* memory is no longer needed and the memory can*/
                        /* be freed.                                    */
                        EventData                                                            = (OTP_Event_Data_t *)Buffer;
                        EventData->Event_Data_Type                                           = etOTP_Get_Directory_Request;
                        EventData->Event_Data_Size                                           = OTP_GET_DIRECTORY_REQUEST_DATA_SIZE;
                        EventData->Event_Data.OTP_Get_Directory_Request_Data                 = (OTP_Get_Directory_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                        EventData->Event_Data.OTP_Get_Directory_Request_Data->OTP_ID         = OTPInfoPtr->OTP_ID;

                        /* Check to see if the Header Information       */
                        /* included a Name.  If a specific name was     */
                        /* specified, then the user is wanting a        */
                        /* directory listing of a child folder off the  */
                        /* current directory.                           */
                        if(OTPInfoPtr->ObjectInfo.FieldMask & OTP_OBJECT_INFO_MASK_NAME)
                        {
                           /* Provide the Name information specified in */
                           /* the request.                              */
                           EventData->Event_Data.OTP_Get_Directory_Request_Data->NameLength = ExtractNameLengthFromObjectInfo(&(OTPInfoPtr->ObjectInfo), TRUE);
                           EventData->Event_Data.OTP_Get_Directory_Request_Data->Name       = ExtractNameFromObjectInfo(&(OTPInfoPtr->ObjectInfo));
                        }
                        else
                        {
                           /* No Name was specified, so Clear the Name  */
                           /* data.                                     */
                           EventData->Event_Data.OTP_Get_Directory_Request_Data->NameLength = 0;
                           EventData->Event_Data.OTP_Get_Directory_Request_Data->Name       = NULL;
                        }
                     }
                     else
                     {
                        /* Deliver another section of the Directory to  */
                        /* the Remote Client.                           */
                        ProcessDirectoryRequest(OTPInfoPtr);
                     }
                  }
                  else
                  {
                     /* The request must be for a File, so setup the    */
                     /* data to be dispatched to the user to get the    */
                     /* data.  Note that there some bytes reserved from */
                     /* the buffer to allow room for some required      */
                     /* headers in the response.                        */
                     EventData                                                     = (OTP_Event_Data_t *)Buffer;
                     EventData->Event_Data_Type                                    = etOTP_Get_Object_Request;
                     EventData->Event_Data_Size                                    = OTP_GET_OBJECT_REQUEST_DATA_SIZE;
                     EventData->Event_Data.OTP_Get_Object_Request_Data             = (OTP_Get_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                     EventData->Event_Data.OTP_Get_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                     EventData->Event_Data.OTP_Get_Object_Request_Data->BufferSize = OTPInfoPtr->MaxPacketSize-OBEX_RESPONSE_HEADER_SIZE_WITH_BODY;
                     EventData->Event_Data.OTP_Get_Object_Request_Data->BufferPtr  = OTPInfoPtr->ResponseBuffer;
                     EventData->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;

                     /* Check to see if this is the 1st request for the */
                     /* file.  Set the callback parameters              */
                     /* appropriately.  The UserInfo parameter can be   */
                     /* used by the upper layer to hold information     */
                     /* about the state of the get process.  For the 1st*/
                     /* callback, clear the parameter.  Following       */
                     /* callback will supply the value that was provided*/
                     /* by the user in the Get Response call.           */
                     if(OTPInfoPtr->CurrentOperation != OTP_OPERATION_GET_FILE)
                     {
                        OTPInfoPtr->CurrentOperation                                  = OTP_OPERATION_GET_FILE;
                        OTPInfoPtr->ObjectInfo.ObjectType                             = otFile;

                        EventData->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                        EventData->Event_Data.OTP_Get_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_FIRST;
                        EventData->Event_Data.OTP_Get_Object_Request_Data->UserInfo   = 0;
                     }
                     else
                     {
                        EventData->Event_Data.OTP_Get_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_CONTINUE;
                        EventData->Event_Data.OTP_Get_Object_Request_Data->UserInfo   = OTPInfoPtr->UserInfo;
                     }
                  }
               }
               else
               {
                  /* This is not a File Browser connection, so we must  */
                  /* be connected to the Inbox or the IrSync server.    */
                  if((OTPInfoPtr->Target == tInbox) || (OTPInfoPtr->Target == tIRSync))
                  {
                     /* The request must be for some Object, so setup   */
                     /* the data to be dispatched to the user to get the*/
                     /* data.  Note that there some bytes reserved from */
                     /* the buffer to allow room for some required      */
                     /* headers in the response.                        */
                     EventData                                                     = (OTP_Event_Data_t *)Buffer;
                     EventData->Event_Data_Type                                    = etOTP_Get_Object_Request;
                     EventData->Event_Data_Size                                    = OTP_GET_OBJECT_REQUEST_DATA_SIZE;
                     EventData->Event_Data.OTP_Get_Object_Request_Data             = (OTP_Get_Object_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                     EventData->Event_Data.OTP_Get_Object_Request_Data->OTP_ID     = OTPInfoPtr->OTP_ID;
                     EventData->Event_Data.OTP_Get_Object_Request_Data->BufferSize = OTPInfoPtr->MaxPacketSize-OBEX_RESPONSE_HEADER_SIZE_WITH_BODY;
                     EventData->Event_Data.OTP_Get_Object_Request_Data->BufferPtr  = OTPInfoPtr->ResponseBuffer;
                     EventData->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;

                     /* Check to see if this is the 1st request for the */
                     /* file.  Set the callback parameters              */
                     /* appropriately.  The UserInfo parameter can be   */
                     /* used by the upper layer to hold information     */
                     /* about the state of the get process.  For the 1st*/
                     /* callback, clear the parameter.  Following       */
                     /* callback will supply the value that was provided*/
                     /* by the user in the Get Response call.           */
                     if(OTPInfoPtr->CurrentOperation != OTP_OPERATION_GET_OBJECT)
                     {
                        OTPInfoPtr->CurrentOperation                                  = OTP_OPERATION_GET_OBJECT;
                        OTPInfoPtr->ObjectInfo.ObjectType                             = otObject;

                        EventData->Event_Data.OTP_Get_Object_Request_Data->ObjectInfo = OTPInfoPtr->ObjectInfo;
                        EventData->Event_Data.OTP_Get_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_FIRST;
                        EventData->Event_Data.OTP_Get_Object_Request_Data->UserInfo   = 0;
                     }
                     else
                     {
                        EventData->Event_Data.OTP_Get_Object_Request_Data->Phase      = OTP_OBJECT_PHASE_CONTINUE;
                        EventData->Event_Data.OTP_Get_Object_Request_Data->UserInfo   = OTPInfoPtr->UserInfo;
                     }
                  }
                  else
                  {
                     /* A non-supported target was specified.  Refuse   */
                     /* the Get request.                                */
                     OTPInfoPtr->ServerState          = ssIdle;
                     OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                     OTPInfoPtr->ObjectInfo.FieldMask = 0;

                     _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_SERVICE_UNAVAILABLE_RESPONSE | OBEX_FINAL_BIT), NULL);
                  }
               }
            }
         }
         else
         {
            /* We are not in a state to process this request.  Send a   */
            /* response to the Client about this error.                 */
            OTPInfoPtr->ServerState          = ssIdle;
            OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
            OTPInfoPtr->ObjectInfo.FieldMask = 0;

            _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_BAD_REQUEST_RESPONSE | OBEX_FINAL_BIT), NULL);
         }

         /* If a Call to the Upper layer is required, attempt to        */
         /* dispatch the data to the upper layers.                      */
         if(EventData)
         {
            /* Finally make the callback.                               */
            __BTPSTRY
            {
               (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }
      }
      else
      {
         _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_SERVICE_UNAVAILABLE_RESPONSE | OBEX_FINAL_BIT), NULL);

         OTPInfoPtr->ServerState = ssIdle;
      }
   }
}

   /* The following function is used to process the results of a Get    */
   /* request to a Remote Server.  The parameters passed to this        */
   /* function include the Bluetooth Stack ID on which the request was  */
   /* received.  The OTPInfoPtr parameter points to an information      */
   /* structure that contains information about the connection that is  */
   /* currently being used.  The Get_Response parameter contains the    */
   /* response information associated with the Put Request.             */
static void OTP_GetResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Get_Confirmation_Data_t *Get_Response)
{
   int                 Index;
   int                 BufferSize;
   Byte_t              ResponseCode;
   unsigned int        TempIndex;
   unsigned int        NumberOfEntries;
   OBEX_Header_t      *HeaderPtr;
   OTP_Event_Data_t   *EventData = NULL;
   OBEX_Header_List_t  HeaderList;

   /* Check to make sure that data that was passed in is semi-valid.    */
   if((OTPInfoPtr) && (Get_Response))
   {
      /* Check the Response Code.  If the Response Code is Continue or  */
      /* OK, then the data has been received successfully.  If any other*/
      /* response, then the get was not successfully.                   */
      ResponseCode = (Byte_t)(Get_Response->Response_Code & ~OBEX_FINAL_BIT);
      if((ResponseCode == OBEX_OK_RESPONSE) || (ResponseCode == OBEX_CONTINUE_RESPONSE))
      {
         /* Set a pointer to the Beginning of the header information and*/
         /* search the list of headers for a Body Header.               */
         HeaderList      = Get_Response->Header_List;
         Index           = FindHeader(hidBody, &HeaderList);
         NumberOfEntries = 0;
         if(Index < 0)
         {
            /* If Index is negative, then a Body Header was not found,  */
            /* so look for a End of Body Header.                        */
            Index = FindHeader(hidEndOfBody, &HeaderList);
         }

         if(Index >= 0)
         {
            /* Either a Body or End of Body header was located.  Load a */
            /* pointer to the information about the header data.        */
            HeaderPtr = &HeaderList.Headers[Index];
            HeaderPtr->Header_Value.ByteSequence.ValuePointer[HeaderPtr->Header_Value.ByteSequence.DataLength] = 0;
            if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_GET_DIRECTORY)
            {
               /* If we are in the process of retrieving the directory  */
               /* information, then convert the XML data that has been  */
               /* received to an array of directory information         */
               /* structures.  Since we do not yet know how many entries*/
               /* exist in the XML data, we do not know how many array  */
               /* elements will be required to save the information.    */
               /* When calling the XML conversion routine with the      */
               /* pointer to the Object Info array structure set to     */
               /* NULL, the function will scan the data and return the  */
               /* number of entries that were found in the XML data.    */
               XMLToFileObject(&NumberOfEntries, NULL, &OTPInfoPtr->DirEntrySegment, (char *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, &(OTPInfoPtr->DirInfo.ParentDirectory));

               /* OK, we only need to call the user back if there are   */
               /* Entries that have been found, OR if this is the       */
               /* last packet (regardless of the Number of Entries that */
               /* have been found).                                     */
               if((NumberOfEntries) || (OTPInfoPtr->DirInfo.ParentDirectory) || ((!NumberOfEntries) && (ResponseCode != OBEX_CONTINUE_RESPONSE)))
               {
                  /* Use the number of entries that was calculated from */
                  /* the previous call to calculate the amount of data  */
                  /* that must be allocated to build the callback       */
                  /* structure.                                         */
                  BufferSize = OTP_GET_DIRECTORY_RESPONSE_DATA_SIZE+OTP_DIRECTORY_INFO_SIZE(NumberOfEntries);
                  EventData  = (OTP_Event_Data_t *)BTPS_AllocateMemory(OTP_EVENT_DATA_SIZE+BufferSize);

                  /* Before continuing, make sure we were able to       */
                  /* allocate the memory we requested.                  */
                  if(EventData)
                  {
                     /* Create the callback information structure.      */
                     EventData->Event_Data_Type                                                     = etOTP_Get_Directory_Response;
                     EventData->Event_Data_Size                                                     = (Word_t)BufferSize;
                     EventData->Event_Data.OTP_Get_Directory_Response_Data                          = (OTP_Get_Directory_Response_Data_t *)((Byte_t *)EventData+OTP_EVENT_DATA_SIZE);
                     EventData->Event_Data.OTP_Get_Directory_Response_Data->OTP_ID                  = OTPInfoPtr->OTP_ID;
                     EventData->Event_Data.OTP_Get_Directory_Response_Data->ResponseCode            = 0;
                     EventData->Event_Data.OTP_Get_Directory_Response_Data->Phase                   = (Byte_t)((ResponseCode == OBEX_OK_RESPONSE)?OTP_OBJECT_PHASE_LAST:OTP_OBJECT_PHASE_CONTINUE);

                     EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries   = NumberOfEntries;
                     EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo      = (OTP_ObjectInfo_t *)((NumberOfEntries)?(((Byte_t *)(EventData->Event_Data.OTP_Get_Directory_Response_Data))+OTP_GET_DIRECTORY_RESPONSE_DATA_SIZE):NULL);
                     EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ParentDirectory = OTPInfoPtr->DirInfo.ParentDirectory;

                     /* If there are entries to be processed, then call */
                     /* the function to convert the XML data to the     */
                     /* array of Objects.                               */
                     if(NumberOfEntries)
                     {
                        XMLToFileObject(&NumberOfEntries, EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo, &OTPInfoPtr->DirEntrySegment, (char *)HeaderPtr->Header_Value.ByteSequence.ValuePointer, NULL);
                     }
                  }
               }
            }
            else
            {
               /* If the data is not a Directory Listing, then it is    */
               /* either File or Object data.  In either case the       */
               /* processing is the same.  Call the user with the new   */
               /* segment of data so that it can be processed.          */
               if((OTPInfoPtr->CurrentOperation == OTP_OPERATION_GET_FILE) || (OTPInfoPtr->CurrentOperation == OTP_OPERATION_GET_OBJECT))
               {
                  /* Build the Callback structure.                      */
                  EventData  = (OTP_Event_Data_t *)BTPS_AllocateMemory(OTP_EVENT_DATA_SIZE+OTP_GET_OBJECT_RESPONSE_DATA_SIZE);

                  /* Before continuing, make sure we were able to       */
                  /* allocate the memory we requested.                  */
                  if(EventData)
                  {
                     EventData->Event_Data_Type                                       = etOTP_Get_Object_Response;
                     EventData->Event_Data_Size                                       = OTP_GET_OBJECT_RESPONSE_DATA_SIZE;
                     EventData->Event_Data.OTP_Get_Object_Response_Data               = (OTP_Get_Object_Response_Data_t *)((Byte_t *)EventData+OTP_EVENT_DATA_SIZE);
                     EventData->Event_Data.OTP_Get_Object_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
                     EventData->Event_Data.OTP_Get_Object_Response_Data->ResponseCode = 0;
                     EventData->Event_Data.OTP_Get_Object_Response_Data->Phase        = (Byte_t)((ResponseCode == OBEX_OK_RESPONSE)?OTP_OBJECT_PHASE_LAST:OTP_OBJECT_PHASE_CONTINUE);
                     EventData->Event_Data.OTP_Get_Object_Response_Data->ObjectInfo   = OTPInfoPtr->ObjectInfo;
                     EventData->Event_Data.OTP_Get_Object_Response_Data->BufferSize   = HeaderPtr->Header_Value.ByteSequence.DataLength;
                     EventData->Event_Data.OTP_Get_Object_Response_Data->BufferPtr    = HeaderPtr->Header_Value.ByteSequence.ValuePointer;
                     EventData->Event_Data.OTP_Get_Object_Response_Data->UserInfo     = OTPInfoPtr->UserInfo;
                  }
               }
            }
         }
      }
      else
      {
         /* If we are here then the remote server has responded to the  */
         /* request with an error code.  Build an appropriate callback  */
         /* structure with information about the error that occurred.   */
         if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_GET_DIRECTORY)
         {
            BufferSize = sizeof(OTP_Get_Directory_Response_Data_t);
            EventData  = (OTP_Event_Data_t *)BTPS_AllocateMemory(OTP_EVENT_DATA_SIZE+BufferSize);

            /* Before continuing, make sure we were able to allocate the*/
            /* memory we requested.                                     */
            if(EventData)
            {
               EventData->Event_Data_Type                                                   = etOTP_Get_Directory_Response;
               EventData->Event_Data_Size                                                   = (Word_t)BufferSize;
               EventData->Event_Data.OTP_Get_Directory_Response_Data                        = (OTP_Get_Directory_Response_Data_t *)((Byte_t *)EventData+OTP_EVENT_DATA_SIZE);
               EventData->Event_Data.OTP_Get_Directory_Response_Data->OTP_ID                = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Get_Directory_Response_Data->ResponseCode          = ResponseCode;
               EventData->Event_Data.OTP_Get_Directory_Response_Data->Phase                 = OTP_OBJECT_PHASE_LAST;

               EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries = 0;
               EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo    = NULL;
            }
         }
         else
         {
            EventData = (OTP_Event_Data_t *)BTPS_AllocateMemory(OTP_EVENT_DATA_SIZE+OTP_GET_OBJECT_RESPONSE_DATA_SIZE);

            /* Before continuing, make sure we were able to allocate the*/
            /* memory we requested.                                     */
            if(EventData)
            {
               EventData->Event_Data_Type                                       = etOTP_Get_Object_Response;
               EventData->Event_Data_Size                                       = OTP_GET_OBJECT_RESPONSE_DATA_SIZE;
               EventData->Event_Data.OTP_Get_Object_Response_Data               = (OTP_Get_Object_Response_Data_t *)((Byte_t *)EventData+OTP_EVENT_DATA_SIZE);
               EventData->Event_Data.OTP_Get_Object_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Get_Object_Response_Data->ResponseCode = ResponseCode;
               EventData->Event_Data.OTP_Get_Object_Response_Data->Phase        = OTP_OBJECT_PHASE_LAST;
               EventData->Event_Data.OTP_Get_Object_Response_Data->ObjectInfo   = OTPInfoPtr->ObjectInfo;
               EventData->Event_Data.OTP_Get_Object_Response_Data->BufferSize   = 0;
               EventData->Event_Data.OTP_Get_Object_Response_Data->BufferPtr    = NULL;
               EventData->Event_Data.OTP_Get_Object_Response_Data->UserInfo     = OTPInfoPtr->UserInfo;
            }
         }
      }

      /* If we have built a callback structure, dispatch the information*/
      /* to the upper layers.                                           */
      if(EventData)
      {
         /* If the response was anything but continue, then the transfer*/
         /* process is over and state information need to be reset.     */
         if(ResponseCode != OBEX_CONTINUE_RESPONSE)
         {
            OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
            OTPInfoPtr->ServerState          = ssIdle;
            OTPInfoPtr->ObjectInfo.FieldMask = 0;

            /* It is possible that we have not cleaned up the Directory */
            /* Segment (error response received or invalid packet), so  */
            /* we need to make sure that we do indeed Delete any        */
            /* memory we might have allocated.                          */
            if(OTPInfoPtr->DirEntrySegment)
               BTPS_FreeMemory(OTPInfoPtr->DirEntrySegment);

            /* Flag that no memory has been allocated.                  */
            OTPInfoPtr->DirEntrySegment = NULL;
         }

         /* Check to see if this event type was a Get Directory         */
         /* Response.                                                   */
         if(EventData->Event_Data_Type == etOTP_Get_Directory_Response)
         {
            /* This was a Get Directory Response, check to see if it    */
            /* contained any entries.                                   */
            if((EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries > 0) && (EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo != NULL))
            {
               /* Loop through each of the entries.                     */
               for(TempIndex=0;TempIndex<EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.NumberEntries;TempIndex++)
               {
                  /* Check to see if this entry contained an extended   */
                  /* Name member.  If so free the memory that was       */
                  /* allocated for this response.                       */
                  if(EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[TempIndex].FieldMask & OTP_OBJECT_INFO_MASK_EXTENDED_NAME)
                     BTPS_FreeMemory((void *)READ_OBJECT_INFO_EXTENDED_NAME(EventData->Event_Data.OTP_Get_Directory_Response_Data->DirInfo.ObjectInfo[TempIndex].Name));
               }
            }
         }

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }

         BTPS_FreeMemory(EventData);
      }
   }
}

   /* The following function is used to process a request from a remote */
   /* client to change the current directory.  The parameters passed to */
   /* this function include the Bluetooth Stack ID on which the request */
   /* was received.  The OTPInfoPtr parameter points to an information  */
   /* structure that contains information about the connection that is  */
   /* currently being used.  The Set_Path_Request parameter contains the*/
   /* information associated with the Request.                          */
static void OTP_SetPathRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Set_Path_Indication_Data_t *Set_Path_Request)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_SET_PATH_REQUEST_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the parameters passed in are semi-valid.              */
   if((OTPInfoPtr) && (Set_Path_Request))
   {
      /* Check to see if we have a callback to the user.                */
      if((OTPInfoPtr) && (OTPInfoPtr->OTP_EventCallback))
      {
         /* Extract the header information from the Header data.        */
         ExtractObjectInfo(OTPInfoPtr, &OTPInfoPtr->ObjectInfo, &Set_Path_Request->Header_List);

         /* Build a callback structure and dispatch the information to  */
         /* the upper layer.                                            */
         EventData                                               = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                              = etOTP_Set_Path_Request;
         EventData->Event_Data_Size                              = OTP_SET_PATH_REQUEST_DATA_SIZE;
         EventData->Event_Data.OTP_Set_Path_Request_Data         = (OTP_Set_Path_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Set_Path_Request_Data->OTP_ID = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Set_Path_Request_Data->Backup = Set_Path_Request->Backup;
         EventData->Event_Data.OTP_Set_Path_Request_Data->Create = Set_Path_Request->CreateDirectory;
         EventData->Event_Data.OTP_Set_Path_Request_Data->Folder = OTPInfoPtr->ObjectInfo.Name;

         /* Clear the Header Mask.                                      */
         OTPInfoPtr->ObjectInfo.FieldMask = 0;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* This function is used to process a response from a remote server  */
   /* for a previous Set Path Request.  The parameters passed to this   */
   /* function include the Bluetooth Stack ID on which the request was  */
   /* received.  The OTPInfoPtr parameter points to an information      */
   /* structure that contains information about the connection that is  */
   /* currently being used.  The Set_Path_Response parameter contains   */
   /* the response information associated with the Request.             */
static void OTP_SetPathResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Set_Path_Confirmation_Data_t *Set_Path_Response)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_SET_PATH_RESPONSE_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Test to make sure that data passed in is semi-valid.              */
   if((OTPInfoPtr) && (Set_Path_Response))
   {
      /* The set path function is a single packet command, so we can    */
      /* reset some state information.                                  */
      OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
      OTPInfoPtr->ServerState          = ssIdle;
      OTPInfoPtr->ObjectInfo.FieldMask = 0;

      /* Check to see if we are holding a handle to some Directory      */
      /* information.  If so, call the user to inform them that the     */
      /* memory is no longer needed.                                    */
      if((OTPInfoPtr) && (OTPInfoPtr->OTP_EventCallback))
      {
         EventData                                                      = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                                     = etOTP_Set_Path_Response;
         EventData->Event_Data_Size                                     = OTP_SET_PATH_RESPONSE_DATA_SIZE;
         EventData->Event_Data.OTP_Set_Path_Response_Data               = (OTP_Set_Path_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Set_Path_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Set_Path_Response_Data->ResponseCode = Set_Path_Response->Response_Code;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* This function is used to process an Abort request from a remote   */
   /* client.  The parameters passed to this function include the       */
   /* Bluetooth Stack ID on which the request was received.  The        */
   /* OTPInfoPtr parameter points to an information structure that      */
   /* contains information about the connection that is currently being */
   /* used.  The Abort_Request parameter contains the information       */
   /* associated with the Request.                                      */
static void OTP_AbortRequestEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Abort_Indication_Data_t *Abort_Request)
{
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+((OTP_ABORT_REQUEST_DATA_SIZE > OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE)?OTP_ABORT_REQUEST_DATA_SIZE:OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE)];
   unsigned int      OTP_ID;
   OTP_Event_Data_t *EventData;

   /* Test to make sure that data passed in is semi-valid.              */
   if((OTPInfoPtr) && (Abort_Request))
   {
      /* Check to see if we are holding a handle to some Directory      */
      /* information.  If so, call the user to inform them that the     */
      /* memory is no longer needed.                                    */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         /* Store off the FTPM Mgr ID to be used later.                 */
         OTP_ID = OTPInfoPtr->OTP_ID;

         if(OTPInfoPtr->DirInfo.ObjectInfoList)
         {
            EventData                                                                              = (OTP_Event_Data_t *)Buffer;
            EventData->Event_Data_Type                                                             = etOTP_Free_Directory_Information;
            EventData->Event_Data_Size                                                             = OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE;
            EventData->Event_Data.OTP_Free_Directory_Information_Data                              = (OTP_Free_Directory_Information_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
            EventData->Event_Data.OTP_Free_Directory_Information_Data->OTP_ID                      = OTPInfoPtr->OTP_ID;

            EventData->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.NumberEntries = OTPInfoPtr->DirInfo.DirectoryIndex;
            EventData->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo    = OTPInfoPtr->DirInfo.ObjectInfoList;

            /* Flag that there is no memory being held anymore.         */
            OTPInfoPtr->DirInfo.NumberEntries                                                      = 0;
            OTPInfoPtr->DirInfo.ObjectInfoList                                                     = NULL;

            /* Finally make the callback.                               */
            __BTPSTRY
            {
               (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
            }
            __BTPSEXCEPT(1)
            {
               /* Do Nothing.                                           */
            }
         }

         /* Wait for access to the lock that guards access to this      */
         /* module.                                                     */
         if(DEVM_AcquireLock())
         {
            /* The FTPM Mgr Context Information was successfully        */
            /* retrieved, next attempt to get the FTPM Mgr Information  */
            /* Entry.                                                   */
            if((Initialized) && (SearchOTPInfoEntry(&OTPInfoList, OTP_ID) != NULL))
            {
               /* The FTPM Mgr Information Entry is still valid, in this*/
               /* case format up the event to make the callback with.   */
               EventData                                                      = (OTP_Event_Data_t *)Buffer;
               EventData->Event_Data_Type                                     = etOTP_Abort_Request;
               EventData->Event_Data_Size                                     = OTP_ABORT_REQUEST_DATA_SIZE;
               EventData->Event_Data.OTP_Abort_Request_Data                   = (OTP_Abort_Request_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
               EventData->Event_Data.OTP_Abort_Request_Data->OTP_ID           = OTPInfoPtr->OTP_ID;
               EventData->Event_Data.OTP_Abort_Request_Data->UserInfo         = OTPInfoPtr->UserInfo;

               /* Finally make the callback.                            */
               __BTPSTRY
               {
                  (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
               }
               __BTPSEXCEPT(1)
               {
                  /* Do Nothing.                                        */
               }
            }

            /* Release the Lock because we are finished with it.        */
            DEVM_ReleaseLock();
         }
      }
   }
}

   /* The following function is used to process the response from a     */
   /* remote server to an Abort request.  The parameters passed to this */
   /* function include the Bluetooth Stack ID on which the request was  */
   /* received.  The OTPInfoPtr parameter points to an information      */
   /* structure that contains information about the connection that is  */
   /* currently being used.  The Abort_Response parameter contains the  */
   /* response information associated with the Request.                 */
static void OTP_AbortResponseEvent(OTPM_Info_t *OTPInfoPtr, OBEX_Abort_Confirmation_Data_t *Abort_Response)
{
   Byte_t            ResponseCode;
   Byte_t            Buffer[OTP_EVENT_DATA_SIZE+OTP_ABORT_RESPONSE_DATA_SIZE];
   OTP_Event_Data_t *EventData;

   /* Verify that the parameters that were passed to this function are  */
   /* semi-valid.                                                       */
   if((OTPInfoPtr) && (Abort_Response))
   {
      /* Extract the response code from the remote server.              */
      ResponseCode = (Byte_t)(Abort_Response->Response_Code & ~OBEX_FINAL_BIT);
      if(ResponseCode == OBEX_OK_RESPONSE)
      {
         /* If the response is OK, then convert this to a successful    */
         /* response for the upper layer.                               */
         ResponseCode = 0;
      }

      /* Check to see if we are holding a handle to some Directory      */
      /* information.  If so, call the user to inform them that the     */
      /* memory is no longer needed.                                    */
      if(OTPInfoPtr->OTP_EventCallback)
      {
         EventData                                                   = (OTP_Event_Data_t *)Buffer;
         EventData->Event_Data_Type                                  = etOTP_Abort_Response;
         EventData->Event_Data_Size                                  = OTP_ABORT_RESPONSE_DATA_SIZE;
         EventData->Event_Data.OTP_Abort_Response_Data               = (OTP_Abort_Response_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
         EventData->Event_Data.OTP_Abort_Response_Data->OTP_ID       = OTPInfoPtr->OTP_ID;
         EventData->Event_Data.OTP_Abort_Response_Data->ResponseCode = ResponseCode;

         /* Finally make the callback.                                  */
         __BTPSTRY
         {
            (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
         }
         __BTPSEXCEPT(1)
         {
            /* Do Nothing.                                              */
         }
      }
   }
}

   /* The following function is used to process a request for Directory */
   /* information.  As much of the directory information will be        */
   /* processed here as possible.  This function should be called again */
   /* due a request from the Client to retrieve further information.    */
static void ProcessDirectoryRequest(OTPM_Info_t *OTPInfoPtr)
{
   int                i;
   unsigned int       BufferSize;
   int                HeaderLength;
   Boolean_t          Success;
   OBEX_Header_List_t HeaderList;
   OBEX_Header_t      Header;
   char              *ResponseBufferPtr;
   OTP_Event_Data_t  *EventData;
   Byte_t             Buffer[OTP_EVENT_DATA_SIZE+OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE];

   Success = FALSE;
   if(OTPInfoPtr)
   {
      /* There is folder information, so we must package up as much     */
      /* information as we can and send the folder information to the   */
      /* Client.  Start by setting a pointer to the beginning of a      */
      /* buffer to hold the XML formatted directory information.        */
      ResponseBufferPtr = (char *)OTPInfoPtr->ResponseBuffer;

      /* Before we can format the data, we need to determine how much   */
      /* data we can format.  We are limited by the size of the buffer  */
      /* that we have to contain the formatted data or by the           */
      /* MaxPacketSize of the connection.  If we are limited by the     */
      /* MaxPacketSize then we also must consider the headers that will */
      /* also be added to the packet.  This amount of data that is      */
      /* required for the Header is 6 bytes.  This is 3 bytes for the   */
      /* Response and Length and 3 bytes for the Header ID and length.  */
      /* Max Packet size when the connection was made.                  */
      BufferSize = (OTPInfoPtr->MaxPacketSize-6);

      /* Check to see if there is Folder Information.                   */
      if(OTPInfoPtr->DirInfo.ObjectInfoList)
      {
         /* Convert the array of directory information into an XML      */
         /* representation of the data.  Pass to the formatting function*/
         /* a pointer to the array of directory entry information and a */
         /* pointer to a buffer in which to place the formatted data,   */
         /* along with the length of the buffer                         */
         i = FileObjectToXML(OTPInfoPtr->DirInfo.NumberEntries, &OTPInfoPtr->DirInfo.ObjectInfoList[OTPInfoPtr->DirInfo.DirectoryIndex], BufferSize, &ResponseBufferPtr, (Boolean_t)!OTPInfoPtr->DirInfo.ParentDirectory, &(OTPInfoPtr->DirInfo.SegmentationState), &(OTPInfoPtr->DirInfo.SegmentedLineLength));
      }
      else
      {
         /* If the DirInfoPtr is NULL, then we must be in a directory   */
         /* that has no file to present to the Client.  Therefore, Build*/
         /* an empty Directory Structure.                               */
         i = FileObjectToXML(OTPInfoPtr->DirInfo.NumberEntries, NULL, BufferSize, &ResponseBufferPtr, (Boolean_t)!OTPInfoPtr->DirInfo.ParentDirectory, &(OTPInfoPtr->DirInfo.SegmentationState), &(OTPInfoPtr->DirInfo.SegmentedLineLength));
      }

      /* The return value of the function StructToXML indicates the     */
      /* number of directory entries that was converted to XML.  The    */
      /* entire directory may not fit in the buffer that is supplied, so*/
      /* we need to update some information so that we can continue from*/
      /* where we left off when later requested.  Note the number of    */
      /* entries that have not been processed and the index of the next */
      /* entry that need to be formatted.  There are 2 return values    */
      /* that are used to indicate special processing is required.  If  */
      /* the return is INVALID_PARAMETER, we will need to report this to*/
      /* the Client.  If the parameter is FOOTER_REMAINING, then all    */
      /* entries were processed, but there was not enough memory left to*/
      /* insert the footer.  In this situation, we will need to be      */
      /* called again in order to format the footer.                    */
      if(i != INVALID_PARAMETER)
      {
         /* If we are here, then we sill send the Client some response, */
         /* so set the status to success.                               */
         Success = TRUE;

         /* Calculate the number of entries that are remaining to be    */
         /* formatted.                                                  */
         OTPInfoPtr->DirInfo.NumberEntries  -= i;
         OTPInfoPtr->DirInfo.DirectoryIndex += i;

         /* Calculate the amount of XML data that has been formatted by */
         /* subtracting the address of the start of the buffer from the */
         /* address of the next available memory address in he output   */
         /* buffer.                                                     */
         HeaderLength = ResponseBufferPtr-(char *)OTPInfoPtr->ResponseBuffer;

         /* Build the Headers that are needed for the data.  There are  */
         /* some conditional information added depending on whether this*/
         /* is the last packet.                                         */
         HeaderList.NumberOfHeaders                    = 1;
         HeaderList.Headers                            = &Header;
         Header.OBEX_Header_ID                         = (OTPInfoPtr->DirInfo.NumberEntries)?hidBody:hidEndOfBody;
         Header.OBEX_Header_Type                       = htByteSequence;
         Header.Header_Value.ByteSequence.DataLength   = (Word_t)HeaderLength;
         Header.Header_Value.ByteSequence.ValuePointer = OTPInfoPtr->ResponseBuffer;

         _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)(((OTPInfoPtr->DirInfo.SegmentationState != ssComplete)?OBEX_CONTINUE_RESPONSE:OBEX_OK_RESPONSE) | OBEX_FINAL_BIT), &HeaderList);

         /* Check to see if we have formatted the last packet.  If so,  */
         /* then we can get rid of the memory that was allocated for the*/
         /* Directory information.  We only have to free memory if there*/
         /* was memory allocated for the directory information.  If     */
         /* there was no directory information, then the Info Pointer is*/
         /* NULL and there is no data to free.                          */
         if(OTPInfoPtr->DirInfo.SegmentationState == ssComplete)
         {
            OTPInfoPtr->ServerState          = ssIdle;
            OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
            OTPInfoPtr->ObjectInfo.FieldMask = 0;

            /* Call the application that allocated the memory so that   */
            /* the memory can be properly deallocated.                  */
            if(OTPInfoPtr->DirInfo.ObjectInfoList)
            {
               if(OTPInfoPtr->OTP_EventCallback)
               {
                  EventData                                                                              = (OTP_Event_Data_t *)Buffer;
                  EventData->Event_Data_Type                                                             = etOTP_Free_Directory_Information;
                  EventData->Event_Data_Size                                                             = OTP_FREE_DIRECTORY_INFORMATION_DATA_SIZE;
                  EventData->Event_Data.OTP_Free_Directory_Information_Data                              = (OTP_Free_Directory_Information_Data_t *)(&Buffer[OTP_EVENT_DATA_SIZE]);
                  EventData->Event_Data.OTP_Free_Directory_Information_Data->OTP_ID                      = OTPInfoPtr->OTP_ID;

                  EventData->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.NumberEntries = OTPInfoPtr->DirInfo.DirectoryIndex;
                  EventData->Event_Data.OTP_Free_Directory_Information_Data->DirectoryInfo.ObjectInfo    = OTPInfoPtr->DirInfo.ObjectInfoList;

                  /* Flag that we have freed the Object Information     */
                  /* List.                                              */
                  OTPInfoPtr->DirInfo.NumberEntries  = 0;
                  OTPInfoPtr->DirInfo.ObjectInfoList = NULL;

                  /* Finally make the callback.                         */
                  __BTPSTRY
                  {
                     (*OTPInfoPtr->OTP_EventCallback)(EventData, OTPInfoPtr->CallbackParameter);
                  }
                  __BTPSEXCEPT(1)
                  {
                     /* Do Nothing.                                     */
                  }
               }
               else
               {
                  /* Flag that we have freed the Object Information     */
                  /* List.                                              */
                  OTPInfoPtr->DirInfo.NumberEntries  = 0;
                  OTPInfoPtr->DirInfo.ObjectInfoList = NULL;
               }
            }
            else
            {
               /* Make sure that the Number Entries is also reset.      */
               OTPInfoPtr->DirInfo.NumberEntries  = 0;
            }
         }
      }

      if(Success == FALSE)
      {
         /* Indicate the request can not be processed due to an internal*/
         /* problem.                                                    */
         _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_INTERNAL_SERVER_ERROR_RESPONSE | OBEX_FINAL_BIT), NULL);

         OTPInfoPtr->ServerState          = ssIdle;
         OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
         OTPInfoPtr->ObjectInfo.FieldMask = 0;
      }
   }
}

   /* The following function is used to send a request to an OBEX Server*/
   /* to save, create or delete an object on the Server.  The function  */
   /* takes as its first parameter the Bluetooth Stack ID to identify   */
   /* the Stack on which the request is to be made.  The second         */
   /* parameter is an FTPM Mgr Information structure which identifies   */
   /* the OBEX Connection on which the request is to be made.  The      */
   /* OTP_ID parameter references the OBEX Connection on which the      */
   /* request is to be made.  The Action parameter is used to specify if*/
   /* this request is being made to Create, Delete or Put an Object.    */
   /* When the action is to Put the object, the Length parameter        */
   /* specified the length of the object in bytes that is to be Put.    */
   /* The Type parameter is a pointer to a NULL terminated string that  */
   /* identifies the Type of object the request is for.  The Name       */
   /* parameter is a NULL terminated string the identifies the name of  */
   /* the object for which the request is for.  UserInfo is a user      */
   /* defined parameter.  This UserInfo parameter will be returned in   */
   /* the associated Put Response callback.  This function returns zero */
   /* if successful, or a negative return value if there was an error.  */
   /* It should be noted that when connected to an OBEX File Browser    */
   /* Service, the Type parameter is optional.  When connected to the   */
   /* OBEX Inbox, the Name parameter is optional.                       */
static int BuildPutObjectRequest(OTPM_Info_t *OTPInfoPtr, unsigned int OTP_ID, Byte_t Action, unsigned int Length, char *Type, char *Name, unsigned long UserInfo)
{
   int                 ret_val;
   int                 NameLength;
   int                 ParamStrLength;
   Byte_t             *UnicodeText = NULL;
   Byte_t             *AppParameter = NULL;
   char                ChangeCountBuffer[33];
   Byte_t              Index;
   OBEX_Header_t       Header[4];
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTPInfoPtr) && (OTP_ID) && (Name) && (*Name))
   {
      /* Check to see if we are in an Idle State.                       */
      if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_NONE)
      {
         Index                        = 0;
         HeaderList.Headers           = Header;
         OTPInfoPtr->UserInfo         = UserInfo;
         OTPInfoPtr->CurrentOperation = OTP_OPERATION_PUT_OBJECT;

         /* Check the Action that is being requested and set the Current*/
         /* Operation based on the Action specified.                    */
         switch(Action)
         {
            case OTP_OBJECT_PUT_ACTION_CREATE:
               OTPInfoPtr->CurrentOperation = OTP_OPERATION_CREATE;
               break;
            case OTP_OBJECT_PUT_ACTION_DELETE:
               OTPInfoPtr->CurrentOperation = OTP_OPERATION_DELETE;
               break;
            default:
               /* Must be an Object Put Operation, now figure out if    */
               /* it's a File or an Object.                             */
               OTPInfoPtr->CurrentOperation = OTP_OPERATION_PUT_OBJECT;

               if(OTPInfoPtr->Target == tFileBrowser)
                  OTPInfoPtr->CurrentOperation = OTP_OPERATION_PUT_FILE;

               if(OTPInfoPtr->Target == tIRSync)
                  OTPInfoPtr->CurrentOperation = OTP_OPERATION_PUT_SYNC_OBJECT;
               break;
         }

         if((OTPInfoPtr->Target == tFileBrowser) || (OTPInfoPtr->Target == tIRSync))
         {
            /* If a Connection ID is assigned for this session then     */
            /* insert the ID else insert the UUID.                      */
            if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
            {
               Header[Index].OBEX_Header_ID             = hidConnectionID;
               Header[Index].OBEX_Header_Type           = htUnsignedInteger4Byte;
               Header[Index].Header_Value.FourByteValue = OTPInfoPtr->ConnectionID;
            }
            else
            {
               Header[Index].OBEX_Header_ID                         = hidTarget;
               Header[Index].OBEX_Header_Type                       = htByteSequence;
               Header[Index].Header_Value.ByteSequence.DataLength   = sizeof(FolderBrowseUUID);
               Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)FolderBrowseUUID;
            }

            Index++;
         }

         if(OTPInfoPtr->Target == tIRSync)
         {
            /* When building a request for IR Sync, the Type field is   */
            /* not used.  Clear the field to insure a Type Header will  */
            /* not be inserted if the call has supplied one.            */
            Type = NULL;

            /* If a Change Count is being used, then we need to insert  */
            /* the Max Expected Change Count value in an OBEX           */
            /* Application Parameter Header.                            */
            if(OTPInfoPtr->SyncAnchor.ChangeCountUsed)
            {
               /* The Sync PUT may require OBEX Application parameters. */
               /* The Application information must be converted from a  */
               /* structure to a Tag, Length and Value series.  Before  */
               /* we can convert the data, we need to determine how many*/
               /* bytes are required to save the data in the Tag, Length*/
               /* and Value format.  Allocate Memory for the data and   */
               /* copy the data from the structure to memory.           */
#if BTPS_CONFIGURATION_FTPM_SUPPORT_LONG_SPRINTF_MODIFIER

               BTPS_SprintF(ChangeCountBuffer, "%lu", (unsigned long)OTPInfoPtr->SyncAnchor.ChangeCount);

#else

               BTPS_SprintF(ChangeCountBuffer, "%u", (unsigned int)OTPInfoPtr->SyncAnchor.ChangeCount);

#endif

               ParamStrLength = BTPS_StringLength(ChangeCountBuffer);

               if((AppParameter = (Byte_t *)BTPS_AllocateMemory(OTP_TAG_LENGTH_VALUE_SIZE(ParamStrLength))) != NULL)
               {
                  ((OTP_Tag_Length_Value_t *)AppParameter)->Tag    = OTP_SYNC_REQUEST_TAG_EXPECTED_CHANGE_COUNTER;
                  ((OTP_Tag_Length_Value_t *)AppParameter)->Length = (Byte_t)ParamStrLength;

                  BTPS_MemCopy(((OTP_Tag_Length_Value_t *)AppParameter)->Value, ChangeCountBuffer, ParamStrLength);

                  Header[Index].OBEX_Header_ID                         = hidApplicationParameters;
                  Header[Index].OBEX_Header_Type                       = htByteSequence;
                  Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)OTP_TAG_LENGTH_VALUE_SIZE(ParamStrLength);
                  Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)AppParameter;

                  Index++;
               }
            }
         }

         /* Insert the Length if supplied.                              */
         if((Length) && (Action == OTP_OBJECT_PUT_ACTION_PUT_OBJECT))
         {
            /* According to Bluetooth Specification, the Length Field is*/
            /* required for Object Push.  required.                     */
            Header[Index].OBEX_Header_ID             = hidLength;
            Header[Index].OBEX_Header_Type           = htUnsignedInteger4Byte;
            Header[Index].Header_Value.FourByteValue = (DWord_t)Length;

            Index++;
         }

         /* Convert the Name parameter from a Byte Sequence to a Unicode*/
         /* string and insert the Header.                               */
         NameLength = BTPS_StringLength(Name);
         if(NameLength)
         {
            NameLength++;
            if((UnicodeText = (Byte_t *)BTPS_AllocateMemory(2*NameLength)) != NULL)
            {
               CharSequenceToUnicodeHeaderValue(Name, UnicodeText, NameLength);

               Header[Index].OBEX_Header_ID                         = hidName;
               Header[Index].OBEX_Header_Type                       = htNullTerminatedUnicodeText;
               Header[Index].Header_Value.UnicodeText.DataLength    = (Word_t)NameLength;
               Header[Index].Header_Value.UnicodeText.ValuePointer  = (Word_t *)UnicodeText;

               Index++;
            }
         }

         /* If the Type parameter is specified, insert the type header. */
         if((Type) && (*Type))
         {
            Header[Index].OBEX_Header_ID                         = hidType;
            Header[Index].OBEX_Header_Type                       = htByteSequence;
            Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)(BTPS_StringLength(Type) + 1);
            Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Type;

            Index++;
         }

         if(Action == OTP_OBJECT_PUT_ACTION_CREATE)
         {
            /* To create and object, the End of Body header is send with*/
            /* an EndOfBody length of zero.                             */
            Header[Index].OBEX_Header_ID                         = hidEndOfBody;
            Header[Index].OBEX_Header_Type                       = htByteSequence;
            Header[Index].Header_Value.ByteSequence.DataLength   = 0;
            Header[Index].Header_Value.ByteSequence.ValuePointer = NULL;

            Index++;
         }

         /* Load the number of headers and send the request.            */
         HeaderList.NumberOfHeaders = Index;

         ret_val = _GOEP_Put_Request(OTPInfoPtr->GOEP_ID, (Boolean_t)((Action == OTP_OBJECT_PUT_ACTION_PUT_OBJECT)?FALSE:TRUE), &HeaderList);

         /* If the GOEP Request failed we need to change the Current    */
         /* Operation back to NO Operation (because we already changed  */
         /* the state earlier).                                         */
         if(ret_val < 0)
            OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;

         /* If memory was allocated to hold the Unicode string then free*/
         /* the memory.                                                 */
         if(UnicodeText)
            BTPS_FreeMemory(UnicodeText);

         /* If memory was allocated to hold the Application Parameters  */
         /* then free the memory.                                       */
         if(AppParameter)
            BTPS_FreeMemory(AppParameter);
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for initializing an FTPM Mgr*/
   /* Context Layerk.  This function returns zero if successful, or a   */
   /* non-zero value if there was an error.                             */
int _FTPM_Initialize(void)
{
   int ret_val;

   /* Wait for access to the lock that guards access to this module.    */
   if(DEVM_AcquireLock())
   {
      /* Clear the pointer to the start of the FTPM Mgr Info List.      */
      OTPInfoList      = NULL;

      /* Flag that this module is now initialized.                      */
      Initialized      = TRUE;

      /* Initialize a unique, starting Connection ID.                   */
      NextConnectionID = 0;

      /* Initialize a unique, starting FTPM Mgr ID.                     */
      NextOTPID        = 0;

      /* Initialize the port information list.                          */
      PortInfoList     = NULL;

      /* Initialize a unique, starting GOEP ID.                         */
      NextGOEPID       = 0;

      /* Return success to the caller.                                  */
      ret_val          = 0;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
   else
      ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;

   /* Finally, return the result to the caller.                         */
   return(ret_val);
}

   /* The following function is responsible for releasing any resources */
   /* that the FTPM Mgr Layer has allocated.  Upon completion of this   */
   /* function, ALL FTPM Mgr functions will fail until _FTPM_Initialize */
   /* is called again successfully.                                     */
void _FTPM_Cleanup(void)
{
   /* Wait for access to the lock that guards access to this module.    */
   if(DEVM_AcquireLock())
   {
      /* Free the FTPM Mgr Info List.                                   */
      FreeOTPInfoList(&(OTPInfoList));

      /* Free the port information list.                                */
      FreeGOEPInfoList(&PortInfoList);

      /* Flag that this module is no longer initialized.                */
      Initialized = FALSE;

      /* Release the Lock because we are finished with it.              */
      DEVM_ReleaseLock();
   }
}

   /* The following function is responsible for establishing an FTPM Mgr*/
   /* Port Server (will wait for a connection to occur on the port      */
   /* established by this function).  This function accepts as input the*/
   /* Port Number to establish, followed by the Port Flags.  The third  */
   /* parameter specifies the type of OBEX Server that is to be         */
   /* established (File Browser, IrSync, or Inbox).  The function also  */
   /* takes as a parameter the Maximum Packet Length that will be       */
   /* accepted for this server.  If the value supplied is outside the   */
   /* valid range, then the value used will be the valid value closest  */
   /* to the value supplied.  The last two parameters specify the FTPM  */
   /* Mgr Event Callback function and Callback Parameter, respectively, */
   /* that will be called with FTPM Mgr Events that occur on the        */
   /* specified FTPM Mgr Port.  This function returns a non-zero,       */
   /* positive, number on success or a negative return error code if an */
   /* error occurred.  A successful return code will be a Port ID that  */
   /* can be used to reference the Opened FTPM Mgr Port in ALL other    */
   /* functions in this module (except the _FTPM_Open_Remote_Port()     */
   /* function).  Once a Server Port is opened, it can only be          */
   /* Un-Registered via a call to the _FTPM_Close_Server_Port() function*/
   /* (passing the return value from this function).  The               */
   /* _FTPM_Close_Port() function can be used to Disconnect a Client    */
   /* from the Server Port (if one is connected, it will NOT Un-Register*/
   /* the Server Port however).                                         */
int _FTPM_Open_Server_Port(unsigned int ServerPort, unsigned long PortFlags, OTP_Target_t Target, Word_t MaxPacketLength, _FTPM_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   int         ret_val;
   OTPM_Info_t OTPInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((ServerPort) && (MaxPacketLength >= OTP_PACKET_LENGTH_MINIMUM) && (MaxPacketLength <= OTP_PACKET_LENGTH_MAXIMUM) && (EventCallback))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            OTPInfo.OTP_ID = GetNextOTP_ID();

            /* Start an OBEX server on the specified port.              */
            ret_val = _GOEP_Open_Server_Port(ServerPort, PortFlags, MaxPacketLength, GOEP_Event_Callback, OTPInfo.OTP_ID);

            /* If the server was successfully opened, save information  */
            /* about the server port in the Context area.               */
            if(ret_val > 0)
            {
               /* Initialize the information about the server.          */
               OTPInfo.GOEP_ID              = ret_val;
               OTPInfo.Server               = TRUE;
               OTPInfo.CallbackParameter    = CallbackParameter;
               OTPInfo.ConnectionID         = INVALID_CONNECTION_ID;
               OTPInfo.GOEPPacketSize       = MaxPacketLength;
               OTPInfo.OTP_EventCallback    = EventCallback;
               OTPInfo.ServerState          = ssIdle;
               OTPInfo.ClientConnected      = FALSE;
               OTPInfo.PortConnectionState  = pcsNotConnected;
               OTPInfo.Target               = Target;
               OTPInfo.ResponseBuffer       = NULL;
               OTPInfo.DirEntrySegment      = NULL;
               OTPInfo.ExtendedNameBuffer   = NULL;
               OTPInfo.ObjectInfo.FieldMask = 0;

               /* Return the ID that references the FTPM Mgr Session.   */
               ret_val                      = OTPInfo.OTP_ID;

               /* Attempt to save the information in the list.  If the  */
               /* adding of the data to the list fails, then we will    */
               /* need to close the server port.                        */
               if(!AddOTPInfoEntry(&OTPInfoList, &OTPInfo))
               {
                  /* Close the Port and set the error value.            */
                  _GOEP_Close_Server_Port(OTPInfo.GOEP_ID);

                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }
         }
         else
         {
            /* The FTPM Mgr Module does not appear to be initialized.   */
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for Un-Registering an FTPM  */
   /* Mgr Port Server (which was Registered by a successful call to the */
   /* _FTPM_Open_Server_Port() function).  This function accepts as     */
   /* input the FTPM Mgr Server ID that is registered.  This function   */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
int _FTPM_Close_Server_Port(unsigned int OTP_ID)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate information about the Server Port that is being   */
            /* referenced.                                              */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* Unregister the Server from the lower layers and remove*/
               /* all information about the server from the Linked List */
               /* of servers.                                           */
               ret_val = _GOEP_Close_Server_Port(OTPInfoPtr->GOEP_ID);

               if((OTPInfoPtr = DeleteOTPInfoEntry(&OTPInfoList, OTP_ID)) != NULL)
                  FreeOTPInfoEntryMemory(OTPInfoPtr);
            }
            else
            {
               /* No Information about the specified server was located.*/
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
         {
            /* The FTPM Mgr layer has not yet been initialized.         */
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
      {
         /* Unable to acquire the Stack information.                    */
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
      }
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for responding to requests  */
   /* to connect to an FTPM Mgr Port Server.  This function accepts as  */
   /* input the FTPM Mgr Port ID (which *MUST* have been obtained by    */
   /* calling the _FTPM_Open_Server_Port() function), and as the final  */
   /* parameter whether to accept the pending connection.  This function*/
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
int _FTPM_Open_Port_Request_Response(unsigned int OTP_ID, Boolean_t AcceptConnection)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information about the FTPM Mgr Session that is*/
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, next check to make*/
               /* sure that this FTPM Mgr Session Information is        */
               /* associated with a Server who's Port has a pending     */
               /* connection.                                           */
               if((OTPInfoPtr->Server) && (OTPInfoPtr->PortConnectionState == pcsConnectionPending))
               {
                  /* This is a server and it is currently has a         */
                  /* connection pending.  Next simply submit the Open   */
                  /* Port Request Response to the lower layer.          */
                  if((ret_val = _GOEP_Open_Port_Request_Response(OTPInfoPtr->GOEP_ID, AcceptConnection)) == 0)
                  {
                     /* The Open Port Request Response was successfully */
                     /* submitted to the lower layer.  Next check to see*/
                     /* if this was a call to reject the connection.    */
                     if(AcceptConnection == FALSE)
                     {
                        /* The connection was reject, flag that we are  */
                        /* no longer connected.                         */
                        OTPInfoPtr->PortConnectionState = pcsNotConnected;
                     }
                  }
               }
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is provided to allow a means to add a      */
   /* Generic OBEX Service Record to the SDP Database.  This function   */
   /* takes as input the FTPM Mgr Port ID (which *MUST* have been       */
   /* obtained by calling the _FTPM_Open_Server_Port() function.  The   */
   /* second parameter (required) specifies any additional SDP          */
   /* Information to add to the record.  The third parameter specifies  */
   /* the Service Name to associate with the SDP Record.  The final     */
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the FTPM Mgr    */
   /*          Port ID that was returned from the                       */
   /*          _FTPM_Open_Server_Port() function.  This function should */
   /*          NEVER be used with the FTPM Mgr Port ID returned from the*/
   /*          _FTPM_Open_Remote_Port() function.                       */
   /* * NOTE * There must be UUID Information specified in the          */
   /*          SDPServiceRecord Parameter, however protocol information */
   /*          is completely optional.  Any Protocol Information that is*/
   /*          specified (if any) will be added in the Protocol         */
   /*          Attribute AFTER the default OBEX Protocol List (L2CAP,   */
   /*          RFCOMM, and OBEX).                                       */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
int _FTPM_Register_SDP_Record(unsigned int OTP_ID, OTP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle)
{
   int                        ret_val;
   OTPM_Info_t                *OTPInfoPtr;
   GOEP_SDP_Service_Record_t  GOEP_SDP_Service_Record;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (ServiceName) && (BTPS_StringLength(ServiceName)) && (SDPServiceRecord) && (SDPServiceRecordHandle))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Now let's find the OBEX Server Port based upon the ID    */
            /* that was specified.                                      */
            if((OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID)) != NULL)
            {
               /* Now let's Add a generic OBEX SDP Record to the SDP    */
               /* Database (we will let GOEP handle all of the grunt    */
               /* work for us).                                         */

               /* Simply map the FTPM Mgr SDP Information directly to   */
               /* the GOEP SDP Information structure.                   */
               GOEP_SDP_Service_Record.NumberServiceClassUUID = SDPServiceRecord->NumberServiceClassUUID;
               GOEP_SDP_Service_Record.SDPUUIDEntries         = SDPServiceRecord->SDPUUIDEntries;
               GOEP_SDP_Service_Record.ProtocolList           = SDPServiceRecord->ProtocolList;

               ret_val = _GOEP_Register_SDP_Record(OTPInfoPtr->GOEP_ID, &GOEP_SDP_Service_Record, ServiceName, SDPServiceRecordHandle);
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for Opening a port to a     */
   /* remote Server on the specified Server Port.  This function accepts*/
   /* the BD_ADDR of the remote server.  The second parameter specifies */
   /* the port on which the server is attached.  The third parameter    */
   /* specifies the optional flags to use when opening the port.  The   */
   /* next parameter specifies the Max Packet Length that this client is*/
   /* capable of receiving.  The final two parameters specify the Event */
   /* Callback function, and callback parameter, respectively, of the   */
   /* Event Callback that is to process the FTPM Mgr Events.  This      */
   /* function returns a non-zero, positive, value if successful, or a  */
   /* negative return error code if this function is unsuccessful.  If  */
   /* this function is successful, the return value will represent the  */
   /* FTPM Mgr ID that can be passed to all other functions that require*/
   /* it.  Once a remote server is opened, it can only be closed via a  */
   /* call to the _FTPM_Close_Port() function (passing the return value */
   /* from this function).                                              */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
int _FTPM_Open_Remote_Port(BD_ADDR_t BD_ADDR, unsigned int ServerPort, unsigned long OpenFlags, Word_t MaxPacketLength, _FTPM_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ConnectionStatus)
{
   int         ret_val;
   OTPM_Info_t OTPInfo;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (ServerPort) && (MaxPacketLength >= OTP_PACKET_LENGTH_MINIMUM) && (MaxPacketLength <= OTP_PACKET_LENGTH_MAXIMUM) && (EventCallback))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            OTPInfo.OTP_ID = GetNextOTP_ID();

            /* Start an OBEX server on the specified port.              */
            ret_val = _GOEP_Open_Remote_Port(BD_ADDR, ServerPort, OpenFlags, MaxPacketLength, GOEP_Event_Callback, OTPInfo.OTP_ID, ConnectionStatus);

            /* If the server was successfully opened, save information  */
            /* about the server port in the Context area.               */
            if(ret_val > 0)
            {
               /* Initialize the information about the server.          */
               OTPInfo.GOEP_ID              = ret_val;
               OTPInfo.Server               = FALSE;
               OTPInfo.ServerState          = ssIdle;
               OTPInfo.ClientConnected      = FALSE;
               OTPInfo.PortConnectionState  = pcsNotConnected;
               OTPInfo.Target               = tUnknown;
               OTPInfo.UserInfo             = 0;
               OTPInfo.ConnectionID         = INVALID_CONNECTION_ID;
               OTPInfo.ResponseBuffer       = NULL;
               OTPInfo.DirEntrySegment      = NULL;
               OTPInfo.ExtendedNameBuffer   = NULL;
               OTPInfo.ObjectInfo.FieldMask = 0;
               OTPInfo.GOEPPacketSize       = MaxPacketLength;
               OTPInfo.OTP_EventCallback    = EventCallback;
               OTPInfo.CallbackParameter    = CallbackParameter;
               OTPInfo.CurrentOperation     = OTP_OPERATION_NONE;
               OTPInfo.GOEPPacketSize       = MaxPacketLength;

               /* Return the ID that references the FTPM Mgr Session.   */
               ret_val                      = OTPInfo.OTP_ID;

               /* Attempt to save the information in the list.  If the  */
               /* adding of the data to the list fails, then we will    */
               /* need to close the server port.                        */
               if(!AddOTPInfoEntry(&OTPInfoList, &OTPInfo))
               {
                  /* Close the Port and set the error value.            */
                  _GOEP_Close_Port(OTPInfo.GOEP_ID);

                  ret_val = BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY;
               }
            }
         }
         else
         {
            /* The FTPM Mgr Module does not appear to be initialized.   */
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to terminate a possible connection */
   /* to a remote server or client.  If this is called by a Server, the */
   /* connection to the client will be terminated, but the Server will  */
   /* remain registered.  This function accepts as input the Port ID    */
   /* that was returned in the _FTPM_Open_Server_Port() or the          */
   /* _FTPM_Open_Remote_Port().  This function returns zero if          */
   /* successful, or a negative return value if there was an error.     */
   /* This function does NOT Un-Register a Server Port from the system, */
   /* it ONLY disconnects any connection that is currently active on the*/
   /* Server Port.  The _FTPM_Close_Server_Port() function can be used  */
   /* to Un-Register the Server.                                        */
int _FTPM_Close_Port(unsigned int OTP_ID)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Find information about the FTPM Mgr Session that is begin*/
            /* referenced.                                              */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* Send the Command to Close the port.                   */
               ret_val = _GOEP_Close_Port(OTPInfoPtr->GOEP_ID);
               if(OTPInfoPtr->Server)
               {
                  /* If this is a Server, then we need to reset the     */
                  /* session specific information.  Check to see if any */
                  /* buffers that were allocated are need to be         */
                  /* released.                                          */
                  if(OTPInfoPtr->ResponseBuffer)
                     BTPS_FreeMemory(OTPInfoPtr->ResponseBuffer);

                  if(OTPInfoPtr->DirEntrySegment)
                     BTPS_FreeMemory(OTPInfoPtr->DirEntrySegment);

                  if(OTPInfoPtr->ExtendedNameBuffer)
                     BTPS_FreeMemory(OTPInfoPtr->ExtendedNameBuffer);

                  /* Reset the state information back to the Idle state.*/
                  OTPInfoPtr->ConnectionID         = INVALID_CONNECTION_ID;
                  OTPInfoPtr->ServerState          = ssIdle;
                  OTPInfoPtr->ClientConnected      = FALSE;
                  OTPInfoPtr->PortConnectionState  = pcsNotConnected;
                  OTPInfoPtr->ResponseBuffer       = NULL;
                  OTPInfoPtr->DirEntrySegment      = NULL;
                  OTPInfoPtr->ExtendedNameBuffer   = NULL;
                  OTPInfoPtr->ObjectInfo.FieldMask = 0;
               }
               else
               {
                  /* Remove the entry for this session from the list of */
                  /* active sessions.                                   */
                  if((OTPInfoPtr = DeleteOTPInfoEntry(&OTPInfoList, OTP_ID)) != NULL)
                     FreeOTPInfoEntryMemory(OTPInfoPtr);
               }
            }
            else
            {
               /* Information for the requested FTPM Mgr session was not*/
               /* located.                                              */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
         {
            /* The FTPM Mgr Layer has not been initialized.             */
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to Send an OBEX Connect Request to */
   /* the Remote Server.  The remote server is referenced by the FTPM   */
   /* Mgr ID that was returned from an _FTPM_Open_Remote_Port.  This    */
   /* function accepts as input the OTP_ID parameter which references   */
   /* the connection on which the connect is to be sent, obtained from  */
   /* the Open Port function.  The Target parameter identifies the      */
   /* service on the remote server to which the connection is targeted. */
   /* The DigestChallenge and DigestResponse parameters are used to pass*/
   /* Authentication Request and Response information between Server and*/
   /* Clients.  These parameters should be set to NULL if authentication*/
   /* is not in use.  This function returns zero if successful, or a    */
   /* negative return value if there was an error.                      */
int _FTPM_Client_Connect(unsigned int OTP_ID, OTP_Target_t Target, OTP_Digest_Challenge_t *DigestChallenge, OTP_Digest_Response_t *DigestResponse)
{
   int                     ret_val;
   int                     HeaderSize;
   Byte_t                  Index;
   OTPM_Info_t            *OTPInfoPtr;
   OBEX_Header_t           Header[3];
   OBEX_Header_List_t      HeaderList;
   OTP_Tag_Length_Value_t *Challenge;
   OTP_Tag_Length_Value_t *Response;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate information about the FTPM Mgr Session that is    */
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are NOT Already Connected, and the specified  */
               /* Session does NOT specify an FTPM Mgr Server.          */
               if((!OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  /* Initialize parameters for the session that is      */
                  /* attempting to be created.                          */
                  Index                        = 0;
                  Challenge                    = NULL;
                  Response                     = NULL;
                  OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;
                  OTPInfoPtr->Target           = Target;
                  OTPInfoPtr->ConnectionID     = INVALID_CONNECTION_ID;
                  HeaderList.Headers           = Header;

                  /* If the connection is the File Browser or the Sync  */
                  /* Server then a UUID must be added to the Connect    */
                  /* Request.                                           */
                  if((Target == tFileBrowser) || (Target == tIRSync))
                  {
                     Header[Index].OBEX_Header_ID                         = hidTarget;
                     Header[Index].OBEX_Header_Type                       = htByteSequence;
                     Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)((Target == tFileBrowser)?sizeof(FolderBrowseUUID):sizeof(IrSyncUUID));
                     Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)((Target == tFileBrowser)?FolderBrowseUUID:IrSyncUUID);
                     Index++;

                     /* Check to see if there is a request to           */
                     /* Authenticate the Server.  If so, DigestChallenge*/
                     /* is a pointer to the Authentication data.        */
                     if(DigestChallenge)
                     {
                        /* The Authentication information must be       */
                        /* converted from a structure to a Tag, Length  */
                        /* and Value series.  Before we can convert the */
                        /* data, we need to determine how many bytes are*/
                        /* required to save the data in the Tag, Length */
                        /* and Value format.  Call the conversion       */
                        /* function first to calculate the amount of    */
                        /* memory required.  Passing the Destination    */
                        /* pointer as NULL instructs the function to    */
                        /* calculate the required amount of memory.     */
                        HeaderSize = StructureToTagLengthValue(hidAuthenticationChallenge, DigestChallenge, NULL);
                        if(HeaderSize)
                        {
                           /* Allocate the memory needed for the Tag,   */
                           /* Length and Value sequence and convert the */
                           /* data.                                     */
                           if((Challenge = (OTP_Tag_Length_Value_t *)BTPS_AllocateMemory(HeaderSize)) != NULL)
                           {
                              StructureToTagLengthValue(hidAuthenticationChallenge, DigestChallenge, Challenge);

                              Header[Index].OBEX_Header_ID                         = hidAuthenticationChallenge;
                              Header[Index].OBEX_Header_Type                       = htByteSequence;
                              Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)HeaderSize;
                              Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Challenge;

                              Index++;
                           }
                        }
                     }

                     /* Check to see if there is a request to supply a  */
                     /* response for an Authorization Challenge from the*/
                     /* Server.  If so, the Digest Response is a pointer*/
                     /* to the Authentication Response data.            */
                     if(DigestResponse)
                     {
                        /* Calculate the amount of memory required to   */
                        /* convert the structure information to a Tag,  */
                        /* Length and Value sequence.                   */
                        HeaderSize = StructureToTagLengthValue(hidAuthenticationResponse, DigestResponse, NULL);
                        if(HeaderSize)
                        {
                           /* Allocate the memory needed for the Tag,   */
                           /* Length and Value sequence and convert the */
                           /* data.                                     */
                           if((Response = (OTP_Tag_Length_Value_t *)BTPS_AllocateMemory(HeaderSize)) != NULL)
                           {
                              StructureToTagLengthValue(hidAuthenticationResponse, DigestResponse, Response);

                              Header[Index].OBEX_Header_ID                         = hidAuthenticationResponse;
                              Header[Index].OBEX_Header_Type                       = htByteSequence;
                              Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)HeaderSize;
                              Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Response;

                              Index++;
                           }
                        }
                     }
                  }

                  /* Load the number of Headers that will be sent with  */
                  /* this request and send the data to GOEP for         */
                  /* processing.                                        */
                  HeaderList.NumberOfHeaders = Index;
                  ret_val = _GOEP_Connect_Request(OTPInfoPtr->GOEP_ID, &HeaderList);

                  /* If memory was allocated for a Tag, Length and Value*/
                  /* sequence, free the memory that was allocated.      */
                  if(Challenge)
                     BTPS_FreeMemory(Challenge);

                  if(Response)
                     BTPS_FreeMemory(Response);
               }
               else
               {
                  /* Unable to issue the Client Connect, so return the  */
                  /* correct reason to the caller.                      */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_ALREADY_CONNECTED;
               }
            }
            else
            {
               /* No information was located for the FTPM Mgr Session   */
               /* that was referenced.                                  */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to Disconnect an OBEX connection.  */
   /* This function will disconnect from an OBEX service on the remote  */
   /* OBEX server without releasing the connection to the Server.  This */
   /* function accepts as input the OTP_ID parameter which references   */
   /* the connection that is to be disconnected.  This function returns */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
int _FTPM_Client_Disconnect(unsigned int OTP_ID)
{
   int                 ret_val;
   Byte_t              Index;
   OTPM_Info_t        *OTPInfoPtr;
   OBEX_Header_t       Header[1];
   OBEX_Header_List_t *HeaderListPtr;
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate information about the FTPM Mgr Session that is    */
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* Make sure that we have an FTPM Mgr Client Connection  */
               /* established before we try to issue the command to     */
               /* GOEP.                                                 */
               if((OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  Index                        = 0;
                  HeaderListPtr                = NULL;
                  OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;

                  /* If we are not connecting to the InBox, then a UUID */
                  /* or Connection ID must be added to the request.     */
                  if((OTPInfoPtr->Target == tFileBrowser) || (OTPInfoPtr->Target == tIRSync))
                  {
                     HeaderList.Headers = Header;
                     HeaderListPtr      = &HeaderList;

                     /* If a Connection ID has been assigned for this   */
                     /* session, the insert the ID.  If not, a UUID must*/
                     /* be added.                                       */
                     if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
                     {
                        Header[Index].OBEX_Header_ID                         = hidConnectionID;
                        Header[Index].OBEX_Header_Type                       = htUnsignedInteger4Byte;
                        Header[Index].Header_Value.FourByteValue             = OTPInfoPtr->ConnectionID;
                     }
                     else
                     {
                        Header[Index].OBEX_Header_ID                         = hidTarget;
                        Header[Index].OBEX_Header_Type                       = htByteSequence;
                        Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)((OTPInfoPtr->Target == tFileBrowser)?sizeof(FolderBrowseUUID):sizeof(IrSyncUUID));
                        Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)((OTPInfoPtr->Target == tFileBrowser)?FolderBrowseUUID:IrSyncUUID);
                     }

                     Index++;
                  }

                  /* Set the number of headers that are being added to  */
                  /* the request and send the request.                  */
                  HeaderList.NumberOfHeaders = Index;

                  ret_val = _GOEP_Disconnect_Request(OTPInfoPtr->GOEP_ID, HeaderListPtr);

                  /* If we were able to successfully issue the          */
                  /* Disconnect flag that we no longer have a Client    */
                  /* Connection.                                        */
                  if(ret_val >= 0)
                     OTPInfoPtr->ClientConnected = FALSE;
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
            {
               /* No information was located for the Session that is    */
               /* being referenced.                                     */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to request a directory listing from*/
   /* a remote OBEX File Browsing Server.  The function takes as its    */
   /* first parameter The OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The Name parameter*/
   /* is a pointer to a ASCIIZ string that identifies the name of the   */
   /* directory that is to be retreived.  When specifying the Name, No  */
   /* path information is allowed.  When retreiving a directory listing,*/
   /* the SETPATH function should be used to set the current directory. */
   /* This function is then called with the Name parameter set to NULL  */
   /* to pull the current directory.  If the Name parameter is not NULL,*/
   /* then Name must point to a ASCIIZ string of the name of a          */
   /* sub-directory that exists off the current directory.  It must also*/
   /* be noted that when the Name parameter is used, a sub-directory    */
   /* listing will be returned for the directory specified, however, the*/
   /* current directory will remain the same and will not be changed to */
   /* the sub-directory specified.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The Name parameter should be formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
int _FTPM_Client_Get_Directory(unsigned int OTP_ID, char *Name)
{
   int                 ret_val;
   int                 NameLength;
   Byte_t              Index;
   Byte_t             *UnicodeText = NULL;
   OTPM_Info_t        *OTPInfoPtr;
   OBEX_Header_t       Header[3];
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate information about the FTPM Mgr Session that is    */
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);

            /* This request is only allowed if we are connected to the  */
            /* File Browsing Service.                                   */
            if((OTPInfoPtr) && (OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server) && (OTPInfoPtr->Target == tFileBrowser))
            {
               Index = 0;

               /* If we are in the Idle mode, then this is the first    */
               /* request.  Only the first request is required to       */
               /* include the Connection ID or a UUID.                  */
               if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_NONE)
               {
                  /* Initialize the Header pointer and set the Operation*/
                  /* to denote that we are retrieving a Directory.      */
                  HeaderList.Headers                      = Header;
                  OTPInfoPtr->CurrentOperation            = OTP_OPERATION_GET_DIRECTORY;

                  /* Initialize the Get Response Parsing Data with      */
                  /* Initial values.                                    */
                  OTPInfoPtr->DirInfo.NumberEntries       = 0;
                  OTPInfoPtr->DirInfo.ObjectInfoList      = NULL;
                  OTPInfoPtr->DirInfo.SegmentationState   = ssStart;
                  OTPInfoPtr->DirInfo.SegmentedLineLength = -1;
                  OTPInfoPtr->DirInfo.ParentDirectory     = FALSE;

                  /* If a Connection ID has been assigned to the session*/
                  /* then include the ID else we must include the UUID. */
                  if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
                  {
                     Header[Index].OBEX_Header_ID                         = hidConnectionID;
                     Header[Index].OBEX_Header_Type                       = htUnsignedInteger4Byte;
                     Header[Index].Header_Value.FourByteValue             = OTPInfoPtr->ConnectionID;
                  }
                  else
                  {
                     Header[Index].OBEX_Header_ID                         = hidTarget;
                     Header[Index].OBEX_Header_Type                       = htByteSequence;
                     Header[Index].Header_Value.ByteSequence.DataLength   = sizeof(FolderBrowseUUID);
                     Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)FolderBrowseUUID;
                  }

                  Index++;

                  /* Add a header to indicate that Directory Information*/
                  /* is being requested.                                */
                  Header[Index].OBEX_Header_ID                         = hidType;
                  Header[Index].OBEX_Header_Type                       = htByteSequence;
                  Header[Index].Header_Value.ByteSequence.DataLength   = sizeof(FolderListingType);
                  Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)FolderListingType;
                  Index++;

                  /* Check to see if a Pointer to a Name exists and that*/
                  /* the pointer does not point to an empty string.     */
                  if((Name) && (*Name))
                  {
                     /* The Name field must be passed as a Unicode      */
                     /* string.  The Name will be passed to this routine*/
                     /* as a Byte Sequence.  Convert the Byte Sequence  */
                     /* to a Unicode String.  Add one byte to the length*/
                     /* of the string to account for a NULL terminator. */
                     NameLength = BTPS_StringLength(Name) + 1;
                     if((UnicodeText = (Byte_t *)BTPS_AllocateMemory(2*NameLength)) != NULL)
                     {
                        CharSequenceToUnicodeHeaderValue(Name, UnicodeText, NameLength);

                        Header[Index].OBEX_Header_ID                        = hidName;
                        Header[Index].OBEX_Header_Type                      = htNullTerminatedUnicodeText;
                        Header[Index].Header_Value.UnicodeText.DataLength   = (Word_t)NameLength;
                        Header[Index].Header_Value.UnicodeText.ValuePointer = (Word_t *)UnicodeText;

                        Index++;
                     }
                  }

                  /* Set the number of headers for the request and send */
                  /* the request.                                       */
                  HeaderList.NumberOfHeaders = Index;

                  ret_val = _GOEP_Get_Request(OTPInfoPtr->GOEP_ID, TRUE, &HeaderList);

                  /* If the GOEP Request failed we need to change the   */
                  /* Current Operation back to NO Operation (because we */
                  /* already changed the state earlier).                */
                  if(ret_val < 0)
                     OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;

                  /* If any memory was allocated to hold the Unicode    */
                  /* string free the memory allocated.                  */
                  if(UnicodeText)
                     BTPS_FreeMemory(UnicodeText);
               }
               else
               {
                  /* If we were not in Idle mode, then this command is  */
                  /* not allowed.                                       */
                  ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
            }
            else
            {
               /* Return the correct error error result to the caller.  */
               if((OTPInfoPtr) && (OTPInfoPtr->ClientConnected))
                  ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               else
                  ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to Pull and Object from a Remote   */
   /* OBEX Server.  The function takes as its first parameter the OTP_ID*/
   /* parameter which references the OBEX Connection on which the       */
   /* request is to be made.  The Type parameter is a pointer to a NULL */
   /* terminated string that describes the type of object to be         */
   /* retreived.  The Name parameter is a pointer to a NULL terminated  */
   /* string that specifies the Name of the Object that is to be        */
   /* retreived.  UserInfo is a user defined parameter.  This UserInfo  */
   /* parameter will be returned in the associated Get Response         */
   /* Callback.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.  It should be noted that when */
   /* connected to an OBEX File Browser Service, the Type parameter is  */
   /* optional.  When connected to the OBEX Inbox, the Name parameter is*/
   /* optional.                                                         */
   /* * NOTE * The Type and Name parameters should be formatted as NULL */
   /*          terminated ASCII strings with UTF-8 encoding.            */
int _FTPM_Client_Get_Object(unsigned int OTP_ID, char *Type, char *Name, unsigned long UserInfo)
{
   int                 ret_val;
   int                 NameLength;
   Byte_t              Index;
   Byte_t             *UnicodeText = NULL;
   OTPM_Info_t        *OTPInfoPtr;
   OBEX_Header_t       Header[3];
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate information about the FTPM Mgr Session that is    */
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session does */
               /* NOT specify an FTPM Mgr Server.                       */
               if((OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  /* If the target is the Inbox, then the Type header is*/
                  /* required.  If the target is the File Browser or the*/
                  /* Sync Server then the Name field is required.       */
                  if(((OTPInfoPtr->Target != tInbox) && (Name) && (*Name)) || ((OTPInfoPtr->Target == tInbox) && (Type) && (*Type)))
                  {
                     /* This request is only valid if we are in an Idle */
                     /* state.                                          */
                     if(OTPInfoPtr->CurrentOperation == OTP_OPERATION_NONE)
                     {
                        Index                = 0;
                        HeaderList.Headers   = Header;
                        OTPInfoPtr->UserInfo = UserInfo;

                        /* If we are not connected to the InBox, then we*/
                        /* will need to include the Connection ID or a  */
                        /* UUID.                                        */
                        if((OTPInfoPtr->Target == tFileBrowser) || (OTPInfoPtr->Target == tIRSync))
                        {
                           /* Set the Operation to Get File.  If we are */
                           /* connection to the Sync Server, we will    */
                           /* consider the object that is being         */
                           /* exchanged as a file.                      */
                           OTPInfoPtr->CurrentOperation = OTP_OPERATION_GET_FILE;

                           /* If an Connection ID is assigned to this   */
                           /* session then add the ID else we need to   */
                           /* add the UUID.                             */
                           if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
                           {
                              Header[Index].OBEX_Header_ID             = hidConnectionID;
                              Header[Index].OBEX_Header_Type           = htUnsignedInteger4Byte;
                              Header[Index].Header_Value.FourByteValue = OTPInfoPtr->ConnectionID;
                           }
                           else
                           {
                              Header[Index].OBEX_Header_ID                         = hidTarget;
                              Header[Index].OBEX_Header_Type                       = htByteSequence;
                              Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)((OTPInfoPtr->Target == tFileBrowser)?sizeof(FolderBrowseUUID):sizeof(IrSyncUUID));
                              Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)((OTPInfoPtr->Target == tFileBrowser)?FolderBrowseUUID:IrSyncUUID);
                           }
                           Index++;

                           /* The Name field is provided as a Byte      */
                           /* Sequence and must be converted to a       */
                           /* Unicode string.  Add one to the length of */
                           /* the Name to account for a NULL terminator.*/
                           NameLength  = BTPS_StringLength(Name)+1;

                           /* Allocate memory for the Unicode string and*/
                           /* convert the Byte sequence to a Unicode    */
                           /* string.                                   */
                           if((UnicodeText = (Byte_t *)BTPS_AllocateMemory(2*NameLength)) != NULL)
                           {
                              CharSequenceToUnicodeHeaderValue(Name, UnicodeText, NameLength);

                              /* Add the Name header information.       */
                              Header[Index].OBEX_Header_ID                        = hidName;
                              Header[Index].OBEX_Header_Type                      = htNullTerminatedUnicodeText;
                              Header[Index].Header_Value.UnicodeText.DataLength   = (Word_t)NameLength;
                              Header[Index].Header_Value.UnicodeText.ValuePointer = (Word_t *)UnicodeText;

                              Index++;
                           }

                           /* If the optional Type information is       */
                           /* included, the add the Type header.        */
                           if((Type) && (*Type))
                           {
                              Header[Index].OBEX_Header_ID                         = hidType;
                              Header[Index].OBEX_Header_Type                       = htByteSequence;
                              Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)(BTPS_StringLength(Type) + 1);
                              Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Type;
                              Index++;
                           }
                        }
                        else
                        {
                           /* Set the operation to Get Object.          */
                           OTPInfoPtr->CurrentOperation = OTP_OPERATION_GET_OBJECT;

                           /* Add the Type information Header.          */
                           Header[Index].OBEX_Header_ID                         = hidType;
                           Header[Index].OBEX_Header_Type                       = htByteSequence;
                           Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)(BTPS_StringLength(Type) + 1);
                           Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Type;
                           Index++;

                           /* If the optional Name information is       */
                           /* supplied, the Convert the Name Byte       */
                           /* Sequence to a Unicode String and add the  */
                           /* Header.                                   */
                           if((Name) && (*Name))
                           {
                              NameLength  = BTPS_StringLength(Name)+1;
                              if((UnicodeText = (Byte_t *)BTPS_AllocateMemory(2*NameLength)) != NULL)
                              {
                                 CharSequenceToUnicodeHeaderValue(Name, UnicodeText, NameLength);

                                 Header[Index].OBEX_Header_ID                        = hidName;
                                 Header[Index].OBEX_Header_Type                      = htNullTerminatedUnicodeText;
                                 Header[Index].Header_Value.UnicodeText.DataLength   = (Word_t)NameLength;
                                 Header[Index].Header_Value.UnicodeText.ValuePointer = (Word_t *)UnicodeText;

                                 Index++;
                              }
                           }
                        }

                        /* Set the number headers and send the request. */
                        HeaderList.NumberOfHeaders = Index;

                        ret_val = _GOEP_Get_Request(OTPInfoPtr->GOEP_ID, TRUE, &HeaderList);

                        /* If the GOEP Request failed we need to change */
                        /* the Current Operation back to NO Operation   */
                        /* (because we already changed the state        */
                        /* earlier).                                    */
                        if(ret_val < 0)
                           OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;

                        /* If memory was allocated for the Unicode      */
                        /* string, free the memory.                     */
                        if(UnicodeText)
                           BTPS_FreeMemory(UnicodeText);
                     }
                     else
                     {
                        /* This request is not allowed at this time.    */
                        ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
                     }
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
            {
               /* No information was located for the session that was   */
               /* referenced.                                           */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to send a request to an OBEX Server*/
   /* to save or create an object on the Server.  The function takes as */
   /* its first parameter the OTP_ID parameter which references the OBEX*/
   /* Connection on which the request is to be made.  The CreateOnly    */
   /* parameter specifies whether or not this request is being made to  */
   /* put an object (CreateOnly equals FALSE), or simply create an      */
   /* object of zero length (CreateOnly equals TRUE).  The length field */
   /* specifies the total size (in bytes of the Object).  The Type      */
   /* parameter is a pointer to a NULL terminated string that identifies*/
   /* the Type of object the request is for.  The Name parameter is a   */
   /* NULL terminated string that identifies the name of the object for */
   /* which the request is for.  UserInfo is a user defined parameter.  */
   /* This UserInfo parameter will be returned in the associated Put    */
   /* Response callback.  This function returns zero if successful, or a*/
   /* negative return value if there was an error.  It should be noted  */
   /* that when connected to an OBEX File Browser Service, the Type     */
   /* parameter is optional.  When connected to the OBEX Inbox, the Name*/
   /* parameter is optional.                                            */
   /* * NOTE * The Type and Name parameters should be formatted as NULL */
   /*          terminated ASCII strings with UTF-8 encoding.            */
int _FTPM_Client_Put_Object_Request(unsigned int OTP_ID, Boolean_t CreateOnly, unsigned int Length, char *Type, char *Name, unsigned long UserInfo)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session does */
               /* NOT specify an FTPM Mgr Server.                       */
               if((OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  /* Check the Target.  If the target is the Inbox, then*/
                  /* a length must be supplied.  This is a requirment of*/
                  /* Bluetooth and not OBEX.                            */
                  if((OTPInfoPtr->Target != tInbox) || ((OTPInfoPtr->Target == tInbox) && (Length)))
                  {
                     /* Now Simply Build and issue an FTPM Mgr Client   */
                     /* Put Request.                                    */
                     ret_val = BuildPutObjectRequest(OTPInfoPtr, OTP_ID, (Byte_t)(CreateOnly?OTP_OBJECT_PUT_ACTION_CREATE:OTP_OBJECT_PUT_ACTION_PUT_OBJECT), Length, Type, Name, UserInfo);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* This function is used to send an object to a remote OBEX Server.  */
   /* This function must be used after an acceptable response is        */
   /* received from the _FTPM_Client_Put_Object_Request and is used to  */
   /* transfer the object itself.  The function takes as its first      */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  The DataLength    */
   /* parameter specifies the number of bytes that are to be transferred*/
   /* in this packet.  The Data parameter is a pointer to DataLength    */
   /* number of bytes that are to be transferred.  The Final parameter  */
   /* is a Boolean_t that denotes if the data that is supplied via the  */
   /* Data parameter is the last block of object data to be transferred.*/
   /* UserInfo is a user defined parameter.  This UserInfo parameter    */
   /* will be returned in the associated Put Response callback.  This   */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
int _FTPM_Client_Put_Object(unsigned int OTP_ID, unsigned int DataLength, Byte_t *Data, Boolean_t Final, unsigned long UserInfo)
{
   int                 ret_val;
   Byte_t              Index;
   OTPM_Info_t        *OTPInfoPtr;
   OBEX_Header_t       Header[3];
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (DataLength) && (Data))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information for the FTPM Mgr session that is  */
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session does */
               /* NOT specify an FTPM Mgr Server.                       */
               if((OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  Index                = 0;
                  HeaderList.Headers   = Header;
                  OTPInfoPtr->UserInfo = UserInfo;

                  if((OTPInfoPtr->Target == tFileBrowser) || (OTPInfoPtr->Target == tIRSync))
                  {
                     /* If a Connection ID has been assigned to this    */
                     /* session then insert the ID else insert the UUID.*/
                     if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
                     {
                        Header[Index].OBEX_Header_ID             = hidConnectionID;
                        Header[Index].OBEX_Header_Type           = htUnsignedInteger4Byte;
                        Header[Index].Header_Value.FourByteValue = OTPInfoPtr->ConnectionID;
                     }
                     else
                     {
                        Header[Index].OBEX_Header_ID                         = hidTarget;
                        Header[Index].OBEX_Header_Type                       = htByteSequence;
                        Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)((OTPInfoPtr->Target == tFileBrowser)?sizeof(FolderBrowseUUID):sizeof(IrSyncUUID));
                        Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)((OTPInfoPtr->Target == tFileBrowser)?FolderBrowseUUID:IrSyncUUID);
                     }

                     Index++;
                  }

                  /* Insert the Body or End of Body header along with   */
                  /* the data.                                          */
                  Header[Index].OBEX_Header_ID                         = (OBEX_Header_ID_t)((Final)?hidEndOfBody:hidBody);
                  Header[Index].OBEX_Header_Type                       = htByteSequence;
                  Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)DataLength;
                  Header[Index].Header_Value.ByteSequence.ValuePointer = Data;

                  Index++;

                  /* Insert the number of Headers and send the request. */
                  HeaderList.NumberOfHeaders = Index;

                  ret_val = _GOEP_Put_Request(OTPInfoPtr->GOEP_ID, (Boolean_t)((Final)?TRUE:FALSE), &HeaderList);

                  /* If the GOEP Request failed we need to change the   */
                  /* Current Operation back to NO Operation (because we */
                  /* already changed the state earlier).                */
                  if(ret_val < 0)
                     OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to create, delete or set the       */
   /* current directory, of remote OBEX server supplying File Browsing  */
   /* Services.  The function takes as its first parameter the OTP_ID   */
   /* parameter which references the OBEX Connection on which the       */
   /* request is to be made.  The Name parameter is a pointer to a NULL */
   /* terminated string that specifies the name of a sub-directory with */
   /* reference to the current directory to which the path is to be set.*/
   /* The Backup parameter is used to request that the path be set to   */
   /* the next higher level.  The Create parameter is used to specify   */
   /* that the directory is to be created if it does not already exist. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.  Note that the Backup flag has the   */
   /* highest priority and The Name parameter will be ignored when      */
   /* Backup is set TRUE.  Also, when the Create parameter is TRUE the  */
   /* Name parameter must also be specified.                            */
   /* * NOTE * The Name parameter should be formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
int _FTPM_Client_Set_Path(unsigned int OTP_ID, char *Name, Boolean_t Backup, Boolean_t Create)
{
   int                 ret_val;
   int                 NameLength;
   Byte_t              Index;
   Byte_t              Flags;
   Byte_t             *UnicodeText = NULL;
   OTPM_Info_t        *OTPInfoPtr;
   OBEX_Header_t       Header[2];
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (!Create || ((Create) && (Name) && (*Name))))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information for the FTPM Mgr session that is  */
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session does */
               /* NOT specify an FTPM Mgr Server.                       */
               if((OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  /* Check to make sure the we are in a state to support*/
                  /* this request.                                      */
                  if((OTPInfoPtr->CurrentOperation == OTP_OPERATION_NONE) && (OTPInfoPtr->Target == tFileBrowser))
                  {
                     Index                        = 0;
                     HeaderList.Headers           = Header;
                     OTPInfoPtr->CurrentOperation = OTP_OPERATION_SET_PATH;

                     /* Convert the parameters to the bitmask flags that*/
                     /* are needed for this request.                    */
                     Flags = (Boolean_t)OBEX_SET_PATH_FLAGS_NO_CREATE_MASK;

                     if(Backup)
                       Flags |= (Boolean_t)OBEX_SET_PATH_FLAGS_BACKUP_MASK;
                     else
                     {
                        if(Create)
                           Flags &= (Boolean_t)~OBEX_SET_PATH_FLAGS_NO_CREATE_MASK;
                     }

                     /* If a Connection ID has been assigned to this    */
                     /* session then inset the ID else inset the UUID.  */
                     if(OTPInfoPtr->ConnectionID != INVALID_CONNECTION_ID)
                     {
                        Header[Index].OBEX_Header_ID             = hidConnectionID;
                        Header[Index].OBEX_Header_Type           = htUnsignedInteger4Byte;
                        Header[Index].Header_Value.FourByteValue = OTPInfoPtr->ConnectionID;
                     }
                     else
                     {
                        Header[Index].OBEX_Header_ID                         = hidTarget;
                        Header[Index].OBEX_Header_Type                       = htByteSequence;
                        Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)((OTPInfoPtr->Target == tFileBrowser)?sizeof(FolderBrowseUUID):sizeof(IrSyncUUID));
                        Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)((OTPInfoPtr->Target == tFileBrowser)?FolderBrowseUUID:IrSyncUUID);
                     }

                     Index++;

                     /* If we have not set the Backup flag, the we need */
                     /* to include a Name Header.                       */
                     /* * NOTE * The Name Header can be empty which     */
                     /*          signifies that we are changing to the  */
                     /*          Root Directory.                        */
                     if(!Backup)
                     {
                        if((Name) && (*Name))
                        {
                           /* Convert the Name to Unicode and insert the*/
                           /* Header.                                   */
                           NameLength  = BTPS_StringLength(Name)+1;
                           if((UnicodeText = (Byte_t *)BTPS_AllocateMemory(2*NameLength)) != NULL)
                           {
                              CharSequenceToUnicodeHeaderValue(Name, UnicodeText, NameLength);

                              Header[Index].OBEX_Header_ID                         = hidName;
                              Header[Index].OBEX_Header_Type                       = htNullTerminatedUnicodeText;
                              Header[Index].Header_Value.UnicodeText.DataLength    = (Word_t)NameLength;
                              Header[Index].Header_Value.UnicodeText.ValuePointer  = (Word_t *)UnicodeText;

                              Index++;
                           }
                        }
                        else
                        {
                           /* No name specified so format an empty Name */
                           /* Header.                                   */
                           Header[Index].OBEX_Header_ID                         = hidName;
                           Header[Index].OBEX_Header_Type                       = htNullTerminatedUnicodeText;
                           Header[Index].Header_Value.UnicodeText.DataLength    = (Word_t)0;
                           Header[Index].Header_Value.UnicodeText.ValuePointer  = NULL;

                           Index++;
                        }
                     }

                     /* Set the number of Headers send the request.     */
                     HeaderList.NumberOfHeaders = Index;

                     ret_val = _GOEP_Set_Path_Request(OTPInfoPtr->GOEP_ID, Flags, &HeaderList);

                     /* If the GOEP Request failed we need to change the*/
                     /* Current Operation back to NO Operation (because */
                     /* we already changed the state earlier).          */
                     if(ret_val < 0)
                        OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;

                     /* If memory was allocated to convert the Unicode  */
                     /* free the memory.                                */
                     if(UnicodeText)
                        BTPS_FreeMemory(UnicodeText);
                  }
                  else
                     ret_val = BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING;
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to send a request to an OBEX Server*/
   /* to delete an object on the Server.  The function takes as its     */
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The Name parameter*/
   /* is a NULL terminated string that identifies the name of the object*/
   /* for which the request is for.  This function returns zero if      */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The Name parameter should be formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
int _FTPM_Client_Delete_Object_Request(unsigned int OTP_ID, char *Name)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Find the information about the connection that this      */
            /* request is associated with.  You are not allowed to      */
            /* delete an object from another persons Inbox, so make sure*/
            /* this connection is not to an Inbox.                      */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if((OTPInfoPtr) && (OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server) && (OTPInfoPtr->Target != tInbox))
            {
               /* Now Simply issue an FTPM Mgr Client Put Request.      */
               ret_val = BuildPutObjectRequest(OTPInfoPtr, OTP_ID, OTP_OBJECT_PUT_ACTION_DELETE, 0, NULL, Name, (unsigned long)NULL);
            }
            else
            {
               /* Return the correct error error result to the caller.  */
               if((OTPInfoPtr) && (OTPInfoPtr->Server))
                  ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
               else
               {
                  if((OTPInfoPtr) && (OTPInfoPtr->ClientConnected))
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
                  else
                     ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
               }
            }
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to Abort a request to the remote   */
   /* server that is outstanding.  The function takes as its first      */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
int _FTPM_Client_Abort_Request(unsigned int OTP_ID)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session does */
               /* NOT specify an FTPM Mgr Server.                       */
               if((OTPInfoPtr->ClientConnected) && (!OTPInfoPtr->Server))
               {
                  /* This will send the Abort out even if the local     */
                  /* status indicates that we are in the Idle State.    */
                  /* This is because we might need to reset a confused  */
                  /* Server.                                            */
                  OTPInfoPtr->CurrentOperation = OTP_OPERATION_NONE;
                  OTPInfoPtr->UserInfo         = 0;

                  ret_val = _GOEP_Abort_Request(OTPInfoPtr->GOEP_ID, NULL);
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
            {
               /* No information was located for the OTP_ID that was    */
               /* specified.                                            */
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
            }
         }
         else
         {
            /* The FTPM Mgr module has not been initialized.            */
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
         }

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to send a response to a remote     */
   /* client due to a request for connection.  The function takes as its*/
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The parameter     */
   /* Accept is used to indicate if the connection request is being     */
   /* accepted or rejected.  When authentication is required for the    */
   /* connection, the structure DigestChallenge and DigestResponse is   */
   /* used to pass the Authentication information.  If authentication is*/
   /* not being used, these parameters should be set to NULL.  The      */
   /* DigestChallenge parameter is used to initiate authentication of   */
   /* the remote Client.  The DigestResponse is used to respond to a    */
   /* Challenge request from the remote client.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
int _FTPM_Connect_Response(unsigned int OTP_ID, Boolean_t Accept, OTP_Digest_Challenge_t *DigestChallenge, OTP_Digest_Response_t *DigestResponse)
{
   int                     ret_val;
   int                     HeaderSize;
   Byte_t                  Index;
   OTPM_Info_t            *OTPInfoPtr;
   OBEX_Header_t           Header[4];
   OBEX_Header_List_t     *HeaderListPtr;
   OBEX_Header_List_t      HeaderList;
   OTP_Tag_Length_Value_t *Challenge;
   OTP_Tag_Length_Value_t *Response;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((!OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  /* Check to see if the connection is being accepted.  */
                  if(Accept)
                  {
                     /* If the connection is being accepted, then check */
                     /* to see if the connection requires               */
                     /* Authentication.  If no Authentication is        */
                     /* required, then we can allocate memory for the   */
                     /* Response Buffer.  Postpone the creation of the  */
                     /* Response Buffer until we are sure the connection*/
                     /* is going to be established.                     */
                     if((DigestChallenge == NULL) && (OTPInfoPtr->ResponseBuffer == NULL))
                     {
                        /* Attempt to allocate memory for the Response  */
                        /* Buffer.                                      */
                        OTPInfoPtr->ResponseBuffer = (Byte_t *)BTPS_AllocateMemory(OTPInfoPtr->MaxPacketSize);
                     }

                     /* If the connection is ready to be established,   */
                     /* then make sure that we could allocate memory for*/
                     /* the Response Buffer.                            */
                     if((DigestChallenge) || ((DigestChallenge == NULL) && (OTPInfoPtr->ResponseBuffer)))
                     {
                        /* We have been requested to accept a connection*/
                        /* to the Folder Browsing Server.  Return the   */
                        /* Folder Browsing UUID in the Who header and   */
                        /* assign a Connection ID.                      */
                        Index                            = 0;
                        HeaderListPtr                    = NULL;
                        Challenge                        = NULL;
                        Response                         = NULL;
                        HeaderList.Headers               = Header;
                        OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                        OTPInfoPtr->ObjectInfo.FieldMask = 0;

                        if((OTPInfoPtr->Target == tFileBrowser) || (OTPInfoPtr->Target == tIRSync))
                        {
                           /* Return the Who Header to the remote       */
                           /* Client.  The Who Header will be the UUID  */
                           /* that was sent in the Connect request if   */
                           /* the connection to the requested target was*/
                           /* successful.                               */
                           HeaderListPtr                                        = &HeaderList;
                           Header[Index].OBEX_Header_ID                         = hidWho;
                           Header[Index].OBEX_Header_Type                       = htByteSequence;
                           Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)((OTPInfoPtr->Target == tFileBrowser)?sizeof(FolderBrowseUUID):sizeof(IrSyncUUID));
                           Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)((OTPInfoPtr->Target == tFileBrowser)?FolderBrowseUUID:IrSyncUUID);
                           Index++;

                           /* Assign a Connection ID for this FTPM Mgr  */
                           /* session.                                  */
                           Header[Index].OBEX_Header_ID                         = hidConnectionID;
                           Header[Index].OBEX_Header_Type                       = htUnsignedInteger4Byte;
                           Header[Index].Header_Value.FourByteValue             = OTPInfoPtr->ConnectionID;
                           Index++;

                           /* If the connection requires Authentication,*/
                           /* then DigestChallenge is a pointer to the  */
                           /* Challenge information.                    */
                           if(DigestChallenge)
                           {
                              /* Calculate the amount of memory required*/
                              /* to convert the Challenge information   */
                              /* into a Authentication string formatted */
                              /* per OBEX.                              */
                              HeaderSize = StructureToTagLengthValue(hidAuthenticationChallenge, DigestChallenge, NULL);
                              if(HeaderSize)
                              {
                                 /* Allocate memory for the             */
                                 /* authentication string and convert   */
                                 /* the Challenge information.          */
                                 if((Challenge  = (OTP_Tag_Length_Value_t *)BTPS_AllocateMemory(HeaderSize)) != NULL)
                                 {
                                    StructureToTagLengthValue(hidAuthenticationChallenge, DigestChallenge, Challenge);

                                    Header[Index].OBEX_Header_ID                         = hidAuthenticationChallenge;
                                    Header[Index].OBEX_Header_Type                       = htByteSequence;
                                    Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)HeaderSize;
                                    Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Challenge;

                                    Index++;
                                 }
                              }
                           }

                           /* If the Connect Request included a         */
                           /* Challenge for the Server, then            */
                           /* DigestResponse is a pointer to the        */
                           /* Challenge Response information.           */
                           if(DigestResponse)
                           {
                              /* Calculate the amount of memory required*/
                              /* to convert the Challenge information   */
                              /* into a Authentication string formatted */
                              /* per OBEX.                              */
                              HeaderSize = StructureToTagLengthValue(hidAuthenticationResponse, DigestResponse, NULL);
                              if(HeaderSize)
                              {
                                 /* Allocate memory for the             */
                                 /* authentication string and convert   */
                                 /* the Challenge Response information. */
                                 if((Response = (OTP_Tag_Length_Value_t *)BTPS_AllocateMemory(HeaderSize)) != NULL)
                                 {
                                    StructureToTagLengthValue(hidAuthenticationResponse, DigestResponse, Response);

                                    Header[Index].OBEX_Header_ID                         = hidAuthenticationResponse;
                                    Header[Index].OBEX_Header_Type                       = htByteSequence;
                                    Header[Index].Header_Value.ByteSequence.DataLength   = (Word_t)HeaderSize;
                                    Header[Index].Header_Value.ByteSequence.ValuePointer = (Byte_t *)Response;

                                    Index++;
                                 }
                              }
                           }
                        }

                        /* Load the number of headers and send the      */
                        /* response.                                    */
                        HeaderList.NumberOfHeaders = Index;

                        ret_val = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)((DigestChallenge)?OBEX_UNAUTHORIZED_RESPONSE:OBEX_OK_RESPONSE), HeaderListPtr);

                        /* If the command was successfully sent to GOEP,*/
                        /* then we need to make sure that have flagged  */
                        /* the connection as established.               */
                        if(!ret_val)
                           OTPInfoPtr->ClientConnected = TRUE;

                        /* If memory is allocated for the Challenge or  */
                        /* Response, then free the memory.              */
                        if(Challenge)
                           BTPS_FreeMemory(Challenge);

                        if(Response)
                           BTPS_FreeMemory(Response);
                     }
                     else
                     {
                        /* We were not able to allocate memory for the  */
                        /* response buffer, so we need to reject the    */
                        /* connection request.                          */
                        ret_val = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, OBEX_INTERNAL_SERVER_ERROR_RESPONSE, NULL);
                     }
                  }
                  else
                  {
                     /* The connection is not accepted, so deny.        */
                     ret_val = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, OBEX_FORBIDDEN_RESPONSE, NULL);
                  }
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to send a response to a remote     */
   /* client due to a request for a Directory listing.  The function    */
   /* takes as its first parameter the OTP_ID parameter which references*/
   /* the OBEX Connection on which the request is to be made.  The      */
   /* parameter DirEntry is a pointer to an array of directory entry    */
   /* structures.  Each entry in the array contains information about a */
   /* file or directory entry that is to be sent in response to the     */
   /* request.  It is important to note that the stack receives the     */
   /* directory information as an array of structures, and will convert */
   /* this information into XML format prior to sending to information  */
   /* to the remote client.  The process of converting the data to XML  */
   /* and sending all of the information to the remote client may       */
   /* require multiple requests and responses from the client and       */
   /* server.  The lower layer stack will handle all of these additional*/
   /* transactions without any further interaction from the application.*/
   /* Since the directory transfer process may take some time to        */
   /* complete, the data pointed to by the parameter DirInfo must be    */
   /* preserved until the transfer process is complete.  When the       */
   /* DirInfo information is no longer needed by the lower stack, a     */
   /* Callback will be generated with the                               */
   /* etOTP_Free_Directory_Information event to inform the application  */
   /* that the directory transfer process is complete and the data can  */
   /* be freed.  The parameter ResponseCode is used to notify the remote*/
   /* client of its ability to satisfy the request.  If the ResponseCode*/
   /* value is non-Zero, then the information pointed to by the DirInfo */
   /* parameter is considered invalid and the ResponseCode value        */
   /* represents the OBEX result code that identifies the reason why the*/
   /* request was not processed.  This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
int _FTPM_Get_Directory_Request_Response(unsigned int OTP_ID, OTP_DirectoryInfo_t *DirInfo, Byte_t ResponseCode)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (OTP_VALID_RESPONSE_CODE_VALUE(ResponseCode)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate information about the FTPM Mgr session being      */
            /* referenced.                                              */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  /* Make sure the ResponseCode does not include the    */
                  /* Final Bit.                                         */
                  ResponseCode &= ((Byte_t)~OBEX_FINAL_BIT);

                  /* If the ResponseCode indicates Success, then change */
                  /* this to Zero to indicate no error.                 */
                  if(ResponseCode == OBEX_OK_RESPONSE)
                     ResponseCode = 0;

                  /* If ResponseCode is non-Zero, then some problem has */
                  /* occurred while processing the request.  If an error*/
                  /* occurred, then we need to set the state back to    */
                  /* Idle.                                              */
                  if(ResponseCode)
                  {
                     OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                     OTPInfoPtr->ObjectInfo.FieldMask = 0;
                     OTPInfoPtr->ServerState          = ssIdle;

                     ret_val = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)(ResponseCode | OBEX_FINAL_BIT), NULL);
                  }
                  else
                  {
                     /* Before we store information about the Directory */
                     /* Information structure, we need to make sure that*/
                     /* the Directory Information structure that was    */
                     /* passed to us is valid.                          */
                     if((DirInfo) && ((!DirInfo->NumberEntries) || ((DirInfo->NumberEntries) && (DirInfo->ObjectInfo))))
                     {
                        /* Save the Directory Info information and      */
                        /* process the process the request.             */
                        OTPInfoPtr->DirInfo.DirectoryIndex      = 0;
                        OTPInfoPtr->DirInfo.SegmentedLineLength = -1;
                        OTPInfoPtr->DirInfo.NumberEntries       = DirInfo->NumberEntries;
                        OTPInfoPtr->DirInfo.ParentDirectory     = DirInfo->ParentDirectory;
                        OTPInfoPtr->DirInfo.ObjectInfoList      = DirInfo->ObjectInfo;
                        OTPInfoPtr->DirInfo.SegmentationState   = ssStart;

                        ProcessDirectoryRequest(OTPInfoPtr);

                        /* Return success to the caller.                */
                        ret_val = 0;
                     }
                     else
                        ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
                  }
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to respond to a request from a     */
   /* remote Client to Change to a new directory, Create, or Delete a   */
   /* directory.  The function takes as its first parameter the OTP_ID  */
   /* parameter which references the OBEX Connection on which the       */
   /* request is to be made.  The parameter ResponseCode is used to     */
   /* notify the remote client of its ability to satisfy the request.   */
   /* If the ResponseCode value is non-Zero, then the request could not */
   /* be satisfied and the ResponseCode parameter contains the OBEX     */
   /* result code that identifies the reason why the request was not    */
   /* processed.  This function returns zero if successful, or a        */
   /* negative return value if there was an error.                      */
int _FTPM_Set_Path_Response(unsigned int OTP_ID, Byte_t ResponseCode)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (OTP_VALID_RESPONSE_CODE_VALUE(ResponseCode)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information about the FTPM Mgr Session that is*/
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  /* Since there is not any continuation to this        */
                  /* command, Set the state back to Idle.               */
                  OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                  OTPInfoPtr->ObjectInfo.FieldMask = 0;
                  OTPInfoPtr->ServerState          = ssIdle;

                  ret_val                          = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)(ResponseCode | OBEX_FINAL_BIT), NULL);
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to respond to a request from a     */
   /* remote Client to Abort an operation.  The function takes as its   */
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  It is not possible*/
   /* to refuse an abort request, so no further parameters are required.*/
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
int _FTPM_Abort_Response(unsigned int OTP_ID)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if(OTP_ID)
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information about the FTPM Mgr Session that is*/
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  /* Since there is not any continuation to this        */
                  /* command, Set the state back to Idle.               */
                  OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                  OTPInfoPtr->ObjectInfo.FieldMask = 0;
                  OTPInfoPtr->ServerState          = ssIdle;

                  ret_val                          = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (OBEX_OK_RESPONSE | OBEX_FINAL_BIT), NULL);
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to respond to a request from a     */
   /* remote Client to Get an Object.  The function takes as its first  */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  When the Get      */
   /* Request was received from the Client, the Application was provided*/
   /* a pointer to a Buffer where the data is to be loaded.  The buffer */
   /* provided to the application is referenced in the OTP_Info_t       */
   /* structure.  The parameter BytesToSend indicates the number of     */
   /* Bytes of data that the application has loaded in this buffer.  The*/
   /* parameter ResponseCode is used to notify the remote client of its */
   /* ability to satisfy the request.  If the ResponseCode value is     */
   /* non-zero, then the request could not be satisfied and the         */
   /* ResponseCode parameter contains the OBEX result code that         */
   /* identifies the reason why the request was not processed.  The     */
   /* parameter UserInfo is a user defined parameter.  The value of this*/
   /* parameter will be passed back to the application on the next Get  */
   /* Request event.  This function returns zero if successful, or a    */
   /* negative return value if there was an error.                      */
int _FTPM_Get_Object_Response(unsigned int OTP_ID, unsigned int BytesToSend, unsigned int ResponseCode, unsigned long UserInfo)
{
   int                 ret_val;
   Byte_t              Index;
   OTPM_Info_t        *OTPInfoPtr;
   OBEX_Header_t       Header;
   OBEX_Header_List_t  HeaderList;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (OTP_VALID_RESPONSE_CODE_VALUE(ResponseCode)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information about the FTPM Mgr Session that is*/
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  Index                = 0;
                  HeaderList.Headers   = NULL;
                  OTPInfoPtr->UserInfo = UserInfo;
                  ResponseCode         = (ResponseCode & ~OBEX_FINAL_BIT);
                  if((ResponseCode == OBEX_OK_RESPONSE) || (ResponseCode == OBEX_CONTINUE_RESPONSE))
                  {
                     /* Build the Headers that are needed for the data. */
                     /* There are some conditional information added    */
                     /* depending on whether this is the last packet.   */
                     HeaderList.Headers                            = &Header;
                     Header.OBEX_Header_ID                         = (ResponseCode == OBEX_CONTINUE_RESPONSE)?hidBody:hidEndOfBody;
                     Header.OBEX_Header_Type                       = htByteSequence;
                     Header.Header_Value.ByteSequence.DataLength   = (Word_t)BytesToSend;
                     Header.Header_Value.ByteSequence.ValuePointer = OTPInfoPtr->ResponseBuffer;

                     Index++;
                  }

                  /* If the response is anything but Continue, then we  */
                  /* need to set the state back to Idle.                */
                  if(ResponseCode != OBEX_CONTINUE_RESPONSE)
                  {
                     ResponseCode                    |= OBEX_FINAL_BIT;

                     OTPInfoPtr->UserInfo             = 0;
                     OTPInfoPtr->ServerState          = ssIdle;
                     OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                     OTPInfoPtr->ObjectInfo.FieldMask = 0;
                  }

                  /* Load the number of Headers in the response and send*/
                  /* the response.                                      */
                  HeaderList.NumberOfHeaders = Index;

                  ret_val = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)(ResponseCode | OBEX_FINAL_BIT), &HeaderList);
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to respond to a request from a     */
   /* remote Client to Delete an Object.  The function takes as its     */
   /* first parameter the OTP_ID parameter which references the OBEX    */
   /* Connection on which the request is to be made.  The parameter     */
   /* ResponseCode is used to notify the remote client of its ability to*/
   /* satisfy the request.  If the ResponseCode value is non-zero, then */
   /* the request could not be satisfied and the ResponseCode parameter */
   /* contains the OBEX result code that identifies the reason why the  */
   /* request was not processed.  This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
int _FTPM_Delete_Object_Response(unsigned int OTP_ID, Byte_t ResponseCode)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (OTP_VALID_RESPONSE_CODE_VALUE(ResponseCode)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Locate the information about the FTPM Mgr Session that is*/
            /* being referenced.                                        */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  /* Since there is not any continuation to this        */
                  /* command, Set the state back to Idle.               */
                  OTPInfoPtr->CurrentOperation     = OTP_OPERATION_NONE;
                  OTPInfoPtr->ObjectInfo.FieldMask = 0;
                  OTPInfoPtr->ServerState          = ssIdle;

                  ret_val = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)(((ResponseCode)?ResponseCode:OBEX_OK_RESPONSE) | OBEX_FINAL_BIT), NULL);
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to respond to a request from a     */
   /* remote Client to Put an Object.  The function takes as its first  */
   /* parameter the OTP_ID parameter which references the OBEX          */
   /* Connection on which the request is to be made.  The parameter     */
   /* ResponseCode is used to notify the remote client of its ability to*/
   /* satisfy the request.  If the ResponseCode value is non-zero, then */
   /* the request could not be satisfied and the ResponseCode parameter */
   /* contains the OBEX result code that identifies the reason why the  */
   /* request was not processed.  This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
int _FTPM_Put_Object_Response(unsigned int OTP_ID, Byte_t ResponseCode)
{
   int          ret_val;
   OTPM_Info_t *OTPInfoPtr;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((OTP_ID) && (OTP_VALID_RESPONSE_CODE_VALUE(ResponseCode)))
   {
      /* Wait for access to the lock that guards access to this module. */
      if(DEVM_AcquireLock())
      {
         /* Make sure that this module was indeed initialized.          */
         if(Initialized)
         {
            /* Search for Information about this connection.            */
            OTPInfoPtr = SearchOTPInfoEntry(&OTPInfoList, OTP_ID);
            if(OTPInfoPtr)
            {
               /* FTPM Mgr Session Information found, now let's verify  */
               /* that we are Connected, and the specified Session      */
               /* specifies an FTPM Mgr Server.                         */
               if((OTPInfoPtr->ClientConnected) && (OTPInfoPtr->Server))
               {
                  /* Mask off the Final bit just in case the user added */
                  /* it to the response.                                */
                  ResponseCode &= (Byte_t)(~OBEX_FINAL_BIT);

                  /* If the response is not a Continue, then we are     */
                  /* finished or some error has occurred.  In either    */
                  /* case, we will need to move to the Idle State.      */
                  if(ResponseCode != OBEX_CONTINUE_RESPONSE)
                  {
                     ResponseCode                     |= OBEX_FINAL_BIT;

                     OTPInfoPtr->ServerState           = ssIdle;
                     OTPInfoPtr->CurrentOperation      = OTP_OPERATION_NONE;
                     OTPInfoPtr->ObjectInfo.FieldMask  = 0;
                  }

                  /* Send the reply to the Client.                      */
                  ret_val  = _GOEP_Command_Response(OTPInfoPtr->GOEP_ID, (Byte_t)(ResponseCode | OBEX_FINAL_BIT), NULL);
               }
               else
               {
                  /* Return the correct error result to the caller.     */
                  if(!OTPInfoPtr->Server)
                     ret_val = BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED;
                  else
                     ret_val = BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED;
               }
            }
            else
               ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;
         }
         else
            ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;

         /* Release the Lock because we are finished with it.           */
         DEVM_ReleaseLock();
      }
      else
         ret_val = BTPM_ERROR_CODE_FTP_NOT_INITIALIZED;
   }
   else
      ret_val = BTPM_ERROR_CODE_INVALID_PARAMETER;

   return(ret_val);
}


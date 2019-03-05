/*****< ftpmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FTPMAPI - Stonestreet One Bluetooth Stack Platform Manager File Transfer  */
/*            Profile API Type Definitions, Constants, and Prototypes.        */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/03/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FTPMAPIH__
#define __FTPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

   /* The following BIT definitions are used to denote the individual   */
   /* FTP Server Permissions that can will be applied to an FTP Client  */
   /* Connection.  These BIT definitions are specified in the           */
   /* FTPM_Open_Server() function.                                      */
#define FTPM_SERVER_PERMISSION_MASK_READ                        0x00000001
#define FTPM_SERVER_PERMISSION_MASK_WRITE                       0x00000002
#define FTPM_SERVER_PERMISSION_MASK_DELETE                      0x00000004

   /* OBEX File Transfer Server Event API Types.                        */
typedef enum
{
   /* Server Events.                                                    */
   etFTPMIncomingConnectionRequest,
   etFTPMConnectIndication,
   etFTPMDirectoryRequestIndication,
   etFTPMChangeDirectoryIndication,
   etFTPMDirectoryDeleteIndication,
   etFTPMDirectoryCreateIndication,
   etFTPMFileDeleteIndication,
   etFTPMFileCreateIndication,
   etFTPMFilePutIndication,
   etFTPMFileGetIndication,

   /* Client Events.                                                    */
   etFTPMConnectConfirmation,
   etFTPMAbortConfirmation,
   etFTPMDirectoryRequestConfirmation,
   etFTPMChangeDirectoryConfirmation,
   etFTPMDirectoryDeleteConfirmation,
   etFTPMDirectoryCreateConfirmation,
   etFTPMFileDeleteConfirmation,
   etFTPMFileCreateConfirmation,
   etFTPMFilePutConfirmation,
   etFTPMFileGetConfirmation,

   /* Common Connection Events.                                         */
   etFTPMDisconnectIndication
} FTPM_Event_Type_t;

   /* Server Event Structures.                                          */

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* requests to connects to a registered FTP Server.  The FTP ID      */
   /* member specifies the Local Server that has received the request to*/
   /* connect and the BD_ADDR member specifies the Client Bluetooth     */
   /* Device that is requesting to connect to the specified Server.     */
typedef struct _tagFTPM_Server_Incoming_Connection_Request_Data_t
{
   unsigned int FTPConnectionID;
   BD_ADDR_t    RemoteDeviceAddress;
} FTPM_Server_Incoming_Connection_Request_Data_t;

#define FTPM_SERVER_INCOMING_CONNECTION_REQUEST_DATA_SIZE      (sizeof(FTPM_Server_Incoming_Connection_Request_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Connects to a registered FTP Server.  The FTP ID member specifies */
   /* the Local Server that has been connected to and the BD_ADDR       */
   /* member specifies the Client Bluetooth Device that has connected   */
   /* to the specified Server.                                          */
typedef struct _tagFTPM_Server_Connect_Indication_Data_t
{
   unsigned int FTPConnectionID;
   BD_ADDR_t    RemoteDeviceAddress;
} FTPM_Server_Connect_Indication_Data_t;

#define FTPM_SERVER_CONNECT_INDICATION_DATA_SIZE               (sizeof(FTPM_Server_Connect_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Requests a Directory from the registered FTP Server.  The FTP ID  */
   /* member specifies the Local Server that the Remote Client has      */
   /* requested.  The Directory Name member specifies the Directory     */
   /* Name of the Directory Listing that was requested.                 */
   /* * NOTE * The DirectoryName member is formatted as a NULL          */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Server_Directory_Request_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *DirectoryName;
} FTPM_Server_Directory_Request_Indication_Data_t;

#define FTPM_SERVER_DIRECTORY_REQUEST_INDICATION_DATA_SIZE     (sizeof(FTPM_Server_Directory_Request_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Changes the Current Directory on the registered FTP Server.  The  */
   /* FTP ID member specifies the Local Server that the Remote Client   */
   /* has changed the directory of.  The Directory Name member specifies*/
   /* the directory path that the Remote Client has changed to.         */
   /* * NOTE * The DirectoryName member is formatted as a NULL          */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Server_Change_Directory_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *DirectoryName;
} FTPM_Server_Change_Directory_Indication_Data_t;

#define FTPM_SERVER_CHANGE_DIRECTORY_INDICATION_DATA_SIZE      (sizeof(FTPM_Server_Change_Directory_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Deletes the specified directory from the registered FTP Server.   */
   /* The FTP ID member specifies the Local Server that the Remote      */
   /* Client has deleted the directory from.  The Deleted Directory Name*/
   /* member specifies the name of the Directory that was deleted.  The */
   /* Directory Name member specifies the name of the Directory from    */
   /* which the specified Directory was deleted.                        */
   /* * NOTE * The Directory Name members are formatted as a NULL       */
   /*          terminated ASCII strings with UTF-8 encoding.            */
typedef struct _tagFTPM_Server_Directory_Delete_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *DeletedDirectoryName;
   char         *DirectoryName;
} FTPM_Server_Directory_Delete_Indication_Data_t;

#define FTPM_SERVER_DIRECTORY_DELETE_INDICATION_DATA_SIZE      (sizeof(FTPM_Server_Directory_Delete_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Creates the specified directory on the registered FTP Server.     */
   /* The FTP ID member specifies the Local Server that the Remote      */
   /* Client has created the directory on.  The Created Directory Name  */
   /* member specifies the name of the Directory that was created.  The */
   /* Directory Name member specifies the name of the Directory in      */
   /* which the specified Directory was created.                        */
   /* * NOTE * The Directory Name members are formatted as a NULL       */
   /*          terminated ASCII strings with UTF-8 encoding.            */
typedef struct _tagFTPM_Server_Directory_Create_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *CreatedDirectoryName;
   char         *DirectoryName;
} FTPM_Server_Directory_Create_Indication_Data_t;

#define FTPM_SERVER_DIRECTORY_CREATE_INDICATION_DATA_SIZE      (sizeof(FTPM_Server_Directory_Create_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Deletes the specified file from the registered FTP Server.  The   */
   /* FTP ID member specifies the Local Server that the Remote Client   */
   /* has deleted the file from.  The Deleted File Name member specifies*/
   /* the name of the File that was deleted.  The Directory Name        */
   /* member specifies the name of the Directory from which the         */
   /* specified file was deleted.                                       */
   /* * NOTE * The Name members are formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
typedef struct _tagFTPM_Server_File_Delete_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *DeletedFileName;
   char         *DirectoryName;
} FTPM_Server_File_Delete_Indication_Data_t;

#define FTPM_SERVER_FILE_DELETE_INDICATION_DATA_SIZE           (sizeof(FTPM_Server_File_Delete_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Creates a file on the registered FTP Server.  The FTP ID member   */
   /* specifies the Local Server that the Remote Client has created     */
   /* the file on.  he Created File Name member specifies the name of   */
   /* File that was created.  The Directory Name member specifies the   */
   /* name of the Directory from which the specified file was created.  */
   /* * NOTE * The Name members are formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
typedef struct _tagFTPM_Server_File_Create_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *CreatedFileName;
   char         *DirectoryName;
} FTPM_Server_File_Create_Indication_Data_t;

#define FTPM_SERVER_FILE_CREATE_INDICATION_DATA_SIZE           (sizeof(FTPM_Server_File_Create_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* puts a file on the registered FTP Server.  The FTP ID member      */
   /* specifies the Local Server that the Remote Client has put the     */
   /* specified file on.  The File Name member specifies the name of    */
   /* the file that was put on the FTP Server.  The Directory Name      */
   /* member specifies name of the Directory in which the specified     */
   /* file is being put.  The Transfer Complete flag specifies whether  */
   /* or not the file transfer has been completed (TRUE when completed, */
   /* FALSE while in progress).  The Transferred Length and Total Length*/
   /* members specify how many bytes have been transferred and how many */
   /* bytes are to be transferred in total.                             */
   /* * NOTE * The Name members are formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
typedef struct _tagFTPM_Server_File_Put_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *FileName;
   char         *DirectoryName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTPM_Server_File_Put_Indication_Data_t;

#define FTPM_SERVER_FILE_PUT_INDICATION_DATA_SIZE              (sizeof(FTPM_Server_File_Put_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* retrieves a file from the registered FTP Server.  The FTP ID      */
   /* member specifies the Local Server that the Remote Client is       */
   /* retrieving the specified file from.  The File Name member         */
   /* specifies the name of the file that is being retrieved from the   */
   /* FTP Server.  The Directory Name member specifies the name of the  */
   /* Directory in which the specified file is being retreived.  The    */
   /* Transfer Complete flag specifies whether or not the file transfer */
   /* has been completed (TRUE when completed, FALSE while in progress).*/
   /* The Transferred Length and Total Length members specify how many  */
   /* bytes have been transferred and how many bytes are to be          */
   /* transferred in total.                                             */
   /* * NOTE * The Name members are formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
typedef struct _tagFTPM_Server_File_Get_Indication_Data_t
{
   unsigned int  FTPConnectionID;
   char         *FileName;
   char         *DirectoryName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTPM_Server_File_Get_Indication_Data_t;

#define FTPM_SERVER_FILE_GET_INDICATION_DATA_SIZE              (sizeof(FTPM_Server_File_Get_Indication_Data_t))

   /* Client Event Structures.                                          */

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* receives the Connection Response from an FTP Server which was     */
   /* previously attempted to be connected to.  The FTP ID member       */
   /* specifies the Local Client that has requested the connection, the */
   /* FTP Open Status represents the Connection Status of the Request,  */
   /* and the BD_ADDR member specifies the Remote Bluetooth Device that */
   /* the Remote Bluetooth FTP Server resides on.                       */
typedef struct _tagFTPM_Client_Connect_Confirmation_Data_t
{
   unsigned int FTPConnectionID;
   unsigned int FTPConnectStatus;
   BD_ADDR_t    RemoteDeviceAddress;
} FTPM_Client_Connect_Confirmation_Data_t;

#define FTPM_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE             (sizeof(FTPM_Client_Connect_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Aborts Response is received from the Remote FTP Server.  The FTP  */
   /* ID member specifies the Local Client that the Remote Server has   */
   /* responded to the Abort Request on.                                */
typedef struct _tagFTPM_Client_Abort_Confirmation_Data_t
{
   unsigned int FTPConnectionID;
} FTPM_Client_Abort_Confirmation_Data_t;

#define FTPM_CLIENT_ABORT_CONFIRMATION_DATA_SIZE               (sizeof(FTPM_Client_Abort_Confirmation_Data_t))

   /* The following defines the bit assignments for the FieldMask       */
   /* parameter in the Directory Entry structure.  The appropriate bit  */
   /* is set in the FieldMask parameter to denote the presence of a     */
   /* parameter in the structure.                                       */
#define FTPM_OBJECT_INFO_MASK_CLEAR                            0x00000000
#define FTPM_OBJECT_INFO_MASK_NAME                             0x00000001
#define FTPM_OBJECT_INFO_MASK_SIZE                             0x00000002
#define FTPM_OBJECT_INFO_MASK_TYPE                             0x00000004
#define FTPM_OBJECT_INFO_MASK_MODIFIED                         0x00000008
#define FTPM_OBJECT_INFO_MASK_CREATED                          0x00000010
#define FTPM_OBJECT_INFO_MASK_ACCESSED                         0x00000020
#define FTPM_OBJECT_INFO_MASK_USER_PERMISSION                  0x00000040
#define FTPM_OBJECT_INFO_MASK_GROUP_PERMISSION                 0x00000080
#define FTPM_OBJECT_INFO_MASK_OTHER_PERMISSION                 0x00000100
#define FTPM_OBJECT_INFO_MASK_OWNER                            0x00000200
#define FTPM_OBJECT_INFO_MASK_GROUP                            0x00000400

   /* The following defines the basic bit assignments for the permission*/
   /* parameter used in the Directory Entry structure.                  */
#define FTPM_PERMISSION_READ_MASK                              0x0001
#define FTPM_PERMISSION_WRITE_MASK                             0x0002
#define FTPM_PERMISSION_DELETE_MASK                            0x0004

   /* The following defines the bit assignments for the permission      */
   /* parameters used in the Directory Entry structure.  Each specific  */
   /* permission is based on the basic bit assignment for the           */
   /* permission.                                                       */
#define FTPM_USER_PERMISSION_READ                              (FTPM_PERMISSION_READ_MASK   << 0)
#define FTPM_USER_PERMISSION_WRITE                             (FTPM_PERMISSION_WRITE_MASK  << 0)
#define FTPM_USER_PERMISSION_DELETE                            (FTPM_PERMISSION_DELETE_MASK << 0)
#define FTPM_GROUP_PERMISSION_READ                             (FTPM_PERMISSION_READ_MASK   << 4)
#define FTPM_GROUP_PERMISSION_WRITE                            (FTPM_PERMISSION_WRITE_MASK  << 4)
#define FTPM_GROUP_PERMISSION_DELETE                           (FTPM_PERMISSION_DELETE_MASK << 4)
#define FTPM_OTHER_PERMISSION_READ                             (FTPM_PERMISSION_READ_MASK   << 8)
#define FTPM_OTHER_PERMISSION_WRITE                            (FTPM_PERMISSION_WRITE_MASK  << 8)
#define FTPM_OTHER_PERMISSION_DELETE                           (FTPM_PERMISSION_DELETE_MASK << 8)

   /* The following structure is used to represent a Time/Data          */
   /* associated with a file when performing directory listing          */
   /* transfers.  Since no AM/PM field is provided, the Time is         */
   /* represented in 24 hour time.  The UTC_Time field is used to       */
   /* indicate whether the time is represented in UTC time ot Local     */
   /* Time.                                                             */
typedef struct _tagFTPM_TimeDate_t
{
   Word_t    Year;
   Word_t    Month;
   Word_t    Day;
   Word_t    Hour;
   Word_t    Minute;
   Word_t    Second;
   Boolean_t UTC_Time;
} FTPM_TimeDate_t;

#define FTPM_TIME_DATE_SIZE                                    (sizeof(FTPM_TimeDate_t))

   /* The following structure is used to hold descriptive information   */
   /* about a Directory Entry that is being processed.  The Field Mask  */
   /* defines the fields in the structure that are valid.  The Directory*/
   /* Entry member specifies whether or not the Entry specifies a       */
   /* Directory Entry (TRUE) or a File Entry (FALSE).                   */
   /* * NOTE * The Name, Type, Owner, and Group members are formatted   */
   /*          as a NULL terminated ASCII strings with UTF-8 encoding.  */
typedef struct _tagFTPM_Directory_Entry_t
{
   Boolean_t         DirectoryEntry;
   unsigned int      FieldMask;
   unsigned int      NameLength;
   char             *Name;
   unsigned int      Size;
   unsigned int      TypeLength;
   char             *Type;
   FTPM_TimeDate_t   Modified;
   FTPM_TimeDate_t   Created;
   FTPM_TimeDate_t   Accessed;
   unsigned long     PermissionMask;
   unsigned int      OwnerLength;
   char             *Owner;
   unsigned int      GroupLength;
   char             *Group;
} FTPM_Directory_Entry_t;

#define FTPM_DIRECTORY_ENTRY_SIZE                              (sizeof(FTPM_Directory_Entry_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Receives a response from the FTP Server regarding a previous      */
   /* FTP Directory Request.  The FTP ID member specifies the Local     */
   /* Client that has connected to the Remote FTP Server.  The Parent   */
   /* Directory member specifies whether or not a Parent Directory      */
   /* exists above the specified Current Directory.  The Request        */
   /* Completed member specifies whether or not the original Directory  */
   /* Request has been completed (i.e. if this member is TRUE then      */
   /* there will be NO further FTP Client Directory Request             */
   /* Confirmations to follow).  The Number Directory Entries member    */
   /* specifies how many FTP Directory Entries are pointed to by the    */
   /* FTP Directory Entries member.  The DirectoryName member specifies */
   /* the Directory that was Requested.  The Directory Name member will */
   /* be NULL if the current directory was requested, or it will be     */
   /* a pointer to a NULL terminated ASCII string that specifies the    */
   /* Directory that was Browsed.  If the Directory Name is NULL then   */
   /* the Current Directory was the Directory Listing that was          */
   /* requested.                                                        */
   /* * NOTE * The DirectoryName member is formatted as a NULL          */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Client_Directory_Request_Confirmation_Data_t
{
   unsigned int            FTPConnectionID;
   Boolean_t               RequestCompleted;
   Boolean_t               ParentDirectory;
   char                   *DirectoryName;
   unsigned int            NumberDirectoryEntries;
   FTPM_Directory_Entry_t *FTPDirectoryEntries;
} FTPM_Client_Directory_Request_Confirmation_Data_t;

#define FTPM_CLIENT_DIRECTORY_REQUEST_CONFIRMATION_DATA_SIZE   (sizeof(FTPM_Client_Directory_Request_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Receives a response from the FTP Server regarding a previous      */
   /* FTP Change Directory Request.  The FTP ID member specifies the    */
   /* Local Client that has connected to the Remote FTP Server.         */
   /* The Success member specifies whether or not the request to change */
   /* the directory was successful or not.                              */
   /* * NOTE * The ChangedDirectoryName member is formatted as a NULL   */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Client_Change_Directory_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   char         *ChangedDirectoryName;
} FTPM_Client_Change_Directory_Confirmation_Data_t;

#define FTPM_CLIENT_CHANGE_DIRECTORY_CONFIRMATION_DATA_SIZE    (sizeof(FTPM_Client_Change_Directory_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Deletes the specified directory from a remote FTP Server and a    */
   /* response from the Server is received.  The FTP ID member specifies*/
   /* the Local FTP Client that the Remote Server has deleted the       */
   /* directory from.  The Deleted Directory Name member specifies the  */
   /* name of the Directory that was deleted.  The Success member       */
   /* specifies whether or not the request to delete the specified      */
   /* Directory from the specified directory was successful or not.     */
   /* If the Success Member is FALSE, the client can look at the        */
   /* DirectoryNotEmpty Flag to determine if the error occured because  */
   /* the Client tried to delete a directory that wasn't empty.         */
   /* * NOTE * The DeletedDirectoryName member is formatted as a NULL   */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Client_Directory_Delete_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   Boolean_t     DirectoryNotEmpty;
   char         *DeletedDirectoryName;
} FTPM_Client_Directory_Delete_Confirmation_Data_t;

#define FTPM_CLIENT_DIRECTORY_DELETE_CONFIRMATION_DATA_SIZE    (sizeof(FTPM_Client_Directory_Delete_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Creates the specified directory on the remote FTP Server and a    */
   /* response from the Server is received.  The FTP ID member specifies*/
   /* the Local Client that the Remote Server has created the directory */
   /* on.  The Created Directory Name member specifies the name of the  */
   /* Directory that was created.                                       */
   /* * NOTE * The CreatedDirectoryName member is formatted as a NULL   */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Client_Directory_Create_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   char         *CreatedDirectoryName;
} FTPM_Client_Directory_Create_Confirmation_Data_t;

#define FTPM_CLIENT_DIRECTORY_CREATE_CONFIRMATION_DATA_SIZE    (sizeof(FTPM_Client_Directory_Create_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Deletes the specified file from a remote FTP Server and a response*/
   /* from the Server is received.  The FTP ID member specifies the     */
   /* Local FTP Client that the Remote Server has deleted the file from.*/
   /* The Deleted File Name member specifies the name of the File that  */
   /* was deleted.  The Success member specifies whether or not the     */
   /* request to delete the specified File from the specified           */
   /* directory was successful or not                                   */
   /* * NOTE * The DeletedFileName member is formatted as a NULL        */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Client_File_Delete_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   char         *DeletedFileName;
} FTPM_Client_File_Delete_Confirmation_Data_t;

#define FTPM_CLIENT_FILE_DELETE_CONFIRMATION_DATA_SIZE         (sizeof(FTPM_Client_File_Delete_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Creates the specified file on the remote FTP Server and a         */
   /* response from the Server is received.  The FTP ID member specifies*/
   /* the Local Client that the Remote Server has created the file on.  */
   /* The Created File Name member specifies the name of the File that  */
   /* was created.  The Success member specifies whether or not the     */
   /* request to create the specified File on the FTP Server was        */
   /* successful or not.                                                */
   /* * NOTE * The CreatedFileName member is formatted as a NULL        */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTPM_Client_File_Create_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   char         *CreatedFileName;
} FTPM_Client_File_Create_Confirmation_Data_t;

#define FTPM_CLIENT_FILE_CREATE_CONFIRMATION_DATA_SIZE         (sizeof(FTPM_Client_File_Create_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* puts a file on the remote FTP Server and the Remote Server has    */
   /* responded to the request.  The FTP ID member specifies the Local  */
   /* Client that has put the specified file on the FTP Server.  The    */
   /* File Name member specifies the name of the file that was put on   */
   /* the FTP Server.  The Transfer Complete flag specifies whether or  */
   /* not the file transfer has been completed (TRUE when completed,    */
   /* FALSE while in progress).  The Transferred Length and Total       */
   /* Length members specify how many bytes have been transferred and   */
   /* how many bytes are to be transferred in total.  The Success       */
   /* member specifies whether or not the request has been completed    */
   /* successfully.                                                     */
   /* * NOTE * The FileName member is formatted as a NULL terminated    */
   /*          ASCII string with UTF-8 encoding.                        */
typedef struct _tagFTPM_Client_File_Put_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   char         *FileName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTPM_Client_File_Put_Confirmation_Data_t;

#define FTPM_CLIENT_FILE_PUT_CONFIRMATION_DATA_SIZE            (sizeof(FTPM_Client_File_Put_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* retrieves a file from a FTP Server and a response is received from*/
   /* the FTP Server.  The FTP ID member specifies the Local Client that*/
   /* is retreiving the specified file.  The File Name member specifies */
   /* the name of the file that is being retrieved from the Remote FTP  */
   /* Server.  The Transfer Complete flag specifies whether or not the  */
   /* file transfer has been completed (TRUE when completed, FALSE      */
   /* while in progress).  The Transferred Length and Total Length      */
   /* members specify how many bytes have been transferred and how      */
   /* many bytes are to be transferred in total.  The Success member    */
   /* specifies whether or not the Request was able to be completed     */
   /* successfully.                                                     */
   /* * NOTE * The FileName member is formatted as a NULL terminated    */
   /*          ASCII string with UTF-8 encoding.                        */
typedef struct _tagFTPM_Client_File_Get_Confirmation_Data_t
{
   unsigned int  FTPConnectionID;
   Boolean_t     Success;
   char         *FileName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTPM_Client_File_Get_Confirmation_Data_t;

#define FTPM_CLIENT_FILE_GET_CONFIRMATION_DATA_SIZE            (sizeof(FTPM_Client_File_Get_Confirmation_Data_t))

   /* Common Connection Event Structures.                               */

   /* The following FTPM Event is dispatched when a FTPM connection is  */
   /* disconnected from the local FTPM device.  The FTP ID member       */
   /* specifies the Local Connection that the Remote Client has         */
   /* disconnected from.                                                */
typedef struct _tagFTPM_Disconnect_Indication_Data_t
{
   unsigned int FTPConnectionID;
} FTPM_Disconnect_Indication_Data_t;

#define FTPM_DISCONNECT_INDICATION_DATA_SIZE                   (sizeof(FTPM_Disconnect_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all FTPM Profile Event Data Structures.                   */
typedef struct _tagFTPM_Event_Data_t
{
   FTPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      /* Server Events.                                                 */
      FTPM_Server_Incoming_Connection_Request_Data_t    ServerIncomingConnectionRequestEventData;
      FTPM_Server_Connect_Indication_Data_t             ServerConnectIndicationEventData;
      FTPM_Server_Directory_Request_Indication_Data_t   ServerDirectoryRequestIndicationEventData;
      FTPM_Server_Change_Directory_Indication_Data_t    ServerChangeDirectoryIndicationEventData;
      FTPM_Server_Directory_Delete_Indication_Data_t    ServerDirectoryDeleteIndicationEventData;
      FTPM_Server_Directory_Create_Indication_Data_t    ServerDirectoryCreateIndicationEventData;
      FTPM_Server_File_Delete_Indication_Data_t         ServerFileDeleteIndicationEventData;
      FTPM_Server_File_Create_Indication_Data_t         ServerFileCreateIndicationEventData;
      FTPM_Server_File_Put_Indication_Data_t            ServerFilePutIndicationEventData;
      FTPM_Server_File_Get_Indication_Data_t            ServerFileGetIndicationEventData;

      /* Client Events.                                                 */
      FTPM_Client_Connect_Confirmation_Data_t           ClientConnectConfirmationEventData;
      FTPM_Client_Abort_Confirmation_Data_t             ClientAbortConfirmationEventData;
      FTPM_Client_Directory_Request_Confirmation_Data_t ClientDirectoryRequestConfirmationEventData;
      FTPM_Client_Change_Directory_Confirmation_Data_t  ClientChangeDirectoryConfirmationEventData;
      FTPM_Client_Directory_Delete_Confirmation_Data_t  ClientDirectoryDeleteConfirmationEventData;
      FTPM_Client_Directory_Create_Confirmation_Data_t  ClientDirectoryCreateConfirmationEventData;
      FTPM_Client_File_Delete_Confirmation_Data_t       ClientFileDeleteConfirmationEventData;
      FTPM_Client_File_Create_Confirmation_Data_t       ClientFileCreateConfirmationEventData;
      FTPM_Client_File_Put_Confirmation_Data_t          ClientFilePutConfirmationEventData;
      FTPM_Client_File_Get_Confirmation_Data_t          ClientFileGetConfirmationEventData;

      /* Common Connection Events.                                      */
      FTPM_Disconnect_Indication_Data_t                 DisconnectIndicationEventData;
   } EventData;
} FTPM_Event_Data_t;

#define FTPM_EVENT_DATA_SIZE                                   (sizeof(FTPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a FTPM Event Receive Data Callback.  This function will be called */
   /* whenever a FTPM Event occurs.  This function passes to the caller */
   /* the FTPM Event Data that occurred and the FTPM Event Callback     */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the FTPM Event Data ONLY*/
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another File Transfer Event will not be processed while this      */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving FTPM Event Packets. */
   /*            A Deadlock WILL occur because NO FTPM Event Callbacks  */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
typedef void (BTPSAPI *FTPM_Event_Callback_t)(FTPM_Event_Data_t *EventData, void *CallbackParameter);

   /* File Transfer Profile Module (FTPM) Installation/Support          */
   /* Functions.                                                        */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager FTP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI FTPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI FTPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* File Transfer Profile Manager (FTPM) Server Functions.            */

   /* The following function is responsible for Opening a FTP File      */
   /* Server on the specified Bluetooth SPP Serial Port.  This function */
   /* accepts as input the Local Serial Port Server Number to use,      */
   /* followed by the the Port Flags, a pointer to a NULL terminated    */
   /* ASCII string which specifies the Local Directory Path of the root */
   /* directory of the File Server, the File Server Permissions to use  */
   /* when the responding to client requests, and the FTP Server Event  */
   /* Callback function (and parameter) to associate with the specified */
   /* File Server.  This function returns a positive, non zero, value if*/
   /* successful or a negative return error code if an error occurs.  A */
   /* successful return code will be a FTP ID that can be used to       */
   /* reference the Opened FTP Server in ALL other FTP Server functions */
   /* in this module.  Once a FTP Server is opened, it can only be      */
   /* Un-Registered via a call to the FTPM_Close_Server() function      */
   /* (passing the return value from this function).                    */
   /* * NOTE * The RootDirectory parameter should be formatted as a NULL*/
   /*          terminated ASCII string with UTF-8 encoding.             */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Open_Server(unsigned int ServerPort, unsigned long PortFlags, char *RootDirectory, unsigned long PermissionMask, FTPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Open_Server_t)(unsigned int ServerPort, unsigned long PortFlags, char *RootDirectory, unsigned long PermissionMask, FTPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering a FTP    */
   /* Server (which was Registered by a successful call to the          */
   /* FTPM_Open_Server() function).  This function accepts as input the */
   /* FTP ID of the server that is to be closed.  This function returns */
   /* zero if successful, or a negative return error code if an error   */
   /* occurred.                                                         */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Close_Server(unsigned int FTPConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Close_Server_t)(unsigned int FTPConnectionID);
#endif

   /* The following function is used to terminate a possible connection */
   /* to the local server.  This function can only be called by the FTP */
   /* Server.  A successfully call to this function will terminate the  */
   /* remote FTP Client connection to the local FTP server.  This       */
   /* function accepts as input the FTP ID that was returned from the   */
   /* FTPM_Open_Server() function.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* This function does NOT Un-Register a FTP Server from the system,  */
   /* it ONLY disconnects any connection that is currently active.  The */
   /* FTPM_Close_Server() function can be used to Un-Register the FTP   */
   /* Server.                                                           */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Close_Server_Connection(unsigned int FTPConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Close_Server_Connection_t)(unsigned int FTPConnectionID);
#endif

   /* The following function is provided to allow a means to respond to */
   /* a request to connect to a FTP Server.  This function accepts as   */
   /* input the FTP ID that was returned from the FTPM_Open_Server()    */
   /* function.  The final parameter to this function is whether to     */
   /* accept the pending connection.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Server_Connect_Request_Response(unsigned int FTPConnectionID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Server_Connect_Request_Response_t)(unsigned int FTPConnectionID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Generic File Server Service Record to the SDP Database.  This     */
   /* function takes as input the FTP Server ID (which *MUST* have been */
   /* obtained by calling the FTPM_Open_Server() function.  The second  */
   /* parameter specifies the Service Name to associate with the SDP    */
   /* Record.  The final parameter is a pointer to a DWord_t which      */
   /* receives the SDP Service Record Handle if this function           */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* and the SDPServiceRecordHandle value will be undefined.           */
   /* * NOTE * This function should only be called with the FTP ID that */
   /*          was returned from the FTPM_Open_Server() function.  This */
   /*          function should NEVER be used with FTP ID returned from  */
   /*          the FTPM_Open_Remote_Server() function.                  */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the FTPM_Close_Server() function.  */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI FTPM_Register_Server_SDP_Record(unsigned int FTPConnectionID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Register_Server_SDP_Record_t)(unsigned int FTPConnectionID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* File Transfer Profile Manager (FTPM) Client Functions.            */

   /* The following function is responsible for opening a Remote FTP    */
   /* File Server.  This function accepts as input the the Remote       */
   /* Bluetooth Device Address of the Bluetooth FTP Server to connect   */
   /* with.  The second parameter specifies the Remote Server Port to   */
   /* connect with.  The third parameter specifies the optional flags to*/
   /* use when opening the port.  The final two parameters specify the  */
   /* FTP Client Event Callback Function and the Callback Parameter to  */
   /* associate with this FTP Client.  This function returns a positive,*/
   /* non zero, value if successful or a negative return error code if  */
   /* an error occurs.  A successful return code will be a FTP ID that  */
   /* can be used to reference the Opened FTP Server in ALL other FTP   */
   /* Client functions in this module.  Once a remote server is opened, */
   /* it can only be closed via a call to the FTPM_Close_Client()       */
   /* function (passing the return value from this function).           */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Open_Remote_File_Server(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long OpenFlags, FTPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Open_Remote_File_Server_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long OpenFlags, FTPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is used to terminate a possible connection */
   /* to a remote server.  This function can only be called by the FTP  */
   /* Client.  A successful call to this function will terminate the    */
   /* remote FTP Connection.  This function accepts as input the FTP ID */
   /* that was returned from the the FTPM_Open_Remote_File_Server()     */
   /* function.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.                               */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Close_Client(unsigned int FTPConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Close_Client_t)(unsigned int FTPConnectionID);
#endif

   /* The following function is provided to allow a FTP Client to       */
   /* request either the current directory (or a sub-directory off of   */
   /* the current directory) listing.  This function accepts as its     */
   /* first parameter the FTP Client ID (returned from a successful call*/
   /* to the FTPM_Open_Remote_File_Server() function).  The second      */
   /* parameter specifies whether or not the current directory listing  */
   /* is to retrieved (NULL) or a sub-directory listing is to be        */
   /* retrieved (NON-NULL).  If a sub-directory is to be retrieved then */
   /* the final parameter points to a NULL terminated ASCII string that */
   /* represents the Remote sub-directory name to browse.  This function*/
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Get_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Get_Directory_t)(unsigned int FTPConnectionID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a FTP Client to change*/
   /* the current working directory on the remote FTP Server.  This     */
   /* function accepts as its first parameter the FTP Client ID         */
   /* (returned from a successful call to the                           */
   /* FTPM_Open_Remote_File_Server() function).  The second parameter   */
   /* specifies the sub-directory to change directories into (NON-NULL),*/
   /* or, instead, whether to back up the parent directory (NULL).  If  */
   /* the RemoteDirectoryName parameter is NULL, then the FTP Client    */
   /* Requests that the FTP Server change working directories to the    */
   /* parent directory of the current directory.  If this parameter is  */
   /* NON-NULL, then it needs to point to a NULL terminated ASCII string*/
   /* that represents the remote sub-directory name to change to.  The  */
   /* RemoteDirectoryName parameter cannot specify path information,    */
   /* only the name of a Remote Directory that exists in the current    */
   /* working directory (if NON-NULL).  This function returns zero if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Set_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Set_Directory_t)(unsigned int FTPConnectionID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a FTP Client to change*/
   /* the current working directory on the remote FTP Server to the Root*/
   /* Directory.  This function accepts as its first parameter the FTP  */
   /* Client ID (returned from a successful call to the                 */
   /* FTPM_Open_Remote_File_Server() function).  This function returns  */
   /* zero successful, or a negative return error code if there was an  */
   /* error.                                                            */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Set_Root_Directory(unsigned int FTPConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Set_Root_Directory_t)(unsigned int FTPConnectionID);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Create a Directory on the Remote FTP Server.  This      */
   /* function accepts as its first parameter the FTP Client ID         */
   /* (returned from a successful call to the                           */
   /* FTPM_Open_Remote_File_Server() function).  The second parameter is*/
   /* a pointer to a NULL terminated ASCII string that specifies the    */
   /* Remote Directory Name of the Remote Directory to Create.  This    */
   /* directory name must be specified and cannot contain any path      */
   /* information.  This directory is created in the current working    */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * The Bluetooth File Transfer Profile (using OBEX)         */
   /*          specifies that when a Remote Directory is created (and   */
   /*          created successfully) that the new working directory is  */
   /*          this newly created directory.                            */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Create_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Create_Directory_t)(unsigned int FTPConnectionID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Delete a Directory on the Remote FTP Server.  This      */
   /* function accepts as its first parameter the second parameter      */
   /* specifies the FTP Client ID (returned from a successful call to   */
   /* the FTPM_Open_Remote_File_Server() function).  The second         */
   /* parameter is a pointer to a NULL terminated ASCII string that     */
   /* specifies the Remote Directory Name of the Remote Directory to    */
   /* Delete.  This directory name must be specified and cannot contain */
   /* any path information.  This directory is deleted in the current   */
   /* working directory on the remote file server.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Due to an OBEX FTP limitation, when a delete operation is*/
   /*          specified, the client cannot be guaranteed whether a file*/
   /*          or directory of that name will be deleted.  In other     */
   /*          words, if the caller specifies this function and a remote*/
   /*          file exists (in the current remote working directory),   */
   /*          the file will be deleted (even thougth it is not a       */
   /*          directory).  This is due to OBEX FTP which does not allow*/
   /*          the specification of what type of Object to delete (only */
   /*          the name of the Object, which could be a file or         */
   /*          directory in this case).                                 */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Delete_Directory(unsigned int FTPConnectionID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Delete_Directory_t)(unsigned int FTPConnectionID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Create a File on the Remote FTP Server.  This function  */
   /* accepts as its first parameter the FTP Client ID (returned from a */
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The second parameter is a pointer to a NULL terminated ASCII      */
   /* string that specifies the Remote File Name of the Remote File to  */
   /* Create.  This File Name must be specified and cannot contain any  */
   /* path information.  This file is created in the current working    */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteFileName parameter should be formatted as a    */
   /*          NULL terminated ASCII string with UTF-8 encoding.        */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Create_File(unsigned int FTPConnectionID, char *RemoteFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Create_File_t)(unsigned int FTPConnectionID, char *RemoteFileName);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Delete a File on the Remote FTP Server.  This function  */
   /* accepts as its first parameter the FTP Client ID (returned from a */
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The second parameter is a pointer to a NULL terminated ASCII      */
   /* string that specifies the Remote File Name of the Remote File to  */
   /* Delete.  This File Name must be specified and cannot contain any  */
   /* path information.  This file is deleted in the current working    */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteFileName parameter should be formatted as a    */
   /*          NULL terminated ASCII string with UTF-8 encoding.        */
   /* * NOTE * Due to an OBEX FTP limitation, when a delete operation is*/
   /*          specified, the client cannot be guaranteed whether a file*/
   /*          or directory of that name will be deleted.  In other     */
   /*          words, if the caller specifies this function and a remote*/
   /*          directory exists (in the current remote working          */
   /*          directory), the directory will be deleted (even thougth  */
   /*          it is not a file).  This is due to OBEX FTP which does   */
   /*          not allow the specification of what type of Object to    */
   /*          delete (only the name of the Object, which could be a    */
   /*          file or directory in this case).                         */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Delete_File(unsigned int FTPConnectionID, char *RemoteFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Delete_File_t)(unsigned int FTPConnectionID, char *RemoteFileName);
#endif

   /* The following function is responsible for retrieving the specified*/
   /* Remote File from the specified Remote FTP Server.  This function  */
   /* accepts as its input parameters the FTP Client ID (returned from a*/
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The third parameter specifies the Remote File Name of the Remote  */
   /* File to retrieve.  This File Name is a pointer to a NULL          */
   /* terminated ASCII string and cannot contain any path information   */
   /* (or be empty or NULL).  The next parameter, RemoteFileLength, is a*/
   /* parameter that specifies the size (in Bytes) of the file that is  */
   /* being retrieved.  The last two parameters specify the local path  */
   /* (and filename) to store the retrieved file.  The LocalPath member */
   /* (if specified) contains the Local Path to store the file name, and*/
   /* the final parameter specifies the Filename to store the file into.*/
   /* The last parameter is not optional and *MUST* be specified.  If   */
   /* the LocalPath parameter is NULL, then the file is written to the  */
   /* current directory (on the local machine).  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The RemoteFileName, LocalPath, and LocalFileName         */
   /*          parameters should be formatted as NULL terminated ASCII  */
   /*          strings with UTF-8 encoding.                             */
   /* * NOTE * The Remote File Length parameter exists simply to allow a*/
   /*          Total Length value to be passed to the caller on File Get*/
   /*          Confirmations.  This value can be any value the caller   */
   /*          would like it to be because the FTP Profile ignores this */
   /*          value and simply passes this value back to the caller in */
   /*          the File Get Confirmation Event.                         */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Get_File(unsigned int FTPConnectionID, char *RemoteFileName, unsigned int RemoteFileLength, char *LocalPath, char *LocalFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Get_File_t)(unsigned int FTPConnectionID, char *RemoteFileName, unsigned int RemoteFileLength, char *LocalPath, char *LocalFileName);
#endif

   /* The following function is responsible for sending the specified   */
   /* Local File to the specified Remote FTP Server.  This function     */
   /* accepts as its input parameters the FTP Client ID (returned from a*/
   /* successful call to the FTPM_Open_Remote_File_Server() function).  */
   /* The second and third parameters specify the Local Path and Local  */
   /* Filename of the source file.  These two parameters are pointers to*/
   /* NULL terminated ASCII strings which specify the Path and File Name*/
   /* (respectively) of the Local File.  The Local Path parameter is    */
   /* optional, and if NON-NULL specifies the Local Path of the Local   */
   /* File.  The Local File Name parameter is NOT optional and specifies*/
   /* the File Name of the Local File.  The RemoteFileName parameter    */
   /* specifies the name of the file that is to be stored on the Remote */
   /* FTP Server.  This file name cannot specify any path information.  */
   /* The file that is sent to the Remote FTP Server is stored in the   */
   /* current working directory on the Remote FTP Server.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The LocalPath, LocalFileName, and RemoteFileName         */
   /*          parameters should be formatted as NULL terminated ASCII  */
   /*          strings with UTF-8 encoding.                             */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTPM_Abort() function) or the current Request*/
   /*          is complete (this is signified by receiving a            */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Put_File(unsigned int FTPConnectionID, char *LocalPath, char *LocalFileName, char *RemoteFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Put_File_t)(unsigned int FTPConnectionID, char *LocalPath, char *LocalFileName, char *RemoteFileName);
#endif

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding FTP Client Request.  This function accepts as its     */
   /* input parameters the FTP Client ID (returned from a successful    */
   /* call to the FTPM_Open_Remote_File_Server() function).  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This FTP Client Request is no different than the rest of */
   /*          the FTP Client Request functions in that a response to   */
   /*          this Request must be received before any other FTP Client*/
   /*          Request can be issued.                                   */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a FTP Client Request that is to be aborted may have */
   /*          completed before the server was able to Abort the        */
   /*          request.  In either case, the caller will be notified via*/
   /*          FTP Client Callback of the status of the previous        */
   /*          Request.                                                 */
   /* * NOTE * Due to the nature of only one outstanding OBEX Command   */
   /*          when an Abort is issued, it may be queued (for           */
   /*          transmission when the response to the currently          */
   /*          outstanding OBEX Command is received.  A problem can     */
   /*          occur if the Remote OBEX Server does not respond to the  */
   /*          original request because the queued Abort Packet will    */
   /*          never be sent.  This is a problem because no new OBEX    */
   /*          commands can be issued because the OBEX layer on the     */
   /*          local machine thinks a Request is outstanding and will   */
   /*          not issue another request.  To aid in error recovery,    */
   /*          this function forces an Abort Request out (and the       */
   /*          clearing of the current OBEX Command Request) if this    */
   /*          function is called twice.  An application can call this  */
   /*          function a second time to force a local cleanup if a     */
   /*          response on the first Abort Packet is never received (via*/
   /*          the FTP Client Callback).  It should be noted that under */
   /*          normal circumstances (i.e.  the Remote Server is         */
   /*          functioning properly) this function will NEVER have to be*/
   /*          called twice.                                            */
BTPSAPI_DECLARATION int BTPSAPI FTPM_Abort(unsigned int FTPConnectionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTPM_Abort_t)(unsigned int FTPConnectionID);
#endif

#endif

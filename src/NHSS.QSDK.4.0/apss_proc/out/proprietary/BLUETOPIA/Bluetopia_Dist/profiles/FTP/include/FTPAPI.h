/*****< ftpapi.h >*************************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FTPAPI - Stonestreet One Bluetooth Stack File Transfer Profile API Type   */
/*           Definitions, Constants, and Prototypes.                          */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/15/01  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __FTPAPIH__
#define __FTPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTFTP_ERROR_INVALID_PARAMETER                             (-1000)
#define BTFTP_ERROR_NOT_INITIALIZED                               (-1001)
#define BTFTP_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTFTP_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTFTP_ERROR_INVALID_ROOT_DIRECTORY                        (-1005)
#define BTFTP_ERROR_REQUEST_ALREADY_OUTSTANDING                   (-1006)
#define BTFTP_ERROR_NOT_ALLOWED_WHILE_NOT_CONNECTED               (-1007)
#define BTFTP_ERROR_UNABLE_TO_CREATE_LOCAL_FILE                   (-1008)
#define BTFTP_ERROR_UNABLE_TO_READ_LOCAL_FILE                     (-1009)
#define BTFTP_ERROR_UNABLE_TO_WRITE_LOCAL_FILE                    (-1010)

   /* SDP Profile UUID's for the OBEX File Transfer Profile.            */

   /* The following MACRO is a utility MACRO that assigns the File      */
   /* Transfer Profile Bluetooth Universally Unique Identifier          */
   /* (FILE_TRANSFER_PROFILE_UUID_16) to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the FILE_TRANSFER_PROFILE_UUID_16     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_16(_x)            ASSIGN_SDP_UUID_16((_x), 0x11, 0x06)

   /* The following MACRO is a utility MACRO that assigns the File      */
   /* Transfer Profile Bluetooth Universally Unique Identifier          */
   /* (FILE_TRANSFER_PROFILE_UUID_32) to the specified UUID_32_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_32_t*/
   /* variable that is to receive the FILE_TRANSFER_PROFILE_UUID_32     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_32(_x)            ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x06)

   /* The following MACRO is a utility MACRO that assigns the File      */
   /* Transfer Profile Bluetooth Universally Unique Identifier          */
   /* (FILE_TRANSFER_PROFILE_UUID_128) to the specified UUID_128_t      */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* FILE_TRANSFER_PROFILE_UUID_128 Constant value.                    */
#define SDP_ASSIGN_FILE_TRANSFER_PROFILE_UUID_128(_x)           ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x06, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* File Transfer Profile Servers.                                    */
#define FTP_PROFILE_VERSION                                     (0x0101)

   /* The following BIT definitions are used to denote the individual   */
   /* FTP Server Permissions that can will be applied to an FTP Client  */
   /* Connection.  These BIT definitions are specified in the           */
   /* FTP_Open_Server() function.                                       */
#define FTP_SERVER_PERMISSION_MASK_READ                         0x00000001
#define FTP_SERVER_PERMISSION_MASK_WRITE                        0x00000002
#define FTP_SERVER_PERMISSION_MASK_DELETE                       0x00000004

   /* The following BIT definitions are used to denote the possible FTP */
   /* Server Modes that can be applied to a FTP Client Connection.      */
   /* These BIT definitions are used with the FTP_Set_Server_Mode() and */
   /* FTP_Get_Server_Mode() mode functions.                             */
#define FTP_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION             (0x00000000)
#define FTP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION                (0x00000001)
#define FTP_SERVER_MODE_CONNECTION_MASK                         (0x00000001)

   /* OBEX File Transfer Server Event API Types.                        */
typedef enum
{
   etFTP_Server_Connect_Indication,
   etFTP_Server_Disconnect_Indication,
   etFTP_Server_Directory_Request_Indication,
   etFTP_Server_Change_Directory_Indication,
   etFTP_Server_Directory_Delete_Indication,
   etFTP_Server_Directory_Create_Indication,
   etFTP_Server_File_Delete_Indication,
   etFTP_Server_File_Create_Indication,
   etFTP_Server_File_Put_Indication,
   etFTP_Server_File_Get_Indication,
   etFTP_Server_Connect_Request_Indication
} FTP_Server_Event_Type_t;

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Connects to a registered FTP Server.  The FTP ID member specifies */
   /* the Local Server that has been connected to and the BD_ADDR       */
   /* member specifies the Client Bluetooth Device that has connected   */
   /* to the specified Server.                                          */
typedef struct _tagFTP_Server_Connect_Indication_Data_t
{
   unsigned int FTPID;
   BD_ADDR_t    BD_ADDR;
} FTP_Server_Connect_Indication_Data_t;

#define FTP_SERVER_CONNECT_INDICATION_DATA_SIZE         (sizeof(FTP_Server_Connect_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Disconnects from a registered FTP Server.  The FTP ID member      */
   /* specifies the Local Server that the Remote Client has disconnected*/
   /* from.                                                             */
typedef struct _tagFTP_Server_Disconnect_Indication_Data_t
{
   unsigned int FTPID;
} FTP_Server_Disconnect_Indication_Data_t;

#define FTP_SERVER_DISCONNECT_INDICATION_DATA_SIZE      (sizeof(FTP_Server_Disconnect_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Requests a Directory from the registered FTP Server.  The FTP ID  */
   /* member specifies the Local Server that the Remote Client has      */
   /* requested.  The Directory Name member specifies the Directory     */
   /* Name of the Directory Listing that was requested.                 */
   /* * NOTE * The DirectoryName member is formatted as a NULL          */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTP_Server_Directory_Request_Indication_Data_t
{
   unsigned int  FTPID;
   char         *DirectoryName;
} FTP_Server_Directory_Request_Indication_Data_t;

#define FTP_SERVER_DIRECTORY_REQUEST_INDICATION_DATA_SIZE (sizeof(FTP_Server_Directory_Request_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Changes the Current Directory on the registered FTP Server.  The  */
   /* FTP ID member specifies the Local Server that the Remote Client   */
   /* has changed the directory of.  The Directory Name member specifies*/
   /* the directory path that the Remote Client has changed to.         */
   /* * NOTE * The DirectoryName member is formatted as a NULL          */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTP_Server_Change_Directory_Indication_Data_t
{
   unsigned int  FTPID;
   char         *DirectoryName;
} FTP_Server_Change_Directory_Indication_Data_t;

#define FTP_SERVER_CHANGE_DIRECTORY_INDICATION_DATA_SIZE (sizeof(FTP_Server_Change_Directory_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Deletes the specified directory from the registered FTP Server.   */
   /* The FTP ID member specifies the Local Server that the Remote      */
   /* Client has deleted the directory from.  The Deleted Directory Name*/
   /* member specifies the name of the Directory that was deleted.  The */
   /* Directory Name member specifies the name of the Directory from    */
   /* which the specified Directory was deleted.                        */
   /* * NOTE * The Directory Name members are formatted as a NULL       */
   /*          terminated ASCII strings with UTF-8 encoding.            */
typedef struct _tagFTP_Server_Directory_Delete_Indication_Data_t
{
   unsigned int  FTPID;
   char         *DeletedDirectoryName;
   char         *DirectoryName;
} FTP_Server_Directory_Delete_Indication_Data_t;

#define FTP_SERVER_DIRECTORY_DELETE_INDICATION_DATA_SIZE (sizeof(FTP_Server_Directory_Delete_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Creates the specified directory on the registered FTP Server.     */
   /* The FTP ID member specifies the Local Server that the Remote      */
   /* Client has created the directory on.  The Created Directory Name  */
   /* member specifies the name of the Directory that was created.  The */
   /* Directory Name member specifies the name of the Directory in      */
   /* which the specified Directory was created.                        */
   /* * NOTE * The Directory Name members are formatted as a NULL       */
   /*          terminated ASCII strings with UTF-8 encoding.            */
typedef struct _tagFTP_Server_Directory_Create_Indication_Data_t
{
   unsigned int  FTPID;
   char         *CreatedDirectoryName;
   char         *DirectoryName;
} FTP_Server_Directory_Create_Indication_Data_t;

#define FTP_SERVER_DIRECTORY_CREATE_INDICATION_DATA_SIZE (sizeof(FTP_Server_Directory_Create_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Deletes the specified file from the registered FTP Server.  The   */
   /* FTP ID member specifies the Local Server that the Remote Client   */
   /* has deleted the file from.  The Deleted File Name member specifies*/
   /* the name of the File that was deleted.  The Directory Name        */
   /* member specifies the name of the Directory from which the         */
   /* specified file was deleted.                                       */
   /* * NOTE * The Name members are formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
typedef struct _tagFTP_Server_File_Delete_Indication_Data_t
{
   unsigned int  FTPID;
   char         *DeletedFileName;
   char         *DirectoryName;
} FTP_Server_File_Delete_Indication_Data_t;

#define FTP_SERVER_FILE_DELETE_INDICATION_DATA_SIZE     (sizeof(FTP_Server_File_Delete_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* Creates a file on the registered FTP Server.  The FTP ID member   */
   /* specifies the Local Server that the Remote Client has created     */
   /* the file on.  he Created File Name member specifies the name of   */
   /* File that was created.  The Directory Name member specifies the   */
   /* name of the Directory from which the specified file was created.  */
   /* * NOTE * The Name members are formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
typedef struct _tagFTP_Server_File_Create_Indication_Data_t
{
   unsigned int  FTPID;
   char         *CreatedFileName;
   char         *DirectoryName;
} FTP_Server_File_Create_Indication_Data_t;

#define FTP_SERVER_FILE_CREATE_INDICATION_DATA_SIZE     (sizeof(FTP_Server_File_Create_Indication_Data_t))

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
typedef struct _tagFTP_Server_File_Put_Indication_Data_t
{
   unsigned int  FTPID;
   char         *FileName;
   char         *DirectoryName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTP_Server_File_Put_Indication_Data_t;

#define FTP_SERVER_FILE_PUT_INDICATION_DATA_SIZE        (sizeof(FTP_Server_File_Put_Indication_Data_t))

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
typedef struct _tagFTP_Server_File_Get_Indication_Data_t
{
   unsigned int  FTPID;
   char         *FileName;
   char         *DirectoryName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTP_Server_File_Get_Indication_Data_t;

#define FTP_SERVER_FILE_GET_INDICATION_DATA_SIZE        (sizeof(FTP_Server_File_Get_Indication_Data_t))

   /* The following FTP Server Event is dispatched when a FTP Client    */
   /* requests to connects to a registered FTP Server.  The FTP ID      */
   /* member specifies the Local Server that has received the request to*/
   /* connect and the BD_ADDR member specifies the Client Bluetooth     */
   /* Device that is requesting to connect to the specified Server.     */
typedef struct _tagFTP_Server_Connect_Request_Indication_Data_t
{
   unsigned int FTPID;
   BD_ADDR_t    BD_ADDR;
} FTP_Server_Connect_Request_Indication_Data_t;

#define FTP_SERVER_CONNECT_REQUEST_INDICATION_DATA_SIZE (sizeof(FTP_Server_Connect_Request_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all FTP Profile Server Event Data Data.                   */
typedef struct _tagFTP_Server_Event_Data_t
{
   FTP_Server_Event_Type_t Event_Data_Type;
   Word_t                  Event_Data_Size;
   union
   {
      FTP_Server_Connect_Indication_Data_t           *FTP_Server_Connect_Indication_Data;
      FTP_Server_Disconnect_Indication_Data_t        *FTP_Server_Disconnect_Indication_Data;
      FTP_Server_Directory_Request_Indication_Data_t *FTP_Server_Directory_Request_Indication_Data;
      FTP_Server_Change_Directory_Indication_Data_t  *FTP_Server_Change_Directory_Indication_Data;
      FTP_Server_Directory_Delete_Indication_Data_t  *FTP_Server_Directory_Delete_Indication_Data;
      FTP_Server_Directory_Create_Indication_Data_t  *FTP_Server_Directory_Create_Indication_Data;
      FTP_Server_File_Delete_Indication_Data_t       *FTP_Server_File_Delete_Indication_Data;
      FTP_Server_File_Create_Indication_Data_t       *FTP_Server_File_Create_Indication_Data;
      FTP_Server_File_Put_Indication_Data_t          *FTP_Server_File_Put_Indication_Data;
      FTP_Server_File_Get_Indication_Data_t          *FTP_Server_File_Get_Indication_Data;
      FTP_Server_Connect_Request_Indication_Data_t   *FTP_Server_Connect_Request_Indication_Data;
   } Event_Data;
} FTP_Server_Event_Data_t;

#define FTP_SERVER_EVENT_DATA_SIZE                      (sizeof(FTP_Server_Event_Data_t))

   /* The following constants represent the FTP Client Open Status      */
   /* Values that are possible in the FTP Client Open Confirmation      */
   /* Event.                                                            */
#define FTP_CLIENT_OPEN_STATUS_SUCCESS                                  0x00
#define FTP_CLIENT_OPEN_STATUS_CONNECTION_TIMEOUT                       0x01
#define FTP_CLIENT_OPEN_STATUS_CONNECTION_REFUSED                       0x02
#define FTP_CLIENT_OPEN_STATUS_UNKNOWN_ERROR                            0x04

   /* OBEX File Transfer Client Event API Types.                        */
typedef enum
{
   etFTP_Client_Connect_Confirmation,
   etFTP_Client_Disconnect_Indication,
   etFTP_Client_Abort_Confirmation,
   etFTP_Client_Directory_Request_Confirmation,
   etFTP_Client_Change_Directory_Confirmation,
   etFTP_Client_Directory_Delete_Confirmation,
   etFTP_Client_Directory_Create_Confirmation,
   etFTP_Client_File_Delete_Confirmation,
   etFTP_Client_File_Create_Confirmation,
   etFTP_Client_File_Put_Confirmation,
   etFTP_Client_File_Get_Confirmation
} FTP_Client_Event_Type_t;

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* receives the Connection Response from an FTP Server which was     */
   /* previously attempted to be connected to.  The FTP ID member       */
   /* specifies the Local Client that has requested the connection, the */
   /* FTP Open Status represents the Connection Status of the Request,  */
   /* and the BD_ADDR member specifies the Remote Bluetooth Device that */
   /* the Remote Bluetooth FTP Server resides on.                       */
typedef struct _tagFTP_Client_Connect_Confirmation_Data_t
{
   unsigned int FTPID;
   unsigned int FTPConnectStatus;
   BD_ADDR_t    BD_ADDR;
} FTP_Client_Connect_Confirmation_Data_t;

#define FTP_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE       (sizeof(FTP_Client_Connect_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Disconnects from a registered FTP Server.  The FTP ID member      */
   /* specifies the Local Client that the Remote Server has disconnected*/
   /* from.  This Event is NOT Dispatched in response to an Client      */
   /* Requesting a Disconnection.  This Event is dispatched when the    */
   /* Remote Server terminates the Connection (and/or Bluetooth Link).  */
typedef struct _tagFTP_Client_Disconnect_Indication_Data_t
{
   unsigned int FTPID;
} FTP_Client_Disconnect_Indication_Data_t;

#define FTP_CLIENT_DISCONNECT_INDICATION_DATA_SIZE      (sizeof(FTP_Client_Disconnect_Indication_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Aborts Response is received from the Remote FTP Server.  The FTP  */
   /* ID member specifies the Local Client that the Remote Server has   */
   /* responded to the Abort Request on.                                */
typedef struct _tagFTP_Client_Abort_Confirmation_Data_t
{
   unsigned int FTPID;
} FTP_Client_Abort_Confirmation_Data_t;

#define FTP_CLIENT_ABORT_CONFIRMATION_DATA_SIZE         (sizeof(FTP_Client_Abort_Confirmation_Data_t))

   /* The following defines the bit assignments for the FieldMask       */
   /* parameter in the Directory Entry structure.  The appropriate bit  */
   /* is set in the FieldMask parameter to denote the presence of a     */
   /* parameter in the structure.                                       */
#define FTP_OBJECT_INFO_MASK_CLEAR                              0x0000
#define FTP_OBJECT_INFO_MASK_NAME                               0x0001
#define FTP_OBJECT_INFO_MASK_SIZE                               0x0002
#define FTP_OBJECT_INFO_MASK_TYPE                               0x0004
#define FTP_OBJECT_INFO_MASK_MODIFIED                           0x0008
#define FTP_OBJECT_INFO_MASK_CREATED                            0x0010
#define FTP_OBJECT_INFO_MASK_ACCESSED                           0x0020
#define FTP_OBJECT_INFO_MASK_USER_PERMISSION                    0x0040
#define FTP_OBJECT_INFO_MASK_GROUP_PERMISSION                   0x0080
#define FTP_OBJECT_INFO_MASK_OTHER_PERMISSION                   0x0100
#define FTP_OBJECT_INFO_MASK_OWNER                              0x0200
#define FTP_OBJECT_INFO_MASK_GROUP                              0x0400

   /* The following defines the bit assignments for the permission      */
   /* parameters used in the Directory Entry structure.  Each specific  */
   /* permission is based on the basic bit assignment for the           */
   /* permission.                                                       */
#define FTP_USER_PERMISSION_READ                                OTP_USER_PERMISSION_READ
#define FTP_USER_PERMISSION_WRITE                               OTP_USER_PERMISSION_WRITE
#define FTP_USER_PERMISSION_DELETE                              OTP_USER_PERMISSION_DELETE
#define FTP_GROUP_PERMISSION_READ                               OTP_GROUP_PERMISSION_READ
#define FTP_GROUP_PERMISSION_WRITE                              OTP_GROUP_PERMISSION_WRITE
#define FTP_GROUP_PERMISSION_DELETE                             OTP_GROUP_PERMISSION_DELETE
#define FTP_OTHER_PERMISSION_READ                               OTP_OTHER_PERMISSION_READ
#define FTP_OTHER_PERMISSION_WRITE                              OTP_OTHER_PERMISSION_WRITE
#define FTP_OTHER_PERMISSION_DELETE                             OTP_OTHER_PERMISSION_DELETE

   /* The following structure is used to represent a Time/Data          */
   /* associated with a file when performing directory listing          */
   /* transfers.  Since no AM/PM field is provided, the Time is         */
   /* represented in 24 hour time.  The UTC_Time field is used to       */
   /* indicate whether the time is represented in UTC time ot Local     */
   /* Time.                                                             */
typedef struct _tagFTP_TimeDate_t
{
   Word_t    Year;
   Word_t    Month;
   Word_t    Day;
   Word_t    Hour;
   Word_t    Minute;
   Word_t    Second;
   Boolean_t UTC_Time;
} FTP_TimeDate_t;

#define FTP_TIME_DATE_SIZE                              (sizeof(FTP_TimeDate_t))

   /* The following structure is used to hold descriptive information   */
   /* about a Directory Entry that is being processed.  The Field Mask  */
   /* defines the fields in the structure that are valid.  The Directory*/
   /* Entry member specifies whether or not the Entry specifies a       */
   /* Directory Entry (TRUE) or a File Entry (FALSE).                   */
   /* * NOTE * The Name, Type, Owner, and Group members are formatted   */
   /*          as a NULL terminated ASCII strings with UTF-8 encoding.  */
typedef struct _tagFTP_Directory_Entry_t
{
   Boolean_t         DirectoryEntry;
   unsigned int      FieldMask;
   unsigned int      NameLength;
   char             *Name;
   unsigned int      Size;
   unsigned int      TypeLength;
   char             *Type;
   FTP_TimeDate_t    Modified;
   FTP_TimeDate_t    Created;
   FTP_TimeDate_t    Accessed;
   Word_t            Permission;
   unsigned int      OwnerLength;
   char             *Owner;
   unsigned int      GroupLength;
   char             *Group;
} FTP_Directory_Entry_t;

#define FTP_DIRECTORY_ENTRY_SIZE                        (sizeof(FTP_Directory_Entry_t))

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
typedef struct _tagFTP_Client_Directory_Request_Confirmation_Data_t
{
   unsigned int           FTPID;
   Boolean_t              RequestCompleted;
   Boolean_t              ParentDirectory;
   char                  *DirectoryName;
   unsigned int           NumberDirectoryEntries;
   FTP_Directory_Entry_t *FTPDirectoryEntries;
} FTP_Client_Directory_Request_Confirmation_Data_t;

#define FTP_CLIENT_DIRECTORY_REQUEST_CONFIRMATION_DATA_SIZE (sizeof(FTP_Client_Directory_Request_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Receives a response from the FTP Server regarding a previous      */
   /* FTP Change Directory Request.  The FTP ID member specifies the    */
   /* Local Client that has connected to the Remote FTP Server.         */
   /* The Success member specifies whether or not the request to change */
   /* the directory was successful or not.                              */
   /* * NOTE * The ChangedDirectoryName member is formatted as a NULL   */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTP_Client_Change_Directory_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   char         *ChangedDirectoryName;
} FTP_Client_Change_Directory_Confirmation_Data_t;

#define FTP_CLIENT_CHANGE_DIRECTORY_CONFIRMATION_DATA_SIZE (sizeof(FTP_Client_Change_Directory_Confirmation_Data_t))

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
typedef struct _tagFTP_Client_Directory_Delete_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   Boolean_t     DirectoryNotEmpty;
   char         *DeletedDirectoryName;
} FTP_Client_Directory_Delete_Confirmation_Data_t;

#define FTP_CLIENT_DIRECTORY_DELETE_CONFIRMATION_DATA_SIZE (sizeof(FTP_Client_Directory_Delete_Confirmation_Data_t))

   /* The following FTP Client Event is dispatched when a FTP Client    */
   /* Creates the specified directory on the remote FTP Server and a    */
   /* response from the Server is received.  The FTP ID member specifies*/
   /* the Local Client that the Remote Server has created the directory */
   /* on.  The Created Directory Name member specifies the name of the  */
   /* Directory that was created.                                       */
   /* * NOTE * The CreatedDirectoryName member is formatted as a NULL   */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagFTP_Client_Directory_Create_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   char         *CreatedDirectoryName;
} FTP_Client_Directory_Create_Confirmation_Data_t;

#define FTP_CLIENT_DIRECTORY_CREATE_CONFIRMATION_DATA_SIZE (sizeof(FTP_Client_Directory_Create_Confirmation_Data_t))

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
typedef struct _tagFTP_Client_File_Delete_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   char         *DeletedFileName;
} FTP_Client_File_Delete_Confirmation_Data_t;

#define FTP_CLIENT_FILE_DELETE_CONFIRMATION_DATA_SIZE (sizeof(FTP_Client_File_Delete_Confirmation_Data_t))

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
typedef struct _tagFTP_Client_File_Create_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   char         *CreatedFileName;
} FTP_Client_File_Create_Confirmation_Data_t;

#define FTP_CLIENT_FILE_CREATE_CONFIRMATION_DATA_SIZE (sizeof(FTP_Client_File_Create_Confirmation_Data_t))

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
typedef struct _tagFTP_Client_File_Put_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   char         *FileName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTP_Client_File_Put_Confirmation_Data_t;

#define FTP_CLIENT_FILE_PUT_CONFIRMATION_DATA_SIZE      (sizeof(FTP_Client_File_Put_Confirmation_Data_t))

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
typedef struct _tagFTP_Client_File_Get_Confirmation_Data_t
{
   unsigned int  FTPID;
   Boolean_t     Success;
   char         *FileName;
   Boolean_t     TransferComplete;
   unsigned int  TransferredLength;
   unsigned int  TotalLength;
} FTP_Client_File_Get_Confirmation_Data_t;

#define FTP_CLIENT_FILE_GET_CONFIRMATION_DATA_SIZE      (sizeof(FTP_Client_File_Get_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all FTP Profile Client Event Data Data.                   */
typedef struct _tagFTP_Client_Event_Data_t
{
   FTP_Client_Event_Type_t Event_Data_Type;
   Word_t                  Event_Data_Size;
   union
   {
      FTP_Client_Connect_Confirmation_Data_t           *FTP_Client_Connect_Confirmation_Data;
      FTP_Client_Disconnect_Indication_Data_t          *FTP_Client_Disconnect_Indication_Data;
      FTP_Client_Abort_Confirmation_Data_t             *FTP_Client_Abort_Confirmation_Data;
      FTP_Client_Directory_Request_Confirmation_Data_t *FTP_Client_Directory_Request_Confirmation_Data;
      FTP_Client_Change_Directory_Confirmation_Data_t  *FTP_Client_Change_Directory_Confirmation_Data;
      FTP_Client_Directory_Delete_Confirmation_Data_t  *FTP_Client_Directory_Delete_Confirmation_Data;
      FTP_Client_Directory_Create_Confirmation_Data_t  *FTP_Client_Directory_Create_Confirmation_Data;
      FTP_Client_File_Delete_Confirmation_Data_t       *FTP_Client_File_Delete_Confirmation_Data;
      FTP_Client_File_Create_Confirmation_Data_t       *FTP_Client_File_Create_Confirmation_Data;
      FTP_Client_File_Put_Confirmation_Data_t          *FTP_Client_File_Put_Confirmation_Data;
      FTP_Client_File_Get_Confirmation_Data_t          *FTP_Client_File_Get_Confirmation_Data;
   } Event_Data;
} FTP_Client_Event_Data_t;

#define FTP_CLIENT_EVENT_DATA_SIZE                      (sizeof(FTP_Client_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a FTP Server Profile Event Receive Data Callback.  This function  */
   /* will be called whenever a FTP Event occurs that is associated     */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the FTP Server Event Data that */
   /* occurred and the FTP Server Event Callback Parameter that was     */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the FTP Server Event Data ONLY in the context */
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e. this function DOES NOT have be reentrant).  It     */
   /* needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* File Transfer Server Profile Event will not be processed while    */
   /* this function call is outstanding).                               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving FTP Server Event    */
   /*            Packets.  A Deadlock WILL occur because NO FTP Server  */
   /*            Event Callbacks will be issued while this function is  */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *FTP_Server_Event_Callback_t)(unsigned int BluetoothStackID, FTP_Server_Event_Data_t *FTP_Server_Event_Data, unsigned long CallbackParameter);

   /* The following declared type represents the Prototype Function for */
   /* a FTP Client Event Receive Data Callback.  This function will be  */
   /* called whenever a FTP Event occurs that is associated with the    */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the FTP Client Event Data that occurred   */
   /* and the FTP Client Event Callback Parameter that was specified    */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the FTP Client Event Data ONLY in the context of this */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e. this function DOES NOT have be reentrant).  It needs to be  */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called   */
   /* in the Thread Context of a Thread that the User does NOT own.     */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another File Transfer*/
   /* Client Event will not be processed while this function call is    */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving FTP Client Event    */
   /*            Packets.  A Deadlock WILL occur because NO FTP Client  */
   /*            Event Callbacks will be issued while this function is  */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *FTP_Client_Event_Callback_t)(unsigned int BluetoothStackID, FTP_Client_Event_Data_t *FTP_Client_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a FTP File      */
   /* Server on the specified Bluetooth SPP Serial Port.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* Instance to use for the FTP File Server, the Local Serial Port    */
   /* Server Number to use, a pointer to a NULL terminated ASCII string */
   /* which specifies the Local Directory Path of the root directory    */
   /* of the File Server, the File Server Permissions to use when the   */
   /* responding to client requests, and the FTP Server Event Callback  */
   /* function (and parameter) to associate with the specified File     */
   /* Server.  The ServerPort parameter *MUST* be between               */
   /* SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.  This        */
   /* function returns a positive, non zero, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a FTP ID that can be used to reference the    */
   /* Opened FTP Server in ALL other FTP Server functions in this       */
   /* module.  Once a FTP Server is opened, it can only be              */
   /* Un-Registered via a call to the FTP_Close_Server() function       */
   /* (passing the return value from this function).                    */
   /* * NOTE * The RootDirectory parameter should be formatted as a NULL*/
   /*          terminated ASCII string with UTF-8 encoding.             */
BTPSAPI_DECLARATION int BTPSAPI FTP_Open_Server(unsigned int BluetoothStackID, unsigned int ServerPort, char *RootDirectory, unsigned long PermissionMask, FTP_Server_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Open_Server_t)(unsigned int BluetoothStackID, unsigned int ServerPort, char *RootDirectory, unsigned long PermissionMask, FTP_Server_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering a FTP    */
   /* Server (which was Registered by a successful call to the          */
   /* FTP_Open_Server() function).  This function accepts as input the  */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the FTP   */
   /* Server specified by the Second Parameter is valid for.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred (see BTERRORS.H).  Note that this       */
   /* function does NOT delete any SDP Service Record Handles.          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Close_Server(unsigned int BluetoothStackID, unsigned int FTPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Close_Server_t)(unsigned int BluetoothStackID, unsigned int FTPID);
#endif

   /* The following function is used to terminate a possible connection */
   /* to the local server.  This function can only be called by the FTP */
   /* Server.  A successfully call to this function will terminate the  */
   /* remote FTP Client connection to the local FTP server.  This       */
   /* function accepts as input the Bluetooth Stack ID of the Bluetooth */
   /* Stack which handles the Server and the FTP ID that was returned   */
   /* from the FTP_Open_Server() function.  This function returns zero  */
   /* if successful, or a negative return value if there was an error.  */
   /* This function does NOT Un-Register a FTP Server from the system,  */
   /* it ONLY disconnects any connection that is currently active.  The */
   /* FTP_Close_Server() function can be used to Un-Register the FTP    */
   /* Server.                                                           */
BTPSAPI_DECLARATION int BTPSAPI FTP_Close_Server_Connection(unsigned int BluetoothStackID, unsigned int FTPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Close_Server_Connection_t)(unsigned int BluetoothStackID, unsigned int FTPID);
#endif

   /* The following function is provided to allow a means to respond to */
   /* a request to connect to a FTP Server.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Stack which handles */
   /* the Server and the FTP ID that was returned from the              */
   /* FTP_Open_Server() function.  The final parameter to this function */
   /* is whether to accept the pending connection.  This function       */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI FTP_Server_Connect_Request_Response(unsigned int BluetoothStackID, unsigned int FTPID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Server_Connect_Request_Response_t)(unsigned int BluetoothStackID, unsigned int FTPID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Generic File Server Service Record to the SDP Database.  This     */
   /* function takes as input the Bluetooth Stack ID of the Local       */
   /* Bluetooth Protocol Stack, the FTP Server ID (which *MUST* have    */
   /* been obtained by calling the FTP_Open_Server() function.  The     */
   /* third parameter specifies the Service Name to associate with the  */
   /* SDP Record.  The final parameter is a pointer to a DWord_t which  */
   /* receives the SDP Service Record Handle if this function           */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be         */
   /* returned (see BTERRORS.H) and the SDPServiceRecordHandle value    */
   /* will be undefined.                                                */
   /* * NOTE * This function should only be called with the FTP ID that */
   /*          was returned from the FTP_Open_Server() function.  This  */
   /*          function should NEVER be used with FTP ID returned from  */
   /*          the FTP_Open_Remote_Server() function.                   */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          FTP_Un_Register_SDP_Record() to the                      */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI FTP_Register_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int FTPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Register_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* FTP Server SDP Service Record (specified by the third parameter)  */
   /* from the SDP Database.  This MACRO simply maps to the             */
   /* SDP_Delete_Service_Record() function.  This MACRO is only         */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the FTP Server ID (returned from    */
   /* a successful call to the FTP_Open_Server() function), and the SDP */
   /* Service Record Handle.  The SDP Service Record Handle was         */
   /* returned via a succesful call to the                              */
   /* FTP_Register_Server_SDP_Record() function.  See the               */
   /* FTP_Register_Server_SDP_Record() function for more information.   */
   /* This MACRO returns the result of the SDP_Delete_Service_Record()  */
   /* function, which is zero for success or a negative return error    */
   /* code (see BTERRORS.H).                                            */
#define FTP_Un_Register_SDP_Record(__BluetoothStackID, __FTPID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for opening a Remote FTP    */
   /* File Server.  This function accepts as input the Bluetooth Stack  */
   /* ID of the Bluetooth Protocol Stack that the FTP Client is         */
   /* associated with.  The second parameter is the Remote Bluetooth    */
   /* Device Address of the Bluetooth FTP Server to connect with.  The  */
   /* third parameter specifies the Remote Server Port to connect with. */
   /* the final two parameters specify the FTP Client Event Callback    */
   /* Function and the Callback Parameter to associate with this        */
   /* FTP Client.  The ServerPort parameter *MUST* be between           */
   /* SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.  This        */
   /* function returns a positive, non zero, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a FTP ID that can be used to reference the    */
   /* Opened FTP Server in ALL other FTP Client functions in this       */
   /* module.  Once a remote server is opened, it can only be closed    */
   /* via a call to the FTP_Close_Client() function (passing the return */
   /* value from this function).                                        */
BTPSAPI_DECLARATION int BTPSAPI FTP_Open_Remote_File_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, FTP_Client_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Open_Remote_File_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, FTP_Client_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is used to terminate a possible connection */
   /* to a remote server.  This function can only be called by the      */
   /* FTP Client.  A successful call to this function will terminate the*/
   /* remote FTP Connection.  This function accepts as input the        */
   /* Bluetooth Stack ID of the Bluetooth Stack which handles the Client*/
   /* and the FTP ID that was returned from the                         */
   /* the FTP_Open_Remote_File_Server() function.  This function        */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI FTP_Close_Client(unsigned int BluetoothStackID, unsigned int FTPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Close_Client_t)(unsigned int BluetoothStackID, unsigned int FTPID);
#endif

   /* The following function is provided to allow a FTP Client to       */
   /* request either the current directory (or a sub-directory off of   */
   /* the current directory) listing.  This function accepts as its     */
   /* first parameter the Bluetooth Stack ID of the Bluetooth Stack     */
   /* that is associated with the specified FTP Client.  The second     */
   /* parameters specifies the FTP Client ID (returned from a           */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third parameter specifies whether or not the current directory*/
   /* listing is to retrieved (NULL) or a sub-directory listing is to   */
   /* be retrieved (NON-NULL).  If a sub-directory is to be retrieved   */
   /* then the final parameter points to a NULL terminated ASCII string */
   /* that represents the Remote sub-directory name to browse.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Get_Directory(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Get_Directory_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a FTP Client to       */
   /* change the current working directory on the remote FTP Server.    */
   /* This function accepts as its first parameter the Bluetooth Stack  */
   /* ID of the Bluetooth Stack that is associated with the specified   */
   /* FTP Client.  The second parameters specifies the FTP Client ID    */
   /* (returned from a successful call to the                           */
   /* FTP_Open_Remote_File_Server() function).  The third parameter     */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Set_Directory(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Set_Directory_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a FTP Client to       */
   /* change the current working directory on the remote FTP Server to  */
   /* the Root Directory.  This function accepts as its first parameter */
   /* the Bluetooth Stack ID of the Bluetooth Stack that is associated  */
   /* with the specified FTP Client.  The second parameters specifies   */
   /* the FTP Client ID (returned from a successful call to the         */
   /* FTP_Open_Remote_File_Server() function).  This function returns   */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Set_Root_Directory(unsigned int BluetoothStackID, unsigned int FTPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Set_Root_Directory_t)(unsigned int BluetoothStackID, unsigned int FTPID);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Create a Directory on the Remote FTP Server.  This      */
   /* function accepts as its first parameter the Bluetooth Stack ID of */
   /* the Bluetooth Stack that is associated with this FTP Client.  The */
   /* second parameter specifies the FTP Client ID (returned from a     */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third parameter is a pointer to a NULL terminated ASCII string*/
   /* that specifies the Remote Directory Name of the Remote Directory  */
   /* to Create.  This directory name must be specified and cannot      */
   /* contain any path information.  This directory is created in the   */
   /* current working directory on the remote file server.  This        */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Create_Directory(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Create_Directory_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Delete a Directory on the Remote FTP Server.  This      */
   /* function accepts as its first parameter the Bluetooth Stack ID of */
   /* the Bluetooth Stack that is associated with this FTP Client.  The */
   /* second parameter specifies the FTP Client ID (returned from a     */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third parameter is a pointer to a NULL terminated ASCII string*/
   /* that specifies the Remote Directory Name of the Remote Directory  */
   /* to Delete.  This directory name must be specified and cannot      */
   /* contain any path information.  This directory is deleted in the   */
   /* current working directory on the remote file server.  This        */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The RemoteDirectoryName parameter should be formatted as */
   /*          a NULL terminated ASCII string with UTF-8 encoding.      */
   /* * NOTE * Due to an OBEX FTP limitation, when a delete operation   */
   /*          is specified, the client cannot be guaranteed whether a  */
   /*          file or directory of that name will be deleted.  In      */
   /*          other words, if the caller specifies this function and   */
   /*          a remote file exists (in the current remote working      */
   /*          directory), the file will be deleted (even thougth it is */
   /*          not a directory).  This is due to OBEX FTP which does    */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Delete_Directory(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Delete_Directory_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteDirectoryName);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Create a File on the Remote FTP Server.  This function  */
   /* accepts as its first parameter the Bluetooth Stack ID of the      */
   /* Bluetooth Stack that is associated with this FTP Client.  The     */
   /* second parameter specifies the FTP Client ID (returned from a     */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third parameter is a pointer to a NULL terminated ASCII string*/
   /* that specifies the Remote File Name of the Remote File to Create. */
   /* This File Name must be specified and cannot contain any path      */
   /* information.  This file is created in the current working         */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Create_File(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Create_File_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteFileName);
#endif

   /* The following function is provided to allow a means for the FTP   */
   /* Client to Delete a File on the Remote FTP Server.  This function  */
   /* accepts as its first parameter the Bluetooth Stack ID of the      */
   /* Bluetooth Stack that is associated with this FTP Client.  The     */
   /* second parameter specifies the FTP Client ID (returned from a     */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third parameter is a pointer to a NULL terminated ASCII string*/
   /* that specifies the Remote File Name of the Remote File to Delete. */
   /* This File Name must be specified and cannot contain any path      */
   /* information.  This file is deleted in the current working         */
   /* directory on the remote file server.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * The RemoteFileName parameter should be formatted as a    */
   /*          NULL terminated ASCII string with UTF-8 encoding.        */
   /* * NOTE * Due to an OBEX FTP limitation, when a delete operation   */
   /*          is specified, the client cannot be guaranteed whether a  */
   /*          file or directory of that name will be deleted.  In      */
   /*          other words, if the caller specifies this function and   */
   /*          a remote directory exists (in the current remote working */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Delete_File(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Delete_File_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteFileName);
#endif

   /* The following function is responsible for retrieving the specified*/
   /* Remote File from the specified Remote FTP Server.  This function  */
   /* accepts as its input parameters the Bluetooth Stack ID of the     */
   /* Bluetooth Stack that is associated with this FTP Client.  The     */
   /* second parameter specifies the FTP Client ID (returned from a     */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third parameter specifies the Remote File Name of the Remote  */
   /* File to retrieve.  This File Name is a pointer to a NULL          */
   /* terminated ASCII string and cannot contain any path information   */
   /* (or be empty or NULL).  The next parameter, RemoteFileLength, is  */
   /* a parameter that specifies the size (in Bytes) of the file that   */
   /* is being retrieved.  The last two parameters specify the local    */
   /* path (and filename) to store the retrieved file.  The LocalPath   */
   /* member (if specified) contains the Local Path to store the file   */
   /* name, and the final parameter specifies the Filename to store     */
   /* the file into.  The last parameter is not optional and *MUST* be  */
   /* specified.  If the LocalPath parameter is NULL, then the file     */
   /* is written to the current directory (on the local machine).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * The RemoteFileName, LocalPath, and LocalFileName         */
   /*          parameters should be formatted as NULL terminated ASCII  */
   /*          strings with UTF-8 encoding.                             */
   /* * NOTE * The Remote File Length parameter exists simply to allow  */
   /*          a Total Length value to be passed to the caller on File  */
   /*          Get Confirmations.  This value can be any value the      */
   /*          caller would like it to be because the FTP Profile       */
   /*          ignores this value and simply passes this value back to  */
   /*          the caller in the File Get Confirmation Event.           */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote FTP Server successfully issued the command.  The  */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the Remote FTP Server successfully executed */
   /*          the Request.                                             */
   /* * NOTE * Due to an OBEX FTP limitation, there can only be one     */
   /*          outstanding FTP Client Request active at any one time.   */
   /*          Because of this, another FTP Client Request cannot be    */
   /*          issued until either the current request is Aborted (by   */
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Get_File(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteFileName, unsigned int RemoteFileLength, char *LocalPath, char *LocalFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Get_File_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *RemoteFileName, unsigned int RemoteFileLength, char *LocalPath, char *LocalFileName);
#endif

   /* The following function is responsible for sending the specified   */
   /* Local File to the specified Remote FTP Server.  This function     */
   /* accepts as its input parameters the Bluetooth Stack ID of the     */
   /* Bluetooth Stack that is associated with this FTP Client.  The     */
   /* second parameter specifies the FTP Client ID (returned from a     */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* The third an fourth parameters specify the Local Path and Local   */
   /* Filename of the source file.  These two parameters are pointers   */
   /* to NULL terminated ASCII strings which specify the Path and File  */
   /* Name (respectively) of the Local File.  The Local Path parameter  */
   /* is optional, and if NON-NULL specifies the Local Path of the      */
   /* Local File.  The Local File Name parameter is NOT optional and    */
   /* specifies the File Name of the Local File.  The RemoteFileName    */
   /* parameter specifies the name of the file that is to be stored on  */
   /* the Remote FTP Server.  This file name cannot specify any path    */
   /* information.  The file that is sent to the Remote FTP Server is   */
   /* stored in the current working directory on the Remote FTP Server. */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
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
   /*          calling the FTP_Abort() function) or the current         */
   /*          Request is complete (this is signified by receiving a    */
   /*          Confirmation Event in the FTP Client Event Callback that */
   /*          was registered when the FTP Client was opened).          */
BTPSAPI_DECLARATION int BTPSAPI FTP_Put_File(unsigned int BluetoothStackID, unsigned int FTPID, char *LocalPath, char *LocalFileName, char *RemoteFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Put_File_t)(unsigned int BluetoothStackID, unsigned int FTPID, char *LocalPath, char *LocalFileName, char *RemoteFileName);
#endif

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding FTP Client Request.  This function accepts as its     */
   /* input parameters the Bluetooth Stack ID of the Bluetooth Stack    */
   /* for which the FTP Client is valid for.  The second parameter to   */
   /* this function specifies the FTP Client ID (returned from a        */
   /* successful call to the FTP_Open_Remote_File_Server() function).   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * This FTP Client Request is no different than the rest of */
   /*          the FTP Client Request functions in that a response to   */
   /*          this Request must be received before any other FTP Client*/
   /*          Request can be issued.                                   */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a FTP Client Request that is to be aborted may      */
   /*          have completed before the server was able to Abort the   */
   /*          request.  In either case, the caller will be notified    */
   /*          via FTP Client Callback of the status of the previous    */
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
   /*          response on the first Abort Packet is never received     */
   /*          (via the FTP Client Callback).  It should be noted that  */
   /*          under normal circumstances (i.e. the Remote Server is    */
   /*          functioning properly) this function will NEVER have to   */
   /*          be called twice.                                         */
BTPSAPI_DECLARATION int BTPSAPI FTP_Abort(unsigned int BluetoothStackID, unsigned int FTPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Abort_t)(unsigned int BluetoothStackID, unsigned int FTPID);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* query the current FTP Server Mode.  This function accepts as input*/
   /* the Bluetooth Stack ID of the Bluetooth Stack which handles the   */
   /* Server and the FTP ID that was returned from the FTP_Open_Server()*/
   /* function, and as the final parameter a pointer to a variable which*/
   /* will receive the current Server Mode Mask.  This function returns */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI FTP_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int FTPID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int FTPID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* change the current FTP Server Mode.  This function accepts as     */
   /* input the Bluetooth Stack ID of the Bluetooth Stack which handles */
   /* the Server and the FTP ID that was returned from the              */
   /* FTP_Open_Server() function, and as the final parameter the new    */
   /* Server Mode Mask to use.  This function returns zero if           */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI FTP_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int FTPID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FTP_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int FTPID, unsigned long ServerModeMask);
#endif

#endif

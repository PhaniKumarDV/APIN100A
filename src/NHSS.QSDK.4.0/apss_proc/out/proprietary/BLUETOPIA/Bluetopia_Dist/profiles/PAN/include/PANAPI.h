/*****< panapi.h >*************************************************************/
/*      Copyright 2004 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PANAPI - Stonestreet One Bluetooth Stack Personal Area Networking (PAN)   */
/*           Profile API Type Definitions, Constants, and Prototypes.         */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname       Description of Modification                  */
/*   --------  -----------       ---------------------------------------------*/
/*   04/23/04  R. Sledge         Initial creation.                            */
/******************************************************************************/
#ifndef __PANAPIH__
#define __PANAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

#include "BNEPType.h"           /* Bluetooth BNEP Type Definitions/Constants. */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTPAN_ERROR_INVALID_PARAMETER                              (-1000)
#define BTPAN_ERROR_NOT_INITIALIZED                                (-1001)
#define BTPAN_ERROR_INVALID_BLUETOOTH_STACK_ID                     (-1002)
#define BTPAN_ERROR_INSUFFICIENT_RESOURCES                         (-1003)
#define BTPAN_ERROR_SERVER_ALREADY_EXISTS                          (-1004)
#define BTPAN_ERROR_CONNECTION_ALREADY_EXISTS                      (-1005)
#define BTPAN_ERROR_CONTEXT_ALREADY_EXISTS                         (-1006)
#define BTPAN_ERROR_INVALID_VNET_DRIVER                            (-1007)
#define BTPAN_ERROR_INVALID_VNET_DRIVER_ADDRESS                    (-1008)

   /* SDP Profile UUID's for the Personal Area Networking Profile.      */

   /* SDP Personal Area Network User Profile UUID's.                    */

   /* The following MACRO is a utility MACRO that assigns the Personal  */
   /* Area Network User (PANU) Bluetooth Universally Unique Identifier  */
   /* (PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_16) to the specified     */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the                         */
   /* PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_16 Constant value.        */
#define SDP_ASSIGN_PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_16(_x)    ASSIGN_SDP_UUID_16((_x), 0x11, 0x15)

   /* The following MACRO is a utility MACRO that assigns the Personal  */
   /* Area Network User (PANU) Bluetooth Universally Unique Identifier  */
   /* (PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_32) to the specified     */
   /* UUID_32_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_32_t variable that is to receive the                         */
   /* PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_32 Constant value.        */
#define SDP_ASSIGN_PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_32(_x)    ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x15)

   /* The following MACRO is a utility MACRO that assigns the Personal  */
   /* Area Network User (PANU) Bluetooth Universally Unique Identifier  */
   /* (PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_128) to the specified    */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_128 Constant value.       */
#define SDP_ASSIGN_PERSONAL_AREA_NETWORK_USER_PROFILE_UUID_128(_x)   ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x15, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Network Access Point UUID's.                                  */

   /* The following MACRO is a utility MACRO that assigns the Network   */
   /* Access Point (NAP) Bluetooth Universally Unique Identifier        */
   /* (NETWORK_ACCESS_POINT_PROFILE_UUID_16) to the specified UUID_16_t */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the                                   */
   /* NETWORK_ACCESS_POINT_PROFILE_UUID_16 Constant value.              */
#define SDP_ASSIGN_NETWORK_ACCESS_POINT_PROFILE_UUID_16(_x)          ASSIGN_SDP_UUID_16((_x), 0x11, 0x16)

   /* The following MACRO is a utility MACRO that assigns the Network   */
   /* Access Point (NAP) Bluetooth Universally Unique Identifier        */
   /* (NETWORK_ACCESS_POINT_PROFILE_UUID_32) to the specified UUID_32_t */
   /* variable.  This MACRO accepts one parameter which is the UUID_32_t*/
   /* variable that is to receive the                                   */
   /* NETWORK_ACCESS_POINT_PROFILE_UUID_32 Constant value.              */
#define SDP_ASSIGN_NETWORK_ACCESS_POINT_PROFILE_UUID_32(_x)          ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x16)

   /* The following MACRO is a utility MACRO that assigns the Network   */
   /* Access Point (NAP) Bluetooth Universally Unique Identifier        */
   /* (NETWORK_ACCESS_POINT_PROFILE_UUID_128) to the specified          */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* NETWORK_ACCESS_POINT_PROFILE_UUID_128 Constant value.             */
#define SDP_ASSIGN_NETWORK_ACCESS_POINT_PROFILE_UUID_128(_x)         ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x16, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Group Ad-hoc Network UUID's.                                  */

   /* The following MACRO is a utility MACRO that assigns the Group     */
   /* Ad-hoc Network (GN) Bluetooth Universally Unique Identifier       */
   /* (GROUP_ADHOC_NETWORK_PROFILE_UUID_16) to the specified UUID_16_t  */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the                                   */
   /* GROUP_ADHOC_NETWORK_PROFILE_UUID_16 Constant value.               */
#define SDP_ASSIGN_GROUP_ADHOC_NETWORK_PROFILE_UUID_16(_x)           ASSIGN_SDP_UUID_16((_x), 0x11, 0x17)

   /* The following MACRO is a utility MACRO that assigns the Group     */
   /* Ad-hoc Network (GN) Bluetooth Universally Unique Identifier       */
   /* (GROUP_ADHOC_NETWORK_PROFILE_UUID_32) to the specified UUID_32_t  */
   /* variable.  This MACRO accepts one parameter which is the UUID_32_t*/
   /* variable that is to receive the                                   */
   /* GROUP_ADHOC_NETWORK_PROFILE_UUID_32 Constant value.               */
#define SDP_ASSIGN_GROUP_ADHOC_NETWORK_PROFILE_UUID_32(_x)           ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x17)

   /* The following MACRO is a utility MACRO that assigns the Group     */
   /* Ad-hoc Network (GN) Bluetooth Universally Unique Identifier       */
   /* (GROUP_ADHOC_NETWORK_PROFILE_UUID_128) to the specified UUID_128_t*/
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* GROUP_ADHOC_NETWORK_PROFILE_UUID_128 Constant value.              */
#define SDP_ASSIGN_GROUP_ADHOC_NETWORK_PROFILE_UUID_128(_x)          ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x17, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following constants represent the Service Discovery Protocol  */
   /* Protocol Service Multiplexor (PSM) values that PAN runs over the  */
   /* L2CAP Protocol (which is the only defined mechanism).             */
#define PAN_L2CAP_PSM                                               (0x0F)

   /* The following value represents the minimum supported L2CAP MTU    */
   /* that a PAN implementation *MUST* support.  The L2CAP MTU can be   */
   /* larger, but it cannot be smaller than this number.                */
#define PAN_MINIMUM_L2CAP_MTU                                     (BNEP_MINIMUM_L2CAP_MTU)

   /* The following define specifies the PAN User (PANU), Network Access*/
   /* Point (NAP), and Gateway (GN) Profile version included in the SDP */
   /* record for the PAN profile, as specified in the relevant          */
   /* specification.                                                    */
#define PAN_PANU_PROFILE_VERSION                                  (0x0100)

#define PAN_NAP_PROFILE_VERSION                                   (0x0100)

#define PAN_GN_PROFILE_VERSION                                    (0x0100)

   /* The following constant defines the BNEP Protocol Version Number   */
   /* used within the SDP Record for PAN Server Service Records.        */
#define PAN_BNEP_PROTOCOL_VERSION                                 (BNEP_PROTOCOL_VERSION)

   /* The following constants represent the bit masks which may be      */
   /* passed to the PAN_Open_Server() function to enable the specified  */
   /* services for the Personal Area Networking Profile Server.         */
#define PAN_PERSONAL_AREA_NETWORK_USER_SERVICE                    (0x00000001)
#define PAN_NETWORK_ACCESS_POINT_SERVICE                          (0x00000002)
#define PAN_GROUP_ADHOC_NETWORK_SERVICE                           (0x00000004)

   /* The following constants represent the Security Description values */
   /* which may be passed in to the PAN_Register_XXX_SDP_Record()       */
   /* functions Security Description parameter.                         */
#define PAN_SECURITY_DESCRIPTION_NONE                             (0x0000)
#define PAN_SECURITY_DESCRIPTION_SERVICE_LEVEL_ENFORCED_SECURITY  (0x0001)
#define PAN_SECURITY_DESCRIPTION_802_1X_SECURITY                  (0x0002)

   /* The following constants represent the Network Access Type values  */
   /* which may be passed in to the                                     */
   /* PAN_Register_Network_Access_Point_SDP_Record() functions Network  */
   /* Access Type parameter.                                            */
#define PAN_NETWORK_ACCESS_TYPE_PSTN                              (0x0000)
#define PAN_NETWORK_ACCESS_TYPE_ISDN                              (0x0001)
#define PAN_NETWORK_ACCESS_TYPE_DSL                               (0x0002)
#define PAN_NETWORK_ACCESS_TYPE_CABLE_MODEM                       (0x0003)
#define PAN_NETWORK_ACCESS_TYPE_10_MB_ETHERNET                    (0x0004)
#define PAN_NETWORK_ACCESS_TYPE_100_MB_ETHERNET                   (0x0005)
#define PAN_NETWORK_ACCESS_TYPE_4_MB_TOKEN_RING                   (0x0006)
#define PAN_NETWORK_ACCESS_TYPE_16_MB_TOKEN_RING                  (0x0007)
#define PAN_NETWORK_ACCESS_TYPE_100_MB_TOKEN_RING                 (0x0008)
#define PAN_NETWORK_ACCESS_TYPE_FDDI                              (0x0009)
#define PAN_NETWORK_ACCESS_TYPE_GSM                               (0x000A)
#define PAN_NETWORK_ACCESS_TYPE_CDMA                              (0x000B)
#define PAN_NETWORK_ACCESS_TYPE_GPRS                              (0x000C)
#define PAN_NETWORK_ACCESS_TYPE_3G_CELLULAR                       (0x000D)
#define PAN_NETWORK_ACCESS_TYPE_OTHER                             (0xFFFE)

   /* The following constants represent the Open Status Values that are */
   /* possible in the Open Confirmation Event.                          */
#define PAN_OPEN_STATUS_SUCCESS                                     (0x00)
#define PAN_OPEN_STATUS_CONNECTION_TIMEOUT                          (0x01)
#define PAN_OPEN_STATUS_CONNECTION_REFUSED                          (0x02)
#define PAN_OPEN_STATUS_UNKNOWN_ERROR                               (0x03)

   /* The following enumerated type represents the supported Server     */
   /* Connection Modes supported by the Personal Area Networking Server.*/
typedef enum
{
   psmAutomaticAccept,
   psmAutomaticReject,
   psmManualAccept
} PAN_Server_Connection_Mode_t;

   /* The following enumerated type specifies the different Service     */
   /* Types that are supported by the Personal Area Networking Profile. */
   /* These types are specified when Opening a Remote Server and        */
   /* returned in the Open Indication Event.                            */
typedef enum
{
   pstPersonalAreaNetworkUser,
   pstNetworkAccessPoint,
   pstGroupAdhocNetwork
} PAN_Service_Type_t;

   /* PAN Event API Types.                                              */
typedef enum
{
   etPAN_Open_Request_Indication,
   etPAN_Open_Indication,
   etPAN_Open_Confirmation,
   etPAN_Close_Indication
} PAN_Event_Type_t;

   /* The following Event is dispatched when a Remote Client Requests a */
   /* Connection to a Local Server.  The PANID member specifies the     */
   /* Identifier of the Connection to the Local Server.  The BD_ADDR    */
   /* member specifies the address of the Remote Client requesting the  */
   /* connection to the Local Server.                                   */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            PAN_Open_Request_Response() function in order to accept*/
   /*            or reject the outstanding Open Request.                */
typedef struct _tagPAN_Open_Request_Indication_Data_t
{
   unsigned int PANID;
   BD_ADDR_t    BD_ADDR;
} PAN_Open_Request_Indication_Data_t;

#define PAN_OPEN_REQUEST_INDICATION_DATA_SIZE           (sizeof(PAN_Open_Request_Indication_Data_t))

   /* The following Event is dispatched when a Remote Client Opens a PAN*/
   /* Connection to a Local Server Service.  The PANID member specifies */
   /* the Identifier of the Connection to the Local Servers service.    */
   /* The BD_ADDR member specifies the address of the Remote Client     */
   /* which has connected to the Local Server.  The ServiceType member  */
   /* specifies the Server Service Type that has been connected to by   */
   /* the Remote Client.                                                */
typedef struct _tagPAN_Open_Indication_Data_t
{
   unsigned int       PANID;
   BD_ADDR_t          BD_ADDR;
   PAN_Service_Type_t ServiceType;
} PAN_Open_Indication_Data_t;

#define PAN_OPEN_INDICATION_DATA_SIZE                   (sizeof(PAN_Open_Indication_Data_t))

   /* The following Event is dispatched to the Local Client to indicate */
   /* success or failure of a previously submitted Open Request.  The   */
   /* PANID member specifies the Identifier of the Local Client that has*/
   /* requested the Connection.  The OpenStatus member specifies the    */
   /* status of the Connection attempt.  Possible values for the Open   */
   /* Status can be found above.                                        */
typedef struct _tagPAN_Open_Confirmation_Data_t
{
   unsigned int PANID;
   unsigned int OpenStatus;
} PAN_Open_Confirmation_Data_t;

#define PAN_OPEN_CONFIRMATION_DATA_SIZE                 (sizeof(PAN_Open_Confirmation_Data_t))

   /* The following event is dispatched when the Remote Device          */
   /* disconnects from the Local Device.  The PANID member specifies the*/
   /* Identifier for the Local Device that was disconnected.  This Event*/
   /* is NOT Dispatched in response to the Local Device requesting the  */
   /* disconnection.  This Event is dispatched only when the remote     */
   /* device terminates the Connection (and/or Bluetooth Link).         */
typedef struct _tagPAN_Close_Indication_Data_t
{
   unsigned int PANID;
} PAN_Close_Indication_Data_t;

#define PAN_CLOSE_INDICATION_DATA_SIZE                  (sizeof(PAN_Close_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all PAN Event Data Data.                                  */
typedef struct _tagPAN_Event_Data_t
{
   PAN_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      PAN_Open_Request_Indication_Data_t *PAN_Open_Request_Indication_Data;
      PAN_Open_Indication_Data_t         *PAN_Open_Indication_Data;
      PAN_Open_Confirmation_Data_t       *PAN_Open_Confirmation_Data;
      PAN_Close_Indication_Data_t        *PAN_Close_Indication_Data;
   } Event_Data;
} PAN_Event_Data_t;

#define PAN_EVENT_DATA_SIZE                             (sizeof(PAN_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a PAN Event Callback.  This function will be called whenever a PAN*/
   /* Event occurs that is associated with the specified Bluetooth Stack*/
   /* ID.  This function passes to the caller the Bluetooth Stack ID,   */
   /* the PAN Event Data that occurred, and the PAN Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the PAN Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It needs to be noted, however, that if the same      */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another PAN Event will not be processed while this function call  */
   /* is outstanding).                                                  */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving PAN Events.  A      */
   /*            deadlock WILL occur because NO PAN Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *PAN_Event_Callback_t)(unsigned int BluetoothStackID, PAN_Event_Data_t *PAN_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for initializing the        */
   /* Personal Area Networking Profile (PAN) Profile.  This function    */
   /* MUST be called before any other PAN Profile function.  The first  */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Stack to be used by this PAN Profile instance.  The     */
   /* final parameter to this function specifies the Virtual Network    */
   /* Driver Index of the Virtual Network Driver in the system to be    */
   /* used by this PAN Profile instance.  This function returns zero if */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** This function may be called once for a given Bluetooth */
   /*            Stack ID.                                              */
BTPSAPI_DECLARATION int BTPSAPI PAN_Initialize(unsigned int BluetoothStackID, unsigned int VNETIndex);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Initialize_t)(unsigned int BluetoothStackID, unsigned int VNETIndex);
#endif

   /* The following function is responsible for cleaning up a previously*/
   /* initialized PAN Profile instance.  After calling this function,   */
   /* PAN_Initialize() must be called for the PAN Profile using the     */
   /* specified Bluetooth Stack again before any other PAN Profile      */
   /* function can be called.  This function accepts the Bluetooth Stack*/
   /* ID of the Bluetooth Stack used by the PAN Profile Instance being  */
   /* cleaned up.  This function returns zero if successful, or a       */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI PAN_Cleanup(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Cleanup_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for opening the Personal    */
   /* Area Networking Server.  The first parameter to this function is  */
   /* the Bluetooth Stack ID of the Bluetooth Stack associated with this*/
   /* Personal Area Networking Server.  The second parameter to this    */
   /* function is a bit mask containing the Service Types that this     */
   /* Personal Area Networking Server provides.  The final two          */
   /* parameters specify the PAN Event Callback function and Callback   */
   /* parameter, respectively, of the PAN Event Callback that is to     */
   /* process any events associated with this Server.  This function    */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** PAN IDs associated with individual connections made to */
   /*            registered services will be returned in the open       */
   /*            indication to that service.                            */
BTPSAPI_DECLARATION int BTPSAPI PAN_Open_Server(unsigned int BluetoothStackID, unsigned long ServicesMask, PAN_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Open_Server_t)(unsigned int BluetoothStackID, unsigned long ServicesMask, PAN_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened Personal Area Networking Server.  The first parameter to   */
   /* this function is the Bluetooth Stack ID of the Bluetooth Stack    */
   /* associated with this Personal Area Networking Server to be closed.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if an error occurred.  Note that this function does NOT*/
   /* delete any SDP Service Record Handles.                            */
BTPSAPI_DECLARATION int BTPSAPI PAN_Close_Server(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Close_Server_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Personal Area Networking */
   /* Server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Stack associated with the Personal Area */
   /* Networking Server that is responding to the request.  The second  */
   /* parameter to this function is the PAN ID of the Personal Area     */
   /* Networking Connection for which a connection request was received.*/
   /* The final parameter to this function specifies whether to accept  */
   /* the pending connection request (or to reject the request).  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
   /* ** NOTE ** The connection to the server is not established until  */
   /*            a etPAN_Open_Request_Indication event has occurred.    */
   /* ** NOTE ** A etPAN_Close_Indication will be dispatched if the     */
   /*            connection isn't fully established after receiving     */
   /*            this event.                                            */
BTPSAPI_DECLARATION int BTPSAPI PAN_Open_Request_Response(unsigned int BluetoothStackID, unsigned int PANID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int PANID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Personal Area Network User (PANU) Record to the SDP Database.     */
   /* This function accepts as its first parameter the Bluetooth Stack  */
   /* ID of the Bluetooth Stack associated with this Personal Area      */
   /* Network User Service Server SDP Record.  The second parameter to  */
   /* this function is the Number of Network Packet Types pointed to by */
   /* the next parameter.  The third parameter to this function is a    */
   /* pointer to an array of Network Packet Types supported by the      */
   /* Server which this SDP Record represents.  The fourth parameter to */
   /* this function is the Service Name to be associated with this SDP  */
   /* Record.  The fifth parameter to this function is the Service      */
   /* Description to be associated with this SDP record.  The sixth     */
   /* parameter to this function is the Security Description to be      */
   /* associated with this SDP Record.  The final parameter is a pointer*/
   /* to a DWord_t which receives the SDP Service Record Handle if this */
   /* function successfully creates an SDP Service Record.  If this     */
   /* function returns zero, then the SDPServiceRecordHandle entry will */
   /* contain the Service Record Handle of the added SDP Service Record.*/
   /* If this function fails, a negative return error code will be      */
   /* returned and the SDPServiceRecordHandle value will be undefined.  */
   /* ** NOTE ** This function may only be called to install a Personal */
   /*            Area Network User Service Record if the                */
   /*            PAN_PERSONAL_AREA_NETWORK_USER_SERVICE flag was        */
   /*            specified in the call to the PAN_Open_Server()         */
   /*            function.                                              */
   /* ** NOTE ** The Service Record Handle that is returned from this   */
   /*            function will remain in the SDP Record Database until  */
   /*            it is deleted by calling the                           */
   /*            SDP_Delete_Service_Record() function.                  */
   /* ** NOTE ** A Language Base Attribute ID List is created that      */
   /*            specifies that 0x0100 is UTF-8 Encoded, English        */
   /*            Language.                                              */
BTPSAPI_DECLARATION int BTPSAPI PAN_Register_Personal_Area_Network_User_SDP_Record(unsigned int BluetoothStackID, unsigned int NumberNetworkPacketTypes, Word_t NetworkPacketTypeList[], char *ServiceName, char *ServiceDescription, Word_t SecurityDescription, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Register_Personal_Area_Network_User_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int NumberNetworkPacketTypes, Word_t NetworkPacketTypeList[], char *ServiceName, char *ServiceDescription, Word_t SecurityDescription, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Network Access Point (NAP) Record to the SDP Database.  This      */
   /* function accepts as its first parameter the Bluetooth Stack ID of */
   /* the Bluetooth Stack associated with this Network Access Point     */
   /* Service Server SDP Record.  The second parameter to this function */
   /* is the Number of Network Packet Types pointed to by the next      */
   /* parameter.  The third parameter to this function is a pointer to  */
   /* an array of Network Packet Types supported by the Server which    */
   /* this SDP Record represents.  The fourth parameter to this function*/
   /* is the Service Name to be associated with this SDP Record.  The   */
   /* fifth parameter to this function is the Service Description to be */
   /* associated with this SDP record.  The sixth parameter to this     */
   /* function is the Security Description to be associated with this   */
   /* SDP Record.  The seventh parameter to this function is the Network*/
   /* Access Type to be associated with this SDP Record.  The eighth    */
   /* parameter to this function is the Maximum Network Access Rate to  */
   /* be associated with this SDP Record, this is the data rate of the  */
   /* connection the Network Access Point has to the network which it is*/
   /* providing access.  The final parameter is a pointer to a DWord_t  */
   /* which receives the SDP Service Record Handle if this function     */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* and the SDPServiceRecordHandle value will be undefined.           */
   /* ** NOTE ** This function may only be called to install a Network  */
   /*            Access Point Service Record if the                     */
   /*            PAN_NETWORK_ACCESS_POINT_SERVICE flag was              */
   /*            specified in the call to the PAN_Open_Server()         */
   /*            function.                                              */
   /* ** NOTE ** The Service Record Handle that is returned from this   */
   /*            function will remain in the SDP Record Database until  */
   /*            it is deleted by calling the                           */
   /*            SDP_Delete_Service_Record() function.                  */
   /* ** NOTE ** A Language Base Attribute ID List is created that      */
   /*            specifies that 0x0100 is UTF-8 Encoded, English        */
   /*            Language.                                              */
BTPSAPI_DECLARATION int BTPSAPI PAN_Register_Network_Access_Point_SDP_Record(unsigned int BluetoothStackID, unsigned int NumberNetworkPacketTypes, Word_t NetworkPacketTypeList[], char *ServiceName, char *ServiceDescription, Word_t SecurityDescription, Word_t NetworkAccessType, DWord_t MaxNetAccessRate, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Register_Network_Access_Point_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int NumberNetworkPacketTypes, Word_t NetworkPacketTypeList[], char *ServiceName, char *ServiceDescription, Word_t SecurityDescription, Word_t NetworkAccessType, DWord_t MaxNetAccessRate, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following function is provided to allow a means to add a Group*/
   /* Ad-hoc Network (GN) Record to the SDP Database.  This function    */
   /* accepts as its first parameter the Bluetooth Stack ID of the      */
   /* Bluetooth Stack associated with this Group Ad-hoc Network Service */
   /* Server SDP Record.  The second parameter to this function is the  */
   /* Number of Network Packet Types pointed to by the next parameter.  */
   /* The third parameter to this function is a pointer to an array of  */
   /* Network Packet Types supported by the Server which this SDP Record*/
   /* represents.  The fourth parameter to this function is the Service */
   /* Name to be associated with this SDP Record.  The fifth parameter  */
   /* to this function is the Service Description to be associated with */
   /* this SDP record.  The sixth parameter to this function is the     */
   /* Security Description to be associated with this SDP Record.  The  */
   /* final parameter is a pointer to a DWord_t which receives the SDP  */
   /* Service Record Handle if this function successfully creates an SDP*/
   /* Service Record.  If this function returns zero, then the          */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned and the               */
   /* SDPServiceRecordHandle value will be undefined.                   */
   /* ** NOTE ** This function may only be called to install a Group    */
   /*            Ad-hoc Network Service Record if the                   */
   /*            PAN_GROUP_ADHOC_NETWORK_SERVICE flag was               */
   /*            specified in the call to the PAN_Open_Server()         */
   /*            function.                                              */
   /* ** NOTE ** The Service Record Handle that is returned from this   */
   /*            function will remain in the SDP Record Database until  */
   /*            it is deleted by calling the                           */
   /*            SDP_Delete_Service_Record() function.                  */
   /* ** NOTE ** A Language Base Attribute ID List is created that      */
   /*            specifies that 0x0100 is UTF-8 Encoded, English        */
   /*            Language.                                              */
BTPSAPI_DECLARATION int BTPSAPI PAN_Register_Group_Adhoc_Network_SDP_Record(unsigned int BluetoothStackID, unsigned int NumberNetworkPacketTypes, Word_t NetworkPacketTypeList[], char *ServiceName, char *ServiceDescription, Word_t SecurityDescription, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Register_Group_Adhoc_Network_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int NumberNetworkPacketTypes, Word_t NetworkPacketTypeList[], char *ServiceName, char *ServiceDescription, Word_t SecurityDescription, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes a      */
   /* Personal Area Networking SDP Service Record (specified by the     */
   /* third parameter) from the SDP Database.  This MACRO simply maps to*/
   /* the SDP_Delete_Service_Record() function.  This MACRO is only     */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on and the SDP Service Record Handle.   */
   /* The SDP Service Record Handle was returned via a successful call  */
   /* to the PAN_Register_Personal_Area_Network_User_SDP_Record(),      */
   /* PAN_Register_Network_Access_Point_SDP_Record, or                  */
   /* PAN_Register_Group_Adhoc_Network_SDP_Record function.  This MACRO */
   /* returns the result of the SDP_Delete_Service_Record() function,   */
   /* which is zero for success or a negative return error code (see    */
   /* BTERRORS.H).                                                      */
#define PAN_Un_Register_SDP_Record(__BluetoothStackID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for opening a connection to */
   /* a specific Remote Personal Area Networking Server Service.  This  */
   /* function accepts as its first parameter the Bluetooth Stack ID of */
   /* the Bluetooth Stack which is to open a connection to the Remote   */
   /* Server.  The second parameter to this function is the Board       */
   /* Address (NON NULL) of the Remote Bluetooth Personal Area          */
   /* Networking Server to connect with.  The third parameter to this   */
   /* function is the Service Type that the Local Personal Area         */
   /* Networking Client will be using for the active connection.  The   */
   /* fourth parameter to this function is the Remote Personal Area     */
   /* Networking Server Service to connect with.  The final two         */
   /* parameters specify the PAN Event Callback function and Callback   */
   /* parameter, respectively, of the PAN Event Callback that is to     */
   /* process any events associated with this Connection.  This function*/
   /* returns a non-zero, positive, number on success or a negative     */
   /* return value if there was an error.  A successful return value    */
   /* will be a PAN ID that can used to reference the Opened Personal   */
   /* Area Networking Client Connection in ALL other PAN Client         */
   /* functions in this module.                                         */
BTPSAPI_DECLARATION int BTPSAPI PAN_Open_Remote_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, PAN_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Open_Remote_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, PAN_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for closing any active      */
   /* Personal Area Networking Client or Server Connection.  The first  */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Stack associated with this Personal Area Networking     */
   /* Connection to be closed.  The final parameter to this function is */
   /* the PAN ID of the Personal Area Networking Connection to close.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if an error occurred.                                  */
BTPSAPI_DECLARATION int BTPSAPI PAN_Close(unsigned int BluetoothStackID, unsigned int PANID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Close_t)(unsigned int BluetoothStackID, unsigned int PANID);
#endif

   /* The following function is responsible for retrieving the current  */
   /* Personal Area Networking Server Connection Mode.  This function   */
   /* accepts as its first parameter the Bluetooth Stack ID of the      */
   /* Bluetooth Stack on which the server exists.  The final parameter  */
   /* to this function is a pointer to a Server Connection Mode variable*/
   /* which will receive the current Server Connection Mode.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
   /* ** NOTE ** The Default Server Connection Mode is                  */
   /*            psmAutomaticAccept.                                    */
   /* ** NOTE ** This function is used for PAN Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI PAN_Get_Server_Connection_Mode(unsigned int BluetoothStackID, PAN_Server_Connection_Mode_t *ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, PAN_Server_Connection_Mode_t *ServerConnectionMode);
#endif

   /* The following function is responsible for setting the Personal    */
   /* Area Networking Server Connection Mode.  This function accepts as */
   /* its first parameter the Bluetooth Stack ID of the Bluetooth Stack */
   /* on which the server exists.  The final parameter to this function */
   /* is the new Server Connection Mode to set the Server to use.  This */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
   /* ** NOTE ** The Default Server Connection Mode is                  */
   /*            psmAutomaticAccept.                                    */
   /* ** NOTE ** This function is used for PAN Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI PAN_Set_Server_Connection_Mode(unsigned int BluetoothStackID, PAN_Server_Connection_Mode_t ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PAN_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, PAN_Server_Connection_Mode_t ServerConnectionMode);
#endif

#endif

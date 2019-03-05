/****< lnsapi.h >**************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LNSAPI - Stonestreet One Bluetooth Location and Navigation Service        */
/*           (GATT based) API Type Definitions, Constants, and Prototypes.    */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/26/13  A. Parashar    Initial creation.                               */
/******************************************************************************/
#ifndef __LNSAPIH__
#define __LNSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "LNSTypes.h"      /* Location and Navigation Service Types/Constants.*/

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define LNS_ERROR_INVALID_PARAMETER                            (-1000)
#define LNS_ERROR_INVALID_BLUETOOTH_STACK_ID                   (-1001)
#define LNS_ERROR_INSUFFICIENT_RESOURCES                       (-1002)
#define LNS_ERROR_INSUFFICIENT_BUFFER_SPACE                    (-1003)
#define LNS_ERROR_SERVICE_ALREADY_REGISTERED                   (-1004)
#define LNS_ERROR_INVALID_INSTANCE_ID                          (-1005)
#define LNS_ERROR_MALFORMATTED_DATA                            (-1006)
#define LNS_ERROR_INDICATION_OUTSTANDING                       (-1007)

   /* The following structure contains the Handles that will need to be */
   /* cached by a LNS client in order to only do service discovery once.*/
typedef struct _tagLNS_Client_Information_t
{
   Word_t LN_Feature;
   Word_t Location_And_Speed;
   Word_t Location_And_Speed_Client_Configuration;
   Word_t Position_Quality;
   Word_t LN_Control_Point;
   Word_t LN_Control_Point_Client_Configuration;
   Word_t Navigation;
   Word_t Navigation_Client_Configuration;
} LNS_Client_Information_t;

#define LNS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(LNS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a LNS Server.                           */
typedef struct _tagLNS_Server_Information_t
{
   Word_t Location_And_Speed_Client_Configuration;
   Word_t LN_Control_Point_Client_Configuration;
   Word_t Navigation_Client_Configuration;
} LNS_Server_Information_t;

#define LNS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(LNS_Server_Information_t))

   /* The following structure represents the format of a LNS Date/Time  */
   /* value.  This is used to represent the Date-Time which contains    */
   /* the Day/Month/Year and Hours:Minutes:Second data.                 */
   /* * NOTE * A value of 0 for the year, month or day fields shall not */
   /*          be used.                                                 */
typedef struct _tagLNS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} LNS_Date_Time_Data_t;

#define LNS_DATE_TIME_DATA_SIZE                          (sizeof(LNS_Date_Time_Data_t))

   /* The following structure contains Location and speed data that is  */
   /* passed to the function that builds the Location and speed packet. */
   /* * NOTE * If                                                       */
   /*          LNS_LOCATION_AND_SPEED_FLAG_INSTANTANEOUS_SPEED_PRESENT  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          InstantaneousSpeed.                                      */
   /* * NOTE * If LNS_LOCATION_AND_SPEED_FLAG_TOTAL_DISTANCE_PRESENT    */
   /*          Flag is set, then a valid value must be entered for      */
   /*          TotalDistance.                                           */
   /* * NOTE * If LNS_LOCATION_AND_SPEED_FLAG_LOCATION_PRESENT Flag is  */
   /*          set, then a valid value must be entered for              */
   /*          LocationLatitude and LocationLongitude.                  */
   /* * NOTE * If LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_PRESENT Flag is */
   /*          set, then a valid value must be entered for Elevation.   */
   /* * NOTE * If LNS_LOCATION_AND_SPEED_FLAG_HEADING_PRESENT Flag is   */
   /*          set, then a valid value must be entered for Heading.     */
   /* * NOTE * If LNS_LOCATION_AND_SPEED_FLAG_ROLLING_TIME_PRESENT      */
   /*          Flag is set, then a valid value must be entered for      */
   /*          RollingTime.                                             */
   /* * NOTE * If LNS_LOCATION_AND_SPEED_FLAG_UTC_TIME_PRESENT Flag is  */
   /*          set, then a valid value must be entered for UTCTime.     */
typedef struct _tagLNS_Location_Speed_Data_t
{
   Word_t               Flags;
   Word_t               InstantaneousSpeed;
   Byte_t               TotalDistance[3];
   SDWord_t             LocationLatitude;
   SDWord_t             LocationLongitude;
   SByte_t              Elevation[3];
   Word_t               Heading;
   Byte_t               RollingTime;
   LNS_Date_Time_Data_t UTCTime;
} LNS_Location_Speed_Data_t;

#define LNS_LOCATION_SPEED_DATA_SIZE                     (sizeof(LNS_Location_Speed_Data_t))

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified Date Time is valid.  The only parameter to this function*/
   /* is the LNS_Date_Time_Data_t structure to valid.  This MACRO       */
   /* returns TRUE if the Date Time is valid or FALSE otherwise.        */
#define LNS_DATE_TIME_VALID(_x)                                ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following structure contains position quality data that is    */
   /* passed to the function that builds the Position quality packet.   */
   /* * NOTE * If                                                       */
   /*   LNS_POSITION_QUALITY_FLAG_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT */
   /*          Flag is set, then a valid value must be entered for      */
   /*          NumberofBeaconsinSolution.                               */
   /* * NOTE * If                                                       */
   /*   LNS_POSITION_QUALITY_FLAG_NUMBER_OF_BEACONS_IN_VIEW_PRESENT     */
   /*          Flag is set, then a valid value must be entered for      */
   /*          NumberofBeaconsinView.                                   */
   /* * NOTE * If LNS_POSITION_QUALITY_FLAG_TIME_TO_FIRST_FIX_PRESENT   */
   /*          Flag is set, then a valid value must be entered for      */
   /*          TimetoFirstfix.                                          */
   /* * NOTE * If LNS_POSITION_QUALITY_FLAG_EHPE_PRESENT Flag is set,   */
   /*          then a valid value must be entered for                   */
   /*          EstimatedHorizantalPositionError.                        */
   /* * NOTE * If LNS_POSITION_QUALITY_FLAG_EVPE_PRESENT Flag is set,   */
   /*          then a valid value must be entered for                   */
   /*          EstimatedVerticalPositionError.                          */
   /* * NOTE * If LNS_POSITION_QUALITY_FLAG_HDOP_PRESENT Flag is set,   */
   /*          then a valid value must be entered for                   */
   /*          HorizontalDilutionofPrecision.                           */
   /* * NOTE * If LNS_POSITION_QUALITY_FLAG_VDOP_PRESENT Flag is set,   */
   /*          then a valid value must be entered for                   */
   /*          VerticalDilutionofPrecision.                             */
typedef struct _tagLNS_Position_Quality_Data_t
{
   Word_t  Flags;
   Byte_t  NumberofBeaconsinSolution;
   Byte_t  NumberofBeaconsinView;
   Word_t  TimetoFirstfix;
   DWord_t EstimatedHorizontalPositionError;
   DWord_t EstimatedVerticalPositionError;
   Byte_t  HorizontalDilutionofPrecision;
   Byte_t  VerticalDilutionofPrecision;
} LNS_Position_Quality_Data_t;

#define LNS_POSITION_QUALITY_DATA_SIZE                   (sizeof(LNS_Position_Quality_Data_t))

   /* The following enumerates the valid values that may be set as the  */
   /* value for the OpCode field of LN Control Point characteristic.    */
typedef enum
{
   lncSetCumulativeValue                     = LNS_LN_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE,
   lncMaskLocationSpeedCharactersticsContent = LNS_LN_CONTROL_POINT_MASK_LOCATION_SPEED_CHARACTERISTIC_CONTENT,
   lncNavigationControl                      = LNS_LN_CONTROL_POINT_NAVIGATION_CONTROL,
   lncRequestNumberOfRoutes                  = LNS_LN_CONTROL_POINT_REQUEST_NUMBER_OF_ROUTES,
   lncRequestNameOfRoute                     = LNS_LN_CONTROL_POINT_REQUEST_NAME_OF_ROUTE,
   lncSelectRoute                            = LNS_LN_CONTROL_POINT_SELECT_ROUTE,
   lncSetFixRate                             = LNS_LN_CONTROL_POINT_SET_FIX_RATE,
   lncSetElevation                           = LNS_LN_CONTROL_POINT_SET_ELEVATION
} LNS_LNCP_Command_Type_t;

   /* The following enumerates the valid values that may be set as the  */
   /* value for the Response Opcode field of Location and Navigation    */
   /* Operation Control Point characteristic.                           */
typedef enum
{
   lncSuccess            = LNS_LN_CONTROL_POINT_RESPONSE_CODE_SUCCESS,
   lncOpcodeNotSupported = LNS_LN_CONTROL_POINT_RESPONSE_OPCODE_NOT_SUPPORTED,
   lncInvalidParameter   = LNS_LN_CONTROL_POINT_RESPONSE_INVALID_PARAMETER,
   lncOperationFailed    = LNS_LN_CONTROL_POINT_RESPONSE_OPERATION_FAILED
} LNS_LNCP_Response_Value_t;

   /* The following structure defines the format of the LNS LN          */
   /* operations Control Point Command Request Data.  This structure is */
   /* passed as a parameter to LNS_Format_LN_Control_Point_Command API. */
typedef struct _tagLNS_LN_Control_Point_Format_Data_t
{
   LNS_LNCP_Command_Type_t CommandType;
   union
   {
      Byte_t  CumulativeValue[3];
      SWord_t LocationSpeedContentMaskFlags;
      Byte_t  NavigationControlCodesFlags;
      Word_t  NameOfRoute;
      Word_t  SelectRoute;
      Byte_t  DesiredFixRateSeconds;
      SByte_t ElevationValue[3];
   } CommandParameters;
} LNS_LN_Control_Point_Format_Data_t;

#define LNS_LN_CONTROL_POINT_FORMAT_DATA_SIZE            (sizeof(LNS_LN_Control_Point_Format_Data_t))

   /* The following defines the format of a LN Control Point Response   */
   /* Data.  This structure will hold the LNCP response data received   */
   /* from remote LNS Server.  The first member specifies the Response  */
   /* Code OpCode set by Remote LNS Server, value of ResponseCodeOpCode */
   /* is of the form LNS_LN_CONTROL_POINT_XXX.  The second member is the*/
   /* request OpCode requested by the LNS Client.  The third memeber is */
   /* the Response Code set by remote LNS Server, value of ResponseCode */
   /* is of the form LNS_LN_CONTROL_POINT_RESPONSE_XXX.  The last member*/
   /* represents the response parameter as per the request made by LNS  */
   /* Client The response parameter type should be selected based on the*/
   /* RequestOpCode value.                                              */
   /* * NOTE * NameOfRoute is a pointer to a NULL terminated UTF-8      */
   /*          string.                                                  */
typedef struct _tagLNS_LN_Control_Point_Response_Data_t
{
   Byte_t                     ResponseCodeOpCode;
   LNS_LNCP_Command_Type_t    RequestOpCode;
   LNS_LNCP_Response_Value_t  ResponseCode;
   union
   {
      Word_t                  NumberOfRoutes;
      char                   *NameOfRoute;
   } ResponseParameter;
} LNS_LN_Control_Point_Response_Data_t;

#define LNS_LN_CONTROL_POINT_RESPONSE_DATA_SIZE          (sizeof(LNS_LN_Control_Point_Response_Data_t))

   /* The following structure contains Navigation data that is passed to*/
   /* the function that builds the Navigation packet.                   */
   /* * NOTE * If LNS_NAVIGATION_FLAG_REMAINING_DISTANCE_PRESENT Flag is*/
   /*          set, then a valid value must be entered for              */
   /*          RemainingDistance.                                       */
   /* * NOTE * If                                                       */
   /*          LNS_NAVIGATION_FLAG_REMAINING_VERTICAL_DISTANCE_PRESENT  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          RemainingVerticalDistance.                               */
   /* * NOTE * If LNS_NAVIGATION_FLAG_ESTIMATED_TIME_OF_ARRIVAL_PRESENT */
   /*          Flag is set, then a valid value must be entered for      */
   /*          EstimatedTimeofArrival.                                  */
typedef struct _tagLNS_Navigation_Data_t
{
   Word_t               Flags;
   Word_t               Bearing;
   Word_t               Heading;
   Byte_t               RemainingDistance[3];
   SByte_t              RemainingVerticalDistance[3];
   LNS_Date_Time_Data_t EstimatedTimeofArrival;
} LNS_Navigation_Data_t;

#define LNS_NAVIGATION_DATA_SIZE                         (sizeof(LNS_Navigation_Data_t))

   /* The following define the valid Read Request types that a server   */
   /* may receive in a etLNS_Server_Read_Client_Configuration_Request or*/
   /* etLNS_Server_Client_Configuration_Update event.  This is also used*/
   /* by the LNS_Send_Notification to denote the characteristic value to*/
   /* notify.                                                           */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctLNSLocationAndSpeed,
   ctLNControlPoint,
   ctNavigation
} LNS_Characteristic_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* LNS Profile.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the LNS_Event_Data_t structure.                                   */
typedef enum
{
   etLNS_Read_Client_Configuration_Request,
   etLNS_Client_Configuration_Update,
   etLNS_LN_Control_Point_Command,
   etLNS_Confirmation_Data
} LNS_Event_Type_t;

   /* The following is dispatched to a LNS Server when a LNP Client     */
   /* is attempting to read the Client Configuration descriptor.  The   */
   /* ConnectionID, and RemoteDevice identifies the Client that is      */
   /* making the request.  The TransactionID specifies the TransactionID*/
   /* of the request, this can be used when responding to the request   */
   /* using the LNS_Client_Configuration_Read_Response() API function.  */
typedef struct _tagLNS_Read_Client_Configuration_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   LNS_Characteristic_Type_t ClientConfigurationType;
} LNS_Read_Client_Configuration_Data_t;

#define LNS_READ_CLIENT_CONFIGURATION_DATA_SIZE          (sizeof(LNS_Read_Client_Configuration_Data_t))

   /* The following is dispatched to a LNS Server when a LNP Client     */
   /* attempts to write to a Client Configuration descriptor.  The      */
   /* ConnectionID and RemoteDevice identify the Client that is making  */
   /* the update request.  The ClientConfiguration value specifies the  */
   /* new Client Configuration value.                                   */
typedef struct _tagLNS_Client_Configuration_Update_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   LNS_Characteristic_Type_t ClientConfigurationType;
   Word_t                    ClientConfiguration;
} LNS_Client_Configuration_Update_Data_t;

#define LNS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE        (sizeof(LNS_Client_Configuration_Update_Data_t))

   /* The following is dispatched to a LNS Server in response to the    */
   /* reception of request from a Client to write to the LN Control     */
   /* Point.                                                            */
typedef struct _tagLNS_LN_Control_Point_Command_Data_t
{
   unsigned int                       InstanceID;
   unsigned int                       ConnectionID;
   unsigned int                       TransactionID;
   GATT_Connection_Type_t             ConnectionType;
   BD_ADDR_t                          RemoteDevice;
   LNS_LN_Control_Point_Format_Data_t FormatData;
} LNS_LN_Control_Point_Command_Data_t;

#define LNS_LN_CONTROL_POINT_COMMAND_DATA_SIZE           (sizeof(LNS_LN_Control_Point_Command_Data_t))

   /* The following LNS Profile Event is dispatched to a LNS Server     */
   /* when a LNS Client has sent a confirmation to a previously sent    */
   /* confirmation.  The ConnectionID, ConnectionType, and RemoteDevice */
   /* specifiy the Client that is making the update.  The final         */
   /* parameter specifies the status of the Indication.                 */
   /* * NOTE * The Status member is set to one of the following values: */
   /*          GATT_CONFIRMATION_STATUS_SUCCESS                         */
   /*          GATT_CONFIRMATION_STATUS_TIMEOUT                         */
typedef struct _tagLNS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
} LNS_Confirmation_Data_t;

#define LNS_CONFIRMATION_DATA_SIZE                       (sizeof(LNS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all LNS Profile Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.The Event_Data_Size member contains the total size */
   /* of the data contained in this event.                              */
typedef struct _tagLNS_Event_Data_t
{
   LNS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      LNS_Read_Client_Configuration_Data_t   *LNS_Read_Client_Configuration_Data;
      LNS_Client_Configuration_Update_Data_t *LNS_Client_Configuration_Update_Data;
      LNS_LN_Control_Point_Command_Data_t    *LNS_LN_Control_Point_Command_Data;
      LNS_Confirmation_Data_t                *LNS_Confirmation_Data;
   } Event_Data;
} LNS_Event_Data_t;

#define LNS_EVENT_DATA_SIZE                              (sizeof(LNS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a LNS Profile Event Receive Data Callback.  This function will be */
   /* called whenever an LNS Profile Event occurs that is associated    */
   /* the specified Bluetooth Stack ID.  This function passes to the    */
   /* caller the Bluetooth Stack ID, the LNS Event Data that occurred   */
   /* and the LNS Profile Event Callback Parameter that was specified   */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the LNS Profile Event Data ONLY in the context of     */
   /* this callback.  If the caller requires the Data for a longer      */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be re-entrant).  It   */
   /* needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible(this argument holds anyway because another  */
   /* LNS Profile Event will not be processed while this function call  */
   /* is outstanding).                                                  */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving LNS Profile Event   */
   /*            Packets.  A Deadlock WILL occur because NO LNS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *LNS_Event_Callback_t)(unsigned int BluetoothStackID, LNS_Event_Data_t *LNS_Event_Data, unsigned long CallbackParameter);

   /* LNS Server API.                                                   */

   /* The following function is responsible for opening a LNS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered LNS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 LNS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI LNS_Initialize_Service(unsigned int BluetoothStackID, LNS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Initialize_Service_t)(unsigned int BluetoothStackID, LNS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a LNS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered LNS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 LNS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI LNS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, LNS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, LNS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened LNS Server.  The first parameter is the Bluetooth Stack    */
   /* ID on which to close the server.  The second parameter is the     */
   /* InstanceID that was returned from a successfull call to           */
   /* LNS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI LNS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is used to perform a suspend of the        */
   /* Bluetooth stack.  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that Bluetopia is to use to collapse it's state information into. */
   /* This function can be called with BufferSize and Buffer set to 0   */
   /* and NULL, respectively.  In this case this function will return   */
   /* the number of bytes that must be passed to this function in order */
   /* to successfully perform a suspend (or 0 if an error occurred, or  */
   /* this functionality is not supported).  If the BufferSize and      */
   /* Buffer parameters are NOT 0 and NULL, this function will attempt  */
   /* to perform a suspend of the stack.  In this case, this function   */
   /* will return the amount of memory that was used from the provided  */
   /* buffers for the suspend (or zero otherwise).                      */
BTPSAPI_DECLARATION unsigned long BTPSAPI LNS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_LNS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* LNS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to LNS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI LNS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

    /* The following function is responsible for querying the number of */
    /* attributes that are contained in the LNS Service that is         */
    /* registered with a call to either of the LNS_Initialize_XXX()     */
    /* functions.  This function returns the non-zero number of         */
    /* attributes that are contained in a LNS Server or zero on failure.*/
BTPSAPI_DECLARATION unsigned int BTPSAPI LNS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_LNS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a LNSRead */
   /* Client Configuration Request.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* LNS_Initialize_Server().  The third is the Transaction ID of the  */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI LNS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for setting the supported   */
   /* LN feature on the specified LNS Instance.  The first parameter is */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* LNS_Initialize_Server().  The final parameter is the supported    */
   /* features to set for the specified LNS Instance.  This function    */
   /* returns a zero if successful or a negative return error code if   */
   /* an error occurs.                                                  */
   /* * NOTE * The SupportedFeatures parameter is a bitmask made up of  */
   /*          bits of the form LNS_LN_FEATURE_FLAG_XXX.                */
BTPSAPI_DECLARATION int BTPSAPI LNS_Set_LN_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Set_LN_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);
#endif

   /* The following function is responsible for querying the LNS        */
   /* Feature on the specified LNS Instance.  The first parameter is    */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is InstanceID returned from a successful call to                  */
   /* LNS_Initialize_Server().  The final parameter is a pointer to     */
   /* return the LNS Feature for the specified LNS Instance.  This      */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI LNS_Query_LN_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Query_LN_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);
#endif

   /* The following function is responsible for sending a Location and  */
   /* Speed notification to a specified remote device.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to LNS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The final parameter is the Location and Speed Data strcuture that */
   /* contains all of the required and optional data for the            */
   /* notification.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI LNS_Notify_Location_And_Speed(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_Location_Speed_Data_t *LocationAndSpeedData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Notify_Location_And_Speed_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_Location_Speed_Data_t *LocationAndSpeedData);
#endif

   /* The following function is responsible for setting the Position    */
   /* Quality on the specified LNS Instance.  The first parameter is the*/
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* LNS_Initialize_Server().  The final parameter is the Position     */
   /* quality data Structure to be set for the specified LNS Instance.  */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The Flag value of Position_Quality parameter is a bitmask*/
   /*          made up of bits of the form                              */
   /*          LNS_POSITION_QUALITY_FLAG_XXX.                           */
BTPSAPI_DECLARATION int BTPSAPI LNS_Set_Position_Quality(unsigned int BluetoothStackID, unsigned int InstanceID, LNS_Position_Quality_Data_t *Position_Quality);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Set_Position_Quality_t)(unsigned int BluetoothStackID, unsigned int InstanceID, LNS_Position_Quality_Data_t *Position_Quality);
#endif

   /* The following function is responsible for querying the Position   */
   /* quality on the specified LNS Instance.  The first parameter is    */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is InstanceID returned from a successful call to                  */
   /* LNS_Initialize_Server().  The final parameter is a pointer to     */
   /* return the Position quality data structure for the specified LNS  */
   /* Instance.  This Fucntion returns a zero if successful or a        */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI LNS_Query_Position_Quality(unsigned int BluetoothStackID, unsigned int InstanceID, LNS_Position_Quality_Data_t *Position_Quality);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Query_Position_Quality_t)(unsigned int BluetoothStackID, unsigned int InstanceID, LNS_Position_Quality_Data_t *Position_Quality);
#endif

   /* The following function is responsible for sending a Navigation    */
   /* notification to a specified remote device.  The first parameter   */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* LNS_Initialize_Server().  The third parameter is the ConnectionID */
   /* of the remote device to send the notification.  The final         */
   /* parameter is the Navigation Data strcuture that contains all of   */
   /* the required and optional data for the notification.  This        */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI LNS_Notify_Navigation(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_Navigation_Data_t *Navigation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Notify_Navigation_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_Navigation_Data_t *Navigation);
#endif

   /* The following function is responsible to responding to a LN       */
   /* Control Point Command received from a remote device.  The first   */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the TransactionID that was received in the LN */
   /* Control Point event.  The final parameter is an error code that is*/
   /* used to determine if the Request is being accepted by the server  */
   /* or if an error response should be issued instead.  This function  */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero than an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject LN Control Point commands when the Server has not */
   /*          been configured properly for LNCP operation, or a LNCP   */
   /*          procedure with the Client is already in progress.        */
   /*          All other reasons should return ZERO for the ErrorCode   */
   /*          and then send LNCP Result indication to indicate any     */
   /*          other errors.                                            */
   /*          For Example: If the Operand in the Request is not        */
   /*          supported by the Server this API should be called with   */
   /*          ErrorCode set to ZERO and then the                       */
   /*          LNS_Indicate_LN_Control_Point_Result() should be called  */
   /*          with the ResponseCode set to                             */
   /*          LNS_LN_RESPONSE_CODE_OPERATOR_NOT_SUPPORTED.             */
BTPSAPI_DECLARATION int BTPSAPI LNS_LN_Control_Point_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_LN_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending a Specific      */
   /* Operation Control Point indication to a specified remote device.  */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to LNS_Initialize_Server().  The third parameter  */
   /* the ConnectionID of the remote device to send the indication to.  */
   /* The fourth parameter is the opcode of the request that this is    */
   /* responding to.  The last parameter is the response code.  This    */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * Only 1 LNCP Request indication may be outstanding per    */
   /*          LNS Instance.                                            */
BTPSAPI_DECLARATION int BTPSAPI LNS_Indicate_LN_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_LNCP_Command_Type_t RequestOpCode, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Indicate_LN_Control_Point_Result_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_LNCP_Command_Type_t RequestOpCode, Byte_t ResponseCode);
#endif

   /* The following function is responsible for Number of Routes        */
   /* indication to a specified remote device.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* LNS_Initialize_Server().  The third parameter is the ConnectionID */
   /* of the remote device to send the notification to.  The fourth     */
   /* parameter is the RequestOpCode.  The Fifth Parameter is the       */
   /* response code.  The last parameter is number of routes to be      */
   /* indicated.  This function returns a zero if successful or a       */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * Only 1 Number of Routes indication may be outstanding    */
   /*          per LNS Instance.                                        */
BTPSAPI_DECLARATION int BTPSAPI LNS_Indicate_Number_Of_Routes(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_LNCP_Command_Type_t RequestOpCode, Byte_t ResponseCode, Word_t NumberOfRoutes);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Indicate_Number_Of_Routes_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_LNCP_Command_Type_t RequestOpCode, Byte_t ResponseCode, Word_t NumberOfRoutes);
#endif

   /* The following function is responsible for Name of Route indication*/
   /* to a specified remote device.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* LNS_Initialize_Server().  The third parameter is the ConnectionID */
   /* of the remote device to send the notification to.  The Forth      */
   /* parameter is the request op code.  The Fifth parameter is the     */
   /* response code.  The last parameter is name of route to be         */
   /* indicated and this value should be (NULL character('\0')          */
   /* terminated string.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
   /* * NOTE * Only 1 Name of Route indication may be outstanding per   */
   /*          LNS Instance.                                            */
BTPSAPI_DECLARATION int BTPSAPI LNS_Indicate_Name_Of_Route(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_LNCP_Command_Type_t RequestOpCode, Byte_t ResponseCode, char *NameOfRoute);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Indicate_Name_Of_Route_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, LNS_LNCP_Command_Type_t RequestOpCode, Byte_t ResponseCode, char *NameOfRoute);
#endif

   /* LNS Client API.                                                   */

   /* The following function is responsible for formatting a LN Control */
   /* Point Command into a user specified buffer.  The first parameter  */
   /* is the input command to format.  The second parameter is size of  */
   /* the input LN Control Point Request Data.  The final parameter is  */
   /* the output that will contain data in Buffer after formatting.     */
   /* This function returns a zero if successful or a negative error    */
   /* code if an error occurs.                                          */
   /* * NOTE * The third parameter BufferLength is the size of input    */
   /*          request and the same will hold the size of output Buffer */
   /*          after formatting.                                        */
BTPSAPI_DECLARATION int BTPSAPI LNS_Format_LN_Control_Point_Command(LNS_LN_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Format_LN_Control_Point_Command_t)(LNS_LN_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote LNS Server interpreting it as a response code of LN */
   /* control point.  The first parameter is the length of the value    */
   /* returned by the remote LNS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote LNS Server.  The final */
   /* parameter is a pointer to store the parsed LN Control Point       */
   /* Response data value.  This function returns a zero if successful  */
   /* or a negative return error code if an error occurs.               */
   /* * NOTE * LNS_Free_LN_Control_Point_Response() should be called    */
   /*          after LNCPResponseData has been processed by the         */
   /*          application.  A memory leak may occur if this function is*/
   /*          not called.                                              */
BTPSAPI_DECLARATION int BTPSAPI LNS_Decode_LN_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, LNS_LN_Control_Point_Response_Data_t *LNCPResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Decode_SC_Control_Point_Response_t)(unsigned int ValueLength, Byte_t *Value, LNS_LN_Control_Point_Response_Data_t *LNCPResponseData);
#endif

   /* The following function is provided to allow a mechanism to free   */
   /* all resources that were allocated to parse a Raw Control Point    */
   /* Response into Parsed Control Point Response.  See the             */
   /* LNS_Decode_LN_Control_Point_Response() function for more          */
   /* information.                                                      */
BTPSAPI_DECLARATION void BTPSAPI LNS_Free_LN_Control_Point_Response(LNS_LN_Control_Point_Response_Data_t *LNCPResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_LNS_Free_LN_Control_Point_Response_t)(LNS_LN_Control_Point_Response_Data_t *LNCPResponseData);
#endif


   /* The following function is responsible for parsing a value received*/
   /* from a remote LNS Server interpreting it as a Location And Speed  */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote LNS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote LNS Server.  The final */
   /* parameter is a pointer to store the parsed Location And Speed     */
   /* value.  This function returns a zero if successful, a negative    */
   /* return error code if an error occurs, or a positive value         */
   /* indicating the required buffer size (ValueLength) needed to parse */
   /* the notification data.                                            */
   /* * NOTE * It is possible that the notification for this            */
   /*          characteristic may have only been partially received if  */
   /*          the MTU size is not large enough to contain the entire   */
   /*          characteristic.  If this function detects that the Value */
   /*          buffer passed in is not the proper size, then the        */
   /*          function will return a positive non-zero value that      */
   /*          indicates the expected size of the data field.  If this  */
   /*          occurs, then the application should store the temporary  */
   /*          buffer and concatenate the next Location and Speed       */
   /*          notifications until the correct buffer size is received. */
BTPSAPI_DECLARATION int BTPSAPI LNS_Decode_Location_And_Speed(unsigned int ValueLength, Byte_t *Value, LNS_Location_Speed_Data_t *LocationAndSpeedData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Decode_Location_And_Speed_t)(unsigned int ValueLength, Byte_t *Value, LNS_Location_Speed_Data_t *LocationAndSpeedData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote LNS Server interpreting it as a navigation          */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote LNS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote LNS Server.  The final */
   /* parameter is a pointer to store the parsed navigation value.  This*/
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI LNS_Decode_Navigation(unsigned int ValueLength, Byte_t *Value, LNS_Navigation_Data_t *NavigationData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Decode_Navigation_t)(unsigned int ValueLength, Byte_t *Value, LNS_Navigation_Data_t *NavigationData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote LNS Server interpreting it as a Position Quality    */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote LNS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote LNS Server.  The final */
   /* parameter is a pointer to store the parsed Position Quality value.*/
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI LNS_Decode_Position_Quality(unsigned int ValueLength, Byte_t *Value, LNS_Position_Quality_Data_t *PositionQualityData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LNS_Decode_Position_Quality_t)(unsigned int ValueLength, Byte_t *Value, LNS_Position_Quality_Data_t *PositionQualityData);
#endif

#endif

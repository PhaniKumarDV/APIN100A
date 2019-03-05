/*****< ctsapi.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CTSAPI - Stonestreet One Bluetooth Current Time Service (GATT             */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/25/12  A. Parashar    Initial creation.                               */
/*   11/29/12  T. Cook        Fixed naming conventions and comments.          */
/******************************************************************************/
#ifndef __CTSAPIH__
#define __CTSAPIH__

#include "SS1BTPS.h"         /* Bluetooth Stack API Prototypes/Constants.     */
#include "SS1BTGAT.h"        /* Bluetooth Stack GATT API Prototypes/Constants.*/
#include "CTSTypes.h"        /* Current Time Service Types/Constants.         */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define CTS_ERROR_INVALID_PARAMETER                      (-1000)
#define CTS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define CTS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define CTS_ERROR_SERVICE_ALREADY_REGISTERED             (-1003)
#define CTS_ERROR_INVALID_INSTANCE_ID                    (-1004)
#define CTS_ERROR_MALFORMATTED_DATA                      (-1005)
#define CTS_ERROR_UNKNOWN_ERROR                          (-1006)

   /* The following defines the values of the Flags parameter that is   */
   /* provided in the CTS_Initialize_Service_Flags().                   */
   /* * NOTE * Other CTS_Initialize_...  API's will implicitly register */
   /*          GATT for LE ONLY and will not allow the use of these     */
   /*          flags.                                                   */
#define CTS_FLAGS_SUPPORT_LE                          0x0001
#define CTS_FLAGS_SUPPORT_BR_EDR                      0x0002
#define CTS_FLAGS_SUPPORT_CURRENT_TIME_GATT_WRITE     0x0004
#define CTS_FLAGS_SUPPORT_LOCAL_TIME_GATT_WRITE       0x0008

#define CTS_DEFAULT_FEATURES_BIT_MASK                 (CTS_FLAGS_SUPPORT_LOCAL_TIME_GATT_WRITE | CTS_FLAGS_SUPPORT_CURRENT_TIME_GATT_WRITE | CTS_FLAGS_SUPPORT_BR_EDR | CTS_FLAGS_SUPPORT_LE)

   /* The following enumerated type represents all of the valid Months  */
   /* of the Year values that may be assigned in the Current Time.      */
typedef enum
{
   myUnknown   = CTS_MONTH_OF_YEAR_UNKNOWN,
   myJanuary   = CTS_MONTH_OF_YEAR_JANUARY,
   myFebruary  = CTS_MONTH_OF_YEAR_FEBRUARY,
   myMarch     = CTS_MONTH_OF_YEAR_MARCH,
   myApril     = CTS_MONTH_OF_YEAR_APRIL,
   myMay       = CTS_MONTH_OF_YEAR_MAY,
   myJune      = CTS_MONTH_OF_YEAR_JUNE,
   myJuly      = CTS_MONTH_OF_YEAR_JULY,
   myAugust    = CTS_MONTH_OF_YEAR_AUGUST,
   mySeptember = CTS_MONTH_OF_YEAR_SEPTEMBER,
   myOctober   = CTS_MONTH_OF_YEAR_OCTOBER,
   myNovember  = CTS_MONTH_OF_YEAR_NOVEMBER,
   myDecember  = CTS_MONTH_OF_YEAR_DECEMBER
} CTS_Month_Of_Year_Type_t;

   /* The following enumerated type represents all of the valid Day of  */
   /* the Week values that may be assigned in the Current Time.         */
typedef enum
{
   wdUnknown   = CTS_DAY_OF_WEEK_UNKNOWN,
   wdMonday    = CTS_DAY_OF_WEEK_MONDAY,
   wdTuesday   = CTS_DAY_OF_WEEK_TUESDAY,
   wdWednesday = CTS_DAY_OF_WEEK_WEDNESDAY,
   wdThursday  = CTS_DAY_OF_WEEK_THURSDAY,
   wdFriday    = CTS_DAY_OF_WEEK_FRIDAY,
   wdSaturday  = CTS_DAY_OF_WEEK_SATURDAY,
   wdSunday    = CTS_DAY_OF_WEEK_SUNDAY
} CTS_Week_Day_Type_t;

   /* The following enumerated type represents all of the valid Time    */
   /* Zone values that may be assigned in the Local Time Information.   */
typedef enum
{
   tzUTCMinus1200 = CTS_TIME_ZONE_UTC_OFFSET_MINUS_12_00,
   tzUTCMinus1100 = CTS_TIME_ZONE_UTC_OFFSET_MINUS_11_00,
   tzUTCMinus1000 = CTS_TIME_ZONE_UTC_OFFSET_MINUS_10_00,
   tzUTCMinus930  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_9_30,
   tzUTCMinus900  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_9_00,
   tzUTCMinus800  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_8_00,
   tzUTCMinus700  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_7_00,
   tzUTCMinus600  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_6_00,
   tzUTCMinus500  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_5_00,
   tzUTCMinus430  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_4_30,
   tzUTCMinus400  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_4_00,
   tzUTCMinus330  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_3_30,
   tzUTCMinus300  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_3_00,
   tzUTCMinus200  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_2_00,
   tzUTCMinus100  = CTS_TIME_ZONE_UTC_OFFSET_MINUS_1_00,
   tzUTCPlus000   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_0_00,
   tzUTCPlus100   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_1_00,
   tzUTCPlus200   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_2_00,
   tzUTCPlus300   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_3_00,
   tzUTCPlus330   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_3_30,
   tzUTCPlus400   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_4_00,
   tzUTCPlus430   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_4_30,
   tzUTCPlus500   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_5_00,
   tzUTCPlus530   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_5_30,
   tzUTCPlus545   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_5_45,
   tzUTCPlus600   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_6_00,
   tzUTCPlus630   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_6_30,
   tzUTCPlus700   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_7_00,
   tzUTCPlus800   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_8_00,
   tzUTCPlus845   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_8_45,
   tzUTCPlus900   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_9_00,
   tzUTCPlus930   = CTS_TIME_ZONE_UTC_OFFSET_PLUS_9_30,
   tzUTCPlus1000  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_10_00,
   tzUTCPlus1030  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_10_30,
   tzUTCPlus1100  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_11_00,
   tzUTCPlus1130  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_11_30,
   tzUTCPlus1200  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_12_00,
   tzUTCPlus1245  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_12_45,
   tzUTCPlus1300  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_13_00,
   tzUTCPlus1400  = CTS_TIME_ZONE_UTC_OFFSET_PLUS_14_00,
   tzUTCUnknown   = CTS_TIME_ZONE_UTC_OFFSET_UNKNOWN
} CTS_Time_Zone_Type_t;

   /* The following enumerated type represents all of the valid Daylight*/
   /* Savings Time (DST) Offset values that may be assigned in the Local*/
   /* Time Information.                                                 */
typedef enum
{
   doStandardTime           = CTS_DST_OFFSET_STANDARD_TIME,
   doHalfAnHourDaylightTime = CTS_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME,
   doDaylightTime           = CTS_DST_OFFSET_DAYLIGHT_TIME,
   doDoubleDaylightTime     = CTS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME,
   doUnknown                = CTS_DST_OFFSET_UNKNOWN
} CTS_DST_Offset_Type_t;

   /* The following enumerated type represents all of the valid Time    */
   /* Source values that may be assigned in the Reference Time          */
   /* Information.                                                      */
typedef enum
{
   tsUnknown             = CTS_TIME_SOURCE_UNKNOWN,
   tsNetworkTimeProtocol = CTS_TIME_SOURCE_NETWORK_TIME_PROTOCOL,
   tsGps                 = CTS_TIME_SOURCE_GPS,
   tsRadioTimeSignal     = CTS_TIME_SOURCE_RADIO_TIME_SIGNAL,
   tsManual              = CTS_TIME_SOURCE_MANUAL,
   tsAtomicClock         = CTS_TIME_SOURCE_ATOMIC_CLOCK,
   tsCellularNetwork     = CTS_TIME_SOURCE_CELLULAR_NETWORK
} CTS_Time_Source_Type_t;

   /* The following structure represents the format of a CTS Date/Time  */
   /* value.  This is used to represent the Date-Time which contains the*/
   /* Day/Month/Year and Hours:Minutes:Second data.                     */
typedef struct _tagCTS_Date_Time_Data_t
{
   Word_t                   Year;
   CTS_Month_Of_Year_Type_t Month;
   Byte_t                   Day;
   Byte_t                   Hours;
   Byte_t                   Minutes;
   Byte_t                   Seconds;
} CTS_Date_Time_Data_t;

#define CTS_DATE_TIME_DATA_SIZE                          (sizeof(CTS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified Date Time is valid.  The only parameter to this function*/
   /* is the CTS_Date_Time_Data_t structure to valid.  This MACRO       */
   /* returns TRUE if the Date Time is valid or FALSE otherwise.        */
#define CTS_DATE_TIME_VALID(_x)                          ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following structure represents the format of a CTS            */
   /* Day/Data/Time value.  This structure is used to represent the     */
   /* Date/Time and the Day of the Week (Sunday - Saturday).            */
typedef  struct _tagCTS_Day_Date_Time_Data_t
{
   CTS_Date_Time_Data_t Date_Time;
   CTS_Week_Day_Type_t  Day_Of_Week;
} CTS_Day_Date_Time_Data_t;

#define CTS_DAY_DATE_TIME_DATA_SIZE                      (sizeof(CTS_Day_Date_Time_Data_t))

   /* The following structure represents the format of a CTS Exact Time */
   /* value.  This structure is used to represent the Day/Date/Time and */
   /* the Fractions256 (1/256 of second) value.                         */
typedef  struct _tagCTS_Exact_Time_Data_t
{
   CTS_Day_Date_Time_Data_t Day_Date_Time;
   Byte_t                   Fractions256;
} CTS_Exact_Time_Data_t;

#define CTS_EXACT_TIME_DATA_SIZE                         (sizeof(CTS_Exact_Time_Data_t))

   /* The following structure represents the formation of a Current Time*/
   /* value.  This is used represent the value of the Current Time      */
   /* Characteristic.  The first member of this structure contains the  */
   /* Extact Time value.  The second member is a bit mask (that must be */
   /* made of CTS_CURRENT_TIME_ADJUST_REASON_XXX bits) that specifies   */
   /* the reason that the time was adjusted.                            */
typedef  struct _tagCTS_Current_Time_Data_t
{
   CTS_Exact_Time_Data_t Exact_Time;
   Byte_t                Adjust_Reason_Mask;
}  CTS_Current_Time_Data_t;

#define CTS_CURRENT_TIME_DATA_SIZE                       (sizeof(CTS_Current_Time_Data_t))

   /* The following structure represents the formation of a Local Time  */
   /* Information value.  This is used to represent the value of the    */
   /* Local Time Information Characteristic.  The first member to this  */
   /* structure contains the Local Time Zone.  The second member        */
   /* contains the current Daylight Savings Time offset.                */
typedef  struct _tagCTS_Local_Time_Information_Data_t
{
   CTS_Time_Zone_Type_t  Time_Zone;
   CTS_DST_Offset_Type_t Daylight_Saving_Time;
}  CTS_Local_Time_Information_Data_t;

#define CTS_LOCAL_TIME_INFORMATION_DATA_SIZE             (sizeof(CTS_Local_Time_Information_Data_t))

   /* The following structure represents the formation of a Reference   */
   /* Time Information value.  This is used to represent the value of   */
   /* the Reference Time Information Characteristic.  The first member  */
   /* to this structure contains the source of the time information.    */
   /* The second member contains the accuracy (drift) of the local time */
   /* information specified in units of 1/8 of a second.  The final two */
   /* member contain the days and hours since the information was       */
   /* updated.                                                          */
typedef  struct _tagCTS_Reference_Time_Information_Data_t
{
   CTS_Time_Source_Type_t Source;
   Byte_t                 Accuracy;
   Byte_t                 Days_Since_Update;
   Byte_t                 Hours_Since_Update;
}  CTS_Reference_Time_Information_Data_t;

#define CTS_REFERENCE_TIME_INFORMATION_DATA_SIZE         (sizeof(CTS_Reference_Time_Information_Data_t))

   /* The following define the valid Read Request types that a server   */
   /* may receive in a etCTS_Server_Read_Client_Configuration_Request   */
   /* or etCTS_Server_Client_Configuration_Update event.This is also    */
   /* used by the CTS_Send_Notification to denote the characteristic    */
   /* value to notify.                                                  */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctCurrentTime
} CTS_Characteristic_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* CTS Service.These are used to determine the type of each event    */
   /* generated, and to ensure the proper union element is accessed for */
   /* the CTS_Event_Data_t structure.                                   */
typedef enum
{
   etCTS_Server_Read_Client_Configuration_Request,
   etCTS_Server_Update_Client_Configuration_Request,
   etCTS_Server_Read_Current_Time_Request,
   etCTS_Server_Write_Current_Time_Request,
   etCTS_Server_Write_Local_Time_Information_Request,
   etCTS_Server_Read_Reference_Time_Information_Request
} CTS_Event_Type_t;

   /* The following CTS Service Event is dispatched to a CTS Server when*/
   /* a CTS Client is attempting to read a descriptor. The ConnectionID */
   /* ConnectionType, and RemoteDevice specifiy the Client that is      */
   /* making the request.The DescriptorType specifies the Descriptor    */
   /* that the Client is attempting to read.The TransactionID specifies */
   /* the TransactionID of the request, this can be used when responding*/
   /* to the request using the CTS_Client_Configuration_Read_Response() */
   /* API function.                                                     */
typedef struct _tagCTS_Read_Client_Configuration_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   CTS_Characteristic_Type_t ClientConfigurationType;
} CTS_Read_Client_Configuration_Data_t;

#define CTS_READ_CLIENT_CONFIGURATION_DATA_SIZE          (sizeof(CTS_Read_Client_Configuration_Data_t))

   /* The following CTS Service Event is dispatched to a CTS Server when*/
   /* a CTS Client has written a Client Configuration descriptor.  The  */
   /* ConnectionID, ConnectionType, and RemoteDevice specifiy the Client*/
   /* that is making the update.  The ClientConfigurationType specifies */
   /* the Descriptor that the Client is writing.  The final member is   */
   /* the new Client Configuration for the specified characteristic.    */
typedef struct _tagCTS_Client_Configuration_Update_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   CTS_Characteristic_Type_t ClientConfigurationType;
   Word_t                    ClientConfiguration;
} CTS_Client_Configuration_Update_Data_t;

#define CTS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE        (sizeof(CTS_Client_Configuration_Update_Data_t))

   /* The following CTS Service Event is dispatched to a CTS Server when*/
   /* a CTS Client sends a request to read the current time data.  The  */
   /* ConnectionID, ConnectionType, and RemoteDevice specifiy the Client*/
   /* that is making the Request.                                       */
typedef struct _tagCTS_Read_Current_Time_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   unsigned int                      TransactionID;
   GATT_Connection_Type_t            ConnectionType;
   BD_ADDR_t                         RemoteDevice;
} CTS_Read_Current_Time_Request_Data_t;

#define CTS_READ_CURRENT_TIME_REQUEST_DATA_SIZE          (sizeof(CTS_Read_Current_Time_Request_Data_t))

   /* The following CTS Service Event is dispatched to a CTS Server when*/
   /* a CTS Client sends a request to write the current time.  The      */
   /* ConnectionID, ConnectionType, and RemoteDevice specifiy the Client*/
   /* that is making the Request.                                       */
typedef struct _tagCTS_Write_Current_Time_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   unsigned int                      TransactionID;
   GATT_Connection_Type_t            ConnectionType;
   BD_ADDR_t                         RemoteDevice;
   CTS_Current_Time_Data_t           CurrentTime;
} CTS_Write_Current_Time_Request_Data_t;

#define CTS_WRITE_CURRENT_TIME_REQUEST_DATA_SIZE         (sizeof(CTS_Write_Current_Time_Request_Data_t))

   /* The following CTS Service Event is dispatched to a CTS Server when*/
   /* a CTS Client sends a request to write the local time.  The        */
   /* ConnectionID, ConnectionType, and RemoteDevice specifiy the Client*/
   /* that is making the Request.                                       */
typedef struct _tagCTS_Write_Local_Time_Information_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   unsigned int                      TransactionID;
   GATT_Connection_Type_t            ConnectionType;
   BD_ADDR_t                         RemoteDevice;
   CTS_Local_Time_Information_Data_t LocalTime;
} CTS_Write_Local_Time_Information_Request_Data_t;

#define CTS_WRITE_LOCAL_TIME_INFORMATION_REQUEST_DATA_SIZE (sizeof(CTS_Write_Local_Time_Information_Request_Data_t))

   /* The following CTS Service Event is dispatched to a CTS Server when*/
   /* a CTS Client sends request to read Reference Time Information     */
   /*data. The ConnectionID, ConnectionType, and RemoteDevice specifiy  */
   /* the Client that is making the Request.                            */
typedef struct _tagCTS_Read_Reference_Time_Information_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   unsigned int                      TransactionID;
   GATT_Connection_Type_t            ConnectionType;
   BD_ADDR_t                         RemoteDevice;
} CTS_Read_Reference_Time_Information_Request_Data_t;

#define CTS_READ_REFERENCE_TIME_INFORMATION_REQUEST_DATA_SIZE (sizeof(CTS_Read_Reference_Time_Information_Request_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all CTS Service Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagCTS_Event_Data_t
{
   CTS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      CTS_Read_Client_Configuration_Data_t               *CTS_Read_Client_Configuration_Data;
      CTS_Client_Configuration_Update_Data_t             *CTS_Client_Configuration_Update_Data;
      CTS_Read_Current_Time_Request_Data_t               *CTS_Read_Current_Time_Request_Data;
      CTS_Write_Current_Time_Request_Data_t              *CTS_Write_Current_Time_Request_Data;
      CTS_Write_Local_Time_Information_Request_Data_t    *CTS_Write_Local_Time_Information_Request_Data;
      CTS_Read_Reference_Time_Information_Request_Data_t *CTS_Read_Reference_Time_Information_Request_Data;
   } Event_Data;
} CTS_Event_Data_t;

#define CTS_EVENT_DATA_SIZE                              (sizeof(CTS_Event_Data_t))

   /* The following structure contains the Handles that will need to be */
   /* cached by a CTS client in order to only do service discovery once.*/
typedef struct _tagCTS_Client_Information_t
{
   Word_t Current_Time;
   Word_t Current_Time_Client_Configuration;
   Word_t Local_Time_Information;
   Word_t Reference_Time_Information;
} CTS_Client_Information_t;

#define CTS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(CTS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a CTS Server.                           */
typedef struct _tagCTS_Server_Information_t
{
   Word_t Current_Time_Client_Configuration;
} CTS_Server_Information_t;

#define CTS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(CTS_Server_Information_t))

   /* The following declared type represents the Prototype Function for */
   /* a CTS Service Event Receive Data Callback.  This function will be */
   /* called whenever an CTS Service Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the CTS Event Data that        */
   /* occurred and the CTS Service Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the CTS Service Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer This function is guaranteed NOT to be invoked */
   /* more than once simultaneously for the specified installed callback*/
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another CTS Service  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving CTS Service Event   */
   /*            Packets.  A Deadlock WILL occur because NO CTS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *CTS_Event_Callback_t)(unsigned int BluetoothStackID, CTS_Event_Data_t *CTS_Event_Data, unsigned long CallbackParameter);

   /* CTS Server API.                                                   */

   /* The following function is responsible for opening a CTS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered CTS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 CTS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * This function does not support CTS 1.1 and will not allow*/
   /*          the optional writing of the Current Time and Local Time  */
   /*          Information Characteristics.                             */
   /* * NOTE * This function does not support CTS 1.1 and will not allow*/
   /*          registering CTS for the BR/EDR transport.                */
   /* * NOTE * If CTS 1.1 is needed, use CTS_Initialize_Service_Flags().*/
BTPSAPI_DECLARATION int BTPSAPI CTS_Initialize_Service(unsigned int BluetoothStackID, CTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Initialize_Service_t)(unsigned int BluetoothStackID, CTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a CTS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered CTS        */
   /* service.  The final parameter is a pointer, that on input can be  */
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This can be used */
   /* to include the service registered by this call.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 CTS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * This function does not support CTS 1.1 and will not allow*/
   /*          the optional writing of the Current Time and Local Time  */
   /*          Information Characteristics.                             */
   /* * NOTE * This function does not support CTS 1.1 and will not allow*/
   /*          registering CTS for the BR/EDR transport.                */
   /* * NOTE * If CTS 1.1 is needed, use CTS_Initialize_Service_Flags().*/
BTPSAPI_DECLARATION int BTPSAPI CTS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, CTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, CTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for opening a CTS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the CTS Flags          */
   /* (CTS_FLAGS_XXX).  These flags MUST be used to register GATT for   */
   /* LE, BR/EDR, or both.  These flags may also be used to enable      */
   /* optional service features (CTS 1.1) needed by the application.    */
   /* The third parameter is the Callback function to call when an event*/
   /* occurs on this Server Port.  The fourth parameter is a            */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The fifth parameter is a      */
   /* pointer to store the GATT Service ID of the registered CTS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is an optional pointer, that on   */
   /* input can be used to control the location of the service in the   */
   /* GATT database, and on ouput to store the service handle range.    */
   /* This function returns the positive, non-zero, Instance ID or a    */
   /* negative error code.                                              */
   /* * NOTE * Only 1 CTS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * This function supports CTS 1.1 and will allow the        */
   /*          optional writing of the Current Time and Local Time      */
   /*          Information Characteristics if the                       */
   /*          CTS_FLAGS_SUPPORT_CURRENT_TIME_GATT_WRITE or             */
   /*          CTS_FLAGS_SUPPORT_LOCAL_TIME_GATT_WRITE are set.         */
   /* * NOTE * If the last parameter is excluded it MUST be set to NULL.*/
BTPSAPI_DECLARATION int BTPSAPI CTS_Initialize_Service_Flags(unsigned int BluetoothStackID, unsigned int Flags, CTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Initialize_Service_Flags_t)(unsigned int BluetoothStackID, unsigned int Flags, CTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened CTS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successful call to            */
   /* CTS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CTS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI CTS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_CTS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* CTS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to CTS_Suspend().  This     */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI CTS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the CTS Service that is          */
   /* registered with a call to CTS_Initialize_Service().  This function*/
   /* returns the non-zero number of attributes that are contained in a */
   /* CTS Server or zero on failure.                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI CTS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_CTS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a CTS Read*/
   /* Current Time Request.The first parameter is the Bluetooth Stack ID*/
   /* of the Bluetooth Device.The second parameter is the Transaction ID*/
   /* of the request. The final parameter contains the Current Time that*/
   /* send to the remote device.This function returns a zero if         */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CTS_Current_Time_Read_Request_Response(unsigned int BluetoothStackID, unsigned int TransactionID, CTS_Current_Time_Data_t *Current_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Current_Time_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, CTS_Current_Time_Data_t *Current_Time);
#endif

   /* The following function is responsible for responding to a CTS Read*/
   /* Current Time Request when an error occured. The first parameter is*/
   /* the Bluetooth Stack ID of the Bluetooth Device.The second         */
   /* parameter is the Transaction ID of the request.The final parameter*/
   /* contains the Error which occured during the Read operation. This  */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI CTS_Current_Time_Read_Request_Error_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Current_Time_Read_Request_Error_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for responding to a CTS     */
   /* Write Current Time Request.  The first parameter is the Bluetooth */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* Transaction ID of the request.  The final parameter contains the  */
   /* Error which occured during the Write operation.  If this parameter*/
   /* is non-zero then then error response will be sent.  This function */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI CTS_Current_Time_Write_Request_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Current_Time_Write_Request_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for setting the Local Time  */
   /* information on the specified CTS Instance.  The first parameter   */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CTS_Initialize_Server().  The final parameter is the Local Time   */
   /* information to set for the specified CTS Instance.  This function */
   /* returns a zero if successful or a negative return error code if   */
   /* an error occurs.                                                  */
BTPSAPI_DECLARATION int BTPSAPI CTS_Set_Local_Time_Information(unsigned int BluetoothStackID, unsigned int InstanceID, CTS_Local_Time_Information_Data_t *Local_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Set_Local_Time_Information_t)(unsigned int BluetoothStackID, unsigned int InstanceID, CTS_Local_Time_Information_Data_t *Local_Time);
#endif

   /* The following function is responsible for querying the Local time */
   /* Information on the specified CTS Instance.The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameteris the InstanceID returned from a successful call to     */
   /* CTS_Initialize_Server().The final parameter is a pointer to return*/
   /* the Local time information for the specified CTS Instance.  This  */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI CTS_Query_Local_Time_Information(unsigned int BluetoothStackID, unsigned int InstanceID, CTS_Local_Time_Information_Data_t *Local_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Query_Local_Time_Information_t)(unsigned int BluetoothStackID, unsigned int InstanceID, CTS_Local_Time_Information_Data_t *Local_Time);
#endif

   /* The following function is responsible for responding to a CTS     */
   /* Write Current Time Request.  The first parameter is the Bluetooth */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* Transaction ID of the request.  The final parameter contains the  */
   /* Error which occured during the Write operation.  If this parameter*/
   /* is non-zero then then error response will be sent.  This function */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI CTS_Local_Time_Information_Write_Request_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Local_Time_Information_Write_Request_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for responding to a CTS Read*/
   /* Reference Time Information Request.  The first parameter is the   */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the Transaction ID of the request.  The final parameter        */
   /* contains the Reference Time Information to send to the remote     */
   /* device.  This function returns a zero if successful or a negative */
   /* return error code if an error occurs.                             */
BTPSAPI_DECLARATION int BTPSAPI CTS_Reference_Time_Information_Read_Request_Response(unsigned int BluetoothStackID, unsigned int TransactionID, CTS_Reference_Time_Information_Data_t *Reference_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Reference_Time_Information_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, CTS_Reference_Time_Information_Data_t *Reference_Time);
#endif

   /* The following function is responsible for responding to a CTS Read*/
   /* Reference Time Information Request when an error occured.  The    */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the Transaction ID of the request.        */
   /* The final parameter contains the Error which occured during the   */
   /* Read operation.  This function returns a zero if successful or a  */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI CTS_Reference_Time_Information_Read_Request_Error_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Reference_Time_Information_Read_Request_Error_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for responding to a CTS Read*/
   /* Client Configuration Request.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* CTS_Initialize_Server().  The third is the Transaction ID of the  */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CTS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t Client_Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t Client_Configuration);
#endif

   /* The following function is responsible for sending an Current      */
   /* Time notification to a specified remote device.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CTS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the notification to.    */
   /* This function returns a zero if successful or a negative          */
   /* return error code if an error occurs.                             */
BTPSAPI_DECLARATION int BTPSAPI CTS_Notify_Current_Time(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CTS_Current_Time_Data_t *Current_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Notify_Current_Time_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CTS_Current_Time_Data_t *Current_Time);
#endif

   /* CTS Client API.                                                   */

   /* The following function is responsible for formatting the Current  */
   /* Time into a buffer.  The first parameter is a pointer that        */
   /* contains the Current Time that will be formatted.  The second     */
   /* parameter is the length of the buffer that will hold the formatted*/
   /* data.  The final parameter is a pointer to the buffer that will   */
   /* contain the formatted data.  This function returns a zero if      */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CTS_Format_Current_Time(CTS_Current_Time_Data_t *Current_Time, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Format_Current_Time_t)(CTS_Current_Time_Data_t *Current_Time, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CTS Server interpreting it as a Current Time        */
   /* characteristic.  The first parameter is the length of             */
   /* the value returned by the remote CTS Server.  The second parameter*/
   /* is a pointer to the data returned by the remote CTS Server.  The  */
   /* final parameter is a pointer to store the parsed Current Time     */
   /* Measurement value.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
BTPSAPI_DECLARATION int BTPSAPI CTS_Decode_Current_Time(unsigned int ValueLength, Byte_t *Value, CTS_Current_Time_Data_t *Current_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Decode_Current_Time_t)(unsigned int ValueLength, Byte_t *Value, CTS_Current_Time_t *Current_Time);
#endif

   /* The following function is responsible for formatting the Local    */
   /* Time Information into a buffer.  The first parameter is a pointer */
   /* to the Local Time Information that will be formatted.  The second */
   /* parameter is the length of the buffer that will hold the Local    */
   /* Time Information.  The final parameter is a pointer to the buffer */
   /* that will contain the formatted data.  This function returns a    */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
BTPSAPI_DECLARATION int BTPSAPI CTS_Format_Local_Time_Information(CTS_Local_Time_Information_Data_t *Local_Time, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Format_Local_Time_Information_t)(CTS_Local_Time_Information_Data_t *Local_Time, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value         */
   /* received from a remote CTS Server interpreting it as a Local Time */
   /* information characteristic.  The first parameter is the length of */
   /* the value returned by the remote CTS Server.The second parameter  */
   /* is a pointer to the data returned by the remote CTS Server.The    */
   /* final parameter is a pointer to store the parsed Local Time       */
   /* information value.This function returns a zero if successful or a */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI CTS_Decode_Local_Time_Information(unsigned int ValueLength, Byte_t *Value, CTS_Local_Time_Information_Data_t *Local_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Decode_Local_Time_Information_t)(unsigned int ValueLength, Byte_t *Value, CTS_Local_Time_Information_Data_t *Local_Time);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CTS Server interpreting it as a Reference Time      */
   /* Information characteristic.  The first parameter is the length of */
   /* the value returned by the remote CTS Server.  The second parameter*/
   /* is a pointer to the data returned by the remote CTS Server.The    */
   /* final parameter is a pointer to store the parsed Reference Time   */
   /* Information value.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
BTPSAPI_DECLARATION int BTPSAPI CTS_Decode_Reference_Time_Information(unsigned int ValueLength, Byte_t *Value, CTS_Reference_Time_Information_Data_t *Reference_Time);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CTS_Decode_Reference_Time_Information_t)(unsigned int ValueLength, Byte_t *Value, CTS_Reference_Time_Information_Data_t *Reference_Time);
#endif

#endif

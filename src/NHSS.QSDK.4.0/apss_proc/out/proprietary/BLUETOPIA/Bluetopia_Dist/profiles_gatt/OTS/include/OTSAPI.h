/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< otsapi.h >*************************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OTSAPI - Qualcomm Technologies Bluetooth Object Transfer Service (GATT    */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __OTSAPIH__
#define __OTSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "OTSTypes.h"      /* Object Service Types/Constants.                 */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define OTS_ERROR_INVALID_PARAMETER                      (-1000)
#define OTS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define OTS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define OTS_ERROR_INSUFFICIENT_BUFFER_SPACE              (-1003)
#define OTS_ERROR_SERVICE_ALREADY_REGISTERED             (-1004)
#define OTS_ERROR_INVALID_INSTANCE_ID                    (-1005)
#define OTS_ERROR_MALFORMATTED_DATA                      (-1006)

   /* OTS API Specific error codes.                                     */
#define OTS_ERROR_INDICATION_OUTSTANDING                 (-1007)
#define OTS_ERROR_INVALID_CHARACTERISTIC_FLAG            (-1008)
#define OTS_ERROR_INVALID_ATTRIBUTE_HANDLE               (-1009)
#define OTS_ERROR_INVALID_CCCD_CHARACTERISTIC_TYPE       (-1010)
#define OTS_ERROR_INVALID_OBJECT_METADATA_TYPE           (-1011)
#define OTS_ERROR_INVALID_OACP_RESULT_CODE               (-1012)
#define OTS_ERROR_INVALID_OLCP_RESULT_CODE               (-1013)
#define OTS_ERROR_INVALID_OBJECT_LIST_FILTER_INSTANCE    (-1014)
#define OTS_ERROR_INVALID_OBJECT_LIST_FILTER_TYPE        (-1015)
#define OTS_ERROR_INVALID_OBJECT_TYPE_UUID_SIZE          (-1016)
#define OTS_ERROR_INVALID_WRITE_MODE_TYPE                (-1017)
#define OTS_ERROR_INVALID_LIST_SORT_ORDER_TYPE           (-1018)

   /* Object Transfer Channel (OTC) API specific error codes.           */
#define OTS_ERROR_NO_GATT_CONNECTION                     (-1019)
#define OTS_ERROR_NO_CONNECTION_INFORMATION              (-1020)
#define OTS_ERROR_INVALID_CONNECTION_TYPE                (-1021)
#define OTS_ERROR_INVALID_CONNECTION_STATE               (-1022)
#define OTS_ERROR_INVALID_CONNECTION_MODE                (-1023)

   /* The following structure contains all of the OTS Client information*/
   /* that will need stored by an OTS Client connected to an OTS Server.*/
   /* * NOTE * This information includes all the attribute handles for  */
   /*          OTS Characteristics found during service discovery.      */
typedef struct _tagOTS_Client_Information_t
{
   Word_t OTS_Feature;
   Word_t Object_Name;
   Word_t Object_Type;
   Word_t Object_Size;
   Word_t Object_First_Created;
   Word_t Object_Last_Modified;
   Word_t Object_ID;
   Word_t Object_Properties;
   Word_t Object_Action_Control_Point;
   Word_t Object_Action_Control_Point_CCCD;
   Word_t Object_List_Control_Point;
   Word_t Object_List_Control_Point_CCCD;
   Word_t Object_List_Filter[3];
   Word_t Object_Changed;
   Word_t Object_Changed_CCCD;
} OTS_Client_Information_t;

#define OTS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(OTS_Client_Information_t))

   /* The following structure contains all of the OTS Server information*/
   /* that will need to be stored for each OTS Client.                  */
typedef struct _tagOTS_Server_Information_t
{
   Word_t  Object_Action_Control_Point_Configuration;
   Word_t  Object_List_Control_Point_Configuration;
   Word_t  Object_Changed_Configuration;
} OTS_Server_Information_t;

#define OTS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(OTS_Server_Information_t))

   /* The following structure represents the format for the OTS Feature */
   /* Characteristic.                                                   */
   /* * NOTE * The OACP_Features and OLCP_Features fields specify the   */
   /*          global (affects ALL clients) supported features of the   */
   /*          OTS Server.                                              */
   /* * NOTE * The OACP_Features field is a bit mask that has the       */
   /*          following form OTS_FEATURE_OACP_XXX and can be found in  */
   /*          OTSTypes.h.                                              */
   /* * NOTE * The OLCP_Features field is a bit mask that has the       */
   /*          following form OTS_FEATURE_OLCP_XXX and can be found in  */
   /*          OTSTypes.h.                                              */
typedef struct _tagOTS_Feature_Data_t
{
   DWord_t OACP_Features;
   DWord_t OLCP_Features;
} OTS_Feature_Data_t;

#define OTS_FEATURE_DATA_SIZE                            (sizeof(OTS_Feature_Data_t))

   /* The following structure represents the format of the OTS Object   */
   /* Name Metadata Characteristic.                                     */
   /* * NOTE * The Buffer_Length field may NOT EXCEED the               */
   /*          OTS_MAXIMUM_OBJECT_NAME_LENGTH found in OTSTypes.h.      */
typedef struct _tagOTS_Name_Data_t
{
   Byte_t *Buffer;
   Byte_t  Buffer_Length;
} OTS_Name_Data_t;

#define OTS_OBJECT_NAME_DATA_SIZE                        (sizeof(OTS_Name_Data_t))

   /* The following structure represents the format for the OTS Object  */
   /* Size Metadata Characteristic.                                     */
   /* * NOTE * The Current_Size field represents the actual size of the */
   /*          OTS Object.  This field MUST be less than or equal to the*/
   /*          value of the Allocated_Size field.                       */
   /* * NOTE * The Allocated_Size field represents the number of octets */
   /*          that have been allocated by the OTS Server for the OTS   */
   /*          Object's contents.                                       */
typedef struct _tagOTS_Object_Size_Data_t
{
   DWord_t Current_Size;
   DWord_t Allocated_Size;
} OTS_Object_Size_Data_t;

#define OTS_OBJECT_SIZE_DATA_SIZE                        (sizeof(OTS_Object_Size_Data_t))

   /* The following structure represents the format for OTS Date/Time   */
   /* data type.                                                        */
   /* * NOTE * This data type will be used for the OTS First Created and*/
   /*          OTS Last Modified Object Metadata Characteristics.       */
   /* * NOTE * The Year field is defined by the Gregorian calendar and  */
   /*          MUST be between 1582-9999.                               */
   /* * NOTE * The Month field is defined by the Gregorian calendar and */
   /*          MUST be between 0-12.  A value of zero means unknown.    */
   /* * NOTE * The Day field is defined by the Gregorian calendar and   */
   /*          MUST be between 0-31.  A vlaue of zero means unknown.    */
   /* * NOTE * The Hours field is the number of hours past midnight and */
   /*          MUST be between 0-23.                                    */
   /* * NOTE * The Minutes field is the number of minutes since the     */
   /*          start of the hour and MUST be between 0-59.              */
   /* * NOTE * The Seconds field is the number of seconds since the     */
   /*          start of the minute and MUST be between 0-59.            */
typedef struct _tagOTS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} OTS_Date_Time_Data_t;

#define OTS_DATE_TIME_DATA_SIZE                          (sizeof(OTS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date/Time is valid.  The only parameter to this  */
   /* function is the OTS_Date_Time_Data_t structure to validate.  This */
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
   /* * NOTE * This MACRO may NOT be used if the OTS Date Time is       */
   /*          invalid.  If the OACP Create Procedure is used then the  */
   /*          OTS Date Time will be invalid (All fields zero).         */
#define OTS_DATE_TIME_VALID(_x)                          ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date/Time is invalid.  The only parameter to this*/
   /* function is the OTS_Date_Time_Data_t structure to validate.  This */
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
   /* * NOTE * This MACRO may be used to determine if an OTS Date Time  */
   /*          is invalid.  If the OACP Create Procedure is used then   */
   /*          the OTS Date Time will be invalid (All fields zero).     */
#define OTS_DATE_TIME_INVALID(_x)                        (((_x).Year == 0) && ((_x).Month == 0) && ((_x).Day == 0) && ((_x).Hours == 0) && ((_x).Minutes == 0) && ((_x).Seconds == 0))

   /* The following structure represents the format for the UINT48 data */
   /* type.                                                             */
   /* * NOTE * This data type will be used for the OTS Object ID        */
   /*          Metadata Characteristic.                                 */
   /* * NOTE * The Lower field represents the first least significant 32*/
   /*          bits and the Upper field represents the most significant */
   /*          16 bits of the 48 bit data type.                         */
typedef struct _tagOTS_UINT48_Data_t
{
   DWord_t Lower;
   Word_t  Upper;
} OTS_UINT48_Data_t;

#define OTS_UINT48_DATA_SIZE                             (sizeof(OTS_UINT48_Data_t))

   /* The following structure represents the format for an OTS Object.  */
   /* This structure contains the OTS Object Metadata Characteristics   */
   /* that are exposed for each OTS Object.                             */
   /* * NOTE * The OTS Directory Listing Object's (a specific OTS       */
   /*          Object) contents is comprised of OTS Object Records      */
   /*          (including one for itself), which includes OTS Object    */
   /*          Metadata about each OTS Object on the OTS Server.  Each  */
   /*          OTS Object Record will be handled internally by the      */
   /*          OTS_Format_Directory_Listing_Object_Contents() and       */
   /*          OTS_Decode_Directory_Listing_Object_Contents() functions.*/
   /*          Some Metadata included in the OTS Object Record is       */
   /*          optional and is controlled by the Flags field in this    */
   /*          structure.  It is included here to centralize what       */
   /*          optional fields of an OTS Object's Metadata will be      */
   /*          included for each OTS Object Record when the OTS         */
   /*          Directory Listing Object's contents are formatted.       */
   /* * NOTE * The Flags field is a bit mask that is used to control the*/
   /*          optional fields of an OTS Object's Metadata that will be */
   /*          included for each OTS Object Record.  Valid values have  */
   /*          the form OTS_OBJECT_RECORD_FLAGS_XXX, and can be found in*/
   /*          OTSTypes.h.                                              */
   /* * NOTE * The OTS Object for the OTS Directory Listing Object has  */
   /*          requirements for some of the fields included in this     */
   /*          structure.  See the notes below for information about    */
   /*          fields for the OTS Directory Listing Object.             */
   /* * NOTE * If this is the OTS Directory Listing Object, then the    */
   /*          Name field should be reflective of its purpose (ie.      */
   /*          "Directory").                                            */
   /* * NOTE * The Type field is for the OTS Object Type Metadata       */
   /*          Characteristic.  Types may only be defined by a 16-bit   */
   /*          UUID or a 128-bit UUID.  The Flags field MUST be set to  */
   /*          OTS_OBJECT_RECORD_FLAGS_TYPE_UUID_SIZE_128 for a 128-bit */
   /*          UUID.  Otherwise it should be excluded to indicate a     */
   /*          16-bit UUID.                                             */
   /* * NOTE * The Size field is optional for an the OTS Object Record  */
   /*          and may be included if specified by the Flags field.     */
   /*          This field MUST be updated for the OTS Directory Listing */
   /*          Object before an OTS Client may read the OTS Directory   */
   /*          Listing Object's contents via the Object Transfer Channel*/
   /*          (OTC).  The current size and allocated size should be the*/
   /*          same for the OTS Directory Listing Object and may be     */
   /*          determined via the                                       */
   /*          OTS_Format_Directory_Listing_Object_Contents() function. */
   /*          See this function for more information.                  */
   /* * NOTE * The First_Created and Last_Modified fields have no       */
   /*          meaning for the OTS Directory Listing Object and may be  */
   /*          defined by a higher level specification.  These fields   */
   /*          are optional for an OTS Object Record and may be included*/
   /*          if specified by the Flags field.                         */
   /* * NOTE * The ID Field is for the OTS Object ID Metadata           */
   /*          Characteristic.  If this is the OTS Directory Listing    */
   /*          Object then the ID field MUST be 0.  This field is       */
   /*          mandatory for an OTS Object Record.                      */
   /* * NOTE * The Properties field is for the OTS Object Properties    */
   /*          Metadata Characteristic.  If this is the OTS Directory   */
   /*          Listing Object, then the Properties field MUST specify   */
   /*          reading of the OTS Directory Listing Object.  The        */
   /*          Properties field MUST also not allow modification or     */
   /*          deletion of the OTS Directory Listing Object.  This field*/
   /*          is optional for the OTS Object Record and may be included*/
   /*          if specfied by the Flags field.                          */
typedef struct _tagOTS_Object_Data_t
{
   Byte_t                 Flags;
   OTS_Name_Data_t        Name;
   GATT_UUID_t            Type;
   OTS_Object_Size_Data_t Size;
   OTS_Date_Time_Data_t   First_Created;
   OTS_Date_Time_Data_t   Last_Modified;
   OTS_UINT48_Data_t      ID;
   DWord_t                Properties;
   Boolean_t              Marked;
} OTS_Object_Data_t;

#define OTS_OBJECT_DATA_SIZE                             (sizeof(OTS_Object_Data_t))

   /* The following enumeration defines the OACP Procedures that may be */
   /* set for the Request_Op_Code field of the OTS_OACP_Request_Data_t  */
   /* and OTS_OACP_Response_Data_t structures.                          */
typedef enum
{
   oacpCreate            = OTS_OACP_OP_CODE_CREATE,
   oacpDelete            = OTS_OACP_OP_CODE_DELETE,
   oacpCalculateChecksum = OTS_OACP_OP_CODE_CALCULATE_CHECKSUM,
   oacpExecute           = OTS_OACP_OP_CODE_EXECUTE,
   oacpRead              = OTS_OACP_OP_CODE_READ,
   oacpWrite             = OTS_OACP_OP_CODE_WRITE,
   oacpAbort             = OTS_OACP_OP_CODE_ABORT
} OTS_OACP_Request_Type_t;

   /* The following structure defines the parameters that are required  */
   /* for a Object Action Control Point Create procedure request.       */
typedef struct _tagOTS_OACP_Create_Request_Data_t
{
   DWord_t     Size;
   GATT_UUID_t UUID;
} OTS_OACP_Create_Request_Data_t;

   /* The following structure defines the parameters that are required  */
   /* for a Object Action Control Point Calculate Checksum procedure    */
   /* request.                                                          */
typedef struct _tagOTS_OACP_Calculate_Checksum_Request_Data_t
{
   DWord_t Offset;
   DWord_t Length;
} OTS_OACP_Calculate_Checksum_Request_Data_t;

   /* The following structure defines the parameters that are required  */
   /* for a Object Action Control Point Execute procedure request.      */
   /* * NOTE * This is defined by a higher level specification.  It is  */
   /*          the application's responsibility to make sure that data  */
   /*          in the buffer is formatted properly.                     */
typedef struct _tagOTS_OACP_Execute_Request_Data_t
{
   Word_t  Buffer_Length;
   Byte_t *Buffer;
} OTS_OACP_Execute_Request_Data_t;

   /* The following structure defines the parameters that are required  */
   /* for a Object Action Control Point Read procedure request.         */
   /* * NOTE * It is worth noting that the OTS Client MUST read the OTS */
   /*          Directory Listing Object's Size Metadata Characteristic  */
   /*          prior to issuing an OACP Read Procedure request to read  */
   /*          the OTS directory Listing Object's contents.  The OTS    */
   /*          Client MUST read the entire OTS Directory Listing        */
   /*          Object's contents.  Otherwise it CANNOT be decoded via   */
   /*          the OTS_Decode_Directory_Listing_Object_Contents()       */
   /*          function when received.  For the OACP Read Procedure, the*/
   /*          Offset MUST be set to zero and the Length MUST be set to */
   /*          to the received value in the read response for the OTS   */
   /*          Directory Listing Object's Size Metadata Characteristic, */
   /*          which represents the size of the OTS Directory Listing   */
   /*          Object's contents.                                       */
typedef struct _tagOTS_OACP_Read_Request_Data_t
{
   DWord_t Offset;
   DWord_t Length;
} OTS_OACP_Read_Request_Data_t;

   /* The following enumerattion defines the valid values that may be   */
   /* set for the Mode field of the OTS_OACP_Write_Request_Data_t       */
   /* structure.                                                        */
typedef enum
{
   wmtNone     = OTS_OACP_WRITE_MODE_NONE,
   wmtTruncate = OTS_OACP_WRITE_MODE_TRUNCATE
} OTS_Write_Mode_Type_t;

   /* The following structure defines the parameters that are required  */
   /* for a Object Action Control Point Write procedure request.        */
typedef struct _tagOTS_OACP_Write_Request_Data_t
{
   DWord_t               Offset;
   DWord_t               Length;
   OTS_Write_Mode_Type_t Mode;
} OTS_OACP_Write_Request_Data_t;

   /* The following structure represents the format for the OTS Object  */
   /* Action Control Point request.  The Request_Op_Code field of this  */
   /* structure specifies which Parameter field is valid.  Currently the*/
   /* following fields are valid for the following values of the        */
   /* Request_Op_Code field:                                            */
   /*                                                                   */
   /*  oacpCreate             - Create_Data is valid.                   */
   /*  oacpCalculateChecksum  - Calculate_Checksum_Data is valid.       */
   /*  oacpExecute            - Execute_Data is valid.                  */
   /*  oacpRead               - Read_Data is valid.                     */
   /*  oacpWrite              - Write_Data is valid.                    */
   /*                                                                   */
typedef struct _tagOTS_OACP_Request_Data_t
{
   OTS_OACP_Request_Type_t Request_Op_Code;
   union
   {
      OTS_OACP_Create_Request_Data_t             Create_Data;
      OTS_OACP_Calculate_Checksum_Request_Data_t Calculate_Checksum_Data;
      OTS_OACP_Execute_Request_Data_t            Execute_Data;
      OTS_OACP_Read_Request_Data_t               Read_Data;
      OTS_OACP_Write_Request_Data_t              Write_Data;
   } Parameter;
} OTS_OACP_Request_Data_t;

#define OTS_OACP_REQUEST_DATA_SIZE                       (sizeof(OTS_OACP_Request_Data_t))

   /* The following enumeration defines the OACP Result types that may  */
   /* be set for the Result_Code field of the OTS_OACP_Response_Data_t  */
   /* structure.                                                        */
typedef enum
{
   oarSuccess               = OTS_OACP_RESULT_CODE_SUCCESS,
   oarOpcodeNotSupported    = OTS_OACP_RESULT_CODE_OPCODE_NOT_SUPPORTED,
   oarInvalidParameter      = OTS_OACP_RESULT_CODE_INVALID_PARAMETER,
   oarInsufficientResources = OTS_OACP_RESULT_CODE_INSUFFICIENT_RESOURCES,
   oarInvalidObject         = OTS_OACP_RESULT_CODE_INVALID_OBJECT,
   oarChannelUnavailable    = OTS_OACP_RESULT_CODE_CHANNEL_UNAVAILABLE,
   oarUnsupportedType       = OTS_OACP_RESULT_CODE_UNSUPPORTED_TYPE,
   oarProcedureNotPermitted = OTS_OACP_RESULT_CODE_PROCEDURE_NOT_PERMITTED,
   oarObjectLocked          = OTS_OACP_RESULT_CODE_OBJECT_LOCKED,
   oarOperationFailed       = OTS_OACP_RESULT_CODE_OPERATION_FAILED
} OTS_OACP_Result_Type_t;

   /* The following structure defines the data that is required for a   */
   /* Object Action Control Point Execute procedure response.           */
   /* * NOTE * This is defined by a higher level specification.  It is  */
   /*          the application's responsibility to make sure that data  */
   /*          in the buffer is formatted properly.                     */
typedef struct _tagOTS_OACP_Execute_Response_Data_t
{
   Word_t  Buffer_Length;
   Byte_t *Buffer;
} OTS_OACP_Execute_Response_Data_t;

   /* The following structure represents the format for the OTS Object  */
   /* Action Control Point response.  The Request_Op_Code field of this */
   /* structure specifies which Parameter field is valid.  Currently the*/
   /* following fields are valid for the following values of the        */
   /* Request_Op_Code field:                                            */
   /*                                                                   */
   /*  oacpCalculateChecksum  - Checksum is valid.                      */
   /*  oacpExecute            - Execute_Data is valid.                  */
   /*                                                                   */
   /* * NOTE * The Parameter field is only VALID if the Result_Code is  */
   /*          oarSuccess.                                              */
typedef struct _tagOTS_OACP_Response_Data_t
{
   OTS_OACP_Request_Type_t Request_Op_Code;
   OTS_OACP_Result_Type_t  Result_Code;
   union
   {
      DWord_t                          Checksum;
      OTS_OACP_Execute_Response_Data_t Execute_Data;
   } Parameter;
} OTS_OACP_Response_Data_t;

#define OTS_OACP_RESPONSE_DATA_SIZE                      (sizeof(OTS_OACP_Response_Data_t))

   /* The following enumeratation defines the OLCP Procedures that may  */
   /* be set for the Request_Op_Code field of OTS_OLCP_Request_Data_t   */
   /* and OTS_OLCP_Response_Data_t structures.                          */
typedef enum
{
   olcpFirst                  = OTS_OLCP_OP_CODE_FIRST,
   olcpLast                   = OTS_OLCP_OP_CODE_LAST,
   olcpPrevious               = OTS_OLCP_OP_CODE_PREVIOUS,
   olcpNext                   = OTS_OLCP_OP_CODE_NEXT,
   olcpGoTo                   = OTS_OLCP_OP_CODE_GO_TO,
   olcpOrder                  = OTS_OLCP_OP_CODE_ORDER,
   olcpRequestNumberOfObjects = OTS_OLCP_OP_CODE_REQUEST_NUMBER_OF_OBJECTS,
   olcpClearMarking           = OTS_OLCP_OP_CODE_CLEAR_MARKING
} OTS_OLCP_Request_Type_t;

   /* The following enumeration defines the valid values that controls  */
   /* how the OTS Objects may be sorted on the OTS Server if the        */
   /* olcpOrder is set for the Request_Op_Code field of the             */
   /* OTS_OLCP_Request_Data_t structure.                                */
typedef enum
{
   lstOrderAscendingByName                = OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_NAME,
   lstOrderAscendingByObjectType          = OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_TYPE,
   lstOrderAscendingByObjectCurrentSize   = OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_CURRENT_SIZE,
   lstOrderAscendingByObjectFirstCreated  = OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_FIRST_CREATED,
   lstOrderAscendingByObjectLastModified  = OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_LAST_MODIFIED,
   lstOrderDescendingByName               = OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_NAME,
   lstOrderDescendingByObjectType         = OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_TYPE,
   lstOrderDescendingByObjectCurrentSize  = OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_CURRENT_SIZE,
   lstOrderDescendingByObjectFirstCreated = OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_FIRST_CREATED,
   lstOrderDescendingByObjectLastModified = OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_LAST_MODIFIED
} OTS_List_Sort_Order_Type_t;

   /* The following structure represents the format for the OTS Object  */
   /* List Control Point request.  The Request_Op_Code field of this    */
   /* structure specifies which Parameter field is valid.  Currently the*/
   /* following fields are valid for the following values of the        */
   /* Request_Op_Code field:                                            */
   /*                                                                   */
   /*  olcpGoTo   - Object_ID is valid.                                 */
   /*  olcpOrder  - List_Sort_Order is valid.                           */
   /*                                                                   */
typedef struct _tagOTS_OLCP_Request_Data_t
{
   OTS_OLCP_Request_Type_t Request_Op_Code;
   union
   {
      OTS_UINT48_Data_t          Object_ID;
      OTS_List_Sort_Order_Type_t List_Sort_Order;
   } Parameter;
} OTS_OLCP_Request_Data_t;

#define OTS_OLCP_REQUEST_DATA_SIZE                       (sizeof(OTS_OLCP_Request_Data_t))

   /* The following enumeration defines the OLCP Result types that may  */
   /* be set for the Result_Code field of the OTS_OLCP_Response_Data_t  */
   /* structure.                                                        */
typedef enum
{
   olrSuccess            = OTS_OLCP_RESULT_CODE_SUCCESS,
   olrOpcodeNotSupported = OTS_OLCP_RESULT_CODE_OPCODE_NOT_SUPPORTED,
   olrInvalidParameter   = OTS_OLCP_RESULT_CODE_INVALID_PARAMETER,
   olrOperationFailed    = OTS_OLCP_RESULT_CODE_OPERATION_FAILED,
   olrOutOfBounds        = OTS_OLCP_RESULT_CODE_OUT_OF_BOUNDS,
   olrTooManyObjects     = OTS_OLCP_RESULT_CODE_TOO_MANY_OBJECTS,
   olrNoObject           = OTS_OLCP_RESULT_CODE_NO_OBJECT,
   olrObjectIDNotFound   = OTS_OLCP_RESULT_CODE_OBJECT_ID_NOT_FOUND,
} OTS_OLCP_Result_Type_t;

   /* The following structure represents the format for the OTS Object  */
   /* List Control Point response.  The Request_Op_Code field of this   */
   /* structure specifies which Parameter field is valid.  Currently the*/
   /* following fields are valid for the following values of the        */
   /* Request_Op_Code field:                                            */
   /*                                                                   */
   /*  olcpRequestNumberOfObjects   - Total_Number_Of_Objects is valid. */
   /*                                                                   */
   /* * NOTE * The Parameter field is only VALID if the Result_Code is  */
   /*          olrSuccess.                                              */
typedef struct _tagOTS_OLCP_Response_Data_t
{
   OTS_OLCP_Request_Type_t Request_Op_Code;
   OTS_OLCP_Result_Type_t  Result_Code;
   union
   {
      DWord_t Total_Number_Of_Objects;
   } Parameter;
} OTS_OLCP_Response_Data_t;

#define OTS_OLCP_RESPONSE_DATA_SIZE                      (sizeof(OTS_OLCP_Response_Data_t))

   /* The following enumeration defines the OTS Object Metadata types.  */
   /* * NOTE * This enumeration is used to make sure the proper data    */
   /*          type is accessed for the OTS_Object_Metadata_Data_t union*/
   /*          below.                                                   */
typedef enum
{
   omtObjectName,
   omtObjectType,
   omtObjectSize,
   omtObjectFirstCreated,
   omtObjectLastModified,
   omtObjectID,
   omtObjectProperties
} OTS_Object_Metadata_Type_t;

   /* The following structure represents the OTS Object Metadata that   */
   /* may be received for an etOTS_Server_Write_Object_Metadata_Request */
   /* event or sent for an etOTS_Server_Read_Object_Metadata_Request    */
   /* event response.                                                   */
typedef union
{
   OTS_Name_Data_t        Name;
   GATT_UUID_t            Type;
   OTS_Object_Size_Data_t Size;
   OTS_Date_Time_Data_t   First_Created;
   OTS_Date_Time_Data_t   Last_Modified;
   OTS_UINT48_Data_t      ID;
   DWord_t                Properties;
} OTS_Object_Metadata_Data_t;

   /* The following enumeration defines the OTS Characteristic types    */
   /* that contain a Client Characteristic Configuration descriptor     */
   /* (CCCD).  This type will indicate the CCCD that has been requested */
   /* for the etOTS_Server_Read_CCCD_Request and                        */
   /* etOTS_Server_Write_CCCD_Request events.                           */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Characteristic Configuration   */
   /*          descriptor (CCCD) based on this value.                   */
typedef enum
{
   cctObjectActionControlPoint,
   cctObjectListControlPoint,
   cctObjectChanged,
   cctServiceChanged
} OTS_CCCD_Characteristic_Type_t;

   /* The following defines the Date/Time Range data type that may be   */
   /* used for the OTS_Object_List_Filter_Data_t structure.             */
typedef struct _tagOTS_Date_Time_Range_Data_t
{
   OTS_Date_Time_Data_t Minimum;
   OTS_Date_Time_Data_t Maximum;
} OTS_Date_Time_Range_Data_t;

#define OTS_DATE_TIME_RANGE_DATA_SIZE                    (sizeof(OTS_Date_Time_Range_Data_t))

   /* The following defines the Size Range data type that may be used   */
   /* for the OTS_Object_List_Filter_Data_t structure.                  */
typedef struct _tagOTS_Size_Range_Data_t
{
   DWord_t Minimum;
   DWord_t Maximum;
} OTS_Size_Range_Data_t;

#define OTS_SIZE_RANGE_DATA_SIZE                         (sizeof(OTS_Size_Range_Data_t))

   /* The following enumeration defines the Object List Filter instance.*/
   /* * NOTE * This enumeratation directly corresponds to the           */
   /*          OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS, found in      */
   /*          OTSTypes.h, that MUST be present if the OTS Object List  */
   /*          Filter is supported by the OTS Server.                   */
typedef enum
{
   lfiOne,
   lfiTwo,
   lfiThree
} OTS_Object_List_Filter_Instance_t;

   /* The following enumeration defines the Object List Filter types    */
   /* that may be set for the Type field of the                         */
   /* OTS_Object_List_Filter_Data_t structure.                          */
typedef enum
{
   lftNoFilter             = OTS_OBJECT_LIST_FILTER_NO_FILTER,
   lftNameStartsWith       = OTS_OBJECT_LIST_FILTER_NAME_STARTS_WITH,
   lftNameEndsWith         = OTS_OBJECT_LIST_FILTER_NAME_ENDS_WITH,
   lftNameContains         = OTS_OBJECT_LIST_FILTER_NAME_CONTAINS,
   lftNameIsExactly        = OTS_OBJECT_LIST_FILTER_NAME_IS_EXACTLY,
   lftObjectType           = OTS_OBJECT_LIST_FILTER_OBJECT_TYPE,
   lftCreatedBetween       = OTS_OBJECT_LIST_FILTER_CREATED_BETWEEN,
   lftModifiedBetween      = OTS_OBJECT_LIST_FILTER_MODIFIED_BETWEEN,
   lftCurrentSizeBetween   = OTS_OBJECT_LIST_FILTER_CURRENT_SIZE_BETWEEN,
   lftAllocatedSizeBetween = OTS_OBJECT_LIST_FILTER_ALLOCATED_SIZE_BETWEEN,
   lftMarkedObjects        = OTS_OBJECT_LIST_FILTER_MARKED_OBJECTS,
} OTS_Object_List_Filter_Type_t;

   /* The following structure represents the format for the OTS Object  */
   /* List Filter Characteristic instance.  The Type field of this      */
   /* structure specifies which Data field is valid.  Currently the     */
   /* following fields are valid for the following values of the Type   */
   /* field:                                                            */
   /*                                                                   */
   /*    lftNameStartsWith        - Name is valid.                      */
   /*    lftNameEndsWith          - Name is valid.                      */
   /*    lftNameContains          - Name is valid.                      */
   /*    lftNameIsExactly         - Name is valid.                      */
   /*    lftObjectType            - Type is valid.                      */
   /*    lftCreatedBetween        - Time_Range is valid.                */
   /*    lftModifiedBetween       - Time_Range is valid.                */
   /*    lftAllocatedSizeBetween  - Size_Range is valid.                */
   /*    lftCurrentSizeBetween    - Size_Range is valid.                */
   /*                                                                   */
typedef struct _tagOTS_Object_List_Filter_Data_t
{
   OTS_Object_List_Filter_Type_t Type;
   union
   {
      OTS_Name_Data_t            Name;
      GATT_UUID_t                Type;
      OTS_Date_Time_Range_Data_t Time_Range;
      OTS_Size_Range_Data_t      Size_Range;
   } Data;
} OTS_Object_List_Filter_Data_t;

#define OTS_OBJECT_LIST_FILTER_DATA_SIZE                 (sizeof(OTS_List_Filter_Data_t))

   /* The following structure represents the format for the OTS Object  */
   /* Changed Characteristic.                                           */
   /* * NOTE * The Flags parameter is a bit mask that has the form      */
   /*          OTS_OBJECT_CHANGED_FLAGS_XXX and can be found in         */
   /*          OTSTypes.h.                                              */
typedef struct _tagOTS_Object_Changed_Data_t
{
   Byte_t             Flags;
   OTS_UINT48_Data_t  Object_ID;
} OTS_Object_Changed_Data_t;

#define OTS_OBJECT_CHANGED_DATA_SIZE                     (sizeof(OTS_Object_Changed_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* OTS Service.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the OTS_Event_Data_t structure.                                   */
typedef enum
{
   etOTS_Server_Read_OTS_Feature_Request,
   etOTS_Server_Read_Object_Metadata_Request,
   etOTS_Server_Write_Object_Metadata_Request,
   etOTS_Server_Write_OACP_Request,
   etOTS_Server_Write_OLCP_Request,
   etOTS_Server_Read_Object_List_Filter_Request,
   etOTS_Server_Write_Object_List_Filter_Request,
   etOTS_Server_Read_CCCD_Request,
   etOTS_Server_Write_CCCD_Request,
   etOTS_Server_Prepare_Write_Request,
   etOTS_Server_Confirmation_Data
} OTS_Event_Type_t;

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to read the OTS Feature     */
   /* Characteristic.  The InstanceID field is the identifier for the   */
   /* instance of OTS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the OTS Client and */
   /* OTS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the OTS Client that  */
   /* made the request.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Read_OTS_Feature_Request_Response() function to  */
   /*          send the response to the request.  This response MUST be */
   /*          sent if this event is received.                          */
typedef struct _tagOTS_Read_OTS_Feature_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
} OTS_Read_OTS_Feature_Request_Data_t;

#define OTS_READ_OTS_FEATURE_REQUEST_DATA_SIZE           (sizeof(OTS_Read_OTS_Feature_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to read one of the Current  */
   /* Object's Metadata Characteristics.  The InstanceID field is the   */
   /* identifier for the instance of OTS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* OTS Client and OTS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the OTS Client that  */
   /* made the request.  The Type field identifies specific Object      */
   /* Metadata Characteristic that has been requested.                  */
   /* * NOTE * The Offset field is used if the Type field is set to     */
   /*          omtObjectName and will be non-zero to indicate that the  */
   /*          OTS Client has requested to read the OTS Object Name from*/
   /*          a specified offset (A GATT Read Long Value request) since*/
   /*          the entire OTS Object Name could not fit in the previous */
   /*          GATT Read Response.  This Offset MUST be set for the     */
   /*          Offset field of the OTS_Name_Data_t structure before the */
   /*          response is sent.                                        */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Read_Object_Metadata_Request_Response() function */
   /*          to send the response to the request.  This response MUST */
   /*          be sent if this event is received.                       */
typedef struct _tagOTS_Read_Object_Metadata_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   OTS_Object_Metadata_Type_t Type;
   Byte_t                     Offset;
} OTS_Read_Object_Metadata_Request_Data_t;

#define OTS_READ_OBJECT_METADATA_REQUEST_DATA_SIZE       (sizeof(OTS_Read_Object_Metadata_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to write one of the Object's*/
   /* Metadata Characteristics.  The InstanceID field is the identifier */
   /* for the instance of OTS that received the request.  The           */
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* OTS Client and OTS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the OTS Client that  */
   /* made the request.  The Type field identifies the specific Object  */
   /* Metadata Characteristic that has been requested.  The Metadata    */
   /* field contains the decoded data received with the request for the */
   /* specified Object Metadata Characteristic.                         */
   /* * NOTE * Only the following Object Metadata types may be written  */
   /*          (if they are supported by the OTS Server):               */
   /*                                                                   */
   /*                  omtObjectName                                    */
   /*                  omtObjectFirstCreated                            */
   /*                  omtObjectLastModified                            */
   /*                  omtObjectProperties                              */
   /*                                                                   */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Write_Object_Metadata_Request_Response() function*/
   /*          to send the response to the request.  This response MUST */
   /*          be sent if this event is received.                       */
typedef struct _tagOTS_Write_Object_Metadata_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   OTS_Object_Metadata_Type_t Type;
   OTS_Object_Metadata_Data_t Metadata;
} OTS_Write_Object_Metadata_Request_Data_t;

#define OTS_WRITE_OBJECT_METADATA_REQUEST_DATA_SIZE      (sizeof(OTS_Write_Object_Metadata_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to write the Object Action  */
   /* Control Point (OACP) Characteristic.  The InstanceID field is the */
   /* identifier for the instance of OTS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* OTS Client and OTS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the OTS Client that  */
   /* made the request.  The RequestData field contains the decoded OACP*/
   /* request data.                                                     */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Write_OACP_Request_Response() function to send   */
   /*          the response to the request.  This response MUST be sent */
   /*          if this event is received.  This response only means that*/
   /*          the Object Action Control Request is valid, not that the */
   /*          Object Action Control Point request has completed.  An   */
   /*          indication with the OACP Response data MUST be sent to   */
   /*          inform the OTS Client that the OACP Procedure has        */
   /*          completed.                                               */
typedef struct _tagOTS_Write_OACP_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
   OTS_OACP_Request_Data_t RequestData;
} OTS_Write_OACP_Request_Data_t;

#define OTS_WRITE_OACP_REQUEST_DATA_SIZE                 (sizeof(OTS_Write_OACP_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to write the Object List    */
   /* Control Point (OLCP) Characteristic.  The InstanceID field is the */
   /* identifier for the instance of OTS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* OTS Client and OTS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TranslistID field identifies the GATT translist for the request.  */
   /* The RemoteDevice is the BD_ADDR of the OTS Client that made the   */
   /* request.  The RequestData field contains the decoded OLCP request */
   /* data.                                                             */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Write_OLCP_Request_Response() function to send   */
   /*          the response to the request.  This response MUST be sent */
   /*          if this event is received.  This response only means that*/
   /*          the Object List Control Request is valid, not that the   */
   /*          Object List Control Point request has completed.  An     */
   /*          indication with the OLCP Response data MUST be sent to   */
   /*          inform the OTS Client that the OLCP Procedure has        */
   /*          completed.                                               */
typedef struct _tagOTS_Write_OLCP_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
   OTS_OLCP_Request_Data_t RequestData;
} OTS_Write_OLCP_Request_Data_t;

#define OTS_WRITE_OLCP_REQUEST_DATA_SIZE                 (sizeof(OTS_Write_OLCP_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to read the Object List     */
   /* Filter Characteristic.  The InstanceID field is the identifier for*/
   /* the instance of OTS that received the request.  The ConnectionID  */
   /* is the identifier for the GATT connection between the OTS Client  */
   /* and OTS Server.  The ConnectionType field specifies the GATT      */
   /* connection type or transport being used for the request.  The     */
   /* TranslistID field identifies the GATT translist for the request.  */
   /* The RemoteDevice is the BD_ADDR of the OTS Client that made the   */
   /* request.  The Instance field identifies the Object List Filter    */
   /* instance that has been requested.                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Read_Object_List_Filter_Request_Response()       */
   /*          function to send the response to the request.  This      */
   /*          response MUST be sent if this event is received.         */
typedef struct _tagOTS_Read_Object_List_Filter_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   GATT_Connection_Type_t            ConnectionType;
   unsigned int                      TransactionID;
   BD_ADDR_t                         RemoteDevice;
   OTS_Object_List_Filter_Instance_t Instance;
   Byte_t                            Offset;
} OTS_Read_Object_List_Filter_Request_Data_t;

#define OTS_READ_OBJECT_LIST_FILTER_REQUEST_DATA_SIZE    (sizeof(OTS_Read_Object_List_Filter_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to write the Object List    */
   /* Filter Characteristic.  The InstanceID field is the identifier for*/
   /* the instance of OTS that received the request.  The ConnectionID  */
   /* is the identifier for the GATT connection between the OTS Client  */
   /* and OTS Server.  The ConnectionType field specifies the GATT      */
   /* connection type or transport being used for the request.  The     */
   /* TranslistID field identifies the GATT translist for the request.  */
   /* The RemoteDevice is the BD_ADDR of the OTS Client that made the   */
   /* request.  The Instance field identifies the Object List Filter    */
   /* instance that has been requested.  The ListFilterData field       */
   /* contains the decoded Object List Filter data for the specified    */
   /* instance.                                                         */
   /* * NOTE * The application is responsible for copying the Name data */
   /*          of the ListFilterData field when the event is received if*/
   /*          the Type field of the OTS_Object_List_Filter_Data_t      */
   /*          structure is set to:                                     */
   /*                                                                   */
   /*                  lftNameStartsWith                                */
   /*                  lftNameEndsWith                                  */
   /*                  lftNameContains                                  */
   /*                  lftNameIsExactly                                 */
   /*                                                                   */
   /*          Otherwise the data will be lost when the callback        */
   /*          returns.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Write_Object_List_Filter_Request_Response()      */
   /*          function to send the response to the request.  This      */
   /*          response MUST be sent if this event is received.         */
typedef struct _tagOTS_Write_Object_List_Filter_Request_Data_t
{
   unsigned int                      InstanceID;
   unsigned int                      ConnectionID;
   GATT_Connection_Type_t            ConnectionType;
   unsigned int                      TransactionID;
   BD_ADDR_t                         RemoteDevice;
   OTS_Object_List_Filter_Instance_t Instance;
   OTS_Object_List_Filter_Data_t     ListFilterData;
} OTS_Write_Object_List_Filter_Request_Data_t;

#define OTS_WRITE_OBJECT_LIST_FILTER_REQUEST_DATA_SIZE   (sizeof(OTS_Write_Object_List_Filter_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to read an OTS              */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD).  The InstanceID field is the identifier for the instance  */
   /* of OTS that received the request.  The ConnectionID is the        */
   /* identifier for the GATT connection between the OTS Client and OTS */
   /* Server.  The ConnectionType field specifies the GATT connection   */
   /* type or transport being used for the request.  The TranslistID    */
   /* field identifies the GATT translist for the request.  The         */
   /* RemoteDevice is the BD_ADDR of the OTS Client that made the       */
   /* request.  The Type field identifes the OTS Characteristic whose   */
   /* CCCD has been requested.                                          */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Read_CCCD_Request_Response() function to send the*/
   /*          response to the request.  This response MUST be sent if  */
   /*          this event is received.                                  */
typedef struct _tagOTS_Read_CCCD_Request_Data_t
{
   unsigned int                   InstanceID;
   unsigned int                   ConnectionID;
   GATT_Connection_Type_t         ConnectionType;
   unsigned int                   TransactionID;
   BD_ADDR_t                      RemoteDevice;
   OTS_CCCD_Characteristic_Type_t Type;
} OTS_Read_CCCD_Request_Data_t;

#define OTS_READ_CCCD_REQUEST_DATA_SIZE                  (sizeof(OTS_Read_CCCD_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to write an OTS             */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD).  The InstanceID field is the identifier for the instance  */
   /* of OTS that received the request.  The ConnectionID is the        */
   /* identifier for the GATT connection between the OTS Client and OTS */
   /* Server.  The ConnectionType field specifies the GATT connection   */
   /* type or transport being used for the request.  The TranslistID    */
   /* field identifies the GATT translist for the request.  The         */
   /* RemoteDevice is the BD_ADDR of the OTS Client that made the       */
   /* request.  The Type field identifes the OTS Characteristic whose   */
   /* CCCD has been requested.  The Configuration field contains the    */
   /* decoded Configuration value.  * NOTE** The service will validate  */
   /* the Configuration before this event is received.  The application */
   /* may simply store the new Configuration value.                     */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Write_CCCD_Request_Response() function to send   */
   /*          the response to the request.  This response MUST be sent */
   /*          if this event is received.                               */
typedef struct _tagOTS_Write_CCCD_Request_Data_t
{
   unsigned int                   InstanceID;
   unsigned int                   ConnectionID;
   GATT_Connection_Type_t         ConnectionType;
   unsigned int                   TransactionID;
   BD_ADDR_t                      RemoteDevice;
   OTS_CCCD_Characteristic_Type_t Type;
   Word_t                         Configuration;
} OTS_Write_CCCD_Request_Data_t;

#define OTS_WRITE_CCCD_REQUEST_DATA_SIZE                 (sizeof(OTS_Write_CCCD_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has made a request to prepare an OTS           */
   /* Characteristic.  The InstanceID field is the identifier for the   */
   /* instance of OTS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the OTS Client and */
   /* OTS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the OTS Client that  */
   /* made the request.                                                 */
   /* * NOTE * This event is primarily provided to reject a GATT Prepare*/
   /*          Write request for optional security reasons such as the  */
   /*          OTS Client has insufficient authentication,              */
   /*          authorization, or encryption.  We will not pass the      */
   /*          prepared data up to the application until the the GATT   */
   /*          Execute Write request has been received by the OTS       */
   /*          Server, and the prepared writes are not cancelled.  If   */
   /*          the prepared data is written, the appropriate OTS event  */
   /*          will be dispatched to the application.  Otherwise the    */
   /*          prepared data will be cleared.                           */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the OTS_Prepare_Write_Request_Response() function to send*/
   /*          the response to the request.  This response MUST be sent */
   /*          if this event is received.                               */
typedef struct _tagOTS_Prepare_Write_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
} OTS_Prepare_Write_Request_Data_t;

#define OTS_PREPARE_WRITE_REQUEST_DATA_SIZE              (sizeof(OTS_Prepare_Write_Request_Data_t))

   /* The following OTS Server event is dispatched to the OTS Server    */
   /* when an OTS Client has confirmed it has received an indication    */
   /* previously sent by the OTS Server.  The InstanceID field is the   */
   /* identifier for the instance of OTS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* OTS Client and OTS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TranslistID field identifies the GATT translist for the request.  */
   /* The RemoteDevice is the BD_ADDR of the OTS Client that made the   */
   /* request.  The Status field identifes the status of the oustanding */
   /* indication.  The BytesWritten field contains the length of data   */
   /* that was successfully indicated to the OTS Client.                */
   /* * NOTE * No response is sent for this event.                      */
typedef struct _tagOTS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
   Word_t                 BytesWritten;
} OTS_Confirmation_Data_t;

#define OTS_CONFIRMATION_DATA_SIZE                       (sizeof(OTS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all OTS Server Event Data.  This structure is received for*/
   /* each event generated.  The Event_Data_Type member is used to      */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagOTS_Event_Data_t
{
   OTS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      OTS_Read_CCCD_Request_Data_t                *OTS_Read_CCCD_Request_Data;
      OTS_Write_CCCD_Request_Data_t               *OTS_Write_CCCD_Request_Data;
      OTS_Read_OTS_Feature_Request_Data_t         *OTS_Read_OTS_Feature_Request_Data;
      OTS_Read_Object_Metadata_Request_Data_t     *OTS_Read_Object_Metadata_Request_Data;
      OTS_Write_Object_Metadata_Request_Data_t    *OTS_Write_Object_Metadata_Request_Data;
      OTS_Write_OACP_Request_Data_t               *OTS_Write_OACP_Request_Data;
      OTS_Write_OLCP_Request_Data_t               *OTS_Write_OLCP_Request_Data;
      OTS_Read_Object_List_Filter_Request_Data_t  *OTS_Read_Object_List_Filter_Request_Data;
      OTS_Write_Object_List_Filter_Request_Data_t *OTS_Write_Object_List_Filter_Request_Data;
      OTS_Prepare_Write_Request_Data_t            *OTS_Prepare_Write_Request_Data;
      OTS_Confirmation_Data_t                     *OTS_Confirmation_Data;
   } Event_Data;
} OTS_Event_Data_t;

#define OTS_EVENT_DATA_SIZE                              (sizeof(OTS_Event_Data_t))

   /* Object Transfer Channel (OTC) enumerations, constants, structures */
   /* and events.                                                       */

   /* The following enumerated type represents the supported connection */
   /* roles for the Object Transfer Channel (OTC).                      */
typedef enum _tagOTS_Channel_Connection_Role_t
{
   crClient,
   crServer
} OTS_Channel_Connection_Role_t;

   /* The following enumeration respresents the Object Transfer Channel */
   /* (OTC) connection types.                                           */
typedef enum _tagOTS_Channel_Connection_Type_t
{
   octLE,
   octBR_EDR
} OTS_Channel_Connection_Type_t;

   /* The following enumeratation represents the supported Object       */
   /* Transfer Channel (OTC) connection modes supported by an OTS       */
   /* Server.                                                           */
   /* * NOTE * The cmManualAccept allows the OTS Server to explicitly   */
   /*          authorize OTC connections from OTS Clients.              */
typedef enum
{
   ocmAutomaticAccept,
   ocmAutomaticReject,
   ocmManualAccept
} OTS_Channel_Connection_Mode_t;

   /* The following enumeration covers all the events generated for the */
   /* Object Transfer Channel (OTC).  These are used to determine the   */
   /* type of each event generated, and to ensure the proper union      */
   /* element is accessed for the OTS_Channel_Event_Data_t structure.   */
typedef enum _tagOTS_Channel_Event_Type_t
{
   etOTS_Channel_Open_Indication,
   etOTS_Channel_Open_Request_Indication,
   etOTS_Channel_Open_Confirmation,
   etOTS_Channel_Close_Indication,
   etOTS_Channel_Data_Indication,
   etOTS_Channel_Data_Error_Indication,
   etOTS_Channel_Buffer_Empty_Indication
} OTS_Channel_Event_Type_t;

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* the OTS Server when a connection request from an OTS Client has   */
   /* been accepted (Connection Mode is cmAutomatic).  This indicates   */
   /* that the OTC connection has been successfully established.  The   */
   /* RemoteDevice is the BD_ADDR of the OTS Client that sent the       */
   /* connection request.  The Role field identifies the role for the   */
   /* OTC connection.  The Type field identifies the OTC connection     */
   /* type.  The CID field is the channel identifier for the OTC        */
   /* Connection.  The MaxSDUSize field represents the maximum service  */
   /* data unit (SDU) that may be used for the OTC.  The Initial Credits*/
   /* field indicates the initial credits that the OTS Server has for   */
   /* the LE based credit mode.                                         */
   /* * NOTE * The OTS Server should store the MaxSDUSize, which is     */
   /*          represents the maximum amount of data that the OTS Server*/
   /*          may send over the OTC.  It is the application's          */
   /*          responsibility to make sure that this size is not        */
   /*          exceeded for the Data_Length parameter of the            */
   /*          OTS_Channel_Send_Data() function.                        */
   /* * NOTE * The OTS Server should store the CID, which is required   */
   /*          for the OTC functions.                                   */
   /* * NOTE * The Role field will ALWAYS be set to crServer for this   */
   /*          event.                                                   */
   /* * NOTE * The InitialCredits field is ONLY VALID if the Type field */
   /*          is set to octLE since only LE OTS Channel connections    */
   /*          support the LE based credit mode.                        */
typedef struct _tagOTS_Channel_Open_Indication_Data_t
{
   BD_ADDR_t                     RemoteDevice;
   OTS_Channel_Connection_Role_t Role;
   OTS_Channel_Connection_Type_t Type;
   Word_t                        CID;
   Word_t                        MaxSDUSize;
   Word_t                        InitialCredits;
} OTS_Channel_Open_Indication_Data_t;

#define OTS_CHANNEL_OPEN_INDICATION_DATA_SIZE            (sizeof(OTS_Channel_Open_Indication_Data_t))

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* the OTS Server when a connection request from an OTS Client has   */
   /* been recevied and needs to be manually accepted (Connection Mode  */
   /* is cmManualAccept).  If this request is accepted via the          */
   /* OTS_Channel_Open_Connection_Request_Response() function, then this*/
   /* indicates that the OTC connection has been successfully           */
   /* established.  The RemoteDevice is the BD_ADDR of the OTS Client   */
   /* that sent the connection request.  The Role field identifies the  */
   /* role for the OTC connection.  The Type field identifies the OTC   */
   /* connection type.  The CID field is the channel identifier for the */
   /* OTC Connection.                                                   */
   /* * NOTE * The OTS Server should store the CID, which is required   */
   /*          for the OTC functions.                                   */
   /* * NOTE * The Role field will ALWAYS be set to crServer for this   */
   /*          event.                                                   */
   /* * NOTE * The OTS_Channel_Open_Connection_Request_Response()       */
   /*          function MUST be called after received this event to send*/
   /*          the response for the OTC connection request.             */
   /* * NOTE * The MaxSDUSize and InitialCredits fields are ONLY VALID  */
   /*          if the Type field is set to octLE since LE OTS Channel   */
   /*          connections will not dispatch the                        */
   /*          etOTS_Channel_Open_Indication event if the connection    */
   /*          request is accepted.  If the Type field is set to        */
   /*          octBR_EDR, then these fields will be set in the          */
   /*          etOTS_Channel_Open_Indication event.                     */
typedef struct _tagOTS_Channel_Open_Request_Indication_Data_t
{
   BD_ADDR_t                     RemoteDevice;
   OTS_Channel_Connection_Role_t Role;
   OTS_Channel_Connection_Type_t Type;
   Word_t                        CID;
   Word_t                        MaxSDUSize;
   Word_t                        InitialCredits;
} OTS_Channel_Open_Request_Indication_Data_t;

#define OTS_CHANNEL_OPEN_REQUEST_INDICATION_DATA_SIZE    (sizeof(OTS_Channel_Open_Request_Indication_Data_t))

   /* The following constants represent the valid values that are       */
   /* possible to received for the Status field of the                  */
   /* etOTS_Channel_Open_Confirmation event.                            */
#define OTS_CHANNEL_OPEN_STATUS_SUCCESS                  (0x0000)
#define OTS_CHANNEL_OPEN_STATUS_CONNECTION_TIMEOUT       (0x0001)
#define OTS_CHANNEL_OPEN_STATUS_CONNECTION_REFUSED       (0x0002)
#define OTS_CHANNEL_OPEN_STATUS_UNKNOWN_ERROR            (0x0003)

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* the OTS Client when a connection response from an OTS Server has  */
   /* been received.  If the Status is OTS_CHANNEL_OPEN_STATUS_SUCCESS, */
   /* then the OTC connection has been successfully established.  The   */
   /* RemoteDevice is the BD_ADDR of the OTS Client that sent the       */
   /* connection request.  The Role field identifies the role for the   */
   /* OTC connection.  The Type field identifies the OTC connection     */
   /* type.  The CID field is the channel identifier for the OTC        */
   /* Connection.  The MaxSDUSize field represents the maximum service  */
   /* data unit (SDU) that may be used for the OTC.  The Initial Credits*/
   /* field indicates the initial credits that the OTS Server has for   */
   /* the LE based credit mode.                                         */
   /* * NOTE * The OTS Client should store the MaxSDUSize, which is     */
   /*          represents the maximum amount of data that the OTS Server*/
   /*          may send over the OTC.  It is the application's          */
   /*          responsibility to make sure that this size is not        */
   /*          exceeded for the Data_Length parameter of the            */
   /*          OTS_Channel_Send_Data() function.                        */
   /* * NOTE * The OTS Client should store the CID, which is required   */
   /*          for the OTC functions.                                   */
   /* * NOTE * The Role field will ALWAYS be set to crClient for this   */
   /*          event.                                                   */
   /* * NOTE * The InitialCredits field is ONLY VALID if the Type field */
   /*          is set to octLE since only LE OTS Channel connections    */
   /*          support the LE based credit mode.                        */
typedef struct _tagOTS_Channel_Open_Confirmation_Data_t
{
   BD_ADDR_t                     RemoteDevice;
   OTS_Channel_Connection_Role_t Role;
   OTS_Channel_Connection_Type_t Type;
   Word_t                        CID;
   Word_t                        Status;
   Word_t                        MaxSDUSize;
   Word_t                        InitialCredits;
} OTS_Channel_Open_Confirmation_Data_t;

#define OTS_CHANNEL_OPEN_CONFIRMATION_DATA_SIZE          (sizeof(OTS_Channel_Open_Confirmation_Data_t))

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* when a close indication is received from a remote device.  The    */
   /* RemoteDevice is the BD_ADDR of the remote device.  The Role field */
   /* identifies the role for the OTC connection.  The Type field       */
   /* identifies the OTC connection type.  The CID field is the channel */
   /* identifier for the OTC Connection.  The Reason field specifies the*/
   /* reason for the disconnection.                                     */
   /* * NOTE * Under normal circumstances, the OTS Server will receive  */
   /*          this event from the OTS Client (OTS Client normally      */
   /*          disconnects the OTC).  However, if an error occurs on the*/
   /*          OTS Server, the OTS Server may disconnect the channel.   */
typedef struct _tagOTS_Channel_Close_Indication_Data_t
{
   BD_ADDR_t                     RemoteDevice;
   OTS_Channel_Connection_Role_t Role;
   OTS_Channel_Connection_Type_t Type;
   Word_t                        CID;
   Byte_t                        Reason;
} OTS_Channel_Close_Indication_Data_t;

#define OTS_CHANNEL_CLOSE_INDICATION_DATA_SIZE           (sizeof(OTS_Channel_Close_Indication_Data_t))

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* when a data indication is received from a remote device.  The     */
   /* RemoteDevice is the BD_ADDR of the remote device.  The Role field */
   /* identifies the role for the OTC connection.  The Type field       */
   /* identifies the OTC connection type.  The CID field is the channel */
   /* identifier for the OTC Connection.  The DataLength field indicates*/
   /* the size of the Data field.  the Data field contains the data that*/
   /* has been received from the remote device.  The CreditsConsumed    */
   /* field indicates the number of credits that were consumed for the  */
   /* data indication.                                                  */
   /* * NOTE * The CreditsConsumed field is ONLY VALID if the Type field*/
   /*          is set to octLE since only LE Connections support the LE */
   /*          based credit mode.                                       */
typedef struct _tagOTS_Channel_Data_Indication_Data_t
{
   BD_ADDR_t                      RemoteDevice;
   OTS_Channel_Connection_Role_t  Role;
   OTS_Channel_Connection_Type_t  Type;
   Word_t                         CID;
   Word_t                         DataLength;
   Byte_t                        *Data;
   Word_t                         CreditsConsumed;
} OTS_Channel_Data_Indication_Data_t;

#define OTS_CHANNEL_DATA_INDICATION_DATA_SIZE            (sizeof(OTS_Channel_Data_Indication_Data_t))

   /* The following constants represent the possible values that may be */
   /* received for the Error field of the                               */
   /* OTS_Channel_Data_Error_Indication_t structure below.              */
#define OTS_DATA_ERROR_MTU_EXCEEDED                      (L2CAP_DATA_READ_STATUS_MTU_EXCEEDED)
#define OTS_DATA_ERROR_PDU_TIMEOUT                       (L2CAP_DATA_READ_STATUS_RECEIVE_TIMEOUT)
#define OTS_DATA_ERROR_PDU_LOST                          (L2CAP_DATA_READ_STATUS_LOST_PACKET_ERROR)
#define OTS_DATA_ERROR_INVALID_SDU_SIZE                  (L2CAP_DATA_READ_STATUS_SIZE_ERROR)

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* when an error has been detected in the reception of data from a   */
   /* remote device.  The RemoteDevice is the BD_ADDR of the remote     */
   /* device.  The Role field identifies the role for the OTC           */
   /* connection.  The Type field identifies the OTC connection type.   */
   /* The CID field is the channel identifier for the OTC Connection.   */
   /* The Error field indicates the error that has occured (See above   */
   /* for possible values).                                             */
   /* * NOTE * This event will only be received if the Type field is set*/
   /*          to octBR_EDR, since BR/EDR OTS Channel connections MUST  */
   /*          use the Enhanced Retransmission Mode (ERTM).             */
   /* * NOTE * If the OTS_DATA_ERROR_MTU_EXCEEDED or                    */
   /*          OTS_DATA_ERROR_INVALID_SDU_SIZE is set for the Error     */
   /*          field, then the local device SHOULD assume the transfer  */
   /*          has failed and disconnect the channel.  These SHOULD NOT */
   /*          be received since these error codes indicate an internal */
   /*          error.                                                   */
   /* * NOTE * Since this event is only for BR/EDR OTS Channel          */
   /*          connections using ERTM, the OTS_DATA_ERROR_PDU_TIMEOUT   */
   /*          and OTS_DATA_ERROR_PDU_LOST errors are only meant to     */
   /*          inform the application that a PDU has not been received. */
   /*          Error recovery will be attempted to recover the PDUs that*/
   /*          have not been received since ERTM is used, however it is */
   /*          worth noting that if the maximum number of transmit      */
   /*          attempts has been reached for the PDU that was not       */
   /*          received, then the OTS Channel will be disconnected.  The*/
   /*          maximum number of transmit attempts for the remote device*/
   /*          can be determined from the ModeInfo field of the         */
   /*          etOTS_Channel_Open_Indication and                        */
   /*          etOTS_Channel_Open_Confirmation events.                  */
typedef struct _tagOTS_Channel_Data_Error_Indication_t
{
   BD_ADDR_t                     RemoteDevice;
   OTS_Channel_Connection_Role_t Role;
   OTS_Channel_Connection_Type_t Type;
   Word_t                        CID;
   Word_t                        Error;
} OTS_Channel_Data_Error_Indication_t;

#define OTS_CHANNEL_DATA_ERROR_INDICATION_DATA_SIZE      (sizeof(OTS_Channel_Data_Error_Indication_t))

   /* The following Object Transfer Channel (OTC) event is dispatched to*/
   /* inform the local device that it may send more data to the remote  */
   /* device.  The RemoteDevice is the BD_ADDR of the remote device.    */
   /* The Role field identifies the role for the OTC connection.  The   */
   /* Type field identifies the OTC connection type.  The CID field is  */
   /* the channel identifier for the OTC Connection.                    */
typedef struct _tagOTS_Channel_Buffer_Empty_Indication_Data_t
{
   BD_ADDR_t                     RemoteDevice;
   OTS_Channel_Connection_Role_t Role;
   OTS_Channel_Connection_Type_t Type;
   Word_t                        CID;
} OTS_Channel_Buffer_Empty_Indication_Data_t;

#define OTS_CHANNEL_BUFFER_EMPTY_INDICATION_DATA_SIZE    (sizeof(OTS_Channel_Buffer_Empty_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all of the Object Transfer Channel (OTC) event data.  This*/
   /* structure is received for each event generated.  The              */
   /* Event_Data_Type me mber is used to determine the appropriate union*/
   /* member element to access the contained data.  The Event_Data_Size */
   /* member contains the total size of the data contained in this      */
   /* event.                                                            */
typedef struct _tagOTS_Channel_Event_Data_t
{
   OTS_Channel_Event_Type_t Event_Data_Type;
   Byte_t                   Event_Data_Size;
   union
   {
      OTS_Channel_Open_Indication_Data_t         *OTS_Channel_Open_Indication_Data;
      OTS_Channel_Open_Request_Indication_Data_t *OTS_Channel_Open_Request_Indication_Data;
      OTS_Channel_Open_Confirmation_Data_t       *OTS_Channel_Open_Confirmation_Data;
      OTS_Channel_Close_Indication_Data_t        *OTS_Channel_Close_Indication_Data;
      OTS_Channel_Data_Indication_Data_t         *OTS_Channel_Data_Indication_Data;
      OTS_Channel_Data_Error_Indication_t        *OTS_Channel_Data_Error_Indication_Data;
      OTS_Channel_Buffer_Empty_Indication_Data_t *OTS_Channel_Buffer_Empty_Indication_Data;
   } Event_Data;
} OTS_Channel_Event_Data_t;

#define OTS_CHANNEL_EVENT_DATA_SIZE                      (sizeof(OTS_Channel_Event_Data_t))

   /* The OTS and OTS Channel Event Callbacks.                          */

   /* The following declared type represents the Prototype Function for */
   /* an OTS Event Receive Data Callback.  This function will be called */
   /* whenever an OTS Event occurs that is associated with the specified*/
   /* Bluetooth Stack ID.  This function passes to the caller the       */
   /* Bluetooth Stack ID, the OTS Event Data that occurred and the OTS  */
   /* Event Callback Parameter that was specified when this Callback was*/
   /* installed.  The caller is free to use the contents of the OTS     */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have to be re-entrant).  It needs to be noted   */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another OTS Profile Event will*/
   /* not be processed while this function call is outstanding).        */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving OTS Profile Event   */
   /*            Packets.  A Deadlock WILL occur because NO OTS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *OTS_Event_Callback_t)(unsigned int BluetoothStackID, OTS_Event_Data_t *OTS_Event_Data, unsigned long CallbackParameter);

   /* The following declared type represents the Prototype Function for */
   /* a OTS Channel Event Receive Data Callback.  This function will be */
   /* called whenever an OTS Channel Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the OTC Event Data that        */
   /* occurred and the OTS Channel Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the OTS Channel Event Data ONLY in context of */
   /* this callback.  If the caller requires the Data for a longer      */
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
   /* possible (this argument holds anyway because another OTS Channel  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving OTS Channel Event   */
   /*            Packets.  A Deadlock WILL occur because NO OTC Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *OTS_Channel_Event_Callback_t)(unsigned int BluetoothStackID, OTS_Channel_Event_Data_t *OTS_Channel_Event_Data, unsigned long CallbackParameter);

   /* The OTS Server Initialization constants and structures.           */
   /* * NOTE * These will be used by the OTS_Initialize_Service() and   */
   /*          OTS_Initialize_Service_Handle_Range() functions to       */
   /*          configure the OTS Server.                                */

   /* The following defines the OTS Characteristic Flags, which is a bit*/
   /* mask that controls what optional OTS Characteristics are supported*/
   /* by the OTS Server.                                                */
   /* * NOTE * If the OTS Server is capable of storing multiple objects */
   /*          (The Multiple_Objects_Supported field of the             */
   /*          OTS_Initialize_Data_t structure MUST be TRUE), then the  */
   /*          OTS_CHARACTERISTIC_FLAGS_OBJECT_LIST_FILTER flag may be  */
   /*          specified.  Otherwise it MUST be excluded.               */
   /* * NOTE * If the OTS_CHARACTERISTIC_FLAGS_OBJECT_LIST_FILTER is    */
   /*          specified and valid, then three instances will be        */
   /*          included in the service.                                 */
#define OTS_CHARACTERISTIC_FLAGS_OBJECT_FIRST_CREATED    0x01
#define OTS_CHARACTERISTIC_FLAGS_OBJECT_LAST_MODIFIED    0x02
#define OTS_CHARACTERISTIC_FLAGS_OBJECT_LIST_FILTER      0x04
#define OTS_CHARACTERISTIC_FLAGS_OBJECT_CHANGED          0x08

   /* The following defines the OTS Characteristic Optional Property    */
   /* Flags, which is a bit mask that controls what optional OTS        */
   /* Characteristic properties are supported by the OTS Server.        */
   /* * NOTE * If the OACP Create Op Code is supported by the OTS Server*/
   /*          (The OACP_Create_Procedure_Supported field of the        */
   /*          OTS_Initialize_Data_t structure is TRUE), then the OTS   */
   /*          Object Name and OTS First Created (If the OTS First      */
   /*          Created is supported) will be writeable.  Otherwise the  */
   /*          following flags may be specified to make ths OTS Object  */
   /*          Name and OTS First Created writeable.                    */
#define OTS_PROPERTY_FLAGS_OBJECT_NAME_ENABLE_WRITE           0x01
#define OTS_PROPERTY_FLAGS_OBJECT_FIRST_CREATED_ENABLE_WRITE  0x02

   /* The following structure defines that data that is required to     */
   /* initialize the Object Transfer Service (a parameter for the       */
   /* OTS_Initialize_Service() and OTS_Initialize_Service_Handle_Range()*/
   /* functions).                                                       */
   /* * NOTE * The OTS_Characteristic_Flags field is a bit mask that    */
   /*          controls what optional OTS Characteristics are included  */
   /*          in the service.  These bit mask values have the form     */
   /*          OTS_CHARACTERISTIC_FLAGS_XXX can can be found above this */
   /*          structure.  Some flags for this field are dependent on   */
   /*          the Multiple_Objects_Supported field.                    */
   /* * NOTE * The OTS_Property_Flags field is a bit mask that controls */
   /*          the optional properties that may be included for         */
   /*          supported OTS Characteristics.  These bit mask values    */
   /*          have the form OTS_PROPERTY_FLAGS_XXX can can be found    */
   /*          above this structure.  This field is dependent on the    */
   /*          OACP_Create_Procedure_Supported field and is only valid  */
   /*          if the OACP_Create_Procedure_Supported is FALSE.         */
   /* * NOTE * If the OACP_Create_Procedure_Supported is TRUE, then the */
   /*          Object Name Characteristic and Object First Created will */
   /*          be writeable and MUST be supported by the OTS Server.    */
   /*          The Object First Created is an optional OTS              */
   /*          Characteristic and MUST be specified by the              */
   /*          OTS_Characteristic_Flags field to be writeable.  If      */
   /*          FALSE, the OTS_Property_Flags may be used to make the    */
   /*          Object Name and Object First-Created Characteristics     */
   /*          writeable.                                               */
   /* * NOTE * If the Multiple_Objects_Supported field is TRUE, then the*/
   /*          Object ID Characteristic and Object List Control Point   */
   /*          Characteristic will be included automatically.  Otherwise*/
   /*          they will not be supported by the service.               */
   /* * NOTE * If the Multiple_Objects_Supported field is TRUE, then the*/
   /*          Object List Filter Characteristic is optional and the    */
   /*          OTS_CHARACTERISTIC_FLAGS_OBJECT_LIST_FILTER may be       */
   /*          specified for the OTS_Characteristic_Flags field.        */
   /* * NOTE * If the Multiple_Objects_Supported field is FALSE, then   */
   /*          the Object ID, Object List Control Point, and Object List*/
   /*          Filter Characteristics will not be supported by the      */
   /*          service.                                                 */
   /* * NOTE * If the Multiple_Objects_Supported is FALSE, then the     */
   /*          Directory Listing Object SHALL NOT be supported.         */
   /* * NOTE * If the Real_Time_Clock_Supported field is TRUE, then the */
   /*          Object Last-Modified Characteristic will not be          */
   /*          writeable.  If FALSE, the Object-Last Modified           */
   /*          Characteristic will be writeable.                        */
   /* * NOTE * The Connection_Mode field will allow the application to  */
   /*          specify the default connection mode for the Object       */
   /*          Transfer Channel (OTC).  If the OTS Server does NOT      */
   /*          support the OACP Read/Write/Abort Procedures, then the   */
   /*          Connection_Mode field should be set to cmAutomaticReject.*/
   /*          Otherwise the Connection_Mode field may be set to        */
   /*          cmAutomaticAccept or cmManualAccept.  The cmManualAccept */
   /*          will allow the OTS Server to explicitly authorize        */
   /*          connection requests.  It is worth noting this may be     */
   /*          updated later for future connections via the             */
   /*          OTS_Channel_Set_Connection_Mode() function.              */
   /* * NOTE * The Default_LE_Channel_Parameters field contains the     */
   /*          default OTS Channel parameters to use for the OTS Server,*/
   /*          when a OTS Channel connection request is received from an*/
   /*          OTS Client over the LE transport.                        */
   /* * NOTE * The EventCallback field is the event callback that will  */
   /*          receive all dispatched OTS Channel (Object Transfer      */
   /*          Channel) events for the user-specified CallbackParameter */
   /*          field.                                                   */
typedef struct _tagOTS_Initialize_Data_t
{
   Byte_t                         OTS_Characteristic_Flags;
   Byte_t                         OTS_Property_Flags;
   Boolean_t                      OACP_Create_Procedure_Supported;
   Boolean_t                      Multiple_Objects_Supported;
   Boolean_t                      Real_Time_Clock_Supported;
   OTS_Channel_Connection_Mode_t  Connection_Mode;
   L2CA_LE_Channel_Parameters_t   Default_LE_Channel_Parameters;
   OTS_Channel_Event_Callback_t   EventCallback;
   unsigned long                  CallbackParameter;
} OTS_Initialize_Data_t;

#define OTS_INITIALIZE_DATA_SIZE                         (sizeof(OTS_Initialize_Data_t))

   /* OTS Server API.                                                   */

   /* The following function is responsible for opening a OTS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the OTS Service Flags  */
   /* (OTS_SERVICE_FLAGS_XXX) found in OTSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an OTS  */
   /* Client, for the specified transport.  This includes connection    */
   /* requests for the Object Transfer Channel (OTC).  The third        */
   /* parameter is a pointer to the data that is REQUIRED to configure  */
   /* the service.  The fourth parameter is the Callback function to    */
   /* call when an OTS Server event is dispatched.  The fifth parameter */
   /* is a user-defined callback parameter that will be passed to the   */
   /* OTS callback function with each event.  The final parameter is a  */
   /* pointer to store the GATT Service ID of the registered OTS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 OTS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client OTS requests will be dispatched to the        */
   /*          EventCallback function that is specified by the fourth   */
   /*          parameter to this function.                              */
   /* * NOTE * See the OTS_Initialize_Data_t structure above for more   */
   /*          information about the InitializeData parameter.  If this */
   /*          parameter is not configured correctly the service will   */
   /*          FAIL to register.                                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Initialize_Service(unsigned int BluetoothStackID, Byte_t ServiceFlags, OTS_Initialize_Data_t *InitializeData, OTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Initialize_Service_t)(unsigned int BluetoothStackID, Byte_t ServiceFlags, OTS_Initialize_Data_t *InitializeData, OTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a OTS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the OTS Service Flags  */
   /* (OTS_SERVICE_FLAGS_XXX) found in OTSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an OTS  */
   /* Client, for the specified transport.  This includes connection    */
   /* requests for the Object Transfer Channel (OTC).  The third        */
   /* parameter is a pointer to the data that is REQUIRED to configure  */
   /* the service.  The fourth parameter is the Callback function to    */
   /* call when an OTS Server event is dispatched.  The fifth parameter */
   /* is a user-defined callback parameter that will be passed to the   */
   /* OTS callback function with each event.  The sixth parameter is a  */
   /* pointer to store the GATT Service ID of the registered OTS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 OTS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client OTS requests will be dispatched to the        */
   /*          EventCallback function that is specified by the fourth   */
   /*          parameter to this function.                              */
   /* * NOTE * See the OTS_Initialize_Data_t structure above for more   */
   /*          information about the InitializeData parameter.  If this */
   /*          parameter is not configured correctly the service will   */
   /*          FAIL to register.                                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, Byte_t ServiceFlags, OTS_Initialize_Data_t *InitializeData, OTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, Byte_t ServiceFlags, OTS_Initialize_Data_t *InitializeData, OTS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previous OTS  */
   /* Server.  The first parameter is the Bluetooth Stack ID on which to*/
   /* close the server.  The second parameter is the InstanceID that was*/
   /* returned from a successful call to either of the                  */
   /* OTS_Initialize_XXX() functions.  This function returns a zero if  */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI OTS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI OTS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_OTS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* OTS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to OTS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the OTS Service that is          */
   /* registered with a call to either of the OTS_Initialize_XXX()      */
   /* API's.  This function returns the non-zero number of attributes   */
   /* that are contained in an OTS Server or zero on failure.           */
   /* * NOTE * This function may be used to determine the attribute     */
   /*          handle range for OTS so that the ServiceHandleRange      */
   /*          parameter of the OTS_Initialize_Service_Handle_Range()   */
   /*          can be configured to register OTS in a specified         */
   /*          attribute handle range in GATT.                          */
BTPSAPI_DECLARATION unsigned int BTPSAPI OTS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_OTS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD) read request received from an OTS Client.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to OTS_Initialize_XXX().  The third parameter is the GATT         */
   /* Transaction ID of the request.  The fourth parameter is an error  */
   /* code that is used to determine if the request has been accepted by*/
   /* the OTS Server or if an error response should be sent.  The fifth */
   /* parameter is OTS CCCD Characteristic type that identifies, which  */
   /* OTS Characteristic's CCCD has been requested to be read.  The     */
   /* final parameter is the current Client Characteristic Configuration*/
   /* value to send to the OTS Client if the request has been accepted. */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request is rejected, the Configuration parameter  */
   /*          will be IGNORED.                                         */
BTPSAPI_DECLARATION int BTPSAPI OTS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_CCCD_Characteristic_Type_t Type, Word_t Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_CCCD_Characteristic_Type_t Type, Word_t Configuration);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD) write request received from an OTS Client.  The first      */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to OTS_Initialize_XXX().  The third parameter is the GATT         */
   /* Transaction ID of the request.  The fourth parameter is an error  */
   /* code that is used to determine if the request has been accepted by*/
   /* the OTS Server or if an error response should be sent.  The final */
   /* parameter is OTS CCCD Characteristic type that identifies, which  */
   /* OTS Characteristic's CCCD has been requested to be written.  This */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_CCCD_Characteristic_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_CCCD_Characteristic_Type_t Type);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Feature Characteristic read request received from an OTS Client.  */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to OTS_Initialize_XXX().  The third parameter is  */
   /* the GATT Transaction ID of the request.  The fourth parameter is  */
   /* an error code that is used to determine if the request has been   */
   /* accepted by the OTS Server or if an error response should be sent.*/
   /* The final parameter is a pointer to the OTS Feature to send to the*/
   /* OTS Client if the request has been accepted.  This function       */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been rejected, the OTSFeature         */
   /*          parameter may be excluded (NULL).                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Read_OTS_Feature_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Feature_Data_t *OTSFeature);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Read_OTS_Feature_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Feature_Data_t *OTSFeature);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Object Metadata Characteristic read request received from an OTS  */
   /* Client.  The first parameter is the Bluetooth Stack ID of the     */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to OTS_Initialize_XXX().  The third        */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is an error code that is used to determine if the       */
   /* request has been accepted by the OTS Server or if an error        */
   /* response should be sent.  The fifth parameter identifies the      */
   /* Object Metadata Characteristic type.  The sixth parameter is a    */
   /* pointer to the Object Metadata Characteristic data to send to the */
   /* OTS Client if the request has been accepted.  The final parameter */
   /* is the Offset to use for a GATT Read Long Value response.  This   */
   /* value should be set to Offset value received with                 */
   /* etOTS_Server_Read_Object_Metadata_Request event.  This function   */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been rejected, the Metadata parameter */
   /*          may be excluded (NULL) and the Offset parameter will be  */
   /*          ignored.                                                 */
BTPSAPI_DECLARATION int BTPSAPI OTS_Read_Object_Metadata_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata, Byte_t Offset);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Read_Object_Metadata_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata, Byte_t Offset);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Object Metadata Characteristic write request received from an OTS */
   /* Client.  The first parameter is the Bluetooth Stack ID of the     */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to OTS_Initialize_XXX().  The third        */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is an error code that is used to determine if the       */
   /* request has been accepted by the OTS Server or if an error        */
   /* response should be sent.  The final parameter identifies the      */
   /* Object Metadata Characteristic type.  This function returns a zero*/
   /* if successful or a negative return error code if an error occurs. */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Write_Object_Metadata_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_Metadata_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Write_Object_Metadata_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_Metadata_Type_t Type);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Object Action Control Point (OACP) Characteristic write request   */
   /* received from an OTS Client.  The first parameter is the Bluetooth*/
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID that was returned from a successful call to            */
   /* OTS_Initialize_XXX().  The third parameter is the GATT            */
   /* TransactionID for the request.  The final parameter is an error   */
   /* code that is used to determine if the request has been accepted by*/
   /* the OTS Server or if an error response should be sent.  This      */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject the OACP write request when the OACP Client       */
   /*          Characteristic Configuration descriptor (CCCD) has not   */
   /*          been configured for indications, the OTS Client does not */
   /*          have proper Authentication, Authorization, or Encryption */
   /*          to write to the OACP, or an OACP request is already in   */
   /*          progress.  All other reasons should return               */
   /*          OTS_ERROR_CODE_SUCCESS for the ErrorCode and then call   */
   /*          the OTS_Indicate_OACP_Response() to indicate the response*/
   /*          once the OACP procedure has completed.                   */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Write_OACP_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Write_OACP_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for indicating the OTS      */
   /* Object Action Control Point (OACP) Characteristic response to an  */
   /* OTS Client.  The first parameter is the Bluetooth Stack ID of the */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to either of the OTS_Initialize_XXX()      */
   /* functions.  The third parameter is the GATT ConnectionID for the  */
   /* connection to the OTS Client.  The final parameter is a pointer to*/
   /* the OACP response data to indicate.  This function returns a      */
   /* positive non-zero value (The GATT TransactionID), which may be    */
   /* used to cancel the request.  Otherwise a negative return error    */
   /* code will be returned if an error occurs.                         */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the OACP CCCD has been configured for indications by the */
   /*          OTS Client this indication is intended for.  An          */
   /*          indication SHOULD NOT be sent if this is not the case.   */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * Only 1 indication may be outstanding per OTS Instance.   */
   /*          Otherwise the error code OTS_ERROR_INDICATION_OUTSTANDING*/
   /*          will be returned.                                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Indicate_OACP_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, OTS_OACP_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Indicate_OACP_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, OTS_OACP_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Object List Control Point (OLCP) Characteristic write request     */
   /* received from an OTS Client.  The first parameter is the Bluetooth*/
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID that was returned from a successful call to            */
   /* OTS_Initialize_XXX().  The third parameter is the GATT            */
   /* TransactionID for the request.  The final parameter is an error   */
   /* code that is used to determine if the request has been accepted by*/
   /* the OTS Server or if an error response should be sent.  This      */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject the OLCP write request when the OLCP Client       */
   /*          Characteristic Configuration descriptor (CCCD) has not   */
   /*          been configured for indications, the OTS Client does not */
   /*          have proper Authentication, Authorization, or Encryption */
   /*          to write to the OLCP, or an OLCP request is already in   */
   /*          progress.  All other reasons should return               */
   /*          OTS_ERROR_CODE_SUCCESS for the ErrorCode and then call   */
   /*          the OTS_Indicate_OLCP_Response() to indicate the response*/
   /*          once the OLCP procedure has completed.                   */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Write_OLCP_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Write_OLCP_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for indicating the OTS      */
   /* Object List Control Point (OLCP) Characteristic response to an OTS*/
   /* Client.  The first parameter is the Bluetooth Stack ID of the     */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to either of the OTS_Initialize_XXX()      */
   /* functions.  The third parameter is the GATT ConnectionID for the  */
   /* connection to the OTS Client.  The final parameter is a pointer to*/
   /* the OLCP response data to indicate.  This function returns a      */
   /* positive non-zero value (The GATT TransactionID), which may be    */
   /* used to cancel the request.  Otherwise a negative return error    */
   /* code will be returned if an error occurs.                         */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the OLCP CCCD has been configured for indications by the */
   /*          OTS Client this indication is intended for.  An          */
   /*          indication SHOULD NOT be sent if this is not the case.   */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * Only 1 indication may be outstanding per OTS Instance.   */
   /*          Otherwise the error code OTS_ERROR_INDICATION_OUTSTANDING*/
   /*          will be returned.                                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Indicate_OLCP_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, OTS_OLCP_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Indicate_OLCP_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, OTS_OLCP_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Object List Filter Characteristic read request received from an   */
   /* OTS Client.  The first parameter is the Bluetooth Stack ID of the */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to OTS_Initialize_XXX().  The third        */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is an error code that is used to determine if the       */
   /* request has been accepted by the OTS Server or if an error        */
   /* response should be sent.  The fifth parameter identifies the      */
   /* Object List Filter Instance.  The sixth parameter is a pointer to */
   /* the Object List Filter Characteristic data to send to the OTS     */
   /* Client if the request has been accepted.  The final parameter is  */
   /* the Offset to use for a GATT Read Long Value response.  This value*/
   /* should be set to Offset value received with                       */
   /* etOTS_Server_Read_Object_List_Filter_Request event.  This function*/
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been rejected, the ListFilterData     */
   /*          parameter may be excluded (NULL) and the Offset parameter*/
   /*          will be ignored.                                         */
BTPSAPI_DECLARATION int BTPSAPI OTS_Read_Object_List_Filter_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_List_Filter_Instance_t Instance, OTS_Object_List_Filter_Data_t *ListFilterData, Byte_t Offset);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Read_Object_List_Filter_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_List_Filter_Instance_t Instance, OTS_Object_List_Filter_Data_t *ListFilterData, Byte_t Offset);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Object List Filter Characteristic write request received from an  */
   /* OTS Client.  The first parameter is the Bluetooth Stack ID of the */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to OTS_Initialize_XXX().  The third        */
   /* parameter is the GATT Transaction ID of the request.  The fourth  */
   /* parameter is an error code that is used to determine if the       */
   /* request has been accepted by the OTS Server or if an error        */
   /* response should be sent.  The final parameter identifies the      */
   /* Object List Filter Instance.  This function returns a zero if     */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Write_Object_List_Filter_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_List_Filter_Instance_t Instance);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Write_Object_List_Filter_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, OTS_Object_List_Filter_Instance_t Instance);
#endif

   /* The following function is responsible for responding to an OTS    */
   /* Characteristic Prepare Write request received from an OTS Client. */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to OTS_Initialize_XXX().  The third parameter is  */
   /* the GATT Transaction ID of the request.  The final parameter is an*/
   /* error code that is used to determine if the request has been      */
   /* accepted by the OTS Server or if an error response should be sent.*/
   /* * NOTE * This function is primarily provided to reject a GATT     */
   /*          Prepare Write request for optional security reasons such */
   /*          as the OTS Client has insufficient authentication,       */
   /*          authorization, or encryption.  We will not pass the      */
   /*          prepared data up to the application until the the GATT   */
   /*          Execute Write request has been received by the OTS       */
   /*          Server, and the prepared writes are not cancelled.  If   */
   /*          the prepared data is written, the appropriate OTS event  */
   /*          will be dispatched to the application.  Otherwise the    */
   /*          prepared data will be cleared.                           */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Prepare_Write_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Prepare_Write_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for indicating the OTS      */
   /* Object Changed Characteristic to an OTS Client.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to either of the OTS_Initialize_XXX() functions.  The third       */
   /* parameter is the GATT ConnectionID for the connection to the OTS  */
   /* Client.  The final parameter is a pointer to the Object Changed   */
   /* data to indicate.  This function returns a positive non-zero value*/
   /* (The GATT TransactionID), which may be used to cancel the request.*/
   /* Otherwise a negative return error code will be returned if an     */
   /* error occurs.                                                     */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the Object Changed CCCD has been configured for          */
   /*          indications by the OTS Client this indication is intended*/
   /*          for.  An indication SHOULD NOT be sent if this is not the*/
   /*          case.                                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          OTS_ERROR_CODE_XXX from OTSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * Only 1 indication may be outstanding per OTS Instance.   */
   /*          Otherwise the error code OTS_ERROR_INDICATION_OUTSTANDING*/
   /*          will be returned.                                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Indicate_Object_Changed(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, OTS_Object_Changed_Data_t *ObjectChangedData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Indicate_Object_Changed_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, OTS_Object_Changed_Data_t *ObjectChangedData);
#endif

   /* The following function is responsible for calculating an ISO/IEC  */
   /* 3309 compliant 32-bit CRC for a specified number of octets.  The  */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the number of bytes from the Buffer       */
   /* parameter that the 32-bit CRC will be calculated over.  The third */
   /* parameter is the buffer containing the data that the CRC will     */
   /* calculated over.  The final parameter will hold the calculated    */
   /* checksum if this function is successful.  This function returns a */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
BTPSAPI_DECLARATION int BTPSAPI OTS_Calculate_CRC_32(unsigned int BluetoothStackID, DWord_t BufferLength, Byte_t *Buffer, DWord_t *Checksum);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Calculate_CRC_32_t)(unsigned int BluetoothStackID, DWord_t BufferLength, Byte_t *Buffer, DWord_t *Checksum);
#endif

   /* OTS Client API.                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote OTS Server interpreting it as the value for the OTS */
   /* Feature Characteristic.  The first parameter is the length of the */
   /* value returned by the remote OTS Server.  The second parameter is */
   /* a pointer to the data returned by the remote OTS Server.  The     */
   /* final parameter is a pointer to store the parsed OTS Feature if   */
   /* this function is successful.  This function returns a zero if     */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_OTS_Feature(unsigned int ValueLength, Byte_t *Value, OTS_Feature_Data_t *OTSFeature);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_OTS_Feature_t)(unsigned int ValueLength, Byte_t *Value, OTS_Feature_Data_t *OTSFeature);
#endif

   /* The following function is responsible for formatting an OTS Object*/
   /* Metadata Characteristic into a user specified buffer, for a GATT  */
   /* Write request, that will be sent to the OTS Server.  This function*/
   /* may also be used to determine the size of the buffer to hold the  */
   /* formatted data (see below).  The first parameter is the type of   */
   /* Object Metadata Characteristic that will be formatted into the    */
   /* buffer.  The second parameter is a pointer to the Object Metadata */
   /* that will be formatted into the buffer.  The third parameter is   */
   /* the size of buffer.  The final parameter is the buffer that will  */
   /* hold the formatted data if this function is successful.  This     */
   /* function returns zero if the Object Metadata has been successfully*/
   /* formatted into the buffer.  If this function is used to determine */
   /* the size of the buffer to hold the formatted data, then a positive*/
   /* non-zero value will be returned.  Otherwise this function will    */
   /* return a negative error code if an error occurs.                  */
   /* * NOTE * This function will NOT format the OTS Object Name.  The  */
   /*          OTS Object Name should already be formatted.             */
   /* * NOTE * If the OTS Object Metadata type is omtObjectType, then   */
   /*          this function expects that the 16 or 128 bit UUID has    */
   /*          already been formatted in Little-Endian.                 */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The OTS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI OTS_Format_Object_Metadata(OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Format_Object_Metadata_t)(OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata, Word_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote OTS Server interpreting it as the value for an OTS  */
   /* Object Metadata Characteristic.  The first parameter is the length*/
   /* of the value returned by the remote OTS Server.  The second       */
   /* parameter is a pointer to the data returned by the remote OTS     */
   /* Server.  The third parameter is the Object Metadata type that     */
   /* controls how the data is decoded.  The final parameter is a       */
   /* pointer to store the parsed Object Metadata if this function is   */
   /* successful.  This function returns a zero if successful or a      */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * If the OTS Object Metadata type is omtObjectType, then   */
   /*          this function will return the 16 or 128 bit UUID in      */
   /*          Little-Endian.                                           */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_Object_Metadata(unsigned int ValueLength, Byte_t *Value, OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_Object_Metadata_t)(unsigned int ValueLength, Byte_t *Value, OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata);
#endif

   /* The following function is responsible for formatting the OTS      */
   /* Object Action Control Point (OACP) Characteristic request data    */
   /* into a user specified buffer, for a GATT Write request, that will */
   /* be sent to the OTS Server.  This function may also be used to     */
   /* determine the size of the buffer to hold the formatted data (see  */
   /* below).  The first parameter is a pointer to the OACP request data*/
   /* that will be formatted into the buffer.  The second parameter is  */
   /* the size of buffer.  The final parameter is the buffer that will  */
   /* hold the formatted data if this function is successful.  This     */
   /* function returns zero if the OACP request data has been           */
   /* successfully formatted into the buffer.  If this function is used */
   /* to determine the size of the buffer to hold the formatted data,   */
   /* then a positive non-zero value will be returned.  Otherwise this  */
   /* function will return a negative error code if an error occurs.    */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The OTS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI OTS_Format_OACP_Request(OTS_OACP_Request_Data_t *RequestData, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Format_OACP_Request_t)(OTS_OACP_Request_Data_t *RequestData, Byte_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote OTS Server interpreting it as the value for the OTS */
   /* Object Action Control Point (OACP) response data.  The first      */
   /* parameter is the length of the value returned by the remote OTS   */
   /* Server.  The second parameter is a pointer to the data returned by*/
   /* the remote OTS Server.  The final parameter is a pointer to store */
   /* the parsed OACP response data if this function is successful.     */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_OACP_Response(unsigned int ValueLength, Byte_t *Value, OTS_OACP_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_OACP_Response_t)(unsigned int ValueLength, Byte_t *Value, OTS_OACP_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for formatting the OTS      */
   /* Object List Control Point (OLCP) Characteristic request data into */
   /* a user specified buffer, for a GATT Write request, that will be   */
   /* sent to the OTS Server.  This function may also be used to        */
   /* determine the size of the buffer to hold the formatted data (see  */
   /* below).  The first parameter is a pointer to the OLCP request data*/
   /* that will be formatted into the buffer.  The second parameter is  */
   /* the size of buffer.  The final parameter is the buffer that will  */
   /* hold the formatted data if this function is successful.  This     */
   /* function returns zero if the OLCP request data has been           */
   /* successfully formatted into the buffer.  If this function is used */
   /* to determine the size of the buffer to hold the formatted data,   */
   /* then a positive non-zero value will be returned.  Otherwise this  */
   /* function will return a negative error code if an error occurs.    */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The OTS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI OTS_Format_OLCP_Request(OTS_OLCP_Request_Data_t *RequestData, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Format_OLCP_Request_t)(OTS_OLCP_Request_Data_t *RequestData, Byte_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote OTS Server interpreting it as the value for the OTS */
   /* Object List Control Point (OLCP) response data.  The first        */
   /* parameter is the length of the value returned by the remote OTS   */
   /* Server.  The second parameter is a pointer to the data returned by*/
   /* the remote OTS Server.  The final parameter is a pointer to store */
   /* the parsed OLCP response data if this function is successful.     */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_OLCP_Response(unsigned int ValueLength, Byte_t *Value, OTS_OLCP_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_OLCP_Response_t)(unsigned int ValueLength, Byte_t *Value, OTS_OLCP_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for formatting the OTS      */
   /* Object List Filter Characteristic into a user specified buffer,   */
   /* for a GATT Write request, that will be sent to the OTS Server.    */
   /* This function may also be used to determine the size of the buffer*/
   /* to hold the formatted data (see below).  The first parameter is a */
   /* pointer to the Object List Filter data that will be formatted into*/
   /* the buffer.  The second parameter is the size of buffer.  The     */
   /* final parameter is the buffer that will hold the formatted data if*/
   /* this function is successful.  This function returns zero if the   */
   /* Object List Filter data has been successfully formatted into the  */
   /* buffer.  If this function is used to determine the size of the    */
   /* buffer to hold the formatted data, then a positive non-zero value */
   /* will be returned.  Otherwise this function will return a negative */
   /* error code if an error occurs.                                    */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The OTS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI OTS_Format_Object_List_Filter_Data(OTS_Object_List_Filter_Data_t *ListFilterData, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Format_Object_List_Filter_Data_t)(OTS_Object_List_Filter_Data_t *ListFilterData, Word_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote OTS Server interpreting it as the value for the OTS */
   /* Object List Filter Characteristic data.  The first parameter is   */
   /* the length of the value returned by the remote OTS Server.  The   */
   /* second parameter is a pointer to the data returned by the remote  */
   /* OTS Server.  The final parameter is a pointer to store the parsed */
   /* Object List Filter data if this function is successful.  This     */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_Object_List_Filter_Data(unsigned int ValueLength, Byte_t *Value, OTS_Object_List_Filter_Data_t *ListFilterData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_Object_List_Filter_Data_t)(unsigned int ValueLength, Byte_t *Value, OTS_Object_List_Filter_Data_t *ListFilterData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote OTS Server interpreting it as the value for the OTS */
   /* Object Changed Characteristic data.  The first parameter is the   */
   /* length of the value returned by the remote OTS Server.  The second*/
   /* parameter is a pointer to the data returned by the remote OTS     */
   /* Server.  The final parameter is a pointer to store the parsed     */
   /* Object Changed data if this function is successful.  This function*/
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_Object_Changed_Data(unsigned int ValueLength, Byte_t *Value, OTS_Object_Changed_Data_t *ObjectChangedData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_Object_Changed_Data_t)(unsigned int ValueLength, Byte_t *Value, OTS_Object_Changed_Data_t *ObjectChangedData);
#endif

   /* Object Transfer Channel (OTC) Server API.                         */

   /* The following function is responsible for responding to a request */
   /* from an OTS Client to open the Object Transfer Channel (OTC).     */
   /* This function accepts as input the Bluetooth Stack ID of the Local*/
   /* Bluetooth Protocol Stack.  The second parameter is the InstanceID */
   /* returned from a successful call to OTS_Initialize_XXX().  The     */
   /* third parameter is the OTS Channel Identifier (CID) for the       */
   /* currently pending OTC connection.  The final parameter is a       */
   /* boolean for whether to accept the pending connection request or   */
   /* reject it.  This function returns zero if successful or a negative*/
   /* return error code if an error occurs.                             */
   /* * NOTE * The OTS Server MUST have it's OTC Connection Mode set to */
   /*          cmManualAccept to use this function.  Otherwise OTC      */
   /*          connections will automatically be accepted/rejected by   */
   /*          the OTS Server.                                          */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Open_Connection_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t CID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Open_Connection_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t CID, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* query the Object Transfer Channel (OTC) Connection Mode for the   */
   /* OTS Server.  The first parameter is the Bluetooth Stack ID of the */
   /* Local Bluetooth Protocol Stack.  The second parameter is the      */
   /* InstanceID returned from a successful call to                     */
   /* OTS_Initialize_XXX().  The final parameter is a pointer to the OTS*/
   /* Server's OTC Connection Mode which will hold the OTS Connection   */
   /* Mode if this function is successful.  This function returns zero  */
   /* if successful, or a negative return value if there was an error.  */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Get_Connection_Mode(unsigned int BluetoothStackID, unsigned int InstanceID, OTS_Channel_Connection_Mode_t *ConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Get_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int InstanceID, OTS_Channel_Connection_Mode_t *ConnectionMode);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* set the Object Transfer Channel (OTC) Connection Mode for the OTS */
   /* Server.  The first parameter is the Bluetooth Stack ID of the     */
   /* Local Bluetooth Protocol Stack.  The second parameter is the      */
   /* InstanceID returned from a successful call to                     */
   /* OTS_Initialize_XXX().  The final parameter is the new Connection  */
   /* Mode to use for the OTS Server.  This function returns zero if    */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The new Connection Mode will only apply to future        */
   /*          connection requests receive by the OTS Server.           */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Set_Connection_Mode(unsigned int BluetoothStackID, unsigned int InstanceID, OTS_Channel_Connection_Mode_t ConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Set_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int InstanceID, OTS_Channel_Connection_Mode_t ConnectionMode);
#endif

   /* The following function is responsible for formatting the OTS      */
   /* Directory Listing Object's contents into a user specified buffer, */
   /* so that the OTS Server may send the OTS Directory Listing Object's*/
   /* contents over the Object Transfer Channel (OTC) to an OTS Client. */
   /* The OTS Directory Listing Object's contents is made up of OTS     */
   /* Object Records (handled internally).  This function will simply   */
   /* take each OTS Object stored on the OTS Server and format each one */
   /* into an OTS Object Record and then store it in the user specified */
   /* buffer.  This function may also be used to determine the size of  */
   /* the buffer needed to hold the OTS Directory Listing Object's      */
   /* contents (see below).  The first parameter is the number of OTS   */
   /* Objects that are contained in the ObjectData parameter.  The      */
   /* second parameter is a pointer to each OTS Object's data that will */
   /* be formatted as an OTS Object Record into the buffer.  The third  */
   /* parameter is the size of buffer.  The final parameter is the      */
   /* buffer that will hold the formatted data if this function is      */
   /* successful.  This function returns zero if the OTS Directory      */
   /* Listing Object's contents has been successfully formatted into the*/
   /* buffer.  If this function is used to determine the size of the    */
   /* buffer to hold the formatted data, then a positive non-zero value */
   /* will be returned.  Otherwise this function will return a negative */
   /* error code if an error occurs.                                    */
   /* * NOTE * This function MUST be called first to return the size of */
   /*          the buffer needed to hold the OTS Directory Listing      */
   /*          Object's contents.  The positive non-zero value returned */
   /*          by this function will represent the OTS Directory Listing*/
   /*          Object's Size Metadata Characteristic (Current size =    */
   /*          Allocated Size = returned value) that MUST be set and    */
   /*          exposed to the OTS Client before the OTS Directory       */
   /*          Listing Object's contents may be read via the OACP Read  */
   /*          Procedure.  This will allow the OTS Client to quickly    */
   /*          determine the expected size to receive for the OTS       */
   /*          Directory Listing Object's contents.  Otherwise the OTS  */
   /*          Client will not know the maximum size to read for the OTS*/
   /*          Directory Listing Object's contents.                     */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The OTS Server*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.  the formatted data.                 */
BTPSAPI_DECLARATION int BTPSAPI OTS_Format_Directory_Listing_Object_Contents(DWord_t NumberOfObjects, OTS_Object_Data_t *ObjectData, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Format_Directory_Listing_Object_Contents_t)(DWord_t NumberOfObjects, OTS_Object_Data_t *ObjectData, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* Object Transfer Channel (OTC) Client API.                         */

   /* The following function is responsible for requesting the creation */
   /* of an Object Transfer Channel (OTC) connection to an OTS Server.  */
   /* This function, accepts as input the Bluetooth Stack ID of the     */
   /* Bluetooth Stack to create the OTC connection.  The second         */
   /* parameter is the GATT Connection type for the current connection  */
   /* to the OTS Server.  The third parameter identifies the OTS role   */
   /* for the local device that will issue the OTC connection request   */
   /* (See note below).  The fourth parameter to this function is the   */
   /* Bluetooth address of the OTS Server that will receive the OTC     */
   /* connection request.  The fifth parameter is the Callback function */
   /* to call when an OTC event occurs.  The sixth parameter is a       */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each OTC event.  The final parameter       */
   /* contains the LE Channel parameters to use for the OTC connection  */
   /* (see below).  This function returns the positive, non-zero,       */
   /* channel identifier (CID) for the OTC connection if successful or a*/
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ChannelParameters is REQUIRED if the GATT Connection */
   /*          type (Type) is gctLE, Otherwise it may be excluded       */
   /*          (NULL).                                                  */
   /* * NOTE * Once a Connection is established it can only be closed   */
   /*          via a call to the OTS_Channel_Close_Connection()         */
   /*          function.  Under normal circumstances (An error has not  */
   /*          occured on the OTS Server), the OTS Client should close  */
   /*          the Object Transfer Channel once a procedure has         */
   /*          completed.                                               */
   /* * NOTE * A Positive return value does NOT mean that the OTC       */
   /*          connection has been established, only that the OTC       */
   /*          connection request has been successfully submitted.      */
   /* * NOTE * The GATT Connection to the OTS Server must already exist */
   /*          before calling this function.                            */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Connect_Request(unsigned int BluetoothStackID, GATT_Connection_Type_t Type, BD_ADDR_t BD_ADDR, OTS_Channel_Event_Callback_t EventCallback, unsigned long CallbackParameter, L2CA_LE_Channel_Parameters_t *ChannelParameters);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Connect_Request_t)(unsigned int BluetoothStackID, GATT_Connection_Type_t Type, BD_ADDR_t BD_ADDR, OTS_Channel_Event_Callback_t EventCallback, unsigned long CallbackParameter, L2CA_LE_Channel_Parameters_t *ChannelParameters);
#endif

   /* The following function is responsible for parsing a buffer        */
   /* received from the OTS Server, over the Object Transfer Channel    */
   /* (OTC), interpreting it as the OTS Directory Listing Object's      */
   /* contents.  The OTS Directory Listing Object's contents is         */
   /* comprised of OTS Object Records (handled internally).  This       */
   /* function will simply decode each OTS Object Record for each OTS   */
   /* Object stored in the buffer.  This function may also be used to   */
   /* determine the number of OTS Objects contained in the buffer (see  */
   /* below).  The first parameter is the length of the buffer.  The    */
   /* second parameter is a pointer to the data in the buffer.  The     */
   /* third parameter is the number of OTS Objects that will be decoded.*/
   /* The final parameter is a pointer to store the parsed OTS Object   */
   /* data for each decoded OTS Object if this function is successful.  */
   /* This function returns a zero if successful.  If this function is  */
   /* used to determine the number of OTS Objects contained in the      */
   /* buffer, then a positive non-zero value will be returned.          */
   /* Otherwise, a negative return error code if an error occurs.       */
   /* * NOTE * If the NumberOfObjects parameter is 0, the ObjectData    */
   /*          parameter may be excluded (NULL), and this function will */
   /*          return a positive non-zero value, which represents the   */
   /*          number of OTS Objects contained in the buffer.  The OTS  */
   /*          Client may use this returned value to allocate enough    */
   /*          memory to store the OTS Object data for each decoded OTS */
   /*          Object contained in the buffer.                          */
BTPSAPI_DECLARATION int BTPSAPI OTS_Decode_Directory_Listing_Object_Contents(unsigned int BufferLength, Byte_t *Buffer, DWord_t NumberOfObjects, OTS_Object_Data_t *ObjectData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Decode_Directory_Listing_Object_Contents_t)(unsigned int BufferLength, Byte_t *Buffer, DWord_t NumberOfObjects, OTS_Object_Data_t *ObjectData);
#endif

   /* Object Transfer Channel (OTC) Common API.                         */

   /* The following function is responsible for requesting the          */
   /* termination of the Object Transfer Channel (OTC) connection.  The */
   /* first parameter to this function is the Bluetooth stack ID of the */
   /* Bluetooth Stack that is maintaining the OTC Connection.  The      */
   /* second parameter is the Channel Identifier (CID) for the OTC      */
   /* connection to close.  This function returns a zero value if the   */
   /* OTC connection was terminated, or a negative, return error code if*/
   /* unsuccessful.                                                     */
   /* * NOTE * Under normal circumstances (An error has not occured on  */
   /*          the OTS Server), the OTS Client should close the Object  */
   /*          Transfer Channel.                                        */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Close_Connection(unsigned int BluetoothStackID, Word_t CID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Close_Connection_t)(unsigned int BluetoothStackID, Word_t CID);
#endif

   /* The following function is responsible for sending data over the   */
   /* established Object Transfer Channel (OTC) connection to the       */
   /* specified OTC remote device.  This function provides additional   */
   /* functionality to control the amount of buffer usage for the OTC   */
   /* connection.  The first parameter is the Bluetooth stack ID of the */
   /* Bluetooth Stack that is maintaining the OTC Connection.  The      */
   /* second parameter is the Channel Identifier for the OTC Connection */
   /* (CID).  The third parameter is optional and is a pointer to a     */
   /* structure that contains conditional queueing parameters to control*/
   /* the handling of data packets.  The fourth and fifth parameters are*/
   /* the Data_Length of the Data Buffer and a pointer to the Data      */
   /* Buffer to Send.  If the QueueingParameters is NULL then this      */
   /* function returns a zero if the data was successfully sent.  If the*/
   /* QueueingParameters is not NULL then see below for possible return */
   /* values.  This function returns a negative return error code if    */
   /* unsuccessful.                                                     */
   /* * NOTE * If this function returns the Error Code:                 */
   /*          BTPS_ERROR_INSUFFICIENT_BUFFER_SPACE then this is a      */
   /*          signal to the caller that the requested data could NOT be*/
   /*          sent because the requested data could not be queued for  */
   /*          the OTC connection.  The caller then, must wait for the  */
   /*          etOTS_Channel_Buffer_Empty_Indication event before trying*/
   /*          to send any more data.                                   */
   /* * NOTE * If this function is called with a non-NULL               */
   /*          QueueingParameters then the data is queued conditionally.*/
   /*          If successful, the return value will indicate the number */
   /*          of packets/bytes that are currently queued for the OTC   */
   /*          connection at the time the function returns.             */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Send_Data(unsigned int BluetoothStackID, Word_t CID, L2CA_Queueing_Parameters_t *QueueingParameters, Word_t Data_Length, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Send_Data_t)(unsigned int BluetoothStackID, Word_t CID, L2CA_Queueing_Parameters_t *QueueingParameters, Word_t Data_Length, Byte_t *Data);
#endif

   /* The following function is responsible for allowing a mechanism of */
   /* granting the specified amount of credits for the Object Transfer  */
   /* Channel (OTC) Connection over the LE transport.  The first        */
   /* parameter is the Bluetooth Stack ID that identifies the stack in  */
   /* which the data is currently queued.  The second parameter is the  */
   /* Channel Identifier (CID) for the OTC Connection.  The final       */
   /* parameter is the total number of credits to grant for the OTC     */
   /* connection.  This function returns zero if successful or a        */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * This function may only be used for an OTC Connection that*/
   /*          has been established over the LE transport and is using  */
   /*          the Manual LE Based Credit Mode.                         */
BTPSAPI_DECLARATION int BTPSAPI OTS_Channel_Grant_Credits(unsigned int BluetoothStackID, Word_t CID, Word_t Credits);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OTS_Channel_Grant_Credits_t)(unsigned int BluetoothStackID, Word_t CID, Word_t Credits);
#endif

#endif
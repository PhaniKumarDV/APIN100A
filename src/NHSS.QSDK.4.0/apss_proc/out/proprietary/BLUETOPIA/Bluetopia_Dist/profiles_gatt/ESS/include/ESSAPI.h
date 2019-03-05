/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< essapi.h >*************************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ESSAPI - Qualcomm Technologies Bluetooth Environmental Sensing Service    */
/*           (GATT based) API Type Definitions, Constants, and Prototypes.    */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/24/15  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __ESSAPIH__
#define __ESSAPIH__

#include "SS1BTPS.h"         /* Bluetooth Stack API Prototypes/Constants.     */
#include "SS1BTGAT.h"        /* Bluetooth Stack GATT API Prototypes/Constants.*/
#include "ESSTypes.h"        /* Environmental Sensing Service Types/Constants.*/

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */

   /* The following error codes are common error codes.                 */
#define ESS_ERROR_INVALID_PARAMETER                              (-1000)
#define ESS_ERROR_INVALID_BLUETOOTH_STACK_ID                     (-1001)
#define ESS_ERROR_INSUFFICIENT_RESOURCES                         (-1002)
#define ESS_ERROR_SERVICE_ALREADY_REGISTERED                     (-1003)
#define ESS_ERROR_INVALID_INSTANCE_ID                            (-1004)
#define ESS_ERROR_MALFORMATTED_DATA                              (-1005)
#define ESS_ERROR_INSUFFICIENT_BUFFER_SPACE                      (-1006)
#define ESS_ERROR_UNKNOWN_ERROR                                  (-1007)

   /* The following error codes may be returned by                      */
   /* ESS_Initialize_Service() or ESS_Initialize_Service_Handle_Range() */
   /* if the ESS_Initialize_Data_t structure parameter and sub          */
   /* structures are not correctly configured.                          */
#define ESS_ERROR_INVALID_NUMBER_OF_CHARACTERISTIC_TYPE_ENTRIES  (-1008)
#define ESS_ERROR_DUPLICATE_CHARACTERISTIC_TYPE                  (-1009)
#define ESS_ERROR_INVALID_NUMBER_OF_TRIGGER_SETTING_INSTANCES    (-1010)
#define ESS_ERROR_MISSING_NOTIFY_CHARACTERISTIC_PROPERTY_FLAG    (-1011)
#define ESS_ERROR_MISSING_ES_MEASUREMENT_DESCRIPTOR_FLAG         (-1012)
#define ESS_ERROR_MISSING_ES_TRIGGER_SETTING_DESCRIPTOR_FLAG     (-1013)
#define ESS_ERROR_DESCRIPTOR_VALUE_CHANGED_NOT_REGISTERED        (-1014)
#define ESS_ERROR_NUMBER_OF_INSTANCES_CHECK_FAILURE              (-1015)

   /* The following error codes may be returned by response, write, and */
   /* decode functions.                                                 */
   /* * NOTE * If                                                       */
   /*          ESS_ERROR_INVALID_ES_TRIGGER_SETTING_TYPE_AND_CONDITION  */
   /*          is returned, this means that the condition is between    */
   /*          (tscLessThanSpecifiedValue <= condition <=               */
   /*          tscGreaterThanOrEqualSpecifiedValue), and an ESS         */
   /*          Characteristic Type has been specified that cannot be    */
   /*          used for the corresponding condition.  See the           */
   /*          ESS_ES_Trigger_Setting_Data_t structure for more         */
   /*          information.                                             */
#define ESS_ERROR_ATTRIBUTE_HANDLE_INFORMATION_NOT_FOUND         (-1016)
#define ESS_ERROR_INVALID_ES_TRIGGER_SETTING_INSTANCE            (-1017)
#define ESS_ERROR_INVALID_ES_TRIGGER_SETTING_CONDITION           (-1018)
#define ESS_ERROR_INVALID_ES_TRIGGER_SETTING_TYPE_AND_CONDITION  (-1019)
#define ESS_ERROR_INVALID_ES_TRIGGER_SETTING_UNIT                (-1020)
#define ESS_ERROR_INVALID_CHARACTERISTIC_TYPE                    (-1021)
#define ESS_ERROR_INVALID_UUID_TYPE                              (-1022)
#define ESS_ERROR_MISSING_EXTENDED_PROPERTIES                    (-1023)

   /* The following enumeration defines the types for ESS               */
   /* characteristics.                                                  */
typedef enum _tagESS_Characteristic_Type_t
{
   ectApparentWindDirection,
   ectApparentWindSpeed,
   ectDewPoint,
   ectElevation,
   ectGustFactor,
   ectHeatIndex,
   ectHumidity,
   ectIrradiance,
   ectPollenConcentration,
   ectRainfall,
   ectPressure,
   ectTemperature,
   ectTrueWindDirection,
   ectTrueWindSpeed,
   ectUVIndex,
   ectWindChill,
   ectBarometricPressureTrend,
   ectMagneticDeclination,
   ectMagneticFluxDensity2D,
   ectMagneticFluxDensity3D
} ESS_Characteristic_Type_t;

   /* The following structure defines the unsigned/signed 24 bit        */
   /* integer.                                                          */
typedef struct _tagESS_Int_24_Data_t
{
   Word_t  Lower;
   Byte_t  Upper;
} ESS_Int_24_Data_t;

#define ESS_INT_24_DATA_SIZE                                   (sizeof(ESS_Int_24_Data_t))

   /* The following defines the enumeration values for the ESS          */
   /* Characteristic: Barometric Pressure Trend.                        */
typedef enum _tagESS_Barometric_Pressure_Trend_t
{
   bptUnknown,
   bptContinuouslyFalling,
   bptContinuouslyRising,
   bptFallingThenSteady,
   bptRisingThenSteady,
   bptFallingBeforeLesserRise,
   bptFallingBeforeGreaterRise,
   bptRisingBeforeGreaterFall,
   bptRisingBeforeLesserFall,
   bptSteady
} ESS_Barometric_Pressure_Trend_t;

   /* The following structure defines the Magnetic Flux Density - 2D    */
   /* Data structure.                                                   */
typedef struct _tagESS_Magnetic_Flux_Density_2D_Data_t
{
   SWord_t  X_Axis;
   SWord_t  Y_Axis;
} ESS_Magnetic_Flux_Density_2D_Data_t;

#define ESS_MAGNETIC_FLUX_DENSITY_2D_DATA_SIZE                 (sizeof(ESS_Magnetic_Flux_Density_2D_Data_t))

   /* The following structure defines the Magnetic Flux Density - 3D    */
   /* Data structure.                                                   */
typedef struct _tagESS_Magnetic_Flux_Density_3D_Data_t
{
   SWord_t  X_Axis;
   SWord_t  Y_Axis;
   SWord_t  Z_Axis;
} ESS_Magnetic_Flux_Density_3D_Data_t;

#define ESS_MAGNETIC_FLUX_DENSITY_3D_DATA_SIZE                 (sizeof(ESS_Magnetic_Flux_Density_3D_Data_t))

   /* The following union defines the ESS Characteristic data types.    */
typedef union _tagESS_Characteristic_Data_t
{
   Word_t                               Apparent_Wind_Direction;
   Word_t                               Apparent_Wind_Speed;
   SByte_t                              Dew_Point;
   ESS_Int_24_Data_t                    Elevation;
   Byte_t                               Gust_Factor;
   SByte_t                              Heat_Index;
   Word_t                               Humidity;
   Word_t                               Irradiance;
   ESS_Int_24_Data_t                    Pollen_Concentration;
   Word_t                               Rainfall;
   DWord_t                              Pressure;
   SWord_t                              Temperature;
   Word_t                               True_Wind_Direction;
   Word_t                               True_Wind_Speed;
   Byte_t                               UV_Index;
   SByte_t                              Wind_Chill;
   ESS_Barometric_Pressure_Trend_t      Barometric_Pressure_Trend;
   Word_t                               Magnetic_Declination;
   ESS_Magnetic_Flux_Density_2D_Data_t  Magnetic_Flux_Density_2D;
   ESS_Magnetic_Flux_Density_3D_Data_t  Magnetic_Flux_Density_3D;
} ESS_Characteristic_Data_t;

   /* The following defines the optional properties for each ESS        */
   /* Characteristic.                                                   */
   /* * NOTE * If this flag is not set then the                         */
   /*          ESS_DESCRIPTOR_FLAGS_ES_TRIGGER_SETTING may not be set   */
   /*          since the ES Trigger Setting and ES Configuration        */
   /*          descriptors CANNOT be included if the ESS Characteristic */
   /*          does not have the Notify property.                       */
#define ESS_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY                        (0x01)

   /* The following defines the optional descriptors that may be added  */
   /* for each ESS Characteristic.                                      */
   /* * NOTE * The ESS_CHARACTERISTIC_DESCRIPTOR_FLAGS_ES_MEASUREMENT is*/
   /*          ONLY OPTIONAL if there is only one instance of a given   */
   /*          ESS Characteristic Type.  This flag is MANDATORY if      */
   /*          multiple instances of the same ESS Characteristic Type   */
   /*          are registered with the service.  If multiple instances  */
   /*          are present, ESS will fail to register if multiple       */
   /*          instances do not have this flag.                         */
   /* * NOTE * The                                                      */
   /*          ESS_CHARACTERISTIC_DESCRIPTOR_FLAGS_ES_TRIGGER_SETTING   */
   /*          SHALL only be set if the                                 */
   /*          ESS_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY is set for the  */
   /*          Characteristic_Property_Flags field of the               */
   /*          ESS_Characteristic_Instance_Entry_t structure.           */
   /* * NOTE * If there are multiple instances (up to 3 instances       */
   /*          maximum) of the ES Trigger Setting descriptor for an ESS */
   /*          Characteristic instance (Set by the                      */
   /*          Trigger_Setting_Instances field of the                   */
   /*          ESS_Characteristic_Instance_Entry_t structure), then the */
   /*          ES Configuration descriptor will automatically be        */
   /*          included.                                                */
#define ESS_DESCRIPTOR_FLAGS_ES_MEASUREMENT                             (0x01)
#define ESS_DESCRIPTOR_FLAGS_ES_TRIGGER_SETTING                         (0x02)
#define ESS_DESCRIPTOR_FLAGS_USER_DESCRIPTION                           (0x04)
#define ESS_DESCRIPTOR_FLAGS_VALID_RANGE                                (0x08)

   /* The following defines the optional descriptor properties.         */
   /* * NOTE * If the                                                   */
   /*          ESS_DESCRIPTOR_PROPERTY_FLAGS_WRITE_ES_TRIGGER_SETTING   */
   /*          property is set and an ESS Characteristic instance has   */
   /*          multiple ES Trigger Setting descriptors, then the ES     */
   /*          Configuration descriptor that is automatically included  */
   /*          for multiple ES Trigger Setting descriptors will also    */
   /*          have the write property.                                 */
#define ESS_DESCRIPTOR_PROPERTY_FLAGS_WRITE_ES_TRIGGER_SETTING          (0x01)
#define ESS_DESCRIPTOR_PROPERTY_FLAGS_WRITE_USER_DESCRIPTION            (0x02)

   /* The following structure defines the information needed to register*/
   /* an ESS Characteristic instance.                                   */
   /* * NOTE * All instances share the same ESS Characteristic Type so  */
   /*          it is implicitly known by the ESS_Characteristic_Entry_t */
   /*          parent structure.                                        */
   /* * NOTE * The Trigger_Setting_Instances is only valid if the       */
   /*          ESS_CHARACTERISTIC_DESCRIPTOR_FLAGS_ES_TRIGGER_SETTING   */
   /*          bit of the Descriptor_Flags is valid.                    */
   /* * NOTE * Each ESS Characteristic instance may have up to 3 ES     */
   /*          Trigger Setting descriptor instances.  The               */
   /*          Trigger_Setting_Instances field MUST be between (1-3).   */
   /* * NOTE * If multiple ES Trigger Setting descriptor instances are  */
   /*          specified by the Trigger_Setting_Instances field, then   */
   /*          the ES Configuration descriptor will automatically be    */
   /*          registered for this ESS Characteristic instance.         */
typedef struct _tagESS_Characteristic_Instance_Entry_t
{
   Byte_t  Characteristic_Property_Flags;
   Byte_t  Descriptor_Flags;
   Byte_t  Descriptor_Property_Flags;
   Byte_t  Trigger_Setting_Instances;
} ESS_Characteristic_Instance_Entry_t;

#define ESS_CHARACTERISTIC_INSTANCE_ENTRY_SIZE                 (sizeof(ESS_Characteristic_Instance_Entry_t))

   /* The following structure defines the information needed to register*/
   /* an ESS Characteristic and all of it's instances.                  */
   /* * NOTE * The Type field identifies the type of the ESS            */
   /*          Characteristic (MUST be UNIQUE, see the                  */
   /*          ESS_Initialize_Data_t structure below).                  */
   /* * NOTE * The Number_Of_Instances field identifies how many ESS    */
   /*          Characteristic instances need to be registered with the  */
   /*          service for the ESS Characteristic Type.                 */
typedef struct _tagESS_Characteristic_Entry_t
{
   ESS_Characteristic_Type_t            Type;
   Word_t                               Number_Of_Instances;
   ESS_Characteristic_Instance_Entry_t *Instances;
} ESS_Characteristic_Entry_t;

#define ESS_CHARACTERISTIC_ENTRY_SIZE                          (sizeof(ESS_Characteristic_Entry_t))

   /* The following structure is used by the ESS_Initialize_Service()   */
   /* and ESS_Initialize_Service_Handle_Range() functions to allow the  */
   /* the ESS Server to specify the optional ESS Characteristics,       */
   /* descriptors, and properties to include when the service is        */
   /* registered.  This structure also will include the optional        */
   /* Descriptor Value Changed Characteristic (Conditions apply, see    */
   /* below).                                                           */
   /* * NOTE * Each ESS_Characteristic_Entry_t structure MUST have a    */
   /*          UNIQUE Type field for each structure in the              */
   /*          Characteristics field.  This is REQUIRED to enforce that */
   /*          all instances for an ESS Characteristic Type will be     */
   /*          registered sequentially.  If a duplicate Type field is   */
   /*          found the service will fail to register.                 */
   /* * NOTE * The Number_Of_Entries field MUST be at least 1 and is the*/
   /*          number of ESS_Characteristic_Entry_t structures that the */
   /*          Entries field holds.  This directly corresponds to the   */
   /*          total number of unique ESS Characteristics Types (Max of */
   /*          20).                                                     */
   /* * NOTE * The Descriptor_Value_Changed field indicates whether the */
   /*          Descriptor Value Changed Characteristic will be          */
   /*          registered.  A Client Characteristic Configuration       */
   /*          Descriptor (CCCD) will be automatically registered with  */
   /*          the service for this characteristic if TRUE.             */
   /* * NOTE * The Descriptor_Value_Changed MUST be TRUE if the User    */
   /*          Description descriptor has the write property for any ESS*/
   /*          Characteristic.                                          */
   /* * NOTE * If the Descriptor_Value_Changed is FALSE, it is the ESS  */
   /*          Server's responsibility to follow the specification and  */
   /*          not change the ES Measurement, ES Trigger Setting, ES    */
   /*          Configuration, or Characteristic User Description        */
   /*          descriptors if they are present, since they are stored in*/
   /*          the application.  If these descriptors are present they  */
   /*          should be initialized ONCE, and, before or immediately   */
   /*          following a call to either of the                        */
   /*          Initialize_Service_XXX() API's.                          */
   /* * NOTE * If the Descriptor_Value_Changed is FALSE, then the       */
   /*          ESS_Indicate_Characteristic(),                           */
   /*          ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response()*/
   /*          and                                                      */
   /*          ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response(*/
   /*          functions will fail since they are dependedent on the    */
   /*          Descriptor Value Changed characteristic.                 */
typedef struct _tagESS_Initialize_Data_t
{
   Byte_t                      Number_Of_Entries;
   ESS_Characteristic_Entry_t *Entries;
   Boolean_t                   Descriptor_Value_Changed;
} ESS_Initialize_Data_t;

#define ESS_INITIALIZE_DATA_SIZE                               (sizeof(ESS_Initialize_Data_t))

   /* The following structure defines the ESS Environmental Sensing     */
   /* Measurement data.                                                 */
   /* * NOTE * Definitions for these fields may be found in ESSTypes.h. */
   /*          See the ESS_ES_Measurement_t structure.                  */
typedef struct _tagESS_ES_Measurement_Data_t
{
   Word_t             Flags;
   Byte_t             Sampling_Function;
   ESS_Int_24_Data_t  Measurement_Period;
   ESS_Int_24_Data_t  Update_Interval;
   Byte_t             Application;
   Byte_t             Measurement_Uncertainty;
} ESS_ES_Measurement_Data_t;

#define ESS_ES_MEASUREMENT_DATA_SIZE                           (sizeof(ESS_ES_Measurement_Data_t))

   /* The following enumeration identifies the ESS Trigger Setting      */
   /* instance.  An ESS Characteristic may have up to 3 instances of    */
   /* this descriptor.                                                  */
typedef enum _tagESS_ES_Trigger_Setting_Instance_t
{
   tsiTriggerSetting_0,
   tsiTriggerSetting_1,
   tsiTriggerSetting_2
} ESS_ES_Trigger_Setting_Instance_t;

   /* The following defines the enumeration values for the Condition    */
   /* field of the ESS_ES_Trigger_Setting_Data_t structure below.       */
   /* * NOTE * ESS Characteristics that hold multiple values (Barometric*/
   /*          Pressure Trend, Magnetic Flux 2D/3D) CANNOT use the      */
   /*          conditions where conditions are between                  */
   /*          (tscLessThanSpecifiedValue <= condition <=               */
   /*          tscGreaterThanOrEqualSpecifiedValue).                    */
typedef enum _tagESS_ES_Trigger_Setting_Condition_t
{
   tscTriggerInactive                  = (ESS_ES_TRIGGER_SETTING_CONDITION_TRIGGER_INACTIVE),
   tscFixedTimeInterval                = (ESS_ES_TRIGGER_SETTING_CONDITION_FIXED_TIME_INTERVAL),
   tscNoLessThanSpecifiedTime          = (ESS_ES_TRIGGER_SETTING_CONDITION_NO_LESS_THAN_SPECIFIED_TIME),
   tscValueChanged                     = (ESS_ES_TRIGGER_SETTING_CONDITION_VALUE_CHANGED),
   tscLessThanSpecifiedValue           = (ESS_ES_TRIGGER_SETTING_CONDITION_LESS_THAN_SPECIFIED_VALUE),
   tscLessThanOrEqualSpecifiedValue    = (ESS_ES_TRIGGER_SETTING_CONDITION_LESS_THAN_OR_EQUAL_SPECIFIED_VALUE),
   tscGreaterThanSpecifiedValue        = (ESS_ES_TRIGGER_SETTING_CONDITION_GREATER_THAN_SPECIFIED_VALUE),
   tscGreaterThanOrEqualSpecifiedValue = (ESS_ES_TRIGGER_SETTING_CONDITION_GREATER_THAN_OR_EQUAL_SPECIFIED_VALUE),
   tscEqualSpecifiedValue              = (ESS_ES_TRIGGER_SETTING_CONDITION_EQUAL_TO_SPECIFIED_VALUE),
   tscNotEqualSpecifiedValue           = (ESS_ES_TRIGGER_SETTING_CONDITION_NOT_EQUAL_TO_SPECIFIED_VALUE)
} ESS_ES_Trigger_Setting_Condition_t;

   /* The following structure defines the ESS Environmental Sensing     */
   /* Trigger Setting data.                                             */
   /* * NOTE * The Condition field identifies the condition that will   */
   /*          trigger a notification.  Multiple instances of this      */
   /*          descriptor will depend on the ES Configuration descriptor*/
   /*          to determine if multiple trigger conditions should be    */
   /*          ANDED/ORED together to trigger a notification for the ESS*/
   /*          Characteristic.                                          */
   /* * NOTE * The tscTriggerInactive and tscValueChanged conditions    */
   /*          have no operand.                                         */
   /* * NOTE * The tscFixedTimeInterval and tscNoLessThanSpecifiedTime  */
   /*          conditions will use the Seconds Operand.                 */
   /* * NOTE * ESS Barometric Pressure Trend, ESS Magnetic Flux 2D, and */
   /*          ESS Magnetic Flux 3D, and future ESS Characteristics that*/
   /*          hold multiple values CANNOT use the specified value      */
   /*          conditions where conditions are between                  */
   /*          (tscLessThanSpecifiedValue <= condition <=               */
   /*          tscGreaterThanOrEqualSpecifiedValue).  If the ESS        */
   /*          Characteristic Type and Condition fields are set so that */
   /*          this requirement does not hold then API's that use this  */
   /*          structure will return                                    */
   /*          ESS_ERROR_INVALID_ES_TRIGGER_SETTING_TYPE_AND_CONDITION. */
typedef struct _tagESS_ES_Trigger_Setting_Data_t
{
   ESS_ES_Trigger_Setting_Condition_t  Condition;
   union
   {
      ESS_Int_24_Data_t                Seconds;
      Word_t                           Apparent_Wind_Direction;
      Word_t                           Apparent_Wind_Speed;
      SByte_t                          Dew_Point;
      ESS_Int_24_Data_t                Elevation;
      Byte_t                           Gust_Factor;
      SByte_t                          Heat_Index;
      Word_t                           Humidity;
      Word_t                           Irradiance;
      ESS_Int_24_Data_t                Pollen_Concentration;
      Word_t                           Rainfall;
      DWord_t                          Pressure;
      SWord_t                          Temperature;
      Word_t                           True_Wind_Direction;
      Word_t                           True_Wind_Speed;
      Byte_t                           UV_Index;
      SByte_t                          Wind_Chill;
      Word_t                           Magnetic_Declination;
   } Operand;
} ESS_ES_Trigger_Setting_Data_t;

#define ESS_ES_TRIGGER_SETTING_DATA_SIZE                       (sizeof(ESS_ES_Trigger_Setting_Data_t))

   /* The following structure defines the ESS Valid Range data.         */
typedef struct _tagESS_Valid_Range_Data_t
{
   ESS_Characteristic_Data_t  Lower;
   ESS_Characteristic_Data_t  Upper;
} ESS_Valid_Range_Data_t;

#define ESS_VALID_RANGE_DATA_SIZE                              (sizeof(ESS_Valid_Range_Data_t))

   /* The following structure defines the ESS Descriptor Value Changed  */
   /* Characteristic.                                                   */
   /* * NOTE * The Flags field is a bit mask and takes bit values of the*/
   /*          form ESS_DESCRIPTOR_VALUE_CHANGED_FLAGS_XXX, where XXX   */
   /*          indicates the Flag.  These can be found in ESSTypes.h.   */
   /* * NOTE * Only the 16 and 128 UUID Types may be used for the UUID  */
   /*          field.                                                   */
typedef struct _tagESS_Descriptor_Value_Changed_Data_t
{
   Word_t       Flags;
   GATT_UUID_t  UUID;
} ESS_Descriptor_Value_Changed_Data_t;

#define ESS_DESCRIPTOR_VALUE_CHANGED_DATA_SIZE                 (sizeof(ESS_Descriptor_Value_Changed_Data_t))

   /* The following structure defines the Characteristic information    */
   /* field that will be included in events below to identify the type  */
   /* of ESS Characteristic as well as the instance.  This allows the   */
   /* ESS Server to store/retrieve the corresponding characteristic or  */
   /* descriptor value for the specified ESS Characteristic instance.   */
   /* * NOTE * The Type field identifies the Type of the ESS            */
   /*          Characteristic.                                          */
   /* * NOTE * The ID field identifies the instance of the ESS          */
   /*          Characteristic.  Instances start at '0' and increment for*/
   /*          each instance registered with the service.               */
   /* * NOTE * This structure will ONLY be used for APIs and events that*/
   /*          are for ESS Characteristics (Not the Descriptor Value    */
   /*          Changed characteristic).                                 */
typedef struct _tagESS_Characteristic_Info_t
{
   ESS_Characteristic_Type_t  Type;
   unsigned int               ID;
} ESS_Characteristic_Info_t;

#define ESS_CHARACTERISTIC_INFO_SIZE                           (sizeof(ESS_Valid_Range_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* ESS Service.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the ESS_Event_Data_t structure.                                   */
typedef enum  _tagESS_Event_Type_t
{
   etESS_Server_Read_Characteristic_Request,
   etESS_Server_Read_Extended_Properties_Request,
   etESS_Server_Read_CCCD_Request,
   etESS_Server_Write_CCCD_Request,
   etESS_Server_Read_ES_Measurement_Request,
   etESS_Server_Read_ES_Trigger_Setting_Request,
   etESS_Server_Write_ES_Trigger_Setting_Request,
   etESS_Server_Read_ES_Configuration_Request,
   etESS_Server_Write_ES_Configuration_Request,
   etESS_Server_Read_User_Description_Request,
   etESS_Server_Write_User_Description_Request,
   etESS_Server_Read_Valid_Range_Request,
   etESS_Server_Read_Descriptor_Value_Changed_CCCD_Request,
   etESS_Server_Write_Descriptor_Value_Changed_CCCD_Request,
   etESS_Server_Confirmation
} ESS_Event_Type_t;

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS Characteristic.   */
   /* The InstanceID field is the identifier for the instance of ESS    */
   /* that received the request.  The ConnectionID is the identifier for*/
   /* the GATT connection between the ESS Client and ESS Server.  The   */
   /* ConnectionType field specifies the GATT connection type.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the ESS Client that  */
   /* made the reqest.  The CharacteristicInfo field contains           */
   /* information about the ESS Characteristic that has been requested  */
   /* to be read.                                                       */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic exists in the registered ESS.             */
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_Characteristic_Request_Response() API to*/
   /*          send the response to the ESS Client.  This API can be    */
   /*          used to send both the read and error responses for the   */
   /*          received event.                                          */
typedef struct _tagESS_Read_Characteristic_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
} ESS_Read_Characteristic_Request_Data_t;

#define ESS_READ_CHARACTERISTIC_REQUEST_DATA_SIZE              (sizeof(ESS_Read_Characteristic_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS Client            */
   /* Characteristic Configuration descriptor (CCCD).  The InstanceID   */
   /* field is the identifier for the instance of ESS that received the */
   /* request.  The ConnectionID is the identifier for the GATT         */
   /* connection between the ESS Client and ESS Server.  The            */
   /* ConnectionType field specifies the GATT connection type.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the ESS Client that  */
   /* made the reqest.  The CharacteristicInfo field contains           */
   /* information about the ESS Characteristic whose CCCD has been      */
   /* requested to be read.                                             */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_CCCD_Request_Response() API to send the */
   /*          response to the ESS Client.  This API can be used to send*/
   /*          both the read and error responses for the received event.*/
typedef struct _tagESS_Read_CCCD_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
} ESS_Read_CCCD_Request_Data_t;

#define ESS_READ_CCCD_REQUEST_DATA_SIZE                        (sizeof(ESS_Read_CCCD_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a write request for an ESS Client           */
   /* Characteristic Configuration descriptor.  The InstanceID field is */
   /* the identifier for the instance of ESS that received the request. */
   /* The ConnectionID is the identifier for the GATT connection between*/
   /* the ESS Client and ESS Server.  The ConnectionType field specifies*/
   /* the GATT connection type.  The TransactionID field identifies the */
   /* GATT transaction for the request.  The RemoteDevice is the BD_ADDR*/
   /* of the ESS Client that made the reqest.  The CharacteristicInfo   */
   /* field contains information about the ESS Characteristic whose CCCD*/
   /* has been requested to be written.  The final field is the         */
   /* ClientConfiguration, which holds the new Client Characteristic    */
   /* Configuration.                                                    */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Write_CCCD_Request_Response() API to send the*/
   /*          response to the ESS Client.  This API can be used to send*/
   /*          both the write and error responses for the received      */
   /*          event.                                                   */
typedef struct _tagESS_Write_CCCD_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   Word_t                     ClientConfiguration;
} ESS_Write_CCCD_Request_Data_t;

#define ESS_WRITE_CCCD_REQUEST_DATA_SIZE                       (sizeof(ESS_Write_CCCD_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS Environmental     */
   /* Sensing Measurement descriptor.  The InstanceID field is the      */
   /* identifier for the instance of ESS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* ESS Client and ESS Server.  The ConnectionType field specifies the*/
   /* GATT connection type.  The TransactionID field identifies the GATT*/
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose ES        */
   /* Measurement descriptor has been requested to be read.             */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_ES_Measurement_Request_Response() API to*/
   /*          send the response to the ESS Client.  This API can be    */
   /*          used to send both the read and error responses for the   */
   /*          received event.                                          */
typedef struct _tagESS_Read_ES_Measurement_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
} ESS_Read_ES_Measurement_Request_Data_t;

#define ESS_READ_ES_MEASUREMENT_REQUEST_DATA_SIZE              (sizeof(ESS_Read_ES_Measurement_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS Environmental     */
   /* Sensing Trigger Setting descriptor.  The InstanceID field is the  */
   /* identifier for the instance of ESS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* ESS Client and ESS Server.  The ConnectionType field specifies the*/
   /* GATT connection type.  The TransactionID field identifies the GATT*/
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The Instance field holds the*/
   /* instance of the ES Trigger Setting that is being read.  The       */
   /* CharacteristicInfo field contains information about the ESS       */
   /* Characteristic whose ES Trigger Setting descriptor has been       */
   /* requested to be read.                                             */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_ES_Trigger_Setting_Request_Response()   */
   /*          API to send the response to the ESS Client.  This API can*/
   /*          be used to send both the read and error responses for the*/
   /*          received event.                                          */
typedef struct _tagESS_Read_ES_Trigger_Setting_Request_Data_t
{
   unsigned int                       InstanceID;
   unsigned int                       ConnectionID;
   GATT_Connection_Type_t             ConnectionType;
   unsigned int                       TransactionID;
   BD_ADDR_t                          RemoteDevice;
   ESS_Characteristic_Info_t          CharacteristicInfo;
   ESS_ES_Trigger_Setting_Instance_t  Instance;
} ESS_Read_ES_Trigger_Setting_Request_Data_t;

#define ESS_READ_ES_TRIGGER_SETTING_REQUEST_DATA_SIZE          (sizeof(ESS_Read_ES_Trigger_Setting_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a write request for an ESS Environmental    */
   /* Sensing Trigger Setting descriptor.  The InstanceID field is the  */
   /* identifier for the instance of ESS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* ESS Client and ESS Server.  The ConnectionType field specifies the*/
   /* GATT connection type.  The TransactionID field identifies the GATT*/
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose ES Trigger*/
   /* Setting descriptor has been requested to be written.  The Instance*/
   /* field, holds the ES Trigger Setting descriptor instance that is   */
   /* being written.  The final field is the TriggerSetting, which holds*/
   /* the new ES Trigger Setting data.                                  */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Write_ES_Trigger_Setting_Request_Response()  */
   /*          API to send the response to the ESS Client.  This API can*/
   /*          be used to send both the write and error responses for   */
   /*          the received event.                                      */
typedef struct _tagESS_Write_ES_Trigger_Setting_Request_Data_t
{
   unsigned int                       InstanceID;
   unsigned int                       ConnectionID;
   GATT_Connection_Type_t             ConnectionType;
   unsigned int                       TransactionID;
   BD_ADDR_t                          RemoteDevice;
   ESS_Characteristic_Info_t          CharacteristicInfo;
   ESS_ES_Trigger_Setting_Instance_t  Instance;
   ESS_ES_Trigger_Setting_Data_t      TriggerSetting;
} ESS_Write_ES_Trigger_Setting_Request_Data_t;

#define ESS_WRITE_ES_TRIGGER_SETTING_REQUEST_DATA_SIZE         (sizeof(ESS_Write_ES_Trigger_Setting_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS Environmental     */
   /* Sensing Configuration descriptor.  The InstanceID field is the    */
   /* identifier for the instance of ESS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* ESS Client and ESS Server.  The ConnectionType field specifies the*/
   /* GATT connection type.  The TransactionID field identifies the GATT*/
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose ES        */
   /* Configuration descriptor has been requested to be read.           */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_ES_Configuration_Request_Response() API */
   /*          to send the response to the ESS Client.  This API can be */
   /*          used to send both the read and error responses for the   */
   /*          received event.                                          */
typedef struct _tagESS_Read_ES_Configuration_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
} ESS_Read_ES_Configuration_Request_Data_t;

#define ESS_READ_ES_CONFIGURATION_REQUEST_DATA_SIZE            (sizeof(ESS_Read_ES_Configuration_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a write request for an ESS Environmental    */
   /* Sensing Configuration descriptor.  The InstanceID field is the    */
   /* identifier for the instance of ESS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* ESS Client and ESS Server.  The ConnectionType field specifies the*/
   /* GATT connection type.  The TransactionID field identifies the GATT*/
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose ES        */
   /* Configuration descriptor has been requested to be written.  The   */
   /* final field is the Configuration, which holds the new ES          */
   /* Configuration.                                                    */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Write_ES_Configuration_Request_Response() API*/
   /*          to send the response to the ESS Client.  This API can be */
   /*          used to send both the write and error responses for the  */
   /*          received event.                                          */
typedef struct _tagESS_Write_ES_Configuration_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   Byte_t                     Configuration;
} ESS_Write_ES_Configuration_Request_Data_t;

#define ESS_WRITE_ES_CONFIGURATION_REQUEST_DATA_SIZE           (sizeof(ESS_Write_ES_Configuration_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS User Description  */
   /* descriptor.  The InstanceID field is the identifier for the       */
   /* instance of ESS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the ESS Client and */
   /* ESS Server.  The ConnectionType field specifies the GATT          */
   /* connection type.  The TransactionID field identifies the GATT     */
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose User      */
   /* Description descriptor has been requested to be read.  The final  */
   /* field is the UserDescriptionOffset and will be set to zero for    */
   /* GATT Read Value request, however if we receive a GATT Read Long   */
   /* Value request this field will be used to indicate the starting    */
   /* offset we should read the User Description from.  This may happen */
   /* if the User Description has a length that is greater than can fit */
   /* in the GATT Read response.  This way we can send the remaining    */
   /* data for the User Description descriptor.  This event may be      */
   /* received multiple times, with the UserDescriptionOffset increased */
   /* until the entire User Description descriptor has been read.       */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_User_Description_Request_Response() API */
   /*          to send the response to the ESS Client.  This API can be */
   /*          used to send both the read and error responses for the   */
   /*          received event.                                          */
typedef struct _tagESS_Read_User_Description_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   Word_t                     UserDescriptionOffset;
} ESS_Read_User_Description_Request_Data_t;

#define ESS_READ_USER_DESCRIPTION_REQUEST_DATA_SIZE            (sizeof(ESS_Read_User_Description_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a write request for an ESS User Description */
   /* descriptor.  The InstanceID field is the identifier for the       */
   /* instance of ESS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the ESS Client and */
   /* ESS Server.  The ConnectionType field specifies the GATT          */
   /* connection type.  The TransactionID field identifies the GATT     */
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose User      */
   /* Description has been requested to be written.  The final fields   */
   /* are the UserDescriptionLength and the UserDescription.            */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Write_User_Description_Request_Response() API*/
   /*          to send the response to the ESS Client.  This API can be */
   /*          used to send both the write and error responses for the  */
   /*          received event.                                          */
typedef struct _tagESS_Write_User_Description_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   Word_t                     UserDescriptionLength;
   Byte_t                    *UserDescription;
} ESS_Write_User_Description_Request_Data_t;

#define ESS_WRITE_USER_DESCRIPTION_REQUEST_DATA_SIZE           (sizeof(ESS_Write_User_Description_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an ESS Valid Range       */
   /* descriptor.  The InstanceID field is the identifier for the       */
   /* instance of ESS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the ESS Client and */
   /* ESS Server.  The ConnectionType field specifies the GATT          */
   /* connection type.  The TransactionID field identifies the GATT     */
   /* transaction for the request.  The RemoteDevice is the BD_ADDR of  */
   /* the ESS Client that made the reqest.  The CharacteristicInfo field*/
   /* contains information about the ESS Characteristic whose Valid     */
   /* Range descriptor has been requested to be read.                   */
   /* * NOTE * This event will only be received if the requested ESS    */
   /*          Characteristic's descriptor exists in the registered ESS.*/
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the ESS_Read_Valid_Range_Request_Response() API to   */
   /*          send the response to the ESS Client.  This API can be    */
   /*          used to send both the read and error responses for the   */
   /*          received event.                                          */
typedef struct _tagESS_Read_Valid_Range_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   ESS_Characteristic_Info_t  CharacteristicInfo;
} ESS_Read_Valid_Range_Request_Data_t;

#define ESS_READ_VALID_RANGE_REQUEST_DATA_SIZE                 (sizeof(ESS_Read_Valid_Range_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a read request for an Descriptor Value      */
   /* Changed Client Characteristic Configuration descriptor (CCCD).    */
   /* The InstanceID field is the identifier for the instance of ESS    */
   /* that received the request.  The ConnectionID is the identifier for*/
   /* the GATT connection between the ESS Client and ESS Server.  The   */
   /* ConnectionType field specifies the GATT connection type.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the ESS Client that  */
   /* made the reqest.                                                  */
   /* * NOTE * This event will only be received if the requested        */
   /*          Descriptor Value Changed Characteristic exists in the    */
   /*          registered ESS.                                          */
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the                                                  */
   /*          ESS_Read_Descriptor_Value_Changed_CCCD_Request_Response()*/
   /*          API to send the response to the ESS Client.  This API can*/
   /*          be used to send both the read and error responses for the*/
   /*          received event.                                          */
typedef struct _tagESS_Read_Descriptor_Value_Changed_CCCD_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
} ESS_Read_Descriptor_Value_Changed_CCCD_Request_Data_t;

#define ESS_READ_DESCRIPTOR_VALUE_CHANGED_CCCD_REQUEST_DATA_SIZE   (sizeof(ESS_Read_Descriptor_Value_Changed_CCCD_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a write request for an Descriptor Value     */
   /* Changed Client Characteristic Configuration descriptor (CCCD).    */
   /* The InstanceID field is the identifier for the instance of ESS    */
   /* that received the request.  The ConnectionID is the identifier for*/
   /* the GATT connection between the ESS Client and ESS Server.  The   */
   /* ConnectionType field specifies the GATT connection type.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the ESS Client that  */
   /* made the reqest.  The final field is the ClientConfiguration,     */
   /* which holds the new Client Characteristic Configuration.          */
   /* * NOTE * This event will only be received if the requested        */
   /*          Descriptor Value Changed Characteristic exists in the    */
   /*          registered ESS.                                          */
   /* * NOTE * Many of these fields will be needed by the application   */
   /*          for the                                                  */
   /*          ESS_Write_Descriptor_Value_Changed_CCCD_Request_Response(*/
   /*          API to send the response to the ESS Client.  This API can*/
   /*          be used to send both the read and error responses for the*/
   /*          received event.                                          */
typedef struct _tagESS_Write_Descriptor_Value_Changed_CCCD_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   Word_t                     ClientConfiguration;
} ESS_Write_Descriptor_Value_Changed_CCCD_Request_Data_t;

#define ESS_WRITE_DESCRIPTOR_VALUE_CHANGED_CCCD_REQUEST_DATA_SIZE  (sizeof(ESS_Write_Descriptor_Value_Changed_CCCD_Request_Data_t))

   /* The following ESS Service Event is dispatched to a ESS Server when*/
   /* a ESS Client has sent a confirmation to a previously sent         */
   /* indication.  The InstanceID field is the identifier for the       */
   /* instance of ESS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the ESS Client and */
   /* ESS Server.  The ConnectionType field specifies the GATT          */
   /* connection type.  The TransactionID field identifies the GATT     */
   /* transaction for the confirmation.  The RemoteDevice is the BD_ADDR*/
   /* of the ESS Client that made the reqest.  The final field Status,  */
   /* holds the status of the confirmation.                             */
typedef struct _tagESS_Confirmation_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
   Byte_t                  Status;
} ESS_Confirmation_Data_t;

#define ESS_CONFIRMATION_DATA_SIZE                             (sizeof(ESS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all ESS Service Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagESS_Event_Data_t
{
   ESS_Event_Type_t  Event_Data_Type;
   Byte_t            Event_Data_Size;
   union
   {
     ESS_Read_Characteristic_Request_Data_t                 *ESS_Read_Characteristic_Request_Data;
     ESS_Read_CCCD_Request_Data_t                           *ESS_Read_CCCD_Request_Data;
     ESS_Write_CCCD_Request_Data_t                          *ESS_Write_CCCD_Request_Data;
     ESS_Read_ES_Measurement_Request_Data_t                 *ESS_Read_ES_Measurement_Request_Data;
     ESS_Read_ES_Trigger_Setting_Request_Data_t             *ESS_Read_ES_Trigger_Setting_Request_Data;
     ESS_Write_ES_Trigger_Setting_Request_Data_t            *ESS_Write_ES_Trigger_Setting_Request_Data;
     ESS_Read_ES_Configuration_Request_Data_t               *ESS_Read_ES_Configuration_Request_Data;
     ESS_Write_ES_Configuration_Request_Data_t              *ESS_Write_ES_Configuration_Request_Data;
     ESS_Read_User_Description_Request_Data_t               *ESS_Read_User_Description_Request_Data;
     ESS_Write_User_Description_Request_Data_t              *ESS_Write_User_Description_Request_Data;
     ESS_Read_Valid_Range_Request_Data_t                    *ESS_Read_Valid_Range_Request_Data;
     ESS_Read_Descriptor_Value_Changed_CCCD_Request_Data_t  *ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data;
     ESS_Write_Descriptor_Value_Changed_CCCD_Request_Data_t *ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data;
     ESS_Confirmation_Data_t                                *ESS_Confirmation_Data;
   } Event_Data;
} ESS_Event_Data_t;

#define ESS_EVENT_DATA_SIZE                             (sizeof(ESS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a ESS Service Event Receive Data Callback.  This function will be */
   /* called whenever an ESS Service Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the ESS Event Data that        */
   /* occurred and the ESS Service Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the ESS Service Event Data ONLY in  context   */
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
   /* possible (this argument holds anyway because another ESS Service  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving ESS Service Event   */
   /*            Packets.  A Deadlock WILL occur because NO ESS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *ESS_Event_Callback_t)(unsigned int BluetoothStackID, ESS_Event_Data_t *ESS_Event_Data, unsigned long CallbackParameter);

   /* ESS Server API's.                                                 */

   /* The following function is responsible for opening an ESS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the ESS Service Flags  */
   /* (ESS_SERVICE_FLAGS_XXX) from ESSTypes.h.  These flags MUST be used*/
   /* to register the GATT service for the correct transport.  The third*/
   /* parameter is a pointer to the data that is REQUIRED to configure  */
   /* the service.  The fourth parameter is the Callback function to    */
   /* call when an event occurs on this Server Port.  The fifth         */
   /* parameter is a user-defined callback parameter that will be passed*/
   /* to the callback function with each event.  The final parameter is */
   /* a pointer to store the GATT Service ID of the registered ESS      */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 ESS may be open at a time, per Bluetooth Stack ID.*/
   /* * NOTE * If BR_EDR is not supported by GATT                       */
   /*          (BTPS_CONFIGURATION_GATT_SUPPORT_BR_EDR = 0), and the    */
   /*          ServiceFlags has the ESS_SERVICE_FLAGS_BR_EDR bit set,   */
   /*          then it will be cleared.  If the ESS_SERVICE_FLAGS_LE bit*/
   /*          is still set when this occurs, the service will register */
   /*          for LE ONLY and not for both LE and BR_EDR.  Otherwise   */
   /*          this function will fail.                                 */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * See the ESS_Initialize_Data_t structure above for more   */
   /*          information about the InitializeData parameter.  If this */
   /*          parameter is not configured correctly the service will   */
   /*          FAIL to register.                                        */
BTPSAPI_DECLARATION int BTPSAPI ESS_Initialize_Service(unsigned int BluetoothStackID, unsigned int ServiceFlags, ESS_Initialize_Data_t *InitializeData, ESS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int ServiceFlags, ESS_Initialize_Data_t *InitializeData, ESS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening an ESS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the ESS Service Flags  */
   /* (ESS_SERVICE_FLAGS_XXX) from ESSTypes.h.  These flags MUST be used*/
   /* to register the GATT service for the correct transport.  The third*/
   /* parameter is a pointer to the data that is REQUIRED to configure  */
   /* the service.  The fourth parameter is the Callback function to    */
   /* call when an event occurs on this Server Port.  The fifth         */
   /* parameter is a user-defined callback parameter that will be passed*/
   /* to the callback function with each event.  The sixth parameter is */
   /* a pointer to store the GATT Service ID of the registered ESS      */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 ESS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * If BR_EDR is not supported by GATT                       */
   /*          (BTPS_CONFIGURATION_GATT_SUPPORT_BR_EDR = 0), and the    */
   /*          ServiceFlags has the ESS_SERVICE_FLAGS_BR_EDR bit set,   */
   /*          then it will be cleared.  If the ESS_SERVICE_FLAGS_LE bit*/
   /*          is still set when this occurs, the service will register */
   /*          for LE ONLY and not for both LE and BR_EDR.  Otherwise   */
   /*          this function will fail.                                 */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * See the ESS_Initialize_Data_t structure above for more   */
   /*          information about the InitializeData parameter.  If this */
   /*          parameter is not configured correctly the service will   */
   /*          FAIL to register.                                        */
BTPSAPI_DECLARATION int BTPSAPI ESS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int ServiceFlags, ESS_Initialize_Data_t *InitializeData, ESS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int ServiceFlags, ESS_Initialize_Data_t *InitializeData, ESS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened ESS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successful call to            */
   /* ESS_Initialize_XXX().  This function returns a zero if successful */
   /* or a negative return error code if an error occurs.               */
BTPSAPI_DECLARATION int BTPSAPI ESS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI ESS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_ESS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* ESS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to ESS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI ESS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the ESS Service that is          */
   /* registered with a call to ESS_Initialize_XXX().  The first        */
   /* parameter is the Bluetooth Stack ID on which to close the server. */
   /* The second parameter is the InstanceID that was returned from a   */
   /* successful call to ESS_Initialize_XXX().  This function returns   */
   /* the non-zero number of attributes that are contained in an ESS    */
   /* Server or zero on failure.                                        */
BTPSAPI_DECLARATION unsigned int BTPSAPI ESS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_ESS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Read Characteristic request.  The first parameter is the Bluetooth*/
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* ESS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The fifth       */
   /* parameter is the CharacteristicInfo, which is needed to identify  */
   /* the ESS Characteristic for the response.  The final parameter     */
   /* contains a pointer to the ESS Characteristic data to send to the  */
   /* ESS Client if the request has been accepted.  This function       */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          CharacteristicData parameter is REQUIRED.  The           */
   /*          CharacteristicData parameter may be excluded (NULL) if   */
   /*          the request is rejected.                                 */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Characteristic_Data_t *CharacteristicData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Characteristic_Data_t *CharacteristicData);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD) read request.  The first parameter is the Bluetooth Stack  */
   /* ID of the Bluetooth Device.  The second parameter is the          */
   /* InstanceID returned from a successful call to                     */
   /* ESS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The fifth       */
   /* parameter is the CharacteristicInfo, which is used to identify the*/
   /* ESS Characteristic that contains the Client Characteristic        */
   /* Configuration desccriptor (CCCD).  The final parameter is the     */
   /* ClientConfiguration and is the current Client Characteristic      */
   /* Configuration value to send to the ESS Client if the request has  */
   /* been accepted.  This function returns a zero if successful or a   */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          ClientConfiguration parameter is REQUIRED.  The          */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.  The           */
   /*          ClientConfiguration parameter will be IGNORED if the     */
   /*          request is rejected.                                     */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD) write request.  The first parameter is the Bluetooth Stack */
   /* ID of the Bluetooth Device.  The second parameter is the          */
   /* InstanceID returned from a successful call to                     */
   /* ESS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The fifth       */
   /* parameter is the CharacteristicInfo, which is used to identify the*/
   /* ESS Characteristic that contains the Client Characteristic        */
   /* Configuration desccriptor (CCCD).  This function returns a zero if*/
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS.  The           */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for querying an ESS         */
   /* Characteristic's Extended Properties descriptor.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to ESS_Initialize_XXX().  The fifth parameter is the              */
   /* CharacteristicInfo, which is used to identify the ESS             */
   /* Characteristic that contains the Extended Properties desccriptor. */
   /* The final parameter is a pointer that will hold the extended      */
   /* properties descriptor value if this function is successful.  This */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * If the error code ESS_ERROR_MISSING_EXTENDED_PROPERTIES  */
   /*          is returned then the Extended Properties descriptor does */
   /*          not exist for the specified ESS Characteristic.          */
BTPSAPI_DECLARATION int BTPSAPI ESS_Query_Extended_Properties(unsigned int BluetoothStackID, unsigned int InstanceID, ESS_Characteristic_Info_t *CharacteristicInfo, Word_t *ExtendedProperties);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Query_Extended_Properties_t)(unsigned int BluetoothStackID, unsigned int InstanceID, ESS_Characteristic_Info_t *CharacteristicInfo, Word_t *ExtendedProperties);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's ES Measurement descriptor read request.  The     */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to ESS_Initialize_XXX().  The third parameter is the GATT    */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is needed to send*/
   /* the error response if the request has been rejected.  The final   */
   /* parameter is a pointer to the ES Measurement data to send to the  */
   /* ESS Client if the request has been accepted.  This function       */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          MeasurementData parameter is REQUIRED.  The              */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.  The           */
   /*          MeasurementData parameter may be excluded (NULL) if the  */
   /*          request is rejected.                                     */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_ES_Measurement_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_ES_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_ES_Measurement_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_ES_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's ES Trigger Setting descriptor read request.  The */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to ESS_Initialize_XXX().  The third parameter is the GATT    */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is used to       */
   /* identify the ESS Characteristic that contains the ES Trigger      */
   /* Setting desccriptor.  The sixth parameter is the ES Trigger       */
   /* Setting descriptor instance.  The seventh parameter is a pointer  */
   /* to the ES Trigger Setting data.  This function returns a zero if  */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS.                */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          TriggerSetting parameter may be excluded (NULL).         */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_ES_Trigger_Setting_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_ES_Trigger_Setting_Instance_t Instance, ESS_ES_Trigger_Setting_Data_t *TriggerSetting);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_ES_Trigger_Setting_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_ES_Trigger_Setting_Instance_t Instance, ESS_ES_Trigger_Setting_Data_t *TriggerSetting);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's ES Trigger Setting descriptor write request.  The*/
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to ESS_Initialize_XXX().  The third parameter is the GATT    */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is used to       */
   /* identify the ESS Characteristic that contains the ES Trigger      */
   /* Setting desccriptor.  The final parameter is the ES Trigger       */
   /* Setting instance.  This function returns a zero if successful or a*/
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS.  The           */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Write_ES_Trigger_Setting_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_ES_Trigger_Setting_Instance_t Instance);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Write_ES_Trigger_Setting_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_ES_Trigger_Setting_Instance_t Instance);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's ES Configuration descriptor read request.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to ESS_Initialize_XXX().  The third parameter is the GATT    */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is used to       */
   /* identify the ESS Characteristic that contains the ES Configuration*/
   /* desccriptor.  The final parameter is the Configuration and holds  */
   /* the ES Configuration data to send to the ESS Client if the request*/
   /* has been accepted.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          Configuration parameter is REQUIRED.  The                */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.  This is needed*/
   /*          so that the service can send the error response for the  */
   /*          correct attribute handle associated with the ESS         */
   /*          Characteristic descriptor.  The Configuration parameter  */
   /*          will be IGNORED if the request is rejected.              */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_ES_Configuration_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, Byte_t Configuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_ES_Configuration_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, Byte_t Configuration);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Write ES Configuration descriptor request.  The first parameter is*/
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* ESS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The final       */
   /* parameter is the CharacteristicInfo, which is used to identify the*/
   /* ESS Characteristic that contains the ES Configuration desccriptor.*/
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS.  The           */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Write_ES_Configuration_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Write_ES_Configuration_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's User Description descriptor read request.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to ESS_Initialize_XXX().  The third parameter is the GATT    */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is used to       */
   /* identify the ESS Characteristic that contains the User Description*/
   /* desccriptor.  The final parameter is the UserDescription and holds*/
   /* the User Description to send to the ESS Client if the request has */
   /* been accepted.  This function returns a zero if successful or a   */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          UserDescription parameter is REQUIRED.  The              */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.  The           */
   /*          UserDescription parameter may be excluded (NULL) if the  */
   /*          request is rejected.                                     */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_User_Description_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, char *UserDescription);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_User_Description_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, char *UserDescription);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's User Description descriptor write request.  The  */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to ESS_Initialize_XXX().  The third parameter is the GATT    */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is used to       */
   /* identify the ESS Characteristic that contains the User Description*/
   /* desccriptor.  This function returns a zero if successful or a     */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS.  The           */
   /*          CharacteristicInfo parameter may be excluded (NULL) if   */
   /*          the request is accepted.                                 */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Write_User_Description_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Write_User_Description_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Characteristic's Valid Range descriptor read request.  The first  */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to ESS_Initialize_XXX().  The third parameter is the GATT         */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which is used to       */
   /* identify the ESS Characteristic that contains the Valid Range     */
   /* desccriptor.  The final parameter is the ValidRange and holds the */
   /* data to send to the ESS Client if the request has been accepted.  */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          ValidRange parameter is REQUIRED.                        */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          CharacteristicInfo parameter is REQUIRED.  The ValidRange*/
   /*          parameter may be excluded (NULL) if the request is       */
   /*          rejected.                                                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_Valid_Range_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Valid_Range_Data_t *ValidRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_Valid_Range_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Valid_Range_Data_t *ValidRange);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Read Descriptor Value Changed Client Characteristic Configuration */
   /* descriptor (CCCD) request.  The first parameter is the Bluetooth  */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* ESS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The fifth       */
   /* parameter is the ClientConfiguration and is the current Client    */
   /* Characteristic Configuration value to send to the ESS Client if   */
   /* the request has been accepted.  This function returns a zero if   */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS and the         */
   /*          ClientConfiguration parameter is REQUIRED.               */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS and the   */
   /*          ClientConfiguration parameter will be ignored.           */
BTPSAPI_DECLARATION int BTPSAPI ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to an ESS    */
   /* Write Descriptor Value Changed Client Characteristic Configuration*/
   /* descriptor (CCCD) request.  The first parameter is the Bluetooth  */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* ESS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  This function   */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          ESS_ERROR_CODE_XXX from ESSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be ESS_ERROR_CODE_SUCCESS.                */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except ESS_ERROR_CODE_SUCCESS.          */
BTPSAPI_DECLARATION int BTPSAPI ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for notifying an ESS        */
   /* Characteristic to an ESS Client for the specified GATT Connection */
   /* ID.  The first parameter is the Bluetooth Stack ID of the         */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to ESS_Initialize_XXX() API.  The third    */
   /* parameter is the GATT ConnectionID of the remote device to send   */
   /* the indication to.  The fourth parameter is the                   */
   /* CharacteristicInfo, which is needed to identify the ESS           */
   /* Characteristic for the notification.  The final parameter contains*/
   /* a pointer to the ESS Characteristic data to notify to the ESS     */
   /* Client.  This function will return a non-negative value that      */
   /* represents the actual length of the attribute value that was      */
   /* notified, or a negative return error code if there was an error.  */
   /* * NOTE * This function SHOULD NOT be called unless the ESS Client */
   /*          has configured the CCCD for the ESS Characteristic, for  */
   /*          notifications.                                           */
   /* * NOTE * All parameters for this function are MANDATORY and MUST  */
   /*          be valid.                                                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Notify_Characteristic(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Characteristic_Data_t *CharacteristicData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Notify_Characteristic_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Characteristic_Data_t *CharacteristicData);
#endif

   /* The following function is responsible for indicating the ESS      */
   /* Descriptor Value Changed Characteristic to an ESS Client for the  */
   /* specified GATT Connection ID.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* ESS_Initialize_XXX() API.  The third parameter is the GATT        */
   /* ConnectionID of the remote device to send the indication to.The   */
   /* final parameter holds the data for the Descriptor Value Changed   */
   /* Characteristic to indicate.  This function will return a          */
   /* non-negative value that represents the Transaction ID for the     */
   /* indication, or a negative return error code if there was an error.*/
   /* * NOTE * This function SHOULD NOT be called unless the ESS Client */
   /*          has configured the CCCD for indications.                 */
   /* * NOTE * All parameters for this function are MANDATORY and MUST  */
   /*          be valid.                                                */
BTPSAPI_DECLARATION int BTPSAPI ESS_Indicate_Descriptor_Value_Changed(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, ESS_Descriptor_Value_Changed_Data_t *DescriptorValueChanged);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Indicate_Descriptor_Value_Changed_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, ESS_Descriptor_Value_Changed_Data_t *DescriptorValueChanged);
#endif

   /* ESS Client API's.                                                 */

   /* The following function is responsible for parsing a value received*/
   /* from a remote ESS Server interpreting it as an ESS Characteristic.*/
   /* The first parameter is the length of the value returned by the    */
   /* remote ESS Server.  The second parameter is a pointer to the data */
   /* returned by the remote ESS Server.  The third parameter is the ESS*/
   /* Characteristic Type of the Characteristic Data.  The final        */
   /* parameter is a pointer to store the parsed ESS Characteristc.  The*/
   /* Data field of the CharacteristicData parameter will hold the      */
   /* decoded ESS Characteristic.  This function returns a zero if      */
   /* successful or a negative return error code if an error occurs.    */
   /* ** NOTE ** The Type parameter MUST be set and valid prior to      */
   /*            calling this function based on the attribute handle so */
   /*            the ESS Characteristic can be correctly decoded for the*/
   /*            specified Type.  The ESS Client can determine the      */
   /*            correct Type to assign based on the ESS Characteristic */
   /*            UUID used to store the attribute handle during service */
   /*            discovery.  Therefore, the ESS Client MUST store the   */
   /*            ESS Characteristic Type as well as the attribute handle*/
   /*            during service discovery.                              */
BTPSAPI_DECLARATION int BTPSAPI ESS_Decode_Characteristic(unsigned int ValueLength, Byte_t *Value, ESS_Characteristic_Type_t Type, ESS_Characteristic_Data_t *CharacteristicData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Decode_Characteristic_t)(unsigned int ValueLength, Byte_t *Value, ESS_Characteristic_Type_t Type, ESS_Characteristic_Data_t *CharacteristicData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote ESS Server interpreting it as an ES Measurement     */
   /* descriptor.  The first parameter is the length of the value       */
   /* returned by the remote ESS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote ESS Server.  The final */
   /* parameter is a pointer to store the parsed ES Measurement         */
   /* descriptor.  This function returns a zero if successful or a      */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI ESS_Decode_ES_Measurement(unsigned int ValueLength, Byte_t *Value, ESS_ES_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Decode_ES_Measurement_t)(unsigned int ValueLength, Byte_t *Value, ESS_ES_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for sending a GATT Write    */
   /* request to write an ES Trigger Setting descriptor on a remote ESS */
   /* Server, based on the GATT Connection ID.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the GATT ConnectionID for current connection with the*/
   /* remote ESS Server.  The third parameter is the AttributeHandle for*/
   /* the ES ES Trigger Setting descriptor to write.  The fourth        */
   /* parameter is the ESS Characteristic Type for the ESS              */
   /* Characteristic that contains the ES Trigger Setting descriptor.   */
   /* The fifth parameter is the ES Trigger Setting data that will be   */
   /* requested to be written.  The sixth and seventh parameters are the*/
   /* GATT ClientEventCallback and CallbackParameter that will receive  */
   /* the GATT Write response or GATT error response.  If successful,   */
   /* this function will return the non-zero positive GATT Transaction  */
   /* ID for the GATT Write request, which may be used to cancel the    */
   /* request.  Otherwise this function will return a negative error    */
   /* code.                                                             */
   /* ** NOTE ** The Type parameter MUST be set and valid prior to      */
   /*            calling this function based on the attribute handle so */
   /*            the ES Trigger Setting can be properly formatted for   */
   /*            the specified Type.  The ESS Client can determine the  */
   /*            correct Type to assign based on the ESS Characteristic */
   /*            UUID used to store the attribute handle during service */
   /*            discovery.  Therefore, the ESS Client MUST store the   */
   /*            ESS Characteristic Type as well as the attribute handle*/
   /*            during service discovery.                              */
BTPSAPI_DECLARATION int BTPSAPI ESS_Write_ES_Trigger_Setting_Request(unsigned int BluetoothStackID, unsigned int ConnectionID, Word_t AttributeHandle, ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Data_t *TriggerSetting, GATT_Client_Event_Callback_t ClientEventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Write_ES_Trigger_Setting_Request_t)(unsigned int BluetoothStackID, unsigned int ConnectionID, Word_t AttributeHandle, ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Data_t *TriggerSetting, GATT_Client_Event_Callback_t ClientEventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote ESS Server interpreting it as an ES Trigger Setting */
   /* descriptor.  The first parameter is the length of the value       */
   /* returned by the remote ESS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote ESS Server.  The third */
   /* parameter is the ESS Characteristic Type for the ESS              */
   /* Characteristic that contains the ES Trigger Setting descriptor.   */
   /* The final parameter is a pointer to store the parsed ES Trigger   */
   /* Setting descriptor.  This function returns a zero if successful or*/
   /* a negative return error code if an error occurs.                  */
   /* ** NOTE ** The Type parameter MUST be set and valid prior to      */
   /*            calling this function based on the attribute handle so */
   /*            the ES Trigger Setting can be properly decoded for the */
   /*            specified Type.  The ESS Client can determine the      */
   /*            correct Type to assign based on the ESS Characteristic */
   /*            UUID used to store the attribute handle during service */
   /*            discovery.  Therefore, the ESS Client MUST store the   */
   /*            ESS Characteristic Type as well as the attribute handle*/
   /*            during service discovery.                              */
   /* * NOTE * ESS Barometric Pressure Trend, ESS Magnetic Flux 2D, and */
   /*          ESS Magnetic Flux 3D, and ESS Characteristics that hold  */
   /*          multiple values CANNOT use the specified value conditions*/
   /*          where conditions are between (tscLessThanSpecifiedValue  */
   /*          <= condition <= tscGreaterThanOrEqualSpecifiedValue).  As*/
   /*          such they CANNOT have an Operand field if the condition  */
   /*          field is within the range above.  If the Type and        */
   /*          Condition fields are set so that this requirement does   */
   /*          not hold then this function will return                  */
   /*          ESS_ERROR_INVALID_ES_TRIGGER_SETTING_TYPE_AND_CONDITION. */
   /* * NOTE * If the error code                                        */
   /*          ESS_ERROR_INVALID_ES_TRIGGER_SETTING_CONDITION is        */
   /*          returned then this means that the ESS Client has received*/
   /*          a Reserved for Future Use (RFU) condition.  The ESS      */
   /*          Client needs to be tolerant of receiving RFU conditions, */
   /*          however this function will still return an error code.   */
   /*          The Operand field of the TriggerSetting parameter will   */
   /*          not be valid.                                            */
BTPSAPI_DECLARATION int BTPSAPI ESS_Decode_ES_Trigger_Setting(unsigned int ValueLength, Byte_t *Value, ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Data_t *TriggerSetting);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Decode_ES_Trigger_Setting_t)(unsigned int ValueLength, Byte_t *Value, ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Data_t *TriggerSetting);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote ESS Server interpreting it as the Valid Range       */
   /* descriptor.  The first parameter is the length of the value       */
   /* returned by the remote ESS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote ESS Server.  The third */
   /* parameter is the ESS Characteristic Type for the ESS              */
   /* Characteristic that contains the Valid Range descriptor.  The     */
   /* final parameter is a pointer to store the parsed Valid Range      */
   /* descriptor.  This function returns a zero if successful or a      */
   /* negative return error code if an error occurs.                    */
   /* ** NOTE ** The Type parameter MUST be set and valid prior to      */
   /*            calling this function based on the attribute handle so */
   /*            the Valid Range can be correctly decoded for the       */
   /*            specified Type.  The ESS Client can determine the      */
   /*            correct Type to assign based on the ESS Characteristic */
   /*            UUID used to store the attribute handle during service */
   /*            discovery.  Therefore, the ESS Client MUST store the   */
   /*            ESS Characteristic Type as well as the attribute handle*/
   /*            during service discovery.                              */
BTPSAPI_DECLARATION int BTPSAPI ESS_Decode_Valid_Range(unsigned int ValueLength, Byte_t *Value, ESS_Characteristic_Type_t Type, ESS_Valid_Range_Data_t *ValidRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Decode_Valid_Range_t)(unsigned int ValueLength, Byte_t *Value, ESS_Characteristic_Type_t Type, ESS_Valid_Range_Data_t *ValidRange);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote ESS Server interpreting it as the Descriptor Value  */
   /* Changed Characteristic.  The first parameter is the length of the */
   /* value returned by the remote ESS Server.  The second parameter is */
   /* a pointer to the data returned by the remote ESS Server.  The     */
   /* final parameter is a pointer to store the parsed Descriptor Value */
   /* Changed Characteristic.  This function returns a zero if          */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI ESS_Decode_Descriptor_Value_Changed(unsigned int ValueLength, Byte_t *Value, ESS_Descriptor_Value_Changed_Data_t *DescriptorValueChanged);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ESS_Decode_Descriptor_Value_Changed_t)(unsigned int ValueLength, Byte_t *Value, ESS_Descriptor_Value_Changed_Data_t *DescriptorValueChanged);
#endif

#endif

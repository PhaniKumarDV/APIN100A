/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< aiosapi.h >************************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AIOSAPI - Qualcomm Technologies Bluetooth Automation IO Service (GATT     */
/*            based) Type Definitions, Prototypes, and Constants.             */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/14/15  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __AIOSAPIH__
#define __AIOSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "AIOSTypes.h"     /* Automation IO Service Types/Constants           */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define AIOS_ERROR_INVALID_PARAMETER                      (-1000)
#define AIOS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define AIOS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define AIOS_ERROR_SERVICE_ALREADY_REGISTERED             (-1003)
#define AIOS_ERROR_INVALID_INSTANCE_ID                    (-1004)
#define AIOS_ERROR_MALFORMATTED_DATA                      (-1005)
#define AIOS_ERROR_INSUFFICIENT_BUFFER_SPACE              (-1006)

   /* Error codes return from AIOS_Initialize_Service() and             */
   /* AIOS_Initialize_Service_Handle_Range() functions if the           */
   /* AIOS_Initialize_Data_t structure parameter is invalid.            */
   /* * NOTE * Other error codes may also be returned.                  */
#define AIOS_ERROR_INVALID_NUMBER_OF_ENTRIES              (-1007)
#define AIOS_ERROR_INVALID_NUMBER_OF_INSTANCES            (-1008)
#define AIOS_ERROR_CHARACTERISTIC_TYPE_ALREADY_REGISTERED (-1009)
#define AIOS_ERROR_INVALID_CHARACTERISTIC_PROPERTY_FLAG   (-1010)
#define AIOS_ERROR_INVALID_DESCRIPTOR_FLAG                (-1011)
#define AIOS_ERROR_INVALID_AGGREGATE_PROPERTY_FLAG        (-1012)

   /* Error codes returned from AIOS API's in addition to common error  */
   /* codes above.                                                      */
#define AIOS_ERROR_INVALID_CHARACTERISTIC_TYPE            (-1013)
#define AIOS_ERROR_INVALID_IO_TYPE                        (-1014)
#define AIOS_ERROR_INVALID_CHARACTERISTIC_INSTANCE_ID     (-1015)
#define AIOS_ERROR_ATTRIBUTE_HANDLE_INFORMATION_NOT_FOUND (-1016)
#define AIOS_ERROR_INVALID_CONDITION_CODE_VALUE           (-1017)
#define AIOS_ERROR_CHARACTERISTIC_MTU_SIZE_EXCEEDED       (-1018)
#define AIOS_ERROR_AGGREGATE_NOT_SUPPORTED                (-1019)
#define AIOS_ERROR_AGGREGATE_CCCD_NOT_SUPPORTED           (-1020)

   /* The following enumeration defines the AIOS Characteristic types.  */
typedef enum
{
   actDigital,
   actAnalog,
   actAggregate
} AIOS_Characteristic_Type_t;

   /* The following enumeration defines the Input/Output type for an    */
   /* AIOS Characteristic.                                              */
   /* * NOTE * Digital and Analog Characteristics may only use the      */
   /*          ioInput and ioOutput enumerations since they CANNOT be   */
   /*          both an input and an output.                             */
   /* * NOTE * The ioInputOutput will be set automatically for the      */
   /*          Aggregate Characteristic if the Aggregate Characteristic */
   /*          is made up of Digital and Analog Characterics that have  */
   /*          both the ioInput and ioOutput enumerations.  If the      */
   /*          Digital and Analog Characteristics that make up the      */
   /*          Aggregate Characteristic have either the ioInput or      */
   /*          ioOutput enumeration then that enumeration will be set   */
   /*          for the Aggregate Characteristic.                        */
typedef enum
{
   ioInput,
   ioOutput,
   ioInputOutput
} AIOS_IO_Type_t;

   /* The following defines the optional AIOS Characteristic properties */
   /* that may be set for Digital/Analog Input Characteristic instances.*/
   /* These bit mask values may be set for the                          */
   /* Characteristic_Property_Flags field of the                        */
   /* AIOS_Characteristic_Instance_Entry_t structure if the IOType field*/
   /* is set to ioInput.                                                */
   /* ** NOTE ** If the IOType field of the                             */
   /*            AIOS_Characteristic_Instance_Entry_t structure is set  */
   /*            to ioInput and any other GATT Characteristic Properties*/
   /*            are detected other than the ones below, the            */
   /*            AIOS_Initialize_XXX() API's will return the            */
   /*            AIOS_ERROR_INVALID_CHARACTERISTIC_PROPERTY_FLAG error. */
   /* * NOTE * The Read property is Mandatory for Digital/Analog        */
   /*          Characteristic Input instances and will automatically be */
   /*          set when the service is initialized.                     */
   /* * NOTE * The AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE and*/
   /*          AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY flags    */
   /*          CANNOT be specified simultaneously.                      */
   /* * NOTE * The AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE and*/
   /*          AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY flags    */
   /*          CANNOT be specified if the Aggregate Characteristic is   */
   /*          supported.                                               */
   /* * NOTE * A Client Characteristic Configuration Descriptor (CCCD)  */
   /*          will automatically be included when the service is       */
   /*          initialized if either the                                */
   /*          AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE or     */
   /*          AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY flag is  */
   /*          specified.                                               */
#define AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE                (GATT_CHARACTERISTIC_PROPERTIES_INDICATE)
#define AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY                  (GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)

   /* The following defines the optional AIOS Characteristic properties */
   /* that may be set for Digital/Analog Output Characteristic          */
   /* instances.  These bit mask values may be set for the              */
   /* Characteristic_Property_Flags field of the                        */
   /* AIOS_Characteristic_Instance_Entry_t structure if the IOType field*/
   /* is set to ioOutput.                                               */
   /* ** NOTE ** If the IOType field of the                             */
   /*            AIOS_Characteristic_Instance_Entry_t structure is set  */
   /*            to ioOutput and any other GATT Characteristic          */
   /*            Properties are detected other than the ones below, the */
   /*            AIOS_Initialize_XXX() API's will return the            */
   /*            AIOS_ERROR_INVALID_CHARACTERISTIC_PROPERTY_FLAG error. */
   /* * NOTE * The AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE and  */
   /*          AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT..*/
   /*          ..._RESPONSE properties are MANDATORY for Digital/Analog */
   /*          Output Characteristic instances and at least one MUST be */
   /*          specified.                                               */
   /* * NOTE * The AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE   */
   /*          and AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY     */
   /*          flags CANNOT be specified if                             */
   /*          AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_READ is not    */
   /*          specified.                                               */
   /* * NOTE * The AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE   */
   /*          and AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY     */
   /*          flags cannot be specified simultaneously.                */
   /* * NOTE * The AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE   */
   /*          and AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY     */
   /*          flags CANNOT be specified if the Aggregate Characteristic*/
   /*          is supported.                                            */
   /* * NOTE * A Client Characteristic Configuration Descriptor (CCCD)  */
   /*          will automatically be included when the service is       */
   /*          initialized if either the                                */
   /*          AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE or    */
   /*          AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY flag is */
   /*          specified.                                               */
#define AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE                  (GATT_CHARACTERISTIC_PROPERTIES_WRITE)
#define AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE (GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE)
#define AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_READ                   (GATT_CHARACTERISTIC_PROPERTIES_READ)
#define AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE               (GATT_CHARACTERISTIC_PROPERTIES_INDICATE)
#define AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY                 (GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)

   /* The following defines the optional AIOS Aggregate Characteristic  */
   /* properties that may be set.  These bit mask values may be set for */
   /* the Aggregate_Property_Flags field of the AIOS_Initialize_Data_t  */
   /* structure.                                                        */
   /* * NOTE * The Read property is Mandatory for the Aggregate         */
   /*          Characteristic and will automatically be set.            */
   /* * NOTE * The AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE and*/
   /*          AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY cannot be*/
   /*          set simultaneously.                                      */
   /* * NOTE * A Client Characteristic Configuration Descriptor (CCCD)  */
   /*          will automatically be included when the service is       */
   /*          initialized if either the                                */
   /*          AIOS_AGGREGATE_PROPERTY_FLAGS_INDICATE or                */
   /*          AIOS_AGGREGATE_PROPERTY_FLAGS_NOTIFY flag is specified.  */
#define AIOS_AGGREGATE_PROPERTY_FLAGS_INDICATE                           (GATT_CHARACTERISTIC_PROPERTIES_INDICATE)
#define AIOS_AGGREGATE_PROPERTY_FLAGS_NOTIFY                             (GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)

   /* The following defines the descriptor flags that may be used to    */
   /* include optional descriptors for AIOS Digital/Analog              */
   /* Characteristic instances.  These bit mask values may be set for   */
   /* the Descriptor_Flags field of the                                 */
   /* AIOS_Characteristic_Instance_Entry_t structure.                   */
   /* ** NOTE ** The AIOS_DESCRIPTOR_FLAG_VALUE_TRIGGER_SETTING and     */
   /*            AIOS_DESCRIPTOR_FLAG_TIME_TRIGGER_SETTING may not be   */
   /*            specfied if the Indicate or Notify Property is not     */
   /*            supported for the AIOS Digital/Analog Characteristic   */
   /*            instance.  However, if the Aggregate Characteristic is */
   /*            supported, and it has either the Notify or Indicate    */
   /*            property then the                                      */
   /*            AIOS_DESCRIPTOR_FLAG_VALUE_TRIGGER_SETTING and         */
   /*            AIOS_DESCRIPTOR_FLAG_TIME_TRIGGER_SETTING may be       */
   /*            specified.                                             */
   /* * NOTE * The AIOS_DESCRIPTOR_FLAGS_PRESENTATION_FORMAT may only be*/
   /*          specified if there is one instance specified by          */
   /*          Number_Of_Instances field of the                         */
   /*          AIOS_Characteristic_Entry_t structure.  The Presentation */
   /*          Format descriptor will automatically be included when the*/
   /*          service is initialized if there is more than one         */
   /*          instance.                                                */
   /* * NOTE * The AIOS_DESCRIPTOR_FLAG_VALUE_TRIGGER_SETTING and       */
   /*          AIOS_DESCRIPTOR_FLAG_TIME_TRIGGER_SETTING may NOT be     */
   /*          included if the Use_Custom_Trigger field of the          */
   /*          AIOS_Characteristic_Instance_Entry_t structure is TRUE.  */
   /* * NOTE * The AIOS_DESCRIPTOR_FLAG_TIME_TRIGGER_SETTING may not be */
   /*          specified if the                                         */
   /*          AIOS_DESCRIPTOR_FLAG_VALUE_TRIGGER_SETTING is not        */
   /*          specified.  This is required since the Time Trigger      */
   /*          Setting descriptor may be set such that a notification or*/
   /*          indication may only be sent once the Value Trigger       */
   /*          Setting descriptor's trigger condition has been triggered*/
   /*          a specified number of times.                             */
   /* * NOTE * Digital Characteristic instances CANNOT use the optional */
   /*          AIOS_DESCRIPTOR_FLAG_VALID_RANGE flag since this flag is */
   /*          only valid for Analog Characteristic instances.          */
#define AIOS_DESCRIPTOR_FLAGS_PRESENTATION_FORMAT                 (0x01)
#define AIOS_DESCRIPTOR_FLAGS_USER_DESCRIPTION                    (0x02)
#define AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING               (0x04)
#define AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING                (0x08)
#define AIOS_DESCRIPTOR_FLAGS_VALID_RANGE                         (0x10)

   /* The following defines the descriptor property flags that may be   */
   /* set for descriptors that are included for a Digital/Analog        */
   /* Characteristic instance.  These bit mask values may be set for the*/
   /* Descriptor_Property_Flags field of the                            */
   /* AIOS_Characteristic_Instance_Entry_t structure.                   */
   /* * NOTE * The AIOS_DESCRIPTOR_FLAG_USER_DESCRIPTION flag MUST be   */
   /*          specfied for the Descriptor_Flags field of the           */
   /*          AIOS_Characteristic_Instance_Entry_t structure for the   */
   /*          AIOS_DESCRIPTOR_PROPERTY_FLAGS_USER_DESCRIPTION_WRITABLE */
   /*          flag to be valid.                                        */
   /* * NOTE * If the                                                   */
   /*          AIOS_DESCRIPTOR_PROPERTY_FLAGS_USER_DESCRIPTION_WRITABLE */
   /*          is specified, and the above note holds, then the Extended*/
   /*          Properties descriptor will automatically be included in  */
   /*          the service when it is initialized, and the 'Write       */
   /*          Auxiliries' bit will automatically be set.               */
#define AIOS_DESCRIPTOR_PROPERTY_FLAGS_USER_DESCRIPTION_WRITABLE  (0x01)

   /* The following structure defines the information that is needed to */
   /* initialize an AIOS Digital/Analog Characteristic instance.        */
   /* ** NOTE ** The IOType field specifies whether the AIOS            */
   /*            Digital/Analog Characteristic instance is an           */
   /*            input(ioInput) or output(ioOutput) signal.  This is    */
   /*            important since the Characteristic_Property_Flags MUST */
   /*            be set by values from AIOS_OUTPUT_CHARACTERISTIC_XXX or*/
   /*            AIOS_INPUT_CHARACTERISTIC_XXX depending on the         */
   /*            enumeration selected for the IO_Type field.            */
   /* ** NOTE ** The ioInputOutput enumeration is NOT VALID for the     */
   /*            IOType field since it is only informative and used with*/
   /*            the Aggregate Characteristic.                          */
   /* * NOTE * See the AIOS_DESCRIPTOR_FLAG_XXX above for more          */
   /*          information about valid bit mask values that may be set  */
   /*          for the Descriptor_Flags field.                          */
   /* * NOTE * See the AIOS_DESCRIPTOR_PROPERTY_FLAG_XXX above for more */
   /*          information about valid bit mask values that may be set  */
   /*          for the Descriptor_Property_Flags field.                 */
   /* * NOTE * The Use_Custom_Trigger allows the application to have a  */
   /*          custom condition that triggers a notification or         */
   /*          indication of a Digital/Analog Characteristic instance if*/
   /*          set to TRUE.  This also means that the                   */
   /*          AIOS_DESCRIPTOR_FLAG_VALUE_TRIGGER_SETTING and           */
   /*          AIOS_DESCRIPTOR_FLAG_TIME_TRIGGER_SETTING values for the */
   /*          Descriptor_Flags field CANNOT be specified since they are*/
   /*          replaced by the custom condition.                        */
typedef struct _tagAIOS_Characteristic_Instance_Entry_t
{
   AIOS_IO_Type_t  IO_Type;
   Byte_t          Characteristic_Property_Flags;
   Byte_t          Descriptor_Flags;
   Byte_t          Descriptor_Property_Flags;
   Boolean_t       Use_Custom_Trigger;
} AIOS_Characteristic_Instance_Entry_t;

#define AIOS_CHARACTERISTIC_INSTANCE_ENTRY_SIZE                (sizeof(AIOS_Characteristic_Instance_Entry_t))

   /* The following structure defines the information that is needed to */
   /* initialize an AIOS Digital/Analog Characteristic and all of its   */
   /* instances.                                                        */
   /* ** NOTE ** The Type field MUST be unique for each                 */
   /*            AIOS_Characteristic_Entry_t structure included in the  */
   /*            Entries field of the AIOS_Initialize_Data_t structure  */
   /*            since all Digital/Analog Characteristic instances MUST */
   /*            be initialized together.                               */
   /* * NOTE * The Type field is an enumeration that is used to identify*/
   /*          whether the AIOS Characteristic instances are            */
   /*          Digital(actDigital) or Analog(actAnalog) Characteristic  */
   /*          instances.  The actAggregate enumeration may not be used */
   /*          for this field since the Aggregate is initialized        */
   /*          separately from Digital/Analog Characteristic instances. */
   /* * NOTE * The Presentation Format descriptor will automatically be */
   /*          included when the service is initialized for each        */
   /*          Characteristic instance if the Number_Of_Instances field */
   /*          is greater than 1.  Otherwise it will be excluded for    */
   /*          only one instance unless the                             */
   /*          AIOS_DESCRIPTOR_FLAGS_PRESENTATION_FORMAT flag is        */
   /*          specified in the Descriptor_Flags field of the           */
   /*          AIOS_Characteristic_Instance_Entry_t structure.          */
   /* * NOTE * If the Type field is (actDigital) then the Mandatory     */
   /*          Number of Digitals descriptor will automatically be      */
   /*          included when the service is initialized for all Digital */
   /*          Characteristic instances.                                */
typedef struct _tagAIOS_Characteristic_Entry_t
{
   AIOS_Characteristic_Type_t            Type;
   Word_t                                Number_Of_Instances;
   AIOS_Characteristic_Instance_Entry_t *Instances;
} AIOS_Characteristic_Entry_t;

#define AIOS_CHARACTERISTIC_ENTRY_SIZE                         (sizeof(AIOS_Characteristic_Entry_t))

   /* The following structure defines the information needed to         */
   /* initialize the AIOS Server.                                       */
   /* ** NOTE ** If the Aggregate Characteristic is supported, then all */
   /*            fields of the AIOS_Characteristic_Instance_Entry_t     */
   /*            stucture that have requirements based on whether the   */
   /*            Aggregate Characteristic is supported MUST be met.     */
   /* * NOTE * The Number_Of_Entries field CANNOT be greater than 2     */
   /*          since this structure will ONLY ALLOW a                   */
   /*          AIOS_Characteristic_Entry_t structure for a Digital      */
   /*          Characteristic and all of its instances or an Analog     */
   /*          Characteristic and all of its instances.                 */
   /* * NOTE * The Aggregate_Supported field will flag that the         */
   /*          Aggregate Characteristic will be included automatically  */
   /*          in the service when it is initialized.                   */
   /* * NOTE * Please see the AIOS_AGGREGATE_PROPERTY_XXX above for     */
   /*          information about valid values for the                   */
   /*          Aggregate_Property_Flags field.                          */
typedef struct _tagAIOS_Initialize_Data_t
{
   Byte_t                       Number_Of_Entries;
   AIOS_Characteristic_Entry_t *Entries;
   Boolean_t                    Aggregate_Supported;
   Byte_t                       Aggregate_Property_Flags;
} AIOS_Initialize_Data_t;

#define AIOS_INITIALIZE_DATA_SIZE                              (sizeof(AIOS_Initialize_Data_t))

   /* The following structure contains the format for a Digital         */
   /* Characteristic.  This structure will simply contain a buffer for  */
   /* all the digital signals supported by the Digital Characteristic   */
   /* instance.  The Buffer field is needed since a Digital             */
   /* Characteristic is made up of digital signals (2-bit values in     */
   /* little-endian order) and may span over many bytes depending on the*/
   /* number of digital signals that is supported by the Digital        */
   /* Characteristic instance.                                          */
   /* ** NOTE ** The AIOS Server should call the                        */
   /*            GATT_Change_Maximum_Supported_MTU() function prior to  */
   /*            initializing the service via a call to                 */
   /*            AIOS_Initialize_XXX(), to set an MTU that is large     */
   /*            enough to support the Digital Characteristic instances */
   /*            and the Aggregate Characteristic.  The AIOS Client MUST*/
   /*            call the GATT_Exchange_MTU_Request() function to       */
   /*            negotiate a higher MTU once a connection has been      */
   /*            established.  The smaller supported MTU by the AIOS    */
   /*            Server and AIOS Client will ALWAYS be the negotiated   */
   /*            MTU.                                                   */
   /* ** NOTE ** The Length field CANNOT be greater than (ATT_MTU-3).   */
   /*            This restriction is imposed by the service             */
   /*            specification and it is the application's              */
   /*            responsibility to ensure that the Length field meets   */
   /*            this requirement.  Otherwise any API that sends a      */
   /*            Digital Characteristic instance will FAIL and the error*/
   /*            AIOS_ERROR_CHARACTERISTIC_MTU_SIZE_EXCEEDED will be    */
   /*            returned.                                              */
   /* * NOTE * The Length field is included to indicate the size of the */
   /*          Buffer, however this does not indicate the number of     */
   /*          digital signals.  The number of digital signals can only */
   /*          be determined from the Number Of Digitals descriptor that*/
   /*          is Mandatory for each Digital Characteristic instance.   */
   /*          The Length field can simply be determined by taking the  */
   /*          Number of Digitals and dividing by four (adding an extra */
   /*          byte for the remainder).  The value of any digital       */
   /*          signals (2-bit values) that have been padded to the extra*/
   /*          byte for a Digital Characteristic instance MUST be       */
   /*          included, however they have no meaning since they are    */
   /*          beyond the number of digital signals supported for a     */
   /*          Digital Characteristic instance.  Therefore, the AIOS    */
   /*          Client CANNOT read/write a Digital Characteristic        */
   /*          instance without first reading it's Number Of Digitals   */
   /*          descriptor to determine the number of digital signals and*/
   /*          any padded bits that have no meaning.                    */
   /* * NOTE * Each digital signal (2-bit value) has the following      */
   /*          definition:                                              */
   /*                                                                   */
   /*            0b00 - Inactive state                                  */
   /*            0b01 - Active state                                    */
   /*            0b10 - Tri-state (if supported by AIOS Server)         */
   /*            0b11 - Unknown state                                   */
   /*                                                                   */
   /* * NOTE * If the Unknown state is received in a write operation    */
   /*          (Outputs ONLY), the server SHALL NOT update the          */
   /*          corresponding output.  This value may be sent in a read, */
   /*          notify, or indicate operation to indicate that the AIOS  */
   /*          Server cannot report the value for this particular       */
   /*          Digital Characteristic instance.                         */
   /* * NOTE * Each byte in the Buffer contains 4 digital signals (2-bit*/
   /*          values).  This can be represented by the following, where*/
   /*          AA is the first digital signal or least signifigant      */
   /*          digital signal (2-bit value):                            */
   /*                                                                   */
   /*                 MSb    ... DD_CC_BB_AA ...    LSb                 */
   /*                                                                   */
   /* * NOTE * Example - If the Digital Characteristic instance's Number*/
   /*          Of Digitals descriptor has 5 digital signals, the Length */
   /*          field will be 2 since a single byte holds 4 digital      */
   /*          signals (four 2-bit values).  We will have another byte  */
   /*          that contains one digital signal (at the least           */
   /*          signifigant 2-bit value which can be see above as AA),   */
   /*          with the remaining bits (BB, CC, and DD) padded and      */
   /*          having no meaning.                                       */
typedef struct _tagAIOS_Digital_Characteristic_Data_t
{
   Word_t  Length;
   Byte_t *Buffer;
} AIOS_Digital_Characteristic_Data_t;

#define AIOS_DIGITAL_CHARACTERISTIC_DATA_SIZE                  (sizeof(AIOS_Digital_Characteristic_Data_t))

   /* The following structure defines the format for AIOS Aggregate     */
   /* Characteristic data.  This structure will simply contain a buffer */
   /* to hold all Digital Characteristic instances and all Analog       */
   /* Characteristic instances that have been formatted for the         */
   /* Aggregate Characteristic.                                         */
   /* ** NOTE ** The AIOS Server should call the                        */
   /*            GATT_Change_Maximum_Supported_MTU() function prior to  */
   /*            initializing the service via a call to                 */
   /*            AIOS_Initialize_XXX(), to set an MTU that is large     */
   /*            enough to support the Digital Characteristic instances */
   /*            and the Aggregate Characteristic.  The AIOS Client MUST*/
   /*            call the GATT_Exchange_MTU_Request() function to       */
   /*            negotiate a higher MTU once a connection has been      */
   /*            established.  The smaller supported MTU by the AIOS    */
   /*            Server and AIOS Client will ALWAYS be the negotiated   */
   /*            MTU.                                                   */
   /* ** NOTE ** The Length field CANNOT be greater than (ATT_MTU-3).   */
   /*            This restriction is imposed by the service             */
   /*            specification and it is the application's              */
   /*            responsibility to ensure that the Length field meets   */
   /*            this requirement.  Otherwise any API that sends the    */
   /*            Aggregate Characteristic will FAIL and the error       */
   /*            AIOS_ERROR_CHARACTERISTIC_MTU_SIZE_EXCEEDED will be    */
   /*            returned.                                              */
   /* ** NOTE ** All Digital Characteristic instances included in the   */
   /*            Aggregate Characteristic MUST be formatted into the    */
   /*            Buffer field first, followed by All Analog             */
   /*            Characteristic instances included in the Aggregate     */
   /*            Characteristic.                                        */
   /* ** NOTE ** All Analog Characteristic instances MUST be formatted  */
   /*            in little-endian order.                                */
   /* ** NOTE ** All requirements for Digital Characteristic instances  */
   /*            found in the AIOS_Digital_Characteristic_Data_t        */
   /*            structure above MUST be followed.  This includes how   */
   /*            Digital Characteristic instances are formatted into the*/
   /*            Buffer field for the Aggregate Characteristic.  Each   */
   /*            Digital Characteristic instance included in the        */
   /*            Aggregate Characteristic will have its length added to */
   /*            the Length field.  This length includes the size needed*/
   /*            to hold a Digital Characteristic instance's digital    */
   /*            signals (2-bit values in little-endian order) and any  */
   /*            padded 2-bit values that may span over multiple octets.*/
   /*            The Length field of the                                */
   /*            AIOS_Digital_Characteristic_Data_t structure directly  */
   /*            corresponds to the length that MUST be added for each  */
   /*            Digital Characteristic instance included in the        */
   /*            Aggregate Characteristic.                              */
   /* ** NOTE ** Since a Digital Characteristic instance has a variable */
   /*            length and the Aggregate Characteristic is made up of  */
   /*            Digital Characteristic instances, as well as Analog    */
   /*            Characteristic instances that have the read property,  */
   /*            the AIOS Server and AIOS Client should use the Number  */
   /*            Of Digitals descriptor to determine the length of each */
   /*            Digital Characteristic instance.  The AIOS Client      */
   /*            CANNOT decode an Aggregate Characteristic that has been*/
   /*            read without first reading each Digital Characteristic */
   /*            instance's Number Of Digitals descriptor prior to      */
   /*            issuing a read request for the Aggregate               */
   /*            Characteristic.  This is so that the AIOS Client can   */
   /*            determine the length of each Digital Characteristic    */
   /*            instance included in the Aggregate Characteristic so   */
   /*            that the AIOS Client can properly separate each Digital*/
   /*            Characteristic instance included in the Aggregate      */
   /*            Characteristic.  The AIOS Client should already know if*/
   /*            the Aggregate Characteristic is supported by the AIOS  */
   /*            Server and if a Digital Characteristic is included the */
   /*            Aggregate Characteristic since service discovery MUST  */
   /*            be performed prior to reading a characteristic or a    */
   /*            descriptor.  The Digital Characteristic instance's     */
   /*            properties are included in service discovery and the   */
   /*            AIOS Client can quickly determine if it is in the      */
   /*            Aggregate by checking the read property.               */
typedef struct _tagAIOS_Aggregate_Characteristic_Data_t
{
   Word_t  Length;
   Byte_t *Buffer;
} AIOS_Aggregate_Characteristic_Data_t;

#define AIOS_AGGREGATE_CHARACTERISTIC_DATA_SIZE                (sizeof(AIOS_Aggregate_Characteristic_Data_t))

   /* The following union defines the data types that may be used for   */
   /* AIOS Characteristics.                                             */
typedef union _tagAIOS_Characteristic_Data_t
{
   AIOS_Digital_Characteristic_Data_t    Digital;
   Word_t                                Analog;
   AIOS_Aggregate_Characteristic_Data_t  Aggregate;
} AIOS_Characteristic_Data_t;

   /* The following structure defines the format for the Presentation   */
   /* Format descriptor.  Some of these possible values for fields below*/
   /* can be found in AIOSTypes.h and have the following form           */
   /* AIOS_XXX_PRESENTATION_FORMAT_XXX.                                 */
   /* * NOTE * The Format field determines how a single value contained */
   /*          in the Characteristic Value is formatted.                */
   /* * NOTE * The Exponent field is used with interger data types to   */
   /*          determine how the Characteristic value is further        */
   /*          formatted.  The actual value = Characteristic Value *    */
   /*          10^Exponent.                                             */
   /* * NOTE * Unit specifies unit of measurement for this AIOS         */
   /*          Characteristic.                                          */
   /* * NOTE * The Name Space field is used to indentify the            */
   /*          organization that is responsible for defining the        */
   /*          enumerations for the description field.                  */
   /* * NOTE * The Description is an enumerated value that is used to   */
   /*          uniquely identify a Characteristic instance from another */
   /*          Characteristic instance.  Digital Characteristic         */
   /*          instances and Analog Characteristic instances that       */
   /*          include the Presentation Format descriptor SHALL have    */
   /*          independent Description values.  Values should start at  */
   /*          0x0001 and go upward.  It is worth noting that the AIOS  */
   /*          Server will also assign a unique ID for each Digital or  */
   /*          Analog Characteristic instance that starts at zero.  This*/
   /*          ID will be used in AIOS Server events that contain the   */
   /*          AIOS_Characteristic_Info_t structure to identify the     */
   /*          Characteristic instance.  This value will be assigned    */
   /*          implicitly in the order that Characteristic instances are*/
   /*          initialized and will ALWAYS correspond to 1 less than the*/
   /*          Description field of that Characteristic instance's      */
   /*          Presentation Format descriptor's description field.      */
typedef struct _tagAIOS_Presentation_Format_Data_t
{
  Byte_t  Format;
  Byte_t  Exponent;
  Word_t  Unit;
  Byte_t  NameSpace;
  Word_t  Description;
} AIOS_Presentation_Format_Data_t;

#define AIOS_PRESENTATION_FORMAT_DATA_SIZE                     (sizeof(AIOS_Presentation_Format_Data_t))

   /* The following structure defines the optional Valid Range          */
   /* descriptor that may be used to expose a valid range for an Analog */
   /* Characteristic instance.                                          */
typedef struct _tagAIOS_Valid_Range_Data_t
{
   Word_t  LowerBound;
   Word_t  UpperBound;
} AIOS_Valid_Range_Data_t;

#define AIOS_VALID_RANGE_DATA_SIZE                             (sizeof(AIOS_Valid_Range_Data_t))

  /* The following defines enumeration values for the possible Condition*/
  /* field of the AIOS_Value_Trigger_Data_t structure.                  */
typedef enum
{
   vttStateChanged                  = AIOS_VALUE_TRIGGER_SETTING_CONDITION_STATE_CHANGED,
   vttCrossedBoundaryAnalogValue    = AIOS_VALUE_TRIGGER_SETTING_CONDITION_CROSSED_BOUNDARY_ANALOG_VALUE,
   vttOnBoundaryAnalogValue         = AIOS_VALUE_TRIGGER_SETTING_CONDITION_ON_BOUNDARY_ANALOG_VALUE,
   vttStateChangedAnalogValue       = AIOS_VALUE_TRIGGER_SETTING_CONDITION_STATE_CHANGED_ANALOG_VALUE,
   vttDigitalStateChangedBitMask    = AIOS_VALUE_TRIGGER_SETTING_CONDITION_DIGITAL_STATE_CHANGED_BIT_MASK,
   vttCrossedBoundaryAnalogInterval = AIOS_VALUE_TRIGGER_SETTING_CONDITION_CROSSED_BOUNDARY_ANALOG_INTERVAL,
   vttOnBoundaryAnalogInterval      = AIOS_VALUE_TRIGGER_SETTING_CONDITION_ON_BOUNDARY_ANALOG_INTERVAL,
   vttNoValueTrigger                = AIOS_VALUE_TRIGGER_SETTING_CONDITION_NO_VALUE_TRIGGER
} AIOS_Value_Trigger_Type_t;

   /* The following structure defines the optional Value Trigger Setting*/
   /* descriptor that may be used for Digital/Analog Characteristic     */
   /* instances.                                                        */
typedef struct _tagAIOS_Value_Trigger_Data_t
{
   AIOS_Value_Trigger_Type_t             Condition;
   union
   {
     AIOS_Digital_Characteristic_Data_t  BitMask;
     Word_t                              AnalogValue;
     AIOS_Valid_Range_Data_t             AnalogInterval;
   } ComparisonValue;
} AIOS_Value_Trigger_Data_t;

#define AIOS_VALUE_TRIGGER_DATA_SIZE                              (sizeof(AIOS_Value_Trigger_Data_t))

   /* The following structure defines the time interval comparison value*/
   /* for the optional Time Trigger Setting descriptor.  This is a      */
   /* UINT24 value in seconds.                                          */
typedef struct _tagAIOS_Time_Interval_t
{
   Byte_t  Upper;
   Word_t  Lower;
} AIOS_Time_Interval_t;

#define AIOS_TIME_INTERVAL_DATA_SIZE                              (sizeof(AIOS_Time_Interval_t))

  /* The following defines enumeration values for the possible Condition*/
  /* field of the AIOS_Time_Trigger_Data_t structure.                   */
typedef enum
{
   tttNoTimeBasedTrigger             = AIOS_TIME_TRIGGER_SETTING_CONDITION_NO_TIME_BASED_TRIGGERING_USED,
   tttTimeIntervalIgnoreValueTrigger = AIOS_TIME_TRIGGER_SETTING_CONDITION_TIME_INTERVAL_IGNORE_VALUE_TRIGGER,
   tttTimeIntervalCheckValueTrigger  = AIOS_TIME_TRIGGER_SETTING_CONDITION_TIME_INTERVAL_CHECK_VALUE_TRIGGER,
   tttCountChangedMoreOftenThan      = AIOS_TIME_TRIGGER_SETTING_CONDITION_COUNT_CHANGED_MORE_OFTEN_THAN
} AIOS_Time_Trigger_Type_t;


   /* The following structure defines the optional Time Trigger Setting */
   /* descriptor that may be used for Digital/Analog Characteristic     */
   /* instances.                                                        */
typedef struct _tagAIOS_Time_Trigger_Data_t
{
   AIOS_Time_Trigger_Type_t  Condition;
   union
   {
     AIOS_Time_Interval_t    TimeInterval;
     Word_t                  Count;
   } ComparisonValue;
} AIOS_Time_Trigger_Data_t;

#define AIOS_TIME_TRIGGER_DATA_SIZE                            (sizeof(AIOS_Time_Trigger_Data_t))

   /* The following information is needed to identify an AIOS           */
   /* Characteristic.  The Type field specifies the type of AIOS        */
   /* Characteristic.  The IOType field specifies whether the AIOS      */
   /* Characteristic is an input or output Characteristic.  The ID field*/
   /* identifies the Characteristic instance for the specified AIOS     */
   /* Characteristic type.                                              */
   /* * NOTE * If the Type field is set to actAggregate then, the IOType*/
   /*          field may be set to ioInput, ioOutput, or ioInputOutput  */
   /*          depending on the Characteristic instance IOTypes that    */
   /*          make up the Aggregate Characteristic when the service was*/
   /*          initialized.                                             */
   /* * NOTE * The ID field starts at 0 for all instances.  The         */
   /*          Aggregate Characteristic will ALWAYS have an ID of zero. */
typedef struct _tagAIOS_Characteristic_Info_t
{
   AIOS_Characteristic_Type_t  Type;
   AIOS_IO_Type_t              IOType;
   unsigned int                ID;
} AIOS_Characteristic_Info_t;

#define AIOS_CHARACTERISTIC_INFO_DATA_SIZE                     (sizeof(AIOS_Characteristic_Info_t))

   /* The following enumeration covers all the events generated by the  */
   /* AIOS Server.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the AIOS_Event_Data_t structure.                                  */
typedef enum _tagAIOS_Event_Type_t
{
   etAIOS_Server_Read_Characteristic_Request,
   etAIOS_Server_Write_Characteristic_Request,
   etAIOS_Server_Read_CCCD_Request,
   etAIOS_Server_Write_CCCD_Request,
   etAIOS_Server_Read_Presentation_Format_Request,
   etAIOS_Server_Read_User_Description_Request,
   etAIOS_Server_Write_User_Description_Request,
   etAIOS_Server_Read_Value_Trigger_Setting_Request,
   etAIOS_Server_Write_Value_Trigger_Setting_Request,
   etAIOS_Server_Read_Time_Trigger_Setting_Request,
   etAIOS_Server_Write_Time_Trigger_Setting_Request,
   etAIOS_Server_Read_Number_Of_Digitals_Request,
   etAIOS_Server_Read_Valid_Range_Request,
   etAIOS_Server_Confirmation
} AIOS_Event_Type_t;

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read an AIOS Characteristic's */
   /* value.  The InstanceID identifies the AIOS Instance that          */
   /* dispatched the event.  The ConnectionID and ConnectionType        */
   /* specifiy the Client that is making the request.  The TransactionID*/
   /* specifies the TransactionID of the request.  The                  */
   /* CharacteristicInfo specifies the AIOS Characteristic information. */
   /* * NOTE * This event can only be received for Digital and Analog   */
   /*          Characteristics or the Aggregate Characteristic.         */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_Characteristic_Request_Response() function.    */
typedef struct _tagAIOS_Read_Characteristic_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_Characteristic_Request_Data_t;

#define AIOS_READ_CHARACTERISTIC_REQUEST_DATA_SIZE             (sizeof(AIOS_Read_Characteristic_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to write an AIOS Characteristic's*/
   /* value.  The InstanceID identifies the AIOS Instance that          */
   /* dispatched the event.  The ConnectionID and ConnectionType        */
   /* specifiy the Client that is making the request.  The              */
   /* CharacteristicInfo specifies the AIOS Characteristic information. */
   /* The Data field contains the new value that will be written for the*/
   /* specified AIOS Characteristic.                                    */
   /* * NOTE * This event can only be received for Digital or Analog    */
   /*          Characteristic's that are Outputs (Inputs cannot be      */
   /*          written).  The Aggregate Characteristic may not be       */
   /*          written.                                                 */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Write_Characteristic_Request_Response() function.   */
typedef struct _tagAIOS_Write_Characteristic_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOS_Characteristic_Data_t  Data;
} AIOS_Write_Characteristic_Request_Data_t;

#define AIOS_WRITE_CHARACTERISTIC_REQUEST_DATA_SIZE            (sizeof(AIOS_Write_Characteristic_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read a Client Characteristic  */
   /* Configuration descriptor (CCCD).  The InstanceID identifies the   */
   /* AIOS Instance that dispatched the event.  The ConnectionID,       */
   /* ConnectionType, and RemoteDevice specifiy the Client that is      */
   /* making the request.  The TransactionID specifies the TransactionID*/
   /* of the request.  The CharacteristicInfo specifies the AIOS        */
   /* Characteristic information.                                       */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a CCCD.                                        */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the AIOS_Read_CCCD_Request_Response()  */
   /*          function.                                                */
typedef struct _tagAIOS_Read_CCCD_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_CCCD_Request_Data_t;

#define AIOS_READ_CCCD_REQUEST_DATA_SIZE                       (sizeof(AIOS_Read_CCCD_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client has written a Client Characteristic            */
   /* Configuration descriptor (CCCD).  The InstanceID identifies the   */
   /* AIOS Instance that dispatched the event.  The ConnectionID,       */
   /* ConnectionType, and RemoteDevice specifiy the Client that is      */
   /* making the update.  The CharacteristicInfo specifies the AIOS     */
   /* Characteristic, whose client CCCD value should be written.  The   */
   /* ClientConfiguration member is the new Client Characteristic       */
   /* Configuration.                                                    */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a CCCD.                                        */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the AIOS_Write_CCCD_Request_Response() */
   /*          function.                                                */
typedef struct _tagAIOS_Write_CCCD_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   Word_t                      ClientConfiguration;
} AIOS_Write_CCCD_Request_Data_t;

#define AIOS_WRITE_CCCD_REQUEST_DATA_SIZE                      (sizeof(AIOS_Write_CCCD_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read a Digital/Analog         */
   /* Characteristic's Presentation Format descriptor.  The InstanceID  */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The TransactionID specifies the TransactionID of the*/
   /* request.  The CharacteristicInfo specifies the AIOS Characteristic*/
   /* information.                                                      */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a Presentation Format descriptor.              */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_Presentation_Format_Request_Response()         */
   /*          function.                                                */
typedef struct _tagAIOS_Read_Presentation_Format_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_Presentation_Format_Request_Data_t;

#define AIOS_READ_PRESENTATION_FORMAT_REQUEST_DATA_SIZE        (sizeof(AIOS_Read_Presentation_Format_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read a Digital/Analog         */
   /* Characteristic's User Description descriptor.  The InstanceID     */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The TransactionID specifies the TransactionID of the*/
   /* request.  The CharacteristicInfo specifies the AIOS Characteristic     */
   /* information.  The Offset field indicates the starting offset that */
   /* the User Description should be read from.                         */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a User Description descriptor.                 */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_User_Description_Request_Response() function.  */
   /* * NOTE * The Offset field will ALWAYS be 0 for GATT Read requests.*/
   /*          However, if the User Description is too long to be all be*/
   /*          sent in the AIOS_Read_User_Description_Request_Response()*/
   /*          function.  The AIOS Client may issue Read Long requests  */
   /*          to receive the rest of the User Description if this is   */
   /*          the case.  The Offset simply indicates where we should   */
   /*          start reading the remaining User Description.            */
typedef struct _tagAIOS_Read_User_Description_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   Word_t                      Offset;
} AIOS_Read_User_Description_Request_Data_t;

#define AIOS_READ_USER_DESCRIPTION_REQUEST_DATA_SIZE           (sizeof(AIOS_Read_User_Description_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to write a Digital/Analog        */
   /* Characteristic's User Description descriptor.  The InstanceID     */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The CharacteristicInfo specifies the AIOS           */
   /* Characteristic information.  The final fields are the             */
   /* UserDescriptionLength and the new UserDescription value.          */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a User Description descriptor.                 */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Write_User_Description_Request_Response() function. */
typedef struct _tagAIOS_Write_User_Description_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   Word_t                      UserDescriptionLength;
   Byte_t                     *UserDescription;
} AIOS_Write_User_Description_Request_Data_t;

#define AIOS_WRITE_USER_DESCRIPTION_REQUEST_DATA_SIZE          (sizeof(AIOS_Write_User_Description_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read a Digital/Analog         */
   /* Characteristic's Value Trigger Setting descriptor.  The InstanceID*/
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The TransactionID specifies the TransactionID of the*/
   /* request.  The CharacteristicInfo specifies the AIOS Characteristic*/
   /* information.                                                      */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a Value Trigger Setting descriptor.            */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_Value_Trigger_Setting_Request_Response()       */
   /*          function.                                                */
typedef struct _tagAIOS_Read_Value_Trigger_Setting_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_Value_Trigger_Setting_Request_Data_t;

#define AIOS_READ_VALUE_TRIGGER_SETTING_REQUEST_DATA_SIZE      (sizeof(AIOS_Read_Value_Trigger_Setting_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to write a Digital/Analog        */
   /* Characteristic's Value Trigger Setting descriptor.  The InstanceID*/
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The CharacteristicInfo specifies the AIOS           */
   /* Characteristic information.  The ValueTriggerSetting contains the */
   /* new value for the Value Trigger Setting descriptor.               */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a Value Trigger Setting descriptor.            */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Write_Value_Trigger_Setting_Request_Response()      */
   /*          function.                                                */
typedef struct _tagAIOS_Write_Value_Trigger_Setting_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOS_Value_Trigger_Data_t   ValueTriggerSetting;
} AIOS_Write_Value_Trigger_Setting_Request_Data_t;

#define AIOS_WRITE_VALUE_TRIGGER_SETTING_REQUEST_DATA_SIZE     (sizeof(AIOS_Write_Value_Trigger_Setting_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read a Digital/Analog         */
   /* Characteristic's Time Trigger Setting descriptor.  The InstanceID */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The TransactionID specifies the TransactionID of the*/
   /* request.  The CharacteristicInfo specifies the AIOS Characteristic*/
   /* information.                                                      */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a Time Trigger Setting descriptor.             */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_Time_Trigger_Setting_Request_Response()        */
   /*          function.                                                */
typedef struct _tagAIOS_Read_Time_Trigger_Setting_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_Time_Trigger_Setting_Request_Data_t;

#define AIOS_READ_TIME_TRIGGER_SETTING_REQUEST_DATA_SIZE      (sizeof(AIOS_Read_Time_Trigger_Setting_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to write a Digital/Analog        */
   /* Characteristic's Time Trigger Setting descriptor.  The InstanceID */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The CharacteristicInfo specifies the AIOS           */
   /* Characteristic information.  The TimeTriggerSetting contains the  */
   /* new time for the Time Trigger Setting descriptor.                 */
   /* * NOTE * This event can only be received for AIOS Characteristics */
   /*          that have a Time Trigger Setting descriptor.             */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Write_Time_Trigger_Setting_Request_Response()       */
   /*          function.                                                */
typedef struct _tagAIOS_Write_Time_Trigger_Setting_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOS_Time_Trigger_Data_t    TimeTriggerSetting;
} AIOS_Write_Time_Trigger_Setting_Request_Data_t;

#define AIOS_WRITE_TIME_TRIGGER_SETTING_REQUEST_DATA_SIZE     (sizeof(AIOS_Write_Time_Trigger_Setting_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read the Digital              */
   /* Characteristic's Number of Digitals descriptor.  The InstanceID   */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The TransactionID specifies the TransactionID of the*/
   /* request.  The CharacteristicInfo specifies the AIOS Characteristic*/
   /* information.                                                      */
   /* * NOTE * This event can only be received for Digital              */
   /*          Characteristics.                                         */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_Number_Of_Digitals_Request_Response() function.*/
typedef struct _tagAIOS_Read_Number_Of_Digitals_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_Number_Of_Digitals_Request_Data_t;

#define AIOS_READ_NUMBER_OF_DIGITALS_REQUEST_DATA_SIZE         (sizeof(AIOS_Read_Number_Of_Digitals_Request_Data_t))

   /* The following AIOS Profile Event is dispatched to a AIOS Server   */
   /* when a AIOS Client is attempting to read the Analog               */
   /* Characteristic's Valid Range descriptor.  The InstanceID          */
   /* identifies the AIOS Instance that dispatched the event.  The      */
   /* ConnectionID and ConnectionType specifiy the Client that is making*/
   /* the request.  The TransactionID specifies the TransactionID of the*/
   /* request.  The CharacteristicInfo specifies the AIOS Characteristic*/
   /* information.                                                      */
   /* * NOTE * This event can only be received for Analog               */
   /*          Characteristics that contain the optional Valid Range    */
   /*          descriptor.                                              */
   /* * NOTE * The following fields may be required when responding to  */
   /*          the request using the                                    */
   /*          AIOS_Read_Valid_Range_Request_Response() function.       */
typedef struct _tagAIOS_Read_Valid_Range_Request_Data_t
{
   unsigned int                InstanceID;
   unsigned int                ConnectionID;
   unsigned int                TransactionID;
   GATT_Connection_Type_t      ConnectionType;
   BD_ADDR_t                   RemoteDevice;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
} AIOS_Read_Valid_Range_Request_Data_t;

#define AIOS_READ_VALID_RANGE_REQUEST_DATA_SIZE                (sizeof(AIOS_Read_Valid_Range_Request_Data_t))

   /* The following is dispatched to a AIOS Server when am AIOS Client  */
   /* confirms an outstanding indication.  The ConnectionID and         */
   /* RemoteDevice identify the AIOS Client that has confirmed an       */
   /* outstanding indication.  The Status field contains the result of  */
   /* the confirmation.  The BytesWritten field indicates the number of */
   /* bytes that were successfully indicated to the AIOS Client.        */
typedef struct _tagAIOS_Confirmation_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   unsigned int            TransactionID;
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDevice;
   Byte_t                  Status;
   Word_t                  BytesWritten;
} AIOS_Confirmation_Data_t;

#define AIOS_CONFIRMATION_DATA_SIZE                            (sizeof(AIOS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all AIOS Profile Event Data.  This structure is received  */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagAIOS_Event_Data_t
{
   AIOS_Event_Type_t  Event_Data_Type;
   Word_t             Event_Data_Size;
   union
   {
      AIOS_Read_Characteristic_Request_Data_t          *AIOS_Read_Characteristic_Request_Data;
      AIOS_Write_Characteristic_Request_Data_t         *AIOS_Write_Characteristic_Request_Data;
      AIOS_Read_CCCD_Request_Data_t                    *AIOS_Read_CCCD_Request_Data;
      AIOS_Write_CCCD_Request_Data_t                   *AIOS_Write_CCCD_Request_Data;
      AIOS_Read_Presentation_Format_Request_Data_t     *AIOS_Read_Presentation_Format_Request_Data;
      AIOS_Read_User_Description_Request_Data_t        *AIOS_Read_User_Description_Request_Data;
      AIOS_Write_User_Description_Request_Data_t       *AIOS_Write_User_Description_Request_Data;
      AIOS_Read_Value_Trigger_Setting_Request_Data_t   *AIOS_Read_Value_Trigger_Setting_Request_Data;
      AIOS_Write_Value_Trigger_Setting_Request_Data_t  *AIOS_Write_Value_Trigger_Setting_Request_Data;
      AIOS_Read_Time_Trigger_Setting_Request_Data_t    *AIOS_Read_Time_Trigger_Setting_Request_Data;
      AIOS_Write_Time_Trigger_Setting_Request_Data_t   *AIOS_Write_Time_Trigger_Setting_Request_Data;
      AIOS_Read_Number_Of_Digitals_Request_Data_t      *AIOS_Read_Number_Of_Digitals_Request_Data;
      AIOS_Read_Valid_Range_Request_Data_t             *AIOS_Read_Valid_Range_Request_Data;
      AIOS_Confirmation_Data_t                         *AIOS_Confirmation_Data;
   } Event_Data;
} AIOS_Event_Data_t;

#define AIOS_EVENT_DATA_SIZE                                   (sizeof(AIOS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a AIOS Profile Event Receive Data Callback.  This function will be*/
   /* called whenever an AIOS Profile Event occurs that is associated   */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the AIOS Event Data that       */
   /* occurred and the AIOS Profile Event Callback Parameter that was   */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the AIOS Profile Event Data ONLY in the       */
   /* context of this callback.  If the caller requires the Data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another Data Buffer This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have to be       */
   /* re-entrant).It needs to be noted however, that if the same        */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another AIOS Profile Event will not be processed while this       */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving AIOS Profile Event  */
   /*            Packets.  A Deadlock WILL occur because NO AIOS Event  */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *AIOS_Event_Callback_t)(unsigned int BluetoothStackID, AIOS_Event_Data_t *AIOS_Event_Data, unsigned long CallbackParameter);

   /* AIOS Server API.                                                  */

   /* The following function is responsible for opening a AIOS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the AIOS Service Flags */
   /* (AIOS_SERVICE_FLAGS_XXX) from AIOSTypes.h.  These flags MUST be   */
   /* used to register the GATT service for the correct transport.  The */
   /* third parameter is a pointer to the AIOS_Initialize_Data_t        */
   /* structure that contains the information needed to initialize and  */
   /* configure the service.  The fourth parameter is the Callback      */
   /* function to call when an event occurs on this Server Port.  The   */
   /* fifth parameter is a user-defined callback parameter that will be */
   /* passed to the callback function with each event.  The final       */
   /* parameter is a pointer to store the GATT Service ID of the        */
   /* registered AIOS service.  This function returns the positive,     */
   /* non-zero, Instance ID or a negative error code.                   */
   /* ** NOTE ** It is the application's responsibilty to meet the size */
   /*            constraints for each Digital Characteristic instance   */
   /*            and the Aggregate Characteristic.  Both sizes MUST be  */
   /*            under (ATT_MTU-3).  This function will NOT check the   */
   /*            size constraints since no connection should be         */
   /*            established prior to calling this function.  API’s that*/
   /*            send these Characteristics to an AIOS Client will check*/
   /*            to make sure that the size constraint is met.  If this */
   /*            constraint is not met then all API's that send a       */
   /*            Digital Characteristic instance or the Aggregate       */
   /*            Characteristic will FAIL with the error                */
   /*            AIOS_ERROR_CHARACTERISTIC_MTU_SIZE_EXCEEDED.           */
   /* ** NOTE ** The AIOS Server should call the                        */
   /*            GATT_Change_Maximum_Supported_MTU() function prior to  */
   /*            initializing the service via a call to                 */
   /*            AIOS_Initialize_XXX(), to set an MTU that is large     */
   /*            enough to support the Digital Characteristic instances */
   /*            and the Aggregate Characteristic.  The AIOS Client MUST*/
   /*            call the GATT_Exchange_MTU_Request() function to       */
   /*            negotiate a higher MTU once a connection has been      */
   /*            established.  The smaller supported MTU by the AIOS    */
   /*            Server and AIOS Client will ALWAYS be the negotiated   */
   /*            MTU.                                                   */
   /* ** NOTE ** The InitializeData parameter and sub-structures MUST be*/
   /*            valid.  This API will FAIL if the structure pointed to */
   /*            by the InitializeData parameter is configured          */
   /*            incorrectly.  Since there is too much information to   */
   /*            cover here, please see the AIOS_Initialize_Data_t      */
   /*            structure in AIOSAPI.h for more information about      */
   /*            configuring this structure.                            */
   /* * NOTE * Only 1 AIOS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, AIOS_Initialize_Data_t *InitializeData, AIOS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, AIOS_Initialize_Data_t *InitializeData, AIOS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a AIOS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the AIOS Service Flags */
   /* (AIOS_SERVICE_FLAGS_XXX) from AIOSTypes.h.  These flags MUST be   */
   /* used to register the GATT service for the correct transport.  The */
   /* third parameter is a pointer to the AIOS_Initialize_Data_t        */
   /* structure that contains the information needed to initialize and  */
   /* configure the service.  The fourth parameter is the Callback      */
   /* function to call when an event occurs on this Server Port.  The   */
   /* fifth parameter is a user-defined callback parameter that will be */
   /* passed to the callback function with each event.  The sixth       */
   /* parameter is a pointer to store the GATT Service ID of the        */
   /* registered AIOS service.  The final parameter is a pointer, that  */
   /* on input can be used to control the location of the service in the*/
   /* GATT database, and on ouput to store the service handle range.    */
   /* This function returns the positive, non-zero, Instance ID or a    */
   /* negative error code.                                              */
   /* ** NOTE ** It is the application's responsibilty to meet the size */
   /*            constraints for each Digital Characteristic instance   */
   /*            and the Aggregate Characteristic.  Both sizes MUST be  */
   /*            under (ATT_MTU-3).  This function will NOT check the   */
   /*            size constraints since no connection should be         */
   /*            established prior to calling this function.  API’s that*/
   /*            send these Characteristics to an AIOS Client will check*/
   /*            to make sure that the size constraint is met.  If this */
   /*            constraint is not met then all API's that send a       */
   /*            Digital Characteristic instance or the Aggregate       */
   /*            Characteristic will FAIL with the error                */
   /*            AIOS_ERROR_CHARACTERISTIC_MTU_SIZE_EXCEEDED.           */
   /* ** NOTE ** The AIOS Server should call the                        */
   /*            GATT_Change_Maximum_Supported_MTU() function prior to  */
   /*            initializing the service via a call to                 */
   /*            AIOS_Initialize_XXX(), to set an MTU that is large     */
   /*            enough to support the Digital Characteristic instances */
   /*            and the Aggregate Characteristic.  The AIOS Client MUST*/
   /*            call the GATT_Exchange_MTU_Request() function to       */
   /*            negotiate a higher MTU once a connection has been      */
   /*            established.  The smaller supported MTU by the AIOS    */
   /*            Server and AIOS Client will ALWAYS be the negotiated   */
   /*            MTU.                                                   */
   /* ** NOTE ** The InitializeData parameter and sub-structures MUST be*/
   /*            valid.  This API will FAIL if the structure pointed to */
   /*            by the InitializeData parameter is configured          */
   /*            incorrectly.  Since there is too much information to   */
   /*            cover here, please see the AIOS_Initialize_Data_t      */
   /*            structure in AIOSAPI.h for more information about      */
   /*            configuring this structure.                            */
   /* * NOTE * Only 1 AIOS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, AIOS_Initialize_Data_t *InitializeData, AIOS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, AIOS_Initialize_Data_t *InitializeData, AIOS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previous AIOS */
   /* Server.  The first parameter is the Bluetooth Stack ID on which to*/
   /* close the server.  The second parameter is the InstanceID that was*/
   /* returned from a successful call to AIOS_Initialize_XXX().  This   */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI AIOS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_AIOS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* AIOS_Suspend()).  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to AIOS_Suspend().  This    */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the AIOS Service that is         */
   /* registered with a call to AIOS_Initialize_XXX().  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to AIOS_Initialize_XXX() API.  This function returns the non-zero */
   /* number of attributes that are contained in a AIOS Server or zero  */
   /* on failure.                                                       */
BTPSAPI_DECLARATION unsigned int BTPSAPI AIOS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_AIOS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Characteristic's Client   */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to AIOS_Initialize_XXX().  The third parameter is the GATT        */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter contains the current Client    */
   /* Characteristic Configuration to send to the AIOS Client.  This    */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The ClientConfiguration parameter is only REQUIRED if the*/
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it will be ignored.                            */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode,  AIOS_Characteristic_Info_t *CharacteristicInfo, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode,  AIOS_Characteristic_Info_t *CharacteristicInfo, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from an AIOS Client for an AIOS Characteristic's Client   */
   /* Characteristic Configuration descriptor (CCCD).  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to AIOS_Initialize_XXX().  The third parameter is the GATT        */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* final parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Characteristic's          */
   /* Presentation Format descriptor.  The first parameter is the       */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is a pointer to the            */
   /* Presentation Format data to send to the AIOS Client.  This        */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The PresentationFormatData parameter is only REQUIRED if */
   /*          the ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.      */
   /*          Otherwise it may excluded (NULL).                        */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_Presentation_Format_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Presentation_Format_Data_t *PresentationFormatData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_Presentation_Format_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Presentation_Format_Data_t *PresentationFormatData);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Characteristic's User     */
   /* Description descriptor.  The first parameter is the Bluetooth     */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is a pointer to the User       */
   /* Description to send to the AIOS Client.  This function returns a  */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The UserDescription parameter is only REQUIRED if the    */
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it may excluded (NULL).                        */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_User_Description_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, char *UserDescription);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_User_Description_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, char *UserDescription);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from an AIOS Client for an AIOS Characteristic's User     */
   /* Description descriptor.  The first parameter is the Bluetooth     */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* final parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Write_User_Description_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Write_User_Description_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Characteristic's Value    */
   /* Trigger Setting descriptor.  The first parameter is the Bluetooth */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is a pointer to the Value      */
   /* Trigger Setting data to send to the AIOS Client.  This function   */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The ValueTriggerData parameter is only REQUIRED if the   */
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it may excluded (NULL).                        */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_Value_Trigger_Setting_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Value_Trigger_Data_t *ValueTriggerData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_Value_Trigger_Setting_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Value_Trigger_Data_t *ValueTriggerData);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from an AIOS Client for an AIOS Characteristic's Value    */
   /* Trigger Setting descriptor.  The first parameter is the Bluetooth */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* final parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Write_Value_Trigger_Setting_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Write_Value_Trigger_Setting_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Characteristic's Time     */
   /* Trigger Setting descriptor.  The first parameter is the Bluetooth*/
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is a pointer to the Time       */
   /* Trigger Setting data to send to the AIOS Client.  This function   */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The TimeTriggerData parameter is only REQUIRED if the    */
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it may excluded (NULL).                        */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_Time_Trigger_Setting_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Time_Trigger_Data_t *TimeTriggerData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_Time_Trigger_Setting_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Time_Trigger_Data_t *TimeTriggerData);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from an AIOS Client for an AIOS Characteristic's Time     */
   /* Trigger Setting descriptor.  The first parameter is the Bluetooth */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* final parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Write_Time_Trigger_Setting_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Write_Time_Trigger_Setting_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Digital Characteristic's  */
   /* Number Of Digitals descriptor.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is the Number of Digitals to   */
   /* send to the AIOS Client.  This function returns a zero if         */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The NumberOfDigitals parameter is only REQUIRED if the   */
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it will be ignored.                            */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_Number_Of_Digitals_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, Byte_t NumberOfDigitals);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_Number_Of_Digitals_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, Byte_t NumberOfDigitals);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Analog Characteristic's   */
   /* Valid Range descriptor.  The first parameter is the Bluetooth     */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* AIOS_Initialize_XXX().  The third parameter is the GATT           */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* fifth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is a pointer the Valid Range   */
   /* data to send to the AIOS Client.  This function returns a zero if */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The Valid Range parameter is only REQUIRED if the        */
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it may be excluded (NULL).                     */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_Valid_Range_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Valid_Range_Data_t *ValidRangeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_Valid_Range_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Valid_Range_Data_t *ValidRangeData);
#endif

   /* The following function is responsible for responding to a read    */
   /* request from an AIOS Client for an AIOS Characteristic's value.   */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to AIOS_Initialize_XXX().  The third parameter is */
   /* the GATT Connection ID of the request.  The fourth parameter is   */
   /* the GATT Connection ID of the request.  The fifth parameter is the*/
   /* ErrorCode to indicate the type of response that will be sent.  The*/
   /* sixth parameter is the CharacteristicInfo, which contains         */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  The final parameter is a pointer to the            */
   /* Characteristic's data to send to the AIOS Client.  This function  */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * All contraints for the AIOS_Digital_Characteristic_Data_t*/
   /*          structure MUST be met if this function is responding to a*/
   /*          request to read a Digital Characteristic's value.        */
   /* * NOTE * All contraints for the                                   */
   /*          AIOS_Aggregate_Characteristic_Data_t structure MUST be   */
   /*          met if this function is responding to a request to read  */
   /*          an Aggregate Characteristic's value.                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * The CharacteristicData parameter is only REQUIRED if the */
   /*          ErrorCode parameter is AIOS_ERROR_CODE_SUCCESS.          */
   /*          Otherwise it may be excluded (NULL).                     */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Read_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Characteristic_Data_t *CharacteristicData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Read_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Characteristic_Data_t *CharacteristicData);
#endif

   /* The following function is responsible for responding to a write   */
   /* request from an AIOS Client for an AIOS Characteristic's value.   */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to AIOS_Initialize_XXX().  The third parameter is */
   /* the GATT Transaction ID of the request.  The fourth parameter is  */
   /* the ErrorCode to indicate the type of response that will be sent. */
   /* The final parameter is the CharacteristicInfo, which contains     */
   /* information about the AIOS Characteristic that is needed to send  */
   /* the response.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          AIOS_ERROR_CODE_XXX from AIOSTypes.h or                  */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Write_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Write_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, AIOS_Characteristic_Info_t *CharacteristicInfo);
#endif

   /* The following function is responsible for sending a notification  */
   /* an AIOS Characteristic's value to an AIOS Client.  The first      */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to AIOS_Initialize_XXX().  The third parameter is the GATT        */
   /* Connection ID.  The fourth parameter is the CharacteristicInfo,   */
   /* which contains information about the AIOS Characteristic that is  */
   /* needed to send the notification.  The final parameter is a pointer*/
   /* to the Characteristic's data to notify to the AIOS Client.  This  */
   /* function returns a positive non-zero value if successful          */
   /* representing the length of the notification or a negative error   */
   /* code if an error occurs.                                          */
   /* * NOTE **It is the application's responsibilty to make sure that  */
   /*          the AIOS Characteristic that is going to be notified has */
   /*          been previously configured for notifications.  An AIOS   */
   /*          Client MUST have written the AIOS Characteristic's CCCD  */
   /*          to enable notifications.                                 */
   /* * NOTE * All contraints for the AIOS_Digital_Characteristic_Data_t*/
   /*          structure MUST be met if this function is to send the    */
   /*          Digital Characteristic's value.                          */
   /* * NOTE * All contraints for the                                   */
   /*          AIOS_Aggregate_Characteristic_Data_t structure MUST be   */
   /*          met if this function is to send the Aggregate            */
   /*          Characteristic's value.                                  */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Notify_Characteristic(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Characteristic_Data_t *CharacteristicData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Notify_Characteristic_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Characteristic_Data_t *CharacteristicData);
#endif

   /* The following function is responsible for sending an indication an*/
   /* AIOS Characteristic's value to an AIOS Client.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to AIOS_Initialize_XXX().  The third parameter is the GATT        */
   /* Connection ID.  The fourth parameter is the CharacteristicInfo,   */
   /* which contains information about the AIOS Characteristic that is  */
   /* needed to send the indication.  The final parameter is a pointer  */
   /* to the Characteristic's data to indicate to the AIOS Client.  This*/
   /* function returns a positive non-zero value, which represents the  */
   /* GATT Transaction ID for the outstanding indication or a negative  */
   /* error code if an error occurs.                                    */
   /* ** NOTE ** It is the application's responsibilty to make sure that*/
   /*            the AIOS Characteristic that is going to be indicated  */
   /*            has been previously configured for indications.  An    */
   /*            AIOS Client MUST have written the AIOS Characteristic's*/
   /*            CCCD to enable indications.                            */
   /* * NOTE * All contraints for the AIOS_Digital_Characteristic_Data_t*/
   /*          structure MUST be met if this function is to send the    */
   /*          Digital Characteristic's value.                          */
   /* * NOTE * All contraints for the                                   */
   /*          AIOS_Aggregate_Characteristic_Data_t structure MUST be   */
   /*          met if this function is to send the Aggregate            */
   /*          Characteristic's value.                                  */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Indicate_Characteristic(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Characteristic_Data_t *CharacteristicData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Indicate_Characteristic_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, AIOS_Characteristic_Info_t *CharacteristicInfo, AIOS_Characteristic_Data_t *CharacteristicData);
#endif

   /* AIOS Client API                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote AIOS Server interpreting it as a Presentation Format*/
   /* Characteristic Descriptor.  The first parameter is the length of  */
   /* the value returned by the remote AIOS Server.  The second         */
   /* parameter is a pointer to the data returned by the remote AIOS    */
   /* Server.The final parameter is a pointer to store the parsed AIOS  */
   /* IO Presentation Format.  This function returns a zero if          */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Decode_Presentation_Format(unsigned int ValueLength, Byte_t *Value, AIOS_Presentation_Format_Data_t *CharacteristicPresentationFormat);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Decode_Presentation_Format_t)(unsigned int ValueLength, Byte_t *Value, AIOS_Presentation_Format_Data_t *CharacteristicPresentationFormat);
#endif

   /* The following function is responsible for formatting the AIOS     */
   /* Value Trigger Setting Characteristic descriptor into a            */
   /* user-specified buffer, for a GATT Write request, that will be sent*/
   /* to the AIOS Server.  This function may also be used to determine  */
   /* the size of the buffer to hold the formatted data (see below).    */
   /* The first parameter is the AIOS Value Trigger Setting             */
   /* Characteristic that will be formatted into the user-specified     */
   /* buffer.  The second parameter is the length of the user-specified */
   /* Buffer.  The final parameter is a pointer to the user-specified   */
   /* Buffer that will hold the formatted data if this function is      */
   /* successful.  This function returns zero if the Value Trigger      */
   /* Setting data has been successfully formatted into the             */
   /* user-specified buffer.  If this function is used to determine the */
   /* size of the buffer to hold the formatted data, then a positive    */
   /* non-zero value will be returned.  Otherwise this function will    */
   /* return a negative error code if an error occurs.                  */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The AIOS      */
   /*          Client may use this size to allocate a buffer necessary  */
   /*          to hold the formatted data.                              */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Format_Value_Trigger_Setting(AIOS_Value_Trigger_Data_t *ValueTriggerData, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Format_Value_Trigger_Setting_t)(AIOS_Value_Trigger_Data_t *ValueTriggerData, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote AIOS Server interpreting it as a Value Trigger      */
   /* Setting Characteristic Descriptor.  The first parameter is the    */
   /* length of the value returned by the remote AIOS Server.  The      */
   /* second parameter is a pointer to the data returned by the remote  */
   /* AIOS Server.  The final parameter is a pointer to store the parsed*/
   /* AIOS Value Trigger Setting.  This function returns a zero if      */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Decode_Value_Trigger_Setting(unsigned int ValueLength, Byte_t *Value, AIOS_Value_Trigger_Data_t *ValueTriggerData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Decode_Value_Trigger_Setting_t)(unsigned int ValueLength, Byte_t *Value, AIOS_Value_Trigger_Data_t *ValueTriggerData);
#endif

   /* The following function is responsible for formatting the AIOS Time*/
   /* Trigger Setting Characteristic descriptor into a user-specified   */
   /* buffer, for a GATT Write request, that will be sent to the AIOS   */
   /* Server.  This function may also be used to determine the size of  */
   /* the buffer to hold the formatted data (see below).  The first     */
   /* parameter is the AIOS Time Trigger Setting Characteristic that    */
   /* will be formatted into the user-specified buffer.  The second     */
   /* parameter is the length of the user-specified Buffer.  The final  */
   /* parameter is a pointer to the user-specified Buffer that will hold*/
   /* the formatted data if this function is successful.  This function */
   /* returns zero if the Time Trigger Setting data has been            */
   /* successfully formatted into the user-specified buffer.  If this   */
   /* function is used to determine the size of the buffer to hold the  */
   /* formatted data, then a positive non-zero value will be returned.  */
   /* Otherwise this function will return a negative error code if an   */
   /* error occurs.                                                     */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The AIOS      */
   /*          Client may use this size to allocate a buffer necessary  */
   /*          to hold the formatted data.                              */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Format_Time_Trigger_Setting(AIOS_Time_Trigger_Data_t *TimeTriggerData, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Format_Time_Trigger_Setting_t)(AIOS_Time_Trigger_Data_t *TimeTriggerData, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote AIOS Server interpreting it as a Time Trigger       */
   /* Setting Characteristic Descriptor.  The first parameter is the    */
   /* length of the value returned by the remote AIOS Server.  The      */
   /* second parameter is a pointer to the data returned by the remote  */
   /* AIOS Server.The final parameter is a pointer to store the parsed  */
   /* AIOS Time Trigger Setting.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Decode_Time_Trigger_Setting(unsigned int ValueLength, Byte_t *Value, AIOS_Time_Trigger_Data_t *TimeTriggerData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Decode_Time_Trigger_Setting_t)(unsigned int ValueLength, Byte_t *Value, AIOS_Time_Trigger_Data_t *TimeTriggerData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote AIOS Server interpreting it as a Valid Range        */
   /* Characteristic Descriptor.  The first parameter is the length of  */
   /* the value returned by the remote AIOS Server.  The second         */
   /* parameter is a pointer to the data returned by the remote AIOS    */
   /* Server.  The final parameter is a pointer to store the parsed AIOS*/
   /* Valid Range.  This function returns a zero if successful or a     */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI AIOS_Decode_Valid_Range(unsigned int ValueLength, Byte_t *Value, AIOS_Valid_Range_Data_t *ValidRangeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_AIOS_Decode_Valid_Range_t)(unsigned int ValueLength, Byte_t *Value, AIOS_Valid_Range_Data_t *ValidRangeData);
#endif

#endif

/*****< ancsapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANCSAPI - Stonestreet One Bluetooth Stack Apple Notification Center       */
/*            API Type Definitions, Constants, and Prototypes.                */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/23/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __ANCSAPIH__
#define __ANCSAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

#include "ANCSType.h"           /* Bluetooth ANCS Type Definitions/Constants. */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define ANCS_ERROR_INVALID_PARAMETER                     (-1000)
#define ANCS_ERROR_INSUFFICIENT_RESOURCES                (-1001)
#define ANCS_ERROR_INVALID_REQUEST_DATA                  (-1002)
#define ANCS_ERROR_INVALID_ATTRIBUTE_DATA                (-1003)
#define ANCS_ERROR_UNKNOWN_ERROR                         (-1004)

   /* The following enumerated type represents the different Category   */
   /* IDs that can be used.                                             */
typedef enum
{
   idOther              = ANCS_CATEGORY_ID_OTHER,
   idIncomingCall       = ANCS_CATEGORY_ID_INCOMING_CALL,
   idMissedCall         = ANCS_CATEGORY_ID_MISSED_CALL,
   idVoicemail          = ANCS_CATEGORY_ID_VOICEMAIL,
   idSocial             = ANCS_CATEGORY_ID_SOCIAL,
   idSchedule           = ANCS_CATEGORY_ID_SCHEDULE,
   idEmail              = ANCS_CATEGORY_ID_EMAIL,
   idNews               = ANCS_CATEGORY_ID_NEWS,
   idHealthAndFitness   = ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,
   idBusinessAndFinance = ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE,
   idLocation           = ANCS_CATEGORY_ID_LOCATION,
   idEntertainment      = ANCS_CATEGORY_ID_ENTERTAINMENT
} ANCS_Category_ID_t;

   /* The following enumerated type represents the different Event IDs  */
   /* that can be used.                                                 */
typedef enum
{
   idNotificationAdded    = ANCS_EVENT_ID_NOTIFICATION_ADDED,
   idNotificationModified = ANCS_EVENT_ID_NOTIFICATION_MODIFIED,
   idNotificationRemoved  = ANCS_EVENT_ID_NOTIFICATION_REMOVED
} ANCS_Event_ID_t;

   /* The following enumerated type represents the different Command    */
   /* IDs that can be used.                                             */
typedef enum
{
   idGetNotificationAttributes = ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES,
   idGetAppAttributes          = ANCS_COMMAND_ID_GET_APP_ATTRIBUTES
} ANCS_Command_ID_t;

   /* The following enumerated type represents the different            */
   /* Notification Attribute IDs that can be used.                      */
typedef enum
{
   idAppIdentifier = ANCS_NOTIFICATION_ATTRIBUTE_ID_APP_IDENTIFIER,
   idTitle         = ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE,
   idSubtitle      = ANCS_NOTIFICATION_ATTRIBUTE_ID_SUBTITLE,
   idMessage       = ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE,
   idMessageSize   = ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE_SIZE,
   idDate          = ANCS_NOTIFICATION_ATTRIBUTE_ID_DATE
} ANCS_Notification_Attribute_ID_t;

   /* The following enumerated type represents the different App        */
   /* Attribute IDs that can be used.                                   */
typedef enum
{
   idDisplayName = ANCS_APP_ATTRIBUTE_ID_DISPLAY_NAME
} ANCS_App_Attribute_ID_t;

   /* The following enumerated type represents the different types of   */
   /* Attribute Data that can exist.                                    */
typedef enum
{
   dtNotification,
   dtApplication
} ANCS_Attribute_Data_Type_t;

   /* The following structure defines the format of the data returned in*/
   /* a notification from the Nofication Source ANCS Characteristic.    */
typedef struct _tagANCS_Notification_Received_Data_t
{
   ANCS_Event_ID_t    EventID;
   Byte_t             EventFlags;
   ANCS_Category_ID_t CategoryID;
   Byte_t             CategoryCount;
   DWord_t            NotificationUID;
} ANCS_Notification_Received_Data_t;

#define ANCS_NOTIFICATION_RECEIVED_DATA_SIZE                (sizeof(ANCS_Notification_Received_Data_t))

   /* The following structure defines the format of the data required   */
   /* for each Notification Attribute being requested.                  */
   /* * NOTE * The member AttributeMaxLength is only required when      */
   /*          requesting specific Notification Attributes.  The        */
   /*          attributes which require this Max Length Parameter are   */
   /*          specified in the Apple Notification Center Service       */
   /*          Specification.                                           */
typedef struct _tagANCS_Notification_Attribute_Request_Data_t
{
   ANCS_Notification_Attribute_ID_t NotificationAttributeID;
   Word_t                           AttributeMaxLength;
} ANCS_Notification_Attribute_Request_Data_t;

#define ANCS_NOTIFCATION_ATTRIBUTE_REQUEST_DATA_SIZE        (sizeof(ANCS_Notification_Attribute_Request_Data_t))

   /* The following structure defines the format of the data required   */
   /* for each App Attribute being requested.                           */
   /* * NOTE * The member AttributeMaxLength is only required when      */
   /*          requesting specific App Attributes.  The attributes      */
   /*          which require this Max Length Parameter are specified in */
   /*          the Apple Notification Center Service Specification.     */
   /* * NOTE * As of the ANCS Specification v1.0, no App Attribute      */
   /*          types require a Max Length Parameter.  This              */
   /*          functionality is put in place in case future versions of */
   /*          the Specification require it.                            */
typedef struct _tagANCS_App_Attribute_Request_Data_t
{
   ANCS_App_Attribute_ID_t AppAttributeID;
   Word_t                  AttributeMaxLength;
} ANCS_App_Attribute_Request_Data_t;

#define ANCS_APP_ATTRIBUTE_REQUEST_DATA_SIZE                (sizeof(ANCS_App_Attribute_Request_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information related to returned app attribute data.               */
   /* * NOTE * The AttributeData member is a null-terminated string. It */
   /*          is possible that an Apple device has no information for  */
   /*          a requested attribute. In this case, the AttributeData   */
   /*          field will be set to NULL.                               */
typedef struct _tagANCS_App_Attribute_Data_t
{
   ANCS_App_Attribute_ID_t  AttributeID;
   Byte_t                  *AttributeData;
} ANCS_App_Attribute_Data_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCS_App_Attribute_Data_t struct     */
   /* based on the length of its Attribute Data.  The first parameter   */
   /* to this MACRO is the length of the member AttributeData in bytes. */
#define ANCS_APP_ATTRIBUTE_DATA_SIZE                        (sizeof(ANCS_App_Attribute_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information related to returned attribute data.                   */
   /* * NOTE * The AttributeData member is a null-terminated string. It */
   /*          is possible that an Apple device has no information for  */
   /*          a requested attribute. In this case, the AttributeData   */
   /*          field will be set to NULL.                               */
typedef struct _tagANCS_Attribute_Data_t
{
   ANCS_Attribute_Data_Type_t Type;
   union
   {
      ANCS_Notification_Attribute_ID_t NotificationAttributeID;
      ANCS_App_Attribute_ID_t          AppAttributeID;
   } AttributeID;

   Byte_t                           *AttributeData;
} ANCS_Attribute_Data_t;

#define ANCS_ATTRIBUTE_DATA_SIZE                            (sizeof(ANCS_Attribute_Data_t))

   /* The following structure holds parsed attribute data for either    */
   /* Notification Attributes or App Attributes.  This data structure   */
   /* is used when calling the                                          */
   /* ANCS_ConvertRawAttributeDataToParsedAttributeData API function.   */
typedef struct _tagANCS_Parsed_Attribute_Data_t
{
   ANCS_Attribute_Data_Type_t  Type;
   union
   {
      DWord_t        NotificationUID;
      unsigned char *AppIdentifier;
   } RequestIdentifier;

   unsigned int               NumberOfAttributes;
   ANCS_Attribute_Data_t     *AttributeData;
} ANCS_Parsed_Attribute_Data_t;

#define ANCS_PARSED_ATTRIBUTE_DATA_SIZE                     (sizeof(ANCS_Parsed_Attribute_Data_t))

   /* The following function is a utility function that exists          */
   /* to decode data received from the Notification Source ANCS         */
   /* characteristic. The BufferLength paramter indicates the length    */
   /* of the data buffer. The Buffer parameter is a pointer to the      */
   /* notification data. The NotificationReceivedData is a parameter    */
   /* is a pointer to the structure in which the decoded data will be   */
   /* placed. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI ANCS_Decode_Notification_Received_Data(unsigned int BufferLength, Byte_t *Buffer, ANCS_Notification_Received_Data_t *NotificationReceivedData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCS_Decode_Notification_Received_Data_t)(unsigned int BufferLength, Byte_t *Buffer, ANCS_Notification_Received_Data_t *NotificationReceivedData);
#endif

   /* The following function is a utility function that exists to encode*/
   /* an Notification Attribute Request in to a raw data buffer. The    */
   /* NotificationUID parameter is the uniqure ID of the notification   */
   /* for which the attributes are being requseted. The NumberAttributes*/
   /* parameters indicates the number of attributes to be encoded. The  */
   /* NotificationAttributes parameter is the list of attributes to be  */
   /* requested. The Buffer parameter is a pointer to a buffer which    */
   /* will be allocated by this function. The TotalLength parameter will*/
   /* be set to contain the total length of the allocated buffer. This  */
   /* function returns zero if successful and a negative error code is  */
   /* there is an error.                                                */
   /* * NOTE * The caller is responsible for freeing this buffer once   */
   /*          they are finished with it with a call to                 */
   /*          ANCS_Free_Notification_Attribute_Request().              */
BTPSAPI_DECLARATION int BTPSAPI ANCS_Encode_Notification_Attribute_Request(DWord_t NotificationUID, unsigned int NumberAttributes, ANCS_Notification_Attribute_Request_Data_t *NotificationAttributes, Byte_t **Buffer, unsigned int *TotalLength);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCS_Encode_Notification_Attribute_Request_t)(DWord_t NotificationUID, unsigned int NumberAttributes, ANCS_Notification_Attribute_Request_Data_t *NotificationAttributes, Byte_t **Buffer, unsigned int *TotalLength);
#endif

   /* The following MACRO is provided as a mechanism to                 */
   /* free an allocated buffer from a successful call to                */
   /* ANCS_Encode_Notification_Attribute_Request.                       */
#define ANCS_Free_Notification_Attribute_Request(_x)        BTPS_FreeMemory(_x)

   /* The following function is a utility function that exists to       */
   /* encode an App Attribute Request in to a raw data buffer. The      */
   /* AppIdentifier parameter is a null-terminated UTF-8 string which   */
   /* specified the App for which attributes are being requested. The   */
   /* NumberAttributes parameters indicates the number of attributes to */
   /* be encoded. The AppAttributes parameter is the list of attributes */
   /* to be requested. The Buffer parameter is a pointer to a buffer    */
   /* which will be allocated by this function. The TotalLength         */
   /* parameter will be set to contain the total length of the allocated*/
   /* buffer. This function returns zero if successful and a negative   */
   /* error code is there is an error.                                  */
   /* * NOTE * The caller is responsible for freeing this buffer once   */
   /*          they are finished with it with a call to                 */
   /*          ANCS_Free_App_Attribute_Request().                       */
BTPSAPI_DECLARATION int BTPSAPI ANCS_Encode_App_Attribute_Request(char *AppIdentifier, unsigned int NumberAttributes, ANCS_App_Attribute_Request_Data_t *AppAttributes, Byte_t **Buffer, unsigned int *TotalLength);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCS_Encode_App_Attribute_Request_t)(char *AppIdentifier, unsigned int NumberAttributes, ANCS_App_Attribute_Request_Data_t *AppAttributes, Byte_t **Buffer, unsigned int *TotalLength);
#endif

   /* The following MACRO is provided as a mechanism to                 */
   /* free an allocated buffer from a successful call to                */
   /* ANCS_Encode_App_Attribute_Request.                                */
#define ANCS_Free_App_Attribute_Request(_x)                 BTPS_FreeMemory(_x)

   /* The following function is a utility function that exists          */
   /* to determine with a specified buffer of data from a ANCS          */
   /* Attribute Data Notifcation is a complete response.  The           */
   /* BufferLength parameter specifies the amount of data present       */
   /* in the buffer.  The Buffer parameter is the data buffer from      */
   /* the received notifications.  The NumberAttributesRequested        */
   /* parameter represents the number of attributes which were          */
   /* requested in the initial request sent.  If this function          */
   /* returns TRUE, then the data is completed and can be sent to the   */
   /* ANCS_Convert_Raw_Attribute_Data_To_Parsed_Attribute_Data function.*/
   /* If this function returns FALSE, then the caller should wait for   */
   /* more notifications from the remote device and add the data to the */
   /* current buffer.                                                   */
BTPSAPI_DECLARATION Boolean_t BTPSAPI ANCS_Is_Buffer_Complete(unsigned int BufferLength, Byte_t *Buffer, unsigned int NumberAttributesRequested);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_ANCS_Is_Buffer_Complete_t)(unsigned int BufferLength, Byte_t* Buffer, unsigned int NumberAttributesRequested);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Raw Attribute Data Stream into Parsed Attribute     */
   /* Data in the form of ANCS_Parsed_Attribute_Data_t.  This function  */
   /* accepts as input the type of attribute data that is being parsed, */
   /* the length of the Raw stream (must be greater than zero),         */
   /* followed by a pointer to the actual Raw Attribute Data Stream.    */
   /* The final parameter is a pointer to a buffer that will contain    */
   /* the header information for the parsed data.  This function        */
   /* returns zero if successful or a negative value if an error        */
   /* occurred.                                                         */
   /* * NOTE * If this function is successful the final parameter *MUST**/
   /*          be passed to the ANCS_FreeParsedAttributeData() to free  */
   /*          any allocated resources that were allocated to track the */
   /*          Parsed Attribute Data Stream.                            */
   /* * NOTE * The Raw Attribute Stream Buffer (third parameter) *MUST* */
   /*          remain active while the data is being processed.         */
BTPSAPI_DECLARATION int BTPSAPI ANCS_Decode_Attribute_Data(unsigned int NumberAttributes, unsigned int RawDataLength, Byte_t *RawAttributeData, ANCS_Parsed_Attribute_Data_t *ParsedAttributeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCS_Decode_Attribute_Data_t)(unsigned int NumberAttributes, unsigned int RawDataLength, Byte_t *RawAttributeData, ANCS_Parsed_Attribute_Data_t *ParsedAttributeData);
#endif

   /* The following function is provided to allow a mechanism           */
   /* to free all resources that were allocated to parse a Raw          */
   /* Attribute Data Stream into Parsed Attribute Data.  See the        */
   /* ANCS_Decode_Attribute_Data() function for more information.       */
BTPSAPI_DECLARATION void BTPSAPI ANCS_Free_Parsed_Attribute_Data(ANCS_Parsed_Attribute_Data_t *ParsedAttributeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANCS_Free_Parsed_Attribute_Data_t)(ANCS_Parsed_Attribute_Data_t *ParsedAttributeData);
#endif

#endif

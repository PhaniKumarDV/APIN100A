/*****< anpmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMTYPE - Alert Notification Manager API Type Definitions and Constants  */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANPMTYPEH__
#define __ANPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "ANPMMSG.h"      /* BTPM Alert Notification Manager Message Formats. */

   /* The following enumerated type is used to indicate the specific    */
   /* type of Alert Notification connection.  A Server value indicates  */
   /* the the connection is acting as an ANP Server.  A client value    */
   /* indicates the connection is acting as a ANP Client to a remote ANP*/
   /* Server.                                                           */
typedef enum
{
   actServer,
   actClient
} ANPM_Connection_Type_t;

   /* The following enumerated type represents all of the Category types*/
   /* that are supported by ANP.                                        */
typedef enum
{
  cmSimpleAlert,
  cmEmail,
  cmNews,
  cmCall,
  cmMissedCall,
  cmSMS_MMS,
  cmVoiceMail,
  cmSchedule,
  cmHighPriorityAlert,
  cmInstantMessage,
  cmAllSupportedCategories
} ANPM_Category_Identification_t;

   /* The following defines the valid ANS Alert Category ID values that */
   /* may be received or sent.                                          */
#define ANPM_ALERT_CATEGORY_BIT_MASK_SIMPLE_ALERT              0x0001
#define ANPM_ALERT_CATEGORY_BIT_MASK_EMAIL                     0x0002
#define ANPM_ALERT_CATEGORY_BIT_MASK_NEWS                      0x0004
#define ANPM_ALERT_CATEGORY_BIT_MASK_CALL                      0x0008
#define ANPM_ALERT_CATEGORY_BIT_MASK_MISSED_CALL               0x0010
#define ANPM_ALERT_CATEGORY_BIT_MASK_SMS_MMS                   0x0020
#define ANPM_ALERT_CATEGORY_BIT_MASK_VOICE_MAIL                0x0040
#define ANPM_ALERT_CATEGORY_BIT_MASK_SCHEDULE                  0x0080
#define ANPM_ALERT_CATEGORY_BIT_MASK_HIGH_PRIORITY_ALERT       0x0100
#define ANPM_ALERT_CATEGORY_BIT_MASK_INSTANT_MESSAGE           0x0200

#define ANPM_ALERT_CATEGORY_BIT_MASK_ALL_CATEGORIES            (ANPM_ALERT_CATEGORY_BIT_MASK_SIMPLE_ALERT | ANPM_ALERT_CATEGORY_BIT_MASK_EMAIL | ANPM_ALERT_CATEGORY_BIT_MASK_NEWS | ANPM_ALERT_CATEGORY_BIT_MASK_CALL | ANPM_ALERT_CATEGORY_BIT_MASK_MISSED_CALL | ANPM_ALERT_CATEGORY_BIT_MASK_SMS_MMS | ANPM_ALERT_CATEGORY_BIT_MASK_VOICE_MAIL | ANPM_ALERT_CATEGORY_BIT_MASK_SCHEDULE | ANPM_ALERT_CATEGORY_BIT_MASK_HIGH_PRIORITY_ALERT | ANPM_ALERT_CATEGORY_BIT_MASK_INSTANT_MESSAGE)

   /* The following constants define the possible asynchronous status   */
   /* codes for ANP client operations.                                  */
#define ANPM_OPERATION_STATUS_SUCCESS                          0x00
#define ANPM_OPERATION_STATUS_INVALID_RESPONSE                 0x01
#define ANPM_OPERATION_STATUS_TIMEOUT                          0x02
#define ANPM_OPERATION_STATUS_ATT_PROTOCOL_ERROR               0x03
#define ANPM_OPERATION_STATUS_UNKNOWN_ERROR                    0xFF

   /* Used internally to represent notification types.                  */
typedef enum
{
   ntNewAlert,
   ntUnreadStatus
} ANPM_Notification_Type_t;

#endif


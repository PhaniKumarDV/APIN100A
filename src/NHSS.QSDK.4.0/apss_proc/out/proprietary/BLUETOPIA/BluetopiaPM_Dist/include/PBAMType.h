/*****< pbamtype.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PBAMTYPE - Phone Book Access Manager API Type Definitions and Constants   */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __PBAMTYPEH__
#define __PBAMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */
#include "PBAMMSG.h"      /* BTPM Phone Book Access Manager Message Formats.  */

   /* The following defines specify the bit locations for the 64-bit    */
   /* filter field specified by the PBAP specification.  Each of these  */
   /* bits can be OR'ed together to form a filter mask that should be   */
   /* passed in the FilterLow parameter where a filter is required.  The*/
   /* FilterHigh portion contains proprietary filter settings, if these */
   /* are not used it should be set to zero.  Refer to the PBAP         */
   /* specification for more information.                               */
#define PBAM_FILTER_VERSION              (1 << 0) /* vCard version                 */
#define PBAM_FILTER_FN                   (1 << 1) /* Formatted Name                */
#define PBAM_FILTER_N                    (1 << 2) /* Name                          */
#define PBAM_FILTER_PHOTO                (1 << 3) /* Associated Image Photo        */
#define PBAM_FILTER_BDAY                 (1 << 4) /* Birthday                      */
#define PBAM_FILTER_ADR                  (1 << 5) /* Delivery Address              */
#define PBAM_FILTER_LABEL                (1 << 6) /* Delivery                      */
#define PBAM_FILTER_TEL                  (1 << 7) /* Telephone Number              */
#define PBAM_FILTER_EMAIL                (1 << 8) /* Electronic Mail Address       */
#define PBAM_FILTER_MAILER               (1 << 9) /* Electronic Mail               */
#define PBAM_FILTER_TZ                   (1 << 10)/* Time Zone                     */
#define PBAM_FILTER_GEO                  (1 << 11)/* Geographic Position           */
#define PBAM_FILTER_TITLE                (1 << 12)/* Job                           */
#define PBAM_FILTER_ROLE                 (1 << 13)/* Role within the Organization  */
#define PBAM_FILTER_LOGO                 (1 << 14)/* Organization Logo             */
#define PBAM_FILTER_AGENT                (1 << 15)/* vCard of Person Representing  */
#define PBAM_FILTER_ORG                  (1 << 16)/* Name of Organization          */
#define PBAM_FILTER_NOTE                 (1 << 17)/* Comments                      */
#define PBAM_FILTER_REV                  (1 << 18)/* Revision                      */
#define PBAM_FILTER_SOUND                (1 << 19)/* Pronunciation of Name         */
#define PBAM_FILTER_URL                  (1 << 20)/* Uniform Resource Locator      */
#define PBAM_FILTER_UID                  (1 << 21)/* Unique ID                     */
#define PBAM_FILTER_KEY                  (1 << 22)/* Public Encryption Key         */
#define PBAM_FILTER_NICKNAME             (1 << 23)/* Nickname                      */
#define PBAM_FILTER_CATEGORIES           (1 << 24)/* Categories                    */
#define PBAM_FILTER_PROID                (1 << 25)/* Product ID                    */
#define PBAM_FILTER_CLASS                (1 << 26)/* Class information             */
#define PBAM_FILTER_SORT_STRING          (1 << 27)/* String used for sorting       */
#define PBAM_FILTER_X_IRMC_CALL_DATETIME (1 << 28)/* Time stamp                    */

#define PBAM_FILTER_NONE                 (0)
#define PBAM_FILTER_ALL                  ((1 << 29)-1)

#define PBAM_FILTER_VCARD21_MINIMUM      (PBAM_FILTER_VERSION | PBAM_FILTER_N | PBAM_FILTER_TEL)
#define PBAM_FILTER_VCARD30_MINIMUM      (PBAM_FILTER_VERSION | PBAM_FILTER_N | PBAM_FILTER_FN | PBAM_FILTER_TEL)

   /* The following define specifies the bit location for the           */
   /* Proprietary Filter bit for the 64-bit Filter bit mask.  Because   */
   /* this implementation handles this 64-bit field as two 32-bit       */
   /* fields, this bit position has been shifted for use in the         */
   /* FilterHigh parameter only.  If a Proprietary Filter is NOT used,  */
   /* the entire FilterHigh parameter should be set to zero.  bits 40-63*/
   /* (shifted by 32 for the FilterHigh parameter) are available for    */
   /* defining proprietary filter bits.  Refer to the PBAP specification*/
   /* for more information.                                             */
#define PBAM_FILTER_PROPRIETARY_FILTER_HIGH  (1 << (39-32))

   /* The following define is the value that is used for the            */
   /* MaxListCount header to indicate that there is not a restriction of*/
   /* the number of items that can be returned.  This is also the       */
   /* default value if this header is not present.                      */
#define PBAM_MAX_LIST_COUNT_NOT_RESTRICTED   (65535)

   /* The following definition defines the delimiter character that is  */
   /* used to delimit the individual sub-paths in a fully qualified path*/
   /* name.                                                             */
#define PBAM_PATH_DELIMETER_CHARACTER        '/'
#define PBAM_PATH_DELIMETER                  "/"

   /* The following definition defines the path immediately off of the  */
   /* root either (main root or SIM root) that contains the PBAM        */
   /* Information Store.                                                */
#define PBAM_TELECOM_PATH_NAME               "telecom"

   /* The following constant presents the path (off of main/absolute    */
   /* root) that contains the PBAM Information Store that is present on */
   /* an external SIM Card (as opposed to stored on the phone directly).*/
#define PBAM_SIM_PATH_NAME                   "SIM1"

   /* The following constants present the path names of all required    */
   /* paths under the PBAM_TELECOM_PATH_NAME that hold the various PBAP */
   /* Information Stores.                                               */
   /* * NOTE * These path names can also be used as a prefix to fetch   */
   /*          an entire store.  For example, if the caller was         */
   /*          currently operating in the PBAP_TELECOM_PATH_NAME path   */
   /*          (/telecom), the fetching the object                      */
   /*          PBAP_PHONEBOOK_PATH_NAME appended with                   */
   /*          PBAM_OBJECT_NAME_SUFFIX would fetch the entire phonebook.*/
#define PBAM_PHONEBOOK_PATH_NAME             "pb"
#define PBAM_INCOMING_CALL_HISTORY_PATH_NAME "ich"
#define PBAM_OUTGOING_CALL_HISTORY_PATH_NAME "och"
#define PBAM_MISSED_CALL_HISTORY_PATH_NAME   "mch"
#define PBAM_COMBINED_CALL_HISTORY_PATH_NAME "cch"
#define PBAM_OBJECT_NAME_SUFFIX              ".vcf"

   /* The following constants represent the error codes that can be     */
   /* returned by the server.                                           */
#define PBAM_RESPONSE_STATUS_CODE_SUCCESS                      0x00000000
#define PBAM_RESPONSE_STATUS_CODE_NOT_FOUND                    0x00000001
#define PBAM_RESPONSE_STATUS_CODE_SERVICE_UNAVAILABLE          0x00000002
#define PBAM_RESPONSE_STATUS_CODE_BAD_REQUEST                  0x00000003
#define PBAM_RESPONSE_STATUS_CODE_NOT_IMPLEMENTED              0x00000004
#define PBAM_RESPONSE_STATUS_CODE_UNAUTHORIZED                 0x00000005
#define PBAM_RESPONSE_STATUS_CODE_PRECONDITION_FAILED          0x00000006
#define PBAM_RESPONSE_STATUS_CODE_NOT_ACCEPTABLE               0x00000007
#define PBAM_RESPONSE_STATUS_CODE_FORBIDDEN                    0x00000008
#define PBAM_RESPONSE_STATUS_CODE_DEVICE_POWER_OFF             0x00000009
#define PBAM_RESPONSE_STATUS_CODE_UNABLE_TO_SUBMIT_REQUEST     0x0000000A
#define PBAM_RESPONSE_STATUS_CODE_UNKNOWN                      0x0000000B

   /* The following enumeration is a list of the valid operations that  */
   /* can be performed using the PBAM_SetPhonebook operation. This      */
   /* correspond to flag settings for the OBEX SetPath command.         */
typedef enum
{
   pspRoot,
   pspDown,
   pspUp
} PBAM_Set_Path_Option_t;

   /* The following enumeration is a list of valid format values that   */
   /* can be used when pulling vCards from a remote PBAP server.        */
typedef enum
{
   pmvCard21,
   pmvCard30,
   pmDefault
} PBAM_VCard_Format_t;

typedef enum
{
   ploIndexed,
   ploAlphabetical,
   ploPhonetical,
   ploDefault
} PBAM_List_Order_t;

typedef enum
{
   psaName,
   psaNumber,
   psaSound,
   psaDefault
} PBAM_Search_Attribute_t;

#endif


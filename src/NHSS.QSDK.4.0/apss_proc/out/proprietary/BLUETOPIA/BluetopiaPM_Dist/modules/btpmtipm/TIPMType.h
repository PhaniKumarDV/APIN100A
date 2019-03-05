/*****< tipmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMTYPE - Time Manager API Type Definitions and Constants for Stonestreet*/
/*             One Bluetooth Protocol Stack Platform Manager.                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __TIPMTYPEH__
#define __TIPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "TIPMMSG.h"      /* BTPM Time Manager Message Formats.               */

   /* The following enumerated type is used to indicate the specific    */
   /* type of Time connection.  A Server value indicates the the        */
   /* connection is acting as an Time Server.  A client value indicates */
   /* the connection is acting as a Time Client to a remote Time Server.*/
typedef enum
{
   tctServer,
   tctClient
} TIPM_Connection_Type_t;

   /* The following enumerated type represents all of the valid Time    */
   /* Source values that may be assigned in the Reference Time          */
   /* Information.                                                      */
typedef enum
{
   tmsUnknown,
   tmsNetworkTimeProtocol,
   tmsGps,
   tmsRadioTimeSignal,
   tmsManual,
   tmsAtomicClock,
   tmsCellularNetwork
} TIPM_Time_Source_Type_t;

   /* The following enumerated type represents all of the valid Time    */
   /* Zone values that may be assigned in the Local Time Information.   */
typedef enum
{
   tmzUTCMinus1200,
   tmzUTCMinus1100,
   tmzUTCMinus1000,
   tmzUTCMinus930,
   tmzUTCMinus900,
   tmzUTCMinus800,
   tmzUTCMinus700,
   tmzUTCMinus600,
   tmzUTCMinus500,
   tmzUTCMinus430,
   tmzUTCMinus400,
   tmzUTCMinus330,
   tmzUTCMinus300,
   tmzUTCMinus200,
   tmzUTCMinus100,
   tmzUTCPlus000,
   tmzUTCPlus100,
   tmzUTCPlus200,
   tmzUTCPlus300,
   tmzUTCPlus330,
   tmzUTCPlus400,
   tmzUTCPlus430,
   tmzUTCPlus500,
   tmzUTCPlus530,
   tmzUTCPlus545,
   tmzUTCPlus600,
   tmzUTCPlus630,
   tmzUTCPlus700,
   tmzUTCPlus800,
   tmzUTCPlus845,
   tmzUTCPlus900,
   tmzUTCPlus930,
   tmzUTCPlus1000,
   tmzUTCPlus1030,
   tmzUTCPlus1100,
   tmzUTCPlus1130,
   tmzUTCPlus1200,
   tmzUTCPlus1245,
   tmzUTCPlus1300,
   tmzUTCPlus1400,
   tmzUTCUnknown
} TIPM_Time_Zone_Type_t;

   /* The following enumerated type represents all of the valid Daylight*/
   /* Savings Time (DST) Offset values that may be assigned in the Local*/
   /* Time Information.                                                 */
typedef enum
{
   tmdStandardTime,
   tmdHalfAnHourDaylightTime,
   tmdDaylightTime,
   tmdDoubleDaylightTime,
   tmdUnknown
} TIPM_DST_Offset_Type_t;

   /* The following enumerated type represents all of the valid Months  */
   /* of the Year values that may be assigned in the Current Time.      */
typedef enum
{
   tmyUnknown,  
   tmyJanuary,  
   tmyFebruary, 
   tmyMarch,    
   tmyApril,    
   tmyMay,      
   tmyJune,     
   tmyJuly,     
   tmyAugust,   
   tmySeptember,
   tmyOctober,  
   tmyNovember, 
   tmyDecember, 
} TIPM_Month_Of_Year_Type_t;

   /* The following enumerated type represents all of the valid Day of  */
   /* the Week values that may be assigned in the Current Time.         */
typedef enum
{
   twdUnknown,  
   twdMonday,   
   twdTuesday,  
   twdWednesday,
   twdThursday, 
   twdFriday,   
   twdSaturday, 
   twdSunday,   
} TIPM_Week_Day_Type_t;

   /* The following enumerated type represents the possible Time Update */
   /* States of a remote TIP server.                                    */
typedef enum
{
   tcsIdle,
   tcsUpdatePending
} TIPM_Time_Update_State_Current_State_t;

   /* The following enumerated type represents the possible result      */
   /* values of a time update.                                          */
typedef enum
{
   treSuccessful,
   treCanceled,
   treNoConnectionToReference,
   treReferenceRespondedWithError,
   treTimeout,
   treUpdateNotAttemptedAfterReset
} TIPM_Time_Update_State_Result_t;

   /* The following structure represents the formation of a Local Time  */
   /* Information value.  This is used to represent the value of the    */
   /* Local Time Information.  The first member to this structure       */
   /* contains the Local Time Zone.  The second member contains the     */
   /* current Daylight Savings Time offset.                             */
typedef  struct _tagTIPM_Local_Time_Information_Data_t
{
   TIPM_Time_Zone_Type_t  Time_Zone;
   TIPM_DST_Offset_Type_t Daylight_Saving_Time;
}  TIPM_Local_Time_Information_Data_t;

#define TIPM_LOCAL_TIME_INFORMATION_DATA_SIZE            (sizeof(TIPM_Local_Time_Information_Data_t))

   /* The following structure represents the formation of a Reference   */
   /* Time Information value.  This is used to represent the value of   */
   /* the Reference Time Information.  The first member to this         */
   /* structure contains the source of the time information.  The second*/
   /* member contains the accuracy (drift) of the local time information*/
   /* specified in units of 1/8 of a second.  The final two member      */
   /* contain the days and hours since the information was updated.     */
typedef struct _tagTIPM_Reference_Time_Information_Data_t
{
   unsigned long           Flags;
   TIPM_Time_Source_Type_t Source;
   unsigned int            Accuracy;
   unsigned int            Days_Since_Update;
   unsigned int            Hours_Since_Update;
}  TIPM_Reference_Time_Information_Data_t;

#define TIPM_REFERENCE_TIME_INFORMATION_DATA_SIZE        (sizeof(TIPM_Reference_Time_Information_Data_t))

   /* The following bit masks define the valid bits that may be set in  */
   /* the Flags member of the TIPM_Reference_Time_Information_Data_t    */
   /* structure.                                                        */
   /* * NOTE * If the                                                   */
   /*       TIPM_REFERENCE_TIME_INFORMATION_FLAGS_REFERENCE_TIME_KNOWN  */
   /*          bit is NOT SET in the Flags member then this indicates   */
   /*          that none of the fields in the                           */
   /*          TIPM_Reference_Time_Information_Data_t are valid and they*/
   /*          are all assumed to be unknown.                           */
   /* * NOTE * If the                                                   */
   /*       TIPM_REFERENCE_TIME_INFORMATION_FLAGS_REFERENCE_TIME_KNOWN  */
   /*          is NOT SET in the Flags member then the Accuracy field   */
   /*          in the TIPM_Reference_Time_Information_Data_t is         */
   /*          considered to be unknown.                                */
#define TIPM_REFERENCE_TIME_INFORMATION_FLAGS_REFERENCE_TIME_KNOWN     0x00000001
#define TIPM_REFERENCE_TIME_INFORMATION_FLAGS_ACCURACY_KNOWN           0x00000002

   /* The following structure represents the format of a TIPM Date/Time */
   /* value.  This is used to represent the Date-Time which contains the*/
   /* Day/Month/Year and Hours:Minutes:Second data.                     */
typedef struct _tagTIPM_Date_Time_Data_t
{
   unsigned int              Year;
   TIPM_Month_Of_Year_Type_t Month;
   unsigned int              Day;
   unsigned int              Hours;
   unsigned int              Minutes;
   unsigned int              Seconds;
} TIPM_Date_Time_Data_t;

#define TIPM_DATE_TIME_DATA_SIZE                         (sizeof(TIPM_Date_Time_Data_t))

   /* The following structure represents the format of a TIPM           */
   /* Day/Data/Time value.  This structure is used to represent the     */
   /* Date/Time and the Day of the Week (Sunday - Saturday).            */
typedef struct _tagTIPM_Day_Date_Time_Data_t
{
   TIPM_Date_Time_Data_t DateTime;
   TIPM_Week_Day_Type_t  DayOfWeek;
} TIPM_Day_Date_Time_Data_t;

#define TIPM_DAY_DATE_TIME_DATA_SIZE                     (sizeof(TIPM_Day_Date_Time_Data_t))

   /* The following structure represents the format of a TIPM Exact Time*/
   /* value.  This structure is used to represent the Day/Date/Time and */
   /* the Fractions256 (1/256 of second) value.                         */
typedef struct _tagTIPM_Exact_Time_Data_t
{
   TIPM_Day_Date_Time_Data_t DayDateTime;
   unsigned int              Fractions256;
} TIPM_Exact_Time_Data_t;

#define TIPM_EXACT_TIME_DATA_SIZE                        (sizeof(TIPM_Exact_Time_Data_t))

   /* The following structure represents the formation of a Current     */
   /* Time value.  This is used represent the value of the Current Time */
   /* Characteristic.  The first member of this structure contains the  */
   /* Extact Time value.  The second member is a bit mask (that must be */
   /* made of TIPM_CURRENT_TIME_ADJUST_REASON_XXX bits) that specifies  */
   /* the reason that the time was adjusted.                            */
typedef struct _tagTIPM_Current_Time_Data_t
{
   TIPM_Exact_Time_Data_t ExactTime;
   unsigned long          AdjustReasonMask;
}  TIPM_Current_Time_Data_t;

#define TIPM_CURRENT_TIME_DATA_SIZE                      (sizeof(TIPM_Current_Time_Data_t))

   /* The following defines the valid adjust reason of the current time.*/
#define TIPM_CURRENT_TIME_ADJUST_REASON_MANUAL_TIME_UPDATE             0x01
#define TIPM_CURRENT_TIME_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE 0x02
#define TIPM_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_TIMEZONE             0x04
#define TIPM_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_DST                  0x08

   /* The following structure represents the time until the next DST    */
   /* change and the DST offset that will go into effect at that time.  */
typedef struct _tagTIPM_Time_With_DST_Data_t
{
   TIPM_Date_Time_Data_t   DateTime;
   TIPM_DST_Offset_Type_t  DSTOffset;
} TIPM_Time_With_DST_Data_t;

   /* The following structure represents the format of the Time Update  */
   /* State data that a remote TIP server will return.                  */
typedef struct _tagTIPM_Time_Update_State_Data_t
{
   TIPM_Time_Update_State_Current_State_t CurrentState;
   TIPM_Time_Update_State_Result_t        Result;
} TIPM_Time_Update_State_Data_t;

   /* The following structure is a container for the data in regards to */
   /* a connected remote Time Profile device.                           */
typedef struct _tagTIPM_Remote_Device_t
{
   TIPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SupportedServicesMask;
} TIPM_Remote_Device_t;
   
#endif


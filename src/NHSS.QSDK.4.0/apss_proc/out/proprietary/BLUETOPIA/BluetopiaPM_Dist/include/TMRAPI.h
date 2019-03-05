/*****< tmrapi.h >*************************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TMRAPI - Timer Module for Stonestreet One Bluetopia Platform Manager      */
/*           API Prototypes and Constants.                                    */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/25/00  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __TMRAPIH__
#define __TMRAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "BTPMCFG.h"            /* BTPM Configuration Settings/Constants.     */

#define TMR_MINIMUM_TIMER_RESOLUTION    (BTPM_CONFIGURATION_TIMER_MINIMUM_TIMER_RESOLUTION_MS)
                                                /* This is the minimum time   */
                                                /* (in Milliseconds) that a   */
                                                /* a Timer can wait.  Any     */
                                                /* Number less than this will */
                                                /* be forced to wait for this */
                                                /* length of time.            */

   /* The following structure represents the Time Stamp Information that*/
   /* is returned by the TMR_GetTimeStamp() API.                        */
typedef struct _tagTMR_TimeStamp_t
{
   unsigned int Hour;
   unsigned int Minute;
   unsigned int Second;
   unsigned int Milliseconds;
   unsigned int Month;
   unsigned int Day;
   unsigned int Year;
} TMR_TimeStamp_t;

#define TMR_TIMESTAMP_SIZE                         (sizeof(TMR_TimeStamp_t))

   /* The following is a type definition of the User Supplied Timer     */
   /* Callback function.  This function is called in the Timer Thread   */
   /* and NOT the thread that the timer was established in.             */
   /* If the Callback function returns TRUE, then the Timer remains     */
   /* active (periodic), else if the function returns FALSE, then the   */
   /* timer is destroyed and the TimerID is invalid.                    */
   /* ** NOTE ** The caller should keep the processing of these Timer   */
   /*            Callbacks small because other Timers will not be able  */
   /*            to be called while one is being serviced.              */
   /* ** NOTE ** If the callback function gets hung in an infinite loop */
   /*            then ALL Timers will be hung and Timer System will be  */
   /*            Useless !!!!!!!  If this sounds like an outrageous     */
   /*            limitation, remember that unless a thread is created   */
   /*            for every timer, there is NO way around this in even   */
   /*            Windows.  This limitation exists in the Windows        */
   /*            SetTimer()/KillTimer() schema as well.  This is a      */
   /*            limitation placed on the way Windows handles Timer     */
   /*            Events (i.e. Not at a system level, but at an          */
   /*            Application level).                                    */
   /* ** NOTE ** TMR_ChangeTimer() can be called from a Timer Callback, */
   /*            however, the constraints placed on this mechanism mean */
   /*            that if the Timer is Changed, and the Callback function*/
   /*            returns FALSE (non periodic), then the Timer is        */
   /*            deleted.  If a Timer Callback changes a timer which    */
   /*            is itself, then the Timer Callback *MUST* return TRUE, */
   /*            or the Timer will be deleted, making the call to       */
   /*            TMR_ChangeTimer() worthless.                           */
   /* ** NOTE ** The return value of this function is irrelevant if     */
   /*            the Timer that the callback is servicing is deleted    */
   /*            via a call to TMR_StopTimer() during the Timer         */
   /*            Callback.                                              */
typedef Boolean_t (BTPSAPI *TMR_TimerCallbackFunction_t)(unsigned int TimerID, void *CallbackParameter);

   /* Start a Timer of the specified Time out (in Milliseconds and NOT  */
   /* zero), and call the specified Timer Callback function when this   */
   /* completes.  The UserParameter is a parameter that is passed to    */
   /* the Timer Callback function when it is called.  This function     */
   /* returns the Timer ID of the newly created Timer.  This Timer ID   */
   /* can be used when calling TMR_StopTimer() if the specified Timer   */
   /* has not expired.  The Timer ID's are greater than zero, and if    */
   /* zero is returned, then a Timer was not established.               */
   /* ** NOTE ** The minimum timout value is                            */
   /*            TMR_MINIMUM_TIMER_RESOLUTION.  If a timeout that is    */
   /*            less than this value (non zero) is specified, the      */
   /*            TMR_MINIMUM_TIMER_RESOLUTION will become the Time Out  */
   /*            value the timer will have !!!!!                        */
BTPSAPI_DECLARATION unsigned int BTPSAPI TMR_StartTimer(void *CallbackParameter, TMR_TimerCallbackFunction_t CallbackFunction, unsigned long TimeOut);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_TMR_StartTimer_t)(void *CallbackParameter, TMR_TimerCallbackFunction_t CallbackFunction, unsigned long TimeOut);
#endif

   /* This function Stops the specified Timer from expiring.  The       */
   /* Timer ID parameter must be obtained via a call to                 */
   /* TMR_StartTimer().  This function returns TRUE if the Timer was    */
   /* aborted, or FALSE if the operation was unable to complete         */
   /* successfully.                                                     */
BTPSAPI_DECLARATION Boolean_t BTPSAPI TMR_StopTimer(unsigned int TimerID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_TMR_StopTimer_t)(unsigned int TimerID);
#endif

   /* This function Changes the specified Timer Time Out Period.  The   */
   /* Timer ID parameter must be obtained via a call to                 */
   /* TMR_StartTimer().  This function returns TRUE if the Timer        */
   /* timeout was changed, FALSE if the operation was unable to complete*/
   /* successfully.                                                     */
   /* ** NOTE ** The minimum timout valus is                            */
   /*            TMR_MINIMUM_TIMER_RESOLUTION.  If a timeout that is    */
   /*            less than this value (non zero) is specified, the      */
   /*            TMR_MINIMUM_TIMER_RESOLUTION will become the Time Out  */
   /*            value the timer will have !!!!!                        */
   /* ** NOTE ** This function can be called from a Timer Callback,     */
   /*            however, the constraints placed on this mechanism mean */
   /*            that if the Timer is Changed, and the Callback function*/
   /*            returns FALSE (non periodic), then the Timer is        */
   /*            deleted.  If a Timer Callback changes a timer which    */
   /*            is itself, then the Timer Callback *MUST* return TRUE, */
   /*            or the Timer will be deleted, making the call to       */
   /*            TMR_ChangeTimer() worthless.                           */
BTPSAPI_DECLARATION Boolean_t BTPSAPI TMR_ChangeTimer(unsigned int TimerID, unsigned long NewTimeOut);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_TMR_ChangeTimer_t)(unsigned int TimerID, unsigned long NewTimeOut);
#endif

   /* This function is used to return the current system millisecond    */
   /* count.  This function returns the millisecond count directly to   */
   /* the caller.                                                       */
BTPSAPI_DECLARATION unsigned long BTPSAPI TMR_GetMillisecondCount(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_TMR_GetMillisecondCount_t)(void);
#endif

   /* This function is used to return a system time stamp.  The only    */
   /* parameter to this function is a pointer to a structure to return  */
   /* the timestamp.  This function returns 0 if successful or a        */
   /* negative error code if not successful.                            */
BTPSAPI_DECLARATION int BTPSAPI TMR_GetTimeStamp(TMR_TimeStamp_t *TimeStamp);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TMR_GetTimeStamp_t)(TMR_TimeStamp_t *TimeStamp);
#endif

#endif

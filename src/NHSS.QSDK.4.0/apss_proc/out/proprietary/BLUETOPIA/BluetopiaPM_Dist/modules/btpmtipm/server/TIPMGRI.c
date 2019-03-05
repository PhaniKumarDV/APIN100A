/*****< tipmgri.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMGRI - TIP Manager Implementation for Stonestreet One Bluetooth        */
/*            Protocol Stack Platform Manager (Platform specific portion).    */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/20/13  D. Lange       Initial creation.                               */
/******************************************************************************/
#include <time.h>
#include <sys/time.h>

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "TIPMGRI.h"             /* TIP Manager Platform Implementation.      */

#include "SS1BTCTS.h"            /* Bluetopia CTS Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to to     */
   /* fetch the current time of the current platform (implementation    */
   /* specific) and formats it in the format required for the Current   */
   /* Time Service.  This function returns TRUE if successful, or FALSE */
   /* if there was an error.                                            */
Boolean_t _TIPM_Get_Current_Platform_Time(CTS_Current_Time_Data_t *CurrentTime, unsigned long AdjustMask)
{
   Boolean_t       ret_val;
   struct tm      *Time;
   struct timeval  TimeVal;

   /* First, make sure the input parameter appears to be semi-valid.    */
   if(CurrentTime)
   {
      if(!gettimeofday(&TimeVal, NULL))
      {
         /* Convert Time of Day return (specified in seconds) to the    */
         /* actual parsed time and date.                                */
         if((Time = localtime(&(TimeVal.tv_sec))) != NULL)
         {
            /* Configure the Current Time structure.                    */
            CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Hours    = (Byte_t)Time->tm_hour;
            CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Minutes  = (Byte_t)Time->tm_min;
            CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Seconds  = (Byte_t)Time->tm_sec;
            CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Day      = (Byte_t)Time->tm_mday;
            CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Month    = (Byte_t)(Time->tm_mon + 1);
            CurrentTime->Exact_Time.Day_Date_Time.Date_Time.Year     = (Word_t)(Time->tm_year + 1900);

            /* Everything is mapped directly from Linux for the Day of  */
            /* the Week EXCEPT Sunday (0 in Linux) so handle this.      */
            if(Time->tm_wday)
               CurrentTime->Exact_Time.Day_Date_Time.Day_Of_Week     = (CTS_Week_Day_Type_t)Time->tm_wday;
            else
            {
               /* Map Sunday to the CTS Sunday Value.                   */
               CurrentTime->Exact_Time.Day_Date_Time.Day_Of_Week     = (CTS_Week_Day_Type_t)CTS_DAY_OF_WEEK_SUNDAY;
            }

            /* Determine the number of 1/256th of a second.             */

            /* First determine the number of milliseconds.              */
            TimeVal.tv_usec                                         /= 1000;
            TimeVal.tv_usec                                         %= 1000;

            /* Next, figure out how many 1/256ths units it is.          */
            TimeVal.tv_usec                                          = (TimeVal.tv_usec * 256)/1000;

            CurrentTime->Exact_Time.Fractions256                     = (Byte_t)TimeVal.tv_usec;
            CurrentTime->Adjust_Reason_Mask                          = (Byte_t)AdjustMask;

            /* Flag success to the caller.                              */
            ret_val                                                  = TRUE;
         }
         else
            ret_val = FALSE;
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


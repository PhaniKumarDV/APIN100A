/*****< pxpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMGR - PXP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PXPMGRH__
#define __PXPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPXPM.h"           /* PXP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PXP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PXP Manager  */
   /* Implementation.                                                   */
int _PXPM_Initialize(void);

   /* The following function is responsible for shutting down the PXP   */
   /* Manager Implementation.  After this function is called the PXP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PXPM_Initialize() function.  */
void _PXPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the PXP Manager      */
   /* Service.  This Callback will be dispatched by the PXP Manager when*/
   /* various PXP Manager Monitor Events occur.  This function returns a*/
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the PXP    */
   /*          Monitor Event Handler ID.  This value can be passed to   */
   /*          the _PXPM_Un_Register_Monitor_Events() function to       */
   /*          Un-Register the Monitor Event Handler.                   */
int _PXPM_Register_Monitor_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PXP Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _PXPM_Register_Monitor_Events() function).  This function accepts */
   /* input the PXP Event Handler ID (return value from                 */
   /* _PXPM_Register_Monitor_Events() function).                        */
int _PXPM_Un_Register_Monitor_Events(unsigned int MonitorEventHandlerID);

   /* The following function is provided to allow a mechanism of        */
   /* changing the refresh time for checking the Path Loss (the time    */
   /* between checking the path loss for a given link).  This function  */
   /* accepts as it's parameter the MonitorEventHandlerID that was      */
   /* returned from a successful call to _PXPM_Register_Monitor_Events()*/
   /* and the Refresh Time (in milliseconds).  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _PXPM_Set_Path_Loss_Refresh_Time(unsigned int MonitorEventHandlerID, unsigned int RefreshTime);

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Threshold for a specified PXP Client       */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert.  This   */
   /* function accepts as it's parameter the MonitorEventHandlerID that */
   /* was returned from a successful call to                            */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Client        */
   /* Connection to set the path loss for, and the Path Loss Threshold  */
   /* to set.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The Path Loss Threshold should be specified in units of  */
   /*          dBm.                                                     */
int _PXPM_Set_Path_Loss_Threshold(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, int PathLossThreshold);

   /* The following function is provided to allow a mechanism of        */
   /* querying the current Path Loss for a specified PXP Monitor        */
   /* Connection.  This function accepts as it's parameter the          */
   /* MonitorEventHandlerID that was returned from a successful call to */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Monitor       */
   /* Connection to set the path loss for, and a pointer to a buffer to */
   /* return the current Path Loss in (if successfull).  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The Path Loss Threshold will be specified in units of    */
   /*          dBm.                                                     */
int _PXPM_Query_Current_Path_Loss(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, int *PathLossThreshold);

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Alert Level for a specified PXP Client     */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorEventHandlerID that was returned from a successful call to */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Client        */
   /* Connection to set the path loss alert level for, and the Path Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int _PXPM_Set_Path_Loss_Alert_Level(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel);

   /* The following function is provided to allow a mechanism of        */
   /* changing the Link Loss Alert Level for a specified PXP Client     */
   /* Connection.  If the Link Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorEventHandlerID that was returned from a successful call to */
   /* _PXPM_Register_Monitor_Events(), the BD_ADDR of the Client        */
   /* Connection to set the path loss alert level for, and the Link Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
int _PXPM_Set_Link_Loss_Alert_Level(unsigned int MonitorEventHandlerID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel);

#endif

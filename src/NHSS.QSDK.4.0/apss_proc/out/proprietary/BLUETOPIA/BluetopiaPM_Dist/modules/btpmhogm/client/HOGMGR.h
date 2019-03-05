/*****< hidmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HOGMGR - HOG Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HOGMGRH__
#define __HOGMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HOGM_Initialize(void);

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HOGM_Initialize() function.  */
void _HOGM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HID Manager      */
   /* Service.  This Callback will be dispatched by the HID Manager when*/
   /* various HID Manager Events occur.  This function returns a        */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the HID    */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HOGM_Un_Register_HID_Events() function to Un-Register   */
   /*          the Event Handler.                                       */
int _HOGM_Register_HID_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _HOGM_Register_HID_Events() function).  This function accepts     */
   /* input the HID Event Handler ID (return value from                 */
   /* _HOGM_Register_HID_Events() function).                            */
int _HOGM_Un_Register_HID_Events(unsigned int HOGEventsHandlerID);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the HID       */
   /* Manager Service to explicitly process HID Data.  This Callback    */
   /* will be dispatched by the HID Manager when various HID Manager    */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HOGM_Send_Report_Data() function to send data).         */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          in the system.                                           */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HOGM_Un_Register_HID_Data_Events() function to          */
   /*          un-register the callback from this module.               */
int _HOGM_Register_HID_Data_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Data Event        */
   /* Callback (registered via a successful call to the                 */
   /* _HOGM_Register_HID_Data_Events() function).  This function accepts*/
   /* as input the HID Manager Event Callback ID (return value from     */
   /* _HOGM_Register_HID_Data_Events() function).                       */
int _HOGM_Un_Register_HID_Data_Events(unsigned int HOGDataEventHandlerID);

   /* The following function is provided to allow a mechanism of setting*/
   /* the HID Protocol Mode on a remote HID Device.  This function      */
   /* accepts as input the HOG Manager Data Event Handler ID (return    */
   /* value from _HOGM_Register_HID_Data_Events() function), the BD_ADDR*/
   /* of the remote HID Device and the Protocol Mode to set.  This      */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
int _HOGM_Set_Protocol_Mode(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Protocol_Mode_t ProtocolMode);

   /* The following function is provided to allow a mechanism of        */
   /* informing the specified remote HID Device that the local HID Host */
   /* is entering/exiting the Suspend State.  This function accepts as  */
   /* input the HOG Manager Data Event Callback ID (return value from   */
   /* _HOGM_Register_HID_Data_Events() function), the BD_ADDR of the    */
   /* remote HID Device and the a Boolean that indicates if the Host is */
   /* entering suspend state (TRUE) or exiting suspend state (FALSE).   */
   /* This function returns zero on success or a negative error code.   */
int _HOGM_Set_Suspend_Mode(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Suspend);

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Get Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from _HOGM_Register_HID_Data_Events() function), */
   /* the BD_ADDR of the remote HID Device and a pointer to a structure */
   /* containing information on the Report to set.  This function       */
   /* returns the positive, non-zero, Transaction ID of the request on  */
   /* success or a negative error code.                                 */
int _HOGM_Get_Report_Request(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation);

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Set Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from _HOGM_Register_HID_Data_Events() function), */
   /* the BD_ADDR of the remote HID Device, a pointer to a structure    */
   /* containing information on the Report to set, a Boolean that       */
   /* indicates if a response is expected, and the Report Data to set.  */
   /* This function returns the positive, non-zero, Transaction ID of   */
   /* the request (if a Response is expected, ZERO if no response is    */
   /* expected) on success or a negative error code.                    */
int _HOGM_Set_Report_Request(unsigned int HOGDataEventHandlerID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation, Boolean_t ResponseExpected, unsigned int ReportDataLength, Byte_t *ReportData);

#endif

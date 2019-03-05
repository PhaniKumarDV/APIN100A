/*****< hrpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMGR - HRP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __HRPMGRH__
#define __HRPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HRP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HRP Manager  */
   /* Implementation.                                                   */
int _HRPM_Initialize(void);

   /* The following function is responsible for shutting down the HRP   */
   /* Manager Implementation.  After this function is called the HRP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HRPM_Initialize() function.  */
void _HRPM_Cleanup(void);

   /* The following function is responsible for informing the HRP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HRP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HRPM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function will parse a HRPM Heart Rate Measurement   */
   /* Event given a Heart Rate Measurement dispatched by this module.   */
   /* The first parameter is a pointer to the Heart Rate Measurement    */
   /* Event dispatched by this module.  The second parameter is a       */
   /* pointer to the Heart Rate Measurement Event to be dispatched by   */
   /* BTPMHRPM.  This function returns zero on success; otherwise, a    */
   /* negative value is returned.                                       */
   /* * NOTE * _HRPM_Free_Heart_Rate_Measurement_Event_RR_Interval must */
   /*          be called with a pointer to the formatted Heart Rate     */
   /*          Measurement Event after the event is no longer needed.   */
int _HRPM_Decode_Heart_Rate_Measurement_Event_To_Event(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent, HRPM_Heart_Rate_Measurement_Event_Data_t *HeartRateMeasurementEventEvent);

   /* The following function will free any resources allocated by a call*/
   /* to _HRPM_Decode_Heart_Rate_Measurement_Event_To_Event.  The first */
   /* parameter is a pointer to the HRPM Heart Rate Measurement Event   */
   /* that was formatted.                                               */
void _HRPM_Free_Heart_Rate_Measurement_Event_RR_Interval(HRPM_Heart_Rate_Measurement_Event_Data_t *HeartRateMeasurementEventEvent);

   /* The following function will parse a HRPM Heart Rate Measurement   */
   /* Message given a Heart Rate Measurement dispatched by this module. */
   /* The first parameter is a pointer to the Heart Rate Measurement    */
   /* Event dispatched by this module.  This function returns a pointer */
   /* to a Heart Rate Measurement Message on success; otherwise, NULL is*/
   /* returned.                                                         */
   /* * NOTE * _HRPM_Free_Heart_Rate_Measurement_Message must be called */
   /*          with a pointer to the formatted Heart Rate Measurement   */
   /*          Event after the event is no longer needed.               */
HRPM_Heart_Rate_Measurement_Message_t *_HRPM_Decode_Heart_Rate_Measurement_Event_To_Message(Heart_Rate_Measurement_Event_t *HeartRateMeasurementEvent);

   /* The following function will free any resources allocated by a call*/
   /* to _HRPM_Decode_Heart_Rate_Measurement_Event_To_Message.  The     */
   /* first parameter is a pointer to the HRPM Heart Rate Measurement   */
   /* Message that was formatted.                                       */
void _HRPM_Free_Heart_Rate_Measurement_Message(HRPM_Heart_Rate_Measurement_Message_t *HeartRateMeasurementMessage);

   /* The following function will submit a Get Body Sensor Location     */
   /* request to a remote sensor.  The first parameter is a pointer to a*/
   /* dynamically allocated Transaction structure that will be used to  */
   /* submit the request and provide information in the corresponding   */
   /* response.  The transaction data will be passed back in the        */
   /* corresponding event, and it is NOT freed internal to this module. */
   /* The second parameter is the Attribute Handle of the Body Sensor   */
   /* Location Characteristic on the remote sensor.  This function will */
   /* return zero on success; otherwise, a negative error code will be  */
   /* returned.                                                         */
int _HRPM_Get_Body_Sensor_Location(Get_Body_Sensor_Location_Transaction_Data_t *TransactionData, Word_t Handle);

   /* The following function will submit a Reset Energy Expended request*/
   /* to a remote sensor.  The first parameter is a pointer to a        */
   /* dynamically allocated Transaction structure that will be used to  */
   /* submit the request and provide information in the corresponding   */
   /* response.  The transaction data will be passed back in the        */
   /* corresponding event, and it is NOT freed internal to this module. */
   /* The second parameter is the Attribute Handle of the Heart Rate    */
   /* Control Point Characteristic on the remote sensor.  This function */
   /* will return zero on success; otherwise, a negative error code will*/
   /* be returned.                                                      */
int _HRPM_Reset_Energy_Expended(Reset_Energy_Expended_Transaction_Data_t *TransactionData, Word_t Handle);

   /* The following function will submit a Write Measurement CCD request*/
   /* to a remote sensor.  The first parameter is a pointer to a        */
   /* dynamically allocated Transaction structure that will be used to  */
   /* submit the request and provide information in the corresponding   */
   /* response.  The transaction data will be passed back in the        */
   /* corresponding event, and it is NOT freed internal to this module. */
   /* The second parameter is the Attribute Handle of the Heart Rate    */
   /* Client Characteristic Descriptor on the remote sensor.  This      */
   /* function will return zero on success; otherwise, a negative error */
   /* code will be returned.                                            */
int _HRPM_Write_Measurement_CCD(Write_Measurement_CCD_Transaction_Data_t *TransactionData, Word_t Handle, Word_t ClientConfigurationValue);

#endif

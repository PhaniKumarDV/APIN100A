/*****< btpmhrpm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMHRPM - HRP Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMHRPMH__
#define __BTPMHRPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTGAT.h"            /* Bluetopia GATT Prototypes/Constants.      */
#include "SS1BTHRS.h"            /* Bluetopia HRS Prototypes/Constants.       */

#include "HRPMAPI.h"             /* HRP Manager API Prototypes/Constants.     */

   /* The following type is used with the HRPM_Update_Data_t structure  */
   /* (which is used with the HRPM_NotifyUpdate() function to inform the*/
   /* HRP Manager that an Update needs to be dispatched.                */
typedef enum
{
   utGATTNotificationEvent,
   utHRPCollectorEvent
} HRPM_Update_Type_t;

   /* The following type represents all Collector Events that will be   */
   /* dispatched by this module.                                        */
typedef enum
{
   cetGetBodySensorLocationResponse,
   cetResetEnergyExpendedResponse,
   cetWriteMeasurementCCDResponse
} HRP_Collector_Event_Type_t;

   /* The following structure holds information used during the Get Body*/
   /* Sensor Location transaction to identify the recipient of the      */
   /* transaction and the internal client requesting the transaction.   */
typedef struct _tagGet_Body_Sensor_Location_Transaction_Data_t
{
   unsigned int AddressID;
   BD_ADDR_t    RemoteSensor;
} Get_Body_Sensor_Location_Transaction_Data_t;

   /* The following structure holds information used during the Reset   */
   /* Energy Expended transaction to identify the recipient of the      */
   /* transaction and the internal client requesting the transaction.   */
typedef struct _tagReset_Energy_Expended_Transaction_Data_t
{
   unsigned int AddressID;
   BD_ADDR_t    RemoteSensor;
} Reset_Energy_Expended_Transaction_Data_t;

   /* The following structure holds information used during the Write   */
   /* Measurement CCD transaction to identify the recipient of the      */
   /* transaction.  This transaction is executed by the HRPM Service, so*/
   /* no internal mapping is needed.                                    */
typedef struct _tagWrite_Measurement_CCD_Transaction_Data_t
{
   BD_ADDR_t RemoteSensor;
} Write_Measurement_CCD_Transaction_Data_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a cetHeartRateMeasurementEvent    */
   /* event.                                                            */
typedef struct _tagHeart_Rate_Measurement_Event_t
{
   BD_ADDR_t  RemoteSensor;
   Word_t     BufferLength;
   Byte_t    *Buffer;
} Heart_Rate_Measurement_Event_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a cetGetBodySensorLocationResponse*/
   /* event.                                                            */
typedef struct _tagGet_Body_Sensor_Location_Response_t
{
   Get_Body_Sensor_Location_Transaction_Data_t *TransactionData;
   Byte_t                                       ResponseStatus;
   Byte_t                                       BodySensorLocation;
} Get_Body_Sensor_Location_Response_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a cetResetEnergyExpendedResponse  */
   /* event.                                                            */
typedef struct _tagReset_Energy_Expended_Response_t
{
   Reset_Energy_Expended_Transaction_Data_t *TransactionData;
   Byte_t                                    ResponseStatus;
} Reset_Energy_Expended_Response_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a cetWriteMeasurementCCDResponse  */
   /* event.                                                            */
typedef struct _tagWrite_Measurement_CCD_Response_t
{
   Write_Measurement_CCD_Transaction_Data_t *TransactionData;
   Byte_t                                    ResponseStatus;
} Write_Measurement_CCD_Response_t;

   /* The following structure represents the container structure for    */
   /* holding all HRP collector event data.                             */
typedef struct _tagHRP_Collector_Event_Data_t
{
   HRP_Collector_Event_Type_t  EventDataType;
   union
   {
      Get_Body_Sensor_Location_Response_t GetBodySensorLocationResponse;
      Reset_Energy_Expended_Response_t    ResetEnergyExpendedResponse;
      Write_Measurement_CCD_Response_t    WriteMeasurementCCDResponse;
   } EventData;
} HRP_Collector_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the HRPM_NotifyUpdate() function).                           */
typedef struct _tagHRPM_Update_Data_t
{
   HRPM_Update_Type_t UpdateType;
   union
   {
      GATT_Server_Notification_Data_t GATTServerNotificationData;
      HRP_Collector_Event_Data_t      CollectorEventData;
   } UpdateData;
} HRPM_Update_Data_t;

   /* The following function is provided to allow the caller the ability*/
   /* to notify the HRP Manager of a specific Update Event.  The HRP    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t HRPM_NotifyUpdate(HRPM_Update_Data_t *UpdateData);

#endif

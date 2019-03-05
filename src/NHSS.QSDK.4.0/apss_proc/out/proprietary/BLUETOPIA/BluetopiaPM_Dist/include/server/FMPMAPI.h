/*****< fmpmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FMPMAPI - Find Me Profile (FMP) Manager API for Stonestreet One Bluetooth */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/27/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FMPMAPIH__
#define __FMPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "FMPMMSG.h"             /* BTPM FMP Manager Message Formats.         */

   /* The following enumerated type represents the FMP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of FMP Manager Changes.                                           */
typedef enum
{
   aetFMPAlert
} FMPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetFMPAlert event.              */
typedef struct _tagFMPM_Alert_Event_Data_t
{
   unsigned int           TargetCallbackID;
   FMPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   FMPM_Alert_Level_t     AlertLevel;
} FMPM_Alert_Event_Data_t;

#define FMPM_ALERT_EVENT_DATA_SIZE                             (sizeof(FMPM_Alert_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Alet Notification Profile (FMP) Manager Event (and Event Data) of */
   /* a FMP Manager Event.                                              */
typedef struct _tagFMPM_Event_Data_t
{
   FMPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      FMPM_Alert_Event_Data_t AlertEventData;
   } EventData;
} FMPM_Event_Data_t;

#define FMPM_EVENT_DATA_SIZE                                   (sizeof(FMPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Alert Notification Profile (FMP) Manager dispatches an event (and */
   /* the client has registered for events).  This function passes to   */
   /* the caller the FMP Manager Event and the Callback Parameter that  */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the Event Data ONLY in the context of */
   /* this callback.  If the caller requires the Data for a longer      */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).        */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* Message will not be processed while this function call is         */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *FMPM_Event_Callback_t)(FMPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager FMP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI FMPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI FMPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (FMP) Manager Service.  This Callback will be        */
   /* dispatched by the FMP Manager when various FMP Manager Server     */
   /* Events occur.  This function accepts the Callback Function and    */
   /* Callback Parameter (respectively) to call when a FMP Manager      */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          FMPM_Un_Register_Target_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI FMPM_Register_Target_Event_Callback(FMPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FMPM_Register_Target_Event_Callback_t)(FMPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered FMP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* FMPM_Register_Target_Event_Callback() function).  This function   */
   /* accepts as input the FMP Manager Target Event Callback ID (return */
   /* value from FMPM_Register_Target_Event_Callback() function).       */
BTPSAPI_DECLARATION void BTPSAPI FMPM_Un_Register_Target_Event_Callback(unsigned int TargetCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_FMPM_Un_Register_Target_Event_Callback_t)(unsigned int TargetCallbackID);
#endif

#endif

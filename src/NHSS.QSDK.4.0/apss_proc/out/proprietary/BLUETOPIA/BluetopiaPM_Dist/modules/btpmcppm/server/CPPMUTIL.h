/*****< CPPMUTIL.h >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CPPMUTIL - Cycling Power Profile Manager Server Utility Functions         */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation.                               */
/******************************************************************************/
#ifndef __CPPMUTILH__
#define __CPPMUTILH__


   /* The ProcedureRecord field, of the Procedure_Record_t type, of the */
   /* sensor instance entry structure is used to track control point    */
   /* procedures when in progress.                                      */
typedef struct _tagProcedure_Record_t
{
   unsigned int  CallbackID;
   unsigned int  AddressID;
   BD_ADDR_t    *BluetoothAddress;
   unsigned int  TimerID;
} Procedure_Record_t;

   /* A list of type Instance_Entry_t is an element of a device entry  */
   /* in the global device list. It is used to record the sensor       */
   /* instances on a particular sensor.                                */	
typedef struct _tagInstance_Entry_t
{
   unsigned int                 InstanceID;
   unsigned int                 StateMask;
   unsigned long                FeatureMask;
   CPS_Client_Information_t     ServiceHandles;
   CPPM_Sensor_Location_t       SensorLocation;
   Procedure_Record_t           ProcedureRecord;
   struct _tagInstance_Entry_t *NextInstanceEntry;
} Instance_Entry_t;

   /* Elements of a global remote sensor list are of the type           */
   /* Device_Entry_t. The sensor instance information is recorded in    */
   /* the InstanceEntryList list field.                                 */
typedef struct _tagDevice_Entry_t
{
   BD_ADDR_t                  BluetoothAddress;
   Instance_Entry_t          *InstanceEntryList;
   struct _tagDevice_Entry_t *NextDeviceEntry;
} Device_Entry_t;

   /* The field TransactionType in the Transaction list is of the type  */
   /* Transaction_Type_t. It identifies what was being attempted.       */   
typedef enum _tagTransaction_Type_t
{
   ttEnableCyclingPowerMeasurement,
   ttEnableCyclingPowerVector,
   ttEnableCyclingPowerControlPoint,
   ttEnableCyclingPowerBroadcast,
   ttDisableCyclingPowerMeasurement,
   ttDisableCyclingPowerVector,
   ttDisableCyclingPowerControlPoint,
   ttDisableCyclingPowerBroadcast,
   ttWriteCyclingPowerControlPoint,
   ttReadCyclingPowerMeasurementClientDescriptor,
   ttReadCyclingPowerMeasurementServerDescriptor,
   ttReadCyclingPowerVectorClientDescriptor,
   ttReadCyclingPowerControlPointClientDescriptor,
   ttReadCyclingPowerSensorLocation,
   ttReadCyclingPowerSensorFeatures
} Transaction_Type_t;

   /* Entries of the type Transaction_Entry_t make up the transaction   */
   /* list of a callback entry. TransactionID is set to the GATM        */
   /* transaction ID.                                                   */ 
typedef struct _tagTransaction_Entry_t
{
   unsigned int                     TransactionID;
   Transaction_Type_t               TransactionType;
   struct _tagTransaction_Entry_t  *NextTransactionEntry;
} Transaction_Entry_t;

   /* The field UnsolicitedUpdateEntryList in a callback entry is a     */
   /* list of Unsolicited_Update_Entry_t type entries. It is used to    */
   /* record what unsolicited updates(notifications or indications)     */
   /* have  been registered for. The Handle field is set to the handle  */
   /* of the measurement, vector or control point characteristic.       */  
typedef struct _tagUnsolicited_Update_Entry_t
{
   BD_ADDR_t                              BluetoothAddress;
   unsigned int                           InstanceID;
   Word_t                                 Handle;
   struct _tagUnsolicited_Update_Entry_t *NextUnsolicitedUpdateEntry;
} Unsolicited_Update_Entry_t;

   /* The entries of the global callback list are of the type           */
   /* Callback_Entry_t. Each callback entry records transactions and    */
   /* registered unsolicited updates.                                   */
typedef struct _tagCallback_Entry_t
{
   unsigned int                 CallbackID;
   unsigned int                 AddressID;
   CPPM_Event_Callback_t        EventCallback;
   void                        *CallbackParameter;
   Transaction_Entry_t         *TransactionEntryList;
   Unsolicited_Update_Entry_t  *UnsolicitedUpdateEntryList;
   struct _tagCallback_Entry_t *NextCallbackEntry;
} Callback_Entry_t;



   /*********************************************************************/
   /* Utility Function Declarations                                     */
   /*    Remote Device List Functions                                   */
   /*       Service Instance List Functions                             */
   /*    Registered Callback List Functions                             */
   /*       Transaction List Functions                                  */
   /*       Active Notifiers and Indicators List Functions              */
   /*********************************************************************/


   /*********************************************************************/
   /* Remote Device List Utility Functions                              */
   /*********************************************************************/
Boolean_t AddDeviceEntryActual(Device_Entry_t **DeviceEntryList, Device_Entry_t *EntryToAdd);
Device_Entry_t *SearchDeviceEntry(Device_Entry_t **DeviceEntryList, BD_ADDR_t *Address);
Device_Entry_t *DeleteDeviceEntry(Device_Entry_t **DeviceEntryList, BD_ADDR_t *Address);
void FreeDeviceEntryList(Device_Entry_t **DeviceEntryList);
void FreeDeviceEntryMemory(Device_Entry_t *EntryToFree);



   /*********************************************************************/
   /* Service Instance List Utility Functions                           */
   /* Instance Lists are elements of Device entries.                    */
   /*********************************************************************/
Instance_Entry_t *SearchDeviceInstanceEntry(Device_Entry_t **DeviceEntryList, BD_ADDR_t *Address, unsigned int InstanceID);
Instance_Entry_t *SearchInstanceEntryByAttributeHandle(Instance_Entry_t **EntryList, unsigned int AttributeOffset, Word_t AttributeHandle);
Instance_Entry_t *SearchInstanceEntry(Instance_Entry_t **EntryList, unsigned int InstanceID);
Boolean_t AddInstanceEntryActual(Instance_Entry_t **EntryList, Instance_Entry_t *EntryToAdd);
unsigned int CalculateNumberOfInstances(Instance_Entry_t *InstanceEntryList);



   /*********************************************************************/
   /* Callback Entry List Utility Functions                             */
   /*********************************************************************/
Callback_Entry_t *AddCallbackEntry(Callback_Entry_t **CallbackEntryList, Callback_Entry_t *EntryToAdd);
Callback_Entry_t *DeleteCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID);
void SearchCallbackEntryByTransactionID(Callback_Entry_t **CallbackEntryList, Callback_Entry_t **CallbackEntry, Transaction_Entry_t **TransactionEntry,  unsigned int TransactionID);
Callback_Entry_t *SearchCallbackEntry(Callback_Entry_t **CallbackEntryList, unsigned int CallbackID);
void FreeCallbackEntryList(Callback_Entry_t **EntryToFree);
void FreeCallbackEntryMemory(Callback_Entry_t *EntryToFree);



   /*********************************************************************/
   /* Transaction List Utility Functions                                */
   /* Transaction Lists are elements of Callback entries.               */
   /*********************************************************************/
Transaction_Entry_t *AddTransactionEntry(Transaction_Entry_t **TransactionEntryList, Transaction_Entry_t *EntryToAdd);
void FreeTransactionEntryMemory(Transaction_Entry_t *EntryToFree);



   /*********************************************************************/
   /* Unsolicited Update Entry List Utility Functions                   */
   /* Unsolicited Update are elements of Callback entries               */
   /*********************************************************************/
Unsolicited_Update_Entry_t *AddUnsolicitedUpdateEntry(Unsolicited_Update_Entry_t **EntryList, Unsolicited_Update_Entry_t *EntryToAdd);
Unsolicited_Update_Entry_t *SearchUnsolicitedUpdateEntry(Unsolicited_Update_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID, Word_t Handle);
Unsolicited_Update_Entry_t *DeleteUnsolicitedUpdateEntry(Unsolicited_Update_Entry_t **EntryList, BD_ADDR_t *BluetoothAddress, unsigned int InstanceID, Word_t Handle);
void FreeUnsolicitedUpdateEntryMemory(Unsolicited_Update_Entry_t *EntryToFree);



void GetNextID(unsigned int *NextID);

#endif

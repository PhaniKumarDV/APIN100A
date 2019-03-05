/*****< rscmgr.h >*************************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  RSCMGR - RSC Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/18/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __RSCMGRH__
#define __RSCMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTRSCM.h"           /* RSC Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the RSC Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager RSC Manager  */
   /* Implementation.                                                   */
int _RSCM_Initialize(void);

   /* The following function is responsible for shutting down the RSC   */
   /* Manager Implementation.  After this function is called the RSC    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _RSCM_Initialize() function.  */
void _RSCM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Collector callback function with the RSC    */
   /* Manager Service.  This Callback will be dispatched by the RSC     */
   /* Manager when various RSC Manager Collector Events occur.  This    */
   /* function returns a positive (non-zero) value if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _RSCM_Un_Register_Collector_Event_Callback() function to */
   /*          un-register the callback from this module.               */
int _RSCM_Register_Collector_Event_Callback();

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered RSC Manager Collector         */
   /* Event Callback (registered via a successful call to the           */
   /* _RSCM_Register_Collector_Event_Callback() function).  This        */
   /* function accepts as input the Collector Event Callback ID (return */
   /* value from _RSCM_Register_Collector_Event_Callback() function).   */
int _RSCM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of entries that the buffer will      */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _RSCM_Query_Connected_Sensors(unsigned int MaximumRemoteDeviceListEntries, RSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices);

   /* The following function will attempt to configure a remote device  */
   /* which supports the RSC sensor role. It will register measurement  */
   /* and control point notifications and set the device up for         */
   /* usage. The RemoteDeviceAddress is the address of the remote       */
   /* sensor. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return from this call does not mean the     */
   /*          device has been configured. An                           */
   /*          etRSCConfigurationStatusChanged event will indicate the  */
   /*          status of the attempt to configure.                      */
int _RSCM_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags);

   /* The following function will un-configure a remote sensor which was*/
   /* previously configured. All notifications will be disabled. The    */
   /* RemoteDeviceAddress is the address of the remote sensor. This     */
   /* function returns zero if success and a negative return code if    */
   /* there was an error.                                               */
int _RSCM_Un_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress);

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
int _RSCM_Get_Connected_Sensor_Info(BD_ADDR_t RemoteDeviceAddress, RSCM_Connected_Sensor_t *DeviceInfo);

   /* The following function queries the current sensor location from a */
   /* remote sensor. The RemoteDeviceAddress parameter is the address of*/
   /* the remote sensor. This function returns zero if successful or a  */
   /* negative error code if there was an error.                        */
   /* * NOTE * If this function is succesful, the status of the request */
   /*          will be returned in a retRSCSensorLocationResponse event.*/
int _RSCM_Get_Sensor_Location(BD_ADDR_t RemoteDeviceAddress);

   /* The following function sends a control point opcode to update     */
   /* the cumulative value on a remote sensor. The RemoteDeviceAddress  */
   /* parameter is the address of the remote sensor. The CumulativeValue*/
   /* parameter is the value to set on the remote sensor. If successful,*/
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          retRSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          retRSCCumulativeValueUpdated event will notify all       */
   /*          registered callbacks that the value has changed.         */
int _RSCM_Update_Cumulative_Value(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue);

   /* The following function sends a control point opcode to update     */
   /* the sensor location on a remote sensor. The RemoteDeviceAddress   */
   /* parameter is the address of the remote sensor. The SensorLocation */
   /* parameter is the new location to set.  If successful, this        */
   /* function returns a postive integer which represents the Procedure */
   /* ID associated with this procedure. If there is an error, this     */
   /* function returns a negative error code.                           */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          retRSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          retRSCSensorLocationUpdated event will notify all        */
   /*          registered callbacks that the location has changed.      */
int _RSCM_Update_Sensor_Location(BD_ADDR_t RemoteDeviceAddress, RSCM_Sensor_Location_t SensorLocation);

   /* The following function sends a control point opcode to start      */
   /* sensor calibration on a remote sensor. The RemoteDeviceAddress    */
   /* parameter is the address of the remote sensor.  If successful,    */
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          retRSCProcedureComplete then attempt to resend.          */
int _RSCM_Start_Sensor_Calibration(BD_ADDR_t RemoteDeviceAddress);

#endif


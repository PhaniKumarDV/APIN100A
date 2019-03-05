/*****< CPPMGR.h >*************************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMGR - Cycling Power Platform Manager Client Header                     */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/
#ifndef __CPPMGRH__
#define __CPPMGRH__

   /* _CPPM_Register_Collector_Event_Callback sends the request to      */
   /* register cycling power event messages. If successful the response */
   /* status value will be the callback ID used in the API functions.   */
int _CPPM_Register_Collector_Event_Callback(void);

   /* _CPPM_Unregister_Collector_Event_Callback sends the request to    */
   /* unregister the client for cycling power callbacks.                */
int _CPPM_Unregister_Collector_Event_Callback(unsigned int CallbackID);

   /* _CPPM_Register_Updates sends the request to register for          */
   /* unsolicited updates (notifications or indications) from the       */
   /* specified sensor. A request can be made for Measurement and       */
   /* Vector notifications and Control Point indications. If            */
   /* EnabledUpdates is true then the remote client configuration       */
   /* descriptor will be written to enable the updates. The             */
   /* MessageFunction parameter identifies the request.                 */
int _CPPM_Register_Updates(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates, unsigned int MessageFunction);

   /* _CPPM_Request is the generic request function used when no data   */
   /* other than the address and instance ID needs to be sent with the  */
   /* request. The supplied message function parameter identifies the   */
   /* request.                                                          */
int _CPPM_Request(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int InstanceID, unsigned int MessageFunction);

   /* _CPPM_Write_Sensor_Control_Point sends an opcode and, depending   */
   /* on which opcode, additional procedure data to the PM server which */
   /* will be written to the control point characteristic of the        */
   /* specified sensor to initiate a procedure. See CPPMType.h for the  */                        
   /* type definition of the CPPM_Procedure_Data_t structure.           */
int _CPPM_Write_Sensor_Control_Point(unsigned int CallbackID, BD_ADDR_t *RemoteSensor, unsigned int InstanceID, CPPM_Procedure_Data_t ProcedureData);

   /* _CPPM_Query_Sensors sends the sensors query and, if necessary,    */
   /* copies the sensor addresses from the response into the provided   */
   /* buffer. The NumberOfSensors field in the response is set to the   */
   /* number of addresses in the Sensors field. It will never be        */
   /* greater than the value pointed to by the NumberOfSensors          */
   /* parameter. See CPPMMSG.h for the defintion of                     */
   /* CPPM_Query_Sensors_Response_t.                                    */
int  _CPPM_Query_Sensors(unsigned int CallbackID, unsigned int *NumberOfSensors, BD_ADDR_t *Sensors);

   /* _CPPM_Query_Sensor_Instances sends the instances query and, if    */
   /* necessary, copies the instance records from the response into the */
   /* provided Instances buffer. The NumberOfInstances field in the     */
   /* response is set to the number of records in the Instances field.  */
   /* It will never be greater than the value pointed to by the         */
   /* NumberOfInstances parameter. See CPPMType.h for the type          */
   /* defintion of Instance_Record_t and CPPMMSG.h for the defintion of */
   /* CPPM_Query_Sensor_Instances_Response_t.                           */
int  _CPPM_Query_Sensor_Instances(unsigned int CallbackID, BD_ADDR_t *Sensor, unsigned int *NumberOfInstances, Instance_Record_t *Instances);

#endif

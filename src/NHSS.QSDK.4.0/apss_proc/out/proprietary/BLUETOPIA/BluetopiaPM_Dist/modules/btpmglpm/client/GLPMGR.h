/*****< glpmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLPMGR - GLP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GLPMGRH__
#define __GLPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the GLP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager GLP Manager  */
   /* Implementation.                                                   */
int _GLPM_Initialize(void);

   /* The following function is responsible for shutting down the GLP   */
   /* Manager Implementation.  After this function is called the GLP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _GLPM_Initialize() function.  */
void _GLPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the GLP Manager      */
   /* Service.  This Callback will be dispatched by the GLP Manager when*/
   /* various GLP Manager Events occur.  This function returns a        */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the GLP    */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _GLPM_Un_Register_Collector_Events() function to         */
   /*          Un-Register the Event Handler.                           */
int _GLPM_Register_Collector_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered GLP Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _GLPM_Register_Collector_Events() function).  This function       */
   /* accepts input the GLP Event Handler ID (return value from         */
   /* _GLPM_Register_Collector_Events() function).                      */
int _GLPM_Un_Register_Collector_Events(unsigned int GLPMCollectorHandlerID);

   /* The following function is provided to allow a mechanism of        */
   /* starting a Glucose Procedure to a remote Glucose Device.  This    */
   /* function accepts as input the GLP Manager Collector Event Handler */
   /* ID (return value from _GLPM_Register_Collector_Events() function),*/
   /* the BD_ADDR of the remote Glucose Device and a pointer to a       */
   /* structure containing the procedure data.  This function returns   */
   /* the positive, non-zero, Procedure ID of the request on success or */
   /* a negative error code.                                            */
int _GLPM_Start_Procedure_Request(unsigned int GLPMCollectorHandlerID, BD_ADDR_t RemoteDeviceAddress, GLPM_Procedure_Data_t *ProcedureData);

   /* The following function is provided to allow a mechanism of        */
   /* stopping a previouly started Glucose Procedure to a remote Glucose*/
   /* Device.  This function accepts as input the GLP Manager Collector */
   /* Event Handler ID (return value from                               */
   /* _GLPM_Register_Collector_Events() function), the BD_ADDR of the   */
   /* remote Glucose Device and the Procedure ID that was returned via a*/
   /* successfull call to GLPM_Start_Procedure_Request().  This function*/
   /* returns zero on success or a negative error code.                 */
int _GLPM_Stop_Procedure_Request(unsigned int GLPMCollectorHandlerID, BD_ADDR_t RemoteDeviceAddress, unsigned int ProcedureID);

#endif

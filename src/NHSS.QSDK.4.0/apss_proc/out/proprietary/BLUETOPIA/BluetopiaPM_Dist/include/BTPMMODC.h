/*****< btpmmodc.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMOD - Installable Module Handler for Stonestreet One Bluetooth        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/12/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMMODCH__
#define __BTPMMODCH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* BTPM Main API Prototypes/Constants.       */

   /* The following type declaration represents the function declaration*/
   /* for the Initialization and Cleanup functions of modules that are  */
   /* to be installed automatically by the Module Handler.  The         */
   /* parameter to this function specifies whether or not the Module is */
   /* to be initialized (TRUE) or cleaned up (FALSE).                   */
   /* * NOTE * The final is used if initialization data needs to be     */
   /*          be passed to the module.                                 */
typedef void (BTPSAPI *MOD_InitializationHandlerFunction_t)(Boolean_t Initialize, void *InitializationData);

   /* The following type declaration represents the function declaration*/
   /* for the DEVM Handler function of modules that are to be installed */
   /* automatically by the Module Handler.  This function is dispatched */
   /* when the Device Manager dispatches an asynchronous event.         */
   /* * NOTE * Modules can process Power Events using this callback.    */
typedef void (BTPSAPI *MOD_DeviceManagerHandlerFunction_t)(DEVM_Event_Data_t *EventData);

   /* The following structure represents a container structure for an   */
   /* individual Module Handler Entry.  These entries will be part of a */
   /* list that will be platform specific.  The individual entries in   */
   /* this list will provide Initialization and Cleanup entry points for*/
   /* the module.  This will allow a platform specific compile time     */
   /* mechanism for extending the framework.                            */
typedef struct _tagMOD_ModuleHandlerEntry_t
{
   MOD_InitializationHandlerFunction_t  InitializationFunction;
   void                                *InitializationData;
   MOD_DeviceManagerHandlerFunction_t   DeviceMangerHandlerFunction;
} MOD_ModuleHandlerEntry_t;

#define MOD_MODULE_HANDLER_ENTRY_SIZE                    (sizeof(MOD_ModuleHandlerEntry_t))

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Module Handler Service.  This function */
   /* returns a pointer to a Module Handler Entry List (or NULL if there*/
   /* were no Modules installed).  The returned list will simply be an  */
   /* array of Module Handler Entries, with the last enty in the list   */
   /* signified by an entry that has NULL present for all Module Handler*/
   /* Functions.                                                        */
MOD_ModuleHandlerEntry_t *BTPSAPI MOD_GetModuleList(void);

   /* The following function is a utility function to be used by modules*/
   /* to add their EIR data to the EIR data managed by DEVM (if DEVM is */
   /* compiled to support this).  This normally is the list of Service  */
   /* Class UUIDs that are supported by the module.  This function      */
   /* simply takes a length and pointer to the preformatted Length - Tag*/
   /* - Value structures of the EIR data to add.                        */
void MOD_AddEIRData(unsigned int EIRDataLength, Byte_t *EIRData);

#endif

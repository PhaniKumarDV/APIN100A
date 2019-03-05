/*****< debugapi.h >***********************************************************/
/*      Copyright 2004 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DEBUGAPI - Stonestreet One Bluetooth Stack File Debugging API Type        */
/*             Definitions, Constants, and Prototypes.                        */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/11/04  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __DEBUGAPIH__
#define __DEBUGAPIH__

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */
#include "BTTypes.h"            /* Bluetooth basic type definitions           */

   /* Error Return Codes.                                               */
#define BTPS_DEBUG_ERROR_INVALID_STACK_ID             (-1)/* Error that       */
                                                        /* denotes that the   */
                                                        /* specified Bluetooth*/
                                                        /* Stack ID is invalid*/
                                                        /* (or that there was */
                                                        /* error registering  */
                                                        /* a debug callback   */
                                                        /* with the specified */
                                                        /* Bluetooth Stack).  */

#define BTPS_DEBUG_ERROR_INVALID_PARAMETER            (-2)/* Error that       */
                                                        /* denotes that one   */
                                                        /* (or more)          */
                                                        /* parameters were    */
                                                        /* invalid.           */

#define BTPS_DEBUG_ERROR_INITIALIZING_DEBUG           (-4)/* Error that       */
                                                        /* denotes that an    */
                                                        /* error occurred     */
                                                        /* during Debug       */
                                                        /* Initialization.    */

#define BTPS_DEBUG_ERROR_INVALID_DEBUG_ID             (-5)/* Error that       */
                                                        /* denotes that the   */
                                                        /* specified Debug ID */
                                                        /* supplied to the    */
                                                        /* Cleanup function   */
                                                        /* is invalid.        */

#define BTPS_DEBUG_ERROR_UNKNOWN                      (-6)/* Error that is not*/
                                                        /* covered by any     */
                                                        /* other Error Code.  */

   /* The following enumeration specifies the available debug output    */
   /* methods.                                                          */
typedef enum
{
   dtLogFile,
   dtDebugTerminal,
   dtFTS
} BTPS_Debug_Type_t;

   /* The following constants represent Bit flags that can be specified */
   /* as Debug Flags in the Debug Parameters structure.                 */
#define BTPS_DEBUG_FLAGS_APPEND_FILE                  0x00000001
#define BTPS_DEBUG_FLAGS_PREPEND_BLUETOOTH_ADDRESS    0x00000002

   /* The following structure is used to specify information that is    */
   /* required to setup the debug option.  Currently the following      */
   /* options are available:                                            */
   /*    - dtLogFile - ASCII dump output to file                        */
   /*         - ParameterString member specifies output file name.      */
   /*         - DebugFlags member specifies BTPS_DEBUG_FLAGS_APPEND_FILE*/
   /*           for append existing file, or zero to create new file.   */
   /*         - DebugFlags member specifies                             */
   /*           BTPS_DEBUG_FLAGS_PREPEND_BLUETOOTH_STACK_ID to prepend  */
   /*           Bluetooth Stack ID for each output line (for possible   */
   /*           filtering).                                             */
   /*    - dtDebugTerminal - ASCII dump output to debug terminal        */
   /*         - ParameterString member is not used.                     */
   /*         - DebugFlags member specifies                             */
   /*           BTPS_DEBUG_FLAGS_PREPEND_BLUETOOTH_STACK_ID to prepend  */
   /*           Bluetooth Stack ID for each output line (for possible   */
   /*           filtering).                                             */
   /*    - dtFTS - FTS (BTSnoop) binary log file output                 */
   /*         - ParameterString member specifies output file name.      */
   /*         - DebugFlags member specifies BTPS_DEBUG_FLAGS_APPEND_FILE*/
   /*           for append existing file, or zero to create new file.   */
   /* * NOTE * In the case of the ParameterString member being used to  */
   /*          specify a Filename, the encoding type is considered to   */
   /*          be UTF-8 encoding on platforms that support character    */
   /*          sets other than ASCII.                                   */
typedef struct _tagBTPS_Debug_Parameters_t
{
   BTPS_Debug_Type_t  DebugType;
   unsigned long      DebugFlags;
   char              *ParameterString;
} BTPS_Debug_Parameters_t;

   /* The following function is responsible for Opening a Debug Hook for*/
   /* a specified stack instance.  The parameter to this function *MUST**/
   /* be specified.  If this function is successful, the caller will    */
   /* receive a non-zero, non-negative return value which serves as the */
   /* BTPSDebugID parameter.  If this function fails then the return    */
   /* value is a negative error code (see error codes above).           */
BTPSAPI_DECLARATION int BTPSAPI BTPS_Debug_Initialize(unsigned int BluetoothStackID, BTPS_Debug_Parameters_t *DebugParameters);

#ifdef INCLUDE_DEBUG_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPS_Debug_Initialize_t)(unsigned int BluetoothStackID, BTPS_Debug_Parameters_t *DebugParameters);
#endif

   /* The following function is responsible for Closing the Debug Hook  */
   /* that was opened via a successful call to the                      */
   /* BTPS_Debug_Initialize() function.  The Input parameter to this    */
   /* function MUST have been acquired by a successful call to          */
   /* BTPS_Debug_Initialize().  Once this function completes, the Debug */
   /* Hook that was closed cannot be accessed again until the Hook is   */
   /* Re-Opened by calling the BTPS_Debug_Initialize() function.        */
BTPSAPI_DECLARATION void BTPSAPI BTPS_Debug_Cleanup(unsigned int BluetoothStackID, unsigned int BTPSDebugID);

#ifdef INCLUDE_DEBUG_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BTPS_Debug_Cleanup_t)(unsigned int BluetoothStackID, unsigned int BTPSDebugID);
#endif

#endif


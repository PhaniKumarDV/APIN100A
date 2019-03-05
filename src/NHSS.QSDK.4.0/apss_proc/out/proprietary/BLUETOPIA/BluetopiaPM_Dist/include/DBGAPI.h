/*****< dbgapi.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DBGAPI - Debug Module API for the Stonestreet One Bluetopia Platform      */
/*           Manager.                                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/25/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __DBGAPIH__
#define __DBGAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMDBGZ.h"            /* Include BTPM Debug Zone definitions.      */

   /* Compiler specific declarations for defining cdecl calling         */
   /* convention.                                                       */
#ifdef BTPM_CONFIGURATION_DEBUG_CDECL_DECLARATION

   #define BTPMDBG_CDECL                           BTPM_CONFIGURATION_DEBUG_CDECL_DECLARATION

#else

   #define BTPMDBG_CDECL

#endif

   /* The following value represents the default Debug Zones that are   */
   /* to be used (if they are not explicitly over-ridden using the      */
   /* SetDebugZone() function.                                          */
#define BTPM_DEFAULT_DEBUG_ZONES_PAGE_0         (BTPM_CONFIGURATION_DEBUG_DEFAULT_DEBUG_ZONES_PAGE_0)
#define BTPM_DEFAULT_DEBUG_ZONES_PAGE_1         (BTPM_CONFIGURATION_DEBUG_DEFAULT_DEBUG_ZONES_PAGE_1)
#define BTPM_DEFAULT_DEBUG_ZONES_PAGE_2         (BTPM_CONFIGURATION_DEBUG_DEFAULT_DEBUG_ZONES_PAGE_2)
#define BTPM_DEFAULT_DEBUG_ZONES_PAGE_3         (BTPM_CONFIGURATION_DEBUG_DEFAULT_DEBUG_ZONES_PAGE_3)

   /* The following definition represents the Debugging prefix that is  */
   /* prepended to all debug output from this module.  This allows the  */
   /* ability for filtering of the debug output in a log file.  If no   */
   /* Prefix is desired then this definition should not be defined (as  */
   /* opposed to being defined as an empty string).                     */
#ifdef BTPM_CONFIGURATION_DEBUG_DEBUG_PREFIX_FORMAT

   #define BTPM_DEBUG_PREFIX_FORMAT                BTPM_CONFIGURATION_DEBUG_DEBUG_PREFIX_FORMAT

   #define BTPM_DEBUG_PREFIX_PARAMETERS            BTPM_CONFIGURATION_DEBUG_DEBUG_PREFIX_PARAMETERS

#endif

   /* The following definition represents the maximum number of bytes   */
   /* that will be output via the DebugDump() MACRO/function.  This     */
   /* constant allows the ability to truncate the total number of bytes */
   /* output for each call to stream-line the debugging output.         */
   /* Specifying a value of zero means to NOT truncate the debug output.*/
#define BTPM_MAX_DEBUG_DUMP                     (BTPM_CONFIGURATION_DEBUG_MAXIMUM_DEBUG_DUMP_LIMIT)

   /* The following constant will allow the inclusion of the debug      */
   /* functionality to be included.                                     */
   /* * NOTE * This also allows the ability for the debugging framework */
   /*          to be built without altering the configuration file (if  */
   /*          this functionality is required).                         */
#if (BTPM_CONFIGURATION_DEBUG_DEBUG_FRAMEWORK_PRESENT != 0)

   #define __BTPMDEBUGAPI__

#endif

   /* The following preprocessor definitions control the inclusion of   */
   /* debugging output.                                                 */
   /*                                                                   */
   /*    - __BTPMDEBUGAPI__                                             */
   /*         - When defined enables debugging.                         */
   /*                                                                   */
   /*          - BTPM_DEFAULT_DEBUG_ZONES                               */
   /*              - When defined (only when __BTPMDEBUG__ is defined)  */
   /*                forces the value of this definition (unsigned long)*/
   /*                to be the Debug Zones that are enabled by default. */

   /* Debugging zones that control Debugging output.                    */

   /* The following represent the various debugging zones that can be   */
   /* enabled for the module.  There can be a maximum of 32 debug zones.*/
   /* * NOTE * These values are the actual Bit Masks and not indexes.   */

   /* The following debug zone defines a method to force debug messages.*/
#define BTPM_DEBUG_LEVEL_ALL                    0x000000FF

   /* The following debug values define the specific Debug Levels       */
#define BTPM_DEBUG_LEVEL_FUNCTION               0x00000001
#define BTPM_DEBUG_LEVEL_DATA_DUMP              0x00000002
#define BTPM_DEBUG_LEVEL_VERBOSE                0x00000004
#define BTPM_DEBUG_LEVEL_TIMEOUT                0x00000008
#define BTPM_DEBUG_LEVEL_WARNING                0x00000010
#define BTPM_DEBUG_LEVEL_FAILURE                0x00000020
#define BTPM_DEBUG_LEVEL_CRITICAL               0x00000040

   /* The following defines the level (equal to or greater that is      */
   /* printed regardless of the zone).                                  */
#define BTPM_DEBUG_LEVEL_FORCE_PRINT            0x00000010

   /* The following constants specify the current Debug Zone Mask Page  */
   /* constants that are applied to the Debug Zone Mask to determine    */
   /* what page is being addressed.                                     */
#define BTPM_DEBUG_ZONE_NUMBER_ZONE_PAGES       0x00000004
#define BTPM_DEBUG_ZONE_PAGE_MASK               0xC0000000

#define BTPM_DEBUG_ZONE_PAGE_SHIFT              0x0000001E

   /* The following defines the zone masks for debug zones of the       */
   /* platform manager.                                                 */
#define BTPM_DEBUG_ZONE_INIT                    0x00000100
#define BTPM_DEBUG_ZONE_SETTINGS                0x00000200
#define BTPM_DEBUG_ZONE_TIMER                   0x00000400
#define BTPM_DEBUG_ZONE_MODULE_MANAGER          0x00000800
#define BTPM_DEBUG_ZONE_IPC                     0x00001000
#define BTPM_DEBUG_ZONE_MESSAGE                 0x00002000
#define BTPM_DEBUG_ZONE_MAIN                    0x00004000
#define BTPM_DEBUG_ZONE_LOCAL_DEVICE            0x00008000
#define BTPM_DEBUG_ZONE_SERIAL_PORT             0x00010000
#define BTPM_DEBUG_ZONE_SCO                     0x00020000
#define BTPM_DEBUG_ZONE_GENERIC_ATTRIBUTE       0x00040000

   /* Note that implementation specific debug zones are included in the */
   /* header file included at the beginning of this file.               */

   /* The follow zone defines a mask for all debug zones for the        */
   /* platform manager.                                                 */
#define BTPM_DEBUG_ZONE_ALL                     (0xFFFFFF00 & (~BTPM_DEBUG_ZONE_PAGE_MASK))

   /* * NOTE * The use of some of these MACRO's requires that the       */
   /*          parameters of each MACRO be bounded by parenthesis (any  */
   /*          parameter that is actually a variable argument length    */
   /*          parameter - currently only a single function has this    */
   /*          limitation).  To clarify, the following examples show    */
   /*          the format must be used when calling each MACRO:         */
   /*                                                                   */
   /*             DebugPrint(ZONE1, ("Result is %d\r\n", Result));      */
   /*             DebugDump(ZONE1, sizeof(Buffer), Buffer);             */
   /*             DebugZoneMask(NewZoneMask);                           */
   /*                                                                   */
   /*          Not all of the MACRO's would require this syntax.  The   */
   /*          only MACRO is the DebugPrint MACRO which requires that   */
   /*          any parameter specified after the Zone ID be enclosed in */
   /*          parenthesis.                                             */
   /* * NOTE * See the function prototypes at the bottom of this file   */
   /*          for the parameters that each of the MACROs accept.       */
   /* * NOTE * All debug strings that are accepted by this module are   */
   /*          in ASCII format.                                         */

   /* Define the correct MACRO's/functions based on the debugging       */
   /* requested (see note above about the preprocessor definitions that */
   /* control the debugging output).                                    */
#ifdef __BTPMDEBUGAPI__

   /* Some form of debugging is to be linked in, now determine the type */
   /* of requested debugging.                                           */

   #define DebugReconfigure(_x)              BTPM_DebugReconfigure(_x)

   #ifdef BTPM_DEBUG_PREFIX_FORMAT

      #if (defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6))

         /* For toolchains that support variadic macros and the C99     */
         /* snprintf API, avoid making multiple calls to BTPM_DebugPrint*/
         /* which, on some platforms, may cause interference in logs    */
         /* from multiple threads.                                      */

         #define _BTPM_DEBUG_EXPAND(...)        __VA_ARGS__

         #define DebugPrint(_x, _y)             do                                                                                                \
                                                {                                                                                                 \
                                                   if(BTPM_DebugQueryZone(_x))                                                                    \
                                                   {                                                                                              \
                                                      char _Prefix[128];                                                                          \
                                                      char _LogMsg[256];                                                                          \
                                                                                                                                                  \
                                                      _Pragma("GCC diagnostic push")                                                              \
                                                      _Pragma("GCC diagnostic ignored \"-Wformat\"")                                              \
                                                                                                                                                  \
                                                      snprintf(_Prefix, sizeof(_Prefix), BTPM_DEBUG_PREFIX_FORMAT, BTPM_DEBUG_PREFIX_PARAMETERS); \
                                                      snprintf(_LogMsg, sizeof(_LogMsg), _BTPM_DEBUG_EXPAND _y );                                 \
                                                                                                                                                  \
                                                      _Pragma("GCC diagnostic pop")                                                               \
                                                                                                                                                  \
                                                      BTPM_DebugPrint("%s%s", _Prefix, _LogMsg);                                                  \
                                                   }                                                                                              \
                                                } while(0)

      #else

         #define DebugPrint(_x, _y)             do                                                                             \
                                                {                                                                              \
                                                   if(BTPM_DebugQueryZone(_x))                                                 \
                                                   {                                                                           \
                                                      BTPM_DebugPrint(BTPM_DEBUG_PREFIX_FORMAT, BTPM_DEBUG_PREFIX_PARAMETERS); \
                                                      BTPM_DebugPrint _y;                                                      \
                                                   }                                                                           \
                                                } while(0)

      #endif

   #else

      #define DebugPrint(_x, _y)                do                                                                             \
                                                {                                                                              \
                                                   if(BTPM_DebugQueryZone(_x))                                                 \
                                                   {                                                                           \
                                                      BTPM_DebugPrint _y;                                                      \
                                                   }                                                                           \
                                                } while(0)
   #endif

   #define DebugPrint_Raw(_x, _y)            do                                                                                \
                                             {                                                                                 \
                                                if(BTPM_DebugQueryZone(_x))                                                    \
                                                {                                                                              \
                                                   BTPM_DebugPrint _y;                                                         \
                                                }                                                                              \
                                             } while(0)

   #define DebugDump(_x, _a, _b, _c)         BTPM_DebugDump(_x, _a, _b, _c)

   #define SetDebugZoneMask(_x)              BTPM_DebugSetZoneMask(_x)

   #define GetDebugZoneMask(_x)              BTPM_DebugGetZoneMask(_x)

   #define DebugQueryZone(_x)                BTPM_DebugQueryZone(_x)

#else

   /* Debugging disabled, make sure no debugging is linked in.          */

   #define DebugReconfigure(_x)

   #define DebugPrint(_x, _y)

   #define DebugPrint_Raw(_x, _y)

   #define DebugDump(_x, _a, _b, _c)

   #define SetDebugZoneMask(_x)

   #define GetDebugZoneMask(_x)              0

   #define DebugQueryZone(_x)

#endif

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Debug Module is initialized.  */
typedef struct _tagBTPM_Debug_Initialization_Data_t
{
   void *PlatformSpecificInitData;
} BTPM_Debug_Initialization_Data_t;

#define BTPM_DEBUG_INITIALIZATION_DATA_SIZE                       (sizeof(BTPM_Debug_Initialization_Data_t))

   /* The following function is provided to allow a mechanism to perform*/
   /* any required Debug Module Implementation reconfiguration          */
   /* functionality (as an example to select a new debug output method).*/
BTPSAPI_DECLARATION void BTPSAPI BTPM_DebugReconfigure(void *DebugReconfigureData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BTPM_DebugReconfigure_t)(void *DebugReconfigureData);
#endif

   /* The following function is a utility function that is responsible  */
   /* for actually formatting the specified Debug string and outputting */
   /* the string to the correct debug output mechansism for the         */
   /* implementation.                                                   */
BTPSAPI_DECLARATION void BTPMDBG_CDECL BTPM_DebugPrint(char *DebugString, ...);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPM_CDECL *PFN_BTPM_DebugPrint_t)(char *DebugString, ...);
#endif

   /* Dump the specified Data to the debug output stream.  The data will*/
   /* be formatted in an ASCII Coded Hexadecimal format with either     */
   /* ASCII characters and the corresponding ASCII coded Hexadecimal    */
   /* equivalent values or simply the ASCII coded Hexadecimal values    */
   /* (depending on the format requested).  The Formatted parameter     */
   /* controls how the output is formatted.  If this flag is TRUE (non  */
   /* zero) then the data is output in ASCII characters and the         */
   /* corresponding Hexadecimal equivalent values (with headers and     */
   /* offsets).  If this flag is FALSE (zero), then the data is simply  */
   /* output in ASCII coded hexadecimal with no formatting (not even a  */
   /* carriage return/line feed).                                       */
BTPSAPI_DECLARATION void BTPSAPI BTPM_DebugDump(unsigned long DebugZone, int Formatted, unsigned int DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BTPM_DebugDump_t)(unsigned long DebugZone, int Formatted, unsigned int DataLength, unsigned char *DataBuffer);
#endif

   /* The following function is a utility function that exists to       */
   /* enable/disable Debug Zones for the current module.                */
BTPSAPI_DECLARATION void BTPSAPI BTPM_DebugSetZoneMask(unsigned long NewDebugZoneMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BTPM_DebugSetZoneMask_t)(unsigned long NewDebugZoneMask);
#endif

   /* The following function is a utility function that exists to       */
   /* retreive the current Debug Zones Mask.                            */
BTPSAPI_DECLARATION unsigned long BTPSAPI BTPM_DebugGetZoneMask(unsigned int PageNumber);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_BTPM_DebugGetZoneMask_t)(unsigned int PageNumber);
#endif

   /* The following function is a utility function that can be used to  */
   /* determine if a specified Zone ID is currently enabled for         */
   /* debugging.                                                        */
BTPSAPI_DECLARATION int BTPSAPI BTPM_DebugQueryZone(unsigned long ZoneID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTPM_DebugQueryZone_t)(unsigned long ZoneID);
#endif

#endif

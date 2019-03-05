/*****< serapi.h >*************************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SERAPI - Stonestreet One Serial Port Driver Layer Static Library          */
/*           Prototypes and Constants.                                        */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/31/01  D. Lange       Initial creation.                               */
/*   06/21/02  R. Sledge      Ported to Linux.                                */
/******************************************************************************/
#ifndef __SERAPIH__
#define __SERAPIH__

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */
#include "BTTypes.h"            /* Bluetooth Type Definitions/Constants.      */

   /* Error Return Codes.                                               */
#define SER_DRIVER_ERROR_INVALID_PORT_NUMBER     (-1)   /* Error that denotes */
                                                        /* that the specified */
                                                        /* SER Port Number is */
                                                        /* invalid.           */

#define SER_DRIVER_ERROR_UNSUPPORTED_PORT        (-2)   /* Error that denotes */
                                                        /* that the specified */
                                                        /* SER Port cannot be */
                                                        /* opened because it  */
                                                        /* cannot be located  */
                                                        /* in the System.     */

#define SER_DRIVER_ERROR_INVALID_CALLBACK_INFORMATION (-3) /* Error that      */
                                                        /* denotes that the   */
                                                        /* specified Callback */
                                                        /* function pointer   */
                                                        /* is invalid.        */

#define SER_DRIVER_ERROR_INITIALIZING_DRIVER     (-4)   /* Error that denotes */
                                                        /* that an error      */
                                                        /* occurred during    */
                                                        /* Driver             */
                                                        /* Configuration.     */

#define SER_DRIVER_ERROR_INVALID_DRIVER_ID       (-5)   /* Error that denotes */
                                                        /* Driver ID supplied */
                                                        /* as function        */
                                                        /* argument is NOT    */
                                                        /* registered.        */

#define SER_DRIVER_ERROR_INVALID_PARAMETER       (-6)   /* Error that denotes */
                                                        /* a parameter that   */
                                                        /* was supplied as a  */
                                                        /* function argument  */
                                                        /* is NOT valid.      */

#define SER_DRIVER_ERROR_WRITING_TO_DEVICE       (-7)   /* Error that denotes */
                                                        /* that there was an  */
                                                        /* error while trying */
                                                        /* to write to the    */
                                                        /* SER Port Device.   */

#define SER_DRIVER_ERROR_READING_FROM_DEVICE     (-8)   /* Error that denotes */
                                                        /* that there was an  */
                                                        /* error while trying */
                                                        /* to read from the   */
                                                        /* SER Port Device.   */

#define SER_DRIVER_ERROR_UNKNOWN                 (-9)   /* Error that is not  */
                                                        /* covered by any     */
                                                        /* other Error Code.  */

   /* The following enumerated type represents the Status of the Break  */
   /* Signal for a specified Serial Port.                               */
typedef enum
{
   cbsBreakCleared,
   cbsBreakReceived
} SER_Break_Status_t;

   /* The following Constants represent the Bit Mask that specifies     */
   /* the value of an individual Modem/Port Control Signal.  If the     */
   /* specified Bit has a binary value of '1', then the Signal is       */
   /* considered to be set, else it is considered NOT set (clear).      */
   /* This Bit Mask is used with the SER_PortStatus() function.         */
#define SER_PORT_STATUS_CLEAR_VALUE                             0x00000000
#define SER_PORT_STATUS_RTS_CTS_BIT                             0x00000001
#define SER_PORT_STATUS_DTR_DSR_BIT                             0x00000002
#define SER_PORT_STATUS_RING_INDICATOR_BIT                      0x00000004
#define SER_PORT_STATUS_CARRIER_DETECT_BIT                      0x00000008

   /* The following Constants represent the Bit Mask that specifies     */
   /* the value of an individual Modem/Port Line Status Signal.  If the */
   /* specified Bit has a binary value of '1', then the Signal is       */
   /* considered to be set, else it is considered NOT set (clear).      */
   /* This Bit Mask is used with the SER_LineStatus() function.         */
#define SER_LINE_STATUS_NO_ERROR_VALUE                          0x00000000
#define SER_LINE_STATUS_OVERRUN_ERROR_BIT                       0x00000001
#define SER_LINE_STATUS_PARITY_ERROR_BIT                        0x00000002
#define SER_LINE_STATUS_FRAMING_ERROR_BIT                       0x00000004

   /* SER Port API Event Types.                                         */
typedef enum
{
   etSER_Break_Indication,
   etSER_Port_Status_Indication,
   etSER_Data_Indication,
   etSER_Line_Status_Indication
} SER_Event_Type_t;

typedef struct _tagSER_Break_Indication_Data_t
{
   unsigned int        SERPortID;
   SER_Break_Status_t  BreakStatus;
} SER_Break_Indication_Data_t;

#define SER_BREAK_INDICATION_DATA_SIZE                  (sizeof(SER_Break_Indication_Data_t))

typedef struct _tagSER_Port_Status_Indication_Data_t
{
   unsigned int SERPortID;
   unsigned int SERPortStatusMask;
} SER_Port_Status_Indication_Data_t;

#define SER_PORT_STATUS_INDICATION_DATA_SIZE            (sizeof(SER_Port_Status_Indication_Data_t))

typedef struct _tagSER_Data_Indication_Data_t
{
   unsigned int SERPortID;
   unsigned int DataLength;
} SER_Data_Indication_Data_t;

#define SER_DATA_INDICATION_DATA_SIZE                   (sizeof(SER_Data_Indication_Data_t))

typedef struct _tagSER_Line_Status_Indication_Data_t
{
   unsigned int SERPortID;
   unsigned int SERLineStatusMask;
} SER_Line_Status_Indication_Data_t;

#define SER_LINE_STATUS_INDICATION_DATA_SIZE            (sizeof(SER_Line_Status_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all SER Event Data Data.                                  */
typedef struct _tagSER_Event_Data_t
{
   SER_Event_Type_t Event_Data_Type;
   union
   {
      SER_Break_Indication_Data_t       SER_Break_Indication_Data;
      SER_Port_Status_Indication_Data_t SER_Port_Status_Indication_Data;
      SER_Data_Indication_Data_t        SER_Data_Indication_Data;
      SER_Line_Status_Indication_Data_t SER_Line_Status_Indication_Data;
   } Event_Data;
} SER_Event_Data_t;

   /* The following declared type represents the Prototype Function for */
   /* a SER Driver Receive Event Callback.  This function will be called*/
   /* whenever an event occurs on the specified SER Port that was opened*/
   /* with the specified Driver ID.  This function passes to the caller */
   /* the SER Driver ID, the Event Information that occurred and the SER*/
   /* Driver Callback Parameter that was specified when this Callback   */
   /* was installed.  The caller is free to use the contents of the SER */
   /* Event ONLY in the context of this callback.  If the caller        */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback function is installed for      */
   /* multiple SER Ports, then the function IS NOT guaranteed to NOT be */
   /* called simultaneously.  The function is only guaranteed to be     */
   /* called serially for a single context in which the Callback has    */
   /* been installed.  This means that if the same Callback function is */
   /* installed for two (or more) SER Ports, then the function COULD be */
   /* called simultaneously for both Events.  It should also be noted   */
   /* that this function is called in the Thread Context of a Thread    */
   /* that the User does NOT own.  Therefore, processing in this        */
   /* function should be as efficient as possible (this argument holds  */
   /* anyway because another Event will not be received while this      */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving SER Events.         */
   /*            A Deadlock WILL occur because NO Other Receive SER     */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *SER_DriverCallback_t)(unsigned int SERDriverID, SER_Event_Data_t *SEREventData, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a Serial Port   */
   /* Driver on the specified Serial Port.  The first parameter to this */
   /* function specifies the Serial Port Number of the actual Serial    */
   /* Port in the system.  The second parameter specifies the Baud Rate */
   /* to open the specified Serial Port with.  The third parameter      */
   /* specifies whether or not to use Hardware Flow Control (RTS/CTS and*/
   /* DTR/DSR), where TRUE means use Flow Control.  The fourth and fifth*/
   /* parameters specify the SER Driver Callback function that is to be */
   /* called when a SER Event Occurs.  All parameters to this function  */
   /* *MUST* be specified.  If this function is successful, the caller  */
   /* will receive a non-zero, non-negative return value which serves as*/
   /* the SERDriverID parameter for all other functions in the Serial   */
   /* Port Driver.  Since all SER functions require a valid SER Port    */
   /* Driver ID, this function must be called successfully before any   */
   /* other function can be called.  If this function fails then the    */
   /* return value is a negative error code (see error codes above).    */
BTPSAPI_DECLARATION int BTPSAPI SER_OpenDriver(unsigned int SERPortNumber, unsigned int BaudRate, Boolean_t HardwareFlowControl, SER_DriverCallback_t SERCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SER_OpenDriver_t)(unsigned int SERPortNumber, unsigned int BaudRate, Boolean_t HardwareFlowControl, SER_DriverCallback_t SERCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Closing the Serial Port */
   /* Driver that was opened via a successful call to the               */
   /* SER_OpenDriver() function.  The Input parameter to this function  */
   /* MUST have been acquired by a successful call to SER_OpenDriver(). */
   /* Once this function completes, the SER Port Driver that was closed */
   /* cannot be accessed again (sending/receiving data) by this module  */
   /* until the Driver is Re-Opened by calling the SER_OpenDriver()     */
   /* function.                                                         */
BTPSAPI_DECLARATION void BTPSAPI SER_CloseDriver(unsigned int SERDriverID);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_SER_CloseDriver_t)(unsigned int SERDriverID);
#endif

   /* The following function is responsible for Reading Serial Data from*/
   /* the specified Serial Port Driver.  The input parameters to this   */
   /* function are the SER Port Driver ID, the Size of the Data Buffer  */
   /* to be used for reading, and a pointer to the Data Buffer.  This   */
   /* function returns the number of data bytes that were successfully  */
   /* read (zero if there were no Data Bytes ready to be read), or a    */
   /* negative return error code if unsuccessful.                       */
BTPSAPI_DECLARATION int BTPSAPI SER_Read(unsigned int SERDriverID, unsigned int DataBufferSize, unsigned char *DataBuffer);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SER_Read_t)(unsigned int SERDriverID, unsigned int DataBufferSize, unsigned char *DataBuffer);
#endif

   /* The following function is responsible for Sending Serial Data to  */
   /* the specified Serial Port Driver.  The SERDriverID parameter that */
   /* is passed to this function MUST have been established via a       */
   /* successful call to the SER_OpenDriver() function.  The remaining  */
   /* parameters to this function are the Length of the Data to send and*/
   /* a pointer to the Data Buffer to Send.  This function returns the  */
   /* number of data bytes that were successfully sent, or a negative   */
   /* return error code if unsuccessful.                                */
BTPSAPI_DECLARATION int BTPSAPI SER_Write(unsigned int SERDriverID, unsigned int DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SER_Write_t)(unsigned int SERDriverID, unsigned int DataLength, unsigned char *DataBuffer);
#endif

   /* The following function is provided to allow the programmer a means*/
   /* to notify the Serial Port Driver of a Break Condition.  This      */
   /* function accepts as input the SER Port Driver ID of the SER Port  */
   /* that is to receive the Break Condition.  This function returns    */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI SER_SendBreak(unsigned int SERDriverID);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SER_SendBreak_t)(unsigned int SERDriverID);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to send the existing state of ALL Modem/Port Control       */
   /* Siganls to the Serial Port Driver.  This function accepts as input*/
   /* the SER Port Driver ID and the Current State of all the Modem     */
   /* Control Signals.  This function returns zero if successful, or a  */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI SER_PortStatus(unsigned int SERDriverID, unsigned int SERPortStatusMask);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SER_PortStatus_t)(unsigned int SERDriverID, unsigned int SERPortStatusMask);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to read the existing state of ALL Modem/Port Control       */
   /* Signals from the Serial Port Driver.  This function accepts as    */
   /* input the SER Port Driver ID and a pointer to a variable that is  */
   /* receive the Current State of all the Modem Control Signals.  This */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI SER_ReadPortStatus(unsigned int SERDriverID, unsigned int *SERPortStatusMask);

#ifdef INCLUDE_SER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SER_ReadPortStatus_t)(unsigned int SERDriverID, unsigned *int SERPortStatusMask);
#endif

#endif

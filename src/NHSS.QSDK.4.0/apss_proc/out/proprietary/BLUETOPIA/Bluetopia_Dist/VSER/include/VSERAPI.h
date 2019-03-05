/*****< vserapi.h >************************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  VSERAPI - Stonestreet One Virtual Serial Port Driver Layer Library Type   */
/*            Definitions, Prototypes, and Constants.                         */
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
#ifndef __VSERAPIH__
#define __VSERAPIH__

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */
#include "BTTypes.h"            /* Bluetooth Type Definitions/Constants.      */

   /* Error Return Codes.                                               */
#define VSER_DRIVER_ERROR_INVALID_PORT_NUMBER    (-1)   /* Error that denotes */
                                                        /* that the specified */
                                                        /* VSER Port Number is*/
                                                        /* invalid.           */

#define VSER_DRIVER_ERROR_UNSUPPORTED_PORT       (-2)   /* Error that denotes */
                                                        /* that the specified */
                                                        /* VSER Port cannot be*/
                                                        /* opened because it  */
                                                        /* cannot be located  */
                                                        /* in the System.     */

#define VSER_DRIVER_ERROR_INVALID_CALLBACK_INFORMATION (-3) /* Error that     */
                                                        /* denotes that the   */
                                                        /* specified Callback */
                                                        /* function pointer   */
                                                        /* is invalid.        */

#define VSER_DRIVER_ERROR_INITIALIZING_DRIVER    (-4)   /* Error that denotes */
                                                        /* that an error      */
                                                        /* occurred during    */
                                                        /* Driver             */
                                                        /* Configuration.     */

#define VSER_DRIVER_ERROR_INVALID_DRIVER_ID      (-5)   /* Error that denotes */
                                                        /* Driver ID supplied */
                                                        /* as function        */
                                                        /* argument is NOT    */
                                                        /* registered.        */

#define VSER_DRIVER_ERROR_INVALID_PARAMETER      (-6)   /* Error that denotes */
                                                        /* a parameter that   */
                                                        /* was supplied as a  */
                                                        /* function argument  */
                                                        /* is NOT valid.      */

#define VSER_DRIVER_ERROR_WRITING_TO_DEVICE      (-7)   /* Error that denotes */
                                                        /* that there was an  */
                                                        /* error while trying */
                                                        /* to write to the    */
                                                        /* VSER Port Device.  */

#define VSER_DRIVER_ERROR_READING_FROM_DEVICE    (-8)   /* Error that denotes */
                                                        /* that there was an  */
                                                        /* error while trying */
                                                        /* to read from the   */
                                                        /* VSER Port Device.  */

#define VSER_DRIVER_ERROR_UNKNOWN                (-9)   /* Error that is not  */
                                                        /* covered by any     */
                                                        /* other Error Code.  */

   /* The following enumerated type represents the Status of the Break  */
   /* Signal for a specified Virtual Serial Port.                       */
typedef enum
{
   vbsBreakCleared,
   vbsBreakReceived
} VSER_Break_Status_t;

   /* The following Constants represent the Bit Mask that specifies     */
   /* the value of an individual Modem/Port Control Signal.  If the     */
   /* specified Bit has a binary value of '1', then the Signal is       */
   /* considered to be set, else it is considered NOT set (clear).      */
   /* This Bit Mask is used with the VSER_PortStatus() function.        */
#define VSER_PORT_STATUS_CLEAR_VALUE                             0x00000000
#define VSER_PORT_STATUS_RTS_CTS_BIT                             0x00000001
#define VSER_PORT_STATUS_DTR_DSR_BIT                             0x00000002
#define VSER_PORT_STATUS_RING_INDICATOR_BIT                      0x00000004
#define VSER_PORT_STATUS_CARRIER_DETECT_BIT                      0x00000008

   /* The following Constants represent the Bit Mask that specifies     */
   /* the value of an individual Modem/Port Line Status Signal.  If the */
   /* specified Bit has a binary value of '1', then the Signal is       */
   /* considered to be set, else it is considered NOT set (clear).      */
   /* This Bit Mask is used with the VSER_LineStatus() function.        */
#define VSER_LINE_STATUS_NO_ERROR_VALUE                          0x00000000
#define VSER_LINE_STATUS_OVERRUN_ERROR_BIT                       0x00000001
#define VSER_LINE_STATUS_PARITY_ERROR_BIT                        0x00000002
#define VSER_LINE_STATUS_FRAMING_ERROR_BIT                       0x00000004

   /* VSER Port API Event Types.                                        */
typedef enum
{
   etVSER_Break_Indication,
   etVSER_Port_Status_Indication,
   etVSER_Data_Indication,
   etVSER_Line_Status_Indication
} VSER_Event_Type_t;

typedef struct _tagVSER_Break_Indication_Data_t
{
   unsigned int        VSERPortID;
   VSER_Break_Status_t BreakStatus;
} VSER_Break_Indication_Data_t;

#define VSER_BREAK_INDICATION_DATA_SIZE                  (sizeof(VSER_Break_Indication_Data_t))

typedef struct _tagVSER_Port_Status_Indication_Data_t
{
   unsigned int VSERPortID;
   unsigned int VSERPortStatusMask;
} VSER_Port_Status_Indication_Data_t;

#define VSER_PORT_STATUS_INDICATION_DATA_SIZE            (sizeof(VSER_Port_Status_Indication_Data_t))

typedef struct _tagVSER_Data_Indication_Data_t
{
   unsigned int VSERPortID;
   unsigned int DataLength;
} VSER_Data_Indication_Data_t;

#define VSER_DATA_INDICATION_DATA_SIZE                   (sizeof(VSER_Data_Indication_Data_t))

typedef struct _tagVSER_Line_Status_Indication_Data_t
{
   unsigned int VSERPortID;
   unsigned int VSERLineStatusMask;
} VSER_Line_Status_Indication_Data_t;

#define VSER_LINE_STATUS_INDICATION_DATA_SIZE            (sizeof(VSER_Line_Status_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all VSER Event Data Data.                                 */
typedef struct _tagVSER_Event_Data_t
{
   VSER_Event_Type_t Event_Data_Type;
   union
   {
      VSER_Break_Indication_Data_t       VSER_Break_Indication_Data;
      VSER_Port_Status_Indication_Data_t VSER_Port_Status_Indication_Data;
      VSER_Data_Indication_Data_t        VSER_Data_Indication_Data;
      VSER_Line_Status_Indication_Data_t VSER_Line_Status_Indication_Data;
   } Event_Data;
} VSER_Event_Data_t;

   /* The following declared type represents the Prototype Function for */
   /* a VSER Driver Receive Event Callback.  This function will be      */
   /* called whenever an event occurs on the specified VSER Port that   */
   /* was opened with the specified Driver ID.  This function passes to */
   /* the caller the VSER Driver ID, the Event Information that occurred*/
   /* and the VSER Driver Callback Parameter that was specified when    */
   /* this Callback was installed.  The caller is free to use the       */
   /* contents of the VSER Event ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback function is installed for      */
   /* multiple VSER Ports, then the function IS NOT guaranteed to NOT be*/
   /* called simultaneously.  The function is only guaranteed to be     */
   /* called serially for a single context in which the Callback has    */
   /* been installed.  This means that if the same Callback function is */
   /* installed for two (or more) VSER Ports, then the function COULD be*/
   /* called simultaneously for both Events.  It should also be noted   */
   /* that this function is called in the Thread Context of a Thread    */
   /* that the User does NOT own.  Therefore, processing in this        */
   /* function should be as efficient as possible (this argument holds  */
   /* anyway because another Event will not be received while this      */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving VSER Events.        */
   /*            A Deadlock WILL occur because NO Other Receive VSER    */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *VSER_DriverCallback_t)(unsigned int VSERDriverID, VSER_Event_Data_t *VSEREventData, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a Virtual Serial*/
   /* Port Driver Driver on the specified Serial Port.  The first       */
   /* parameter to this function specifies the Serial Port Number of the*/
   /* Actual Serial Port in the system.  The second and third parameters*/
   /* specify the VSER Driver Callback function that is to be called    */
   /* when a VSER Event Occurs.  All parameters to this function *MUST* */
   /* be specified.  If this function is successful, the caller will    */
   /* receive a non-zero, non-negative return value which serves as the */
   /* VSERDriverID parameter for all other functions in the VSER Driver.*/
   /* Since all VSER functions require a valid VSER Driver ID, this     */
   /* function must be called successfully before any other function can*/
   /* be called.  If this function fails then the return value is a     */
   /* negative error code (see error codes above).                      */
BTPSAPI_DECLARATION int BTPSAPI VSER_OpenDriver(unsigned int VSERPortNumber, VSER_DriverCallback_t VSERCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_OpenDriver_t)(unsigned int VSERPortNumber, VSER_DriverCallback_t VSERCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Closing the Virtual     */
   /* Serial Port Driver that was opened via a successful call to the   */
   /* VSER_OpenDriver() function.  The Input parameter to this function */
   /* MUST have been acquired by a successful call to VSER_OpenDriver().*/
   /* Once this function completes, the Virtual SER Port Driver that was*/
   /* closed cannot be accessed again (sending/receiving data) by this  */
   /* module until the Driver is Re-Opened by calling the               */
   /* VSER_OpenDriver() function.                                       */
BTPSAPI_DECLARATION void BTPSAPI VSER_CloseDriver(unsigned int VSERDriverID);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_VSER_CloseDriver_t)(unsigned int VSERDriverID);
#endif

   /* The following function is responsible for Reading Serial Data from*/
   /* the specified Virtual Serial Port Driver.  The input parameters to*/
   /* this function are the Virtual SER Port Driver ID, the Size of the */
   /* Data Buffer to be used for reading, and a pointer to the Data     */
   /* Buffer.  This function returns the number of data bytes that were */
   /* successfully read (zero if there were no Data Bytes ready to be   */
   /* read), or a negative return error code if unsuccessful.           */
BTPSAPI_DECLARATION int BTPSAPI VSER_Read(unsigned int VSERDriverID, unsigned int DataBufferSize, unsigned char *DataBuffer);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_Read_t)(unsigned int VSERDriverID, unsigned int DataBufferSize, unsigned char *DataBuffer);
#endif

   /* The following function is responsible for Sending Serial Data to  */
   /* the specified Virtual Serial Port Driver.  The VSERDriverID       */
   /* parameter that is passed to this function MUST have been          */
   /* established via a successful call to the VSER_OpenDriver()        */
   /* function.  The remaining parameters to this function are the      */
   /* Length of the Data to send and a pointer to the Data Buffer to    */
   /* Send.  This function returns the number of data bytes that were   */
   /* successfully sent, or a negative return error code if             */
   /* unsuccessful.                                                     */
BTPSAPI_DECLARATION int BTPSAPI VSER_Write(unsigned int VSERDriverID, unsigned int DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_Write_t)(unsigned int VSERDriverID, unsigned int DataLength, unsigned char *DataBuffer);
#endif

   /* The following function is provided to allow the programmer a means*/
   /* to notify the Virtual Serial Port Driver of a Break Condition.    */
   /* This function accepts as input the Virtual SER Port Driver ID of  */
   /* the Virtual SER Port that is to receive the Break Condition.  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI VSER_SendBreak(unsigned int VSERDriverID);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_SendBreak_t)(unsigned int VSERDriverID);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to send the existing state of ALL Modem/Port Control       */
   /* Siganls to the Virtual Serial Port Driver.  This function accepts */
   /* as input the Virtual SER Port Driver ID and the Current State of  */
   /* all the Modem Control Signals.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI VSER_PortStatus(unsigned int VSERDriverID, unsigned int VSERPortStatusMask);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_PortStatus_t)(unsigned int VSERDriverID, unsigned int VSERPortStatusMask);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to read the existing state of ALL Modem/Port Control       */
   /* Signals from the Virtual Serial Port Driver.  This function       */
   /* accepts as input the Virtual SER Port Driver ID and a pointer to a*/
   /* variable that is receive the Current State of all the Modem       */
   /* Control Signals.  This function returns zero if successful, or a  */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI VSER_ReadPortStatus(unsigned int VSERDriverID, unsigned int *VSERPortStatusMask);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_ReadPortStatus_t)(unsigned int VSERDriverID, unsigned *int VSERPortStatusMask);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to send the existing state of the Line Status to the       */
   /* Virtual Serial Port.  This function accepts as input the Virtual  */
   /* SER Port Driver ID and the Current Line Status State.  This       */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI VSER_LineStatus(unsigned int VSERDriverID, unsigned int VSERLineStatusMask);

#ifdef INCLUDE_VSER_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VSER_LineStatus_t)(unsigned int VSERDriverID, unsigned int VSERLineStatusMask);
#endif

#endif

/*****< vnetapi.h >************************************************************/
/*      Copyright 2003 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  VNETAPI - Stonestreet One Virtual Network Device Driver Interface API     */
/*            Type Definitions, Prototypes, and Constants.                    */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/24/03  D. Lange       Initial creation.                               */
/*   07/30/08  J. Toole       Converted for GEM usage.                        */
/******************************************************************************/
#ifndef __VNETAPIH__
#define __VNETAPIH__

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */
#include "BTTypes.h"            /* Bluetooth Type Definitions/Constants.      */

   /* Error Return Codes.                                               */
#define VNET_DRIVER_ERROR_INVALID_INDEX_NUMBER (-1)     /* Error that denotes */
                                                        /* that the specified */
                                                        /* Virtual Network    */
                                                        /* Index Number is    */
                                                        /* invalid.           */

#define VNET_DRIVER_ERROR_UNSUPPORTED_INDEX    (-2)     /* Error that denotes */
                                                        /* that the specified */
                                                        /* Virtual Network    */
                                                        /* Index Number cannot*/
                                                        /* be opened because  */
                                                        /* it cannot be       */
                                                        /* located in the     */
                                                        /* System.            */

#define VNET_DRIVER_ERROR_INVALID_CALLBACK_INFORMATION (-3)  /* Error that    */
                                                        /* denotes that the   */
                                                        /* specified Callback */
                                                        /* function pointer   */
                                                        /* is invalid.        */

#define VNET_DRIVER_ERROR_INITIALIZING_DRIVER  (-4)     /* Error that denotes */
                                                        /* that an error      */
                                                        /* occurred during    */
                                                        /* Driver             */
                                                        /* Configuration.     */

#define VNET_DRIVER_ERROR_INVALID_DRIVER_ID    (-5)     /* Error that denotes */
                                                        /* Driver ID supplied */
                                                        /* as function        */
                                                        /* argument is NOT    */
                                                        /* registered with    */
                                                        /* the Library.       */

#define VNET_DRIVER_ERROR_INVALID_PARAMETER    (-6)     /* Error that denotes */
                                                        /* a parameter that   */
                                                        /* was supplied as a  */
                                                        /* function argument  */
                                                        /* is NOT valid.      */

#define VNET_DRIVER_ERROR_WRITING_TO_DEVICE    (-7)     /* Error that denotes */
                                                        /* that there was an  */
                                                        /* error while trying */
                                                        /* to write to the    */
                                                        /* Virtual Network    */
                                                        /* Device.            */

#define VNET_DRIVER_ERROR_READING_FROM_DEVICE  (-8)     /* Error that denotes */
                                                        /* that there was an  */
                                                        /* error while trying */
                                                        /* to read from the   */
                                                        /* Virtual Network    */
                                                        /* Device.            */

#define VNET_DRIVER_ERROR_UNKNOWN              (-9)     /* Error that is not  */
                                                        /* covered by any     */
                                                        /* other Error Code.  */

   /* The following constant represents the largest possible payload    */
   /* size of an Ethernet packet (not counting the Ethernet Header).    */
#define VNET_MAXIMUM_ETHERNET_PAYLOAD_SIZE     1500

   /* The following type definition represents the container class for a*/
   /* Virtual Network Ethernet Address.                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagVNET_Ethernet_Address_t
{
   unsigned char Address0;
   unsigned char Address1;
   unsigned char Address2;
   unsigned char Address3;
   unsigned char Address4;
   unsigned char Address5;
} __PACKED_STRUCT_END__ VNET_Ethernet_Address_t;

#define VNET_ETHERNET_ADDRESS_SIZE                      (sizeof(VNET_Ethernet_Address_t))

   /* The following MACRO is a utility MACRO that exists to assign the  */
   /* individual Byte values into the specified VNET_Ethernet_Address   */
   /* variable.  The Bytes are NOT in Little Endian Format and are      */
   /* assigned to the VNET_Ethernet_Address Variable in BIG Endian      */
   /* Format.  The first parameter is the VNET_Ethernet_Address Variable*/
   /* (of type VNET_Ethernet_Address_t) to assign, and the next six     */
   /* parameters are the Individual Ethernet Address Byte values to     */
   /* assign to the variable.                                           */
#define VNET_ASSIGN_ETHERNET_ADDRESS(_dest, _a, _b, _c, _d, _e, _f) \
{                                                                   \
   (_dest).Address0 = (_a);                                         \
   (_dest).Address1 = (_b);                                         \
   (_dest).Address2 = (_c);                                         \
   (_dest).Address3 = (_d);                                         \
   (_dest).Address4 = (_e);                                         \
   (_dest).Address5 = (_f);                                         \
}

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* Comparison of two VNET_Ethernet_Address_t variables.  This MACRO  */
   /* only returns whether the two VNET_Ethernet_Address_t variables are*/
   /* equal (MACRO returns Boolean_t result) NOT less than/greater than.*/
   /* The two parameters to this MACRO are both of type                 */
   /* VNET_Ethernet_Address_t and represent the VNET_Ethernet_Address_t */
   /* variables to compare.                                             */
#define VNET_COMPARE_ADDRESSES(_x, _y)                                                                         \
(                                                                                                               \
   ((_x).Address0 == (_y).Address0) && ((_x).Address1 == (_y).Address1) && ((_x).Address2  == (_y).Address2) && \
   ((_x).Address3 == (_y).Address3) && ((_x).Address4 == (_y).Address4) && ((_x).Address5  == (_y).Address5)    \
)

   /* The following type definition represents the container class for  */
   /* all Ethernet Header Information (associated with each Ethernet    */
   /* Packet).                                                          */
typedef struct _tagVNET_Ethernet_Header_t
{
   VNET_Ethernet_Address_t DestinationAddress;
   VNET_Ethernet_Address_t SourceAddress;
   unsigned short          EthernetTypeField;
} VNET_Ethernet_Header_t;

#define VNET_ETHERNET_HEADER_SIZE                       (sizeof(VNET_Ethernet_Header_t))

   /* Virtual Network Driver API Event Types.                           */
typedef enum
{
   etVNET_Data_Indication
} VNET_Event_Type_t;

   /* The following structure represents the event data that required to*/
   /* signal the arrival of Data.                                       */
typedef struct _tagVNET_Data_Indication_Data_t
{
   unsigned int            VNETDriverID;
   VNET_Ethernet_Header_t  EthernetHeader;
   unsigned int            PayloadLength;
   unsigned char          *PayloadBuffer;
} VNET_Data_Indication_Data_t;

#define VNET_DATA_INDICATION_DATA_SIZE                  (sizeof(VNET_Data_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all VNET Event Data Data.                                 */
typedef struct _tagVNET_Event_Data_t
{
   VNET_Event_Type_t Event_Data_Type;
   union
   {
      VNET_Data_Indication_Data_t VNET_Data_Indication_Data;
   } Event_Data;
} VNET_Event_Data_t;

   /* The following declared type represents the Prototype Function for */
   /* a VNET Driver Receive Event Callback.  This function will be      */
   /* called whenever an event occurs on the specified virtual network  */
   /* device that was opened with the specified Driver ID.  This        */
   /* function passes to the caller the VNET Driver ID, the Event       */
   /* Information that occurred and the VNET Driver Callback Parameter  */
   /* that was specified when this Callback was installed.  The caller  */
   /* is free to use the contents of the VNET Event ONLY in the context */
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e. this function DOES NOT have be reentrant).  It     */
   /* needs to be noted however, that if the same Callback function is  */
   /* installed for multiple Virtual Network Ports, then the function IS*/
   /* NOT guaranteed to NOT be called simultaneously.  The function is  */
   /* only guaranteed to be called serially for a single context in     */
   /* which the Callback has been installed.  This means that if the    */
   /* same Callback function is installed for two (or more) Virtual     */
   /* Network Ports, then the function COULD be called simultaneously   */
   /* for both Events.  It should also be noted that this function is is*/
   /* called in the Thread Context of a Thread that the User does NOT   */
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* Event will not be received while this function call is            */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving VNET Events.        */
   /*            A Deadlock WILL occur because NO Other Receive VNET    */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *VNET_Driver_Callback_t)(unsigned int VNETDriverID, VNET_Event_Data_t *VNETEventData, unsigned long CallbackParameter);

   /* The following function is responsible for changing the current    */
   /* Ethernet Address that is being used by the specified Virtual      */
   /* Network Driver.  The first parameter to this function specifies   */
   /* the Virtual Network Driver Index of the Actual Virtual Network    */
   /* Driver in the system.  The second parameter specifies the actual  */
   /* Ethernet Address to use for the specified Driver (based on the    */
   /* first parameter).  This function returns the following status     */
   /* information:                                                      */
   /*    - Zero (No change - already configured with specified address).*/
   /*    - Positive (Changed - previously configured with a different   */
   /*      address).                                                    */
   /*    - Negative return error code (error).                          */
BTPSAPI_DECLARATION int BTPSAPI VNET_Configure_Ethernet_Address(unsigned int VNETIndex, VNET_Ethernet_Address_t EthernetAddress);

#ifdef INCLUDE_VNET_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VNET_Configure_Ethernet_Address_t)(unsigned int VNETIndex, VNET_Ethernet_Address_t EthernetAddress);
#endif

   /* The following function is responsible for Opening a virtual       */
   /* Network Driver with the specified Index.  The first parameter to  */
   /* this function specifies the Virtual Network Driver Index of the   */
   /* Actual Virtual Network Driver in the system.  The second and third*/
   /* parameters specify the VNET Driver Callback function that is to be*/
   /* called when a VNET Event Occurs.  All parameters to this function */
   /* *MUST* be specified.  If this function is successful, the caller  */
   /* will receive a non-zero, non-negative return value which serves as*/
   /* the VNETDriverID parameter for all other functions in the VNET    */
   /* Driver.  Since all VNET functions require a valid VNET Driver ID, */
   /* this function must be called successfully before any other        */
   /* function can be called.  If this function fails then the return   */
   /* value is a negative error code (see error codes above).           */
   /* * NOTE * If this function call is successful, no Ethernet Packets */
   /*          can be sent or received, until the Ethernet Connected    */
   /*          State is set to connected.                               */
   /*          The VNET_Set_Ethernet_Connected_State() function is      */
   /*          used for this purpose.  In other words, the default      */
   /*          Ethernet Connected State is disconnected (i.e. no        */
   /*          Ethernet Packets can be sent/received).                  */
BTPSAPI_DECLARATION int BTPSAPI VNET_Open_Driver(unsigned int VNETIndex, VNET_Driver_Callback_t VNETCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_VNET_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VNET_Open_Driver_t)(unsigned int VNETIndex, VNET_Driver_Callback_t VNETCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Closing the Virtual     */
   /* Network Driver that was opened via a successful call to the       */
   /* VNET_Open_Driver() function.  The Input parameter to this function*/
   /* MUST have been acquired by a successful call to                   */
   /* VNET_Open_Driver().  Once this function completes, the Virtual    */
   /* Network Driver that was closed cannot be accessed again           */
   /* (sending/receiving data) by this module until the Driver is       */
   /* Re-Opened by calling the VNET_Open_Driver() function.             */
BTPSAPI_DECLARATION void BTPSAPI VNET_Close_Driver(unsigned int VNETDriverID);

#ifdef INCLUDE_VNET_DRIVER_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_VNET_Close_Driver_t)(unsigned int VNETDriverID);
#endif

   /* The following function is responsible for querying the current    */
   /* Ethernet Address that is being used by the specified Virtual      */
   /* Network Driver.  The VNETDriverID parameter that is passed to this*/
   /* function MUST have been established via a successful call to the  */
   /* VNET_Open_Driver() function.  The final parameter to this function*/
   /* is a pointer to a buffer that will hold the current Ethernet      */
   /* Address of the Virtual Network Driver (if this function is        */
   /* successful).  This function returns zero if successful or a       */
   /* negative return error code if there was an error.  If this        */
   /* function is successful then the buffer pointed to by the          */
   /* EthernetAddress parameter will contain the currently opened       */
   /* Virtual Network Driver.  If this function is unsuccessful then the*/
   /* contents of the EthernetAddress parameter buffer will be          */
   /* undefined.                                                        */
BTPSAPI_DECLARATION int BTPSAPI VNET_Query_Ethernet_Address(unsigned int VNETDriverID, VNET_Ethernet_Address_t *EthernetAddress);

#ifdef INCLUDE_VNET_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VNET_Query_Ethernet_Address_t)(unsigned int VNETDriverID, VNET_Ethernet_Address_t *EthernetAddress);
#endif

   /* The following function is responsible for specifying the current  */
   /* Ethernet Connected State that is to be used by the specified      */
   /* Virtual Network Driver.  The VNETDriverID parameter that is passed*/
   /* to this function MUST have been established via a successful call */
   /* to the VNET_Open_Driver() function.  The final parameter to this  */
   /* function is a BOOLEAN parameter that specifies whether or not an  */
   /* Ethernet Cable is connected (TRUE) or disconnected (FALSE).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * This function has to be called with the second parameter */
   /*          set to TRUE (connected) before ANY Ethernet Packets can  */
   /*          be sent or received.                                     */
   /* * NOTE * When the Ethernet Connected State is disconnected, no    */
   /*          Ethenet Packets can be sent or received.                 */
BTPSAPI_DECLARATION int BTPSAPI VNET_Set_Ethernet_Connected_State(unsigned int VNETDriverID, Boolean_t Connected);

#ifdef INCLUDE_VNET_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VNET_Set_Ethernet_Connected_State_t)(unsigned int VNETDriverID, Boolean_t Connected);
#endif

   /* The following function is responsible for sending an Ethernet     */
   /* Packet to the specified Virtual Network Driver.  The VNETDriverID */
   /* parameter that is passed to this function MUST have been          */
   /* established via a successful call to the VNET_Open_Driver()       */
   /* function.  The remaining parameters to this function are the      */
   /* Ethernet Packet Header to use for the packet, followed by the     */
   /* Length of the Data to send and a pointer to the Data Buffer to    */
   /* Send (Payload only).  This function returns zero if successful,   */
   /* or a negative return error code if unsuccessful.                  */
   /* * NOTE * If the Ethernet Connected State is disconnected, then    */
   /*          no Ethernet packets will be passed through the Virtual   */
   /*          Network Driver.  The Ethernet connected state is set via */
   /*          a call to the VNET_Set_Ethernet_Connected_State()        */
   /*          function.                                                */
   /* * NOTE * The default state of the Ethernet Connected State is     */
   /*          disconnected.                                            */
BTPSAPI_DECLARATION int BTPSAPI VNET_Write_Ethernet_Packet(unsigned int VNETDriverID, VNET_Ethernet_Header_t *EthernetHeader, unsigned int PayloadLength, unsigned char *PayloadBuffer);

#ifdef INCLUDE_VNET_DRIVER_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_VNET_Write_Ethernet_Packet_t)(unsigned int VNETDriverID, VNET_Ethernet_Header_t *EthernetHeader, unsigned int PayloadLength, unsigned char *PayloadBuffer);
#endif

#endif


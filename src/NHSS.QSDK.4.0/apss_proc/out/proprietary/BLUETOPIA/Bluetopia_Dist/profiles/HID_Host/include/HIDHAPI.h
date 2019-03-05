/*****< hidhapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDHAPI - Stonestreet One Bluetooth Stack Human Interface Device (HID)    */
/*            Host Profile API Type Definitions, Constants, and Prototypes.   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname       Description of Modification                  */
/*   --------  -----------       ---------------------------------------------*/
/*   02/20/10  D. Lange          Initial creation.                            */
/******************************************************************************/
#ifndef __HIDHAPIH__
#define __HIDHAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

#include "SS1BTHID.h"           /* Bluetooth HID Profile API Prototypes.      */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTHID_HOST_ERROR_INVALID_PARAMETER                        (-2000)
#define BTHID_HOST_ERROR_NOT_INITIALIZED                          (-2001)
#define BTHID_HOST_ERROR_INVALID_BLUETOOTH_STACK_ID               (-2002)
#define BTHID_HOST_ERROR_INSUFFICIENT_RESOURCES                   (-2004)
#define BTHID_HOST_ERROR_INVALID_OPERATION                        (-2005)
#define BTHID_HOST_ERROR_ALREADY_CONNECTED                        (-2006)

   /* Usage bit masks for the keys (CONTROL, SHIFT, ALT, GUI) in the    */
   /* modifier byte.                                                    */
#define HID_HOST_MODIFIER_FLAG_LEFT_CTRL                             0x01
#define HID_HOST_MODIFIER_FLAG_LEFT_SHIFT                            0x02
#define HID_HOST_MODIFIER_FLAG_LEFT_ALT                              0x04
#define HID_HOST_MODIFIER_FLAG_LEFT_GUI                              0x08
#define HID_HOST_MODIFIER_FLAG_RIGHT_CTRL                            0x10
#define HID_HOST_MODIFIER_FLAG_RIGHT_SHIFT                           0x20
#define HID_HOST_MODIFIER_FLAG_RIGHT_ALT                             0x40
#define HID_HOST_MODIFIER_FLAG_RIGHT_GUI                             0x80

   /* The following defines are from Universal Serial Bus HID Usage     */
   /* Tables section 10 and indicate the minimum usage codes for a 104  */
   /* boot keyboard.  They are used with the keybord event to indicate  */
   /* the key.                                                          */
#define HID_HOST_RESERVED                                            0x00
#define HID_HOST_KEYBOARD_ERROR_ROLL_OVER                            0x01
#define HID_HOST_KEYBOARD_POST_FAIL                                  0x02
#define HID_HOST_KEYBOARD_ERROR_UNDEFINED                            0x03
#define HID_HOST_KEYBOARD_A                                          0x04
#define HID_HOST_KEYBOARD_B                                          0x05
#define HID_HOST_KEYBOARD_C                                          0x06
#define HID_HOST_KEYBOARD_D                                          0x07
#define HID_HOST_KEYBOARD_E                                          0x08
#define HID_HOST_KEYBOARD_F                                          0x09
#define HID_HOST_KEYBOARD_G                                          0x0A
#define HID_HOST_KEYBOARD_H                                          0x0B
#define HID_HOST_KEYBOARD_I                                          0x0C
#define HID_HOST_KEYBOARD_J                                          0x0D
#define HID_HOST_KEYBOARD_K                                          0x0E
#define HID_HOST_KEYBOARD_L                                          0x0F
#define HID_HOST_KEYBOARD_M                                          0x10
#define HID_HOST_KEYBOARD_N                                          0x11
#define HID_HOST_KEYBOARD_O                                          0x12
#define HID_HOST_KEYBOARD_P                                          0x13
#define HID_HOST_KEYBOARD_Q                                          0x14
#define HID_HOST_KEYBOARD_R                                          0x15
#define HID_HOST_KEYBOARD_S                                          0x16
#define HID_HOST_KEYBOARD_T                                          0x17
#define HID_HOST_KEYBOARD_U                                          0x18
#define HID_HOST_KEYBOARD_V                                          0x19
#define HID_HOST_KEYBOARD_W                                          0x1A
#define HID_HOST_KEYBOARD_X                                          0x1B
#define HID_HOST_KEYBOARD_Y                                          0x1C
#define HID_HOST_KEYBOARD_Z                                          0x1D
#define HID_HOST_KEYBOARD_1                                          0x1E
#define HID_HOST_KEYBOARD_2                                          0x1F
#define HID_HOST_KEYBOARD_3                                          0x20
#define HID_HOST_KEYBOARD_4                                          0x21
#define HID_HOST_KEYBOARD_5                                          0x22
#define HID_HOST_KEYBOARD_6                                          0x23
#define HID_HOST_KEYBOARD_7                                          0x24
#define HID_HOST_KEYBOARD_8                                          0x25
#define HID_HOST_KEYBOARD_9                                          0x26
#define HID_HOST_KEYBOARD_0                                          0x27
#define HID_HOST_KEYBOARD_RETURN                                     0x28
#define HID_HOST_KEYBOARD_ESCAPE                                     0x29
#define HID_HOST_KEYBOARD_DELETE                                     0x2A
#define HID_HOST_KEYBOARD_TAB                                        0x2B
#define HID_HOST_KEYBOARD_SPACE_BAR                                  0x2C
#define HID_HOST_KEYBOARD_MINUS                                      0x2D /* '-' */
#define HID_HOST_KEYBOARD_EQUAL                                      0x2E /* '=' */
#define HID_HOST_KEYBOARD_LEFT_BRACKET                               0x2F /* '[' */
#define HID_HOST_KEYBOARD_RIGHT_BRACKET                              0x30 /* ']' */
#define HID_HOST_KEYBOARD_BACK_SLASH                                 0x31 /* '\' */
#define HID_HOST_KEYBOARD_NON_US_POUND                               0x32 /* '#' */
#define HID_HOST_KEYBOARD_SEMICOLON                                  0x33 /* ';' */
#define HID_HOST_KEYBOARD_APOSTROPHE                                 0x34 /* ''' */
#define HID_HOST_KEYBOARD_GRAVE_ACCENT                               0x35 /* '`' */
#define HID_HOST_KEYBOARD_COMMA                                      0x36 /* ',' */
#define HID_HOST_KEYBOARD_DOT                                        0x37 /* '.' */
#define HID_HOST_KEYBOARD_SLASH                                      0x38 /* '/' */
#define HID_HOST_KEYBOARD_CAPS_LOCK                                  0x39
#define HID_HOST_KEYBOARD_F1                                         0x3A
#define HID_HOST_KEYBOARD_F2                                         0x3B
#define HID_HOST_KEYBOARD_F3                                         0x3C
#define HID_HOST_KEYBOARD_F4                                         0x3D
#define HID_HOST_KEYBOARD_F5                                         0x3E
#define HID_HOST_KEYBOARD_F6                                         0x3F
#define HID_HOST_KEYBOARD_F7                                         0x40
#define HID_HOST_KEYBOARD_F8                                         0x41
#define HID_HOST_KEYBOARD_F9                                         0x42
#define HID_HOST_KEYBOARD_F10                                        0x43
#define HID_HOST_KEYBOARD_F11                                        0x44
#define HID_HOST_KEYBOARD_F12                                        0x45
#define HID_HOST_KEYBOARD_PRINT_SCREEN                               0x46
#define HID_HOST_KEYBOARD_SCROLL_LOCK                                0x47
#define HID_HOST_KEYBOARD_PAUSE                                      0x48
#define HID_HOST_KEYBOARD_INSERT                                     0x49
#define HID_HOST_KEYBOARD_HOME                                       0x4A
#define HID_HOST_KEYBOARD_PAGE_UP                                    0x4B
#define HID_HOST_KEYBOARD_DELETE_FORWARD                             0x4C
#define HID_HOST_KEYBOARD_END                                        0x4D
#define HID_HOST_KEYBOARD_PAGE_DOWN                                  0x4E
#define HID_HOST_KEYBOARD_RIGHT_ARROW                                0x4F
#define HID_HOST_KEYBOARD_LEFT_ARROW                                 0x50
#define HID_HOST_KEYBOARD_DOWN_ARROW                                 0x51
#define HID_HOST_KEYBOARD_UP_ARROW                                   0x52
#define HID_HOST_KEYPAD_NUM_LOCK                                     0x53
#define HID_HOST_KEYPAD_SLASH                                        0x54 /* '/' */
#define HID_HOST_KEYPAD_ASTERISK                                     0x55 /* '*' */
#define HID_HOST_KEYPAD_MINUS                                        0x56 /* '-' */
#define HID_HOST_KEYPAD_PLUS                                         0x57 /* '+' */
#define HID_HOST_KEYPAD_ENTER                                        0x58
#define HID_HOST_KEYPAD_1                                            0x59
#define HID_HOST_KEYPAD_2                                            0x5A
#define HID_HOST_KEYPAD_3                                            0x5B
#define HID_HOST_KEYPAD_4                                            0x5C
#define HID_HOST_KEYPAD_5                                            0x5D
#define HID_HOST_KEYPAD_6                                            0x5E
#define HID_HOST_KEYPAD_7                                            0x5F
#define HID_HOST_KEYPAD_8                                            0x60
#define HID_HOST_KEYPAD_9                                            0x61
#define HID_HOST_KEYPAD_0                                            0x62
#define HID_HOST_KEYPAD_DOT                                          0x63 /* '.' */
#define HID_HOST_KEYBOARD_NON_US_SLASH                               0x64
#define HID_HOST_KEYBOARD_APPLICATION                                0x65

#define HID_HOST_KEYBOARD_LEFT_CONTROL                               0xE0
#define HID_HOST_KEYBOARD_LEFT_SHIFT                                 0xE1
#define HID_HOST_KEYBOARD_LEFT_ALT                                   0xE2
#define HID_HOST_KEYBOARD_LEFT_GUI                                   0xE3
#define HID_HOST_KEYBOARD_RIGHT_CONTROL                              0xE4
#define HID_HOST_KEYBOARD_RIGHT_SHIFT                                0xE5
#define HID_HOST_KEYBOARD_RIGHT_ALT                                  0xE6
#define HID_HOST_KEYBOARD_RIGHT_GUI                                  0xE7

   /* The following defines are used with the mouse event to indicate   */
   /* a change in the mouse buttons.                                    */
#define HID_HOST_LEFT_BUTTON_UP                                    0x0001
#define HID_HOST_LEFT_BUTTON_DOWN                                  0x0002
#define HID_HOST_RIGHT_BUTTON_UP                                   0x0004
#define HID_HOST_RIGHT_BUTTON_DOWN                                 0x0008
#define HID_HOST_MIDDLE_BUTTON_UP                                  0x0010
#define HID_HOST_MIDDLE_BUTTON_DOWN                                0x0020

   /* The following constants represent the device open status values   */
   /* that are possible in the open confirmation event.                 */
#define HID_HOST_OPEN_STATUS_SUCCESS                                 0x00
#define HID_HOST_OPEN_STATUS_CONNECTION_TIMEOUT                      0x01
#define HID_HOST_OPEN_STATUS_CONNECTION_REFUSED                      0x02
#define HID_HOST_OPEN_STATUS_UNKNOWN_ERROR                           0x03

   /* The following constants are used with the                         */
   /* HID_Host_Open_Request_Response() and the                          */
   /* HID_Host_Connect_Remote_Device() function to specify how the      */
   /* incoming or outgoing connection should be processed.              */
#define HID_HOST_CONNECTION_FLAGS_REPORT_MODE                   0x00000001
#define HID_HOST_CONNECTION_FLAGS_PARSE_BOOT                    0x00000002

   /* HID Host Event API Types.                                         */
typedef enum
{
   etHID_Host_Open_Request_Indication,
   etHID_Host_Open_Indication,
   etHID_Host_Open_Confirmation,
   etHID_Host_Close_Indication,
   etHID_Host_Boot_Keyboard_Data_Indication,
   etHID_Host_Boot_Keyboard_Repeat_Indication,
   etHID_Host_Boot_Mouse_Data_Indication,
   etHID_Host_Data_Indication,
   etHID_Host_Get_Report_Confirmation,
   etHID_Host_Set_Report_Confirmation,
   etHID_Host_Get_Protocol_Confirmation,
   etHID_Host_Set_Protocol_Confirmation,
   etHID_Host_Get_Idle_Confirmation,
   etHID_Host_Set_Idle_Confirmation
} HID_Host_Event_Type_t;

   /* The following event is dispatched when a remote service is        */
   /* requesting a connection to the local HID Host service.  The       */
   /* BD_ADDR specifies the Bluetooth address of the remote device that */
   /* is connecting.                                                    */
   /* * NOTE * This event is only dispatched to servers that are in     */
   /*          manual accept mode.                                      */
   /* * NOTE * This event must be responded to with the                 */
   /*          HID_Host_Open_Request_Response() function in order to    */
   /*          accept or reject the outstanding open request.           */
typedef struct _tagHID_Host_Open_Request_Indication_Data_t
{
   BD_ADDR_t BD_ADDR;
} HID_Host_Open_Request_Indication_Data_t;

#define HID_HOST_OPEN_REQUEST_INDICATION_DATA_SIZE       (sizeof(HID_Host_Open_Request_Indication_Data_t))

   /* The following event is dispatched when a client connects to a     */
   /* registered server.  The BD_ADDR member specifies the address of   */
   /* the client which is connecting to the locally registered server.  */
typedef struct _tagHID_Host_Open_Indication_Data_t
{
   BD_ADDR_t BD_ADDR;
} HID_Host_Open_Indication_Data_t;

#define HID_HOST_OPEN_INDICATION_DATA_SIZE               (sizeof(HID_Host_Open_Indication_Data_t))

   /* The following event is dispatched to the local client to indicate */
   /* success or failure of a previously submitted connection request.  */
   /* The OpenStatus member specifies the status of the connection      */
   /* attempt.  Possible values for the OpenStatus can be found above.  */
typedef struct _tagHID_Host_Open_Confirmation_Data_t
{
   BD_ADDR_t    BD_ADDR;
   unsigned int OpenStatus;
} HID_Host_Open_Confirmation_Data_t;

#define HID_HOST_OPEN_CONFIRMATION_DATA_SIZE             (sizeof(HID_Host_Open_Confirmation_Data_t))

   /* The following event is dispatched when a remote device disconnects*/
   /* from the local host.  The BD_ADDR member specifies the Bluetooth  */
   /* device address of the device that is disconnecting.  This event is*/
   /* NOT dispatched in response to the local host requesting the       */
   /* disconnection.  This event is dispatched only when the remote     */
   /* device terminates the connection (and/or Bluetooth Link).         */
typedef struct _tagHID_Host_Close_Indication_Data_t
{
   BD_ADDR_t BD_ADDR;
} HID_Host_Close_Indication_Data_t;

#define HID_HOST_CLOSE_INDICATION_DATA_SIZE              (sizeof(HID_Host_Close_Indication_Data_t))

   /* The following event is dispatched when the remote device reports  */
   /* mouse movement.  The BD_ADDR member specifies the Bluetooth device*/
   /* address of the device sending the mouse movement report.  The CX, */
   /* CY member specifies the relative mouse movement.  The ButtonState */
   /* member indicates the mouse button activity.                       */
typedef struct _tagHID_Host_Boot_Mouse_Data_t
{
   BD_ADDR_t BD_ADDR;
   SByte_t   CX;
   SByte_t   CY;
   Byte_t    ButtonState;
   SByte_t   CZ;
} HID_Host_Boot_Mouse_Data_t;

#define HID_HOST_BOOT_MOUSE_DATA_SIZE                    (sizeof(HID_Host_Boot_Mouse_Data_t))

   /* The following event is dispatched when the local entity repeats   */
   /* keys and indicates the key thatis being reapeated.                */
typedef struct _tagHID_Host_Boot_Keyboard_Repeat_Data_t
{
   BD_ADDR_t BD_ADDR;
   Byte_t    KeyModifiers;
   Byte_t    Key;
} HID_Host_Boot_Keyboard_Repeat_Data_t;

#define HID_HOST_BOOT_KEYBOARD_REPEAT_DATA_SIZE          (sizeof(HID_Host_Boot_Keyboard_Repeat_Data_t))

   /* The following event is dispatched when the local host receives a  */
   /* BOOT keyboard HID event.  The BD_ADDR member specifies the        */
   /* Bluetooth device address of the device.  The KeyDown member is    */
   /* TRUE if the key is currently pressed or FALSE if the key is up    */
   /* (from a previous key down event).  The KeyModifiers member is an  */
   /* informative bit mask indicating what modifier keys are being      */
   /* pressed.  Note that a key event will also occur for the modifier  */
   /* keys.  The Key member specifies the current key of the keyboard.  */
typedef struct _tagHID_Host_Boot_Keyboard_Data_t
{
   BD_ADDR_t BD_ADDR;
   Boolean_t KeyDown;
   Byte_t    KeyModifiers;
   Byte_t    Key;
} HID_Host_Boot_Keyboard_Data_t;

#define HID_HOST_BOOT_KEYBOARD_DATA_SIZE                 (sizeof(HID_Host_Boot_Keyboard_Data_t))

   /* The following event is dispatched when the local host receives a  */
   /* HID Host data transaction.  The ReportLength member specifies the */
   /* length of the ReportDataPayload member.  The ReportDataPayload is */
   /* a pointer to the actual report data.                              */
typedef struct _tagHID_Host_Data_Indication_Data_t
{
   BD_ADDR_t  BD_ADDR;
   Word_t     ReportLength;
   Byte_t    *ReportDataPayload;
} HID_Host_Data_Indication_Data_t;

#define HID_HOST_DATA_INDICATION_DATA_SIZE               (sizeof(HID_Host_Data_Indication_Data_t))

   /* The following event is dispatched when the local HID Host         */
   /* receives a response to an outstanding GET_REPORT Transaction.     */
   /* The Status member specifies the Result Type of the Response.  If  */
   /* the Status is rtData the members which follow it will be valid.   */
   /* The rtSuccessful value is an invalid Result Type and the other    */
   /* possible Status types rtErr and rtNotReady indicate an error      */
   /* occurred.  The Report Type member specifies the type of the Report*/
   /* the HID Device is returning.  The ReportLength member specifies   */
   /* the Length of the Report Data Payload Member.  The Report Data    */
   /* Payload is the actual report data.  The ReportType, ReportLength, */
   /* and ReportDataPayload members will only be valid if the Status is */
   /* of type rtData.                                                   */
typedef struct _tagHID_Host_Get_Report_Confirmation_Data_t
{
   BD_ADDR_t               BD_ADDR;
   HID_Result_Type_t       Status;
   HID_Report_Type_Type_t  ReportType;
   Word_t                  ReportLength;
   Byte_t                 *ReportDataPayload;
} HID_Host_Get_Report_Confirmation_Data_t;

#define HID_HOST_GET_REPORT_CONFIRMATION_DATA_SIZE       (sizeof(HID_Host_Get_Report_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding SET_REPORT Transaction.  The Status  */
   /* member specifies the Result Type of the Response.  If the Status  */
   /* is rtSuccessful the SET_REPORT Transaction was successful.  The   */
   /* Status Result rtData is an invalid Result Type and the other      */
   /* possible Status types rtErr and rtNotReady indicate that an error */
   /* occurred.                                                         */
typedef struct _tagHID_Host_Set_Report_Confirmation_Data_t
{
   BD_ADDR_t         BD_ADDR;
   HID_Result_Type_t Status;
} HID_Host_Set_Report_Confirmation_Data_t;

#define HID_HOST_SET_REPORT_CONFIRMATION_DATA_SIZE       (sizeof(HID_Host_Set_Report_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding GET_PROTOCOL Transaction.  The Status*/
   /* member specifies the Result Type of the Response.  If the Status  */
   /* is rtData, the members which follow it will be valid.  The Result */
   /* rtSuccessful is an invalid Result Type and the other possible     */
   /* Status types rtErr and rtNotReady indicate an error occurred.  The*/
   /* Protocol member specifies the current protocol.  The Protocol     */
   /* member will only be valid if the status is of type rtData.        */
typedef struct _tagHID_Host_Get_Protocol_Confirmation_Data_t
{
   BD_ADDR_t           BD_ADDR;
   HID_Result_Type_t   Status;
   HID_Protocol_Type_t Protocol;
} HID_Host_Get_Protocol_Confirmation_Data_t;

#define HID_HOST_GET_PROTOCOL_CONFIRMATION_DATA_SIZE     (sizeof(HID_Host_Get_Protocol_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding SET_PROTOCOL Transaction.  If        */
   /* the Status is rtSuccessful, the SET_PROTOCOL Transaction was      */
   /* successful.  The Status rtData is an invalid Result Type and the  */
   /* other possible Status types rtErr and rtNotReady indicate that an */
   /* error occurred.                                                   */
typedef struct _tagHID_Host_Set_Protocol_Confirmation_Data_t
{
   BD_ADDR_t         BD_ADDR;
   HID_Result_Type_t Status;
} HID_Host_Set_Protocol_Confirmation_Data_t;

#define HID_HOST_SET_PROTOCOL_CONFIRMATION_DATA_SIZE     (sizeof(HID_Host_Set_Protocol_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding GET_IDLE Transaction.  The Status    */
   /* member specifies the Result Type of the Response.  If the Status  */
   /* is rtData the members which follow it will be valid.  The Status  */
   /* value rtSuccessful is an invalid Result Type and the other        */
   /* possible Status types rtErr and rtNotReady indicate an error      */
   /* occurred.  The IdleRate member specifies the current idle rate.   */
   /* The IdleRate member will only be valid if the status is of type   */
   /* rtData.                                                           */
typedef struct _tagHID_Host_Get_Idle_Confirmation_Data_t
{
   BD_ADDR_t         BD_ADDR;
   HID_Result_Type_t Status;
   Byte_t            IdleRate;
} HID_Host_Get_Idle_Confirmation_Data_t;

#define HID_HOST_GET_IDLE_CONFIRMATION_DATA_SIZE         (sizeof(HID_Host_Get_Idle_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding SET_IDLE Transaction.  If the Status */
   /* is rtSuccessful then the SET_IDLE Transaction was successful.     */
   /* The Status value rtData is an invalid Result Type and the other   */
   /* possible Status types rtErr and rtNotReady indicate that an error */
   /* occurred.                                                         */
typedef struct _tagHID_Host_Set_Idle_Confirmation_Data_t
{
   BD_ADDR_t         BD_ADDR;
   HID_Result_Type_t Status;
} HID_Host_Set_Idle_Confirmation_Data_t;

#define HID_HOST_SET_IDLE_CONFIRMATION_DATA_SIZE         (sizeof(HID_Host_Set_Idle_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all HID Host event data data.                             */
typedef struct _tagHID_Host_Event_Data_t
{
   HID_Host_Event_Type_t Event_Data_Type;
   Word_t                Event_Data_Size;
   union
   {
      HID_Host_Open_Request_Indication_Data_t   *HID_Host_Open_Request_Indication_Data;
      HID_Host_Open_Indication_Data_t           *HID_Host_Open_Indication_Data;
      HID_Host_Open_Confirmation_Data_t         *HID_Host_Open_Confirmation_Data;
      HID_Host_Close_Indication_Data_t          *HID_Host_Close_Indication_Data;
      HID_Host_Boot_Keyboard_Data_t             *HID_Host_Boot_Keyboard_Data;
      HID_Host_Data_Indication_Data_t           *HID_Host_Data_Indication_Data;
      HID_Host_Boot_Keyboard_Repeat_Data_t      *HID_Host_Boot_Keyboard_Repeat_Data;
      HID_Host_Boot_Mouse_Data_t                *HID_Host_Boot_Mouse_Data;
      HID_Host_Get_Report_Confirmation_Data_t   *HID_Host_Get_Report_Confirmation_Data;
      HID_Host_Set_Report_Confirmation_Data_t   *HID_Host_Set_Report_Confirmation_Data;
      HID_Host_Get_Protocol_Confirmation_Data_t *HID_Host_Get_Protocol_Confirmation_Data;
      HID_Host_Set_Protocol_Confirmation_Data_t *HID_Host_Set_Protocol_Confirmation_Data;
      HID_Host_Get_Idle_Confirmation_Data_t     *HID_Host_Get_Idle_Confirmation_Data;
      HID_Host_Set_Idle_Confirmation_Data_t     *HID_Host_Set_Idle_Confirmation_Data;
   } Event_Data;
} HID_Host_Event_Data_t;

#define HID_HOST_EVENT_DATA_SIZE                         (sizeof(HID_Host_Event_Data_t))

   /* The following declared type represents the prototype function for */
   /* a Human Interface Device (HID) Host profile event callback.  This */
   /* function will be called whenever a HID Host event occurs that is  */
   /* associated with the specified Bluetooth stack ID.  This function  */
   /* passes to the caller the Bluetooth stack ID, the HID Host event   */
   /* data that occurred and the HID Host event callback parameter that */
   /* was specified when this callback was installed.  The caller is    */
   /* free to use the contents of the HID Host event data ONLY in the   */
   /* context of this callback.  If the caller requires the data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another data buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the thread context of a thread that the*/
   /* user does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another HID Host profile event will not be processed while this   */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by receiving HID Host Events.    */
   /*            A deadlock WILL occur because NO HID Host event        */
   /*            callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *HID_Host_Event_Callback_t)(unsigned int BluetoothStackID, HID_Host_Event_Data_t *HID_Host_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for registering a HID Host  */
   /* server.  Note that only one server can be registered for each     */
   /* Bluetooth stack.  This function accepts the Bluetooth stack ID of */
   /* the Bluetooth stack which this server is to be associated with.   */
   /* The second parameter to this function is the HID configuration    */
   /* specification to be used in the negotiation of the L2CAP channels */
   /* associated with this Host server.  The final two parameters       */
   /* specify the HID Host event callback function and callback         */
   /* parameter, respectively, of the HID Host event callback that is to*/
   /* process any further events associated with this HID Host server.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if an error occurred (see BTERRORS.H).                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Initialize(unsigned int BluetoothStackID, HID_Configuration_t *HIDConfiguration, HID_Host_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Initialize_t)(unsigned int BluetoothStackID, HID_Configuration_t *HIDConfiguration, HID_Host_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for un-registering a HID    */
   /* Host server (which was registered by a successful call to the     */
   /* HID_Host_Initialize() function.  This function accepts as input   */
   /* the Bluetooth stack ID of the Bluetooth protocol stack that the   */
   /* server was registered for.  This function returns zero if         */
   /* successful, or a negative return error code if an error occurred  */
   /* (see BTERRORS.H).                                                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Un_Initialize(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Un_Initialize_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local HID Host server.  The    */
   /* first parameter to this function is the Bluetooth stack ID of the */
   /* Bluetooth stack associated with the HID Host server that is       */
   /* responding to the request.  The second parameter specifies the    */
   /* Bluetooth device address of the device that is connecting.  The   */
   /* third parameter to this function specifies whether to accept the  */
   /* pending connection request (or to reject the request).  The final */
   /* parameter specifies optional connection flags to indicate the     */
   /* report handling behavior.  This function returns zero if          */
   /* successful, or a negative return error code if an error occurred. */
   /* * NOTE * The connection to the server is not established until a  */
   /*          etHID_Host_Open_Indication event has occurred.           */
   /* * NOTE * This event will only be dispatched if the server mode    */
   /*          was explicitly set to hsmManualAccept via the            */
   /*          HID_Host_Set_Server_Connection_Mode() function.          */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Open_Request_Response(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Boolean_t AcceptConnection, DWord_t ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Open_Request_Response_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Boolean_t AcceptConnection, DWord_t ConnectionFlags);
#endif

   /* The following function is responsible for opening a connection to */
   /* a Remote HID device on the specified Bluetooth device.  This      */
   /* function accepts as its first parameter the Bluetooth stack ID of */
   /* the Bluetooth stack which is to open the HID connection.  The     */
   /* second parameter specifies the Bluetooth device address (NON NULL)*/
   /* of the remote Bluetooth device to connect with.  The final        */
   /* parameter specifies optional connection flags that are used to    */
   /* indicate report handling behavior.  This function returns zero if */
   /* successful or a negative return error code if this function is    */
   /* unsuccessful.  Once a connection is opened to a remote device it  */
   /* can only be closed via a call to the HID_Host_Close_Connection()  */
   /* function.                                                         */
   /* * NOTE * The HID_Host_Initialize() function *MUST* be called prior*/
   /*          to this function.  Also note that a remote connection    */
   /*          cannot be made if there already exists a HID connection  */
   /*          to the local HID Host server.                            */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Connect_Remote_Device(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, DWord_t ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Connect_Remote_Device_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, DWord_t ConnectionFlags);
#endif

   /* The following function is responsible for closing a HID connection*/
   /* established through a connection made to the local HID Host server*/
   /* or a connection that was made by calling the                      */
   /* HID_Host_Open_Remote_Device() function.  This function accepts as */
   /* input the Bluetooth stack ID of the Bluetooth protocol stack      */
   /* followed by the Bluetooth device address of the device to         */
   /* disconnect.  The final parameter indicates if the host should send*/
   /* the HID virtual cable disconnection (so that the device does not  */
   /* automatically attempt to re-connect back).  This function returns */
   /* zero if successful, or a negative return error code if an error   */
   /* occurred.                                                         */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Close_Connection(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Boolean_t SendVirtualCableDisconnect);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Close_Connection_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Boolean_t SendVirtualCableDisconnect);
#endif

   /* The following function is responsible for sending HID reports over*/
   /* the HID interrupt channel.  This function accepts the Bluetooth   */
   /* stack ID of the Bluetooth stack which is to send the report data  */
   /* and the Bluetooth device address to send the report to.  The final*/
   /* two parameters are the length of the report payload to send and a */
   /* pointer to the report payload that will be sent (respectively).   */
   /* This function returns a zero if successful, or a negative return  */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Data_Write(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Data_Write_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);
#endif

   /* The following function is responsible for sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Stack which is to send    */
   /* the request and the Bluetooth device address to send the request  */
   /* to.  The third parameter is the descriptor that indicates how the */
   /* device is to determine the size of the buffer that the host has   */
   /* allocated.  The fourth parameter is the type of report requested. */
   /* The fifth parameter is the Report ID determined by the Device's   */
   /* SDP record.  Passing HID_INVALID_REPORT_ID as the value for this  */
   /* parameter will indicate that this parameter is not used and will  */
   /* exclude the appropriate byte from the transaction payload.  The   */
   /* fifth parameters use is based on the parameter passed as Size.  If*/
   /* the host indicates it has allocated a buffer of a size smaller    */
   /* than the report it is requesting, this parameter will be used as  */
   /* the size of the report returned.  Otherwise, the appropriate bytes*/
   /* will not be included in the transaction payload.  This function   */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Host Get Report Confirmation event    */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Get_Report_Request(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Get_Report_Size_Type_t Size, HID_Report_Type_Type_t ReportType, Byte_t ReportID, Word_t BufferSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Get_Report_Request_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Get_Report_Size_Type_t Size, HID_Report_Type_Type_t ReportType, Byte_t ReportID, Word_t BufferSize);
#endif

   /* The following function is responsible for sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* transaction and the Bluetooth device address to send the request  */
   /* to.  The third parameter is the type of report being sent.  Note  */
   /* that rtOther is an Invalid Report Type for use with this function.*/
   /* The final two parameters to this function are the Length of the   */
   /* Report Payload to send and a pointer to the Report Payload that   */
   /* will be sent.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Set Report Confirmation event         */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Set_Report_Request(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Set_Report_Request_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);
#endif

   /* The following function is responsible for sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Stack which is to   */
   /* send the request and the Bluetooth device address to send the     */
   /* request to.  This function returns a zero if successful, or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Get Protocol Confirmation event       */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Get_Protocol_Request(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Get_Protocol_Request_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);
#endif

   /* The following function is responsible for sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts the  */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* request and the Bluetooth device address to send the request      */
   /* to.  The last parameter is the protocol to be set.  This function */
   /* returns a zero if successful, or a negative return error code if  */
   /* there was an error.                                               */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Set Protocol Confirmation event       */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Set_Protocol_Request(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Protocol_Type_t Protocol);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Set_Protocol_Request_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Protocol_Type_t Protocol);
#endif

   /* The following function is responsible for sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts the  */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* request and the Bluetooth device address to send the request to.  */
   /* This function returns a zero if successful, or a negative return  */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Get Idle Confirmation event           */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Get_Idle_Request(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Get_Idle_Request_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);
#endif

   /* The following function is responsible for sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts the  */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* request and the Bluetooth device address to send the request to.  */
   /* The last parameter is the Idle Rate to be set.  The Idle Rate LSB */
   /* is weighted to 4ms (i.e. the Idle Rate resolution is 4ms with     */
   /* a range from 4ms to 1.020s).  This function returns a zero if     */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Set Idle Confirmation event           */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Set_Idle_Request(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Byte_t IdleRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Set_Idle_Request_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, Byte_t IdleRate);
#endif

   /* The following function is used to set the internal repeat key     */
   /* behavior.  Some host systems natively support key repeat, however,*/
   /* for the host systems that do not - this function will             */
   /* enable/disable key repeat events generated by this module.  This  */
   /* function accepts the Bluetooth stack ID of the Bluetooth stack to */
   /* set the repeat key parameters.  The second parameter is the amount*/
   /* of delay (specified in milliseconds) before starting the initial  */
   /* key repeat.  The third parameter specifies the rate of the key    */
   /* repeat (specified in milliseconds).  Note that setting the        */
   /* RepeatDelay to zero will disable keyboard key repeat              */
   /* functionality.  This function returns zero if successful, or a    */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Set_Keyboard_Repeat(unsigned int BluetoothStackID, unsigned int RepeatDelay, unsigned int RepeatRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Set_Keyboard_Repeat_t)(unsigned int BluetoothStackID, unsigned int RepeatDelay, unsigned int RepeatRate);
#endif

   /* The following function is responsible for retrieving the current  */
   /* HID Host server connection mode.  This function accepts as its    */
   /* first parameter the Bluetooth stack ID of the Bluetooth stack on  */
   /* which the HID Host server exists.  The final parameter to this    */
   /* function is a pointer to a server connection mode variable which  */
   /* will receive the current server connection mode.  This function   */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* * NOTE * The default server connection mode is:                   */
   /*             hsmAutomaticAccept.                                   */
   /* * NOTE * This function is used for HID servers which use          */
   /*          Bluetooth Security Mode 2.                               */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Get_Server_Connection_Mode(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t *ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t *ServerConnectionMode);
#endif

   /* The following function is responsible for setting the HID Host    */
   /* server connection mode.  This function accepts as its first       */
   /* parameter the Bluetooth stack ID of the Bluetooth stack in which  */
   /* the server exists.  The second parameter to this function is the  */
   /* new server connection mode to set the server to use.  HID Host    */
   /* connection requests will not be dispatched unless the server mode */
   /* (second parameter) is set to hsmManualAccept.  In this case the   */
   /* callback that was registered with the server will be invoked      */
   /* whenever a remote Bluetooth device attempts to connect to the     */
   /* local HID host server.  If the server mode (second parameter) is  */
   /* set to anything other than hsmManualAccept then no connection     */
   /* request indication events will be dispatched.  This function      */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* * NOTE * The default server connection mode is:                   */
   /*             hsmAutomaticAccept.                                   */
   /* * NOTE * This function is used for HID servers which use          */
   /*          Bluetooth Security Mode 2.                               */
BTPSAPI_DECLARATION int BTPSAPI HID_Host_Set_Server_Connection_Mode(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Host_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t ServerConnectionMode);
#endif

#endif

/*****< btpmerr.c >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMERR - Bluetopia Platform Manager Error constants and error message    */
/*            conversion functions.                                           */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/19/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "BTPMERR.h"             /* BTPM Error Code Constants.                */

   /* The following function is a utility function that exists to       */
   /* retrieve the human readable string value for the specified error  */
   /* code (first parameter).  This function will ALWAYS return a       */
   /* NON-NULL value that is a pointer to a static string.  The memory  */
   /* that this string points to owned by the error handler module and  */
   /* cannot be changed (i.e.  the pointer must be treated as pointing  */
   /* to constant data.                                                 */
char *BTPSAPI ERR_ConvertErrorCodeToString(int ErrorCode)
{
   char *ret_val;

   /* Simply convert the specified error code into the correct string   */
   /* and return a pointer to it.                                       */
   switch(ErrorCode)
   {
      case BTPM_ERROR_CODE_INVALID_PARAMETER:
         ret_val = "Invalid Parameter";
         break;
      case BTPM_ERROR_CODE_INVALID_CALLBACK_SPECIFIED:
         ret_val = "Invalid callback specified";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CREATE_MAILBOX:
         ret_val = "Unable to create Mailbox";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CREATE_MUTEX:
         ret_val = "Unable to create Mutex";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CREATE_EVENT:
         ret_val = "Unable to create Event";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CREATE_THREAD:
         ret_val = "Unable to create Thread";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_MEMORY:
         ret_val = "Unable to allocate Memory";
         break;
      case BTPM_ERROR_CODE_INSUFFICIENT_BUFFER_SIZE:
         ret_val = "Insufficient Buffer Size";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_REGISTER_HANDLER:
         ret_val = "Unable to register Handler";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_ADD_ENTRY:
         ret_val = "Unable to Add Entry";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_LOCK_CONTEXT:
         ret_val = "Unable to Lock Context";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_POST_MAILBOX:
         ret_val = "Unable to Post to Mailbox";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_ALLOCATE_TIMER:
         ret_val = "Unable to Allocate Timer";
         break;
      case BTPM_ERROR_CODE_ALREADY_INITIALIZED:
         ret_val = "Already Initialized";
         break;
      case BTPM_ERROR_CODE_PLATFORM_MANAGER_NOT_INITIALIZED:
         ret_val = "Platform Manager not Initialized";
         break;
      case BTPM_ERROR_CODE_TIMER_SYSTEM_NOT_INITIALIZED:
         ret_val = "Timer System not Initialized";
         break;
      case BTPM_ERROR_CODE_IPC_MODULE_NOT_INITIALIZED:
         ret_val = "IPC Module not Initialized";
         break;
      case BTPM_ERROR_CODE_IPC_UNABLE_TO_OPEN_DEVICE:
         ret_val = "Unable to Open Device (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_UNABLE_TO_OPEN_PIPE:
         ret_val = "Unable to Open Pipe (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_UNABLE_TO_OPEN_SOCKET:
         ret_val = "Unable to Open Socket (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_SERVER_ALREADY_RUNNING:
         ret_val = "Server already running (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_PIPE_ERROR:
         ret_val = "Pipe Error (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_SOCKET_ERROR:
         ret_val = "Socket Error (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_UNABLE_TO_SEND_DATA:
         ret_val = "Unable to Send Data (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_UNABLE_TO_RECEIVE_DATA:
         ret_val = "Unable to Receive Data (IPC)";
         break;
      case BTPM_ERROR_CODE_IPC_SERVER_CLOSED:
         ret_val = "Server Closed Connection (IPC)";
         break;
      case BTPM_ERROR_CODE_MESSAGE_SYSTEM_NOT_INITIALIZED:
         ret_val = "Message System not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_MESSAGE_GROUP:
         ret_val = "Message Group Invalid";
         break;
      case BTPM_ERROR_CODE_INVALID_MESSAGE_FUNCTION:
         ret_val = "Message Function Invalid";
         break;
      case BTPM_ERROR_CODE_INVALID_MESSAGE_ID:
         ret_val = "Message ID Invalid";
         break;
      case BTPM_ERROR_CODE_RESPONSE_MESSAGE_INVALID:
         ret_val = "Response Message Invalid";
         break;
      case BTPM_ERROR_CODE_RESPONSE_MESSAGE_TIMEOUT:
         ret_val = "Response Message Timeout";
         break;
      case BTPM_ERROR_CODE_MESSAGE_PROCESSING_ERROR:
         ret_val = "Error while processing Message";
         break;
      case BTPM_ERROR_CODE_LOCAL_DEVICE_NOT_INITIALIZED:
         ret_val = "Local Device Manager not Initialized";
         break;
      case BTPM_ERROR_CODE_LOCAL_DEVICE_POWERED_DOWN:
         ret_val = "Local Device is Powered Down";
         break;
      case BTPM_ERROR_CODE_LOCAL_DEVICE_ALREADY_POWERED_DOWN:
         ret_val = "Local Device is already Powered Down";
         break;
      case BTPM_ERROR_CODE_LOCAL_DEVICE_ALREADY_POWERED_UP:
         ret_val = "Local Device is already Powered Up";
         break;
      case BTPM_ERROR_CODE_LOCAL_DEVICE_POWERING_DOWN:
         ret_val = "Local Device is currently Powering Down";
         break;
      case BTPM_ERROR_CODE_LOCAL_DEVICE_POWERING_UP:
         ret_val = "Local Device is currently Powering Up";
         break;
      case BTPM_ERROR_CODE_BLUETOOTH_DEBUG_ALREADY_ENABLED:
         ret_val = "Bluetooth Debugging already enabled";
         break;
      case BTPM_ERROR_CODE_UNKNOWN_BLUETOOTH_DEVICE:
         ret_val = "Unknown Bluetooth device";
         break;
      case BTPM_ERROR_CODE_BLUETOOTH_DEVICE_ALREADY_EXISTS:
         ret_val = "Bluetooth Device already exists";
         break;
      case BTPM_ERROR_CODE_BLUETOOTH_DEVICE_ALREADY_PAIRED:
         ret_val = "Bluetooth Device already Paired";
         break;
      case BTPM_ERROR_CODE_DEVICE_CONNECTION_IN_PROGRESS:
         ret_val = "Bluetooth Device Connection in Progress";
         break;
      case BTPM_ERROR_CODE_DEVICE_IS_CURRENTLY_CONNECTED:
         ret_val = "Bluetooth Device is currently Connected";
         break;
      case BTPM_ERROR_CODE_DEVICE_IS_NOT_CURRENTLY_CONNECTED:
         ret_val = "Bluetooth Device is not currently Connected";
         break;
      case BTPM_ERROR_CODE_DEVICE_DISCOVERY_IN_PROGRESS:
         ret_val = "Device Discovery in Progress";
         break;
      case BTPM_ERROR_CODE_DEVICE_PAIRING_IN_PROGRESS:
         ret_val = "Device Pairing in Progress";
         break;
      case BTPM_ERROR_CODE_DEVICE_NOT_CURRENTLY_IN_PAIRING_PROCESS:
         ret_val = "Device is not currently in Pairing Process";
         break;
      case BTPM_ERROR_CODE_PROPERTY_UPDATE_IN_PROGRESS:
         ret_val = "Property Update in Progress";
         break;
      case BTPM_ERROR_CODE_SERVICE_DISCOVERY_IN_PROGRESS:
         ret_val = "Service Discovery in Progress";
         break;
      case BTPM_ERROR_CODE_AUTHENTICATION_HANDLER_ALREADY_REGISTERED:
         ret_val = "Authentication Handler already Registered";
         break;
      case BTPM_ERROR_CODE_AUTHENTICATION_HANDLER_NOT_REGISTERED:
         ret_val = "Authentication Handler not Registered";
         break;
      case BTPM_ERROR_CODE_INVALID_AUTHENTICATION_ACTION_SPECIFIED:
         ret_val = "Invalid Authentication Action Specified";
         break;
      case BTPM_ERROR_CODE_DEVICE_SERVICES_NOT_KNOWN:
         ret_val = "Device Services not Known";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_PARSE_SERVICE_DATA:
         ret_val = "Unable to Parse Service Data";
         break;
      case BTPM_ERROR_CODE_SERVICE_DATA_INVALID:
         ret_val = "Service Data Invalid";
         break;
      case BTPM_ERROR_CODE_DEVICE_IN_NON_PAIRABLE_MODE:
         ret_val = "Device is currently in Non-pairable Mode";
         break;
      case BTPM_ERROR_CODE_DEVICE_COMMUNICATION_ERROR:
         ret_val = "Local Device Communication Error";
         break;
      case BTPM_ERROR_CODE_DEVICE_ALREADY_AUTHENTICATED:
         ret_val = "Device is already Authenticated";
         break;
      case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_IN_PROGRESS:
         ret_val = "Device Authentication in Progress";
         break;
      case BTPM_ERROR_CODE_DEVICE_AUTHENTICATION_FAILED:
         ret_val = "Device Authentication Failed";
         break;
      case BTPM_ERROR_CODE_DEVICE_ALREADY_ENCRYPTED:
         ret_val = "Device is already Encrypted";
         break;
      case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_IN_PROGRESS:
         ret_val = "Device Encryption in Progress";
         break;
      case BTPM_ERROR_CODE_DEVICE_ENCRYPTION_FAILED:
         ret_val = "Device Encryption Failed";
         break;
      case BTPM_ERROR_CODE_DEVICE_CONNECTION_FAILED:
         ret_val = "Device Connection Failed";
         break;
      case BTPM_ERROR_CODE_DEVICE_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "Device Connection Retries Exceeded";
         break;
      case BTPM_ERROR_CODE_DEVICE_EXIT_SNIFF_MODE_ERROR:
         ret_val = "Exit Sniff Mode Error";
         break;
      case BTPM_ERROR_CODE_DEVICE_SNIFF_MODE_NOT_ACTIVE:
         ret_val = "Sniff Mode not active";
         break;
      case BTPM_ERROR_CODE_SERVICE_RECORD_ATTRIBUTE_DATA_INVALID:
         ret_val = "Service Record Attribute Data Invalid";
         break;
      case BTPM_ERROR_CODE_SERVICE_RECORD_ALREADY_REGISTERED:
         ret_val = "Service Record already Registered";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_NOT_INITIALIZED:
         ret_val = "Serial Port Manager not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_CONNECTION_STATE:
         ret_val = "Invalid Connection State";
         break;
      case BTPM_ERROR_CODE_INVALID_SERIAL_PORT:
         ret_val = "Invalid Serial Port";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_SERIAL_PORT:
         ret_val = "Unable to connect to Remote Serial Port";
         break;
      case BTPM_ERROR_CODE_REMOTE_SERIAL_PORT_CONNECTION_REFUSED:
         ret_val = "Remote Serial Port Connection Refused";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_CLOSE_IN_PROGRESS:
         ret_val = "Serial Port is closing";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_READ_IN_PROGRESS:
         ret_val = "Serial Port Read operation in progress";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_WRITE_IN_PROGRESS:
         ret_val = "Serial Port Write operation in progress";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_WRITE_DATA_FLUSH_STARTED:
         ret_val = "Serial Port Write Data Flush Started";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_INVALID_BLOCKING_OPERATION:
         ret_val = "Serial Port Blocking Operation not Allowed";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_IS_STILL_CONNECTED:
         ret_val = "Serial Port is still Connected";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "Serial Port Connection Retries Exceeded";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_INVALID_MFI_CONFIGURATION:
         ret_val = "Invalid MFi Configuration specified";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_AUTHENTICATION_COPROCESSOR:
         ret_val = "Serial Port Authentication Co-Processor Error";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_OPERATION_ERROR:
         ret_val = "Serial Port Operation Error";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_INVALID_SESSION_ID:
         ret_val = "Invalid Serial Port Session ID";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_MFI_DISCOVERY_ERROR:
         ret_val = "MFi Discovery Error";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_MFI_NOT_SUPPORTED:
         ret_val = "MFi not supported";
         break;
      case BTPM_ERROR_CODE_SERIAL_PORT_IS_NOT_AVAILABLE:
         ret_val = "Serial Port is not available";
         break;
      case BTPM_ERROR_CODE_LOW_ENERGY_NOT_SUPPORTED:
         ret_val = "Low Energy is not supported/available";
         break;
      case BTPM_ERROR_CODE_LOW_ENERGY_NOT_CONNECTED:
         ret_val = "Not connected using Low Energy to device";
         break;
      case BTPM_ERROR_CODE_NO_CONFIGURED_LOCAL_DEVICE_ID_INFORMATION:
         ret_val = "No configured Local Device ID Information";
         break;
      case BTPM_ERROR_CODE_INVALID_FEATURE_ACTION:
         ret_val = "Invalid feature change request";
         break;
      case BTPM_ERROR_CODE_LOW_ENERGY_NOT_CURRENTLY_ENABLED:
         ret_val = "Low Energy is not currently enabled";
         break;
      case BTPM_ERROR_CODE_ANT_PLUS_NOT_SUPPORTED:
         ret_val = "ANT+ is not supported";
         break;
      case BTPM_ERROR_CODE_ANT_PLUS_NOT_CURRENTLY_ENABLED:
         ret_val = "ANT+ is not currently enabled";
         break;
      case BTPM_ERROR_CODE_INVALID_LOW_ENERGY_MASTER_SLAVE_ROLE:
         ret_val = "Invalid Low Energy Master/Slave role configuration request";
         break;
      case BTPM_ERROR_CODE_TIMER_PLATFORM_ERROR:
         ret_val = "Timer platform specific error";
         break;
      case BTPM_ERROR_CODE_OBSERVATION_SCAN_IN_PROGESS:
         ret_val = "Operation not allowed, observation scan in progress";
         break;
      case BTPM_ERROR_CODE_INTERLEAVED_ADVERTISING_ACTIVE:
         ret_val = "Operation not allowed, interleaved advertising scheduling active";
         break;
      case BTPM_ERROR_CODE_ADVERTISING_NOT_CURRENTLY_ENABLED:
         ret_val = "Operation not allowed, advertising is not currently enabled";
         break;
      case BTPM_ERROR_CODE_INTERLEAVED_ADVERTISING_SUSPENDED:
         ret_val = "Operation not allowed, interleaved advertising is not currently enabled";
         break;
      case BTPM_ERROR_CODE_SCO_NOT_INITIALIZED:
         ret_val = "SCO Manager not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_SCO_CONNECTION:
         ret_val = "Invalid SCO Connection";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_SCO_CONNECTION:
         ret_val = "Unable to Connect Remote SCO Connection";
         break;
      case BTPM_ERROR_CODE_SCO_CONNECTION_CLOSE_IN_PROGRESS:
         ret_val = "SCO Connection is closing";
         break;
      case BTPM_ERROR_CODE_INVALID_AUDIO_INITIALIZATION_DATA:
         ret_val = "Invalid Audio Initialization Data";
         break;
      case BTPM_ERROR_CODE_AUDIO_NOT_INITIALIZED:
         ret_val = "Audio Manager not Initialized";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_AUDIO_STREAM:
         ret_val = "Unable to connect Remote Audio Stream";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_CONNECTION_IN_PROGRESS:
         ret_val = "Remote Audio Stream Connection in progress";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_ALREADY_CONNECTED:
         ret_val = "Audio Stream already connected";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_NOT_CONNECTED:
         ret_val = "Audio Stream not connected";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "Audio Stream Event Handler not Registered";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_STATE_CHANGE_IN_PROGRESS:
         ret_val = "Audio Stream State Change in progress";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_FORMAT_IS_NOT_SUPPORTED:
         ret_val = "Audio Stream Format is not supported";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_FORMAT_CHANGE_IN_PROGRESS:
         ret_val = "Audio Stream Format Change in progress";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_STATE_IS_ALREADY_STARTED:
         ret_val = "Audio Stream State is already started";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_STATE_IS_ALREADY_SUSPENDED:
         ret_val = "Audio Stream State is already suspended";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_DATA_ALREADY_REGISTERED:
         ret_val = "Audio Stream Data Event already registered";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_ACTION_NOT_PERMITTED:
         ret_val = "Audio Stream Action not permitted";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_EVENT_ALREADY_REGISTERED:
         ret_val = "Remote Control Event already registered";
         break;
      case BTPM_ERROR_CODE_UNKNOWN_REMOTE_CONTROL_EVENT_TYPE:
         ret_val = "Unknown Remote Control Event type";
         break;
      case BTPM_ERROR_CODE_INVALID_REMOTE_CONTROL_EVENT_DATA:
         ret_val = "Invalid Remote Control Event data";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_SEND_REMOTE_CONTROL_COMMAND:
         ret_val = "Unable to send Remote Control Command";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_SEND_REMOTE_CONTROL_RESPONSE:
         ret_val = "Unable to send Remote Control Response";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_RESPONSE_TIMEOUT_ERROR:
         ret_val = "Remote Control Command Response Timeout Error";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_RESPONSE_UNKNOWN_ERROR:
         ret_val = "Remote Control Command Response Unknown Error";
         break;
      case BTPM_ERROR_CODE_AUDIO_STREAM_NOT_INITIALIZED:
         ret_val = "Audio Stream not initialized";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_CONNECTION_IN_PROGRESS:
         ret_val = "Remote Control Connection in progress";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_ALREADY_CONNECTED:
         ret_val = "Remote Control already connected";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_NOT_CONNECTED:
         ret_val = "Remote Control not connected";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_CONTROL:
         ret_val = "Unable to connect Remote Control";
         break;
      case BTPM_ERROR_CODE_REMOTE_CONTROL_ROLE_IS_NOT_REGISTERED:
         ret_val = "Remote control service type role is not registered";
         break;
      case BTPM_ERROR_CODE_HID_NOT_INITIALIZED:
         ret_val = "HID not Initialized";
         break;
      case BTPM_ERROR_CODE_HID_DEVICE_ALREADY_CONNECTED:
         ret_val = "HID Device already connected";
         break;
      case BTPM_ERROR_CODE_HID_DEVICE_CONNECTION_IN_PROGRESS:
         ret_val = "HID Device connection in progress";
         break;
      case BTPM_ERROR_CODE_UNABLE_TO_CONNECT_REMOTE_HID_DEVICE:
         ret_val = "Unable to connect to remote HID Device";
         break;
      case BTPM_ERROR_CODE_HID_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "HID Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_HID_DATA_EVENTS_ALREADY_REGISTERED:
         ret_val = "HID Data Event handler already registered";
         break;
      case BTPM_ERROR_CODE_HID_DATA_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "HID Data Event handler not registered";
         break;
      case BTPM_ERROR_CODE_HID_UNABLE_TO_CLOSE_REMOTE_HID_DEVICE:
         ret_val = "Unable to close remote HID Device";
         break;
      case BTPM_ERROR_CODE_NO_ASSOCIATED_HID_DEVICES_FOR_WAKEUP:
         ret_val = "No associated HID Devices for remote wakeup";
         break;
      case BTPM_ERROR_CODE_MAXIMUM_ASSOCIATED_HID_DEVICES_REACHED:
         ret_val = "Maximum number of concurrent associated HID Devices reached";
         break;
      case BTPM_ERROR_CODE_HID_DEVICE_NOT_ASSOCIATED:
         ret_val = "HID Device not associated";
         break;
      case BTPM_ERROR_CODE_HID_DEVICE_ALREADY_ASSOCIATED:
         ret_val = "HID Device already associated";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_NOT_INITIALIZED:
         ret_val = "Hands Free not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_HANDS_FREE_INITIALIZATION_DATA:
         ret_val = "Invalid Hands Free Initialization Data";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_ROLE_NOT_SUPPORTED:
         ret_val = "Hands Free role not supported";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_INVALID_OPERATION:
         ret_val = "Hands Free operation not allowed";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_IN_PROGRESS:
         ret_val = "Hands Free connection in progress";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_ALREADY_CONNECTED:
         ret_val = "Hands Free already connected";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_NOT_CONNECTED:
         ret_val = "Hands Free not connected";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_IS_STILL_CONNECTED:
         ret_val = "Device is still connected";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "Unable to connect to remote device";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "Number connection retries exceeded";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "Unable to disconnect remote device";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_ALREADY_REGISTERED:
         ret_val = "Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_ALREADY_REGISTERED:
         ret_val = "Data Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_HANDS_FREE_DATA_HANDLER_NOT_REGISTERED:
         ret_val = "Data Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_INITIALIZED:
         ret_val = "Phone Book Access not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_PHONE_BOOK_ACCESS_INITIALIZATION_DATA:
         ret_val = "Invalid Phone Book Access Initialization Data";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_CLOSE_IN_PROGRESS:
         ret_val = "Phone Book Access close connection operation in progress";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_ROLE_NOT_SUPPORTED:
         ret_val = "Phone Book Access role not supported";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_INVALID_OPERATION:
         ret_val = "Phone Book Access operation not allowed";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_CONNECTION_IN_PROGRESS:
         ret_val = "Phone Book Access connection in progress";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_ALREADY_CONNECTED:
         ret_val = "Phone Book Access connection in progress";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_NOT_CONNECTED:
         ret_val = "Phone Book Access not connected";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_IS_STILL_CONNECTED:
         ret_val = "Phone Book Access still connected";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "Unable to connect to remote device";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "Connection retries exceeded";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "Unable to disconnect Phone Book Access device";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_EVENT_HANDLER_ALREADY_REGISTERED:
         ret_val = "Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_DATA_HANDLER_ALREADY_REGISTERED:
         ret_val = "Data Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_PHONE_BOOK_ACCESS_DATA_HANDLER_NOT_REGISTERED:
         ret_val = "Data Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_PAN_NOT_INITIALIZED:
         ret_val = "PAN not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_PAN_INITIALIZATION_DATA:
         ret_val = "Invalid PAN Initialization Data";
         break;
      case BTPM_ERROR_CODE_PAN_ROLE_NOT_SUPPORTED:
         ret_val = "PAN role not supported";
         break;
      case BTPM_ERROR_CODE_PAN_INVALID_OPERATION:
         ret_val = "PAN operation not allowed";
         break;
      case BTPM_ERROR_CODE_PAN_CONNECTION_IN_PROGRESS:
         ret_val = "Device connection in progress";
         break;
      case BTPM_ERROR_CODE_PAN_DEVICE_ALREADY_CONNECTED:
         ret_val = "Device already connected";
         break;
      case BTPM_ERROR_CODE_PAN_DEVICE_NOT_CONNECTED:
         ret_val = "Device not connected";
         break;
      case BTPM_ERROR_CODE_PAN_DEVICE_IS_STILL_CONNECTED:
         ret_val = "Device is still connected";
         break;
      case BTPM_ERROR_CODE_PAN_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "Unable to connect to remote device";
         break;
      case BTPM_ERROR_CODE_PAN_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "Unable to disconnect remote device";
         break;
      case BTPM_ERROR_CODE_PAN_EVENT_HANDLER_ALREADY_REGISTERED:
         ret_val = "Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_PAN_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_NOT_INITIALIZED:
         ret_val = "Message Access not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_MESSAGE_ACCESS_INITIALIZATION_DATA:
         ret_val = "Invalid Message Access Initialization Data";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_OPERATION:
         ret_val = "Message Access operation not allowed";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_CONNECTION_IN_PROGRESS:
         ret_val = "Device connection in progress";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_ALREADY_CONNECTED:
         ret_val = "Device already connected";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_DEVICE_NOT_CONNECTED:
         ret_val = "Device not connected";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "Unable to connect to remote device";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "Connection retries exceeded";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "Unable to disconnect Message Access device";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_DUPLICATE_INSTANCE_ID:
         ret_val = "Instance ID already registered";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_INSTANCE_ID:
         ret_val = "Invalid Instance ID";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_SERVICE_NOT_ADVERTISED:
         ret_val = "Message Access Service not advertised by device";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_NOTIFICATIONS_NOT_ENABLED:
         ret_val = "Notifications have not been enabled";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_ABORT_OPERATION_IN_PROGRESS:
         ret_val = "Abort operation in progress";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_NO_OPERATION_IN_PROGRESS:
         ret_val = "No operation in progress";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_FOLDER_NAME:
         ret_val = "Invalid Folder Name";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_MESSAGE_HANDLE:
         ret_val = "Invalid Message Handle";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_PATH:
         ret_val = "Invalid Path";
         break;
      case BTPM_ERROR_CODE_MESSAGE_ACCESS_INVALID_CLIENT:
         ret_val = "Invalid Client";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_NOT_INITIALIZED:
         ret_val = "GATT not initialized";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_NOT_SUPPORTED:
         ret_val = "GATT not supported";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_EVENT_CALLBACK_ID:
         ret_val = "GATT Invalid Event Callback ID";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_CONNECTION:
         ret_val = "GATT Invalid Connection";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_SERVICE_TABLE:
         ret_val = "GATT Invalid Service Table";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_OPERATION:
         ret_val = "GATT Invalid Operation";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_ATTRIBUTE_HANDLE:
         ret_val = "GATT Invalid Attribute Handle";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INSUFFICIENT_HANDLES:
         ret_val = "GATT Insufficient Handles";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_PERSISTENT_UID:
         ret_val = "GATT Invalid Persistent UID";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_ATTRIBUTE_OFFSET:
         ret_val = "GATT Invalid Attribute Offset";
         break;
      case BTPM_ERROR_CODE_GENERIC_ATTRIBUTE_INVALID_SERVICE_STATE:
         ret_val = "GATT Invalid Service State";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_INITIALIZED:
         ret_val = "ANP Manager not initialized";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_EVENT_ALREADY_REGISTERED:
         ret_val = "ANP events already registered";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_INVALID_CALLBACK_ID:
         ret_val = "ANP Invalid Callback ID";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_REQUEST_IN_PROGRESS:
         ret_val = "ANP Request Already In Progress";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_ALREADY_ENABLED:
         ret_val = "ANP Notifications or Category Already Enabled";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_ENABLED:
         ret_val = "ANP Notifications of Category Not Enabled";
         break;
      case BTPM_ERROR_CODE_ALERT_NOTIFICATION_NOT_CONNECTED:
         ret_val = "ANP Device Not Connected (or does not support ANP)";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_NOT_INITIALIZED:
         ret_val = "HRP Manager not initialized";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_CALLBACK_ALREADY_REGISTERED:
         ret_val = "HRP callback already registered";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_CALLBACK_NOT_REGISTERED:
         ret_val = "HRP callback not registered";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_SENSOR_NOT_AVAILABLE:
         ret_val = "HRP Sensor not available on remote device";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_BODY_SENSOR_LOCATION_NOT_SUPPORTED:
         ret_val = "HRP Body Sensor Location not supported on remote device";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_BODY_SENSOR_LOCATION_OUTSTANDING:
         ret_val = "HRP Body Sensor Location request currently outstanding";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_RESET_ENERGY_EXPENDED_NOT_SUPPORTED:
         ret_val = "HRP Reset Energy Expended not supported on remote device";
         break;
      case BTPM_ERROR_CODE_HEART_RATE_RESET_ENERGY_EXPENDED_OUTSTANDING:
         ret_val = "HRP Reset Energy Expended request currently outstanding";
         break;
      case BTPM_ERROR_CODE_PROXIMITY_NOT_INITIALIZED:
         ret_val = "PXP Manager not initialized";
         break;
      case BTPM_ERROR_CODE_PROXIMITY_EVENT_ALREADY_REGISTERED:
         ret_val = "PXP events already registered";
         break;
      case BTPM_ERROR_CODE_PROXIMITY_NOT_CONNECTED_TO_DEVICE:
         ret_val = "PXP not connected to device";
         break;
      case BTPM_ERROR_CODE_PROXIMITY_TX_POWER_NOT_VALID:
         ret_val = "PXP Tx Power not valid";
         break;
      case BTPM_ERROR_CODE_FIND_ME_NOT_INITIALIZED:
         ret_val = "FMP Manager not initialized";
         break;
      case BTPM_ERROR_CODE_TIME_NOT_INITIALIZED:
         ret_val = "TIP Manager not initialized";
         break;
      case BTPM_ERROR_CODE_TIME_SERVER_CALLBACK_ALREADY_REGISTERED:
         ret_val = "TIP Server callback already registered";
         break;
      case BTPM_ERROR_CODE_TIME_UNABLE_TO_READ_TIME:
         ret_val = "TIP Un-able to read local time";
         break;
      case BTPM_ERROR_CODE_TIME_CLIENT_NOT_CONNECTED:
         ret_val = "TIP Client not connected";
         break;
      case BTPM_ERROR_CODE_TIME_NOTIFICATIONS_ALREADY_ENABLED:
         ret_val = "TIP Notifications already enabled";
         break;
      case BTPM_ERROR_CODE_TIME_REMOTE_OPERATION_NOT_SUPPORTED:
         ret_val = "TIP Remote Operation not supported";
         break;
      case BTPM_ERROR_CODE_TIME_UNKNOWN_ERROR:
         ret_val = "TIP Unknown Error";
         break;
      case BTPM_ERROR_CODE_PHONE_ALERT_STATUS_NOT_INITIALIZED:
         ret_val = "PAS Manager not initialized";
         break;
      case BTPM_ERROR_CODE_BATTERY_NOT_INITIALIZED:
         ret_val = "BAS Manager not initialized";
         break;
      case BTPM_ERROR_CODE_BATTERY_CALLBACK_NOT_REGISTERED:
         ret_val = "BAS callback not registered";
         break;
      case BTPM_ERROR_CODE_BATTERY_INVALID_INSTANCE:
         ret_val = "BAS Invalid Battery Service Instance specified";
         break;
      case BTPM_ERROR_CODE_BATTERY_NOTIFICATIONS_DISABLED:
         ret_val = "BAS Notifications are disabled";
         break;
      case BTPM_ERROR_CODE_BATTERY_NOTIFY_UNSUPPORTED:
         ret_val = "BAS Notify is not supported on this instance";
         break;
      case BTPM_ERROR_CODE_BATTERY_IDENTIFICATION_UNSUPPORTED:
         ret_val = "BAS Identification is not supported on this device";
         break;
      case BTPM_ERROR_CODE_BATTERY_READ_REQUEST:
         ret_val = "BAS Could not send read request";
         break;
      case BTPM_ERROR_CODE_BATTERY_SAME_REQUEST_OUTSTANDING:
         ret_val = "BAS Same request is already outstanding";
         break;
      case BTPM_ERROR_CODE_BATTERY_INVALID_TRANSACTION_ID:
         ret_val = "BAS Invalid Transaction ID specified";
         break;
      case BTPM_ERROR_CODE_HID_OVER_GATT_NOT_INITIALIZED:
         ret_val = "HOG not initialized";
         break;
      case BTPM_ERROR_CODE_HID_OVER_GATT_FEATURE_NOT_SUPPORTED:
         ret_val = "HOG Feature not supported";
         break;
      case BTPM_ERROR_CODE_HID_OVER_GATT_REQUEST_NOT_VALID:
         ret_val = "HOG Request not valid";
         break;
      case BTPM_ERROR_CODE_HID_OVER_GATT_PROTOCOL_MODE_CURRENT:
         ret_val = "HOG Protocol mode already set";
         break;
      case BTPM_ERROR_CODE_HID_OVER_GATT_INVALID_REPORT:
         ret_val = "HOG Invalid Report";
         break;
      case BTPM_ERROR_CODE_HID_OVER_GATT_DATA_EVENTS_ALREADY_REGISTERED:
         ret_val = "HOG Data Events already registered";
         break;
      case BTPM_ERROR_CODE_HEALTH_THERMOMETER_NOT_INITIALIZED:
         ret_val = "HTPM not initialized";
         break;
      case BTPM_ERROR_CODE_HEALTH_THERMOMETER_FEATURE_NOT_SUPPORTED:
         ret_val = "HTPM feature not supported";
         break;
      case BTPM_ERROR_CODE_HEALTH_THERMOMETER_OPERATION_IN_PROGRESS:
         ret_val = "HTPM operation in progress";
         break;
      case BTPM_ERROR_CODE_GLUCOSE_NOT_INITIALIZED:
         ret_val = "GLPM not initialized";
         break;
      case BTPM_ERROR_CODE_GLUOCSE_INVALID_PROCEDURE_DATA:
         ret_val = "GLPM invalid procedure data";
         break;
      case BTPM_ERROR_CODE_GLUOCSE_PROCEDURE_IN_PROGRESS:
         ret_val = "GLPM procedure already in progress";
         break;
      case BTPM_ERROR_CODE_GLUOCSE_STOP_PROCEDURE_IN_PROGRESS:
         ret_val = "GLPM stop procedure already in progress";
         break;
      case BTPM_ERROR_CODE_FTP_NOT_INITIALIZED:
         ret_val = "FTPM not initialized";
         break;
      case BTPM_ERROR_CODE_GOEP_COMMAND_NOT_ALLOWED:
         ret_val = "FTPM GOEP Command not allowed";
         break;
      case BTPM_ERROR_CODE_FTP_ACTION_NOT_ALLOWED:
         ret_val = "FTPM Action not allowed";
         break;
      case BTPM_ERROR_CODE_FTP_UNABLE_TO_CREATE_LOCAL_FILE:
         ret_val = "FTPM unable to create local file";
         break;
      case BTPM_ERROR_CODE_FTP_INVALID_ROOT_DIRECTORY:
         ret_val = "FTPM invalid root directory";
         break;
      case BTPM_ERROR_CODE_FTP_REQUEST_OUTSTANDING:
         ret_val = "FTPM request outstanding";
         break;
      case BTPM_ERROR_CODE_FTP_ALREADY_CONNECTED:
         ret_val = "FTPM already connected to device";
         break;
      case BTPM_ERROR_CODE_HEADSET_NOT_INITIALIZED:
         ret_val = "Headset not Initialized";
         break;
      case BTPM_ERROR_CODE_HEADSET_ROLE_NOT_SUPPORTED:
         ret_val = "Headset role not supported";
         break;
      case BTPM_ERROR_CODE_HEADSET_INVALID_OPERATION:
         ret_val = "Headset operation not allowed";
         break;
      case BTPM_ERROR_CODE_HEADSET_CONNECTION_IN_PROGRESS:
         ret_val = "Headset connection in progress";
         break;
      case BTPM_ERROR_CODE_HEADSET_ALREADY_CONNECTED:
         ret_val = "Headset already connected";
         break;
      case BTPM_ERROR_CODE_HEADSET_NOT_CONNECTED:
         ret_val = "Headset not connected";
         break;
      case BTPM_ERROR_CODE_HEADSET_IS_STILL_CONNECTED:
         ret_val = "Device is still connected";
         break;
      case BTPM_ERROR_CODE_HEADSET_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "Unable to connect to remote device";
         break;
      case BTPM_ERROR_CODE_HEADSET_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "Number connection retries exceeded";
         break;
      case BTPM_ERROR_CODE_HEADSET_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "Unable to disconnect remote device";
         break;
      case BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_ALREADY_REGISTERED:
         ret_val = "Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_HEADSET_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_HEADSET_DATA_HANDLER_ALREADY_REGISTERED:
         ret_val = "Data Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_HEADSET_DATA_HANDLER_NOT_REGISTERED:
         ret_val = "Data Event Handler not registered";
         break;
      case BTPM_ERROR_CODE_INVALID_HEADSET_INITIALIZATION_DATA:
         ret_val = "Invalid Headset Initialization Data";
         break;
      case BTPM_ERROR_CODE_ANT_NOT_INITIALIZED:
         ret_val = "ANT not initialized";
         break;
      case BTPM_ERROR_CODE_ANT_UNABLE_TO_SEND_ANT_MESSAGE:
         ret_val = "Unable to send ANT message";
         break;
      case BTPM_ERROR_CODE_ANT_MESSAGE_RESPONSE_ERROR:
         ret_val = "ANT message response error";
         break;
      case BTPM_ERROR_CODE_ANT_INVALID_NETWORK_KEY:
         ret_val = "Invalid ANT network key";
         break;
      case BTPM_ERROR_CODE_ANT_FEATURE_NOT_AVAILABLE:
         ret_val = "ANT feature not available";
         break;
      case BTPM_ERROR_CODE_ANT_INVALID_CALLBACK_ID:
         ret_val = "Invalid ANT Callback ID";
         break;
      case BTPM_ERROR_CODE_ANT_EVENT_HANDLER_ALREADY_REGISTERED:
         ret_val = "ANT Event Handler already registered";
         break;
      case BTPM_ERROR_CODE_ANCS_NOT_INITIALIZED:
         ret_val = "ANC Manager not initialized";
         break;
      case BTPM_ERROR_CODE_ANCS_CALLBACK_ALREADY_REGISTERED:
         ret_val = "ANC Manager callback already registered";
         break;
      case BTPM_ERROR_CODE_ANCS_CALLBACK_NOT_REGISTERED:
         ret_val = "ANC Manager callback not registered";
         break;
      case BTPM_ERROR_CODE_ANCS_INVALID_COMMAND_DATA:
         ret_val = "ANCM: Invalid command data";
         break;
      case BTPM_ERROR_CODE_ANCS_SERVICE_NOT_PRESENT:
         ret_val = "ANCM: The Apple Notification Center Service is not present on the remote device.";
         break;
      case BTPM_ERROR_CODE_ANCS_REQUEST_CURRENTLY_PENDING:
         ret_val = "ANCM: A Get Attributes request is currently pending for this device.";
         break;
      case BTPM_ERROR_CODE_ANCS_UNABLE_TO_PARSE_DATA:
         ret_val = "ANCM: The ANCM Module failed to parse incoming data.";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_NOT_INITIALIZED:
         ret_val = "OPPM Not Initialized.";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_OPERATION:
         ret_val = "OPPM Invalid Operation.";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_CONNECTION_IN_PROGRESS:
         ret_val = "OPPM Connection In Progress";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_ALREADY_CONNECTED:
         ret_val = "OPPM Device Already Connected";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_DEVICE_NOT_CONNECTED:
         ret_val = "OPPM Device Not Connected";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "OPPM Unable To Connect To Device";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_CONNECTION_RETRIES_EXCEEDED:
         ret_val = "OPPM Connection Retries Exceeded";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "OPPM Unable To Disconnect Device";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_SERVICE_NOT_ADVERTISED:
         ret_val = "OPPM Service Not Advertised";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_ABORT_OPERATION_IN_PROGRESS:
         ret_val = "OPPM Abort In Progress";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_NO_OPERATION_IN_PROGRESS:
         ret_val = "OPPM No Operation In Progress";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_CLIENT:
         ret_val = "OPPM Invalid Client";
         break;
      case BTPM_ERROR_CODE_OBJECT_PUSH_INVALID_SERVER_ID:
         ret_val = "OPPM Invalid Server ID";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_INITIALIZED:
         ret_val = "HDPM Not Initialized";
         break;
      case BTPM_ERROR_CODE_INVALID_HEALTH_DEVICE_INITIALIZATION_DATA:
         ret_val = "Invalid HDPM Initialization Data";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_SDP:
         ret_val = "HDPM Unable to Register SDP Record";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_DEVICE:
         ret_val = "HDPM Unable to Connect to Device";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_CONNECT_TO_ENDPOINT:
         ret_val = "HDPM Unable to Connect to Endpoint";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_DISCONNECT_DEVICE:
         ret_val = "HDPM Unable to Disconnect Device";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_DISCONNECT_ENDPOINT:
         ret_val = "HDPM Unable to Disconnect Endpoint";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_UNABLE_TO_REGISTER_ENDPOINT:
         ret_val = "HDPM Unable to Register Endpoint";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_ENDPOINT_ID:
         ret_val = "HDPM Invalid Endpoint ID";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_REGISTERED:
         ret_val = "HDPM Endpoint Not Registered";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_REMOTE_INSTANCE_IN_USE:
         ret_val = "HDPM Remote Instance in Use";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_CONNECTION_IN_PROGRESS:
         ret_val = "HDPM Connection in Progress";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_NOT_CONNECTED:
         ret_val = "HDPM Not Connected";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_ENDPOINT_NOT_CONNECTED:
         ret_val = "HDPM Endpoint Not Connected";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_SERVICE_NOT_FOUND:
         ret_val = "HDPM Service Not Found";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_DATALINK_ID:
         ret_val = "HDPM Invalid Data Link ID";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_INVALID_CHANNEL_MODE:
         ret_val = "HDPM Invalid Channel Mode (Refer to the HDP specification to determine valid source/sink channel mode combinations)";
         break;
      case BTPM_ERROR_CODE_HEALTH_DEVICE_INSTANCE_ALREADY_CONNECTED:
         ret_val = "HDPM Already Connected to Remote Instance";
         break;
      case BTPM_ERROR_CODE_HDD_NOT_INITIALIZED:
         ret_val = "HDDM Not Initialized";
         break;
      case BTPM_ERROR_CODE_HDD_HOST_ALREADY_CONNECTED:
         ret_val = "HDDM Host Already Connected";
         break;
      case BTPM_ERROR_CODE_HDD_CONNECTION_IN_PROGRESS:
         ret_val = "HDDM Connection In Progress";
         break;
      case BTPM_ERROR_CODE_HDD_UNABLE_TO_CONNECT_REMOTE_HOST:
         ret_val = "HDDM Unable To Connect Remote Host";
         break;
      case BTPM_ERROR_CODE_HDD_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "HDDM Event Handler Not Registered";
         break;
      case BTPM_ERROR_CODE_HDD_DATA_EVENTS_ALREADY_REGISTERED:
         ret_val = "HDDM Data Events Already Registered";
         break;
      case BTPM_ERROR_CODE_HDD_DATA_EVENT_HANDLER_NOT_REGISTERED:
         ret_val = "HDDM Data Event Handler Not Registered";
         break;
      case BTPM_ERROR_CODE_HDD_HOST_NOT_CONNECTED:
         ret_val = "HDDM Host Not Connected";
         break;
      case BTPM_ERROR_CODE_CSC_NOT_INITIALIZED:
         ret_val = "CSCM Not Initialized";
         break;
      case BTPM_ERROR_CODE_CSC_NOT_CONNECTED:
         ret_val = "CSCM Device Not Connected Or Does Not Support CSCS";
         break;
      case BTPM_ERROR_CODE_CSC_ALREADY_CONFIGURED:
         ret_val = "CSCM Device Already Configured";
         break;
      case BTPM_ERROR_CODE_CSC_NOT_CONFIGURED:
         ret_val = "CSCM Device Not Configured";
         break;
      case BTPM_ERROR_CODE_CSC_CONTROL_POINT_PROCEDURE_IN_PROGRESS:
         ret_val = "CSCM Control Point Procedure In Progress";
         break;
      case BTPM_ERROR_CODE_CSC_INVALID_CALLBACK_ID:
         ret_val = "CSCM Invalid Callback ID";
         break;
      case BTPM_ERROR_CODE_CSC_CURRENTLY_UN_CONFIGURING:
         ret_val = "CSCM Currently Un-Configuring Remote Device";
         break;
      case BTPM_ERROR_CODE_CSC_CURRENTLY_CONFIGURING:
         ret_val = "CSCM Currently Configuring Remote Device";
         break;
      case BTPM_ERROR_CODE_CSC_SAME_REQUEST_ALREADY_OUTSTANDING:
         ret_val = "CSCM Same Request Already Outstanding";
         break;
      case BTPM_ERROR_CODE_CSC_REMOTE_FEATURE_NOT_SUPPORTED:
         ret_val = "CSCM Remote Feature Not Supported";
         break;
      case BTPM_ERROR_CODE_RSC_NOT_INITIALIZED:
         ret_val = "RSCM Not Initialized";
         break;
      case BTPM_ERROR_CODE_RSC_NOT_CONNECTED:
         ret_val = "RSCM Device Not Connected Or Does Not Support RSCS";
         break;
      case BTPM_ERROR_CODE_RSC_ALREADY_CONFIGURED:
         ret_val = "RSCM Device Already Configured";
         break;
      case BTPM_ERROR_CODE_RSC_NOT_CONFIGURED:
         ret_val = "RSCM Device Not Configured";
         break;
      case BTPM_ERROR_CODE_RSC_CONTROL_POINT_PROCEDURE_IN_PROGRESS:
         ret_val = "RSCM Control Point Procedure In Progress";
         break;
      case BTPM_ERROR_CODE_RSC_INVALID_CALLBACK_ID:
         ret_val = "RSCM Invalid Callback ID";
         break;
      case BTPM_ERROR_CODE_RSC_CURRENTLY_UN_CONFIGURING:
         ret_val = "RSCM Currently Un-Configuring Remote Device";
         break;
      case BTPM_ERROR_CODE_RSC_CURRENTLY_CONFIGURING:
         ret_val = "RSCM Currently Configuring Remote Device";
         break;
      case BTPM_ERROR_CODE_RSC_SAME_REQUEST_ALREADY_OUTSTANDING:
         ret_val = "RSCM Same Request Already Outstanding";
         break;
      case BTPM_ERROR_CODE_RSC_REMOTE_FEATURE_NOT_SUPPORTED:
         ret_val = "RSCM Remote Feature Not Supported";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_NOT_INITIALIZED:
         ret_val = "Cycling Power Not Initialized";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_CALLBACK_NOT_REGISTERED:
         ret_val = "Cycling Power Callback Not Registered";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_INVALID_INSTANCE:
         ret_val = "Cycling Power Invalid Instance";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_INVALID_SENSOR:
         ret_val = "Cycling Power Invalid Sensor";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_UPDATE_NOT_REGISTERED:
         ret_val = "Cycling Power Update Not Registered";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_TRANSACTION_TIMEOUT:
         ret_val = "Cycling Power Transaction Timeout";
         break;
      case BTPM_ERROR_CODE_CYCLING_POWER_UNKNOWN_ERROR:
         ret_val = "Cycling Power Unknown Error";
         break;
      case BTPM_ERROR_CODE_3D_SYNC_NOT_INITIALIZED:
         ret_val = "TDSM Not Initialized";
         break;
      case BTPM_ERROR_CODE_3D_SYNC_INVALID_INITIALIZATION_DATA:
         ret_val = "TDSM Invalid Initialization Data";
         break;
      case BTPM_ERROR_CODE_3D_SYNC_INVALID_CALLBACK_ID:
         ret_val = "TDSM Invalid Callback ID";
         break;
      case BTPM_ERROR_CODE_3D_SYNC_CONTROL_ALREADY_REGISTERED:
         ret_val = "TDSM Control Callback Already Registered";
         break;
      case BTPM_ERROR_CODE_3D_SYNC_UNKNOWN_ERROR:
         ret_val = "TDSM Unknown Error";
         break;
      case BTPM_ERROR_CODE_DEVICE_ROLE_SWITCH_ERROR:
         ret_val = "Device Role Switch Error";
         break;


      default:
         /* If the value passed is NOT negative then assume this is a   */
         /* successful error code (i.e. no error).                      */
         if(ErrorCode >= 0)
            ret_val = "Success";
         else
            ret_val = "Unknown Error";
         break;
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}


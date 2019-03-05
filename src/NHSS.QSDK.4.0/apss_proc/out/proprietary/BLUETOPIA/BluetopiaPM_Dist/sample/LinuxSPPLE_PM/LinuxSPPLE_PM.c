/*****< linuxspple_pm.c >******************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXSPPLE_PM - Simple Linux application using Bluetopia Platform         */
/*                    Manager Generic Attribute Client API to implement SPP   */
/*                    over Low Energy.                                        */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/23/12  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>           /* Include for getpid().                        */

#include "LinuxSPPLE_PM.h"    /* Main Application Prototypes and Constants.   */

#include "SS1BTPM.h"          /* BTPM Application Programming Interface.      */

#define MAX_SUPPORTED_COMMANDS                     (64)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                        (128)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define DEFAULT_IO_CAPABILITY          (icDisplayYesNo)  /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Secure  */
                                                         /* Simple Pairing.   */

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

#define INDENT_LENGTH                                 3  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

#define NO_COMMAND_ERROR                           (-1)  /* Denotes that no   */
                                                         /* command was       */
                                                         /* specified to the  */
                                                         /* parser.           */

#define INVALID_COMMAND_ERROR                      (-2)  /* Denotes that the  */
                                                         /* Command does not  */
                                                         /* exist for         */
                                                         /* processing.       */

#define EXIT_CODE                                  (-3)  /* Denotes that the  */
                                                         /* Command specified */
                                                         /* was the Exit      */
                                                         /* Command.          */

#define FUNCTION_ERROR                             (-4)  /* Denotes that an   */
                                                         /* error occurred in */
                                                         /* execution of the  */
                                                         /* Command Function. */

#define TO_MANY_PARAMS                             (-5)  /* Denotes that there*/
                                                         /* are more          */
                                                         /* parameters then   */
                                                         /* will fit in the   */
                                                         /* UserCommand.      */

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* one or more of the*/
                                                         /* required          */
                                                         /* parameters were   */
                                                         /* invalid.          */

#define PLATFORM_MANAGER_NOT_INITIALIZED_ERROR     (-7)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* the Platform      */
                                                         /* Manager has not   */
                                                         /* been initialized. */

#define SPPLE_DATA_BUFFER_LENGTH  (BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE)
                                                         /* Defines the length*/
                                                         /* of a SPPLE Data   */
                                                         /* Buffer.           */

#define SPPLE_DATA_CREDITS        (SPPLE_DATA_BUFFER_LENGTH*3) /* Defines the */
                                                         /* number of credits */
                                                         /* in an SPPLE Buffer*/

   /* The following MACRO is used to convert an ASCII character into the*/
   /* equivalent decimal value.  The MACRO converts lower case          */
   /* characters to upper case before the conversion.                   */
#define ToInt(_x)                                  (((_x) > 0x39)?(((_x) & ~0x20)-0x37):((_x)-0x30))

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char         *strParam;
   unsigned int  intParam;
} Parameter_t;

   /* The following type definition represents the structure which holds*/
   /* a list of parameters that are to be associated with a command The */
   /* NumberofParameters variable holds the value of the number of      */
   /* parameters in the list.                                           */
typedef struct _tagParameterList_t
{
   int         NumberofParameters;
   Parameter_t Params[MAX_NUM_OF_PARAMETERS];
} ParameterList_t;

   /* The following type definition represents the structure which holds*/
   /* the command and parameters to be executed.                        */
typedef struct _tagUserCommand_t
{
   char            *Command;
   ParameterList_t  Parameters;
} UserCommand_t;

   /* The following type definition represents the generic function     */
   /* pointer to be used by all commands that can be executed by the    */
   /* test program.                                                     */
typedef int (*CommandFunction_t)(ParameterList_t *TempParam);

   /* The following type definition represents the structure which holds*/
   /* information used in the interpretation and execution of Commands. */
typedef struct _tagCommandTable_t
{
   char              *CommandName;
   CommandFunction_t  CommandFunction;
} CommandTable_t;

   /* The following structure holds status information about a send     */
   /* process.                                                          */
typedef struct _tagSend_Info_t
{
   DWord_t BytesToSend;
   DWord_t BytesSent;
} Send_Info_t;

   /* The following defines the format of a SPPLE Data Buffer.          */
typedef struct __tagSPPLE_Data_Buffer_t
{
   unsigned int  InIndex;
   unsigned int  OutIndex;
   unsigned int  BytesFree;
   unsigned int  BufferSize;
   Byte_t        Buffer[SPPLE_DATA_BUFFER_LENGTH*3];
} SPPLE_Data_Buffer_t;

   /* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices. For slave we will not use this */
   /* structure.                                                        */
typedef struct _tagDeviceInfoInfo_t
{
   Byte_t                       Flags;
   BD_ADDR_t                    ConnectionBD_ADDR;
   Boolean_t                    SupportCredits;
   unsigned int                 TransmitCredits;
   unsigned int                 TransmitHandle;
   unsigned int                 TransmitCCDHandle;
   unsigned int                 ReceiveHandle;
   unsigned int                 TxCreditsHandle;
   unsigned int                 RxCreditsHandle;
   unsigned int                 RxCreditsCCDHandle;
   SPPLE_Data_Buffer_t          ReceiveBuffer;
   SPPLE_Data_Buffer_t          TransmitBuffer;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static Boolean_t           Initialized;             /* Variable which is used to hold  */
                                                    /* the current state of the        */
                                                    /* Bluetopia Platform Manager      */
                                                    /* Initialization.                 */

static unsigned int        DEVMCallbackID;          /* Variable which holds the        */
                                                    /* Callback ID of the currently    */
                                                    /* registered Device Manager       */
                                                    /* Callback ID.                    */

static unsigned int        GATMCallbackID;          /* Variable which holds the        */
                                                    /* Callback ID of the currently    */
                                                    /* registered Generic Attribute    */
                                                    /* Profile Manager Event Callback. */

static unsigned int        AuthenticationCallbackID;/* Variable which holds the        */
                                                    /* current Authentication Callback */
                                                    /* ID that is assigned from the    */
                                                    /* Device Manager when the local   */
                                                    /* client registers for            */
                                                    /* Authentication.                 */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static Boolean_t           CurrentLowEnergy;        /* Variable which holds the        */
                                                    /* current LE state of the device  */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static GAP_IO_Capability_t IOCapability;            /* Variable which holds the        */
                                                    /* current I/O Capabilities that   */
                                                    /* are to be used for Secure Simple*/
                                                    /* Pairing.                        */

static Boolean_t           OOBSupport;              /* Variable which flags whether    */
                                                    /* or not Out of Band Secure Simple*/
                                                    /* Pairing exchange is supported.  */

static Boolean_t           MITMProtection;          /* Variable which flags whether or */
                                                    /* not Man in the Middle (MITM)    */
                                                    /* protection is to be requested   */
                                                    /* during a Secure Simple Pairing  */
                                                    /* procedure.                      */
static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static BD_ADDR_t           ConnectionBD_ADDR;       /* Holds the BD_ADDR of the        */
                                                    /* currently connected device.     */

static Boolean_t           Connected;               /* Holds the Connection ID of the  */
                                                    /* currently connected device.     */

static DeviceInfo_t        DeviceInfo;              /* Holds state information for an  */
                                                    /* active connection.              */

Mutex_t                    DeviceInfoLock;          /* Lock to protect the DeviceInfo  */
                                                    /* state.                          */

static Send_Info_t         SendInfo;                /* Variable that contains          */
                                                    /* information about a data        */
                                                    /* transfer process.               */

static Byte_t              SPPLEBuffer[SPPLE_DATA_BUFFER_LENGTH+1];  /* Buffer that is */
                                                    /* used for Sending/Receiving      */
                                                    /* SPPLE Service Data.             */

static Boolean_t           LoopbackActive;          /* Variable which flags whether or */
                                                    /* not the application is currently*/
                                                    /* operating in Loopback Mode      */
                                                    /* (TRUE) or not (FALSE).          */

static Boolean_t           DisplayRawData;          /* Variable which flags whether or */
                                                    /* not the application is to       */
                                                    /* simply display the Raw Data     */
                                                    /* when it is received (when not   */
                                                    /* operating in Loopback Mode).    */

static Boolean_t           AutomaticReadActive;     /* Variable which flags whether or */
                                                    /* not the application is to       */
                                                    /* automatically read all data     */
                                                    /* as it is received.              */

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
};

   /* The following defines a data sequence that will be used to        */
   /* generate message data.                                            */
static char  DataStr[]  = "~!@#$%^&*()_+`1234567890-=:;\"'<>?,./@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]`abcdefghijklmnopqrstuvwxyz{|}<>\r\n";
static int   DataStrLen = (sizeof(DataStr)-1);

   /* The following is used to map from ATT Error Codes to a printable  */
   /* string.                                                           */
static char *ErrorCodeStr[] = {
   "No Error",
   "Invalid Handle",
   "Read not permitted",
   "Write not permitted",
   "Invalid PDU",
   "Insufficient authentication",
   "Request not supported",
   "Invalid offset",
   "Insufficient authorization",
   "Prepare queue full",
   "Attribute not found",
   "Attribute not long",
   "Insufficient encryption key size",
   "Invalid attribute value length",
   "Unlikely error",
   "Insufficient encryption",
   "Unsupported group type",
   "Insufficient resources",
};

#define NUMBER_OF_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

   /* Internal function prototypes.                                     */
static void UserInterface(void);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *UserInput);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address);
static void StrToUUIDEntry(char *UUIDStr, SDP_UUID_Entry_t *UUIDEntry);
static void FormatAdvertisingData(DEVM_Advertising_Information_t *AdvertisingInformation, Byte_t *AdvertisingBuffer);

static unsigned int AddDataToBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int DataLength, Byte_t *Data);
static unsigned int RemoveDataFromBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int BufferLength, Byte_t *Buffer);
static void InitializeBuffer(SPPLE_Data_Buffer_t *DataBuffer);

static int EnableDisableNotificationsIndications(unsigned int ClientConfigurationHandle, Word_t ClientConfigurationValue);

static unsigned int FillBufferWithString(SPPLE_Data_Buffer_t *DataBuffer, unsigned *CurrentBufferLength, unsigned int MaxLength, Byte_t *Buffer);

static void SendProcess(void);
static void SendCredits(unsigned int DataLength);
static void ReceiveCreditEvent(unsigned int Credits);
static Boolean_t SendData(unsigned int DataLength, Byte_t *Data);
static void DataIndicationEvent(unsigned int DataLength, Byte_t *Data);
static int ReadData(unsigned int BufferLength, Byte_t *Buffer);

static int DisplayHelp(ParameterList_t *TempParam);

static int Initialize(ParameterList_t *TempParam);
static int Cleanup(ParameterList_t *TempParam);
static int RegisterEventCallback(ParameterList_t *TempParam);
static int UnRegisterEventCallback(ParameterList_t *TempParam);
static int SetDevicePower(ParameterList_t *TempParam);
static int QueryDevicePower(ParameterList_t *TempParam);
static int SetLocalRemoteDebugZoneMask(ParameterList_t *TempParam);
static int QueryLocalRemoteDebugZoneMask(ParameterList_t *TempParam);
static int ShutdownService(ParameterList_t *TempParam);
static int QueryLocalDeviceProperties(ParameterList_t *TempParam);
static int SetLocalDeviceName(ParameterList_t *TempParam);
static int SetLocalDeviceAppearance(ParameterList_t *TempParam);
static int SetLocalClassOfDevice(ParameterList_t *TempParam);
static int SetDiscoverable(ParameterList_t *TempParam);
static int SetConnectable(ParameterList_t *TempParam);
static int SetPairable(ParameterList_t *TempParam);
static int StartDeviceDiscovery(ParameterList_t *TempParam);
static int StopDeviceDiscovery(ParameterList_t *TempParam);
static int StartObservationScan(ParameterList_t *TempParam);
static int StopObservationScan(ParameterList_t *TempParam);
static int QueryRemoteDeviceList(ParameterList_t *TempParam);
static int QueryRemoteDeviceProperties(ParameterList_t *TempParam);
static int AddRemoteDevice(ParameterList_t *TempParam);
static int DeleteRemoteDevice(ParameterList_t *TempParam);
static int UpdateRemoteDeviceApplicationData(ParameterList_t *TempParam);
static int DeleteRemoteDevices(ParameterList_t *TempParam);
static int PairWithRemoteDevice(ParameterList_t *TempParam);
static int CancelPairWithRemoteDevice(ParameterList_t *TempParam);
static int UnPairRemoteDevice(ParameterList_t *TempParam);
static int QueryRemoteDeviceServices(ParameterList_t *TempParam);
static int QueryRemoteDeviceServiceSupported(ParameterList_t *TempParam);
static int QueryRemoteDevicesForService(ParameterList_t *TempParam);
static int QueryRemoteDeviceServiceClasses(ParameterList_t *TempParam);
static int AuthenticateRemoteDevice(ParameterList_t *TempParam);
static int EncryptRemoteDevice(ParameterList_t *TempParam);
static int ConnectWithRemoteDevice(ParameterList_t *TempParam);
static int DisconnectRemoteDevice(ParameterList_t *TempParam);
static int SetRemoteDeviceLinkActive(ParameterList_t *TempParam);
static int CreateSDPRecord(ParameterList_t *TempParam);
static int DeleteSDPRecord(ParameterList_t *TempParam);
static int AddSDPAttribute(ParameterList_t *TempParam);
static int DeleteSDPAttribute(ParameterList_t *TempParam);
static int EnableBluetoothDebug(ParameterList_t *TempParam);
static int RegisterAuthentication(ParameterList_t *TempParam);
static int UnRegisterAuthentication(ParameterList_t *TempParam);

static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);

static int RegisterGATMEventCallback(ParameterList_t *TempParam);
static int UnRegisterGATMEventCallback(ParameterList_t *TempParam);

static int GATTQueryConnectedDevices(ParameterList_t *TempParam);

static int ConfigureSPPLE(ParameterList_t *TempParam);
static int SendDataCommand(ParameterList_t *TempParam);
static int ReadDataCommand(ParameterList_t *TempParam);

static int Loopback(ParameterList_t *TempParam);
static int DisplayRawModeData(ParameterList_t *TempParam);
static int AutomaticReadMode(ParameterList_t *TempParam);

static int StartAdvertising(ParameterList_t *TempParam);
static int StopAdvertising(ParameterList_t *TempParam);

static void DisplayGATTUUID(GATT_UUID_t *UUID, char *Prefix, unsigned int Level);
static void DisplayParsedGATTServiceData(DEVM_Parsed_Services_Data_t *ParsedGATTData);
static void DisplayParsedSDPServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData);
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

static void DisplayLocalDeviceProperties(unsigned long UpdateMask, DEVM_Local_Device_Properties_t *LocalDeviceProperties);
static void DisplayRemoteDeviceProperties(unsigned long UpdateMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

   /* BTPM Server Un-Registration Callback function prototype.          */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter);

   /* BTPM Local Device Manager Callback function prototype.            */
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter);

   /* BTPM Local Device Manager Authentication Callback function        */
   /* prototype.                                                        */
static void BTPSAPI DEVM_Authentication_Callback(DEVM_Authentication_Information_t *AuthenticationRequestInformation, void *CallbackParameter);

   /* GATM Manager Callback Function prototype.                         */
static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface(void)
{
   UserCommand_t TempCommand;
   int  Result = !EXIT_CODE;
   char UserInput[MAX_COMMAND_LENGTH];

   /* First let's make sure that we start on new line.                  */
   printf("\r\n");

   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("INITIALIZE", Initialize);
   AddCommand("CLEANUP", Cleanup);
   AddCommand("QUERYDEBUGZONEMASK", QueryLocalRemoteDebugZoneMask);
   AddCommand("SETDEBUGZONEMASK", SetLocalRemoteDebugZoneMask);
   AddCommand("SHUTDOWNSERVICE", ShutdownService);
   AddCommand("REGISTEREVENTCALLBACK", RegisterEventCallback);
   AddCommand("UNREGISTEREVENTCALLBACK", UnRegisterEventCallback);
   AddCommand("QUERYDEVICEPOWER", QueryDevicePower);
   AddCommand("SETDEVICEPOWER", SetDevicePower);
   AddCommand("QUERYLOCALDEVICEPROPERTIES", QueryLocalDeviceProperties);
   AddCommand("SETLOCALDEVICENAME", SetLocalDeviceName);
   AddCommand("SETLOCALCLASSOFDEVICE", SetLocalClassOfDevice);
   AddCommand("SETDISCOVERABLE", SetDiscoverable);
   AddCommand("SETCONNECTABLE", SetConnectable);
   AddCommand("SETPAIRABLE", SetPairable);
   AddCommand("STARTDEVICEDISCOVERY", StartDeviceDiscovery);
   AddCommand("STOPDEVICEDISCOVERY", StopDeviceDiscovery);
   AddCommand("QUERYREMOTEDEVICELIST", QueryRemoteDeviceList);
   AddCommand("QUERYREMOTEDEVICEPROPERTIES", QueryRemoteDeviceProperties);
   AddCommand("ADDREMOTEDEVICE", AddRemoteDevice);
   AddCommand("DELETEREMOTEDEVICE", DeleteRemoteDevice);
   AddCommand("UPDATEREMOTEDEVICEAPPDATA", UpdateRemoteDeviceApplicationData);
   AddCommand("DELETEREMOTEDEVICES", DeleteRemoteDevices);
   AddCommand("PAIRWITHREMOTEDEVICE", PairWithRemoteDevice);
   AddCommand("CANCELPAIRWITHREMOTEDEVICE", CancelPairWithRemoteDevice);
   AddCommand("UNPAIRREMOTEDEVICE", UnPairRemoteDevice);
   AddCommand("QUERYREMOTEDEVICESERVICES", QueryRemoteDeviceServices);
   AddCommand("QUERYREMOTEDEVICESERVICESUPPORTED", QueryRemoteDeviceServiceSupported);
   AddCommand("QUERYREMOTEDEVICESFORSERVICE", QueryRemoteDevicesForService);
   AddCommand("QUERYREMOTEDEVICESERVICECLASSES", QueryRemoteDeviceServiceClasses);
   AddCommand("AUTHENTICATEREMOTEDEVICE", AuthenticateRemoteDevice);
   AddCommand("ENCRYPTREMOTEDEVICE", EncryptRemoteDevice);
   AddCommand("CONNECTWITHREMOTEDEVICE", ConnectWithRemoteDevice);
   AddCommand("DISCONNECTREMOTEDEVICE", DisconnectRemoteDevice);
   AddCommand("SETREMOTEDEVICELINKACTIVE", SetRemoteDeviceLinkActive);
   AddCommand("CREATESDPRECORD", CreateSDPRecord);
   AddCommand("DELETESDPRECORD", DeleteSDPRecord);
   AddCommand("ADDSDPATTRIBUTE", AddSDPAttribute);
   AddCommand("DELETESDPATTRIBUTE", DeleteSDPAttribute);
   AddCommand("ENABLEBLUETOOTHDEBUG", EnableBluetoothDebug);
   AddCommand("REGISTERAUTHENTICATION", RegisterAuthentication);
   AddCommand("UNREGISTERAUTHENTICATION", UnRegisterAuthentication);
   AddCommand("PINCODERESPONSE", PINCodeResponse);
   AddCommand("PASSKEYRESPONSE", PassKeyResponse);
   AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
   AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
   AddCommand("REGISTERGATTCALLBACK", RegisterGATMEventCallback);
   AddCommand("UNREGISTERGATTCALLBACK", UnRegisterGATMEventCallback);
   AddCommand("QUERYGATTCONNECTIONS", GATTQueryConnectedDevices);
   AddCommand("SETLOCALDEVICEAPPEARANCE", SetLocalDeviceAppearance);
   AddCommand("CONFIGURESPPLE", ConfigureSPPLE);
   AddCommand("SEND", SendDataCommand);
   AddCommand("READ", ReadDataCommand);
   AddCommand("LOOPBACK", Loopback);
   AddCommand("DISPLAYRAWMODEDATA", DisplayRawModeData);
   AddCommand("AUTOMATICREADMODE", AutomaticReadMode);
   AddCommand("STARTADVERTISING", StartAdvertising);
   AddCommand("STOPADVERTISING", StopAdvertising);
   AddCommand("STARTOBSERVATIONSCAN", StartObservationScan);
   AddCommand("STOPOBSERVATIONSCAN", StopObservationScan);

   AddCommand("HELP", DisplayHelp);

   /* This is the main loop of the program.  It gets user input from the*/
   /* command window, make a call to the command parser, and command    */
   /* interpreter.  After the function has been ran it then check the   */
   /* return value and displays an error message when appropriate. If   */
   /* the result returned is ever the EXIT_CODE the loop will exit      */
   /* leading the the exit of the program.                              */
   while(Result != EXIT_CODE)
   {
      /* Initialize the value of the variable used to store the users   */
      /* input and output "Input: " to the command window to inform the */
      /* user that another command may be entered.                      */
      UserInput[0] = '\0';

      /* Output an Input Shell-type prompt.                             */
      printf("SPPMLE>");

      /* Retrieve the command entered by the user and store it in the   */
      /* User Input Buffer.  Note that this command will fail if the    */
      /* application receives a signal which cause the standard file    */
      /* streams to be closed.  If this happens the loop will be broken */
      /* out of so the application can exit.                            */
      if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
      {
         /* Start a newline for the results.                            */
         printf("\r\n");

         /* Next, check to see if a command was input by the user.      */
         if(strlen(UserInput))
         {
            /* The string input by the user contains a value, now run   */
            /* the string through the Command Parser.                   */
            if(CommandParser(&TempCommand, UserInput) >= 0)
            {
               /* The Command was successfully parsed, run the Command. */
               Result = CommandInterpreter(&TempCommand);

               switch(Result)
               {
                  case INVALID_COMMAND_ERROR:
                     printf("Invalid Command.\r\n");
                     break;
                  case FUNCTION_ERROR:
                     printf("Function Error.\r\n");
                     break;
               }
            }
            else
               printf("Invalid Input.\r\n");
         }
      }
      else
         Result = EXIT_CODE;
   }
}

   /* The following function is responsible for converting number       */
   /* strings to there unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned int StringToUnsignedInteger(char *StringInteger)
{
   int          IsHex;
   unsigned int Index;
   unsigned int ret_val = 0;

   /* Before proceeding make sure that the parameter that was passed as */
   /* an input appears to be at least semi-valid.                       */
   if((StringInteger) && (strlen(StringInteger)))
   {
      /* Initialize the variable.                                       */
      Index = 0;

      /* Next check to see if this is a hexadecimal number.             */
      if(strlen(StringInteger) > 2)
      {
         if((StringInteger[0] == '0') && ((StringInteger[1] == 'x') || (StringInteger[1] == 'X')))
         {
            IsHex = 1;

            /* Increment the String passed the Hexadecimal prefix.      */
            StringInteger += 2;
         }
         else
            IsHex = 0;
      }
      else
         IsHex = 0;

      /* Process the value differently depending on whether or not a    */
      /* Hexadecimal Number has been specified.                         */
      if(!IsHex)
      {
         /* Decimal Number has been specified.                          */
         while(1)
         {
            /* First check to make sure that this is a valid decimal    */
            /* digit.                                                   */
            if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               ret_val += (StringInteger[Index] & 0xF);

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 10.                                */
                  ret_val *= 10;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
      else
      {
         /* Hexadecimal Number has been specified.                      */
         while(1)
         {
            /* First check to make sure that this is a valid Hexadecimal*/
            /* digit.                                                   */
            if(((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9')) || ((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f')) || ((StringInteger[Index] >= 'A') && (StringInteger[Index] <= 'F')))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
                  ret_val += (StringInteger[Index] & 0xF);
               else
               {
                  if((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f'))
                     ret_val += (StringInteger[Index] - 'a' + 10);
                  else
                     ret_val += (StringInteger[Index] - 'A' + 10);
               }

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 16.                                */
                  ret_val *= 16;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for parsing strings into    */
   /* components.  The first parameter of this function is a pointer to */
   /* the String to be parsed.  This function will return the start of  */
   /* the string upon success and a NULL pointer on all errors.         */
static char *StringParser(char *String)
{
   int   Index;
   char *ret_val = NULL;

   /* Before proceeding make sure that the string passed in appears to  */
   /* be at least semi-valid.                                           */
   if((String) && (strlen(String)))
   {
      /* The string appears to be at least semi-valid.  Search for the  */
      /* first space character and replace it with a NULL terminating   */
      /* character.                                                     */
      for(Index=0,ret_val=String;Index < strlen(String);Index++)
      {
         /* Is this the space character.                                */
         if((String[Index] == ' ') || (String[Index] == '\r') || (String[Index] == '\n'))
         {
            /* This is the space character, replace it with a NULL      */
            /* terminating character and set the return value to the    */
            /* begining character of the string.                        */
            String[Index] = '\0';
            break;
         }
      }
   }

   return(ret_val);
}

   /* This function is responsable for taking command strings and       */
   /* that is parsed into the UserCommand structure.  Successful        */
   /* execution of this function is denoted by a retrun value of zero.  */
   /* Negative return values denote an error in the parsing of the      */
   /* string parameter.                                                 */
static int CommandParser(UserCommand_t *TempCommand, char *UserInput)
{
   int            ret_val;
   int            StringLength;
   char          *LastParameter;
   unsigned int   Count         = 0;

   /* Before proceeding make sure that the passed parameters appear to  */
   /* be at least semi-valid.                                           */
   if((TempCommand) && (UserInput) && (strlen(UserInput)))
   {
      /* Retrieve the first token in the string, this should be the     */
      /* commmand.                                                      */
      TempCommand->Command = StringParser(UserInput);

      /* Flag that there are NO Parameters for this Command Parse.      */
      TempCommand->Parameters.NumberofParameters = 0;

      /* Check to see if there is a Command.                            */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val = 0;

         /* Adjust the UserInput pointer and StringLength to remove     */
         /* the Command from the data passed in before parsing the      */
         /* parameters.                                                 */
         UserInput    += strlen(TempCommand->Command)+1;
         StringLength  = strlen(UserInput);

         /* There was an available command, now parse out the parameters*/
         while((StringLength > 0) && ((LastParameter = StringParser(UserInput)) != NULL))
         {
            /* There is an available parameter, now check to see if     */
            /* there is room in the UserCommand to store the parameter  */
            if(Count < (sizeof(TempCommand->Parameters.Params)/sizeof(Parameter_t)))
            {
               /* Save the parameter as a string.                       */
               TempCommand->Parameters.Params[Count].strParam = LastParameter;

               /* Save the parameter as an unsigned int intParam will   */
               /* have a value of zero if an error has occurred.        */
               TempCommand->Parameters.Params[Count].intParam = StringToUnsignedInteger(LastParameter);

               Count++;
               UserInput    += strlen(LastParameter)+1;
               StringLength -= strlen(LastParameter)+1;

               ret_val = 0;
            }
            else
            {
               /* Be sure we exit out of the Loop.                      */
               StringLength = 0;

               ret_val      = TO_MANY_PARAMS;
            }
         }

         /* Set the number of parameters in the User Command to the     */
         /* number of found parameters                                  */
         TempCommand->Parameters.NumberofParameters = Count;
      }
      else
      {
         /* No command was specified                                    */
         ret_val = NO_COMMAND_ERROR;
      }
   }
   else
   {
      /* One or more of the passed parameters appear to be invalid.     */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* This function is responsible for determining the command in which */
   /* the user entered and running the appropriate function associated  */
   /* with that command.  The first parameter of this function is a     */
   /* structure containing information about the commmand to be issued. */
   /* This information includes the command name and multiple parameters*/
   /* which maybe be passed to the function to be executed.  Successful */
   /* execution of this function is denoted by a return value of zero.  */
   /* A negative return value implies that that command was not found   */
   /* and is invalid.                                                   */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               i;
   int               ret_val;
   CommandFunction_t CommandFunction;

   /* If the command is not found in the table return with an invaild   */
   /* command error                                                     */
   ret_val = INVALID_COMMAND_ERROR;

   /* Let's make sure that the data passed to us appears semi-valid.    */
   if((TempCommand) && (TempCommand->Command))
   {
      /* Now, let's make the Command string all upper case so that we   */
      /* compare against it.                                            */
      for(i=0;i<strlen(TempCommand->Command);i++)
      {
         if((TempCommand->Command[i] >= 'a') && (TempCommand->Command[i] <= 'z'))
            TempCommand->Command[i] -= ('a' - 'A');
      }

      /* Check to see if the command which was entered was exit.        */
      if(memcmp(TempCommand->Command, "QUIT", strlen("QUIT")) != 0)
      {
         /* The command entered is not exit so search for command in    */
         /* table.                                                      */
         if((CommandFunction = FindCommand(TempCommand->Command)) != NULL)
         {
            /* The command was found in the table so call the command.  */
            if(!((*CommandFunction)(&TempCommand->Parameters)))
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
               ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* The command entered is exit, set return value to EXIT_CODE  */
         /* and return.                                                 */
         ret_val = EXIT_CODE;
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

   /* The following function is provided to allow a means to            */
   /* programatically add Commands the Global (to this module) Command  */
   /* Table.  The Command Table is simply a mapping of Command Name     */
   /* (NULL terminated ASCII string) to a command function.  This       */
   /* function returns zero if successful, or a non-zero value if the   */
   /* command could not be added to the list.                           */
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction)
{
   int ret_val;

   /* First, make sure that the parameters passed to us appear to be    */
   /* semi-valid.                                                       */
   if((CommandName) && (CommandFunction))
   {
      /* Next, make sure that we still have room in the Command Table   */
      /* to add commands.                                               */
      if(NumberCommands < MAX_SUPPORTED_COMMANDS)
      {
         /* Simply add the command data to the command table and        */
         /* increment the number of supported commands.                 */
         CommandTable[NumberCommands].CommandName       = CommandName;
         CommandTable[NumberCommands++].CommandFunction = CommandFunction;

         /* Return success to the caller.                               */
         ret_val                                        = 0;
      }
      else
         ret_val = 1;
   }
   else
      ret_val = 1;

   return(ret_val);
}

   /* The following function searches the Command Table for the         */
   /* specified Command.  If the Command is found, this function returns*/
   /* a NON-NULL Command Function Pointer.  If the command is not found */
   /* this function returns NULL.                                       */
static CommandFunction_t FindCommand(char *Command)
{
   unsigned int      Index;
   CommandFunction_t ret_val;

   /* First, make sure that the command specified is semi-valid.        */
   if(Command)
   {
      /* Special shortcut: If it's simply a one or two digit number,    */
      /* convert the command directly based on the Command Index.       */
      if((strlen(Command) == 1) && (Command[0] >= '1') && (Command[0] <= '9'))
      {
         Index = atoi(Command);

         if(Index < NumberCommands)
            ret_val = CommandTable[Index - 1].CommandFunction;
         else
            ret_val = NULL;
      }
      else
      {
         if((strlen(Command) == 2) && (Command[0] >= '0') && (Command[0] <= '9') && (Command[1] >= '0') && (Command[1] <= '9'))
         {
            Index = atoi(Command);

            if(Index < NumberCommands)
               ret_val = CommandTable[Index?(Index-1):Index].CommandFunction;
            else
               ret_val = NULL;
         }
         else
         {
            /* Now loop through each element in the table to see if     */
            /* there is a match.                                        */
            for(Index=0,ret_val=NULL;((Index<NumberCommands) && (!ret_val));Index++)
            {
               if((strlen(Command) == strlen(CommandTable[Index].CommandName)) && (memcmp(Command, CommandTable[Index].CommandName, strlen(CommandTable[Index].CommandName)) == 0))
                  ret_val = CommandTable[Index].CommandFunction;
            }
         }
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is provided to allow a means to clear out  */
   /* all available commands from the command table.                    */
static void ClearCommands(void)
{
   /* Simply flag that there are no commands present in the table.      */
   NumberCommands = 0;
}

   /* The following function is responsible for converting data of type */
   /* BD_ADDR to a string.  The first parameter of this function is the */
   /* BD_ADDR to be converted to a string.  The second parameter of this*/
   /* function is a pointer to the string in which the converted BD_ADDR*/
   /* is to be stored.                                                  */
static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr)
{
   sprintf(BoardStr, "%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5,
                                                 Board_Address.BD_ADDR4,
                                                 Board_Address.BD_ADDR3,
                                                 Board_Address.BD_ADDR2,
                                                 Board_Address.BD_ADDR1,
                                                 Board_Address.BD_ADDR0);
}

   /* The following function is responsible for the specified string    */
   /* into data of type BD_ADDR.  The first parameter of this function  */
   /* is the BD_ADDR string to be converted to a BD_ADDR.  The second   */
   /* parameter of this function is a pointer to the BD_ADDR in which   */
   /* the converted BD_ADDR String is to be stored.                     */
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address)
{
   unsigned int Address[sizeof(BD_ADDR_t)];

   if((BoardStr) && (strlen(BoardStr) == sizeof(BD_ADDR_t)*2) && (Board_Address))
   {
      sscanf(BoardStr, "%02X%02X%02X%02X%02X%02X", &(Address[5]), &(Address[4]), &(Address[3]), &(Address[2]), &(Address[1]), &(Address[0]));

      Board_Address->BD_ADDR5 = (Byte_t)Address[5];
      Board_Address->BD_ADDR4 = (Byte_t)Address[4];
      Board_Address->BD_ADDR3 = (Byte_t)Address[3];
      Board_Address->BD_ADDR2 = (Byte_t)Address[2];
      Board_Address->BD_ADDR1 = (Byte_t)Address[1];
      Board_Address->BD_ADDR0 = (Byte_t)Address[0];
   }
   else
   {
      if(Board_Address)
         BTPS_MemInitialize(Board_Address, 0, sizeof(BD_ADDR_t));
   }
}

   /* The following function is responsible for converting the specified*/
   /* string into data of type SDP_UUID_Entry_t. The first parameter    */
   /* of this function is the UUID string to be converted to an SDP     */
   /* UUID Entry. The second parameter of this function is a pointer to */
   /* the SDP_UUID_Entry_t in which the converted UUID String is to be  */
   /* stored.                                                           */
static void StrToUUIDEntry(char *UUIDStr, SDP_UUID_Entry_t *UUIDEntry)
{
   Byte_t       *UUIDBuffer;
   unsigned int  RemainingDigits;

   if((UUIDStr) && (UUIDEntry))
   {
      BTPS_MemInitialize(UUIDEntry, 0, sizeof(SDP_UUID_Entry_t));

      /* We always treat UUIDs as hex, so skip any "hex format" prefix. */
      if((strlen(UUIDStr) >= 2) && (UUIDStr[0] == '0') && ((UUIDStr[1] == 'x') || (UUIDStr[1] == 'X')))
         UUIDStr += 2;

      switch(strlen(UUIDStr))
      {
         case 4:
            UUIDEntry->SDP_Data_Element_Type = deUUID_16;
            UUIDBuffer                       = (Byte_t *)(&UUIDEntry->UUID_Value.UUID_16);
            RemainingDigits                  = 4;
            break;
         case 8:
            UUIDEntry->SDP_Data_Element_Type = deUUID_32;
            UUIDBuffer                       = (Byte_t *)(&UUIDEntry->UUID_Value.UUID_32);
            RemainingDigits                  = 8;
            break;
         case 32:
         case 36:
            UUIDEntry->SDP_Data_Element_Type = deUUID_128;
            UUIDBuffer                       = (Byte_t *)(&UUIDEntry->UUID_Value.UUID_128);
            RemainingDigits                  = 32;
            break;
         default:
            UUIDEntry->SDP_Data_Element_Type = deNULL;
            UUIDBuffer                       = NULL;
            RemainingDigits                  = 0;
            break;
      }

      while((UUIDStr) && (UUIDBuffer) && (*UUIDStr != '\0') && (RemainingDigits > 0))
      {
         if((*UUIDStr == '-') && (UUIDEntry->SDP_Data_Element_Type == deUUID_128))
         {
            /* Skip the dashes that appear in full-length UUID values.  */
            UUIDStr++;
         }
         else
         {
            if((*UUIDStr >= '0') && (*UUIDStr <= '9'))
            {
               *UUIDBuffer += (*UUIDStr - '0');
               RemainingDigits--;
            }
            else
            {
               if((*UUIDStr >= 'a') && (*UUIDStr <= 'f'))
               {
                  *UUIDBuffer += (*UUIDStr - 'a' + 10);
                  RemainingDigits--;
               }
               else
               {
                  if((*UUIDStr >= 'A') && (*UUIDStr <= 'F'))
                  {
                     *UUIDBuffer += (*UUIDStr - 'A' + 10);
                     RemainingDigits--;
                  }
                  else
                  {
                     /* Invalid hex character. Abort the conversion.    */
                     UUIDEntry->SDP_Data_Element_Type = deNULL;
                  }
               }
            }

            UUIDStr++;

            if((RemainingDigits % 2) == 0)
            {
               /* A full byte was just completed. Move to the next byte */
               /* in the UUID.                                          */
               UUIDBuffer++;
            }
            else
            {
               /* The first half of a byte was processed. Shift the new */
               /* nibble into the upper-half of the byte.               */
               *UUIDBuffer = (*UUIDBuffer << 4);
            }
         }
      }
   }
}

   /* The following function is a utility function that is used to      */
   /* format the advertising data for this application in the           */
   /* advertising data.                                                 */
static void FormatAdvertisingData(DEVM_Advertising_Information_t *AdvertisingInformation, Byte_t *AdvertisingBuffer)
{
   Byte_t Length;

   /* Verify that the input parameters are semi-valid.                  */
   if((AdvertisingInformation) && (AdvertisingBuffer))
   {
      /* Format the data.                                               */
      Length = (UUID_128_SIZE + NON_ALIGNED_BYTE_SIZE);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(AdvertisingBuffer[0]), Length);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(AdvertisingBuffer[NON_ALIGNED_BYTE_SIZE]), HCI_LE_ADVERTISING_REPORT_DATA_TYPE_SERVICE_SOLICITATION_128_BIT_SERVICE_UUID);
      SPPLE_ASSIGN_SPPLE_SERVICE_UUID_128(&(AdvertisingBuffer[NON_ALIGNED_WORD_SIZE]));

      /* Format the structure.                                          */
      AdvertisingInformation->AdvertisingDataLength = (Length + NON_ALIGNED_BYTE_SIZE);
      AdvertisingInformation->AdvertisingData       = AdvertisingBuffer;
   }
}

   /* The following function is a utility function that is used to add  */
   /* data (using InIndex as the buffer index) from the buffer specified*/
   /* by the DataBuffer parameter.  The second and third parameters     */
   /* specified the length of the data to add and the pointer to the    */
   /* data to add to the buffer.  This function returns the actual      */
   /* number of bytes that were added to the buffer (or 0 if none were  */
   /* added).                                                           */
static unsigned int AddDataToBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int DataLength, Byte_t *Data)
{
   unsigned int BytesAdded = 0;
   unsigned int Count;

   /* Verify that the input parameters are valid.                       */
   if((DataBuffer) && (DataLength) && (Data))
   {
      /* Loop while we have data AND space in the buffer.               */
      while(DataLength)
      {
         /* Get the number of bytes that can be placed in the buffer    */
         /* until it wraps.                                             */
         Count = DataBuffer->BufferSize - DataBuffer->InIndex;

         /* Determine if the number of bytes free is less than the      */
         /* number of bytes till we wrap and choose the smaller of the  */
         /* numbers.                                                    */
         Count = (DataBuffer->BytesFree < Count)?DataBuffer->BytesFree:Count;

         /* Cap the Count that we add to buffer to the length of the    */
         /* data provided by the caller.                                */
         Count = (Count > DataLength)?DataLength:Count;

         if(Count)
         {
            /* Copy the data into the buffer.                           */
            BTPS_MemCopy(&DataBuffer->Buffer[DataBuffer->InIndex], Data, Count);

            /* Update the counts.                                       */
            DataBuffer->InIndex   += Count;
            DataBuffer->BytesFree -= Count;
            DataLength            -= Count;
            BytesAdded            += Count;
            Data                  += Count;

            /* Wrap the InIndex if necessary.                           */
            if(DataBuffer->InIndex >= DataBuffer->BufferSize)
               DataBuffer->InIndex = 0;
         }
         else
            break;
      }
   }

   return(BytesAdded);
}

   /* The following function is a utility function that is used to      */
   /* removed data (using OutIndex as the buffer index) from the buffer */
   /* specified by the DataBuffer parameter The second parameter        */
   /* specifies the length of the Buffer that is pointed to by the third*/
   /* parameter.  This function returns the actual number of bytes that */
   /* were removed from the DataBuffer (or 0 if none were added).       */
   /* * NOTE * Buffer is optional and if not specified up to            */
   /*          BufferLength bytes will be deleted from the Buffer.      */
static unsigned int RemoveDataFromBuffer(SPPLE_Data_Buffer_t *DataBuffer, unsigned int BufferLength, Byte_t *Buffer)
{
   unsigned int Count;
   unsigned int BytesRemoved = 0;
   unsigned int MaxRemove;

   /* Verify that the input parameters are valid.                       */
   if((DataBuffer) && (BufferLength))
   {
      /* Loop while we have data to remove and space in the buffer to   */
      /* place it.                                                      */
      while(BufferLength)
      {
         /* Determine the number of bytes that are present in the       */
         /* buffer.                                                     */
         Count = DataBuffer->BufferSize - DataBuffer->BytesFree;
         if(Count)
         {
            /* Calculate the maximum number of bytes that I can remove  */
            /* from the buffer before it wraps.                         */
            MaxRemove = DataBuffer->BufferSize - DataBuffer->OutIndex;

            /* Cap max we can remove at the BufferLength of the caller's*/
            /* buffer.                                                  */
            MaxRemove = (MaxRemove > BufferLength)?BufferLength:MaxRemove;

            /* Cap the number of bytes I will remove in this iteration  */
            /* at the maximum I can remove or the number of bytes that  */
            /* are in the buffer.                                       */
            Count = (Count > MaxRemove)?MaxRemove:Count;

            /* Copy the data into the caller's buffer (If specified).   */
            if(Buffer)
            {
               BTPS_MemCopy(Buffer, &DataBuffer->Buffer[DataBuffer->OutIndex], Count);
               Buffer += Count;
            }

            /* Update the counts.                                       */
            DataBuffer->OutIndex  += Count;
            DataBuffer->BytesFree += Count;
            BytesRemoved          += Count;
            BufferLength          -= Count;

            /* Wrap the OutIndex if necessary.                          */
            if(DataBuffer->OutIndex >= DataBuffer->BufferSize)
               DataBuffer->OutIndex = 0;
         }
         else
            break;
      }
   }

   return(BytesRemoved);
}

   /* The following function is used to initialize the specified buffer */
   /* to the defaults.                                                  */
static void InitializeBuffer(SPPLE_Data_Buffer_t *DataBuffer)
{
   /* Verify that the input parameters are valid.                       */
   if(DataBuffer)
   {
      DataBuffer->BufferSize = SPPLE_DATA_CREDITS;
      DataBuffer->BytesFree  = SPPLE_DATA_CREDITS;
      DataBuffer->InIndex    = 0;
      DataBuffer->OutIndex   = 0;
   }
}

   /* The following function is used to enable/disable notifications on */
   /* a specified handle.  This function returns the positive non-zero  */
   /* Transaction ID of the Write Request or a negative error code.     */
static int EnableDisableNotificationsIndications(unsigned int ClientConfigurationHandle, Word_t ClientConfigurationValue)
{
   int              ret_val;
   NonAlignedWord_t Buffer;

   /* Verify the input parameters.                                      */
   if((Connected) && (ClientConfigurationHandle))
   {
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, ClientConfigurationValue);

      ret_val = GATM_WriteValueWithoutResponse(GATMCallbackID, ConnectionBD_ADDR, ClientConfigurationHandle, FALSE, sizeof(Buffer), (Byte_t *)&Buffer);
   }
   else
      ret_val = BTPS_ERROR_INVALID_PARAMETER;

   printf("EnableDisable Result: %d\n", ret_val);

   return(ret_val);
}

   /* The following function is a utility function that exists to fill  */
   /* the specified buffer with the DataStr that is used to send data.  */
   /* This function will fill from the CurrentBufferLength up to Max    */
   /* Length in Buffer.  CurrentBufferLength is used to return the total*/
   /* length of the buffer.  The first parameter specifies the          */
   /* DeviceInfo which is used to fill any remainder of the string so   */
   /* that there are no breaks in the pattern.  This function returns   */
   /* the number of bytes added to the transmit buffer of the specified */
   /* device.                                                           */
static unsigned int FillBufferWithString(SPPLE_Data_Buffer_t *DataBuffer, unsigned *CurrentBufferLength, unsigned int MaxLength, Byte_t *Buffer)
{
   unsigned int DataCount;
   unsigned int Length;
   unsigned int Added2Buffer = 0;

   /* Verify that the input parameter is semi-valid.                    */
   if((DataBuffer) && (CurrentBufferLength) && (MaxLength) && (Buffer))
   {
      /* Copy as much of the DataStr into the Transmit buffer as is     */
      /* possible.                                                      */
      while(*CurrentBufferLength < MaxLength)
      {
         /* Cap the data to copy at the maximum of the string length and*/
         /* the remaining amount that can be placed in the buffer.      */
         DataCount = (DataStrLen > (MaxLength-*CurrentBufferLength))?(MaxLength-*CurrentBufferLength):DataStrLen;

         /* Build the data string into the SPPLEBuffer.                 */
         BTPS_MemCopy(&Buffer[*CurrentBufferLength], DataStr, DataCount);

         /* Increment the index.                                        */
         *CurrentBufferLength += DataCount;

         /* Add whatever bytes remaining in the DataStr into the        */
         /* transmit buffer to keep the pattern consistent.             */
         Length = DataStrLen-DataCount;
         if(Length)
         {
            /* Add the bytes remaining in the string.                   */
            Added2Buffer += AddDataToBuffer(DataBuffer, Length, (Byte_t *)&DataStr[DataCount]);
         }
      }
   }

   return(Added2Buffer);
}

   /* The following function is responsible for handling a Send Process.*/
static void SendProcess()
{
   int          Result;
   Boolean_t    Done = FALSE;
   unsigned int TransmitIndex;
   unsigned int DataCount;
   unsigned int MaxLength;
   unsigned int SPPLEBufferLength;
   unsigned int Added2Buffer;

   /* Verify that the input parameter is semi-valid.                    */
   if(Connected)
   {
      /* Loop while we have data to send and we have not used up all    */
      /* Transmit Credits.                                              */
      TransmitIndex     = 0;
      SPPLEBufferLength = 0;
      Added2Buffer      = 0;

      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         while((SendInfo.BytesToSend) && ((!DeviceInfo.SupportCredits) || ((DeviceInfo.SupportCredits) && (DeviceInfo.TransmitCredits))) && (!Done))
         {
            /* Get the maximum length of what we can send in this       */
            /* transaction.                                             */
            if(DeviceInfo.SupportCredits)
               MaxLength = (SendInfo.BytesToSend > DeviceInfo.TransmitCredits)?DeviceInfo.TransmitCredits:SendInfo.BytesToSend;
            else
               MaxLength = SendInfo.BytesToSend;

            MaxLength = (MaxLength > SPPLE_DATA_BUFFER_LENGTH)?SPPLE_DATA_BUFFER_LENGTH:MaxLength;

            /* If we do not have any outstanding data get some more     */
            /* data.                                                    */
            if(!SPPLEBufferLength)
            {
               /* Send any buffered data first.                         */
               if(DeviceInfo.TransmitBuffer.BytesFree != DeviceInfo.TransmitBuffer.BufferSize)
               {
                  /* Remove the queued data from the Transmit Buffer.   */
                  SPPLEBufferLength = RemoveDataFromBuffer(&(DeviceInfo.TransmitBuffer), MaxLength, SPPLEBuffer);

                  /* If we added some data to the transmit buffer       */
                  /* decrement what we just removed.                    */
                  if(Added2Buffer)
                     Added2Buffer -= SPPLEBufferLength;
               }

               /* Fill up the rest of the buffer with the data string.  */
               Added2Buffer     += FillBufferWithString(&(DeviceInfo.TransmitBuffer), &SPPLEBufferLength, MaxLength, SPPLEBuffer);

               /* Set the count of data that we can send.               */
               DataCount         = SPPLEBufferLength;

               /* Reset the Transmit Index to 0.                        */
               TransmitIndex     = 0;
            }
            else
            {
               /* Move the data that to the beginning of the buffer.    */
               BTPS_MemMove(SPPLEBuffer, &SPPLEBuffer[TransmitIndex], SPPLEBufferLength);

               /* Send any buffered data first.                         */
               if(DeviceInfo.TransmitBuffer.BytesFree != DeviceInfo.TransmitBuffer.BufferSize)
               {
                  /* Remove the queued data from the Transmit Buffer.   */
                  TransmitIndex = RemoveDataFromBuffer(&(DeviceInfo.TransmitBuffer), MaxLength-SPPLEBufferLength, &SPPLEBuffer[SPPLEBufferLength]);

                  /* If we added some data to the transmit buffer       */
                  /* decrement what we just removed.                    */
                  if(Added2Buffer)
                     Added2Buffer -= TransmitIndex;

                  /* Increment the buffer length.                       */
                  SPPLEBufferLength += TransmitIndex;
               }

               /* Reset the Transmit Index to 0.                        */
               TransmitIndex     = 0;

               /* Fill up the rest of the buffer with the data string.  */
               Added2Buffer += FillBufferWithString(&(DeviceInfo.TransmitBuffer), &SPPLEBufferLength, MaxLength, SPPLEBuffer);

               /* We have data to send so cap it at the maximum that can*/
               /* be transmitted.                                       */
               DataCount     = (SPPLEBufferLength > MaxLength)?MaxLength:SPPLEBufferLength;
            }

            /* We are acting as SPPLE Client, so write to the Rx        */
            /* Characteristic.                                          */
            Result = GATM_WriteValueWithoutResponse(GATMCallbackID, ConnectionBD_ADDR, DeviceInfo.ReceiveHandle, FALSE, DataCount, SPPLEBuffer);

            /* Check to see if any data was written.                    */
            if(!Done)
            {
               /* Check to see if the data was written successfully.    */
               if(Result >= 0)
               {
                  /* Adjust the counters.                               */
                  SendInfo.BytesToSend        -= (unsigned int)Result;
                  SendInfo.BytesSent          += (unsigned int)Result;
                  TransmitIndex               += (unsigned int)Result;
                  SPPLEBufferLength           -= (unsigned int)Result;

                  if(DeviceInfo.SupportCredits)
                     DeviceInfo.TransmitCredits -= (unsigned int)Result;

                  /* If we have no more remaining Tx Credits AND we have*/
                  /* data built up to send, we need to queue this in the*/
                  /* Tx Buffer.                                         */
                  if((DeviceInfo.SupportCredits) && (!(DeviceInfo.TransmitCredits)) && (SPPLEBufferLength))
                  {
                     /* Add the remaining data to the transmit buffer.  */
                     AddDataToBuffer(&(DeviceInfo.TransmitBuffer), SPPLEBufferLength, &SPPLEBuffer[TransmitIndex]);

                     SPPLEBufferLength = 0;
                  }
               }
               else
               {
                  printf("SEND failed with error %d\r\n", Result);

                  SendInfo.BytesToSend  = 0;
               }
            }
         }

         /* If we have added more bytes to the transmit buffer than we  */
         /* can send in this process remove the extra.                  */
         if(Added2Buffer > SendInfo.BytesToSend)
            RemoveDataFromBuffer(&(DeviceInfo.TransmitBuffer), Added2Buffer-SendInfo.BytesToSend, NULL);

         BTPS_ReleaseMutex(DeviceInfoLock);
      }

      /* Display a message if we have sent all required data.           */
      if((!SendInfo.BytesToSend) && (SendInfo.BytesSent))
      {
         printf("\r\nSend Complete, Sent %u.\r\n", (unsigned int)SendInfo.BytesSent);
         //DisplayPrompt();

         SendInfo.BytesSent = 0;
      }
   }
}

   /* The following function is responsible for transmitting the        */
   /* specified number of credits to the remote device.                 */
static void SendCredits(unsigned int DataLength)
{
   NonAlignedWord_t Credits;

   /* Verify that the input parameters are semi-valid.                  */
   if((Connected) && (DataLength))
   {
      /* Format the credit packet.                                      */
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Credits, DataLength);

      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         /* We are acting as a client so send a Write Without Response  */
         /* packet to the Tx Credit Characteristic.                     */
         GATM_WriteValueWithoutResponse(GATMCallbackID, ConnectionBD_ADDR, DeviceInfo.TxCreditsHandle, FALSE, WORD_SIZE, (Byte_t *)&Credits);

         BTPS_ReleaseMutex(DeviceInfoLock);
      }
   }
}

   /* The following function is responsible for handling a received     */
   /* credit, event.                                                    */
static void ReceiveCreditEvent(unsigned int Credits)
{
   /* Verify that the input parameters are semi-valid.                  */
   if(Connected)
   {
      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         /* If this is a real credit event store the number of credits. */
         DeviceInfo.TransmitCredits += Credits;

         /* Handle any active send process.                             */
         SendProcess();

         /* Send all queued data.                                       */
         SendData(0, NULL);

         /* It is possible that we have received data queued, so call   */
         /* the Data Indication Event to handle this.                   */
         DataIndicationEvent(0, NULL);

         BTPS_ReleaseMutex(DeviceInfoLock);
      }
   }
}

   /* The following function sends the specified data to the specified  */
   /* data.  This function will queue any of the data that does not go  */
   /* out.  This function returns TRUE if all the data was sent, or     */
   /* FALSE.                                                            */
   /* * NOTE * If DataLength is 0 and Data is NULL then all queued data */
   /*          will be sent.                                            */
static Boolean_t SendData(unsigned int DataLength, Byte_t *Data)
{
   int          Result;
   Boolean_t    DataSent = FALSE;
   Boolean_t    Done;
   unsigned int DataCount;
   unsigned int MaxLength;
   unsigned int TransmitIndex;
   unsigned int SPPLEBufferLength;

   /* Verify that the input parameters are semi-valid.                  */
   if(Connected)
   {
      /* Loop while we have data to send and we can send it.            */
      Done              = FALSE;
      TransmitIndex     = 0;
      SPPLEBufferLength = 0;

      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         while(!Done)
         {
            /* Check to see if we have credits to use to transmit the   */
            /* data.                                                    */
            if((!DeviceInfo.SupportCredits) || ((DeviceInfo.SupportCredits) && (DeviceInfo.TransmitCredits)))
            {
               /* Get the maximum length of what we can send in this    */
               /* transaction.                                          */
               if(DeviceInfo.SupportCredits)
                  MaxLength = (SPPLE_DATA_BUFFER_LENGTH > DeviceInfo.TransmitCredits)?DeviceInfo.TransmitCredits:SPPLE_DATA_BUFFER_LENGTH;
               else
                  MaxLength = SPPLE_DATA_BUFFER_LENGTH;

               /* If we do not have any outstanding data get some more  */
               /* data.                                                 */
               if(!SPPLEBufferLength)
               {
                  /* Send any buffered data first.                      */
                  if(DeviceInfo.TransmitBuffer.BytesFree != DeviceInfo.TransmitBuffer.BufferSize)
                  {
                     /* Remove the queued data from the Transmit Buffer.*/
                     SPPLEBufferLength = RemoveDataFromBuffer(&(DeviceInfo.TransmitBuffer), MaxLength, SPPLEBuffer);
                  }
                  else
                  {
                     /* Check to see if we have data to send.           */
                     if((DataLength) && (Data))
                     {
                        /* Copy the data to send into the SPPLEBuffer.  */
                        SPPLEBufferLength = (DataLength > MaxLength)?MaxLength:DataLength;
                        BTPS_MemCopy(SPPLEBuffer, Data, SPPLEBufferLength);

                        DataLength -= SPPLEBufferLength;
                        Data       += SPPLEBufferLength;
                     }
                     else
                     {
                        /* No data queued or data left to send so exit  */
                        /* the loop.                                    */
                        Done = TRUE;
                     }
                  }

                  /* Set the count of data that we can send.            */
                  DataCount         = SPPLEBufferLength;

                  /* Reset the Transmit Index to 0.                     */
                  TransmitIndex     = 0;
               }
               else
               {
                  /* We have data to send so cap it at the maximum that */
                  /* can be transmitted.                                */
                  DataCount = (SPPLEBufferLength > MaxLength)?MaxLength:SPPLEBufferLength;
               }

               /* Try to write data if not exiting the loop.            */
               if(!Done)
               {
                  Result = GATM_WriteValueWithoutResponse(GATMCallbackID, ConnectionBD_ADDR, DeviceInfo.ReceiveHandle, FALSE, DataCount, &SPPLEBuffer[TransmitIndex]);

                  /* Check to see if any data was written.              */
                  if(!Done)
                  {
                     /* Check to see if the data was written            */
                     /* successfully.                                   */
                     if(Result >= 0)
                     {
                        /* Adjust the counters.                         */
                        TransmitIndex               += (unsigned int)Result;
                        SPPLEBufferLength           -= (unsigned int)Result;

                        if(DeviceInfo.SupportCredits)
                           DeviceInfo.TransmitCredits -= (unsigned int)Result;

                        /* Flag that data was sent.                     */
                        DataSent                     = TRUE;

                        /* If we have no more remaining Tx Credits AND  */
                        /* we have data built up to send, we need to    */
                        /* queue this in the Tx Buffer.                 */
                        if((DeviceInfo.SupportCredits) && (!(DeviceInfo.TransmitCredits)) && (SPPLEBufferLength))
                        {
                           /* Add the remaining data to the transmit    */
                           /* buffer.                                   */
                           AddDataToBuffer(&(DeviceInfo.TransmitBuffer), SPPLEBufferLength, &SPPLEBuffer[TransmitIndex]);

                           SPPLEBufferLength = 0;
                        }
                     }
                     else
                     {
                        printf("SEND failed with error %d\r\n", Result);

                        DataSent  = FALSE;
                     }
                  }
               }
            }
            else
            {
               /* We have no transmit credits, so buffer the data.      */
               DataCount = AddDataToBuffer(&(DeviceInfo.TransmitBuffer), DataLength, Data);
               if(DataCount == DataLength)
                  DataSent = TRUE;
               else
                  DataSent = FALSE;

               /* Exit the loop.                                        */
               Done = TRUE;
            }
         }

         BTPS_ReleaseMutex(DeviceInfoLock);
      }
   }

   return(DataSent);
}

   /* The following function is responsible for handling a data         */
   /* indication event.                                                 */
static void DataIndicationEvent(unsigned int DataLength, Byte_t *Data)
{
   Boolean_t    Done;
   unsigned int ReadLength;
   unsigned int Length;

   /* Verify that the input parameters are semi-valid.                  */
   if(Connected)
   {
      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         /* If we are automatically reading the data, go ahead and      */
         /* credit what we just received, as well as reading everything */
         /* in the buffer.                                              */
         if((AutomaticReadActive) || (LoopbackActive))
         {
            /* Loop until we read all of the data queued.               */
            Done = FALSE;
            while(!Done)
            {
               /* If in loopback mode cap what we remove at the max of  */
               /* what we can send or queue.                            */
               if((DeviceInfo.SupportCredits) && (LoopbackActive))
                  ReadLength = (SPPLE_DATA_BUFFER_LENGTH > (DeviceInfo.TransmitCredits + DeviceInfo.TransmitBuffer.BytesFree))?(DeviceInfo.TransmitCredits + DeviceInfo.TransmitBuffer.BytesFree):SPPLE_DATA_BUFFER_LENGTH;
               else
                  ReadLength = SPPLE_DATA_BUFFER_LENGTH;

               /* Read all queued data.                                 */
               Length = ReadData(ReadLength, SPPLEBuffer);
               if(Length > 0)
               {
                  /* If loopback is active, loopback the data.          */
                  if(LoopbackActive)
                     SendData(Length, SPPLEBuffer);

                  /* If we are displaying the data then do that here.   */
                  if(DisplayRawData)
                  {
                     SPPLEBuffer[Length] = '\0';
                     printf("%s\n",(char *)SPPLEBuffer);
                  }
               }
               else
                  Done = TRUE;
            }

            /* Only send/display data just received if any is specified */
            /* in the call to this function.                            */
            if((DataLength) && (Data))
            {
               /* If loopback is active, loopback the data just         */
               /* received.                                             */
               if((AutomaticReadActive) || (LoopbackActive))
               {
                  /* If we are displaying the data then do that here.   */
                  if(DisplayRawData)
                  {
                     BTPS_MemCopy(SPPLEBuffer, Data, DataLength);
                     SPPLEBuffer[DataLength] = '\0';
                     printf("%s\n",(char *)SPPLEBuffer);
                  }

                  /* Check to see if Loopback is active, if it is we    */
                  /* will loopback the data we just received.           */
                  if(LoopbackActive)
                  {
                     /* Only queue the data in the receive buffer that  */
                     /* we cannot send.                                 */
                     if(DeviceInfo.SupportCredits)
                        ReadLength = (DataLength > (DeviceInfo.TransmitCredits + DeviceInfo.TransmitBuffer.BytesFree))?(DeviceInfo.TransmitCredits + DeviceInfo.TransmitBuffer.BytesFree):DataLength;
                     else
                        ReadLength = DataLength;

                     /* Send the data.                                  */
                     if(SendData(ReadLength, Data))
                     {
                        /* Credit the data we just sent.                */
                        if(DeviceInfo.SupportCredits)
                           SendCredits(ReadLength);

                        /* Increment what was just sent.                */
                        DataLength -= ReadLength;
                        Data       += ReadLength;
                     }
                  }
                  else
                  {
                     /* Loopback is not active so just credit back the  */
                     /* data we just received.                          */
                     if(DeviceInfo.SupportCredits)
                        SendCredits(DataLength);

                     DataLength = 0;
                  }

                  /* If we have data left that cannot be sent, queue    */
                  /* this in the receive buffer.                        */
                  if((DataLength) && (Data))
                  {
                     /* We are not in Loopback or Automatic Read Mode so*/
                     /* just buffer all the data.                       */
                     Length = AddDataToBuffer(&(DeviceInfo.ReceiveBuffer), DataLength, Data);
                     if(Length != DataLength)
                        printf("Receive Buffer Overflow of %u bytes", DataLength - Length);
                  }
               }

               /* If we are displaying the data then do that here.      */
               if(DisplayRawData)
               {
                  BTPS_MemCopy(SPPLEBuffer, Data, DataLength);
                  SPPLEBuffer[DataLength] = '\0';
                  printf("%s\n",(char *)SPPLEBuffer);
               }
            }
         }
         else
         {
            if((DataLength) && (Data))
            {
               /* Display a Data indication event.                      */
               printf("\r\nData Indication Event, Received %u bytes.\r\n", DataLength);

               /* We are not in Loopback or Automatic Read Mode so just */
               /* buffer all the data.                                  */
               Length = AddDataToBuffer(&(DeviceInfo.ReceiveBuffer), DataLength, Data);
               if(Length != DataLength)
                  printf("Receive Buffer Overflow of %u bytes.\r\n", DataLength - Length);
            }
         }

         BTPS_ReleaseMutex(DeviceInfoLock);
      }
   }
}

   /* The following function is used to read data from the specified    */
   /* device.  The final two parameters specify the BufferLength and the*/
   /* Buffer to read the data into.  On success this function returns   */
   /* the number of bytes read.  If an error occurs this will return a  */
   /* negative error code.                                              */
static int ReadData(unsigned int BufferLength, Byte_t *Buffer)
{
   int          ret_val;
   Boolean_t    Done;
   unsigned int Length;
   unsigned int TotalLength;

   /* Verify that the input parameters are semi-valid.                  */
   if((Connected) && (BufferLength) && (Buffer))
   {
      Done        = FALSE;
      TotalLength = 0;

      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         while(!Done)
         {
            Length = RemoveDataFromBuffer(&(DeviceInfo.ReceiveBuffer), BufferLength, Buffer);
            if(Length > 0)
            {
               BufferLength -= Length;
               Buffer       += Length;
               TotalLength   = Length;
            }
            else
               Done = TRUE;
         }

         /* Credit what we read.                                        */
         if(DeviceInfo.SupportCredits)
            SendCredits(TotalLength);

         BTPS_ReleaseMutex(DeviceInfoLock);
      }

      /* Return the total number of bytes read.                         */
      ret_val = (int)TotalLength;
   }
   else
      ret_val = BTPS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for displaying the current  */
   /* Command Options for this Device Manager Sample Application.  The  */
   /* input parameter to this function is completely ignored, and only  */
   /* needs to be passed in because all Commands that can be entered at */
   /* the Prompt pass in the parsed information.  This function displays*/
   /* the current Command Options that are available and always returns */
   /* zero.                                                             */
static int DisplayHelp(ParameterList_t *TempParam)
{
   /* Note the order they are listed here *MUST* match the order in     */
   /* which then are added to the Command Table.                        */
   printf("******************************************************************\r\n");
   printf("* Command Options: 1) Initialize                                 *\r\n");
   printf("*                  2) Cleanup                                    *\r\n");
   printf("*                  3) QueryDebugZoneMask                         *\r\n");
   printf("*                  4) SetDebugZoneMask                           *\r\n");
   printf("*                  5) ShutdownService                            *\r\n");
   printf("*                  6) RegisterEventCallback,                     *\r\n");
   printf("*                  7) UnRegisterEventCallback,                   *\r\n");
   printf("*                  8) QueryDevicePower                           *\r\n");
   printf("*                  9) SetDevicePower                             *\r\n");
   printf("*                  10)QueryLocalDeviceProperties                 *\r\n");
   printf("*                  11)SetLocalDeviceName                         *\r\n");
   printf("*                  12)SetLocalClassOfDevice                      *\r\n");
   printf("*                  13)SetDiscoverable                            *\r\n");
   printf("*                  14)SetConnectable                             *\r\n");
   printf("*                  15)SetPairable                                *\r\n");
   printf("*                  16)StartDeviceDiscovery                       *\r\n");
   printf("*                  17)StopDeviceDiscovery                        *\r\n");
   printf("*                  18)QueryRemoteDeviceList                      *\r\n");
   printf("*                  19)QueryRemoteDeviceProperties                *\r\n");
   printf("*                  20)AddRemoteDevice                            *\r\n");
   printf("*                  21)DeleteRemoteDevice                         *\r\n");
   printf("*                  22)UpdateRemoteDeviceAppData                  *\r\n");
   printf("*                  23)DeleteRemoteDevices                        *\r\n");
   printf("*                  24)PairWithRemoteDevice                       *\r\n");
   printf("*                  25)CancelPairWithRemoteDevice                 *\r\n");
   printf("*                  26)UnPairRemoteDevice                         *\r\n");
   printf("*                  27)QueryRemoteDeviceServices                  *\r\n");
   printf("*                  28)QueryRemoteDeviceServiceSupported          *\r\n");
   printf("*                  29)QueryRemoteDevicesForService               *\r\n");
   printf("*                  30)QueryRemoteDeviceServiceClasses            *\r\n");
   printf("*                  31)AuthenticateRemoteDevice                   *\r\n");
   printf("*                  32)EncryptRemoteDevice                        *\r\n");
   printf("*                  33)ConnectWithRemoteDevice                    *\r\n");
   printf("*                  34)DisconnectRemoteDevice                     *\r\n");
   printf("*                  35)SetRemoteDeviceLinkActive                  *\r\n");
   printf("*                  36)CreateSDPRecord                            *\r\n");
   printf("*                  37)DeleteSDPRecord                            *\r\n");
   printf("*                  38)AddSDPAttribute                            *\r\n");
   printf("*                  39)DeleteSDPAttribute                         *\r\n");
   printf("*                  40)EnableBluetoothDebug                       *\r\n");
   printf("*                  41)RegisterAuthentication                     *\r\n");
   printf("*                  42)UnRegisterAuthentication                   *\r\n");
   printf("*                  43)PINCodeResponse                            *\r\n");
   printf("*                  44)PassKeyResponse                            *\r\n");
   printf("*                  45)UserConfirmationResponse                   *\r\n");
   printf("*                  46)ChangeSimplePairingParameters              *\r\n");
   printf("*                  47)RegisterGATTCallback                       *\r\n");
   printf("*                  48)UnRegisterGATTCallback                     *\r\n");
   printf("*                  49)QueryGATTConnections                       *\r\n");
   printf("*                  50)SetLocalDeviceAppearance                   *\r\n");
   printf("*                  51)ConfigureSPPLE                             *\r\n");
   printf("*                  52)Send                                       *\r\n");
   printf("*                  53)Read                                       *\r\n");
   printf("*                  54)Loopback                                   *\r\n");
   printf("*                  55)DisplayRawModeData                         *\r\n");
   printf("*                  56)AutomaticReadMode                          *\r\n");
   printf("*                  57)StartAdvertising                           *\r\n");
   printf("*                  58)StopAdvertising                            *\r\n");
   printf("*                  59)StartObservationScan                       *\r\n");
   printf("*                  60)StopObservationScan                        *\r\n");
   printf("*                  Help, Quit.                                   *\r\n");
   printf("******************************************************************\r\n");

   return(0);
}

   /* The following function is responsible for Initializing the        */
   /* Bluetopia Platform Manager Framework.  This function returns      */
   /* zero if successful and a negative value if an error occurred.     */
static int Initialize(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(!Initialized)
   {
      /* Determine if the user would like to Register an Event Callback */
      /* with the calling of this command.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now actually initialize the Platform Manager Service.       */
         Result = BTPM_Initialize((unsigned long)getpid(), NULL, ServerUnRegistrationCallback, NULL);

         if(!Result)
         {
            /* Initialization successful, go ahead and inform the user  */
            /* that it was successful and flag that the Platform Manager*/
            /* has been initialized.                                    */
            printf("BTPM_Initialize() Success: %d.\r\n", Result);

            /* If the caller would like to Register an Event Callback   */
            /* then we will do that at this time.                       */
            if(TempParam->Params[0].intParam)
            {
               if((Result = DEVM_RegisterEventCallback(DEVM_Event_Callback, NULL)) > 0)
               {
                  printf("DEVM_RegisterEventCallback() Success: %d.\r\n", Result);

                  /* Note the Callback ID and flag success.             */
                  DEVMCallbackID = (unsigned int)Result;

                  Initialized    = TRUE;

                  ret_val        = 0;
               }
               else
               {
                  /* Error registering the Callback, inform user and    */
                  /* flag an error.                                     */
                  printf("DEVM_RegisterEventCallback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;

                  /* Since there was an error, go ahead and clean up the*/
                  /* library.                                           */
                  BTPM_Cleanup();
               }
            }
            else
            {
               /* Nothing more to do, simply flag success to the caller.*/
               Initialized = TRUE;

               ret_val     = 0;
            }
         }
         else
         {
            /* Error initializing Platform Manager, inform the user.    */
            printf("BTPM_Initialize() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Initialize [0/1 - Register for Events].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Already initialized, flag an error.                            */
      printf("Initialization Failure: Already initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Cleaning up/Shutting    */
   /* down the Bluetopia Platform Manager Framework.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int Cleanup(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there was an Event Callback Registered, then we need to     */
      /* un-register it.                                                */
      if(DEVMCallbackID)
         DEVM_UnRegisterEventCallback(DEVMCallbackID);

      if(GATMCallbackID)
         GATM_UnRegisterEventCallback(GATMCallbackID);

      /* Nothing to do other than to clean up the Bluetopia Platform    */
      /* Manager Service and flag that it is no longer initialized.     */
      BTPM_Cleanup();

      Initialized    = FALSE;
      DEVMCallbackID = 0;

      ret_val        = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Registering a Local     */
   /* Device Manager Callback with the Bluetopia Platform Manager       */
   /* Framework.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int RegisterEventCallback(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Event Callback Registered, then we need to flag */
      /* an error.                                                      */
      if(!DEVMCallbackID)
      {
         /* Callback has not been registered, go ahead and attempt to   */
         /* register it.                                                */
         if((Result = DEVM_RegisterEventCallback(DEVM_Event_Callback, NULL)) > 0)
         {
            printf("DEVM_RegisterEventCallback() Success: %d.\r\n", Result);

            /* Note the Callback ID and flag success.                   */
            DEVMCallbackID = (unsigned int)Result;

            ret_val        = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("DEVM_RegisterEventCallback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Device Manager Event Callback already registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Registering a Local  */
   /* Device Manager Callback that has previously been registered with  */
   /* the Bluetopia Platform Manager Framework.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int UnRegisterEventCallback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(DEVMCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         DEVM_UnRegisterEventCallback(DEVMCallbackID);

         printf("DEVM_UnRegisterEventCallback() Success.\r\n");

         /* Flag that there is no longer a Callback registered.         */
         DEVMCallbackID = 0;

         ret_val        = 0;
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Device Manager Event Callback is not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Setting the Device Power*/
   /* of the Local Device.  This function returns zero if successful and*/
   /* a negative value if an error occurred.                            */
static int SetDevicePower(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Determine if the user would like to Power On or Off the Local  */
      /* Device with the calling of this command.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now actually Perform the command.                           */
         if(TempParam->Params[0].intParam)
            Result = DEVM_PowerOnDevice();
         else
            Result = DEVM_PowerOffDevice();

         if(!Result)
         {
            /* Device Power request was successful, go ahead and inform */
            /* the User.                                                */
            printf("DEVM_Power%sDevice() Success: %d.\r\n", TempParam->Params[0].intParam?"On":"Off", Result);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Powering On/Off the device, inform the user.       */
            printf("DEVM_Power%sDevice() Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"On":"Off", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDevicePower [0/1 - Power Off/Power On].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* device power for the local device.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int QueryDevicePower(ParameterList_t *TempParam)
{
   int       ret_val;
   Boolean_t Result;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Framework has been initialized, go ahead and query the current */
      /* Power state.                                                   */
      Result = DEVM_QueryDevicePowerState();

      if(Result >= 0)
         printf("DEVM_QueryDevicePowerState() Success: %s.\r\n", Result?"On":"Off");
      else
         printf("DEVM_QueryDevicePowerState() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

      /* Flag success to the caller.                                    */
      ret_val = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Setting the Debug Zone  */
   /* Mask (either Local or Remote).  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int SetLocalRemoteDebugZoneMask(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Determine if the user would like to set the Local Library Debug*/
      /* Mask or the Remote Services Debug Mask.                        */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Now actually Perform the command.                           */
         Result = BTPM_SetDebugZoneMask((Boolean_t)((TempParam->Params[0].intParam)?TRUE:FALSE), (unsigned long)TempParam->Params[1].intParam);

         if(!Result)
         {
            /* Set Debug Zone Mask request was successful, go ahead and */
            /* inform the User.                                         */
            printf("BTPM_SetDebugZoneMask(%s) Success: 0x%08lX.\r\n", TempParam->Params[0].intParam?"Remote":"Local", (unsigned long)TempParam->Params[1].intParam);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Debug Zone Mask, inform the user.         */
            printf("BTPM_SetDebugZoneMask(%s) Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"Remote":"Local", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDebugZoneMask [0/1 - Local/Service] [Debug Zone Mask].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the current    */
   /* Debug Zone Mask (either Local or Remote).  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int QueryLocalRemoteDebugZoneMask(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   unsigned long DebugZoneMask;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Determine if the user would like to query the Local Library    */
      /* Debug Mask or the Remote Services Debug Mask.                  */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now actually Perform the command.                           */
         Result = BTPM_QueryDebugZoneMask((Boolean_t)((TempParam->Params[0].intParam)?TRUE:FALSE), (TempParam->NumberofParameters > 1)?TempParam->Params[1].intParam:0, &DebugZoneMask);

         if(!Result)
         {
            /* Query Debug Zone Mask request was successful, go ahead   */
            /* and inform the User.                                     */
            printf("BTPM_QueryDebugZoneMask(%s) Success: 0x%08lX.\r\n", TempParam->Params[0].intParam?"Remote":"Local", DebugZoneMask);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Debug Zone Mask, inform the user.         */
            printf("BTPM_QueryDebugZoneMask(%s, %d) Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"Remote":"Local", (TempParam->NumberofParameters > 1)?TempParam->Params[1].intParam:0, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: QueryDebugZoneMask [0/1 - Local/Service] [Page Number - optional, default 0].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for shutting down the remote*/
   /* server.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int ShutdownService(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to shutdown the server.      */
      if((Result = BTPM_ShutdownService()) == 0)
      {
         printf("BTPM_ShutdownService() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error shutting down the service, inform the user and flag an*/
         /* error.                                                      */
         printf("BTPM_ShutdownService() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for displaying the current  */
   /* local device properties of the server.  This function returns zero*/
   /* if successful and a negative value if an error occurred.          */
static int QueryLocalDeviceProperties(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and query the Local Device Properties.   */
      if((Result = DEVM_QueryLocalDeviceProperties(&LocalDeviceProperties)) >= 0)
      {
         printf("DEVM_QueryLocalDeviceProperties() Success: %d.\r\n", Result);

         /* Next, go ahead and display the properties.                  */
         DisplayLocalDeviceProperties(0, &LocalDeviceProperties);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error querying the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_QueryLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Name  */
   /* Device Property of the local device.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int SetLocalDeviceName(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      if((TempParam) && (TempParam->NumberofParameters))
      {
         LocalDeviceProperties.DeviceNameLength = strlen(TempParam->Params[0].strParam);
         strcpy(LocalDeviceProperties.DeviceName, TempParam->Params[0].strParam);
      }

      printf("Attempting to set Device Name to: \"%s\".\r\n", LocalDeviceProperties.DeviceName);

      if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DEVICE_NAME, &LocalDeviceProperties)) >= 0)
      {
         printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error updating the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Name  */
   /* Device Property of the local device.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int SetLocalDeviceAppearance(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      if((TempParam) && (TempParam->NumberofParameters))
         LocalDeviceProperties.DeviceAppearance = (Word_t)TempParam->Params[0].intParam;

      printf("Attempting to set Device Appearance to: %u.\r\n", (unsigned int)LocalDeviceProperties.DeviceAppearance);

      if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DEVICE_APPEARANCE, &LocalDeviceProperties)) >= 0)
      {
         printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error updating the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Class */
   /* of Device Device Property of the local device.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SetLocalClassOfDevice(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         ASSIGN_CLASS_OF_DEVICE(LocalDeviceProperties.ClassOfDevice, (Byte_t)((TempParam->Params[0].intParam) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 16) & 0xFF));

         printf("Attempting to set Class Of Device to: 0x%02X%02X%02X.\r\n", LocalDeviceProperties.ClassOfDevice.Class_of_Device0, LocalDeviceProperties.ClassOfDevice.Class_of_Device1, LocalDeviceProperties.ClassOfDevice.Class_of_Device2);

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CLASS_OF_DEVICE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetLocalClassOfDevice [Class of Device].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local       */
   /* Discoverability Mode Device Property of the local device.  This   */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetDiscoverable(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         LocalDeviceProperties.DiscoverableMode = (Boolean_t)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters > 1)
            LocalDeviceProperties.DiscoverableModeTimeout = TempParam->Params[1].intParam;

         if(TempParam->Params[0].intParam)
         {
            if(LocalDeviceProperties.DiscoverableModeTimeout)
               printf("Attempting to set Discoverability Mode: Limited (%d Seconds).\r\n", LocalDeviceProperties.DiscoverableModeTimeout);
            else
               printf("Attempting to set Discoverability Mode: General.\r\n");
         }
         else
            printf("Attempting to set Discoverability Mode: None.\r\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DISCOVERABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetDiscoverable [Enable/Disable] [Timeout (Enable only)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local       */
   /* Connectability Mode Device Property of the local device.  This    */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetConnectable(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         LocalDeviceProperties.ConnectableMode = (Boolean_t)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters > 1)
            LocalDeviceProperties.ConnectableModeTimeout = TempParam->Params[1].intParam;

         if(TempParam->Params[0].intParam)
         {
            if(LocalDeviceProperties.ConnectableModeTimeout)
               printf("Attempting to set Connectability Mode: Connectable (%d Seconds).\r\n", LocalDeviceProperties.ConnectableModeTimeout);
            else
               printf("Attempting to set Connectability Mode: Connectable.\r\n");
         }
         else
            printf("Attempting to set Connectability Mode: Non-Connectable.\r\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CONNECTABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetConnectable [Enable/Disable] [Timeout (Enable only)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local       */
   /* Pairability Mode Device Property of the local device.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SetPairable(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      BTPS_MemInitialize(&LocalDeviceProperties, 0, sizeof(LocalDeviceProperties));

      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         LocalDeviceProperties.PairableMode = (Boolean_t)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters > 1)
            LocalDeviceProperties.PairableModeTimeout = TempParam->Params[1].intParam;

         if(TempParam->Params[0].intParam)
         {
            if(LocalDeviceProperties.PairableModeTimeout)
               printf("Attempting to set Pairability Mode: Pairable (%d Seconds).\r\n", LocalDeviceProperties.PairableModeTimeout);
            else
               printf("Attempting to set Pairability Mode: Pairable.\r\n");
         }
         else
            printf("Attempting to set Pairability Mode: Non-Pairable.\r\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_PAIRABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetPairable [Enable/Disable] [Timeout (Enable only)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for starting a Device       */
   /* Discovery on the local device.  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int StartDeviceDiscovery(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         if(TempParam->Params[1].intParam)
            printf("Attempting to Start Discovery (%d Seconds).\r\n", TempParam->Params[1].intParam);
         else
            printf("Attempting to Start Discovery (INDEFINITE).\r\n");

         /* Check to see if we are doing an LE or BR/EDR Discovery      */
         /* Process.                                                    */
         if((Boolean_t)TempParam->Params[0].intParam)
         {
            if((Result = DEVM_StartDeviceScan(TempParam->Params[1].intParam)) >= 0)
            {
               printf("DEVM_StartDeviceScan() Success: %d.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
               printf("DEVM_StartDeviceScan() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            if((Result = DEVM_StartDeviceDiscovery(TempParam->Params[1].intParam)) >= 0)
            {
               printf("DEVM_StartDeviceDiscovery() Success: %d.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
               printf("DEVM_StartDeviceDiscovery() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartDeviceDiscovery [Type (1 = LE, 0 = BR/EDR)] [Duration].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for stopping a Device       */
   /* Discovery on the local device.  This function returns zero if     */
   /* successful and a negative value if an error occurred.             */
static int StopDeviceDiscovery(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Check to see what type of discovery should be stopped.      */
         if((Boolean_t)TempParam->Params[0].intParam)
         {
            /* Initialized, go ahead and attempt to stop LE Device      */
            /* Discovery.                                               */
            if((Result = DEVM_StopDeviceScan()) >= 0)
            {
               printf("DEVM_StopDeviceScan() Success: %d.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error stopping Device Discovery, inform the user and  */
               /* flag an error.                                        */
               printf("DEVM_StopDeviceScan() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Initialized, go ahead and attempt to stop BR/EDR Device  */
            /* Discovery.                                               */
            if((Result = DEVM_StopDeviceDiscovery()) >= 0)
            {
               printf("DEVM_StopDeviceDiscovery() Success: %d.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error stopping Device Discovery, inform the user and  */
               /* flag an error.                                        */
               printf("DEVM_StopDeviceDiscovery() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StopDeviceDiscovery [Type (1 = LE, 0 = BR/EDR)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for starting a Observation  */
   /* Scan on the local device.  This function returns zero if          */
   /* successful and a negative value if an error occurred.             */
static int StartObservationScan(ParameterList_t *TempParam)
{
   int                           Result;
   int                           ret_val;
   DEVM_Observation_Parameters_t ObservationParameters;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Format the observation parameters.                          */
         BTPS_MemInitialize(&ObservationParameters, 0, sizeof(ObservationParameters));

         ObservationParameters.ReportingFrequency = (unsigned int)TempParam->Params[0].intParam;

         if(TempParam->NumberofParameters >= 3)
         {
            ObservationParameters.ObservationParameterFlags = DEVM_OBSERVATION_PARAMETER_FLAGS_SCAN_WINDOW_INTERVAL_VALID;
            ObservationParameters.ScanWindow                = TempParam->Params[1].intParam;
            ObservationParameters.ScanInterval              = TempParam->Params[2].intParam;
         }

         /* Initialized, go ahead and attempt to start the LE           */
         /* observation scan process.                                   */
         if((Result = DEVM_StartObservationScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY, &ObservationParameters)) >= 0)
         {
            printf("DEVM_StartObservationScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error starting the observation scan process, inform the  */
            /* user and flag an error.                                  */
            printf("DEVM_StartObservationScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartObservationScan [Reporting Frequency] [Scan Window (optional)] [Scan Interval (must be specified if Window is)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for stopping a Observation  */
   /* Scan on the local device.  This function returns zero if          */
   /* successful and a negative value if an error occurred.             */
static int StopObservationScan(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to stop the LE Observation   */
      /* Scan process.                                                  */
      if((Result = DEVM_StopObservationScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY)) >= 0)
      {
         printf("DEVM_StopDeviceScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error stopping the observation scan process, inform the user*/
         /* and flag an error.                                          */
         printf("DEVM_StopDeviceScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Address List.  This function returns zero if successful and*/
   /* a negative value if an error occurred.                            */
static int QueryRemoteDeviceList(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   char               Buffer[32];
   BD_ADDR_t         *BD_ADDRList;
   unsigned int       Index;
   unsigned int       Filter;
   unsigned int       TotalNumberDevices;
   Class_of_Device_t  ClassOfDevice;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         printf("Attempting Query %d Devices.\r\n", TempParam->Params[0].intParam);

         if(TempParam->Params[0].intParam)
            BD_ADDRList = (BD_ADDR_t *)BTPS_AllocateMemory(sizeof(BD_ADDR_t)*TempParam->Params[0].intParam);
         else
            BD_ADDRList = NULL;

         if((!TempParam->Params[0].intParam) || ((TempParam->Params[0].intParam) && (BD_ADDRList)))
         {
            /* If there were any optional parameters specified, go ahead*/
            /* and note them.                                           */
            if(TempParam->NumberofParameters > 1)
               Filter = TempParam->Params[1].intParam;
            else
               Filter = 0;

            if(TempParam->NumberofParameters > 2)
            {
               ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, (Byte_t)((TempParam->Params[2].intParam) & 0xFF), (Byte_t)(((TempParam->Params[2].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[2].intParam) >> 16) & 0xFF));
            }
            else
            {
               ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, 0, 0, 0);
            }

            if((Result = DEVM_QueryRemoteDeviceList(Filter, ClassOfDevice, TempParam->Params[0].intParam, BD_ADDRList, &TotalNumberDevices)) >= 0)
            {
               printf("DEVM_QueryRemoteDeviceList() Success: %d, Total Number Devices: %d.\r\n", Result, TotalNumberDevices);

               if((Result) && (BD_ADDRList))
               {
                  printf("Returned device list (%d Entries):\r\n", Result);

                  for(Index=0;Index<Result;Index++)
                  {
                     BD_ADDRToStr(BD_ADDRList[Index], Buffer);

                     printf("%2d. %s\r\n", (Index+1), Buffer);
                  }
               }

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to query the Remote Device List,     */
               /* inform the user and flag an error.                    */
               printf("DEVM_QueryRemoteDeviceList() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Free any memory that was allocated.                      */
            if(BD_ADDRList)
               BTPS_FreeMemory(BD_ADDRList);
         }
         else
         {
            /* Unable to allocate memory for List.                      */
            printf("Unable to allocate memory for %d Devices.\r\n", TempParam->Params[0].intParam);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceList [Number of Devices] [Filter (Optional)] [COD Filter (Optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Properties of a specific Remote Device.  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryRemoteDeviceProperties(ParameterList_t *TempParam)
{
   int                             Result;
   int                             ret_val;
   BD_ADDR_t                       BD_ADDR;
   unsigned long                   QueryFlags;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Check to see what kind of query is being done.              */
         if(TempParam->Params[1].intParam)
            QueryFlags = DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY;
         else
            QueryFlags = 0;

         if((TempParam->NumberofParameters >= 3) && (TempParam->Params[2].intParam))
            QueryFlags |= DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_FORCE_UPDATE;

         printf("Attempting to Query %s Device Properties: %s, ForceUpdate: %s.\r\n", TempParam->Params[0].strParam, (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_LOW_ENERGY)?"LE":"BR/EDR", (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_PROPERTIES_FLAGS_FORCE_UPDATE)?"TRUE":"FALSE");

         if((Result = DEVM_QueryRemoteDeviceProperties(BD_ADDR, QueryFlags, &RemoteDeviceProperties)) >= 0)
         {
            printf("DEVM_QueryRemoteDeviceProperties() Success: %d.\r\n", Result);

            /* Display the Remote Device Properties.                    */
            DisplayRemoteDeviceProperties(0, &RemoteDeviceProperties);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Remote Device, inform the user and flag an*/
            /* error.                                                   */
            printf("DEVM_QueryRemoteDeviceProperties() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceProperties [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Force Update].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Adding the specified    */
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int AddRemoteDevice(ParameterList_t *TempParam)
{
   int                                   Result;
   int                                   ret_val;
   BD_ADDR_t                             BD_ADDR;
   Class_of_Device_t                     ClassOfDevice;
   DEVM_Remote_Device_Application_Data_t ApplicationData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Check to see if a Class of Device was specified.            */
         if(TempParam->NumberofParameters > 1)
         {
            ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, (Byte_t)((TempParam->Params[1].intParam) & 0xFF), (Byte_t)(((TempParam->Params[1].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[1].intParam) >> 16) & 0xFF));
         }
         else
         {
            ASSIGN_CLASS_OF_DEVICE(ClassOfDevice, 0, 0, 0);
         }

         printf("Attempting to Add Device: %s.\r\n", TempParam->Params[0].strParam);

         /* Check to see if Application Information was specified.      */
         if(TempParam->NumberofParameters > 2)
         {
            ApplicationData.FriendlyNameLength = strlen(TempParam->Params[2].strParam);

            strcpy(ApplicationData.FriendlyName, TempParam->Params[2].strParam);

            ApplicationData.ApplicationInfo = 0;

            if(TempParam->NumberofParameters > 3)
               ApplicationData.ApplicationInfo = TempParam->Params[3].intParam;
         }

         if((Result = DEVM_AddRemoteDevice(BD_ADDR, ClassOfDevice, (TempParam->NumberofParameters > 2)?&ApplicationData:NULL)) >= 0)
         {
            printf("DEVM_AddRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Adding Remote Device, inform the user and flag an  */
            /* error.                                                   */
            printf("DEVM_AddRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AddRemoteDevice [BD_ADDR] [[COD (Optional)] [Friendly Name (Optional)] [Application Info (Optional)]].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Deleting the specified  */
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int DeleteRemoteDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Delete Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_DeleteRemoteDevice(BD_ADDR)) >= 0)
         {
            printf("DEVM_DeleteRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Deleting Remote Device, inform the user and flag an*/
            /* error.                                                   */
            printf("DEVM_DeleteRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteRemoteDevice [BD_ADDR].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Updating the specified  */
   /* Remote Device's Applicatoin Data.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int UpdateRemoteDeviceApplicationData(ParameterList_t *TempParam)
{
   int                                   Result;
   int                                   ret_val;
   BD_ADDR_t                             BD_ADDR;
   DEVM_Remote_Device_Application_Data_t ApplicationData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Verify that the correct Application Data formats are        */
         /* present.                                                    */
         if((!TempParam->Params[1].intParam) || ((TempParam->Params[1].intParam) && (TempParam->NumberofParameters > 3)))
         {
            /* Check to see if Application Information was specified.   */
            if((TempParam->Params[1].intParam) && (TempParam->NumberofParameters > 3))
            {
               ApplicationData.FriendlyNameLength = strlen(TempParam->Params[2].strParam);

               strcpy(ApplicationData.FriendlyName, TempParam->Params[2].strParam);

               ApplicationData.ApplicationInfo = TempParam->Params[3].intParam;
            }

            printf("Attempting to Update Device Application Data: %s.\r\n", TempParam->Params[0].strParam);

            if((Result = DEVM_UpdateRemoteDeviceApplicationData(BD_ADDR, TempParam->Params[1].intParam?&ApplicationData:NULL)) >= 0)
            {
               printf("DEVM_UpdateRemoteDeviceApplicationData() Success: %d.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error Deleting Remote Device, inform the user and flag*/
               /* an error.                                             */
               printf("DEVM_UpdateRemoteDeviceApplicationData() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: UpdateRemoteDeviceAppData [BD_ADDR] [Data Valid] [Friendly Name] [Application Info].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: UpdateRemoteDeviceAppData [BD_ADDR] [Data Valid] [Friendly Name] [Application Info].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Deleting the specified  */
   /* Remote Devices.  This function returns zero if successful and a   */
   /* negative value if an error occurred.                              */
static int DeleteRemoteDevices(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         printf("Attempting to Delete Remote Devices, Filter %d.\r\n", TempParam->Params[0].intParam);

         if((Result = DEVM_DeleteRemoteDevices(TempParam->Params[0].intParam)) >= 0)
         {
            printf("DEVM_DeleteRemoteDevices() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Deleting Remote Devices, inform the user and flag  */
            /* an error.                                                */
            printf("DEVM_DeleteRemoteDevices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteRemoteDevices [Device Delete Filter].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Pairing the specified   */
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int PairWithRemoteDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned long Flags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 2)
            Flags = TempParam->Params[2].intParam;
         else
            Flags = 0;

         if(TempParam->Params[1].intParam)
            Flags |= DEVM_PAIR_WITH_REMOTE_DEVICE_FLAGS_LOW_ENERGY;

         printf("Attempting to Pair With Remote %s Device: %s.\r\n", (Flags & DEVM_PAIR_WITH_REMOTE_DEVICE_FLAGS_LOW_ENERGY)?"LE":"BR/EDR", TempParam->Params[0].strParam);

         if((Result = DEVM_PairWithRemoteDevice(BD_ADDR, Flags)) >= 0)
         {
            printf("DEVM_PairWithRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Pairing with Remote Device, inform the user and    */
            /* flag an error.                                           */
            printf("DEVM_PairWithRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: PairWithRemoteDevice [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Pair Flags (optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Cancelling an on-going  */
   /* Pairing operation with the specified Remote Device.  This function*/
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int CancelPairWithRemoteDevice(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Cancel Pair With Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_CancelPairWithRemoteDevice(BD_ADDR)) >= 0)
         {
            printf("DEVM_CancelPairWithRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Cancelling Pairing with Remote Device, inform the  */
            /* user and flag an error.                                  */
            printf("DEVM_CancelPairWithRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: CancelPairWithRemoteDevice [BD_ADDR].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Pairing the specified*/
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int UnPairRemoteDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned long UnPairFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Un-Pair Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if(TempParam->Params[1].intParam)
            UnPairFlags = DEVM_UNPAIR_REMOTE_DEVICE_FLAGS_LOW_ENERGY;
         else
            UnPairFlags = 0;

         if((Result = DEVM_UnPairRemoteDevice(BD_ADDR, UnPairFlags)) >= 0)
         {
            printf("DEVM_UnPairRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Un-Pairing with Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_UnPairRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: UnPairRemoteDevice [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] .\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the Remote     */
   /* Device Services of the specified Remote Device.  This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryRemoteDeviceServices(ParameterList_t *TempParam)
{
   int                          Result;
   int                          ret_val;
   BD_ADDR_t                    BD_ADDR;
   unsigned int                 Index;
   unsigned int                 TotalServiceSize;
   unsigned char               *ServiceData;
   unsigned long                QueryFlags;
   DEVM_Parsed_SDP_Data_t       ParsedSDPData;
   DEVM_Parsed_Services_Data_t  ParsedGATTData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 2))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Check to see what kind of Services are being requested.     */
         if(TempParam->Params[1].intParam)
            QueryFlags = DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY;
         else
            QueryFlags = 0;

         /* Check to see if a forced update is requested.               */
         if(TempParam->Params[2].intParam)
            QueryFlags |= DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE;

         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting Query Remote Device %s For %s Services.\r\n", TempParam->Params[0].strParam, (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY)?"GATT":"SDP");

         if(!(QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE))
         {
            /* Caller has requested to actually retrieve it locally,    */
            /* determine how many bytes were requested.                 */
            if(TempParam->NumberofParameters > 3)
               ServiceData = (unsigned char *)BTPS_AllocateMemory(TempParam->Params[3].intParam);
            else
               ret_val = INVALID_PARAMETERS_ERROR;
         }
         else
            ServiceData = NULL;

         if(!ret_val)
         {
            if((QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE) || ((!(QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE)) && (ServiceData)))
            {
               if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, QueryFlags, (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE)?0:TempParam->Params[3].intParam, ServiceData, &TotalServiceSize)) >= 0)
               {
                  printf("DEVM_QueryRemoteDeviceServices() Success: %d, Total Number Service Bytes: %d.\r\n", Result, (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE)?0:TotalServiceSize);

                  /* Now convert the Raw Data to parsed data.           */
                  if((Result) && (ServiceData))
                  {
                     printf("Returned Service Data (%d Bytes):\r\n", Result);

                     for(Index=0;Index<Result;Index++)
                        printf("%02X", ServiceData[Index]);

                     printf("\r\n");
                     printf("\r\n");

                     /* Check to see what kind of stream was requested. */
                     if(QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY)
                     {
                        /* Convert the Raw GATT Stream to a Parsed GATT */
                        /* Stream.                                      */
                        Result = DEVM_ConvertRawServicesStreamToParsedServicesData((unsigned int)Result, ServiceData, &ParsedGATTData);
                        if(!Result)
                        {
                           /* Display the Parsed GATT Service Data.     */
                           DisplayParsedGATTServiceData(&ParsedGATTData);

                           /* All finished with the parsed data, so free*/
                           /* it.                                       */
                           DEVM_FreeParsedServicesData(&ParsedGATTData);
                        }
                        else
                        {
                           printf("DEVM_ConvertRawServicesStreamToParsedServicesData() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
                        }
                     }
                     else
                     {
                        /* Convert the Raw SDP Stream to a Parsed       */
                        /* Stream.                                      */
                        Result = DEVM_ConvertRawSDPStreamToParsedSDPData((unsigned int)Result, ServiceData, &ParsedSDPData);

                        if(!Result)
                        {
                           /* Success, Display the Parsed Data.         */
                           DisplayParsedSDPServiceData(&ParsedSDPData);

                           /* All finished with the parsed data, so free*/
                           /* it.                                       */
                           DEVM_FreeParsedSDPData(&ParsedSDPData);
                        }
                        else
                        {
                           printf("DEVM_ConvertRawSDPStreamToParsedSDPData() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
                        }
                     }
                  }

                  /* Flag success.                                      */
                  ret_val = 0;
               }
               else
               {
                  /* Error attempting to query Services, inform the user*/
                  /* and flag an error.                                 */
                  printf("DEVM_QueryRemoteDeviceServices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free any memory that was allocated.                   */
               if(ServiceData)
                  BTPS_FreeMemory(ServiceData);
            }
            else
            {
               /* Unable to allocate memory for List.                   */
               printf("Unable to allocate memory for %d Service Bytes.\r\n", TempParam->Params[2].intParam);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Force Update] [Bytes to Query (specified if Force is 0)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Force Update] [Bytes to Query (specified if Force is 0)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying whether a      */
   /* Remote Device claims support for the specified Service. This      */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int QueryRemoteDeviceServiceSupported(ParameterList_t *TempParam)
{
   int              Result;
   int              ret_val;
   BD_ADDR_t        BD_ADDR;
   SDP_UUID_Entry_t ServiceUUID;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Convert the first parameter to a Bluetooth Device Address.  */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Convert the parameter to a SDP Service Class UUID.          */
         StrToUUIDEntry(TempParam->Params[1].strParam, &ServiceUUID);

         if(ServiceUUID.SDP_Data_Element_Type != deNULL)
         {
            printf("Attempting Query Remote Device %s Support for Service %s.\r\n", TempParam->Params[0].strParam, TempParam->Params[1].strParam);

            if((Result = DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR, ServiceUUID)) >= 0)
            {
               printf("DEVM_QueryRemoteDeviceServiceSupported() Success: %d.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to query Services, inform the user   */
               /* and flag an error.                                    */
               printf("DEVM_QueryRemoteDeviceServiceSupported() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteDeviceServiceSupported [BD_ADDR] [Service UUID].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceServiceSupported [BD_ADDR] [Service UUID].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying which Remote   */
   /* Devices claim support for the specified Service. This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryRemoteDevicesForService(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   char              Buffer[32];
   BD_ADDR_t        *BD_ADDRList;
   unsigned int      Index;
   unsigned int      TotalNumberDevices;
   SDP_UUID_Entry_t  ServiceUUID;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Convert the parameter to a SDP Service Class UUID.          */
         StrToUUIDEntry(TempParam->Params[0].strParam, &ServiceUUID);

         if(ServiceUUID.SDP_Data_Element_Type != deNULL)
         {
            printf("Attempting Query Up To %d Devices For Service %s.\r\n", TempParam->Params[1].intParam, TempParam->Params[0].strParam);

            if(TempParam->Params[1].intParam)
               BD_ADDRList = (BD_ADDR_t *)BTPS_AllocateMemory(sizeof(BD_ADDR_t)*TempParam->Params[1].intParam);
            else
               BD_ADDRList = NULL;

            if((!TempParam->Params[1].intParam) || ((TempParam->Params[1].intParam) && (BD_ADDRList)))
            {
               if((Result = DEVM_QueryRemoteDevicesForService(ServiceUUID, TempParam->Params[1].intParam, BD_ADDRList, &TotalNumberDevices)) >= 0)
               {
                  printf("DEVM_QueryRemoteDevicesForService() Success: %d, Total Number Devices: %d.\r\n", Result, TotalNumberDevices);

                  if((Result) && (BD_ADDRList))
                  {
                     printf("Returned device list (%d Entries):\r\n", Result);

                     for(Index=0;Index<Result;Index++)
                     {
                        BD_ADDRToStr(BD_ADDRList[Index], Buffer);

                        printf("%2d. %s\r\n", (Index+1), Buffer);
                     }
                  }

                  /* Flag success.                                      */
                  ret_val = 0;
               }
               else
               {
                  /* Error attempting to query the Remote Devices,      */
                  /* inform the user and flag an error.                 */
                  printf("DEVM_QueryRemoteDevicesForService() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free any memory that was allocated.                   */
               if(BD_ADDRList)
                  BTPS_FreeMemory(BD_ADDRList);
            }
            else
            {
               /* Unable to allocate memory for List.                   */
               printf("Unable to allocate memory for %d Devices.\r\n", TempParam->Params[0].intParam);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteDevicesForService [Service UUID] [Number of Devices].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDevicesForService [Service UUID] [Number of Devices].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Querying the supported  */
   /* Service Classes of the specified Remote Device. This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int QueryRemoteDeviceServiceClasses(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   BD_ADDR_t         BD_ADDR;
   unsigned int      Index;
   unsigned int      TotalNumberServiceClasses;
   SDP_UUID_Entry_t *ServiceClassList;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting Query Device %s For Up To %d Service Classes.\r\n", TempParam->Params[0].strParam, TempParam->Params[1].intParam);

         if(TempParam->Params[1].intParam)
            ServiceClassList = (SDP_UUID_Entry_t *)BTPS_AllocateMemory(sizeof(SDP_UUID_Entry_t)*TempParam->Params[1].intParam);
         else
            ServiceClassList = NULL;

         if((!TempParam->Params[1].intParam) || ((TempParam->Params[1].intParam) && (ServiceClassList)))
         {
            if((Result = DEVM_QueryRemoteDeviceServiceClasses(BD_ADDR, TempParam->Params[1].intParam, ServiceClassList, &TotalNumberServiceClasses)) >= 0)
            {
               printf("DEVM_QueryRemoteDeviceServiceClasses() Success: %d, Total Number Service Classes: %d.\r\n", Result, TotalNumberServiceClasses);

               if((Result) && (ServiceClassList))
               {
                  printf("Returned service classes (%d Entries):\r\n", Result);

                  for(Index=0;Index<Result;Index++)
                  {
                     switch(ServiceClassList[Index].SDP_Data_Element_Type)
                     {
                        case deUUID_16:
                           printf("%2d. 0x%02X%02X\r\n", (Index+1), ServiceClassList[Index].UUID_Value.UUID_16.UUID_Byte0, ServiceClassList[Index].UUID_Value.UUID_16.UUID_Byte1);
                           break;
                        case deUUID_32:
                           printf("%2d. 0x%02X%02X%02X%02X\r\n", (Index+1), ServiceClassList[Index].UUID_Value.UUID_32.UUID_Byte0, ServiceClassList[Index].UUID_Value.UUID_32.UUID_Byte1, ServiceClassList[Index].UUID_Value.UUID_32.UUID_Byte2, ServiceClassList[Index].UUID_Value.UUID_32.UUID_Byte3);
                           break;
                        case deUUID_128:
                           printf("%2d. %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\r\n", (Index+1), ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte0, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte1, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte2, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte3, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte4, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte5, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte6, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte7, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte8, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte9, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte10, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte11, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte12, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte13, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte14, ServiceClassList[Index].UUID_Value.UUID_128.UUID_Byte15);
                           break;
                        default:
                           printf("%2d. (invalid: %u)\r\n", (Index+1), ServiceClassList[Index].SDP_Data_Element_Type);
                           break;
                     }
                  }
               }

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to query the Remote Devices, inform  */
               /* the user and flag an error.                           */
               printf("DEVM_QueryRemoteDeviceServiceClasses() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Unable to allocate memory for List.                      */
            printf("Unable to allocate memory for %d Devices.\r\n", TempParam->Params[0].intParam);

            ret_val = FUNCTION_ERROR;
         }

         /* Free any memory that was allocated.                         */
         if(ServiceClassList)
            BTPS_FreeMemory(ServiceClassList);
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceServiceClasses [BD_ADDR] [Number of Service Classes].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Authenticating the      */
   /* specified Remote Device.  This function returns zero if successful*/
   /* and a negative value if an error occurred.                        */
static int AuthenticateRemoteDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned long AuthenticateFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Authenticate Remote Device: %s over %s.\r\n", TempParam->Params[0].strParam, TempParam->Params[1].intParam?"LE":"BR/EDR");

         /* Check to see what type of encryption operation to perform.  */
         if(TempParam->Params[1].intParam)
            AuthenticateFlags = DEVM_AUTHENTICATE_REMOTE_DEVICE_FLAGS_LOW_ENERGY;
         else
            AuthenticateFlags = 0;

         if((Result = DEVM_AuthenticateRemoteDevice(BD_ADDR, AuthenticateFlags)) >= 0)
         {
            printf("DEVM_AuthenticateRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Authenticating Remote Device, inform the user and  */
            /* flag an error.                                           */
            printf("DEVM_AuthenticateRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AuthenticateRemoteDevice [BD_ADDR] [Type (LE = 1, BR/EDR = 0)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Encrypting the specified*/
   /* Remote Device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int EncryptRemoteDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned long EncryptFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to Encrypt Remote Device: %s over %s.\r\n", TempParam->Params[0].strParam, TempParam->Params[1].intParam?"LE":"BR/EDR");

         /* Check to see what type of encryption operation to perform.  */
         if(TempParam->Params[1].intParam)
            EncryptFlags = DEVM_ENCRYPT_REMOTE_DEVICE_FLAGS_LOW_ENERGY;
         else
            EncryptFlags = 0;

         if((Result = DEVM_EncryptRemoteDevice(BD_ADDR, EncryptFlags)) >= 0)
         {
            printf("DEVM_EncryptRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Encrypting Remote Device, inform the user and flag */
            /* an error.                                                */
            printf("DEVM_EncryptRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: EncryptRemoteDevice [BD_ADDR] [Type (LE = 1, BR/EDR = 0)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Connecting with the     */
   /* specified Remote Device.  This function returns zero if successful*/
   /* and a negative value if an error occurred.                        */
static int ConnectWithRemoteDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned long ConnectFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters>=2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 2)
            ConnectFlags = (unsigned long)(TempParam->Params[2].intParam);
         else
            ConnectFlags = 0;

         /* Check to see if we should connect LE.                       */
         if(TempParam->Params[1].intParam)
            ConnectFlags |= DEVM_CONNECT_WITH_REMOTE_DEVICE_FORCE_LOW_ENERGY;

         printf("Attempting to Connect With (%s) Remote Device: %s (Flags = 0x%08lX).\r\n", (ConnectFlags & DEVM_CONNECT_WITH_REMOTE_DEVICE_FORCE_LOW_ENERGY)?"LE":"BR/EDR", TempParam->Params[0].strParam, ConnectFlags);

         if((Result = DEVM_ConnectWithRemoteDevice(BD_ADDR, ConnectFlags)) >= 0)
         {
            printf("DEVM_ConnectWithRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Connecting With Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_ConnectWithRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ConnectWithRemoteDevice [BD_ADDR] [Connect LE (1 = LE, 0 = BR/EDR)] [ConnectFlags (Optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Disconnecting from the  */
   /* specified Remote Device.  This function returns zero if successful*/
   /* and a negative value if an error occurred.                        */
static int DisconnectRemoteDevice(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   BD_ADDR_t     BD_ADDR;
   unsigned long DisconnectFlags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Determine if the Force Flag was specified.                  */
         if(TempParam->NumberofParameters > 2)
            DisconnectFlags = (TempParam->Params[2].intParam?DEVM_DISCONNECT_FROM_REMOTE_DEVICE_FLAGS_FORCE:0);
         else
            DisconnectFlags = 0;

         /* Disconnect from an LE Device.                               */
         if(TempParam->Params[1].intParam)
            DisconnectFlags |= DEVM_DISCONNECT_FROM_REMOTE_DEVICE_FLAGS_LOW_ENERGY;

         printf("Attempting to Disconnect Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_DisconnectRemoteDevice(BD_ADDR, DisconnectFlags)) >= 0)
         {
            printf("DEVM_DisconnectRemoteDevice() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Disconnecting Remote Device, inform the user and   */
            /* flag an error.                                           */
            printf("DEVM_DisconnectRemoteDevice() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DisconnectRemoteDevice [BD_ADDR] [LE Device (1= LE, 0 = BR/EDR)] [Force Flag (Optional).\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the link for a  */
   /* specified Remote Device to active.  This function returns zero if */
   /* successful and a negative value if an error occurred.             */
static int SetRemoteDeviceLinkActive(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting to set link active for Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_SetRemoteDeviceLinkActive(BD_ADDR)) >= 0)
         {
            printf("DEVM_SetRemoteDeviceLinkActive() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error setting link for Remote Device, inform the user and*/
            /* flag an error.                                           */
            printf("DEVM_SetRemoteDeviceLinkActive() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetRemoteDeviceLinkActive [BD_ADDR].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Registering a sample SDP*/
   /* Record in the SDP Database of the Local Device.  This function    */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int CreateSDPRecord(ParameterList_t *TempParam)
{
   int                ret_val;
   int                Result;
   long               RecordHandle;
   char               ServiceName[32];
   Boolean_t          Persistent;
   unsigned int       Port;
   SDP_UUID_Entry_t   SDPUUIDEntries;
   SDP_Data_Element_t SDP_Data_Element[3];
   SDP_Data_Element_t SDP_Data_Element_L2CAP;
   SDP_Data_Element_t SDP_Data_Element_RFCOMM[2];
   SDP_Data_Element_t SDP_Data_Element_Language[4];

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
         Port = TempParam->Params[0].intParam;
      else
         Port = 1;

      if((TempParam) && (TempParam->NumberofParameters > 1))
         Persistent = (Boolean_t)((TempParam->Params[1].intParam)?TRUE:FALSE);
      else
         Persistent = (Boolean_t)FALSE;

      printf("Attempting to Add sample SPP SDP Record (Port %d, Persistent: %s).\r\n", Port, Persistent?"TRUE":"FALSE");

      /* Initialize the Serial Port Profile.                            */
      SDPUUIDEntries.SDP_Data_Element_Type = deUUID_16;
      SDP_ASSIGN_SERIAL_PORT_PROFILE_UUID_16(SDPUUIDEntries.UUID_Value.UUID_16);

      if((RecordHandle = DEVM_CreateServiceRecord(Persistent, 1, &SDPUUIDEntries)) >= 0)
      {
         printf("DEVM_CreateServiceRecord() Success: %ld (0x%08lX).\r\n", (unsigned long)RecordHandle, (unsigned long)RecordHandle);

         /* Next, add the Protocol Descriptor List.                     */
         SDP_Data_Element[0].SDP_Data_Element_Type                      = deSequence;
         SDP_Data_Element[0].SDP_Data_Element_Length                    = 2;
         SDP_Data_Element[0].SDP_Data_Element.SDP_Data_Element_Sequence = &SDP_Data_Element[1];

         SDP_Data_Element[1].SDP_Data_Element_Type                      = deSequence;
         SDP_Data_Element[1].SDP_Data_Element_Length                    = 1;
         SDP_Data_Element[1].SDP_Data_Element.SDP_Data_Element_Sequence = &SDP_Data_Element_L2CAP;

         SDP_Data_Element_L2CAP.SDP_Data_Element_Type                   = deUUID_16;
         SDP_Data_Element_L2CAP.SDP_Data_Element_Length                 = sizeof(SDP_Data_Element_L2CAP.SDP_Data_Element.UUID_16);
         SDP_ASSIGN_L2CAP_UUID_16(SDP_Data_Element_L2CAP.SDP_Data_Element.UUID_16);

         SDP_Data_Element[2].SDP_Data_Element_Type                      = deSequence;
         SDP_Data_Element[2].SDP_Data_Element_Length                    = 2;
         SDP_Data_Element[2].SDP_Data_Element.SDP_Data_Element_Sequence = SDP_Data_Element_RFCOMM;

         SDP_Data_Element_RFCOMM[0].SDP_Data_Element_Type                 = deUUID_16;
         SDP_Data_Element_RFCOMM[0].SDP_Data_Element_Length               = UUID_16_SIZE;
         SDP_ASSIGN_RFCOMM_UUID_16(SDP_Data_Element_RFCOMM[0].SDP_Data_Element.UUID_16);

         SDP_Data_Element_RFCOMM[1].SDP_Data_Element_Type                 = deUnsignedInteger1Byte;
         SDP_Data_Element_RFCOMM[1].SDP_Data_Element_Length               = sizeof(SDP_Data_Element_RFCOMM[1].SDP_Data_Element.UnsignedInteger1Byte);
         SDP_Data_Element_RFCOMM[1].SDP_Data_Element.UnsignedInteger1Byte = (Byte_t)Port;

         Result = DEVM_AddServiceRecordAttribute((unsigned long)RecordHandle, SDP_ATTRIBUTE_ID_PROTOCOL_DESCRIPTOR_LIST, SDP_Data_Element);

         if(!Result)
         {
            /* Add a default Language Attribute ID List Entry for UTF-8 */
            /* English.                                                 */
            SDP_Data_Element_Language[0].SDP_Data_Element_Type                      = deSequence;
            SDP_Data_Element_Language[0].SDP_Data_Element_Length                    = 3;
            SDP_Data_Element_Language[0].SDP_Data_Element.SDP_Data_Element_Sequence = &SDP_Data_Element_Language[1];
            SDP_Data_Element_Language[1].SDP_Data_Element_Type                      = deUnsignedInteger2Bytes;
            SDP_Data_Element_Language[1].SDP_Data_Element_Length                    = SDP_UNSIGNED_INTEGER_2_BYTES_SIZE;
            SDP_Data_Element_Language[1].SDP_Data_Element.UnsignedInteger2Bytes     = SDP_NATURAL_LANGUAGE_ENGLISH_UTF_8;
            SDP_Data_Element_Language[2].SDP_Data_Element_Type                      = deUnsignedInteger2Bytes;
            SDP_Data_Element_Language[2].SDP_Data_Element_Length                    = SDP_UNSIGNED_INTEGER_2_BYTES_SIZE;
            SDP_Data_Element_Language[2].SDP_Data_Element.UnsignedInteger2Bytes     = SDP_UTF_8_CHARACTER_ENCODING;
            SDP_Data_Element_Language[3].SDP_Data_Element_Type                      = deUnsignedInteger2Bytes;
            SDP_Data_Element_Language[3].SDP_Data_Element_Length                    = SDP_UNSIGNED_INTEGER_2_BYTES_SIZE;
            SDP_Data_Element_Language[3].SDP_Data_Element.UnsignedInteger2Bytes     = SDP_DEFAULT_LANGUAGE_BASE_ATTRIBUTE_ID;

            Result = DEVM_AddServiceRecordAttribute((unsigned long)RecordHandle, SDP_ATTRIBUTE_ID_LANGUAGE_BASE_ATTRIBUTE_ID_LIST, SDP_Data_Element_Language);

            /* Finally Add the Service Name to the SDP Database.        */
            if(!Result)
            {
               /* Build the Service Name.                               */
               sprintf(ServiceName, "Sample Serial Port: %d.", Port);

               SDP_Data_Element[0].SDP_Data_Element_Type       = deTextString;
               SDP_Data_Element[0].SDP_Data_Element_Length     = BTPS_StringLength(ServiceName);
               SDP_Data_Element[0].SDP_Data_Element.TextString = (Byte_t *)ServiceName;

               if(!(Result = DEVM_AddServiceRecordAttribute((unsigned long)RecordHandle, (SDP_DEFAULT_LANGUAGE_BASE_ATTRIBUTE_ID + SDP_ATTRIBUTE_OFFSET_ID_SERVICE_NAME), SDP_Data_Element)))
               {
                  printf("Sample SPP SDP Record successfully created: %ld (%08lX).\r\n", (unsigned long)RecordHandle, (unsigned long)RecordHandle);

                  /* Flag success.                                      */
                  ret_val = 0;
               }
               else
               {
                  printf("DEVM_AddServiceRecordAttribute() Failure - Service Name: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("DEVM_AddServiceRecordAttribute() Failure - Language Attribute ID: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("DEVM_AddServiceRecordAttribute() Failure - Protocol Desriptor List: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }

         /* If an error occurred, to ahead and delete the record.       */
         if((Result) && (((unsigned long)RecordHandle) > 0))
            DEVM_DeleteServiceRecord((unsigned long)RecordHandle);
      }
      else
      {
         /* Error attempting to create Record, inform the user and flag */
         /* an error.                                                   */
         printf("DEVM_CreateServiceRecord() Failure: %ld, %s.\r\n", RecordHandle, ERR_ConvertErrorCodeToString(RecordHandle));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Deleting an SDP Record  */
   /* in the SDP Database of the Local Device.  This function returns   */
   /* zero if successful and a negative value if an error occurred.     */
static int DeleteSDPRecord(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   unsigned long RecordHandle;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         RecordHandle = (unsigned long)TempParam->Params[0].intParam;

         printf("Attempting to Delete SDP Record %ld (0x%08lX).\r\n", RecordHandle, RecordHandle);

         if(!(Result = DEVM_DeleteServiceRecord(RecordHandle)))
         {
            printf("DEVM_DeleteServiceRecord() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to delete Record, inform the user and   */
            /* flag an error.                                           */
            printf("DEVM_DeleteServiceRecord() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteSDPRecord [Service Record Handle].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for adding an SDP Attribute */
   /* to an SDP Record in the SDP Database of the Local Device.  This   */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int AddSDPAttribute(ParameterList_t *TempParam)
{
   int                ret_val;
   int                Result;
   unsigned int       Value;
   unsigned long      RecordHandle;
   SDP_Data_Element_t SDP_Data_Element;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         RecordHandle = (unsigned long)TempParam->Params[0].intParam;

         printf("Attempting to Create SDP Attribute %d (0x%08X) for Record %ld (0x%08lX).\r\n", TempParam->Params[1].intParam, TempParam->Params[1].intParam, RecordHandle, RecordHandle);

         if(TempParam->NumberofParameters > 2)
            Value = TempParam->Params[2].intParam;
         else
            Value = 1;

         SDP_Data_Element.SDP_Data_Element_Type                  = deUnsignedInteger4Bytes;
         SDP_Data_Element.SDP_Data_Element_Length                = sizeof(SDP_Data_Element.SDP_Data_Element.UnsignedInteger4Bytes);
         SDP_Data_Element.SDP_Data_Element.UnsignedInteger4Bytes = (DWord_t)Value;

         if(!(Result = DEVM_AddServiceRecordAttribute(RecordHandle, TempParam->Params[1].intParam, &SDP_Data_Element)))
         {
            printf("DEVM_AddServiceRecordAttribute() Success.\r\n");

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to delete Record, inform the user and   */
            /* flag an error.                                           */
            printf("DEVM_AddServiceRecordAttribute() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AddSDPAttribute [Service Record Handle] [Attribute ID] [Attribute Value (optional)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for deleting an SDP         */
   /* Attribute from an SDP Record in the SDP Database of the Local     */
   /* Device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int DeleteSDPAttribute(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   unsigned long RecordHandle;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         RecordHandle = (unsigned long)TempParam->Params[0].intParam;

         printf("Attempting to Delete SDP Attribute %d (0x%08X) from Record %ld (0x%08lX).\r\n", TempParam->Params[1].intParam, TempParam->Params[1].intParam, RecordHandle, RecordHandle);

         if(!(Result = DEVM_DeleteServiceRecordAttribute(RecordHandle, TempParam->Params[1].intParam)))
         {
            printf("DEVM_DeleteServiceRecordAttribute() Success.\r\n");

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to delete Record, inform the user and   */
            /* flag an error.                                           */
            printf("DEVM_DeleteServiceRecordAttribute() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteSDPRecord [Service Record Handle] [Attribute ID].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* Thw following function is responsible for enabling/disabling      */
   /* Bluetooth Debugging.  This function returns zero if successful and*/
   /* a negative value if an error occurred.                            */
static int EnableBluetoothDebug(ParameterList_t *TempParam)
{
   int            Result;
   int            ret_val;
   unsigned long  Flags;
   unsigned int   Type;
   unsigned int   ParameterDataLength;
   unsigned char *ParameterData;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Make sure the input parameters have been specified.            */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Initialize no parameters.                                   */
         ret_val             = 0;

         Flags               = 0;
         ParameterDataLength = 0;
         ParameterData       = NULL;

         /* Check to see if a Disable operation is being requested.     */
         if(!TempParam->Params[0].intParam)
         {
            /* Disable - No other parameters are required.              */
            Flags = 0;
            Type  = 0;
         }
         else
         {
            /* Enable - Make sure we have the Type specified.           */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Note the Debug Type.                                  */
               Type = (unsigned int)TempParam->Params[1].intParam;

               /* Check for optional flags.                             */
               if(TempParam->NumberofParameters >= 3)
                  Flags = (unsigned int)TempParam->Params[2].intParam;
               else
                  Flags = 0;

               /* Determine if there are any Parameters specified.      */
               if((TempParam->NumberofParameters >= 4) && (TempParam->Params[3].strParam))
               {
                  ParameterDataLength = BTPS_StringLength(TempParam->Params[3].strParam) + 1;
                  ParameterData       = (unsigned char *)TempParam->Params[3].strParam;
               }
            }
            else
            {
               printf("Usage: EnableBluetoothDebug [Enable (0/1)] [Type (1 - ASCII File, 2 - Terminal, 3 - FTS File)] [Debug Flags] [Debug Parameter String (no spaces)].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }

         if(!ret_val)
         {
            /* Now actually Perform the command.                        */
            Result = DEVM_EnableBluetoothDebug((Boolean_t)(TempParam->Params[0].intParam), Type, Flags, ParameterDataLength, ParameterData);

            if(!Result)
            {
               /* Enable Bluetooth Debugging request was successful, go */
               /* ahead and inform the User.                            */
               printf("DEVM_EnableBluetoothDebug(%s) Success.\r\n", TempParam->Params[0].intParam?"TRUE":"FALSE");

               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
            {
               /* Error Enabling Bluetooth Debugging, inform the user.  */
               printf("DEVM_EnableBluetoothDebug(%s) Failure: %d, %s.\r\n", TempParam->Params[0].intParam?"TRUE":"FALSE", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         printf("Usage: EnableBluetoothDebug [Enable (0/1)] [Type (1 - ASCII File, 2 - Terminal, 3 - FTS File)] [Debug Flags] [Debug Parameter String (no spaces)].\r\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Registering the Local   */
   /* Client to receive (and process) Authentication Events.  This      */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int RegisterAuthentication(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to Register for              */
      /* Authentication.                                                */
      if((Result = DEVM_RegisterAuthentication(DEVM_Authentication_Callback, NULL)) >= 0)
      {
         printf("DEVM_RegisterAuthentication() Success: %d.\r\n", Result);

         /* Note the Authentication Callback ID.                        */
         AuthenticationCallbackID = (unsigned int)Result;

         /* Flag success.                                               */
         ret_val                  = 0;
      }
      else
      {
         /* Error Registering for Authentication, inform the user and   */
         /* flag an error.                                              */
         printf("DEVM_RegisterAuthentication() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Registering the Local*/
   /* Client to receive (and process) Authentication Events.  This      */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int UnRegisterAuthentication(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Initialized, go ahead and attempt to Register for              */
      /* Authentication.                                                */
      DEVM_UnRegisterAuthentication(AuthenticationCallbackID);

      printf("DEVM_UnRegisterAuthentication() Success.\r\n");

      /* Clear the Authentication Callback ID.                          */
      AuthenticationCallbackID = 0;

      /* Flag success.                                                  */
      ret_val                  = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a PIN Code value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PINCodeResponse(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         NullADDR;
   PIN_Code_t                        PINCode;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam) > 0) && (strlen(TempParam->Params[0].strParam) <= sizeof(PIN_Code_t)))
         {
            /* Parameters appear to be valid, go ahead and convert the  */
            /* input parameter into a PIN Code.                         */

            /* Initialize the PIN code.                                 */
            ASSIGN_PIN_CODE(PINCode, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            memcpy(&PINCode, TempParam->Params[0].strParam, strlen(TempParam->Params[0].strParam));

            /* Populate the response structure.                         */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                    = CurrentRemoteBD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction       = DEVM_AUTHENTICATION_ACTION_PIN_CODE_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength   = (Byte_t)(strlen(TempParam->Params[0].strParam));

            AuthenticationResponseInformation.AuthenticationData.PINCode = PINCode;

            /* Submit the Authentication Response.                      */
            Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("DEVM_AuthenticationResponse(), Pin Code Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PINCodeResponse [PIN Code].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue PIN Code Authentication Response: Authentication is not currently in progress.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PassKeyResponse(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         NullADDR;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (strlen(TempParam->Params[0].strParam) <= GAP_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                    = CurrentRemoteBD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction       = DEVM_AUTHENTICATION_ACTION_PASSKEY_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength   = sizeof(AuthenticationResponseInformation.AuthenticationData.Passkey);

            AuthenticationResponseInformation.AuthenticationData.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            if(CurrentLowEnergy)
               AuthenticationResponseInformation.AuthenticationAction |= DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;

            /* Submit the Authentication Response.                      */
            Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("DEVM_AuthenticationResponse(), Passkey Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PassKeyResponse [Numeric Passkey (0 - 999999)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue Pass Key Authentication Response: Authentication is not currently in progress.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a User Confirmation value specified  */
   /* via the input parameter.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int UserConfirmationResponse(ParameterList_t *TempParam)
{
   int                               Result;
   int                               ret_val;
   BD_ADDR_t                         NullADDR;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                         = CurrentRemoteBD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

            AuthenticationResponseInformation.AuthenticationData.Confirmation = (Boolean_t)(TempParam->Params[0].intParam?TRUE:FALSE);

            if(CurrentLowEnergy)
               AuthenticationResponseInformation.AuthenticationAction |= DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;

            /* Submit the Authentication Response.                      */
            Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               printf("DEVM_AuthenticationResponse(), User Confirmation Response Success.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: UserConfirmationResponse [Confirmation (0 = No, 1 = Yes)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue User Confirmation Authentication Response: Authentication is not currently in progress.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for changing the Secure     */
   /* Simple Pairing Parameters that are exchanged during the Pairing   */
   /* procedure when Secure Simple Pairing (Security Level 4) is used.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int ChangeSimplePairingParameters(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam <= 3))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            IOCapability = icDisplayOnly;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               IOCapability = icDisplayYesNo;
            else
            {
               if(TempParam->Params[0].intParam == 2)
                  IOCapability = icKeyboardOnly;
               else
                  IOCapability = icNoInputNoOutput;
            }
         }

         /* Finally map the Man in the Middle (MITM) Protection valud.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capablities.                 */
         printf("Current I/O Capabilities: %s, MITM Protection: %s.\r\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangeSimplePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output)] [MITM Requirement (0 = No, 1 = Yes)].\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}


   /* The following function is responsible for Registering a Local     */
   /* Generic Attribute Profile Manager Callback with the Bluetopia     */
   /* Platform Manager Framework.  This function returns zero if        */
   /* successful and a negative value if an error occurred.             */
static int RegisterGATMEventCallback(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Event Callback Registered, then we need to flag */
      /* an error.                                                      */
      if(!GATMCallbackID)
      {
         /* Callback has not been registered, go ahead and attempt to   */
         /* register it.                                                */
         if((Result = GATM_RegisterEventCallback(GATM_Event_Callback, NULL)) > 0)
         {
            printf("GATM_RegisterEventCallback() Success: %d.\r\n", Result);

            /* Note the Callback ID and flag success.                   */
            GATMCallbackID = (unsigned int)Result;

            ret_val        = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("GATM_RegisterEventCallback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Generic Attribute Profile Manager Event Callback already registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for Un-Registering a Local  */
   /* Generic Attribute Profile Manager Callback that has previously    */
   /* been registered with the Bluetopia Platform Manager Framework.    */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static int UnRegisterGATMEventCallback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(GATMCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         if(!(ret_val = GATM_UnRegisterEventCallback(GATMCallbackID)))
         {
            printf("GATM_UnRegisterEventCallback() Success.\r\n");

            /* Flag that there is no longer a Callback registered.      */
            GATMCallbackID = 0;
         }
         else
         {
            /* Error un-registering the Callback, inform user and flag  */
            /* an error.                                                */
            printf("GATM_UnRegisterEventCallback() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Generic Attribute Profile Manager Event Callback is not registered.\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Current    */
   /* active GATT connections.  This function returns zero if successful*/
   /* and a negative value if an error occurred.                        */
static int GATTQueryConnectedDevices(ParameterList_t *TempParam)
{
   int                            Result;
   int                            ret_val;
   char                           Buffer[32];
   unsigned int                   Index;
   unsigned int                   TotalConnected;
   GATM_Connection_Information_t *ConnectionList;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First determine the total number of connected devices.         */
      if(!(Result = GATM_QueryConnectedDevices(0, NULL, &TotalConnected)))
      {
         printf("Number of GATT Connections: %u.\r\n\r\n", TotalConnected);
         ret_val = 0;

         if(TotalConnected)
         {
            if((ConnectionList = BTPS_AllocateMemory(TotalConnected*sizeof(GATM_Connection_Information_t))) != NULL)
            {
               /* Finally attempt to query the number of gatt           */
               /* connections.                                          */
               if((Result = GATM_QueryConnectedDevices(TotalConnected, ConnectionList, NULL)) >= 0)
               {
                  for(Index=0;Index<Result;Index++)
                  {
                     printf("%u: Connection Type: %s.\r\n", Index+1, ((ConnectionList[Index].ConnectionType == gctLE)?"LE":"BR/EDR"));
                     BD_ADDRToStr(ConnectionList[Index].RemoteDeviceAddress, Buffer);
                     printf("%u: BD_ADDR:         %s.\r\n", Index+1, Buffer);
                  }

                  printf("\r\n");

                  /* Finally return success to the caller.              */
                  ret_val = 0;
               }
               else
               {
                  /* Error querying number of connected devices, inform */
                  /* the user and flag an error.                        */
                  printf("GATM_QueryConnectedDevices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free the memory that was previously allocated.        */
               BTPS_FreeMemory(ConnectionList);
            }
            else
            {
               printf("Error allocating memory for connection list.\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* Error querying number of connected devices, inform the user */
         /* and flag an error.                                          */
         printf("GATM_QueryConnectedDevices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* Find SPPLE Service.                                               */
static int ConfigureSPPLE(ParameterList_t *TempParam)
{
   int                         ret_val;
   int                         Result;
   Boolean_t                   Found;
   BD_ADDR_t                   BD_ADDR;
   UUID_128_t                  CharacteristicUUID;
   unsigned int                ServiceSize;
   unsigned int                Index;
   unsigned int                Index1;
   unsigned int                Index2;
   unsigned int                CharacteristicHandle;
   unsigned int                CCDHandle;
   unsigned char               ServiceData[1024];
   DEVM_Parsed_Services_Data_t ParsedGATTData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((!Connected) && (TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Attempt to get the remote device service info.              */
         if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY, 1024, ServiceData, &ServiceSize)) >= 0)
         {
            /* Now, attempt to parse the remote service data.           */
            Result = DEVM_ConvertRawServicesStreamToParsedServicesData((unsigned int)Result, ServiceData, &ParsedGATTData);

            if(!Result)
            {
               /* Note that we have not found any SPPLE service.        */
               Found = FALSE;

               if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
               {
                  /* Loop through the remote services.                  */
                  for(Index = 0; (Index < ParsedGATTData.NumberServices && !Found); Index++)
                  {
                     /* Check if this service matches the SPPLE UUID.   */
                     if(SPPLE_COMPARE_SPPLE_SERVICE_UUID_TO_UUID_128(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID.UUID.UUID_128))
                     {
                        /* We found SPPLE. Initialize the handles.      */
                        DeviceInfo.TransmitHandle  = 0;
                        DeviceInfo.ReceiveHandle   = 0;
                        DeviceInfo.TxCreditsHandle = 0;
                        DeviceInfo.RxCreditsHandle = 0;

                        /* Now, loop through the characteristics.       */
                        for(Index1 = 0; Index1 < ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics; Index1++)
                        {
                           CharacteristicHandle = 0;
                           CCDHandle            = 0;

                           /* Look for the CCDs for notifications.      */
                           for(Index2 = 0; Index2 < ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].NumberOfDescriptors; Index2++)
                           {

                              /* CCDs are UUID_16s.                     */
                              if(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                              {
                                 /* Check if this is the CCD UUID.      */
                                 if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID.UUID_16))
                                 {
                                    /* Note the CCD Handle.             */
                                    CCDHandle = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_Handle;
                                 }
                              }
                           }

                           /* Note the characteristic handle and UUID.  */
                           CharacteristicHandle = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle;
                           CharacteristicUUID   = ParsedGATTData.GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID.UUID.UUID_128;

                           /* Look for the the characteristic handles.  */
                           if(SPPLE_COMPARE_SPPLE_TX_UUID_TO_UUID_128(CharacteristicUUID))
                           {
                              DeviceInfo.TransmitHandle = CharacteristicHandle;

                              /* Note if we have a CCD handle.          */
                              if(CCDHandle)
                                 DeviceInfo.TransmitCCDHandle = CCDHandle;
                           }
                           else
                           {
                              if(SPPLE_COMPARE_SPPLE_RX_UUID_TO_UUID_128(CharacteristicUUID))
                              {
                                 DeviceInfo.ReceiveHandle = CharacteristicHandle;
                              }
                              else
                              {
                                 if(SPPLE_COMPARE_SPPLE_TX_CREDITS_UUID_TO_UUID_128(CharacteristicUUID))
                                 {
                                    DeviceInfo.TxCreditsHandle = CharacteristicHandle;
                                 }
                                 else
                                 {
                                    if(SPPLE_COMPARE_SPPLE_RX_CREDITS_UUID_TO_UUID_128(CharacteristicUUID))
                                    {
                                       DeviceInfo.RxCreditsHandle = CharacteristicHandle;
                                       /* Note if we have a CCD handle. */
                                       if(CCDHandle)
                                          DeviceInfo.RxCreditsCCDHandle = CCDHandle;
                                    }
                                 }
                              }
                           }
                        }

                        /* Make sure we found the proper                */
                        /* characteristics.                             */
                        if((DeviceInfo.TransmitHandle) && (DeviceInfo.ReceiveHandle))
                        {
                           /* Note we found a valid SPPLE service.      */
                           Found = TRUE;

                           /* Save the information about the connected  */
                           /* Device.                                   */
                           Connected         = TRUE;
                           ConnectionBD_ADDR = BD_ADDR;

                           InitializeBuffer(&(DeviceInfo.ReceiveBuffer));
                           InitializeBuffer(&(DeviceInfo.TransmitBuffer));

                           /* Enable Tx Notifications.                  */
                           if(DeviceInfo.TransmitCCDHandle)
                              EnableDisableNotificationsIndications(DeviceInfo.TransmitCCDHandle,GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);

                           /* Check if we should support credits.       */
                           if((DeviceInfo.TxCreditsHandle) && (DeviceInfo.RxCreditsHandle))
                              DeviceInfo.SupportCredits = TRUE;
                           else
                              DeviceInfo.SupportCredits = FALSE;

                           /* Attempt to enable credit notifcations.    */
                           if((DeviceInfo.SupportCredits) && (DeviceInfo.RxCreditsCCDHandle))
                              EnableDisableNotificationsIndications(DeviceInfo.RxCreditsCCDHandle, GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE);
                           else
                              DeviceInfo.SupportCredits = FALSE;

                           /* Send the initial credits.                 */
                           if(DeviceInfo.SupportCredits)
                              SendCredits(DeviceInfo.TransmitBuffer.BufferSize);

                           printf("SPPLE service configured. SupportCredits: %s.\n", (DeviceInfo.SupportCredits)?"TRUE":"FALSE");
                        }
                     }
                  }

                  BTPS_ReleaseMutex(DeviceInfoLock);
               }

               DEVM_FreeParsedServicesData(&ParsedGATTData);

               if(!Found)
               {
                  /* We did not find any SPPLE services.                */
                  printf("No SPPLE Services found.\n");
               }

               ret_val = 0;
            }
            else
            {
               printf("Unable to parse remote services data.\n");

               ret_val = FUNCTION_ERROR;
            }

         }
         else
         {
            /* Error attempting to query Services, inform the user and  */
            /* flag an error.                                           */
            printf("DEVM_QueryRemoteDeviceServices() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         if(Connected)
            printf("Already connected.\n");
         else
            printf("Usage: ConfigureSPPLE [BD_ADDR].\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a number of     */
   /* characters to a remote device to which a connection exists.  The  */
   /* function receives a parameter that indicates the number of byte to*/
   /* be transferred.  This function will return zero on successful     */
   /* execution and a negative value on errors.                         */
static int SendDataCommand(ParameterList_t *TempParam)
{
   int ret_val;

   /* Make sure that all of the parameters required for this function   */
   /* appear to be at least semi-valid.                                 */
   if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam > 0))
   {
      /* Verify that there is a connection that is established.         */
      if(Connected)
      {
         if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
         {
            /* Check to see if we are sending to another port.             */
            if(!SendInfo.BytesToSend)
            {
                  /* Get the count of the number of bytes to send.         */
                  SendInfo.BytesToSend  = (DWord_t)TempParam->Params[0].intParam;
                  SendInfo.BytesSent    = 0;

                  /* Kick start the send process.                          */
                  SendProcess();

                  ret_val = 0;
            }
            else
            {
               printf("Send Currently in progress.\r\n");

               ret_val = FUNCTION_ERROR;
            }

            BTPS_ReleaseMutex(DeviceInfoLock);
         }
         else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         printf("No Connection Established\r\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters is/are invalid.        */
      printf("Usage: Send [Number of Bytes].\r\n");

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading data sent by a  */
   /* remote device to which a connection exists.  This function will   */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int ReadDataCommand(ParameterList_t *TempParam)
{
   Byte_t       *Ptr;
   Boolean_t     Done;
   unsigned int  Temp;

   /* Verify that there is a connection that is established.            */
   if(Connected)
   {
      if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
      {
         /* Determine the number of bytes we are going to read.         */
         Temp = DeviceInfo.ReceiveBuffer.BufferSize - DeviceInfo.ReceiveBuffer.BytesFree;

         printf("Read: %u.\r\n", Temp);

         /* Loop and read all of the data.                              */
         Done = FALSE;
         while(!Done)
         {
            /* Read the data.                                           */
            Temp = ReadData(SPPLE_DATA_BUFFER_LENGTH, SPPLEBuffer);
            if(Temp > 0)
            {
               /* Display the data.                                     */
               Ptr = SPPLEBuffer;

               while(Temp > 0)
               {
                  printf("%c", *Ptr);
                  Ptr++;
                  Temp--;
               }
            }
            else
               Done = TRUE;
         }

         BTPS_ReleaseMutex(DeviceInfoLock);
      }
   }
   else
      printf("No Connection Established\r\n");

   printf("\r\n");


   return(0);
}

   /* The following function is responsible for setting the application */
   /* state to support loopback mode.  This function will return zero on*/
   /* successful execution and a negative value on errors.              */
static int Loopback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(Connected)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         if(TempParam->Params->intParam)
            LoopbackActive = TRUE;
         else
            LoopbackActive = FALSE;
      }
      else
         LoopbackActive = (LoopbackActive?FALSE:TRUE);

      /* Finally output the current Loopback state.                     */
      printf("Current Loopback Mode set to: %s.\r\n", LoopbackActive?"ACTIVE":"INACTIVE");

      /* Flag success.                                                  */
      ret_val = 0;
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support displaying Raw Data.  This function will return  */
   /* zero on successful execution and a negative value on errors.      */
static int DisplayRawModeData(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(Connected)
   {
      /* Check to see if Loopback is active.  If it is then we will not */
      /* process this command (and we will inform the user).            */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               DisplayRawData = TRUE;
            else
               DisplayRawData = FALSE;
         }
         else
            DisplayRawData = (DisplayRawData?FALSE:TRUE);

         /* Output the current Raw Data Display Mode state.             */
         printf("Current Raw Data Display Mode set to: %s.\r\n", DisplayRawData?"ACTIVE":"INACTIVE");

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         printf("Unable to process Raw Mode Display Request when operating in Loopback Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support Automatically reading all data that is received  */
   /* through SPP.  This function will return zero on successful        */
   /* execution and a negative value on errors.                         */
static int AutomaticReadMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(Connected)
   {
      /* Check to see if Loopback is active.  If it is then we will not */
      /* process this command (and we will inform the user).            */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               AutomaticReadActive = TRUE;
            else
               AutomaticReadActive = FALSE;
         }
         else
            AutomaticReadActive = (AutomaticReadActive?FALSE:TRUE);

         /* Output the current Automatic Read Mode state.               */
         printf("Current Automatic Read Mode set to: %s.\r\n", AutomaticReadActive?"ACTIVE":"INACTIVE");

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         printf("Unable to process Automatic Read Mode Request when operating in Loopback Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for starting an advertising */
   /* process.  This function returns zero if successful and a negative */
   /* value if an error occurred.                                       */
static int StartAdvertising(ParameterList_t *TempParam)
{
   int                            ret_val;
   Byte_t                         AdvertisingBuffer[32];
   DEVM_Advertising_Information_t AdvertisingInfo;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[1].intParam))
      {
         /* Format the Advertising Information.                         */
         BTPS_MemInitialize(&AdvertisingInfo, 0, sizeof(DEVM_Advertising_Information_t));

         AdvertisingInfo.AdvertisingFlags    = TempParam->Params[0].intParam;
         AdvertisingInfo.AdvertisingDuration = TempParam->Params[1].intParam;

         FormatAdvertisingData(&AdvertisingInfo, AdvertisingBuffer);

         /* Submit the Start Advertising Command.                       */
         if((ret_val = DEVM_StartAdvertising(&AdvertisingInfo)) == 0)
         {
            printf("DEVM_StartAdvertising() Success: Duration %lu seconds.\r\n", AdvertisingInfo.AdvertisingDuration);
         }
         else
         {
            /* Error Connecting With Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_StartAdvertising() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartAdvertising [Flags] [Duration].\r\n");
         printf("where Flags is a bitmask of\r\n");
         printf("    0x00000001 = Use Public Address\r\n");
         printf("    0x00000002 = Discoverable\r\n");
         printf("    0x00000004 = Connectable\r\n");
         printf("    0x00000010 = Advertise Name\r\n");
         printf("    0x00000020 = Advertise Tx Power\r\n");
         printf("    0x00000040 = Advertise Appearance\r\n");
         printf("    0x00000080 = Use Extended Packets\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for stopping an advertising */
   /* process.  This function returns zero if successful and a negative */
   /* value if an error occurred.                                       */
static int StopAdvertising(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Submit the Stop Advertising Command.                        */
         if((ret_val = DEVM_StopAdvertising(TempParam->Params[0].intParam)) == 0)
         {
            printf("DEVM_StopAdvertising() Success.\r\n");
         }
         else
         {
            /* Error Connecting With Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_StopAdvertising() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StopAdvertising [Flags].\r\n");
         printf("where Flags bitmask of\r\n");
         printf("    0x00000001 = Force advertising stop\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* display either the entire Local Device Property information (first*/
   /* parameter is zero) or portions that have changed.                 */
static void DisplayLocalDeviceProperties(unsigned long UpdateMask, DEVM_Local_Device_Properties_t *LocalDeviceProperties)
{
   char Buffer[64];

   if(LocalDeviceProperties)
   {
      /* First, display any information that is not part of any update  */
      /* mask.                                                          */
      if(!UpdateMask)
      {
         BD_ADDRToStr(LocalDeviceProperties->BD_ADDR, Buffer);

         printf("BD_ADDR:      %s\r\n", Buffer);
         printf("HCI Ver:      0x%04X\r\n", (Word_t)LocalDeviceProperties->HCIVersion);
         printf("HCI Rev:      0x%04X\r\n", (Word_t)LocalDeviceProperties->HCIRevision);
         printf("LMP Ver:      0x%04X\r\n", (Word_t)LocalDeviceProperties->LMPVersion);
         printf("LMP Sub Ver:  0x%04X\r\n", (Word_t)LocalDeviceProperties->LMPSubVersion);
         printf("Device Man:   0x%04X (%s)\r\n", (Word_t)LocalDeviceProperties->DeviceManufacturer, DEVM_ConvertManufacturerNameToString(LocalDeviceProperties->DeviceManufacturer));
         printf("Device Flags: 0x%08lX\r\n", LocalDeviceProperties->LocalDeviceFlags);
      }
      else
      {
         if(UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS)
            printf("Device Flags: 0x%08lX\r\n", LocalDeviceProperties->LocalDeviceFlags);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_BLE_ADDRESS))
      {
         switch(LocalDeviceProperties->BLEAddressType)
         {
            case atPublic:
               printf("BLE Address Type: %s\r\n", "Public");
               break;
            case atStatic:
               printf("BLE Address Type: %s\r\n", "Static");
               break;
            case atPrivate_Resolvable:
               printf("BLE Address Type: %s\r\n", "Resolvable Random");
               break;
            case atPrivate_NonResolvable:
               printf("BLE Address Type: %s\r\n", "Non-Resolvable Random");
               break;
         }

         BD_ADDRToStr(LocalDeviceProperties->BLEBD_ADDR, Buffer);

         printf("BLE BD_ADDR:      %s\r\n", Buffer);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE))
         printf("COD:          0x%02X%02X%02X\r\n", LocalDeviceProperties->ClassOfDevice.Class_of_Device0, LocalDeviceProperties->ClassOfDevice.Class_of_Device1, LocalDeviceProperties->ClassOfDevice.Class_of_Device2);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:  %s\r\n", (LocalDeviceProperties->DeviceNameLength)?LocalDeviceProperties->DeviceName:"");

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DISCOVERABLE_MODE))
         printf("Disc. Mode:   %s, 0x%08X\r\n", LocalDeviceProperties->DiscoverableMode?"TRUE ":"FALSE", LocalDeviceProperties->DiscoverableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CONNECTABLE_MODE))
         printf("Conn. Mode:   %s, 0x%08X\r\n", LocalDeviceProperties->ConnectableMode?"TRUE ":"FALSE", LocalDeviceProperties->ConnectableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_PAIRABLE_MODE))
         printf("Pair. Mode:   %s, 0x%08X\r\n", LocalDeviceProperties->PairableMode?"TRUE ":"FALSE", LocalDeviceProperties->PairableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
      {
         printf("LE Scan Mode:    %s, 0x%08X\r\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_SCANNING_IN_PROGRESS)?"TRUE":"FALSE", LocalDeviceProperties->ScanTimeout);
         printf("LE Obsv Mode:    %s\r\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_OBSERVATION_IN_PROGRESS)?"TRUE":"FALSE");
         printf("LE Adv Mode:     %s, 0x%08X\r\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_ADVERTISING_IN_PROGRESS)?"TRUE":"FALSE", LocalDeviceProperties->AdvertisingTimeout);
         printf("LE Slv Mode:     %s\r\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_ROLE_IS_CURRENTLY_SLAVE)?"In Slave Mode":"Not in Slave Mode");
      }
   }
}

   /* The following function is a utility function that exists to       */
   /* display either the entire Remote Device Property information      */
   /* (first parameter is zero) or portions that have changed.          */
static void DisplayRemoteDeviceProperties(unsigned long UpdateMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   char          Buffer[64];
   Boolean_t     SingleMode;
   unsigned long LEFlags;

   if(RemoteDeviceProperties)
   {
      /* First, display any information that is not part of any update  */
      /* mask.                                                          */
      BD_ADDRToStr(RemoteDeviceProperties->BD_ADDR, Buffer);

      /* Determine what type of device this is.                         */
      LEFlags    = (RemoteDeviceProperties->RemoteDeviceFlags & (DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_BR_EDR | DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY));
      SingleMode = (LEFlags == DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY);

      /* Print the BR/EDR + LE Common Information.                      */
      printf("BD_ADDR:             %s\r\n", Buffer);

      if(LEFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY)
      {
         /* Print the address type.                                     */
         switch(RemoteDeviceProperties->BLEAddressType)
         {
            default:
            case atPublic:
               printf("Address Type:        %s\r\n", "Public");
               break;
            case atStatic:
               printf("Address Type:        %s\r\n", "Static");
               break;
            case atPrivate_Resolvable:
               printf("Address Type:        %s\r\n", "Resolvable Random Address.");
               break;
            case atPrivate_NonResolvable:
               printf("Address Type:        %s\r\n", "Non-resolvable Random Address.");
               break;
         }
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:         %s\r\n", (RemoteDeviceProperties->DeviceNameLength)?RemoteDeviceProperties->DeviceName:"");

      if(LEFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY)
         printf("LE Type:             %s\r\n", (!SingleMode)?"Dual Mode":"Single Mode");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
         printf("Device Flags:        0x%08lX\r\n", RemoteDeviceProperties->RemoteDeviceFlags);

      /* Print the LE Information.                                      */
      if(LEFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY)
      {
         if(((!UpdateMask) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceProperties->PriorResolvableBD_ADDR))) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS))
         {
            BD_ADDRToStr(RemoteDeviceProperties->PriorResolvableBD_ADDR, Buffer);
            printf("Resolv. BD_ADDR:     %s\r\n\r\n", Buffer);
         }

         if(((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_APPEARANCE_KNOWN)) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_APPEARANCE))
            printf("Device Appearance:   %u\r\n", RemoteDeviceProperties->DeviceAppearance);

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_RSSI))
            printf("LE RSSI:             %d\r\n", RemoteDeviceProperties->LE_RSSI);

         if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_TX_POWER_KNOWN))
            printf("LE Trans. Power:     %d\r\n", RemoteDeviceProperties->LETransmitPower);

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE))
            printf("LE Paired State :    %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE))
            printf("LE Connect State:    %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE))
            printf("LE Encrypt State:    %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
            printf("GATT Services Known: %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)?"TRUE":"FALSE");

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_OBSERVED)) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LAST_OBSERVED_KNOWN))
            printf("LE Last Observed:    %u/%u/%u %u:%u:%u.%u\r\n", RemoteDeviceProperties->BLELastObservedTime.Month, RemoteDeviceProperties->BLELastObservedTime.Day, RemoteDeviceProperties->BLELastObservedTime.Year, RemoteDeviceProperties->BLELastObservedTime.Hour, RemoteDeviceProperties->BLELastObservedTime.Minute, RemoteDeviceProperties->BLELastObservedTime.Second, RemoteDeviceProperties->BLELastObservedTime.Milliseconds);

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_ADV_PACKET)) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LAST_ADV_PACKET_KNOWN))
         {
            switch(RemoteDeviceProperties->BLELastAdvertisingPacket)
            {
               case rtConnectableUndirected:
                  printf("LE Last Adv. Packet: Connectable undirected advertisement\r\n");
                  break;
               case rtConnectableDirected:
                  printf("LE Last Adv. Packet: Connectable directed advertisement\r\n");
                  break;
               case rtScannableUndirected:
                  printf("LE Last Adv. Packet: Scannable undirected advertisement\r\n");
                  break;
               default:
               case rtNonConnectableUndirected:
                  printf("LE Last Adv. Packet: Non-connectable undirected advertisement\r\n");
                  break;
            }
         }
      }

      /* Print the BR/EDR Only information.                             */
      if(!SingleMode)
      {
         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_RSSI))
            printf("RSSI:                %d\r\n", RemoteDeviceProperties->RSSI);

         if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_TX_POWER_KNOWN))
            printf("Trans. Power:        %d\r\n", RemoteDeviceProperties->TransmitPower);

         if(((!UpdateMask) || ((UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_APPLICATION_DATA) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_APPLICATION_DATA_VALID))))
         {
            printf("Friendly Name:       %s\r\n", (RemoteDeviceProperties->ApplicationData.FriendlyNameLength)?RemoteDeviceProperties->ApplicationData.FriendlyName:"");

            printf("App. Info:   :       %08lX\r\n", RemoteDeviceProperties->ApplicationData.ApplicationInfo);
         }

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PAIRING_STATE))
            printf("Paired State :       %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CONNECTION_STATE))
            printf("Connect State:       %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_ENCRYPTION_STATE))
            printf("Encrypt State:       %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_ENCRYPTED)?"TRUE":"FALSE");

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SNIFF_STATE)))
         {
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_SNIFF_MODE)
               printf("Sniff State  :       TRUE (%u ms)\r\n", RemoteDeviceProperties->SniffInterval);
            else
               printf("Sniff State  :       FALSE\r\n");
         }

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE)))
            printf("COD:                 0x%02X%02X%02X\r\n", RemoteDeviceProperties->ClassOfDevice.Class_of_Device0, RemoteDeviceProperties->ClassOfDevice.Class_of_Device1, RemoteDeviceProperties->ClassOfDevice.Class_of_Device2);

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SERVICES_STATE))
            printf("SDP Serv. Known :    %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SERVICES_KNOWN)?"TRUE":"FALSE");
      }
   }
}

   /* The following function is a utility function that is used to      */
   /* dispay a GATT UUID.                                               */
static void DisplayGATTUUID(GATT_UUID_t *UUID, char *Prefix, unsigned int Level)
{
   if((UUID) && (Prefix))
   {
      if(UUID->UUID_Type == guUUID_16)
         printf("%*s %s: %02X%02X\r\n", (Level*INDENT_LENGTH), "", Prefix, UUID->UUID.UUID_16.UUID_Byte1, UUID->UUID.UUID_16.UUID_Byte0);
      else
      {
         if(UUID->UUID_Type == guUUID_128)
         {
            printf("%*s %s: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", Prefix, UUID->UUID.UUID_128.UUID_Byte15, UUID->UUID.UUID_128.UUID_Byte14, UUID->UUID.UUID_128.UUID_Byte13,
                                                                                               UUID->UUID.UUID_128.UUID_Byte12, UUID->UUID.UUID_128.UUID_Byte11, UUID->UUID.UUID_128.UUID_Byte10,
                                                                                               UUID->UUID.UUID_128.UUID_Byte9,  UUID->UUID.UUID_128.UUID_Byte8,  UUID->UUID.UUID_128.UUID_Byte7,
                                                                                               UUID->UUID.UUID_128.UUID_Byte6,  UUID->UUID.UUID_128.UUID_Byte5,  UUID->UUID.UUID_128.UUID_Byte4,
                                                                                               UUID->UUID.UUID_128.UUID_Byte3,  UUID->UUID.UUID_128.UUID_Byte2,  UUID->UUID.UUID_128.UUID_Byte1,
                                                                                               UUID->UUID.UUID_128.UUID_Byte0);
         }
      }
   }
}

   /* The following function is responsible for displaying the contents */
   /* of Parsed Remote Device Services Data to the display.             */
static void DisplayParsedGATTServiceData(DEVM_Parsed_Services_Data_t *ParsedGATTData)
{
   unsigned int Index;
   unsigned int Index1;
   unsigned int Index2;

   /* First, check to see if Service Records were returned.             */
   if(ParsedGATTData)
   {
      /* Print the number of GATT Services in the Record.               */
      printf("Number of Services: %u\r\n", ParsedGATTData->NumberServices);

      for(Index=0;Index<ParsedGATTData->NumberServices;Index++)
      {
         DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID), "Service UUID", 0);

         printf("%*s Start Handle: 0x%04X (%d)\r\n", (1*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle);
         printf("%*s End Handle:   0x%04X (%d)\r\n", (1*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle);

         /* Check to see if there are included services.                */
         if(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfIncludedService)
         {
            for(Index1=0;Index1<ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfIncludedService;Index1++)
            {
               DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].UUID), "Included Service UUID", 2);

               printf("%*s Start Handle: 0x%04X (%d)\r\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].Service_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].Service_Handle);
               printf("%*s End Handle:   0x%04X (%d)\r\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].End_Group_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].End_Group_Handle);
               printf("\r\n");
            }
         }

         /* Check to see if there are characteristics.                  */
         if(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics)
         {
            for(Index1=0;Index1<ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics;Index1++)
            {
               DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID), "Characteristic UUID", 2);

               printf("%*s Handle:     0x%04X (%d)\r\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle);
               printf("%*s Properties: 0x%02X\r\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Properties);

               /* Loop through the descriptors for this characteristic. */
               for(Index2=0;Index2<ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].NumberOfDescriptors;Index2++)
               {
                  if(Index2==0)
                     printf("\r\n");

                  DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_UUID), "Descriptor UUID", 3);
                  printf("%*s Handle:     0x%04X (%d)\r\n", (3*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_Handle);
               }

               if(Index2>0)
                  printf("\r\n");
            }
         }
      }
   }
   else
      printf("No GATT Service Records Found.\r\n");
}

   /* The following function is responsible for displaying the contents */
   /* of Parsed Remote Device Services Data to the display.             */
static void DisplayParsedSDPServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData)
{
   unsigned int Index;

   /* First, check to see if Service Records were returned.             */
   if((ParsedSDPData) && (ParsedSDPData->NumberServiceRecords))
   {
      /* Loop through all returned SDP Service Records.                 */
      for(Index=0;Index<ParsedSDPData->NumberServiceRecords;Index++)
      {
         /* First display the number of SDP Service Records we are      */
         /* currently processing.                                       */
         printf("Service Record: %u:\r\n", (Index + 1));

         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         DisplaySDPAttributeResponse(&(ParsedSDPData->SDPServiceAttributeResponseData[Index]), 1);
      }
   }
   else
      printf("No SDP Service Records Found.\r\n");
}

   /* The following function is responsible for Displaying the contents */
   /* of an SDP Service Attribute Response to the display.              */
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel)
{
   int Index;

   /* First, check to make sure that there were Attributes returned.    */
   if(SDPServiceAttributeResponse->Number_Attribute_Values)
   {
      /* Loop through all returned SDP Attribute Values.                */
      for(Index = 0; Index < SDPServiceAttributeResponse->Number_Attribute_Values; Index++)
      {
         /* First Print the Attribute ID that was returned.             */
         printf("%*s Attribute ID 0x%04X\r\n", (InitLevel*INDENT_LENGTH), "", SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].Attribute_ID);

         /* Now Print out all of the SDP Data Elements that were        */
         /* returned that are associated with the SDP Attribute.        */
         DisplayDataElement(SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].SDP_Data_Element, (InitLevel + 1));
      }
   }
   else
      printf("No SDP Attributes Found.\r\n");
}

   /* The following function is responsible for actually displaying an  */
   /* individual SDP Data Element to the Display.  The Level Parameter  */
   /* is used in conjunction with the defined INDENT_LENGTH constant to */
   /* make readability easier when displaying Data Element Sequences    */
   /* and Data Element Alternatives.  This function will recursively    */
   /* call itself to display the contents of Data Element Sequences and */
   /* Data Element Alternatives when it finds these Data Types (and     */
   /* increments the Indent Level accordingly).                         */
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level)
{
   unsigned int Index;
   char         Buffer[256];

   switch(SDPDataElement->SDP_Data_Element_Type)
   {
      case deNIL:
         /* Display the NIL Type.                                       */
         printf("%*s Type: NIL\r\n", (Level*INDENT_LENGTH), "");
         break;
      case deNULL:
         /* Display the NULL Type.                                      */
         printf("%*s Type: NULL\r\n", (Level*INDENT_LENGTH), "");
         break;
      case deUnsignedInteger1Byte:
         /* Display the Unsigned Integer (1 Byte) Type.                 */
         printf("%*s Type: Unsigned Int = 0x%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger1Byte);
         break;
      case deUnsignedInteger2Bytes:
         /* Display the Unsigned Integer (2 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%04X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger2Bytes);
         break;
      case deUnsignedInteger4Bytes:
         /* Display the Unsigned Integer (4 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%08X\r\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.UnsignedInteger4Bytes);
         break;
      case deUnsignedInteger8Bytes:
         /* Display the Unsigned Integer (8 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[7],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[6],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[5],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[4],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[3],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[2],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[1],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[0]);
         break;
      case deUnsignedInteger16Bytes:
         /* Display the Unsigned Integer (16 Bytes) Type.               */
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[15],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[14],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[13],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[12],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[11],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[10],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[9],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[8],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[7],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[6],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[5],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[4],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[3],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[2],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[1],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[0]);
         break;
      case deSignedInteger1Byte:
         /* Display the Signed Integer (1 Byte) Type.                   */
         printf("%*s Type: Signed Int = 0x%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger1Byte);
         break;
      case deSignedInteger2Bytes:
         /* Display the Signed Integer (2 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%04X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger2Bytes);
         break;
      case deSignedInteger4Bytes:
         /* Display the Signed Integer (4 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%08X\r\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.SignedInteger4Bytes);
         break;
      case deSignedInteger8Bytes:
         /* Display the Signed Integer (8 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[7],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[6],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[5],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[4],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[3],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[2],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[1],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[0]);
         break;
      case deSignedInteger16Bytes:
         /* Display the Signed Integer (16 Bytes) Type.                 */
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[15],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[14],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[13],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[12],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[11],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[10],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[9],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[8],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[7],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[6],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[5],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[4],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[3],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[2],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[1],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[0]);
         break;
      case deTextString:
         /* First retrieve the Length of the Text String so that we can */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the Text String into the Buffer and then NULL terminate*/
         /* it.                                                         */
         memcpy(Buffer, SDPDataElement->SDP_Data_Element.TextString, Index);
         Buffer[Index] = '\0';

         printf("%*s Type: Text String = %s\r\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deBoolean:
         printf("%*s Type: Boolean = %s\r\n", (Level*INDENT_LENGTH), "", (SDPDataElement->SDP_Data_Element.Boolean)?"TRUE":"FALSE");
         break;
      case deURL:
         /* First retrieve the Length of the URL String so that we can  */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the URL String into the Buffer and then NULL terminate */
         /* it.                                                         */
         memcpy(Buffer, SDPDataElement->SDP_Data_Element.URL, Index);
         Buffer[Index] = '\0';

         printf("%*s Type: URL = %s\r\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deUUID_16:
         printf("%*s Type: UUID_16 = 0x%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte0,
                                                                                 SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte1);
         break;
      case deUUID_32:
         printf("%*s Type: UUID_32 = 0x%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte0,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte1,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte2,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte3);
         break;
      case deUUID_128:
         printf("%*s Type: UUID_128 = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte0,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte1,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte2,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte3,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte4,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte5,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte6,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte7,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte8,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte9,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte10,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte11,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte12,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte13,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte14,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte15);
         break;
      case deSequence:
         /* Display that this is a SDP Data Element Sequence.           */
         printf("%*s Type: Data Element Sequence\r\n", (Level*INDENT_LENGTH), "");

         /* Loop through each of the SDP Data Elements in the SDP Data  */
         /* Element Sequence.                                           */
         for(Index = 0; Index < SDPDataElement->SDP_Data_Element_Length; Index++)
         {
            /* Call this function again for each of the SDP Data        */
            /* Elements in this SDP Data Element Sequence.              */
            DisplayDataElement(&(SDPDataElement->SDP_Data_Element.SDP_Data_Element_Sequence[Index]), (Level + 1));
         }
         break;
      case deAlternative:
         /* Display that this is a SDP Data Element Alternative.        */
         printf("%*s Type: Data Element Alternative\r\n", (Level*INDENT_LENGTH), "");

         /* Loop through each of the SDP Data Elements in the SDP Data  */
         /* Element Alternative.                                        */
         for(Index = 0; Index < SDPDataElement->SDP_Data_Element_Length; Index++)
         {
            /* Call this function again for each of the SDP Data        */
            /* Elements in this SDP Data Element Alternative.           */
            DisplayDataElement(&(SDPDataElement->SDP_Data_Element.SDP_Data_Element_Alternative[Index]), (Level + 1));
         }
         break;
      default:
         printf("%*s Unknown SDP Data Element Type\r\n", (Level*INDENT_LENGTH), "");
         break;
   }
}

   /* The following function is the Callback function that is installed */
   /* to be notified when any local IPC connection to the server has    */
   /* been lost.  This case only occurs when the Server exits.  This    */
   /* callback allows the application mechanism to be notified so that  */
   /* all resources can be cleaned up (i.e.  call BTPM_Cleanup().       */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter)
{
   printf("Server has been Un-Registered.\r\n");

   printf("SPPMLE>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the Device Manager Event Callback       */
   /* function that is Registered with the Device Manager.  This        */
   /* callback is responsible for processing all Device Manager Events. */
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter)
{
   char Buffer[32];

   if(EventData)
   {
      printf("\r\n");

      switch(EventData->EventType)
      {
         case detDevicePoweredOn:
            printf("Device Powered On.\r\n");
            break;
         case detDevicePoweringOff:
            printf("Device Powering Off Event, Timeout: 0x%08X.\r\n", EventData->EventData.DevicePoweringOffEventData.PoweringOffTimeout);
            break;
         case detDevicePoweredOff:
            printf("Device Powered Off.\r\n");
            break;
         case detLocalDevicePropertiesChanged:
            printf("Local Device Properties Changed.\r\n");

            DisplayLocalDeviceProperties(EventData->EventData.LocalDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.LocalDevicePropertiesChangedEventData.LocalDeviceProperties));
            break;
         case detDeviceScanStarted:
            printf("LE Device Discovery Started.\r\n");
            break;
         case detDeviceScanStopped:
            printf("LE Device Discovery Stopped.\r\n");
            break;
         case detDeviceObservationScanStarted:
            printf("Device Observation Scan Started: Flags 0x%08lX.\r\n", EventData->EventData.ObservationScanStartedEventData.ObservationScanFlags);
            break;
         case detDeviceObservationScanStopped:
            printf("Device Observation Scan Stopped: Flags 0x%08lX.\r\n", EventData->EventData.ObservationScanStoppedEventData.ObservationScanFlags);
            break;
         case detDeviceAdvertisingStarted:
            printf("LE Advertising Started.\r\n");
            break;
         case detDeviceAdvertisingStopped:
            printf("LE Advertising Stopped.\r\n");
            break;
         case detAdvertisingTimeout:
            printf("LE Advertising Timeout.\r\n");
            break;
         case detDeviceDiscoveryStarted:
            printf("Device Discovery Started.\r\n");
            break;
         case detDeviceDiscoveryStopped:
            printf("Device Discovery Stopped.\r\n");
            break;
         case detRemoteDeviceFound:
            printf("Remote Device Found.\r\n");

            DisplayRemoteDeviceProperties(0, &(EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties));
            break;
         case detRemoteDeviceDeleted:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Deleted: %s.\r\n", Buffer);
            break;
         case detRemoteDevicePropertiesChanged:
            printf("Remote Device Properties Changed.\r\n");

            DisplayRemoteDeviceProperties(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));

            /* If the remote device has become disconnected, note it.   */
            if((EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE) && (!(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)) && (COMPARE_BD_ADDR(ConnectionBD_ADDR, EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties.BD_ADDR)))
               Connected = FALSE;
            break;
         case detRemoteDeviceAddressChanged:
            printf("Remote Device Address Changed.\r\n");

            BD_ADDRToStr(EventData->EventData.RemoteDeviceAddressChangeEventData.RemoteDeviceAddress, Buffer);
            printf("Remote Device Address:          %s.\r\n", Buffer);
            BD_ADDRToStr(EventData->EventData.RemoteDeviceAddressChangeEventData.PreviousRemoteDeviceAddress, Buffer);
            printf("Previous Remote Device Address: %s.\r\n", Buffer);
            break;
         case detRemoteDevicePropertiesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties.BD_ADDR, Buffer);

            printf("Remote Device Properties Status: %s, %s.\r\n", Buffer, EventData->EventData.RemoteDevicePropertiesStatusEventData.Success?"SUCCESS":"FAILURE");

            DisplayRemoteDeviceProperties(0, &(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties));
            break;
         case detRemoteDeviceServicesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceServicesStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device %s Services Status: %s, %s.\r\n", Buffer, (EventData->EventData.RemoteDeviceServicesStatusEventData.StatusFlags & DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_LOW_ENERGY)?"LE":"BR/EDR", (EventData->EventData.RemoteDeviceServicesStatusEventData.StatusFlags & DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_SUCCESS)?"SUCCESS":"FAILURE");
            break;
         case detRemoteDevicePairingStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePairingStatusEventData.RemoteDeviceAddress, Buffer);

            printf("%s Remote Device Pairing Status: %s, %s (0x%02X)\r\n", ((EventData->EventData.RemoteDevicePairingStatusEventData.AuthenticationStatus & DEVM_REMOTE_DEVICE_PAIRING_STATUS_FLAGS_LOW_ENERGY)?"LE":"BR/EDR"), Buffer, (EventData->EventData.RemoteDevicePairingStatusEventData.Success)?"SUCCESS":"FAILURE", EventData->EventData.RemoteDevicePairingStatusEventData.AuthenticationStatus);
            break;
         case detRemoteDeviceAuthenticationStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Authentication Status: %s, %d (%s)\r\n", Buffer, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status, (EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status):"SUCCESS");
            break;
         case detRemoteDeviceEncryptionStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Encryption Status: %s, %d (%s)\r\n", Buffer, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status, (EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status):"SUCCESS");
            break;
         case detRemoteDeviceConnectionStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Connection Status: %s, %d (%s)\r\n", Buffer, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status, (EventData->EventData.RemoteDeviceConnectionStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceConnectionStatusEventData.Status):"SUCCESS");
            break;
         default:
            printf("Unknown Device Manager Event Received: 0x%08X, Length: 0x%08X.\r\n", (unsigned int)EventData->EventType, EventData->EventLength);
            break;
      }
   }
   else
      printf("\r\nDEVM Event Data is NULL.\r\n");

   printf("SPPMLE>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the Device Manager Authentication Event */
   /* Callback function that is Registered with the Device Manager.     */
   /* This callback is responsible for processing all Device Manager    */
   /* Authentication Request Events.                                    */
static void BTPSAPI DEVM_Authentication_Callback(DEVM_Authentication_Information_t *AuthenticationRequestInformation, void *CallbackParameter)
{
   int                               Result;
   char                              Buffer[32];
   Boolean_t                         LowEnergy;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   if(AuthenticationRequestInformation)
   {
      printf("\r\n");

      BD_ADDRToStr(AuthenticationRequestInformation->BD_ADDR, Buffer);

      printf("Authentication Request received for %s.\r\n", Buffer);

      /* Check to see if this is an LE event.                           */
      if(AuthenticationRequestInformation->AuthenticationAction & DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK)
      {
         AuthenticationRequestInformation->AuthenticationAction &= ~DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;

         LowEnergy = TRUE;
      }
      else
         LowEnergy = FALSE;

      switch(AuthenticationRequestInformation->AuthenticationAction)
      {
         case DEVM_AUTHENTICATION_ACTION_PIN_CODE_REQUEST:
            printf("PIN Code Request.\r\n.");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* PIN Code.                                                */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            /* Inform the user that they will need to respond with a PIN*/
            /* Code Response.                                           */
            printf("\r\nRespond with the command: PINCodeResponse\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST:
            printf("User Confirmation Request %s.\r\n", (LowEnergy?"LE":"BR/EDR"));

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* User Confirmation.                                       */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;
            CurrentLowEnergy     = FALSE;

            if(!LowEnergy)
            {
               if(IOCapability != icDisplayYesNo)
               {
                  /* Invoke Just works.                                 */

                  printf("\r\nAuto Accepting: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

                  BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

                  AuthenticationResponseInformation.BD_ADDR                         = AuthenticationRequestInformation->BD_ADDR;
                  AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE;
                  AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

                  AuthenticationResponseInformation.AuthenticationData.Confirmation = (Boolean_t)TRUE;

                  if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
                     printf("DEVM_AuthenticationResponse() Success.\r\n");
                  else
                     printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               }
               else
               {
                  printf("User Confirmation: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

                  /* Inform the user that they will need to respond with*/
                  /* a PIN Code Response.                               */
                  printf("\r\nRespond with the command: UserConfirmationResponse\r\n");
               }
            }
            else
            {
               /* Flag that this is LE Pairing.                         */
               CurrentLowEnergy = TRUE;

               /* Inform the user that they will need to respond with a */
               /* PIN Code Response.                                    */
               printf("\tSecure Connections: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
               printf("\tJust Works Pairing: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");

               if(AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_JUST_WORKS_PAIRING)
               {
                  /* Just respond for just works pairing.               */
                  printf("\r\nInvoking Just Works\r\n");

                  AuthenticationResponseInformation.BD_ADDR                         = AuthenticationRequestInformation->BD_ADDR;
                  AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE | DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;
                  AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

                  AuthenticationResponseInformation.AuthenticationData.Confirmation = TRUE;

                  if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
                     printf("DEVM_AuthenticationResponse() Success.\r\n");
                  else
                     printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               }
               else
               {
                  if(IOCapability != icNoInputNoOutput)
                  {
                     printf("\r\nRespond with the command: UserConfirmationResponse\r\n");
                     if(AuthenticationRequestInformation->AuthenticationDataLength > 0)
                        printf("\tConfirm Value:      %u.\r\n", AuthenticationRequestInformation->AuthenticationData.UserConfirmationRequestData.Passkey);
                  }
                  else
                  {
                     /* Just respond for just works pairing.            */
                     printf("\r\nInvoking Just Works\r\n");

                     AuthenticationResponseInformation.BD_ADDR                         = AuthenticationRequestInformation->BD_ADDR;
                     AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE | DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;
                     AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

                     AuthenticationResponseInformation.AuthenticationData.Confirmation = TRUE;

                     if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
                        printf("DEVM_AuthenticationResponse() Success.\r\n");
                     else
                        printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                     /* Flag that there is no longer a current          */
                     /* Authentication procedure in progress.           */
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  }
               }
            }
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_REQUEST:
            printf("Passkey Request %s.\r\n", (LowEnergy?"LE":"BR/EDR"));
            printf("\tSecure Connections: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
            printf("\tJust Works Pairing: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* Passkey.                                                 */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;
            CurrentLowEnergy     = LowEnergy;

            /* Inform the user that they will need to respond with a    */
            /* Passkey Response.                                        */
            printf("\r\nRespond with the command: PassKeyResponse\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION:
            printf("PassKey Indication %s.\r\n", (LowEnergy?"LE":"BR/EDR"));
            printf("\tSecure Connections: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
            printf("\tJust Works Pairing: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");

            printf("PassKey: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);
            break;
         case DEVM_AUTHENTICATION_ACTION_KEYPRESS_INDICATION:
            printf("Keypress Indication.\r\n");

            printf("Keypress: %d\r\n", (int)AuthenticationRequestInformation->AuthenticationData.Keypress);
            break;
         case DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST:
            printf("Out of Band Data Request: %s.\r\n", (LowEnergy?"LE":"BR/EDR"));

            /* This application does not support OOB data so respond    */
            /* with a data length of Zero to force a negative reply.    */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                  = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction     = DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength = 0;

            if(LowEnergy)
               AuthenticationResponseInformation.AuthenticationAction |= DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;

            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\r\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST:
            printf("I/O Capability Request: %s.\r\n", (LowEnergy?"LE":"BR/EDR"));

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* Passkey.                                                 */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;
            CurrentLowEnergy     = LowEnergy;

            /* Respond with the currently configured I/O Capabilities.  */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR              = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction = DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE;

            if(!CurrentLowEnergy)
            {
               AuthenticationResponseInformation.AuthenticationDataLength                                    = sizeof(AuthenticationResponseInformation.AuthenticationData.IOCapabilities);

               AuthenticationResponseInformation.AuthenticationData.IOCapabilities.IO_Capability             = (GAP_IO_Capability_t)IOCapability;
//xxx Check if MITM should be set
               AuthenticationResponseInformation.AuthenticationData.IOCapabilities.MITM_Protection_Required  = MITMProtection;
               AuthenticationResponseInformation.AuthenticationData.IOCapabilities.OOB_Data_Present          = OOBSupport;

//xxx Here check if this will default to "just works" or auto accept.
            }
            else
            {
               AuthenticationResponseInformation.AuthenticationAction                                       |= DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;
               AuthenticationResponseInformation.AuthenticationDataLength                                    = sizeof(AuthenticationResponseInformation.AuthenticationData.LEIOCapabilities);
               AuthenticationResponseInformation.AuthenticationData.LEIOCapabilities.IO_Capability           = (GAP_LE_IO_Capability_t)IOCapability;
               AuthenticationResponseInformation.AuthenticationData.LEIOCapabilities.Bonding_Type            = lbtBonding;
               AuthenticationResponseInformation.AuthenticationData.LEIOCapabilities.MITM                    = MITMProtection;
               AuthenticationResponseInformation.AuthenticationData.LEIOCapabilities.OOB_Present             = OOBSupport;
            }

            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\r\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE:
            printf("I/O Capability Response.\r\n");

            /* Inform the user of the Remote I/O Capablities.           */
            printf("Remote I/O Capabilities: %s, MITM Protection: %s.\r\n", IOCapabilitiesStrings[AuthenticationRequestInformation->AuthenticationData.IOCapabilities.IO_Capability], AuthenticationRequestInformation->AuthenticationData.IOCapabilities.MITM_Protection_Required?"TRUE":"FALSE");
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_STATUS_RESULT:
            printf("Authentication Status: .\r\n");

            printf("Status: %d\r\n", AuthenticationRequestInformation->AuthenticationData.AuthenticationStatus);
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_START:
            printf("Authentication Start.\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_END:
            printf("Authentication End.\r\n");
            break;
         default:
            printf("Unknown Device Manager Authentication Event Received: 0x%08X, Length: 0x%08X.\r\n", (unsigned int)AuthenticationRequestInformation->AuthenticationAction, AuthenticationRequestInformation->AuthenticationDataLength);
            break;
      }
   }
   else
      printf("\r\nDEVM Authentication Request Data is NULL.\r\n");

   printf("SPPMLE>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the GATM Event Callback function that is*/
   /* Registered with the Generic Attribute Profile Manager.  This      */
   /* callback is responsible for processing all GATM Events.           */
static void BTPSAPI GATM_Event_Callback(GATM_Event_Data_t *EventData, void *CallbackParameter)
{
   char      Buffer[128];
   Boolean_t DisplayPrompt;

   /* Flag we want to show the prompt after this event.                 */
   DisplayPrompt = TRUE;

   if(EventData)
   {
      switch(EventData->EventType)
      {
         /* GATM Connection Events.                                     */
         case getGATTConnected:
            printf("\r\nGATT Connection\r\n");

            BD_ADDRToStr(EventData->EventData.ConnectedEventData.RemoteDeviceAddress, Buffer);

            printf("    Connection Type: %s\r\n", (EventData->EventData.ConnectedEventData.ConnectionType == gctLE)?"LE":"BR/EDR");
            printf("    Remote Address:  %s\r\n", Buffer);
            printf("    MTU:             %u\r\n", EventData->EventData.ConnectedEventData.MTU);
            break;
         case getGATTDisconnected:
            printf("\r\nGATT Disconnect\r\n");

            BD_ADDRToStr(EventData->EventData.DisconnectedEventData.RemoteDeviceAddress, Buffer);

            printf("    Connection Type: %s\r\n", (EventData->EventData.DisconnectedEventData.ConnectionType == gctLE)?"LE":"BR/EDR");
            printf("    Remote Address:  %s\r\n", Buffer);

            /* Check to see if this matches the device that we are      */
            /* connected to.                                            */
            if(COMPARE_BD_ADDR(ConnectionBD_ADDR, EventData->EventData.DisconnectedEventData.RemoteDeviceAddress))
            {
               Connected = FALSE;
               if(BTPS_WaitMutex(DeviceInfoLock, 500))
               {
                  printf("Erase Device Info\n");
                  BTPS_MemInitialize(&DeviceInfo, 0, sizeof(DeviceInfo));
                  BTPS_MemInitialize(&SendInfo, 0, sizeof(SendInfo));
                  BTPS_MemInitialize(SPPLEBuffer, 0, sizeof(SPPLEBuffer));

                  BTPS_ReleaseMutex(DeviceInfoLock);
               }
            }
            break;
         case getGATTConnectionMTUUpdate:
            printf("\r\nGATT Connection MTU Update\r\n");

            BD_ADDRToStr(EventData->EventData.ConnectionMTUUpdateEventData.RemoteDeviceAddress, Buffer);

            printf("    Connection Type: %s\r\n", (EventData->EventData.ConnectionMTUUpdateEventData.ConnectionType == gctLE)?"LE":"BR/EDR");
            printf("    Remote Address:  %s\r\n", Buffer);
            printf("    New MTU:         %u\r\n", EventData->EventData.ConnectionMTUUpdateEventData.MTU);
            break;
         case getGATTHandleValueData:
            /* If this is a credit event, handle it.                    */
            if(COMPARE_BD_ADDR(ConnectionBD_ADDR, EventData->EventData.HandleValueDataEventData.RemoteDeviceAddress))
            {
               if(BTPS_WaitMutex(DeviceInfoLock, BTPS_INFINITE_WAIT))
               {
                  if(EventData->EventData.HandleValueDataEventData.AttributeHandle == DeviceInfo.RxCreditsHandle)
                     ReceiveCreditEvent(READ_UNALIGNED_WORD_LITTLE_ENDIAN(EventData->EventData.HandleValueDataEventData.AttributeValue));
                  else
                  {
                     if(EventData->EventData.HandleValueDataEventData.AttributeHandle == DeviceInfo.TransmitHandle)
                        DataIndicationEvent(EventData->EventData.HandleValueDataEventData.AttributeValueLength, EventData->EventData.HandleValueDataEventData.AttributeValue);
                  }

                  BTPS_ReleaseMutex(DeviceInfoLock);
               }
            }

            /* We do not need to show a prompt after this event.        */
            DisplayPrompt = FALSE;

            break;

         /* GATM Client Events.                                         */
         case getGATTReadResponse:
            /* We should never need to read in this module.             */
            DisplayPrompt = FALSE;
            break;
         case getGATTWriteResponse:
            /* We always send without response in this module.          */
            DisplayPrompt = FALSE;
            break;
         case getGATTErrorResponse:
            printf("\r\nGATT Error Response Data\r\n");

            BD_ADDRToStr(EventData->EventData.ErrorResponseEventData.RemoteDeviceAddress, Buffer);

            printf("    Connection Type:  %s\r\n", (EventData->EventData.ErrorResponseEventData.ConnectionType == gctLE)?"LE":"BR/EDR");
            printf("    Remote Address:   %s\r\n", Buffer);
            printf("    Transaction ID:   %u\r\n", EventData->EventData.ErrorResponseEventData.TransactionID);
            printf("    Attribute Handle: 0x%04X (%u)\r\n", EventData->EventData.ErrorResponseEventData.Handle, EventData->EventData.ErrorResponseEventData.Handle);
            switch(EventData->EventData.ErrorResponseEventData.ErrorType)
            {
               case retErrorResponse:
                  if(EventData->EventData.ErrorResponseEventData.AttributeProtocolErrorCode < NUMBER_OF_ERROR_CODES)
                     printf("    Error Reason:     %s.\r\n", ErrorCodeStr[EventData->EventData.ErrorResponseEventData.AttributeProtocolErrorCode]);
                  else
                     printf("    Error Code:       0x%02X.\r\n", EventData->EventData.ErrorResponseEventData.AttributeProtocolErrorCode);
                  break;
               case retProtocolTimeout:
                  printf("    Error Type:       ProtocolTimeout.\r\n");
                  break;
               case retPrepareWriteDataMismatch:
                  printf("    Error Type:       Prepare Write Data Mismatch.\r\n");
                  break;
            }
            break;
         default:
            printf("\r\nUnhandled event.\r\n");
            break;
      }

      if(DisplayPrompt)
         puts("\r\n");
   }
   else
      printf("\r\nGATM Event Data is NULL.\r\n");

   if(DisplayPrompt)
   {
      printf("SPPMLE>");

      /* Make sure the output is displayed to the user.                 */
      fflush(stdout);
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   /* Initialize the default Secure Simple Pairing parameters.          */
   IOCapability     = DEFAULT_IO_CAPABILITY;
   OOBSupport       = FALSE;
   MITMProtection   = DEFAULT_MITM_PROTECTION;

   DeviceInfoLock = BTPS_CreateMutex(FALSE);

   /* Nothing really to do here aside from running the main application */
   /* code.                                                             */
   UserInterface();

   BTPS_CloseMutex(DeviceInfoLock);
   DeviceInfoLock = NULL;

   return 0;
}


/*****< linuxdevm.h >**********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXDEVM - Simple Linux application using Bluetopia Platform Manager     */
/*              Device Manager Application Programming (API) Interface.       */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/11/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>        /* Include for getpid().                           */

#include "LinuxANTM.h"     /* Main Application Prototypes and Constants.      */

#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#include "SS1BTANTM.h"     /* ANT+ Manager Application Programming Interface. */

#define MAX_SUPPORTED_COMMANDS                    (104)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                        (512)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                     (256)  /* Denotes the max   */
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

static unsigned int        AuthenticationCallbackID;/* Variable which holds the        */
                                                    /* current Authentication Callback */
                                                    /* ID that is assigned from the    */
                                                    /* Device Manager when the local   */
                                                    /* client registers for            */
                                                    /* Authentication.                 */
static unsigned int        ANTMEventCallbackID;
static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
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

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
} ;

   /* The following string table is used to map the API Bonding Type    */
   /* values to an easily displayable string.                           */
static char *BondingStrings[] =
{
   "None",
   "Dedicated",
   "General"
} ;

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

static int DisplayHelp(ParameterList_t *TempParam);

static int Initialize(ParameterList_t *TempParam);
static int Cleanup(ParameterList_t *TempParam);
static int RegisterEventCallback(ParameterList_t *TempParam);
static int UnRegisterEventCallback(ParameterList_t *TempParam);
static int SetDevicePower(ParameterList_t *TempParam);
static int QueryDevicePower(ParameterList_t *TempParam);
static int SetLocalRemoteDebugZoneMask(ParameterList_t *TempParam);
static int QueryLocalRemoteDebugZoneMask(ParameterList_t *TempParam);
static int SetDebugZoneMaskPID(ParameterList_t *TempParam);
static int ShutdownService(ParameterList_t *TempParam);
static int QueryLocalDeviceProperties(ParameterList_t *TempParam);
static int SetLocalDeviceName(ParameterList_t *TempParam);
static int SetLocalClassOfDevice(ParameterList_t *TempParam);
static int SetDiscoverable(ParameterList_t *TempParam);
static int SetConnectable(ParameterList_t *TempParam);
static int SetPairable(ParameterList_t *TempParam);
static int StartDeviceDiscovery(ParameterList_t *TempParam);
static int StopDeviceDiscovery(ParameterList_t *TempParam);
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
static int EnableLocalDeviceFeature(ParameterList_t *TempParam);
static int DisableLocalDeviceFeature(ParameterList_t *TempParam);
static int QueryActiveLocalDeviceFeatures(ParameterList_t *TempParam);
static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);
static int SendRawHCICommand(ParameterList_t *TempParam);

static int ANTMRegisterServerEventCallback(ParameterList_t *TempParam);
static int ANTMUnRegisterServerEventCallback(ParameterList_t *TempParam);
static int AssignChannel(ParameterList_t *TempParam);
static int UnassignChannel(ParameterList_t *TempParam);
static int SetChannelID(ParameterList_t *TempParam);
static int SetChannelPeriod(ParameterList_t *TempParam);
static int SetChannelSearchTimeout(ParameterList_t *TempParam);
static int SetChannelRFFrequency(ParameterList_t *TempParam);
static int SetNetworkKey(ParameterList_t *TempParam);
static int SetTransmitPower(ParameterList_t *TempParam);
static int AddChannelID(ParameterList_t *TempParam);
static int ConfigureInclusionExclusionList(ParameterList_t *TempParam);
static int SetChannelTransmitPower(ParameterList_t *TempParam);
static int SetLowPriorityChannelSearchTimeout(ParameterList_t *TempParam);
static int SetSerialNumberChannelID(ParameterList_t *TempParam);
static int EnableExtendedMessages(ParameterList_t *TempParam);
static int EnableLED(ParameterList_t *TempParam);
static int EnableCrystal(ParameterList_t *TempParam);
static int ConfigureExtendedMessages(ParameterList_t *TempParam);
static int ConfigureFrequencyAgility(ParameterList_t *TempParam);
static int SetProximitySearch(ParameterList_t *TempParam);
static int SetChannelSearchPriority(ParameterList_t *TempParam);
static int SetUSBDescriptorString(ParameterList_t *TempParam);
static int ResetSystem(ParameterList_t *TempParam);
static int OpenChannel(ParameterList_t *TempParam);
static int CloseChannel(ParameterList_t *TempParam);
static int RequestMessage(ParameterList_t *TempParam);
static int OpenRxScanMode(ParameterList_t *TempParam);
static int SleepMessage(ParameterList_t *TempParam);
static int SendBroadcastData(ParameterList_t *TempParam);
static int SendAcknowledgedData(ParameterList_t *TempParam);
static int SendBurstTransferData(ParameterList_t *TempParam);
static int InitializeCWTestMode(ParameterList_t *TempParam);
static int SetCWTestMode(ParameterList_t *TempParam);
static int SendRawPacket(ParameterList_t *TempParam);
static int SendRawPacketAsync(ParameterList_t *TempParam);

static void DisplayParsedServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData);
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

   /* BTPM ANT+ Manager Callback function prototype.                    */
static void BTPSAPI ANTM_Event_Callback(ANTM_Event_Data_t *EventData, void *CallbackParameter);

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
   AddCommand("SETDEBUGZONEMASKPID", SetDebugZoneMaskPID);
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
   AddCommand("ENABLEFEATURE", EnableLocalDeviceFeature);
   AddCommand("DISABLEFEATURE", DisableLocalDeviceFeature);
   AddCommand("QUERYACTIVEFEATURES", QueryActiveLocalDeviceFeatures);
   AddCommand("SENDRAWHCICOMMAND", SendRawHCICommand);

   AddCommand("REGISTERSERVEREVENTCALLBACK", ANTMRegisterServerEventCallback);
   AddCommand("UNREGISTERSERVEREVENTCALLBACK", ANTMUnRegisterServerEventCallback);
   AddCommand("ASSIGNCHANNEL", AssignChannel);
   AddCommand("UNASSIGNCHANNEL", UnassignChannel);
   AddCommand("SETCHANNELID", SetChannelID);
   AddCommand("SETCHANNELPERIOD", SetChannelPeriod);
   AddCommand("SETCHANNELSEARCHTIMEOUT", SetChannelSearchTimeout);
   AddCommand("SETCHANNELRFFREQUENCY", SetChannelRFFrequency);
   AddCommand("SETNETWORKKEY", SetNetworkKey);
   AddCommand("SETTRANSMITPOWER", SetTransmitPower);
   AddCommand("ADDCHANNELID", AddChannelID);
   AddCommand("CONFIGUREINCLUSIONEXCLUSIONLIST", ConfigureInclusionExclusionList);
   AddCommand("SETCHANNELTRANSMITPOWER", SetChannelTransmitPower);
   AddCommand("SETLOWPRIORITYCHANNELSEARCHTIMEOUT", SetLowPriorityChannelSearchTimeout);
   AddCommand("SETSERIALNUMBERCHANNELID", SetSerialNumberChannelID);
   AddCommand("ENABLEEXTENDEDMESSAGES", EnableExtendedMessages);
   AddCommand("ENABLELED", EnableLED);
   AddCommand("ENABLECRYSTAL", EnableCrystal);
   AddCommand("CONFIGUREEXTENDEDMESSAGES", ConfigureExtendedMessages);
   AddCommand("CONFIGUREFREQUENCYAGILITY", ConfigureFrequencyAgility);
   AddCommand("SETPROXIMITYSEARCH", SetProximitySearch);
   AddCommand("SETCHANNELSEARCHPRIORITY", SetChannelSearchPriority);
   AddCommand("SETUSBDESCRIPTORSTRING", SetUSBDescriptorString);
   AddCommand("RESETSYSTEM", ResetSystem);
   AddCommand("OPENCHANNEL", OpenChannel);
   AddCommand("CLOSECHANNEL", CloseChannel);
   AddCommand("REQUESTMESSAGE", RequestMessage);
   AddCommand("OPENRXSCANMODE", OpenRxScanMode);
   AddCommand("SLEEPMESSAGE", SleepMessage);
   AddCommand("SENDBROADCASTDATA", SendBroadcastData);
   AddCommand("SENDACKNOWLEDGEDDATA", SendAcknowledgedData);
   AddCommand("SENDBURSTTRANSFERDATA", SendBurstTransferData);
   AddCommand("INITALIZECWTESTMODE", InitializeCWTestMode);
   AddCommand("SETCWTESTMODE", SetCWTestMode);
   AddCommand("SENDRAWPACKET", SendRawPacket);
   AddCommand("SENDRAWPACKETASYNC", SendRawPacketAsync);

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
      printf("ANTM>");

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
   /* parsing them into a command, param1, and param2.  After parsing   */
   /* this string the data is stored into a UserCommand_t structure to  */
   /* be used by the interpreter.  The first parameter of this function */
   /* is the structure used to pass the parsed command string out of the*/
   /* function.  The second parameter of this function is the string    */
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
   printf("*                  5) SetDebugZoneMaskPID                        *\r\n");
   printf("*                  6) ShutdownService                            *\r\n");
   printf("*                  7) RegisterEventCallback,                     *\r\n");
   printf("*                  8) UnRegisterEventCallback,                   *\r\n");
   printf("*                  9) QueryDevicePower                           *\r\n");
   printf("*                  10)SetDevicePower                             *\r\n");
   printf("*                  11)QueryLocalDeviceProperties                 *\r\n");
   printf("*                  12)SetLocalDeviceName                         *\r\n");
   printf("*                  13)SetLocalClassOfDevice                      *\r\n");
   printf("*                  14)SetDiscoverable                            *\r\n");
   printf("*                  15)SetConnectable                             *\r\n");
   printf("*                  16)SetPairable                                *\r\n");
   printf("*                  17)StartDeviceDiscovery                       *\r\n");
   printf("*                  18)StopDeviceDiscovery                        *\r\n");
   printf("*                  19)QueryRemoteDeviceList                      *\r\n");
   printf("*                  20)QueryRemoteDeviceProperties                *\r\n");
   printf("*                  21)AddRemoteDevice                            *\r\n");
   printf("*                  22)DeleteRemoteDevice                         *\r\n");
   printf("*                  23)UpdateRemoteDeviceAppData                  *\r\n");
   printf("*                  24)DeleteRemoteDevices                        *\r\n");
   printf("*                  25)PairWithRemoteDevice                       *\r\n");
   printf("*                  26)CancelPairWithRemoteDevice                 *\r\n");
   printf("*                  27)UnPairRemoteDevice                         *\r\n");
   printf("*                  28)QueryRemoteDeviceServices                  *\r\n");
   printf("*                  29)QueryRemoteDeviceServiceSupported          *\r\n");
   printf("*                  30)QueryRemoteDevicesForService               *\r\n");
   printf("*                  31)QueryRemoteDeviceServiceClasses            *\r\n");
   printf("*                  32)AuthenticateRemoteDevice                   *\r\n");
   printf("*                  33)EncryptRemoteDevice                        *\r\n");
   printf("*                  34)ConnectWithRemoteDevice                    *\r\n");
   printf("*                  35)DisconnectRemoteDevice                     *\r\n");
   printf("*                  36)SetRemoteDeviceLinkActive                  *\r\n");
   printf("*                  37)CreateSDPRecord                            *\r\n");
   printf("*                  38)DeleteSDPRecord                            *\r\n");
   printf("*                  39)AddSDPAttribute                            *\r\n");
   printf("*                  40)DeleteSDPAttribute                         *\r\n");
   printf("*                  41)EnableBluetoothDebug                       *\r\n");
   printf("*                  42)RegisterAuthentication                     *\r\n");
   printf("*                  43)UnRegisterAuthentication                   *\r\n");
   printf("*                  44)PINCodeResponse                            *\r\n");
   printf("*                  45)PassKeyResponse                            *\r\n");
   printf("*                  46)UserConfirmationResponse                   *\r\n");
   printf("*                  47)ChangeSimplePairingParameters              *\r\n");
   printf("*                  48)EnableFeature                              *\r\n");
   printf("*                  49)DisableFeature                             *\r\n");
   printf("*                  50)QueryActiveFeatures                        *\r\n");
   printf("*                  51)SendRawHCICommand                          *\r\n");
   printf("*                  52)RegisterServerEventCallback                *\r\n");
   printf("*                  53)UnRegisterServerEventCallback              *\r\n");
   printf("*                  54)AssignChannel                              *\r\n");
   printf("*                  55)UnassignChannel                            *\r\n");
   printf("*                  56)SetChannelID                               *\r\n");
   printf("*                  57)SetChannelPeriod                           *\r\n");
   printf("*                  58)SetChannelSearchTimeout                    *\r\n");
   printf("*                  59)SetChannelRFFrequency                      *\r\n");
   printf("*                  60)SetNetworkKey                              *\r\n");
   printf("*                  61)SetTransmitPower                           *\r\n");
   printf("*                  62)AddChannelID                               *\r\n");
   printf("*                  63)ConfigureInclusionExclusionList            *\r\n");
   printf("*                  64)SetChannelTransmitPower                    *\r\n");
   printf("*                  65)SetLowPriorityChannelSearchTimeout         *\r\n");
   printf("*                  66)SetSerialNumberChannelID                   *\r\n");
   printf("*                  67)EnableExtendedMessages                     *\r\n");
   printf("*                  68)EnableLED                                  *\r\n");
   printf("*                  69)EnableCrystal                              *\r\n");
   printf("*                  70)ConfigureExtendedMessages                  *\r\n");
   printf("*                  71)ConfigureFrequencyAgility                  *\r\n");
   printf("*                  72)SetProximitySearch                         *\r\n");
   printf("*                  73)SetChannelSearchPriority                   *\r\n");
   printf("*                  74)SetUSBDescriptorString                     *\r\n");
   printf("*                  75)ResetSystem                                *\r\n");
   printf("*                  76)OpenChannel                                *\r\n");
   printf("*                  77)CloseChannel                               *\r\n");
   printf("*                  78)RequestMessage                             *\r\n");
   printf("*                  79)OpenRxScanMode                             *\r\n");
   printf("*                  80)SleepMessaage                              *\r\n");
   printf("*                  81)SendBroadcastData                          *\r\n");
   printf("*                  82)SendAcknowledgedData                       *\r\n");
   printf("*                  83)SendBurstTransferData                      *\r\n");
   printf("*                  84)InitalizeCWTestMode                        *\r\n");
   printf("*                  85)SetCWTestMode                              *\r\n");
   printf("*                  86)SendRawPacket                              *\r\n");
   printf("*                  87)SendRawPacketAsync                         *\r\n");
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

   /* The following function is responsible for updating the Debug Zone */
   /* Mask of a Remote Client (based on Process ID).  This function     */
   /* returns zero if successful and a negative value if an error       */
   /* occurred.                                                         */
static int SetDebugZoneMaskPID(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we are not already initialized.    */
   if(Initialized)
   {
      /* Make sure the input parameters have been specified.            */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Now actually Perform the command.                           */
         Result = BTPM_SetDebugZoneMaskPID((unsigned long)(TempParam->Params[0].intParam), (unsigned long)(TempParam->Params[1].intParam));

         if(!Result)
         {
            /* Set Debug Zone Mask request was successful, go ahead and */
            /* inform the User.                                         */
            printf("BTPM_SetDebugZoneMaskPID(%lu) Success: 0x%08lX.\r\n", (unsigned long)TempParam->Params[0].intParam, (unsigned long)TempParam->Params[1].intParam);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Setting Debug Zone Mask, inform the user.          */
            printf("BTPM_SetDebugZoneMaskPID(%lu) Failure: %d, %s.\r\n", (unsigned long)(TempParam->Params[0].intParam), Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDebugZoneMaskPID [Process ID] [Debug Zone Mask].\r\n");

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
      if((TempParam) && (TempParam->NumberofParameters))
      {
         if(TempParam->Params[0].intParam)
            printf("Attempting to Start Discovery (%d Seconds).\r\n", TempParam->Params[0].intParam);
         else
            printf("Attempting to Start Discovery (INDEFINITE).\r\n");

         if((Result = DEVM_StartDeviceDiscovery(TempParam->Params[0].intParam)) >= 0)
         {
            printf("DEVM_StartDeviceDiscovery() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to start Device Discovery, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_StartDeviceDiscovery() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartDeviceDiscovery [Duration].\r\n");

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
      /* Initialized, go ahead and attempt to stop Device Discovery.    */
      if((Result = DEVM_StopDeviceDiscovery()) >= 0)
      {
         printf("DEVM_StopDeviceDiscovery() Success: %d.\r\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error stopping Device Discovery, inform the user and flag an*/
         /* error.                                                      */
         printf("DEVM_StopDeviceDiscovery() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

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
   Boolean_t                       ForceUpdate;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 1)
            ForceUpdate = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);
         else
            ForceUpdate = FALSE;

         printf("Attempting to Query Device Properties: %s, ForceUpdate: %s.\r\n", TempParam->Params[0].strParam, ForceUpdate?"TRUE":"FALSE");

         if((Result = DEVM_QueryRemoteDeviceProperties(BD_ADDR, ForceUpdate, &RemoteDeviceProperties)) >= 0)
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
         printf("Usage: QueryRemoteDeviceProperties [BD_ADDR] [Force Update].\r\n");

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
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;
   Boolean_t Force;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 1)
            Force = (Boolean_t)(TempParam->Params[1].intParam?1:0);
         else
            Force = (Boolean_t)0;

         printf("Attempting to Pair With Remote Device: %s (Force = %d).\r\n", TempParam->Params[0].strParam, Force);

         if((Result = DEVM_PairWithRemoteDevice(BD_ADDR, Force)) >= 0)
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
         printf("Usage: PairWithRemoteDevice [BD_ADDR] [Force Pair (Optional)].\r\n");

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

         printf("Attempting to Un-Pair Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_UnPairRemoteDevice(BD_ADDR, 0)) >= 0)
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
         printf("Usage: UnPairRemoteDevice [BD_ADDR].\r\n");

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
   int                     Result;
   int                     ret_val;
   BD_ADDR_t               BD_ADDR;
   unsigned int            Index;
   unsigned int            TotalServiceSize;
   unsigned char          *ServiceData;
   DEVM_Parsed_SDP_Data_t  ParsedSDPData;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Initialize success.                                         */
         ret_val = 0;

         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         printf("Attempting Query Remote Device %s For Services.\r\n", TempParam->Params[0].strParam);

         if(!TempParam->Params[1].intParam)
         {
            /* Caller has requested to actually retrieve it locally,    */
            /* determine how many bytes were requested.                 */
            if(TempParam->NumberofParameters > 2)
               ServiceData = (unsigned char *)BTPS_AllocateMemory(TempParam->Params[2].intParam);
            else
               ret_val = INVALID_PARAMETERS_ERROR;
         }
         else
            ServiceData = NULL;

         if(!ret_val)
         {
            if((TempParam->Params[1].intParam) || ((!TempParam->Params[1].intParam) && (ServiceData)))
            {
               if((Result = DEVM_QueryRemoteDeviceServices(BD_ADDR, (Boolean_t)TempParam->Params[1].intParam, TempParam->Params[1].intParam?0:TempParam->Params[2].intParam, ServiceData, &TotalServiceSize)) >= 0)
               {
                  printf("DEVM_QueryRemoteDeviceServices() Success: %d, Total Number Service Bytes: %d.\r\n", Result, (TempParam->Params[1].intParam)?0:TotalServiceSize);

                  /* Now convert the Raw Data to parsed data.           */
                  if((Result) && (ServiceData))
                  {
                     printf("Returned Service Data (%d Bytes):\r\n", Result);

                     for(Index=0;Index<Result;Index++)
                        printf("%02X", ServiceData[Index]);

                     printf("\r\n");

                     Result = DEVM_ConvertRawSDPStreamToParsedSDPData(Result, ServiceData, &ParsedSDPData);

                     if(!Result)
                     {
                        /* Success, Display the Parsed Data.            */
                        DisplayParsedServiceData(&ParsedSDPData);

                        /* All finished with the parsed data, so free   */
                        /* it.                                          */
                        DEVM_FreeParsedSDPData(&ParsedSDPData);
                     }
                     else
                     {
                        printf("DEVM_ConvertRawSDPStreamToParsedSDPData() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
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
            printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Force Update] [Bytes to Query (specified if Force is 0)].\r\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Force Update] [Bytes to Query (specified if Force is 0)].\r\n");

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

         printf("Attempting to Authenticate Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_AuthenticateRemoteDevice(BD_ADDR, 0)) >= 0)
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
         printf("Usage: AuthenticateRemoteDevice [BD_ADDR].\r\n");

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

         printf("Attempting to Encrypt Remote Device: %s.\r\n", TempParam->Params[0].strParam);

         if((Result = DEVM_EncryptRemoteDevice(BD_ADDR, 0)) >= 0)
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
         printf("Usage: EncryptRemoteDevice [BD_ADDR].\r\n");

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
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         if(TempParam->NumberofParameters > 1)
            ConnectFlags = (unsigned long)(TempParam->Params[1].intParam);
         else
            ConnectFlags = 0;

         printf("Attempting to Connect With Remote Device: %s (Flags = 0x%08lX).\r\n", TempParam->Params[0].strParam, ConnectFlags);

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
         printf("Usage: ConnectWithRemoteDevice [BD_ADDR] [ConnectFlags (Optional)].\r\n");

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
      if((TempParam) && (TempParam->NumberofParameters))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Determine if the Force Flag was specified.                  */
         if(TempParam->NumberofParameters > 1)
            DisconnectFlags = (TempParam->Params[1].intParam?DEVM_DISCONNECT_FROM_REMOTE_DEVICE_FLAGS_FORCE:0);
         else
            DisconnectFlags = 0;

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
         printf("Usage: DisconnectRemoteDevice [BD_ADDR] [Force Flag (Optional).\r\n");

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
                  printf("Sample SPP SDP Record successfully created: %ld (%08lX).\r\n", (long)RecordHandle, (unsigned long)RecordHandle);

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
         printf("DEVM_CreateServiceRecord() Failure: %ld, %s.\r\n", (long)RecordHandle, ERR_ConvertErrorCodeToString(RecordHandle));

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

         printf("Attempting to Delete SDP Record %ld (0x%08lX).\r\n", (long)RecordHandle, RecordHandle);

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

   /* The following function is responsible for enabling a specified    */
   /* local device feature.  This function returns zero if successful   */
   /* and a negative value if an error occurred.                        */
static int EnableLocalDeviceFeature(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   unsigned long Feature;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         Feature = (unsigned long)TempParam->Params[0].intParam;

         if(!(Result = DEVM_EnableLocalDeviceFeature(Feature)))
         {
            printf("DEVM_EnableLocalDeviceFeature() Success: Feature 0x%08X.\r\n", (unsigned int)Feature);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to enable feature, inform the user and  */
            /* flag an error.                                           */
            printf("DEVM_EnableFeature() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: EnableFeature [Feature (1 = LE, 2 = ANT)].\r\n");

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

   /* The following function is responsible for disabling a specified   */
   /* local device feature.  This function returns zero if successful   */
   /* and a negative value if an error occurred.                        */
static int DisableLocalDeviceFeature(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   unsigned long Feature;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         Feature = (unsigned long)TempParam->Params[0].intParam;

         if(!(Result = DEVM_DisableLocalDeviceFeature(Feature)))
         {
            printf("DEVM_DisableLocalDeviceFeature() Success: Feature 0x%08X.\r\n", (unsigned int)Feature);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error attempting to enable feature, inform the user and  */
            /* flag an error.                                           */
            printf("DEVM_DisableLocalDeviceFeature() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DisableFeature [Feature (1 = LE, 2 = ANT)].\r\n");

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

   /* The following function is responsible for querying the currently  */
   /* active local device properties.  This function returns zero if    */
   /* successful and a negative value if an error occurred.             */
static int QueryActiveLocalDeviceFeatures(ParameterList_t *TempParam)
{
   int           ret_val;
   int           Result;
   unsigned long Features;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Attempt to query the active features.                          */
      if(!(Result = DEVM_QueryActiveLocalDeviceFeatures(&Features)))
      {
         printf("Active Features:\r\n");
         printf("   LE:  %s.\r\n", (Features & DEVM_LOCAL_DEVICE_FEATURE_BLUETOOTH_LOW_ENERGY)?"ACTIVE":"NOT ACTIVE");
         printf("   ANT: %s.\r\n", (Features & DEVM_LOCAL_DEVICE_FEATURE_ANT_PLUS)?"ACTIVE":"NOT ACTIVE");

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error attempting to enable feature, inform the user and flag*/
         /* an error.                                                   */
         printf("DEVM_QueryActiveLocalDeviceFeatures() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

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

   /* The following function is responsible for issuing user-specified  */
   /* HCI commands. This function returns zero on successful execution  */
   /* and a negative value on all errors.                               */
static int SendRawHCICommand(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   unsigned int  Index;
   unsigned int  DataStringLength;
   unsigned char CommandLength;
   unsigned char CommandData[256];
   unsigned char ResultStatus;
   unsigned char ResultLength;
   unsigned char ResultBuffer[256];

   ret_val = 0;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Convert the command data into a byte array.                 */
         if(TempParam->NumberofParameters == 4)
         {
            BTPS_MemInitialize(CommandData, 0, sizeof(CommandData));

            DataStringLength = BTPS_StringLength(TempParam->Params[3].strParam);

            for(Index = 0, CommandLength = 0; Index < DataStringLength; Index += 2, CommandLength++)
            {
               /* Check that there is enough data available for a whole */
               /* byte.                                                 */
               if((Index + 1) < DataStringLength)
               {
                  /* Process the first digit.                           */
                  if((TempParam->Params[3].strParam[Index] >= '0') && (TempParam->Params[3].strParam[Index] <= '9'))
                     CommandData[CommandLength] = (TempParam->Params[3].strParam[Index] & 0xF);
                  else
                  {
                     if((TempParam->Params[3].strParam[Index] >= 'a') && (TempParam->Params[3].strParam[Index] <= 'f'))
                        CommandData[CommandLength] = (TempParam->Params[3].strParam[Index] - 'a' + 10);
                     else
                     {
                        if((TempParam->Params[3].strParam[Index] >= 'A') && (TempParam->Params[3].strParam[Index] <= 'F'))
                           CommandData[CommandLength] = (TempParam->Params[3].strParam[Index] - 'A' + 10);
                        else
                        {
                           /* The data string is invalid.               */
                           printf("The Command Data parameter contains an invalid character.\r\n");
                           ret_val = INVALID_PARAMETERS_ERROR;
                           break;
                        }
                     }
                  }

                  /* Shift the first nibble into the upper half of the  */
                  /* byte.                                              */
                  CommandData[CommandLength] <<= 4;

                  /* Process the second digit.                          */
                  if((TempParam->Params[3].strParam[Index+1] >= '0') && (TempParam->Params[3].strParam[Index+1] <= '9'))
                     CommandData[CommandLength] += (TempParam->Params[3].strParam[Index+1] & 0xF);
                  else
                  {
                     if((TempParam->Params[3].strParam[Index+1] >= 'a') && (TempParam->Params[3].strParam[Index+1] <= 'f'))
                        CommandData[CommandLength] += (TempParam->Params[3].strParam[Index+1] - 'a' + 10);
                     else
                     {
                        if((TempParam->Params[3].strParam[Index+1] >= 'A') && (TempParam->Params[3].strParam[Index+1] <= 'F'))
                           CommandData[CommandLength] += (TempParam->Params[3].strParam[Index+1] - 'A' + 10);
                        else
                        {
                           /* The data string is invalid.               */
                           printf("The Command Data parameter contains an invalid character.\r\n");
                           ret_val = INVALID_PARAMETERS_ERROR;
                           break;
                        }
                     }
                  }
               }
               else
               {
                  /* The data string is missing data.                   */
                  printf("The Command Data parameter contains an odd number of characters.\r\n");
                  ret_val = INVALID_PARAMETERS_ERROR;
                  break;
               }
            }
         }
         else
            CommandLength = 0;

         printf("Attempting to Send Raw HCI Command: 0x%02x 0x%04x ", TempParam->Params[0].intParam, TempParam->Params[1].intParam);
         for(Index = 0; Index < CommandLength; Index++)
            printf("%02x", CommandData[Index]);
         printf("\r\n");

         /* Initialize results.                                         */
         ResultStatus = 0;
         ResultLength = 0;
         BTPS_MemInitialize(ResultBuffer, 0, sizeof(ResultBuffer));

         /* Check whether we should wait for a response.                */
         if(TempParam->Params[2].intParam == 0)
            Result = DEVM_SendRawHCICommand(TempParam->Params[0].intParam, TempParam->Params[1].intParam, CommandLength, CommandData, NULL, NULL, NULL, FALSE);
         else
         {
            ResultLength = (unsigned char)sizeof(ResultBuffer);

            Result = DEVM_SendRawHCICommand(TempParam->Params[0].intParam, TempParam->Params[1].intParam, CommandLength, CommandData, &ResultStatus, &ResultLength, ResultBuffer, TRUE);
         }

         if(Result == 0)
         {
            printf("DEVM_SendRawHCICommand() Success: %d.\r\n", Result);

            if(TempParam->Params[2].intParam != 0)
            {
               printf("   Result:          %u\r\n", ResultStatus);
               printf("   Response Length: %u\r\n", ResultLength);
               printf("   Response Data:   ");

               for(Index = 0; Index < ResultLength; Index++)
                  printf("%02x", ResultBuffer[Index]);

               printf("\r\n");
            }

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Connecting With Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_SendRawHCICommand() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SendRawHCICommand [OGF] [OCF] [Wait For Response] [Command Data (Optional, specify in hexadecimal)].\r\n");

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

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a event callback function with the ANT+       */
   /* Manager Service.  This Callback will be dispatched by the ANT+    */
   /* Manager when various ANT+ Manager Events occur.  This function    */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a ANT+ Manager Event needs to be      */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANTM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
static int ANTMRegisterServerEventCallback(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Event Callback Registered, then we need to flag */
      /* an error.                                                      */
      if(!ANTMEventCallbackID)
      {
         /* Callback has not been registered, go ahead and attempt to   */
         /* register it.                                                */
         if((Result = ANTM_Register_Event_Callback(ANTM_Event_Callback, NULL)) > 0)
         {
            printf("ANTM_Register_Event_Callback() Success: %d.\r\n", Result);

            ANTMEventCallbackID = (unsigned int)Result;

            ret_val             = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("ANTM_Register_Event_Callback() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("ANTM Manager Event Callback already registered.\r\n");

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


   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANT+ Manager Event Callback   */
   /* (registered via a successful call to the                          */
   /* ANTM_Register_Event_Callback() function).  This function accepts  */
   /* as input the ANT+ Manager Event Callback ID (return value from    */
   /* ANTM_Register_Event_Callback() function).                         */
static int ANTMUnRegisterServerEventCallback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(ANTMEventCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         ANTM_Un_Register_Event_Callback(ANTMEventCallbackID);

         printf("ANTM_Un_Register_Event_Callback() Success.\r\n");

         /* Flag that there is no longer a Callback registered.         */
         ANTMEventCallbackID = 0;

         ret_val             = 0;
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("ANTM Manager Event Callback is not registered.\r\n");

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


   /* The following function is responsible for assigning an ANT channel*/
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to register.  This function   */
   /* accepts as it's third argument, the channel type to be assigned to*/
   /* the channel.  This function accepts as it's fourth argument, the  */
   /* network number to be used for the channel.  Zero should be        */
   /* specified for this argument to use the default public network.    */
   /* This function accepts as it's fifth argument, the extended        */
   /* assignment to be used for the channel.  Zero should be specified  */
   /* for this argument if no extended capabilities are to be used.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
static int AssignChannel(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 4))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Assign_Channel(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam, TempParam->Params[3].intParam)) == 0)
               printf("ANTM_Assign_Channel() Success.\r\n");
            else
            {
               printf("ANTM_Assign_Channel() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: AssignChannel [ChannelNumber] [ChannelType] [NetworkNumber] [ExtendedAssignment]\r\n");
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


   /* The following function is responsible for un-assigning an ANT     */
   /* channel on the local ANT+ system.  A channel must be unassigned   */
   /* before it can be reassigned using the ANTM_Assign_Channel() API.  */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to un-assign.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
static int UnassignChannel(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Un_Assign_Channel(ANTMEventCallbackID, TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Un_Assign_Channel() Success.\r\n");
            else
            {
               printf("ANTM_Un_Assign_Channel() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: UnAssignChannel [ChannelNumber]\r\n");
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


   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, the channel number to configure.  The ANT   */
   /* channel must be assigned using ANTM_Assign_Channel() before       */
   /* calling this function.  This function accepts as it's third       */
   /* argument, the device number to search for on the channel.  Zero   */
   /* should be specified for this argument to scan for any device      */
   /* number.  This function accepts as it's fourth argument, the device*/
   /* type to search for on the channel.  Zero should be specified for  */
   /* this argument to scan for any device type.  This function accepts */
   /* as it's fifth argument, the transmission type to search for on the*/
   /* channel.  Zero should be specified for this argument to scan for  */
   /* any transmission type.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
static int SetChannelID(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 4))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Channel_ID(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam, TempParam->Params[3].intParam)) == 0)
               printf("ANTM_Set_Channel_ID() Success.\r\n");
            else
            {
               printf("ANTM_Set_Channel_ID() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetChannelID [ChannelNumber] [DeviceNumber] [DeviceType] [TransmissionType]\r\n");
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


   /* The following function is responsible for configuring the         */
   /* messaging period for an ANT channel on the local ANT+ system.     */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel messaging period to   */
   /* set on the channel.  This function returns zero if successful,    */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be MessagePeriod * 32768 (e.g.  to send / receive a */
   /*          message at 4Hz, set MessagePeriod to 32768/4 = 8192).    */
static int SetChannelPeriod(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Channel_Period(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Channel_Period() Success.\r\n");
            else
            {
               printf("ANTM_Set_Channel_Period() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetChannelPeriod [ChannelNumber] [MessagingPeriod]\r\n");
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

   /* The following function is responsible for configuring the amount  */
   /* of time that the receiver will search for an ANT channel before   */
   /* timing out.  This function accepts as it's first argument the     */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the search timeout to set on the  */
   /* channel.  This function returns zero if successful, otherwise this*/
   /* function returns a negative error code.                           */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable high priority search  */
   /*          mode on Non-AP1 devices.  A special search value of 255  */
   /*          will result in an infinite search timeout.  Specifying   */
   /*          these search values on AP1 devices will not have any     */
   /*          special effect.                                          */
static int SetChannelSearchTimeout(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Channel_Search_Timeout(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Channel_Search_Timeout() Success.\r\n");
            else
            {
               printf("ANTM_Set_Channel_Search_Timeout() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetChannelSearchTimeout [ChannelNumber] [Search Timeout]\r\n");
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


   /* The following function is responsible for configuring the channel */
   /* frequency for an ANT channel.  This function accepts as it's first*/
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the channel frequency to set on   */
   /* the channel.  This function returns zero if successful, otherwise */
   /* this function returns a negative error code.                      */
   /* * NOTE * The actual messaging period calculated by the ANT device */
   /*          will be (2400 + RFFrequency) MHz.                        */
static int SetChannelRFFrequency(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Channel_RF_Frequency(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Channel_RF_Frequency() Success.\r\n");
            else
            {
               printf("ANTM_Set_Channel_RF_Frequency() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetChannelRFFrequency [ChannelNumber] [ChannelFrequency]\r\n");
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


   /* The following function is responsible for configuring the network */
   /* key for an ANT channel.  This function accepts as it's first      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, a pointer to the ANT network key  */
   /* to set on the channel.  This function returns zero if successful, */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * Setting the network key is not required when using the   */
   /*          default public network.                                  */
static int SetNetworkKey(ParameterList_t *TempParam)
{
   int               ret_val;
   ANT_Network_Key_t NetworkKey;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(NetworkKey.ANT_Network_Key4), (DWord_t)TempParam->Params[1].intParam);
            ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&(NetworkKey.ANT_Network_Key0), (DWord_t)TempParam->Params[2].intParam);

            if((ret_val = ANTM_Set_Network_Key(ANTMEventCallbackID, TempParam->Params[0].intParam, NetworkKey)) == 0)
               printf("ANTM_Set_Network_Key() Success.\r\n");
            else
            {
               printf("ANTM_Set_Network_Key() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetNetworkKey [NetworkNumber] [Upper 4 bytes of Network Key] [Lower 4 bytes of Network Key]\r\n");
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


   /* The following function is responsible for configuring the transmit*/
   /* power on the local ANT system.  This function accepts as it's     */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument the transmit power to set on the device.     */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
static int SetTransmitPower(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Transmit_Power(ANTMEventCallbackID, TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Set_Transmit_Power() Success.\r\n");
            else
            {
               printf("ANTM_Set_Transmit_Power() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetTransmitPower [Transmit Power]\r\n");
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


   /* The following function is responsible for adding a channel number */
   /* to the device's inclusion / exclusion list.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to add to the */
   /* list.  This function accepts as it's third argument, the device   */
   /* number to add to the list.  This function accepts as it's fourth  */
   /* argument, the device type to add to the list.  This function      */
   /* accepts as it's fifth argument, the transmission type to add to   */
   /* the list.  This function accepts as it's sixth argument, the the  */
   /* list index to overwrite with the updated entry.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
static int AddChannelID(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 5))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Add_Channel_ID(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam, TempParam->Params[3].intParam, TempParam->Params[4].intParam)) == 0)
               printf("ANTM_Add_Channel_ID() Success.\r\n");
            else
            {
               printf("ANTM_Add_Channel_ID() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: AddChannelID [ChannelNumber] [DeviceNumber] [DeviceType] [TransmissionType] [ListIndex]\r\n");
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


   /* The following function is responsible for configuring the         */
   /* inclusion / exclusion list on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* on which the list should be configured.  This function accepts as */
   /* it's third argument, the size of the list.  This function accepts */
   /* as it's fourth argument, the list type.  Zero should be specified */
   /* to configure the list for inclusion, and one should be specified  */
   /* to configure the list for exclusion.  This function returns zero  */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
static int ConfigureInclusionExclusionList(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Configure_Inclusion_Exclusion_List(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam)) == 0)
               printf("ANTM_Configure_Inclusion_Exclusion_List() Success.\r\n");
            else
            {
               printf("ANTM_Configure_Inclusion_Exclusion_List() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: ConfigureInclusionExclusionList [ChannelNumber] [ListSize] [ListType]\r\n");
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



   /* The following function is responsible for configuring the transmit*/
   /* power for an ANT channel.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  This function  */
   /* accepts as it's third argument, the transmit power level for the  */
   /* specified channel.  This function returns zero if successful,     */
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
static int SetChannelTransmitPower(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Channel_Transmit_Power(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Channel_Transmit_Power() Success.\r\n");
            else
            {
               printf("ANTM_Set_Channel_Transmit_Power() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetChannelTransmitPower [ChannelNumber] [TransmitPowerLevel]\r\n");
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


   /* The following function is responsible for configuring the duration*/
   /* in which the receiver will search for a channel in low priority   */
   /* mode before switching to high priority mode.  This function       */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to   */
   /* configure.  This function accepts as it's third argument, the     */
   /* search timeout to set on the channel.  This function returns zero */
   /* if successful, otherwise this function returns a negative error   */
   /* code.                                                             */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The actual search timeout calculated by the ANT device   */
   /*          will be SearchTimeout * 2.5 seconds.  A special search   */
   /*          timeout value of zero will disable low priority search   */
   /*          mode.  A special search value of 255 will result in an   */
   /*          infinite low priority search timeout.                    */
static int SetLowPriorityChannelSearchTimeout(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Low_Priority_Channel_Search_Timeout(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Low_Priority_Channel_Search_Timeout() Success.\r\n");
            else
            {
               printf("ANTM_Set_Low_Priority_Channel_Search_Timeout() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetLowPriorityChannelSearchTimeout [ChannelNumber] [SearchTimeout]\r\n");
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


   /* The following function is responsible for configuring an ANT      */
   /* channel on the local ANT+ system.  This function configures the   */
   /* channel ID in the same way as ANTM_Set_Channel_ID(), except it    */
   /* uses the two LSB of the device's serial number as the device's    */
   /* number.  This function accepts as it's first argument the Callback*/
   /* ID that was returned from a successful call                       */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to configure.  The ANT channel*/
   /* must be assigned using ANTM_Assign_Channel() before calling this  */
   /* function.  This function accepts as it's third argument, the      */
   /* device type to search for on the channel.  Zero should be         */
   /* specified for this argument to scan for any device type.  This    */
   /* function accepts as it's fourth argument, the transmission type to*/
   /* search for on the channel.  Zero should be specified for this     */
   /* argument to scan for any transmission type.  This function returns*/
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
static int SetSerialNumberChannelID(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Serial_Number_Channel_ID(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam)) == 0)
               printf("ANTM_Set_Serial_Number_Channel_ID() Success.\r\n");
            else
            {
               printf("ANTM_Set_Serial_Number_Channel_ID() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetSerialNumberChannelID [ChannelNumber] [DeviceType] [TransmissionType]\r\n");
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


   /* The following function is responsible for enabling or disabling   */
   /* extended Rx messages for an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument whether or not to enable extended Rx messages.    */
   /* This function returns zero if successful, otherwise this function */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
static int EnableExtendedMessages(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Enable_Extended_Messages(ANTMEventCallbackID, (Boolean_t)TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Enable_Extended_Messages() Success.\r\n");
            else
            {
               printf("ANTM_Enable_Extended_Messages() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: EnableExtendedMessages [Enable]\r\n");
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


   /* The following function is responsible for enabling or disabling   */
   /* the LED on the local ANT+ system.  This function accepts as it's  */
   /* first argument the Callback ID that was returned from a successful*/
   /* call ANTM_Register_Event_Callback().  This function accepts as    */
   /* it's second argument, whether or not to enable the LED.  This     */
   /* function returns zero if successful, otherwise this function      */
   /* returns a negative error code.                                    */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
static int EnableLED(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Enable_LED(ANTMEventCallbackID, (Boolean_t)TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Enable_LED() Success.\r\n");
            else
            {
               printf("ANTM_Enable_LED() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: EnableLED [Enable]\r\n");
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


   /* The following function is responsible for enabling the 32kHz      */
   /* crystal input on the local ANT+ system.  This function accepts as */
   /* it's only argument the Callback ID that was returned from a       */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This function should only be sent when a startup message */
   /*          is received.                                             */
static int EnableCrystal(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(ANTMEventCallbackID)
      {
         if((ret_val = ANTM_Enable_Crystal(ANTMEventCallbackID)) == 0)
            printf("ANTM_Enable_Crystal() Success.\r\n");
         else
         {
            printf("ANTM_Enable_Crystal() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("ANT+ Manager Event Callback is not registered.\r\n");

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


   /* The following function is responsible for enabling or disabling   */
   /* each extended Rx message on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the bitmask of extended */
   /* Rx messages that shall be enabled or disabled.  This function     */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
static int ConfigureExtendedMessages(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Configure_Extended_Messages(ANTMEventCallbackID, TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Configure_Extended_Messages() Success.\r\n");
            else
            {
               printf("ANTM_Configure_Extended_Messages() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: ConfigureExtendedMessages [Bitmask]\r\n");
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


   /* The following function is responsible for configuring the three   */
   /* operating frequencies for an ANT channel.  This function accepts  */
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the channel number to configure. */
   /* This function accepts as it's third, fourth, and fifth arguments, */
   /* the three operating agility frequencies to set.  This function    */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * The operating frequency agilities should only be         */
   /*          configured after channel assignment and only if frequency*/
   /*          agility bit has been set in the ExtendedAssignment       */
   /*          argument of ANTM_Assign_Channel.  Frequency agility      */
   /*          should NOT be used with shared, Tx only, or Rx only      */
   /*          channels.                                                */
static int ConfigureFrequencyAgility(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 4))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Configure_Frequency_Agility(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam, TempParam->Params[3].intParam)) == 0)
               printf("ANTM_Configure_Frequency_Agility() Success.\r\n");
            else
            {
               printf("ANTM_Configure_Frequency_Agility() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: ConfigureFrequencyAgility [ChannelNumber] [Frequency1] [Frequency2] [Frequency3]\r\n");
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


   /* The following function is responsible for configuring the         */
   /* proximity search requirement on the local ANT+ system.  This      */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search threshold to set.  This function returns zero if           */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * The search threshold value is cleared once a proximity   */
   /*          search has completed successfully.  If another proximity */
   /*          search is desired after a successful search, then the    */
   /*          threshold value must be reset.                           */
static int SetProximitySearch(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Proximity_Search(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Proximity_Search() Success.\r\n");
            else
            {
               printf("ANTM_Set_Proximity_Search() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetProximitySearch [ChannelNumber] [SearchThreshold]\r\n");
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


   /* The following function is responsible for configuring the search  */
   /* priority of an ANT channel on the local ANT+ system.  This        */
   /* function accepts as it's first argument the Callback ID that was  */
   /* returned from a successful call ANTM_Register_Event_Callback().   */
   /* This function accepts as it's second argument, the channel number */
   /* to configure.  This function accepts as it's third argument, the  */
   /* search priority to set.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
   /* * NOTE * This feature is not available on all ANT devices.        */
static int SetChannelSearchPriority(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_Channel_Search_Priority(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_Channel_Search_Priority() Success.\r\n");
            else
            {
               printf("ANTM_Set_Channel_Search_Priority() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetChannelSearchPriority [ChannelNumber] [SearchPriority]\r\n");
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


   /* The following function is responsible for configuring the USB     */
   /* descriptor string on the local ANT+ system.  This function accepts*/
   /* as it's first argument the Callback ID that was returned from a   */
   /* successful call ANTM_Register_Event_Callback().  This function    */
   /* accepts as it's second argument, the descriptor string type to    */
   /* set.  This function accepts as it's third argument, the           */
   /* NULL-terminated descriptor string to be set.  This function       */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * This feature is not available on all ANT devices.        */
static int SetUSBDescriptorString(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_USB_Descriptor_String(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].strParam)) == 0)
               printf("ANTM_Set_USB_Descriptor_String() Success.\r\n");
            else
            {
               printf("ANTM_Set_USB_Descriptor_String() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetUSBDescriptorString [DescriptorStringType] [DescriptorString]\r\n");
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


   /* The following function is responsible for resetting the ANT module*/
   /* on the local ANT+ system.  This function accepts as it's only     */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  A delay of at least 500ms is     */
   /* suggested after calling this function to allow time for the module*/
   /* to reset.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
static int ResetSystem(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(ANTMEventCallbackID)
      {
         if((ret_val = ANTM_Reset_System(ANTMEventCallbackID)) == 0)
            printf("ANTM_Reset_System() Success.\r\n");
         else
         {
            printf("ANTM_Reset_System() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("ANT+ Manager Event Callback is not registered.\r\n");

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


   /* The following function is responsible for opening an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be opened.  The channel    */
   /* specified must have been assigned and configured before calling   */
   /* this function.  This function returns zero if successful,         */
   /* otherwise this function returns a negative error code.            */
static int OpenChannel(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Open_Channel(ANTMEventCallbackID, TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Open_Channel() Success.\r\n");
            else
            {
               printf("ANTM_Open_Channel() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: OpenChannel [ChannelNumber]\r\n");
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


   /* The following function is responsible for closing an ANT channel  */
   /* on the local ANT+ system.  This function accepts as it's first    */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number to be closed.  This function  */
   /* returns zero if successful, otherwise this function returns a     */
   /* negative error code.                                              */
   /* * NOTE * No operations can be performed on channel being closed   */
   /*          until the aetANTMChannelResponse event has been received */
   /*          with the Message_Code member specifying:                 */
   /*             ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED        */
static int CloseChannel(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Close_Channel(ANTMEventCallbackID, TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Close_Channel() Success.\r\n");
            else
            {
               printf("ANTM_Close_Channel() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: CloseChannel [ChannelNumber]\r\n");
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


   /* The following function is responsible for requesting an           */
   /* information message from an ANT channel on the local ANT+ system. */
   /* This function accepts as it's first argument the Callback ID that */
   /* was returned from a successful call                               */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the channel number that the request will be sent */
   /* to.  This function accepts as it's third argument, the message ID */
   /* being requested from the channel.  This function returns zero if  */
   /* successful, otherwise this function returns a negative error code.*/
static int RequestMessage(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Request_Message(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Request_Message() Success.\r\n");
            else
            {
               printf("ANTM_Request_Message() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: RequestMessage [ChannelNumber] [MessageID]\r\n");
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


   /* The following function is responsible for opening an ANT channel  */
   /* in continuous scan mode on the local ANT+ system.  This function  */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number to be*/
   /* opened.  The channel specified must have been assigned and        */
   /* configured as a SLAVE Rx ONLY channel before calling this         */
   /* function.  This function returns zero if successful, otherwise    */
   /* this function returns a negative error code.                      */
   /* * NOTE * This feature is not available on all ANT devices.  Check */
   /*          the request capabilities of the device before using this */
   /*          function.                                                */
   /* * NOTE * No other channels can operate when a single channel is   */
   /*          opened in Rx scan mode.                                  */
static int OpenRxScanMode(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Open_Rx_Scan_Mode(ANTMEventCallbackID, TempParam->Params[0].intParam)) == 0)
               printf("ANTM_Open_Rx_Scan_Mode() Success.\r\n");
            else
            {
               printf("ANTM_Open_Rx_Scan_Mode() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: OpenRXScanMode [ChannelNumber]\r\n");
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


   /* The following function is responsible for putting the ANT+ system */
   /* in ultra low-power mode.  This function accepts as it's only      */
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature is not available on all ANT devices.        */
   /* * NOTE * This feature must be used in conjunction with setting the*/
   /*          SLEEP/(!MSGREADY) line on the ANT chip to the appropriate*/
   /*          value.                                                   */
static int SleepMessage(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(ANTMEventCallbackID)
      {
         if((ret_val = ANTM_Sleep_Message(ANTMEventCallbackID)) == 0)
            printf("ANTM_Sleep_Message() Success.\r\n");
         else
         {
            printf("ANTM_Sleep_Message() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("ANT+ Manager Event Callback is not registered.\r\n");

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


   /* The following function is responsible for sending broadcast data  */
   /* from an ANT channel on the local ANT+ system.  This function      */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be broadcast on.  This function accepts as it's     */
   /* third argument the length of the data to send.  This function     */
   /* accepts as it's fourth argument a pointer to a byte array of the  */
   /* broadcast data to send.  This function returns zero if successful,*/
   /* otherwise this function returns a negative error code.            */
static int SendBroadcastData(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Send_Broadcast_Data(ANTMEventCallbackID, TempParam->Params[0].intParam, BTPS_StringLength(TempParam->Params[1].strParam), (Byte_t *)TempParam->Params[1].strParam)) == 0)
               printf("ANTM_Send_Broadcast_Data() Success.\r\n");
            else
            {
               printf("ANTM_Send_Broadcast_Data() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SendBroadcastData [ChannelNumber] [Data String]\r\n");
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


   /* The following function is responsible for sending acknowledged    */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the channel number that */
   /* the data will be sent on.  This function accepts as it's third    */
   /* argument the length of the data to send.  This function accepts as*/
   /* it's fourth argument, a pointer to a byte array of the            */
   /* acknowledged data to send.  This function returns zero if         */
   /* successful, otherwise this function returns a negative error code.*/
static int SendAcknowledgedData(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Send_Acknowledged_Data(ANTMEventCallbackID, TempParam->Params[0].intParam, BTPS_StringLength(TempParam->Params[1].strParam), (Byte_t *)TempParam->Params[1].strParam)) == 0)
               printf("ANTM_Send_Acknowledged_Data() Success.\r\n");
            else
            {
               printf("ANTM_Send_Acknowledged_Data() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SendAcknowledgedData [ChannelNumber] [Data String]\r\n");
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

   /* The following function is responsible for sending burst transfer  */
   /* data from an ANT channel on the local ANT+ system.  This function */
   /* accepts as it's first argument the Callback ID that was returned  */
   /* from a successful call ANTM_Register_Event_Callback().  This      */
   /* function accepts as it's second argument, the sequence / channel  */
   /* number that the data will be sent on.  The upper three bits of    */
   /* this argument are the sequence number, and the lower five bits are*/
   /* the channel number.  This function accepts as it's third argument */
   /* the length of the data to send.  This function accepts as it's    */
   /* fourth argument, a pointer to a byte array of the burst data to   */
   /* send.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
static int SendBurstTransferData(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Send_Burst_Transfer_Data(ANTMEventCallbackID, TempParam->Params[0].intParam, BTPS_StringLength(TempParam->Params[1].strParam), (Byte_t *)TempParam->Params[1].strParam)) == 0)
               printf("ANTM_Send_Burst_Transfer_Data() Success.\r\n");
            else
            {
               printf("ANTM_Send_Burst_Transfer_Data() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SendBurstTransferData [ChannelNumber] [Data String]\r\n");
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


   /* The following function is responsible for putting the ANT+ system */
   /* in CW test mode.  This function accepts as it's only argument the */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function returns zero if    */
   /* successful, otherwise this function returns a negative error code.*/
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          resetting the ANT module.                                */
static int InitializeCWTestMode(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(ANTMEventCallbackID)
      {
         if((ret_val = ANTM_Initialize_CW_Test_Mode(ANTMEventCallbackID)) == 0)
            printf("ANTM_Initialize_CW_Test_Mode() Success.\r\n");
         else
         {
            printf("ANTM_Initialize_CW_Test_Mode() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("ANT+ Manager Event Callback is not registered.\r\n");

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

   /* The following function is responsible for putting the ANT module  */
   /* in CW test mode using a given transmit power level and RF         */
   /* frequency.  This function accepts as it's first argument the      */
   /* Callback ID that was returned from a successful call              */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the transmit power level to be used.  This       */
   /* function accepts as it's third argument, the RF frequency to be   */
   /* used.  This function returns zero if successful, otherwise this   */
   /* function returns a negative error code.                           */
   /* * NOTE * This feature should be used ONLY immediately after       */
   /*          calling ANTM_Initialize_CW_Test_Mode().                  */
static int SetCWTestMode(ParameterList_t *TempParam)
{
   int   ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((ret_val = ANTM_Set_CW_Test_Mode(ANTMEventCallbackID, TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
               printf("ANTM_Set_CW_Test_Mode() Success.\r\n");
            else
            {
               printf("ANTM_Set_CW_Test_Mode() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SetCWTestMode [TransmitPowerLevel] [RFFrequency]\r\n");
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

   /* The following function is responsible for sending a preformmatted */
   /* ANT packet to the ANT module.  This function accepts as it's first*/
   /* argument the Callback ID that was returned from a successful call */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the preformmated packet.  This function returns  */
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
static int SendRawPacket(ParameterList_t *TempParam)
{
   int     Index;
   int     ret_val;
   Byte_t *Packet;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((Packet = (Byte_t *)BTPS_AllocateMemory(TempParam->NumberofParameters)) != NULL)
            {
               for(Index = 0; Index < TempParam->NumberofParameters; Index++)
                  Packet[Index] = TempParam->Params[Index].intParam;

               if((ret_val = ANTM_Send_Raw_Packet(ANTMEventCallbackID, TempParam->NumberofParameters, Packet)) == 0)
                  printf("ANTM_Send_Raw_Packet() Success.\r\n");
               else
               {
                  printf("ANTM_Send_Raw_Packet() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

                  ret_val = FUNCTION_ERROR;
               }

               BTPS_FreeMemory(Packet);
            }
            else
            {
               printf("Unable to allocate memory\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SendRawPacket [Packet data (specified as bytes separated by spaces)]\r\n");
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

   /* The following function is responsible for sending a preformatted  */
   /* ANT packet to the ANT module asynchronously (without a return     */
   /* code).  This function accepts as it's first argument the          */
   /* Callback ID that was returned from a successful call to           */
   /* ANTM_Register_Event_Callback().  This function accepts as it's    */
   /* second argument, the preformatted packet.  This function returns  */
   /* zero if successful, otherwise this function returns a negative    */
   /* error code.                                                       */
static int SendRawPacketAsync(ParameterList_t *TempParam)
{
   int     Index;
   int     ret_val;
   Byte_t *Packet;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Next, check to make sure that there is an Event Callback    */
         /* already registered.                                         */
         if(ANTMEventCallbackID)
         {
            if((Packet = (Byte_t *)BTPS_AllocateMemory(TempParam->NumberofParameters)) != NULL)
            {
               for(Index = 0; Index < TempParam->NumberofParameters; Index++)
                  Packet[Index] = TempParam->Params[Index].intParam;

               if((ret_val = ANTM_Send_Raw_Packet_Async(ANTMEventCallbackID, TempParam->NumberofParameters, Packet)) == 0)
                  printf("ANTM_Send_Raw_Packet_Async() Success.\r\n");
               else
               {
                  printf("ANTM_Send_Raw_Packet_Async() Failure: %d, %s.\r\n", ret_val, ERR_ConvertErrorCodeToString(ret_val));

                  ret_val = FUNCTION_ERROR;
               }

               BTPS_FreeMemory(Packet);
            }
            else
            {
               printf("Unable to allocate memory\r\n");

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Callback NOT registered, go ahead and notify the user.   */
            printf("ANT+ Manager Event Callback is not registered.\r\n");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("%s", "Usage: SendRawPacket [Packet data (specified as bytes separated by spaces)]\r\n");
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

         if(LocalDeviceProperties->LocalDeviceFlags)
         {
            if(LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_DEVICE_DISCOVERY_IN_PROGRESS)
               printf("         DEVICE_DISCOVERY_IN_PROGRESS\r\n");
         }
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
   }
}

   /* The following function is a utility function that exists to       */
   /* display either the entire Remote Device Property information      */
   /* (first parameter is zero) or portions that have changed.          */
static void DisplayRemoteDeviceProperties(unsigned long UpdateMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties)
{
   char Buffer[64];

   if(RemoteDeviceProperties)
   {
      /* First, display any information that is not part of any update  */
      /* mask.                                                          */
      BD_ADDRToStr(RemoteDeviceProperties->BD_ADDR, Buffer);

      printf("BD_ADDR:       %s\r\n", Buffer);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE))
         printf("COD:           0x%02X%02X%02X\r\n", RemoteDeviceProperties->ClassOfDevice.Class_of_Device0, RemoteDeviceProperties->ClassOfDevice.Class_of_Device1, RemoteDeviceProperties->ClassOfDevice.Class_of_Device2);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:   %s\r\n", (RemoteDeviceProperties->DeviceNameLength)?RemoteDeviceProperties->DeviceName:"");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
      {
         printf("Device Flags:  0x%08lX\r\n", RemoteDeviceProperties->RemoteDeviceFlags);

         if(RemoteDeviceProperties->RemoteDeviceFlags)
         {
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_NAME_KNOWN)
               printf("         NAME_KNOWN\r\n");
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_INITIATED_LOCALLY)
               printf("         LINK_INITIATED_LOCALLY\r\n");
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_AUTHENTICATED_KEY)
               printf("         AUTHENTICATED_KEY\r\n");
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_EIR_DATA_KNOWN)
               printf("         EIR_DATA_KNOWN\r\n");
         }
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_RSSI))
         printf("RSSI:          %d\r\n", RemoteDeviceProperties->RSSI);

      if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_TX_POWER_KNOWN))
         printf("Trans. Power:  %d\r\n", RemoteDeviceProperties->TransmitPower);

      if((!UpdateMask) || ((UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_APPLICATION_DATA) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_APPLICATION_DATA_VALID)))
      {
         printf("Friendly Name: %s\r\n", (RemoteDeviceProperties->ApplicationData.FriendlyNameLength)?RemoteDeviceProperties->ApplicationData.FriendlyName:"");

         printf("App. Info:   : %08lX\r\n", RemoteDeviceProperties->ApplicationData.ApplicationInfo);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PAIRING_STATE))
         printf("Paired State : %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED)?"TRUE":"FALSE");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CONNECTION_STATE))
         printf("Connect State: %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED)?"TRUE":"FALSE");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_ENCRYPTION_STATE))
         printf("Encrypt State: %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_ENCRYPTED)?"TRUE":"FALSE");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SNIFF_STATE))
      {
         if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_SNIFF_MODE)
            printf("Sniff State  : TRUE (%u ms)\r\n", RemoteDeviceProperties->SniffInterval);
         else
            printf("Sniff State  : FALSE\r\n");
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SERVICES_STATE))
         printf("Serv. Known  : %s\r\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SERVICES_KNOWN)?"TRUE":"FALSE");
   }
}

   /* The following function is responsible for displaying the contents */
   /* of Parsed Remote Device Services Data to the display.             */
static void DisplayParsedServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData)
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

   printf("ANTM>");

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

            printf("Remote Device Pairing Status: %s, %s (0x%02X)\r\n", Buffer, (EventData->EventData.RemoteDevicePairingStatusEventData.Success)?"SUCCESS":"FAILURE", EventData->EventData.RemoteDevicePairingStatusEventData.AuthenticationStatus);
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

   printf("ANTM>");

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
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   if(AuthenticationRequestInformation)
   {
      printf("\r\n");

      BD_ADDRToStr(AuthenticationRequestInformation->BD_ADDR, Buffer);

      printf("Authentication Request received for %s.\r\n", Buffer);

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
            printf("User Confirmation Request.\r\n");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* User Confirmation.                                       */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            if(IOCapability != icDisplayYesNo)
            {
               /* Invoke Just works.                                    */

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

               /* Flag that there is no longer a current Authentication */
               /* procedure in progress.                                */
               ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
            }
            else
            {
               printf("User Confirmation: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

               /* Inform the user that they will need to respond with a */
               /* PIN Code Response.                                    */
               printf("\r\nRespond with the command: UserConfirmationResponse\r\n");
            }
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_REQUEST:
            printf("Passkey Request.\r\n");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* Passkey.                                                 */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            /* Inform the user that they will need to respond with a    */
            /* Passkey Response.                                        */
            printf("\r\nRespond with the command: PassKeyResponse\r\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION:
            printf("PassKey Indication.\r\n");

            printf("PassKey: %lu\r\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);
            break;
         case DEVM_AUTHENTICATION_ACTION_KEYPRESS_INDICATION:
            printf("Keypress Indication.\r\n");

            printf("Keypress: %d\r\n", (int)AuthenticationRequestInformation->AuthenticationData.Keypress);
            break;
         case DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST:
            printf("Out of Band Data Request.\r\n");

            /* This application does not support OOB data so respond    */
            /* with a data length of Zero to force a negative reply.    */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                  = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction     = DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength = 0;

            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\r\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST:
            printf("I/O Capability Request.\r\n");

            /* Respond with the currently configured I/O Capabilities.  */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                                                    = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction                                       = DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength                                   = sizeof(AuthenticationResponseInformation.AuthenticationData.IOCapabilities);

            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.IO_Capability            = (GAP_IO_Capability_t)IOCapability;
//xxx Check if MITM should be set
            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.MITM_Protection_Required = MITMProtection;
            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.OOB_Data_Present         = OOBSupport;

//xxx Here check if this will default to "just works" or auto accept.
            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\r\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE:
            printf("I/O Capability Response.\r\n");

            /* Inform the user of the Remote I/O Capablities.           */
            printf("Remote I/O Capabilities: %s, MITM Protection: %s, OOB Data: %s, Bonding: %s.\r\n", IOCapabilitiesStrings[AuthenticationRequestInformation->AuthenticationData.IOCapabilities.IO_Capability], AuthenticationRequestInformation->AuthenticationData.IOCapabilities.MITM_Protection_Required?"TRUE":"FALSE", AuthenticationRequestInformation->AuthenticationData.IOCapabilities.OOB_Data_Present?"TRUE":"FALSE", BondingStrings[AuthenticationRequestInformation->AuthenticationData.IOCapabilities.Bonding_Type]);
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_STATUS_RESULT:
            printf("Authentication Status.\r\n");

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

   printf("ANTM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the ANT+ Manager Callback function that */
   /* is Registered with the ANT+ Manager.  This callback is responsible*/
   /* for processing all ANT+ Manager Events.                           */
static void BTPSAPI ANTM_Event_Callback(ANTM_Event_Data_t *EventData, void *CallbackParameter)
{
   unsigned int Index;

   if(EventData)
   {
      printf("\r\n");

      switch(EventData->EventType)
      {
         case aetANTMStartupMessage:
            printf("ANTM StartupMessage\r\n");
            printf("\tStartupMessage:     %d\r\n", EventData->EventData.StartupMessageEventData.StartupMessage);
            break;
         case aetANTMChannelResponse:
            printf("ANTM Channel Response\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.ChannelResponseEventData.ChannelNumber);
            printf("\tMessageID:          %d\r\n", EventData->EventData.ChannelResponseEventData.MessageID);
            printf("\tMessageCode:        %d\r\n", EventData->EventData.ChannelResponseEventData.MessageCode);
            break;
         case aetANTMChannelStatus:
            printf("ANTM Channel Status\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.ChannelStatusEventData.ChannelNumber);
            printf("\tChannelStatus:      %d\r\n", EventData->EventData.ChannelStatusEventData.ChannelStatus);
            break;
         case aetANTMChannelID:
            printf("ANTM Channel ID\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.ChannelIDEventData.ChannelNumber);
            printf("\tDeviceNumber:       %d\r\n", EventData->EventData.ChannelIDEventData.DeviceNumber);
            printf("\tDeviceTypeID:       %d\r\n", EventData->EventData.ChannelIDEventData.DeviceTypeID);
            printf("\tTransmissionType:   %d\r\n", EventData->EventData.ChannelIDEventData.TransmissionType);
            break;
         case aetANTMANTVersion:
            printf("ANTM ANT Version\r\n");
            printf("%s", "\tVersionData:        ");

            for (Index = 0; Index < EventData->EventData.ANTVersionEventData.VersionDataLength; Index++)
               printf("0x%02X ", EventData->EventData.ANTVersionEventData.VersionData[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMCapabilities:
            printf("ANTM Capabilities\r\n");
            printf("\tMaxChannels:        %d\r\n", EventData->EventData.CapabilitiesEventData.MaxChannels);
            printf("\tMaxNetworks:        %d\r\n", EventData->EventData.CapabilitiesEventData.MaxNetworks);
            printf("\tStandardOptions:    %d\r\n", EventData->EventData.CapabilitiesEventData.StandardOptions);
            printf("\tAdvancedOptions:    %d\r\n", EventData->EventData.CapabilitiesEventData.AdvancedOptions);
            printf("\tAdvancedOptions2:   %d\r\n", EventData->EventData.CapabilitiesEventData.AdvancedOptions2);
            printf("\tReserved:           %d\r\n", EventData->EventData.CapabilitiesEventData.Reserved);
            break;
         case aetANTMBroadcastDataPacket:
            printf("ANTM Broadcast Data Packet\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.BroadcastDataPacketEventData.ChannelNumber);
            printf("%s", "\tData:               ");

            for (Index = 0; Index < EventData->EventData.BroadcastDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.BroadcastDataPacketEventData.Data[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMAcknowledgedDataPacket:
            printf("ANTM Acknowledged Data Packet\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.AcknowledgedDataPacketEventData.ChannelNumber);
            printf("%s", "\tData:               ");

            for (Index = 0; Index < EventData->EventData.AcknowledgedDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.AcknowledgedDataPacketEventData.Data[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMBurstDataPacket:
            printf("ANTM Burst Data Packet\r\n");
            printf("\tSequenceChannelNumber:    %d\r\n", EventData->EventData.BurstDataPacketEventData.SequenceChannelNumber);
            printf("%s", "\tData:               ");

            for (Index = 0; Index < EventData->EventData.BurstDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.BurstDataPacketEventData.Data[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMExtendedBroadcastDataPacket:
            printf("ANTM Extended Broadcast Data Packet\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.ExtendedBroadcastDataPacketEventData.ChannelNumber);
            printf("\tDeviceNumber:       %d\r\n", EventData->EventData.ExtendedBroadcastDataPacketEventData.DeviceNumber);
            printf("\tDeviceType:         %d\r\n", EventData->EventData.ExtendedBroadcastDataPacketEventData.DeviceType);
            printf("\tTransmissionType:   %d\r\n", EventData->EventData.ExtendedBroadcastDataPacketEventData.TransmissionType);
            printf("%s", "\tData:               ");

            for (Index = 0; Index < EventData->EventData.ExtendedBroadcastDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.ExtendedBroadcastDataPacketEventData.Data[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMExtendedAcknowledgedDataPacket:
            printf("ANTM Extended Acknowledged Data Packet\r\n");
            printf("\tChannelNumber:      %d\r\n", EventData->EventData.ExtendedAcknowledgedDataPacketEventData.ChannelNumber);
            printf("\tDeviceNumber:       %d\r\n", EventData->EventData.ExtendedAcknowledgedDataPacketEventData.DeviceNumber);
            printf("\tDeviceType:         %d\r\n", EventData->EventData.ExtendedAcknowledgedDataPacketEventData.DeviceType);
            printf("\tTransmissionType:   %d\r\n", EventData->EventData.ExtendedAcknowledgedDataPacketEventData.TransmissionType);
            printf("%s", "\tData:               ");

            for (Index = 0; Index < EventData->EventData.ExtendedAcknowledgedDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.ExtendedAcknowledgedDataPacketEventData.Data[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMExtendedBurstDataPacket:
            printf("ANTM Extended Burst Data Packet\r\n");
            printf("\tSequenceChannelNumber: %d\r\n", EventData->EventData.ExtendedBurstDataPacketEventData.SequenceChannelNumber);
            printf("\tDeviceNumber:          %d\r\n", EventData->EventData.ExtendedBurstDataPacketEventData.DeviceNumber);
            printf("\tDeviceType:            %d\r\n", EventData->EventData.ExtendedBurstDataPacketEventData.DeviceType);
            printf("\tTransmissionType:      %d\r\n", EventData->EventData.ExtendedBurstDataPacketEventData.TransmissionType);
            printf("%s", "\tData:               ");

            for (Index = 0; Index < EventData->EventData.ExtendedBurstDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.ExtendedBurstDataPacketEventData.Data[Index]);

            printf("%s", "\r\n");
            break;
         case aetANTMRawDataPacket:
            printf("ANTM Raw Data Packet\r\n");
            printf("\tPacket Length: %u\r\n", EventData->EventData.RawDataPacketEventData.DataLength);

            printf("\tData:\r\n\t\t");

            for (Index = 0; Index < EventData->EventData.RawDataPacketEventData.DataLength; Index++)
               printf("0x%02X ", EventData->EventData.RawDataPacketEventData.Data[Index]);

            printf("\r\n");
            break;
         default:
            printf("ANTM: Unknown event received: 0x%08X, 0x%08X.\r\n", (unsigned int) EventData->EventType, EventData->EventLength);
            break;
      }
   }
   else
      printf("\r\nANTM Event Data is NULL.\r\n");

   printf("ANTM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   /* Initialize the default Secure Simple Pairing parameters.          */
   IOCapability     = DEFAULT_IO_CAPABILITY;
   OOBSupport       = FALSE;
   MITMProtection   = DEFAULT_MITM_PROTECTION;

   /* Nothing really to do here aside from running the main application */
   /* code.                                                             */
   UserInterface();

   return 0;
}


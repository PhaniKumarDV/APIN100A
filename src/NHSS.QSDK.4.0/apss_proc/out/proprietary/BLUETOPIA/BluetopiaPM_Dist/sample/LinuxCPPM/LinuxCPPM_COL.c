/*****< LinuxCPPM_COL.c >******************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  LinuxCPPM_COL - Linux Cycling Power Collector Role Sample                 */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>        /* Include for getpid().                           */

#include "LinuxCPPM_COL.h" /* Main Application Prototypes and Constants.      */

#include "SS1BTCPPM.h"     /* CPP Manager Application Programming Interface.  */
#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#define MAX_SUPPORTED_COMMANDS                     (64)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                        (256)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                      (10)  /* Denotes the max   */
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

   /* The following converts an ASCII character ranging from 0 to 9,    */
   /* a to f, or A to F, to its integer equivalent.                     */
#define HexCharToInt(_x)                   ((_x & 0x0F) + ((_x > '9') ? 9 : 0))

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

static unsigned int        EventCallbackID;         /* Variable which holds the        */
                                                    /* current Event Callback ID that  */
                                                    /* is assigned from the Module     */
                                                    /* Manager when the local client   */
                                                    /* registers for Module Events.    */

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

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
};

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
static int DeleteRemoteDevices(ParameterList_t *TempParam);
static int PairWithRemoteDevice(ParameterList_t *TempParam);
static int CancelPairWithRemoteDevice(ParameterList_t *TempParam);
static int UnPairRemoteDevice(ParameterList_t *TempParam);
static int QueryRemoteDeviceServices(ParameterList_t *TempParam);
static int RegisterAuthentication(ParameterList_t *TempParam);
static int UnRegisterAuthentication(ParameterList_t *TempParam);

static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);
static int ConnectWithRemoteDevice(ParameterList_t *TempParam);
static int DisconnectRemoteDevice(ParameterList_t *TempParam);

static int RegisterCollectorEventsCPP(ParameterList_t *TempParam);
static int UnregisterCollectorEventsCPP(ParameterList_t *TempParam);
static int RegisterMeasurementsCPP(ParameterList_t *TempParam);
static int UnregisterMeasurementsCPP(ParameterList_t *TempParam);
static int RegisterVectorsCPP(ParameterList_t *TempParam);
static int UnregisterVectorsCPP(ParameterList_t *TempParam);
static int RegisterProceduresCPP(ParameterList_t *TempParam);
static int UnregisterProceduresCPP(ParameterList_t *TempParam);
static int EnableBroadcastsCPP(ParameterList_t *TempParam);
static int DisableBroadcastsCPP(ParameterList_t *TempParam);
static int ReadSensorFeaturesCPP(ParameterList_t *TempParam);
static int ReadSensorLocationCPP(ParameterList_t *TempParam);
static int WriteSensorControlPointCPP(ParameterList_t *TempParam);
static int QuerySensorsCPP(ParameterList_t *TempParam);
static int QuerySensorInstancesCPP(ParameterList_t *TempParam);

static char *LocationToString(CPPM_Sensor_Location_t Location);
static char * ErrorToString(unsigned int Status);

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

   /* BTPM BAS Manager Callback function prototype.                     */
static void BTPSAPI CPPM_Event_Callback(CPPM_Event_Data_t *EventData, void *CallbackParameter);

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
   printf("\n");

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
   AddCommand("STARTOBSERVATIONSCAN", StartObservationScan);
   AddCommand("STOPOBSERVATIONSCAN", StopObservationScan);
   AddCommand("QUERYREMOTEDEVICELIST", QueryRemoteDeviceList);
   AddCommand("QUERYREMOTEDEVICEPROPERTIES", QueryRemoteDeviceProperties);
   AddCommand("ADDREMOTEDEVICE", AddRemoteDevice);
   AddCommand("DELETEREMOTEDEVICE", DeleteRemoteDevice);
   AddCommand("DELETEREMOTEDEVICES", DeleteRemoteDevices);
   AddCommand("PAIRWITHREMOTEDEVICE", PairWithRemoteDevice);
   AddCommand("CANCELPAIRWITHREMOTEDEVICE", CancelPairWithRemoteDevice);
   AddCommand("UNPAIRREMOTEDEVICE", UnPairRemoteDevice);
   AddCommand("QUERYREMOTEDEVICESERVICES", QueryRemoteDeviceServices);
   AddCommand("REGISTERAUTHENTICATION", RegisterAuthentication);
   AddCommand("UNREGISTERAUTHENTICATION", UnRegisterAuthentication);
   AddCommand("PINCODERESPONSE", PINCodeResponse);
   AddCommand("PASSKEYRESPONSE", PassKeyResponse);
   AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
   AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
   AddCommand("CONNECTWITHREMOTEDEVICE", ConnectWithRemoteDevice);
   AddCommand("DISCONNECTREMOTEDEVICE", DisconnectRemoteDevice);
   AddCommand("REGISTERCOLLECTOREVENTSCPP", RegisterCollectorEventsCPP);
   AddCommand("UNREGISTERCOLLECTOREVENTSCPP", UnregisterCollectorEventsCPP);
   AddCommand("REGISTERMEASUREMENTSCPP", RegisterMeasurementsCPP);
   AddCommand("UNREGISTERMEASUREMENTSCPP", UnregisterMeasurementsCPP);
   AddCommand("REGISTERVECTORSCPP", RegisterVectorsCPP);
   AddCommand("UNREGISTERVECTORSCPP", UnregisterVectorsCPP);
   AddCommand("REGISTERPROCEDURESCPP", RegisterProceduresCPP);
   AddCommand("UNREGISTERPROCEDURESCPP", UnregisterProceduresCPP);
   AddCommand("ENABLEBROADCASTSCPP", EnableBroadcastsCPP);
   AddCommand("DISABLEBROADCASTSCPP", DisableBroadcastsCPP);
   AddCommand("READSENSORFEATURESCPP", ReadSensorFeaturesCPP);
   AddCommand("READSENSORLOCATIONCPP", ReadSensorLocationCPP);
   AddCommand("WRITESENSORCONTROLPOINTCPP", WriteSensorControlPointCPP);
   AddCommand("QUERYSENSORSCPP", QuerySensorsCPP);
   AddCommand("QUERYSENSORINSTANCESCPP", QuerySensorInstancesCPP);

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
      printf("CPPM>");

      /* Retrieve the command entered by the user and store it in the   */
      /* User Input Buffer.  Note that this command will fail if the    */
      /* application receives a signal which cause the standard file    */
      /* streams to be closed.  If this happens the loop will be broken */
      /* out of so the application can exit.                            */
      if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
      {
         /* Start a newline for the results.                            */
         printf("\n");

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
                     printf("Invalid Command.\n");
                     break;
                  case FUNCTION_ERROR:
                     printf("Function Error.\n");
                     break;
               }
            }
            else
               printf("Invalid Input.\n");
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
      for(Index=0,ret_val=String; Index < strlen(String); Index++)
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
      /* First get the initial string length.                           */
      StringLength = strlen(UserInput);

      /* Retrieve the first token in the string, this should be the     */
      /* commmand.                                                      */
      TempCommand->Command = StringParser(UserInput);

      /* Flag that there are NO Parameters for this Command Parse.      */
      TempCommand->Parameters.NumberofParameters = 0;

      /* Check to see if there is a Command                             */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val    = 0;

         /* Adjust the UserInput pointer and StringLength to remove the */
         /* Command from the data passed in before parsing the          */
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
      for(i=0; i<strlen(TempCommand->Command); i++)
         TempCommand->Command[i] = toupper(TempCommand->Command[i]);

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
            for(Index=0,ret_val=NULL; ((Index<NumberCommands) && (!ret_val)); Index++)
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
   int  i;
   char val;
   char *ptr;

   ptr = BoardStr;
   if((ptr[1] == 'x') || (ptr[1] == 'X'))
   {
      ptr += 2;
   }
   for(i=0; i<6; i++)
   {
      val  = (char)(HexCharToInt(*ptr) * 0x10);
      ptr++;
      val += (char)HexCharToInt(*ptr);
      ptr++;
      ((char *)Board_Address)[5-i] = (Byte_t)val;
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
   printf("******************************************************************\n");
   printf("* Command Options: 1) Initialize                                 *\n");
   printf("*                  2) Cleanup                                    *\n");
   printf("*                  3) QueryDebugZoneMask                         *\n");
   printf("*                  4) SetDebugZoneMask                           *\n");
   printf("*                  5) ShutdownService                            *\n");
   printf("*                  6) RegisterEventCallback,                     *\n");
   printf("*                  7) UnRegisterEventCallback                    *\n");
   printf("*                  8) QueryDevicePower                           *\n");
   printf("*                  9) SetDevicePower                             *\n");
   printf("*                  10)QueryLocalDeviceProperties                 *\n");
   printf("*                  11)SetLocalDeviceName                         *\n");
   printf("*                  12)SetLocalClassOfDevice                      *\n");
   printf("*                  13)SetDiscoverable                            *\n");
   printf("*                  14)SetConnectable                             *\n");
   printf("*                  15)SetPairable                                *\n");
   printf("*                  16)StartDeviceDiscovery                       *\n");
   printf("*                  17)StopDeviceDiscovery                        *\n");
   printf("*                  18)StartObservationScan                       *\n");
   printf("*                  19)StopObservationScan                        *\n");
   printf("*                  20)QueryRemoteDeviceList                      *\n");
   printf("*                  21)QueryRemoteDeviceProperties                *\n");
   printf("*                  22)AddRemoteDevice                            *\n");
   printf("*                  23)DeleteRemoteDevice                         *\n");
   printf("*                  24)DeleteRemoteDevices                        *\n");
   printf("*                  25)PairWithRemoteDevice                       *\n");
   printf("*                  26)CancelPairWithRemoteDevice                 *\n");
   printf("*                  27)UnPairRemoteDevice                         *\n");
   printf("*                  28)QueryRemoteDeviceServices                  *\n");
   printf("*                  29)RegisterAuthentication                     *\n");
   printf("*                  30)UnRegisterAuthentication                   *\n");
   printf("*                  31)PINCodeResponse                            *\n");
   printf("*                  32)PassKeyResponse                            *\n");
   printf("*                  33)UserConfirmationResponse                   *\n");
   printf("*                  34)ChangeSimplePairingParameters              *\n");
   printf("*                  35)ConnectWithRemoteDevice                    *\n");
   printf("*                  36)DisconnectRemoteDevice                     *\n");
   printf("*                  37)RegisterCollectorEventsCPP                 *\n");
   printf("*                  38)UnregisterCollectorEventsCPP               *\n");
   printf("*                  39)RegisterMeasurementsCPP                    *\n");
   printf("*                  40)UnregisterMeasurementsCPP                  *\n");
   printf("*                  41)RegisterVectorsCPP                         *\n");
   printf("*                  42)UnregisterVectorsCPP                       *\n");
   printf("*                  43)RegisterProceduresCPP                      *\n");
   printf("*                  44)UnregisterProceduresCPP                    *\n");
   printf("*                  45)EnableBroadcastsCPP                        *\n");
   printf("*                  46)DisableBroadcastsCPP                       *\n");
   printf("*                  47)ReadSensorFeaturesCPP                      *\n");
   printf("*                  48)ReadSensorLocationCPP                      *\n");
   printf("*                  49)WriteSensorControlPointCPP                 *\n");
   printf("*                  50)QuerySensorsCPP                            *\n");
   printf("*                  51)QuerySensorInstancesCPP                    *\n");
   printf("*                  Help, Quit.                                   *\n");
   printf("******************************************************************\n");

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
            printf("BTPM_Initialize() Success: %d.\n", Result);

            /* If the caller would like to Register an Event Callback   */
            /* then we will do that at this time.                       */
            if(TempParam->Params[0].intParam)
            {
               if((Result = DEVM_RegisterEventCallback(DEVM_Event_Callback, NULL)) > 0)
               {
                  printf("DEVM_RegisterEventCallback() Success: %d.\n", Result);

                  /* Note the Callback ID and flag success.             */
                  DEVMCallbackID = (unsigned int)Result;

                  Initialized    = TRUE;

                  ret_val        = 0;
               }
               else
               {
                  /* Error registering the Callback, inform user and    */
                  /* flag an error.                                     */
                  printf("DEVM_RegisterEventCallback() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

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
            printf("BTPM_Initialize() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: Initialize [0/1 - Register for Events].\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Already initialized, flag an error.                            */
      printf("Initialization Failure: Already initialized.\n");

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
      printf("Platform Manager has not been initialized.\n");

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
            printf("DEVM_RegisterEventCallback() Success: %d.\n", Result);

            /* Note the Callback ID and flag success.                   */
            DEVMCallbackID = (unsigned int)Result;

            ret_val        = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("DEVM_RegisterEventCallback() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Device Manager Event Callback already registered.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("DEVM_UnRegisterEventCallback() Success.\n");

         /* Flag that there is no longer a Callback registered.         */
         DEVMCallbackID = 0;

         ret_val        = 0;
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("Device Manager Event Callback is not registered.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
            printf("DEVM_Power%sDevice() Success: %d.\n", TempParam->Params[0].intParam?"On":"Off", Result);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Powering On/Off the device, inform the user.       */
            printf("DEVM_Power%sDevice() Failure: %d, %s.\n", TempParam->Params[0].intParam?"On":"Off", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDevicePower [0/1 - Power Off/Power On].\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
         printf("DEVM_QueryDevicePowerState() Success: %s.\n", Result?"On":"Off");
      else
         printf("DEVM_QueryDevicePowerState() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

      /* Flag success to the caller.                                    */
      ret_val = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
            printf("BTPM_SetDebugZoneMask(%s) Success: 0x%08lX.\n", TempParam->Params[0].intParam?"Remote":"Local", (unsigned long)TempParam->Params[1].intParam);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Debug Zone Mask, inform the user.         */
            printf("BTPM_SetDebugZoneMask(%s) Failure: %d, %s.\n", TempParam->Params[0].intParam?"Remote":"Local", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: SetDebugZoneMask [0/1 - Local/Service] [Debug Zone Mask].\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
            printf("BTPM_QueryDebugZoneMask(%s) Success: 0x%08lX.\n", TempParam->Params[0].intParam?"Remote":"Local", DebugZoneMask);

            /* Return success to the caller.                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Debug Zone Mask, inform the user.         */
            printf("BTPM_QueryDebugZoneMask(%s, %d) Failure: %d, %s.\n", TempParam->Params[0].intParam?"Remote":"Local", (TempParam->NumberofParameters > 1)?TempParam->Params[1].intParam:0, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         printf("Usage: QueryDebugZoneMask [0/1 - Local/Service] [Page Number - optional, default 0].\n");

         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
         printf("BTPM_ShutdownService() Success: %d.\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error shutting down the service, inform the user and flag an*/
         /* error.                                                      */
         printf("BTPM_ShutdownService() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
         printf("DEVM_QueryLocalDeviceProperties() Success: %d.\n", Result);

         /* Next, go ahead and display the properties.                  */
         DisplayLocalDeviceProperties(0, &LocalDeviceProperties);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error querying the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_QueryLocalDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

      printf("Attempting to set Device Name to: \"%s\".\n", LocalDeviceProperties.DeviceName);

      if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DEVICE_NAME, &LocalDeviceProperties)) >= 0)
      {
         printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error updating the Local Device Properties, inform the user */
         /* and flag an error.                                          */
         printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to set Class Of Device to: 0x%02X%02X%02X.\n", LocalDeviceProperties.ClassOfDevice.Class_of_Device0, LocalDeviceProperties.ClassOfDevice.Class_of_Device1, LocalDeviceProperties.ClassOfDevice.Class_of_Device2);

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CLASS_OF_DEVICE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetLocalClassOfDevice [Class of Device].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
               printf("Attempting to set Discoverability Mode: Limited (%d Seconds).\n", LocalDeviceProperties.DiscoverableModeTimeout);
            else
               printf("Attempting to set Discoverability Mode: General.\n");
         }
         else
            printf("Attempting to set Discoverability Mode: None.\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_DISCOVERABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetDiscoverable [Enable/Disable] [Timeout (Enable only)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
            if(LocalDeviceProperties.DiscoverableModeTimeout)
               printf("Attempting to set Connectability Mode: Connectable (%d Seconds).\n", LocalDeviceProperties.ConnectableModeTimeout);
            else
               printf("Attempting to set Connectability Mode: Connectable.\n");
         }
         else
            printf("Attempting to set Connectability Mode: Non-Connectable.\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_CONNECTABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetConnectable [Enable/Disable] [Timeout (Enable only)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
               printf("Attempting to set Pairability Mode: Pairable (%d Seconds).\n", LocalDeviceProperties.PairableModeTimeout);
            else
               printf("Attempting to set Pairability Mode: Pairable.\n");
         }
         else
            printf("Attempting to set Pairability Mode: Non-Pairable.\n");

         if((Result = DEVM_UpdateLocalDeviceProperties(DEVM_UPDATE_LOCAL_DEVICE_PROPERTIES_PAIRABLE_MODE, &LocalDeviceProperties)) >= 0)
         {
            printf("DEVM_UpdateLocalDeviceProperties() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error updating the Local Device Properties, inform the   */
            /* user and flag an error.                                  */
            printf("DEVM_UpdateLocalDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SetPairable [Enable/Disable] [Timeout (Enable only)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
            printf("Attempting to Start Discovery (%d Seconds).\n", TempParam->Params[1].intParam);
         else
            printf("Attempting to Start Discovery (INDEFINITE).\n");

         /* Check to see if we are doing an LE or BR/EDR Discovery      */
         /* Process.                                                    */
         if((Boolean_t)TempParam->Params[0].intParam)
         {
            if((Result = DEVM_StartDeviceScan(TempParam->Params[1].intParam)) >= 0)
            {
               printf("DEVM_StartDeviceScan() Success: %d.\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
               printf("DEVM_StartDeviceScan() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            if((Result = DEVM_StartDeviceDiscovery(TempParam->Params[1].intParam)) >= 0)
            {
               printf("DEVM_StartDeviceDiscovery() Success: %d.\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
               printf("DEVM_StartDeviceDiscovery() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartDeviceDiscovery [Type (1 = LE, 0 = BR/EDR)] [Duration].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
               printf("DEVM_StopDeviceScan() Success: %d.\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error stopping Device Discovery, inform the user and  */
               /* flag an error.                                        */
               printf("DEVM_StopDeviceScan() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Initialized, go ahead and attempt to stop BR/EDR Device  */
            /* Discovery.                                               */
            if((Result = DEVM_StopDeviceDiscovery()) >= 0)
            {
               printf("DEVM_StopDeviceDiscovery() Success: %d.\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error stopping Device Discovery, inform the user and  */
               /* flag an error.                                        */
               printf("DEVM_StopDeviceDiscovery() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StopDeviceDiscovery [Type (1 = LE, 0 = BR/EDR)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
            printf("DEVM_StartObservationScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error starting the observation scan process, inform the  */
            /* user and flag an error.                                  */
            printf("DEVM_StartObservationScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: StartObservationScan [Reporting Frequency] [Scan Window (optional)] [Scan Interval (must be specified if Window is)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
         printf("DEVM_StopDeviceScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Success: %d.\n", Result);

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error stopping the observation scan process, inform the user*/
         /* and flag an error.                                          */
         printf("DEVM_StopDeviceScan(DEVM_OBSERVATION_SCAN_FLAGS_LOW_ENERGY) Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
         printf("Attempting Query %d Devices.\n", TempParam->Params[0].intParam);

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
               printf("DEVM_QueryRemoteDeviceList() Success: %d, Total Number Devices: %d.\n", Result, TotalNumberDevices);

               if((Result) && (BD_ADDRList))
               {
                  printf("Returned device list (%d Entries):\r\n", Result);

                  for(Index = 0; Index < Result; Index++)
                  {
                     BD_ADDRToStr(BD_ADDRList[Index], Buffer);

                     printf("%2d. %s\r\n", (Index + 1), Buffer);
                  }
               }

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
               printf("DEVM_QueryRemoteDeviceList() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Free any memory that was allocated.                      */
            if(BD_ADDRList)
               BTPS_FreeMemory(BD_ADDRList);
         }
         else
         {
            /* Unable to allocate memory for List.                      */
            printf("Unable to allocate memory for %d Devices.\n", TempParam->Params[0].intParam);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceList [Number of Devices] [Filter (Optional)] [COD Filter (Optional)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Query Device Properties: %s, ForceUpdate: %s.\n", TempParam->Params[0].strParam, ForceUpdate?"TRUE":"FALSE");

         if((Result = DEVM_QueryRemoteDeviceProperties(BD_ADDR, ForceUpdate, &RemoteDeviceProperties)) >= 0)
         {
            printf("DEVM_QueryRemoteDeviceProperties() Success: %d.\n", Result);

            /* Display the Remote Device Properties.                    */
            DisplayRemoteDeviceProperties(0, &RemoteDeviceProperties);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Querying Remote Device, inform the user and flag an*/
            /* error.                                                   */
            printf("DEVM_QueryRemoteDeviceProperties() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceProperties [BD_ADDR] [Force Update].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Add Device: %s.\n", TempParam->Params[0].strParam);

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
            printf("DEVM_AddRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Adding Remote Device, inform the user and flag an  */
            /* error.                                                   */
            printf("DEVM_AddRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: AddRemoteDevice [BD_ADDR] [[COD (Optional)] [Friendly Name (Optional)] [Application Info (Optional)]].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Delete Device: %s.\n", TempParam->Params[0].strParam);

         if((Result = DEVM_DeleteRemoteDevice(BD_ADDR)) >= 0)
         {
            printf("DEVM_DeleteRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Deleting Remote Device, inform the user and flag an*/
            /* error.                                                   */
            printf("DEVM_DeleteRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteRemoteDevice [BD_ADDR].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
         printf("Attempting to Delete Remote Devices, Filter %d.\n", TempParam->Params[0].intParam);

         if((Result = DEVM_DeleteRemoteDevices(TempParam->Params[0].intParam)) >= 0)
         {
            printf("DEVM_DeleteRemoteDevices() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Deleting Remote Devices, inform the user and flag  */
            /* an error.                                                */
            printf("DEVM_DeleteRemoteDevices() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DeleteRemoteDevices [Device Delete Filter].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Pair With Remote %s Device: %s.\n", (Flags & DEVM_PAIR_WITH_REMOTE_DEVICE_FLAGS_LOW_ENERGY)?"LE":"BR/EDR", TempParam->Params[0].strParam);

         if((Result = DEVM_PairWithRemoteDevice(BD_ADDR, Flags)) >= 0)
         {
            printf("DEVM_PairWithRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Pairing with Remote Device, inform the user and    */
            /* flag an error.                                           */
            printf("DEVM_PairWithRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: PairWithRemoteDevice [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Pair Flags (optional)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Cancel Pair With Remote Device: %s.\n", TempParam->Params[0].strParam);

         if((Result = DEVM_CancelPairWithRemoteDevice(BD_ADDR)) >= 0)
         {
            printf("DEVM_CancelPairWithRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Cancelling Pairing with Remote Device, inform the  */
            /* user and flag an error.                                  */
            printf("DEVM_CancelPairWithRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: CancelPairWithRemoteDevice [BD_ADDR].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Un-Pair Remote Device: %s.\n", TempParam->Params[0].strParam);

         if(TempParam->Params[1].intParam)
            UnPairFlags = DEVM_UNPAIR_REMOTE_DEVICE_FLAGS_LOW_ENERGY;
         else
            UnPairFlags = 0;

         if((Result = DEVM_UnPairRemoteDevice(BD_ADDR, UnPairFlags)) >= 0)
         {
            printf("DEVM_UnPairRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Un-Pairing with Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_UnPairRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: UnPairRemoteDevice [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] .\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting Query Remote Device %s For %s Services.\n", TempParam->Params[0].strParam, (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_LOW_ENERGY)?"GATT":"SDP");

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
                  printf("DEVM_QueryRemoteDeviceServices() Success: %d, Total Number Service Bytes: %d.\n", Result, (QueryFlags & DEVM_QUERY_REMOTE_DEVICE_SERVICES_FLAGS_FORCE_UPDATE)?0:TotalServiceSize);

                  /* Now convert the Raw Data to parsed data.           */
                  if((Result) && (ServiceData))
                  {
                     printf("Returned Service Data (%d Bytes):\n", Result);

                     for(Index=0;Index<Result;Index++)
                        printf("%02X", ServiceData[Index]);

                     printf("\n");
                     printf("\n");

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
                           printf("DEVM_ConvertRawServicesStreamToParsedServicesData() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));
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
                           printf("DEVM_ConvertRawSDPStreamToParsedSDPData() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));
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
                  printf("DEVM_QueryRemoteDeviceServices() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }

               /* Free any memory that was allocated.                   */
               if(ServiceData)
                  BTPS_FreeMemory(ServiceData);
            }
            else
            {
               /* Unable to allocate memory for List.                   */
               printf("Unable to allocate memory for %d Service Bytes.\n", TempParam->Params[2].intParam);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Force Update] [Bytes to Query (specified if Force is 0)].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryRemoteDeviceServices [BD_ADDR] [Type (1 = LE, 0 = BR/EDR)] [Force Update] [Bytes to Query (specified if Force is 0)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
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
         printf("DEVM_RegisterAuthentication() Success: %d.\n", Result);

         /* Note the Authentication Callback ID.                        */
         AuthenticationCallbackID = (unsigned int)Result;

         /* Flag success.                                               */
         ret_val                  = 0;
      }
      else
      {
         /* Error Registering for Authentication, inform the user and   */
         /* flag an error.                                              */
         printf("DEVM_RegisterAuthentication() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

      printf("DEVM_UnRegisterAuthentication() Success.\n");

      /* Clear the Authentication Callback ID.                          */
      AuthenticationCallbackID = 0;

      /* Flag success.                                                  */
      ret_val                  = 0;
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
   PIN_Code_t                        PINCode;
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
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
               printf("DEVM_AuthenticationResponse(), Pin Code Response Success.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PINCodeResponse [PIN Code].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue PIN Code Authentication Response: Authentication is not currently in progress.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
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
               printf("DEVM_AuthenticationResponse(), Passkey Response Success.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: PassKeyResponse [Numeric Passkey (0 - 999999)].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue Pass Key Authentication Response: Authentication is not currently in progress.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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
   DEVM_Authentication_Information_t AuthenticationResponseInformation;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
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
               printf("DEVM_AuthenticationResponse(), User Confirmation Response Success.\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            printf("Usage: UserConfirmationResponse [Confirmation (0 = No, 1 = Yes)].\n");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         printf("Unable to issue User Confirmation Authentication Response: Authentication is not currently in progress.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Connect With (%s) Remote Device: %s (Flags = 0x%08lX).\n", (ConnectFlags & DEVM_CONNECT_WITH_REMOTE_DEVICE_FORCE_LOW_ENERGY)?"LE":"BR/EDR", TempParam->Params[0].strParam, ConnectFlags);

         if((Result = DEVM_ConnectWithRemoteDevice(BD_ADDR, ConnectFlags)) >= 0)
         {
            printf("DEVM_ConnectWithRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Connecting With Remote Device, inform the user and */
            /* flag an error.                                           */
            printf("DEVM_ConnectWithRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ConnectWithRemoteDevice [BD_ADDR] [Connect LE (1 = LE, 0 = BR/EDR)] [ConnectFlags (Optional)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         printf("Attempting to Disconnect Remote Device: %s.\n", TempParam->Params[0].strParam);

         if((Result = DEVM_DisconnectRemoteDevice(BD_ADDR, DisconnectFlags)) >= 0)
         {
            printf("DEVM_DisconnectRemoteDevice() Success: %d.\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error Disconnecting Remote Device, inform the user and   */
            /* flag an error.                                           */
            printf("DEVM_DisconnectRemoteDevice() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: DisconnectRemoteDevice [BD_ADDR] [LE Device (1= LE, 0 = BR/EDR)] [Force Flag (Optional).\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

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

         /* Finally map the Man in the Middle (MITM) Protection value.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capablities.                 */
         printf("Current I/O Capabilities: %s, MITM Protection: %s.\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE");

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangeSimplePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output)] [MITM Requirement (0 = No, 1 = Yes)].\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function registers to receive CPP collector events. */
   /* It returns zero or a negative error code.                         */
static int RegisterCollectorEventsCPP(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* If there is an Event Callback Registered, then we need to flag */
      /* an error.                                                      */
      if(!EventCallbackID)
      {
         /* Callback has not been registered, go ahead and attempt to   */
         /* register it.                                                */
         if((Result = CPPM_Register_Collector_Event_Callback(CPPM_Event_Callback, NULL)) > 0)
         {
            printf("CPPM_Register_Collector_Event_Callback() Success: %d\n", Result);

            /* Note the Callback ID and flag success.                   */
            EventCallbackID = (unsigned int)Result;

            ret_val         = 0;
         }
         else
         {
            /* Error registering the Callback, inform user and flag an  */
            /* error.                                                   */
            printf("CPPM_Register_Collector_Event_Callback() Failure: %d, %s\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Callback already registered, go ahead and notify the user.  */
         printf("CPP Manager Event Callback already registered.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function unregisters a CPP collectors.              */
   /* It returns zero or a negative error code.                         */
static int UnregisterCollectorEventsCPP(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next, check to make sure that there is an Event Callback       */
      /* already registered.                                            */
      if(EventCallbackID)
      {
         /* Callback has been registered, go ahead and attempt to       */
         /* un-register it.                                             */
         CPPM_Unregister_Collector_Event_Callback(EventCallbackID);

         printf("CPPM_Unregister_Collector_Event_Callback() Success\n");

         /* Flag that there is no longer a Callback registered.         */
         EventCallbackID = 0;

         ret_val            = 0;
      }
      else
      {
         /* Callback NOT registered, go ahead and notify the user.      */
         printf("CPP Manager Event Callback is not registered.\n");

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Platform Manager has not been initialized.\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

static int RegisterMeasurementsCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 2))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Register_Measurements(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam, TempParam->Params[2].intParam ? TRUE : FALSE);

	    printf("CPPM_Register_Measurements Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Register_Measurements [BD_ADDR] [InstanceID] [1 (Enable Notifications) 0 (Register only)]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int UnregisterMeasurementsCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Unregister_Measurements(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Unregister_Measurements Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Unregister_Measurements [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int RegisterVectorsCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 2))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Register_Vectors(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam, TempParam->Params[2].intParam ? TRUE : FALSE);

	    printf("CPPM_Register_Vectors Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Register_Vectors [BD_ADDR] [InstanceID] [1 (Enable Notifications) 0 (Register only)]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int UnregisterVectorsCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Unregister_Vectors(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Unregister_Vectors Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Unregister_Vectors [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int RegisterProceduresCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 2))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Register_Procedures(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam, TempParam->Params[2].intParam ? TRUE : FALSE);

	    printf("CPPM_Register_Procedures Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Register_Procedures [BD_ADDR] [InstanceID] [1 (Enable Indications) 0 (Register only)]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int UnregisterProceduresCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Unregister_Procedures(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Unregister_Procedures Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Unregister_Procedures [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int EnableBroadcastsCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Enable_Broadcasts(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Enable_Broadcasts Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Enable_Broadcasts [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int DisableBroadcastsCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Disable_Broadcasts(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Disable_Broadcasts Result: %d, %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result < 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Disable_Broadcasts [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int ReadSensorFeaturesCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Read_Sensor_Features(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Read_Sensor_Features Result: %d, %s\n", Result, Result == 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result <= 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Read_Sensor_Features [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int ReadSensorLocationCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t BluetoothAddress;
   int       Result;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters > 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            /* Submit the command.                                      */
            Result = CPPM_Read_Sensor_Location(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam);

            printf("CPPM_Read_Sensor_Location Result: %d, %s\n", Result, Result == 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result <= 0)
               Result = FUNCTION_ERROR;
            else
               Result = 0;
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Read_Sensor_Location [BD_ADDR] [InstanceID]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int WriteSensorControlPointCPP(ParameterList_t *TempParam)
{
   BD_ADDR_t              BluetoothAddress;
   int                    Result;
   CPPM_Procedure_Data_t  ProcedureData;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         if((TempParam) && (TempParam->NumberofParameters >= 3))
         {
            Result = 0;

            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &BluetoothAddress);

            ProcedureData.Opcode = TempParam->Params[2].intParam;

            switch(TempParam->Params[2].intParam)
            {
               case CPPM_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.CumulativeValue = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               case CPPM_CONTROL_POINT_OPCODE_UPDATE_SENSOR_LOCATION:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.SensorLocation = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               case CPPM_CONTROL_POINT_OPCODE_SET_CRANK_LENGTH:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.CrankLength = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               case CPPM_CONTROL_POINT_OPCODE_SET_CHAIN_LENGTH:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.ChainLength = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               case CPPM_CONTROL_POINT_OPCODE_SET_CHAIN_WEIGHT:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.ChainWeight = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               case CPPM_CONTROL_POINT_OPCODE_SET_SPAN_LENGTH:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.SpanLength = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               case CPPM_CONTROL_POINT_OPCODE_MASK_MEASUREMENT_CHARACTERISTIC_CONTENT:
                  if(TempParam->NumberofParameters > 3)
                     ProcedureData.ProcedureParameter.ContentMask = TempParam->Params[3].intParam;
                  else
                     Result = FUNCTION_ERROR;
                  break;
               default:
                  break;
            }

            if(!Result)
            {
               /* Submit the command.                                   */
               Result = CPPM_Write_Sensor_Control_Point(EventCallbackID, BluetoothAddress, TempParam->Params[1].intParam, ProcedureData);

               printf("CPPM_Write_Sensor_Control_Point Result: %d, %s\n", Result, Result == 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

               if(Result <= 0)
                  Result = FUNCTION_ERROR;
               else
                  Result = 0;
            }
            else
            {
               printf("Usage: CPPM_Write_Sensor_Control_Point [BD_ADDR] [InstanceID] [Procedure Opcode] [Procedure Parameter(if necessary)]\n");
            }
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Write_Sensor_Control_Point [BD_ADDR] [InstanceID] [Procedure Opcode] [Procedure Parameter(if necessary)]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int QuerySensorsCPP(ParameterList_t *TempParam)
{
   int           Result;
   unsigned int  Index;
   unsigned int  NumberOfSensors;
   BD_ADDR_t    *RemoteSensors;

   if(Initialized)
   {
      if(EventCallbackID)
      {
         /* Submit the command.                                         */
         Result = CPPM_Query_Sensors(EventCallbackID, NULL, NULL);

         printf(" CPPM_Query_Sensors Result: %d  %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

         if(Result > 0)
         {
            RemoteSensors = NULL;

            if((RemoteSensors = (BD_ADDR_t *)malloc(Result * sizeof(BD_ADDR_t))) != NULL)
            {
               NumberOfSensors = Result;

               Result = CPPM_Query_Sensors(EventCallbackID, &NumberOfSensors, RemoteSensors);

               printf(" CPPM_Query_Sensors Result: %d  %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

               if(Result > 0)
               {
                  printf("\n");
                  printf("  Number Of Sensors:          %u\n", Result);
                  printf("  Number Of Returned Sensors: %u\n", NumberOfSensors);
                  printf("  Sensor Addresses:\n");

                  for(Index = 0; Index < NumberOfSensors; Index++)
                  {
                     printf("                              %02X%02X%02X%02X%02X%02X\n", RemoteSensors[Index].BD_ADDR5, RemoteSensors[Index].BD_ADDR4,
                                                                                        RemoteSensors[Index].BD_ADDR3, RemoteSensors[Index].BD_ADDR2,
                                                                                        RemoteSensors[Index].BD_ADDR1, RemoteSensors[Index].BD_ADDR0);
                  }
               }

               Result = 0;
            }
            else
               Result = FUNCTION_ERROR;
         }
         else
         {
            if(Result == 0)
            {
               printf("\n");
               printf("  Number Of Sensors:          %u\n", Result);
            }
            else
            {
               Result = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
}

static int QuerySensorInstancesCPP(ParameterList_t *TempParam)
{
   int                Result;
   unsigned int       Index;
   BD_ADDR_t          Sensor;
   unsigned int       NumberOfInstances;
   Instance_Record_t *Instances;

   if(Initialized)
   {
      if(EventCallbackID)
      {
          if((TempParam) && (TempParam->NumberofParameters >= 1))
         {
            /* Convert the parameter into a Bluetooth Address.          */
            StrToBD_ADDR(TempParam->Params[0].strParam, &Sensor);

            /* Submit the command.                                      */
            Result = CPPM_Query_Sensor_Instances(EventCallbackID, Sensor, NULL, NULL);

            printf(" CPPM_Query_Sensor_Instances Result: %d  %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

            if(Result > 0)
            {
               Instances = NULL;

               if((Instances = (Instance_Record_t *)malloc(Result * sizeof(Instance_Record_t))) != NULL)
               {
                  NumberOfInstances = Result;

                  Result = CPPM_Query_Sensor_Instances(EventCallbackID, Sensor, &NumberOfInstances, Instances);

                  printf(" CPPM_Query_Sensor_Instances Result: %d  %s\n", Result, Result >= 0 ? "Success" : ERR_ConvertErrorCodeToString(Result));

                  if(Result > 0)
                  {
                     printf("\n");
                     printf("  Sensor:                       %02X%02X%02X%02X%02X%02X\n", Sensor.BD_ADDR5, Sensor.BD_ADDR4, Sensor.BD_ADDR3, Sensor.BD_ADDR2, Sensor.BD_ADDR1, Sensor.BD_ADDR0);
                     printf("  Number Of Instances:          %u\n", Result);
                     printf("  Number Of Returned Instances: %u\n", NumberOfInstances);
                     printf("  Sensor Instances:\n");

                     for(Index = 0; Index < NumberOfInstances; Index++)
                     {
                        printf("   Instance ID:                 %u\n", Instances[Index].InstanceID);
                        printf("   State Mask:\n");

                        if(Instances[Index].StateMask & CPPM_SENSOR_STATE_MEASUREMENT_ENABLED)
                           printf("                                Measurement Notifications Enabled\n");

                        if(Instances[Index].StateMask & CPPM_SENSOR_STATE_VECTOR_ENABLED)
                           printf("                                Vector Notifications Enabled\n");

                        if(Instances[Index].StateMask & CPPM_SENSOR_STATE_CONTROL_POINT_ENABLED)
                           printf("                                Control Point Indications Enabled\n");

                        if(Instances[Index].StateMask & CPPM_SENSOR_STATE_BROADCAST_ENABLED)
                           printf("                                Measurement Broadcasts Enabled\n");

                        printf("   Sensor Features:             %lu\n", Instances[Index].FeatureMask);

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_PEDAL_POWER_BALANCE_SUPPORTED)
                           printf("                                power balance supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_ACCUMULATED_TORQUE_SUPPORTED)
                           printf("                                accumulated torque supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_WHEEL_REVOLUTION_DATA_SUPPORTED)
                           printf("                                wheel revolution data supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_CRANK_REVOLUTION_DATA_SUPPORTED)
                           printf("                                crank revolution data supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_EXTREME_MAGNITUDES_SUPPORTED)
                           printf("                                extreme magnitudes supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_EXTREME_ANGLES_SUPPORTED)
                           printf("                                extreme angles supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_TOP_AND_BOTTOM_DEAD_SPOT_ANGLES_SUPPORTED)
                           printf("                                top and bottom dead spot angles supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_ACCUMULATED_ENERGY_SUPPORTED)
                           printf("                                accumulated energy supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_OFFSET_COMPENSATION_INDICATOR_SUPPORTED)
                           printf("                                offset compensation indicator supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_OFFSET_COMPENSATION_SUPPORTED)
                           printf("                                offset compensation supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_MEASUREMENT_CHARACTERISTIC_CONTENT_MASKING_SUPPORTED)
                           printf("                                measurement characteristic content masking supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED)
                           printf("                                multiple sensor locations supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_CRANK_LENGTH_ADJUSTMENT_SUPPORTED)
                           printf("                                crank length adjustment supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_CHAIN_LENGTH_ADJUSTMENT_SUPPORTED)
                           printf("                                chain length adjustment supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_CHAIN_WEIGHT_ADJUSTMENT_SUPPORTED)
                           printf("                                chain weight adjustment supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_SPAN_LENGTH_ADJUSTMENT_SUPPORTED)
                           printf("                                span length adjustment supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_SENSOR_MEASUREMENT_CONTEXT_TORQUE)
                           printf("                                sensor measurement context: torque\n");
                        else
                           printf("                                sensor measurement context: force\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_INSTANTANEOUS_MEASUREMENT_DIRECTION_SUPPORTED)
                           printf("                                instantaneous measurement direction supported\n");

                        if(Instances[Index].FeatureMask & CPPM_FEATURE_FACTORY_CALIBRATION_DATE_SUPPORTED)
                           printf("                                factory calibration date supported\n");

                        printf("   Sensor Location:             %u\n", Instances[Index].SensorLocation);
                        printf("                                %s\n", LocationToString(Instances[Index].SensorLocation));
                     }
                  }

                  Result = 0;
               }
               else
                  Result = FUNCTION_ERROR;
            }
            else
            {
               if(Result == 0)
               {
                  printf("\n");
                  printf("  Sensor:                     %02X%02X%02X%02X%02X%02X\n", Sensor.BD_ADDR5, Sensor.BD_ADDR4, Sensor.BD_ADDR3, Sensor.BD_ADDR2, Sensor.BD_ADDR1, Sensor.BD_ADDR0);
                  printf("  Number Of Sensors:          %u\n", Result);
               }
               else
               {
                  Result = FUNCTION_ERROR;
               }
            }
         }
         else
         {
            Result = FUNCTION_ERROR;

            printf("Usage: CPPM_Query_Sensor_Instances [BD_ADDR]\n");
         }
      }
      else
      {
         Result = FUNCTION_ERROR;

         printf("An event callback must be registered with the CPPM Manager. Use RegisterCollectorEventsCPP.\n");
      }
   }
   else
   {
      printf("Platform Manager has not been initialized.\n");

      Result = FUNCTION_ERROR;
   }

   return(Result);
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

         printf("BD_ADDR:      %s\n", Buffer);
         printf("HCI Ver:      0x%04X\n", (Word_t)LocalDeviceProperties->HCIVersion);
         printf("HCI Rev:      0x%04X\n", (Word_t)LocalDeviceProperties->HCIRevision);
         printf("LMP Ver:      0x%04X\n", (Word_t)LocalDeviceProperties->LMPVersion);
         printf("LMP Sub Ver:  0x%04X\n", (Word_t)LocalDeviceProperties->LMPSubVersion);
         printf("Device Man:   0x%04X (%s)\n", (Word_t)LocalDeviceProperties->DeviceManufacturer, DEVM_ConvertManufacturerNameToString(LocalDeviceProperties->DeviceManufacturer));
         printf("Device Flags: 0x%08lX\n", LocalDeviceProperties->LocalDeviceFlags);
      }
      else
      {
         if(UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS)
            printf("Device Flags: 0x%08lX\n", LocalDeviceProperties->LocalDeviceFlags);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_BLE_ADDRESS))
      {
         switch(LocalDeviceProperties->BLEAddressType)
         {
            case atPublic:
               printf("BLE Address Type: %s\n", "Public");
               break;
            case atStatic:
               printf("BLE Address Type: %s\n", "Static");
               break;
            case atPrivate_Resolvable:
               printf("BLE Address Type: %s\n", "Resolvable Random");
               break;
            case atPrivate_NonResolvable:
               printf("BLE Address Type: %s\n", "Non-Resolvable Random");
               break;
         }

         BD_ADDRToStr(LocalDeviceProperties->BLEBD_ADDR, Buffer);

         printf("BLE BD_ADDR:      %s\n", Buffer);
      }

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE))
         printf("COD:          0x%02X%02X%02X\n", LocalDeviceProperties->ClassOfDevice.Class_of_Device0, LocalDeviceProperties->ClassOfDevice.Class_of_Device1, LocalDeviceProperties->ClassOfDevice.Class_of_Device2);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:  %s\n", (LocalDeviceProperties->DeviceNameLength)?LocalDeviceProperties->DeviceName:"");

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DISCOVERABLE_MODE))
         printf("Disc. Mode:   %s, 0x%08X\n", LocalDeviceProperties->DiscoverableMode?"TRUE ":"FALSE", LocalDeviceProperties->DiscoverableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_CONNECTABLE_MODE))
         printf("Conn. Mode:   %s, 0x%08X\n", LocalDeviceProperties->ConnectableMode?"TRUE ":"FALSE", LocalDeviceProperties->ConnectableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_PAIRABLE_MODE))
         printf("Pair. Mode:   %s, 0x%08X\n", LocalDeviceProperties->PairableMode?"TRUE ":"FALSE", LocalDeviceProperties->PairableModeTimeout);

      if((!UpdateMask) || (UpdateMask & DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
      {
         printf("LE Scan Mode:    %s, 0x%08X\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_SCANNING_IN_PROGRESS)?"TRUE":"FALSE", LocalDeviceProperties->ScanTimeout);
         printf("LE Obsv Mode:    %s\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_OBSERVATION_IN_PROGRESS)?"TRUE":"FALSE");
         printf("LE Adv Mode:     %s, 0x%08X\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_ADVERTISING_IN_PROGRESS)?"TRUE":"FALSE", LocalDeviceProperties->AdvertisingTimeout);
         printf("LE Slv Mode:     %s\n", (LocalDeviceProperties->LocalDeviceFlags & DEVM_LOCAL_DEVICE_FLAGS_LE_ROLE_IS_CURRENTLY_SLAVE)?"In Slave Mode":"Not in Slave Mode");
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
      printf("BD_ADDR:             %s\n", Buffer);

      if(LEFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY)
      {
         /* Print the address type.                                     */
         switch(RemoteDeviceProperties->BLEAddressType)
         {
            default:
            case atPublic:
               printf("Address Type:        %s\n", "Public");
               break;
            case atStatic:
               printf("Address Type:        %s\n", "Static");
               break;
            case atPrivate_Resolvable:
               printf("Address Type:        %s\n", "Resolvable Random Address.");
               break;
            case atPrivate_NonResolvable:
               printf("Address Type:        %s\n", "Non-resolvable Random Address.");
               break;
         }
      }

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:         %s\n", (RemoteDeviceProperties->DeviceNameLength)?RemoteDeviceProperties->DeviceName:"");

      if(LEFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY)
         printf("LE Type:             %s\n", (!SingleMode)?"Dual Mode":"Single Mode");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
         printf("Device Flags:        0x%08lX\n", RemoteDeviceProperties->RemoteDeviceFlags);

      /* Print the LE Information.                                      */
      if(LEFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY)
      {
         if(((!UpdateMask) && (!COMPARE_NULL_BD_ADDR(RemoteDeviceProperties->PriorResolvableBD_ADDR))) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PRIOR_RESOLVABLE_ADDRESS))
         {
            BD_ADDRToStr(RemoteDeviceProperties->PriorResolvableBD_ADDR, Buffer);
            printf("Resolv. BD_ADDR:     %s\n\n", Buffer);
         }

         if(((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_APPEARANCE_KNOWN)) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_APPEARANCE))
            printf("Device Appearance:   %u\n", RemoteDeviceProperties->DeviceAppearance);

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_RSSI))
            printf("LE RSSI:             %d\n", RemoteDeviceProperties->LE_RSSI);

         if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_TX_POWER_KNOWN))
            printf("LE Trans. Power:     %d\n", RemoteDeviceProperties->LETransmitPower);

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_PAIRING_STATE))
            printf("LE Paired State :    %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED_OVER_LE)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_CONNECTION_STATE))
            printf("LE Connect State:    %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED_OVER_LE)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_ENCRYPTION_STATE))
            printf("LE Encrypt State:    %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LINK_CURRENTLY_ENCRYPTED)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_SERVICES_STATE))
            printf("GATT Services Known: %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_SERVICES_KNOWN)?"TRUE":"FALSE");

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_OBSERVED)) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LAST_OBSERVED_KNOWN))
            printf("LE Last Observed:    %u/%u/%u %u:%u:%u.%u\n", RemoteDeviceProperties->BLELastObservedTime.Month, RemoteDeviceProperties->BLELastObservedTime.Day, RemoteDeviceProperties->BLELastObservedTime.Year, RemoteDeviceProperties->BLELastObservedTime.Hour, RemoteDeviceProperties->BLELastObservedTime.Minute, RemoteDeviceProperties->BLELastObservedTime.Second, RemoteDeviceProperties->BLELastObservedTime.Milliseconds);

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_ADV_PACKET)) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LE_LAST_ADV_PACKET_KNOWN))
         {
            switch(RemoteDeviceProperties->BLELastAdvertisingPacket)
            {
               case rtConnectableUndirected:
                  printf("LE Last Adv. Packet: Connectable undirected advertisement\n");
                  break;
               case rtConnectableDirected:
                  printf("LE Last Adv. Packet: Connectable directed advertisement\n");
                  break;
               case rtScannableUndirected:
                  printf("LE Last Adv. Packet: Scannable undirected advertisement\n");
                  break;
               default:
               case rtNonConnectableUndirected:
                  printf("LE Last Adv. Packet: Non-connectable undirected advertisement\n");
                  break;
            }
         }
      }

      /* Print the BR/EDR Only information.                             */
      if(!SingleMode)
      {
         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_RSSI))
            printf("RSSI:                %d\n", RemoteDeviceProperties->RSSI);

         if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_TX_POWER_KNOWN))
            printf("Trans. Power:        %d\n", RemoteDeviceProperties->TransmitPower);

         if(((!UpdateMask) || ((UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_APPLICATION_DATA) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_APPLICATION_DATA_VALID))))
         {
            printf("Friendly Name:       %s\n", (RemoteDeviceProperties->ApplicationData.FriendlyNameLength)?RemoteDeviceProperties->ApplicationData.FriendlyName:"");

            printf("App. Info:   :       %08lX\n", RemoteDeviceProperties->ApplicationData.ApplicationInfo);
         }

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_PAIRING_STATE))
            printf("Paired State :       %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_PAIRED)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CONNECTION_STATE))
            printf("Connect State:       %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED)?"TRUE":"FALSE");

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_ENCRYPTION_STATE))
            printf("Encrypt State:       %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_ENCRYPTED)?"TRUE":"FALSE");

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SNIFF_STATE)))
         {
            if(RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_LINK_CURRENTLY_SNIFF_MODE)
               printf("Sniff State  :       TRUE (%u ms)\n", RemoteDeviceProperties->SniffInterval);
            else
               printf("Sniff State  :       FALSE\n");
         }

         if(((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE)))
            printf("COD:                 0x%02X%02X%02X\n", RemoteDeviceProperties->ClassOfDevice.Class_of_Device0, RemoteDeviceProperties->ClassOfDevice.Class_of_Device1, RemoteDeviceProperties->ClassOfDevice.Class_of_Device2);

         if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_SERVICES_STATE))
            printf("SDP Serv. Known :    %s\n", (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_SERVICES_KNOWN)?"TRUE":"FALSE");
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
         printf("%*s %s: %02X%02X\n", (Level*INDENT_LENGTH), "", Prefix, UUID->UUID.UUID_16.UUID_Byte1, UUID->UUID.UUID_16.UUID_Byte0);
      else
      {
         if(UUID->UUID_Type == guUUID_128)
         {
            printf("%*s %s: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "", Prefix, UUID->UUID.UUID_128.UUID_Byte15, UUID->UUID.UUID_128.UUID_Byte14, UUID->UUID.UUID_128.UUID_Byte13,
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
      printf("Number of Services: %u\n", ParsedGATTData->NumberServices);

      for(Index=0;Index<ParsedGATTData->NumberServices;Index++)
      {
         DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.UUID), "Service UUID", 0);

         printf("%*s Start Handle: 0x%04X (%d)\n", (1*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.Service_Handle);
         printf("%*s End Handle:   0x%04X (%d)\n", (1*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].ServiceInformation.End_Group_Handle);

         /* Check to see if there are included services.                */
         if(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfIncludedService)
         {
            for(Index1=0;Index1<ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfIncludedService;Index1++)
            {
               DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].UUID), "Included Service UUID", 2);

               printf("%*s Start Handle: 0x%04X (%d)\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].Service_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].Service_Handle);
               printf("%*s End Handle:   0x%04X (%d)\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].End_Group_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].IncludedServiceList[Index1].End_Group_Handle);
               printf("\n");
            }
         }

         /* Check to see if there are characteristics.                  */
         if(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics)
         {
            for(Index1=0;Index1<ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].NumberOfCharacteristics;Index1++)
            {
               DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_UUID), "Characteristic UUID", 2);

               printf("%*s Handle:     0x%04X (%d)\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Handle);
               printf("%*s Properties: 0x%02X\n", (2*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].Characteristic_Properties);

               /* Loop through the descriptors for this characteristic. */
               for(Index2=0;Index2<ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].NumberOfDescriptors;Index2++)
               {
                  if(Index2==0)
                     printf("\n");

                  DisplayGATTUUID(&(ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_UUID), "Descriptor UUID", 3);
                  printf("%*s Handle:     0x%04X (%d)\n", (3*INDENT_LENGTH), "", ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_Handle, ParsedGATTData->GATTServiceDiscoveryIndicationData[Index].CharacteristicInformationList[Index1].DescriptorList[Index2].Characteristic_Descriptor_Handle);
               }

               if(Index2>0)
                  printf("\n");
            }
         }
      }
   }
   else
      printf("No GATT Service Records Found.\n");
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
      for(Index=0; Index<ParsedSDPData->NumberServiceRecords; Index++)
      {
         /* First display the number of SDP Service Records we are      */
         /* currently processing.                                       */
         printf("Service Record: %u:\n", (Index + 1));

         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         DisplaySDPAttributeResponse(&(ParsedSDPData->SDPServiceAttributeResponseData[Index]), 1);
      }
   }
   else
      printf("No SDP Service Records Found.\n");
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
         printf("%*s Attribute ID 0x%04X\n", (InitLevel*INDENT_LENGTH), "", SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].Attribute_ID);

         /* Now Print out all of the SDP Data Elements that were        */
         /* returned that are associated with the SDP Attribute.        */
         DisplayDataElement(SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].SDP_Data_Element, (InitLevel + 1));
      }
   }
   else
      printf("No SDP Attributes Found.\n");
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
         printf("%*s Type: NIL\n", (Level*INDENT_LENGTH), "");
         break;
      case deNULL:
         /* Display the NULL Type.                                      */
         printf("%*s Type: NULL\n", (Level*INDENT_LENGTH), "");
         break;
      case deUnsignedInteger1Byte:
         /* Display the Unsigned Integer (1 Byte) Type.                 */
         printf("%*s Type: Unsigned Int = 0x%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger1Byte);
         break;
      case deUnsignedInteger2Bytes:
         /* Display the Unsigned Integer (2 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%04X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger2Bytes);
         break;
      case deUnsignedInteger4Bytes:
         /* Display the Unsigned Integer (4 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%08X\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.UnsignedInteger4Bytes);
         break;
      case deUnsignedInteger8Bytes:
         /* Display the Unsigned Integer (8 Bytes) Type.                */
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                                                 SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[7],
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
         printf("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                                                                                 SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[15],
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
         printf("%*s Type: Signed Int = 0x%02X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger1Byte);
         break;
      case deSignedInteger2Bytes:
         /* Display the Signed Integer (2 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%04X\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger2Bytes);
         break;
      case deSignedInteger4Bytes:
         /* Display the Signed Integer (4 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%08X\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.SignedInteger4Bytes);
         break;
      case deSignedInteger8Bytes:
         /* Display the Signed Integer (8 Bytes) Type.                  */
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                                               SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[7],
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
         printf("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                                                                               SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[15],
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

         printf("%*s Type: Text String = %s\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deBoolean:
         printf("%*s Type: Boolean = %s\n", (Level*INDENT_LENGTH), "", (SDPDataElement->SDP_Data_Element.Boolean)?"TRUE":"FALSE");
         break;
      case deURL:
         /* First retrieve the Length of the URL String so that we can  */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the URL String into the Buffer and then NULL terminate */
         /* it.                                                         */
         memcpy(Buffer, SDPDataElement->SDP_Data_Element.URL, Index);
         Buffer[Index] = '\0';

         printf("%*s Type: URL = %s\n", (Level*INDENT_LENGTH), "", Buffer);
         break;
      case deUUID_16:
         printf("%*s Type: UUID_16 = 0x%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                    SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte0,
                                                    SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte1);
         break;
      case deUUID_32:
         printf("%*s Type: UUID_32 = 0x%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte0,
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte1,
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte2,
                                                            SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte3);
         break;
      case deUUID_128:
         printf("%*s Type: UUID_128 = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", (Level*INDENT_LENGTH), "",
                                                                                                             SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte0,
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
         printf("%*s Type: Data Element Sequence\n", (Level*INDENT_LENGTH), "");

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
         printf("%*s Type: Data Element Alternative\n", (Level*INDENT_LENGTH), "");

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
         printf("%*s Unknown SDP Data Element Type\n", (Level*INDENT_LENGTH), "");
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

   /* If a module callback has been registered, then unregister it.     */
   if(EventCallbackID)
   {
      CPPM_Unregister_Collector_Event_Callback(EventCallbackID);

      /* Flag that there is no longer a Callback registered.            */
      EventCallbackID = 0;
   }

   printf("Server has been Unregistered.\n");

   printf("CPPM>");

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
      printf("\n");

      switch(EventData->EventType)
      {
         case detDevicePoweredOn:
            printf("Device Powered On.\n");
            break;
         case detDevicePoweringOff:
            printf("Device Powering Off Event, Timeout: 0x%08X.\n", EventData->EventData.DevicePoweringOffEventData.PoweringOffTimeout);
            break;
         case detDevicePoweredOff:
            printf("Device Powered Off.\n");
            break;
         case detLocalDevicePropertiesChanged:
            printf("Local Device Properties Changed.\n");

            DisplayLocalDeviceProperties(EventData->EventData.LocalDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.LocalDevicePropertiesChangedEventData.LocalDeviceProperties));
            break;
         case detDeviceScanStarted:
            printf("LE Device Discovery Started.\n");
            break;
         case detDeviceScanStopped:
            printf("LE Device Discovery Stopped.\n");
            break;
         case detDeviceObservationScanStarted:
            printf("Device Observation Scan Started: Flags 0x%08lX.\n", EventData->EventData.ObservationScanStartedEventData.ObservationScanFlags);
            break;
         case detDeviceObservationScanStopped:
            printf("Device Observation Scan Stopped: Flags 0x%08lX.\n", EventData->EventData.ObservationScanStoppedEventData.ObservationScanFlags);
            break;
         case detDeviceAdvertisingStarted:
            printf("LE Advertising Started.\n");
            break;
         case detDeviceAdvertisingStopped:
            printf("LE Advertising Stopped.\n");
            break;
         case detAdvertisingTimeout:
            printf("LE Advertising Timeout.\n");
            break;
         case detDeviceDiscoveryStarted:
            printf("Device Discovery Started.\n");
            break;
         case detDeviceDiscoveryStopped:
            printf("Device Discovery Stopped.\n");
            break;
         case detRemoteDeviceFound:
            printf("Remote Device Found.\n");

            DisplayRemoteDeviceProperties(0, &(EventData->EventData.RemoteDeviceFoundEventData.RemoteDeviceProperties));
            break;
         case detRemoteDeviceDeleted:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Deleted: %s.\n", Buffer);
            break;
         case detRemoteDevicePropertiesChanged:
            printf("Remote Device Properties Changed.\n");

            DisplayRemoteDeviceProperties(EventData->EventData.RemoteDevicePropertiesChangedEventData.ChangedMemberMask, &(EventData->EventData.RemoteDevicePropertiesChangedEventData.RemoteDeviceProperties));
            break;
         case detRemoteDevicePropertiesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties.BD_ADDR, Buffer);

            printf("Remote Device Properties Status: %s, %s.\n", Buffer, EventData->EventData.RemoteDevicePropertiesStatusEventData.Success?"SUCCESS":"FAILURE");

            DisplayRemoteDeviceProperties(0, &(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties));
            break;
         case detRemoteDeviceServicesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceServicesStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device %s Services Status: %s, %s.\n", Buffer, (EventData->EventData.RemoteDeviceServicesStatusEventData.StatusFlags & DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_LOW_ENERGY)?"LE":"BR/EDR", (EventData->EventData.RemoteDeviceServicesStatusEventData.StatusFlags & DEVM_REMOTE_DEVICE_SERVICES_STATUS_FLAGS_SUCCESS)?"SUCCESS":"FAILURE");
            break;
         case detRemoteDevicePairingStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePairingStatusEventData.RemoteDeviceAddress, Buffer);

            printf("%s Remote Device Pairing Status: %s, %s (0x%02X)\n", ((EventData->EventData.RemoteDevicePairingStatusEventData.AuthenticationStatus & DEVM_REMOTE_DEVICE_PAIRING_STATUS_FLAGS_LOW_ENERGY)?"LE":"BR/EDR"), Buffer, (EventData->EventData.RemoteDevicePairingStatusEventData.Success)?"SUCCESS":"FAILURE", EventData->EventData.RemoteDevicePairingStatusEventData.AuthenticationStatus);
            break;
         case detRemoteDeviceAuthenticationStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Authentication Status: %s, %d (%s)\n", Buffer, EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status, (EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceAuthenticationStatusEventData.Status):"SUCCESS");
            break;
         case detRemoteDeviceEncryptionStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceEncryptionStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Encryption Status: %s, %d (%s)\n", Buffer, EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status, (EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceEncryptionStatusEventData.Status):"SUCCESS");
            break;
         case detRemoteDeviceConnectionStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceConnectionStatusEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Connection Status: %s, %d (%s)\n", Buffer, EventData->EventData.RemoteDeviceConnectionStatusEventData.Status, (EventData->EventData.RemoteDeviceConnectionStatusEventData.Status)?ERR_ConvertErrorCodeToString(EventData->EventData.RemoteDeviceConnectionStatusEventData.Status):"SUCCESS");
            break;
         default:
            printf("Unknown Device Manager Event Received: 0x%08X, Length: 0x%08X.\n", (unsigned int)EventData->EventType, EventData->EventLength);
            break;
      }
   }
   else
      printf("\nDEVM Event Data is NULL.\n");

   printf("CPPM>");

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
      printf("\n");

      BD_ADDRToStr(AuthenticationRequestInformation->BD_ADDR, Buffer);

      printf("Authentication Request received for %s.\n", Buffer);

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
            printf("PIN Code Request.\n.");

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* PIN Code.                                                */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;

            /* Inform the user that they will need to respond with a PIN*/
            /* Code Response.                                           */
            printf("\nRespond with the command: PINCodeResponse\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_REQUEST:
            printf("User Confirmation Request %s.\n", (LowEnergy?"LE":"BR/EDR"));

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* User Confirmation.                                       */
            CurrentRemoteBD_ADDR = AuthenticationRequestInformation->BD_ADDR;
            CurrentLowEnergy     = FALSE;

            if(!LowEnergy)
            {
               if(IOCapability != icDisplayYesNo)
               {
                  /* Invoke Just works.                                 */

                  printf("\nAuto Accepting: %lu\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

                  BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

                  AuthenticationResponseInformation.BD_ADDR                         = AuthenticationRequestInformation->BD_ADDR;
                  AuthenticationResponseInformation.AuthenticationAction            = DEVM_AUTHENTICATION_ACTION_USER_CONFIRMATION_RESPONSE;
                  AuthenticationResponseInformation.AuthenticationDataLength        = sizeof(AuthenticationResponseInformation.AuthenticationData.Confirmation);

                  AuthenticationResponseInformation.AuthenticationData.Confirmation = (Boolean_t)TRUE;

                  if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
                     printf("DEVM_AuthenticationResponse() Success.\n");
                  else
                     printf("DEVM_AuthenticationResponse() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
               }
               else
               {
                  printf("User Confirmation: %lu\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);

                  /* Inform the user that they will need to respond with*/
                  /* a PIN Code Response.                               */
                  printf("\nRespond with the command: UserConfirmationResponse\n");
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
            printf("\nRespond with the command: PassKeyResponse\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_PASSKEY_INDICATION:
            printf("PassKey Indication %s.\r\n", (LowEnergy?"LE":"BR/EDR"));
            printf("\tSecure Connections: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
            printf("\tJust Works Pairing: %s.\r\n", (AuthenticationRequestInformation->Flags & DEVM_AUTHENTICATION_INFORMATION_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");

            printf("PassKey: %lu\n", (unsigned long)AuthenticationRequestInformation->AuthenticationData.Passkey);
            break;
         case DEVM_AUTHENTICATION_ACTION_KEYPRESS_INDICATION:
            printf("Keypress Indication.\n");

            printf("Keypress: %d\n", (int)AuthenticationRequestInformation->AuthenticationData.Keypress);
            break;
         case DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_REQUEST:
            printf("Out of Band Data Request: %s.\n", (LowEnergy?"LE":"BR/EDR"));

            /* This application does not support OOB data so respond    */
            /* with a data length of Zero to force a negative reply.    */
            BTPS_MemInitialize(&AuthenticationResponseInformation, 0, sizeof(AuthenticationResponseInformation));

            AuthenticationResponseInformation.BD_ADDR                  = AuthenticationRequestInformation->BD_ADDR;
            AuthenticationResponseInformation.AuthenticationAction     = DEVM_AUTHENTICATION_ACTION_OUT_OF_BAND_DATA_RESPONSE;
            AuthenticationResponseInformation.AuthenticationDataLength = 0;

            if(LowEnergy)
               AuthenticationResponseInformation.AuthenticationAction |= DEVM_AUTHENTICATION_ACTION_LOW_ENERGY_OPERATION_MASK;

            if((Result = DEVM_AuthenticationResponse(AuthenticationCallbackID, &AuthenticationResponseInformation)) >= 0)
               printf("DEVM_AuthenticationResponse() Success.\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_REQUEST:
            printf("I/O Capability Request: %s.\n", (LowEnergy?"LE":"BR/EDR"));

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
               printf("DEVM_AuthenticationResponse() Success.\n");
            else
               printf("DEVM_AuthenticationResponse() Failure: %d, %s.\n", Result, ERR_ConvertErrorCodeToString(Result));
            break;
         case DEVM_AUTHENTICATION_ACTION_IO_CAPABILITIES_RESPONSE:
            printf("I/O Capability Response.\n");

            /* Inform the user of the Remote I/O Capablities.           */
            printf("Remote I/O Capabilities: %s, MITM Protection: %s.\n", IOCapabilitiesStrings[AuthenticationRequestInformation->AuthenticationData.IOCapabilities.IO_Capability], AuthenticationRequestInformation->AuthenticationData.IOCapabilities.MITM_Protection_Required?"TRUE":"FALSE");
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_STATUS_RESULT:
            printf("Authentication Status: .\n");

            printf("Status: %d\n", AuthenticationRequestInformation->AuthenticationData.AuthenticationStatus);
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_START:
            printf("Authentication Start.\n");
            break;
         case DEVM_AUTHENTICATION_ACTION_AUTHENTICATION_END:
            printf("Authentication End.\n");
            break;
         default:
            printf("Unknown Device Manager Authentication Event Received: 0x%08X, Length: 0x%08X.\n", (unsigned int)AuthenticationRequestInformation->AuthenticationAction, AuthenticationRequestInformation->AuthenticationDataLength);
            break;
      }
   }
   else
      printf("\nDEVM Authentication Request Data is NULL.\n");

   printf("CPPM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the Heart Rate Manager Event Callback   */
   /* function that is Registered with the Heart Rate Manager.  This    */
   /* callback is responsible for processing all Heart Rate Manager     */
   /* Events.                                                           */
static void BTPSAPI CPPM_Event_Callback(CPPM_Event_Data_t *EventData, void *CallbackParameter)
{
   char         BoardStr[16];
   unsigned int Index;

   if(EventData)
   {
      printf("\n");

      switch(EventData->EventType)
      {
         case cetConnectedCPP:
            printf("\n cetConnectedCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_CONNECTED_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.ConnectedEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID:   %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address:   %s\n", BoardStr);
               printf("  Connected Flags:     0x%08X\n", (unsigned int)EventData->EventData.ConnectedEventData.ConnectedFlags);
               printf("  Connection Type:     %s\n", (EventData->EventData.ConnectedEventData.ConnectionType == cctSensor) ? "Sensor" : "Collector");
               printf("  Number of Instances: %u\n", EventData->EventData.ConnectedEventData.NumberOfInstances);
            }
            break;
         case cetDisconnectedCPP:
            printf("\n cetDisconnectedCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_DISCONNECTED_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.DisconnectedEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID:  %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address:  %s\n", BoardStr);
               printf("  Connection Type:    %s\n", (EventData->EventData.DisconnectedEventData.ConnectionType == cctSensor) ? "Sensor" : "Collector");
            }
            break;
         case cetMeasurementsSetCPP:
            printf("\n cetMeasurementsSetCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.WriteResponseEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID: %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address: %s\n", BoardStr);
               printf("  InstanceID:        %u\n", EventData->EventData.WriteResponseEventData.InstanceID);
               printf("  TransactionID:     %u\n", EventData->EventData.WriteResponseEventData.TransactionID);
               printf("  Status:            %d\n", EventData->EventData.WriteResponseEventData.Status);
            }
            else
            {
               printf("   Error:            %s\n", ErrorToString(EventData->EventData.WriteResponseEventData.Status));
            }
            break;
         case cetVectorsSetCPP:
            printf("\n cetVectorsSetCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.WriteResponseEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID: %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address: %s\n", BoardStr);
               printf("  InstanceID:        %u\n", EventData->EventData.WriteResponseEventData.InstanceID);
               printf("  TransactionID:     %u\n", EventData->EventData.WriteResponseEventData.TransactionID);
               printf("  Status:            %d\n", EventData->EventData.WriteResponseEventData.Status);
            }
            else
            {
               printf("   Error:            %s\n", ErrorToString(EventData->EventData.WriteResponseEventData.Status));
            }
            break;
         case cetProceduresSetCPP:
            printf("\n cetProceduresSetCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.WriteResponseEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID: %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address: %s\n", BoardStr);
               printf("  InstanceID:        %u\n", EventData->EventData.WriteResponseEventData.InstanceID);
               printf("  TransactionID:     %u\n", EventData->EventData.WriteResponseEventData.TransactionID);
               printf("  Status:            %d\n", EventData->EventData.WriteResponseEventData.Status);
            }
            else
            {
               printf("   Error:            %s\n", ErrorToString(EventData->EventData.WriteResponseEventData.Status));
            }
            break;
         case cetBroadcastsSetCPP:
            printf("\n cetBroadcastsSetCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.WriteResponseEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID: %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address: %s\n", BoardStr);
               printf("  InstanceID:        %u\n", EventData->EventData.WriteResponseEventData.InstanceID);
               printf("  TransactionID:     %u\n", EventData->EventData.WriteResponseEventData.TransactionID);
               printf("  Status:            %d\n", EventData->EventData.WriteResponseEventData.Status);
            }
            else
            {
               printf("   Error:            %s\n", ErrorToString(EventData->EventData.WriteResponseEventData.Status));
            }
            break;
         case cetProcedureBegunCPP:
            printf("\n cetProcedureBegunCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.WriteResponseEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID: %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address: %s\n", BoardStr);
               printf("  InstanceID:        %u\n", EventData->EventData.WriteResponseEventData.InstanceID);
               printf("  TransactionID:     %u\n", EventData->EventData.WriteResponseEventData.TransactionID);
               printf("  Status:            %d\n", EventData->EventData.WriteResponseEventData.Status);
            }
            else
            {
               printf("   Error:            %s\n", ErrorToString(EventData->EventData.WriteResponseEventData.Status));
            }
            break;
         case cetMeasurementCPP:
            printf("\n cetMeasurementCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_MEASUREMENT_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.MeasurementEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID:            %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address:            %s\n", BoardStr);
               printf("  InstanceID:                   %u\n", EventData->EventData.MeasurementEventData.InstanceID);

               printf("  Measurement Flags:            %u\n", EventData->EventData.MeasurementEventData.Measurement.Flags);

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_PRESENT)
               {
                  printf("    pedal power balance present\n");

                  if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_REFERENCE_LEFT)
                     printf("      left\n");
                  else
                     printf("      right\n");
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_PRESENT)
               {
                  printf("    accumulated torque present\n");

                  if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_SOURCE_CRANK_BASED)
                     printf("      crank based\n");
                  else
                     printf("      wheel based\n");
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT)
                  printf("    wheel revolution data present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT)
                  printf("    crank revolution data present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_EXTREME_FORCE_MAGNITUDES_PRESENT)
                  printf("    extreme force magnitudes present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_EXTREME_TORQUE_MAGNITUDES_PRESENT)
                  printf("    extreme torque magnitudes present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_EXTREME_ANGLES_PRESENT)
                  printf("    extreme angles present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT)
                  printf("    top dead spot angle present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT)
                  printf("    bottom dead spot angle present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_ACCUMULATED_ENERGY_PRESENT)
                  printf("    accumulated energy present\n");

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_OFFSET_COMPENSATION_INDICATOR)
                  printf("    offset compensation indicator\n");

               printf("  Instantaneous Power:          %u\n", EventData->EventData.MeasurementEventData.Measurement.InstantaneousPower);

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_PRESENT)
                  printf("  Pedal Power Balance:          %u\n", EventData->EventData.MeasurementEventData.Measurement.PedalPowerBalance);

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_PRESENT)
                  printf("  Pedal Power Balance:          %u\n", EventData->EventData.MeasurementEventData.Measurement.AccumulatedTorque);

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT)
               {
                  printf("  Cumulative Wheel Revolutions: %u\n", EventData->EventData.MeasurementEventData.Measurement.WheelRevolutionData.CumulativeWheelRevolutions);
                  printf("  Last Wheel Event Time:        %u\n", EventData->EventData.MeasurementEventData.Measurement.WheelRevolutionData.LastWheelEventTime);
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT)
               {
                  printf("  Cumulative Crank Revolutions: %u\n", EventData->EventData.MeasurementEventData.Measurement.CrankRevolutionData.CumulativeCrankRevolutions);
                  printf("  Last Crank Event Time:        %u\n", EventData->EventData.MeasurementEventData.Measurement.CrankRevolutionData.LastCrankEventTime);
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_EXTREME_FORCE_MAGNITUDES_PRESENT)
               {
                  printf("  Maximum Force Magnitude:      %u\n", EventData->EventData.MeasurementEventData.Measurement.ExtremeForceMagnitudes.MaximumForceMagnitude);
                  printf("  Minimum Force Magnitude:      %u\n", EventData->EventData.MeasurementEventData.Measurement.ExtremeForceMagnitudes.MinimumForceMagnitude);
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_EXTREME_TORQUE_MAGNITUDES_PRESENT)
               {
                  printf("  Maximum Torque Magnitude:     %u\n", EventData->EventData.MeasurementEventData.Measurement.ExtremeTorqueMagnitudes.MaximumTorqueMagnitude);
                  printf("  Minimum Torque Magnitude:     %u\n", EventData->EventData.MeasurementEventData.Measurement.ExtremeTorqueMagnitudes.MinimumTorqueMagnitude);
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_EXTREME_ANGLES_PRESENT)
               {
                  printf("  Maximum Angle:                %u\n", EventData->EventData.MeasurementEventData.Measurement.ExtremeAngles.MaximumAngle);
                  printf("  Minimum Angle:                %u\n", EventData->EventData.MeasurementEventData.Measurement.ExtremeAngles.MinimumAngle);
               }

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT)
                  printf("  Top Dead Spot Angle:          %u\n", EventData->EventData.MeasurementEventData.Measurement.TopDeadSpotAngle);

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT)
                  printf("  Bottom Dead Spot Angle:       %u\n", EventData->EventData.MeasurementEventData.Measurement.BottomDeadSpotAngle);

               if(EventData->EventData.MeasurementEventData.Measurement.Flags & CPPM_MEASUREMENT_FLAGS_ACCUMULATED_ENERGY_PRESENT)
                  printf("  Accumulated Energy:           %u\n", EventData->EventData.MeasurementEventData.Measurement.AccumulatedEnergy);
            }
            break;
         case cetVectorCPP:
            printf("\n cetVectorCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_VECTOR_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.VectorEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID:            %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address:            %s\n", BoardStr);
               printf("  InstanceID:                   %u\n", EventData->EventData.VectorEventData.InstanceID);

               printf("  Vector Flags:                 %u\n", EventData->EventData.VectorEventData.Vector.Flags);

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_CRANK_REVOLUTION_DATA_PRESENT)
                  printf("    crank revolution data present\n");

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT)
                  printf("    first crank measurement angle present\n");

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_INSTANTANEOUS_FORCE_MAGNITUDE_ARRAY_PRESENT)
                  printf("    instantaneous force magnitude array present\n");

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_INSTANTANEOUS_TORQUE_MAGNITUDE_ARRAY_PRESENT)
                  printf("    instantaneous torque magnitude array present\n");

               switch((EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_BITS) >> 4)
               {
                  case(CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_TANGENTIAL_COMPONENT):
                     printf("    instantaneous measurement direction: tangential component\n");
                     break;
                  case(CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_RADIAL_COMPONENT):
                     printf("    instantaneous measurement direction: radial component\n");
                     break;
                  case(CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_LATERAL_COMPONENT):
                     printf("    instantaneous measurement direction: lateral component\n");
                     break;
                  default:
                     break;
               }

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_CRANK_REVOLUTION_DATA_PRESENT)
               {
                  printf("  Cumulative Crank Revolutions: %u\n", EventData->EventData.VectorEventData.Vector.CrankRevolutionData.CumulativeCrankRevolutions);
                  printf("  Last Crank Event Time:        %u\n", EventData->EventData.VectorEventData.Vector.CrankRevolutionData.LastCrankEventTime);
               }

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT)
               {
                  printf("  First Crank Measurement Angle: %u\n", EventData->EventData.VectorEventData.Vector.FirstCrankMeasurementAngle);
               }

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT)
               {
                  printf("  Magnitude Data Length:         %u\n", EventData->EventData.VectorEventData.Vector.MagnitudeDataLength);
               }

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_INSTANTANEOUS_FORCE_MAGNITUDE_ARRAY_PRESENT)
               {
                  printf("  Force Magnitude Data:\n");

                  for(Index = 0; Index < EventData->EventData.VectorEventData.Vector.MagnitudeDataLength; Index++)
                     printf("                        %u\n", EventData->EventData.VectorEventData.Vector.InstantaneousMagnitude[Index]);
               }

               if(EventData->EventData.VectorEventData.Vector.Flags & CPPM_VECTOR_FLAGS_INSTANTANEOUS_TORQUE_MAGNITUDE_ARRAY_PRESENT)
               {
                  printf("  Torque Magnitude Data:\n");

                  for(Index = 0; Index < EventData->EventData.VectorEventData.Vector.MagnitudeDataLength; Index++)
                     printf("                        %u\n", EventData->EventData.VectorEventData.Vector.InstantaneousMagnitude[Index]);
               }
            }
            break;
         case cetControlPointCPP:
            printf("\n cetControlPointCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_CONTROL_POINT_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.ControlPointEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID:            %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address:            %s\n", BoardStr);
               printf("  InstanceID:                   %u\n", EventData->EventData.ControlPointEventData.InstanceID);

               printf("  Procedure Timeout:            %s\n", EventData->EventData.ControlPointEventData.Timeout ? "True" : "False");

               if(!(EventData->EventData.ControlPointEventData.Timeout))
               {
                  printf("  Response Code:                %d\n", EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode);

                  switch(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode)
                  {
                     case prcSuccessCPP:
                        printf("                                Success\n");
                        break;
                     case prcOpcodeNotSupported:
                        printf("                                Opcode Not Supported\n");
                        break;
                     case prcInvalidParameter:
                        printf("                                Invalid Parameter\n");
                        break;
                     case prcOperationFailed:
                        printf("                                Operation Failed\n");
                        break;
                     default:
                        break;
                  }

                  printf("  Operation Code:               %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Opcode);

                  switch(EventData->EventData.ControlPointEventData.ControlPoint.Opcode)
                  {
                     case pocSetCumulativeValue:
                        printf("                                Set Cumulative Value\n");
                        break;
                     case pocUdateSensorLocation:
                        printf("                                Update Sensor Location\n");
                        break;
                     case pocRequestSupportedSensorLocations:
                        printf("                                Request Supported Sensor Locations\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Supported Locs. Count:      %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.NumberOfSensorLocations);
                           printf("    Supported Locs.:\n");
                           for(Index = 0; Index < EventData->EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.NumberOfSensorLocations; Index++)
                           {
                              printf("                              0x%02X %s\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.SensorLocations[Index], LocationToString(EventData->EventData.ControlPointEventData.ControlPoint.Parameter.SupportedSensorLocations.SensorLocations[Index]));
                           }
                        }
                        break;
                     case pocSetCrankLength:
                        printf("                                Set Crank Length\n");
                        break;
                     case pocRequestCrankLength:
                        printf("                                Request Crank Length\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Crank Length:               %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.CrankLength);
                        }
                        break;
                     case pocSetChainLength:
                        printf("                                Set Chain Length\n");
                        break;
                     case pocRequestChainLength:
                        printf("                                Request Chain Length\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Chain Length:               %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.ChainLength);
                        }
                        break;
                     case pocSetChainWeight:
                        printf("                                Set Chain Weight\n");
                        break;
                     case pocRequestChainWeight:
                        printf("                                Request Chain Weight\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Chain Weight:               %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.ChainWeight);
                        }
                        break;
                     case pocSetSpanLength:
                        printf("                                Set Span Length\n");
                        break;
                     case pocRequestSpanLength:
                        printf("                                Request Span Length\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Span Length:                %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.SpanLength);
                        }
                        break;
                     case pocStartOffsetCompensation:
                        printf("                                Start Offset Compensation\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Offset Comp.:               %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.OffsetCompensation);
                        }
                        break;
                     case pocMaskMeasurementCharacteristicContent:
                        printf("                                Mask Measurement Characteristic Content\n");
                        break;
                     case pocRequestSamplingRate:
                        printf("                                Request Sampling Rate\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Sampling Rate:              %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.SamplingRate);
                        }
                        break;
                     case pocRequestFactoryCalibrationDate:
                        printf("                                Request Factory Calibration Date\n");

                        if(EventData->EventData.ControlPointEventData.ControlPoint.ResponseCode == prcSuccessCPP)
                        {
                           printf("    Fact. Calib. Date:\n");
                           printf("      Year:                     %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.FactoryCalibrationDate.Year);
                           printf("      Month:                    %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.FactoryCalibrationDate.Month);
                           printf("      Day:                      %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.FactoryCalibrationDate.Day);
                           printf("      Hours:                    %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.FactoryCalibrationDate.Hours);
                           printf("      Minutes:                  %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.FactoryCalibrationDate.Minutes);
                           printf("      Seconds:                  %u\n", EventData->EventData.ControlPointEventData.ControlPoint.Parameter.FactoryCalibrationDate.Seconds);
                        }
                        break;
                     default:
                        break;
                  }
               }
            }
            break;
         case cetSensorFeaturesCPP:
            printf("\n cetSensorFeaturesCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_SENSOR_FEATURES_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.SensorFeaturesEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID:     %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address:     %s\n", BoardStr);
               printf("  InstanceID:            %u\n", EventData->EventData.SensorFeaturesEventData.InstanceID);
               printf("  TransactionID:         %u\n", EventData->EventData.SensorFeaturesEventData.TransactionID);
               printf("  Status:                %d\n", EventData->EventData.SensorFeaturesEventData.Status);

               /* Check for a successful status.                        */
               if(!EventData->EventData.SensorFeaturesEventData.Status)
               {
                  printf("  Sensor Features:       %lu\n\n", EventData->EventData.SensorFeaturesEventData.Features);

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_PEDAL_POWER_BALANCE_SUPPORTED)
                     printf("    power balance supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_ACCUMULATED_TORQUE_SUPPORTED)
                     printf("    accumulated torque supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_WHEEL_REVOLUTION_DATA_SUPPORTED)
                     printf("    wheel revolution data supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_CRANK_REVOLUTION_DATA_SUPPORTED)
                     printf("    crank revolution data supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_EXTREME_MAGNITUDES_SUPPORTED)
                     printf("    extreme magnitudes supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_EXTREME_ANGLES_SUPPORTED)
                     printf("    extreme angles supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_TOP_AND_BOTTOM_DEAD_SPOT_ANGLES_SUPPORTED)
                     printf("    top and bottom dead spot angles supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_ACCUMULATED_ENERGY_SUPPORTED)
                     printf("    accumulated energy supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_OFFSET_COMPENSATION_INDICATOR_SUPPORTED)
                     printf("    offset compensation indicator supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_OFFSET_COMPENSATION_SUPPORTED)
                     printf("    offset compensation supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_MEASUREMENT_CHARACTERISTIC_CONTENT_MASKING_SUPPORTED)
                     printf("    measurement characteristic content masking supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED)
                     printf("    multiple sensor locations supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_CRANK_LENGTH_ADJUSTMENT_SUPPORTED)
                     printf("    crank length adjustment supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_CHAIN_LENGTH_ADJUSTMENT_SUPPORTED)
                     printf("    chain length adjustment supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_CHAIN_WEIGHT_ADJUSTMENT_SUPPORTED)
                     printf("    chain weight adjustment supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_SPAN_LENGTH_ADJUSTMENT_SUPPORTED)
                     printf("    span length adjustment supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_SENSOR_MEASUREMENT_CONTEXT_TORQUE)
                     printf("    sensor measurement context: torque\n");
                  else
                     printf("    sensor measurement context: force\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_INSTANTANEOUS_MEASUREMENT_DIRECTION_SUPPORTED)
                     printf("    instantaneous measurement direction supported\n");

                  if(EventData->EventData.SensorFeaturesEventData.Features & CPPM_FEATURE_FACTORY_CALIBRATION_DATE_SUPPORTED)
                     printf("    factory calibration date supported\n");

               }
               else
               {
                  printf("   Error:            %s\n", ErrorToString(EventData->EventData.SensorFeaturesEventData.Status));
               }
            }
            break;
         case cetSensorLocationCPP:
            printf("\n cetSensorLocationCPP with size %u:\n", EventData->EventLength);
            if(EventData->EventLength >= CPPM_SENSOR_LOCATION_EVENT_DATA_SIZE)
            {
               BD_ADDRToStr(EventData->EventData.SensorLocationEventData.RemoteDeviceAddress, BoardStr);

               printf("  Event Callback ID: %u\n", EventData->EventCallbackID);
               printf("  Bluetooth Address: %s\n", BoardStr);
               printf("  InstanceID:        %u\n", EventData->EventData.SensorLocationEventData.InstanceID);
               printf("  TransactionID:     %u\n", EventData->EventData.SensorLocationEventData.TransactionID);
               printf("  Status:            %d\n", EventData->EventData.SensorLocationEventData.Status);

               /* Check for a successful status.                        */
               if(!EventData->EventData.SensorLocationEventData.Status)
               {
                  printf("  Sensor Location:   %u\n\n", EventData->EventData.SensorLocationEventData.Location);
                  printf("                     %s\n", LocationToString(EventData->EventData.SensorLocationEventData.Location));
               }
               else
               {
                  printf("   Error:            %s\n", ErrorToString(EventData->EventData.SensorLocationEventData.Status));
               }
            }
            break;
         default:
            printf("Unknown Cycling Power Profile Manager Event Received: 0x%08X, Length: 0x%08X\n", (unsigned int)EventData->EventType, EventData->EventLength);
            break;
      }
   }
   else
      printf("\nCPPM Event Data is NULL.\n");

   printf("\nCPPM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

static char *LocationToString(CPPM_Sensor_Location_t Location)
{
   switch(Location)
   {
      case slOther:
         return("other");
         break;

      case slTopOfShoe:
         return("top of shoe");
         break;

      case slInShoe:
         return("in shoe");
         break;

      case slHip:
         return("hip");
         break;

      case slFrontWheel:
         return("front wheel");
         break;

      case slLeftCrank:
         return("left crank");
         break;

      case slRightCrank:
         return("right crank");
         break;

      case slLeftPedal:
         return("left pedal");
         break;

      case slRightPedal:
         return("right pedal");
         break;

      case slFrontHub:
         return("front hub");
         break;

      case slRearDropout:
         return("rear dropout");
         break;

      case slChainstay:
         return("chainstay");
         break;

      case slRearWheel:
         return("rear wheel hub");
         break;

      case slRearHub:
         return("rear hub");
         break;

      case slChest:
         return("chest");
         break;

      default:
         return("unknown");
         break;
   }
}

static char * ErrorToString(unsigned int Status)
{
   char *ErrorString;

   switch(Status)
   {
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE:
         ErrorString = "invalid handle";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED:
         ErrorString = "read not permitted";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED:
         ErrorString = "write not permitted";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_PDU:
         ErrorString = "invalid PDU";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION:
         ErrorString = "insufficient authentication";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED:
         ErrorString = "request not suppported";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET:
         ErrorString = "invalid offset";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION:
         ErrorString = "insufficient authorization";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL:
         ErrorString = "prepare queue full";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND:
         ErrorString = "attribute not found";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG:
         ErrorString = "attribute not long";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
         ErrorString = "insufficient encryption key size";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH:
         ErrorString = "invalid attribute value length";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR:
         ErrorString = "unlikely error";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION:
         ErrorString = "insufficient encryption";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE:
         ErrorString = "unsupported group type";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES:
         ErrorString = "insufficient resources";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED:
         ErrorString = "CCCD improperly configured";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS:
         ErrorString = "procedure already in progress";
         break;
      case BTPM_ATT_PROTOCOL_ERROR_CODE_OUT_OF_RANGE:
         ErrorString = "code out of range";
         break;
      default:
         ErrorString = "unknown error";
   }

   return (ErrorString);
}
   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   /* Initialize the default Secure Simple Pairing parameters.          */
   IOCapability   = DEFAULT_IO_CAPABILITY;
   OOBSupport     = FALSE;
   MITMProtection = DEFAULT_MITM_PROTECTION;

   /* Nothing really to do here aside from running the main application */
   /* code.                                                             */
   UserInterface();

   /* Return success.                                                   */
   return(0);
}

/*****< linuxsppm.c >**********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXSPPM - Simple Linux application using Bluetopia Platform Manager     */
/*              Device Manager Application Programming (API) Interface.       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/01/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "LinuxSPPM.h"     /* Main Application Prototypes and Constants.      */
#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#define MAX_SUPPORTED_COMMANDS                     (48)  /* Denotes the       */
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

#define INDENT_LENGTH                               (3)  /* Denotes the number*/
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

#define DEFAULT_TRANSMIT_BUFFER_SIZE  (16 * 1024 * 1024) /* Specifies the     */
                                                         /* default size of   */
                                                         /* the transmit      */
                                                         /* buffer used for   */
                                                         /* loopback mode.    */

#define TRANSMIT_THREAD_STACK_SIZE               (16384) /* Specifies the size*/
                                                         /* of the send thread*/
                                                         /* that is used for  */
                                                         /* bulk sending and  */
                                                         /* with loop back    */
                                                         /* mode.             */

#define MAX_TRANSMIT_THREAD_TRANSMIT_SIZE         (2048) /* Specifies the     */
                                                         /* maximum number of */
                                                         /* bytes to send when*/
                                                         /* writing SPP data. */

#define MAX_RECEIVE_THREAD_RECEIVE_SIZE           (2048) /* Specifies the     */
                                                         /* maximum number of */
                                                         /* bytes to receive  */
                                                         /* when receiving SPP*/
                                                         /* data.             */

#define MIN(_x, _y)          ((_x) < (_y) ? (_x) : (_y)) /* Helper macro that */
                                                         /* returns the       */
                                                         /* minimum of 2      */
                                                         /* values.           */

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

   /* The following type definition specifies information about a ring  */
   /* buffer.                                                           */
typedef struct _tagRingBuffer_t
{
   unsigned long  BufferSize;
   unsigned char *Buffer;
   unsigned long  NumInUse;
   unsigned long  NumFree;
   unsigned long  InIndex;
   unsigned long  OutIndex;
} RingBuffer_t;

   /* The following type definition stores the information used by the  */
   /* Receive Thread.                                                   */
typedef struct _tagReceiveThreadInfo_t
{
   Boolean_t      Initialized;
   DWord_t        AvailableDataSize;
   Boolean_t      WaitingForData;
   unsigned int   PortHandle;
   Event_t        DataAvailableEvent;
   Event_t        ThreadShutDownEvent;
   Byte_t         DisplayRawDataPosition;
   DWord_t        ReceiveCountStatus;
   DWord_t        ReceiveCount;
   unsigned int   AutomaticReadBufferSize;
   unsigned char *AutomaticReadBuffer;
} ReceiveThreadInfo_t;

   /* The following type definition stores the information used by the  */
   /* Transmit Thread.                                                  */
typedef struct _tagTransmitThreadInfo_t
{
   Boolean_t    Initialized;
   RingBuffer_t TransmitBuffer;
   Boolean_t    WaitingForData;
   unsigned int PortHandle;
   Mutex_t      TransmitBufferMutex;
   Mutex_t      TransmitBufferReallocMutex;
   Event_t      DataAvailableEvent;
   Event_t      ThreadShutDownEvent;
   DWord_t      WriteDataCommandSize;
   DWord_t      WriteDataCommandNumSent;
   DWord_t      WriteDataCommandStatus;
   DWord_t      Flags;
} TransmitThreadInfo_t;

   /* The following constants are used with the flags member in the     */
   /* transmit thread info structure.                                   */
#define TRANSMIT_THREAD_INFO_FLAGS_WRITE_DATA_COMMAND  0x00000001
#define TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK            0x00000002

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
static Boolean_t            Initialized;             /* Variable which is used to hold  */
                                                     /* the current state of the        */
                                                     /* Bluetopia Platform Manager      */
                                                     /* Initialization.                 */

static unsigned int         DEVMCallbackID;          /* Variable which holds the        */
                                                     /* Callback ID of the currently    */
                                                     /* registered Device Manager       */
                                                     /* Callback ID.                    */

static unsigned int         AuthenticationCallbackID;/* Variable which holds the        */
                                                     /* current Authentication Callback */
                                                     /* ID that is assigned from the    */
                                                     /* Device Manager when the local   */
                                                     /* client registers for            */
                                                     /* Authentication.                 */

static BD_ADDR_t            CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                     /* current BD_ADDR of the device   */
                                                     /* which is currently pairing or   */
                                                     /* authenticating.                 */

static GAP_IO_Capability_t  IOCapability;            /* Variable which holds the        */
                                                     /* current I/O Capabilities that   */
                                                     /* are to be used for Secure Simple*/
                                                     /* Pairing.                        */

static Boolean_t            OOBSupport;              /* Variable which flags whether    */
                                                     /* or not Out of Band Secure Simple*/
                                                     /* Pairing exchange is supported.  */

static Boolean_t            MITMProtection;          /* Variable which flags whether or */
                                                     /* not Man in the Middle (MITM)    */
                                                     /* protection is to be requested   */
                                                     /* during a Secure Simple Pairing  */
                                                     /* procedure.                      */

static unsigned int         NumberCommands;          /* Variable which is used to hold  */
                                                     /* the number of Commands that are */
                                                     /* supported by this application.  */
                                                     /* Commands are added individually.*/

static CommandTable_t       CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                     /* used to hold the actual Commands*/
                                                     /* that are supported by this      */
                                                     /* application.                    */

static TransmitThreadInfo_t TransmitThreadInfo;      /* Variable stores information     */
                                                     /* about the transmit thread.      */

static ReceiveThreadInfo_t  ReceiveThreadInfo;       /* Variable stores information     */
                                                     /* about the receive thread.       */

static Boolean_t            LoopbackActive;          /* Variable which flags whether or */
                                                     /* not the application is currently*/
                                                     /* operating in Loopback Mode      */
                                                     /* (TRUE) or not (FALSE).          */

static Boolean_t            AutomaticReadActive;     /* Variable which flags whether or */
                                                     /* not the application is to       */
                                                     /* automatically read all data     */
                                                     /* as it is received.              */

static Boolean_t            DisplayRawDataActive;    /* Variable which specifies whether*/
                                                     /* or not the application displays */
                                                     /* received data in terminal while */
                                                     /* in Automatic Read Mode or       */
                                                     /* Loopback Mode.                  */

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
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
static int QueryRemoteDeviceList(ParameterList_t *TempParam);
static int QueryRemoteDeviceProperties(ParameterList_t *TempParam);
static int AddRemoteDevice(ParameterList_t *TempParam);
static int DeleteRemoteDevice(ParameterList_t *TempParam);
static int PairWithRemoteDevice(ParameterList_t *TempParam);
static int CancelPairWithRemoteDevice(ParameterList_t *TempParam);
static int UnPairRemoteDevice(ParameterList_t *TempParam);
static int QueryRemoteDeviceServices(ParameterList_t *TempParam);
static int RegisterAuthentication(ParameterList_t *TempParam);
static int UnRegisterAuthentication(ParameterList_t *TempParam);

static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);
static int ChangeSimplePairingParameters(ParameterList_t *TempParam);

static int RegisterServerPort(ParameterList_t *TempParam);
static int OpenServerPortRequestResponse(ParameterList_t *TempParam);
static int UnRegisterServerPort(ParameterList_t *TempParam);
static int OpenRemotePort(ParameterList_t *TempParam);
static int ClosePort(ParameterList_t *TempParam);
static int ReadData(ParameterList_t *TempParam);
static int WriteData(ParameterList_t *TempParam);
static int SendLineStatus(ParameterList_t *TempParam);
static int SendPortStatus(ParameterList_t *TempParam);
static int QueryServerPresent(ParameterList_t *TempParam);
static int FindFreeServerPort(ParameterList_t *TempParam);
static int ChangeBufferSize(ParameterList_t *TempParam);
static int LoopbackMode(ParameterList_t *TempParam);
static int AutomaticReadMode(ParameterList_t *TempParam);
static int DisplayRawDataMode(ParameterList_t *TempParam);

static void DisplayRawData(unsigned int DataLength, unsigned char *Data, Byte_t *DataPosition);

static void DisplayLocalDeviceProperties(unsigned long UpdateMask, DEVM_Local_Device_Properties_t *LocalDeviceProperties);
static void DisplayRemoteDeviceProperties(unsigned long UpdateMask, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

static void DisplayParsedServiceData(DEVM_Parsed_SDP_Data_t *ParsedSDPData);
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

static Boolean_t InitializeReceiveThread(unsigned int PortHandle);
static void CleanupReceiveThread(void);
static Boolean_t AddReceiveThreadData(unsigned int PortHandle, unsigned int NumberBytes);
static void *ReceiveThread(void *ThreadParameter);

static Boolean_t InitializeTransmitThread(unsigned int PortHandle);
static void CleanupTransmitThread(void);
static Boolean_t AddTransmitThreadData(unsigned int PortHandle, DWord_t Flags, unsigned long NumberBytes);
static void *TransmitThread(void *ThreadParameter);

   /* BTPM Server Un-Registration Callback function prototype.          */
static void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter);

   /* BTPM Local Device Manager Callback function prototype.            */
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter);

   /* BTPM Local Device Manager Authentication Callback function        */
   /* prototype.                                                        */
static void BTPSAPI DEVM_Authentication_Callback(DEVM_Authentication_Information_t *AuthenticationRequestInformation, void *CallbackParameter);

   /* BTPM Serial Port Profile Manager Callback function prototype.     */
static void BTPSAPI SPPM_Event_Callback(SPPM_Event_Data_t *EventData, void *CallbackParameter);

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

   /* Initialize the command buffer.                                    */
   BTPS_MemInitialize(&TempCommand, 0x00, sizeof(TempCommand));

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
   AddCommand("REGISTERSERVERPORT", RegisterServerPort);
   AddCommand("OPENSERVERPORTREQUESTRESPONSE", OpenServerPortRequestResponse);
   AddCommand("UNREGISTERSERVERPORT", UnRegisterServerPort);
   AddCommand("OPENREMOTEPORT", OpenRemotePort);
   AddCommand("CLOSEPORT", ClosePort);
   AddCommand("READDATA", ReadData);
   AddCommand("WRITEDATA", WriteData);
   AddCommand("SENDLINESTATUS", SendLineStatus);
   AddCommand("SENDPORTSTATUS", SendPortStatus);
   AddCommand("QUERYSERVERPRESENT", QueryServerPresent);
   AddCommand("FINDFREESERVERPORT", FindFreeServerPort);
   AddCommand("CHANGEBUFFERSIZE", ChangeBufferSize);
   AddCommand("LOOPBACKMODE", LoopbackMode);
   AddCommand("AUTOMATICREADMODE", AutomaticReadMode);
   AddCommand("DISPLAYRAWDATAMODE", DisplayRawDataMode);

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
      printf("SPPM>");

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
   unsigned int  Index;
   char         *ret_val = NULL;

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
   unsigned int      i;
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
   /* programmatically add Commands the Global (to this module) Command */
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
   printf("*                  22)PairWithRemoteDevice                       *\r\n");
   printf("*                  23)CancelPairWithRemoteDevice                 *\r\n");
   printf("*                  24)UnPairRemoteDevice                         *\r\n");
   printf("*                  25)QueryRemoteDeviceServices                  *\r\n");
   printf("*                  26)RegisterAuthentication                     *\r\n");
   printf("*                  27)UnRegisterAuthentication                   *\r\n");
   printf("*                  28)PINCodeResponse                            *\r\n");
   printf("*                  29)PassKeyResponse                            *\r\n");
   printf("*                  30)UserConfirmationResponse                   *\r\n");
   printf("*                  31)ChangeSimplePairingParameters              *\r\n");
   printf("*                  32)RegisterServerPort                         *\r\n");
   printf("*                  33)OpenServerPortRequestResponse              *\r\n");
   printf("*                  34)UnRegisterServerPort                       *\r\n");
   printf("*                  35)OpenRemotePort                             *\r\n");
   printf("*                  36)ClosePort                                  *\r\n");
   printf("*                  37)ReadData                                   *\r\n");
   printf("*                  38)WriteData                                  *\r\n");
   printf("*                  39)SendLineStatus                             *\r\n");
   printf("*                  40)SendPortStatus                             *\r\n");
   printf("*                  41)QueryServerPresent                         *\r\n");
   printf("*                  42)FindFreeServerPort                         *\r\n");
   printf("*                  43)ChangeBufferSize                           *\r\n");
   printf("*                  44)LoopbackMode                               *\r\n");
   printf("*                  45)AutomaticReadMode                          *\r\n");
   printf("*                  46)DisplayRawDataMode                         *\r\n");
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
         Result = BTPM_Initialize((unsigned long)getpid(),NULL, ServerUnRegistrationCallback, NULL);

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
      /* Cleanup the receive thread.                                    */
      if(ReceiveThreadInfo.Initialized)
         CleanupReceiveThread();

      /* Cleanup the transmit thread.                                   */
      if(TransmitThreadInfo.Initialized)
         CleanupTransmitThread();

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

DEVM_EnableBluetoothDebug(TRUE, 3, 0, sizeof("mylog.cfa"), (unsigned char *)"mylog.cfa");

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

                  for(Index=0;Index<((unsigned int)Result);Index++)
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
               /* Error attempting to start Device Discovery, inform the*/
               /* user and flag an error.                               */
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

                     for(Index=0;Index<((unsigned int)Result);Index++)
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

   /* The following function is responsible for the registering of an   */
   /* SPPM serveron the local device.  This function returns zero if    */
   /* successful, of a negative value if an error occurred.             */
static int RegisterServerPort(ParameterList_t *TempParam)
{
   int                                ret_val;
   long                               Result;
   char                               ServiceName[48];
   unsigned int                       PortHandle;
   unsigned long                      Flags;
   SPPM_Service_Record_Information_t  ServiceRecordInformation;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Check to see if the flags parameter was specified.          */
         if(TempParam->NumberofParameters > 1)
            Flags = TempParam->Params[1].intParam;
         else
            Flags = 0;

         if((Result = SPPM_RegisterServerPort(TempParam->Params[0].intParam, Flags, SPPM_Event_Callback, NULL)) >= 0)
         {
            /* Note the Port Handle.                                    */
            PortHandle = (unsigned int)Result;

            /* Next, attempt to register the SDP Record for the server. */

            /* Build the structure to hold the SDP Data                 */
            ServiceRecordInformation.NumberServiceClassUUID = 0;
            ServiceRecordInformation.SDPUUIDEntries         = NULL;;
            ServiceRecordInformation.ProtocolList           = NULL;
            ServiceRecordInformation.ServiceName            = ServiceName;

            /* Generate the name for the service.                       */
            sprintf(ServiceName, "SPPM Serial Port: %d", TempParam->Params[0].intParam);

            /* Finally, register the SDP information.                   */
            if((Result = SPPM_RegisterServerPortServiceRecord(PortHandle, &ServiceRecordInformation)) > 0)
            {
               /* Everything initialized, go ahead and inform the user  */
               /* of the results.                                       */
               printf("SPPM_RegisterServerPort(%u) Success. Port Handle: %d.\r\n", TempParam->Params[0].intParam, PortHandle);

               printf("SPPM_RegisterServerPortServiceRecord Success. Record Handle: %lu (0x%08lX).\r\n", (unsigned long)Result, (unsigned long)Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error registering the SDP Service Record, inform the  */
               /* user and flag an error.                               */
               printf("SPPM_RegisterServerPortServiceRecord Failure: %d, %s.\r\n", (int)Result, ERR_ConvertErrorCodeToString((int)Result));

               /* Go ahead and un-register the server port.             */
               SPPM_UnRegisterServerPort(PortHandle);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Error registering the SPPM server port, inform the user  */
            /* and flag an error.                                       */
            printf("SPPM_RegisterServerPort(%u) Failure: %d, %s.\r\n", TempParam->Params[0].intParam, (int)Result, ERR_ConvertErrorCodeToString((int)Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: RegisterServerPort [Port Handle] [Flags (1: Authorization, 2: Authentication, 4: Encryption) (optional)].\r\n");

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

   /* The following function is responsible for accepting or rejecting  */
   /* an incoming server connection.  It should be called in response to*/
   /* receiving the setServerOpenRequest event.  This function returns  */
   /* zero of successful and a negative value if an error occured.      */
static int OpenServerPortRequestResponse(ParameterList_t *TempParam)
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
         if((Result = SPPM_OpenServerPortRequestResponse(TempParam->Params[0].intParam, (Boolean_t)TempParam->Params[1].intParam)) == 0)
         {
            printf("SPPM_OpenServerPortRequestResponse(%s) Success: %d.\r\n", TempParam->Params[1].intParam?"Accept":"Reject", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error accepting/rejecting the request, inform the user   */
            /* and flag an error.                                       */
            printf("SPPM_OpenServerPortRequestResponse(%s) Failure: %d, %s.\r\n", TempParam->Params[1].intParam?"Accept":"Reject", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: OpenServerPortRequestResponse [Port Handle] [Accept/Reject Flag].\r\n");

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

   /* The following function is responsible for unregistering an SPPM   */
   /* server on the local device.  This function returns zero if it is  */
   /* successful.                                                       */
static int UnRegisterServerPort(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         if((Result = SPPM_UnRegisterServerPort(TempParam->Params[0].intParam)) == 0)
         {
            printf("SPPM_UnRegisterServerPort(%u) Success: %d.\r\n", TempParam->Params[0].intParam, Result);

            /* Note that we do not have to delete the service record    */
            /* because it will automatically be deleted.                */

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error unregistering the SPPM Server Port, inform the user*/
            /* and flag an error.                                       */
            printf("SPPM_UnRegisterServerPort(%u) Failure: %d, %s.\r\n", TempParam->Params[0].intParam, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: UnRegisterServerPort [Port Handle].\r\n");

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

   /* The following function is responsible for opening an SPPM         */
   /* connection to a remote device.  This function returns zero if it  */
   /* is successful.                                                    */
static int OpenRemotePort(ParameterList_t *TempParam)
{
   int          Result;
   int          ret_val;
   BD_ADDR_t    BD_ADDR;
   unsigned int Flags;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].strParam) && (strlen(TempParam->Params[0].strParam)))
      {
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* Determine if flags were specified.                          */
         if(TempParam->NumberofParameters > 2)
            Flags = TempParam->Params[2].intParam;
         else
            Flags = 0;

         if((Result = SPPM_OpenRemotePort(BD_ADDR, TempParam->Params[1].intParam, Flags, SPPM_Event_Callback, NULL, NULL)) > 0)
         {
            printf("SPPM_OpenRemotePort(%u) Success. Port Handle: %d.\r\n", TempParam->Params[1].intParam, Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error opening the remote SPPM connection, inform the user*/
            /* and flag an error.                                       */
            printf("SPPM_OpenRemotePort(%u) Failure: %d, %s.\r\n", TempParam->Params[1].intParam, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: OpenRemotePort [BD_ADDR] [Server Port] [Flags (Optional - 1:Authentication, 2:Encryption)]\r\n");

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

   /* The following function is responsible for closing an active SPPM  */
   /* connection.  This function returns zero if it is successful.      */
static int ClosePort(ParameterList_t *TempParam)
{
   int          Result;
   int          ret_val;
   unsigned int Timeout;
   unsigned int PortHandle;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         PortHandle = (unsigned int)TempParam->Params[0].intParam;

         /* Check to see if a Timeout was specified.                    */
         if(TempParam->NumberofParameters > 1)
            Timeout = TempParam->Params[1].intParam;
         else
            Timeout = 0;

         if((Result = SPPM_ClosePort(PortHandle, Timeout)) == 0)
         {
            printf("SPPM_ClosePort Success: %d.\r\n", Result);

            /* Check if we need to cleanup and stop the receive thread. */
            if((ReceiveThreadInfo.Initialized) && (ReceiveThreadInfo.PortHandle == PortHandle))
            {
               /* Cleanup and stop the receive thread.                  */
               CleanupReceiveThread();
            }

            /* Check if we need to cleanup and stop the transmit thread.*/
            if((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.PortHandle == PortHandle))
            {
               /* Cleanup and stop the transmit thread.                 */
               CleanupTransmitThread();
            }

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error closing the port, inform the user and flag an      */
            /* error.                                                   */
            printf("SPPM_ClosePort Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ClosePort [Port Handle] [Timeout (optional)].\r\n");

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

   /* The following function is responsible for reading data from an    */
   /* SPPM connection.  This function returns zero if no data was       */
   /* available, a positive number representing the number of bytes     */
   /* read, or a negative failure if an error occurs.                   */
static int ReadData(ParameterList_t *TempParam)
{
   int            Result;
   int            ret_val;
   unsigned int   Timeout;
   unsigned char *Buffer;
   unsigned char  DataBuffer[64];

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Check to see if a Timeout was specified.                    */
         if(TempParam->NumberofParameters > 2)
            Timeout = TempParam->Params[2].intParam;
         else
            Timeout = 0;

         /* Next, allocate a buffer to receive the data into (if the    */
         /* internal buffer is too small).                              */
         if(TempParam->Params[1].intParam < sizeof(DataBuffer))
            Buffer = DataBuffer;
         else
            Buffer = BTPS_AllocateMemory(TempParam->Params[1].intParam);

         /* Make sure that there is a buffer to read the data into.     */
         if(Buffer)
         {
            if((Result = SPPM_ReadData(TempParam->Params[0].intParam, Timeout, TempParam->Params[1].intParam, TempParam->Params[1].intParam?Buffer:NULL)) >= 0)
            {
               printf("SPPM_ReadData(%u) Success: %d bytes read.\r\n", TempParam->Params[1].intParam, Result);

               /* If data was read, display the data to the user.       */
               if((TempParam->Params[1].intParam) && (Result))
               {
                  printf("Data:");
                  DisplayRawData((unsigned int)Result, Buffer, NULL);
                  printf("\r\n\r\n");
               }

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error reading the data from the SPPM port, inform the */
               /* user and flag an error.                               */
               printf("SPPM_ReadData(%u) Failure: %d, %s.\r\n", TempParam->Params[1].intParam, Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }

            /* If a buffer was allocated, go ahead and free the memory. */
            if((Buffer) && (Buffer != DataBuffer))
               BTPS_FreeMemory(Buffer);
         }
         else
         {
            /* Unable to allocate memory for the input buffer.          */
            printf("Unable to allocate memory: %u bytes.\r\n", TempParam->Params[1].intParam);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ReadData [Port Handle] [DataLength] [Timeout (optional)].\r\n");

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

   /* The following function is responsible for writing data to a       */
   /* specified SPPM connection.  This function returns zero if it is   */
   /* successful.                                                       */
static int WriteData(ParameterList_t *TempParam)
{
   int          Result;
   int          ret_val;
   unsigned int Timeout;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[1].strParam) && (strlen(TempParam->Params[1].strParam)))
      {
         /* Check to see if a Timeout was specified.                    */
         if(TempParam->NumberofParameters > 2)
            Timeout = TempParam->Params[2].intParam;
         else
            Timeout = 0;

         if((TempParam->Params[1].strParam[0] < '0') || (TempParam->Params[1].strParam[0] > '9'))
         {
            if((Result = SPPM_WriteData(TempParam->Params[0].intParam, Timeout, strlen(TempParam->Params[1].strParam), (unsigned char *)(TempParam->Params[1].strParam))) >= 0)
            {
               printf("SPPM_WriteData Success: %d bytes written.\r\n", Result);

               /* Flag success.                                         */
               ret_val = 0;
            }
            else
            {
               /* Error writing the data, inform the user and flag an   */
               /* error.                                                */
               printf("SPPM_WriteData Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            if(AddTransmitThreadData((unsigned int)TempParam->Params[0].intParam, TRANSMIT_THREAD_INFO_FLAGS_WRITE_DATA_COMMAND, (DWord_t)TempParam->Params[1].intParam))
            {
               printf("Success, sending data.\r\n");

               ret_val = 0;
            }
            else
            {
               ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: WriteData [Port Handle] [String | NumberBytes] [Timeout (optional)].\r\n");

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

   /* The following function is responsible for sending a line status   */
   /* notification over an SPPM Port.  This function returns zero if it */
   /* is successful.                                                    */
static int SendLineStatus(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         if((Result = SPPM_SendLineStatus(TempParam->Params[0].intParam, TempParam->Params[1].intParam)) == 0)
         {
            printf("SPPM_SendLineStatus(0x%08X) Success: %d.\r\n", TempParam->Params[1].intParam, Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error sending the line status, inform the user and flag  */
            /* an error.                                                */
            printf("SPPM_SendLineStatus(0x%08X) Failure: %d, %s.\r\n", TempParam->Params[1].intParam, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SendLineStatus [Port Handle] [Status Mask].\r\n");

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

   /* The following function is responsible for sending a port status   */
   /* notification over an SPPM Port.  This function returns zero if it */
   /* is successful.                                                    */
static int SendPortStatus(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   Boolean_t          BreakSignal;
   unsigned int       BreakTimeout;
   SPPM_Port_Status_t PortStatus;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Initialize the defaults for the Break Signal.               */
         BreakSignal  = (Boolean_t)FALSE;
         BreakTimeout = 0;

         if(TempParam->NumberofParameters >= 3)
         {
            BreakSignal = (Boolean_t)TempParam->Params[2].intParam;

            if(TempParam->NumberofParameters > 3)
               BreakTimeout = TempParam->Params[3].intParam;
         }

         /* Build the Port Status Information.                          */
         PortStatus.PortStatusMask = TempParam->Params[1].intParam;
         PortStatus.BreakSignal    = BreakSignal;
         PortStatus.BreakTimeout   = BreakTimeout;

         if((Result = SPPM_SendPortStatus(TempParam->Params[0].intParam, &PortStatus)) == 0)
         {
            printf("SPPM_SendPortStatus(0x%08X) Success: %d.\r\n", TempParam->Params[1].intParam, Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error sending the line status, inform the user and flag  */
            /* an error.                                                */
            printf("SPPM_SendPortStatus(0x%08X) Failure: %d, %s.\r\n", TempParam->Params[1].intParam, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: SendPortstatus [Port Handle] [Status Mask] [Break Flag - 0/1 (optional)] [Break Length (optional)].\r\n");

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

   /* The following function is responsible for querying whether a      */
   /* particular SPPM Port current has an SPP Server allocated to it.   */
   /* This function returns zero if it is successful.                   */
static int QueryServerPresent(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   Boolean_t Present;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters == 1))
      {
         if((Result = SPPM_QueryServerPresent(TempParam->Params[0].intParam, &Present)) == 0)
         {
            printf("SPPM_QueryServerPresent(0x%08X) Success: %d, %s.\r\n", TempParam->Params[0].intParam, Result, (Present?"TRUE":"FALSE"));

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error querying for a server port, inform the user and    */
            /* flag an error.                                           */
            printf("SPPM_QueryServerPresent(0x%08X) Failure: %d, %s.\r\n", TempParam->Params[0].intParam, Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: QueryServerPresent [Server Port].\r\n");

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

   /* The following function is responsible for locating an SPPM Port   */
   /* which is available to be used as an SPPM Server. This function    */
   /* returns zero if it is successful.                                 */
static int FindFreeServerPort(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters == 0))
      {
         if((Result = SPPM_FindFreeServerPort()) > 0)
         {
            printf("SPPM_FindFreeServerPort() Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error querying for a server port, inform the user and    */
            /* flag an error.                                           */
            printf("SPPM_FindFreeServerPort() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: FindFreeServerPort.\r\n");

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

   /* The following function is responsible for changing the buffer size*/
   /* for a specified SPPM Port.  This function returns zero if it is   */
   /* successful.                                                       */
static int ChangeBufferSize(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Go ahead and attempt to change the buffer size.             */
         if((Result = SPPM_ChangeBufferSize(TempParam->Params[0].intParam, TempParam->Params[1].intParam, TempParam->Params[2].intParam)) == 0)
         {
            printf("SPPM_ChangeBufferSize Success: %d.\r\n", Result);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error sending the line status, inform the user and flag  */
            /* an error.                                                */
            printf("SPPM_ChangeBufferSize Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         printf("Usage: ChangeBufferSize [Port Handle] [Receive Buffer Size] [Transmit Buffer Size].\r\n");

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

   /* The following function is responsible for setting the application */
   /* state to support loopback mode.  This function will return zero on*/
   /* successful execution and a negative value on errors.              */
static int LoopbackMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         if(TempParam->Params->intParam)
            LoopbackActive = TRUE;
         else
         {
            LoopbackActive = FALSE;

            /* Check if we need to cleanup and stop the receive thread. */
            if((ReceiveThreadInfo.Initialized) && (!AutomaticReadActive))
            {
               /* Cleanup and stop the receive thread.                  */
               CleanupReceiveThread();
            }

            /* Check if we need to stop the transmit thread.  If the    */
            /* transmit thread has been initialized and any looped-back */
            /* data has been sent from the thread, then stop the thread.*/
            /* This will essentially allow the user to enable and       */
            /* disable loopback mode while data added to the buffer by  */
            /* the WriteData command is being sent.  This will however  */
            /* stop the data that is being sent even if it was added to */
            /* the buffer by the WriteData command and any looped-back  */
            /* data was added to the send buffer while the data from the*/
            /* WriteData command was being sent.  The same is true if   */
            /* WriteData is called while data is being "looped-back",   */
            /* i.e. the thread will be stopped in this case too.        */
            if((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.Flags & TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK))
               CleanupTransmitThread();
         }
      }

      /* Output the current Loopback Mode.                              */
      printf("Current Loopback Mode is: %s.\r\n", LoopbackActive?"ACTIVE":"INACTIVE");

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

   /* The following function is responsible for enabling Automatic Read */
   /* Mode in the application.  While in Automatic Read Mode any        */
   /* received SPP data is automatically read from the SPPM buffer.     */
   /* This function will return zero on successful execution and a      */
   /* negative value on errors.                                         */
static int AutomaticReadMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Check to see if LoopbackMode is active.  If it is then we will */
      /* not process this command (and we will inform the user).        */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               AutomaticReadActive = TRUE;
            else
            {
               AutomaticReadActive = FALSE;

               /* Loop back has been disabled, let's see if we need to  */
               /* stop receive thread.                                  */
               if((ReceiveThreadInfo.Initialized) && (!LoopbackActive))
               {
                  /* Stop the receive thread.  Note that the transmit   */
                  /* thread will stop itself after it has flushed its   */
                  /* buffers and sees that loopback mode has been       */
                  /* disabled.                                          */
                  CleanupReceiveThread();
               }
            }
         }

         /* Output the current Automatic Read Mode state.               */
         printf("Current Automatic Read Mode is: %s.\r\n", AutomaticReadActive?"ACTIVE":"INACTIVE");

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         printf("Error: Automatic Read Mode cannot be used when operating in Loopback Mode.\r\n");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* Not initialized, flag an error.                                */
      printf("Error: Platform Manager has not been initialized.\r\n");

      ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for specifying if the data  */
   /* received while in Automatic Read Mode or Loopback Mode should be  */
   /* displayed in the terminal.  This function will return zero on     */
   /* successful execution and a negative value on errors.              */
static int DisplayRawDataMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check to make sure that we have already been initialized.  */
   if(Initialized)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         if(TempParam->Params->intParam)
            DisplayRawDataActive = TRUE;
         else
            DisplayRawDataActive = FALSE;
      }
      else
      {
         printf("The DisplayRawDataMode command is used to specify if raw\r\n"
                "data that is received while in Automatic Read Mode or while\r\n"
                "in Loopback Mode is displayed in the terminal.\r\n\r\n");
      }

      /* Output the current Display Raw Data Mode state.                */
      printf("Current Display Raw Data Mode set to: %s.\r\n", DisplayRawDataActive?"ACTIVE":"INACTIVE");

      /* Flag that the function was successful.                         */
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

   /* The following helper function displays raw data in terminal.      */
static void DisplayRawData(unsigned int DataLength, unsigned char *Data, Byte_t *DataPosition)
{
   unsigned int Index;
   Byte_t       TempPostion;

   /* Check if the caller specified the current data position.          */
   if(!DataPosition)
   {
      /* The caller did not specify a data position, specify one now.   */
      TempPostion  = 0;
      DataPosition = &TempPostion;
   }

   for(Index=0;Index<DataLength;Index++)
   {
      /* Check if the current terminal position is greater than 16.     */
      if((*DataPosition) >= 16)
      {
         /* The position is greater than 16, insert a new line.  This   */
         /* will result in 16 values being displayed in each row.       */
         printf("\r\n");

         (*DataPosition) = 0;
      }

      (*DataPosition)++;

      /* Display the data in the terminal.                              */
      printf("0x%02X ", Data[Index]);
   }

   /* Flush the output buffer.                                          */
   fflush(stdout);
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

      printf("BD_ADDR:      %s\r\n", Buffer);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASS_OF_DEVICE))
         printf("COD:           0x%02X%02X%02X\r\n", RemoteDeviceProperties->ClassOfDevice.Class_of_Device0, RemoteDeviceProperties->ClassOfDevice.Class_of_Device1, RemoteDeviceProperties->ClassOfDevice.Class_of_Device2);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_NAME))
         printf("Device Name:   %s\r\n", (RemoteDeviceProperties->DeviceNameLength)?RemoteDeviceProperties->DeviceName:"");

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_DEVICE_FLAGS))
         printf("Device Flags:  0x%08lX\r\n", RemoteDeviceProperties->RemoteDeviceFlags);

      if((!UpdateMask) || (UpdateMask & DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_RSSI))
         printf("RSSI:          %d\r\n", RemoteDeviceProperties->RSSI);

      if((!UpdateMask) && (RemoteDeviceProperties->RemoteDeviceFlags & DEVM_REMOTE_DEVICE_FLAGS_DEVICE_CURRENTLY_CONNECTED))
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

   /* The following function initializes the receive thread.            */
static Boolean_t InitializeReceiveThread(unsigned int PortHandle)
{
   Boolean_t ret_val;

   /* Check if the receive thread is initialized.                       */
   if(!ReceiveThreadInfo.Initialized)
   {
      /* The receive thread is not initialized, go ahead and create the */
      /* events for the thread.                                         */
      if((ReceiveThreadInfo.DataAvailableEvent = BTPS_CreateEvent(FALSE)) && (ReceiveThreadInfo.ThreadShutDownEvent = BTPS_CreateEvent(FALSE)))
      {
         /* Initialize variables used by the thread before we start it, */
         /* if thread creation fails these variables will get cleared in*/
         /* the cleanup function which is called below if an error      */
         /* occurs.                                                     */
         ReceiveThreadInfo.Initialized    = TRUE;
         ReceiveThreadInfo.WaitingForData = TRUE;
         ReceiveThreadInfo.PortHandle     = PortHandle;

         /* Successfully created the events, next try to start the      */
         /* thread.                                                     */
         if(BTPS_CreateThread(ReceiveThread, TRANSMIT_THREAD_STACK_SIZE, NULL))
         {
            /* Flag success to the caller.                              */
            ret_val = TRUE;
         }
         else
         {
            printf("Error: Could not create receive thread.\r\n");

            ret_val = FALSE;
         }
      }
      else
      {
         printf("Error: Could not create receive thread's events.\r\n");

         ret_val = FALSE;
      }

      /* Check if any errors occurred.                                  */
      if(!ret_val)
      {
         /* An error occurred, cleanup the thread's variables.          */
         CleanupReceiveThread();
      }
   }
   else
   {
      printf("Error: Receive thread already initialized.\r\n");

      /* Flag the error back to the caller.                             */
      ret_val = FALSE;
   }

   return(ret_val);
}

   /* The following function is responsible for freeing any resources   */
   /* allocated for the receive thread.                                 */
static void CleanupReceiveThread(void)
{
   /* Check if the thread has been initialized.                         */
   if(ReceiveThreadInfo.Initialized)
   {
      /* Flag that the thread is no longer initialized, do this first   */
      /* because the thread checks on this variable before it stops to  */
      /* determine if it needs to cleanup its state.  Setting this at   */
      /* the end of this function instead of here would cause the       */
      /* application to try to cleanup the thread's variables twice.    */
      ReceiveThreadInfo.Initialized = FALSE;

      /* Check if the thread is waiting for data.                       */
      if(ReceiveThreadInfo.WaitingForData)
      {
         /* The thread is waiting for data, set the data available event*/
         /* so that the thread stops waiting.                           */
         BTPS_SetEvent(ReceiveThreadInfo.DataAvailableEvent);
      }

      /* Wait for the thread to set the shutdown event.                 */
      BTPS_WaitEvent(ReceiveThreadInfo.ThreadShutDownEvent, BTPS_INFINITE_WAIT);

      if(ReceiveThreadInfo.AutomaticReadBuffer)
         BTPS_FreeMemory(ReceiveThreadInfo.AutomaticReadBuffer);

      if(ReceiveThreadInfo.DataAvailableEvent)
         BTPS_CloseEvent(ReceiveThreadInfo.DataAvailableEvent);

      if(ReceiveThreadInfo.ThreadShutDownEvent)
         BTPS_CloseEvent(ReceiveThreadInfo.ThreadShutDownEvent);

      BTPS_MemInitialize(&ReceiveThreadInfo, 0x00, sizeof(ReceiveThreadInfo));
   }
}

   /* The following function is responsible for notifying the receive   */
   /* thread that data is available for it to process.                  */
static Boolean_t AddReceiveThreadData(unsigned int PortHandle, unsigned int NumberBytes)
{
   Boolean_t ret_val = FALSE;

   /* Check for valid input parameters.                                 */
   if((NumberBytes) && ((!ReceiveThreadInfo.Initialized) || ((ReceiveThreadInfo.Initialized) && (ReceiveThreadInfo.PortHandle == PortHandle))))
   {
      /* The input parameters are valid, check if the thread is         */
      /* initialized and if it is not then initialize it now.           */
      if((ReceiveThreadInfo.Initialized) || (ret_val = InitializeReceiveThread(PortHandle)))
      {
         /* Update the number of bytes available for the receive thread */
         /* to process.                                                 */
         ReceiveThreadInfo.AvailableDataSize += NumberBytes;

         /* Check if the receive thread is waiting for data.            */
         if(ReceiveThreadInfo.WaitingForData)
         {
            /* The receive thread is waiting for data, set the data     */
            /* available event so that it will continue processing.     */
            BTPS_SetEvent(ReceiveThreadInfo.DataAvailableEvent);
         }

         /* Flag success to the caller.                                 */
         ret_val = TRUE;
      }
      else
      {
         printf("Error: Could not initialize receive thread.\r\n");

         ret_val = FALSE;
      }
   }
   else
   {
      /* An input parameter is invalid, determine which input parameter */
      /* caused the error.                                              */
      if(!NumberBytes)
         printf("Error: Number of bytes to add to receive buffer is 0.\r\n");
      else
      {
         if((ReceiveThreadInfo.Initialized) && (ReceiveThreadInfo.PortHandle == PortHandle))
         {
            printf("Error: Could not add data to receive thread's buffer because thread is receiving on a different port.\r\n");
         }
      }

      ret_val = FALSE;
   }

   return(ret_val);
}

   /* The following function is responsible for reading data from the   */
   /* SPPM module.                                                      */
static void *ReceiveThread(void *ThreadParameter)
{
   int       Result;
   DWord_t   NumBytesToReceive;
   Boolean_t Error;

   printf("Receive thread started.\r\n");

   Error = FALSE;

   /* Loop while the thread is initialized and while no errors have     */
   /* occurred.                                                         */
   while((ReceiveThreadInfo.Initialized) && (!Error))
   {
      /* Flag that the thread is waiting for data.                      */
      ReceiveThreadInfo.WaitingForData = TRUE;

      /* Wait for the data available event.                             */
      BTPS_WaitEvent(ReceiveThreadInfo.DataAvailableEvent, BTPS_INFINITE_WAIT);

      /* Flag that we are no longer waiting for data.                   */
      ReceiveThreadInfo.WaitingForData = FALSE;

      /* Reset the data available event.                                */
      BTPS_ResetEvent(ReceiveThreadInfo.DataAvailableEvent);

      /* Loop while the thread is initialized, there is data available  */
      /* to read, and while no errors have occurred.                    */
      while((ReceiveThreadInfo.Initialized) && (ReceiveThreadInfo.AvailableDataSize > 0) && (!Error))
      {
         /* Determine the number of bytes to read.                      */
         NumBytesToReceive = MIN(ReceiveThreadInfo.AvailableDataSize, MAX_RECEIVE_THREAD_RECEIVE_SIZE);

         /* Check if automatic read mode is active and loopback mode is */
         /* not active.                                                 */
         if((AutomaticReadActive) && (!LoopbackActive))
         {
            /* Automatic read mode is active and loopback mode is not   */
            /* active, check if we have a buffer to read the data in to */
            /* and if there is enough room in the buffer to read the    */
            /* data.                                                    */
            if((!ReceiveThreadInfo.AutomaticReadBuffer) || (NumBytesToReceive > ReceiveThreadInfo.AutomaticReadBufferSize))
            {
               /* We need to allocate a buffer or increase the size of  */
               /* it, first check if we already have a buffer.          */
               if(ReceiveThreadInfo.AutomaticReadBuffer)
               {
                  /* A buffer has been allocated already and we now need*/
                  /* a larger buffer, free the buffer now.              */
                  BTPS_FreeMemory(ReceiveThreadInfo.AutomaticReadBuffer);
               }

               /* Allocate memory for the buffer.                       */
               ReceiveThreadInfo.AutomaticReadBuffer = (unsigned char *)BTPS_AllocateMemory(NumBytesToReceive);

               /* Check if we were able to allocate a buffer.           */
               if(ReceiveThreadInfo.AutomaticReadBuffer)
               {
                  /* We were able to allocate a buffer, set the size of */
                  /* it now.                                            */
                  ReceiveThreadInfo.AutomaticReadBufferSize = NumBytesToReceive;
               }
               else
               {
                  printf("Error: Could not allocate %lu bytes for read buffer.\r\n", (unsigned long)NumBytesToReceive);

                  /* We were not able to allocate a buffer, specify that*/
                  /* the size of the buffer is 0.                       */
                  ReceiveThreadInfo.AutomaticReadBufferSize = 0;

                  /* Flag the error so that this thread stops.          */
                  Error = TRUE;
               }
            }

            /* Check if we have a buffer and if there is enough space in*/
            /* the buffer to read the data.                             */
            if((ReceiveThreadInfo.AutomaticReadBuffer) && (ReceiveThreadInfo.AutomaticReadBufferSize >= NumBytesToReceive))
            {
               /* Read the data.                                        */
               if((Result = SPPM_ReadData(ReceiveThreadInfo.PortHandle, 0, NumBytesToReceive, ReceiveThreadInfo.AutomaticReadBuffer)) >= 0)
               {
                  /* Decrement the amount of available data to read from*/
                  /* the SPPM module.                                   */
                  ReceiveThreadInfo.AvailableDataSize -= Result;

                  /* Check if display raw data mode is active.          */
                  if(DisplayRawDataActive)
                  {
                     /* Display raw data mode is active, display the    */
                     /* data in the terminal.                           */
                     DisplayRawData(Result, ReceiveThreadInfo.AutomaticReadBuffer, &ReceiveThreadInfo.DisplayRawDataPosition);
                  }

                  /* Check if we were able to read all of the data.     */
                  if(Result != ((int)NumBytesToReceive))
                  {
                     /* We were not able to read all of the data, inform*/
                     /* the user and flag an error.                     */
                     printf("SPPM_ReadData() Error: Bytes Requested = %lu and Bytes Actually Read = %d\r\n", (unsigned long)NumBytesToReceive, Result);

                     Error = TRUE;
                  }
               }
               else
               {
                  /* Error reading the data from the SPPM port, inform  */
                  /* the user and flag an error.                        */
                  printf("SPPM_ReadData(%lu) Failure: %d, %s.\r\n", (unsigned long)NumBytesToReceive, Result, ERR_ConvertErrorCodeToString(Result));

                  Error = TRUE;
               }
            }
         }
         else
         {
            /* Check if loopback mode is active.                        */
            if(LoopbackActive)
            {
               /* Loopback mode is active, add the data to the transmit */
               /* thread's buffer.                                      */
               if(AddTransmitThreadData(ReceiveThreadInfo.PortHandle, TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK, NumBytesToReceive))
               {
                  /* We successfully added the data to the transmit     */
                  /* thread's buffer, decrement the number of bytes     */
                  /* available to read from the SPPM module.            */
                  ReceiveThreadInfo.AvailableDataSize -= NumBytesToReceive;
               }
               else
               {
                  /* An error occurred adding the data to the transmit  */
                  /* thread's buffer, flag the error so that this thread*/
                  /* stops processing.                                  */
                  Error = TRUE;
               }
            }
         }

         /* Check if automatic read mode or loopback are active.        */
         if((AutomaticReadActive) || (LoopbackActive))
         {
            /* Either automatic read mode or loopback mode is active,   */
            /* update the count of the number of bytes we've receive so */
            /* far.                                                     */
            ReceiveThreadInfo.ReceiveCount       += NumBytesToReceive;
            ReceiveThreadInfo.ReceiveCountStatus += NumBytesToReceive;

            /* Check if we've received a megabyte or more of data.      */
            if(ReceiveThreadInfo.ReceiveCountStatus >= (1000 * 1000))
            {
               /* We've received a megabyte or more of data, display how*/
               /* many megabytes we've received so far in the terminal. */
               printf("Status: Received%s %lu.%03lu MB of data.\r\n",
                  (LoopbackActive ? " and looped-back (sent)" : ""),
                  (unsigned long)(ReceiveThreadInfo.ReceiveCount / (1000 * 1000)),
                  (unsigned long)((ReceiveThreadInfo.ReceiveCount % (1000 * 1000)) / 1000));

               ReceiveThreadInfo.ReceiveCountStatus = 0;
            }
         }
      }
   }

   /* Set the thread shut down event.                                   */
   BTPS_SetEvent(ReceiveThreadInfo.ThreadShutDownEvent);

   /* Check if the the thread is still initialized.                     */
   if(ReceiveThreadInfo.Initialized)
   {
      /* The thread is still initialized, cleanup the thread's          */
      /* variables.                                                     */
      CleanupReceiveThread();
   }

   printf("Receive thread stopped.\r\n");

   return(NULL);
}

   /* The following function initializes the transmit thread.           */
static Boolean_t InitializeTransmitThread(unsigned int PortHandle)
{
   Boolean_t ret_val;

   /* Check if the transmit thread is already initialized.              */
   if(!TransmitThreadInfo.Initialized)
   {
      /* The thread is not initialized, create the mutexes used by the  */
      /* thread.                                                        */
      if((TransmitThreadInfo.TransmitBufferMutex = BTPS_CreateMutex(FALSE)) && (TransmitThreadInfo.TransmitBufferReallocMutex = BTPS_CreateMutex(FALSE)))
      {
         /* Mutexes were created successfully, create the events used by*/
         /* the thread.                                                 */
         if((TransmitThreadInfo.DataAvailableEvent = BTPS_CreateEvent(FALSE)) && (TransmitThreadInfo.ThreadShutDownEvent = BTPS_CreateEvent(FALSE)))
         {
            /* Initialize variables used by the thread before we start  */
            /* it, if thread creation fails these variables will get    */
            /* cleared in the cleanup function which is called below if */
            /* an error occurs.                                         */
            TransmitThreadInfo.Initialized    = TRUE;
            TransmitThreadInfo.PortHandle     = PortHandle;
            TransmitThreadInfo.WaitingForData = TRUE;

            BTPS_MemInitialize(&TransmitThreadInfo.TransmitBuffer, 0x00, sizeof(TransmitThreadInfo.TransmitBuffer));

            /* Events created successfully, create the thread.          */
            if(BTPS_CreateThread(TransmitThread, TRANSMIT_THREAD_STACK_SIZE, NULL))
            {
               /* Flag success to the caller.                           */
               ret_val = TRUE;
            }
            else
            {
               printf("Error: Could not create transmit thread.\r\n");

               ret_val = FALSE;
            }
         }
         else
         {
            printf("Error: Could not create transmit thread's events.\r\n");

            ret_val = FALSE;
         }
      }
      else
      {
         printf("Error: Could not create transmit thread's mutexes.\r\n");

         ret_val = FALSE;
      }

      /* Check if any errors occurred.                                  */
      if(!ret_val)
      {
         /* An error occurred, cleanup the thread's variables.          */
         CleanupTransmitThread();
      }
   }
   else
   {
      printf("Error: Transmit thread already initialized.\r\n");

      /* Flag the error back to the caller.                             */
      ret_val = FALSE;
   }

   return(ret_val);
}

   /* The following function is responsible for freeing any resources   */
   /* allocated for the transmit thread.                                */
static void CleanupTransmitThread(void)
{
   /* Check if the thread has been initialized.                         */
   if(TransmitThreadInfo.Initialized)
   {
      /* Flag that the thread is no longer initialized, do this first   */
      /* because the thread checks on this variable before it stops to  */
      /* determine if it needs to cleanup its state.  Setting this at   */
      /* the end of this function instead of here would cause the       */
      /* application to try to cleanup the thread's variables twice.    */
      TransmitThreadInfo.Initialized = FALSE;

      /* Check if the thread is waiting for data.                       */
      if(TransmitThreadInfo.WaitingForData)
      {
         /* The thread is waiting for data, set the data available event*/
         /* so that the thread stops waiting.                           */
         BTPS_SetEvent(TransmitThreadInfo.DataAvailableEvent);
      }

      /* Wait for the thread to set the shutdown event.                 */
      BTPS_WaitEvent(TransmitThreadInfo.ThreadShutDownEvent, BTPS_INFINITE_WAIT);

      if(TransmitThreadInfo.TransmitBufferMutex)
         BTPS_CloseMutex(TransmitThreadInfo.TransmitBufferMutex);

      if(TransmitThreadInfo.TransmitBufferReallocMutex)
         BTPS_CloseMutex(TransmitThreadInfo.TransmitBufferReallocMutex);

      if(TransmitThreadInfo.TransmitBuffer.Buffer)
         BTPS_FreeMemory(TransmitThreadInfo.TransmitBuffer.Buffer);

      if(TransmitThreadInfo.DataAvailableEvent)
         BTPS_CloseEvent(TransmitThreadInfo.DataAvailableEvent);

      if(TransmitThreadInfo.ThreadShutDownEvent)
         BTPS_CloseEvent(TransmitThreadInfo.ThreadShutDownEvent);

      BTPS_MemInitialize(&TransmitThreadInfo, 0x00, sizeof(TransmitThreadInfo));
   }
}

   /* The following function is responsible for adding data to the      */
   /* transmit thread's buffer.                                         */
static Boolean_t AddTransmitThreadData(unsigned int PortHandle, DWord_t Flags, unsigned long NumberBytes)
{
   Boolean_t      ret_val = FALSE;
   int            Result;
   unsigned char *PreviousBufferEnd;
   unsigned char *NewBufferEnd;
   unsigned long  Index;
   unsigned long  AllocationSize;
   DWord_t        NumRemaining;
   DWord_t        NumToCopy;
   DWord_t        NumContinuous;
   Byte_t         DataArray[256];

   /* Check for valid input parameters.                                 */
   if((NumberBytes) && ((!TransmitThreadInfo.Initialized) || ((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.PortHandle == PortHandle))))
   {
      /* Check if the thread has been initialized, if it has not then   */
      /* initialize it.                                                 */
      if((TransmitThreadInfo.Initialized) || (ret_val = InitializeTransmitThread(PortHandle)))
      {
         /* Wait for the transmit buffer mutex.                         */
         BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferMutex, BTPS_INFINITE_WAIT);

         /* Check if the buffer has been allocated or if there is not   */
         /* enough room in the buffer to add the data.                  */
         if((TransmitThreadInfo.TransmitBuffer.Buffer) || (TransmitThreadInfo.TransmitBuffer.NumFree < NumberBytes))
         {
            /* Check if the buffer has already been allocated.          */
            if(!TransmitThreadInfo.TransmitBuffer.Buffer)
            {
               /* The buffer has not been allocated, check if this is a */
               /* loopback event.                                       */
               if(Flags & TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK)
               {
                  /* This is a loopback event, allocate the default     */
                  /* transmit buffer size.                              */
                  AllocationSize = DEFAULT_TRANSMIT_BUFFER_SIZE;
               }
               else
               {
                  /* This is not a loopback event, allocate the number  */
                  /* of bytes requested by the caller.                  */
                  AllocationSize = NumberBytes;
               }

               /* Allocate memory for the buffer.                       */
               TransmitThreadInfo.TransmitBuffer.Buffer = BTPS_AllocateMemory(AllocationSize);

               /* Check if the allocation succeeded.                    */
               if(TransmitThreadInfo.TransmitBuffer.Buffer)
               {
                  /* The allocation succeeded, update the buffer's      */
                  /* state.                                             */
                  TransmitThreadInfo.TransmitBuffer.BufferSize = AllocationSize;
                  TransmitThreadInfo.TransmitBuffer.NumFree    = AllocationSize;
               }
            }
            else
            {
               /* The buffer already exists, wait for the transmit      */
               /* buffer reallocation mutex.                            */
               BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferReallocMutex, BTPS_INFINITE_WAIT);

               /* The transmit buffer already exists, let's increase the*/
               /* size of it by the number of bytes needed, even if that*/
               /* is more data than we actually need.  Allocating more  */
               /* than we need like this may help to reduce the number  */
               /* of calls to realloc() if this case is hit while in    */
               /* loopback mode.                                        */
               TransmitThreadInfo.TransmitBuffer.Buffer = realloc(TransmitThreadInfo.TransmitBuffer.Buffer, TransmitThreadInfo.TransmitBuffer.BufferSize + NumberBytes);

               /* Check if the reallocation succeeded.                  */
               if(TransmitThreadInfo.TransmitBuffer.Buffer)
               {
                  /* The reallocation succeeded, check if there is data */
                  /* in the buffer that needs to be shifted to the new  */
                  /* end.                                               */
                  if((TransmitThreadInfo.TransmitBuffer.NumInUse) && (TransmitThreadInfo.TransmitBuffer.OutIndex >= TransmitThreadInfo.TransmitBuffer.InIndex))
                  {
                     /* Shift the data at end of the old buffer to the  */
                     /* end of the new buffer to make room for the new  */
                     /* data.                                           */

                     /* Initialize the number of remaining bytes.       */
                     NumRemaining      = TransmitThreadInfo.TransmitBuffer.BufferSize - TransmitThreadInfo.TransmitBuffer.OutIndex;

                     /* Save the pointers to previous buffer end and the*/
                     /* new buffer end.                                 */
                     PreviousBufferEnd = &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.BufferSize];
                     NewBufferEnd      = &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.BufferSize + NumberBytes];

                     /* Loop until we have shift all of the bytes to the*/
                     /* end of the buffer.  We could write a loop to    */
                     /* copy in reverse order but that could be very    */
                     /* slow for large buffer sizes, attempt to copy    */
                     /* moving forward copying as many bytes as possible*/
                     /* (we might not be able copy them all because of  */
                     /* the possibility of overwriting data in the case */
                     /* that the number of bytes added is less than     */
                     /* NumRemaining).                                  */
                     while(NumRemaining)
                     {
                        NumToCopy          = MIN(NumRemaining, NumberBytes);
                        PreviousBufferEnd -= NumToCopy;
                        NewBufferEnd      -= NumToCopy;

                        BTPS_MemCopy(NewBufferEnd, PreviousBufferEnd, NumToCopy);

                        NumRemaining      -= NumToCopy;
                     }

                     /* Increment the out index to its new value.       */
                     TransmitThreadInfo.TransmitBuffer.OutIndex += NumberBytes;

                     /* Check if we need to wrap the out index back to  */
                     /* start of the buffer.                            */
                     if(TransmitThreadInfo.TransmitBuffer.OutIndex >= TransmitThreadInfo.TransmitBuffer.BufferSize)
                        TransmitThreadInfo.TransmitBuffer.OutIndex = 0;
                  }

                  /* Update the buffer's state.                         */
                  TransmitThreadInfo.TransmitBuffer.BufferSize += NumberBytes;
                  TransmitThreadInfo.TransmitBuffer.NumFree    += NumberBytes;
               }

               /* We are finished reallocating the buffer, release the  */
               /* reallocate buffer mutex.                              */
               BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferReallocMutex);
            }
         }

         /* Check if we were able to allocate a transmit buffer and if  */
         /* there is enough room in the buffer to add data.             */
         if((TransmitThreadInfo.TransmitBuffer.Buffer) && (TransmitThreadInfo.TransmitBuffer.NumFree >= NumberBytes))
         {
            /* Determine the number of continuous bytes that we can add.*/
            if(TransmitThreadInfo.TransmitBuffer.InIndex >= TransmitThreadInfo.TransmitBuffer.OutIndex)
               NumContinuous = TransmitThreadInfo.TransmitBuffer.BufferSize - TransmitThreadInfo.TransmitBuffer.InIndex;
            else
               NumContinuous = TransmitThreadInfo.TransmitBuffer.OutIndex - TransmitThreadInfo.TransmitBuffer.InIndex;

            /* Set the number of continuous to the minimum of the       */
            /* maximum possible continuous found just above, and the    */
            /* number of bytes requested to add by the caller.          */
            NumContinuous = MIN(NumContinuous, NumberBytes);

            /* Initialize the number of remaining bytes to add.         */
            NumRemaining  = NumberBytes;

            /* Check if this is a request to add loopback data.         */
            if(Flags & TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK)
            {
               /* This is a call to add loopback data, first release the*/
               /* transmit buffer mutex before we call in to            */
               /* SPPM_ReadData() so that the transmit thread can       */
               /* continue processing.                                  */
               BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferMutex);

               /* Attempt to read the data in to the transmit buffer.   */
               Result = SPPM_ReadData(TransmitThreadInfo.PortHandle, 0, (unsigned int)NumContinuous, &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.InIndex]);

               /* Wait for the transmit buffer mutex.                   */
               BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferMutex, BTPS_INFINITE_WAIT);

               /* Check if successfully read all of the data.           */
               if(Result == ((int)NumContinuous))
               {
                  /* We successfully read the data, check if display raw*/
                  /* data mode is active.                               */
                  if(DisplayRawDataActive)
                  {
                     /* Display raw data mode is active, display the    */
                     /* data in the terminal.                           */
                     DisplayRawData((unsigned int)NumContinuous, &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.InIndex], &ReceiveThreadInfo.DisplayRawDataPosition);
                  }

                  /* Increment the in index.                            */
                  TransmitThreadInfo.TransmitBuffer.InIndex += NumContinuous;

                  /* Check if the in index needs to be wrapped back to  */
                  /* the start of the buffer.                           */
                  if(TransmitThreadInfo.TransmitBuffer.InIndex >= TransmitThreadInfo.TransmitBuffer.BufferSize)
                     TransmitThreadInfo.TransmitBuffer.InIndex = 0;

                  /* Decrement the number of remaining bytes.           */
                  NumRemaining -= NumContinuous;

                  /* Check if there is any data remaining to read.      */
                  if(!NumRemaining)
                  {
                     /* There is no data remaining, flag success to the */
                     /* caller.                                         */
                     ret_val = TRUE;
                  }
                  else
                  {
                     /* Release the transmit buffer mutex before we call*/
                     /* in to SPPM_ReadData() so that the transmit      */
                     /* thread can continue processing.                 */
                     BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferMutex);

                     /* Attempt to read the data in to the transmit     */
                     /* buffer.                                         */
                     Result = SPPM_ReadData(TransmitThreadInfo.PortHandle, 0, (unsigned int)NumRemaining, &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.InIndex]);

                     /* Wait for the transmit buffer mutex.             */
                     BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferMutex, BTPS_INFINITE_WAIT);

                     /* Check if successfully read all of the data.     */
                     if(Result == ((int)NumRemaining))
                     {
                        /* We successfully read the data, check if      */
                        /* display raw data mode is active.             */
                        if(DisplayRawDataActive)
                        {
                           /* Display raw data mode is active, display  */
                           /* the data in the terminal.                 */
                           DisplayRawData((unsigned int)NumRemaining, &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.InIndex], &ReceiveThreadInfo.DisplayRawDataPosition);
                        }

                        /* Increment the in index.                      */
                        TransmitThreadInfo.TransmitBuffer.InIndex += NumRemaining;

                        /* Check if the in index needs to be wrapped    */
                        /* back to the start of the buffer.             */
                        if(TransmitThreadInfo.TransmitBuffer.InIndex >= TransmitThreadInfo.TransmitBuffer.BufferSize)
                           TransmitThreadInfo.TransmitBuffer.InIndex = 0;

                        /* Flag success to the caller.                  */
                        ret_val = TRUE;
                     }
                     else
                     {
                        printf("SPPM_ReadData(%lu) Failure: %d, %s.\r\n", (unsigned long)NumRemaining, Result, ERR_ConvertErrorCodeToString(Result));

                        ret_val = FALSE;
                     }
                  }
               }
               else
               {
                  printf("SPPM_ReadData(%lu) Failure: %d, %s.\r\n", (unsigned long)NumContinuous, Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FALSE;
               }
            }
            else
            {
               /* This is not a request to loopback data so it must be  */
               /* an command to send data, create an array 256 bytes    */
               /* where each byte contains a different value.  We will  */
               /* use this array to copy data into the transmit buffer. */
               for(Index=0;Index<sizeof(DataArray);Index++)
                  DataArray[Index] = (Byte_t)Index;

               /* Loop through add the data to the buffer.              */
               while(NumContinuous)
               {
                  NumToCopy = MIN(NumContinuous, sizeof(DataArray));

                  BTPS_MemCopy(&TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.InIndex], DataArray, NumToCopy);

                  NumContinuous                             -= NumToCopy;
                  NumRemaining                              -= NumToCopy;
                  TransmitThreadInfo.TransmitBuffer.InIndex += NumToCopy;
               }

               /* Check if the in index needs to be wrapped back to the */
               /* start of the buffer.                                  */
               if(TransmitThreadInfo.TransmitBuffer.InIndex >= TransmitThreadInfo.TransmitBuffer.BufferSize)
                  TransmitThreadInfo.TransmitBuffer.InIndex = 0;

               /* Loop until we have added the number of remaining      */
               /* bytes.                                                */
               while(NumRemaining)
               {
                  NumToCopy = MIN(NumRemaining, sizeof(DataArray));

                  BTPS_MemCopy(&TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.InIndex], DataArray, NumToCopy);

                  NumRemaining                              -= NumToCopy;
                  TransmitThreadInfo.TransmitBuffer.InIndex += NumToCopy;
               }

               /* Check if the in index needs to be wrapped back to the */
               /* start of the buffer.                                  */
               if(TransmitThreadInfo.TransmitBuffer.InIndex >= TransmitThreadInfo.TransmitBuffer.BufferSize)
                  TransmitThreadInfo.TransmitBuffer.InIndex = 0;

               /* Increment the number of bytes added to the transmit   */
               /* buffer by the WriteData command.                      */
               TransmitThreadInfo.WriteDataCommandSize += NumberBytes;

               /* Flag success to the caller.                           */
               ret_val = TRUE;
            }

            /* Check if any errors occurred.                            */
            if(ret_val)
            {
               /* No errors occurred, updated the state of the transmit */
               /* buffer.                                               */
               TransmitThreadInfo.TransmitBuffer.NumFree  -= NumberBytes;
               TransmitThreadInfo.TransmitBuffer.NumInUse += NumberBytes;

               /* Add the flags to transmit thread's flags.             */
               TransmitThreadInfo.Flags                   |= Flags;

               /* Check if the transmit thread is waiting for data.     */
               if(TransmitThreadInfo.WaitingForData)
               {
                  /* The transmit thread is waiting for data, set the   */
                  /* event that data is available so that the thread    */
                  /* continues processing.                              */
                  BTPS_SetEvent(TransmitThreadInfo.DataAvailableEvent);
               }
            }
         }
         else
         {
            printf("Error: Could not allocated memory for the transmit buffer.\r\n");

            ret_val = FALSE;
         }

         /* Release the transmit buffer mutex.                          */
         BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferMutex);
      }
      else
      {
         printf("Error: Could not initialize transmit thread.\r\n");

         ret_val = FALSE;
      }
   }
   else
   {
      /* The caller passed in invalid data, determine which parameter is*/
      /* invalid.                                                       */
      if(!NumberBytes)
         printf("Error: Number of bytes to add to transmit buffer is 0.\r\n");
      else
      {
         if((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.PortHandle == PortHandle))
         {
            printf("Error: Could not add data to transmit thread's buffer because thread is transmitting on a different port.\r\n");
         }
      }

      ret_val = FALSE;
   }

   return(ret_val);
}

   /* The following thread is responsible for sending SPP data.         */
static void *TransmitThread(void *ThreadParameter)
{
   DWord_t   NumContinuous;
   int       Result;
   Boolean_t Error;

   printf("Transmit thread started.\r\n");

   Error = FALSE;

   /* Loop while the thread is initialized and no errors have occurred. */
   while((TransmitThreadInfo.Initialized) && (!Error))
   {
      /* Flag that we are waiting for data.                             */
      TransmitThreadInfo.WaitingForData = TRUE;

      /* Wait for the data available event.                             */
      BTPS_WaitEvent(TransmitThreadInfo.DataAvailableEvent, BTPS_INFINITE_WAIT);

      /* Flag that we are no longer waiting for data.                   */
      TransmitThreadInfo.WaitingForData = FALSE;

      /* Reset the data available event.                                */
      BTPS_ResetEvent(TransmitThreadInfo.DataAvailableEvent);

      /* Loop while the transmit thread is initialized, there is data   */
      /* available to send, and no errors have occurred.                */
      while((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.TransmitBuffer.NumInUse > 0) && (!Error))
      {
         /* Wait for the transmit buffer mutex.                         */
         BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferMutex, BTPS_INFINITE_WAIT);

         /* Determine the number of continuous bytes that we can read   */
         /* from the buffer.                                            */
         if(TransmitThreadInfo.TransmitBuffer.OutIndex >= TransmitThreadInfo.TransmitBuffer.InIndex)
            NumContinuous = TransmitThreadInfo.TransmitBuffer.BufferSize - TransmitThreadInfo.TransmitBuffer.OutIndex;
         else
            NumContinuous = TransmitThreadInfo.TransmitBuffer.InIndex - TransmitThreadInfo.TransmitBuffer.OutIndex;

         /* Set the number of continuous bytes to the minimum of the    */
         /* number of continuous bytes found above and the maximum      */
         /* transmit size.                                              */
         NumContinuous = MIN(NumContinuous, MAX_TRANSMIT_THREAD_TRANSMIT_SIZE);

         /* Release the transmit buffer mutex while we are sending data */
         /* so that data can still be added to the buffer while we are  */
         /* sending data.                                               */
         BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferMutex);

         /* Wait for the buffer reallocation mutex.  The buffer cannot  */
         /* be reallocated while we are sending data because the buffer */
         /* could get move and the data in the buffer may get shifted   */
         /* during reallocation.                                        */
         BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferReallocMutex, BTPS_INFINITE_WAIT);

         /* Write the data.                                             */
         Result = SPPM_WriteData(TransmitThreadInfo.PortHandle, SPPM_WRITE_DATA_WRITE_TIMEOUT_INFINITE, (unsigned int)NumContinuous, &TransmitThreadInfo.TransmitBuffer.Buffer[TransmitThreadInfo.TransmitBuffer.OutIndex]);

         /* Release the buffer reallocation mutex.                      */
         BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferReallocMutex);

         /* Check if all bytes were successfully written.               */
         if(Result == ((int)NumContinuous))
         {
            /* The data was successfully sent, wait for the transmit    */
            /* buffer mutex.                                            */
            BTPS_WaitMutex(TransmitThreadInfo.TransmitBufferMutex, BTPS_INFINITE_WAIT);

            /* Update the transmit buffer's state.                      */
            TransmitThreadInfo.TransmitBuffer.NumFree  += NumContinuous;
            TransmitThreadInfo.TransmitBuffer.NumInUse -= NumContinuous;
            TransmitThreadInfo.TransmitBuffer.OutIndex += NumContinuous;

            /* Check if the transmit buffer's out index needs to be     */
            /* wrapped back to the start.                               */
            if(TransmitThreadInfo.TransmitBuffer.OutIndex >= TransmitThreadInfo.TransmitBuffer.BufferSize)
               TransmitThreadInfo.TransmitBuffer.OutIndex = 0;

            /* Release the transmit buffer's mutex.                     */
            BTPS_ReleaseMutex(TransmitThreadInfo.TransmitBufferMutex);

            /* Check if this the current transmission was initiated by a*/
            /* WriteData command.                                       */
            if((TransmitThreadInfo.Flags & TRANSMIT_THREAD_INFO_FLAGS_WRITE_DATA_COMMAND) && (!(TransmitThreadInfo.Flags & TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK)))
            {
               /* This transmission was initiated by the WriteData      */
               /* command, update the WriteData command's variables.    */
               TransmitThreadInfo.WriteDataCommandNumSent += NumContinuous;
               TransmitThreadInfo.WriteDataCommandStatus  += NumContinuous;

               /* Check if we've sent a megabyte or more of data.       */
               if(TransmitThreadInfo.WriteDataCommandStatus >= (1000 * 1000))
               {
                  /* We've sent a megabyte or more of data, display the */
                  /* number of megabytes we've sent so far in the       */
                  /* terminal.                                          */
                  printf("Status: Sent %lu.%03lu MB of %lu.%03lu MB.\r\n",
                     (unsigned long)(TransmitThreadInfo.WriteDataCommandNumSent / (1000 * 1000)),
                     (unsigned long)((TransmitThreadInfo.WriteDataCommandNumSent % (1000 * 1000)) / 1000),
                     (unsigned long)(TransmitThreadInfo.WriteDataCommandSize / (1000 * 1000)),
                     (unsigned long)((TransmitThreadInfo.WriteDataCommandSize % (1000 * 1000)) / 1000));

                  TransmitThreadInfo.WriteDataCommandStatus = 0;
               }
            }
         }
         else
         {
            /* Error writing the data, inform the user and flag an      */
            /* error.                                                   */
            printf("SPPM_WriteData Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

            Error = TRUE;
         }
      }

      /* Determine if we should stop processing.                        */
      if((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.Flags & TRANSMIT_THREAD_INFO_FLAGS_WRITE_DATA_COMMAND) && (!(TransmitThreadInfo.Flags & TRANSMIT_THREAD_INFO_FLAGS_LOOPBACK)) && (!Error))
      {
         printf("Send complete.\r\n");

         break;
      }
   }

   /* Set the event to signal that this thread is shutting down.        */
   BTPS_SetEvent(TransmitThreadInfo.ThreadShutDownEvent);

   /* Cleanup the thread if is still initialized.                       */
   if(TransmitThreadInfo.Initialized)
      CleanupTransmitThread();

   printf("Transmit thread stopped.\r\n");

   return(NULL);
}

   /* The following function is the Callback function that is installed */
   /* to be notified when any local IPC connection to the server has    */
   /* been lost.  This case only occurs when the Server exits.  This    */
   /* callback allows the application mechanism to be notified so that  */
   /* all resources can be cleaned up (i.e.  call BTPM_Cleanup().       */
static void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter)
{
   printf("Server has been Un-Registered.\r\n");

   printf("SPPM>");

   /* Cleanup the receive thread.                                       */
   if(ReceiveThreadInfo.Initialized)
      CleanupReceiveThread();

   /* Cleanup the transmit thread.                                      */
   if(TransmitThreadInfo.Initialized)
      CleanupTransmitThread();

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

   printf("SPPM>");

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
            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.MITM_Protection_Required = MITMProtection;
            AuthenticationResponseInformation.AuthenticationData.IOCapabilities.OOB_Data_Present         = OOBSupport;

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

   printf("SPPM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is the serial port profile event callback  */
   /* function that is registered with the Serial Port Profile Manager. */
   /* This callback is responsible for processing all Serial Port       */
   /* Profile Manager (SPPM) events.                                    */
static void BTPSAPI SPPM_Event_Callback(SPPM_Event_Data_t *EventData, void *CallbackParameter)
{
   char Buffer[128];

   if(EventData)
   {
      if(EventData->EventType != setDataReceived)
         printf("\r\n");

      switch(EventData->EventType)
      {
         case setServerPortOpenRequest:
            printf("Server Port Open Request. \r\n");

            BD_ADDRToStr(EventData->EventData.ServerPortOpenRequestEventData.RemoteDeviceAddress, Buffer);

            printf("    Port Handle:    %u\r\n", EventData->EventData.ServerPortOpenRequestEventData.PortHandle);
            printf("    Remote Address: %s\r\n", Buffer);

            printf("\r\nRespond with the command: OpenServerPortRequestResponse\r\n");
            break;
         case setServerPortOpen:
            printf("Server Port Open.\r\n");

            BD_ADDRToStr(EventData->EventData.ServerPortOpenEventData.RemoteDeviceAddress, Buffer);

            printf("    Port Handle:    %u\r\n", EventData->EventData.ServerPortOpenEventData.PortHandle);
            printf("    Remote Address: %s\r\n", Buffer);
            break;
         case setPortClose:
            printf("Port Closed. \r\n");

            printf("    Port Handle: %u\r\n", EventData->EventData.PortCloseEventData.PortHandle);

            /* Check if the receive thread is initialized and if this is*/
            /* the port the thread is using.                            */
            if((ReceiveThreadInfo.Initialized) && (ReceiveThreadInfo.PortHandle == EventData->EventData.PortCloseEventData.PortHandle))
            {
               /* The receive thread is initialized and this is the port*/
               /* the thread is using, cleanup and stop the thread.     */
               CleanupReceiveThread();
            }

            /* Check if the transmit thread is initialized and if this  */
            /* is the port the thread is using.                         */
            if((TransmitThreadInfo.Initialized) && (TransmitThreadInfo.PortHandle == EventData->EventData.PortCloseEventData.PortHandle))
            {
               /* The transmit thread is initialized and this is the    */
               /* port the thread is using, cleanup and stop the thread.*/
               CleanupTransmitThread();
            }

            break;
         case setRemotePortOpenStatus:
            printf("Remote Port Open Status.\r\n");

            printf("    Port Handle: %u\r\n", EventData->EventData.RemotePortOpenStatusEventData.PortHandle);

            switch(EventData->EventData.RemotePortOpenStatusEventData.Status)
            {
               case SPPM_OPEN_REMOTE_PORT_STATUS_SUCCESS:
                  sprintf(Buffer, "Remote Port Open Successful");
                  break;
               case SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_TIMEOUT:
                  sprintf(Buffer, "Remote Port Open Failure: Timeout");
                  break;
               case SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_REFUSED:
                  sprintf(Buffer, "Remote Port Open Failure: Refused");
                  break;
               case SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_SECURITY:
                  sprintf(Buffer, "Remote Port Open Failure: Security");
                  break;
               case SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_DEVICE_POWER_OFF:
                  sprintf(Buffer, "Remote Port Open Failure: Power Off");
                  break;
               case SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_UNKNOWN:
                  sprintf(Buffer, "Remote Port Open Failure: Unknown");
                  break;
               default:
                  sprintf(Buffer, "Unknown Port Status: %d", EventData->EventData.RemotePortOpenStatusEventData.Status);
                  break;
            }

            printf("    Status:      %s\r\n", Buffer);
            break;
         case setLineStatusChanged:
            printf("Line Status Changed.\r\n");

            printf("    Port Handle:    %u\r\n", EventData->EventData.LineStatusChangedEventData.PortHandle);
            printf("    LineStatusMask: 0x%08lX\r\n", EventData->EventData.LineStatusChangedEventData.LineStatusMask);
            break;
         case setPortStatusChanged:
            printf("Port Status Changed.\r\n");

            printf("    Port Handle: %u\r\n", EventData->EventData.PortStatusChangedEventData.PortHandle);
            printf("    Port Status:\r\n");
            printf("       Mask:         0x%08lX\r\n", EventData->EventData.PortStatusChangedEventData.PortStatus.PortStatusMask);
            printf("       BreakSignal:  %u\r\n", EventData->EventData.PortStatusChangedEventData.PortStatus.BreakSignal);
            printf("       BreakTimeout: %u\r\n", EventData->EventData.PortStatusChangedEventData.PortStatus.BreakTimeout);
            break;
         case setDataReceived:
            /* Check if loopback mode and automatic read mode are both  */
            /* disabled.                                                */
            if((!LoopbackActive) && (!AutomaticReadActive))
            {
               /* Loopback mode and automatic read mode are both        */
               /* disabled, display the data indication in the terminal.*/
               printf("\r\n");
               printf("Data Received.\r\n");
               printf("    Port Handle: %u\r\n", EventData->EventData.DataReceivedEventData.PortHandle);
               printf("    Data Length: %u\r\n", EventData->EventData.DataReceivedEventData.DataLength);
               printf("SPPM>");
            }

            /* Check if either of loopback mode or automatic read mode  */
            /* is active.                                               */
            if((LoopbackActive) || (AutomaticReadActive))
            {
               /* Either loopback mode or automatic read mode is active,*/
               /* notify the receive thread that data is available.     */
               AddReceiveThreadData(EventData->EventData.DataReceivedEventData.PortHandle, EventData->EventData.DataReceivedEventData.DataLength);
            }

            break;
         case setTransmitBufferEmpty:
            printf("Transmit Buffer Empty.\r\n");

            printf("    Port Handle: %u\r\n", EventData->EventData.TransmitBufferEmptyEventData.PortHandle);
            break;
         default:
            printf("SPPM: Unknown SPP Manager Event Received: 0x%08X, Length: 0x%08X.\r\n", (unsigned int)EventData->EventType, EventData->EventLength);
            break;
      }

      if(EventData->EventType != setDataReceived)
         printf("SPPM>");
   }
   else
      printf("\r\nSPPM Event Data is NULL.\r\n");

   fflush(stdout);
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   /* Initialize the default Secure Simple Pairing parameters.          */
   IOCapability   = DEFAULT_IO_CAPABILITY;
   OOBSupport     = FALSE;
   MITMProtection = DEFAULT_MITM_PROTECTION;

   /* Nothing really to do here aside from running the main application */
   /* code.                                                             */
   UserInterface();

   /* Cleanup the receive thread.                                       */
   if(ReceiveThreadInfo.Initialized)
      CleanupReceiveThread();

   /* Cleanup the transmit thread.                                      */
   if(TransmitThreadInfo.Initialized)
      CleanupTransmitThread();

   return(0);
}

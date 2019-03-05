/*****< linuxhci.c >***********************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXHCI - Simple Linux application using the Bluetooth Host Controller   */
/*             Interface.                                                     */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/27/01  R. Sledge      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "LinuxHCI.h"      /* Main Application Prototypes and Constants.      */

#include "SS1BTPS.h"       /* Includes the SS1 Bluetooth Protocol Stack.      */
#include "SS1BTDBG.h"      /* Includes/Constants for Bluetooth Debugging.     */

#define NUM_EXPECTED_PARAMETERS_USB                 (2)  /* Denotes the number*/
                                                         /* of command line   */
                                                         /* parameters        */
                                                         /* accepted at Run   */
                                                         /* Time when running */
                                                         /* in USB Mode.      */

#define NUM_EXPECTED_PARAMETERS_UART                (4)  /* Denotes the       */
                                                         /* number of command */
                                                         /* line parameters   */
                                                         /* accepted at Run   */
                                                         /* Time when running */
                                                         /* in UART Mode.     */

#define USB_PARAMETER_VALUE                         (0)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* USB.              */

#define UART_PARAMETER_VALUE                        (1)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* UART.             */

#define BCSP_PARAMETER_VALUE                        (2)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* BCSP.             */

#define MAX_SUPPORTED_COMMANDS                     (32)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_NUM_OF_PARAMETERS                       (3)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_INQUIRY_RESULTS                        (32)  /* Denotes the max   */
                                                         /* number of inquiry */
                                                         /* results.          */

#define ACL_DATA_TEST_MESSAGE         ("Test Message.")  /* Denotes the test  */
                                                         /* data sent by a    */
                                                         /* Call to the       */
                                                         /* SendACLData()     */
                                                         /* Function.         */

#define ACL_DATA_TEST_MESSAGE_LENGTH               (14)  /* Denotes the Length*/
                                                         /* of the ACL Test   */
                                                         /* Message including */
                                                         /* the NULL          */
                                                         /* terminator.       */

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

#define UNABLE_TO_INITIALIZE_STACK                 (-7)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* while Initializing*/
                                                         /* the Bluetooth     */
                                                         /* Protocol Stack.   */

#define INVALID_STACK_ID_ERROR                     (-8)  /* Denotes that an   */
                                                         /* occurred due to   */
                                                         /* attempted         */
                                                         /* execution of a    */
                                                         /* Command when a    */
                                                         /* Bluetooth Protocol*/
                                                         /* Stack has not been*/
                                                         /* opened.           */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME  "LinuxHCI_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME  "LinuxHCI_FTS.log"

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
static unsigned int       BluetoothStackID;         /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static unsigned int       DebugID;                  /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static Word_t             ACLConnectionHandle;      /* Variable which holds the        */
                                                    /* current ACL Connection Handle   */
                                                    /* of the current ACL Connection.  */

static Word_t             SCOConnectionHandle;      /* Variable which holds the        */
                                                    /* current SCO Connection Handle   */
                                                    /* of the current SCO Connection.  */

static int                ConnectionRequestIndexID; /* Variable which holds the Index  */
                                                    /* (into the InquiryResultList[]   */
                                                    /* array) of the Bluetooth Device  */
                                                    /* that is attempting to connect   */
                                                    /* to the Local Device.  This      */
                                                    /* value is used by the            */
                                                    /* AcceptConnection() and the      */
                                                    /* RejectConnection() functions.   */

static BD_ADDR_t          InquiryResultList[MAX_INQUIRY_RESULTS];  /*Variable which    */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned int       NumberofValidResponses;   /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static unsigned int       NumberCommands;           /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t     CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is   */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static char *HCIVersionStrings[] =
{
   "1.0b",
   "1.1",
   "1.2",
   "2.0",
   "2.1",
   "3.0",
   "4.0",
   "4.1",
   "4.2",
   "5.0",
   "Unknown (greater 5.0)"
} ;

#define NUM_SUPPORTED_HCI_VERSIONS     (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

   /* Internal function prototypes.                                     */
static void UserInterface(void);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int  CommandParser(UserCommand_t *TempCommand, char *UserInput);
static int  CommandInterpreter(UserCommand_t *TempCommand);
static int  AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation);
static int CloseStack(void);

static int DisplayHelp(ParameterList_t *TempParam);
static int EnableDebug(ParameterList_t *TempParam);
static int Reset(ParameterList_t *TempParam);
static int Version(ParameterList_t *TempParam);
static int Inquiry(ParameterList_t *TempParam);
static int GetBoardAddress(ParameterList_t *TempParam);
static int ScanMode(ParameterList_t *TempParam);
static int AutoAccept(ParameterList_t *TempParam);
static int AcceptConnection(ParameterList_t *TempParam);
static int RejectConnection(ParameterList_t *TempParam);
static int ConnectACL(ParameterList_t *TempParam);
static int SendACLData(ParameterList_t *TempParam);
static int DisconnectACL(ParameterList_t *TempParam);
static int AddSCOConnection(ParameterList_t *TempParam);
static int DisconnectSCO(ParameterList_t *TempParam);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter);

static void BTPSAPI HCI_ACL_Data_Callback(unsigned int BluetoothStackID, Word_t Connection_Handle, Word_t Flags, Word_t ACLDataLength, Byte_t *ACLData, unsigned long CallbackParameter);

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

   AddCommand("RESET", Reset);
   AddCommand("VERSION", Version);
   AddCommand("INQUIRY", Inquiry);
   AddCommand("GETBD_ADDR", GetBoardAddress);
   AddCommand("SETSCANMODE", ScanMode);
   AddCommand("SETAUTOACCEPT", AutoAccept);
   AddCommand("ACCEPT", AcceptConnection);
   AddCommand("REJECT", RejectConnection);
   AddCommand("CONNECTACL", ConnectACL);
   AddCommand("SENDACLDATA", SendACLData);
   AddCommand("DISCONNECTACL", DisconnectACL);
   AddCommand("ADDSCO", AddSCOConnection);
   AddCommand("DISCONNECTSCO", DisconnectSCO);
   AddCommand("ENABLEDEBUG", EnableDebug);
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
      printf("HCI>");

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
      for(Index=0, ret_val=String;Index < strlen(String);Index++)
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

       /* Check to see if there is a Command                            */
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
      /* Now loop through each element in the table to see if there is  */
      /* a match.                                                       */
      for(Index=0,ret_val=NULL;((Index<NumberCommands) && (!ret_val));Index++)
      {
         if(memcmp(Command, CommandTable[Index].CommandName, strlen(CommandTable[Index].CommandName)) == 0)
            ret_val = CommandTable[Index].CommandFunction;
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

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation)
{
   int           Result;
   int           ret_val = 0;
   HCI_Version_t HCIVersion;

   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
      if(HCI_DriverInformation)
      {
         /* Initialize the Stack                                        */
         Result = BSC_Initialize(HCI_DriverInformation, BSC_INITIALIZE_FLAG_NO_L2CAP | BSC_INITIALIZE_FLAG_NO_SCO | BSC_INITIALIZE_FLAG_NO_SDP | BSC_INITIALIZE_FLAG_NO_RFCOMM | BSC_INITIALIZE_FLAG_NO_GAP | BSC_INITIALIZE_FLAG_NO_SPP | BSC_INITIALIZE_FLAG_NO_GOEP | BSC_INITIALIZE_FLAG_NO_OTP);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            if(HCI_DriverInformation->DriverType == hdtUSB)
               printf("Stack Initialization on USB Successful.\r\n");
            else
               printf("Stack Initialization on Port %d %ld (%s) Successful.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"));

            BluetoothStackID         = Result;

            if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion))
            {
               printf("Device Chipset Version: %s", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]);
            }

            /* Now that we have initialized the Bluetooth Stack ID, we  */
            /* need to initialize all other variables so that they      */
            /* flag that we have found no devices on Inquiry, no device */
            /* has attempted to connect to us, and there are currently  */
            /* no outstanding connections (ACL and/or SCO).             */
            NumberofValidResponses   = 0;

            ConnectionRequestIndexID = -1;

            DebugID                  = 0;

            ACLConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;
            SCOConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;

            /* Next, let's register for HCI Event and HCI ACL Data      */
            /* Callbacks.                                               */
            Result = HCI_Register_Event_Callback(BluetoothStackID, HCI_Event_Callback, 0);

            if(Result >= 0)
            {
               printf("Return Value is %d HCI_Register_Event_Callback() SUCCESS.\r\n", Result);

               Result = HCI_Register_ACL_Data_Callback(BluetoothStackID, HCI_ACL_Data_Callback, 0);

               if(Result >= 0)
                  printf("Return Value is %d HCI_Register_ACL_Data_Callback() SUCCESS.\r\n", Result);
               else
               {
                  printf("Return Value is %d HCI_Register_ACL_Data_Callback() FAILURE.\r\n", Result);

                  ret_val = UNABLE_TO_INITIALIZE_STACK;
               }
            }
            else
            {
               printf("Return Value is %d HCI_Register_Event_Callback() FAILURE.\r\n", Result);

               ret_val = UNABLE_TO_INITIALIZE_STACK;
            }
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            if(HCI_DriverInformation->DriverType == hdtUSB)
               printf("Stack Initialization on USB Failed: %d.\r\n", Result);
            else
            {
               if(HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber == -1)
               {
                  printf("Stack Initialization using device file %s %ld (%s) Failed: %d.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMDeviceName, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"), Result);
               }
               else
               {
                  printf("Stack Initialization on Port %d %ld (%s) Failed: %d.\r\n", HCI_DriverInformation->DriverInformation.COMMDriverInformation.COMPortNumber, HCI_DriverInformation->DriverInformation.COMMDriverInformation.BaudRate, ((HCI_DriverInformation->DriverInformation.COMMDriverInformation.Protocol == cpBCSP)?"BCSP":"UART"), Result);
               }
            }

            BluetoothStackID = 0;

            ret_val          = UNABLE_TO_INITIALIZE_STACK;
         }
      }
      else
      {
         /* One or more of the necessary parameters are invalid.        */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* A valid Stack ID already exists, inform to user.               */
      printf("Stack Already Initialized.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for closing the SS1         */
   /* Bluetooth Protocol Stack.  This function requires that the        */
   /* Bluetooth Protocol stack previously have been initialized via the */
   /* OpenStack() function.  This function returns zero on successful   */
   /* execution and a negative value on all errors.                     */
static int CloseStack(void)
{
   int ret_val;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* If debugging is enabled, go ahead and clean it up.             */
      if(DebugID)
         BTPS_Debug_Cleanup(BluetoothStackID, DebugID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      printf("Stack Shutdown Successfully.\r\n");

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID = 0;

      DebugID          = 0;

      /* Flag success to the caller.                                    */
      ret_val          = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
      printf("Stack not Initialized.\r\n");

      ret_val = UNABLE_TO_INITIALIZE_STACK;
   }

   return(ret_val);
}

   /* The following function is responsible for displaying the current  */
   /* Command Options for the Application.  The input parameter to this */
   /* function is completely ignored, and only needs to be passed in    */
   /* because all Commands that can be entered at the Prompt pass in the*/
   /* parsed information.  This function displays the current Command   */
   /* Options that are available and always returns zero.               */
static int DisplayHelp(ParameterList_t *TempParam)
{
   printf("******************************************************************\r\n");
   printf("* Command Options: Reset, Version, GetBD_ADDR, SetScanMode,      *\r\n");
   printf("*                  SetAutoAccept, Accept, Reject, Inquiry,       *\r\n");
   printf("*                  ConnectACL, SendACLData, DisconnectACL,       *\r\n");
   printf("*                  AddSCO, DisconnectSCO, EnableDebug, Help,     *\r\n");
   printf("*                  Quit.                                         *\r\n");
   printf("******************************************************************\r\n");

   return(0);
}

   /* The following function is responsible for enabling or disabling   */
   /* HCI Debugging.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int EnableDebug(ParameterList_t *TempParam)
{
   int                      ret_val;
   int                      Result;
   char                    *LogFileName;
   BTPS_Debug_Parameters_t  DebugParameters;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, go ahead and check to see if   */
      /* the user is enabling or disabling Debugging.                   */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= dtLogFile) && (TempParam->Params[0].intParam <= dtFTS))
      {
         /* Check to see if the user is requesting to enable or disable */
         /* debugging.                                                  */
         if(!TempParam->Params[0].intParam)
         {
            /* Disable, check to see if debugging is enabled.           */
            if(DebugID)
            {
               BTPS_Debug_Cleanup(BluetoothStackID, DebugID);

               printf("Debugging is now disabled.\r\n");

               /* Flag that debugging is no longer enabled.             */
               DebugID = 0;

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               printf("Debugging is not currently enabled.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
         }
         else
         {
            /* Enable, check to see if debugging is not already enabled.*/
            if(!DebugID)
            {
               /* Parameters appear to be semi-valid, AND the user is   */
               /* requesting to enable debugging.  Go ahead and see if  */
               /* any additional parameters were specified.             */
               switch(TempParam->Params[1].intParam)
               {
                  case dtLogFile:
                     if((TempParam->NumberofParameters > 2) && (TempParam->Params[2].strParam))
                        LogFileName = TempParam->Params[2].strParam;
                     else
                        LogFileName = DEFAULT_DEBUG_LOG_FILE_NAME;
                     break;
                  case dtFTS:
                     if((TempParam->NumberofParameters > 2) && (TempParam->Params[2].strParam))
                        LogFileName = TempParam->Params[2].strParam;
                     else
                        LogFileName = DEFAULT_DEBUG_FTS_FILE_NAME;
                     break;
                  default:
                     LogFileName = NULL;
                     break;
               }

               /* Verify that all specified parameters were correct.    */
               if((TempParam->Params[1].intParam == dtDebugTerminal) || ((TempParam->Params[1].intParam != dtDebugTerminal) && (LogFileName)  && (strlen(LogFileName))))
               {
                  DebugParameters.DebugType       = (BTPS_Debug_Type_t)TempParam->Params[1].intParam;
                  DebugParameters.DebugFlags      = 0;
                  DebugParameters.ParameterString = LogFileName;

                  if((Result = BTPS_Debug_Initialize(BluetoothStackID, &DebugParameters)) > 0)
                  {
                     DebugID = (unsigned int)Result;

                     printf("BTPS_Debug_Initialize() Success: %d.\r\n", Result);

                     if(TempParam->Params[1].intParam != dtDebugTerminal)
                        printf("   Log File Name: %s\r\n", LogFileName);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }
                  else
                  {
                     printf("BTPS_Debug_Initialize() Failure: %d.\r\n", Result);

                     /* Flag that an error occurred while submitting the*/
                     /* command.                                        */
                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Invalid parameters specified so flag an error to   */
                  /* the user.                                          */
                  printf("Usage: EnableDebug [Enable/Disable (Enable = 1, Disable = 0)] [DebugType (ASCII File = 0, Debug Console = 1, FTS File = 2)] [[Log File Name] (optional)].\r\n");

                  /* Flag that an error occurred while submitting the   */
                  /* command.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               printf("Debugging is already enabled.\r\n");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
         }
      }
      else
      {
         /* Invalid parameters specified so flag an error to the user.  */
         printf("Usage: EnableDebug [Enable/Disable (Enable = 1, Disable = 0)] [DebugType (ASCII File = 0, Debug Console = 1, FTS File = 2)] [[Log File Name] (optional)].\r\n");

         /* Flag that an error occurred while submitting the command.   */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an HCI_Reset()  */
   /* command Request on the Local Device.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int Reset(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the HCI_Reset()*/
      /* Command.                                                       */
      Result = HCI_Reset(BluetoothStackID, &StatusResult);

      if(!Result)
      {
         /* HCI_Reset() command was issued successfully, so display the */
         /* Device Result to the user.                                  */
         printf("HCI_Reset() Success: %d.\r\n", (int)StatusResult);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Error submitting the HCI_Reset() command so flag the error  */
         /* result to the user.                                         */
         printf("HCI_Reset() Failure: %d.\r\n", Result);

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Read_Local_Version_Information() command Request on the Local */
   /* Device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int Version(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;
   Byte_t HCI_VersionResult;
   Word_t HCI_RevisionResult;
   Byte_t LMP_VersionResult;
   Word_t Manufacturer_NameResult;
   Word_t LMP_SubversionResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Read_Local_Version_Information() Command.                  */
      Result = HCI_Read_Local_Version_Information(BluetoothStackID,
                                                  &StatusResult,
                                                  &HCI_VersionResult,
                                                  &HCI_RevisionResult,
                                                  &LMP_VersionResult,
                                                  &Manufacturer_NameResult,
                                                  &LMP_SubversionResult);

      if(!Result)
      {
         /* HCI_Read_Local_Version_Information() command was issued     */
         /* successfully, so display the Device Result to the user.     */
         printf("HCI_Read_Local_Version_Information() Success: %d\r\n", (int)StatusResult);

         /* If the Status was successful simply display the Version     */
         /* Information to the user.                                    */
         if(!StatusResult)
         {
            printf("     HCI Version       : 0x%02X\r\n", (int)HCI_VersionResult);

            printf("     HCI Revision      : 0x%04X\r\n", (int)HCI_RevisionResult);

            printf("     LMP Version       : 0x%02X\r\n", (int)LMP_VersionResult);

            printf("     Manufacturer Name : 0x%04X\r\n", (int)Manufacturer_NameResult);

            printf("     LMP SubVersion    : 0x%04X\r\n", (int)LMP_SubversionResult);
         }

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Error submitting the HCI_Read_Local_Version_Information()   */
         /* command so flag the error result to the user.               */
         printf("HCI_Read_Local_Version_Information() Failure: %d.\r\n", Result);

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an HCI_Inquiry()*/
   /* command Request on the Local Device.  This function returns zero  */
   /* if successful and a negative value if an error occurred.          */
static int Inquiry(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   LAP_t  LAP;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Inquiry() Command.                                         */
      HCI_ASSIGN_GIAC_LAP(LAP);

      Result = HCI_Inquiry(BluetoothStackID, LAP, 10, MAX_INQUIRY_RESULTS, &StatusResult);
      if(!Result)
      {
         /* HCI_Inquiry() command was issued successfully, so display   */
         /* the Device Result to the user.                              */
         printf("HCI_Inquiry() Success: %d.\r\n", (int)StatusResult);

         /* Flag that we have found NO Bluetooth Devices.               */
         NumberofValidResponses = 0;

         /* Flag success to the caller.                                 */
         ret_val                = 0;
      }
      else
      {
         /* Error submitting the HCI_Inquiry() command so flag the error*/
         /* result to the user.                                         */
         printf("HCI_Inquiry() Failure: %d.\r\n", Result);

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Read_BD_ADDR() command Request on the Local Device.  This     */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int GetBoardAddress(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   char      BoardStr[13];
   Byte_t    StatusResult;
   BD_ADDR_t BD_ADDR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Read_BD_ADDR() Command.                                    */
      Result = HCI_Read_BD_ADDR(BluetoothStackID, &StatusResult, &BD_ADDR);
      if(!Result)
      {
         /* HCI_Read_BD_ADDR() command was issued successfully, so      */
         /* display the Device Result to the user.                      */
         printf("HCI_Read_BD_ADDR() Success: %d\r\n", (int)StatusResult);

         /* If the Device returned success then we need to display the  */
         /* returned Board Address to the user.                         */
         if(!StatusResult)
         {
            BD_ADDRToStr(BD_ADDR, BoardStr);

            printf("Local Device Address is : 0x%s\r\n", BoardStr);
         }

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Error submitting the HCI_Read_BD_ADDR() command so flag the */
         /* error result to the user.                                   */
         printf("HCI_Read_BD_ADDR() Failure: %d.\r\n", Result);

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Write_Scan_Enable() command Request on the Local Device.  This*/
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int ScanMode(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Write_Scan_Enable() Command.                               */
      Result = HCI_Write_Scan_Enable(BluetoothStackID, HCI_SCAN_ENABLE_INQUIRY_SCAN_ENABLED_PAGE_SCAN_ENABLED, &StatusResult);
      if(!Result)
      {
         /* HCI_Write_Scan_Enable() command was issued successfully, so */
         /* display the Device Result to the user.                      */
         printf("HCI_Write_Scan_Enable() Success: %d.\r\n", (int)StatusResult);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Error submitting the HCI_Write_Scan_Enable() command so flag*/
         /* the error result to the user.                               */
         printf("HCI_Write_Scan_Enable() Failure: %d.\r\n", Result);

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Set_Event_Filter() command Request (to enable the Auto        */
   /* Acceptance of ALL Incoming Bluetooth Connections) on the Local    */
   /* Device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int AutoAccept(ParameterList_t *TempParam)
{
   int         ret_val;
   int         Result;
   Byte_t      StatusResult;
   Condition_t Condition;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Set_Event_Filter() Command.                                */
      Condition.Condition.Connection_Setup_Filter_Type_All_Devices_Condition.Auto_Accept_Flag = HCI_AUTO_ACCEPT_FLAG_DO_AUTO_ACCEPT;

      Result = HCI_Set_Event_Filter(BluetoothStackID, HCI_FILTER_TYPE_CONNECTION_SETUP, HCI_FILTER_CONDITION_TYPE_CONNECTION_SETUP_ALL_DEVICES, Condition, &StatusResult);
      if(!Result)
      {
         /* HCI_Set_Event_Filter() command was issued successfully, so  */
         /* display the Device Result to the user.                      */
         printf("HCI_Set_Event_Filter() Success: %d.\r\n", (int)StatusResult);

         /* Initialize that there is NO Device that is attempting to    */
         /* connect with us (negates the user of the Accept/Reject      */
         /* Connection functions).                                      */
         if(!StatusResult)
            ConnectionRequestIndexID = -1;

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Error submitting the HCI_Set_Event_Filter() command so flag */
         /* the error result to the user.                               */
         printf("HCI_Set_Event_Filter() Failure: %d.\r\n", Result);

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Accept_Connection_Request() command Request on the Local      */
   /* Device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int AcceptConnection(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Accept_Connection_Request() Command.                       */
      /* * NOTE * We will only allow this command to be issued when a   */
      /*          connection request has arrived (we check this by      */
      /*          looking at the ConnectionRequestIndexID value.        */
      if(ConnectionRequestIndexID != (-1))
      {
         /* A Connection Request is outstanding so go ahead an issue the*/
         /* HCI_Accept_Connection_Request() Command Request.            */
         Result = HCI_Accept_Connection_Request(BluetoothStackID, InquiryResultList[ConnectionRequestIndexID], HCI_ROLE_SWITCH_REMAIN_SLAVE, &StatusResult);
         if(!Result)
         {
            /* HCI_Accept_Connection_Request() command was issued       */
            /* successfully, so display the Device Result to the user.  */
            printf("HCI_Accept_Connection_Request() Success: %d.\r\n", (int)StatusResult);

            /* Flag that there is no longer a Connection Request        */
            /* outstanding.                                             */
            ConnectionRequestIndexID = -1;

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error submitting the HCI_Accept_Connection_Request()     */
            /* command so flag the error result to the user.            */
            printf("HCI_Accept_Connection_Request() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* No Connection Request Event has occurred so inform the user */
         /* why we are flagging an error.                               */
         printf("You can only Accept a Connection when a Connection Request Event has occurred.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Reject_Connection_Request() command Request on the Local      */
   /* Device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int RejectConnection(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so simply issue the            */
      /* HCI_Reject_Connection_Request() Command.                       */
      /* * NOTE * We will only allow this command to be issued when a   */
      /*          connection request has arrived (we check this by      */
      /*          looking at the ConnectionRequestIndexID value.        */
      if(ConnectionRequestIndexID != (-1))
      {
         /* A Connection Request is outstanding so go ahead an issue the*/
         /* HCI_Reject_Connection_Request() Command Request.            */
         Result = HCI_Reject_Connection_Request(BluetoothStackID, InquiryResultList[ConnectionRequestIndexID], HCI_ERROR_CODE_HOST_REJECTED_DUE_TO_SECURITY_REASONS, &StatusResult);
         if(!Result)
         {
            /* HCI_Reject_Connection_Request() command was issued       */
            /* successfully, so display the Device Result to the user.  */
            printf("HCI_Reject_Connection_Request() Success: %d.\r\n", (int)StatusResult);

            /* Flag that there is no longer a Connection Request        */
            /* outstanding.                                             */
            ConnectionRequestIndexID = -1;

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error submitting the HCI_Reject_Connection_Request()     */
            /* command so flag the error result to the user.            */
            printf("HCI_Reject_Connection_Request() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* No Connection Request Event has occurred so inform the user */
         /* why we are flagging an error.                               */
         printf("You can only Reject a Connection when a Connection Request Event has occurred.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Create_Connection() command Request on the Local Device.  This*/
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int ConnectACL(ParameterList_t *TempParam)
{
   int       ret_val;
   int       Result;
   Byte_t    StatusResult;
   BD_ADDR_t NullADDR;

   ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so now check to see if there   */
      /* already exists an ACL Connection.  If not we will attempt to   */
      /* create one.                                                    */
      if(ACLConnectionHandle == HCI_CONNECTION_HANDLE_INVALID_VALUE)
      {
         /* Next, let's determine if the user has specified a valid     */
         /* Bluetooth Device Address to connect with.                   */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            /* All the parameters appear to be valid, so let's go ahead */
            /* and finally issue the HCI_Create_Connection() Command    */
            /* Request.                                                 */
            Result = HCI_Create_Connection(BluetoothStackID, InquiryResultList[TempParam->Params[0].intParam - 1], (HCI_PACKET_ACL_TYPE_DM1 | HCI_PACKET_ACL_TYPE_DH1), 0, 0, 0, HCI_ROLE_SWITCH_LOCAL_MASTER_NO_ROLE_SWITCH, &StatusResult);
            if(!Result)
            {
               /* HCI_Create_Connection() command was issued            */
               /* successfully, so display the Device Result to the     */
               /* user.                                                 */
               printf("HCI_Create_Connection() Success: %d.\r\n", (int)StatusResult);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error submitting the HCI_Create_Connection() command  */
               /* so flag the error result to the user.                 */
               printf("HCI_Create_Connection() Failure: %d.\r\n", Result);

               /* Flag that an error occurred while submitting the      */
               /* command.                                              */
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Invalid parameters specified so flag an error to the     */
            /* user.                                                    */
            printf("Usage: ConnectACL [Inquiry Index].\r\n");

            /* Flag that an error occurred while submitting the command.*/
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* An ACL Connection already exists so inform the user of the  */
         /* error.                                                      */
         printf("Unable to create ACL Connection.\r\nACL Connection already exists.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing sending         */
   /* ACL Data over the ACL Connection on the Local Device.  This       */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int SendACLData(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t Buffer[ACL_DATA_TEST_MESSAGE_LENGTH+4];

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so now check to see if there   */
      /* exists an ACL Connection.  If not, we cannot send ACL Data.    */
      if(ACLConnectionHandle != HCI_CONNECTION_HANDLE_INVALID_VALUE)
      {
         /* Add the L2CAP Header - Note this is required by Ericsson    */
         /* Modules.                                                    */
         *((Word_t *)(&Buffer[0])) = (Word_t)(ACL_DATA_TEST_MESSAGE_LENGTH);
         Buffer[2] = 0x40;
         Buffer[3] = 0x00;
         memcpy((char *)&Buffer[4], ACL_DATA_TEST_MESSAGE, ACL_DATA_TEST_MESSAGE_LENGTH);

         /* Submit the request to send the data to the remote device.   */
         Result = HCI_Send_ACL_Data(BluetoothStackID, ACLConnectionHandle, HCI_ACL_FLAGS_PACKET_BOUNDARY_FIRST_PACKET, (ACL_DATA_TEST_MESSAGE_LENGTH+4), Buffer);
         if(Result >= 0)
         {
            /* HCI_Send_ACL_Data() command was issued successfully, so  */
            /* display the Device Result to the user.                   */
            printf("HCI_Send_ACL_Data() Success: %d.\r\n", (int)Result);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error submitting the HCI_Send_ACL_Data() command so flag */
            /* the error result to the user.                            */
            printf("HCI_Send_ACL_Data() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* An ACL Connection does not exist so inform the user of the  */
         /* error.                                                      */
         printf("Unable to send ACL Data.\r\nACL Connection does not exist.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Disconnect() command Request on the Local Device for a        */
   /* currently active ACL Connection.  This function returns zero if   */
   /* successful and a negative value if an error occurred.             */
static int DisconnectACL(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so now check to see if there   */
      /* already exists an ACL Connection.  If so we will attempt to    */
      /* disconnect it.                                                 */
      if(ACLConnectionHandle != HCI_CONNECTION_HANDLE_INVALID_VALUE)
      {
         /* A Connection exists, so let's go ahead and issue the        */
         /* HCI_Disconnect() Command Request.                           */
         Result = HCI_Disconnect(BluetoothStackID, ACLConnectionHandle, HCI_ERROR_CODE_OTHER_END_TERMINATED_CONNECTION_USER_ENDED, &StatusResult);
         if(!Result)
         {
            /* HCI_Disconnect() command was issued successfully, so     */
            /* display the Device Result to the user.                   */
            printf("HCI_Disconnect() Success: %d.\r\n", (int)StatusResult);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error submitting the HCI_Disconnect() command so flag the*/
            /* error result to the user.                                */
            printf("HCI_Disconnect() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* An ACL Connection does not exist so inform the user of the  */
         /* error.                                                      */
         printf("Unable to Disconnect ACL Connection.\r\nACL Connection does not exist.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Add_SCO_Connection() command Request on the Local Device.     */
   /* This function returns zero if successful and a negative value if  */
   /* an error occurred.                                                */
static int AddSCOConnection(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so now check to see if there   */
      /* already exists a SCO Connection.  If not we will attempt to    */
      /* create one.                                                    */
      if(SCOConnectionHandle == HCI_CONNECTION_HANDLE_INVALID_VALUE)
      {
         /* Next, we need to verify that an ACL Connection is present.  */
         /* A SCO Connection CANNOT be added without an ACL Connection  */
         /* being present.                                              */
         if(ACLConnectionHandle != HCI_CONNECTION_HANDLE_INVALID_VALUE)
         {
            /* No SCO Connection is present and there does exist an ACL */
            /* Connection so issue the HCI_Add_SCO_Connection() Command */
            /* Request.                                                 */
            Result = HCI_Add_SCO_Connection(BluetoothStackID, ACLConnectionHandle, HCI_PACKET_SCO_TYPE_HV1, &StatusResult);
            if(!Result)
            {
               /* HCI_Add_SCO_Connection() command was issued           */
               /* successfully, so display the Device Result to the     */
               /* user.                                                 */
               printf("HCI_Add_SCO_Connection() Success: %d.\r\n", (int)StatusResult);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Error submitting the HCI_Add_SCO_Connection() command */
               /* so flag the error result to the user.                 */
               printf("HCI_Add_SCO_Connection() Failure: %d.\r\n", Result);

               /* Flag that an error occurred while submitting the      */
               /* command.                                              */
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Flag an error to the user because there doesn't exist    */
            /* an ACL Connection.                                       */
            printf("Unable to create SCO Connection.\r\nAn ACL Connection MUST exist.\r\n");

            /* Flag that an error occurred.                             */
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* A SCO Connection already exists so inform the user of the   */
         /* error.                                                      */
         printf("Unable to create SCO Connection.\r\nA SCO Connection already exists.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing an              */
   /* HCI_Disconnect() command Request on the Local Device for a        */
   /* currently active SCO Connection.  This function returns zero if   */
   /* successful and a negative value if an error occurred.             */
static int DisconnectSCO(ParameterList_t *TempParam)
{
   int    ret_val;
   int    Result;
   Byte_t StatusResult;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Bluetooth Stack is initialized, so now check to see if there   */
      /* already exists an SCO Connection.  If so we will attempt to    */
      /* disconnect it.                                                 */
      if(SCOConnectionHandle != HCI_CONNECTION_HANDLE_INVALID_VALUE)
      {
         /* A Connection exists, so let's go ahead and issue the        */
         /* HCI_Disconnect() Command Request.                           */
         Result = HCI_Disconnect(BluetoothStackID, SCOConnectionHandle, HCI_ERROR_CODE_OTHER_END_TERMINATED_CONNECTION_USER_ENDED, &StatusResult);
         if(!Result)
         {
            /* HCI_Disconnect() command was issued successfully, so     */
            /* display the Device Result to the user.                   */
            printf("HCI_Disconnect() Success: %d.\r\n", (int)StatusResult);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Error submitting the HCI_Disconnect() command so flag the*/
            /* error result to the user.                                */
            printf("HCI_Disconnect() Failure: %d.\r\n", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* A SCO Connection does not exist so inform the user of the   */
         /* error.                                                      */
         printf("Unable to Disconnect SCO Connection.\r\nSCO Connection does not exist.\r\n");

         /* Flag that an error occurred.                                */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /*********************************************************************/
   /*                         Event Callbacks                           */
   /*********************************************************************/

   /* The following function is for the HCI Event Callback.  This       */
   /* function will be called whenever a Callback has been registered   */
   /* for the specified HCI Action that is associated with the Bluetooth*/
   /* Stack.  This function passes to the caller the HCI Event Data of  */
   /* the specified Event and the HCI Event Callback Parameter that was */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the HCI Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e. this function DOES NOT have be reentrant).  It Needs to be  */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because other HCI Events will*/
   /* not be processed while this function call is outstanding).        */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other HCI Events.  A  */
   /*          Deadlock WILL occur because NO HCI Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter)
{
   int    Index;
   int    Index1;
   char   BoardStr[13];
   Byte_t StatusResult;

   if((BluetoothStackID) && (HCI_Event_Data))
   {
      switch(HCI_Event_Data->Event_Data_Type)
      {
         case etInquiry_Result_Event:
            /* If the Inquiry List is NOT full and we have not seen the */
            /* Device that has been returned yet, add it to the Inquiry */
            /* Result List.                                             */
            printf("\r\n");

            printf("etInquiry_Result_Event Received.\r\n");

            for(Index=0;Index<HCI_Event_Data->Event_Data.HCI_Inquiry_Result_Event_Data->Num_Responses;Index++)
            {
               /* Go ahead and print out the Bluetooth Device Address.  */
               BD_ADDRToStr(HCI_Event_Data->Event_Data.HCI_Inquiry_Result_Event_Data->HCI_Inquiry_Result_Data[Index].BD_ADDR, BoardStr);

               printf("Bluetooth Address : 0x%s\r\n", BoardStr);

               if(NumberofValidResponses != MAX_INQUIRY_RESULTS)
               {
                  for(Index1=0;Index1<NumberofValidResponses;Index1++)
                  {
                     if(COMPARE_BD_ADDR(InquiryResultList[Index1], HCI_Event_Data->Event_Data.HCI_Inquiry_Result_Event_Data->HCI_Inquiry_Result_Data[Index].BD_ADDR))
                        break;
                  }

                  /* Only add the Bluetooth Address to the list if it is*/
                  /* not already present in the list.                   */
                  if(Index1 == NumberofValidResponses)
                     InquiryResultList[NumberofValidResponses++] = HCI_Event_Data->Event_Data.HCI_Inquiry_Result_Event_Data->HCI_Inquiry_Result_Data[Index].BD_ADDR;
               }
            }

            printf("HCI>");
            break;
         case etInquiry_Complete_Event:
            /* Simply Inform the user of the number of Bluetooth Devices*/
            /* that were found.                                         */
            printf("\r\n");
            printf("etInquiry_Complete_Event Received.\r\n");
            printf("%u Device%s found.\r\n", NumberofValidResponses, (NumberofValidResponses != 1)?"s":"");

            for(Index=0;Index < NumberofValidResponses;Index++)
            {
               BD_ADDRToStr(InquiryResultList[Index], BoardStr);

               printf("Inquiry Result : %d: %s.\n", (Index+1), BoardStr);
            }

            printf("HCI>");
            break;
         case etConnection_Complete_Event:
            /* Now check to see if this Connection Complete Event is for*/
            /* an ACL or SCO Connection because this Callback is only   */
            /* setup to process ACL and SCO Connections.                */
            printf("\r\n");
            if(HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Link_Type == HCI_LINK_TYPE_ACL_CONNECTION)
            {
               /* ACL Connection Complete Event.                        */
               printf("etConnection_Complete_Event: Status 0x%02X, Connection_Handle 0x%04X, Type: ACL\r\n", HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Status,
                                                                                                             HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Connection_Handle);
               BD_ADDRToStr(HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR, BoardStr);
               printf("BD_ADDR:  0x%s\r\n", BoardStr);

               /* Check to see if the Status indicates that the         */
               /* connection was successfully created.                  */
               if(!HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Status)
               {
                  /* The Status Indicates that the Connection was       */
                  /* successfully established, save the Connection      */
                  /* Handle and BD_ADDR for later use.                  */
                  ACLConnectionHandle = HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Connection_Handle;

                  /* Let's make sure that we know about the specified   */
                  /* Bluetooth Device by making sure it is present in   */
                  /* Inquiry List.                                      */
                  /* * NOTE * If the list is full we will replace the   */
                  /*          last entry in the list.                   */
                  for(Index=0;Index<NumberofValidResponses;Index++)
                  {
                     if(COMPARE_BD_ADDR(InquiryResultList[Index], HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR))
                        break;
                  }

                  /* Only add the Bluetooth Address to the list if it is*/
                  /* not already present in the list.                   */
                  if(Index == NumberofValidResponses)
                  {
                     if(NumberofValidResponses == MAX_INQUIRY_RESULTS)
                        InquiryResultList[NumberofValidResponses - 1] = HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR;
                     else
                        InquiryResultList[NumberofValidResponses++] = HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR;
                  }
               }
               else
               {
                  /* Flag that there is no Connection present.          */
                  ACLConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;
                  SCOConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;

                  /* Flag that there is no Connection Request           */
                  /* outstanding.                                       */
                  ConnectionRequestIndexID = -1;
               }
            }
            else
            {
               if(HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Link_Type == HCI_LINK_TYPE_SCO_CONNECTION)
               {
                  /* SCO Connection Complete Event.                     */
                  printf("etConnection_Complete_Event: Status 0x%02X, Connection_Handle 0x%04X, Type: SCO\r\n", HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Status,
                                                                                                                HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Connection_Handle);
                  BD_ADDRToStr(HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR, BoardStr);
                  printf("BD_ADDR:  0x%s\r\n", BoardStr);

                  /* Check to see if the Status indicates that the      */
                  /* connection was successfully created.               */
                  if(!HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Status)
                  {
                     /* The Status Indicates that the Connection was    */
                     /* successfully established, save the Connection   */
                     /* Handle and BD_ADDR for later use.               */
                     SCOConnectionHandle = HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Connection_Handle;

                     /* Let's make sure that we know about the specified*/
                     /* Bluetooth Device by making sure it is present in*/
                     /* Inquiry List.                                   */
                     /* * NOTE * If the list is full we will replace    */
                     /*          the last entry in the list.            */
                     for(Index=0;Index<NumberofValidResponses;Index++)
                     {
                        if(COMPARE_BD_ADDR(InquiryResultList[Index], HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR))
                           break;
                     }

                     /* Only add the Bluetooth Address to the list if   */
                     /* it is not already present in the list.          */
                     if(Index == NumberofValidResponses)
                     {
                        if(NumberofValidResponses == MAX_INQUIRY_RESULTS)
                           InquiryResultList[NumberofValidResponses - 1] = HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR;
                        else
                           InquiryResultList[NumberofValidResponses++] = HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->BD_ADDR;
                     }
                  }
                  else
                  {
                     /* Flag that there is no SCO Connection present.   */
                     SCOConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;

                     /* Flag that there is no Connection Request        */
                     /* outstanding.                                    */
                     ConnectionRequestIndexID = -1;
                  }
               }
            }

            printf("HCI>");
            break;
         case etConnection_Request_Event:
            /* Now make sure that this Connection Request Event is for  */
            /* an ACL Connection because this Callback is only setup to */
            /* accept ACL Connection.                                   */
            printf("\r\n");
            BD_ADDRToStr(HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->BD_ADDR, BoardStr);
            printf("etConnection_Request_Event: BD_ADDR: 0x%s, Type: %s\r\n", BoardStr, (HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->Link_Type == HCI_LINK_TYPE_ACL_CONNECTION)?"ACL":"SCO");

            /* Let's make sure that we know about the specified         */
            /* Bluetooth Device by making sure it is present in Inquiry */
            /* List.                                                    */
            /* * NOTE * If the list is full we will replace the last    */
            /*          entry in the list.                              */
            for(Index=0;Index<NumberofValidResponses;Index++)
            {
               if(COMPARE_BD_ADDR(InquiryResultList[Index], HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->BD_ADDR))
                  break;
            }

            /* Only add the Bluetooth Address to the list if it is not  */
            /* already present in the list.                             */
            if(Index == NumberofValidResponses)
            {
               if(NumberofValidResponses == MAX_INQUIRY_RESULTS)
                  InquiryResultList[NumberofValidResponses - 1] = HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->BD_ADDR;
               else
                  InquiryResultList[NumberofValidResponses++] = HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->BD_ADDR;
            }

            if(HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->Link_Type == HCI_LINK_TYPE_ACL_CONNECTION)
            {
               /* Since this application can only support one open ACL  */
               /* Connection at a time check to see if an ACL Connection*/
               /* currently exists.                                     */
               if(ACLConnectionHandle == HCI_CONNECTION_HANDLE_INVALID_VALUE)
               {
                  /* No ACL Connection currently exists, so note the    */
                  /* Index of the Bluetooth Device that has requested   */
                  /* the connection.                                    */
                  ConnectionRequestIndexID = Index;
               }
               else
               {
                  /* An ACL Connection currently exists, reject the     */
                  /* incoming connection request.                       */
                  /* * NOTE * We are doing this because we only support */
                  /*          a single ACL Connection with this program.*/
                  if(!HCI_Reject_Connection_Request(BluetoothStackID, HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->BD_ADDR, HCI_ERROR_CODE_HOST_REJECTED_DUE_TO_LIMITED_RESOURCES, &StatusResult))
                     printf("Successfully Rejected ACL Connection.\r\n");
                  else
                     printf("UN-Successfully Rejected ACL Connection.\r\n");
               }
            }
            else
            {
               /* Since this application can only support one open SCO  */
               /* Connection at a time check to see if an ACL Connection*/
               /* currently exists.                                     */
               if(SCOConnectionHandle == HCI_CONNECTION_HANDLE_INVALID_VALUE)
               {
                  /* No SCO Connection currently exists, so note the    */
                  /* Index of the Bluetooth Device that has requested   */
                  /* the connection.                                    */
                  ConnectionRequestIndexID = Index;
               }
               else
               {
                  /* A SCO Connection currently exists, reject the      */
                  /* incoming connection request.                       */
                  /* * NOTE * We are doing this because we only support */
                  /*          a single SCO Connection with this program.*/
                  if(!HCI_Reject_Connection_Request(BluetoothStackID, HCI_Event_Data->Event_Data.HCI_Connection_Request_Event_Data->BD_ADDR, HCI_ERROR_CODE_HOST_REJECTED_DUE_TO_LIMITED_RESOURCES, &StatusResult))
                     printf("Successfully Rejected SCO Connection.\r\n");
                  else
                     printf("UN-Successfully Rejected SCO Connection.\r\n");
               }
            }

            printf("HCI>");
            break;
         case etDisconnection_Complete_Event:
            /* Check to see if the ACL Connection was the Connection    */
            /* terminated.  Note that this callback only processes ACL  */
            /* connection events.                                       */
            if(HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Connection_Handle == ACLConnectionHandle)
            {
               printf("\r\n");
               printf("etDisconnection_Complete_Event: Status 0x%02X, Reason 0x%04X, Type: ACL.\r\n", HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Status,
                                                                                                      HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Reason);

               /* Check the status to see if the Disconnect complete    */
               /* indicates success.                                    */
               if(!HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Status)
               {
                  /* Since the ACL Connection is down we need to note   */
                  /* this and also note that the SCO Connection is down */
                  /* because a SCO Connection cannot exist without an   */
                  /* ACL Connection.                                    */
                  ACLConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;
                  SCOConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;

                  /* Flag that there is no Connection Request           */
                  /* outstanding.                                       */
                  ConnectionRequestIndexID = -1;
               }

               printf("HCI>");
            }
            else
            {
               if(HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Connection_Handle == SCOConnectionHandle)
               {
                  printf("\r\n");
                  printf("etDisconnection_Complete_Event: Status 0x%02X, Reason 0x%04X, Type: SCO.\r\n", HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Status,
                                                                                                         HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Reason);

                  /* Check the status to see if the Disconnect complete */
                  /* indicates success.                                 */
                  if(!HCI_Event_Data->Event_Data.HCI_Disconnection_Complete_Event_Data->Status)
                  {
                     /* Flag that the SCO Connection is no longer       */
                     /* present.                                        */
                     SCOConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;

                     /* Flag that there is no Connection Request        */
                     /* outstanding.                                    */
                     ConnectionRequestIndexID = -1;
                  }

                  printf("HCI>");
               }
            }
            break;
         case etDevice_Reset_Event:
            printf("\r\n");
            printf("etDevice_Reset_Event.\r\n");

            /* Invalidate all of the connection information because a   */
            /* device reset closes all connections.                     */
            ACLConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;
            SCOConnectionHandle      = HCI_CONNECTION_HANDLE_INVALID_VALUE;

            /* Flag that there is no Connection Request outstanding.    */
            ConnectionRequestIndexID = -1;

            printf("HCI>");
            break;
         default:
            break;
      }
   }

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* The following function is for the HCI ACL Data Callback.  This    */
   /* function will be called whenever a Callback has been registered   */
   /* for the specified HCI Action that is associated with the Bluetooth*/
   /* Stack.  This function passes to the caller the HCI ACL Data and   */
   /* the HCI ACL Data Callback Parameter that was specified when this  */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the HCI ACL Data ONLY in the context of this callback.  If the    */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because other HCI ACL Data will not be*/
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other HCI ACL Data.  A*/
   /*          Deadlock WILL occur because NO HCI Data Callbacks will   */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI HCI_ACL_Data_Callback(unsigned int BluetoothStackID, Word_t Connection_Handle, Word_t Flags, Word_t ACLDataLength, Byte_t *ACLData, unsigned long CallbackParameter)
{
   Word_t Index;

   /* First check to make sure that the paramters passed into us appear */
   /* to be at least semi-valid.                                        */
   if((BluetoothStackID) && (ACLDataLength) && (ACLData))
   {
      /* First check to see if this is a ACL start packet               */
      if(Flags & HCI_ACL_FLAGS_PACKET_BOUNDARY_FIRST_PACKET)
      {
         /* This is a L2CAP start packet now check to make sure that    */
         /* the length is at least greater then 4, i.e the packet       */
         /* contains the L2CAP header information.                      */
         if(ACLDataLength > 4)
         {
            /* Inform the User of the ACL Data that was received.       */
            printf("\r\nReceived %u Byte%s from 0x%04X\r\n", ACLDataLength-4, ((ACLDataLength-4) != 1)?"s":"", Connection_Handle);

            printf("Data: ");

            Index = 4;
            while(Index < ACLDataLength)
            {
               if(ACLData[Index])
                  printf("%c", ACLData[Index]);

               Index++;
            }

            printf("\r\nHCI>");
         }
         else
         {
            printf("\r\nReceived %u from 0x%04X\r\n", ACLDataLength, Connection_Handle);

            printf("HCI>");
         }

         /* Make sure the output is displayed to the user.              */
         fflush(stdout);
      }
      else
      {
         /* The received packet is a L2CAP continuation packet.         */
         if(ACLDataLength)
         {
            /* Inform the User of the ACL Data that was received.       */
            printf("\r\nReceived %u Byte%s from 0x%04X\r\n", ACLDataLength, ((ACLDataLength) != 1)?"s":"", Connection_Handle);

            printf("Data: ");

            Index = 0;
            while(Index < ACLDataLength)
            {
               if(ACLData[Index])
                  printf("%c", ACLData[Index]);

               Index++;
            }

            printf("\r\nHCI>");
         }
         else
         {
            printf("\r\nReceived %u from 0x%04X\r\n", ACLDataLength, Connection_Handle);

            printf("HCI>");
         }

         /* Make sure the output is displayed to the user.              */
         fflush(stdout);
      }
   }
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   char                    *endptr = NULL;
   unsigned int             CommPortNumber;
   unsigned int             BaudRate;
   HCI_DriverInformation_t  HCI_DriverInformation;
   HCI_DriverInformation_t *HCI_DriverInformationPtr;

   /* First, check to see if the right number of parameters where       */
   /* entered at the Command Line.                                      */
   if(argc >= NUM_EXPECTED_PARAMETERS_USB)
   {
      /* The minimum number of parameters were entered at the Command   */
      /* Line.  Check the first parameter to determine if the           */
      /* application is supposed to run in UART or USB mode.            */
      switch(strtol(argv[1], &endptr, 10))
      {
         case USB_PARAMETER_VALUE:
            /* The Transport selected was USB, setup the Driver         */
            /* Information Structure to use USB as the HCI Transport.   */
            HCI_DRIVER_SET_USB_INFORMATION(&HCI_DriverInformation);

            HCI_DriverInformationPtr = &HCI_DriverInformation;
            break;
         case UART_PARAMETER_VALUE:
            /* The Transport selected was UART, check to see if the     */
            /* number of parameters entered on the command line is      */
            /* correct.                                                 */
            if(argc == NUM_EXPECTED_PARAMETERS_UART)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate       = strtol(argv[3], &endptr, 10);
               
               /* Either a port number or a device file can be used.    */
               if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[2], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, cpUART);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, cpUART, 0, argv[2]);
               }

               HCI_DriverInformationPtr = &HCI_DriverInformation;
            }
            else
               HCI_DriverInformationPtr =  NULL;
            break;
         case BCSP_PARAMETER_VALUE:
            /* The Transport selected was BCSP, check to see if the     */
            /* number of parameters entered on the command line is      */
            /* correct.                                                 */
            if(argc == NUM_EXPECTED_PARAMETERS_UART)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate = strtol(argv[3], &endptr, 10);
               
               /* Either a port number or a device file can be used.    */
               if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
               {
                  CommPortNumber = strtol(argv[2], &endptr, 10);
                  HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, cpBCSP);
               }
               else
               {
                  HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, cpBCSP, 0, argv[2]);
               }

               HCI_DriverInformationPtr = &HCI_DriverInformation;
            }
            else
               HCI_DriverInformationPtr =  NULL;
            break;
         default:
            HCI_DriverInformationPtr =  NULL;
            break;
      }

      /* Check to see if the HCI_Driver Information Strucuture was      */
      /* successfully setup.                                            */
      if(HCI_DriverInformationPtr)
      {
         /* Try to Open the stack and check if it was successful.       */
         if(!OpenStack(HCI_DriverInformationPtr))
         {
            /* The stack was opened successfully.  Next, simply open the*/
            /* User Interface and start the user interaction.           */

            /* Start the User Interface.                                */
            UserInterface();

            /* Close the Bluetooth Stack.                               */
            CloseStack();
         }
         else
         {
            /* There was an error while attempting to open the Stack.   */
            printf("Unable to open the stack.\r\n");
         }
      }
      else
      {
         /* One or more of the Command Line parameters appear to be     */
         /* invalid.                                                    */
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}


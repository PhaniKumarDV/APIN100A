/*****< SS1Tool.c >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SS1TOOL - Command-line driven Linux application using Bluetopia Platform  */
/*            Manager Device Manager Application Programming (API) Interface. */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/25/12  G. Hensley     Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>        /* Include for getpid().                           */

#include "SS1Tool.h"       /* Main Application Prototypes and Constants.      */

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

#define MAX_NUM_OF_PARAMETERS                     (512)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

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

#define TOO_MANY_PARAMS                            (-5)  /* Denotes that there*/
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

#define HELP_PARAMETER                         "-help-"  /* Constant value    */
                                                         /* used by the help  */
                                                         /* system to indicate*/
                                                         /* that a command    */
                                                         /* display it's help */
                                                         /* text rather than  */
                                                         /* execute.          */

#define MAX_LINE_COUNT                             (20)  /* Denotes the       */
                                                         /* Maximum amount of */
                                                         /* bytes to display  */
                                                         /* on a single line. */

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

static char               *RunAsName;               /* Variable which points to the    */
                                                    /* name by which this utility was  */
                                                    /* run.                            */

static int                 ArgCount;                /* Variable which holds the number */
                                                    /* of command-line arguments       */
                                                    /* supplied to this program.       */

static char              **ArgList;                 /* Variable which points to the    */
                                                    /* list of arguments supplied to   */
                                                    /* this program.                   */

static unsigned int        DEVMCallbackID;          /* Variable which holds the        */
                                                    /* Callback ID of the currently    */
                                                    /* registered Device Manager       */
                                                    /* Callback ID.                    */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

   /* Internal function prototypes.                                     */
static int UserInterface(void);
static int StringToUnsignedInteger(char *StringInteger, unsigned int *IntegerDest);
static int CommandParser(UserCommand_t *TempCommand);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);

static int DisplayHelp(ParameterList_t *TempParam);

static int Cmd(ParameterList_t *TempParam);

   /* BTPM Server Un-Registration Callback function prototype.          */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter);

   /* BTPM Local Device Manager Callback function prototype.            */
static void BTPSAPI DEVM_Event_Callback(DEVM_Event_Data_t *EventData, void *CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static int UserInterface(void)
{
   int           Result;
   UserCommand_t TempCommand;

   Result = 0;

   ClearCommands();
   AddCommand("CMD", Cmd);
   AddCommand("HELP", DisplayHelp);

   /* Next, check to see if a command was input by the user.            */
   if(ArgCount > 0)
   {
      /* The user provided input, now run the input parameters through  */
      /* the Command Parser.                                            */
      Result = CommandParser(&TempCommand);
   }
   else
   {
      /* No input was provided. Assume the default command of "help", in*/
      /* order to display usage information.                            */
      TempCommand.Command                       = "HELP";
      TempCommand.Parameters.NumberofParameters = 0;
   }

   if(Result >= 0)
   {
      /* The Command was successfully parsed, run the Command.          */
      Result = CommandInterpreter(&TempCommand);

      switch(Result)
      {
         case INVALID_COMMAND_ERROR:
            printf("Invalid Command.\r\n");
            break;
         case FUNCTION_ERROR:
            printf("Function Error.\r\n");
            break;
         default:
            printf("\r\n");
            break;
      }
   }
   else
      printf("Invalid Input.\r\n");

   return Result;
}

   /* The following function is responsible for converting number       */
   /* strings to there unsigned integer equivalent. This function can   */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values. This function takes as its input*/
   /* the string which is to be converted and the located to which the  */
   /* integer value should be stored. The function returns zero if an   */
   /* error occurs otherwise it returns the length of the value, in     */
   /* bytes, parsed from the string passed as the input parameter.      */
static int StringToUnsignedInteger(char *StringInteger, unsigned int *IntegerDest)
{
   int          IsHex;
   unsigned int Index;
   unsigned int ret_val;

   ret_val      = 0;
   *IntegerDest = 0;

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
               *IntegerDest += (StringInteger[Index] & 0xF);

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 10.                                */
                  *IntegerDest *= 10;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }

         /* Since this was a decimal number, manually determine the     */
         /* number of bytes required for representation.                */
         if(*IntegerDest & 0xFF000000)
            ret_val = 4;
         else
         {
            if(*IntegerDest & 0x00FF0000)
               ret_val = 3;
            else
            {
               if(*IntegerDest & 0x0000FF00)
                  ret_val = 2;
               else
               {
                  if(*IntegerDest & 0x000000FF)
                     ret_val = 1;
               }
            }
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
               /* Add this nibble to the current count.                 */
               ret_val++;

               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
                  *IntegerDest += (StringInteger[Index] & 0xF);
               else
               {
                  if((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f'))
                     *IntegerDest += (StringInteger[Index] - 'a' + 10);
                  else
                     *IntegerDest += (StringInteger[Index] - 'A' + 10);
               }

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < strlen(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 16.                                */
                  *IntegerDest *= 16;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }

         /* Convert the nibble count to a byte count.                   */
         ret_val = ((ret_val + 1) / 2);
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
static int CommandParser(UserCommand_t *TempCommand)
{
   int            ret_val;
   unsigned int   Count         = 0;

   /* Before proceeding make sure that the passed parameters appear to  */
   /* be at least semi-valid.                                           */
   if((TempCommand) && (ArgCount > 0))
   {
      /* Retrieve the first token in the parameter list, this should be */
      /* the commmand.                                                  */
      TempCommand->Command = ArgList[0];

      /* The remaining arguments are the command parameters.            */
      TempCommand->Parameters.NumberofParameters = (ArgCount - 1);

      /* Check to see if there is a Command.                            */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val = 0;

         /* There was an available command, now parse out the parameters*/
         while(Count < TempCommand->Parameters.NumberofParameters)
         {
            /* There is an available parameter, now check to see if     */
            /* there is room in the UserCommand to store the parameter  */
            if(Count < (sizeof(TempCommand->Parameters.Params)/sizeof(Parameter_t)))
            {
               /* Save the parameter as a string.                       */
               TempCommand->Parameters.Params[Count].strParam = ArgList[Count+1];

               /* Save the parameter as an unsigned int intParam will   */
               /* have a value of zero if an error has occurred.        */
               StringToUnsignedInteger(ArgList[Count+1], &(TempCommand->Parameters.Params[Count].intParam));

               Count++;

               ret_val = 0;
            }
            else
            {
               /* Be sure we exit out of the Loop.                      */
               Count   = TempCommand->Parameters.NumberofParameters;

               ret_val = TOO_MANY_PARAMS;
            }
         }
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
      /* Now loop through each element in the table to see if there is a*/
      /* match.                                                         */
      for(Index=0,ret_val=NULL;((Index<NumberCommands) && (!ret_val));Index++)
      {
         if((strlen(Command) == strlen(CommandTable[Index].CommandName)) && (memcmp(Command, CommandTable[Index].CommandName, strlen(CommandTable[Index].CommandName)) == 0))
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

   /* The following function is responsible for displaying the current  */
   /* Command Options for this Device Manager Sample Application.  The  */
   /* input parameter to this function is completely ignored, and only  */
   /* needs to be passed in because all Commands that can be entered at */
   /* the Prompt pass in the parsed information.  This function displays*/
   /* the current Command Options that are available and always returns */
   /* zero.                                                             */
static int DisplayHelp(ParameterList_t *TempParam)
{
   int               ret_val;
   unsigned int      Index;
   unsigned int      CommandLength;
   CommandFunction_t CommandFunction;

   ret_val = 0;

   /* Check if any parameters were supplied.                            */
   if((TempParam) && (TempParam->NumberofParameters == 1))
   {
      /* Check if the user requested help for the 'help' command.       */
      if((strncmp(HELP_PARAMETER, TempParam->Params[0].strParam, strlen(HELP_PARAMETER)) == 0) && (TempParam->Params[0].intParam = (unsigned int)(-1)))
      {
         printf("Usage: %s help [command]\r\n", RunAsName);
         printf("\r\n");
         printf("   Displays a list of available commands or, if [command] is given, displays\r\n");
         printf("   usage details for a specific command.\r\n");

         ret_val         = 0;
         CommandFunction = NULL;
      }
      else
      {
         /* The first (and only) parameter should be the name of a      */
         /* command.  First, make this name all upper case for searching*/
         /* the command list.                                           */
         CommandLength = BTPS_StringLength(TempParam->Params[0].strParam);
         for(Index = 0; Index < CommandLength; Index++)
         {
            if((TempParam->Params[0].strParam[Index] >= 'a') && (TempParam->Params[0].strParam[Index] <= 'z'))
               TempParam->Params[0].strParam[Index] -= ('a' - 'A');
         }

         /* Attempt to find the command in the command table.           */
         if((CommandFunction = FindCommand(TempParam->Params[0].strParam)) != NULL)
         {
            /* Reconfigure the parameter list to indicate that we want  */
            /* the command-specific help printed.                       */
            TempParam->NumberofParameters = 1;
            TempParam->Params[0].strParam = "-help-";
            TempParam->Params[0].intParam = (unsigned int)(-1);

            /* The command was found in the table so call the command.  */
            if(!((*CommandFunction)(TempParam)))
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
               ret_val = FUNCTION_ERROR;
         }
         else
            ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      CommandFunction = NULL;
      ret_val         = INVALID_PARAMETERS_ERROR;
   }

   if((!CommandFunction) && (ret_val == INVALID_PARAMETERS_ERROR))
   {
      printf("Usage: %s [command] [parameters]\r\n", RunAsName);
      printf("\r\n");
      printf("Available Commands:\r\n");
      printf("\r\n");
      printf("    cmd [ogf] [ocf] [parameters]     Send an arbitrary HCI command.\r\n");
      printf("\r\n");
      printf("    help [command]                   Display this help information or\r\n");
      printf("                                     command-specific help.\r\n");
   }

   return(0);
}

   /* The following function implements the "CMD" command which executes*/
   /* a raw HCI command. This function returns zero if successful and a */
   /* negative value if an error occurred.                              */
static int Cmd(ParameterList_t *TempParam)
{
   int           Result;
   int           ret_val;
   char         *InputString;
   long          InputValue;
   Byte_t        TempByte;
   Byte_t        OGF;
   Word_t        OCF;
   unsigned int  Index;
   unsigned int  LineCount;
   unsigned char CommandLength;
   unsigned char CommandData[256];
   unsigned char ResultStatus;
   unsigned char ResultLength;
   unsigned char ResultBuffer[256];

   ret_val = 0;

   if((TempParam) && (TempParam->NumberofParameters >= 1) && (strncmp(HELP_PARAMETER, TempParam->Params[0].strParam, BTPS_StringLength(HELP_PARAMETER)) == 0))
   {
      printf("Usage: cmd [OGF] [OCF] [Parameters]\r\n");
      printf("\r\n");
      printf("    Parameters are specified as hexadecimal bytes (optionally prefixed\r\n");
      printf("    with '0x'), separated by spaces. Up to 255 bytes may be given.\r\n");
      printf("\r\n");
      printf("Example: cmd 03 0013 41 42 43 44 00\r\n");
   }
   else
   {
      /* First, check to make sure that we have already been            */
      /* initialized.                                                   */
      if(Initialized)
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->NumberofParameters <= 257))
         {
            /* Read the parameters in a fashion similar to BlueZ's      */
            /* hcitool. That is, values are always assumed to be        */
            /* hexadecimal, multiple bytes are never concatenated into  */
            /* one parameter, and individual parameters are always      */
            /* truncated to the last two characters (or four characters,*/
            /* in the case of the OCF).                                 */

            /* Parse the OGF.                                           */
            InputString = TempParam->Params[0].strParam;
            if(BTPS_StringLength(InputString) > 2)
               InputString += BTPS_StringLength(InputString) - 2;

            errno      = 0;
            InputValue = strtol(TempParam->Params[0].strParam, NULL, 16);

            /* If strtol failed, errno will hold a non-zero error code. */
            if(!errno)
            {
               /* Parsing succeeded. Store the OGF and continue with the*/
               /* OCF.                                                  */
               OGF = (Byte_t)InputValue;
            }
            else
            {
               /* The data string is invalid. Display an error and exit */
               /* the parsing loop.                                     */
               printf("Unable to parse the OGF (%s). Either it is not a number or is out of range.\r\n", TempParam->Params[0].strParam);
               ret_val = INVALID_PARAMETERS_ERROR;
            }

            /* Parse the OCF.                                           */
            if(!ret_val)
            {
               InputString = TempParam->Params[1].strParam;
               if(BTPS_StringLength(InputString) > 4)
                  InputString += BTPS_StringLength(InputString) - 4;

               errno      = 0;
               InputValue = strtol(TempParam->Params[1].strParam, NULL, 16);

               /* If strtol failed, errno will hold a non-zero error    */
               /* code.                                                 */
               if(!errno)
               {
                  /* Parsing succeeded. Store the OGF and continue with */
                  /* the OCF.                                           */
                  OCF = (Word_t)InputValue;
               }
               else
               {
                  /* The data string is invalid. Display an error and   */
                  /* exit the parsing loop.                             */
                  printf("Unable to parse the OCF (%s). Either it is not a number or is out of range.\r\n", TempParam->Params[1].strParam);
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }

            /* Convert the command data into a byte array.              */
            if(!ret_val)
            {
               if(TempParam->NumberofParameters >= 3)
               {
                  BTPS_MemInitialize(CommandData, 0, sizeof(CommandData));
                  CommandLength = 0;

                  for(Index = 2; Index < TempParam->NumberofParameters; Index++)
                  {
                     if(BTPS_StringLength(TempParam->Params[Index].strParam))
                     {
                        InputString = TempParam->Params[Index].strParam;

                        /* Truncate the parameter down to the last two  */
                        /* characters.                                  */
                        if(BTPS_StringLength(InputString) > 2)
                           InputString += BTPS_StringLength(InputString) - 2;

                        /* Parse the parameter has a hexadecimal value. */
                        errno      = 0;
                        InputValue = strtol(InputString, NULL, 16);

                        /* If strtol failed, errno will hold a non-zero */
                        /* error code.                                  */
                        if(!errno)
                        {
                           /* Parsing succeeded. Store the byte and     */
                           /* continue with the next parameter.         */
                           CommandData[CommandLength] = (Byte_t)InputValue;
                           CommandLength             += 1;
                        }
                        else
                        {
                           /* The data string is invalid. Display an    */
                           /* error and exit the parsing loop.          */
                           printf("Unable to parse parameter \"%s\". Either it is not a number or is out of range.\r\n", TempParam->Params[Index].strParam);
                           ret_val = INVALID_PARAMETERS_ERROR;
                           Index   = TempParam->NumberofParameters;
                        }
                     }
                  }
               }
               else
                  CommandLength = 0;
            }

            if(!ret_val)
            {
               printf("< HCI Command: ogf 0x%02X, ocf 0x%04X, plen %u\r\n",OGF, OCF, CommandLength);
               if(CommandLength)
               {
                  printf(" ");

                  for(Index = 0,LineCount = 0; Index < CommandLength; Index++,LineCount++)
                  {
                     printf(" %02X", CommandData[Index]);

                     if((LineCount == MAX_LINE_COUNT) && ((Index+1) < CommandLength))
                     {
                        printf("\n ");
                        LineCount = 1;
                     }
                  }

                  printf("\n");
               }

               ResultLength = (unsigned char)sizeof(ResultBuffer);
               BTPS_MemInitialize(ResultBuffer, 0, sizeof(ResultBuffer));

               Result = DEVM_SendRawHCICommand(OGF, OCF, CommandLength, CommandData, &ResultStatus, &ResultLength, ResultBuffer, TRUE);

               if(Result == 0)
               {
                  printf("> HCI Event: 0x0e plen %u\n", ResultLength+3);
                  printf("  01");

                  TempByte = 0;
                  TempByte = OCF & 0x00FF;

                  printf(" %02X", TempByte);

                  TempByte  = 0;
                  TempByte |= OGF << 2;
                  TempByte |= (OCF >> 8) & 0x00FF;

                  printf(" %02X", TempByte);

                  for(Index = 0,LineCount = 4; Index < ResultLength; Index++,LineCount++)
                  {
                     printf(" %02X", ResultBuffer[Index]);

                     if(LineCount == MAX_LINE_COUNT)
                     {
                        printf("\r\n ");
                        LineCount = 0;
                     }
                  }

                  /* Flag success.                                      */
                  ret_val = 0;
               }
               else
               {
                  /* Error Connecting With Remote Device, inform the    */
                  /* user and flag an error.                            */
                  printf("DEVM_SendRawHCICommand() Failure: %d, %s.\r\n", Result, ERR_ConvertErrorCodeToString(Result));

                  ret_val = FUNCTION_ERROR;
               }
            }
         }
         else
         {
            if((TempParam) && (TempParam->NumberofParameters > 257))
            {
               printf("Parameter list is too long.\r\n");
               ret_val = TOO_MANY_PARAMS;
            }
            else
            {
               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               printf("Missing required parameters.\r\n");

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
      }
      else
      {
         /* Not initialized, flag an error.                             */
         printf("Platform Manager has not been initialized.\r\n");

         ret_val = PLATFORM_MANAGER_NOT_INITIALIZED_ERROR;
      }
   }

   return(ret_val);
}

   /* The following function is the Callback function that is installed */
   /* to be notified when any local IPC connection to the server has    */
   /* been lost.  This case only occurs when the Server exits.  This    */
   /* callback allows the application mechanism to be notified so that  */
   /* all resources can be cleaned up (i.e.  call BTPM_Cleanup().       */
void BTPSAPI ServerUnRegistrationCallback(void *CallbackParameter)
{
   printf("Server has been Un-Registered.\r\n");

   printf("DEVM>");

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
            break;
         case detDeviceDiscoveryStarted:
            printf("Device Discovery Started.\r\n");
            break;
         case detDeviceDiscoveryStopped:
            printf("Device Discovery Stopped.\r\n");
            break;
         case detRemoteDeviceFound:
            printf("Remote Device Found.\r\n");
            break;
         case detRemoteDeviceDeleted:
            BD_ADDRToStr(EventData->EventData.RemoteDeviceDeletedEventData.RemoteDeviceAddress, Buffer);

            printf("Remote Device Deleted: %s.\r\n", Buffer);
            break;
         case detRemoteDevicePropertiesChanged:
            printf("Remote Device Properties Changed.\r\n");
            break;
         case detRemoteDevicePropertiesStatus:
            BD_ADDRToStr(EventData->EventData.RemoteDevicePropertiesStatusEventData.RemoteDeviceProperties.BD_ADDR, Buffer);

            printf("Remote Device Properties Status: %s, %s.\r\n", Buffer, EventData->EventData.RemoteDevicePropertiesStatusEventData.Success?"SUCCESS":"FAILURE");
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

   printf("DEVM>");

   /* Make sure the output is displayed to the user.                    */
   fflush(stdout);
}

   /* Main Program Entry Point.                                         */
int main(int argc, char* argv[])
{
   int Result;

   RunAsName = ((argc >= 1) ? argv[0] : "SS1Tool");

   ArgCount  = (argc - 1);
   ArgList   = ((argc > 1) ? &argv[1] : NULL);

   /* Initialize the Platform Manager client.                           */
   if((Result = BTPM_Initialize((unsigned long)getpid(), NULL, ServerUnRegistrationCallback, NULL)) == 0)
   {
      /* Make sure the device is powered.                               */
      Result = DEVM_PowerOnDevice();

      if((Result == 0) || (Result == BTPM_ERROR_CODE_LOCAL_DEVICE_ALREADY_POWERED_UP))
      {
         /* Register the Device Manager callback, in case the user      */
         /* command requires it.                                        */
         if((Result = DEVM_RegisterEventCallback(DEVM_Event_Callback, NULL)) > 0)
         {
            DEVMCallbackID = (unsigned int)Result;
            Initialized    = TRUE;
            Result         = 0;

            /* Begin processing the user command.                       */
            UserInterface();

            /* Unregister the Device Manager callback.                  */
            DEVM_UnRegisterEventCallback(DEVMCallbackID);
         }
      }
      else
         printf("The Bluetooth device is disabled and an error occurred while attempting enable it (%d).\r\n", Result);

      /* Clean up the connection to the Platform Manager service.       */
      BTPM_Cleanup();
   }
   else
      printf("Bluetopia Platform Manager service cannot be found. Exiting.\r\n");

   return (!Result ? 0 : -1);
}


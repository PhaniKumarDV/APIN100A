/*****< ss1sxmlp.c >***********************************************************/
/*      Copyright 2006 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SS1SXMLP - Stonestreet One Simple XML Parser Implementation.              */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/14/06  R. Sledge      Initial creation.                               */
/******************************************************************************/
#include "SS1SXMLP.h"               /* Simple XML Parser Prototypes/Constants.*/

   /* Denotes the constant values returned from the FindTag() function. */
#define XMLP_FIND_TAG_SUCCESS                                    (0)
#define XMLP_FIND_TAG_INVALID_PARAMETER_ERROR                    (-1)
#define XMLP_FIND_TAG_NOT_FOUND_ERROR                            (-2)

   /* Denotes the constant values returned from the FindElementName()   */
   /* function.                                                         */
#define XMLP_FIND_ELEMENT_NAME_SUCCESS                           (0)
#define XMLP_FIND_ELEMENT_NAME_INVALID_PARAMETER_ERROR           (-1)
#define XMLP_FIND_ELEMENT_NAME_NOT_FOUND_ERROR                   (-2)

   /* Denotes the constant values returned from the FindTagType()       */
   /* function.                                                         */
#define XMLP_FIND_TAG_TYPE_SUCCESS                               (0)
#define XMLP_FIND_TAG_TYPE_INVALID_PARAMETER_ERROR               (-1)

   /* Denotes some common characters used in the parsing of XML data.   */
#define _TAB_                                           ((char)(0x09))
#define _SPACE_                                          ((char)(' '))
#define _CARRIAGE_RETURN_                               ((char)('\r'))
#define _LINE_FEED_                                     ((char)('\n'))
#define _WHITE_SPACE_CHARACTER_                         ((char)(0xFE))
#define _LESS_THAN_                                      ((char)('<'))
#define _GREATER_THAN_                                   ((char)('>'))
#define _FORWARD_SLASH_                                  ((char)('/'))

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* testing of a character to see if it is a White Space Character.   */
   /* White Space characters are considered to be the same as the       */
   /* White Space characters that the C Run Time Library isspace()      */
   /* MACRO/function returns.  This MACRO returns a BOOLEAN TRUE if     */
   /* the input argument is either (or FALSE otherwise):                */
   /*   - TAB                   0x09                                    */
   /*   - New Line              0x0A                                    */
   /*   - Vertical TAB          0x0B                                    */
   /*   - Form Feed             0x0C                                    */
   /*   - Carriage Return       0x0D                                    */
   /*   - Space                 0x20                                    */
#define IS_WHITE_SPACE_CHARACTER(_x)             (((_x) == _SPACE_) || (((_x) >= _TAB_) && ((_x) <= _CARRIAGE_RETURN_)))

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* testing of characters to see if the specified character is an     */
   /* Alphabetical Character.  Alphabetical Characters are consider to  */
   /* be a to z and A to Z.  This MACRO returns a BOOLEAN TRUE if the   */
   /* input argument is an Alphabetical Character or FALSE otherwise.   */
#define IS_ALPHABETICAL_CHARACTER(_x)            ((((_x) >= 'a') && ((_x) <= 'z')) || (((_x) >= 'A') && ((_x) <= 'Z')))

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* testing of characters to see if the specified character is a Name */
   /* Space Separator Character.  The Name Space Separator Character is */
   /* a ':'.  This MACRO returns a BOOLEAN TRUE if the input argument is*/
   /* a Name Space Character or FALSE otherwise.                        */
#define IS_NAME_SPACE_SEPARATOR_CHARACTER(_x)    ((_x) == ':')

   /* The following type definition represents the states of the element*/
   /* name builder.                                                     */
typedef enum
{
   besWaitingStart,
   besWaitingSecondCharacter,
   besWaitingThirdCharacter,
   besWaitingEnd,
   besBuildingComplete
} XMLPBuildElementNameState_t;

   /* The following type definition represents the possible tag types.  */
typedef enum
{
   ttOpening,
   ttClosing,
   ttOpeningClosing
} XMLPTagType_t;

   /* Internal Function Prototypes.                                     */
static void XMLPStartDocument(unsigned int BufferLength, unsigned char *Buffer, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
static void XMLPEndDocument(unsigned int BufferLength, unsigned char *Buffer, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
static void XMLPStartElement(unsigned int ElementNameLength, unsigned char *ElementName, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
static void XMLPEndElement(unsigned int ElementNameLength, unsigned char *ElementName, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
static void XMLPCharacter(unsigned int CharacterStringLength, unsigned char *CharacterString, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

static int FindTag(unsigned int BufferLength, unsigned char *Buffer, unsigned int *TagStartIndex, unsigned int *TagLength);
static int FindElementName(unsigned int TagLength, unsigned char *TagBuffer, unsigned int *ElementNameStartIndex, unsigned int *ElementNameLength);
static int FindTagType(unsigned int TagLength, unsigned char *TagBuffer, XMLPTagType_t *TagType);

   /* The following function is responsible for making a Start Document */
   /* Callback.  The first parameter to this function is the length of  */
   /* the buffer pointed to by the next parameter.  The second parameter*/
   /* to this function is a pointer to the buffer containing the XML    */
   /* data to be parsed.  The third and fourth parameters are the Event */
   /* Callback function and application defined Callback Parameter to   */
   /* call.  This function returns no return value.                     */
static void XMLPStartDocument(unsigned int BufferLength, unsigned char *Buffer, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   XMLP_Event_Data_t          EventData;
   XMLP_Start_Document_Data_t StartDocumentData;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((BufferLength) && (Buffer) && (EventCallback))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Format up a Start Document Event.                              */
      EventData.Event_Data_Type                     = etXMLP_Start_Document;
      EventData.Event_Data_Size                     = XMLP_START_DOCUMENT_DATA_SIZE;
      EventData.Event_Data.XMLP_Start_Document_Data = &StartDocumentData;

      StartDocumentData.BufferLength                = BufferLength;
      StartDocumentData.Buffer                      = Buffer;

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is responsible for making a End Document   */
   /* Callback.  The first parameter to this function is the length of  */
   /* the buffer pointed to by the next parameter.  The second parameter*/
   /* to this function is a pointer to the buffer containing the data   */
   /* that was left over that was not able to be parsed.  The third and */
   /* fourth parameters are the Event Callback function and application */
   /* defined Callback Parameter to call.  This function returns no     */
   /* return value.                                                     */
static void XMLPEndDocument(unsigned int BufferLength, unsigned char *Buffer, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   XMLP_Event_Data_t        EventData;
   XMLP_End_Document_Data_t EndDocumentData;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((BufferLength) && (Buffer) && (EventCallback))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Format up an End Document Event.                               */
      EventData.Event_Data_Type                   = etXMLP_End_Document;
      EventData.Event_Data_Size                   = XMLP_END_DOCUMENT_DATA_SIZE;
      EventData.Event_Data.XMLP_End_Document_Data = &EndDocumentData;

      EndDocumentData.BufferLength                = BufferLength;
      EndDocumentData.Buffer                      = Buffer;

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is responsible for making a Start Element  */
   /* Callback.  The first parameter to this function is the length of  */
   /* the element name pointed to by the next parameter.  The second    */
   /* parameter to this function is a pointer to the buffer containing  */
   /* the element name.  The third and fourth parameters are the Event  */
   /* Callback function and application defined Callback Parameter to   */
   /* call.  This function returns no return value.                     */
static void XMLPStartElement(unsigned int ElementNameLength, unsigned char *ElementName, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   XMLP_Event_Data_t         EventData;
   XMLP_Start_Element_Data_t StartElementData;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((ElementNameLength) && (ElementName) && (EventCallback))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Format up a Start Element Event.                               */
      EventData.Event_Data_Type                    = etXMLP_Start_Element;
      EventData.Event_Data_Size                    = XMLP_START_ELEMENT_DATA_SIZE;
      EventData.Event_Data.XMLP_Start_Element_Data = &StartElementData;

      StartElementData.ElementNameLength           = ElementNameLength;
      StartElementData.ElementName                 = ElementName;

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is responsible for making a End Element    */
   /* Callback.  The first parameter to this function is the length of  */
   /* the element name pointed to by the next parameter.  The second    */
   /* parameter to this function is a pointer to the buffer containing  */
   /* the element name.  The third and fourth parameters are the Event  */
   /* Callback function and application defined Callback Parameter to   */
   /* call.  This function returns no return value.                     */
static void XMLPEndElement(unsigned int ElementNameLength, unsigned char *ElementName, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   XMLP_Event_Data_t       EventData;
   XMLP_End_Element_Data_t EndElementData;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((ElementNameLength) && (ElementName) && (EventCallback))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Format up a End Element Event.                                 */
      EventData.Event_Data_Type                  = etXMLP_End_Element;
      EventData.Event_Data_Size                  = XMLP_END_ELEMENT_DATA_SIZE;
      EventData.Event_Data.XMLP_End_Element_Data = &EndElementData;

      EndElementData.ElementNameLength           = ElementNameLength;
      EndElementData.ElementName                 = ElementName;

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is responsible for making a Character      */
   /* Callback.  The first parameter to this function is the length of  */
   /* the character string pointed to by the next parameter.  The second*/
   /* parameter to this function is a pointer to the buffer containing  */
   /* the character string.  The third and fourth parameters are the    */
   /* Event Callback function and application defined Callback Parameter*/
   /* to call.  This function returns no return value.                  */
static void XMLPCharacter(unsigned int CharacterStringLength, unsigned char *CharacterString, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   XMLP_Event_Data_t     EventData;
   XMLP_Character_Data_t CharacterData;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((CharacterStringLength) && (CharacterString) && (EventCallback))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Format up a Character Event.                                   */
      EventData.Event_Data_Type                = etXMLP_Character;
      EventData.Event_Data_Size                = XMLP_CHARACTER_DATA_SIZE;
      EventData.Event_Data.XMLP_Character_Data = &CharacterData;

      CharacterData.CharacterStringLength      = CharacterStringLength;
      CharacterData.CharacterString            = CharacterString;

      /* Finally make the callback.                                     */
      __BTPSTRY
      {
         (*EventCallback)(&EventData, CallbackParameter);
      }
      __BTPSEXCEPT(1)
      {
         /* Do Nothing.                                                 */
      }
   }
}

   /* The following function is responsible for locating the first XML  */
   /* tag in the specified buffer.  The first parameter to this function*/
   /* is the length of the buffer pointed to by the next parameter.  The*/
   /* second parameter to this function is a pointer to the buffer      */
   /* containing the XML data to be parsed.  The third parameter to this*/
   /* function is a pointer to a variable that will be used to return   */
   /* the Tag Start Index up on successful execution.  The fourth       */
   /* parameter to this function is a pointer to a variable that will be*/
   /* used to return the Tag Length up on successful execution.  This   */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
static int FindTag(unsigned int BufferLength, unsigned char *Buffer, unsigned int *TagStartIndex, unsigned int *TagLength)
{
   int          ret_val;
   unsigned int StartIndex;
   unsigned int Index;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((BufferLength) && (Buffer) && (TagStartIndex) && (TagLength))
   {
      /* The parameters passed in appear to be at least semi-valid, next*/
      /* initialize the return value to indicate that no tag was        */
      /* located.                                                       */
      ret_val = XMLP_FIND_TAG_NOT_FOUND_ERROR;

      /* Intialize the other required variables to a known state.       */
      Index            = 0;
      StartIndex       = 0;
      (*TagStartIndex) = 0;
      (*TagLength)     = 0;

      /* Now attempt to located the start of the first tag in the       */
      /* specified buffer.                                              */
      while((Index < BufferLength) && (Buffer[Index] != _LESS_THAN_))
         Index++;

      /* Check to see if the start of the first tag was located in the  */
      /* specified buffer.                                              */
      if(Buffer[Index] == _LESS_THAN_)
      {
         /* The start of the first tag was located in the specifed      */
         /* buffer, save the index of the located start of the first    */
         /* tag.                                                        */
         StartIndex = Index;

         /* Next attempt to located the end of the located tag.         */
         while((Index < BufferLength) && (Buffer[Index] != _GREATER_THAN_))
            Index++;

         /* Check to see if the end of the located tag was found.       */
         if(Buffer[Index] == _GREATER_THAN_)
         {
            /* The end of the located tag was found.  Set the Tag Start */
            /* Index and Tag Length and return with success to the      */
            /* caller.                                                  */
            (*TagStartIndex) = StartIndex;
            (*TagLength)     = (Index + (sizeof(unsigned char)/sizeof(unsigned char))) - StartIndex;
            ret_val          = XMLP_FIND_TAG_SUCCESS;
         }
      }
   }
   else
      ret_val = XMLP_FIND_TAG_INVALID_PARAMETER_ERROR;

   return(ret_val);
}

   /* The following function is responsible for locating the Element    */
   /* Name in the specified XML tag buffer.  The first parameter to this*/
   /* function is the length of the tag buffer pointed to by the next   */
   /* parameter.  The second parameter to this function is a pointer to */
   /* the buffer containing the XML tag data.  The third parameter to   */
   /* this function is a pointer to a variable that will be used to     */
   /* return the Element Name Start Index up on successful execution.   */
   /* The fourth parameter to this function is a pointer to a variable  */
   /* that will be used to return the Element Name Length up on         */
   /* successful execution.  This function returns zero if successful,  */
   /* or a negative return value if there was an error.                 */
static int FindElementName(unsigned int TagLength, unsigned char *TagBuffer, unsigned int *ElementNameStartIndex, unsigned int *ElementNameLength)
{
   int                         ret_val;
   unsigned int                StartIndex;
   unsigned int                Index;
   XMLPBuildElementNameState_t BuildElementNameState;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((TagLength) && (TagBuffer) && (ElementNameStartIndex) && (ElementNameLength))
   {
      /* The parameters passed in appear to be at least semi-valid, next*/
      /* initialize the return value to indicate that no element name   */
      /* was located.                                                   */
      ret_val = XMLP_FIND_ELEMENT_NAME_NOT_FOUND_ERROR;

      /* Intialize the other required variables to a known state.       */
      Index                    = 0;
      StartIndex               = 0;
      BuildElementNameState    = besWaitingStart;
      (*ElementNameStartIndex) = 0;
      (*ElementNameLength)     = 0;

      /* According to www.w3school.com/xml/xml_element.asp element      */
      /* naming must follow the following rules.                        */
      /*    - Names can contain letters, numbers and other characters.  */
      /*    - Names must not start with a number or punctuation         */
      /*      characters.                                               */
      /*    - Names must not start with the letters xml (or XML, or     */
      /*      Xml, etc).                                                */
      /*    - Names cannot contain spaces.                              */
      /*    - The ':' character is reserved to denote the start of a    */
      /*      prefix or namespace.                                      */

      /* Loop through the Tag, searching for the element name.          */
      while((Index < TagLength) && (BuildElementNameState != besBuildingComplete))
      {
         /* Determine what the current building state is.               */
         switch(BuildElementNameState)
         {
            case besWaitingStart:
               /* Currently looking for the start of the element name.  */
               /* The start begins at the first alphabetic character.   */
               if(IS_ALPHABETICAL_CHARACTER(TagBuffer[Index]))
               {
                  /* The start appears to have been located, save the   */
                  /* index of the start of the element name.            */
                  StartIndex = Index;

                  /* Now check to see if this character appears to be a */
                  /* 'x' or a 'X' since names may not start with any    */
                  /* variation of XML.                                  */
                  if((TagBuffer[Index] == 'x') || (TagBuffer[Index] == 'X'))
                  {
                     /* The first character is a 'x' or a 'X'.  Not in  */
                     /* violation yet, move to the Waiting Second       */
                     /* Character state to check to see if it is a 'm'  */
                     /* or 'M'.                                         */
                     BuildElementNameState = besWaitingSecondCharacter;
                  }
                  else
                  {
                     /* The first character is not a 'x' or a 'X'.  Move*/
                     /* to the Waiting End state to continue building   */
                     /* the name.                                       */
                     BuildElementNameState = besWaitingEnd;
                  }
               }
               break;
            case besWaitingSecondCharacter:
               /* The first character must have been a 'x' or a 'X'.    */
               /* Next check to see if the current character is a white */
               /* space character or a '>' character.                   */
               if((IS_WHITE_SPACE_CHARACTER(TagBuffer[Index])) || (TagBuffer[Index] == _GREATER_THAN_))
               {
                  /* The current character is a white space character or*/
                  /* a '>' character.  Both of these character represent*/
                  /* the end of the Element Name.  Set the Building     */
                  /* State to complete to exit the loop, and the return */
                  /* value to indicate success.                         */
                  BuildElementNameState = besBuildingComplete;
                  ret_val               = XMLP_FIND_ELEMENT_NAME_SUCCESS;
               }
               else
               {
                  /* The current character is not a white space         */
                  /* character, check to see if the current character is*/
                  /* a Name Space Separator Character.                  */
                  if(IS_NAME_SPACE_SEPARATOR_CHARACTER(TagBuffer[Index]))
                  {
                     /* The current Character is a Name Space Separator */
                     /* Character.  In this case, all characters up     */
                     /* until this point have been name space prefix    */
                     /* information.  Reset the start index and the     */
                     /* building state to now start looking for the     */
                     /* element name.                                   */
                     StartIndex            = 0;
                     BuildElementNameState = besWaitingStart;
                  }
                  else
                  {
                     /* The current character is not a Name Space       */
                     /* Separator Character.  Next check to see if the  */
                     /* current character is a 'm' or a 'M'.            */
                     if((TagBuffer[Index] == 'm') || (TagBuffer[Index] == 'M'))
                     {
                        /* The second character is a 'm' or a 'M'.  Not */
                        /* in violation yet, move to the Waiting Third  */
                        /* Character state to check to see if it is a   */
                        /* 'l' or 'L'.                                  */
                        BuildElementNameState = besWaitingSecondCharacter;
                     }
                     else
                     {
                        /* The second character is not a 'm' or a 'M'.  */
                        /* Move to the Waiting End state to continue    */
                        /* building the name.                           */
                        BuildElementNameState = besWaitingEnd;
                     }
                  }
               }
               break;
            case besWaitingThirdCharacter:
               /* The second character must have been a 'm' or a 'M'.   */
               /* Next check to see if the current character is a white */
               /* space character or a '>' character.                   */
               if((IS_WHITE_SPACE_CHARACTER(TagBuffer[Index])) || (TagBuffer[Index] == _GREATER_THAN_))
               {
                  /* The current character is a white space character or*/
                  /* a '>' character.  Both of these character represent*/
                  /* the end of the Element Name.  Set the Building     */
                  /* State to complete to exit the loop, and the return */
                  /* value to indicate success.                         */
                  BuildElementNameState = besBuildingComplete;
                  ret_val               = XMLP_FIND_ELEMENT_NAME_SUCCESS;
               }
               else
               {
                  /* The current character is not a white space         */
                  /* character, check to see if the current character is*/
                  /* a Name Space Separator Character.                  */
                  if(IS_NAME_SPACE_SEPARATOR_CHARACTER(TagBuffer[Index]))
                  {
                     /* The current Character is a Name Space Separator */
                     /* Character.  In this case, all characters up     */
                     /* until this point have been name space prefix    */
                     /* information.  Reset the start index and the     */
                     /* building state to now start looking for the     */
                     /* element name.                                   */
                     StartIndex            = 0;
                     BuildElementNameState = besWaitingStart;
                  }
                  else
                  {
                     /* The current character is not a Name Space       */
                     /* Separator Character.  Next check to see if the  */
                     /* current character is a 'l' or a 'L'.            */
                     if((TagBuffer[Index] == 'l') || (TagBuffer[Index] == 'L'))
                     {
                        /* The third character is a 'l' or a 'L'.  This */
                        /* is a violation of the Element Naming rules.  */
                        /* Set the Building State to complete to exit   */
                        /* the loop, leaving the return value to        */
                        /* indicate an error.                           */
                        BuildElementNameState = besBuildingComplete;
                     }
                     else
                     {
                        /* The third character is not a 'l' or a 'L'.   */
                        /* Move to the Waiting End state to continue    */
                        /* building the name.                           */
                        BuildElementNameState = besWaitingEnd;
                     }
                  }
               }
               break;
            case besWaitingEnd:
               /* We are looking for the end of the Element Name.  Next */
               /* check to see if the current character is a white space*/
               /* character or a '>' character.                         */
               if((IS_WHITE_SPACE_CHARACTER(TagBuffer[Index])) || (TagBuffer[Index] == _GREATER_THAN_))
               {
                  /* The current character is a white space character or*/
                  /* a '>' character.  Both of these character represent*/
                  /* the end of the Element Name.  Set the Building     */
                  /* State to complete to exit the loop, and the return */
                  /* value to indicate success.                         */
                  BuildElementNameState = besBuildingComplete;
                  ret_val               = XMLP_FIND_ELEMENT_NAME_SUCCESS;
               }
               else
               {
                  /* The current character is not a white space         */
                  /* character, check to see if the current character is*/
                  /* a Name Space Separator Character.                  */
                  if(IS_NAME_SPACE_SEPARATOR_CHARACTER(TagBuffer[Index]))
                  {
                     /* The current Character is a Name Space Separator */
                     /* Character.  In this case, all characters up     */
                     /* until this point have been name space prefix    */
                     /* information.  Reset the start index and the     */
                     /* building state to now start looking for the     */
                     /* element name.                                   */
                     StartIndex            = 0;
                     BuildElementNameState = besWaitingStart;
                  }
               }
               break;
            default:
               /* We do not care about any remaining Build Element      */
               /* States at this time.                                  */
               break;
         }

         /* Adjust the index to the next character in the tag.          */
         Index++;
      }

      /* Check to see if the element name was successfully located.     */
      if((BuildElementNameState == besBuildingComplete) && (ret_val == XMLP_FIND_ELEMENT_NAME_SUCCESS))
      {
         /* The element name was successfully located.  Set the Element */
         /* Name Start Index and Element Name Length and return with    */
         /* success to the caller.                                      */
         (*ElementNameStartIndex) = StartIndex;
         (*ElementNameLength)     = ((Index - (sizeof(unsigned char)/sizeof(unsigned char))) - StartIndex);
      }
   }
   else
      ret_val = XMLP_FIND_ELEMENT_NAME_INVALID_PARAMETER_ERROR;

   return(ret_val);
}

   /* The following function is responsible for finding the type of the */
   /* specified tag.  The first parameter to this function is the length*/
   /* of the tag buffer pointed to by the next parameter.  The second   */
   /* parameter to this function is a pointer to the buffer containing  */
   /* the XML tag data.  The final parameter to this function is a      */
   /* pointer to a variable that will be used to return the Tag Type up */
   /* on successful execution.  This function returns zero if           */
   /* successful, or a negative return value if there was an error.     */
static int FindTagType(unsigned int TagLength, unsigned char *TagBuffer, XMLPTagType_t *TagType)
{
   int ret_val;

   /* First check to see if the parameters passed in appears to be at   */
   /* least semi-valid.                                                 */
   if((TagLength) && (TagBuffer) && (TagType))
   {
      /* The parameters passed in appear to be at least semi-valid, Next*/
      /* check to make sure that the tag size is at least long enough to*/
      /* be an opening tag.                                             */
      if(TagLength >= (sizeof(_LESS_THAN_) + sizeof(_GREATER_THAN_)))
      {
         /* The tag size is at least long enought to be an opening tag, */
         /* check to make sure that it appears to be at least an opening*/
         /* tag.                                                        */
         if((TagBuffer[0] == _LESS_THAN_) && (TagBuffer[TagLength-1] == _GREATER_THAN_))
         {
            /* It appears to be at lease an opening tag, initialize the */
            /* required variables to a known state.                     */
            ret_val    = XMLP_FIND_TAG_TYPE_SUCCESS;
            (*TagType) = ttOpening;

            /* Next check to see if the tag is long enought to be a     */
            /* opening/closing tag, or just a closing tag.              */
            if(TagLength >= (sizeof(_LESS_THAN_) + sizeof(_FORWARD_SLASH_) + sizeof(_GREATER_THAN_)))
            {
               /* The tag is long enough to be a opening/closing tag, or*/
               /* just a closing tag.  Check to see if it is an empty   */
               /* opening/closing tag.                                  */
               if((TagBuffer[0] == _LESS_THAN_) && (TagBuffer[1] == _FORWARD_SLASH_) && (TagBuffer[2] == _GREATER_THAN_))
               {
                  /* This is an opening/closing tag, set the tag type   */
                  /* appropriately.                                     */
                  (*TagType) = ttOpeningClosing;
               }
               else
               {
                  /* Not an empty opening/closing tag.  Check to see if */
                  /* it is a non-empty opening/closing tag.             */
                  if((TagBuffer[0] == _LESS_THAN_) && (TagBuffer[TagLength-2] == _FORWARD_SLASH_) && ((TagBuffer[TagLength-1] == _GREATER_THAN_)))
                  {
                     /* This is an opening/closing tag, set the tag type*/
                     /* appropriately.                                  */
                     (*TagType) = ttOpeningClosing;
                  }
                  else
                  {
                     /* Not a non-empty opening/closing tag.  Check to  */
                     /* see if it is just a closing tag.                */
                     if((TagBuffer[0] == _LESS_THAN_) && ((TagBuffer[1] == _FORWARD_SLASH_)))
                     {
                        /* This is a closing tag, set the tag type      */
                        /* appropriately.                               */
                        (*TagType) = ttClosing;
                     }
                  }
               }
            }
         }
         else
            ret_val = XMLP_FIND_TAG_TYPE_INVALID_PARAMETER_ERROR;
      }
      else
         ret_val = XMLP_FIND_TAG_TYPE_INVALID_PARAMETER_ERROR;
   }
   else
      ret_val = XMLP_FIND_TAG_TYPE_INVALID_PARAMETER_ERROR;

   return(ret_val);
}

   /* The following function is responsible for XML data.  The first    */
   /* parameter to this function is the length of the buffer pointed to */
   /* by the next parameter.  The second parameter to this function is a*/
   /* pointer to the buffer containing the XML data to be parsed.  The  */
   /* third and fourth parameters are the Event Callback function and   */
   /* application defined Callback Parameter to callback for all parse  */
   /* events.  This function returns zero if successful, or a negative  */
   /* return value if there was an error.                               */
int BTPSAPI XMLP_Parse_Buffer(unsigned int BufferLength, unsigned char *Buffer, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter)
{
   int           ret_val;
   int           Result;
   unsigned int  BufferIndex;
   unsigned int  TagStartIndex;
   unsigned int  TagLength;
   unsigned int  ElementNameStartIndex;
   unsigned int  ElementNameLength;
   XMLPTagType_t TagType;

   /* First check to see if the parameters passed in appear to be at    */
   /* least semi-valid.                                                 */
   if((BufferLength) && (Buffer) && (EventCallback))
   {
      /* The parameters passed in appear to be at least semi-valid.     */
      /* Next make a Start Document Callback.                           */
      XMLPStartDocument(BufferLength, Buffer, EventCallback, CallbackParameter);

      /* Now initialize the required variables to a known state.        */
      ret_val               = 0;
      BufferIndex           = 0;
      TagStartIndex         = 0;
      TagLength             = 0;
      ElementNameStartIndex = 0;
      ElementNameLength     = 0;

      /* Loop through the buffer parsing the XML data.                  */
      while((ret_val == 0) && (BufferIndex < BufferLength))
      {
         /* Attempt to find the first available tag in the buffer.      */
         Result = FindTag((BufferLength - BufferIndex), &(Buffer[BufferIndex]), &TagStartIndex, &TagLength);

         /* Check to see if a tag was successfully located in the       */
         /* buffer.                                                     */
         if(Result == XMLP_FIND_TAG_SUCCESS)
         {
            /* A tag was located, next check to if there appears to be  */
            /* character data in between tags.                          */
            if((BufferIndex + TagStartIndex) > BufferIndex)
            {
               /* There appears to be character data in between tags.   */
               /* Make a Character Callback with this data.             */
               XMLPCharacter(((BufferIndex + TagStartIndex) - BufferIndex), &(Buffer[BufferIndex]), EventCallback, CallbackParameter);

               /* The callback with the character data has been made,   */
               /* now adjust the buffer index to the tag start index.   */
               BufferIndex += TagStartIndex;
            }

            /* Next attempt to locate the Element Name inside of the    */
            /* located tag.                                             */
            if(FindElementName(TagLength, &(Buffer[BufferIndex]), &ElementNameStartIndex, &ElementNameLength) == XMLP_FIND_ELEMENT_NAME_SUCCESS)
            {
               /* The Element Name was successfully located, next       */
               /* determine the type of tag that this is.               */
               if(FindTagType(TagLength, &Buffer[BufferIndex], &TagType) == XMLP_FIND_TAG_TYPE_SUCCESS)
               {
                  /* The Tag Type was successfully determined, check to */
                  /* see if it is an opening tag or an opening/closing  */
                  /* tag.                                               */
                  if((TagType == ttOpening) || (TagType == ttOpeningClosing))
                  {
                     /* The tag is an opening tag or an opening/closing */
                     /* tag.  In this case make a start element         */
                     /* callback.                                       */
                     XMLPStartElement(ElementNameLength, &(Buffer[BufferIndex + ElementNameStartIndex]), EventCallback, CallbackParameter);
                  }

                  /* Next check to see if the tag is a closing tag or an*/
                  /* opening/closing tag.                               */
                  if((TagType == ttClosing) || (TagType == ttOpeningClosing))
                  {
                     /* The tag is a closing tag or an opening/closing  */
                     /* tag.  In this case make a end element callback. */
                     XMLPEndElement(ElementNameLength, &(Buffer[BufferIndex + ElementNameStartIndex]), EventCallback, CallbackParameter);
                  }
               }

               /* Adjust the Buffer Index past the processed tag.       */
               BufferIndex += TagLength;
            }
            else
               ret_val = XMLP_ERROR_ELEMENT_NAME_PARSE;
         }
         else
         {
            /* A tag was not successfully located, check to see if the  */
            /* reason is because there are not more tags in the buffer. */
            if(Result == XMLP_FIND_TAG_NOT_FOUND_ERROR)
            {
               /* There appears to be no more tags in the buffer.  Break*/
               /* out of the loop since processing has completed.       */
               break;
            }
            else
               ret_val = XMLP_ERROR_TAG_PARSE;
         }
      }

      /* All through with the parsing of the buffer.  Make a End        */
      /* Document Callback.                                             */
      XMLPEndDocument((BufferLength - BufferIndex), &(Buffer[BufferIndex]), EventCallback, CallbackParameter);
   }
   else
      ret_val = XMLP_ERROR_INVALID_PARAMETER;

   return(ret_val);
}


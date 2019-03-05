/*****< sxmlpapi.h >***********************************************************/
/*      Copyright 2006 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SXMLPAPI - Stonestreet One Simple XML Parser API Definitions, Constants,  */
/*             and Prototypes.                                                */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/14/06  R. Sledge      Initial creation.                               */
/******************************************************************************/
#ifndef __SXMLPAPIH__
#define __SXMLPAPIH__

#include "BTAPITyp.h"           /* Bluetooth API Type Definitions.            */

   /* Error Return Codes.                                               */
#define XMLP_ERROR_INVALID_PARAMETER                             (-10000)
#define XMLP_ERROR_TAG_PARSE                                     (-10001)
#define XMLP_ERROR_ELEMENT_NAME_PARSE                            (-10002)

   /* Simple XML Parser Event API Types.                                */
typedef enum
{
   etXMLP_Start_Document,
   etXMLP_End_Document,
   etXMLP_Start_Element,
   etXMLP_End_Element,
   etXMLP_Character
} XMLP_Event_Type_t;

   /* The following event is dispatched at the start of the XML         */
   /* document.  The BufferLength member specifies the length of data   */
   /* pointed to by the Buffer member.  The Buffer member points to a   */
   /* buffer containing the actual XML document that is being parsed.   */
typedef struct _tagXMLP_Start_Document_Data_t
{
   unsigned int   BufferLength;
   unsigned char *Buffer;
} XMLP_Start_Document_Data_t;

#define XMLP_START_DOCUMENT_DATA_SIZE                   (sizeof(XMLP_Start_Document_Data_t))

   /* The following event is dispatched at the end of the XML document. */
   /* The BufferLength member specifies the length of data pointed to by*/
   /* the Buffer member.  The Buffer member points to a buffer          */
   /* containing the the data left over that was not able to be parsed. */
typedef struct _tagXMLP_End_Document_Data_t
{
   unsigned int   BufferLength;
   unsigned char *Buffer;
} XMLP_End_Document_Data_t;

#define XMLP_END_DOCUMENT_DATA_SIZE                     (sizeof(XMLP_End_Document_Data_t))

   /* The following event is dispatched when a XML start element is     */
   /* located in the XML document.  The ElementNameLength member        */
   /* specifies the length of the data pointed to by the ElementName    */
   /* member.  The ElementName member points to a buffer containing the */
   /* actual Element Name.                                              */
typedef struct _tagXMLP_Start_Element_Data_t
{
   unsigned int   ElementNameLength;
   unsigned char *ElementName;
} XMLP_Start_Element_Data_t;

#define XMLP_START_ELEMENT_DATA_SIZE                    (sizeof(XMLP_Start_Element_Data_t))

   /* The following event is dispatched when a XML end element is       */
   /* located in the XML document.  The ElementNameLength member        */
   /* specifies the length of the data pointed to by the ElementName    */
   /* member.  The ElementName member points to a buffer containing the */
   /* actual Element Name.                                              */
typedef struct _tagXMLP_End_Element_Data_t
{
   unsigned int   ElementNameLength;
   unsigned char *ElementName;
} XMLP_End_Element_Data_t;

#define XMLP_END_ELEMENT_DATA_SIZE                      (sizeof(XMLP_End_Element_Data_t))

   /* The following event is dispatched when non-markup is located in   */
   /* the XML document.  The CharacterStringLength member specifies the */
   /* length of the data pointed to by the CharacterString member.  The */
   /* CharacterString member points to a buffer containing the actual   */
   /* Character String.                                                 */
   /* ** NOTE ** This data may contain leading and trailing white space */
   /*            characters or may be only white space characters.      */
typedef struct _tagXMLP_Character_Data_t
{
   unsigned int   CharacterStringLength;
   unsigned char *CharacterString;
} XMLP_Character_Data_t;

#define XMLP_CHARACTER_DATA_SIZE                        (sizeof(XMLP_Character_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all Simple XML Parser Data.                               */
typedef struct _tagXMLP_Event_Data_t
{
   XMLP_Event_Type_t Event_Data_Type;
   unsigned long     Event_Data_Size;
   union
   {
      XMLP_Start_Document_Data_t *XMLP_Start_Document_Data;
      XMLP_End_Document_Data_t   *XMLP_End_Document_Data;
      XMLP_Start_Element_Data_t  *XMLP_Start_Element_Data;
      XMLP_End_Element_Data_t    *XMLP_End_Element_Data;
      XMLP_Character_Data_t      *XMLP_Character_Data;
   } Event_Data;
} XMLP_Event_Data_t;

   /* The following declared type represents the Prototype Function for */
   /* the XML Parse Event Callback.                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by other XML Parse Events from   */
   /*            the same document data.  A Deadlock WILL occur because */
   /*            NO XML Parse Event Callbacks will be issued while this */
   /*            function is currently outstanding.                     */
typedef void (BTPSAPI *XMLP_Event_Callback_t)(XMLP_Event_Data_t *XMLP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for XML data.  The first    */
   /* parameter to this function is the length of the buffer pointed to */
   /* by the next parameter.  The second parameter to this function is a*/
   /* pointer to the buffer containing the XML data to be parsed.  The  */
   /* third and fourth parameters are the Event Callback function and   */
   /* application defined Callback Parameter to callback for all parse  */
   /* events.  This function returns zero if successful, or a negative  */
   /* return value if there was an error.                               */
   /* ** NOTE ** Event callbacks that occur during parsing are made from*/
   /*            the context of this function call.                     */
int BTPSAPI XMLP_Parse_Buffer(unsigned int BufferLength, unsigned char *Buffer, XMLP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#endif

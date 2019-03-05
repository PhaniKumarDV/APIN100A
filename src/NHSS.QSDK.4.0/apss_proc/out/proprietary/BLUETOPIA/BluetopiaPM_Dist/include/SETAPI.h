/*****< setapi.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SETAPI - Stonestreet One Bluetooth Protocol Stack Platform Manager        */
/*           settings and configuration API functions that can be utilized by */
/*           the Platform Manager to read/write configuration data on the     */
/*           device.                                                          */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/23/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SETAPIH__
#define __SETAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

   /* The following definition defines the maximum length (in bytes)    */
   /* that an individual Line contained in the Config file can be.  Any */
   /* lines that are greater than this value will simply be truncated.  */
#define SETTINGS_CONFIG_FILE_MAXIMUM_LINE_LENGTH            (BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_LINE_LENGTH)

   /* The following definition defines the maximum length (in bytes     */
   /* including the NULL terminator character) that a Default INI File  */
   /* Directory can be set to.  This value is used with the             */
   /* SetPrivateProfileSearchPath() function.                           */
#define SETTINGS_CONFIG_FILE_MAXIMUM_DEFAULT_PATH_LENGTH    (BTPM_CONFIGURATION_SETTINGS_MAXIMUM_FILE_PATH_LENGTH)

   /* The following constants represent the Path Delimeters (in both    */
   /* Character and String representations of the Path Delimeter for the*/
   /* current platform.                                                 */
#define SETTINGS_CONFIG_FILE_PATH_DELIMETER_STRING          (BTPM_CONFIGURATION_SETTINGS_PATH_DELIMETER_STRING)
#define SETTINGS_CONFIG_FILE_PATH_DELIMETER_CHARACTER       (BTPM_CONFIGURATION_SETTINGS_PATH_DELIMETER_CHARACTER)

   /* The functions in this file operate on Config Initialization files */
   /* (INI) that were made popular by the Microsoft Windows Operating   */
   /* Systems.  An INI File is simply an ASCII Text file that contains  */
   /* unique Sections.  Each Section contains Keys, that contain        */
   /* information specific to the Section Key.  Section names in INI    */
   /* files are enclosed in the brackets ([]'s), and Keys are simply the*/
   /* Key Name followed by an equal sign (followed by the Key value).   */
   /* As an example, the file below contains two sections entitled 'This*/
   /* is Section 1', and 'This is Section 2', and each Section contains */
   /* a single Key.  Spaces are allowed in Section Names, however NO    */
   /* spaces are allowed in Key Names.                                  */
   /*                                                                   */
   /* <Beginning of File> <-- Note this line is not present in the File.*/
   /* [This is Section 1]                                               */
   /* Section1Key1=This is the value for Key 1 in Section 1.            */
   /*                                                                   */
   /* [This is Section 2]                                               */
   /* Section2Key1=This is the value for Key 1 in Section 2.            */
   /*                                                                   */
   /* <End of File> <-- Note this line is not present in the File.      */
   /*                                                                   */
   /* * NOTE * Comments can be present in INI files by having a         */
   /*          semi-colon (';') in front of the comment.  Multi-line    */
   /*          comments are possible, however, each individual line     */
   /*          * MUST * have a semi-colon preceding it.                 */
   /* * NOTE * Keys are unique for each section, so the same Key Name   */
   /*          can exist (legally) in different Sections.               */
   /* * NOTE * Duplicate Key Names (in the same Section) are NOT allowed*/
   /*          and the behaviour of this is undefined - Note for this   */
   /*          case to occur, the INI file name would have to have been */
   /*          formatted by some other means than the functions in this */
   /*          file.                                                    */
   /* * NOTE * Section and Key Names ARE NOT CASE SENSITIVE.            */
   /* * NOTE * The Entire Key Value *MUST* be present on the same line  */
   /*          the Key Name is listed on.  This means that there cannot */
   /*          be multiple line Keys.                                   */
   /*                                                                   */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Settings Module is            */
   /* initialized.                                                      */
typedef struct _tagSET_Initialization_Data_t
{
   char *InitialSearchPath;
   void *PlatformSpecificInitData;
} SET_Initialization_Data_t;

#define SET_INITIALIZATION_DATA_SIZE                        (sizeof(SET_Initialization_Data_t))

   /* The following function is provided to allow a mechanism to set    */
   /* the Default Path that is used to search for INI Configuration     */
   /* Files in the functions provided by this module.  This function    */
   /* exists to allow a mechanism for INI Files to be searched for in   */
   /* a specific location, as opposed to the current directory.  Note   */
   /* that this path is ONLY used when there is NO Path information     */
   /* specified in the INI Configuration File Name.  The search order   */
   /* for ALL Configuration Files is the following:                     */
   /*                                                                   */
   /*    1. The current working directory.                              */
   /*    2. The default Profile path.                                   */
   /*                                                                   */
   /* This function accepts as input a pointer to a NULL terminated     */
   /* ASCII string that specifies the fully qualified Path of the INI   */
   /* Search Directory.  This function returns a zero value if the new  */
   /* Search Path was successfully set, or a non-zero value if there    */
   /* was an error setting the specified directory.                     */
   /* * NOTE * The above order is ONLY searched if there is NO Path     */
   /*          information specified in the Configuration File Name.    */
   /* * NOTE * Calling this function is optional, and DOES NOT actually */
   /*          change the working directory of the program.             */
   /* * NOTE * If this function is NOT called (or not called            */
   /*          successfully, then ONLY the Current Working Directory    */
   /*          is searched.                                             */
   /* * NOTE * This function can be called with NULL as the first       */
   /*          parameter to clear/remove any default Search Paths.      */
BTPSAPI_DECLARATION int BTPSAPI SET_SetSearchPath(char *DefaultPath);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SET_SetSearchPath_t)(char *DefaultPath);
#endif

   /* The following function retrieves an Integer value from the        */
   /* specified INI File (with the specified Section Name and Key Name).*/
   /* This function accepts as its first parameter a pointer to a NULL  */
   /* terminated ASCII string that represents the Section Name in which */
   /* the Key to query is present in.  The second parameter to this     */
   /* function is a pointer to a NULL terminated ASCII string which     */
   /* specifies the Key Name of the Key to query (located in the        */
   /* specified Section by the first parameter).  The third parameter   */
   /* to this function specifies the value to return from this function */
   /* if the Key is not located, or an error occurs.  The final         */
   /* parameter to this function is a pointer to a NULL terminated      */
   /* ASCII String which represents the name of the Initialization File */
   /* to query.  This function returns the value of the Key if it was   */
   /* found in the specified Initialization File, or the Default Key    */
   /* Value if the Key was not found or an error occurred while trying  */
   /* to retrieve the specified Key.                                    */
   /* * NOTE * The search order for determining the correct INI File    */
   /*          is as follows (ONLY IF THE 'FileName' PARAMETER DOES NOT */
   /*          CONTAIN ANY PATH INFORMATION):                           */
   /*                                                                   */
   /*             1. The current directory (working directory of the    */
   /*                program).                                          */
   /*             2. The Private Profile Search Path (ONLY if this path */
   /*                was successfully set via a call to the             */
   /*                SetPrivateProfileSearchPath() function).           */
   /* * NOTE * The 'SectionName', 'KeyName', and 'FileName' parameters  */
   /*          *MUST* have a value (i.e. non-NULL) and must contain     */
   /*          valid information (i.e. the length of each is greater    */
   /*          than zero).                                              */
BTPSAPI_DECLARATION unsigned int BTPSAPI SET_ReadInteger(char *SectionName, char *KeyName, unsigned int DefaultValue, char *FileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_SET_ReadInteger_t)(char *SectionName, char *KeyName, unsigned int DefaultValue, char *FileName);
#endif

   /* The following function is responsible for writing the specified   */
   /* Key Value to the specified INI File (with the specified Section   */
   /* Name and Key Name) in ASCII (Integer) form.  This function accepts*/
   /* as its first parameter a pointer to a NULL terminated ASCII string*/
   /* that represents the Section Name in which the Key to write is     */
   /* present in.  The second parameter to this function is a pointer to*/
   /* a NULL terminated ASCII string which specifies the Key Name of the*/
   /* Key to write (located in the specified Section by the first       */
   /* parameter).  The third parameter to this function is an integer   */
   /* value that represents the new value of the specified Key.  The    */
   /* final parameter to this function is a pointer to a NULL terminated*/
   /* ASCII String which represents the name of the Initialization File */
   /* to write the specified Key Value into.  This function returns a   */
   /* non-zero value if the specified Key value was written             */
   /* successfully, or a value of zero if an error occurred.            */
   /* * NOTE * The search order for determining where the INI file will */
   /*          be written is as follows (ONLY IF THE 'FileName'         */
   /*          PARAMETER DOES NOT CONTAIN ANY PATH INFORMATION):        */
   /*                                                                   */
   /*             1. The current directory (working directory of the    */
   /*                program - ONLY IF A CONFIGURATION FILE OF THE      */
   /*                SPECIFIED NAME EXISTS).                            */
   /*             2. The Private Profile Search Path (ONLY if this path */
   /*                was successfully set via a call to the             */
   /*                SetPrivateProfileSearchPath() function).           */
   /* * NOTE * The 'SectionName', 'KeyName', and 'FileName' parameters  */
   /*          *MUST* have a value (i.e. non-NULL) and must contain     */
   /*          valid information (i.e. the length of each is greater    */
   /*          than zero).                                              */
   /* * NOTE * Although a Carriage Return/Line Feed will be inserted at */
   /*          the end of the Key Value (because each Key is on a single*/
   /*          line), this value SHOULD NOT be present at the end of    */
   /*          the line being written.  In fact, New line characters    */
   /*          should NOT be specified in Key Value or this function    */
   /*          will fail.                                               */
BTPSAPI_DECLARATION int BTPSAPI SET_WriteInteger(char *SectionName, char *KeyName, unsigned int Value, char *FileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SET_WriteInteger_t)(char *SectionName, char *KeyName, unsigned int Value, char *FileName);
#endif

   /* The following function retrieves the Key value from the specified */
   /* INI File (with the specified Section Name and Key Name) in ASCII  */
   /* form.  This function accepts as its first parameter a pointer to  */
   /* a NULL terminated ASCII string that represents the Section Name   */
   /* in which the Key to query is present in.  The second parameter to */
   /* this function is a pointer to a NULL terminated ASCII string      */
   /* which specifies the Key Name of the Key to query (located in the  */
   /* specified Section by the first parameter).  The third parameter   */
   /* to this function is a pointer to a NULL terminated ASCII string   */
   /* that represents the value of the Key that is to be returned from  */
   /* this function if the Key is not located, or an error occurs.      */
   /* The fourth and fifth parameters specify a pointer to a Memory     */
   /* Buffer and the size (in Bytes) of the Memory Buffer that the Key  */
   /* value is to be returned into, respectively.  The final parameter  */
   /* to this function is a pointer to a NULL terminated ASCII String   */
   /* which represents the name of the Initialization File to query.    */
   /* This function returns the number of bytes that were copied into   */
   /* the specified Buffer (not including the NULL terminating          */
   /* character) on success, or zero if there was an error.  Note that  */
   /* this function copies the value of the Key if it was found in the  */
   /* specified Initialization File, or the Default Key Value if the    */
   /* Key was not found or an error occurred while trying to retrieve   */
   /* the specified Key.                                                */
   /* * NOTE * The search order for determining the correct INI File    */
   /*          is as follows (ONLY IF THE 'FileName' PARAMETER DOES NOT */
   /*          CONTAIN ANY PATH INFORMATION):                           */
   /*                                                                   */
   /*             1. The current directory (working directory of the    */
   /*                program).                                          */
   /*             2. The Private Profile Search Path (ONLY if this path */
   /*                was successfully set via a call to the             */
   /*                SetPrivateProfileSearchPath() function).           */
   /* * NOTE * The 'SectionName', 'KeyName', and 'FileName' parameters  */
   /*          *MUST* have a value (i.e. non-NULL) and must contain     */
   /*          valid information (i.e. the length of each is greater    */
   /*          than zero).                                              */
   /* * NOTE * The 'DefaultValue' parameter can be NULL, indicating     */
   /*          that there is NO Data to be copied into the specified    */
   /*          Buffer.                                                  */
   /* * NOTE * The 'Buffer' and 'BufferSize' parameters *MUST* be       */
   /*          specified (i.e. not NULL or zero, respectively), and     */
   /*          the size of the Buffer that Buffer points to *MUST* be   */
   /*          AT LEAST the number of bytes that is specified by the    */
   /*          'BufferSize' parameter.                                  */
   /* * NOTE * The NULL terminated ASCII Buffer that is returned from   */
   /*          this function WILL NOT INCLUDE ANY CARRIAGE RETURN       */
   /*          AND/OR LINE FEED CHARACTERS.                             */
BTPSAPI_DECLARATION unsigned int BTPSAPI SET_ReadString(char *SectionName, char *KeyName, char *DefaultValue, char *Buffer, unsigned int BufferSize, char *FileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_SET_ReadString_t)(char *SectionName, char *KeyName, char *DefaultValue, char *Buffer, unsigned int BufferSize, char *FileName);
#endif

   /* The following function is responsible for writing the specified   */
   /* Key Value to the specified INI File (with the specified Section   */
   /* Name and Key Name) in ASCII form.  This function accepts as its   */
   /* first parameter a pointer to a NULL terminated ASCII string that  */
   /* represents the Section Name in which the Key to write is present  */
   /* in.  The second parameter to this function is a pointer to a NULL */
   /* terminated ASCII string which specifies the Key Name of the Key   */
   /* to write (located in the specified Section by the first           */
   /* parameter).  The third parameter to this function is a pointer to */
   /* a NULL terminated ASCII string that represents the new value of   */
   /* the specified Key.  The final parameter to this function is a     */
   /* pointer to a NULL terminated ASCII String which represents the    */
   /* name of the Initialization File to write the specified Key Value  */
   /* into.  This function returns a non-zero value if the specified    */
   /* Key value was written successfully, or a value of zero if an      */
   /* error occurred.                                                   */
   /* * NOTE * Specifying a Buffer of NULL will delete the specified Key*/
   /*          from the INI file (if present).                          */
   /* * NOTE * The search order for determining where the INI file will */
   /*          be written is as follows (ONLY IF THE 'FileName'         */
   /*          PARAMETER DOES NOT CONTAIN ANY PATH INFORMATION):        */
   /*                                                                   */
   /*             1. The current directory (working directory of the    */
   /*                program - ONLY IF A CONFIGURATION FILE OF THE      */
   /*                SPECIFIED NAME EXISTS).                            */
   /*             2. The Private Profile Search Path (ONLY if this path */
   /*                was successfully set via a call to the             */
   /*                SetPrivateProfileSearchPath() function).           */
   /* * NOTE * The 'SectionName', 'KeyName', and 'FileName' parameters  */
   /*          *MUST* have a value (i.e. non-NULL) and must contain     */
   /*          valid information (i.e. the length of each is greater    */
   /*          than zero).                                              */
   /* * NOTE * Although a Carriage Return/Line Feed will be inserted at */
   /*          the end of the Key Value (because each Key is on a single*/
   /*          line), this value SHOULD NOT be present at the end of    */
   /*          the line being written.  In fact, New line characters    */
   /*          should NOT be specified in Key Value or this function    */
   /*          will fail.                                               */
BTPSAPI_DECLARATION int BTPSAPI SET_WriteString(char *SectionName, char *KeyName, char *Buffer, char *FileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SET_WriteString_t)(char *SectionName, char *KeyName, char *Buffer, char *FileName);
#endif

   /* The following function retrieves the Key value from the specified */
   /* INI File (with the specified Section Name and Key Name) in Binary */
   /* form.  This function accepts as its first parameter a pointer to a*/
   /* NULL terminated ASCII string that represents the Section Name in  */
   /* which the Key to query is present in.  The second parameter to    */
   /* this function is a pointer to a NULL terminated ASCII string which*/
   /* specifies the Key Name of the Key to query (located in the        */
   /* specified Section by the first parameter).  The third and fourth  */
   /* parameters specify a pointer to a Memory Buffer and the size (in  */
   /* Bytes) of the Memory Buffer that the Key value is to be returned  */
   /* into, respectively.  The final parameter to this function is a    */
   /* pointer to a NULL terminated ASCII String which represents the    */
   /* name of the Initialization File to query.  This function returns  */
   /* the number of bytes that were copied into the specified Buffer on */
   /* success, or zero if there was an error.                           */
   /* * NOTE * If both the Buffer and BufferSize parameters are NULL    */
   /*          and zero (respectively), then this function will return  */
   /*          the length (in bytes) of the binary data that is         */
   /*          present (i.e. it can be used to determine the size (in   */
   /*          bytes of the buffer required to hold the data that is    */
   /*          stored).                                                 */
   /* * NOTE * The search order for determining the correct INI File    */
   /*          is as follows (ONLY IF THE 'FileName' PARAMETER DOES NOT */
   /*          CONTAIN ANY PATH INFORMATION):                           */
   /*                                                                   */
   /*             1. The current directory (working directory of the    */
   /*                program).                                          */
   /*             2. The Private Profile Search Path (ONLY if this path */
   /*                was successfully set via a call to the             */
   /*                SetPrivateProfileSearchPath() function).           */
   /* * NOTE * The 'SectionName', 'KeyName', and 'FileName' parameters  */
   /*          *MUST* have a value (i.e. non-NULL) and must contain     */
   /*          valid information (i.e. the length of each is greater    */
   /*          than zero).                                              */
   /* * NOTE * The 'DefaultValue' parameter can be NULL, indicating     */
   /*          that there is NO Data to be copied into the specified    */
   /*          Buffer.                                                  */
   /* * NOTE * The 'Buffer' and 'BufferSize' parameters *MUST* be       */
   /*          specified (i.e. not NULL or zero, respectively), and     */
   /*          the size of the Buffer that Buffer points to *MUST* be   */
   /*          AT LEAST the number of bytes that is specified by the    */
   /*          'BufferSize' parameter.                                  */
BTPSAPI_DECLARATION unsigned int BTPSAPI SET_ReadBinaryData(char *SectionName, char *KeyName, unsigned char *Buffer, unsigned int BufferSize, char *FileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_SET_ReadBinaryData_t)(char *SectionName, char *KeyName, unsigned char *Buffer, unsigned int BufferSize, char *FileName);
#endif

   /* The following function is responsible for writing the specified   */
   /* Key Value to the specified INI File (with the specified Section   */
   /* Name and Key Name) in Binary form.  This function accepts as its  */
   /* first parameter a pointer to a NULL terminated ASCII string that  */
   /* represents the Section Name in which the Key to write is present  */
   /* in.  The second parameter to this function is a pointer to a NULL */
   /* terminated ASCII string which specifies the Key Name of the Key to*/
   /* write (located in the specified Section by the first parameter).  */
   /* The third parameter to this function is a pointer to a buffer     */
   /* which contains the binary data of the specified Key.  The fourth  */
   /* parameter specifies the length (in bytes) of the binary data that */
   /* is pointed to by the third parameter.  The final parameter to this*/
   /* function is a pointer to a NULL terminated ASCII String which     */
   /* represents the name of the Initialization File to write the       */
   /* specified Key Value into.  This function returns a non-zero value */
   /* if the specified Key value was written successfully, or a value of*/
   /* zero if an error occurred.                                        */
   /* * NOTE * Specifying a Buffer of NULL AND a BufferSize of zero will*/
   /*          delete the specified Key from the INI file (if present). */
   /* * NOTE * The search order for determining where the INI file will */
   /*          be written is as follows (ONLY IF THE 'FileName'         */
   /*          PARAMETER DOES NOT CONTAIN ANY PATH INFORMATION):        */
   /*                                                                   */
   /*             1. The current directory (working directory of the    */
   /*                program - ONLY IF A CONFIGURATION FILE OF THE      */
   /*                SPECIFIED NAME EXISTS).                            */
   /*             2. The Private Profile Search Path (ONLY if this path */
   /*                was successfully set via a call to the             */
   /*                SetPrivateProfileSearchPath() function).           */
   /* * NOTE * The 'SectionName', 'KeyName', and 'FileName' parameters  */
   /*          *MUST* have a value (i.e. non-NULL) and must contain     */
   /*          valid information (i.e. the length of each is greater    */
   /*          than zero).                                              */
BTPSAPI_DECLARATION int BTPSAPI SET_WriteBinaryData(char *SectionName, char *KeyName, unsigned char *Buffer, unsigned int BufferSize, char *FileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SET_WriteBinaryData_t)(char *SectionName, char *KeyName, unsigned char *Buffer, unsigned int BufferSize, char *FileName);
#endif

#endif

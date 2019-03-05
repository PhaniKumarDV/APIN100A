/*****< encoder.c >************************************************************/
/*      Copyright 2003 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ENCODER - Stonestreet One Subband Codec (SBC) Encoder sample application  */
/*            for Linux Implementation.                                       */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/13/03  R. Sledge      Initial creation.                               */
/*   07/01/06  D. Lange       Updated with new API based on Bit Rate.         */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SS1SBC.h"                    /* Subband Codec Prototypes/Constants. */

   /* Constant that defines the maximum allowable path and file name    */
   /* size (used to cap buffers).                                       */
#define MAXIMUM_PATH_LENGTH                                       256

   /* Constants that represent the command line parameter arguments.    */
#define SAMPLING_FREQUENCY_DELIMITER                              "-SF"
#define SAMPLING_FREQUENCY_16_KHZ                                 "16"
#define SAMPLING_FREQUENCY_32_KHZ                                 "32"
#define SAMPLING_FREQUENCY_441_KHZ                                "441"
#define SAMPLING_FREQUENCY_48_KHZ                                 "48"

#define CHANNEL_MODE_ARGUMENT_DELIMITER                           "-CM"
#define CHANNEL_MODE_ARGUMENT_MONO                                "M"
#define CHANNEL_MODE_ARGUMENT_DUAL_CHANNEL                        "D"
#define CHANNEL_MODE_ARGUMENT_STEREO                              "S"
#define CHANNEL_MODE_ARGUMENT_JOINT_STEREO                        "J"

#define BLOCK_LENGTH_ARGUMENT_DELIMITER                           "-BL"
#define BLOCK_LENGTH_ARGUMENT_FOUR                                "4"
#define BLOCK_LENGTH_ARGUMENT_EIGHT                               "8"
#define BLOCK_LENGTH_ARGUMENT_TWELVE                              "12"
#define BLOCK_LENGTH_ARGUMENT_FIFTEEN                             "15"
#define BLOCK_LENGTH_ARGUMENT_SIXTEEN                             "16"

#define SUBBANDS_DELIMITER                                        "-SB"
#define SUBBANDS_FOUR                                             "4"
#define SUBBANDS_EIGHT                                            "8"

#define ALLOCATION_METHOD_DELIMITER                               "-AM"
#define ALLOCATION_METHOD_SNR                                     "S"
#define ALLOCATION_METHOD_LOUDNESS                                "L"

#define BIT_RATE_DELIMITER                                        "-BR"

#define SKIP_WAVE_FILE_HEADER_DELIMITER                           "-W"

#define INPUT_FILE_DELIMITER                                      "-I"
#define OUTPUT_FILE_DELIMITER                                     "-O"

   /* Constant that defines the number of expected input parameters.    */
#define NUMBER_REQUIRED_INPUT_PARAMETERS                          (8)

   /* Constants that define the Input Buffer Sizes for encoding the     */
   /* input data.                                                       */
#define READ_BUFFER_SIZE                                          (4096)

   /* The following constants represent offsets and constants relating  */
   /* to the WAV Header.  These constants should not be changed.        */
#define MINIMUM_WAVE_FILE_HEADER_BUFFER_LENGTH                    (44)

#define WAVE_FILE_HEADER_CHUNK_SIZE_OFFSET                        (4)

#define WAVE_FILE_HEADER_SUB_CHUNK_1_ID_OFFSET                    (12)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_SIZE_OFFSET                  (16)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_AUDIO_FORMAT_OFFSET          (20)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_NUMBER_CHANNELS_OFFSET       (22)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_SAMPLE_RATE_OFFSET           (24)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_BYTE_RATE_OFFSET             (28)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_BLOCK_ALIGN_OFFSET           (32)
#define WAVE_FILE_HEADER_SUB_CHUNK_1_BITS_PER_SAMPLE_OFFSET       (34)

#define WAVE_FILE_HEADER_SUB_CHUNK_2_ID_OFFSET                    (36)
#define WAVE_FILE_HEADER_SUB_CHUNK_2_SIZE_OFFSET                  (40)

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static char InputFilePath[MAXIMUM_PATH_LENGTH+1];     /* Variable used to hold*/
                                                      /* the path of the input*/
                                                      /* file that is to be   */
                                                      /* encoded.             */

static char OutputFilePath[MAXIMUM_PATH_LENGTH+1];    /* Variable used to hold*/
                                                      /* the path of the      */
                                                      /* output file that was */
                                                      /* encoded from the     */
                                                      /* input file.          */

   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   int                           ret_val;
   int                           ArgIndex;
   unsigned long                 BitRateValue;
   time_t                        StartTime;
   time_t                        EndTime;
   FILE                         *InputFileHandle;
   FILE                         *OutputFileHandle;
   Boolean_t                     Error;
   Boolean_t                     SkipWAVHeader;
   Encoder_t                     EncoderHandle;
   unsigned int                  Index;
   unsigned int                  Index2;
   unsigned int                  NumberParametersFound;
   SBC_Encode_Data_t             EncodeData;
   SBC_Encode_Configuration_t    EncodeConfiguration;
   SBC_Encode_Bit_Stream_Data_t  EncodedBitStreamData;

   static unsigned short         TempData[READ_BUFFER_SIZE];
   unsigned int                  TempDataLength;
   unsigned int                  TempDataSize;

   unsigned char                 EncodedBitStream[8192];

   static unsigned short         LeftChannelData[READ_BUFFER_SIZE];
   static unsigned short         RightChannelData[READ_BUFFER_SIZE/2];

   TempDataSize = sizeof(TempData)/sizeof(unsigned short);

   /* Flag that we are initially NOT to skip any WAV Header.            */
   SkipWAVHeader = FALSE;

   /* Initialize the command line parsing variables.                    */
   NumberParametersFound = 0;
   Error                 = FALSE;

   /* Next, loop through the command line arguements and pick out the   */
   /* commands we are interested in.                                    */
   for(ArgIndex = 1; (ArgIndex < argc) && (!Error); ArgIndex++)
   {
      /* Check to see if this is the Sampling Frequency Argument.       */
      if((strlen(argv[ArgIndex]) >= strlen(SAMPLING_FREQUENCY_DELIMITER)) && (!strncasecmp(argv[ArgIndex], SAMPLING_FREQUENCY_DELIMITER, strlen(SAMPLING_FREQUENCY_DELIMITER))))
      {
         /* Verify that a parameter has been entered                    */
         if((strlen(argv[ArgIndex]) == strlen(SAMPLING_FREQUENCY_DELIMITER)) && ((ArgIndex+1) < argc))
         {
            Index = 0;
            ArgIndex++;
         }
         else
            Index = strlen(SAMPLING_FREQUENCY_DELIMITER);

         /* This is the Sampling Frequency Argument, check to make sure */
         /* that it is valid.                                           */
         if((strlen(&(argv[ArgIndex][Index])) >= strlen(SAMPLING_FREQUENCY_16_KHZ)) && (!strncasecmp(&(argv[ArgIndex][Index]), SAMPLING_FREQUENCY_16_KHZ, strlen(SAMPLING_FREQUENCY_16_KHZ))))
         {
            EncodeConfiguration.SamplingFrequency = sf16kHz;
            NumberParametersFound++;
         }
         else
         {
            if((strlen(&(argv[ArgIndex][Index])) >= strlen(SAMPLING_FREQUENCY_32_KHZ)) && (!strncasecmp(&(argv[ArgIndex][Index]), SAMPLING_FREQUENCY_32_KHZ, strlen(SAMPLING_FREQUENCY_32_KHZ))))
            {
               EncodeConfiguration.SamplingFrequency = sf32kHz;
               NumberParametersFound++;
            }
            else
            {
               if((strlen(&(argv[ArgIndex][Index])) >= strlen(SAMPLING_FREQUENCY_441_KHZ)) && (!strncasecmp(&(argv[ArgIndex][Index]), SAMPLING_FREQUENCY_441_KHZ, strlen(SAMPLING_FREQUENCY_441_KHZ))))
               {
                  EncodeConfiguration.SamplingFrequency = sf441kHz;
                  NumberParametersFound++;
               }
               else
               {
                  if((strlen(&(argv[ArgIndex][Index])) >= strlen(SAMPLING_FREQUENCY_48_KHZ)) && (!strncasecmp(&(argv[ArgIndex][Index]), SAMPLING_FREQUENCY_48_KHZ, strlen(SAMPLING_FREQUENCY_48_KHZ))))
                  {
                     EncodeConfiguration.SamplingFrequency = sf48kHz;
                     NumberParametersFound++;
                  }
                  else
                     Error = TRUE;
               }
            }
         }

         if(Error)
            printf("\r\nInvalid Sample Frequency. Expected: -sf16, -sf32, -s441, -sf48\r\n\n");
      }
      else
      {
         /* Check to see if this is the Channel Mode Argument.          */
         if((strlen(argv[ArgIndex]) >= strlen(CHANNEL_MODE_ARGUMENT_DELIMITER)) && (!strncasecmp(argv[ArgIndex], CHANNEL_MODE_ARGUMENT_DELIMITER, strlen(CHANNEL_MODE_ARGUMENT_DELIMITER))))
         {
            /* Verify that a parameter has been entered                 */
            if((strlen(argv[ArgIndex]) == strlen(CHANNEL_MODE_ARGUMENT_DELIMITER)) && ((ArgIndex+1) < argc))
            {
               Index = 0;
               ArgIndex++;
            }
            else
               Index = strlen(CHANNEL_MODE_ARGUMENT_DELIMITER);

            /* It is the Channel Mode Argument.  Now make sure that it  */
            /* specifies a valid Channel Mode.                          */
            if((strlen(&(argv[ArgIndex][Index])) >= strlen(CHANNEL_MODE_ARGUMENT_MONO)) && (!strncasecmp(&(argv[ArgIndex][Index]), CHANNEL_MODE_ARGUMENT_MONO, strlen(CHANNEL_MODE_ARGUMENT_MONO))))
            {
               EncodeConfiguration.ChannelMode = cmMono;
               NumberParametersFound++;
            }
            else
            {
               if((strlen(&(argv[ArgIndex][Index])) >= strlen(CHANNEL_MODE_ARGUMENT_DUAL_CHANNEL)) && (!strncasecmp(&(argv[ArgIndex][Index]), CHANNEL_MODE_ARGUMENT_DUAL_CHANNEL, strlen(CHANNEL_MODE_ARGUMENT_DUAL_CHANNEL))))
               {
                  EncodeConfiguration.ChannelMode = cmDualChannel;
                  NumberParametersFound++;
               }
               else
               {
                  if((strlen(&(argv[ArgIndex][Index])) >= strlen(CHANNEL_MODE_ARGUMENT_STEREO)) && (!strncasecmp(&(argv[ArgIndex][Index]), CHANNEL_MODE_ARGUMENT_STEREO, strlen(CHANNEL_MODE_ARGUMENT_STEREO))))
                  {
                     EncodeConfiguration.ChannelMode = cmStereo;
                     NumberParametersFound++;
                  }
                  else
                  {
                     if((strlen(&(argv[ArgIndex][Index])) >= strlen(CHANNEL_MODE_ARGUMENT_JOINT_STEREO)) && (!strncasecmp(&(argv[ArgIndex][Index]), CHANNEL_MODE_ARGUMENT_JOINT_STEREO, strlen(CHANNEL_MODE_ARGUMENT_JOINT_STEREO))))
                     {
                        EncodeConfiguration.ChannelMode = cmJointStereo;
                        NumberParametersFound++;
                     }
                     else
                        Error = TRUE;
                  }
               }
            }

            if(Error)
               printf("\r\nInvalid Channel Mode. Expected: -cmm, -cmd, -cms, -cmj\r\n\n");
         }
         else
         {
            /* Check to see if this is the Block Length Argument.       */
            if((strlen(argv[ArgIndex]) >= strlen(BLOCK_LENGTH_ARGUMENT_DELIMITER)) && (!strncasecmp(argv[ArgIndex], BLOCK_LENGTH_ARGUMENT_DELIMITER, strlen(BLOCK_LENGTH_ARGUMENT_DELIMITER))))
            {
               /* Verify that a parameter has been entered              */
               if((strlen(argv[ArgIndex]) == strlen(BLOCK_LENGTH_ARGUMENT_DELIMITER)) && ((ArgIndex+1) < argc))
               {
                  Index = 0;
                  ArgIndex++;
               }
               else
                  Index = strlen(CHANNEL_MODE_ARGUMENT_DELIMITER);

               /* This is the Block Length Argument.  Check to see if   */
               /* the value is correct.                                 */
               if((strlen(&(argv[ArgIndex][Index])) >= strlen(BLOCK_LENGTH_ARGUMENT_FOUR)) && (!strncasecmp(&(argv[ArgIndex][Index]), BLOCK_LENGTH_ARGUMENT_FOUR, strlen(BLOCK_LENGTH_ARGUMENT_FOUR))))
               {
                  EncodeConfiguration.BlockSize = bsFour;
                  NumberParametersFound++;
               }
               else
               {
                  if((strlen(&(argv[ArgIndex][Index])) >= strlen(BLOCK_LENGTH_ARGUMENT_EIGHT)) && (!strncasecmp(&(argv[ArgIndex][Index]), BLOCK_LENGTH_ARGUMENT_EIGHT, strlen(BLOCK_LENGTH_ARGUMENT_EIGHT))))
                  {
                     EncodeConfiguration.BlockSize = bsEight;
                     NumberParametersFound++;
                  }
                  else
                  {
                     if((strlen(&(argv[ArgIndex][Index])) >= strlen(BLOCK_LENGTH_ARGUMENT_TWELVE)) && (!strncasecmp(&(argv[ArgIndex][Index]), BLOCK_LENGTH_ARGUMENT_TWELVE, strlen(BLOCK_LENGTH_ARGUMENT_TWELVE))))
                     {
                        EncodeConfiguration.BlockSize = bsTwelve;
                        NumberParametersFound++;
                     }
                     else
                     {
                        if((strlen(&(argv[ArgIndex][Index])) >= strlen(BLOCK_LENGTH_ARGUMENT_SIXTEEN)) && (!strncasecmp(&(argv[ArgIndex][Index]), BLOCK_LENGTH_ARGUMENT_SIXTEEN, strlen(BLOCK_LENGTH_ARGUMENT_SIXTEEN))))
                        {
                           EncodeConfiguration.BlockSize = bsSixteen;
                           NumberParametersFound++;
                        }
                        else
                        {
                           if((strlen(&(argv[ArgIndex][Index])) >= strlen(BLOCK_LENGTH_ARGUMENT_FIFTEEN)) && (!strncasecmp(&(argv[ArgIndex][Index]), BLOCK_LENGTH_ARGUMENT_FIFTEEN, strlen(BLOCK_LENGTH_ARGUMENT_FIFTEEN))))
                           {
                              EncodeConfiguration.BlockSize = bsFifteen;
                              NumberParametersFound++;
                           }
                           else
                              Error = TRUE;
                        }
                     }
                  }
               }

               if(Error)
                  printf("\r\nInvalid Block Length. Expected: -bl4, -bl8, -bl12, -bl15, -bl16\r\n\n");
            }
            else
            {
               /* Check to see if the number of Subbands argument.      */
               if((strlen(argv[ArgIndex]) >= strlen(SUBBANDS_DELIMITER)) && (!strncasecmp(argv[ArgIndex], SUBBANDS_DELIMITER, strlen(SUBBANDS_DELIMITER))))
               {
                  /* Verify that a parameter has been entered           */
                  if((strlen(argv[ArgIndex]) == strlen(SUBBANDS_DELIMITER)) && ((ArgIndex+1) < argc))
                  {
                     Index = 0;
                     ArgIndex++;
                  }
                  else
                     Index = strlen(CHANNEL_MODE_ARGUMENT_DELIMITER);

                  /* This is the number of Subbands argument, so now    */
                  /* check to make sure it is valid.                    */
                  if((strlen(&(argv[ArgIndex][Index])) >= strlen(SUBBANDS_FOUR)) && (!strncasecmp(&(argv[ArgIndex][Index]), SUBBANDS_FOUR, strlen(SUBBANDS_FOUR))))
                  {
                     EncodeConfiguration.Subbands = sbFour;
                     NumberParametersFound++;
                  }
                  else
                  {
                     if((strlen(&(argv[ArgIndex][Index])) >= strlen(SUBBANDS_EIGHT)) && (!strncasecmp(&(argv[ArgIndex][Index]), SUBBANDS_EIGHT, strlen(SUBBANDS_EIGHT))))
                     {
                        EncodeConfiguration.Subbands = sbEight;
                        NumberParametersFound++;
                     }
                     else
                        Error = TRUE;
                  }

                  if(Error)
                     printf("\r\nInvalid Sub Bands. Expected: -sb4, -sb8\r\n\n");
               }
               else
               {
                  /* Check to see if this is the Allocation Method      */
                  /* Argument.                                          */
                  if((strlen(argv[ArgIndex]) >= strlen(ALLOCATION_METHOD_DELIMITER)) && (!strncasecmp(argv[ArgIndex], ALLOCATION_METHOD_DELIMITER, strlen(ALLOCATION_METHOD_DELIMITER))))
                  {
                     /* Verify that a parameter has been entered        */
                     if((strlen(argv[ArgIndex]) == strlen(ALLOCATION_METHOD_DELIMITER)) && ((ArgIndex+1) < argc))
                     {
                        Index = 0;
                        ArgIndex++;
                     }
                     else
                        Index = strlen(CHANNEL_MODE_ARGUMENT_DELIMITER);

                     /* This is the Allocation Method Argument, so now  */
                     /* determine the type of Allocation Method.        */
                     if((strlen(&(argv[ArgIndex][Index])) >= strlen(ALLOCATION_METHOD_SNR)) && (!strncasecmp(&(argv[ArgIndex][Index]), ALLOCATION_METHOD_SNR, strlen(ALLOCATION_METHOD_SNR))))
                     {
                        EncodeConfiguration.AllocationMethod  = amSNR;
                        NumberParametersFound++;
                     }
                     else
                     {
                        if((strlen(&(argv[ArgIndex][Index])) >= strlen(ALLOCATION_METHOD_LOUDNESS)) && (!strncasecmp(&(argv[ArgIndex][Index]), ALLOCATION_METHOD_LOUDNESS, strlen(ALLOCATION_METHOD_LOUDNESS))))
                        {
                           EncodeConfiguration.AllocationMethod  = amLoudness;
                           NumberParametersFound++;
                        }
                        else
                           Error = TRUE;
                     }

                     if(Error)
                        printf("\r\nInvalid Allocation Method. Expected: -ams, -aml\r\n\n");
                  }
                  else
                  {
                     /* Check to see if this is the Bit Rate Argument.  */
                     if((strlen(argv[ArgIndex]) >= strlen(BIT_RATE_DELIMITER)) && (!strncasecmp(argv[ArgIndex], BIT_RATE_DELIMITER, strlen(BIT_RATE_DELIMITER))))
                     {
                        /* Verify that a parameter has been entered     */
                        if((strlen(argv[ArgIndex]) == strlen(BIT_RATE_DELIMITER)) && ((ArgIndex+1) < argc))
                        {
                           Index = 0;
                           ArgIndex++;
                        }
                        else
                           Index = strlen(CHANNEL_MODE_ARGUMENT_DELIMITER);

                        /* This is the Bit Rate Argument, so determine  */
                        /* the specified Bit Rate.                      */
                        if((strlen(&(argv[ArgIndex][Index])) >= 2) && (argv[ArgIndex][0] == '0') && ((argv[ArgIndex][1] == 'x') || (argv[ArgIndex][1] == 'X')))
                           sscanf(&(argv[ArgIndex][Index]), "0x%08lX", &BitRateValue);
                        else
                           sscanf(&(argv[ArgIndex][Index]), "%lu", &BitRateValue);

                        EncodeConfiguration.MaximumBitRate = (unsigned long)BitRateValue;
                        NumberParametersFound++;
                     }
                     else
                     {
                        /* Check to see if this is the Skip WAV File    */
                        /* Header Argument.                             */
                        if((strlen(argv[ArgIndex]) >= strlen(SKIP_WAVE_FILE_HEADER_DELIMITER)) && (!strncasecmp(argv[ArgIndex], SKIP_WAVE_FILE_HEADER_DELIMITER, strlen(SKIP_WAVE_FILE_HEADER_DELIMITER))))
                        {
                           /* Because this is an optional parameter we  */
                           /* do not need to count it as a Parameter    */
                           /* that was found.                           */
                           SkipWAVHeader = TRUE;
                        }
                        else
                        {
                           /* Check to see if this is the Input File    */
                           /* Name Argument.                            */
                           if((strlen(argv[ArgIndex]) >= strlen(INPUT_FILE_DELIMITER)) && (!strncasecmp(argv[ArgIndex], INPUT_FILE_DELIMITER, strlen(INPUT_FILE_DELIMITER))))
                           {
                              /* Verify that a parameter has been       */
                              /* entered                                */
                              if((strlen(argv[ArgIndex]) == strlen(INPUT_FILE_DELIMITER)) && ((ArgIndex+1) < argc))
                              {
                                 Index = 0;
                                 ArgIndex++;
                              }
                              else
                                 Index = strlen(INPUT_FILE_DELIMITER);

                              /* Input File Name specified so note the  */
                              /* File Name.                             */
                              strncpy(InputFilePath, &(argv[ArgIndex][Index]), (strlen(&(argv[ArgIndex][Index])) > sizeof(InputFilePath)?(sizeof(InputFilePath)):(strlen(&(argv[ArgIndex][Index])))));

                              NumberParametersFound++;
                           }
                           else
                           {
                              /* Check to see if this is the Output File*/
                              /* Name Argument.                         */
                              if((strlen(argv[ArgIndex]) >= strlen(OUTPUT_FILE_DELIMITER)) && (!strncasecmp(argv[ArgIndex], OUTPUT_FILE_DELIMITER, strlen(OUTPUT_FILE_DELIMITER))))
                              {
                                 /* Verify that a parameter has been    */
                                 /* entered                             */
                                 if((strlen(argv[ArgIndex]) == strlen(OUTPUT_FILE_DELIMITER)) && ((ArgIndex+1) < argc))
                                 {
                                    Index = 0;
                                    ArgIndex++;
                                 }
                                 else
                                    Index = strlen(OUTPUT_FILE_DELIMITER);

                                 /* Output File Name specified so note  */
                                 /* the File Name.                      */
                                 strncpy(OutputFilePath, &(argv[ArgIndex][Index]), (strlen(&(argv[ArgIndex][Index])) > sizeof(OutputFilePath)?(sizeof(OutputFilePath)):(strlen(&(argv[ArgIndex][Index])))));

                                 NumberParametersFound++;
                              }
                              else
                              {
                                 Error = TRUE;
                                 printf("\r\nInvalid Options %s.\r\n\n", argv[ArgIndex]);
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }


   /* Next, let's make sure that the command line information was parsed*/
   /* successfully.                                                     */
   if((!Error) && (NumberParametersFound >= NUMBER_REQUIRED_INPUT_PARAMETERS))
   {
      /* Information was successful, now let's attempt to open the Input*/
      /* File.                                                          */
      if((InputFileHandle = fopen(InputFilePath, "rb")) != NULL)
      {
         /* Input file has been opened, now lets open the output file   */
         /* that will contain the encoded output SBC data.              */
         if((OutputFileHandle = fopen(OutputFilePath, "wb")) != NULL)
         {
            /* If we have been instructed to skip a WAV File Header then*/
            /* we need to seek past the WAV File Header.                */
            if(SkipWAVHeader)
               fseek(InputFileHandle, MINIMUM_WAVE_FILE_HEADER_BUFFER_LENGTH, SEEK_SET);

            /* Note the start time (just so we can dispaly it to the    */
            /* user).                                                   */
            time(&StartTime);

            /* Now, we are ready to start encoding.  First, let's       */
            /* initialize the Encoder.                                  */
            if((EncoderHandle = SBC_Initialize_Encoder(&EncodeConfiguration)) != NULL)
            {
               EncodedBitStreamData.BitStreamDataSize   = sizeof(EncodedBitStream);
               EncodedBitStreamData.BitStreamDataLength = 0;
               EncodedBitStreamData.BitStreamDataPtr    = EncodedBitStream;

               /* Encoder has been initialized, so now let's read all   */
               /* available data from the Input File.                   */
               while((TempDataLength = fread(TempData, sizeof(unsigned short), TempDataSize, InputFileHandle)) > 0)
               {
                  /* We need to format the data differently based upon  */
                  /* the Channel Mode.                                  */
                  if(EncodeConfiguration.ChannelMode == cmMono)
                     memcpy(LeftChannelData, TempData, TempDataLength*sizeof(unsigned short));
                  else
                  {
                     /* The Channel Mode is NOT Mono, so we need to     */
                     /* split the samples up into channels.  The code   */
                     /* below assumes an even number of samples were    */
                     /* read.                                           */
                     for(Index = 0, Index2 = 0; Index < TempDataLength; Index+=2, Index2++)
                     {
                        /* Store the Left Channel Data (odd).           */
                        LeftChannelData[Index2] = TempData[Index];

                        /* Store the Right Channel Data (even).         */
                        RightChannelData[Index2] = TempData[Index+1];
                     }

                     /* Adjust the length to be half (which is the      */
                     /* number of samples in each channel).             */
                     TempDataLength = TempDataLength/2;
                  }

                  /* Next, format the EncodeData structure to pass to   */
                  /* the Encoder.                                       */
                  EncodeData.LeftChannelDataPtr      = LeftChannelData;
                  EncodeData.RightChannelDataPtr     = RightChannelData;
                  EncodeData.ChannelDataLength       = TempDataLength;
                  EncodeData.UnusedChannelDataLength = EncodeData.ChannelDataLength;

                  /* Now loop through the Encoder until ALL of the data */
                  /* has been processed by the Encoder.                 */
                  while(EncodeData.UnusedChannelDataLength > 0)
                  {
                     /* Pass the remaining data to the Encoder.         */
                     ret_val = SBC_Encode_Data(EncoderHandle, &EncodeData, &EncodedBitStreamData);

                     /* Adjust the Input Data pointers to point to the  */
                     /* first location of the unprocessed data.         */
                     EncodeData.LeftChannelDataPtr  = (unsigned short *)&(EncodeData.LeftChannelDataPtr[TempDataLength-EncodeData.UnusedChannelDataLength]);
                     EncodeData.ChannelDataLength   = EncodeData.UnusedChannelDataLength;

                     if(EncodeConfiguration.ChannelMode != cmMono)
                        EncodeData.RightChannelDataPtr = (unsigned short *)&(EncodeData.RightChannelDataPtr[TempDataLength-EncodeData.UnusedChannelDataLength]);

                     /* Adjust the amount of data remaining to be       */
                     /* processed.                                      */
                     TempDataLength = EncodeData.UnusedChannelDataLength;

                     /* Next check to see if any data was encoded.  If  */
                     /* it was then we need to write the data.          */
                     if(ret_val == SBC_PROCESSING_COMPLETE)
                     {
                        /* Simply write out the Bitstream.              */
                        fwrite(EncodedBitStreamData.BitStreamDataPtr, sizeof(unsigned char), EncodedBitStreamData.BitStreamDataLength, OutputFileHandle);
                     }
                     else
                     {
                        /* If an error occurred we need to inform the   */
                        /* user.                                        */
                        if(ret_val < 0)
                           printf("SBC_Encode_Data Function Error ret_val: %d\r\n", ret_val);
                     }
                  }
               }

               /* We are all finished with the Encoder, so we can inform*/
               /* the Library that we are finished with the Handle that */
               /* we opened.                                            */
               SBC_Cleanup_Encoder(EncoderHandle);

               /* Now, let's note the end time so we can inform the user*/
               /* how long it took.                                     */
               time(&EndTime);

               Index        = SBC_CalculateEncoderFrameLength(&EncodeConfiguration);
               Index2       = SBC_CalculateEncoderBitPoolSize(&EncodeConfiguration);
               BitRateValue = SBC_CalculateEncoderBitRate(&EncodeConfiguration);

               printf("Bit Rate  : %lu\n", BitRateValue);
               printf("Bit Pool  : %u\n", Index2);
               printf("Frame Size: %u\n", Index);

               printf("Time to encode: %lu:%.2lu\r\n", (unsigned long)((EndTime - StartTime)/60), (unsigned long)((EndTime - StartTime)%60));
            }
            else
               printf("Failed to initialize the encoder.\r\n");

            /* Finished with the output file, so close it.              */
            fclose(OutputFileHandle);
         }
         else
            printf("Unable to open specified Output File: %s\r\n", OutputFilePath);

         /* Finished with the input file so close it.                   */
         fclose(InputFileHandle);
      }
      else
         printf("Unable to open specified Input File: %s\r\n", InputFilePath);
   }
   else
   {
      printf("Usage: Encoder [-sf(sampling frequency value)] [-cm(channel mode value)]\r\n");
      printf("               [-bl(block length value)] [-sb(subbands value)]\r\n");
      printf("               [-am(allocation method value)] [-br(bit rate)]\r\n");
      printf("               [-w(WAV header present)] [-i(PCM/WAV input file)]\r\n");
      printf("               [-o(SBC output file)]\r\n");
   }

   return(0);
}

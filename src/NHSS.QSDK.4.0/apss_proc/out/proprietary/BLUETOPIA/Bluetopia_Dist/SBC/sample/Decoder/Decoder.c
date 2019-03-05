/*****< decoder.c >************************************************************/
/*      Copyright 2003 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DECODER - Stonestreet One Subband Codec (SBC) Decoder sample application  */
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
#define INPUT_FILE_DELIMITER                                      "-I"
#define OUTPUT_FILE_DELIMITER                                     "-O"

   /* Constant that defines the number of expected input parameters.    */
#define NUMBER_REQUIRED_INPUT_PARAMETERS                          (2)

   /* Constants that define the Input/Output Buffer Sizes for decoding  */
   /* the input the data.                                               */
#define READ_BUFFER_SIZE                                          (4096)
#define WRITE_BUFFER_SIZE                                         (4096)

   /* Constants that define the Decoded Output Buffer Sizes for decoding*/
   /* the input data.                                                   */
#define CHANNEL_BUFFER_SIZE                                       (4096)

   /* The following constants represent offsets and constants relating  */
   /* to the WAV Header.  These constants should not be changed.        */
#define WAVE_FILE_HEADER_WRITE_BUFFER_SIZE                        (128)
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
                                                      /* decoded.             */

static char OutputFilePath[MAXIMUM_PATH_LENGTH+1];    /* Variable used to hold*/
                                                      /* the path of the      */
                                                      /* output file that was */
                                                      /* decoded from the     */
                                                      /* input file.          */

   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   int                         ret_val;
   int                         ArgIndex;
   FILE                       *InputFileHandle;
   FILE                       *OutputFileHandle;
   time_t                      StartTime;
   time_t                      EndTime;
   Boolean_t                   Error;
   Decoder_t                   DecoderHandle;
   unsigned int                Index;
   unsigned int                Index2;
   unsigned int                NumberDecodedFrames;
   unsigned int                NumberParametersFound;
   static unsigned char        TempData[READ_BUFFER_SIZE];
   unsigned int                TempDataLength;
   unsigned int                TempDataSize;
   unsigned int                UnusedDataLength;

   unsigned int                NumberChannels;
   unsigned int                NumberSampleFrequency;

   unsigned int                TotalNumberSamples;
   unsigned int                NumberSamples;

   static unsigned char        WaveFileHeaderBuffer[WAVE_FILE_HEADER_WRITE_BUFFER_SIZE];

   static unsigned short       WaveFileWriteBuffer[WRITE_BUFFER_SIZE];
   unsigned int                WaveFileWriteBufferLength;
   unsigned int                WaveFileWriteBufferSize;

   SBC_Decode_Data_t           DecodedData;
   SBC_Decode_Configuration_t  DecodeConfiguration;

   static unsigned short       LeftChannelData[CHANNEL_BUFFER_SIZE];
   static unsigned short       RightChannelData[CHANNEL_BUFFER_SIZE];

   TempDataSize = sizeof(TempData)/sizeof(unsigned char);

   /* Initialize the Wave File Buffer Information.                      */
   WaveFileWriteBufferSize = sizeof(WaveFileWriteBuffer)/sizeof(unsigned short);

   /* Initialized that nothing has been decoded.                        */
   NumberDecodedFrames = 0;
   TotalNumberSamples  = 0;

   /* Finally, initialize the command line parsing variables.           */
   NumberParametersFound = 0;
   Error                 = FALSE;

   /* Next, loop through the command line arguements and pick out the   */
   /* commands we are interested in.                                    */
   for(ArgIndex = 1; (ArgIndex < argc) && (!Error); ArgIndex++)
   {
      if((strlen(argv[ArgIndex]) >= strlen(INPUT_FILE_DELIMITER)) && (!strncasecmp(argv[ArgIndex], INPUT_FILE_DELIMITER, strlen(INPUT_FILE_DELIMITER))))
      {
         /* Verify that a parameter has been entered                    */
         if((strlen(argv[ArgIndex]) == strlen(INPUT_FILE_DELIMITER)) && ((ArgIndex+1) < argc))
         {
            Index = 0;
            ArgIndex++;
         }
         else
            Index = strlen(INPUT_FILE_DELIMITER);

         strncpy(InputFilePath, &(argv[ArgIndex][Index]), (strlen(&(argv[ArgIndex][Index])) > sizeof(InputFilePath)?(sizeof(InputFilePath)):(strlen(&(argv[ArgIndex][Index])))));

         NumberParametersFound++;
      }
      else
      {
         if((strlen(argv[ArgIndex]) >= strlen(OUTPUT_FILE_DELIMITER)) && (!strncasecmp(argv[ArgIndex], OUTPUT_FILE_DELIMITER, strlen(OUTPUT_FILE_DELIMITER))))
         {
            /* Verify that a parameter has been entered                 */
            if((strlen(argv[ArgIndex]) == strlen(OUTPUT_FILE_DELIMITER)) && ((ArgIndex+1) < argc))
            {
               Index = 0;
               ArgIndex++;
            }
            else
               Index = strlen(OUTPUT_FILE_DELIMITER);

            strncpy(OutputFilePath, &(argv[ArgIndex][Index]), (strlen(&(argv[ArgIndex][Index])) > sizeof(OutputFilePath)?(sizeof(OutputFilePath)):(strlen(&(argv[ArgIndex][Index])))));

            NumberParametersFound++;
         }
         else
            Error = TRUE;
      }
   }

   /* Next, let's make sure that the command line information was parsed*/
   /* successfully.                                                     */
   if((!Error) && (NumberParametersFound >= NUMBER_REQUIRED_INPUT_PARAMETERS))
   {
      /* Information was successful, now let's attempt to open the Input*/
      /* SBC Encoded File.                                              */
      if((InputFileHandle = fopen(InputFilePath, "rb")) != NULL)
      {
         /* Input file has been opened, now lets open the output file   */
         /* that will contain the decoded output WAV data.              */
         if((OutputFileHandle = fopen(OutputFilePath, "wb")) != NULL)
         {
            /* Note the start time (just so we can dispaly it to the    */
            /* user).                                                   */
            time(&StartTime);

            /* Now, we are ready to start decoding.  First, let's       */
            /* initialize the Decoder.                                  */
            if((DecoderHandle = SBC_Initialize_Decoder()) != NULL)
            {
               DecodedData.ChannelDataSize        = CHANNEL_BUFFER_SIZE;
               DecodedData.LeftChannelDataLength  = 0;
               DecodedData.RightChannelDataLength = 0;
               DecodedData.LeftChannelDataPtr     = LeftChannelData;
               DecodedData.RightChannelDataPtr    = RightChannelData;

               /* Decoder has been initialized, so now let's read all   */
               /* available data from the input SBC Encoded file.       */
               while((TempDataLength = fread(TempData, sizeof(unsigned char), TempDataSize, InputFileHandle)) > 0)
               {
                  UnusedDataLength = TempDataLength;
                  while(UnusedDataLength > 0)
                  {
                     /* Pass the SBC data into the Decoder.  If a       */
                     /* complete Frame was decoded then we need to write*/
                     /* the decoded data to the output file.            */
                     if((ret_val = SBC_Decode_Data(DecoderHandle, UnusedDataLength, &TempData[(TempDataLength - UnusedDataLength)], &DecodeConfiguration, &DecodedData, &UnusedDataLength)) == SBC_PROCESSING_COMPLETE)
                     {
                        /* If this is the very first frame that was     */
                        /* decoded we need to write out the WAV Header. */
                        if(NumberDecodedFrames == 0)
                        {
                           /* This is the first frame decoded.  first   */
                           /* add the wave header to the file using the */
                           /* configuration from the decoded output.    */

                           /* First add the RIFF chunk descriptor.      */
                           /* Leaving room for the Chunk Size to be     */
                           /* added later.                              */
                           sprintf((char *)WaveFileHeaderBuffer, "RIFF    WAVE");

                           /* Next add the fmt sub-chunk.  Starting with*/
                           /* the header.                               */
                           sprintf((char *)&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_ID_OFFSET], "fmt ");

                           /* Then the sub-chunk size, this is always 16*/
                           /* for PCM.  This value must be written      */
                           /* little endian.                            */
                           ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_SIZE_OFFSET], 16);

                           /* Next the Audio Format.  This value is     */
                           /* always 1 for PCM.  This value must be     */
                           /* written little endian.                    */
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_AUDIO_FORMAT_OFFSET], 1);

                           /* Next the Number of Channels.  This value  */
                           /* will be the value from the frame of       */
                           /* decoded data.  This value must be written */
                           /* little endian.                            */
                           if(DecodeConfiguration.ChannelMode == cmMono)
                              NumberChannels = 1;
                           else
                              NumberChannels = 2;

                           /* Finally write out the number of channels. */
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_NUMBER_CHANNELS_OFFSET], NumberChannels);

                           /* Next the Sample Rate.  This value will be */
                           /* the value from the frame of decoded data. */
                           /* This value must be written little endian. */
                           switch(DecodeConfiguration.SamplingFrequency)
                           {
                              case sf16kHz:
                                 NumberSampleFrequency = 16000;
                                 break;
                              case sf32kHz:
                                 NumberSampleFrequency = 32000;
                                 break;
                              case sf441kHz:
                                 NumberSampleFrequency = 44100;
                                 break;
                              case sf48kHz:
                                 NumberSampleFrequency = 48000;
                                 break;
                              default:
                                 /* This case should never occur.       */
                                 NumberSampleFrequency = 44100;
                                 break;
                           }

                           /* Finally write out the Sample Frequency.   */
                           ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_SAMPLE_RATE_OFFSET], NumberSampleFrequency);

                           /* Next the Byte Rate.  This value is        */
                           /* (SampleRate*NumChannels*BitsPerSample/8). */
                           /* Bits Per Sample will always be 16 in our  */
                           /* case.  This value must be written little  */
                           /* endian.                                   */
                           ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_BYTE_RATE_OFFSET], (NumberSampleFrequency * NumberChannels * 16/8));

                           /* Next the block alignment.  This value is  */
                           /* (NumChannels*BitsPerSample/8).  Bits Per  */
                           /* sample will always be 16 in our case.     */
                           /* This value must be written little endian. */
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_BLOCK_ALIGN_OFFSET], (NumberChannels * 16/8));

                           /* Next write the finally data in sub chunk  */
                           /* 1, the Bits Per Sample.  This will always */
                           /* be 16 in our case.  This value must be    */
                           /* written little endian.                    */
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_BITS_PER_SAMPLE_OFFSET], 16);

                           /* All through writing sub chunk 1, next     */
                           /* write the sub chunk 2 ID.                 */
                           sprintf((char *)&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_2_ID_OFFSET], "data");

                           /* Next leave some space for the sub chunk 2 */
                           /* size.  This value must be written little  */
                           /* endian.                                   */
                           ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_2_SIZE_OFFSET], 0);

                           /* The rest of sub chunk two is data.  Write */
                           /* out the built wave header.                */
                           fwrite(WaveFileHeaderBuffer, sizeof(unsigned char), MINIMUM_WAVE_FILE_HEADER_BUFFER_LENGTH, OutputFileHandle);
                        }

                        /* Now we need to write out the samples.  We    */
                        /* need to possibly interleave them if this is  */
                        /* stereo.                                      */
                        NumberSamples = DecodedData.LeftChannelDataLength;
                        if(DecodeConfiguration.ChannelMode != cmMono)
                           NumberSamples += DecodedData.RightChannelDataLength;

                        while(NumberSamples > 0)
                        {
                           WaveFileWriteBufferLength = NumberSamples;
                           if(WaveFileWriteBufferLength > WaveFileWriteBufferSize)
                              WaveFileWriteBufferLength = WaveFileWriteBufferSize;

                           for(Index = 0, Index2 = 0; Index < WaveFileWriteBufferLength; Index++, Index2++)
                           {
                              WaveFileWriteBuffer[Index] = DecodedData.LeftChannelDataPtr[Index2];

                              if(DecodeConfiguration.ChannelMode != cmMono)
                              {
                                 Index++;
                                 WaveFileWriteBuffer[Index] = DecodedData.RightChannelDataPtr[Index2];
                              }
                           }

                           /* Now that the Samples have been written    */
                           /* into the buffer we need to write them to  */
                           /* the output file.                          */
                           fwrite(WaveFileWriteBuffer, sizeof(unsigned short), WaveFileWriteBufferLength, OutputFileHandle);

                           /* Decrement the number of samples that were */
                           /* written.                                  */
                           NumberSamples -= WaveFileWriteBufferLength;
                        }

                        /* Adjust the total number of samples and number*/
                        /* of decoded frames.                           */
                        TotalNumberSamples += (DecodedData.LeftChannelDataLength + DecodedData.RightChannelDataLength);
                        NumberDecodedFrames++;

                        DecodedData.LeftChannelDataLength  = 0;
                        DecodedData.RightChannelDataLength = 0;
                     }
                     else
                     {
                        /* If an error occurred we need to inform the   */
                        /* user.                                        */
                        if(ret_val < 0)
                           printf("SBC_Decode_Data Function Error ret_val: %d\r\n", ret_val);
                     }
                  }
               }

               /* All done writing all of the data to the wave file,    */
               /* now we need to go back in and update the wave header  */
               /* with the appropriate length information.  First seek  */
               /* to the location where the Chunk Size is to be written.*/
               fseek(OutputFileHandle, WAVE_FILE_HEADER_CHUNK_SIZE_OFFSET, SEEK_SET);

               /* Next, Build the Chunk Size.                           */
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(WaveFileHeaderBuffer, (36 + (TotalNumberSamples * 16/8)));

               /* Write out the Chuck Size.                             */
               fwrite(WaveFileHeaderBuffer, sizeof(unsigned char), sizeof(unsigned long), OutputFileHandle);

               /* Now seek to the location where the Sub Chunk 2 Size is*/
               /* to be written.                                        */
               fseek(OutputFileHandle, WAVE_FILE_HEADER_SUB_CHUNK_2_SIZE_OFFSET, SEEK_SET);

               /* Next, build the sub chunk 2 size.  This value will be */
               /* the (total number of samples * bits per sample/8)     */
               /* where bits per sample in our case is always 16.  This */
               /* value must be written little endian.                  */
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(WaveFileHeaderBuffer, (TotalNumberSamples * 16/8));

               /* Write out the Sub Chunk 2 Size.                       */
               fwrite(WaveFileHeaderBuffer, sizeof(unsigned char), sizeof(unsigned long), OutputFileHandle);

               /* We are all finished with the Decoder, so we can inform*/
               /* the Library that we are finished with the Handle that */
               /* we opened.                                            */
               SBC_Cleanup_Decoder(DecoderHandle);

               /* Now, let's note the end time so we can inform the user*/
               /* how long it took.                                     */
               time(&EndTime);

               printf("Bit Rate  : %lu\r\n", DecodeConfiguration.BitRate);
               printf("Bit Pool  : %u\r\n", DecodeConfiguration.BitPool);
               printf("Frame Size: %u\r\n", DecodeConfiguration.FrameLength);

               printf("Time to decode: %lu:%.2lu\r\n", (unsigned long)((EndTime - StartTime)/60), (unsigned long)((EndTime - StartTime)%60));
            }

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
      printf("Usage: Decoder [-i(SBC input file)] [-o(WAV output file)]\r\n");

   /* Return success to the caller.                                     */
   return(0);
}

/*****< audioencoder.c >*******************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/* AUDIOENCODER  - Sample code using Bluetopia Platform Manager Audio Manager */
/*                 Application Programming Interface (API).                   */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/13/11  G. Hensley     Initial creation. (Based on LinuxAUDM)          */
/******************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "SS1BTAUDM.h"     /* Audio Manager Application Programming Interface.*/
#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#include "AudioDecoder.h"  /* Audio Decoder sample.                           */

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

   /* Use the newer ALSA API                                            */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

#endif

   /* Constant which defines the size of the ring buffer that holds SBC */
   /* data for processing.                                              */
#define RING_BUFFER_SIZE                                          (1024 * 64)

   /* Constant which defines the size of the buffer which is used by the*/
   /* Playback Thread to read SBC data from the Ring Buffer.            */
#define RING_BUFFER_READ_BUFFER_SIZE                              1024
   /* Constant which defines the size of the PCM buffer for decoding    */
   /* SBC.                                                              */
#define PCM_BUFFER_SIZE                                           (4096 * 4)

   /* Constants that define the Decoded Output Buffer Sizes for decoding*/
   /* the input data.                                                   */
#define CHANNEL_BUFFER_SIZE                                       4096

   /* Constants that determine the length (in microseconds) of the ALSA */
   /* buffer and each period within the buffer.                         */
#define ALSA_BUFFER_SIZE_MICROSECONDS                             500000
#define ALSA_BUFFER_NUMBER_PERIODS                                5
#define ALSA_PERIOD_SIZE_MICROSECONDS                             ALSA_BUFFER_SIZE_MICROSECONDS/ALSA_BUFFER_NUMBER_PERIODS

   /* Bitmask to check if any flags are currently enabled that require  */
   /* us to actually process data.                                      */
#define DECODER_CONFIG_FLAGS_PROCESSING_ENABLED_MASK              (DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT | DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT | DECODER_CONFIG_FLAGS_DEBUG_LOG_SBC)
#define DECODER_CONFIG_FLAGS_THREAD_ENABLED_MASK                  (DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT | DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT)

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
#define WAVE_FILE_HEADER_TOTAL_SUB_CHUNK_HEADER_SIZE              (36)

typedef struct _tagRingBuffer_t
{
   unsigned char Buffer[RING_BUFFER_SIZE];
   unsigned int  ReadIndex;
   unsigned int  WriteIndex;
} RingBuffer_t;

   /* The following type definition represents the structure which holds*/
   /* the current audio playback context information.                   */
typedef struct _tagPlaybackContext
{
   unsigned int                ConfigFlags;
   Decoder_t                   DecoderHandle;
   AUD_Stream_Configuration_t  StreamConfig;
   SBC_Decode_Configuration_t  DecodeConfiguration;

   char                       *WavFileName;
   FILE                       *WavFileDescriptor;
   unsigned long               NumberBytesWritten;

   int                         SBCFileDescriptor;

   unsigned short              LeftChannelData[CHANNEL_BUFFER_SIZE];
   unsigned short              RightChannelData[CHANNEL_BUFFER_SIZE];
   RingBuffer_t                ReceiveBuffer;

   Mutex_t                     Mutex;
   Event_t                     StartProcessing;
   Event_t                     ShutdownEvent;

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

   snd_pcm_t                  *PcmHandle;
   snd_pcm_uframes_t           PcmPeriodSize;
   snd_pcm_uframes_t           PcmBufferSize;

#endif

} PlaybackContext_t;

   /* Single instance of the Playback Context                           */
static PlaybackContext_t PlaybackContext;

   /* Flag to indicated if the module is initialized                    */
static Boolean_t Initialized = FALSE;

static void RingBufferInit(RingBuffer_t *Buffer);
static int RingBufferSize(RingBuffer_t *Buffer);
static int RingBufferAvailable(RingBuffer_t *Buffer);
static int WriteRingBuffer(RingBuffer_t *Buffer, unsigned int Length, unsigned char *Data);
static int ReadRingBuffer(RingBuffer_t *Buffer, unsigned int Length, unsigned char *Data);

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

static int  ConfigureAudio(snd_pcm_t *PcmHandle);

#endif

static void WriteWavAudioDataToFile(unsigned char *Buffer, unsigned int Length);
static void *PlaybackThread(void *ThreadParameter);

   /* Utility function to initialize a ring buffer to empty.            */
static void RingBufferInit(RingBuffer_t *Buffer)
{
   if(Buffer)
   {
      BTPS_MemInitialize(Buffer->Buffer, 0, RING_BUFFER_SIZE);
      Buffer->ReadIndex  = 0;
      Buffer->WriteIndex = 0;
   }
}

   /* Utility function to return the amount of data currently in a ring */
   /* buffer.                                                           */
static int RingBufferSize(RingBuffer_t *Buffer)
{
   int ret_val = -1;

   if(Buffer)
   {
      if(Buffer->WriteIndex >= Buffer->ReadIndex)
         ret_val = Buffer->WriteIndex - Buffer->ReadIndex;
      else
         ret_val = Buffer->WriteIndex + RING_BUFFER_SIZE - Buffer->ReadIndex;
   }

   return ret_val;
}

   /* Utility function to return the amount of space left in a ring     */
   /* buffer that is available to be written.                           */
static int RingBufferAvailable(RingBuffer_t *Buffer)
{
   int ret_val = -1;

   if(Buffer)
      ret_val = RING_BUFFER_SIZE - RingBufferSize(Buffer);

   return(ret_val);
}

   /* Utility function to write data to a ring buffer. This function    */
   /* returns the number of bytes actually written on success or a      */
   /* negative value on error.                                          */
static int WriteRingBuffer(RingBuffer_t *Buffer, unsigned int Length, unsigned char *Data)
{
   int ret_val = -1;
   int Available;

   if(Buffer)
   {
      Available = RingBufferAvailable(Buffer);

      if(Available < Length)
         Length = Available;

      ret_val = Length;

      if(Buffer->WriteIndex + Length < RING_BUFFER_SIZE)
      {
         BTPS_MemCopy(&Buffer->Buffer[Buffer->WriteIndex], Data, Length);
         Buffer->WriteIndex += Length;
      }
      else
      {
         BTPS_MemCopy(&Buffer->Buffer[Buffer->WriteIndex], Data, RING_BUFFER_SIZE - Buffer->WriteIndex);
         Length -= RING_BUFFER_SIZE - Buffer->WriteIndex;
         BTPS_MemCopy(Buffer->Buffer, &Data[RING_BUFFER_SIZE - Buffer->WriteIndex], Length);
         Buffer->WriteIndex = Length;
      }
   }

   return(ret_val);
}

   /* Utility function to read data from a ring buffer. This function   */
   /* returns the number of bytes actually read on success or a negative*/
   /* value on error.                                                   */
static int ReadRingBuffer(RingBuffer_t *Buffer, unsigned int Length, unsigned char *Data)
{
   int ret_val = -1;
   int Available;

   if(Buffer)
   {
      Available = RingBufferSize(Buffer);

      if(Available < Length)
         Length = Available;

      ret_val = Length;

      if(Buffer->ReadIndex + Length < RING_BUFFER_SIZE)
      {
         BTPS_MemCopy(Data, &Buffer->Buffer[Buffer->ReadIndex], Length);
         Buffer->ReadIndex += Length;
      }
      else
      {
         BTPS_MemCopy(Data, &Buffer->Buffer[Buffer->ReadIndex], RING_BUFFER_SIZE - Buffer->ReadIndex);
         Length -= RING_BUFFER_SIZE - Buffer->ReadIndex;
         BTPS_MemCopy(&Data[RING_BUFFER_SIZE-Buffer->ReadIndex], Buffer->Buffer, Length);
         Buffer->ReadIndex = Length;
      }
   }

   return(ret_val);
}

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

   /* The following function is used to configure the host audio        */
   /* subsystem                                                         */
static int ConfigureAudio(snd_pcm_t *PcmHandle)
{
   int                  Result      = -1;
   int                  SubUnitDirection;
   unsigned int         RVal;
   snd_pcm_uframes_t    FVal;
   snd_pcm_hw_params_t *PcmHwParams;
   snd_pcm_sw_params_t *PcmSwParams;

   SubUnitDirection = 0;
   PcmHwParams      = NULL;
   PcmSwParams      = NULL;

   /* Allocate a hardware parameters object.                            */
   snd_pcm_hw_params_alloca(&PcmHwParams);

   /* Fill it in with default values.                                   */
   if((Result = snd_pcm_hw_params_any(PcmHandle, PcmHwParams)) < 0)
   {
      printf("Unable to initialize hw params: %d\n", Result);
      return Result;
   }

   if((Result = snd_pcm_hw_params_set_rate_resample(PcmHandle, PcmHwParams, 1)) < 0)
   {
      printf("Unable to set rate resample: %d\n", Result);
      return Result;
   }

   if((Result = snd_pcm_hw_params_set_access(PcmHandle, PcmHwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
   {
      printf("Unable to set access: %d\n", Result);
      return Result;
   }
   if((Result = snd_pcm_hw_params_set_format(PcmHandle, PcmHwParams, SND_PCM_FORMAT_S16)) < 0)
   {
      printf("Unable to set PCM format: %d\n", Result);
      return Result;
   }

   /* * NOTE * We are hard-coding to stereo. If the input stream is     */
   /*          Mono, we will rely on SBC to duplicate Mono frames to the*/
   /*          second channel, creating a fake stereo.                  */
   if((Result = snd_pcm_hw_params_set_channels(PcmHandle, PcmHwParams, 2)) < 0)
   {
      printf("Unable to set channels: %d\n", Result);
      return Result;
   }
   RVal = PlaybackContext.StreamConfig.StreamFormat.SampleFrequency;
   if((Result = snd_pcm_hw_params_set_rate_near(PcmHandle, PcmHwParams, &RVal, &SubUnitDirection)) < 0)
   {
      printf("Unable to set sample rate: %d\n", Result);
      return Result;
   }
   if(RVal != PlaybackContext.StreamConfig.StreamFormat.SampleFrequency)
      printf("!!!! Sample Rate Changed !!!!\n");

   printf("Sample rate set to %lu\n", PlaybackContext.StreamConfig.StreamFormat.SampleFrequency);

   RVal = ALSA_BUFFER_SIZE_MICROSECONDS;
   if((Result = snd_pcm_hw_params_set_buffer_time_near(PcmHandle, PcmHwParams, &RVal, &SubUnitDirection)) < 0)
   {
      printf("Unable to set buffer time: %d\n", Result);
      return Result;
   }

   if((Result = snd_pcm_hw_params_get_buffer_size(PcmHwParams, &FVal)) < 0)
   {
      printf("Unable to get buffer size: %d\n", Result);
      return Result;
   }

   PlaybackContext.PcmBufferSize = FVal;

   RVal = ALSA_PERIOD_SIZE_MICROSECONDS;
   if((Result = snd_pcm_hw_params_set_period_time_near(PcmHandle, PcmHwParams, &RVal, &SubUnitDirection)) < 0)
   {
      printf("Unable to set period time: %d\n", Result);
      return Result;
   }

   if((Result = snd_pcm_hw_params_get_period_size(PcmHwParams, &(PlaybackContext.PcmPeriodSize), &SubUnitDirection)) < 0)
   {
      printf("Unable to get period size: %d\n", Result);
      return Result;
   }

   if((Result = snd_pcm_hw_params(PcmHandle, PcmHwParams)) < 0)
   {
      printf("Unable to set hw params: %d\n", Result);
      return Result;
   }

   snd_pcm_sw_params_alloca(&PcmSwParams);

   if((Result = snd_pcm_sw_params_current(PcmHandle, PcmSwParams)) < 0)
   {
      printf("Unable to get current sw params: %d\n", Result);
      return Result;
   }

   /* Set ALSA not to start playback until the buffer is nearly filled  */
   /* (all but one period).                                             */
   if((Result = snd_pcm_sw_params_set_start_threshold(PcmHandle, PcmSwParams, PlaybackContext.PcmBufferSize)) < 0)
   {
      printf("Unable to set start threshold: %d\n", Result);
      return Result;
   }

   if((Result = snd_pcm_sw_params(PcmHandle, PcmSwParams)) < 0)
   {
      printf("Unable to set sw params: %d\n", Result);
      return Result;
   }

   Result = 0;

   return Result;
}

#endif

   /* The following function performs the decoding of SBC data          */
static int SbcDecode(unsigned char *SbcData, unsigned int SbcDataLength, unsigned char *PcmData, unsigned int *PcmDataLength)
{
   SBC_Decode_Data_t   DecodedData;
   unsigned int        UnusedDataLength;
   int                 Result;
   Word_t             *OutputBufferSamples;
   unsigned int        TotalSamples;
   unsigned int        SamplesRead;

   if(Initialized)
   {
      if((SbcData) && (SbcDataLength) && (PcmData) && (PcmDataLength) && (*PcmDataLength % 4 == 0))
      {
         /* Note the output size.                                       */
         OutputBufferSamples         = (Word_t *)PcmData;
         DecodedData.ChannelDataSize = (*PcmDataLength)/4;

         /* Clear the length, because we are going to update it to the  */
         /* actual length                                               */
         *PcmDataLength = 0;

         TotalSamples = 0;

         UnusedDataLength = SbcDataLength;
         while(UnusedDataLength > 0)
         {
            /* Note that each channels size is have the interleaved     */
            /* output buffer.                                           */
            DecodedData.LeftChannelDataLength  = 0;
            DecodedData.RightChannelDataLength = 0;

            /* Setup the decode function to interleaved the PCM directly*/
            /* into the output buffer.                                  */
            DecodedData.LeftChannelDataPtr     = (Word_t *)&OutputBufferSamples[TotalSamples];
            DecodedData.RightChannelDataPtr    = (Word_t *)&DecodedData.LeftChannelDataPtr[1];

            /* Pass the SBC data into the decoder.                      */
            if((Result = SBC_Decode_Data(PlaybackContext.DecoderHandle, UnusedDataLength, &SbcData[(SbcDataLength - UnusedDataLength)], &PlaybackContext.DecodeConfiguration, &DecodedData, &UnusedDataLength)) == SBC_PROCESSING_COMPLETE)
            {
               /* Note the total number of samples that we have read.   */
               SamplesRead = DecodedData.LeftChannelDataLength + DecodedData.RightChannelDataLength;

               /* Note the total length                              */
               *PcmDataLength = (*PcmDataLength) + (SamplesRead * 2);
               TotalSamples  += SamplesRead;
            }
            else
            {
               if(Result == SBC_PROCESSING_DATA)
                  Result = 0;
               else
                  printf("SBC Error: %d\n", Result);
            }
         }

      }
      else
      {
         Result = -2;
         printf("invalid parameter(s)\n");
      }
   }
   else
   {
      Result = -1;
   }

   return Result;
}

   /* The following utility function writes the raw audio data to a     */
   /* file.                                                             */
static void WriteWavAudioDataToFile(unsigned char *Buffer, unsigned int Length)
{
   int                  Result;
   static unsigned char WaveFileHeaderBuffer[WAVE_FILE_HEADER_WRITE_BUFFER_SIZE];
   unsigned int         NumberChannels;
   unsigned int         NumberSampleFrequency;

   /* Validate the module is initialized                                */
   if(Initialized)
   {
      if((Length) && (Buffer))
      {
         /* If the file is not open, attempt to open it.                */
         if(NULL == PlaybackContext.WavFileDescriptor)
         {
            if(NULL != (PlaybackContext.WavFileDescriptor = fopen(PlaybackContext.WavFileName,"w")))
            {
               /* This is the first frame decoded. First add the wave   */
               /* header to the file using the configuration from the   */
               /* decoded output.                                       */
               PlaybackContext.NumberBytesWritten = 0;

               /* First add the RIFF chunk descriptor. Leaving room for */
               /* the Chunk Size to be added later.                     */
               sprintf((char *)WaveFileHeaderBuffer, "RIFF    WAVE");

               /* Next add the fmt sub-chunk. Starting with the header. */
               sprintf((char *)&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_ID_OFFSET], "fmt ");

               /* Then the sub-chunk size, this is always 16 for PCM.   */
               /* This value must be written little endian.             */
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_SIZE_OFFSET], 16);

               /* Next the Audio Format. This value is always 1 for PCM.*/
               /* This value must be written little endian.             */
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_AUDIO_FORMAT_OFFSET], 1);

               /* Force number of channels to 2, since mono SBC data has*/
               /* been duplicated to dual-channel.                      */
               NumberChannels = 2;

               /* Finally write out the number of channels.             */
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_NUMBER_CHANNELS_OFFSET], NumberChannels);

               /* Next the Sample Rate. This value will be the value    */
               /* from the frame of decoded data. This value must be    */
               /* written little endian.                                */
               NumberSampleFrequency = PlaybackContext.StreamConfig.StreamFormat.SampleFrequency;

               /* Finally write out the Sample Frequency.               */
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_SAMPLE_RATE_OFFSET], NumberSampleFrequency);

               /* Next the Byte Rate. This value is                     */
               /* (SampleRate*NumChannels*BitsPerSample/8). Bits Per    */
               /* Sample will always be 16 in our case. This value must */
               /* be written little endian.                             */
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_BYTE_RATE_OFFSET], (NumberSampleFrequency * NumberChannels * 16/8));

               /* Next the block alignment. This value is               */
               /* (NumChannels*BitsPerSample/8). Bits Per sample will   */
               /* always be 16 in our case. This value must be written  */
               /* little endian.                                        */
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_BLOCK_ALIGN_OFFSET], (NumberChannels * 16/8));

               /* Next write the finally data in sub chunk 1, the Bits  */
               /* Per Sample. This will always be 16 in our case. This  */
               /* value must be written little endian.                  */
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_1_BITS_PER_SAMPLE_OFFSET], 16);

               /* All through writing sub chunk 1, next write the sub   */
               /* chunk 2 ID.                                           */
               sprintf((char *)&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_2_ID_OFFSET], "data");

               /* Next leave some space for the sub chunk 2 size. This  */
               /* value must be written little endian.                  */
               ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&WaveFileHeaderBuffer[WAVE_FILE_HEADER_SUB_CHUNK_2_SIZE_OFFSET], 0);

               /* The rest of sub chunk two is data. Write out the built*/
               /* wave header.                                          */
               fwrite(WaveFileHeaderBuffer, sizeof(unsigned char), MINIMUM_WAVE_FILE_HEADER_BUFFER_LENGTH, PlaybackContext.WavFileDescriptor);
            }
         }

         if(NULL != PlaybackContext.WavFileDescriptor)
         {
            Result = fwrite(Buffer, 1, Length, PlaybackContext.WavFileDescriptor);

            if(Result < 0)
            {
               printf("failed to write\n");
            }
            else 
            {
               if(Result != Length)
                  printf("Partial write %d\n", Result);

               PlaybackContext.NumberBytesWritten += Result;
            }
         }
         else
         {
            printf("Failed to open file\n");
         }
      }
      else
      {
         printf("Invalid param\n");
      }
   }
   else
   {
      printf("Invalid parameters\n");
   }
}

   /* The following function is the Main function for the playback      */
   /* thread. This function reads data from the SBC ring buffer, decodes*/
   /* it, and streams it to ALSA (if enabled).                          */
static void *PlaybackThread(void *ThreadParameter)
{
   int                  BytesRead;
   int                  Available;
   unsigned int         PcmBufferSize;
   static unsigned char SbcBuffer[RING_BUFFER_READ_BUFFER_SIZE];
   static unsigned char PcmBuffer[PCM_BUFFER_SIZE];

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

   int            Result;
   unsigned long  FramesToWrite;
   unsigned char *PcmPtr;

   /* Make sure Alsa is setup.                                          */
   if(!ConfigureAudio(PlaybackContext.PcmHandle))
   {

#endif

      /* Continue processing data until the decoder is cleaned.         */
      while(Initialized)
      {
         /* Determine how much data is available in the Ring Buffer.    */
         Available = RingBufferSize(&PlaybackContext.ReceiveBuffer);

         /* If there is no data available, wait for some to be written. */
         if(Available > 0)
         {
            /* Read SBC data from the buffer.                           */
            BTPS_WaitMutex(PlaybackContext.Mutex, BTPS_INFINITE_WAIT);
            BytesRead = ReadRingBuffer(&PlaybackContext.ReceiveBuffer, (Available < RING_BUFFER_READ_BUFFER_SIZE)?Available:RING_BUFFER_READ_BUFFER_SIZE, SbcBuffer);
            BTPS_ReleaseMutex(PlaybackContext.Mutex);

            /* Make sure we have data to process.                       */
            if(BytesRead > 0)
            {
               PcmBufferSize = PCM_BUFFER_SIZE;

               /* Decode the SBC to PCM.                                */
               /* * NOTE * This function will both interleave the       */
               /*          channels and duplicate mono to dual-channel, */
               /*          so that we always have 2 channels.           */
               if(!SbcDecode(SbcBuffer, BytesRead, PcmBuffer, &PcmBufferSize))
               {
                  /* Write the PCM to a WAVE file if configured.        */
                  if(PlaybackContext.ConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT)
                     WriteWavAudioDataToFile(PcmBuffer, PcmBufferSize);

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

                  /* Stream the PCM to ALSA if configured.              */
                  if(PlaybackContext.ConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_ALSA_OUTPUT)
                  {
                     /* Since we are guaranteed to have 2 16-bit        */
                     /* channels, each frame is 4 bytes.                */
                     FramesToWrite = PcmBufferSize / 4;
                     PcmPtr = PcmBuffer;

                     /* Make sure all frames are written.               */
                     while(FramesToWrite)
                     {
                        /* Write the frames to ALSA.                    */
                        Result = snd_pcm_writei(PlaybackContext.PcmHandle, PcmPtr, FramesToWrite);

                        if (Result == -EPIPE)
                        {
                           /* EPIPE means underrun                      */
                           printf("Alsa buffer starved. Refilling. (Playback from source might be slow)\n");
                           snd_pcm_prepare(PlaybackContext.PcmHandle);
                        }
                        else if (Result < 0)
                        {
                           printf("Error from writei: %s\n", snd_strerror(Result));
                           break;
                        }

                        /* Advance to the next offset of frames to      */
                        /* write.                                       */
                        FramesToWrite -= Result;
                        PcmPtr        += Result * 4;
                     }
                  }

#endif

               }
            }
         }
         else
         {
            /* Wait for the producer to notify us that more data is     */
            /* available.                                               */
            BTPS_WaitEvent(PlaybackContext.StartProcessing, BTPS_INFINITE_WAIT);

            BTPS_ResetEvent(PlaybackContext.StartProcessing);
         }
      }

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

      /* Empty the PCM buffers.                                         */
      snd_pcm_drain(PlaybackContext.PcmHandle);

#endif

      /* Set the event to shutdown                                      */
      BTPS_SetEvent(PlaybackContext.ShutdownEvent);

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

   }

#endif 

   return NULL;
}

   /* The following function initializes the audio decoder. The first   */
   /* parameter is a valid Audio Manager Data Handler ID (registered    */
   /* via call to the AUDM_Register_Data_Event_Callback() function),    */
   /* followed by optional configuration flags. This function will      */
   /* return zero on success and negative on error.                     */
int InitializeAudioDecoder(unsigned int ConfigFlags, BD_ADDR_t BD_ADDR, const char *WaveFileName)
{
   int Result = -1;

   if(!Initialized)
   {
      memset(&PlaybackContext, 0, sizeof(PlaybackContext_t));

      /* Check whether the flags actually indicate we have any          */
      /* processing to do.                                              */
      if((ConfigFlags & DECODER_CONFIG_FLAGS_PROCESSING_ENABLED_MASK) != 0)
      {
         if((Result = AUDM_Query_Audio_Stream_Configuration(BD_ADDR, astSNK, &PlaybackContext.StreamConfig)) == 0)
         {
            /* Note the config flags.                                      */
            PlaybackContext.ConfigFlags = ConfigFlags;

            if(PlaybackContext.ConfigFlags & DECODER_CONFIG_FLAGS_ENABLE_WAV_OUTPUT)
            {
               if(WaveFileName)
                  PlaybackContext.WavFileName = (char *)WaveFileName;
               else
                  PlaybackContext.WavFileName = DECODER_DEFAULT_WAV_FILE_NAME;
            }

            if((PlaybackContext.StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_SBC) && (PlaybackContext.StreamConfig.MediaCodecInfoLength == sizeof(A2DP_SBC_Codec_Specific_Information_Element_t)))
            {
               if((NULL != (PlaybackContext.ShutdownEvent = BTPS_CreateEvent(FALSE))) && (NULL != (PlaybackContext.Mutex = BTPS_CreateMutex(FALSE))) && (NULL != (PlaybackContext.StartProcessing = BTPS_CreateEvent(FALSE))))
               {

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

                  if((Result = snd_pcm_open(&PlaybackContext.PcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) >= 0)
                  {

#endif

                     /* Now, we are ready to start decoding. First, let's  */
                     /* initialize the Decoder.                            */
                     if((PlaybackContext.DecoderHandle = SBC_Initialize_Decoder()) != NULL)
                     {
                        Initialized = TRUE;

                        RingBufferInit(&PlaybackContext.ReceiveBuffer);

                        if(PlaybackContext.ConfigFlags & DECODER_CONFIG_FLAGS_DEBUG_LOG_SBC)
                        {
                           if((PlaybackContext.SBCFileDescriptor = open(DECODER_DEBUG_SBC_FILE_NAME, O_CREAT|O_WRONLY, 0664)) < 0)
                              perror("Unable to open SBC Log file.");
                        }

                        if(BTPS_CreateThread(PlaybackThread, 16384, NULL))
                        {
                           Result = 0;
                        }
                        else
                        {
                           Initialized = FALSE;
                           printf("Unable to start Playback Thread.\r\n");
                        }
                     }
                     else
                     {
                        printf("Failed to decoder.\r\n");
                     }

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

                  }
                  else
                  {
                     printf("Failed to open pcm.\r\n");
                  }

#endif

               }
               else
               {
                  printf("Failed to create mutex/event.\r\n");
               }
            }
            else
            {
               printf("Unsupported stream type or invalid configuration\n");
            }
         }
      }
      else
      {
         printf("No known config flags set. Incoming Audio will be ignored.\n");
         Result = 0;
      }
   }
   else
   {
      printf("Unable to query audio connection configuration (%d)\n", Result);
   }

   return Result;
}

   /* The following function is responsible for freeing all resources   */
   /* that were previously allocated for an audio decoder.              */
void CleanupAudioDecoder(void)
{
   DWord_t      Length;

   Initialized = FALSE;

   /* Set the event to kill the thread                                  */
   if((PlaybackContext.ShutdownEvent) && (PlaybackContext.StartProcessing))
   {
      BTPS_SetEvent(PlaybackContext.StartProcessing);

      printf("Waiting for thread...\n");
      BTPS_WaitEvent(PlaybackContext.ShutdownEvent, BTPS_INFINITE_WAIT);
      printf("Wait done\n");

      BTPS_CloseEvent(PlaybackContext.ShutdownEvent);
      PlaybackContext.ShutdownEvent = NULL;

      BTPS_CloseEvent(PlaybackContext.StartProcessing);
      PlaybackContext.StartProcessing = NULL;
   }


   if(NULL != PlaybackContext.WavFileDescriptor)
   {
      /* Go back and set the lengths in the header before we close the  */
      /* file.                                                          */
      ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&Length, PlaybackContext.NumberBytesWritten);
      fseek(PlaybackContext.WavFileDescriptor, WAVE_FILE_HEADER_SUB_CHUNK_2_SIZE_OFFSET, SEEK_SET);
      fwrite(&Length, 1, DWORD_SIZE, PlaybackContext.WavFileDescriptor);

      ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&Length, Length+WAVE_FILE_HEADER_TOTAL_SUB_CHUNK_HEADER_SIZE);
      fseek(PlaybackContext.WavFileDescriptor, WAVE_FILE_HEADER_CHUNK_SIZE_OFFSET, SEEK_SET);
      fwrite(&Length, 1, DWORD_SIZE, PlaybackContext.WavFileDescriptor);

      fclose(PlaybackContext.WavFileDescriptor);
      PlaybackContext.WavFileDescriptor = NULL;
   }

   /* Since this is part of a CLI sample, we assume that STDIN has taken*/
   /* fd 0, therefore we do not count it as valid.                      */
   if(PlaybackContext.SBCFileDescriptor > 0)
   {
      close(PlaybackContext.SBCFileDescriptor);
      PlaybackContext.SBCFileDescriptor = -1;
   }

   if(NULL != PlaybackContext.DecoderHandle)
   {
      /* We are all finished with the decoder, so we can inform the     */
      /* library that we are finished with the handle that we opened.   */
      SBC_Cleanup_Decoder(PlaybackContext.DecoderHandle);
      PlaybackContext.DecoderHandle = NULL;
   }

#ifndef DISABLE_AUDIO_SINK_ALSA_OUTPUT

   if(NULL != PlaybackContext.PcmHandle)
   {
      PlaybackContext.PcmHandle = NULL;
   }

#endif

   if(PlaybackContext.Mutex)
   {
      BTPS_CloseMutex(PlaybackContext.Mutex);
      PlaybackContext.Mutex = NULL;
   }
}

int ProcessAudioData(void *RawAudioDataFrame, unsigned int RawAudioDataFrameLength)
{
   int ret_val = -1;
   int Available;
   Boolean_t Wakeup;

   if(Initialized)
   {
      /* Ignore the data if we are not configured to actually do any    */
      /* processing.                                                    */
      if((PlaybackContext.ConfigFlags & DECODER_CONFIG_FLAGS_PROCESSING_ENABLED_MASK) != 0)
      {
         if((RawAudioDataFrameLength) && (RawAudioDataFrame))
         {
            if((PlaybackContext.ConfigFlags & DECODER_CONFIG_FLAGS_DEBUG_LOG_SBC) && (PlaybackContext.SBCFileDescriptor >= 0))
            {
               if(write(PlaybackContext.SBCFileDescriptor, RawAudioDataFrame, RawAudioDataFrameLength) < 0)
               {
                  perror("SBC Log Write Error");
                  close(PlaybackContext.SBCFileDescriptor);
                  PlaybackContext.SBCFileDescriptor = -1;
               }
            }

            Available = RingBufferAvailable(&PlaybackContext.ReceiveBuffer);

            Wakeup = (Boolean_t)(Available == RING_BUFFER_SIZE);

            if(Available >= RawAudioDataFrameLength)
            {
               BTPS_WaitMutex(PlaybackContext.Mutex, BTPS_INFINITE_WAIT);
               if((ret_val = WriteRingBuffer(&PlaybackContext.ReceiveBuffer, RawAudioDataFrameLength, RawAudioDataFrame)) != RawAudioDataFrameLength)
               {
                  printf("Unable to copy entire SBC Frame!!!, %d, %u\n", ret_val, RawAudioDataFrameLength);

               }
               BTPS_ReleaseMutex(PlaybackContext.Mutex);

               if(Wakeup)
                  BTPS_SetEvent(PlaybackContext.StartProcessing);

               ret_val = 0;
            }
            else
            {
               printf("Ring buffer overflow! Dropping SBC frames! %d, %u\n", Available, RawAudioDataFrameLength);
            }
         }
      }
      else
         ret_val = 0;
   }

   return(ret_val);
}

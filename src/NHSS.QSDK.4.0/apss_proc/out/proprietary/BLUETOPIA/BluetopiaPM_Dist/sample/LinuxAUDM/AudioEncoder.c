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

#include "SS1BTAUDM.h"     /* Audio Manager Application Programming Interface.*/
#include "SS1BTPM.h"       /* BTPM Application Programming Interface.         */

#include "AudioEncoder.h"  /* Audio Encoder sample.                           */

#define MAXIMUM_BIT_POOL                             55  /* Denotes the       */
                                                         /* Maximum Bit Pool  */
                                                         /* value to use when */
                                                         /* SBC encoding.     */

   /* Denote the number of nanoseconds per unit of time (used in        */
   /* calculation of timing Audio Data).                                */
#define NSEC_PER_SEC                               (1000000000L)
#define NSEC_PER_MSEC                              (1000000L)
#define NSEC_PER_USEC                              (1000L)

   /* The following type definition represents the structure which holds*/
   /* the current audio playback context information.                   */
typedef struct _tagPlaybackContext
{
   Encoder_t           Encoder;
   unsigned char      *SBCFrameBuffer;
   unsigned int        SBCFrameSize;
   unsigned int        SBCFramesPerPacket;
   unsigned int        SampleSize;

   long long           ExtraTimePerInterval;
   long long           UnderrunTime;
   long long           ExtraTimeCount;

   struct timespec     NextWriteTime;
   struct timespec     PacketDelay;

   AUD_Stream_State_t  StreamState;

   unsigned int        CallbackId;
   unsigned int        BlockAlign;

   unsigned int        PacketCount;
   unsigned int        SBCFrameCount;
   unsigned long long  SBCFrameTotal;
} PlaybackContext_t;

   /* Single instance of the Playback Context                           */
static PlaybackContext_t PlaybackContext;

   /* Flag to indicated if the module is initialized                    */
static Boolean_t Initialized = FALSE;

static void NormalizeTimespec(struct timespec *TS);
static struct timespec AddTimespec(const struct timespec *TS1, const struct timespec *TS2);
static struct timespec DiffTimespec(const struct timespec *TS1, const struct timespec *TS2);
static int CmpTimespec(const struct timespec *TS1, const struct timespec *TS2);

   /* The following function is used to transform an invalid timespec   */
   /* structure into a valid form. See IsValidTimespec() for a          */
   /* definition of a valid timespec. The result will be a timespec     */
   /* which encodes a positive magnitude where both component vectors   */
   /* (tv_sec and tv_nsec) are greater than or equal to zero.           */
static void NormalizeTimespec(struct timespec *TS)
{
   if(TS)
   {
      /* Ensure the tv_nsec field is positive so we can easily correct a*/
      /* magnitude of 1 second (1e9 nanoseconds) or more.               */
      if(TS->tv_nsec < 0)
      {
         TS->tv_sec  = -(TS->tv_sec);
         TS->tv_nsec = -(TS->tv_nsec);
      }

      /* Fix a positive out-of-bounds state in the tv_nsec field. If the*/
      /* tv_nsec field's value represents one second of time or more,   */
      /* add the number of whole seconds to the tv_sec field and store  */
      /* only the sub-second component in tv_nsec.                      */
      if(TS->tv_nsec >= (2 * NSEC_PER_SEC))
      {
         TS->tv_sec += (TS->tv_nsec / NSEC_PER_SEC);
         TS->tv_nsec = (TS->tv_nsec % NSEC_PER_SEC);
      }
      else
      {
         if(TS->tv_nsec >= NSEC_PER_SEC)
         {
            TS->tv_sec  += 1;
            TS->tv_nsec -= NSEC_PER_SEC;
         }
      }

      /* Now, the nanosecond offset is guaranteed to be a positive      */
      /* value between 0 and 999999999, and the magnitude component     */
      /* represented by the tv_sec field is either zero or greater than */
      /* that of tv_nsec. This means the overall sign of the timespec is*/
      /* determined by the tv_sec field.                                */

      /* Check whether the timespec is negative.                        */
      if(TS->tv_sec < 0)
      {
         /* The timespec is negative, so we'll flip the timespec and    */
         /* re-normalize the possibly negative nanosecond field.        */
         TS->tv_sec = -(TS->tv_sec);

         if(TS->tv_nsec > 0)
         {
            /* tv_nsec was positive and will become negative when       */
            /* flipped. Fix this by shifting one positive second of time*/
            /* from tv_sec to tv_nsec so tv_nsec will again contain a   */
            /* positive offset. This can't make tv_sec negative again   */
            /* because it must have contained a magnitude of at least 1 */
            /* in order to reach this codepath.                         */
            TS->tv_sec -= 1;
            TS->tv_nsec = (NSEC_PER_SEC - TS->tv_nsec);
         }
      }
   }
}

   /* The following function calculates the sum of two timespec         */
   /* structures. The timespec structures must be valid according       */
   /* to IsValidTimespec(). If either parameter is NULL, a timespec     */
   /* representing zero time will be returned. If either parameter is an*/
   /* invalid timespec structure, the result is undefined.              */
static struct timespec AddTimespec(const struct timespec *TS1, const struct timespec *TS2)
{
   struct timespec Dest;

   if((TS1) && (TS2))
   {
      Dest.tv_sec  = TS1->tv_sec + TS2->tv_sec;
      Dest.tv_nsec = TS1->tv_nsec + TS2->tv_nsec;

      NormalizeTimespec(&Dest);
   }
   else
      memset(&Dest, 0, sizeof(Dest));

   return(Dest);
}

   /* The following function calculates the absolute difference         */
   /* between two timespec structures. The timespec structures must     */
   /* be valid according to IsValidTimespec(). If either parameter is   */
   /* NULL, a timespec representing zero time will be returned. If      */
   /* either parameter is an invalid timespec structure, the result is  */
   /* undefined.                                                        */
static struct timespec DiffTimespec(const struct timespec *TS1, const struct timespec *TS2)
{
   struct timespec Dest;

   if((TS1) && (TS2))
   {
      Dest.tv_sec  = TS1->tv_sec - TS2->tv_sec;
      Dest.tv_nsec = TS1->tv_nsec - TS2->tv_nsec;

      NormalizeTimespec(&Dest);
   }
   else
      memset(&Dest, 0, sizeof(Dest));

   return(Dest);
}

   /* The following function is used to compare two timespec structures.*/
   /* The structures must be valid according to IsValidTimespec(). The  */
   /* function returns -1, 0, or 1 if TS1 is, respectively, less than,  */
   /* equal to, or greater than TS2. If either parameter is NULL, the   */
   /* value 0 is returned. If either parameter is an invalid timespec   */
   /* structure, the result is undefined.                               */
static int CmpTimespec(const struct timespec *TS1, const struct timespec *TS2)
{
   int ret_val;

   if((TS1) && (TS2))
   {
      if(TS1->tv_sec == TS2->tv_sec)
      {
         if(TS1->tv_nsec == TS2->tv_nsec)
            ret_val = 0;
         else
         {
            if(TS1->tv_nsec > TS2->tv_nsec)
               ret_val = 1;
            else
               ret_val = -1;
         }
      }
      else
      {
         if(TS1->tv_sec > TS2->tv_sec)
            ret_val = 1;
         else
            ret_val = 0;
      }
   }
   else
      ret_val = 0;

   return(ret_val);
}

   /* The following function initializes the Audio encoder.  The first  */
   /* parameter is a valid Audio Manager Data Handler ID (registered via*/
   /* call to the AUDM_Register_Data_Event_Callback() function),        */
   /* followed by the samples per second of the audio source.  The      */
   /* finial parameter is the number channels (1 or 2) of the audio     */
   /* source.  This function will return zero on success and negative on*/
   /* error.                                                            */
int InitializeAudioEncoder(unsigned int CallbackId, unsigned int SamplesPerSecond, unsigned int NumberOfChannels, BD_ADDR_t BD_ADDR)
{
   int                                           Result;
   int                                           ret_val = -1;
   unsigned int                                  MTU;
   unsigned int                                  Subbands;
   unsigned int                                  Channels;
   unsigned int                                  Frequency;
   unsigned int                                  BlockSize;
   unsigned long long                            PacketDelayNS;
   unsigned long long                            PacketDelayNSTrunc;
   SBC_Encode_Configuration_t                    EncoderConfig;
   AUD_Stream_Configuration_t                    StreamConfig;
   A2DP_SBC_Codec_Specific_Information_Element_t CodecInfo;

   memset(&PlaybackContext, 0, sizeof(PlaybackContext_t));

   if((Result = AUDM_Query_Audio_Stream_Configuration(BD_ADDR, astSRC, &StreamConfig)) == 0)
   {
      if((StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_SBC) && (StreamConfig.MediaCodecInfoLength == sizeof(A2DP_SBC_Codec_Specific_Information_Element_t)))
      {
         PlaybackContext.CallbackId = CallbackId;

         /* Only support 1 and 2 channels which is 4 and 2 BlockAlign   */
         if(NumberOfChannels == 2)
            PlaybackContext.BlockAlign = 4;
         else
            PlaybackContext.BlockAlign = 2;

         memcpy(&CodecInfo, &StreamConfig.MediaCodecInformation, sizeof(CodecInfo));

         printf("Building SBC configuration based on these connection parameters:\n");
         printf("    MediaMTU:     %u\n",       (StreamConfig.MediaMTU));
         printf("    Frequency:    %lu\n",      (StreamConfig.StreamFormat.SampleFrequency));
         printf("    Channels:     %u\n",       (StreamConfig.StreamFormat.NumberChannels));
         printf("    FormatFlags:  0x%02lX\n",  (StreamConfig.StreamFormat.FormatFlags));
         printf("    CodecType:    %s\n",       (StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_SBC ? "SBC" : (StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_MPEG_1_2_AUDIO ? "MPEG 1/2" : (StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_MPEG_2_4_AUDIO ? "MPEG 2/4" : (StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_ATRAC ? "ATRAC" : (StreamConfig.MediaCodecType == A2DP_MEDIA_CODEC_TYPE_NON_A2DP ? "NON-A2DP" : "UNKNOWN"))))));
         printf("    SFCM:         0x%02hhX\n", (CodecInfo.SamplingFrequencyChannelMode));
         printf("    BLSAM:        0x%02hhX\n", (CodecInfo.BlockLengthSubbandsAllocationMethod));
         printf("    Min Bit Pool: %hhu\n",     (CodecInfo.MinimumBitPoolValue));
         printf("    Max Bit Pool: %hhu\n",     (CodecInfo.MaximumBitPoolValue));
         printf("\n");

         /* We use an MTU one byte less than the connection's MTU to    */
         /* account for the size of the A2DP Header.                    */
         MTU       = StreamConfig.MediaMTU - 1;
         Frequency = StreamConfig.StreamFormat.SampleFrequency;

         switch(Frequency)
         {
            case 48000:
               EncoderConfig.SamplingFrequency = sf48kHz;
               break;
            case 44100:
               EncoderConfig.SamplingFrequency = sf441kHz;
               break;
            case 32000:
               EncoderConfig.SamplingFrequency = sf32kHz;
               break;
            case 16000:
               EncoderConfig.SamplingFrequency = sf16kHz;
               break;
            default:
               printf("Unsupported frequency %u\n", Frequency);

               EncoderConfig.SamplingFrequency = sf441kHz;
               Frequency                       = 0;
               break;
         }

         switch(CodecInfo.BlockLengthSubbandsAllocationMethod & A2DP_SBC_BLOCK_LENGTH_MASK)
         {
            case A2DP_SBC_BLOCK_LENGTH_SIXTEEN_VALUE:
               EncoderConfig.BlockSize = bsSixteen;
               BlockSize               = 16;
               break;
            case A2DP_SBC_BLOCK_LENGTH_TWELVE_VALUE:
               EncoderConfig.BlockSize = bsTwelve;
               BlockSize               = 12;
               break;
            case A2DP_SBC_BLOCK_LENGTH_EIGHT_VALUE:
               EncoderConfig.BlockSize = bsEight;
               BlockSize               = 8;
               break;
            case A2DP_SBC_BLOCK_LENGTH_FOUR_VALUE:
               EncoderConfig.BlockSize = bsFour;
               BlockSize               = 4;
               break;
            default:
               printf("Unsupported block length (0x%02X)\n", (CodecInfo.BlockLengthSubbandsAllocationMethod & A2DP_SBC_BLOCK_LENGTH_MASK));

               EncoderConfig.BlockSize = bsSixteen;
               BlockSize               = 0;
               break;
         }

         switch(CodecInfo.SamplingFrequencyChannelMode & A2DP_SBC_CHANNEL_MODE_MASK)
         {
            case A2DP_SBC_CHANNEL_MODE_JOINT_STEREO_VALUE:
               EncoderConfig.ChannelMode = cmJointStereo;
               Channels                  = 2;
               break;
            case A2DP_SBC_CHANNEL_MODE_STEREO_VALUE:
               EncoderConfig.ChannelMode = cmStereo;
               Channels                  = 2;
               break;
            case A2DP_SBC_CHANNEL_MODE_DUAL_CHANNEL_VALUE:
               EncoderConfig.ChannelMode = cmDualChannel;
               Channels                  = 2;
               break;
            case A2DP_SBC_CHANNEL_MODE_MONO_VALUE:
               EncoderConfig.ChannelMode = cmMono;
               Channels                  = 1;
               break;
            default:
               printf("Unsuppored channel mode (0x%02X)\n", (CodecInfo.SamplingFrequencyChannelMode & A2DP_SBC_CHANNEL_MODE_MASK));

               EncoderConfig.ChannelMode = cmJointStereo;
               Channels                  = 2;
               break;
         }

         switch(CodecInfo.BlockLengthSubbandsAllocationMethod & A2DP_SBC_ALLOCATION_METHOD_MASK)
         {
            case A2DP_SBC_ALLOCATION_METHOD_LOUDNESS_VALUE:
               EncoderConfig.AllocationMethod = amLoudness;
               break;
            case A2DP_SBC_ALLOCATION_METHOD_SNR_VALUE:
               EncoderConfig.AllocationMethod = amSNR;
               break;
            default:
               printf("Unsupported allocation method (0x%02X)\n", (CodecInfo.BlockLengthSubbandsAllocationMethod & A2DP_SBC_ALLOCATION_METHOD_MASK));

               EncoderConfig.AllocationMethod = amLoudness;
               break;
         }

         switch(CodecInfo.BlockLengthSubbandsAllocationMethod & A2DP_SBC_SUBBANDS_MASK)
         {
            case A2DP_SBC_SUBBANDS_EIGHT_VALUE:
               EncoderConfig.Subbands = sbEight;
               Subbands               = 8;
               break;
            case A2DP_SBC_SUBBANDS_FOUR_VALUE:
               EncoderConfig.Subbands = sbFour;
               Subbands               = 4;
               break;
            default:
               printf("Unsupported number of subbands (0x%02X)\n", (CodecInfo.BlockLengthSubbandsAllocationMethod & A2DP_SBC_SUBBANDS_MASK));

               EncoderConfig.Subbands = sbEight;
               Subbands               = 0;
               break;
         }

         /* The MaximumBitRate field doubles as a BitPool field when the*/
         /* value is < 256. Choose the lesser bit pool of either the    */
         /* sink's maximum supported bit pool or the maximum allowed bit*/
         /* pool (defined as MAXIMUM_BIT_POOL).                         */
         EncoderConfig.MaximumBitRate = ((CodecInfo.MaximumBitPoolValue < MAXIMUM_BIT_POOL) ? CodecInfo.MaximumBitPoolValue : MAXIMUM_BIT_POOL);

         if(SamplesPerSecond != Frequency)
            printf("Frequency mismatch: Requested = %u, Actual = %u\n", SamplesPerSecond, Frequency);

         printf("Initializing SBC encoder using parameters:\n");
         printf("    Freq:         %u\n",  (EncoderConfig.SamplingFrequency == sf16kHz ? 16000 : (EncoderConfig.SamplingFrequency == sf32kHz ? 32000 : (EncoderConfig.SamplingFrequency == sf441kHz ? 44100 : (EncoderConfig.SamplingFrequency == sf48kHz ? 48000 : -1)))));
         printf("    Block Size:   %u\n",  (EncoderConfig.BlockSize == bsFour ? 4 : (EncoderConfig.BlockSize == bsEight ? 8 : (EncoderConfig.BlockSize == bsTwelve ? 12 : (EncoderConfig.BlockSize == bsSixteen ? 16 : -1)))));
         printf("    Channel Mode: %s\n",  (EncoderConfig.ChannelMode == cmMono ? "Mono" : (EncoderConfig.ChannelMode == cmDualChannel ? "DualChan" : (EncoderConfig.ChannelMode == cmStereo ? "Stereo" : (EncoderConfig.ChannelMode == cmJointStereo ? "Joint Stereo" : "UNKNOWN")))));
         printf("    Allocation:   %s\n",  (EncoderConfig.AllocationMethod == amLoudness ? "Loudness" : (EncoderConfig.AllocationMethod == amSNR ? "SNR": "UNKNOWN")));
         printf("    Subbands:     %u\n",  (EncoderConfig.Subbands == sbFour ? 4 : (EncoderConfig.Subbands == sbEight ? 8 : -1)));
         printf("    Bit Pool:     %lu\n", (unsigned long)(EncoderConfig.MaximumBitRate));
         printf("\n");

         if((PlaybackContext.Encoder = SBC_Initialize_Encoder(&EncoderConfig)) != NULL)
         {
            PlaybackContext.SampleSize = 2 * Channels;

            if((BlockSize > 0) && ((ret_val = SBC_CalculateEncoderFrameLength(&EncoderConfig)) > 0))
            {
               PlaybackContext.SBCFrameSize        = (unsigned int)ret_val;
               PlaybackContext.SBCFramesPerPacket  = (MTU / (unsigned int)ret_val);

               /* Only up to 15 SBC frames can be packed into a packet, */
               /* no matter how many could actually fit in the payload. */
               if(PlaybackContext.SBCFramesPerPacket > 15)
                  PlaybackContext.SBCFramesPerPacket = 15;

               /* Calculate the timing delay between packets. Round the */
               /* delay down to the nearest millisecond.                */
               PacketDelayNS      = (((Subbands * BlockSize * (long long)PlaybackContext.SBCFramesPerPacket) * NSEC_PER_SEC) / Frequency);
               PacketDelayNSTrunc = ((PacketDelayNS / NSEC_PER_USEC) * NSEC_PER_USEC);

               PlaybackContext.PacketDelay.tv_sec  = (time_t)(PacketDelayNSTrunc / NSEC_PER_SEC);
               PlaybackContext.PacketDelay.tv_nsec = (long)(PacketDelayNSTrunc % NSEC_PER_SEC);

               PlaybackContext.ExtraTimePerInterval = (PacketDelayNS - PacketDelayNSTrunc);
               PlaybackContext.UnderrunTime         = (NSEC_PER_MSEC - PlaybackContext.ExtraTimePerInterval);

               printf("Timing Parameters:\n");
               printf("    MTU:                %u\n", MTU);
               printf("    Frame Size:         %u\n", PlaybackContext.SBCFrameSize);
               printf("    Frames per Packet:  %u\n", PlaybackContext.SBCFramesPerPacket);
               printf("    Packet Delay (NS):  %llu\n", PacketDelayNS);
               printf("    Packet Delay:       %lu.%09ld sec\n", PlaybackContext.PacketDelay.tv_sec, PlaybackContext.PacketDelay.tv_nsec);
               printf("    Extra Time/Pkt:     %llu\n", PlaybackContext.ExtraTimePerInterval);
               printf("    Underrun Time:      %llu\n", PlaybackContext.UnderrunTime);
               printf("\n");

               /* Allocate buffer with enough room for                  */
               /* SBCFramesPerPacket frames plus one Word_t. The extra  */
               /* Word_t is to hold the 1-byte AVDTP Header Information */
               /* structure. While the structure only consumes one      */
               /* byte, we load the buffer in units of Word_t (2-bytes),*/
               /* so adding two bytes to the buffer size keeps          */
               /* everything word-aligned. To this end, we will start   */
               /* loading the buffer at SBCFrameBuffer[2], store the    */
               /* AVDTP header at SBCFrameBuffer[1], and specificy the  */
               /* buffer to be sent as &(SBCFrameBuffer[1]).            */
               if((PlaybackContext.SBCFrameBuffer = (unsigned char *)calloc(((PlaybackContext.SBCFrameSize * PlaybackContext.SBCFramesPerPacket) + sizeof(Word_t)), sizeof(unsigned char))) != NULL)
               {
                  Initialized = TRUE;
                  ret_val     = 0;
               }
               else
                  printf("Out of memory\n");
            }
            else
               printf("Encoder parameters are invalid -  Block size (%u) or could not determine frame length) (%d)\n", BlockSize, ret_val);
         }
         else
            printf("Could not initialize SBC encoder (%p)\n", PlaybackContext.Encoder);

         if(ret_val != 0)
            free(PlaybackContext.SBCFrameBuffer);
      }
      else
         printf("Unsupported stream type or invalid configuration\n");
   }
   else
      printf("Unable to query audio connection configuration (%d)\n", Result);

   return(ret_val);
}

   /* The following function is responsible for freeing all resources   */
   /* that were previously allocated for an Audio Encoder.              */
void CleanupAudioEncoder(void)
{
   if(Initialized)
   {
      if(PlaybackContext.Encoder)
      {
         SBC_Cleanup_Encoder(PlaybackContext.Encoder);
         PlaybackContext.Encoder = NULL;
      }

      if(PlaybackContext.SBCFrameBuffer)
      {
         free(PlaybackContext.SBCFrameBuffer);
         PlaybackContext.SBCFrameBuffer = NULL;
      }

      Initialized = FALSE;
   }
}

   /* The following function is used to send audio data.  The first     */
   /* parameter is buffer to the audio (4 bytes min, 1 left sample, 1   */
   /* right sample, interleaved).  The second parameter is the length   */
   /* of the audio buffer.  This function blocks until all data is      */
   /* consumed, sending the data at the correct time.  A negative value */
   /* will be returned on error                                         */
int SendAudioData(void *Buffer, unsigned int Length, BD_ADDR_t BD_ADDR)
{
   int                          Result;
   int                          RemainingAudioFrames;
   unsigned int                 BufferRemaining;
   struct timespec              CurrentTime;
   struct timespec              TimeDifference;
   SBC_Encode_Data_t            EncoderInput;
   SBC_Encode_Bit_Stream_Data_t EncoderOutput;

   BufferRemaining  = Length;

   /* Confirm we have a buffer, and the length is a multiple of 4 bytes.*/
   if((Buffer != NULL) && (Length >= AUDIO_ENCODER_MIN_BUFFER_SIZE) && ((Length % 4) == 0))
   {
      /* Each interleaved stereo audio frame contains one sample for    */
      /* each channel. We want to count these pairs of samples.         */
      RemainingAudioFrames = (BufferRemaining / PlaybackContext.BlockAlign);

      /* Process all the data we've been given, so long as an error     */
      /* hasn't occurred.                                               */
      Result = 0;
      while((RemainingAudioFrames > 0) && (Result >= 0))
      {
         /* Process the next frame's worth of samples. We'll just feed  */
         /* as much data as we have into the encoder and it will stop   */
         /* when it has consumed enough for a frame. In case it runs out*/
         /* of samples before finishing the SBC frame, the encoder will */
         /* maintain its state (referenced by PlaybackContext.Encoder)  */
         /* until we feed it more data.                                 */
         EncoderInput.LeftChannelDataPtr  = (Word_t *)Buffer;
         EncoderInput.RightChannelDataPtr = EncoderInput.LeftChannelDataPtr + 1;
         EncoderInput.ChannelDataLength   = RemainingAudioFrames;

         EncoderOutput.BitStreamDataSize  =  (PlaybackContext.SBCFramesPerPacket - PlaybackContext.SBCFrameCount) * PlaybackContext.SBCFrameSize;
         EncoderOutput.BitStreamDataPtr   = &(PlaybackContext.SBCFrameBuffer[2]) + (PlaybackContext.SBCFrameCount * PlaybackContext.SBCFrameSize);

         if((Result = SBC_Encode_Data(PlaybackContext.Encoder, &EncoderInput, &EncoderOutput)) >= 0)
         {
            RemainingAudioFrames = EncoderInput.UnusedChannelDataLength;
            memmove(Buffer, (Buffer + ((EncoderInput.ChannelDataLength - RemainingAudioFrames) * PlaybackContext.BlockAlign)), (RemainingAudioFrames * PlaybackContext.BlockAlign));

            if(Result == SBC_PROCESSING_COMPLETE)
               PlaybackContext.SBCFrameCount += 1;
         }
         else
         {
            /* Encoding has failed. This should never happen, so abort  */
            /* the process now.                                         */
            printf("SBC encoding failed (%d)", Result);
         }

         /* We'll send the next packet as soon as either we have a full */
         /* SBC packet or what we do have encoded is the last of the    */
         /* audio data.                                                 */
         while((PlaybackContext.SBCFrameCount >= PlaybackContext.SBCFramesPerPacket) || ((BufferRemaining == 0) && (PlaybackContext.SBCFrameCount > 0)))
         {
            /* We're definitely in the 'playing' state and we have an   */
            /* SBC packet ready to send.  Next, we'll wait for timing,  */
            /* send the pending SBC packet, and update the timing data. */

            /* If this is the first time we've written a packet to this */
            /* connection (NextWriteTime is zero), initialize the next  */
            /* scheduled transmission time to the current time.         */
            /*  Otherwise, wait for the next transmit time.             */
            if((PlaybackContext.NextWriteTime.tv_sec == 0) && (PlaybackContext.NextWriteTime.tv_nsec == 0))
               clock_gettime(CLOCK_MONOTONIC, &(PlaybackContext.NextWriteTime));
            else
            {
               clock_gettime(CLOCK_MONOTONIC, &CurrentTime);
               if(CmpTimespec(&CurrentTime, &(PlaybackContext.NextWriteTime)) > 0)
               {
                  TimeDifference = DiffTimespec(&CurrentTime, &(PlaybackContext.NextWriteTime));
                  if((TimeDifference.tv_sec > 0) || (TimeDifference.tv_nsec > (50 * NSEC_PER_MSEC)))
                     clock_gettime(CLOCK_MONOTONIC, &(PlaybackContext.NextWriteTime));
               }
               else
               {
                  while(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(PlaybackContext.NextWriteTime), NULL) == EINTR)
                  {
                     /* In case we get interrupted, keep sleeping until */
                     /* we hit the scheduled time.                      */
                  }
               }
            }

            /* Set the AVDTP Header Information structure. SSince we're */
            /* not fragmenting BC frames, all we care about is the      */
            /* number of SBC frames in Sthe buffer.                     */
            PlaybackContext.SBCFrameBuffer[1] = (PlaybackContext.SBCFrameCount & A2DP_SBC_HEADER_NUMBER_FRAMES_MASK);

            /* Send the encoded buffer.  We've been loading the buffer  */
            /* from index [2] to leave room for the AVDTP header while  */
            /* maintaining word-alignment.  The header is only one byte,*/
            /* so we stored the header at index [1] and send using index*/
            /* [1] as our base buffer address.                          */
            Result = AUDM_Send_Encoded_Audio_Data(PlaybackContext.CallbackId, BD_ADDR, ((PlaybackContext.SBCFrameCount * PlaybackContext.SBCFrameSize) + 1), &(PlaybackContext.SBCFrameBuffer[1]));

            if(Result == 0)
            {
               PlaybackContext.PacketCount   += 1;
               PlaybackContext.SBCFrameTotal += PlaybackContext.SBCFrameCount;
               if((PlaybackContext.PacketCount % 100) == 0)
                  printf("Sent SBC packet # %u (%llu total frames)\n", PlaybackContext.PacketCount, PlaybackContext.SBCFrameTotal);


               PlaybackContext.NextWriteTime = AddTimespec(&(PlaybackContext.NextWriteTime), &(PlaybackContext.PacketDelay));
               PlaybackContext.SBCFrameCount = 0;

               if(PlaybackContext.ExtraTimeCount >= PlaybackContext.UnderrunTime)
               {
                  /* The last packet we sent put us over the skew       */
                  /* threashold, so the next packet needs to be sent    */
                  /* after a longer (corrective) interval.              */
                  PlaybackContext.NextWriteTime.tv_nsec += NSEC_PER_MSEC;
                  if(PlaybackContext.NextWriteTime.tv_nsec >= NSEC_PER_SEC)
                  {
                     PlaybackContext.NextWriteTime.tv_nsec -= NSEC_PER_SEC;
                     PlaybackContext.NextWriteTime.tv_sec  += 1;
                  }

                  PlaybackContext.ExtraTimeCount -= PlaybackContext.UnderrunTime;
               }
               else
               {
                  /* After sending the last packet, we're still under   */
                  /* the skew threshold, so we'll use the normal        */
                  /* interval for the next packet.                      */
                  PlaybackContext.ExtraTimeCount += PlaybackContext.ExtraTimePerInterval;
               }
            }
            else
            {
               /* We received an error even though we should still be   */
               /* playing audio.  This is unexpected, so abort.         */
               printf("Playback Thread aborting for error sending packet: %d, %s", Result, ERR_ConvertErrorCodeToString(Result));

               BufferRemaining               = 0;
               PlaybackContext.SBCFrameCount = 0;
            }
         }
      }
   }
   else
      Result = -1;

   return(Result);
}

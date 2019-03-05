/*****< audioencoder.h >*******************************************************/
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
/*   01/13/11  G. Hensley     Initial creation. (Based on LinuxDEVM)          */
/******************************************************************************/
#ifndef __AUDIOENCODERH__
#define __AUDIOENCODERH__

#define AUDIO_ENCODER_MIN_BUFFER_SIZE              4

   /* The following function initializes the Audio encoder.  The first  */
   /* parameter is a valid Audio Manager Data Handler ID (registered via*/
   /* call to the AUDM_Register_Data_Event_Callback() function),        */
   /* followed by the samples per second of the audio source.  The      */
   /* finial parameter is the number channels (1 or 2) of the audio     */
   /* source.  This function will return zero on success and negative on*/
   /* error.                                                            */
int InitializeAudioEncoder(unsigned int CallbackId, unsigned int SamplesPerSecond, unsigned int NumberOfChannels, BD_ADDR_t BD_ADDR);

   /* The following function is responsible for freeing all resources   */
   /* that were previously allocated for an Audio Encoder.              */
void CleanupAudioEncoder(void);

   /* The following function is used to send audio data.  The first     */
   /* parameter is buffer to the audio (4 bytes min, 1 left sample, 1   */
   /* right sample, interleaved).  The second parameter is the length   */
   /* of the audio buffer.  This function blocks until all data is      */
   /* consumed, sending the data at the correct time.  A negative value */
   /* will be returned on error                                         */
int SendAudioData(void *Buffer, unsigned int Length, BD_ADDR_t BD_ADDR);

#endif


/*****< uhid.c >***************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  UHID - Stonestreet One interface for Linux UHID.                          */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/23/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>

#include "UHID.h"
#include "UHIDTyp.h"

   /* The following represents the format of a UHID Device.             */
typedef struct _tagUHID_Device_t
{
	int			           uhid_fd;
	guint			           uhid_watch_id;
   BD_ADDR_t              DeviceAddr;
   UHID_Event_Callback_t  UHIDEventCallback;
   void                  *CallbackParameter;
} UHID_Device_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static UHID_Device_t UHIDDevice;

   /* Internal Function Prototypes.                                     */
static gboolean UHID_Event_Callback(GIOChannel *io, GIOCondition cond, gpointer user_data);

   /* The following callback is called whenever a UHID Event occurrs.   */
static gboolean UHID_Event_Callback(GIOChannel *io, GIOCondition cond, gpointer user_data)
{
	int                 FileDescriptor;
	ssize_t             BytesRead;
   gboolean            Result;
   UHID_Event_Data_t   EventData;
   UHID_Command_Data_t CommandData;

   /* Make sure that an error didn't occur.                             */
	if(!(cond & (G_IO_ERR | G_IO_NVAL)))
   {
      /* Get the file descriptor.                                       */
      FileDescriptor = g_io_channel_unix_get_fd(io);
      memset(&CommandData, 0, sizeof(CommandData));

      BytesRead = read(FileDescriptor, &CommandData, sizeof(CommandData));
      if(BytesRead > 0)
      {
         printf("UHID event type %u received", (unsigned int)CommandData.EventType);

         switch(CommandData.EventType)
         {
            case UHID_OUTPUT:
               EventData.EventType                                  = CommandData.EventType;
               EventData.EventData.OutputReportEventData.ReportType = CommandData.EventData.OutputReportData.ReportType;
               EventData.EventData.OutputReportEventData.Size       = CommandData.EventData.OutputReportData.Size;

               BTPS_MemCopy(EventData.EventData.OutputReportEventData.Data, CommandData.EventData.OutputReportData.Data, EventData.EventData.OutputReportEventData.Size);

               if(UHIDDevice.UHIDEventCallback)
                  (*(UHIDDevice.UHIDEventCallback))(UHIDDevice.DeviceAddr, &EventData, UHIDDevice.CallbackParameter);
               break;
            case UHID_FEATURE:
               EventData.EventType                                     = CommandData.EventType;
               EventData.EventData.FeatureReportEventData.id           = CommandData.EventData.FeatureReportRequestData.id;
               EventData.EventData.FeatureReportEventData.ReportNumber = CommandData.EventData.FeatureReportRequestData.ReportNumber;
               EventData.EventData.FeatureReportEventData.ReportType   = CommandData.EventData.FeatureReportRequestData.ReportType;

               if(UHIDDevice.UHIDEventCallback)
                  (*(UHIDDevice.UHIDEventCallback))(UHIDDevice.DeviceAddr, &EventData, UHIDDevice.CallbackParameter);
               break;
            case UHID_OUTPUT_EV:
               printf("UHID output event: type %d code %d value %d",
                        (int)CommandData.EventData.OutputReportEvData.Type,
                        (int)CommandData.EventData.OutputReportEvData.Code,
                        (int)CommandData.EventData.OutputReportEvData.Value);
               break;
            default:
               break;
         }

         /* return success.                                             */
         Result = TRUE;
      }
      else
      {
         printf("uhid-dev read: %s(%d)", strerror(errno),errno);

         Result = FALSE;
      }
   }
   else
      Result = FALSE;

	return(Result);
}

   /* The following function is a utility function that is used to      */
   /* initialize the UHID Interface.                                    */
Boolean_t UHID_Initialize(void)
{
   Boolean_t     ret_val;
	GIOChannel   *io;
	GIOCondition  cond = G_IO_IN | G_IO_ERR | G_IO_NVAL;

   /* Make sure we aren't already initialized.                          */
   if(UHIDDevice.uhid_fd <= 0)
   {
      UHIDDevice.uhid_fd = open("/dev/uhid", (O_RDWR | O_CLOEXEC));
      if(UHIDDevice.uhid_fd > 0)
      {
         /* Configure the GLIB callback that will be called when an     */
         /* event occurs.                                               */
         io = g_io_channel_unix_new(UHIDDevice.uhid_fd);
         g_io_channel_set_encoding(io, NULL, NULL);
         UHIDDevice.uhid_watch_id = g_io_add_watch(io, cond, UHID_Event_Callback, NULL);
         g_io_channel_unref(io);

         /* Return success to the caller.                               */
         ret_val = TRUE;
      }
      else
      {
         printf("\r\nFailed to open UHID device: %s (%d).\r\n\r\n", strerror(errno), errno);

         printf("UHID services are not available to this device and HID reports will not be forwarded to the system.\r\n");

         if(errno == 2)
            printf("If you wish for HID reports to be processed then please update the kernel and verify support for UHID.\r\n");

         if(errno == 13)
            printf("If you wish for HID reports to be processed then please run the application as root.\r\n"); 

         ret_val = FALSE;
      }
   }
   else
      ret_val = FALSE;

	return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* cleanup the UHID Interface.                                       */
Boolean_t UHID_Cleanup(void)
{
   Boolean_t ret_val;

   /* Make sure that the device is already open.                        */
   if(UHIDDevice.uhid_fd >= 0)
   {
      /* Remove the GLIB event callback.                                */
      if(UHIDDevice.uhid_watch_id)
      {
   		g_source_remove(UHIDDevice.uhid_watch_id);
   		UHIDDevice.uhid_watch_id = 0;
      }

      /* Close the UHID file descriptor.                                */
   	close(UHIDDevice.uhid_fd);
   	UHIDDevice.uhid_fd = -1;

      /* Return success to the caller.                                  */
      ret_val = TRUE;
   }
   else
      ret_val = FALSE;

	return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* create a UHID device with the specified information.              */
Boolean_t UHID_Create_Device(BD_ADDR_t DeviceAddr, HIDS_HID_Information_Data_t *HIDSInformation, unsigned int ReportDescriptorLength, Byte_t *ReportDescriptor, UHID_Event_Callback_t EventCallback, void *CallbackParameter)
{
   Boolean_t           ret_val;
   UHID_Command_Data_t CommandData;

   /* Verify that the input parameters are semi-valid                   */
   if((!COMPARE_NULL_BD_ADDR(DeviceAddr)) && (HIDSInformation) && (EventCallback))
   {
      /* Make sure that the device is already open.                     */
      if(((UHIDDevice.uhid_fd <= 0) && (UHID_Initialize())) || (UHIDDevice.uhid_fd > 0))
      {
         /* Format the command to create the device.                    */
      	BTPS_MemInitialize(&CommandData, 0, sizeof(CommandData));
         
         CommandData.EventType                                          = UHID_CREATE;
      	CommandData.EventData.CreateRequestData.VendorID               = 0;
      	CommandData.EventData.CreateRequestData.ProductID              = 0;
      	CommandData.EventData.CreateRequestData.ProductVersion         = HIDSInformation->Version;
      	CommandData.EventData.CreateRequestData.CountryCode            = HIDSInformation->CountryCode;
      	CommandData.EventData.CreateRequestData.BusType                = UHID_BUS_TYPE_BLUETOOTH;
      	CommandData.EventData.CreateRequestData.ReportDescriptor       = ReportDescriptor;
      	CommandData.EventData.CreateRequestData.ReportDescriptorLength = ReportDescriptorLength;
      	BTPS_StringCopy((char *)CommandData.EventData.CreateRequestData.DeviceName, "Bluetopia-HOG-Device");

      	if(write(UHIDDevice.uhid_fd, &CommandData, sizeof(CommandData)) > 0)
         {
            /* Store the callback information.                          */
            UHIDDevice.UHIDEventCallback = EventCallback;
            UHIDDevice.CallbackParameter = CallbackParameter;
            UHIDDevice.DeviceAddr        = DeviceAddr;

            /* Return success to the caller.                            */
            ret_val                      = TRUE;
         }
         else
         {
            printf("\r\nFailed to create UHID device: %s\r\n", strerror(errno));

            ret_val = FALSE;
         }
      }
      else
         ret_val = FALSE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is a utility function that is used to      */
   /* destroy a previously created UHID Device.                         */
Boolean_t UHID_Destroy_Device(void)
{
   Boolean_t           ret_val;
   UHID_Command_Data_t CommandData;

   /* Make sure that the device is already open.                        */
   if(UHIDDevice.uhid_fd > 0)
   {
      /* Destroy the device.                                            */
      memset(&CommandData, 0, sizeof(CommandData));
   	CommandData.EventType = UHID_DESTROY;
   	if (write(UHIDDevice.uhid_fd, &CommandData, sizeof(CommandData)) < 0)
   		printf("Failed to destroy UHID device: %s", strerror(errno));

      /* Cleanup the module.                                            */
      UHID_Cleanup();

      /* return success to the caller.                                  */
      ret_val = TRUE;
   }
   else
      ret_val = FALSE;

	return(ret_val);
}

   /* The following function is a utility function that is used to write*/
   /* a UHID device.                                                    */
Boolean_t UHID_Write_Device(UHID_Command_Data_t *CommandData)
{
   Boolean_t ret_val;
   
   /* Make sure that the device is already open and the command         */
   /* parameters are correct.                                           */
   if((UHIDDevice.uhid_fd > 0) && (CommandData))
   {
      /* Write the command to the UHID device.                          */
   	if(write(UHIDDevice.uhid_fd, CommandData, sizeof(UHID_Command_Data_t)) > 0)
      {
         /* Return success to the caller.                               */
         ret_val = TRUE;
      }
      else
      {
   		printf("Failed to write command to UHID device: %s", strerror(errno));

         ret_val = FALSE;
      }
   }
   else
      ret_val = FALSE;

	return(ret_val);
}


/*****< btpmsrvr.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMSRVR - Bluetopia Platform Manager Main Application Entry point for    */
/*             Linux.                                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/07/10  D. Lange        Initial creation.                              */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "SS1BTPM.h"             /* BTPM API Prototypes and Constants.        */

#include "BTPMSRVR.h"            /* BTPM Main Application Proto./Constants.   */

#define USB_PARAMETER_VALUE                         (0)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* USB.              */

#define UART_PARAMETER_VALUE                        (1)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* UART.             */

#define BCSP_PARAMETER_VALUE                        (2)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* BCSP.             */


#define SIBS_PARAMETER_VALUE                        (3)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* SIBS.             */


   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   int                              ret_val;
   char                            *endptr = NULL;
   Boolean_t                        COMMDevice = FALSE;
   unsigned int                     CommPortNumber;
   unsigned int                     BaudRate;
   HCI_COMM_Protocol_t              Protocol = cpUART_RTS_CTS;
   BTPM_Initialization_Info_t       InitializationInfo;
   BTPM_Debug_Initialization_Data_t DebugInitializationInfo;
   HCI_DriverInformation_t          HCI_DriverInformation;
   HCI_DriverInformation_t         *HCI_DriverInformationPtr = NULL;
   DEVM_Initialization_Data_t       DEVMInitializationInfo;

   /* First, check to see if the right number of parameters where       */
   /* entered at the Command Line.                                      */
   if(argc >= 2)
   {
      /* The minimum number of parameters were entered at the Command   */
      /* Line.  Check the first parameter to determine if the           */
      /* application is supposed to run in UART or USB mode.            */
      switch(strtol(argv[1], &endptr, 10))
      {
         case USB_PARAMETER_VALUE:
            /* The Transport selected was USB, setup the Driver         */
            /* Information Structure to use USB as the HCI Transport.   */
            HCI_DRIVER_SET_USB_INFORMATION(&HCI_DriverInformation);

            HCI_DriverInformationPtr = &HCI_DriverInformation;
            break;

         case BCSP_PARAMETER_VALUE:
            Protocol = cpBCSP;
            COMMDevice = TRUE;
            break;
         case SIBS_PARAMETER_VALUE:
            Protocol = cpSIBS_RTS_CTS;
            COMMDevice = TRUE;
            break;
         case UART_PARAMETER_VALUE:
            Protocol   = cpUART_RTS_CTS;
            COMMDevice = TRUE;
            break;
         default:
            break;
      }

      if(COMMDevice)
      {
         /* The Transport selected was UART, check to see if the number */
         /* of parameters entered on the command line is correct.       */
         if(argc >= 3)
         {
            if(argc >= 4)
            {
               /* The correct number of parameters were entered, convert*/
               /* the command line parameters to the proper format.     */
               BaudRate = strtol(argv[3], &endptr, 10);
            }
            else 
               BaudRate = 115200;
               
            /* Either a port number or a device file can be used.       */
            if((argv[2][0] >= '0') && (argv[2][0] <= '9'))
            {
               CommPortNumber = strtol(argv[2], &endptr, 10);
               HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, CommPortNumber, BaudRate, Protocol);
            }
            else
            {
               HCI_DRIVER_SET_EXTENDED_COMM_INFORMATION_DEVICE_NAME(&HCI_DriverInformation, -1, BaudRate, Protocol, 0, argv[2]);
            }

            HCI_DriverInformationPtr = &HCI_DriverInformation;
         }
      }
   }

   if(HCI_DriverInformationPtr)
   {
      /* This is an initialization that specifies the Debug output string  */
      /* prefix.                                                           */
   
      /* Let's go ahead and specify a Platform Specific Debugging string.  */
      DebugInitializationInfo.PlatformSpecificInitData = "SS1BTPMS";
      /* Now that we have initialized the Debug information, let's go ahead*/
      /* and initialize the correct global configuration structure.        */
      BTPS_MemInitialize(&InitializationInfo, 0, sizeof(InitializationInfo));
   
      BTPS_MemInitialize(&DEVMInitializationInfo, 0, sizeof(DEVM_Initialization_Data_t));
      DEVMInitializationInfo.HCI_DriverInformation = HCI_DriverInformationPtr;
      
      InitializationInfo.DebugInitializationInfo         = &DebugInitializationInfo;
      InitializationInfo.DeviceManagerInitializationInfo = &DEVMInitializationInfo;
   
      /* Do nothing other than call the Library entry point.               */
      ret_val = BTPM_Main(&InitializationInfo, NULL, NULL);
   
   }
   else
   {
       printf("Invalid Parameter\n");
       printf("  Format: SS1BTPS [USB = 0, UART = 1, BCSP = 2, SIBS = 3 Flag] [IF !USB [Comm Port or Device File] <Baud Rate ; Default 115200>\r\n");
       ret_val = -1;
   }
   
   return(ret_val);
}


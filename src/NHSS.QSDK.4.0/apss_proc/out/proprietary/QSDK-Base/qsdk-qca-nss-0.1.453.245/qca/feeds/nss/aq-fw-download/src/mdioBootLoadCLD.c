/* mdioBootLoadCLD.c */

/************************************************************************************
* Copyright (c) 2015, Aquantia
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
* $File: //depot/icm/proj/Dena/rev1.0/c/Systems/tools/windows/flashUtilities/src/mdioBootLoadCLD.c $
*
* $Revision: #12 $
*
* $DateTime: 2014/05/19 15:34:49 $
*
* $Author: joshd $
*
* $Label: $
*
************************************************************************************/

/*! \file
This file contains the main (int, char**) file for the mdioBootLoadCLD program, which burns a flash image into a target
Aquantia PHY using the AQ_API. This program calls the API function: <BR><BR>

    uint8_t AQ_API_WriteBootLoadImage (uint8_t PHY_ID, uint8_t *image, uint16_t *crc16) <BR><BR>

to boot load a cld flash image into an Aquantia PHY */

/*! \addtogroup mdioBootLoad
@{
*/



/*! \def DEBUG
Uncomment this to compile in debug mode.  This sets the source to an arbitrary file, defined by DEBUG_FILENAME,
and an arbitrary PHY_ID, defined by DEBUG_PHY_ID. */
/* #define DEBUG */

/*! The debug source file name */
#define DEBUG_FILENAME "HelloWorld.cld"

/*! The debug PHY ID */
#define DEBUG_PHY_ID 0


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "AQ_API.h"

int sock;
char devname[7];

int sock_init()
{
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int main ( int argc, char **argp)
{
  /* declare local variables */
  FILE *pFile;
  uint8_t* image;
  uint8_t byte;
  unsigned int PHY_ID, boardNameSpecified = 0;
  AQ_Retcode resultCode;
  AQ_Retcode resultCodes[4];
  uint32_t i;
  uint32_t imageSize;
  char* pEnd;
  char inputBuffer[100];
  char** deviceList;
  char sourceFileName[1000];
  char boardName[100];
  AQ_API_Port targetPort0;
  AQ_API_Port* targetPorts[1];
  AQ_API_Port broadcastPort;
  unsigned int numDevices;
  unsigned int provisioningAddresses[1] = {0};
  uint32_t reg1, reg2;

  targetPorts[0] = &targetPort0;

  if(argc < 2) {
	  fprintf (stderr, "enter file name\n");
	  return (101);
  }

  /*Interface name will be eth1*/
  strcpy(devname, "eth1");

  /*Copy the file name from command line arg*/
  strcpy (sourceFileName, argp[1]);

  /* PHY Address will be always 16 */
  PHY_ID = 0x10;

  /* FIXME: set port and device type */
  targetPort0.device = AQ_DEVICE_HHD;
  targetPort0.PHY_ID = PHY_ID;

  broadcastPort.device = AQ_DEVICE_HHD;
  broadcastPort.PHY_ID = PHY_ID;

  /* open the source in binary read mode */
  pFile = fopen(sourceFileName, "rb");
  if (pFile == NULL)
  {
    fprintf (stderr, "Unable to open source file %s\n", sourceFileName);
    return (101);
  }
  fseek (pFile, 0, SEEK_END);
  imageSize = ftell (pFile);

  image = (uint8_t*) malloc (imageSize * sizeof(uint8_t));
  fseek (pFile, 0, SEEK_SET);

  /* load the file */
  for (i = 0; i < imageSize; i++)
  {
    byte = (uint8_t) fgetc (pFile);
    image[i] = byte;
  }
  fclose(pFile);


  if (sock_init() < 0)
  {
    fprintf (stderr, "Unable to initialize interface\n");
    return (200);
  }

   /* Write in the Auantia phy scratch pad register,
    * read back the same reg and match the values written.
    */
   AQ_API_MDIO_Write(PHY_ID, 0x1e, 0x300, 0xdead);
   AQ_API_MDIO_Write(PHY_ID, 0x1e, 0x301, 0xbeaf);
   reg1 = AQ_API_MDIO_Read(PHY_ID, 0x1e, 0x300);
   reg2 = AQ_API_MDIO_Read(PHY_ID, 0x1e, 0x301);
   if(reg1 != 0xdead && reg2 != 0xbeaf) {
	printf (stderr, "Scratchpad Read/Write test fail\n");
	return (101);
   }

  /* call the boot-load function */
  resultCode = AQ_API_WriteBootLoadImage(targetPorts, 1, provisioningAddresses, resultCodes, &imageSize, image, PHY_ID, &broadcastPort);

  switch (resultCode)
  {
    case 0:
      printf("Image load good - mailbox CRC-16 matches\n");
      free (image);
      close(sock);
      return 0;

    case 1:
      fprintf (stderr, "CRC-16 on file is bad\n");
      free (image);
      close(sock);
      return 1;

    case 2:
      fprintf (stderr, "CRC-16 check on image load failed (mailbox CRC-16 check)\n");
      free (image);
      close(sock);
      return 2;

    default:
      fprintf (stderr, "Invalid return code\n");
      free (image);
      close(sock);
  }
  return 12;
}
/*@}*/

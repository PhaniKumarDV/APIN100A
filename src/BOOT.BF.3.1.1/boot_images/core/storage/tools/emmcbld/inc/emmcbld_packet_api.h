#ifndef _EMMCBLD_PACKET_API_H_
#define _EMMCBLD_PACKET_API_H_

/*==================================================================
 *
 * FILE:        emmcbld_packet_api.h
 *
 * SERVICES:    None
 *
 * DESCRIPTION:
 *   This header file contains externalized definitions from the packet layer.
 *   It is used by the stream buffering layer.
 *
 * Copyright (c) 2010 Qualcomm Technologies Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *==================================================================*/

/*===================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/inc/emmcbld_packet_api.h#1 $
 *   $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why 
 * ----------   ---     ----------------------------------------------
 * 2010-10-29   rh      Initial version
 *==================================================================*/

/*===================================================================
 *
 *                     Include Files
 *
 ====================================================================*/
#include "comdef.h"

/** \mainpage EMMCBLD USB interface packet processor interface
 *
 * EMMCBLD exposes a set of APIs for adding external modules to
 * the EMMCBLD binary to allow extra function to be added to EMMCBLD
 * without having to duplicate the USB interface functionality
 *
 */


/**
 * @defgroup EMMCBLD USB packet processor interface
 *
 * Basic interface for connecting to the EMMCBLD USB packet processor
 *
 */

/*@{*/

/**
 * Structure for exchanging incoming and outgoing
 *  packet information for the external command handler 
 */
struct stream_dl_packet_info
{
   uint8 *buffer;           /**< Incoming packet buffer, contain command information and payload */ 
   uint32 in_pckt_length;   /**< Incoming packet buffer length */ 
   uint8 *resp_buffer;      /**< Outgoing response buffer */
   uint32 *out_pckt_length; /**< Outgoing response buffer length, 0 indicate no response data */
};

/**
 * Structure to hold function pointers that contain command handling function
 */
struct emmcbld_init_info
{
   boolean (*handle_command)(struct stream_dl_packet_info *pckt_data); /**< 
                              * Packet processor will call this function to process unknown commands */
   boolean (*handle_hello_command)(void); /**< 
                              * This function is called during the process of hello command
                              * Used by some external function to initialize module after
                              * USB driver is connected */
};

/**
 * Define the miximum number of external function that can be connected to EMMCBLD
 */
#define EMMCBLD_MAX_NUM_EXT_INIT_FN    4

/** \details
 *  This function transmits an error response with the specified error code and
 *  specified error message.
 *
 * @param[in] code
 *    A 32 bit error code to be logged by the PC side software
 *
 * @param[in] message
 *    Error text to be logged by the PC side software
 *
 */
void transmit_error (uint32 code, char *message);

/** \details
 *  This function transmits the specified log message.
 *
 * @param[in] message
 *    Error text to be logged by the PC side software
 *
 */
void xmit_log_message (char *message);

/** \details
 *  This setup the prototype for the init function that is called during
 *  the startup of EMMCBLD to initialize all the attached external functional block
 *
 * @param[in] api_info
 *    Pointer to the emmcbld_init_info structure which specify the exposed
 *    API to be called to process application specific commands
 *
 */
void emmcbld_init_fn1(struct emmcbld_init_info *api_info);

/** \details
 *  This setup the prototype for the init function that is called during
 *  the startup of EMMCBLD to initialize all the attached external functional block
 *
 * @param[in] api_info
 *    Pointer to the emmcbld_init_info structure which specify the exposed
 *    API to be called to process application specific commands
 *
 */

void emmcbld_init_fn2(struct emmcbld_init_info *api_info);
/** \details
 *  This setup the prototype for the init function that is called during
 *  the startup of EMMCBLD to initialize all the attached external functional block
 *
 * @param[in] api_info
 *    Pointer to the emmcbld_init_info structure which specify the exposed
 *    API to be called to process application specific commands
 *
 */

void emmcbld_init_fn3(struct emmcbld_init_info *api_info);
/** \details
 *  This setup the prototype for the init function that is called during
 *  the startup of EMMCBLD to initialize all the attached external functional block
 *
 * @param[in] api_info
 *    Pointer to the emmcbld_init_info structure which specify the exposed
 *    API to be called to process application specific commands
 *
 */
void emmcbld_init_fn4(struct emmcbld_init_info *api_info);


#endif /* _HOSTDL_PACKET_API_H_ */

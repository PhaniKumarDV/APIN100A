/*****< ss1btusb.c >***********************************************************/
/* Copyright (c) 2015, The Linux Foundation. All rights reserved.             */
/*                                                                            */
/* Permission to use, copy, modify, and/or distribute this software for       */
/* any purpose with or without fee is hereby granted, provided that           */
/* the above copyright notice and this permission notice appear in all        */
/* copies.                                                                    */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL              */
/* WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED              */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE           */
/* AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL       */
/* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA         */
/* OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER          */
/* TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR           */
/* PERFORMANCE OF THIS SOFTWARE.                                              */
/*                                                                            */
/* Copyright 2007 - 2014 Qualcomm Atheros, Inc.                               */
/******************************************************************************/
/*                                                                            */
/*  SS1BTUSB - Main Entry and Exit Points for Stonestreet One Bluetooth HCI   */
/*             USB Device Driver (Linux Only).                                */
/*                                                                            */
/******************************************************************************/
#include <linux/module.h>       /* Included for module MACRO's.               */
#include <linux/init.h>         /* Included for MACRO's used in marking module*/
                                /* initialization and cleanup functions.      */
#include <linux/kernel.h>       /* Included for printk().                     */
#include <linux/slab.h>         /* Included for kmalloc() and kfree().        */
#include <linux/usb.h>          /* Included for USB Prototypes/Constants.     */
#include <asm/uaccess.h>        /* Included for copy_to/from_user().          */
#include <asm/string.h>         /* Included for memcpy()/memset().            */
#include <asm/atomic.h>         /* Included for Atomic Operations functions.  */
#include <linux/usb.h>          /* Included for usb interfaces                */
#include <linux/spinlock.h>     /* Included for spinclock functions           */

#include "SS1BTUSB.h"           /* Main Module Prototypes/Constants.          */

#include <linux/version.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/interrupt.h>

#include <linux/errno.h>

//#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE

   #define DEBUGPRINT(format, args...)  printk(format, ##args)

#else

   #define DEBUGPRINT(format, args...)

#endif

   /* Defines the Device Name of the SS1 Bluetooth USB Driver.          */
#define SS1BTUSB_DRIVER_NAME            "SS1BTUSB"

#define SS1BTUSB_CLASS_NAME             (SS1BTUSB_DEVICE_NAME_PREFIX "%d")

#define SS1BTUSB_CLASS_MINOR_BASE       192

   /* The USB Class/Subclass/Protocol ID's that follow are taken from   */
   /* the Bluetooth Specification Version 1.1 from the Chapter entitled */
   /* "HCI USB Transport Layer.".                                       */
#define BLUETOOTH_USB_CLASS_ID          0xE0    /* Bluetooth Defined USB      */
                                                /* Device Class (Wireless     */
                                                /* Controller Device).        */

#define BLUETOOTH_USB_SUBCLASS_ID       0x01    /* Bluetooth Defined USB      */
                                                /* Device Subclass (RF        */
                                                /* Controller Device).        */

#define BLUETOOTH_USB_PROTOCOL_ID       0x01    /* Bluetooth Defined USB      */
                                                /* Device Protocol (Bluetooth */
                                                /* Programming Protocol).     */

#define NUMBER_USB_INTERFACES           0x02    /* This number represents the */
                                                /* number of concurrent USB   */
                                                /* Interfaces that are active */
                                                /* at any given time.         */

#define NUMBER_ENDPOINTS_INTERFACE_0    0x03    /* This number represents the */
                                                /* number of USB Endpoints    */
                                                /* that are present on        */
                                                /* Interface 0 (Not including */
                                                /* the Control Endpoint (0).  */

#define NUMBER_ENDPOINTS_INTERFACE_1    0x02    /* This number represents the */
                                                /* number of USB Endpoints    */
                                                /* that are present on        */
                                                /* Interface 1.               */

#define BLUETOOTH_USB_INTERFACE_0       0x00    /* This number represents the */
                                                /* interface number of the    */
                                                /* interface containing the   */
                                                /* HCI Event and ACL Data     */
                                                /* Endpoints.                 */

#define BLUETOOTH_USB_INTERFACE_1       0x01    /* This number represents the */
                                                /* interface number of the    */
                                                /* interface containing the   */
                                                /* SCO Data Enpoints.         */

#define ISOC_RX_BUFFER_SIZE            16384    /* The size of the circular   */
                                                /* buffer to hold the received*/
                                                /* audio data.                */

#define NON_ISOC_RX_BUFFER_SIZE        65536    /* The size of the circular   */
                                                /* buffer to hold received    */
                                                /* non-audio data.            */

#define MAX_ISO_BUFFERS                    3    /* Number of extra ping pong  */
                                                /* Isochronous buffers for    */
                                                /* buffering received data.   */

#define MAX_ISO_PACKETS                   16    /* The number of ISO packets  */
                                                /* asked for as urb space is  */
                                                /* allocated.                 */

#define MAX_WRITE_BUFFERS                 10    /* Maximum Number of          */
                                                /* Outstanding Write Requests.*/

#define BULK_READ_SIZE                  4096    /* Size of Read Buffer that   */
                                                /* is submitted to Bulk Read  */
                                                /* endpoint (in URB).         */

#define BULK_WRITE_SIZE                 4096    /* Size of Write Buffer that  */
                                                /* is submitted to Bulk Write */
                                                /* endpoint (in URB).         */

#define CONTROL_WRITE_SIZE               260    /* Size of Write Buffer that  */
                                                /* is submitted to Control    */
                                                /* Write endpoint (in URB).   */


#define HCI_CTRL_REQ                    0x20    /* Value used as the          */
                                                /* bRequestType member of the */
                                                /* control transfer to specify*/
                                                /* Bluetooth HCI Command Data.*/

   /* The following declaration List Entry Structure.                   */
typedef struct _tagListEntry_t
{
   struct _tagListEntry_t *NextEntry;
   struct _tagListEntry_t *PrevEntry;
} ListEntry_t;

   /* The following Macros are used to retreive the Next/Prev List Entry*/
   /* given a Current List Entry.                                       */
#define NEXT_ENTRY(_x)                 (((ListEntry_t *)_x)->NextEntry)
#define PREV_ENTRY(_x)                 (((ListEntry_t *)_x)->PrevEntry)

   /* Predeclaration of the USBBufferInformation_t struct.              */
struct _tagUSBBufferInformation_t;

   /* Predeclaration of the USBDeviceInformation_t struct.              */
struct _tagUSBDeviceInformation_t;

   /* The following declaration is used to track an URB and associated  */
   /* URB Buffer.                                                       */
typedef struct _tagURBBufferInfo_t
{
   ListEntry_t                        ListEntry;
   struct _tagUSBBufferInformation_t *USBBufferInformation;
   unsigned char                     *URBBuffer;
   struct urb                        *URB;
} URBBufferInfo_t;

typedef struct _tagSCOAlternateInterfaceMapping_t
{
   unsigned int                    SCOEnableValue;
   unsigned int                    RequiredBandwidth;
   unsigned int                    AlternateSetting;
   unsigned int                    MaxPacketSize;
   struct usb_host_interface      *HostInterface;
   struct usb_endpoint_descriptor *InEndpointDescriptor;
   struct usb_endpoint_descriptor *OutEndpointDescriptor;
} SCOAlternateInterfaceMapping_t;

MODULE_LICENSE("Dual BSD/GPL");

static const SCOAlternateInterfaceMapping_t SCOAlternateInterfaceRequirements[] =
{
   { BTUSB_SCO_DATA_NO_CHANNELS,                                         0, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_ONE_EIGHT_BIT_CHANNEL,                               9, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_TWO_EIGHT_BIT_CHANNELS,                             17, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_THREE_EIGHT_BIT_CHANNELS,                           25, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_CHANNEL,                            17, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_TWO_SIXTEEN_BIT_CHANNELS,                           33, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_THREE_SIXTEEN_BIT_CHANNELS,                         49, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_WBS_CHANNEL,                        33, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_CHANNEL_ONE_SIXTEEN_BIT_WBS_CHANNEL,49, 0, 0, NULL, NULL, NULL },
   { BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_MSBC_CHANNEL,                       63, 0, 0, NULL, NULL, NULL }
} ;

#define NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS    (sizeof(SCOAlternateInterfaceRequirements)/sizeof(SCOAlternateInterfaceMapping_t))

#define NEW_FW_DL_MECHANISM
#ifdef NEW_FW_DL_MECHANISM

#define BT_ERR(fmt, arg...)             printk(KERN_ERR "%s: " fmt "\n" , __func__ , ## arg)
#define BT_INFO(fmt, arg...)            printk(KERN_INFO "Bluetooth: " fmt "\n" , ## arg)
#define BT_DBG(fmt, arg...)             pr_debug("%s: " fmt "\n" , __func__ , ## arg)

#include <linux/firmware.h>
#define ROME1_1_USB_RAMPATCH_FILE       "ar3k/rampatch_1.1.img"
#define ROME1_1_USB_NVM_FILE            "ar3k/nvm_tlv_usb_1.1.bin"

#define ROME2_1_USB_RAMPATCH_FILE       "ar3k/rampatch_tlv_usb_2.1.tlv"
#define ROME2_1_USB_NVM_FILE            "ar3k/nvm_tlv_usb_2.1.bin"

#define ROME3_0_USB_RAMPATCH_FILE       "ar3k/rampatch_tlv_usb_3.0.tlv"
#define ROME3_0_USB_NVM_FILE            "ar3k/nvm_tlv_usb_3.0.bin"

#define ROME3_2_USB_RAMPATCH_FILE       "ar3k/rampatch_tlv_usb_3.2.tlv"
#define ROME3_2_USB_NVM_FILE            "ar3k/nvm_tlv_usb_3.2.bin"

#define TF1_1_USB_RAMPATCH_FILE         "ar3k/rampatch_tlv_usb_tf_1.1.tlv"
#define TF1_1_USB_NVM_FILE              "ar3k/nvm_tlv_usb_tf_1.1.bin"


#define NPL1_0_USB_RAMPATCH_FILE        "ar3k/rampatch_tlv_usb_npl_1.0.tlv"
#define NPL1_0_USB_NVM_FILE             "ar3k/nvm_tlv_usb_npl_1.0.bin"

#define ROME2_1_USB_RAMPATCH_HEADER     sizeof(struct rome2_1_version)
#define ROME1_1_USB_RAMPATCH_HEADER     sizeof(struct rome1_1_version)

#define ROME1_1_USB_NVM_HEADER          0x04
#define ROME2_1_USB_NVM_HEADER          0x04

#define ROME1_1_USB_CHIP_VERSION        0x101
#define ROME2_1_USB_CHIP_VERSION        0x200
#define ROME3_0_USB_CHIP_VERSION        0x300
#define ROME3_2_USB_CHIP_VERSION        0x302
#define NPL1_0_USB_CHIP_VERSION         0xc0100

#define ATH3K_DNLOAD                    0x01
#define ATH3K_GETSTATE                  0x05
#define ATH3K_SET_NORMAL_MODE           0x07
#define ATH3K_GETVERSION                0x09
#define USB_REG_SWITCH_VID_PID          0x0a

#define ATH3K_MODE_MASK                 0x3F
#define ATH3K_NORMAL_MODE               0x0E

#define ATH3K_PATCH_UPDATE              0xA0
#define ATH3K_SYSCFG_UPDATE             0x60
#define ATH3K_PATCH_SYSCFG_UPDATE       (ATH3K_PATCH_UPDATE | ATH3K_SYSCFG_UPDATE)

#define USB_REQ_DFU_DNLOAD              1
#define BULK_SIZE                       4096
#define FW_HDR_SIZE                     20

#define ATH3K_XTAL_FREQ_26M             0x00
#define ATH3K_XTAL_FREQ_40M             0x01
#define ATH3K_XTAL_FREQ_19P2            0x02
#define ATH3K_NAME_LEN                  0xFF

#define BTUSB_ATH3012                   0x80

struct ath3k_version
{
   unsigned int  rom_version;
   unsigned int  build_version;
   unsigned int  ram_version;
   unsigned char ref_clock;
   unsigned char reserved1[0x03];
   unsigned char clkClass;
   unsigned char soc_version;
   unsigned char reserved2[0x02];
};

struct __packed rome1_1_version
{
   u8  type;
   u8  length[3];
   u8  sign_ver;
   u8  sign_algo;
   u8  resv1[2];
   u16 product_id;
   u16 build_ver;
   u16 patch_ver;
   u8  resv2[2];
   u32 entry_addr;
};

struct __packed rome2_1_version
{
   u8  type;
   u8  length[3];
   u32 total_len;
   u32 patch_len;
   u8  sign_ver;
   u8  sign_algo;
   u8  resv1[2];
   u16 product_id;
   u16 build_ver;
   u16 patch_ver;
   u8  resv2[2];
   u32 entry_addr;
};

#endif

static struct usb_device_id BluetoothDeviceIDTable[] =
{
   /* Generic Bluetooth USB device.                                     */
   { USB_DEVICE_INFO(BLUETOOTH_USB_CLASS_ID, BLUETOOTH_USB_SUBCLASS_ID, BLUETOOTH_USB_PROTOCOL_ID)},

   /* Atheros Devices.                                                  */
   { USB_DEVICE(0x045E, 0x02C8) },
   /* Qualcomm Devices.                                                  */
   { USB_DEVICE(0x0CF3, 0x3004) },

   /* AVM BlueFRITZ!  USB v2.0.                                         */
   { USB_DEVICE(0x057C, 0x3800) },

   /* CSR Module.                                                       */
   { USB_DEVICE(0x0A12, 0x0001) },
   { USB_DEVICE(0x0A12, 0x0043) },
   { USB_DEVICE(0x0A12, 0x1000) },

   /* Bluetooth Ultraport Module from IBM.                              */
   { USB_DEVICE(0x04BF, 0x030a) },

   /* ALPS Modules with non-standard ID.                                */
   { USB_DEVICE(0x044E, 0x3001) },
   { USB_DEVICE(0x044E, 0x3002) },

   /* Ericsson with non-standard ID.                                    */
   { USB_DEVICE(0x0BDB, 0x1000) },
   { USB_DEVICE(0x0BDB, 0x1001) },
   { USB_DEVICE(0x0BDB, 0x1002) },
   { USB_DEVICE(0x442B, 0xABBA) },
   { USB_DEVICE(0x08EA, 0xABBA) },
   { USB_DEVICE(0x08EA, 0xABBA) },

   /* National Semiconductor Devices.                                   */
   { USB_DEVICE(0x0400, 0x0807) },

   /* Silicon Wave Devices.                                             */
   { USB_DEVICE(0x0C10, 0x0000) },

   /* Philips Semiconductor Devices.                                    */
   { USB_DEVICE(0x0471, 0x0809) },

   /* Broadcom Devices.                                                 */
   { USB_DEVICE(0x0A5C, 0x2001) },
   { USB_DEVICE(0x0A5C, 0x200A) },
   { USB_DEVICE(0x0A5C, 0x2101) },
   { USB_DEVICE(0x0A5C, 0x2148) },
   { USB_DEVICE(0x0A5C, 0x22BE) },
   { USB_DEVICE(0x0A5C, 0x21E8) },
   { USB_DEVICE(0x0A5C, 0x2154) },
   { USB_DEVICE(0x0A5C, 0x2111) },
   { USB_DEVICE(0x0A5C, 0x21E6) },

   /* Belkin V2.1 Dongle (Broadcom chipset).                            */
   { USB_DEVICE(0x050D, 0x016A) },
   { USB_DEVICE(0x050D, 0x0131) },

   /* Terminating entry.                                                */
   {}
};

MODULE_DEVICE_TABLE(usb, BluetoothDeviceIDTable);

   /* The following structure holds all information that is necessary   */
   /* to buffer and continually check (and signal) any information read */
   /* from a USB Device.                                                */
typedef struct _tagUSBBufferInformation_t
{
   ListEntry_t                        ListEntry;
   unsigned int                       CircleBufferSize;
   unsigned int                       CircleBufferBytesAvailable;
   unsigned char                     *CircleBuffer;
   unsigned int                       CircleBufferReadIndex;
   unsigned int                       CircleBufferWriteIndex;
   unsigned int                       ReadBufferSize;
   unsigned char                     *ReadBuffer;
   struct urb                        *ReadURBHandle;
   unsigned char                     *ExtraReadBuffer[MAX_ISO_BUFFERS];
   struct urb                        *IsoExtraReadURBHandle[MAX_ISO_BUFFERS];
   wait_queue_head_t                  WriteWaitEvent;
   spinlock_t                         WriteInformationSpinLock;
   unsigned int                       WriteBufferSize;
   URBBufferInfo_t                    URBBufferInfo[MAX_WRITE_BUFFERS];
   ListEntry_t                        FreeURBBufferList;
   atomic_t                           OutstandingWrites;
   ListEntry_t                        OutstandingWritesList;
   struct _tagUSBDeviceInformation_t *USBDeviceInformation;
} USBBufferInformation_t;

   /* The following structure holds all Instance information associated */
   /* with a particular USB Device.                                     */
typedef struct _tagUSBDeviceInformation_t
{
   struct usb_endpoint_descriptor    *SCODataInDescriptor;
   struct usb_endpoint_descriptor    *SCODataOutDescriptor;
   struct usb_endpoint_descriptor    *HCIEventInDescriptor;
   struct usb_endpoint_descriptor    *ACLDataInDescriptor;
   struct usb_endpoint_descriptor    *ACLDataOutDescriptor;
   struct usb_device                 *USBDevice;
   struct usb_interface              *AsyncInterfacePtr;
   struct usb_ctrlrequest             USBCtrlRequest;
   struct usb_class_driver            ClassDriver;
   unsigned int                       SCOAlternateInterface;
   unsigned int                       LargestSCOMaxPacketSize;
   SCOAlternateInterfaceMapping_t     SCOAlternateInterfaceMapping[NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS];
   unsigned int                       MinorDeviceNumber;
   int                                NumInterfacesOwned;
   int                                Interface0Owned;
   int                                Interface1Owned;
   int                                OpenedByUser;
   int                                BeingRemoved;
   atomic_t                           HCIEventInStalled;
   atomic_t                           ACLDataInStalled;
   atomic_t                           ACLDataOutStalled;
   struct work_struct                 ClearStallWork;
   unsigned int                       SCO_Enabled;
   USBBufferInformation_t             ACLBufferInformation;
   USBBufferInformation_t             HCIBufferInformation;
   USBBufferInformation_t             SCOBufferInformation;
   spinlock_t                         ReadInformationSpinLock;
   wait_queue_head_t                  ReadWaitEvent;
   struct _tagUSBDeviceInformation_t *NextDeviceInformationPtr;
} USBDeviceInformation_t;

   /* The following structure holds all Context Information associated  */
   /* with this particular USB Device Driver Module.                    */
typedef struct _tagUSBContextInformation_t
{
   struct usb_driver       USBDriver;
   struct file_operations  FileOperations;
   USBDeviceInformation_t *DeviceInformationList;
} USBContextInformation_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the compiler*/
   /* as part of standard C/C++).                                       */
static USBContextInformation_t ContextInformation;

   /* Mutex that guards Access to the Context Information for this USB  */
   /* Resource Manager.                                                 */
static spinlock_t ContextInformationSpinLock;

   /* These function are the List routines that you will need to keep   */
   /* USB device information                                            */
static USBDeviceInformation_t *AddDeviceInformationEntry(USBDeviceInformation_t **ListHead, USBDeviceInformation_t *EntryToAdd);
static USBDeviceInformation_t *SearchDeviceInformationEntry(USBDeviceInformation_t **ListHead, int DevNum);
static USBDeviceInformation_t *SearchDeviceInformationEntryByMinorDev(USBDeviceInformation_t **ListHead, int MinorNum);
static USBDeviceInformation_t *DeleteDeviceInformationEntry(USBDeviceInformation_t **ListHead, int DevNum);
static void FreeDeviceWriteInformationEntry(USBBufferInformation_t *EntryToFree);
static void FreeDeviceInformationEntry(USBDeviceInformation_t *EntryToFree);
static void FreeDeviceInformationList(USBDeviceInformation_t **ListHead);

   /* Internal Helper Device Function Prototypes.                       */
static void InitializeList(ListEntry_t *ListHead);
static int IsItemInList(ListEntry_t *ListHead, ListEntry_t *ListItem);
static int AddListEntry(ListEntry_t *ListHead, ListEntry_t *EntryToAdd);
static int RemoveListEntry(ListEntry_t *EntryToRemove);
static ListEntry_t *RemoveFirstListEntry(ListEntry_t *List);

static void InitializeReadInformation(USBBufferInformation_t *BufferInformation, int EndpointType, int MaxPacketSize);
static int InitializeWriteInformation(USBBufferInformation_t *BufferInformation, int EndpointType, int MaxPacketSize);
static int InitializeInterface0(struct usb_host_interface *AltSetting, USBDeviceInformation_t *USBDeviceInformationPtr);
static int InitializeInterface1(USBDeviceInformation_t *USBDeviceInformationPtr);
static void CopyURBMemoryToLocalReadMemory(struct urb *URBPtr, USBBufferInformation_t *BufferInfo);
static void CloseDevice(USBDeviceInformation_t *USBDeviceInformationPtr);
static void StopHCIEventACL(USBDeviceInformation_t *USBDeviceInformationPtr);
static void StartHCIEventACL(USBDeviceInformation_t *USBDeviceInformationPtr, int DevNum);

   /* Internal Device Function Prototypes that will be called from      */
   /* external modules.                                                 */
static void usb_fill_write_isoc_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int frame_size, int endpoint_size, usb_complete_t complete, void *context, int interval);
static void usb_fill_isoc_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int frame_size, int endpoint_size, usb_complete_t complete, void *context, int interval);
static void refill_isoc_urb(struct urb *urb, struct usb_device *dev, int frame_size, int endpoint_size);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))

   static void USBReadCallback(struct urb *URBPtr, struct pt_regs *Regs);
   static void USBWriteCallback(struct urb *URBPtr, struct pt_regs *Regs);

#else

   static void USBReadCallback(struct urb *URBPtr);
   static void USBWriteCallback(struct urb *URBPtr);

#endif

static void ClearStall(struct work_struct *Work);

static int USBProbe(struct usb_interface *InterfacePtr, const struct usb_device_id *DeviceID);
static void USBDisconnect(struct usb_interface *InterfacePtr);
static int USBOpen(struct inode *INode, struct file *FilePointer);
static int USBClose(struct inode *INode, struct file *FilePointer);

static long USBIoctlImpl(struct file *FilePointer, unsigned int Command, unsigned long Parameter, uint8_t Compat);

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
   static int USBIoctl(struct inode *INode, struct file *FilePointer, unsigned int Command, unsigned long Parameter);
   static int USBIoctlCompat(struct inode *INode, struct file *FilePointer, unsigned int Command, unsigned long Parameter);
#else
   static long USBIoctl(struct file *FilePointer, unsigned int Command, unsigned long Parameter);
   static long USBIoctlCompat(struct file *FilePointer, unsigned int Command, unsigned long Parameter);
#endif

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function adds the specified entry to the list.  This  */
   /* function will return NULL if NO Entry was added.  This can occur  */
   /* if the element passed in was deemed invalid or the actual List    */
   /* Head was invalid.                                                 */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Device Number field is the same as an entry already    */
   /*            in the list.  When this occurs, this function returns  */
   /*            NULL.                                                  */
static USBDeviceInformation_t *AddDeviceInformationEntry(USBDeviceInformation_t **ListHead, USBDeviceInformation_t *EntryToAdd)
{
   USBDeviceInformation_t *tmpEntry;

   /* First let's verify that values passed in are semi-valid.          */
   if((ListHead) && (EntryToAdd))
   {
      /* OK, data seems semi-valid, let's add it to the list.           */

      /* Now Add it to the end of the list.                             */
      EntryToAdd->NextDeviceInformationPtr = NULL;

      /* First, let's check to see if there are any elements already    */
      /* present in the List that was passed in.                        */
      if((tmpEntry = *ListHead) != NULL)
      {
         /* Head Pointer was not NULL, so we will traverse the list     */
         /* until we reach the last element.                            */
         while(tmpEntry)
         {
            if(tmpEntry->USBDevice->devnum == EntryToAdd->USBDevice->devnum)
            {
               /* Entry was already added, flag an error to the caller. */
               EntryToAdd = NULL;

               /* Abort the Search.                                     */
               tmpEntry   = NULL;
            }
            else
            {
               /* OK, we need to see if we are at the last element of   */
               /* the List.  If we are, we simply break out of the list */
               /* traversal because we know there are NO duplicates AND */
               /* we are at the end of the list.                        */
               if(tmpEntry->NextDeviceInformationPtr)
                  tmpEntry = tmpEntry->NextDeviceInformationPtr;
               else
                  break;
            }
         }

         if(EntryToAdd)
         {
            /* Last element found, simply Add the entry.                */
            tmpEntry->NextDeviceInformationPtr = EntryToAdd;
         }
      }
      else
         *ListHead = EntryToAdd;
   }
   else
      EntryToAdd = NULL;

   return(EntryToAdd);
}

   /* The following function searches the specified List for the        */
   /* specified Device Number.  This function returns NULL if either the*/
   /* List Head is invalid, the Device Number is invalid, or the        */
   /* specified Device Number was NOT found.                            */
static USBDeviceInformation_t *SearchDeviceInformationEntry(USBDeviceInformation_t **ListHead, int DevNum)
{
   USBDeviceInformation_t *FoundEntry = NULL;

   /* Let's make sure the list and Minor Device Number to search for    */
   /* appear to be valid.                                               */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->USBDevice->devnum != DevNum))
         FoundEntry = FoundEntry->NextDeviceInformationPtr;
   }

   return(FoundEntry);
}

   /* The following function searches the specified List for the        */
   /* specified Device Number.  This function returns NULL if either the*/
   /* List Head is invalid, the Device Number is invalid, or the        */
   /* specified Device Number was NOT found.                            */
static USBDeviceInformation_t *SearchDeviceInformationEntryByMinorDev(USBDeviceInformation_t **ListHead, int MinorNum)
{
   USBDeviceInformation_t *FoundEntry = NULL;

   /* Let's make sure the list and Minor Device Number to search for    */
   /* appear to be valid.                                               */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->MinorDeviceNumber != MinorNum))
         FoundEntry = FoundEntry->NextDeviceInformationPtr;
   }

   return(FoundEntry);
}

   /* The following function searches the specified Driver List for the */
   /* specified Device Number and removes it from the List.  This       */
   /* function returns NULL if either the Driver List Head is invalid,  */
   /* the Device Number is invalid, or the specified Driver was NOT     */
   /* present in the list.  The entry returned will have the Next Entry */
   /* field set to NULL, and the caller is responsible for deleting the */
   /* memory associated with this entry by calling usb_detach().        */
static USBDeviceInformation_t *DeleteDeviceInformationEntry(USBDeviceInformation_t **ListHead, int DevNum)
{
   USBDeviceInformation_t *FoundEntry = NULL;
   USBDeviceInformation_t *LastEntry  = NULL;

   /* Let's make sure the List and Driver ID to search for appear to be */
   /* semi-valid.                                                       */
   if(ListHead)
   {
      /* Now, let's search the list until we find the correct entry.    */
      FoundEntry = *ListHead;

      while((FoundEntry) && (FoundEntry->USBDevice->devnum != DevNum))
      {
         LastEntry  = FoundEntry;
         FoundEntry = FoundEntry->NextDeviceInformationPtr;
      }

      /* Check to see if we found the specified entry.                  */
      if(FoundEntry)
      {
         /* OK, now let's remove the entry from the list.  We have to   */
         /* check to see if the entry was the first entry in the list.  */
         if(LastEntry)
         {
            /* Entry was NOT the first entry in the list.               */
            LastEntry->NextDeviceInformationPtr = FoundEntry->NextDeviceInformationPtr;
         }
         else
            *ListHead = FoundEntry->NextDeviceInformationPtr;

         FoundEntry->NextDeviceInformationPtr = NULL;
      }
   }

   return(FoundEntry);
}

   /* This function frees all the write information structures and      */
   /* memory.                                                           */
static void FreeDeviceWriteInformationEntry(USBBufferInformation_t *EntryToFree)
{
   int Index;

   for(Index=0;Index<MAX_WRITE_BUFFERS;Index++)
   {
      if(EntryToFree->URBBufferInfo[Index].URBBuffer)
      {
         kfree(EntryToFree->URBBufferInfo[Index].URBBuffer);

         EntryToFree->URBBufferInfo[Index].URBBuffer = NULL;
      }

      if(EntryToFree->URBBufferInfo[Index].URB)
      {
         usb_free_urb(EntryToFree->URBBufferInfo[Index].URB);

         EntryToFree->URBBufferInfo[Index].URB = NULL;
      }
   }
}

   /* This function frees the specified Device Information Entry member.*/
static void FreeDeviceInformationEntry(USBDeviceInformation_t *EntryToFree)
{
   int Index;

   if(EntryToFree)
   {
      /* Free ACL information.                                          */
      if(EntryToFree->ACLBufferInformation.CircleBuffer)
      {
         kfree(EntryToFree->ACLBufferInformation.CircleBuffer);

         EntryToFree->ACLBufferInformation.CircleBuffer = NULL;
      }

      if(EntryToFree->ACLBufferInformation.ReadBuffer)
      {
         kfree(EntryToFree->ACLBufferInformation.ReadBuffer);

         EntryToFree->ACLBufferInformation.ReadBuffer = NULL;
      }

      if(EntryToFree->ACLBufferInformation.ReadURBHandle)
      {
         usb_free_urb(EntryToFree->ACLBufferInformation.ReadURBHandle);

         EntryToFree->ACLBufferInformation.ReadURBHandle = NULL;
      }

      /* Free the ACL Write buffer information.                         */
      FreeDeviceWriteInformationEntry(&(EntryToFree->ACLBufferInformation));

      /* Now handle HCI information.                                    */
      if(EntryToFree->HCIBufferInformation.CircleBuffer)
      {
         kfree(EntryToFree->HCIBufferInformation.CircleBuffer);

         EntryToFree->HCIBufferInformation.CircleBuffer = NULL;
      }

      if(EntryToFree->HCIBufferInformation.ReadBuffer)
      {
         kfree(EntryToFree->HCIBufferInformation.ReadBuffer);

         EntryToFree->HCIBufferInformation.ReadBuffer = NULL;
      }

      if(EntryToFree->HCIBufferInformation.ReadURBHandle)
      {
         usb_free_urb(EntryToFree->HCIBufferInformation.ReadURBHandle);

         EntryToFree->HCIBufferInformation.ReadURBHandle = NULL;
      }

      /* Free the HCI Write buffer information.                         */
      FreeDeviceWriteInformationEntry(&(EntryToFree->HCIBufferInformation));

      /* Now handle SCO buffer information.                             */
      if(EntryToFree->SCOBufferInformation.CircleBuffer)
      {
         kfree(EntryToFree->SCOBufferInformation.CircleBuffer);

         EntryToFree->SCOBufferInformation.CircleBuffer = NULL;
      }

      if(EntryToFree->SCOBufferInformation.ReadBuffer)
      {
         kfree(EntryToFree->SCOBufferInformation.ReadBuffer);

         EntryToFree->SCOBufferInformation.ReadBuffer = NULL;
      }

      if(EntryToFree->SCOBufferInformation.ReadURBHandle)
      {
         usb_free_urb(EntryToFree->SCOBufferInformation.ReadURBHandle);

         EntryToFree->SCOBufferInformation.ReadURBHandle = NULL;
      }

      for(Index=0;Index<MAX_ISO_BUFFERS;Index++)
      {
         if(EntryToFree->SCOBufferInformation.IsoExtraReadURBHandle[Index])
         {
            usb_free_urb(EntryToFree->SCOBufferInformation.IsoExtraReadURBHandle[Index]);

            EntryToFree->SCOBufferInformation.IsoExtraReadURBHandle[Index] = NULL;
         }

         if(EntryToFree->SCOBufferInformation.ExtraReadBuffer[Index])
         {
            kfree(EntryToFree->SCOBufferInformation.ExtraReadBuffer[Index]);

            EntryToFree->SCOBufferInformation.ExtraReadBuffer[Index] = NULL;
         }
      }

      /* Free the SCO Write buffer information.                         */
      FreeDeviceWriteInformationEntry(&(EntryToFree->SCOBufferInformation));

      kfree(EntryToFree);
   }
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Device Information List.  Upon return of */
   /* this function, the Head Pointer is set to NULL.                   */
static void FreeDeviceInformationList(USBDeviceInformation_t **ListHead)
{
   USBDeviceInformation_t *EntryToFree;
   USBDeviceInformation_t *tmpEntry;

   if(ListHead)
   {
      /* Simply traverse the list and free every element present.       */
      EntryToFree = *ListHead;

      while(EntryToFree)
      {
         tmpEntry    = EntryToFree;
         EntryToFree = EntryToFree->NextDeviceInformationPtr;

         if(tmpEntry)
         {
            FreeDeviceInformationEntry(tmpEntry);
            tmpEntry = NULL;
         }
      }

      /* Make sure the List appears to be empty.                        */
      *ListHead = NULL;
   }
}

   /* The following function initialized a List Entry structure by      */
   /* setting the Previous and Next Pointers to point to itself.  This  */
   /* indicates an empty List.                                          */
static void InitializeList(ListEntry_t *ListHead)
{
   if(ListHead)
   {
      ListHead->NextEntry = ListHead;
      ListHead->PrevEntry = ListHead;
   }
}

   /* The following function is used to check to see if the specified   */
   /* item entry is present in the specified List.  The function takes  */
   /* as it's parameters the List Head of the List to search, and the   */
   /* List Item entry pointer to search for.  This function returns a   */
   /* non-zero value if the item was found in the list, or zero if the  */
   /* item was not present in the list.                                 */
static int IsItemInList(ListEntry_t *ListHead, ListEntry_t *ListItem)
{
   int          ret_val = 0;
   ListEntry_t *ListEntry;

   if((ListEntry = ListHead) != NULL)
   {
      ListEntry = ListEntry->NextEntry;
      while((ListEntry != ListHead) && (!ret_val))
      {
         if(ListEntry == ListItem)
            ret_val = 1;
         else
            ListEntry = ListEntry->NextEntry;
      }
   }

   return(ret_val);
}

   /* The following function adds the generic Entry to the end of a     */
   /* specified List.                                                   */
   /* * NOTE * This function requires the pointer to the next list entry*/
   /*          be the first element in the structure.                   */
static int AddListEntry(ListEntry_t *ListHead, ListEntry_t *EntryToAdd)
{
   int ret_val = -1;

   /* Verify that the parameters passed in appear valid.                */
   if((ListHead) && (EntryToAdd))
   {
      PREV_ENTRY(EntryToAdd)           = PREV_ENTRY(ListHead);
      NEXT_ENTRY(EntryToAdd)           = ListHead;
      NEXT_ENTRY(PREV_ENTRY(ListHead)) = EntryToAdd;
      PREV_ENTRY(ListHead)             = EntryToAdd;

      ret_val                          = 0;
   }

   return(ret_val);
}

   /* The following function removes the generic Entry from a specified */
   /* List.                                                             */
   /* * NOTE * This function requires the pointer to the next list entry*/
   /*          be the first element in the structure.                   */
static int RemoveListEntry(ListEntry_t *EntryToRemove)
{
   int ret_val = -1;

   /* Verify that the parameters passed in appear valid.                */
   if(EntryToRemove)
   {
      NEXT_ENTRY(PREV_ENTRY(EntryToRemove)) = NEXT_ENTRY(EntryToRemove);
      PREV_ENTRY(NEXT_ENTRY(EntryToRemove)) = PREV_ENTRY(EntryToRemove);
      PREV_ENTRY(EntryToRemove)             = EntryToRemove;
      NEXT_ENTRY(EntryToRemove)             = EntryToRemove;
   }

   return(ret_val);
}

   /* The following function removes and return the first Entry from a  */
   /* specified List.  If the List is empty, the function return NULL.  */
static ListEntry_t *RemoveFirstListEntry(ListEntry_t *List)
{
   ListEntry_t *ret_val = NULL;;

   /* Verify that the parameters passed in appear valid.                */
   if((List) && (NEXT_ENTRY(List) != List))
   {
      ret_val = List->NextEntry;

      RemoveListEntry(ret_val);
   }

   return(ret_val);
}

   /* The following function is used to initialize the frame descriptor */
   /* field of an ISOC URB used to write data to device.  The function  */
   /* The function takes as its first parameter a pointer to the URB    */
   /* that is to be initialized.  The second parameter is usb_device    */
   /* structure that defines the device.  The third parameter is the    */
   /* 'pipe' associated with this endpoint address.  The fourth         */
   /* parameter is a pointer to the buffer holding the ISO data.        */
   /* The fifth parameter specifies the size of this buffer.  The sixth */
   /* parameter specifies the size of frame associated with this usb    */
   /* endpoint.  The seventh parameter is the address of the completion */
   /* handler called by the USB core when it finishes processing the    */
   /* urb.  The eighth parameter is the context variable the usb core   */
   /* will return to the completion handler.  The final parameter is the*/
   /* interval value associated with this USB endpoint.                 */
static void usb_fill_write_isoc_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int frame_size, int endpoint_size, usb_complete_t complete, void *context, int interval)
{
   int                               offset;
   int                               number_of_packets;
   int                               iso_packet_length = endpoint_size;
   struct usb_iso_packet_descriptor *desc;

   urb->dev                    = dev;
   urb->pipe                   = pipe;
   urb->transfer_buffer        = transfer_buffer;
   urb->transfer_buffer_length = frame_size;
   urb->number_of_packets      = frame_size / endpoint_size;

   /* Adjust if we have an uneven amount of data.                       */
   if(frame_size % endpoint_size)
      urb->number_of_packets++;

   urb->transfer_flags         = URB_ISO_ASAP;
   urb->interval               = interval;
   urb->complete               = complete;
   urb->context                = context;

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
   spin_lock_init(&urb->lock);
#endif

   /* Initialize the offset and set a pointer to the first descriptor.  */
   offset                      = 0;
   desc                        = urb->iso_frame_desc;

   number_of_packets           = urb->number_of_packets;

   /* While there is data remaining, fill in the descriptor information.*/
   while(number_of_packets > 0)
   {
      desc->offset         = offset;
      desc->length         = iso_packet_length;
      desc->actual_length  = iso_packet_length;
      desc->status         = 0;

      DEBUGPRINT(KERN_ERR "%s: offset %d length %d number_of_packets %d\n", __FUNCTION__, desc->offset, desc->length, number_of_packets);

      offset              += iso_packet_length;

      number_of_packets--;

      desc++;
   }
}

   /* The following function is used to initialize the frame descriptor */
   /* fields of an ISOC URB.  The function takes as its first parameter */
   /* a pointer to the URB that is to be initialized.  The second       */
   /* parameter is usb_device structure that defines the device.  The   */
   /* third parameter is the 'pipe' associated with this endpoint       */
   /* address.  The fourth parameter is a pointer to the buffer created */
   /* to hold the ISO data. The fifth parameter specifies the size of   */
   /* this buffer.  The sixth parameter is the address of the completion*/
   /* handler called by the USB core when it finishes processing the    */
   /* urb.  The seventh parameter is the context variable the usb core  */
   /* will return to the completion handler.  The final parameter is the*/
   /* interval value associated with this USB endpoint.                 */
   /* This function is used when setting up urbs for receiving ISO data */
static void usb_fill_isoc_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int frame_size, int endpoint_size, usb_complete_t complete, void *context, int interval)
{
   int                               offset;
   int                               number_of_packets;
   int                               iso_packet_length = endpoint_size;
   struct usb_iso_packet_descriptor *desc;

   urb->dev                    = dev;
   urb->pipe                   = pipe;
   urb->transfer_buffer        = transfer_buffer;
   urb->transfer_buffer_length = frame_size;
   urb->transfer_flags         = URB_ISO_ASAP;
   urb->interval               = interval;
   urb->complete               = complete;
   urb->context                = context;

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
   spin_lock_init(&urb->lock);
#endif

   /* Initialize the offset and set a pointer to the first descriptor.  */
   offset                      = 0;
   desc                        = urb->iso_frame_desc;
   urb->number_of_packets      = MAX_ISO_PACKETS;
   number_of_packets           = MAX_ISO_PACKETS;

   /* While there is data remaining, fill in the descriptor information.*/
   while(number_of_packets > 0)
   {
      desc->offset         = offset;
      desc->length         = iso_packet_length;
      desc->actual_length  = 0;
      desc->status         = 0;

      offset              += iso_packet_length;

      number_of_packets--;

      desc++;
   }
}

   /* The following function is used to re-initialize any values in the */
   /* field of an ISOC URB. The function takes as its first parameter a */
   /* pointer to the URB being used. The design assumption is that all  */
   /* fields stay the same for isochronous URB's, but status and offset */
   /* values should be reset, just in case the USB core monkeyed with   */
   /* them.                                                             */
static void refill_isoc_urb(struct urb *urb, struct usb_device *dev, int frame_size, int endpoint_size)
{
   int                               offset;
   int                               number_of_packets;
   int                               iso_packet_length = endpoint_size;
   struct usb_iso_packet_descriptor *desc;

   urb->dev               = dev;
   urb->status            = 0;
   urb->transfer_flags    = URB_ISO_ASAP;

   /* Initialize the offset and set a pointer to the first descriptor.  */
   offset                 = 0;
   desc                   = urb->iso_frame_desc;
   urb->number_of_packets = MAX_ISO_PACKETS;
   number_of_packets      = MAX_ISO_PACKETS;

   /* While there is data remaining, fill in the descriptor information.*/
   while(number_of_packets > 0)
   {
      desc->offset         = offset;
      desc->length         = iso_packet_length;
      desc->actual_length  = 0;
      desc->status         = 0;
      offset              += iso_packet_length;

      number_of_packets--;

      desc++;
   }
}

   /* The following function is responsible for allocation all resources*/
   /* associated with the Read Information structure. The first         */
   /* parameter is a pointer to the ReadInformation structure that      */
   /* is already initalized. The second parameter specifies the actual  */
   /* Endpoint type.  The Final parameter specifies the Maximum Packet  */
   /* Size supported by the endpoint itself.  This function allocates   */
   /* the required buffer sizes (constants defined above) based on the  */
   /* endpoint type.                                                    */
static void InitializeReadInformation(USBBufferInformation_t *BufferInformation, int EndpointType, int MaxPacketSize)
{
   int BufferSize;
   int Index;
   int NumberISOPackets;
   int ReadBufferSize;

   /* Make sure it looks like the parameters are valid.                 */
   if(BufferInformation)
   {
      switch(EndpointType)
      {
         case USB_ENDPOINT_XFER_INT:
            BufferSize       = NON_ISOC_RX_BUFFER_SIZE;
            ReadBufferSize   = MaxPacketSize;
            NumberISOPackets = 0;
            break;
         case USB_ENDPOINT_XFER_BULK:
            BufferSize       = NON_ISOC_RX_BUFFER_SIZE;
            ReadBufferSize   = BULK_READ_SIZE;
            NumberISOPackets = 0;
            break;
         case USB_ENDPOINT_XFER_ISOC:
            BufferSize       = ISOC_RX_BUFFER_SIZE;
            ReadBufferSize   = (MAX_ISO_PACKETS * MaxPacketSize);
            NumberISOPackets = MAX_ISO_PACKETS;
            break;
         default:
            BufferSize       = 0;
            ReadBufferSize   = 0;
            NumberISOPackets = 0;
            break;
      }

      if(BufferSize)
      {
         BufferInformation->ReadURBHandle  = usb_alloc_urb(NumberISOPackets, GFP_KERNEL);
         BufferInformation->ReadBufferSize = ReadBufferSize;
         BufferInformation->ReadBuffer     = kmalloc(BufferInformation->ReadBufferSize, GFP_KERNEL);

         DEBUGPRINT(KERN_ERR "%s : ReadURBHandle %p, ReadBuffer %p, ReadBufferSize %d\n", __FUNCTION__, BufferInformation->ReadURBHandle, BufferInformation->ReadBuffer, BufferInformation->ReadBufferSize);

         if(NumberISOPackets)
         {
            for(Index=0;Index<MAX_ISO_BUFFERS;Index++)
            {
               /* Allocate multiple URB's to insure we can keep up with */
               /* the data stream.                                      */
               BufferInformation->IsoExtraReadURBHandle[Index] = usb_alloc_urb(NumberISOPackets, GFP_KERNEL);
               BufferInformation->ExtraReadBuffer[Index]       = kmalloc(BufferInformation->ReadBufferSize, GFP_KERNEL);

               DEBUGPRINT(KERN_ERR "%s : IsoExtraReadURBHandle[%d] %p, ExtraReadBuffer[%d] %p, \n", __FUNCTION__, Index, BufferInformation->IsoExtraReadURBHandle[Index], Index, BufferInformation->ExtraReadBuffer[Index]);
            }
         }

         if((BufferInformation->CircleBuffer = kmalloc(BufferSize, GFP_KERNEL)) != NULL)
            BufferInformation->CircleBufferSize = BufferSize;
         else
            BufferInformation->CircleBufferSize = 0;
      }
      else
      {
         BufferInformation->ReadURBHandle  = NULL;
         BufferInformation->ReadBufferSize = 0;
         BufferInformation->ReadBuffer     = NULL;
      }

      BufferInformation->CircleBufferBytesAvailable = 0;
      BufferInformation->CircleBufferReadIndex      = 0;
      BufferInformation->CircleBufferWriteIndex     = 0;
   }
}

   /* The following function is responsible for allocation all resources*/
   /* associated with the Write Information structure. The first        */
   /* parameter is a pointer to the ReadInformation structure that      */
   /* is already initalized. The second parameter specifies the actual  */
   /* endpoint type.  The final parameter specifies the Maximum Packet  */
   /* Size supported by the Endpoint.                                   */
   /* Returns True on success, or false if some problem is encountered. */
static int InitializeWriteInformation(USBBufferInformation_t *BufferInformation, int EndpointType, int MaxPacketSize)
{
   int ret_val = 1;
   int UrbCount;
   int WriteBufferSize;
   int NumberISOPackets;

   /* Make sure it looks like the parameters are valid.                 */
   if(BufferInformation)
   {
      /* Initialize the URB/Buffer List to be empty.                    */
      InitializeList(&(BufferInformation->FreeURBBufferList));

      /* Initialize the list of outstanding write URBs.                 */
      InitializeList(&(BufferInformation->OutstandingWritesList));

      /* Initialize the count of outstanding writes.                    */
      atomic_set(&(BufferInformation->OutstandingWrites), 0);

      switch(EndpointType)
      {
         case USB_ENDPOINT_XFER_BULK:
            WriteBufferSize  = BULK_WRITE_SIZE;
            NumberISOPackets = 0;
            break;
         case USB_ENDPOINT_XFER_ISOC:
            WriteBufferSize  = (MAX_ISO_PACKETS * MaxPacketSize);
            NumberISOPackets = MAX_ISO_PACKETS;
            break;
         case USB_ENDPOINT_XFER_CONTROL:
            WriteBufferSize  = CONTROL_WRITE_SIZE;
            NumberISOPackets = 0;
            break;
         default:
            WriteBufferSize  = 0;
            NumberISOPackets = 0;
            break;
      }

      if(WriteBufferSize)
      {
         BufferInformation->WriteBufferSize = WriteBufferSize;

         /* Obtain a Write Wait Queue for write's.                      */
         init_waitqueue_head(&(BufferInformation->WriteWaitEvent));

         DEBUGPRINT(KERN_ERR "%s : WriteWaitEvent Created\n", __FUNCTION__);

         spin_lock_init(&(BufferInformation->WriteInformationSpinLock));

         DEBUGPRINT(KERN_ERR "%s : WriteInformation SpinLock Created\n", __FUNCTION__);

         for(UrbCount=0;UrbCount<MAX_WRITE_BUFFERS;UrbCount++)
         {
            InitializeList(&(BufferInformation->URBBufferInfo[UrbCount].ListEntry));

            BufferInformation->URBBufferInfo[UrbCount].URB       = NULL;
            BufferInformation->URBBufferInfo[UrbCount].URBBuffer = NULL;

            /* First allocate urbs and buffers, checking as we go.      */
            BufferInformation->URBBufferInfo[UrbCount].URB = usb_alloc_urb(NumberISOPackets, GFP_KERNEL);
            if(BufferInformation->URBBufferInfo[UrbCount].URB)
            {
               BufferInformation->URBBufferInfo[UrbCount].URBBuffer = kmalloc(BufferInformation->WriteBufferSize , GFP_KERNEL);
               if(BufferInformation->URBBufferInfo[UrbCount].URBBuffer)
               {
                  /* Everything is allocated, now update the bookeeping */
                  /* fields.                                            */

                  /* Note the Buffer Information that this URB Buffer is*/
                  /* associated with.                                   */
                  BufferInformation->URBBufferInfo[UrbCount].USBBufferInformation = BufferInformation;

                  /* Add the entry to the Free URB/Buffer List.         */
                  AddListEntry(&(BufferInformation->FreeURBBufferList), &(BufferInformation->URBBufferInfo[UrbCount].ListEntry));

                  DEBUGPRINT(KERN_ERR "%s : WriteURB[%d] %p, WriteBuffer[%d] %p\n", __FUNCTION__, UrbCount, BufferInformation->URBBufferInfo[UrbCount].URB, UrbCount, BufferInformation->URBBufferInfo[UrbCount].URBBuffer);
               }
               else
               {
                  /* Failed to get a buffer, break the loop and clean up*/
                  /* the mess.                                          */
                  break;
               }
            }
            else
            {
               /* Failed to get a buffer, break the loop and clean up   */
               /* the mess.                                             */
               break;
            }
         }

         /* If UrbCount didn't reach max, we have partial allocation We */
         /* could try to go ahead and work if we have at least a couple */
         /* of buffers, or we can just take the KISS way out, and give  */
         /* up.  Which is what this code does.                          */
         if(UrbCount < MAX_WRITE_BUFFERS)
         {
            ret_val = 0;

            /* Clean up buffers and Urbs.                               */
            while(UrbCount >= 0)
            {
               if(BufferInformation->URBBufferInfo[UrbCount].URB)
                  usb_free_urb(BufferInformation->URBBufferInfo[UrbCount].URB);

               if(BufferInformation->URBBufferInfo[UrbCount].URBBuffer)
                  kfree(BufferInformation->URBBufferInfo[UrbCount].URBBuffer);

               BufferInformation->URBBufferInfo[UrbCount].URB       = NULL;
               BufferInformation->URBBufferInfo[UrbCount].URBBuffer = NULL;

               InitializeList(&(BufferInformation->URBBufferInfo[UrbCount].ListEntry));

               UrbCount--;
            }

            /* Initialize the list to be empty.                         */
            InitializeList(&(BufferInformation->FreeURBBufferList));
         }
      }
      else
         ret_val = 0;
   }
   else
      ret_val = 0;

   return(ret_val);
}

   /* The following function is responsible for allocating all resources*/
   /* associated with interface zero.  This function will search all    */
   /* configurations until a matching device configuration is found.    */
   /* Once the device is found to match the required device, then the   */
   /* interface is selected and all resources are allocated.  This      */
   /* function returns TRUE upon successfully or a FALSE if not.        */
static int InitializeInterface0(struct usb_host_interface *AltSetting, USBDeviceInformation_t *USBDeviceInformationPtr)
{
   int                             i;
   int                             ret_val = 0;
   struct usb_endpoint_descriptor *endpoint;

   /* Check to make sure we have something valid.                       */
   if((AltSetting) && (USBDeviceInformationPtr))
   {
      /* Make sure this interface has the correct number of Endpoints   */
      /* before preceding.                                              */
      if(AltSetting->desc.bNumEndpoints == NUMBER_ENDPOINTS_INTERFACE_0)
      {
         /* Loop through all the endpoint and save the descriptions that*/
         /* we need.                                                    */
         for(i=0;i<AltSetting->desc.bNumEndpoints;i++)
         {
            endpoint = &AltSetting->endpoint[i].desc;

            /* Switch on the description endpoint attribute.            */
            switch(endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
            {
               case USB_ENDPOINT_XFER_INT:
                  /* This is a HCI Transfer Interupt endpoint.          */
                  if(endpoint->bEndpointAddress & USB_DIR_IN)
                  {
                     DEBUGPRINT(KERN_ERR "%s : %d byte Intr\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));

                     USBDeviceInformationPtr->HCIEventInDescriptor = endpoint;

                     /* Initialize and allocate all information that    */
                     /* belongs to the Buffer Information structure.    */
                     InitializeReadInformation(&USBDeviceInformationPtr->HCIBufferInformation, USB_ENDPOINT_XFER_INT, le16_to_cpu(endpoint->wMaxPacketSize));
                  }
                  break;
               case USB_ENDPOINT_XFER_BULK:
                  /* This is a ACL Transfer Bulk endpoint.              */
                  if(endpoint->bEndpointAddress & USB_DIR_IN)
                  {
                     DEBUGPRINT(KERN_ERR "%s : %d byte Bulk In\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));

                     USBDeviceInformationPtr->ACLDataInDescriptor = endpoint;

                     /* Initialize and malloc all information that      */
                     /* belongs to the Read Information structure.      */
                     InitializeReadInformation(&USBDeviceInformationPtr->ACLBufferInformation, USB_ENDPOINT_XFER_BULK, le16_to_cpu(endpoint->wMaxPacketSize));
                  }
                  else
                  {
                     DEBUGPRINT(KERN_ERR "%s : %d byte Bulk Out\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));

                     USBDeviceInformationPtr->ACLDataOutDescriptor = &AltSetting->endpoint[i].desc;
                     if(!InitializeWriteInformation(&USBDeviceInformationPtr->ACLBufferInformation, USB_ENDPOINT_XFER_BULK, le16_to_cpu(endpoint->wMaxPacketSize)))
                     {
                        /* Had a problem initializing write data        */
                        /* structures, don't claim success.             */
                        USBDeviceInformationPtr->ACLDataOutDescriptor = NULL;

                        DEBUGPRINT(KERN_ERR "%s : InitializeWriteInformation Failed\n", __FUNCTION__);
                     }
                  }
                  break;
               case USB_ENDPOINT_XFER_CONTROL:
                  DEBUGPRINT(KERN_ERR "%s : %d byte Xfer Control\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));
                  break;
               default:
                  DEBUGPRINT(KERN_ERR "%s : %d byte In default case\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));
                  break;
            }
         }
      }
   }

   /* Double Check that we have all the correct endpoints.              */
   if((USBDeviceInformationPtr->ACLDataInDescriptor) && (USBDeviceInformationPtr->ACLDataOutDescriptor) && (USBDeviceInformationPtr->HCIEventInDescriptor))
      ret_val = 1;

   return(ret_val);
}

   /* The following function is responsible for allocating all resources*/
   /* associated with interface one.  This function will search all     */
   /* configurations until a matching device configuration is found.    */
   /* Once the device is found to match the required device, then the   */
   /* interface is selected and all resources are allocated.  This      */
   /* function returns TRUE upon successfully or a FALSE if not.        */
static int InitializeInterface1(USBDeviceInformation_t *USBDeviceInformationPtr)
{
   int                             i;
   int                             ret_val = 0;
   unsigned int                    Index;
   struct usb_host_interface      *AltSetting;
   struct usb_endpoint_descriptor *endpoint;

   /* Check to make sure we have something valid.                       */
   if(USBDeviceInformationPtr)
   {
      for(Index=0;Index<NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS;Index++)
      {
         if((USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].AlternateSetting) && ((AltSetting = USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].HostInterface) != NULL))
         {
            /* Make sure this interface has the correct number of       */
            /* Endpoints before preceding.                              */
            if(AltSetting->desc.bNumEndpoints == NUMBER_ENDPOINTS_INTERFACE_1)
            {
               /* Loop through all the endpoint and save the            */
               /* descriptions that we need.                            */
               for(i=0;i<AltSetting->desc.bNumEndpoints;i++)
               {
                  endpoint = &(AltSetting->endpoint[i].desc);

                  /* Switch on the description endpoint attribute.      */
                  switch(endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
                  {
                     case USB_ENDPOINT_XFER_ISOC:
                        /* This is a ISOCH (SCO) Transfer Interrupt     */
                        /* endpoint.                                    */
                        if(endpoint->bEndpointAddress & USB_DIR_IN)
                        {
                           DEBUGPRINT(KERN_ERR "%s : %d byte SCO In\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));

                           USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].InEndpointDescriptor = endpoint;

                           USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].MaxPacketSize        = le16_to_cpu(endpoint->wMaxPacketSize);
                        }
                        else
                        {
                           DEBUGPRINT(KERN_ERR "%s : %d byte SCO Out\n", __FUNCTION__, le16_to_cpu(endpoint->wMaxPacketSize));

                           USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].OutEndpointDescriptor = endpoint;
                        }
                        break;
                     default:
                        break;
                  }
               }
            }
            else
            {
               /* Error, flag that this Endpoint is not supported.      */
               USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].AlternateSetting = 0;
               USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].HostInterface    = NULL;
            }
         }
      }

      /* Loop through and verify that there exists at least a single    */
      /* valid mapping (two endpoints mapped).                          */
      for(Index=0,ret_val=0;Index<NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS;Index++)
      {
         if((USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].AlternateSetting) && ((AltSetting = USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].HostInterface) != NULL))
         {
            if((USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].InEndpointDescriptor) && (USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].OutEndpointDescriptor))
               ret_val = 1;
            else
            {
               /* Invalid Alternate.                                    */
               USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].AlternateSetting      = 0;
               USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].HostInterface         = NULL;
               USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].InEndpointDescriptor  = NULL;
               USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].OutEndpointDescriptor = NULL;
            }
         }
      }

      if(ret_val)
      {
         /* Initialize and allocate all information that belongs to the */
         /* Read Information structure.                                 */
         InitializeReadInformation(&USBDeviceInformationPtr->SCOBufferInformation, USB_ENDPOINT_XFER_ISOC, USBDeviceInformationPtr->LargestSCOMaxPacketSize);

         /* Initialize and allocate all information that belongs to the */
         /* Read Information structure.                                 */
         if(!InitializeWriteInformation(&USBDeviceInformationPtr->SCOBufferInformation, USB_ENDPOINT_XFER_ISOC, USBDeviceInformationPtr->LargestSCOMaxPacketSize))
         {
            /* Had a problem initializing write data structures, don't  */
            /* claim success.                                           */
            DEBUGPRINT(KERN_ERR "%s : InitializeWriteInformation Failed\n", __FUNCTION__);

            ret_val = 0;
         }
      }
   }

   return(ret_val);
}

   /* This function copy's the URB memory to the circlur buffers in     */
   /* the ReadInformation parameter that is passed in. The first        */
   /* parameter is a pointer to the URBPtr that stores the memory       */
   /* to be copied over. The second parameter is a pointer to the       */
   /* USBBufferInformation that store's the memory to be copied to.     */
static void CopyURBMemoryToLocalReadMemory(struct urb *URBPtr, USBBufferInformation_t *BufferInfo)
{
   int            LengthOfData;
   unsigned char *TempPtr;

   /* Lets make sure the URB Pointer is semi-valid.                     */
   if(URBPtr)
   {
      /* Get the data length and a pointer to the data.                 */
      LengthOfData = URBPtr->actual_length;
      TempPtr      = URBPtr->transfer_buffer;

      /* Lets make sure we have room available.                         */
      if((LengthOfData <= (BufferInfo->CircleBufferSize - BufferInfo->CircleBufferBytesAvailable)))
      {
         /* Now Let's copy the Data into the Input Data Buffer.         */

         /* We need to see if copying the Data into the Buffer directly */
         /* will cause the Buffer Pointer to wrap.                      */
         if((BufferInfo->CircleBufferWriteIndex + LengthOfData) > BufferInfo->CircleBufferSize)
         {
            /* Copy the data to fill up to the end of the Buffer.       */
            memcpy(&(BufferInfo->CircleBuffer[BufferInfo->CircleBufferWriteIndex]), TempPtr, (BufferInfo->CircleBufferSize - BufferInfo->CircleBufferWriteIndex));

            /* Now Copy the data starting at the beginning of the       */
            /* Buffer.                                                  */
            memcpy(&(BufferInfo->CircleBuffer[0]), &TempPtr[(BufferInfo->CircleBufferSize - BufferInfo->CircleBufferWriteIndex)], (LengthOfData - (BufferInfo->CircleBufferSize - BufferInfo->CircleBufferWriteIndex)));
         }
         else
         {
            /* The buffer will not wrap, so simply copy the data into   */
            /* the Buffer.                                              */
            memcpy(&(BufferInfo->CircleBuffer[BufferInfo->CircleBufferWriteIndex]), TempPtr, LengthOfData);
         }

         /* Increment the Data Write Pointer and the amount of data put */
         /* into the Input Buffer.                                      */
         BufferInfo->CircleBufferBytesAvailable += LengthOfData;
         BufferInfo->CircleBufferWriteIndex     += LengthOfData;

         /* Wrap the Input Buffer Write Pointer (if needed).            */
         BufferInfo->CircleBufferWriteIndex %= BufferInfo->CircleBufferSize;
      }
      else
      {
         DEBUGPRINT(KERN_ERR "BTUSB Rx Buffer Overflow\n");
      }
   }
}

   /* This function copy's the URB memory to the circlur buffers in     */
   /* the ReadInformation parameter that is passed in. The first        */
   /* parameter is a pointer to the URBPtr that stores the memory       */
   /* to be copied over. The second parameter is a pointer to the       */
   /* USBBufferInformation that store's the memory to be copied to.     */
static void CopyIsoURBMemoryToLocalReadMemory(struct urb *URBPtr, USBBufferInformation_t *BufferInfo)
{
   int            LengthOfData;
   int            NumberIsoPacket;
   int            Status;
   unsigned char *TempPtr;

   /* Lets make sure the URB Pointer is semi-valid.                     */
   if(URBPtr)
   {
      for(NumberIsoPacket=0;NumberIsoPacket<URBPtr->number_of_packets;NumberIsoPacket++)
      {
         /* Get the data length and a pointer to the data.              */
         LengthOfData = URBPtr->iso_frame_desc[NumberIsoPacket].actual_length;
         TempPtr      = URBPtr->transfer_buffer + URBPtr->iso_frame_desc[NumberIsoPacket].offset;
         Status       = URBPtr->iso_frame_desc[NumberIsoPacket].status;

         if(Status >= 0)
         {
            /* Lets make sure we have room available.                   */
            if((LengthOfData <= (BufferInfo->CircleBufferSize - BufferInfo->CircleBufferBytesAvailable)))
            {
               /* Now Let's copy the Data into the Input Data Buffer.   */

               /* We need to see if copying the Data into the Buffer    */
               /* directly will cause the Buffer Pointer to wrap.       */
               if((BufferInfo->CircleBufferWriteIndex + LengthOfData) > BufferInfo->CircleBufferSize)
               {
                  /* Copy the data to fill up to the end of the Buffer. */
                  memcpy(&(BufferInfo->CircleBuffer[BufferInfo->CircleBufferWriteIndex]), TempPtr, (BufferInfo->CircleBufferSize - BufferInfo->CircleBufferWriteIndex));

                  /* Now Copy the data starting at the beginning of the */
                  /* Buffer.                                            */
                  memcpy(&(BufferInfo->CircleBuffer[0]), &TempPtr[(BufferInfo->CircleBufferSize - BufferInfo->CircleBufferWriteIndex)], (LengthOfData - (BufferInfo->CircleBufferSize - BufferInfo->CircleBufferWriteIndex)));
               }
               else
               {
                  /* The buffer will not wrap, so simply copy the data  */
                  /* into the Buffer.                                   */
                  memcpy(&(BufferInfo->CircleBuffer[BufferInfo->CircleBufferWriteIndex]), TempPtr, LengthOfData);
               }

               /* Increment the Data Write Pointer and the amount of    */
               /* data put into the Input Buffer.                       */
               BufferInfo->CircleBufferBytesAvailable += LengthOfData;
               BufferInfo->CircleBufferWriteIndex     += LengthOfData;

               /* Wrap the Input Buffer Write Pointer (if needed).      */
               BufferInfo->CircleBufferWriteIndex %= BufferInfo->CircleBufferSize;
            }
            else
            {
               DEBUGPRINT(KERN_ERR "BTUSB ISO Rx Buffer Overflow\n");
            }
         }
         else
         {
            DEBUGPRINT(KERN_ERR "BTUSB ISO Packet Error status %d\n", Status);
         }
      }
   }
}

   /* The following function is a support function used to close a      */
   /* device.  The function can be triggered by a user program caling   */
   /* the close device function, or in the event the device is removed  */
   /* from the system.  In either case, the response is to try and give */
   /* writes time to finish, then kill the urbs used for the writes.    */
   /* Next reads are stopped.  Finally, all memory pointers are reset.  */
static void CloseDevice(USBDeviceInformation_t *USBDeviceInformationPtr)
{
   int Index;

   /* First check for pending SCO data write urbs in process.           */
   DEBUGPRINT(KERN_ERR "%s Pending SCO writes %d\n", __FUNCTION__, atomic_read(&(USBDeviceInformationPtr->SCOBufferInformation.OutstandingWrites)));

   if(atomic_read(&(USBDeviceInformationPtr->SCOBufferInformation.OutstandingWrites)))
   {
      for(Index=0;Index<MAX_WRITE_BUFFERS;Index++)
      {
         /* Cancel all write URB's that is not in the free list.        */
         if(!IsItemInList(&(USBDeviceInformationPtr->SCOBufferInformation.FreeURBBufferList), &(USBDeviceInformationPtr->SCOBufferInformation.URBBufferInfo[Index].ListEntry)))
         {
            /* Kill urb, note the callback function will be invoked.    */
            usb_kill_urb(USBDeviceInformationPtr->SCOBufferInformation.URBBufferInfo[Index].URB);

            DEBUGPRINT(KERN_ERR "%s Killing SCO urb %p\n", __FUNCTION__, USBDeviceInformationPtr->SCOBufferInformation.URBBufferInfo[Index].URB);
         }
      }
   }

   /* Next check for pending ACL data write urbs in process.            */
   DEBUGPRINT(KERN_ERR "%s Pending ACL writes %d\n", __FUNCTION__, atomic_read(&(USBDeviceInformationPtr->ACLBufferInformation.OutstandingWrites)));

   if(atomic_read(&(USBDeviceInformationPtr->ACLBufferInformation.OutstandingWrites)))
   {
      for(Index=0;Index<MAX_WRITE_BUFFERS;Index++)
      {
         /* Cancel all write URB's that is not in the free list.        */
         if(!IsItemInList(&(USBDeviceInformationPtr->ACLBufferInformation.FreeURBBufferList), &(USBDeviceInformationPtr->ACLBufferInformation.URBBufferInfo[Index].ListEntry)))
         {
            /* Kill urb, note the callback function will be invoked.    */
            usb_kill_urb(USBDeviceInformationPtr->ACLBufferInformation.URBBufferInfo[Index].URB);

            DEBUGPRINT(KERN_ERR "%s Killing ACL urb %p\n", __FUNCTION__, USBDeviceInformationPtr->ACLBufferInformation.URBBufferInfo[Index].URB);
         }
      }
   }

   /* Last check for pending HCI data write urbs in process.            */
   DEBUGPRINT(KERN_ERR "%s Pending HCI writes %d\n", __FUNCTION__, atomic_read(&(USBDeviceInformationPtr->HCIBufferInformation.OutstandingWrites)));
   if(atomic_read(&(USBDeviceInformationPtr->HCIBufferInformation.OutstandingWrites)))
   {
      for(Index=0;Index<MAX_WRITE_BUFFERS;Index++)
      {
         /* Cancel all write URB's that aren't on the free list.        */
         if(!IsItemInList(&(USBDeviceInformationPtr->HCIBufferInformation.FreeURBBufferList), &(USBDeviceInformationPtr->HCIBufferInformation.URBBufferInfo[Index].ListEntry)))
         {
            /* Kill urb, note the callback function will be invoked.    */
            usb_kill_urb(USBDeviceInformationPtr->HCIBufferInformation.URBBufferInfo[Index].URB);

            DEBUGPRINT(KERN_ERR "%s Killing HCI urb %p\n", __FUNCTION__, USBDeviceInformationPtr->HCIBufferInformation.URBBufferInfo[Index].URB);
         }
      }
   }

   DEBUGPRINT(KERN_ERR "%s : Stop read URB's\n", __FUNCTION__);

   /* Tell the USB system to quit using the ACL and HCI URB's.          */
   StopHCIEventACL(USBDeviceInformationPtr);

   /* Check to see if SCO is currently enabled.                         */
   if(USBDeviceInformationPtr->SCO_Enabled)
   {
      /* Kill the primary SCO URB used for reading.                     */
      usb_kill_urb(USBDeviceInformationPtr->SCOBufferInformation.ReadURBHandle);

      DEBUGPRINT(KERN_ERR "%s SCO usb_kill_urb\n", __FUNCTION__);

      for(Index=0;Index<MAX_ISO_BUFFERS;Index++)
      {
         /* Kill secondary SCO URB's used for reading.                  */
         usb_kill_urb(USBDeviceInformationPtr->SCOBufferInformation.IsoExtraReadURBHandle[Index]);

         DEBUGPRINT(KERN_ERR "%s AuxSCO usb_kill_urb\n", __FUNCTION__);
      }

      USBDeviceInformationPtr->SCO_Enabled = 0;

      /* We need to set the interface to the default alternate setting  */
      /* that does not support ISOC transfers.                          */
      usb_set_interface(USBDeviceInformationPtr->USBDevice, BLUETOOTH_USB_INTERFACE_1, 0);
   }

   /* Flush all the memory since the device is closed.                  */
   USBDeviceInformationPtr->ACLBufferInformation.CircleBufferBytesAvailable = 0;
   USBDeviceInformationPtr->ACLBufferInformation.CircleBufferReadIndex      = 0;
   USBDeviceInformationPtr->ACLBufferInformation.CircleBufferWriteIndex     = 0;
   USBDeviceInformationPtr->HCIBufferInformation.CircleBufferBytesAvailable = 0;
   USBDeviceInformationPtr->HCIBufferInformation.CircleBufferReadIndex      = 0;
   USBDeviceInformationPtr->HCIBufferInformation.CircleBufferWriteIndex     = 0;
}

   /* The following function is the Completion Callback function that is*/
   /* installed for the HCI Event Interrupt Transfer and ACL Data Bulk  */
   /* Transfers that are submitted to the USB Driver. The first value,  */
   /* is a pointer to the URBPtr which stores the memory that was read. */
   /* The second parameter is not used.                                 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
   static void USBReadCallback(struct urb *URBPtr, struct pt_regs *Regs)
#else
   static void USBReadCallback(struct urb *URBPtr)
#endif
{
   int                     LengthOfData;
   int                     pipe;
   int                     DevNum;
   int                     ret_val;
   USBDeviceInformation_t *USBDeviceInformationPtr;

   /* Make sure it looks like the parameters are valid.                 */
   if(URBPtr)
   {
      DEBUGPRINT(KERN_ERR "%s : %p Urb Status %d\n", __FUNCTION__, URBPtr, URBPtr->status);

      spin_lock(&ContextInformationSpinLock);

      /* Get the DeviceNumber that we save with this File pointer.  Then*/
      /* search and see if this device is still in use with our driver. */
      DevNum                  = (uintptr_t)URBPtr->context;
      USBDeviceInformationPtr = SearchDeviceInformationEntryByMinorDev(&ContextInformation.DeviceInformationList, DevNum);

      spin_unlock(&ContextInformationSpinLock);

      /* Check to make sure we have something valid.                    */
      if(USBDeviceInformationPtr)
      {
         DEBUGPRINT(KERN_ERR "%s : received %d bytes\n", __FUNCTION__, URBPtr->actual_length);

         /* Check that the data recieved is somewhat valid.             */
         LengthOfData = URBPtr->actual_length;
         if((!URBPtr->status) || (URBPtr->status == -ECOMM))
         {
            if(URBPtr->status == -ECOMM)
            {
               /* Data received faster than it could be written to      */
               /* system memory.                                        */
               DEBUGPRINT(KERN_ERR "%s : Possible data loss\n", __FUNCTION__);
            }

            /* Next get the SpinLock used to protect the Read           */
            /* Information Buffer.                                      */
            spin_lock(&(USBDeviceInformationPtr->ReadInformationSpinLock));

            /* Lets find out which urb this is from, ACL Data, SCO Data,*/
            /* or HCI Event.                                            */
            if(URBPtr == USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle)
            {
               DEBUGPRINT(KERN_ERR "%s : HCI Event %d bytes\n", __FUNCTION__, URBPtr->actual_length);

               /* Copy over the URB information into our circlur buffer */
               /* so the user can read the information back.            */
               CopyURBMemoryToLocalReadMemory(URBPtr, &USBDeviceInformationPtr->HCIBufferInformation);

               /* Release the Read Information SpinLock.                */
               spin_unlock(&(USBDeviceInformationPtr->ReadInformationSpinLock));

               /* Resubmit the URB Get the pipe number that the endpoint*/
               /* is attached to.                                       */
               pipe = usb_rcvintpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->HCIEventInDescriptor->bEndpointAddress);

               /* Clear out any flags that aren't covered by the        */
               /* usb_fill_xxx_urb() MACRO's.                           */
               URBPtr->status         = 0;
               URBPtr->actual_length  = 0;
               URBPtr->transfer_flags = 0;

               /* Fill or Set the URB ptr with the proper information.  */
               usb_fill_int_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle, USBDeviceInformationPtr->USBDevice, pipe, USBDeviceInformationPtr->HCIBufferInformation.ReadBuffer, USBDeviceInformationPtr->HCIBufferInformation.ReadBufferSize, USBReadCallback, (void *)(uintptr_t)DevNum, USBDeviceInformationPtr->HCIEventInDescriptor->bInterval);

               /* Submit the URB for reading.                           */
               ret_val = usb_submit_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle, GFP_ATOMIC);

               DEBUGPRINT(KERN_ERR "%s : resubmit HCI urb %d\n", __FUNCTION__, ret_val);

               wake_up(&(USBDeviceInformationPtr->ReadWaitEvent));
            }
            else
            {
               /* Check for ACL or SCO data.                            */
               if(URBPtr == USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle)
               {
                  DEBUGPRINT(KERN_ERR "%s : ACL Data %d bytes\n", __FUNCTION__, URBPtr->actual_length);

                  /* Copy over the URB information into our circular    */
                  /* buffer so the user can read the information back.  */
                  CopyURBMemoryToLocalReadMemory(URBPtr, &USBDeviceInformationPtr->ACLBufferInformation);

                  /* Release the Read Information SpinLock.             */
                  spin_unlock(&(USBDeviceInformationPtr->ReadInformationSpinLock));

                  /* Resubmit the URB Get the pipe number that the      */
                  /* endpoint is attached to.                           */
                  pipe = usb_rcvbulkpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->ACLDataInDescriptor->bEndpointAddress);

                  /* Clear out any flags that aren't covered by the     */
                  /* usb_fill_xxx_urb() MACRO's.                        */
                  URBPtr->status         = 0;
                  URBPtr->actual_length  = 0;
                  URBPtr->transfer_flags = 0;

                  /* Fill or Set the URB pointer with the proper        */
                  /* information.                                       */
                  usb_fill_bulk_urb(USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle, USBDeviceInformationPtr->USBDevice, pipe, USBDeviceInformationPtr->ACLBufferInformation.ReadBuffer, USBDeviceInformationPtr->ACLBufferInformation.ReadBufferSize, USBReadCallback, (void *)(uintptr_t)DevNum);

                  /* Submit the URB for reading.                        */
                  ret_val = usb_submit_urb(USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle, GFP_ATOMIC);

                  DEBUGPRINT(KERN_ERR "%s : resubmit ACL urb %d\n", __FUNCTION__, ret_val);

                  wake_up(&(USBDeviceInformationPtr->ReadWaitEvent));
               }
               else
               {
                  /* For safety, it would probably be good to check the */
                  /* return pointer and make sure it is SCO...          */
                  DEBUGPRINT(KERN_ERR "%s : SCO Data %d bytes\n", __FUNCTION__, URBPtr->actual_length);

                  if(URBPtr->status == -ENOENT)
                  {
                     /* We have tried to cancel this URB.               */
                     DEBUGPRINT(KERN_ERR "%s usb_kill_urb likely resulted in -ENOENT\n", __FUNCTION__);

                     /* Release the Read Information SpinLock.          */
                     spin_unlock(&(USBDeviceInformationPtr->ReadInformationSpinLock));
                  }
                  else
                  {
                     /* Check to make sure that SCO data is still       */
                     /* enabled before we submit another read urb.      */
                     if(USBDeviceInformationPtr->SCO_Enabled)
                     {
                        /* Copy over the URB information into our       */
                        /* circular buffer so the user can read the     */
                        /* information back.                            */
                        CopyIsoURBMemoryToLocalReadMemory(URBPtr, &USBDeviceInformationPtr->SCOBufferInformation);

                        /* Release the Read Information SpinLock.       */
                        spin_unlock(&(USBDeviceInformationPtr->ReadInformationSpinLock));

                        /* Re-submit this urb, regardless of whether it */
                        /* is an extra or main one.  if there were ever */
                        /* different buffer sizes, the readbuffersize   */
                        /* handling wouldn't work.                      */
                        refill_isoc_urb(URBPtr, USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->SCOBufferInformation.ReadBufferSize, le16_to_cpu(USBDeviceInformationPtr->SCODataInDescriptor->wMaxPacketSize));

                        /* Submit the URB for reading.                  */
                        ret_val = usb_submit_urb(URBPtr, GFP_ATOMIC);

                        DEBUGPRINT(KERN_ERR "%s : resubmit SCO urb %d\n", __FUNCTION__, ret_val);

                        wake_up(&(USBDeviceInformationPtr->ReadWaitEvent));
                     }
                     else
                     {
                        /* Release the Read Information SpinLock.       */
                        spin_unlock(&(USBDeviceInformationPtr->ReadInformationSpinLock));
                     }
                  }
               }
            }
         }
         else
         {
            /* Here if length of data is zero, or an error status is    */
            /* set, except for special case of possible data overrun.   */
            /* In that case, we left the code in the above segment to   */
            /* try and continue while absorbing the data loss.          */

            /* Note if we have SCO enabled, it is possible to get some  */
            /* zero data length packets.  We need to resubmit the URB's */
            /* in this case.  Otherwise, we process this as an error.   */
            if((!LengthOfData) && (!((URBPtr == USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle) || (URBPtr == USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle))))
            {
               if((USBDeviceInformationPtr->SCO_Enabled) && (!URBPtr->status))
               {
                  /* resubmit the URB, if SCO is enabled and not error  */
                  /* status Re-submit this urb, regardless of whether it*/
                  /* is an extra or main one.  if there were ever       */
                  /* different buffer sizes, the readbuffersize handling*/
                  /* wouldn't work.                                     */
                  refill_isoc_urb(URBPtr, USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->SCOBufferInformation.ReadBufferSize, le16_to_cpu(USBDeviceInformationPtr->SCODataInDescriptor->wMaxPacketSize));

                  /* Submit the URB for reading.                        */
                  ret_val = usb_submit_urb(URBPtr, GFP_ATOMIC);

                  DEBUGPRINT(KERN_ERR "%s : resubmit ISO urb (error) %d\n", __FUNCTION__, ret_val);
               }
               else
               {
                  DEBUGPRINT(KERN_ERR "%s : zero length ISO URB\n", __FUNCTION__);
               }
            }
            else
            {
               switch(URBPtr->status)
               {
                  case -ENOENT :
                     /* We have tried to cancel this URB with a call to */
                     /* usb_kill_urb. This is an expected result during */
                     /* device cleanup.                                 */
                     DEBUGPRINT(KERN_ERR "%s usb_kill_urb likely resulted in -ENOENT\n", __FUNCTION__);

                     /* Now, try to stop things.                        */
                     URBPtr->status        = 0;
                     URBPtr->actual_length = 0;
                     URBPtr->complete      = NULL;
                     break;
                  case -EINPROGRESS :
                     /* URB still being process by USB host controller. */
                     /* According to Device Driver book, if we see this,*/
                     /* it means we have a bug in our driver.           */
                     DEBUGPRINT(KERN_ERR "%s URB status says -EINPROGRESS\n", __FUNCTION__);

                     /* Now, try to stop things.                        */
                     URBPtr->status        = 0;
                     URBPtr->actual_length = 0;
                     URBPtr->complete      = NULL;
                     break;
                  case -ECONNRESET :
                     /* Urb cancelled with a call to usb_unlink_urb     */
                     /* (with transfer_flags = URB_ASYNC_UNLINK, for    */
                     /* kernels <=2.6.9). We shouldn't see this.        */
                     DEBUGPRINT(KERN_ERR "%s URB Status -ECONNRESET\n", __FUNCTION__);

                     /* Now, try to stop things.                        */
                     URBPtr->status        = 0;
                     URBPtr->actual_length = 0;
                     URBPtr->complete      = NULL;
                     break;
                  case -EPIPE :
                     /* Endpoint is stalled. Recovering from this status*/
                     /* may require sleeping, so we have to schedule the*/
                     /* recovery in a work queue so it will be handled  */
                     /* in a non-interrupt context.                     */
                     DEBUGPRINT(KERN_ERR "%s URB Status -EPIPE\n", __FUNCTION__);
                     if(URBPtr == USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle)
                     {
                        atomic_set(&(USBDeviceInformationPtr->HCIEventInStalled), 1);
                        schedule_work(&(USBDeviceInformationPtr->ClearStallWork));
                     }
                     else
                     {
                        if(URBPtr == USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle)
                        {
                           atomic_set(&(USBDeviceInformationPtr->ACLDataInStalled), 1);
                           schedule_work(&(USBDeviceInformationPtr->ClearStallWork));
                        }
                     }
                     break;
                  case -EPROTO :
                     /* If this is an incoming HCI packet (USB Interrupt*/
                     /* URB), simply ignore the contents and resubmit   */
                     /* the URB to prepare for the next packet.         */
                     if(URBPtr == USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle)
                     {
                        /* Resubmit the URB Get the pipe number that the*/
                        /* endpoint is attached to.                     */
                        pipe = usb_rcvintpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->HCIEventInDescriptor->bEndpointAddress);

                        /* Clear out any flags that aren't covered by   */
                        /* the usb_fill_xxx_urb() MACRO's.              */
                        URBPtr->status         = 0;
                        URBPtr->actual_length  = 0;
                        URBPtr->transfer_flags = 0;

                        /* Fill or Set the URB ptr with the proper      */
                        /* information.                                 */
                        usb_fill_int_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle, USBDeviceInformationPtr->USBDevice, pipe, USBDeviceInformationPtr->HCIBufferInformation.ReadBuffer, USBDeviceInformationPtr->HCIBufferInformation.ReadBufferSize, USBReadCallback, (void *)(uintptr_t)DevNum, USBDeviceInformationPtr->HCIEventInDescriptor->bInterval);

                        /* Submit the URB for reading.                  */
                        ret_val = usb_submit_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle, GFP_ATOMIC);

                        DEBUGPRINT(KERN_ERR "%s : Resubmitted interrupt urb (%d)\n", __FUNCTION__, ret_val);
                     }
                     break;
                  case -EILSEQ :
                  case -EOVERFLOW :
                     /* According to LINUX Device Driver, these         */
                     /* indicates a hardware, firmware or cable issue   */
                     /* with the device.                                */
                     DEBUGPRINT(KERN_ERR "%s : Possible hardware issue, URB status %d\n", __FUNCTION__, URBPtr->status);

                     /* Now, try to stop things.                        */
                     URBPtr->status        = 0;
                     URBPtr->actual_length = 0;
                     URBPtr->complete      = NULL;
                     break;
                  default :
                     DEBUGPRINT(KERN_ERR "%s URB Status %d\n", __FUNCTION__, URBPtr->status);

                     /* Now, try to stop things.                        */
                     URBPtr->status        = 0;
                     URBPtr->actual_length = 0;
                     URBPtr->complete      = NULL;
                     break;
               }
            }
         }
      }
      else
      {
         DEBUGPRINT(KERN_ERR "%s : Dev Not Found: %d\n", __FUNCTION__, DevNum);
      }
   }
}

   /* This function will be callback back by the write submit method.   */
   /* The parameter URBPtr is a pointer to the urb we submitted.        */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
   static void USBWriteCallback(struct urb *URBPtr, struct pt_regs *Regs)
#else
   static void USBWriteCallback(struct urb *URBPtr)
#endif
{
   USBBufferInformation_t *USBBufferInformationPtr;

   /* Make sure the URB Pointer looks valid.                            */
   if((URBPtr) && (URBPtr->context))
   {
      USBBufferInformationPtr = ((URBBufferInfo_t *)URBPtr->context)->USBBufferInformation;

      /* Check to see if we found a USBBufferInformationPtr.            */
      if(USBBufferInformationPtr)
      {
         /* Check for any errors that might have occured.  Plus the     */
         /* count of bytes that were sent.                              */
         if((URBPtr->status) || (!URBPtr->actual_length))
         {
            DEBUGPRINT(KERN_ERR ": ERROR Writing URB Status = %d, Count = %d\n", URBPtr->status, URBPtr->actual_length);
         }

         /* Obtain the write SpinLock.                                  */
         DEBUGPRINT(KERN_ERR "%s: Waiting for Write Information SpinLock\n", __FUNCTION__);

         spin_lock(&(USBBufferInformationPtr->WriteInformationSpinLock));

         /* Decrement count of outstanding writes.                      */
         if(atomic_add_negative(-1, &(USBBufferInformationPtr->OutstandingWrites)))
         {
            DEBUGPRINT(KERN_ERR "%s: ERROR OutstandingWrites is negative: %d. This should never happen.\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));
         }

         /* Process the URB.                                            */
         if((URBPtr->status == -EPIPE) && ((USBBufferInformationPtr->USBDeviceInformation) && (USBBufferInformationPtr == &(USBBufferInformationPtr->USBDeviceInformation->ACLBufferInformation))))
         {
            /* The only endpoint that we can ever see stall here is the */
            /* ACL data path since the only bulk transfer URBs can stall*/
            /* during a Send. If any other outbound endpoint stalls, we */
            /* either won't be notified (SCO) or there's nothing we can */
            /* do about it (HCI).                                       */
            DEBUGPRINT(KERN_ERR "%s: Incoming ACL endpoint stalled. Scheduling recovery routine.\n", __FUNCTION__);

            atomic_set(&(USBBufferInformationPtr->USBDeviceInformation->ACLDataOutStalled), 1);

            /* Schedule the work to clear the stall. We leave the URB on*/
            /* the Outstanding Writes list so it will be re-transmitted */
            /* after the stall is cleared.                              */
            schedule_work(&(USBBufferInformationPtr->USBDeviceInformation->ClearStallWork));
         }
         else
         {
            if(URBPtr->status == -ECONNRESET)
            {
               /* This URB was cancelled by a call to usb_unlink_urb.   */
               /* A cancelled URB indicates that an error occurred but  */
               /* we intend to recover from it. In that case, the URB   */
               /* has already been flagged for retransmission. We leave */
               /* the URB on the Outstanding Writes list so it will be  */
               /* re-transmitted after the stall is cleared.            */
               DEBUGPRINT(KERN_ERR "%s: URB %p was cancelled for later retransmission\n", __FUNCTION__, URBPtr);
            }
            else
            {
               /* Either the URB was sent successfully, or it           */
               /* encountered an error we can't recover from.           */
               DEBUGPRINT(KERN_ERR "%s: Snd URB completed, moving URB back to free list.\n", __FUNCTION__);

               /* Remove the URB from the outstanding writes list.      */
               /* Sanity check that the URB is in the list, first.      */
               if(IsItemInList(&(USBBufferInformationPtr->OutstandingWritesList), (ListEntry_t *)(URBPtr->context)))
               {
                  RemoveListEntry((ListEntry_t *)(URBPtr->context));
               }
               else
               {
                  DEBUGPRINT(KERN_ERR "%s: URB BufferInfo %p was not found on the outstanding writes list\n", __FUNCTION__, URBPtr->context);
               }

               /* Push the Urb back onto the free list.                 */
               AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)(URBPtr->context));
            }
         }

         /* Set the Event to wake up the blocking Wait event in the     */
         /* ioctl function. This is in case we are blocking in the write*/
         /* ioctl function until an URB becomes free.                   */
         DEBUGPRINT(KERN_ERR "%s: Set Write Wait Event\n", __FUNCTION__);
         wake_up(&(USBBufferInformationPtr->WriteWaitEvent));

         DEBUGPRINT(KERN_ERR "%s:  URB %p, buffer %p, BufferInfo %p processed, OutstandingWrites %d\n", __FUNCTION__, URBPtr, URBPtr->transfer_buffer, URBPtr->context, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

         /* Release the SpinLock.                                       */
         DEBUGPRINT(KERN_ERR "%s: Releasing Write Information SpinLock\n", __FUNCTION__);

         spin_unlock(&(USBBufferInformationPtr->WriteInformationSpinLock));
      }
   }
}

   /* The following function is a schedulable work unit for clearing a  */
   /* STALL status of a USB endpoint.                                   */
static void ClearStall(struct work_struct *Work)
{
   int                     Result;
   ListEntry_t            *ListEntry;
   ListEntry_t             ResubmitListHead;
   unsigned int            Pipe;
   unsigned long           Flags;
   URBBufferInfo_t        *URBBufferInfo;
   USBDeviceInformation_t *USBDeviceInformation;

   DEBUGPRINT(KERN_ERR "%s: Begin STALL clearing routine\n", __FUNCTION__);

   if((USBDeviceInformation = container_of(Work, USBDeviceInformation_t, ClearStallWork)) != NULL)
   {
      if(atomic_read(&(USBDeviceInformation->HCIEventInStalled)))
      {
         DEBUGPRINT(KERN_ERR "%s: Clearing STALL on HCI Event endpoint\n", __FUNCTION__);

         /* There should be no outstanding Receive URBS on the HCI Event*/
         /* endpoint since only one is ever in use and it completed     */
         /* in order to report the stall. Just clear the stall and      */
         /* re-submit the URB.                                          */
         Pipe = usb_rcvintpipe(USBDeviceInformation->USBDevice, USBDeviceInformation->HCIEventInDescriptor->bEndpointAddress);

         if(!usb_clear_halt(USBDeviceInformation->USBDevice, Pipe))
         {
            /* The stall is cleared, so flag the condition and re-submit*/
            /* the Read URB for this endpoint.                          */
            atomic_set(&(USBDeviceInformation->HCIEventInStalled), 0);

            /* Next get the SpinLock used to protect the Read           */
            /* Information Buffer.                                      */
            spin_lock_irqsave(&(USBDeviceInformation->ReadInformationSpinLock), Flags);

            /* Clear out any flags that aren't covered by the           */
            /* usb_fill_xxx_urb() MACRO's.                              */
            USBDeviceInformation->HCIBufferInformation.ReadURBHandle->status         = 0;
            USBDeviceInformation->HCIBufferInformation.ReadURBHandle->actual_length  = 0;
            USBDeviceInformation->HCIBufferInformation.ReadURBHandle->transfer_flags = 0;

            /* Fill or Set the URB ptr with the proper information.     */
            usb_fill_int_urb(USBDeviceInformation->HCIBufferInformation.ReadURBHandle, USBDeviceInformation->USBDevice, Pipe, USBDeviceInformation->HCIBufferInformation.ReadBuffer, USBDeviceInformation->HCIBufferInformation.ReadBufferSize, USBReadCallback, USBDeviceInformation->HCIBufferInformation.ReadURBHandle->context, USBDeviceInformation->HCIEventInDescriptor->bInterval);

            /* Submit the URB for reading.                              */
            Result = usb_submit_urb(USBDeviceInformation->HCIBufferInformation.ReadURBHandle, GFP_ATOMIC);

            DEBUGPRINT(KERN_ERR "%s: resubmit HCI rcv urb %d\n", __FUNCTION__, Result);

            /* Release the Read Information SpinLock.                   */
            spin_unlock_irqrestore(&(USBDeviceInformation->ReadInformationSpinLock), Flags);
         }
      }

      if(atomic_read(&(USBDeviceInformation->ACLDataInStalled)))
      {
         DEBUGPRINT(KERN_ERR "%s: Clearing STALL on ACL Data (incoming) endpoint\n", __FUNCTION__);

         /* There should be no outstanding Receive URBS on the ACL Data */
         /* endpoint since only one is ever in use and it completed     */
         /* in order to report the stall. Just clear the stall and      */
         /* re-submit the URB.                                          */
         Pipe = usb_rcvbulkpipe(USBDeviceInformation->USBDevice, USBDeviceInformation->ACLDataInDescriptor->bEndpointAddress);

         if(!usb_clear_halt(USBDeviceInformation->USBDevice, Pipe))
         {
            /* The stall is cleared, so flag the condition and re-submit*/
            /* the Read URB for this endpoint.                          */
            atomic_set(&(USBDeviceInformation->ACLDataInStalled), 0);

            /* Next get the SpinLock used to protect the Read           */
            /* Information Buffer.                                      */
            spin_lock_irqsave(&(USBDeviceInformation->ReadInformationSpinLock), Flags);

            /* Clear out any flags that aren't covered by the           */
            /* usb_fill_xxx_urb() MACRO's.                              */
            USBDeviceInformation->ACLBufferInformation.ReadURBHandle->status         = 0;
            USBDeviceInformation->ACLBufferInformation.ReadURBHandle->actual_length  = 0;
            USBDeviceInformation->ACLBufferInformation.ReadURBHandle->transfer_flags = 0;

            /* Fill or Set the URB pointer with the proper information. */
            usb_fill_bulk_urb(USBDeviceInformation->ACLBufferInformation.ReadURBHandle, USBDeviceInformation->USBDevice, Pipe, USBDeviceInformation->ACLBufferInformation.ReadBuffer, USBDeviceInformation->ACLBufferInformation.ReadBufferSize, USBReadCallback, USBDeviceInformation->ACLBufferInformation.ReadURBHandle->context);

            /* Submit the URB for reading.                              */
            Result = usb_submit_urb(USBDeviceInformation->ACLBufferInformation.ReadURBHandle, GFP_ATOMIC);

            DEBUGPRINT(KERN_ERR "%s: resubmit ACL rcv urb %d\n", __FUNCTION__, Result);

            /* Release the Read Information SpinLock.                   */
            spin_unlock_irqrestore(&(USBDeviceInformation->ReadInformationSpinLock), Flags);
         }
      }

      if(atomic_read(&(USBDeviceInformation->ACLDataOutStalled)))
      {
         DEBUGPRINT(KERN_ERR "%s: ACL Data (outgoing) endpoint is STALLed\n", __FUNCTION__);

         InitializeList(&ResubmitListHead);

         spin_lock_irqsave(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

         while(atomic_read(&(USBDeviceInformation->ACLBufferInformation.OutstandingWrites)) > 0)
         {
            DEBUGPRINT(KERN_ERR "%s: There appears to be %d outstanding snd URBs. This could be a combination of active and failed URBs.\n", __FUNCTION__, atomic_read(&(USBDeviceInformation->ACLBufferInformation.OutstandingWrites)));

            ListEntry = USBDeviceInformation->ACLBufferInformation.OutstandingWritesList.NextEntry;

            while(ListEntry != &(USBDeviceInformation->ACLBufferInformation.OutstandingWritesList))
            {
               URBBufferInfo = container_of(ListEntry, URBBufferInfo_t, ListEntry);

               if(URBBufferInfo)
               {
                  if((Result = usb_unlink_urb(URBBufferInfo->URB)) == -EINPROGRESS)
                  {
                     DEBUGPRINT(KERN_ERR "%s: Successfully unlinked URB %p of URBBufferInfo %p\n", __FUNCTION__, URBBufferInfo->URB, URBBufferInfo);
                  }
                  else
                  {
                     if((Result == -EBUSY) || (Result == -EIDRM))
                     {
                        DEBUGPRINT(KERN_ERR "%s: URB %p or URBBufferInfo %p, appears to already be inactive (%d). Either it's being completed now, or it's already queued for resend due to a STALL.\n", __FUNCTION__, URBBufferInfo->URB, URBBufferInfo, Result);
                     }
                  }
               }

               ListEntry = ListEntry->NextEntry;
            }

            /* Release the SpinLock.                                    */
            spin_unlock_irqrestore(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

            DEBUGPRINT(KERN_ERR "%s: Waiting until all outstanding writes (%d) are cleared out\n", __FUNCTION__, atomic_read(&(USBDeviceInformation->ACLBufferInformation.OutstandingWrites)));

            /* Wait for the URB cancellations to take affect.           */
            wait_event_interruptible(USBDeviceInformation->ACLBufferInformation.WriteWaitEvent, (atomic_read(&(USBDeviceInformation->ACLBufferInformation.OutstandingWrites)) == 0));

            /* Reacquire the spinlock.                                  */
            spin_lock_irqsave(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

            /* All outstanding URBs should now be cancelled and left on */
            /* the Outstanding Writes list. Move them all to our local  */
            /* list so we can avoid locking later.                      */
            DEBUGPRINT(KERN_ERR "%s: Emptying OutstandingWritesList\n", __FUNCTION__);
            while((ListEntry = RemoveFirstListEntry(&(USBDeviceInformation->ACLBufferInformation.OutstandingWritesList))) != NULL)
            {
               if((container_of(ListEntry, URBBufferInfo_t, ListEntry)->URB) && ((container_of(ListEntry, URBBufferInfo_t, ListEntry)->URB->status == -EPIPE) || (container_of(ListEntry, URBBufferInfo_t, ListEntry)->URB->status == -ECONNRESET)))
                  AddListEntry(&ResubmitListHead, ListEntry);
            }

#ifdef DEBUG_ENABLE
            if(USBDeviceInformation->ACLBufferInformation.OutstandingWritesList.NextEntry != &(USBDeviceInformation->ACLBufferInformation.OutstandingWritesList))
               DEBUGPRINT(KERN_ERR "%s: ERROR OutstandingWritesList not empty\n", __FUNCTION__);
#endif

         }

         /* Release the SpinLock.                                       */
         spin_unlock_irqrestore(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

         DEBUGPRINT(KERN_ERR "%s: Clearing STALL on ACL Data (outgoing) endpoint\n", __FUNCTION__);

         /* Now there are no outstanding URBs. Resubmit the cancelled   */
         /* URBs and clear the stalled status.                          */
         Pipe = usb_rcvintpipe(USBDeviceInformation->USBDevice, USBDeviceInformation->HCIEventInDescriptor->bEndpointAddress);

         if(!(Result = usb_clear_halt(USBDeviceInformation->USBDevice, Pipe)))
         {
            if(ResubmitListHead.NextEntry != &ResubmitListHead)
            {
               /* The stall is cleared, so resubmit any pending URBs.   */
               DEBUGPRINT(KERN_ERR "%s: Resending all cancelled URBs\n", __FUNCTION__);

               /* Stall was cleared successfully. Resubmit all the URBs */
               /* that were cancelled.                                  */
               while((ListEntry = RemoveFirstListEntry(&ResubmitListHead)) != NULL)
               {
                  URBBufferInfo = container_of(ListEntry, URBBufferInfo_t, ListEntry);

                  if((URBBufferInfo) && ((URBBufferInfo->URB->status == -EPIPE) || (URBBufferInfo->URB->status == -ECONNRESET)))
                  {
                     /* Increment the Outstanding Writes count and      */
                     /* add the URB to the Outstanding Writes List in   */
                     /* preparation for submitting it.                  */
                     atomic_inc(&(USBDeviceInformation->ACLBufferInformation.OutstandingWrites));

                     spin_lock_irqsave(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

                     AddListEntry(&(USBDeviceInformation->ACLBufferInformation.OutstandingWritesList), ListEntry);

                     spin_unlock_irqrestore(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

                     /* Submit the cancelled URB to the usb core. All   */
                     /* the initialized fields should still be valid.   */
                     if(!(Result = usb_submit_urb(URBBufferInfo->URB, GFP_KERNEL)))
                     {
                        /* URB resubmited scucessfully.                 */
                        DEBUGPRINT(KERN_ERR "%s: Resubmitted cancelled URB %p\n", __FUNCTION__, URBBufferInfo->URB);
                     }
                     else
                     {
                        /* Failed to resubmit URB. Move it from the     */
                        /* outstanding list back to the free list.      */
                        DEBUGPRINT(KERN_ERR "%s: ERROR Failed to resubmit cancelled URB %p, Result %d\n", __FUNCTION__, URBBufferInfo->URB, Result);

                        atomic_dec(&(USBDeviceInformation->ACLBufferInformation.OutstandingWrites));

                        spin_lock_irqsave(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

                        RemoveListEntry(ListEntry);
                        AddListEntry(&(USBDeviceInformation->ACLBufferInformation.FreeURBBufferList), ListEntry);

                        spin_unlock_irqrestore(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);
                     }
                  }
                  else
                  {
                     /* The current URB from the ResubmitListHead list  */
                     /* is not in a cancelled state. This should never  */
                     /* happen.                                         */
                     DEBUGPRINT(KERN_ERR "%s: ERROR URB scheduled for resending is not in a cancelled or stalled state\n", __FUNCTION__);
                     AddListEntry(&(USBDeviceInformation->ACLBufferInformation.OutstandingWritesList), ListEntry);
                  }

                  ListEntry = ListEntry->NextEntry;
               }
            }
            else
            {
               DEBUGPRINT(KERN_ERR "%s: No pending URBs to resubmit\n", __FUNCTION__);
            }

            /* Flag the stalled status as cleared now that all URBs have*/
            /* been re-submitted.                                       */
            atomic_set(&(USBDeviceInformation->ACLDataOutStalled), 0);
         }
         else
         {
            /* The call to clear the stall status failed. Pull all the  */
            /* cancelled URBs off the ResubmitListHead list and push    */
            /* then back to the free list.                              */
            DEBUGPRINT(KERN_ERR "%s: Unable to clear stalled status on ACL Out endpoint: %d\n", __FUNCTION__, Result);
            DEBUGPRINT(KERN_ERR "%s: Aborting all suspended URBs.\n", __FUNCTION__);

            while((ListEntry = RemoveFirstListEntry(&ResubmitListHead)) != NULL)
            {
               spin_lock_irqsave(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);

               DEBUGPRINT(KERN_ERR "%s: URBBufferInfo %p moved to free list\n", __FUNCTION__, container_of(ListEntry, URBBufferInfo_t, ListEntry));
               AddListEntry(&(USBDeviceInformation->ACLBufferInformation.FreeURBBufferList), ListEntry);

               spin_unlock_irqrestore(&(USBDeviceInformation->ACLBufferInformation.WriteInformationSpinLock), Flags);
            }
         }

         DEBUGPRINT(KERN_ERR "%s: Set Write Wait Event\n", __FUNCTION__);

         wake_up(&(USBDeviceInformation->ACLBufferInformation.WriteWaitEvent));
      }
   }
}

#ifdef NEW_FW_DL_MECHANISM

static int ath3k_get_state(struct usb_device *udev, unsigned char *state)
{
   int pipe = 0;

   pipe = usb_rcvctrlpipe(udev, 0);
   return usb_control_msg(udev, pipe, ATH3K_GETSTATE,
                          USB_TYPE_VENDOR | USB_DIR_IN, 0, 0,
                          state, 0x01, USB_CTRL_SET_TIMEOUT);
}

static int ath3k_get_version(struct usb_device *udev, struct ath3k_version *version)
{
   int pipe = 0;

   pipe = usb_rcvctrlpipe(udev, 0);
   return usb_control_msg(udev, pipe, ATH3K_GETVERSION,
                          USB_TYPE_VENDOR | USB_DIR_IN, 0, 0, version,
                          sizeof(struct ath3k_version),
                          USB_CTRL_SET_TIMEOUT);
}

int get_rome_version(struct usb_device *udev)
{
   struct ath3k_version fw_version;
   int ret = 0;

   ret = ath3k_get_version(udev, &fw_version);
   if (ret < 0)
   {
      BT_ERR("Failed to get Rome Firmware version");
      return ret;
   }

   switch (fw_version.rom_version)
   {
      case ROME1_1_USB_CHIP_VERSION:
      case ROME2_1_USB_CHIP_VERSION:
      case ROME3_0_USB_CHIP_VERSION:
      case ROME3_2_USB_CHIP_VERSION:
      case NPL1_0_USB_CHIP_VERSION:
         ret = fw_version.rom_version;
         break;
      default:
         ret = 0;
         break;
   }
   return ret;
}

static int ath3k_load_fwfile(struct usb_device *udev, const struct firmware *firmware, int header_h)
{
   u8 *send_buf;
   int err, pipe, len, size, count, sent = 0;
   int ret;

   count = firmware->size;

   send_buf = kmalloc(BULK_SIZE, GFP_KERNEL);
   if (!send_buf)
   {
      BT_ERR("Can't allocate memory chunk for firmware");
      return -ENOMEM;
   }

   size = min_t(uint, count, header_h);
   memcpy(send_buf, firmware->data, size);

   pipe = usb_sndctrlpipe(udev, 0);
   ret = usb_control_msg(udev, pipe, ATH3K_DNLOAD,
                         USB_TYPE_VENDOR, 0, 0, send_buf,
                         size, USB_CTRL_SET_TIMEOUT);
   if (ret < 0)
   {
      BT_ERR("Can't change to loading configuration err");
      kfree(send_buf);
      return ret;
   }

   sent += size;
   count -= size;

   while (count)
   {
      size = min_t(uint, count, BULK_SIZE);
      pipe = usb_sndbulkpipe(udev, 0x02);

      memcpy(send_buf, firmware->data + sent, size);

      err = usb_bulk_msg(udev, pipe, send_buf, size,
                         &len, 3000);
      if (err || (len != size))
      {
         BT_ERR("Error in firmware loading err = %d,"
                "len = %d, size = %d", err, len, size);
         kfree(send_buf);
         return err;
      }
      sent  += size;
      count -= size;
   }

   kfree(send_buf);
   return 0;
}

static int ath3k_load_patch(struct usb_device *udev)
{
   unsigned char fw_state;
   char filename[ATH3K_NAME_LEN] = {0};
   const struct firmware *firmware;
   struct ath3k_version fw_version, pt_version;
   struct rome2_1_version *rome2_1_version;
   struct rome1_1_version *rome1_1_version;
   int ret;

   ret = ath3k_get_state(udev, &fw_state);
   if (ret < 0)
   {
      BT_ERR("Can't get state to change to load ram patch err: %d", ret);
      return ret;
   }

   if ((fw_state == ATH3K_PATCH_UPDATE) ||
         (fw_state == ATH3K_PATCH_SYSCFG_UPDATE))
   {
      BT_INFO("%s: Patch already downloaded(fw_state: %d)", __func__,
              fw_state);
      return 0;
   }
   else
      BT_ERR("%s: Downloading RamPatch(fw_state: %d)", __func__,
             fw_state);

   ret = ath3k_get_version(udev, &fw_version);
   if (ret < 0)
   {
      BT_ERR("Can't get version to change to load ram patch err");
      return ret;
   }
   if (fw_version.rom_version == ROME1_1_USB_CHIP_VERSION)
   {
      BT_DBG("Chip Detected as ROME1.1");
      snprintf(filename, ATH3K_NAME_LEN, ROME1_1_USB_RAMPATCH_FILE);
   }
   else if (fw_version.rom_version == ROME2_1_USB_CHIP_VERSION)
   {
      BT_DBG("Chip Detected as ROME2.1");
      snprintf(filename, ATH3K_NAME_LEN, ROME2_1_USB_RAMPATCH_FILE);
   }
   else if (fw_version.rom_version == ROME3_0_USB_CHIP_VERSION)
   {
      BT_DBG("Chip Detected as ROME3.0");
      snprintf(filename, ATH3K_NAME_LEN, ROME3_0_USB_RAMPATCH_FILE);
   }
   else if (fw_version.rom_version == ROME3_2_USB_CHIP_VERSION)
   {
      if(fw_version.soc_version == 0x23)
      {
         BT_DBG("Chip Detected as TF 1.1");
         snprintf(filename, ATH3K_NAME_LEN, TF1_1_USB_RAMPATCH_FILE);
      }
      else
      {
         BT_DBG("Chip Detected as Rome 3.2");
         snprintf(filename, ATH3K_NAME_LEN, ROME3_2_USB_RAMPATCH_FILE);
      }
   }
   else if (fw_version.rom_version == NPL1_0_USB_CHIP_VERSION)
   {
      BT_DBG("Chip Detected as Naples1.0");
      snprintf(filename, ATH3K_NAME_LEN, NPL1_0_USB_RAMPATCH_FILE);
   }
   else
   {
      BT_DBG("Chip Detected as Ath3k");
      snprintf(filename, ATH3K_NAME_LEN, "ar3k/AthrBT_0x%08x.dfu",
               fw_version.rom_version);
   }
   ret = request_firmware(&firmware, filename, &udev->dev);
   if (ret < 0)
   {
      BT_ERR("Patch file not found %s", filename);
      return ret;
   }

   if ((fw_version.rom_version == ROME2_1_USB_CHIP_VERSION) ||
         (fw_version.rom_version == ROME3_0_USB_CHIP_VERSION) ||
         (fw_version.rom_version == ROME3_2_USB_CHIP_VERSION) ||
         (fw_version.rom_version == NPL1_0_USB_CHIP_VERSION))
   {
      rome2_1_version = (struct rome2_1_version *) firmware->data;
      pt_version.rom_version = rome2_1_version->build_ver;
      pt_version.build_version = rome2_1_version->patch_ver;
      BT_DBG("pt_ver.rome_ver : 0x%x", pt_version.rom_version);
      BT_DBG("pt_ver.build_ver: 0x%x", pt_version.build_version);
      BT_DBG("fw_ver.rom_ver: 0x%x", fw_version.rom_version);
      BT_DBG("fw_ver.build_ver: 0x%x", fw_version.build_version);
   }
   else if (fw_version.rom_version == ROME1_1_USB_CHIP_VERSION)
   {
      rome1_1_version = (struct rome1_1_version *) firmware->data;
      pt_version.build_version = rome1_1_version->build_ver;
      pt_version.rom_version = rome1_1_version->patch_ver;
      BT_DBG("pt_ver.rom1.1_ver : 0x%x", pt_version.rom_version);
      BT_DBG("pt_ver.build1.1_ver: 0x%x", pt_version.build_version);
      BT_DBG("fw_ver.rom1.1_ver: 0x%x", fw_version.rom_version);
      BT_DBG("fw_ver.build1.1_ver: 0x%x", fw_version.build_version);
   }
   else
   {
      pt_version.rom_version = *(int *)(firmware->data +
                                        firmware->size - 8);
      pt_version.build_version = *(int *)
                                 (firmware->data + firmware->size - 4);
   }
   if ((pt_version.rom_version != (fw_version.rom_version & 0xffff)) ||
         (pt_version.build_version <= fw_version.build_version))
   {
      BT_ERR("Patch file version did not match with firmware, %d, %d, %d, %d", pt_version.rom_version, fw_version.rom_version, pt_version.build_version, fw_version.build_version);
      release_firmware(firmware);
      return -EINVAL;
   }

   if ((fw_version.rom_version == ROME2_1_USB_CHIP_VERSION) ||
         (fw_version.rom_version == ROME3_0_USB_CHIP_VERSION) ||
         (fw_version.rom_version == ROME3_2_USB_CHIP_VERSION) ||
         (fw_version.rom_version == NPL1_0_USB_CHIP_VERSION))
   {
      BT_ERR("%s: Loading RAMPATCH...", __func__);
      ret = ath3k_load_fwfile(udev, firmware,
                              ROME2_1_USB_RAMPATCH_HEADER);
   }
   else if (fw_version.rom_version == ROME1_1_USB_CHIP_VERSION)
      ret = ath3k_load_fwfile(udev, firmware,
                              ROME1_1_USB_RAMPATCH_HEADER);
   else
      ret = ath3k_load_fwfile(udev, firmware, FW_HDR_SIZE);

   release_firmware(firmware);

   return ret;
}

static int ath3k_load_syscfg(struct usb_device *udev)
{
   unsigned char fw_state;
   char filename[ATH3K_NAME_LEN] = {0};
   const struct firmware *firmware;
   struct ath3k_version fw_version;
   int clk_value, ret;

   ret = ath3k_get_state(udev, &fw_state);
   if (ret < 0)
   {
      BT_ERR("Can't get state to change to load configuration err");
      return -EBUSY;
   }

   if ((fw_state == ATH3K_SYSCFG_UPDATE) ||
         (fw_state == ATH3K_PATCH_SYSCFG_UPDATE))
   {
      BT_INFO("%s: NVM already downloaded(fw_state: %d)", __func__,
              fw_state);
      return 0;
   }
   else
      BT_ERR("%s: Downloading NVM(fw_state: %d)", __func__, fw_state);

   ret = ath3k_get_version(udev, &fw_version);
   if (ret < 0)
   {
      BT_ERR("Can't get version to change to load ram patch err");
      return ret;
   }

   switch (fw_version.ref_clock)
   {

      case ATH3K_XTAL_FREQ_26M:
         clk_value = 26;
         break;
      case ATH3K_XTAL_FREQ_40M:
         clk_value = 40;
         break;
      case ATH3K_XTAL_FREQ_19P2:
         clk_value = 19;
         break;
      default:
         clk_value = 0;
         break;
   }

   if (fw_version.rom_version == ROME2_1_USB_CHIP_VERSION)
      snprintf(filename, ATH3K_NAME_LEN, ROME2_1_USB_NVM_FILE);
   else if (fw_version.rom_version == ROME3_0_USB_CHIP_VERSION)
      snprintf(filename, ATH3K_NAME_LEN, ROME3_0_USB_NVM_FILE);
   else if (fw_version.rom_version == ROME3_2_USB_CHIP_VERSION)
   {
      if(fw_version.soc_version == 0x23)
         snprintf(filename, ATH3K_NAME_LEN, TF1_1_USB_NVM_FILE);
      else
         snprintf(filename, ATH3K_NAME_LEN, ROME3_0_USB_NVM_FILE);
   }
   else if (fw_version.rom_version == ROME1_1_USB_CHIP_VERSION)
      snprintf(filename, ATH3K_NAME_LEN, ROME1_1_USB_NVM_FILE);
   else if (fw_version.rom_version == NPL1_0_USB_CHIP_VERSION)
      snprintf(filename, ATH3K_NAME_LEN, NPL1_0_USB_NVM_FILE);
   else
      snprintf(filename, ATH3K_NAME_LEN, "ar3k/ramps_0x%08x_%d%s",
               fw_version.rom_version, clk_value, ".dfu");

   ret = request_firmware(&firmware, filename, &udev->dev);
   if (ret < 0)
   {
      BT_ERR("Configuration file not found %s", filename);
      return ret;
   }

   if ((fw_version.rom_version == ROME2_1_USB_CHIP_VERSION) ||
         (fw_version.rom_version == ROME3_0_USB_CHIP_VERSION) ||
         (fw_version.rom_version == ROME3_2_USB_CHIP_VERSION) ||
         (fw_version.rom_version == NPL1_0_USB_CHIP_VERSION))
   {
      BT_ERR("%s: Loading NVM...", __func__);
      ret = ath3k_load_fwfile(udev, firmware, ROME2_1_USB_NVM_HEADER);
   }
   else if (fw_version.rom_version == ROME1_1_USB_CHIP_VERSION)
      ret = ath3k_load_fwfile(udev, firmware, ROME1_1_USB_NVM_HEADER);
   else
      ret = ath3k_load_fwfile(udev, firmware, FW_HDR_SIZE);
   release_firmware(firmware);

   return ret;
}

int rome_download(struct usb_device *udev)
{
   int ret;

   BT_ERR("%s: Starting RAMPATCH DW...\n", __func__);
   ret = ath3k_load_patch(udev);
   if (ret < 0)
   {
      BT_ERR("Loading patch file failed");
      return ret;
   }
   BT_ERR("%s: Starting NVM DW...", __func__);
   ret = ath3k_load_syscfg(udev);
   if (ret < 0)
   {
      BT_ERR("Loading sysconfig file failed");
      return ret;
   }

   return ret;
}

static struct usb_device_id blacklist_table[] =
{
   { USB_DEVICE(0x0cf3, 0x3004), .driver_info = BTUSB_ATH3012 },
   { USB_DEVICE(0x0cf3, 0xe300), .driver_info = BTUSB_ATH3012 },
   { USB_DEVICE(0x0cf3, 0xe500), .driver_info = BTUSB_ATH3012 },
   { } /* Terminating entry */
};

#endif

   /* The following function is responsible for checking the device and */
   /* determining whether the device is a bluetooth usb device.  If     */
   /* success, the function will return zero or a negative error if     */
   /* incorrect.  This function also sets up the endpoints and pipes to */
   /* the usb device.                                                   */
static int USBProbe(struct usb_interface *InterfacePtr, const struct usb_device_id *DeviceID)
{
   int                        Index;
   int                        Index1;
   int                        retval = -ENXIO;
   int                        IsNewDeviceInformation;
   int                        ErrorOccurred          = 1;
   int                        ClosestMatch;
   unsigned int               Temp;
   unsigned int               InterfaceNum;
   unsigned long              flags;
   struct usb_device         *USBDevice;
   USBDeviceInformation_t    *USBDeviceInformation;
   struct usb_host_interface *HostInterface = NULL;
   const struct usb_device_id *match;

   /* Get device info and Interface Number.                             */
   USBDevice    = interface_to_usbdev(InterfacePtr);
   InterfaceNum = InterfacePtr->cur_altsetting->desc.bInterfaceNumber;

   DEBUGPRINT(KERN_ERR "%s : devnum %d Interface %d\n", __FUNCTION__, USBDevice->devnum, InterfaceNum);

   if (!DeviceID->driver_info)
   {
      match = usb_match_id(InterfacePtr, blacklist_table);
      if (match)
      {
         if (match->driver_info & BTUSB_ATH3012)
         {
            if (get_rome_version(USBDevice))
            {
               printk("Rome detected, fw dnld in-progress...");
               rome_download(USBDevice);
            }
         }
      }
   }

   /* Verify that this is a valid device and it has the interfaces that */
   /* we are interested in.                                             */
   if((USBDevice != NULL) && ((InterfaceNum == BLUETOOTH_USB_INTERFACE_0) || (InterfaceNum == BLUETOOTH_USB_INTERFACE_1)))
   {
      /* Grab the spinlock that protects the Device List.               */
      spin_lock_irqsave(&ContextInformationSpinLock, flags);

      /* Check to see if we have seen this device before.               */
      if((USBDeviceInformation = SearchDeviceInformationEntry(&ContextInformation.DeviceInformationList, USBDevice->devnum)) == NULL)
      {
         DEBUGPRINT(KERN_ERR "%s : Creating a new USBDevice\n", __FUNCTION__);

         if((USBDeviceInformation = (USBDeviceInformation_t *)kmalloc(sizeof(USBDeviceInformation_t), GFP_ATOMIC)) != NULL)
         {
            /* The Device Information Entry for this device was not     */
            /* successfully found in the list. Next lets initialize the */
            /* device information structure.                            */
            memset(USBDeviceInformation, 0, sizeof(USBDeviceInformation_t));

            USBDeviceInformation->USBDevice = USBDevice;

            /* Link the contained Buffer Information structures back    */
            /* to the owning Device Information structure for upward    */
            /* navigation.                                              */
            USBDeviceInformation->HCIBufferInformation.USBDeviceInformation = USBDeviceInformation;
            USBDeviceInformation->ACLBufferInformation.USBDeviceInformation = USBDeviceInformation;
            USBDeviceInformation->SCOBufferInformation.USBDeviceInformation = USBDeviceInformation;

            /* Initialize STALL state tracking.                         */
            atomic_set(&(USBDeviceInformation->HCIEventInStalled), 0);
            atomic_set(&(USBDeviceInformation->ACLDataInStalled), 0);
            atomic_set(&(USBDeviceInformation->ACLDataOutStalled), 0);

            /* Mark that this is a new Device Information.              */
            IsNewDeviceInformation = 1;

            /* Attempt to create an entry in the list.                  */
            if(AddDeviceInformationEntry(&ContextInformation.DeviceInformationList, USBDeviceInformation))
            {
               /* Release the Context Information lock.                 */
               spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

               /* We have an information pointer, initialize the control*/
               /* channel write information.                            */
               if(!InitializeWriteInformation(&USBDeviceInformation->HCIBufferInformation, USB_ENDPOINT_XFER_CONTROL, 8))
               {
                  /* Couldn't get HCI control data structure            */
                  /* initialized, cleanup, and don't claim the          */
                  /* interface.                                         */
                  DEBUGPRINT(KERN_ERR "%s : couldn't initialize HCI command pipe\n", __FUNCTION__);

                  /* Grab the spinlock that protects the Device List.   */
                  spin_lock_irqsave(&ContextInformationSpinLock, flags);

                  DeleteDeviceInformationEntry(&ContextInformation.DeviceInformationList, USBDevice->devnum);

                  /* Release the Context Information lock.              */
                  spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

                  kfree(USBDeviceInformation);

                  USBDeviceInformation = NULL;
               }
            }
            else
            {
               /* Release the Context Information lock.                 */
               spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

               /* Error adding device information.                      */
               kfree(USBDeviceInformation);

               USBDeviceInformation = NULL;

               DEBUGPRINT(KERN_ERR "%s : Error adding new USBDevice\n", __FUNCTION__);
            }
         }
         else
         {
            /* Release the Context Information lock.                    */
            spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

            IsNewDeviceInformation = 0;

            DEBUGPRINT(KERN_ERR "%s : Error allocating memory for new USBDevice\n", __FUNCTION__);
         }
      }
      else
      {
         /* Release the Context Information lock.                       */
         spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

         IsNewDeviceInformation = 0;
      }

      /* Verify that we have a pointer to the Device Information.       */
      if(USBDeviceInformation)
      {
         /* Make sure the device is in a known state.                   */
         usb_reset_configuration(USBDevice);

         DEBUGPRINT(KERN_ERR "%s : Checking Interface %d\n", __FUNCTION__, InterfaceNum);

         /* Switch on the interface and store either 0 or 1.            */
         switch(InterfaceNum)
         {
            case BLUETOOTH_USB_INTERFACE_0:
               DEBUGPRINT(KERN_ERR "%s : Found Interface 0\n", __FUNCTION__);

               /* Check to see if we already own this interface.        */
               if(!USBDeviceInformation->Interface0Owned)
               {
                  /* Try to initialize this interface by storing all the*/
                  /* proper information that we will need to claim this */
                  /* interface.                                         */
                  if(InitializeInterface0(InterfacePtr->cur_altsetting, USBDeviceInformation))
                  {
                     /* Flag that we have found the Async Endpoint.     */
                     USBDeviceInformation->AsyncInterfacePtr = InterfacePtr;
                     USBDeviceInformation->Interface0Owned   = 1;
                     ErrorOccurred                           = 0;

                     /* Increment the reference count for this device.  */
                     usb_get_dev(USBDeviceInformation->USBDevice);

                     /* Increment the number of interfaces owned.       */
                     USBDeviceInformation->NumInterfacesOwned++;
                  }
               }
               else
               {
                  DEBUGPRINT(KERN_ERR "%s : Error : Interface Already Owned.", __FUNCTION__);
               }
               break;
            case BLUETOOTH_USB_INTERFACE_1:
               /* Initialize the SCO Alternate Interface Requirements.  */
               memcpy(USBDeviceInformation->SCOAlternateInterfaceMapping, &SCOAlternateInterfaceRequirements, sizeof(SCOAlternateInterfaceRequirements));

               DEBUGPRINT(KERN_ERR "%s : Found Interface 1\n", __FUNCTION__);

               /* Check to see if we already own this interface.        */
               if(!USBDeviceInformation->Interface1Owned)
               {
                  /* Build the Alternate Settings Mapping. To do this   */
                  /* we need to loop through all the interfaces to      */
                  /* determine the bandwidth required on each interface.*/
                  /* From this, we will map it as closely as possible to*/
                  /* the required bandwidth we need for the specified   */
                  /* Interface Mapping.                                 */
                  for(Index1=0,USBDeviceInformation->LargestSCOMaxPacketSize=0;Index1<NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS;Index1++)
                  {
                     for(Index=0,Temp=0xFFFFFFFF,ClosestMatch=-1;Index<InterfacePtr->num_altsetting;Index++)
                     {
                        HostInterface = &(InterfacePtr->altsetting[Index]);

                        /* Attempt to calulate the largest Max Packet   */
                        /* Size.                                        */
                        if(le16_to_cpu((&HostInterface->endpoint[0].desc)->wMaxPacketSize) > USBDeviceInformation->LargestSCOMaxPacketSize)
                           USBDeviceInformation->LargestSCOMaxPacketSize = le16_to_cpu((&HostInterface->endpoint[0].desc)->wMaxPacketSize);

                        if(le16_to_cpu((&HostInterface->endpoint[0].desc)->wMaxPacketSize) == USBDeviceInformation->SCOAlternateInterfaceMapping[Index1].RequiredBandwidth)
                        {
                           USBDeviceInformation->SCOAlternateInterfaceMapping[Index1].AlternateSetting = Index;
                           break;
                        }
                        else
                        {
                           if(le16_to_cpu((&HostInterface->endpoint[0].desc)->wMaxPacketSize) > USBDeviceInformation->SCOAlternateInterfaceMapping[Index1].RequiredBandwidth)
                           {
                              if(le16_to_cpu((&HostInterface->endpoint[0].desc)->wMaxPacketSize) < Temp)
                              {
                                 Temp         = le16_to_cpu((&HostInterface->endpoint[0].desc)->wMaxPacketSize);
                                 ClosestMatch = Index;
                              }
                           }
                        }
                     }

                     if((Index == InterfacePtr->num_altsetting) && (ClosestMatch >= 0))
                        USBDeviceInformation->SCOAlternateInterfaceMapping[Index1].AlternateSetting = ClosestMatch;

                     /* If an interface was found to satisfy this       */
                     /* bandwidth requirement, go ahead and note the    */
                     /* Interface.                                      */
                     if(USBDeviceInformation->SCOAlternateInterfaceMapping[Index1].AlternateSetting)
                        USBDeviceInformation->SCOAlternateInterfaceMapping[Index1].HostInterface = HostInterface;
                  }

                  /* Try to initialize this interface by storing all the*/
                  /* proper information that we will need to claim this */
                  /* interface.                                         */
                  if(InitializeInterface1(USBDeviceInformation))
                  {
                     /* Flag that we have found the Isoch Endpoint.     */
                     USBDeviceInformation->Interface1Owned   = 1;
                     ErrorOccurred                           = 0;

                     /* Increment the reference count for this device.  */
                     usb_get_dev(USBDeviceInformation->USBDevice);

                     /* Increment the number of interfaces owned.       */
                     USBDeviceInformation->NumInterfacesOwned++;
                  }
               }
               else
               {
                  DEBUGPRINT(KERN_ERR "%s : Error : Interface Already Owned.", __FUNCTION__);
               }
               break;
            default:
               DEBUGPRINT(KERN_ERR "%s : Not Processing Interface %d\n", __FUNCTION__, InterfaceNum);
               break;
         }
      }

      /* If this is a new device and there was no errors, add a node in */
      /* /dev directory.                                                */
      if((IsNewDeviceInformation) && (!ErrorOccurred))
      {
         DEBUGPRINT(KERN_ERR "%s : We have a new device\n", __FUNCTION__);

         spin_lock_init(&(USBDeviceInformation->ReadInformationSpinLock));

         DEBUGPRINT(KERN_ERR "%s : ReadInformation SpinLock Created\n", __FUNCTION__);

         init_waitqueue_head(&(USBDeviceInformation->ReadWaitEvent));

         DEBUGPRINT(KERN_ERR "%s : ReadWait Event Created\n", __FUNCTION__);

         /* Initialize the work struct for recovering an endpoint from  */
         /* STALL status.                                               */
         INIT_WORK(&(USBDeviceInformation->ClearStallWork), ClearStall);

         /* Fill in usb_class_driver information.                       */
         USBDeviceInformation->ClassDriver.name       = SS1BTUSB_CLASS_NAME;
         USBDeviceInformation->ClassDriver.fops       = &(ContextInformation.FileOperations);
         USBDeviceInformation->ClassDriver.minor_base = SS1BTUSB_CLASS_MINOR_BASE;

         /* We can register the device now, as it is ready.             */
         retval = usb_register_dev(InterfacePtr, &(USBDeviceInformation->ClassDriver));
         if(retval)
         {
            DEBUGPRINT(KERN_ERR "%s: usb_register_dev failed %d\n", __FUNCTION__, retval);

            /* Grab the spinlock that protects the Device List.         */
            spin_lock_irqsave(&ContextInformationSpinLock, flags);

            /* Cleanup all of the information for this Device.          */
            DeleteDeviceInformationEntry(&ContextInformation.DeviceInformationList, USBDevice->devnum);

            /* Release the Context Information lock.                    */
            spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

            /* Call usb_put_dev() for each interface that is owned to   */
            /* decrement the reference count for this device.           */
            if(USBDeviceInformation->Interface0Owned)
               usb_put_dev(USBDeviceInformation->USBDevice);

            if(USBDeviceInformation->Interface1Owned)
               usb_put_dev(USBDeviceInformation->USBDevice);

            /* Free the memory allocated for this entry.                */
            FreeDeviceInformationEntry(USBDeviceInformation);

            USBDeviceInformation = NULL;

            /* Flag that there was an error.                            */
            ErrorOccurred        = 1;
         }
         else
         {
            /* Bump reference count.                                    */
            try_module_get(THIS_MODULE);

            DEBUGPRINT(KERN_ERR "%s : registered device %s interface->minor %d\n", __FUNCTION__, USBDeviceInformation->ClassDriver.name, InterfacePtr->minor);

            /* Save minor device number to easy mapping later.          */
            USBDeviceInformation->MinorDeviceNumber = InterfacePtr->minor;
         }
      }
   }

   return(ErrorOccurred?retval:0);
}

   /* The following function is called by the USB Host Stack when a USB */
   /* device is removed from the system.  This function is responsible  */
   /* for cleaning up all allocated resources associated with the       */
   /* removed device.  This function also removes the node from the     */
   /* system and detaches from the Process Manager.                     */
static void USBDisconnect(struct usb_interface *InterfacePtr)
{
   unsigned int            InterfaceNum;
   unsigned long           flags;
   struct usb_device      *USBDevice;
   USBDeviceInformation_t *USBDeviceInformationPtr;

   DEBUGPRINT(KERN_ERR "%s : enter\n", __FUNCTION__);

   /* Get the device info.                                              */
   USBDevice    = interface_to_usbdev(InterfacePtr);
   InterfaceNum = InterfacePtr->cur_altsetting->desc.bInterfaceNumber;

   DEBUGPRINT(KERN_ERR "%s : devnum %d Interface %d\n", __FUNCTION__, USBDevice->devnum, InterfaceNum);

   /* Protect the Device List.                                          */
   spin_lock_irqsave(&ContextInformationSpinLock, flags);

   /* Get the DevNum that we save with this interface pointer.  Then    */
   /* search and see if this device is still in use with our driver.    */
   USBDeviceInformationPtr = SearchDeviceInformationEntry(&ContextInformation.DeviceInformationList, USBDevice->devnum);

   /* If we have found the interface then flag it as being removed.     */
   if(USBDeviceInformationPtr)
   {
      USBDeviceInformationPtr->BeingRemoved = 1;
   }

   /* Release the Context Information lock.                             */
   spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

   /* Make sure we obtained the lock for this Context Information.      */
   if(USBDeviceInformationPtr)
   {
      DEBUGPRINT(KERN_ERR "%s : NumberOwned: %d\n", __FUNCTION__, USBDeviceInformationPtr->NumInterfacesOwned);

      /* The device is owned by a user and the device is being removed, */
      /* then signal the event that the user may be waiting on.         */
      if(USBDeviceInformationPtr->OpenedByUser)
      {
         CloseDevice(USBDeviceInformationPtr);

         wake_up(&(USBDeviceInformationPtr->ReadWaitEvent));
      }

      /* Check to see if we own the interface that is being removed.    */
      if((InterfaceNum == BLUETOOTH_USB_INTERFACE_0) && (USBDeviceInformationPtr->Interface0Owned))
      {
         /* Decrement count of interfaces owned.                        */
         USBDeviceInformationPtr->Interface0Owned = 0;
         USBDeviceInformationPtr->NumInterfacesOwned--;

         /* Decrement the device reference acquired with usb_get_dev()  */
         /* (in USBProbe which gets called for each interface) since the*/
         /* device is disconnected.                                     */
         usb_put_dev(USBDeviceInformationPtr->USBDevice);
      }
      else
      {
         if((InterfaceNum == BLUETOOTH_USB_INTERFACE_1) && (USBDeviceInformationPtr->Interface1Owned))
         {
            /* Decrement count of interfaces owned.                     */
            USBDeviceInformationPtr->Interface1Owned = 0;
            USBDeviceInformationPtr->NumInterfacesOwned--;

            /* Decrement the device reference acquired with             */
            /* usb_get_dev() (in USBProbe which gets called for each    */
            /* interface) since the device is disconnected.             */
            usb_put_dev(USBDeviceInformationPtr->USBDevice);
         }
      }

      /* If all of the interface have been released, then we need to    */
      /* unregister the device.                                         */
      if(!USBDeviceInformationPtr->NumInterfacesOwned)
      {
         DEBUGPRINT(KERN_ERR "%s : usb_deregister_dev USBDeviceInformationPtr->MinorDeviceNumber %d\n", __FUNCTION__, USBDeviceInformationPtr->MinorDeviceNumber);

         /* Protect the Device List.                                    */
         spin_lock_irqsave(&ContextInformationSpinLock, flags);

         /* Remove the Device from the list so that no more functions   */
         /* can be handled for this device..                            */
         DeleteDeviceInformationEntry(&ContextInformation.DeviceInformationList, USBDevice->devnum);

         /* Release the Context Information lock.                       */
         spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

         /* Give back our minor.                                        */
         usb_deregister_dev(USBDeviceInformationPtr->AsyncInterfacePtr, &(USBDeviceInformationPtr->ClassDriver));

         /* Free the memory allocated for this device.                  */
         FreeDeviceInformationEntry(USBDeviceInformationPtr);

         /* Decrement the use count of the module.                      */
         module_put(THIS_MODULE);
      }
   }
}

   /* The following function is responsible for opening a Device for    */
   /* Reading and/or Writing.  This function is called in response to   */
   /* the user issuing a open() function call (specifying the device    */
   /* name as the path parameter).  This function is responsible for    */
   /* starting the HCI Event and ACL Data Receive Transfers and setting */
   /* the device to the open state. This function passes in a INode     */
   /* pointer to the node an a file pointer to the file. This function  */
   /* returns zero upon successfully execution.                         */
static int USBOpen(struct inode *INode, struct file *FilePointer)
{
   int                     ret_val = -EIO;
   int                     DevNum  = MINOR(INode->i_rdev);
   unsigned long           flags;
   USBDeviceInformation_t *USBDeviceInformationPtr = NULL;

   DEBUGPRINT(KERN_ERR "%s : Enter, Inode %p FilePointer %p\n", __FUNCTION__, INode, FilePointer);

   /* Check that the parameters passed in look somewhat valid.          */
   if((INode) && (FilePointer))
   {
      spin_lock_irqsave(&ContextInformationSpinLock, flags);

      /* Get the DevNum that we save with this File pointer.  Then      */
      /* search and see if this device is still in use with our driver. */
      USBDeviceInformationPtr = SearchDeviceInformationEntryByMinorDev(&ContextInformation.DeviceInformationList, DevNum);

      /* Make sure we have somewhat valid data.                         */
      if((USBDeviceInformationPtr) && (!USBDeviceInformationPtr->BeingRemoved))
      {
         /* Check to see if the device is already open.                 */
         if(!USBDeviceInformationPtr->OpenedByUser)
         {
            /* Device has not been opened, flag it as opened.           */
            USBDeviceInformationPtr->OpenedByUser = 1;

            spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

            DEBUGPRINT(KERN_ERR "%s : Submit URBs\n", __FUNCTION__);

            StartHCIEventACL(USBDeviceInformationPtr, DevNum);

            DEBUGPRINT(KERN_ERR "%s : Possible Urb Status ENOENT -%d ECONNRESET -%d EINPROGRESS -%d EPIPE -%d EREMOTEIO -%d ENODEV -%d EINVAL -%d\n", __FUNCTION__,
                         ENOENT, ECONNRESET, EINPROGRESS, EPIPE, EREMOTEIO, ENODEV, EINVAL);

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Device has already been opened, flag an error.           */
            spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

            DEBUGPRINT(KERN_ERR "%s : Device Already opened\n", __FUNCTION__);

            ret_val = -EINVAL;
         }
      }
      else
      {
         /* Error with context, flag an error.                          */
         spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

         DEBUGPRINT(KERN_ERR "%s : Device Being Removed\n", __FUNCTION__);

         ret_val = -ENODEV;
      }
   }

   return(ret_val);
}

   /* The following function is called to close a device.  This function*/
   /* is called in response to the user issuing a close() function call.*/
   /* This function is responsible for stopping all ongoing transfers   */
   /* and setting the device open state to closed. The INode pointer is */
   /* not used. The FilePointer pointer is used to get our instance of  */
   /* USBDeviceInformation_t that is used with this close. This function*/
   /* returns zero upon successfully execution.                         */
static int USBClose(struct inode *INode, struct file *FilePointer)
{
   int                     DevNum = MINOR(INode->i_rdev);
   unsigned int            ret_val;
   unsigned long           flags;
   USBDeviceInformation_t *USBDeviceInformationPtr;

   DEBUGPRINT(KERN_ERR "%s : Enter INode %p FilePointer %p \n", __FUNCTION__, INode, FilePointer);

   /* Check that the input parameters look valid.                       */
   if((INode) && (FilePointer))
   {
      spin_lock_irqsave(&ContextInformationSpinLock, flags);

      /* Get the DevNum that we save with this File pointer.  Then      */
      /* search and see if this device is still in use with our driver. */
      USBDeviceInformationPtr = SearchDeviceInformationEntryByMinorDev(&ContextInformation.DeviceInformationList, DevNum);

      /* Note that we do not need to keep hold of the lock to flag that */
      /* the device is no longer open.                                  */
      spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

      /* Make sure we have somewhat valid data.                         */
      if((USBDeviceInformationPtr) && (!USBDeviceInformationPtr->BeingRemoved))
      {
         CloseDevice(USBDeviceInformationPtr);

         /* Flag that the device is no longer opened.                   */
         USBDeviceInformationPtr->OpenedByUser = 0;

         ret_val = 0;
      }
      else
         ret_val = -ENODEV;
   }
   else
      ret_val = -EIO;

   return(ret_val);
}


static int CopyUSBPacketFromUser(unsigned long IoctlParameter, USBPacketInformation_t *USBPacketInformation, uint8_t Compat)
{
   int                     ret_val;
   uint32_t                Buffer32;
   USBPacketInformation_t *USBPacketInformationPtr;

   DEBUGPRINT(KERN_ERR "%s : Enter\n", __FUNCTION__);

   if((IoctlParameter) && (USBPacketInformation))
   {
      USBPacketInformationPtr = (USBPacketInformation_t *)IoctlParameter;
      if(!Compat)
      {
         /* Simply copy the buffer from userspace.                      */
         ret_val = copy_from_user(USBPacketInformation, USBPacketInformationPtr, sizeof(USBPacketInformation_t));
      }
      else
      {
         if((ret_val = get_user(USBPacketInformation->PacketType, &USBPacketInformationPtr->PacketType)) == 0)
         {
            if((ret_val = get_user(USBPacketInformation->BufferSize, &USBPacketInformationPtr->BufferSize)) == 0)
            {
               /* Copy the buffer pointer out as a 32 bit pointer.            */
               if((ret_val = copy_from_user(&Buffer32, &USBPacketInformationPtr->Buffer, sizeof(Buffer32))) == 0)
               {
                  /* Now upscale the 32 bit pointer to 64 bit.                */
                  USBPacketInformation->Buffer = (char *)(uintptr_t)Buffer32;
               }
            }
         }
      }
   }
   else
      ret_val = -EINVAL;

   DEBUGPRINT(KERN_ERR "%s : Exit(%d)\n", __FUNCTION__, ret_val);

   return(ret_val);
}

   /* This routine processes Write Data IOCTL requests.  Writes can fail*/
   /* if there are no URB's available for submitting to the USB core for*/
   /* longer than 100 msec.  Otherwise, the write is submitted, and     */
   /* success is returned.  Note that the I/O hasn't really completed at*/
   /* this point.  The I/O is really complete when the write callback is*/
   /* called.  However, in order to keep a full stream of data going, it*/
   /* is better to return so a full stream of data can be kept available*/
   /* to the USB core.  Otherwise, data transfer rates will be limited  */
   /* to an artifically low rate.                                       */
static int WriteDataIoctl(USBDeviceInformation_t *USBDeviceInformationPtr, unsigned long Parameter, uint8_t Compat)
{
   int                     MaxPacketSize;
   int                     BytesToSend;
   int                     WaitOutstandingWrites;
   unsigned int            Pipe;
   unsigned int            Tries;
   unsigned int            ret_val = -EIO;
   unsigned long           flags = 0;
   URBBufferInfo_t        *URBBufferInfo;
   USBPacketInformation_t  USBPacketInformation;
   USBBufferInformation_t *USBBufferInformationPtr;
   struct usb_ctrlrequest *USBCtrlRequest;

   if(Parameter)
   {
      /* Copy the information from user space to kernel space.          */
      if(CopyUSBPacketFromUser(Parameter, &USBPacketInformation, Compat) == 0)
      {
         switch(USBPacketInformation.PacketType)
         {
            case BTUSB_PACKET_TYPE_COMMAND:
               USBCtrlRequest          = &USBDeviceInformationPtr->USBCtrlRequest;
               USBBufferInformationPtr = &USBDeviceInformationPtr->HCIBufferInformation;

               MaxPacketSize           = USBBufferInformationPtr->WriteBufferSize;
               Pipe                    = usb_sndctrlpipe(USBDeviceInformationPtr->USBDevice, 0);
               BytesToSend             = USBPacketInformation.BufferSize;

               /* Set the ret_val to the status of a successful         */
               /* submission.                                           */
               ret_val                 = 0;

               while(USBPacketInformation.BufferSize)
               {
                  /* Next get the lock used to protect the Write        */
                  /* Information                                        */
                  spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                  URBBufferInfo = (URBBufferInfo_t *)RemoveFirstListEntry(&(USBBufferInformationPtr->FreeURBBufferList));
                  for(Tries = 2; (!URBBufferInfo) && (Tries <= MAX_WRITE_BUFFERS); Tries++)
                  {
                     DEBUGPRINT(KERN_ERR "%s: (HCI) No free URBs are available. Wait until one is completed.\n", __FUNCTION__);

                     /* No free Urb's available, store current          */
                     /* outstanding for wait event test.                */
                     WaitOutstandingWrites = atomic_read(&(USBBufferInformationPtr->OutstandingWrites));

                     /* Release the lock.                               */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     /* Now wait for one to become free.                */
                     wait_event_interruptible(USBBufferInformationPtr->WriteWaitEvent, (atomic_read(&(USBBufferInformationPtr->OutstandingWrites)) < WaitOutstandingWrites));

                     spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     /* Now try again, if we don't get one now, we fail */
                     /* the I/O.                                        */
                     URBBufferInfo = (URBBufferInfo_t *)RemoveFirstListEntry(&(USBBufferInformationPtr->FreeURBBufferList));

                     DEBUGPRINT(KERN_ERR "%s: (HCI) URB, try %d: %p\n", __FUNCTION__, Tries, (URBBufferInfo)?URBBufferInfo->URB:NULL);
                  }

                  if(URBBufferInfo)
                  {
                     /* Release the lock.                               */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     /* Make sure we found a valid buffer item.         */
                     if((URBBufferInfo->URB) && (URBBufferInfo->URBBuffer))
                     {
                        BytesToSend = (USBPacketInformation.BufferSize <= BTUSB_HCI_MAX_COMMAND_LENGTH)?USBPacketInformation.BufferSize:BTUSB_HCI_MAX_COMMAND_LENGTH;

                        DEBUGPRINT(KERN_ERR "%s: (HCI) URB %p Data %p\n", __FUNCTION__, URBBufferInfo->URB, URBBufferInfo->URBBuffer);

                        /* Set the values for the USB Ctrl Request.     */
                        USBCtrlRequest->bRequestType = HCI_CTRL_REQ;
                        USBCtrlRequest->bRequest     = 0;
                        USBCtrlRequest->wIndex       = 0;
                        USBCtrlRequest->wValue       = 0;
                        USBCtrlRequest->wLength      = __cpu_to_le16(BytesToSend);

                        if(copy_from_user(URBBufferInfo->URBBuffer, USBPacketInformation.Buffer, BytesToSend) == 0)
                        {
                           /* Fill our URB with the correct information */
                           /* that we will submit.                      */
                           /* Fill in the transfer_flags for the URB.   */
                           URBBufferInfo->URB->transfer_flags = 0;

                           usb_fill_control_urb(URBBufferInfo->URB,
                                                USBDeviceInformationPtr->USBDevice,
                                                Pipe,
                                                (unsigned char *)USBCtrlRequest,
                                                URBBufferInfo->URBBuffer,
                                                BytesToSend,
                                                USBWriteCallback,
                                                (void *)URBBufferInfo);

                           /* Increment count of outstanding writes.    */
                           atomic_inc(&(USBBufferInformationPtr->OutstandingWrites));

                           DEBUGPRINT(KERN_ERR "%s: (HCI) Adding BufferInfo %p (%p,%p) to OutstandingWritesList %p (%p,%p)\n", __FUNCTION__, &(URBBufferInfo->ListEntry), URBBufferInfo->ListEntry.PrevEntry, URBBufferInfo->ListEntry.NextEntry, &(USBBufferInformationPtr->OutstandingWritesList), USBBufferInformationPtr->OutstandingWritesList.PrevEntry, USBBufferInformationPtr->OutstandingWritesList.NextEntry);

                           /* Get the lock used to protect the Write    */
                           /* Information.                              */
                           spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Push URB Information onto the outstanding */
                           /* writes list.                              */
                           AddListEntry(&(USBBufferInformationPtr->OutstandingWritesList), (ListEntry_t *)URBBufferInfo);

                           DEBUGPRINT(KERN_ERR "%s: (HCI) OutstandingWrites %d\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

                           /* Release the lock.                         */
                           spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Submit the urb to the endpoint. If        */
                           /* submission passes, return success. The    */
                           /* write callback will handle putting the URB*/
                           /* and buffers back on the free lists.       */
                           if(usb_submit_urb(URBBufferInfo->URB, GFP_KERNEL) != 0)
                           {
                              /* Indicate failure to the caller, and    */
                              /* stop trying to send.                   */
                              ret_val = -EIO;

                              DEBUGPRINT(KERN_ERR "%s: (HCI) Unable to submit URB, returning it to the free list.\n", __FUNCTION__);

                              /* Undo changes to the current Outstanding*/
                              /* Writes count.                          */
                              atomic_dec(&(USBBufferInformationPtr->OutstandingWrites));

                              /* Get the lock used to protect the Write */
                              /* Information.                           */
                              spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                              /* Pull the URB back off the Outstanding  */
                              /* Writes List.                           */
                              RemoveListEntry((ListEntry_t *)URBBufferInfo);

                              DEBUGPRINT(KERN_ERR "%s: (HCI) OutstandingWrites %d\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

                              /* Push URB Information back onto the free*/
                              /* list.                                  */
                              AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)URBBufferInfo);

                              /* Release the lock.                      */
                              spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                              break;
                           }
                           else
                           {
                              /* URB submitted successfully. Note for   */
                              /* the caller the amount of data actually */
                              /* sent.                                  */
                              USBPacketInformation.Buffer     += BytesToSend;
                              USBPacketInformation.BufferSize -= BytesToSend;
                           }
                        }
                        else
                        {
                           /* Indicate failure to the caller, and stop  */
                           /* trying to send.                           */
                           ret_val = -EIO;

                           /* Since the write failed, we won't get a    */
                           /* callback.                                 */

                           /* Get the lock used to protect the Write    */
                           /* Information.                              */
                           spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Push URB Information back onto the free   */
                           /* list.                                     */
                           AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)URBBufferInfo);

                           spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           break;
                        }
                     }
                     else
                     {
                        /* No buffer available, Indicate failure to the */
                        /* caller, and stop trying to send.             */
                        ret_val = -EIO;

                        break;
                     }
                  }
                  else
                  {
                     /* No Urb available, Indicate failure to the       */
                     /* caller, and stop trying to send.                */
                     ret_val = -EIO;

                     /* Release Spin Lock we acquired earlier.          */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     break;
                  }
               }
               break;
            case BTUSB_PACKET_TYPE_SCO:
               USBBufferInformationPtr = &USBDeviceInformationPtr->SCOBufferInformation;
               MaxPacketSize           = USBBufferInformationPtr->WriteBufferSize;
               Pipe                    = usb_sndisocpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->SCODataOutDescriptor->bEndpointAddress);
               BytesToSend             = USBPacketInformation.BufferSize;

               DEBUGPRINT(KERN_ERR "%s : (SCO) BTUSB_PACKET_TYPE_SCO sending %d\n", __FUNCTION__, BytesToSend);

               /* Set the ret_val to the status of a successful         */
               /* submission.                                           */
               ret_val                 = 0;
               while(USBPacketInformation.BufferSize)
               {
                  /* Next get the lock used to protect the Write        */
                  /* Information                                        */
                  spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                  URBBufferInfo = (URBBufferInfo_t *)RemoveFirstListEntry(&(USBBufferInformationPtr->FreeURBBufferList));
                  if(!URBBufferInfo)
                  {
                     /* No free Urb's available, store current          */
                     /* outstanding for wait event test.                */
                     WaitOutstandingWrites = atomic_read(&(USBBufferInformationPtr->OutstandingWrites));

                     /* Release the lock.                               */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     DEBUGPRINT(KERN_ERR "%s: (SCO) No URBs, Event Reset, SpinLock Released\n", __FUNCTION__);

                     /* Now wait for one to become free.                */
                     wait_event_interruptible(USBBufferInformationPtr->WriteWaitEvent, (atomic_read(&(USBBufferInformationPtr->OutstandingWrites)) < WaitOutstandingWrites));

                     DEBUGPRINT(KERN_ERR "%s: (SCO) Wait Event Complete\n", __FUNCTION__);

                     /* Next get the lock used to protect the Write     */
                     /* Information.                                    */
                     spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     /* Now try again, if we don't get one now, we fail */
                     /* the I/O.                                        */
                     URBBufferInfo = (URBBufferInfo_t *)RemoveFirstListEntry(&(USBBufferInformationPtr->FreeURBBufferList));

                     DEBUGPRINT(KERN_ERR "%s: (SCO) URB, try 2: %p\n", __FUNCTION__, (URBBufferInfo)?URBBufferInfo->URB:NULL);
                  }

                  if(URBBufferInfo)
                  {
                     /* Release the lock.                               */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     /* Make sure we found a valid buffer item.         */
                     if((URBBufferInfo->URB) && (URBBufferInfo->URBBuffer))
                     {
                        BytesToSend = (USBPacketInformation.BufferSize <= MaxPacketSize)?USBPacketInformation.BufferSize:MaxPacketSize;
                        if(copy_from_user(URBBufferInfo->URBBuffer, USBPacketInformation.Buffer, BytesToSend) ==0)
                        {
                           DEBUGPRINT(KERN_ERR "%s: (SCO) URB %p Data %p\n", __FUNCTION__, URBBufferInfo->URB, URBBufferInfo->URBBuffer);

                           /* Fill our URB will the correct information */
                           /* that we will submit.                      */
                           usb_fill_write_isoc_urb(URBBufferInfo->URB,
                                                   USBDeviceInformationPtr->USBDevice,
                                                   Pipe,
                                                   URBBufferInfo->URBBuffer,
                                                   BytesToSend,
                                                   le16_to_cpu(USBDeviceInformationPtr->SCODataOutDescriptor->wMaxPacketSize),
                                                   USBWriteCallback,
                                                   (void *)URBBufferInfo,
                                                   USBDeviceInformationPtr->SCODataOutDescriptor->bInterval);

                           /* Increment count of outstanding writes.    */
                           atomic_inc(&(USBBufferInformationPtr->OutstandingWrites));

                           /* Get the lock used to protect the Write    */
                           /* Information.                              */
                           spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Push URB Information onto the outstanding */
                           /* writes list.                              */
                           AddListEntry(&(USBBufferInformationPtr->OutstandingWritesList), (ListEntry_t *)URBBufferInfo);

                           DEBUGPRINT(KERN_ERR "%s: (SCO) OutstandingWrites %d\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

                           /* Release the lock.                         */
                           spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Submit the urb to the endpoint.  Update   */
                           /* bookkeeping variables as needed.          */
                           if(usb_submit_urb(URBBufferInfo->URB, GFP_KERNEL) != 0)
                           {
                              /* Indicate failure to the caller, and    */
                              /* stop trying to send.                   */
                              ret_val = -EIO;

                              DEBUGPRINT(KERN_ERR "%s: (SCO) Unable to submit URB, returning it to the free list.\n", __FUNCTION__);

                              /* Undo changes to the current Outstanding*/
                              /* Writes count.                          */
                              atomic_dec(&(USBBufferInformationPtr->OutstandingWrites));

                              /* Get the lock used to protect the Write */
                              /* Information.                           */
                              DEBUGPRINT(KERN_ERR "%s: (SCO) Waiting for Write Information SpinLock\n", __FUNCTION__);

                              spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                              /* Pull the URB back off the Outstanding  */
                              /* Writes List.                           */
                              RemoveListEntry((ListEntry_t *)URBBufferInfo);

                              DEBUGPRINT(KERN_ERR "%s: (SCO) OutstandingWrites %d\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

                              /* Push URB Information back onto the free*/
                              /* list.                                  */
                              AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)URBBufferInfo);

                              /* Release the lock.                      */
                              spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                              break;
                           }
                           else
                           {
                              /* URB submitted successfully. Note for   */
                              /* the caller the amount of data actually */
                              /* sent.                                  */
                              USBPacketInformation.Buffer     += BytesToSend;
                              USBPacketInformation.BufferSize -= BytesToSend;
                           }
                        }
                        else
                        {
                           /* Next get the lock used to protect the     */
                           /* Write Information.                        */
                           spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Indicate failure to the caller, and stop  */
                           /* trying to send.                           */
                           ret_val = -EIO;

                           /* Since the write failed, we won't get a    */
                           /* callback.                                 */

                           /* Push URB Information back onto the free   */
                           /* list.                                     */
                           AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)URBBufferInfo);

                           /* Release the lock.                         */
                           spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           break;
                        }
                     }
                     else
                     {
                        /* No buffer available, Indicate failure to the */
                        /* caller, and stop trying to send.             */
                        ret_val = -EIO;

                        break;
                     }
                  }
                  else
                  {
                     /* No Urb available, Indicate failure to the       */
                     /* caller, and stop trying to send.                */
                     ret_val = -EIO;

                     /* Release Spin Lock we acquired earlier.          */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     break;
                  }
               }
               break;
            case BTUSB_PACKET_TYPE_ACL:
               USBBufferInformationPtr = &USBDeviceInformationPtr->ACLBufferInformation;
               MaxPacketSize           = USBBufferInformationPtr->WriteBufferSize;
               Pipe                    = usb_sndbulkpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->ACLDataOutDescriptor->bEndpointAddress);
               BytesToSend             = USBPacketInformation.BufferSize;

               /* Set the ret_val to the status of a successful         */
               /* submission.                                           */
               ret_val                 = 0;
               while(USBPacketInformation.BufferSize)
               {
                  /* Next get the lock used to protect the Write        */
                  /* Information                                        */
                  spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                  URBBufferInfo = (URBBufferInfo_t *)RemoveFirstListEntry(&(USBBufferInformationPtr->FreeURBBufferList));
                  for(Tries = 2; (!URBBufferInfo) && (Tries <= 10); Tries++)
                  {
                     /* No free Urb's available, store current          */
                     /* outstanding for wait event test.                */
                     WaitOutstandingWrites = atomic_read(&(USBBufferInformationPtr->OutstandingWrites));

                     /* Release the lock.                               */
                     spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     DEBUGPRINT(KERN_ERR "%s: (ACL) No URBs, Event Reset, SpinLock Released\n", __FUNCTION__);

                     /* Now wait for one to become free.                */
                     wait_event_interruptible(USBBufferInformationPtr->WriteWaitEvent, (atomic_read(&(USBBufferInformationPtr->OutstandingWrites)) < WaitOutstandingWrites));

                     DEBUGPRINT(KERN_ERR "%s: (ACL) Wait Event Complete\n", __FUNCTION__);

                     spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                     /* Now try again, if we don't get one now, we fail */
                     /* the I/O.                                        */
                     URBBufferInfo = (URBBufferInfo_t *)RemoveFirstListEntry(&(USBBufferInformationPtr->FreeURBBufferList));

                     DEBUGPRINT(KERN_ERR "%s: (ACL) URB, try %d: %p\n", __FUNCTION__, Tries, (URBBufferInfo)?URBBufferInfo->URB:NULL);
                  }

                  /* Release the lock.                                  */
                  spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                  if(URBBufferInfo)
                  {
                     /* Make sure we found a valid buffer item.         */
                     if((URBBufferInfo->URB) && (URBBufferInfo->URBBuffer))
                     {
                        BytesToSend = (USBPacketInformation.BufferSize <= MaxPacketSize)?USBPacketInformation.BufferSize:MaxPacketSize;
                        if(copy_from_user(URBBufferInfo->URBBuffer, USBPacketInformation.Buffer, BytesToSend) == 0)
                        {
                           DEBUGPRINT(KERN_ERR "%s: (ACL) URB %p Data %p\n", __FUNCTION__, URBBufferInfo->URB, URBBufferInfo->URBBuffer);

                           /* Fill in the transfer_flags for the URB.   */
                           URBBufferInfo->URB->transfer_flags = 0;

                           /* Fill our URB will the correct information */
                           /* that we will submit.                      */
                           usb_fill_bulk_urb(URBBufferInfo->URB,
                                             USBDeviceInformationPtr->USBDevice,
                                             Pipe,
                                             URBBufferInfo->URBBuffer,
                                             BytesToSend,
                                             USBWriteCallback,
                                             (void *)URBBufferInfo);

                           /* Just before sending, make sure this ACL   */
                           /* bulk endpoint is not stalled. If it is,   */
                           /* wait for the stall to be cleared.         */
                           if(atomic_read(&(USBDeviceInformationPtr->ACLDataOutStalled)))
                           {
                              DEBUGPRINT(KERN_ERR "%s: (ACL) Endpoint STALLed, waiting for STALL to clear or device to close.\n", __FUNCTION__);

                              while(wait_event_interruptible(USBBufferInformationPtr->WriteWaitEvent, ((atomic_read(&(USBDeviceInformationPtr->ACLDataOutStalled)) == 0) || (USBDeviceInformationPtr->BeingRemoved))) == -ERESTARTSYS)
                              {
                                 /* Keep waiting if we get interrupted. */
                              }
                           }

                           /* Increment count of outstanding writes.    */
                           atomic_inc(&(USBBufferInformationPtr->OutstandingWrites));

                           /* Get the lock used to protect the Write    */
                           /* Information.                              */
                           spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Push URB Information onto the outstanding */
                           /* writes list.                              */
                           AddListEntry(&(USBBufferInformationPtr->OutstandingWritesList), (ListEntry_t *)URBBufferInfo);

                           DEBUGPRINT(KERN_ERR "%s: (ACL) OutstandingWrites %d\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

                           /* Release the lock.                         */
                           spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Submit the urb to the endpoint. If        */
                           /* submission passes, Block until the write  */
                           /* callback returns with a status.           */
                           if(usb_submit_urb(URBBufferInfo->URB, GFP_KERNEL) != 0)
                           {
                              /* Indicate failure to the caller, and    */
                              /* stop trying to send.                   */
                              ret_val = -EIO;

                              DEBUGPRINT(KERN_ERR "%s: (ACL) Unable to submit URB, returning it to the free list.\n", __FUNCTION__);

                              /* Undo changes to the current Outstanding*/
                              /* Writes count.                          */
                              atomic_dec(&(USBBufferInformationPtr->OutstandingWrites));

                              /* Get the lock used to protect the Write */
                              /* Information.                           */
                              spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                              /* Pull the URB back off the Outstanding  */
                              /* Writes List.                           */
                              RemoveListEntry((ListEntry_t *)URBBufferInfo);

                              DEBUGPRINT(KERN_ERR "%s: (ACL) OutstandingWrites %d\n", __FUNCTION__, atomic_read(&(USBBufferInformationPtr->OutstandingWrites)));

                              /* Push URB Information back onto the free*/
                              /* list.                                  */
                              AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)URBBufferInfo);

                              /* Release the lock.                      */
                              spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                              break;
                           }
                           else
                           {
                              /* URB submitted successfully. Note for   */
                              /* the caller the amount of data actually */
                              /* sent.                                  */
                              USBPacketInformation.Buffer     += BytesToSend;
                              USBPacketInformation.BufferSize -= BytesToSend;
                           }
                        }
                        else
                        {
                           /* Indicate failure to the caller, and stop  */
                           /* trying to send.                           */
                           ret_val = -EIO;

                           /* Since the write failed, we won't get a    */
                           /* callback.                                 */

                           /* Get the lock used to protect the Write    */
                           /* Information.                              */
                           spin_lock_irqsave(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           /* Push URB Information back onto the free   */
                           /* list.                                     */
                           AddListEntry(&(USBBufferInformationPtr->FreeURBBufferList), (ListEntry_t *)URBBufferInfo);

                           spin_unlock_irqrestore(&(USBBufferInformationPtr->WriteInformationSpinLock), flags);

                           break;
                        }
                     }
                     else
                     {
                        /* No buffer available, Indicate failure to the */
                        /* caller, and stop trying to send.             */
                        ret_val = -EIO;

                        break;
                     }
                  }
                  else
                  {
                     /* No Urb available, Indicate failure to the       */
                     /* caller, and stop trying to send.                */
                     ret_val = -EIO;

                     break;
                  }
               }
               break;
            default:
               DEBUGPRINT(KERN_ERR "%s : Invalid Packet type %d specified\n", __FUNCTION__, USBPacketInformation.PacketType);

               USBPacketInformation.BufferSize = 0;
               USBBufferInformationPtr         = NULL;

               ret_val                         = -EIO;
               break;
         }
      }
   }

   return(ret_val);
}

   /* The following function stops (kills) the URBS associated with the */
   /* HCI and ACL connection.  Used when switching the active interface */
   /* to enable or disable the SCO endpoints, and closing the device.   */
static void StopHCIEventACL(USBDeviceInformation_t *USBDeviceInformationPtr)
{
   /* First, check to see if input parameters appear to be semi-valid.  */
   if(USBDeviceInformationPtr)
   {
      /* Tell the USB core to quit using the ACL and HCI URB's for      */
      /* reading                                                        */
      usb_kill_urb(USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle);

      DEBUGPRINT(KERN_ERR "%s ACL usb_kill_urb\n", __FUNCTION__);

      usb_kill_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle);

      DEBUGPRINT(KERN_ERR "%s HCI usb_kill_urb\n", __FUNCTION__);
   }
}

   /* The following function resubmits the URB's associated with the    */
   /* HCI and ACL connection.  Used when switching the active interface */
   /* to enable or disable the SCO endpoints.                           */
static void StartHCIEventACL(USBDeviceInformation_t *USBDeviceInformationPtr, int DevNum)
{
   int          ret_val;
   unsigned int Pipe;

   /* First, check to see if input parameters appear to be semi-valid.  */
   if(USBDeviceInformationPtr)
   {
      /* Re-enable HCI and ACL read URB's.  Get the endpoint pipe       */
      /* number.                                                        */
      Pipe = usb_rcvintpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->HCIEventInDescriptor->bEndpointAddress);

      DEBUGPRINT(KERN_ERR "%s : HCI Pipe %x\n", __FUNCTION__, (unsigned int)Pipe);

      /* Clear out any flags that aren't covered by the                 */
      /* usb_fill_xxx_urb() MACRO's.                                    */
      USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle->transfer_flags = 0;
      USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle->actual_length  = 0;
      USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle->status         = 0;

      /* Fill or Set the URB ptr with the proper information.           */
      usb_fill_int_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle,
                       USBDeviceInformationPtr->USBDevice,
                       Pipe,
                       USBDeviceInformationPtr->HCIBufferInformation.ReadBuffer,
                       USBDeviceInformationPtr->HCIBufferInformation.ReadBufferSize,
                       USBReadCallback,
                       (void *)(uintptr_t)DevNum,
                       USBDeviceInformationPtr->HCIEventInDescriptor->bInterval);

      ret_val = usb_submit_urb(USBDeviceInformationPtr->HCIBufferInformation.ReadURBHandle, GFP_KERNEL);

      DEBUGPRINT(KERN_ERR "%s : Submit HCI URB ret_val %d\n", __FUNCTION__, ret_val );

      /* Resubmit the URB Get the endpoint pipe number.                 */
      Pipe = usb_rcvbulkpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->ACLDataInDescriptor->bEndpointAddress);

      DEBUGPRINT(KERN_ERR "%s : ACL Pipe %x\n", __FUNCTION__, (unsigned int)Pipe);

      /* Clear out any flags that aren't covered by the                 */
      /* usb_fill_xxx_urb() MACRO's.                                    */
      USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle->transfer_flags = 0;
      USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle->actual_length  = 0;
      USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle->status         = 0;

      /* Fill or Set the URB ptr with the proper information.           */
      usb_fill_bulk_urb(USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle,
                        USBDeviceInformationPtr->USBDevice,
                        Pipe,
                        USBDeviceInformationPtr->ACLBufferInformation.ReadBuffer,
                        USBDeviceInformationPtr->ACLBufferInformation.ReadBufferSize,
                        USBReadCallback,
                        (void *)(uintptr_t)DevNum);

      ret_val = usb_submit_urb(USBDeviceInformationPtr->ACLBufferInformation.ReadURBHandle, GFP_KERNEL);

      DEBUGPRINT(KERN_ERR "%s : Submit ACL URB ret_val %d\n", __FUNCTION__, ret_val );
   }
}


   /* The following function is called when the user issues a ioctl()   */
   /* function call.  The INode pointer is not used in this method. The */
   /* File Pointer is only used to store our USB Device Information     */
   /* pointer in order to recall this pointer in later fuctions. The    */
   /* Command stores a value that we will switch on in order to find    */
   /* out what we are supposed to be processing. Last the Parameter     */
   /* value stores a pointer address to a USB Information structure     */
   /* that will have the memory to either read from or copy to. This    */
   /* function return a value of zero if successfull or an error code   */
   /* with the error value.                                             */
static long USBIoctlImpl(struct file *FilePointer, unsigned int Command, unsigned long Parameter, uint8_t Compat)
{
   int                     BytesToRead;
   int                     DevNum;
   int                     WaitEventMask;
   int                     Index;
   long                    ret_val = -EIO;
   unsigned int            Pipe;
   unsigned long           cflags = 0;
   USBPacketInformation_t  USBPacketInformation;
   USBDeviceInformation_t *USBDeviceInformationPtr;
   USBBufferInformation_t *USBBufferInformationPtr;

   DEBUGPRINT(KERN_ERR "%s : Enter\n", __FUNCTION__);

   /* Check that the parameters passed in look somewhat valid.          */
   if((FilePointer) && (_IOC_TYPE(Command) == BTUSB_IOCTL_MAGIC))
   {
      /* Obtain the device minor number. This is equivalent to the INode*/
      /* parameter from kernel versions prior to 2.6.11.                */
#if(LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0))
      DevNum = MINOR(FilePointer->f_dentry->d_inode->i_rdev);
#else
      DevNum = MINOR(file_inode(FilePointer)->i_rdev);
#endif

      spin_lock_irqsave(&ContextInformationSpinLock, cflags);

      /* Search and see if this device is still in use with our driver. */
      USBDeviceInformationPtr = SearchDeviceInformationEntryByMinorDev(&ContextInformation.DeviceInformationList, DevNum);

      spin_unlock_irqrestore(&ContextInformationSpinLock, cflags);

      /* Next attempt to acquire the Remove Lock used to protect this   */
      /* Device Information Entry from removal.                         */
      if((USBDeviceInformationPtr) && (!USBDeviceInformationPtr->BeingRemoved))
      {
         /* Now determine what command this is so the appropriate       */
         /* operation can be taken.                                     */
         switch(_IOC_NR(Command))
         {
            case _IOC_NR(BTUSB_IOCTL_WAIT_DATA):
               DEBUGPRINT(KERN_ERR "%s : Wait Data\n", __FUNCTION__);

               /* Check to see if the pointer passed in appears valid.  */
               if(Parameter)
               {
                  /* Check to see if there are any packets in the queue.*/
                  WaitEventMask  = (USBDeviceInformationPtr->ACLBufferInformation.CircleBufferBytesAvailable)?BTUSB_WAIT_DRIVER_EVENT_ACL_DATA:0;
                  WaitEventMask |= (USBDeviceInformationPtr->SCOBufferInformation.CircleBufferBytesAvailable)?BTUSB_WAIT_DRIVER_EVENT_SCO_DATA:0;
                  WaitEventMask |= (USBDeviceInformationPtr->HCIBufferInformation.CircleBufferBytesAvailable)?BTUSB_WAIT_DRIVER_EVENT_HCI_EVENT:0;
                  if(!WaitEventMask)
                  {
                     DEBUGPRINT(KERN_ERR "%s : Sleep\n", __FUNCTION__);

                     /* There are no packets currently, so we will wait */
                     /* for one to arrive.                              */
                     if((wait_event_interruptible(USBDeviceInformationPtr->ReadWaitEvent, ((USBDeviceInformationPtr->ACLBufferInformation.CircleBufferBytesAvailable != 0) || (USBDeviceInformationPtr->SCOBufferInformation.CircleBufferBytesAvailable != 0) || (USBDeviceInformationPtr->HCIBufferInformation.CircleBufferBytesAvailable != 0) || USBDeviceInformationPtr->BeingRemoved))) == -ERESTARTSYS)
                     {
                        /* We woke due to a system event, so return an  */
                        /* error.                                       */
                        DEBUGPRINT(KERN_ERR "%s : Wakeup by SYSEVENT\n", __FUNCTION__);

                        ret_val = -ERESTARTSYS;
                        break;
                     }

                     WaitEventMask  = (USBDeviceInformationPtr->ACLBufferInformation.CircleBufferBytesAvailable)?BTUSB_WAIT_DRIVER_EVENT_ACL_DATA:0;
                     WaitEventMask |= (USBDeviceInformationPtr->SCOBufferInformation.CircleBufferBytesAvailable)?BTUSB_WAIT_DRIVER_EVENT_SCO_DATA:0;
                     WaitEventMask |= (USBDeviceInformationPtr->HCIBufferInformation.CircleBufferBytesAvailable)?BTUSB_WAIT_DRIVER_EVENT_HCI_EVENT:0;

                     DEBUGPRINT(KERN_ERR "%s : Wakeup WaitEventMask 0x%x\n", __FUNCTION__, WaitEventMask);
                  }

                  /* Return the mask to the user.                       */
                  put_user(WaitEventMask, (int *)Parameter);

                  ret_val = WaitEventMask;
               }
               break;
            case _IOC_NR(BTUSB_IOCTL_READ_PACKET):
               DEBUGPRINT(KERN_ERR "%s : Read Data\n", __FUNCTION__);

               /* Check to see if the pointer passed in appears valid.  */
               if(Parameter)
               {
                  /* Copy the information from user space to kernel     */
                  /* space.                                             */
                  if(CopyUSBPacketFromUser(Parameter, &USBPacketInformation, Compat) == 0)
                  {
                     switch(USBPacketInformation.PacketType)
                     {
                        case BTUSB_PACKET_TYPE_ACL:
                           USBBufferInformationPtr = &USBDeviceInformationPtr->ACLBufferInformation;
                           break;
                        case BTUSB_PACKET_TYPE_SCO:
                           USBBufferInformationPtr = &USBDeviceInformationPtr->SCOBufferInformation;
                           break;
                        case BTUSB_PACKET_TYPE_EVENT:
                           USBBufferInformationPtr = &USBDeviceInformationPtr->HCIBufferInformation;
                           break;
                        default:
                           DEBUGPRINT(KERN_ERR "%s : Invalid Packet type %d specified\n", __FUNCTION__, USBPacketInformation.PacketType);

                           USBBufferInformationPtr = NULL;
                           ret_val                 = -EIO;
                           break;
                     }

                     /* Verify that the Packet Type that was specified  */
                     /* is a valid packet type for this function.       */
                     if(USBBufferInformationPtr)
                     {
                        BytesToRead = USBBufferInformationPtr->CircleBufferBytesAvailable;
                        ret_val     = (USBPacketInformation.BufferSize < BytesToRead)?USBPacketInformation.BufferSize:BytesToRead;

                        /* Copy the information back to Userspace       */
                        /* memory.                                      */
                        if((USBBufferInformationPtr->CircleBufferReadIndex + ret_val) <= USBBufferInformationPtr->CircleBufferSize)
                        {
                           if(copy_to_user(USBPacketInformation.Buffer, &USBBufferInformationPtr->CircleBuffer[USBBufferInformationPtr->CircleBufferReadIndex], ret_val) != 0)
                              ret_val = -EFAULT;
                        }
                        else
                        {
                           BytesToRead = USBBufferInformationPtr->CircleBufferSize-USBBufferInformationPtr->CircleBufferReadIndex;

                           if(copy_to_user(USBPacketInformation.Buffer, &USBBufferInformationPtr->CircleBuffer[USBBufferInformationPtr->CircleBufferReadIndex], BytesToRead) != 0)
                              ret_val = -EFAULT;
                           else
                           {
                              if(copy_to_user(&USBPacketInformation.Buffer[BytesToRead], USBBufferInformationPtr->CircleBuffer, (ret_val - BytesToRead)) != 0)
                                 ret_val = BytesToRead;
                           }
                        }

                        if(ret_val > 0)
                        {
                           USBBufferInformationPtr->CircleBufferBytesAvailable -= ret_val;
                           USBBufferInformationPtr->CircleBufferReadIndex      += ret_val;
                           USBBufferInformationPtr->CircleBufferReadIndex      %= USBBufferInformationPtr->CircleBufferSize;
                        }

                        DEBUGPRINT(KERN_ERR "%s : Read %ld bytes of type %d\n", __FUNCTION__, ret_val, USBPacketInformation.PacketType);
                     }
                  }
                  else
                  {
                     /* The parameter value is NULL, so return an error.*/
                     ret_val = -EFAULT;
                  }
               }
               break;
            case _IOC_NR(BTUSB_IOCTL_SEND_PACKET):
               DEBUGPRINT(KERN_ERR "%s : Send Data\n", __FUNCTION__);

               ret_val = WriteDataIoctl(USBDeviceInformationPtr, Parameter, Compat);
               break;
            case _IOC_NR(BTUSB_IOCTL_ENABLE_SCO_DATA):
               DEBUGPRINT(KERN_ERR "BTUSB_IOCTL_ENABLE_SCO_DATA Parameter %d\n", (int)Parameter);

               /* BTUSB_IOCTL_ENABLE_SCO_DATA is the IOCTL command,     */
               /* Parameter has data that determines if the request is  */
               /* enabling SCO Data transfer.                           */
               if((Parameter >= BTUSB_SCO_DATA_NO_CHANNELS) && (Parameter <= BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_MSBC_CHANNEL))
               {
                   if(Parameter == BTUSB_SCO_DATA_NO_CHANNELS)
                   {
                      /* Only accept this if we are currently enabled.  */
                      if(USBDeviceInformationPtr->SCO_Enabled)
                      {
                         /* Flag that SCO is now disabled.              */
                         USBDeviceInformationPtr->SCO_Enabled           = 0;

                         USBDeviceInformationPtr->SCOAlternateInterface = 0;
                         USBDeviceInformationPtr->SCODataInDescriptor   = NULL;
                         USBDeviceInformationPtr->SCODataOutDescriptor  = NULL;

                         /* Tell USB subsystem to quit using this urb.  */
                         usb_kill_urb(USBDeviceInformationPtr->SCOBufferInformation.ReadURBHandle);

                         DEBUGPRINT(KERN_ERR "%s usb_kill_urb Handle %p\n", __FUNCTION__, USBDeviceInformationPtr->SCOBufferInformation.ReadURBHandle);

                         for(Index=0;Index<MAX_ISO_BUFFERS;Index++)
                         {
                            /* Now do the same with the extra urbs used */
                            /* for buffering.                           */
                            usb_kill_urb(USBDeviceInformationPtr->SCOBufferInformation.IsoExtraReadURBHandle[Index]);

                            DEBUGPRINT(KERN_ERR "%s usb_kill_urb %p\n", __FUNCTION__, USBDeviceInformationPtr->SCOBufferInformation.IsoExtraReadURBHandle[Index]);
                         }

                         /* We need to set the interface to the default */
                         /* alternate setting that does not support ISOC*/
                         /* transfers.                                  */
                         ret_val = usb_set_interface(USBDeviceInformationPtr->USBDevice, BLUETOOTH_USB_INTERFACE_1, 0);

                         DEBUGPRINT(KERN_ERR "%s usb_set_interface (disable) ret_val %ld\n", __FUNCTION__, ret_val);
                      }
                      else
                         ret_val = -EINVAL;
                   }
                   else
                   {
                      /* Only accept this if we support ISOC on an      */
                      /* interface and we are not currently enabled.    */
                      if((USBDeviceInformationPtr->LargestSCOMaxPacketSize) && (!USBDeviceInformationPtr->SCO_Enabled))
                      {
                         /* Look for a valid SCO Mapping.               */
                         for(Index=0,ret_val=0;Index<NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS;Index++)
                         {
                            if((USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].SCOEnableValue >= Parameter) && (USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].AlternateSetting))
                            {
                               /* Flag that SCO is now enabled.         */
                               USBDeviceInformationPtr->SCO_Enabled                         = 1;

                               USBDeviceInformationPtr->SCOAlternateInterface               = USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].AlternateSetting;
                               USBDeviceInformationPtr->SCODataInDescriptor                 = USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].InEndpointDescriptor;
                               USBDeviceInformationPtr->SCODataOutDescriptor                = USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].OutEndpointDescriptor;

                               /* Calculate new Read Buffer size based  */
                               /* on Maximum Packet Size.               */
                               USBDeviceInformationPtr->SCOBufferInformation.ReadBufferSize = USBDeviceInformationPtr->SCOAlternateInterfaceMapping[Index].MaxPacketSize * MAX_ISO_PACKETS;

                               /* We need to set the interface to the   */
                               /* alternate settings that has the needed*/
                               /* ISOC data size.                       */
                               ret_val = usb_set_interface(USBDeviceInformationPtr->USBDevice, BLUETOOTH_USB_INTERFACE_1, USBDeviceInformationPtr->SCOAlternateInterface);
                               DEBUGPRINT(KERN_ERR "%s usb_set_interface (enable) ret_val %ld\n", __FUNCTION__, ret_val);

                               /* Start reading from the SCO Endpoint   */
                               /* Get the pipe number that the endpoint */
                               /* is attached to.                       */
                               Pipe = usb_rcvisocpipe(USBDeviceInformationPtr->USBDevice, USBDeviceInformationPtr->SCODataInDescriptor->bEndpointAddress);

                               DEBUGPRINT(KERN_ERR "%s : SCO Pipe (%d) %08X\n", __FUNCTION__, Pipe, Pipe);

                               /* Fill or Set the URB ptr with the      */
                               /* proper information.                   */
                               usb_fill_isoc_urb(USBDeviceInformationPtr->SCOBufferInformation.ReadURBHandle,
                                                 USBDeviceInformationPtr->USBDevice,
                                                 Pipe,
                                                 USBDeviceInformationPtr->SCOBufferInformation.ReadBuffer,
                                                 USBDeviceInformationPtr->SCOBufferInformation.ReadBufferSize,
                                                 le16_to_cpu(USBDeviceInformationPtr->SCODataInDescriptor->wMaxPacketSize),
                                                 USBReadCallback,
                                                 (void *)(uintptr_t)DevNum,
                                                 USBDeviceInformationPtr->SCODataInDescriptor->bInterval);

                               /* Submit the URB for reading.           */
                               ret_val = usb_submit_urb(USBDeviceInformationPtr->SCOBufferInformation.ReadURBHandle, GFP_KERNEL);

                               DEBUGPRINT(KERN_ERR "%s usb_submit_urb ret_val %ld\n", __FUNCTION__, (long)ret_val);

                               for(Index=0;Index<MAX_ISO_BUFFERS;Index++)
                               {
                                  /* Now do the same with the extra urbs*/
                                  /* for rate buffering Fill or Set the */
                                  /* URB ptr with the proper            */
                                  /* information.                       */
                                  usb_fill_isoc_urb(USBDeviceInformationPtr->SCOBufferInformation.IsoExtraReadURBHandle[Index],
                                                    USBDeviceInformationPtr->USBDevice,
                                                    Pipe,
                                                    USBDeviceInformationPtr->SCOBufferInformation.ExtraReadBuffer[Index],
                                                    USBDeviceInformationPtr->SCOBufferInformation.ReadBufferSize,
                                                    le16_to_cpu(USBDeviceInformationPtr->SCODataInDescriptor->wMaxPacketSize),
                                                    USBReadCallback,
                                                    (void *)(uintptr_t)DevNum,
                                                    USBDeviceInformationPtr->SCODataInDescriptor->bInterval);

                                  /* Submit the URB for reading.        */
                                  ret_val = usb_submit_urb(USBDeviceInformationPtr->SCOBufferInformation.IsoExtraReadURBHandle[Index], GFP_KERNEL);

                                  DEBUGPRINT(KERN_ERR "%s usb_submit_urb ret_val %ld\n", __FUNCTION__, ret_val);
                               }

                               break;
                            }
                         }

                         if(Index == NUMBER_SUPPORTED_SCO_INTERFACE_MAPPINGS)
                            ret_val = -EINVAL;
                     }
                  }
               }
               break;
            default:
               DEBUGPRINT(KERN_ERR "%s : Invalid Ioctl\n", __FUNCTION__);
               ret_val = -ENOTTY;
               break;
         }
      }
      else
         ret_val = -ENODEV;
   }
   else
   {
      if(FilePointer)
         ret_val = -ENOTTY;
   }

   DEBUGPRINT(KERN_ERR "%s : Exit(%ld)\n", __FUNCTION__, ret_val);

   return(ret_val);
}

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
   static int USBIoctl(struct inode *INode, struct file *FilePointer, unsigned int Command, unsigned long Parameter)
#else
   static long USBIoctl(struct file *FilePointer, unsigned int Command, unsigned long Parameter)
#endif
{
   long ret_val;

   DEBUGPRINT(KERN_ERR "%s : Enter\n", __FUNCTION__);

   ret_val = USBIoctlImpl(FilePointer, Command, Parameter, 0);

   DEBUGPRINT(KERN_ERR "%s : Exit(%ld)\n", __FUNCTION__, ret_val);

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
   return((int)ret_val);
#else
   return(ret_val);
#endif
}


#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
   static int USBIoctlCompat(struct inode *INode, struct file *FilePointer, unsigned int Command, unsigned long Parameter)
#else
   static long USBIoctlCompat(struct file *FilePointer, unsigned int Command, unsigned long Parameter)
#endif
{
   long ret_val;

   DEBUGPRINT(KERN_ERR "%s : Enter\n", __FUNCTION__);

   ret_val = USBIoctlImpl(FilePointer, Command, Parameter, 1);

   DEBUGPRINT(KERN_ERR "%s : Exit(%ld)\n", __FUNCTION__, ret_val);

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
   return((int)ret_val);
#else
   return(ret_val);
#endif
}


   /* The following function is responsible for initializing the        */
   /* Bluetooth USB module and ALL data structures required by the      */
   /* module.  This function is called upon insertion of the module into*/
   /* the kernel via the system call insmod.  This function return zero */
   /* upon successful execution and a negative value on all errors.     */
int __init InitializeBTUSBModule(void)
{
   int ret_val = -EIO;

   DEBUGPRINT(KERN_ERR "%s : enter\n", __FUNCTION__);

   /* The module has not been loaded and someone has plugged in a       */
   /* Bluetooth dongle.  Initialize the lock used to guard access to the*/
   /* USB Context Information structure.                                */
   spin_lock_init(&ContextInformationSpinLock);

   /* Initialize/setup the File Operations structure.                   */
   ContextInformation.FileOperations.owner            = THIS_MODULE;
   ContextInformation.FileOperations.open             = USBOpen;
   ContextInformation.FileOperations.release          = USBClose;

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11))
   ContextInformation.FileOperations.ioctl            = USBIoctl;
#else
   ContextInformation.FileOperations.unlocked_ioctl   = USBIoctl;
   ContextInformation.FileOperations.compat_ioctl     = USBIoctlCompat;
#endif

   /* The file operations structure has been initialized, next          */
   /* initialize the USB Driver structure to register for this USB      */
   /* driver module.                                                    */
   ContextInformation.USBDriver.name       = SS1BTUSB_DRIVER_NAME;
   ContextInformation.USBDriver.probe      = USBProbe;
   ContextInformation.USBDriver.disconnect = USBDisconnect;
   ContextInformation.USBDriver.id_table   = BluetoothDeviceIDTable;

   /* Now that the USBDriver is setup, register the Driver with the the */
   /* USB Core that the kernel runs.                                    */
   if((ret_val = usb_register(&ContextInformation.USBDriver)) < 0)
   {
      DEBUGPRINT(KERN_ERR "%s : Failed to register USB driver ret_val in %d\n", __FUNCTION__ , ret_val);
   }

   DEBUGPRINT(KERN_ERR "%s : exit\n", __FUNCTION__);

   return(ret_val);
}

   /* The following function is responsible for cleaning up the         */
   /* Bluetooth USB module and all data structures declared within.     */
   /* This function is called upon removal of the module from the       */
   /* kernel via the system call rmmod.  Note that this function should */
   /* not be called when devices are owned and active.                  */
void __exit CleanupBTUSBModule(void)
{
   unsigned long flags = 0;

   DEBUGPRINT(KERN_ERR "%s : enter\n", __FUNCTION__);

   /* Make sure we obtain the lock for this Context Information.        */
   spin_lock_irqsave(&ContextInformationSpinLock, flags);

   /* Clean up all the memory that we have been using to keep track of  */
   /* all our devices.                                                  */
   FreeDeviceInformationList(&ContextInformation.DeviceInformationList);

   /* Release the Context Information lock.                             */
   spin_unlock_irqrestore(&ContextInformationSpinLock, flags);

   /* unregister the USB Device driver from the USB Core so our probe   */
   /* will not be called.                                               */
   usb_deregister(&ContextInformation.USBDriver);

   DEBUGPRINT(KERN_ERR "%s : exit\n", __FUNCTION__);
}

   /* These MACRO's are used to change the name of the init_module and  */
   /* cleanup_module function.                                          */
module_init(InitializeBTUSBModule);
module_exit(CleanupBTUSBModule);


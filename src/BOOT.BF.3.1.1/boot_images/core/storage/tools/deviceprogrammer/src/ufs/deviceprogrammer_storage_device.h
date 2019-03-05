/*==================================================================
 *
 * FILE:        deviceprogrammer_storage_device.h
 *
 * DESCRIPTION:
 *   
 *
 *        Copyright © 2008-2013 Qualcomm Technologies, Inc.
 *               All Rights Reserved.
 *               QUALCOMM Proprietary
 *==================================================================*/

/*===================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/deviceprogrammer/src/ufs/deviceprogrammer_storage_device.h#2 $ 
 *   $DateTime: 2015/07/22 10:35:57 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2014-03-03   dks     Added MMC FFU Support
 * 2013-06-03   ah      Added legal header
 * 2013-05-31   ab      Initial checkin
 */

#ifndef DEVICEPROGRAMMER_STORAGE_DEVICE_H
#define DEVICEPROGRAMMER_STORAGE_DEVICE_H

#include "deviceprogrammer_utils.h"
#include "ufs_api.h"

#define MAX_UFS_PARTITIONS 8

typedef struct {
    struct ufs_handle *partition_handles[MAX_UFS_PARTITIONS];
    struct ufs_handle *partition_config;
    int16 drivenum;
    int current_handle_index;
    int lun_to_grow;
    SIZE_T sector_size;
    SIZE_T blocks_per_alloc_unit;
    struct ufs_config_descr extras;
} storage_device_t;

extern const int BLOCK_SIZE;

boolean init_storage_device_hw(storage_device_t *storedev);
void init_storage_device_struct(storage_device_t *storedev);
char * storage_device_name (storage_device_t *storedev);
boolean storage_device_open_partition(storage_device_t *storedev, byte partition);
boolean storageDeviceClosePartition(storage_device_t *storedev);
boolean storageDeviceSetBootableStorageDrive(storage_device_t *storedev);
SIZE_T storageDeviceGetPartitionSizeSectors(storage_device_t *storedev);
SIZE_T storageDeviceGetLowerBoundSector(storage_device_t *storedev);
SIZE_T storageDeviceGetLowerBoundSector(storage_device_t *storedev);
boolean storageDeviceErase(storage_device_t *storedev);
boolean storageDeviceRead(storage_device_t *storedev,
                          byte* buffer,
                          SIZE_T sector_address,
                          SIZE_T sector_length);
boolean storageDeviceWrite(storage_device_t *storedev,
                           byte* buffer,
                           SIZE_T sector_address,
                           SIZE_T sector_length,
                           int *errno);
boolean storageDeviceFWUpdate(storage_device_t *storedev,
                              byte* fw_bin,
                              SIZE_T num_blocks,
                              int *errno);
boolean storageDeviceSetExtras(storage_device_t *storedev,
                            const char* attribute_name,
                            const char* attribute_value);
boolean storageDeviceExtrasCommit(storage_device_t *storedev,
                                  int *errno);
boolean storageDeviceGetStorageInfo(storage_device_t *storedev);
void storageDeviceGetExtras(storage_device_t *storedev);

#endif


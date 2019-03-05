/*==================================================================
 *
 * FILE:        deviceprogrammer_storage_device.c
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
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/deviceprogrammer/src/ufs/deviceprogrammer_storage_device.c#2 $ 
 *   $DateTime: 2015/07/22 10:35:57 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2014-03-03   dks     Added MMC FFU Support
 * 2013-07-15   ah      Removed warning
 * 2013-06-03   ah      Added legal header
 * 2013-05-31   ab      Initial checkin
 */

#include "deviceprogrammer_storage_device.h"
#include "deviceprogrammer_firehose.h"

#define BYTES_PER_KB 1024

void init_storage_device_struct(storage_device_t *storedev) {
    int i;

    for (i = 0; i < MAX_UFS_PARTITIONS; i++) {
        storedev->partition_handles[i] = NULL;
    }
    storedev->partition_config = NULL;
    storedev->current_handle_index = 0;
    storedev->lun_to_grow = -1;
    storedev->drivenum = 0;
    storedev->sector_size = 4096;
    storedev->blocks_per_alloc_unit = 1024;
}

char * storage_device_name(storage_device_t *storedev)
{
   char *storage_name={"UFS"};
   return storage_name;
}

static void open_handles(storage_device_t *storedev) {
    unsigned int i;
    for (i = 0; i < MAX_UFS_PARTITIONS; i++)
    {
        storedev->partition_handles[i] = ufs_open(storedev->drivenum, i);
    }
}

boolean init_storage_device_hw(storage_device_t *storedev) {
    struct ufs_info_type mem_info;

    storedev->partition_config = ufs_open(storedev->drivenum, UFS_UFS_DEVICE);
    if (storedev->partition_config != NULL && ufs_get_device_info(storedev->partition_config, &mem_info) == 0) {
        storedev->sector_size = mem_info.bMinAddrBlockSize * 512;
        storedev->blocks_per_alloc_unit = (mem_info.dSegmentSize * mem_info.bAllocationUnitSize * 512) / storedev->sector_size;

        open_handles(storedev);

        return TRUE;
    }
    else {
        return FALSE;
    }
}

boolean storage_device_open_partition(storage_device_t *storedev, byte partition) {
    if (/* partition < 0 || */
        partition >= MAX_UFS_PARTITIONS)
        return FALSE;

    storedev->current_handle_index = partition;
    return TRUE;
}

boolean storageDeviceClosePartition(storage_device_t *storedev) {
    return TRUE;
}

boolean storageDeviceSetBootableStorageDrive(storage_device_t *storedev) {
    int32_t rc = ufs_set_bootable(storedev->partition_config, storedev->current_handle_index);
    if (rc != 0) {
        return FALSE;
    }
    return TRUE;
}

boolean storageDeviceErase(storage_device_t *storedev) {
    int32 rc;

    rc = ufs_format(storedev->partition_handles[storedev->current_handle_index]);
    if (0 != rc)
        return FALSE;

    return TRUE;
}

boolean storageDeviceGetStorageInfo(storage_device_t *storedev) {
    int32 rc;
    struct ufs_info_type mem_info;

    rc = ufs_get_device_info(storedev->partition_handles[storedev->current_handle_index], &mem_info);
    if (0 != rc)
        return 0;

    logMessage("num_partition_sectors="SIZE_T_FORMAT, mem_info.dLuTotalBlocks);
    return TRUE;
}

SIZE_T storageDeviceGetPartitionSizeSectors(storage_device_t *storedev) {
    int32 rc;
    struct ufs_info_type mem_info;

    rc = ufs_get_device_info(storedev->partition_handles[storedev->current_handle_index], &mem_info);
    if (0 != rc)
        return 0;

    return mem_info.dLuTotalBlocks;
}

SIZE_T storageDeviceGetLowerBoundSector(storage_device_t *storedev) {
    if (0 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_0
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_0;
        #endif
    }
    else if (1 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_1
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_1;
        #endif
    }
    else if (2 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_2
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_2;
        #endif
    }
    else if (3 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_3
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_3;
        #endif
    }
    else if (4 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_4
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_4;
        #endif
    }
    else if (5 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_5
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_5;
        #endif
    }
    else if (6 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_6
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_6;
        #endif
    }
    else if (7 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_7
            return ALLOW_WRITING_BEGINNING_AT_SECTOR_ADDR_7;
        #endif
    }
    return 0;
}

SIZE_T storageDeviceGetUpperBoundSector(storage_device_t *storedev) {
    if (0 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_0
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_0;
        #endif
    }
    else if (1 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_1
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_1;
        #endif
    }
    else if (2 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_2
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_2;
        #endif
    }
    else if (3 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_3
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_3;
        #endif
    }
    else if (4 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_4
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_4;
        #endif
    }
    else if (5 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_5
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_5;
        #endif
    }
    else if (6 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_6
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_6;
        #endif
    }
    else if (7 == storedev->current_handle_index) {
        #ifdef ALLOW_WRITING_UP_TO_SECTOR_ADDR_7
            return ALLOW_WRITING_UP_TO_SECTOR_ADDR_7;
        #endif
    }
    return storageDeviceGetPartitionSizeSectors(storedev);
}

boolean storageDeviceRead(storage_device_t *storedev,
                          byte* buffer,
                          SIZE_T sector_address,
                          SIZE_T sector_length) {
    int32 rc;

    //FREEZE_WATCHDOG();
    rc = ufs_read (
            storedev->partition_handles[storedev->current_handle_index],
            buffer,
            sector_address,
            sector_length);

    if (rc != 0)
        return FALSE;

    return TRUE;
}

boolean storageDeviceWrite(storage_device_t *storedev,
                           byte* buffer,
                           SIZE_T sector_address,
                           SIZE_T sector_length,
                           int *errno) {
    int32 rc;

    //FREEZE_WATCHDOG();
    rc = ufs_write (
            storedev->partition_handles[storedev->current_handle_index],
            buffer,
            sector_address,
            sector_length);

    if (rc != 0) {
        if (errno != NULL)
            *errno = (int) rc;
        return FALSE;
    }

    return TRUE;
}

boolean storageDeviceFWUpdate(storage_device_t *storedev,
                              byte* fw_bin,
                              SIZE_T num_sectors,
                              int *errno)
{
    (void) storedev;
    (void) fw_bin;
    (void) num_sectors;
    (void) errno;
    logMessage ("Standard UFS UPdate process not implemented yet");
    return FALSE;
}

boolean storageDeviceSetExtras(storage_device_t *storedev,
                            const char* attribute_name,
                            const char* attribute_value) {
    static int lu_num;
    int temp;
    boolean num_conversion;

    if (strcasecmp(attribute_name, "LUNtoGrow") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->lun_to_grow = temp;
    }
    else if (strcasecmp(attribute_name, "bNumberLU") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bNumberLU = temp;
    }
    else if (strcasecmp(attribute_name, "bBootEnable") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bBootEnable = temp;
    }
    else if (strcasecmp(attribute_name, "bDescrAccessEn") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bDescrAccessEn = temp;
    }
    else if (strcasecmp(attribute_name, "bInitPowerMode") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bInitPowerMode = temp;
    }
    else if (strcasecmp(attribute_name, "bHighPriorityLUN") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bHighPriorityLUN = temp;
    }
    else if (strcasecmp(attribute_name, "bSecureRemovalType") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bSecureRemovalType = temp;
    }
    else if (strcasecmp(attribute_name, "bInitActiveICCLevel") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.bInitActiveICCLevel = temp;
    }
    else if (strcasecmp(attribute_name, "wPeriodicRTCUpdate") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.wPeriodicRTCUpdate = temp;
    }
    else if (strcasecmp(attribute_name, "LUNum") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion) {
            return FALSE;
        }
        else {
            lu_num = temp;
        }
    }
    else if (strcasecmp(attribute_name, "bLUEnable") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bLUEnable = temp;
    }
    else if (strcasecmp(attribute_name, "bBootLunID") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bBootLunID = temp;
    }
    else if (strcasecmp(attribute_name, "bLUWriteProtect") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bLUWriteProtect = temp;
    }
    else if (strcasecmp(attribute_name, "bMemoryType") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bMemoryType = temp;
    }
    else if (strcasecmp(attribute_name, "size_in_KB") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        if (temp % (storedev->sector_size / BYTES_PER_KB) != 0)
            return FALSE;
        temp = temp / (storedev->sector_size / BYTES_PER_KB); // temp is now in # of sectors
        if (temp % storedev->blocks_per_alloc_unit != 0)
            return FALSE;
        storedev->extras.unit[lu_num].dNumAllocUnits = temp / storedev->blocks_per_alloc_unit;
    }
    else if (strcasecmp(attribute_name, "bDataReliability") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bDataReliability = temp;
    }
    else if (strcasecmp(attribute_name, "bLogicalBlockSize") == 0) {
        // TODO : Add support in stringToNumber for hex conversion
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bLogicalBlockSize = temp;
    }
    else if (strcasecmp(attribute_name, "bProvisioningType") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].bProvisioningType = temp;
    }
    else if (strcasecmp(attribute_name, "wContextCapabilities") == 0) {
        temp = stringToNumber(attribute_value, &num_conversion);
        if (FALSE == num_conversion)
            return FALSE;
        else
            storedev->extras.unit[lu_num].wContextCapabilities = temp;
    }
    else {
        return FALSE;
    }
    return TRUE;
}

void storageDeviceGetExtras(storage_device_t *storedev) {
    int i;
    for (i = 0; i < MAX_UFS_PARTITIONS; i++) {
        logMessage("Alloc %d: %d", i, storedev->extras.unit[i].dNumAllocUnits);
    }
}

boolean storageDeviceExtrasCommit(storage_device_t *storedev,
                                  int *errno) {
    struct ufs_info_type mem_info;
    int32 rc = 0;
    SIZE_T alloc_units, units_to_create = 0;
    SIZE_T capacity_to_alloc_factor;
    SIZE_T enhanced1_units = 0, enhanced2_units = 0;
    SIZE_T conversion_ratio = 1;
    int i;

    if(NULL == storedev->partition_config) {
        logMessage("Opening UFS device LU failed");
        return FALSE;
    }

    ufs_set_refclk_freq(storedev->partition_config, 19200000);

    rc = ufs_get_device_info(storedev->partition_config, &mem_info);
    if (0 != rc) {
        logMessage("Could not get partition info");
        return FALSE;
    }
    capacity_to_alloc_factor = (storedev->blocks_per_alloc_unit * storedev->sector_size) / 512;
    if (mem_info.qTotalRawDeviceCapacity % capacity_to_alloc_factor != 0) {
        logMessage("Raw capacity not a multiple of alloc unit size");
        return FALSE;
    }
    alloc_units = (mem_info.qTotalRawDeviceCapacity / capacity_to_alloc_factor) ;

    units_to_create = 0;
    enhanced1_units = enhanced2_units = 0;
    for (i = 0; i < MAX_UFS_PARTITIONS && units_to_create <= alloc_units; i++) {
        if (0 == storedev->extras.unit[i].bMemoryType) {
            // Do nothing. This is just to ensure an error is not thrown
            // for this memory type
        }
        else if (3 == storedev->extras.unit[i].bMemoryType) {
            storedev->extras.unit[i].dNumAllocUnits = storedev->extras.unit[i].dNumAllocUnits * (mem_info.wEnhanced1CapAdjFac / 0x100);
            enhanced1_units += storedev->extras.unit[i].dNumAllocUnits;
        }
        else if (4 == storedev->extras.unit[i].bMemoryType) {
            storedev->extras.unit[i].dNumAllocUnits = storedev->extras.unit[i].dNumAllocUnits * (mem_info.wEnhanced2CapAdjFac / 0x100);
            enhanced2_units += storedev->extras.unit[i].dNumAllocUnits;
        }
        else {
            logMessage("Unsupported memory type %d", storedev->extras.unit[i].bMemoryType);
            return FALSE;
        }

        units_to_create += storedev->extras.unit[i].dNumAllocUnits;
    }
    if (enhanced1_units > mem_info.dEnhanced1MaxNAllocU) {
        logMessage("Size %d exceeds max enhanced1 area size %d", enhanced1_units, mem_info.dEnhanced1MaxNAllocU);
        return FALSE;
    }
    if (enhanced2_units > mem_info.dEnhanced2MaxNAllocU) {
        logMessage("Size %d exceeds max enhanced2 area size %d", enhanced2_units, mem_info.dEnhanced2MaxNAllocU);
        return FALSE;
    }
    if (units_to_create > alloc_units) {
        logMessage("Specified size %d exceeds device size %d", units_to_create, alloc_units);
        return FALSE;
    }

    if (storedev->lun_to_grow != -1) {
        if (0 == storedev->extras.unit[storedev->lun_to_grow].bMemoryType)
            conversion_ratio = 1;
        else if (3 == storedev->extras.unit[storedev->lun_to_grow].bMemoryType)
            conversion_ratio = (mem_info.wEnhanced1CapAdjFac / 0x100);
        else if (4 == storedev->extras.unit[storedev->lun_to_grow].bMemoryType)
            conversion_ratio = (mem_info.wEnhanced2CapAdjFac / 0x100);
        else if (5 == storedev->extras.unit[storedev->lun_to_grow].bMemoryType)
            conversion_ratio = (mem_info.wEnhanced3CapAdjFac / 0x100);
        else if (6 == storedev->extras.unit[storedev->lun_to_grow].bMemoryType)
            conversion_ratio = (mem_info.wEnhanced4CapAdjFac / 0x100);

        storedev->extras.unit[storedev->lun_to_grow].dNumAllocUnits += ( (alloc_units - units_to_create) / conversion_ratio );
    }

    rc = ufs_configure_device (storedev->partition_config, &(storedev->extras));
    if (rc != 0) {
        logMessage("Failed to provision UFS device. Err %d", rc);
        return FALSE;
    }
    return TRUE;
}


/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

#include "storage.h"

LOG_MODULE_REGISTER(storage, LOG_LEVEL_INF);

#define BOOT_COUNT_PATH "/lfs/boot_count"

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(lfs_data);

static struct fs_mount_t lfs_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &lfs_data,
    .storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
    .mnt_point = "/lfs",
};

static uint32_t boot_count;

int storage_init(void)
{
    struct fs_file_t file;
    int rc;

    rc = fs_mount(&lfs_mnt);
    if (rc < 0) {
        LOG_ERR("littlefs mount failed: %d", rc);
        return rc;
    }

    fs_file_t_init(&file);
    rc = fs_open(&file, BOOT_COUNT_PATH, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        return rc;
    }
    if (fs_read(&file, &boot_count, sizeof(boot_count)) != sizeof(boot_count)) {
        boot_count = 0;
    }
    boot_count++;
    fs_seek(&file, 0, FS_SEEK_SET);
    fs_write(&file, &boot_count, sizeof(boot_count));
    fs_close(&file);

    LOG_INF("boot #%u", boot_count);
    return 0;
}

uint32_t storage_boot_count(void)
{
    return boot_count;
}

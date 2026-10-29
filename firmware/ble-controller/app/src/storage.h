/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_STORAGE_H
#define ACME_STORAGE_H

#include <stdint.h>

/* Mount littlefs and increment the persistent boot counter. */
int storage_init(void);
uint32_t storage_boot_count(void);

#endif

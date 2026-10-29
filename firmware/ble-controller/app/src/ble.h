/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_BLE_H
#define ACME_BLE_H

int ble_init(void);

/* Push the current motor status to subscribed centrals. */
void ble_notify_status(void);

#endif

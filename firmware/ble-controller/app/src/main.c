/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Acme BLE motor controller: exposes a GATT service the gateway uses to
 * command a motor and stream its status.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ble.h"
#include "motor.h"
#include "storage.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("acme ble-controller %s", ACME_FW_VERSION);

    storage_init();
    motor_init();
    if (ble_init() != 0) {
        return 0;
    }

    for (;;) {
        k_sleep(K_SECONDS(1));
        ble_notify_status();
    }
    return 0;
}

/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Simulated environmental sensor. The production board reads an SHT4x over
 * I2C; under QEMU we synthesize plausible values.
 */
#include "sensor.h"

static uint32_t lcg_state = 0xACE1u;
static uint32_t seq;

static uint32_t lcg_next(void)
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state >> 16;
}

void sensor_read(sensor_sample_t *out)
{
    out->seq = ++seq;
    out->temp_centi_c = 2200 + (int32_t)(lcg_next() % 300);
    out->hum_deci_pct = 380 + lcg_next() % 80;
    out->vbat_mv = 3300 - (seq % 50);
}

/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_SENSOR_H
#define ACME_SENSOR_H

#include <stdint.h>

typedef struct {
    uint32_t seq;
    int32_t temp_centi_c;   /* 2341 == 23.41 C */
    uint32_t hum_deci_pct;  /* 412 == 41.2 %RH */
    uint32_t vbat_mv;
} sensor_sample_t;

void sensor_read(sensor_sample_t *out);

#endif

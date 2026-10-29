/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_FRAME_H
#define ACME_FRAME_H

#include <stdint.h>

typedef struct {
    char node[16];
    uint32_t seq;
    int32_t temp_centi_c;
    uint32_t hum_deci_pct;
    uint32_t vbat_mv;
} telemetry_t;

/*
 * Parse one JSON line emitted by the sensor node, verify the HMAC over the
 * embedded CBOR payload and decode the CBOR into *out. Returns 0 on success.
 */
int frame_parse_line(const char *line, telemetry_t *out);

#endif

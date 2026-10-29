/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_TELEMETRY_H
#define ACME_TELEMETRY_H

#include <stddef.h>
#include <stdint.h>

#include "sensor.h"

#define TELEMETRY_MAC_LEN 32

/* Encode a sample as CBOR. Returns encoded length, or 0 on error. */
size_t telemetry_encode_cbor(const sensor_sample_t *s, uint8_t *buf, size_t cap);

/* HMAC-SHA256 over buf using the device key. Returns 0 on success. */
int telemetry_sign(const uint8_t *buf, size_t len, uint8_t mac[TELEMETRY_MAC_LEN]);

/* Emit one JSON line on the console for the gateway to ingest. */
void telemetry_print_json(const sensor_sample_t *s, const uint8_t *cbor, size_t cbor_len,
                          const uint8_t mac[TELEMETRY_MAC_LEN]);

#endif

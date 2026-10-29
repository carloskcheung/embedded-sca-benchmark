/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_NET_H
#define ACME_NET_H

#include <stddef.h>
#include <stdint.h>

#define ACME_TELEMETRY_PORT 5683

void net_start(void);
int net_send_telemetry(const uint8_t *buf, size_t len);

#endif

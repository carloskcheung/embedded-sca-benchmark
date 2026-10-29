/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Minimal mbedTLS build: HMAC-SHA256 for telemetry authentication.
 */
#ifndef ACME_MBEDTLS_CONFIG_H
#define ACME_MBEDTLS_CONFIG_H

#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_PLATFORM_C

#endif /* ACME_MBEDTLS_CONFIG_H */

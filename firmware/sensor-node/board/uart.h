/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_UART_H
#define ACME_UART_H

#include <stddef.h>

void uart_init(void);
void uart_write(const char *buf, size_t len);

#endif

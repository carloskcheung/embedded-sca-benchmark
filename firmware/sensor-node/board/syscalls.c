/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Route newlib stdout/stderr to UART0.
 */
#include "uart.h"

int _write(int fd, const char *buf, int len)
{
    (void)fd;
    uart_write(buf, (unsigned)len);
    return len;
}

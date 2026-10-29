/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Polled driver for the CMSDK APB UART0 on MPS2 AN385.
 */
#include <stdint.h>

#include "uart.h"

#define UART0_BASE 0x40004000UL

typedef struct {
    volatile uint32_t DATA;
    volatile uint32_t STATE;
    volatile uint32_t CTRL;
    volatile uint32_t INTSTATUS;
    volatile uint32_t BAUDDIV;
} cmsdk_uart_t;

#define UART0          ((cmsdk_uart_t *)UART0_BASE)
#define UART_STATE_TXF (1u << 0)
#define UART_CTRL_TXEN (1u << 0)

void uart_init(void)
{
    UART0->BAUDDIV = 16;
    UART0->CTRL = UART_CTRL_TXEN;
}

void uart_write(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        while (UART0->STATE & UART_STATE_TXF) {
        }
        UART0->DATA = (uint32_t)buf[i];
    }
}

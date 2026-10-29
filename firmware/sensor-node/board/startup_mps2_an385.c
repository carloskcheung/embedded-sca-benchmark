/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Minimal Cortex-M3 startup for the MPS2 AN385 board.
 */
#include <stdint.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern int main(void);

/* FreeRTOS port handlers (ARM_CM3) */
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

void Reset_Handler(void);

static void Default_Handler(void)
{
    for (;;) {
    }
}

static void HardFault_Handler(void)
{
    for (;;) {
    }
}

__attribute__((section(".isr_vector"), used))
static void (*const vector_table[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,     /* NMI */
    HardFault_Handler,   /* HardFault */
    Default_Handler,     /* MemManage */
    Default_Handler,     /* BusFault */
    Default_Handler,     /* UsageFault */
    0, 0, 0, 0,
    vPortSVCHandler,     /* SVCall */
    Default_Handler,     /* DebugMon */
    0,
    xPortPendSVHandler,  /* PendSV */
    xPortSysTickHandler, /* SysTick */
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata) {
        *dst++ = *src++;
    }
    for (dst = &_sbss; dst < &_ebss; dst++) {
        *dst = 0;
    }

    main();

    for (;;) {
    }
}

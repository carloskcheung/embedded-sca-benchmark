/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Acme sensor node: samples the environment once a second, encodes the
 * sample as CBOR, authenticates it with HMAC-SHA256 and publishes it both
 * over UDP (lwIP) and as a JSON line on the console UART.
 */
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cJSON.h"

#include "net.h"
#include "sensor.h"
#include "telemetry.h"
#include "uart.h"

#define SAMPLE_PERIOD_MS 1000

static void sampler_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    sensor_sample_t sample;
    uint8_t cbor[64];
    uint8_t mac[TELEMETRY_MAC_LEN];
    size_t len;

    (void)arg;
    net_start();

    for (;;) {
        sensor_read(&sample);
        len = telemetry_encode_cbor(&sample, cbor, sizeof(cbor));
        if (len > 0 && telemetry_sign(cbor, len, mac) == 0) {
            telemetry_print_json(&sample, cbor, len, mac);
            net_send_telemetry(cbor, len);
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

int main(void)
{
    cJSON_Hooks hooks = { pvPortMalloc, vPortFree };

    uart_init();
    cJSON_InitHooks(&hooks);
    printf("# acme sensor-node %s (FreeRTOS %s)\n", ACME_FW_VERSION, tskKERNEL_VERSION_NUMBER);

    xTaskCreate(sampler_task, "sampler", 2048, NULL, 2, NULL);
    vTaskStartScheduler();

    for (;;) {
    }
}

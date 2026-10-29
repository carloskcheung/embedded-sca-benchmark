/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * UDP telemetry transport over lwIP. Under QEMU everything goes over the
 * loopback interface and a local listener task acknowledges each datagram.
 */
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"

#include "net.h"

static struct netconn *tx_conn;

static void listener_task(void *arg)
{
    struct netconn *conn;
    struct netbuf *nb;

    (void)arg;
    conn = netconn_new(NETCONN_UDP);
    netconn_bind(conn, IP_ADDR_ANY, ACME_TELEMETRY_PORT);

    for (;;) {
        if (netconn_recv(conn, &nb) == ERR_OK) {
            printf("# net: loopback rx %u bytes\n", (unsigned)netbuf_len(nb));
            netbuf_delete(nb);
        }
    }
}

void net_start(void)
{
    tcpip_init(NULL, NULL);
    tx_conn = netconn_new(NETCONN_UDP);
    xTaskCreate(listener_task, "udp_rx", 512, NULL, 3, NULL);
}

int net_send_telemetry(const uint8_t *buf, size_t len)
{
    struct netbuf *nb = netbuf_new();
    ip_addr_t dst;
    err_t err;

    if (nb == NULL) {
        return -1;
    }
    IP_ADDR4(&dst, 127, 0, 0, 1);
    netbuf_ref(nb, buf, (u16_t)len);
    err = netconn_sendto(tx_conn, nb, &dst, ACME_TELEMETRY_PORT);
    netbuf_delete(nb);
    return err == ERR_OK ? 0 : -1;
}

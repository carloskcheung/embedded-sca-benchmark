/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * lwIP configuration: IPv4 + UDP over the loopback interface. The production
 * board adds an Ethernet netif; QEMU builds only exercise loopback.
 */
#ifndef ACME_LWIPOPTS_H
#define ACME_LWIPOPTS_H

#define NO_SYS                      0
#define SYS_LIGHTWEIGHT_PROT        1
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define LWIP_UDP                    1
#define LWIP_TCP                    0
#define LWIP_ARP                    0
#define LWIP_ETHERNET               0
#define LWIP_DHCP                   0
#define LWIP_DNS                    0
#define LWIP_IGMP                   0
#define LWIP_ICMP                   1
#define LWIP_RAW                    0

#define LWIP_NETCONN                1
#define LWIP_SOCKET                 0
#define LWIP_NETIF_LOOPBACK         1
#define LWIP_HAVE_LOOPIF            1
#define LWIP_LOOPBACK_MAX_PBUFS     8

#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (16 * 1024)
#define MEMP_NUM_PBUF               16
#define MEMP_NUM_UDP_PCB            4
#define MEMP_NUM_NETCONN            4
#define PBUF_POOL_SIZE              16

#define TCPIP_THREAD_NAME           "tcpip"
#define TCPIP_THREAD_STACKSIZE      1024
#define TCPIP_THREAD_PRIO           4
#define TCPIP_MBOX_SIZE             8
#define DEFAULT_UDP_RECVMBOX_SIZE   8
#define DEFAULT_THREAD_STACKSIZE    512

#define LWIP_STATS                  0
#define LWIP_DEBUG                  0

#endif /* ACME_LWIPOPTS_H */

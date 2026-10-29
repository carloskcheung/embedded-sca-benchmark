/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_ARCH_CC_H
#define ACME_ARCH_CC_H

#include <stdio.h>
#include <stdlib.h>

#define LWIP_PLATFORM_DIAG(x)   do { printf x; } while (0)
#define LWIP_PLATFORM_ASSERT(x) do { printf("lwip assert: %s\n", x); for (;;) {} } while (0)
#define LWIP_RAND()             ((u32_t)rand())
#define LWIP_ERRNO_STDINCLUDE   1

#endif /* ACME_ARCH_CC_H */

/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_STORE_H
#define ACME_STORE_H

#include "frame.h"

int store_open(const char *path);
int store_put(const telemetry_t *t);
void store_close(void);

#endif

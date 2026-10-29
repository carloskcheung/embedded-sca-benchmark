/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_PUBLISH_H
#define ACME_PUBLISH_H

#include "frame.h"

int publish_connect(const char *host, int port, const char *topic_prefix);
int publish_telemetry(const telemetry_t *t);
void publish_disconnect(void);

#endif

/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#include <stdio.h>
#include <string.h>

#include <cjson/cJSON.h>
#include <mosquitto.h>

#include "publish.h"

static struct mosquitto *mosq;
static char prefix[128];

int publish_connect(const char *host, int port, const char *topic_prefix)
{
    snprintf(prefix, sizeof(prefix), "%s", topic_prefix);
    mosquitto_lib_init();
    mosq = mosquitto_new("acme-gatewayd", true, NULL);
    if (mosq == NULL) {
        return -1;
    }
    if (mosquitto_connect(mosq, host, port, 30) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "publish: cannot connect to %s:%d, buffering locally\n", host, port);
        return -1;
    }
    return mosquitto_loop_start(mosq) == MOSQ_ERR_SUCCESS ? 0 : -1;
}

int publish_telemetry(const telemetry_t *t)
{
    char topic[sizeof(prefix) + sizeof(t->node) + 1];
    cJSON *msg;
    char *payload;
    int rc;

    msg = cJSON_CreateObject();
    cJSON_AddNumberToObject(msg, "seq", t->seq);
    cJSON_AddNumberToObject(msg, "temp_c", t->temp_centi_c / 100.0);
    cJSON_AddNumberToObject(msg, "hum_pct", t->hum_deci_pct / 10.0);
    cJSON_AddNumberToObject(msg, "vbat_mv", t->vbat_mv);
    payload = cJSON_PrintUnformatted(msg);
    cJSON_Delete(msg);
    if (payload == NULL) {
        return -1;
    }

    snprintf(topic, sizeof(topic), "%s/%s", prefix, t->node);
    rc = mosquitto_publish(mosq, NULL, topic, (int)strlen(payload), payload, 1, false);
    cJSON_free(payload);
    return rc == MOSQ_ERR_SUCCESS ? 0 : -1;
}

void publish_disconnect(void)
{
    if (mosq != NULL) {
        mosquitto_disconnect(mosq);
        mosquitto_loop_stop(mosq, false);
        mosquitto_destroy(mosq);
    }
    mosquitto_lib_cleanup();
}

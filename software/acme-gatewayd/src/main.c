/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * acme-gatewayd: ingests authenticated telemetry from the sensor node's
 * UART, buffers it in SQLite and forwards it to the MQTT broker.
 *
 * Usage: acme-gatewayd [-c config] [-i input]   ("-i -" reads stdin)
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frame.h"
#include "publish.h"
#include "store.h"

struct config {
    char sensor_tty[128];
    char db_path[128];
    char mqtt_host[128];
    int mqtt_port;
    char mqtt_topic_prefix[128];
};

static volatile sig_atomic_t running = 1;

static void on_signal(int sig)
{
    (void)sig;
    running = 0;
}

static void load_config(const char *path, struct config *cfg)
{
    char line[256], key[64], value[128];
    FILE *f = fopen(path, "r");

    if (f == NULL) {
        return;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        if (line[0] == '#' || sscanf(line, "%63[^=]=%127s", key, value) != 2) {
            continue;
        }
        if (strcmp(key, "sensor_tty") == 0) {
            snprintf(cfg->sensor_tty, sizeof(cfg->sensor_tty), "%s", value);
        } else if (strcmp(key, "db_path") == 0) {
            snprintf(cfg->db_path, sizeof(cfg->db_path), "%s", value);
        } else if (strcmp(key, "mqtt_host") == 0) {
            snprintf(cfg->mqtt_host, sizeof(cfg->mqtt_host), "%s", value);
        } else if (strcmp(key, "mqtt_port") == 0) {
            cfg->mqtt_port = atoi(value);
        } else if (strcmp(key, "mqtt_topic_prefix") == 0) {
            snprintf(cfg->mqtt_topic_prefix, sizeof(cfg->mqtt_topic_prefix), "%s", value);
        }
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    struct config cfg = {
        .sensor_tty = "/dev/ttyAMA1",
        .db_path = "/var/lib/acme/telemetry.db",
        .mqtt_host = "localhost",
        .mqtt_port = 1883,
        .mqtt_topic_prefix = "acme/telemetry",
    };
    const char *config_path = "/etc/acme-gatewayd.conf";
    const char *input = NULL;
    char line[512];
    telemetry_t t;
    FILE *in;
    int opt;

    while ((opt = getopt(argc, argv, "c:i:")) != -1) {
        switch (opt) {
        case 'c': config_path = optarg; break;
        case 'i': input = optarg; break;
        default:
            fprintf(stderr, "usage: %s [-c config] [-i input]\n", argv[0]);
            return 2;
        }
    }
    load_config(config_path, &cfg);
    if (input == NULL) {
        input = cfg.sensor_tty;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    fprintf(stderr, "acme-gatewayd %s reading %s\n", ACME_GATEWAYD_VERSION, input);
    in = strcmp(input, "-") == 0 ? stdin : fopen(input, "r");
    if (in == NULL) {
        perror(input);
        return 1;
    }
    if (store_open(cfg.db_path) != 0) {
        return 1;
    }
    publish_connect(cfg.mqtt_host, cfg.mqtt_port, cfg.mqtt_topic_prefix);

    while (running && fgets(line, sizeof(line), in) != NULL) {
        if (frame_parse_line(line, &t) != 0) {
            continue;
        }
        store_put(&t);
        publish_telemetry(&t);
        printf("%s seq=%u temp=%.2fC hum=%.1f%% vbat=%umV\n", t.node, t.seq,
               t.temp_centi_c / 100.0, t.hum_deci_pct / 10.0, t.vbat_mv);
        fflush(stdout);
    }

    publish_disconnect();
    store_close();
    if (in != stdin) {
        fclose(in);
    }
    return 0;
}

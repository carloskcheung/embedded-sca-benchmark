/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Local telemetry buffer so readings survive uplink outages.
 */
#include <stdio.h>

#include <sqlite3.h>

#include "store.h"

static sqlite3 *db;
static sqlite3_stmt *insert;

int store_open(const char *path)
{
    const char *schema =
        "CREATE TABLE IF NOT EXISTS telemetry ("
        "  ts INTEGER DEFAULT (strftime('%s','now')),"
        "  node TEXT, seq INTEGER, temp_centi_c INTEGER,"
        "  hum_deci_pct INTEGER, vbat_mv INTEGER)";

    if (sqlite3_open(path, &db) != SQLITE_OK ||
        sqlite3_exec(db, schema, NULL, NULL, NULL) != SQLITE_OK ||
        sqlite3_prepare_v2(db,
            "INSERT INTO telemetry (node, seq, temp_centi_c, hum_deci_pct, vbat_mv) "
            "VALUES (?, ?, ?, ?, ?)", -1, &insert, NULL) != SQLITE_OK) {
        fprintf(stderr, "store: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    return 0;
}

int store_put(const telemetry_t *t)
{
    int rc;

    sqlite3_bind_text(insert, 1, t->node, -1, SQLITE_STATIC);
    sqlite3_bind_int(insert, 2, (int)t->seq);
    sqlite3_bind_int(insert, 3, t->temp_centi_c);
    sqlite3_bind_int(insert, 4, (int)t->hum_deci_pct);
    sqlite3_bind_int(insert, 5, (int)t->vbat_mv);
    rc = sqlite3_step(insert);
    sqlite3_reset(insert);
    return rc == SQLITE_DONE ? 0 : -1;
}

void store_close(void)
{
    sqlite3_finalize(insert);
    sqlite3_close(db);
}

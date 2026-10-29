/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#include <string.h>

#include <cjson/cJSON.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <tinycbor/cbor.h>

#include "frame.h"

#define MAX_CBOR 64

/* Must match the key provisioned on the sensor node. */
static const uint8_t device_key[16] = {
    0x41, 0x43, 0x4d, 0x45, 0x2d, 0x44, 0x45, 0x4d,
    0x4f, 0x2d, 0x4b, 0x45, 0x59, 0x2d, 0x30, 0x31,
};

static int from_hex(const char *hex, uint8_t *out, size_t cap)
{
    size_t n = strlen(hex);

    if (n % 2 != 0 || n / 2 > cap) {
        return -1;
    }
    for (size_t i = 0; i < n / 2; i++) {
        unsigned int byte;

        if (sscanf(hex + 2 * i, "%2x", &byte) != 1) {
            return -1;
        }
        out[i] = (uint8_t)byte;
    }
    return (int)(n / 2);
}

static int verify_mac(const uint8_t *cbor, size_t len, const char *mac_hex)
{
    uint8_t expected[32], got[32];
    unsigned int expected_len = sizeof(expected);

    if (from_hex(mac_hex, got, sizeof(got)) != (int)sizeof(got)) {
        return -1;
    }
    HMAC(EVP_sha256(), device_key, sizeof(device_key), cbor, len, expected, &expected_len);
    return CRYPTO_memcmp(expected, got, sizeof(got)) == 0 ? 0 : -1;
}

static int decode_cbor(const uint8_t *buf, size_t len, telemetry_t *out)
{
    CborParser parser;
    CborValue map, it;
    char key[8];
    size_t key_len;

    if (cbor_parser_init(buf, len, 0, &parser, &map) != CborNoError || !cbor_value_is_map(&map)) {
        return -1;
    }
    if (cbor_value_enter_container(&map, &it) != CborNoError) {
        return -1;
    }
    while (!cbor_value_at_end(&it)) {
        key_len = sizeof(key);
        if (!cbor_value_is_text_string(&it) ||
            cbor_value_copy_text_string(&it, key, &key_len, &it) != CborNoError) {
            return -1;
        }
        if (strcmp(key, "node") == 0) {
            size_t n = sizeof(out->node);
            cbor_value_copy_text_string(&it, out->node, &n, NULL);
        } else {
            int64_t v = 0;
            cbor_value_get_int64(&it, &v);
            if (strcmp(key, "seq") == 0) {
                out->seq = (uint32_t)v;
            } else if (strcmp(key, "t") == 0) {
                out->temp_centi_c = (int32_t)v;
            } else if (strcmp(key, "h") == 0) {
                out->hum_deci_pct = (uint32_t)v;
            } else if (strcmp(key, "vb") == 0) {
                out->vbat_mv = (uint32_t)v;
            }
        }
        if (cbor_value_advance(&it) != CborNoError) {
            return -1;
        }
    }
    return 0;
}

int frame_parse_line(const char *line, telemetry_t *out)
{
    uint8_t cbor[MAX_CBOR];
    cJSON *root, *cbor_hex, *mac_hex;
    int len, rc = -1;

    if (line[0] != '{') {
        return -1; /* console chatter, not telemetry */
    }
    root = cJSON_Parse(line);
    if (root == NULL) {
        return -1;
    }
    cbor_hex = cJSON_GetObjectItemCaseSensitive(root, "cbor");
    mac_hex = cJSON_GetObjectItemCaseSensitive(root, "mac");
    if (cJSON_IsString(cbor_hex) && cJSON_IsString(mac_hex)) {
        len = from_hex(cbor_hex->valuestring, cbor, sizeof(cbor));
        if (len > 0 && verify_mac(cbor, (size_t)len, mac_hex->valuestring) == 0) {
            memset(out, 0, sizeof(*out));
            rc = decode_cbor(cbor, (size_t)len, out);
        }
    }
    cJSON_Delete(root);
    return rc;
}

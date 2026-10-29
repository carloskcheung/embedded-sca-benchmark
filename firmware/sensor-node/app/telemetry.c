/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "cJSON.h"
#include "cbor.h"
#include "mbedtls/md.h"

#include "telemetry.h"

#define NODE_ID "sn-01"

/* Provisioned per-device at the factory; fixed here for the demo build. */
static const uint8_t device_key[16] = {
    0x41, 0x43, 0x4d, 0x45, 0x2d, 0x44, 0x45, 0x4d,
    0x4f, 0x2d, 0x4b, 0x45, 0x59, 0x2d, 0x30, 0x31,
};

size_t telemetry_encode_cbor(const sensor_sample_t *s, uint8_t *buf, size_t cap)
{
    CborEncoder enc, map;

    cbor_encoder_init(&enc, buf, cap, 0);
    if (cbor_encoder_create_map(&enc, &map, 5) != CborNoError) {
        return 0;
    }
    cbor_encode_text_stringz(&map, "node");
    cbor_encode_text_stringz(&map, NODE_ID);
    cbor_encode_text_stringz(&map, "seq");
    cbor_encode_uint(&map, s->seq);
    cbor_encode_text_stringz(&map, "t");
    cbor_encode_int(&map, s->temp_centi_c);
    cbor_encode_text_stringz(&map, "h");
    cbor_encode_uint(&map, s->hum_deci_pct);
    cbor_encode_text_stringz(&map, "vb");
    cbor_encode_uint(&map, s->vbat_mv);
    if (cbor_encoder_close_container_checked(&enc, &map) != CborNoError) {
        return 0;
    }
    return cbor_encoder_get_buffer_size(&enc, buf);
}

int telemetry_sign(const uint8_t *buf, size_t len, uint8_t mac[TELEMETRY_MAC_LEN])
{
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    return mbedtls_md_hmac(md, device_key, sizeof(device_key), buf, len, mac);
}

static void to_hex(const uint8_t *in, size_t len, char *out)
{
    static const char digits[] = "0123456789abcdef";

    for (size_t i = 0; i < len; i++) {
        out[2 * i] = digits[in[i] >> 4];
        out[2 * i + 1] = digits[in[i] & 0xf];
    }
    out[2 * len] = '\0';
}

void telemetry_print_json(const sensor_sample_t *s, const uint8_t *cbor, size_t cbor_len,
                          const uint8_t mac[TELEMETRY_MAC_LEN])
{
    char cbor_hex[2 * 64 + 1];
    char mac_hex[2 * TELEMETRY_MAC_LEN + 1];
    cJSON *root;
    char *line;

    if (cbor_len > 64) {
        return;
    }
    to_hex(cbor, cbor_len, cbor_hex);
    to_hex(mac, TELEMETRY_MAC_LEN, mac_hex);

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "node", NODE_ID);
    cJSON_AddNumberToObject(root, "seq", s->seq);
    cJSON_AddNumberToObject(root, "temp_c", s->temp_centi_c / 100.0);
    cJSON_AddNumberToObject(root, "hum_pct", s->hum_deci_pct / 10.0);
    cJSON_AddNumberToObject(root, "vbat_mv", s->vbat_mv);
    cJSON_AddStringToObject(root, "cbor", cbor_hex);
    cJSON_AddStringToObject(root, "mac", mac_hex);

    line = cJSON_PrintUnformatted(root);
    if (line != NULL) {
        printf("%s\n", line);
        cJSON_free(line);
    }
    cJSON_Delete(root);
}

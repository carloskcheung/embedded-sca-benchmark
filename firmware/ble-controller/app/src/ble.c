/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Acme Motor GATT service. Commands and status are nanopb-encoded protobufs
 * (see proto/motor.proto); each status frame is logged with a SHA-256 digest
 * so the gateway can correlate frames across links.
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>

#include <mbedtls/sha256.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "ble.h"
#include "motor.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

#define ACME_MOTOR_SVC_UUID \
    BT_UUID_128_ENCODE(0x6d6f746f, 0x7200, 0x4163, 0x6d65, 0x000000000001ULL)
#define ACME_MOTOR_CMD_UUID \
    BT_UUID_128_ENCODE(0x6d6f746f, 0x7200, 0x4163, 0x6d65, 0x000000000002ULL)
#define ACME_MOTOR_STATUS_UUID \
    BT_UUID_128_ENCODE(0x6d6f746f, 0x7200, 0x4163, 0x6d65, 0x000000000003ULL)

static struct bt_uuid_128 svc_uuid = BT_UUID_INIT_128(ACME_MOTOR_SVC_UUID);
static struct bt_uuid_128 cmd_uuid = BT_UUID_INIT_128(ACME_MOTOR_CMD_UUID);
static struct bt_uuid_128 status_uuid = BT_UUID_INIT_128(ACME_MOTOR_STATUS_UUID);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, ACME_MOTOR_SVC_UUID),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static size_t encode_status(uint8_t *buf, size_t cap)
{
    acme_motor_MotorStatus status = acme_motor_MotorStatus_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(buf, cap);

    motor_get_status(&status);
    if (!pb_encode(&stream, acme_motor_MotorStatus_fields, &status)) {
        LOG_ERR("status encode failed: %s", PB_GET_ERROR(&stream));
        return 0;
    }
    return stream.bytes_written;
}

static ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    acme_motor_MotorCommand cmd = acme_motor_MotorCommand_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(buf, len);

    if (offset != 0) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }
    if (!pb_decode(&stream, acme_motor_MotorCommand_fields, &cmd)) {
        LOG_WRN("bad command: %s", PB_GET_ERROR(&stream));
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }
    motor_apply(&cmd);
    return len;
}

static ssize_t read_status(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                           void *buf, uint16_t len, uint16_t offset)
{
    uint8_t frame[acme_motor_MotorStatus_size];
    size_t n = encode_status(frame, sizeof(frame));

    return bt_gatt_attr_read(conn, attr, buf, len, offset, frame, n);
}

static void status_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    LOG_INF("status notifications %s", value == BT_GATT_CCC_NOTIFY ? "on" : "off");
}

BT_GATT_SERVICE_DEFINE(acme_motor_svc,
    BT_GATT_PRIMARY_SERVICE(&svc_uuid),
    BT_GATT_CHARACTERISTIC(&cmd_uuid.uuid, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE_ENCRYPT, NULL, write_cmd, NULL),
    BT_GATT_CHARACTERISTIC(&status_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ, read_status, NULL, NULL),
    BT_GATT_CCC(status_ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

void ble_notify_status(void)
{
    uint8_t frame[acme_motor_MotorStatus_size];
    uint8_t digest[32];
    size_t n = encode_status(frame, sizeof(frame));

    if (n == 0) {
        return;
    }
    mbedtls_sha256(frame, n, digest, 0);
    LOG_DBG("status frame %zu bytes, sha256 %02x%02x%02x%02x...", n,
            digest[0], digest[1], digest[2], digest[3]);
    bt_gatt_notify(NULL, &acme_motor_svc.attrs[3], frame, n);
}

int ble_init(void)
{
    int err = bt_enable(NULL);

    if (err) {
        LOG_ERR("bt_enable failed: %d", err);
        return err;
    }
    err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        LOG_ERR("advertising failed: %d", err);
        return err;
    }
    LOG_INF("advertising as \"%s\"", CONFIG_BT_DEVICE_NAME);
    return 0;
}

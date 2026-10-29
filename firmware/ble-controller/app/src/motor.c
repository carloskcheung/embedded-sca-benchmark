/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 *
 * Simulated BLDC motor: ramps toward the commanded RPM every 100 ms. The
 * production board drives a TMC6300 through PWM; that driver is not part of
 * this demo.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "motor.h"
#include "storage.h"

LOG_MODULE_REGISTER(motor, LOG_LEVEL_INF);

#define RAMP_STEP_RPM 50
#define MAX_RPM       6000

static struct {
    bool enabled;
    uint32_t target_rpm;
    uint32_t rpm;
} motor;

static void ramp_handler(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(ramp_work, ramp_handler);

static void ramp_handler(struct k_work *work)
{
    uint32_t target = motor.enabled ? motor.target_rpm : 0;

    if (motor.rpm < target) {
        motor.rpm = MIN(motor.rpm + RAMP_STEP_RPM, target);
    } else if (motor.rpm > target) {
        motor.rpm = motor.rpm > target + RAMP_STEP_RPM ? motor.rpm - RAMP_STEP_RPM : target;
    }
    k_work_reschedule(k_work_delayable_from_work(work), K_MSEC(100));
}

void motor_init(void)
{
    k_work_reschedule(&ramp_work, K_MSEC(100));
}

void motor_apply(const acme_motor_MotorCommand *cmd)
{
    motor.enabled = cmd->enable;
    motor.target_rpm = MIN(cmd->target_rpm, MAX_RPM);
    LOG_INF("command: enable=%d target=%u rpm", motor.enabled, motor.target_rpm);
}

void motor_get_status(acme_motor_MotorStatus *status)
{
    status->state = motor.rpm > 0 ? acme_motor_MotorState_MOTOR_RUNNING
                                  : acme_motor_MotorState_MOTOR_IDLE;
    status->rpm = motor.rpm;
    status->boot_count = storage_boot_count();
    status->uptime_s = (uint32_t)(k_uptime_get() / 1000);
}

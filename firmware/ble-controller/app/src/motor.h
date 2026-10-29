/*
 * Copyright (c) 2026 Acme Industrial Systems, Inc. All rights reserved.
 * SPDX-License-Identifier: LicenseRef-Acme-Proprietary
 */
#ifndef ACME_MOTOR_H
#define ACME_MOTOR_H

#include "motor.pb.h"

void motor_init(void);
void motor_apply(const acme_motor_MotorCommand *cmd);
void motor_get_status(acme_motor_MotorStatus *status);

#endif

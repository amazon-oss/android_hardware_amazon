/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define ASP_CMD_SET_VOIP_MODE 3
#define ASP_CMD_SET_VOLUME 5

#define ASP_VOLUME_MAX 30

int asp_send_command(int code, int value);

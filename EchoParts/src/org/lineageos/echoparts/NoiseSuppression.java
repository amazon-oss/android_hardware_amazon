/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.content.Context;
import android.media.AudioManager;
import android.os.SystemProperties;

public final class NoiseSuppression {

    private static final String PARAMETER = "amazon_noise_suppression";
    private static final String PROP_ENABLED = "persist.sys.audio.noise_suppression";

    private NoiseSuppression() {}

    public static boolean isEnabled() {
        return SystemProperties.getBoolean(PROP_ENABLED, true);
    }

    public static void setEnabled(Context context, boolean enabled) {
        SystemProperties.set(PROP_ENABLED, enabled ? "1" : "0");
        apply(context);
    }

    public static void apply(Context context) {
        AudioManager audioManager = context.getSystemService(AudioManager.class);
        audioManager.setParameters(PARAMETER + "=" + (isEnabled() ? "on" : "off"));
    }
}

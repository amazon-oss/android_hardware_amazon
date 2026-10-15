/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.content.Context;
import android.media.AudioManager;
import android.os.SystemProperties;

public final class MicProcessing {

    private static final String MODE_VOICE = "voice";
    private static final String PARAMETER = "amazon_mic_processing";
    private static final String PROP_MODE = "persist.sys.audio.mic_processing";

    private MicProcessing() {}

    public static String getMode() {
        return SystemProperties.get(PROP_MODE, MODE_VOICE);
    }

    public static void setMode(Context context, String mode) {
        SystemProperties.set(PROP_MODE, mode);
        apply(context);
    }

    public static void apply(Context context) {
        AudioManager audioManager = context.getSystemService(AudioManager.class);
        audioManager.setParameters(PARAMETER + "=" + getMode());
    }
}

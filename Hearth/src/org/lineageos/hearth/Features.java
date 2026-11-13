/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth;

import android.os.SystemProperties;

public final class Features {

    private static final String PROP_LED_PATTERN = "ro.hearth.led_pattern";
    private static final String PROP_OZWPAN_IFACE = "ro.hearth.ozwpan_iface";
    private static final String PROP_USB_ROLE = "ro.hearth.usb_role";

    private Features() {
    }

    public static String getLedPattern() {
        return SystemProperties.get(PROP_LED_PATTERN);
    }

    public static boolean hasLed() {
        return !getLedPattern().isEmpty();
    }

    public static boolean hasUsbRole() {
        return !SystemProperties.get(PROP_USB_ROLE).isEmpty();
    }

    public static boolean hasWifiRemotes() {
        return !SystemProperties.get(PROP_OZWPAN_IFACE).isEmpty();
    }
}

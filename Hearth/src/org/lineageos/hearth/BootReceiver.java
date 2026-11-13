/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import org.lineageos.hearth.led.LedService;
import org.lineageos.hearth.wifi.WifiRemoteService;

public class BootReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (Features.hasLed()) {
            context.startService(new Intent(context, LedService.class));
        }
        if (Features.hasWifiRemotes()) {
            context.startService(new Intent(context, WifiRemoteService.class));
        }
    }
}

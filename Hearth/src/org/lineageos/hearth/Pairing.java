/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth;

import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;

import org.lineageos.hearth.bluetooth.PairingService;
import org.lineageos.hearth.wifi.WifiRemoteService;

public final class Pairing {

    private Pairing() {
    }

    public static void start(Context context) {
        if (Features.hasWifiRemotes()) {
            WifiManager wifi = context.getSystemService(WifiManager.class);
            if (!wifi.isWifiEnabled()) {
                wifi.setWifiEnabled(true);
            }
            context.startService(new Intent(context, WifiRemoteService.class)
                    .setAction(WifiRemoteService.ACTION_PAIR));
        }
        context.startService(new Intent(context, PairingService.class)
                .setAction(PairingService.ACTION_UPDATE)
                .putExtra(PairingService.EXTRA_OPEN_WINDOW, true));
    }
}

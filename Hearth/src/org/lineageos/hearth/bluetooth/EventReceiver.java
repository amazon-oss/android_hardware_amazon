/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothInputDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class EventReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!Remotes.isAutoPairingEnabled()) {
            return;
        }

        String action = intent.getAction();
        boolean openWindow;
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            openWindow = true;
        } else if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
            if (intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1)
                    != BluetoothAdapter.STATE_ON) {
                return;
            }
            openWindow = true;
        } else if (BluetoothInputDevice.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
            if (intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1)
                    != BluetoothProfile.STATE_DISCONNECTED) {
                return;
            }
            openWindow = true;
        } else if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(action)) {
            openWindow = false;
        } else {
            return;
        }

        context.startService(new Intent(context, PairingService.class)
                .setAction(PairingService.ACTION_UPDATE)
                .putExtra(PairingService.EXTRA_OPEN_WINDOW, openWindow));
    }
}

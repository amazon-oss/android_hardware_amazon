/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class PairingRequestReceiver extends BroadcastReceiver {

    private static final String TAG = "RemotePairing";

    private static final String DEFAULT_PIN = "0000";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!BluetoothDevice.ACTION_PAIRING_REQUEST.equals(intent.getAction())) {
            return;
        }

        BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        int variant = intent.getIntExtra(BluetoothDevice.EXTRA_PAIRING_VARIANT,
                BluetoothDevice.ERROR);
        if (device == null || !Remotes.isAutoPairingEnabled()) {
            return;
        }
        if (variant != BluetoothDevice.PAIRING_VARIANT_CONSENT
                && variant != BluetoothDevice.PAIRING_VARIANT_PIN) {
            return;
        }
        if (variant == BluetoothDevice.PAIRING_VARIANT_PIN
                && Remotes.isKeyboard(device.getBluetoothClass())) {
            return;
        }
        if (!PairingService.isTarget(device) && !isAllowed(context, device)) {
            return;
        }

        boolean replied;
        if (variant == BluetoothDevice.PAIRING_VARIANT_PIN) {
            replied = device.setPin(BluetoothDevice.convertPinToBytes(DEFAULT_PIN));
        } else {
            replied = device.setPairingConfirmation(true);
        }
        Log.i(TAG, "Accepted pairing variant " + variant + " from " + device
                + " (" + device.getName() + "): " + replied);
        if (replied && isOrderedBroadcast()) {
            abortBroadcast();
        }
    }

    private static boolean isAllowed(Context context, BluetoothDevice device) {
        if (!Remotes.isPairingCandidate(device)) {
            return false;
        }
        if (Remotes.isPairingScreenOnTop(context)) {
            return true;
        }
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        return PairingService.isWindowOpen() && adapter != null
                && !Remotes.hasConnectedInput(context) && !Remotes.hasBondedInput(adapter);
    }
}

/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.SystemProperties;
import android.provider.Settings;

public final class BluetoothAudio {

    public static final int MODE_SINK = 0;
    public static final int MODE_SOURCE = 1;

    private static final long SINK_PROFILES =
            (1 << BluetoothProfile.A2DP_SINK) | (1 << BluetoothProfile.AVRCP_CONTROLLER);
    private static final long SOURCE_PROFILES =
            (1 << BluetoothProfile.A2DP) | (1 << BluetoothProfile.AVRCP);

    private static final int COD_LOUDSPEAKER = 0x200414;
    private static final int COD_TABLET = 0x20011c;

    private static final String PROP_NEW_AVRCP = "persist.bluetooth.enablenewavrcp";

    private BluetoothAudio() {}

    private static long getDisabledProfiles(ContentResolver resolver) {
        return Settings.Global.getLong(resolver, Settings.Global.BLUETOOTH_DISABLED_PROFILES, 0);
    }

    public static int getMode(Context context) {
        long disabled = getDisabledProfiles(context.getContentResolver());
        return (disabled & SINK_PROFILES) != 0 ? MODE_SOURCE : MODE_SINK;
    }

    public static void setMode(Context context, int mode) {
        ContentResolver resolver = context.getContentResolver();
        long disabled = getDisabledProfiles(resolver) & ~(SINK_PROFILES | SOURCE_PROFILES);
        boolean source = mode == MODE_SOURCE;

        Settings.Global.putLong(resolver, Settings.Global.BLUETOOTH_DISABLED_PROFILES,
                disabled | (source ? SINK_PROFILES : SOURCE_PROFILES));
        Settings.Global.putInt(resolver, Settings.Global.BLUETOOTH_CLASS_OF_DEVICE,
                source ? COD_TABLET : COD_LOUDSPEAKER);
        SystemProperties.set(PROP_NEW_AVRCP, source ? "true" : "false");

        restartBluetooth(context.getApplicationContext());
    }

    public static void ensureSingleMode(Context context) {
        long disabled = getDisabledProfiles(context.getContentResolver());
        if ((disabled & (SINK_PROFILES | SOURCE_PROFILES)) == 0) {
            setMode(context, MODE_SINK);
        }
    }

    private static void restartBluetooth(Context context) {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter == null || !adapter.isEnabled()) {
            return;
        }

        context.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1);
                if (state == BluetoothAdapter.STATE_OFF) {
                    context.unregisterReceiver(this);
                    adapter.enable();
                }
            }
        }, new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED));
        adapter.disable();
    }
}

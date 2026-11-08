/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.led;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.util.Log;

import java.io.FileOutputStream;
import java.io.IOException;

import org.lineageos.hearth.Features;
import org.lineageos.hearth.SettingsActivity;
import org.lineageos.hearth.wifi.WifiRemoteService;

public class LedService extends Service
        implements SharedPreferences.OnSharedPreferenceChangeListener {

    private static final String TAG = "LedService";

    private static final int PATTERN_OFF = 0;
    private static final int PATTERN_WHITE_BLINK = 1;
    private static final int PATTERN_WHITE_BREATHE = 5;
    private static final int PATTERN_AMBER_BREATHE = 6;

    private static final long BLUETOOTH_TIMEOUT_MS = 60 * 1000;
    private static final long INTERNET_GRACE_MS = 30 * 1000;

    private final Handler mHandler = new Handler();

    private ConnectivityManager mConnectivityManager;
    private SharedPreferences mPrefs;

    private boolean mStandby;
    private boolean mBonding;
    private boolean mRemotePairing;
    private boolean mDiscovering;
    private boolean mNoInternet;
    private int mPattern = -1;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        mPrefs.registerOnSharedPreferenceChangeListener(this);

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        filter.addAction(WifiRemoteService.ACTION_PAIRING_CHANGED);
        registerReceiver(mReceiver, filter);

        mConnectivityManager = getSystemService(ConnectivityManager.class);
        mConnectivityManager.registerDefaultNetworkCallback(mNetworkCallback);
        mHandler.postDelayed(mInternetCheck, INTERNET_GRACE_MS);

        update();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        mHandler.removeCallbacksAndMessages(null);
        mConnectivityManager.unregisterNetworkCallback(mNetworkCallback);
        unregisterReceiver(mReceiver);
        mPrefs.unregisterOnSharedPreferenceChangeListener(this);
        super.onDestroy();
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
        if (SettingsActivity.KEY_STATUS_LED.equals(key)) {
            update();
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()) {
                case Intent.ACTION_SCREEN_OFF:
                    mStandby = true;
                    break;
                case Intent.ACTION_SCREEN_ON:
                    mStandby = false;
                    break;
                case BluetoothAdapter.ACTION_DISCOVERY_STARTED:
                    mDiscovering = true;
                    mHandler.removeCallbacks(mDiscoveryTimeout);
                    mHandler.postDelayed(mDiscoveryTimeout, BLUETOOTH_TIMEOUT_MS);
                    break;
                case BluetoothAdapter.ACTION_DISCOVERY_FINISHED:
                    mDiscovering = false;
                    break;
                case BluetoothDevice.ACTION_BOND_STATE_CHANGED:
                    mBonding = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                            BluetoothDevice.BOND_NONE) == BluetoothDevice.BOND_BONDING;
                    mHandler.removeCallbacks(mBondTimeout);
                    if (mBonding) {
                        mHandler.postDelayed(mBondTimeout, BLUETOOTH_TIMEOUT_MS);
                    }
                    break;
                case WifiRemoteService.ACTION_PAIRING_CHANGED:
                    mRemotePairing = intent.getBooleanExtra(WifiRemoteService.EXTRA_PAIRING,
                            false);
                    break;
            }
            update();
        }
    };

    private final ConnectivityManager.NetworkCallback mNetworkCallback =
            new ConnectivityManager.NetworkCallback() {
        @Override
        public void onCapabilitiesChanged(Network network, NetworkCapabilities caps) {
            mHandler.post(mInternetCheck);
        }

        @Override
        public void onLost(Network network) {
            mHandler.post(mInternetCheck);
        }
    };

    private final Runnable mInternetCheck = new Runnable() {
        @Override
        public void run() {
            Network network = mConnectivityManager.getActiveNetwork();
            NetworkCapabilities caps = network != null
                    ? mConnectivityManager.getNetworkCapabilities(network) : null;
            mNoInternet = caps == null
                    || !caps.hasCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED);
            update();
        }
    };

    private final Runnable mDiscoveryTimeout = new Runnable() {
        @Override
        public void run() {
            mDiscovering = false;
            update();
        }
    };

    private final Runnable mBondTimeout = new Runnable() {
        @Override
        public void run() {
            mBonding = false;
            update();
        }
    };

    private void update() {
        int pattern = PATTERN_OFF;
        if (!mPrefs.getBoolean(SettingsActivity.KEY_STATUS_LED, true) || mStandby) {
            pattern = PATTERN_OFF;
        } else if (mBonding || mRemotePairing) {
            pattern = PATTERN_WHITE_BLINK;
        } else if (mDiscovering) {
            pattern = PATTERN_WHITE_BREATHE;
        } else if (mNoInternet) {
            pattern = PATTERN_AMBER_BREATHE;
        }

        if (pattern == mPattern) {
            return;
        }
        mPattern = pattern;

        String path = Features.getLedPattern();
        try (FileOutputStream out = new FileOutputStream(path)) {
            out.write(Integer.toString(pattern).getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Failed to write " + path, e);
        }
    }
}

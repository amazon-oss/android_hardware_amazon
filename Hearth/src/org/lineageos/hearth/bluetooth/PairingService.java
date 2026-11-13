/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.bluetooth;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothInputDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.input.InputManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

import org.lineageos.hearth.R;

public class PairingService extends Service {

    private static final String TAG = "RemotePairing";

    public static final String ACTION_UPDATE = "org.lineageos.hearth.bluetooth.action.UPDATE";
    public static final String EXTRA_OPEN_WINDOW = "open_window";

    private static final int MODE_IDLE = 0;
    private static final int MODE_ASSIST = 1;
    private static final int MODE_RECOVER = 2;
    private static final int MODE_RESCUE = 3;

    private static final long WINDOW_MS = 10 * 60 * 1000;
    private static final long POLL_MS = 3 * 1000;
    private static final long SCAN_RESTART_MS = 10 * 60 * 1000;
    private static final long DISCOVERY_GAP_MS = 5 * 1000;
    private static final long BOND_TIMEOUT_MS = 30 * 1000;
    private static final long CONNECT_TIMEOUT_MS = 15 * 1000;
    private static final long RETRY_DELAY_MS = 15 * 1000;
    private static final long DISMISS_DELAY_MS = 2 * 1000;
    private static final int CONNECT_ATTEMPTS = 2;
    private static final int MIN_RSSI = -75;

    private static volatile String sTarget;
    private static volatile long sWindowEnd;

    private final Handler mHandler = new Handler();
    private final HashMap<String, Long> mRetryAfter = new HashMap<>();
    private final HashSet<String> mSeen = new HashSet<>();
    private final HashSet<String> mUnbonding = new HashSet<>();

    private BluetoothAdapter mAdapter;
    private BluetoothLeScanner mScanner;
    private BluetoothInputDevice mInput;
    private InputManager mInputManager;

    private int mMode = MODE_IDLE;
    private int mScanMode = -1;
    private long mScanStarted;
    private boolean mDiscovering;
    private long mNextDiscovery;
    private boolean mDismissPending;

    private BluetoothDevice mTarget;
    private String mTargetLabel;
    private boolean mTargetFromScreen;
    private boolean mBonded;
    private int mConnectAttempts;

    static boolean isTarget(BluetoothDevice device) {
        String target = sTarget;
        return target != null && device != null && target.equals(device.getAddress());
    }

    static boolean isWindowOpen() {
        return SystemClock.elapsedRealtime() < sWindowEnd;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mInputManager = getSystemService(InputManager.class);
        mInputManager.registerInputDeviceListener(mInputListener, mHandler);

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        filter.addAction(BluetoothInputDevice.ACTION_CONNECTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        if (mAdapter != null) {
            mAdapter.getProfileProxy(this, mProfileListener, BluetoothProfile.INPUT_DEVICE);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && intent.getBooleanExtra(EXTRA_OPEN_WINDOW, false)) {
            sWindowEnd = SystemClock.elapsedRealtime() + WINDOW_MS;
        }
        update();
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        mHandler.removeCallbacksAndMessages(null);
        stopScan();
        stopDiscovery();
        unregisterReceiver(mReceiver);
        mInputManager.unregisterInputDeviceListener(mInputListener);
        if (mAdapter != null && mInput != null) {
            mAdapter.closeProfileProxy(BluetoothProfile.INPUT_DEVICE, mInput);
        }
        sTarget = null;
        super.onDestroy();
    }

    // State

    private int computeMode() {
        if (mAdapter == null || !Remotes.isAutoPairingEnabled()) {
            return MODE_IDLE;
        }
        if (Remotes.isPairingScreenOnTop(this)) {
            return MODE_ASSIST;
        }
        if (!isWindowOpen() || Remotes.hasConnectedInput(this)) {
            return MODE_IDLE;
        }
        return Remotes.hasBondedInput(mAdapter) ? MODE_RECOVER : MODE_RESCUE;
    }

    private void update() {
        mHandler.removeCallbacks(mUpdateRunnable);
        if (mTarget != null) {
            return;
        }

        int mode = computeMode();
        if (mode != mMode) {
            Log.i(TAG, "Mode " + modeName(mMode) + " -> " + modeName(mode));
            mMode = mode;
            mSeen.clear();
            mUnbonding.clear();
        }

        if (mode == MODE_IDLE) {
            stopScan();
            stopDiscovery();
            if (!mDismissPending) {
                stopSelf();
            }
            return;
        }

        mHandler.postDelayed(mUpdateRunnable, POLL_MS);

        if (!mAdapter.isEnabled()) {
            stopScan();
            mDiscovering = false;
            if (mode == MODE_RESCUE && !isProvisioned()
                    && mAdapter.getState() == BluetoothAdapter.STATE_OFF) {
                Log.i(TAG, "Enabling Bluetooth");
                mAdapter.enable();
            }
            return;
        }

        startScan(mode == MODE_RECOVER
                ? ScanSettings.SCAN_MODE_LOW_POWER : ScanSettings.SCAN_MODE_LOW_LATENCY);
        if (mode == MODE_RESCUE) {
            startDiscovery();
        } else {
            stopDiscovery();
        }
    }

    private boolean isProvisioned() {
        return Settings.Global.getInt(getContentResolver(),
                Settings.Global.DEVICE_PROVISIONED, 0) != 0;
    }

    private static String modeName(int mode) {
        switch (mode) {
            case MODE_ASSIST:
                return "assist";
            case MODE_RECOVER:
                return "recover";
            case MODE_RESCUE:
                return "rescue";
            default:
                return "idle";
        }
    }

    // Scanning

    private void startScan(int scanMode) {
        long now = SystemClock.elapsedRealtime();
        if (mScanner != null && mScanMode == scanMode && now - mScanStarted < SCAN_RESTART_MS) {
            return;
        }
        stopScan();

        BluetoothLeScanner scanner = mAdapter.getBluetoothLeScanner();
        if (scanner == null) {
            return;
        }
        ScanSettings settings = new ScanSettings.Builder().setScanMode(scanMode).build();
        scanner.startScan(new ArrayList<ScanFilter>(), settings, mScanCallback);
        mScanner = scanner;
        mScanMode = scanMode;
        mScanStarted = now;
    }

    private void stopScan() {
        if (mScanner != null) {
            try {
                mScanner.stopScan(mScanCallback);
            } catch (IllegalStateException e) {
                Log.w(TAG, "Failed to stop scan", e);
            }
            mScanner = null;
        }
        mScanMode = -1;
    }

    private void startDiscovery() {
        if (mDiscovering || mAdapter.isDiscovering()
                || SystemClock.elapsedRealtime() < mNextDiscovery) {
            return;
        }
        mDiscovering = mAdapter.startDiscovery();
    }

    private void stopDiscovery() {
        if (mDiscovering) {
            mAdapter.cancelDiscovery();
            mDiscovering = false;
        }
    }

    private boolean isRetryBlocked(BluetoothDevice device) {
        Long until = mRetryAfter.get(device.getAddress());
        return until != null && SystemClock.elapsedRealtime() < until;
    }

    private void onLeResult(ScanResult result) {
        BluetoothDevice device = result.getDevice();
        ScanRecord record = result.getScanRecord();
        if (mTarget != null || mMode == MODE_IDLE || device == null || record == null) {
            return;
        }
        int bondState = device.getBondState();
        if (bondState == BluetoothDevice.BOND_BONDING || isRetryBlocked(device)) {
            return;
        }

        Remotes.Advert adv = Remotes.Advert.parse(record.getBytes());
        if (!adv.isDiscoverable() || (!adv.isAmazon() && !adv.isInput())) {
            return;
        }
        if (mSeen.add(device.getAddress())) {
            Log.i(TAG, "Found " + device + " rssi=" + result.getRssi() + " bond=" + bondState
                    + " " + adv);
        }

        boolean strong = result.getRssi() >= MIN_RSSI;
        boolean wanted;
        switch (mMode) {
            case MODE_ASSIST:
                wanted = adv.isAmazon() && !adv.isListedByTvSettings(device);
                break;
            case MODE_RECOVER:
                wanted = adv.isAmazon() && strong;
                break;
            case MODE_RESCUE:
                wanted = strong;
                break;
            default:
                wanted = false;
                break;
        }
        if (!wanted) {
            return;
        }

        if (bondState == BluetoothDevice.BOND_BONDED) {
            if (adv.isAmazon() && !isInputConnected(device)
                    && mUnbonding.add(device.getAddress())) {
                Log.i(TAG, "Removing stale bond of " + device + " in pairing mode");
                device.removeBond();
            }
            return;
        }
        bond(device, BluetoothDevice.TRANSPORT_LE, adv.label(this, device), adv.toString());
    }

    private boolean isInputConnected(BluetoothDevice device) {
        return mInput == null
                || mInput.getConnectionState(device) != BluetoothProfile.STATE_DISCONNECTED;
    }

    private void onDeviceFound(Intent intent) {
        if (mTarget != null || mMode != MODE_RESCUE || !mDiscovering) {
            return;
        }

        BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
        BluetoothClass cls = intent.getParcelableExtra(BluetoothDevice.EXTRA_CLASS);
        short rssi = intent.getShortExtra(BluetoothDevice.EXTRA_RSSI, Short.MIN_VALUE);
        if (device == null || device.getType() == BluetoothDevice.DEVICE_TYPE_LE
                || device.getBondState() != BluetoothDevice.BOND_NONE
                || isRetryBlocked(device)) {
            return;
        }
        if (!Remotes.isInputClass(cls) || (rssi != Short.MIN_VALUE && rssi < MIN_RSSI)) {
            return;
        }

        String name = device.getName();
        bond(device, BluetoothDevice.TRANSPORT_BREDR, name != null ? name : device.getAddress(),
                "class=" + cls + " rssi=" + rssi);
    }

    // Pairing

    private void bond(BluetoothDevice device, int transport, String label, String reason) {
        Log.i(TAG, "Pairing " + device + " over "
                + (transport == BluetoothDevice.TRANSPORT_LE ? "LE" : "BR/EDR") + ": " + reason);

        mHandler.removeCallbacks(mUpdateRunnable);
        stopScan();
        stopDiscovery();
        if (mAdapter.isDiscovering()) {
            mAdapter.cancelDiscovery();
        }

        mTarget = device;
        mTargetLabel = label;
        mTargetFromScreen = mMode == MODE_ASSIST;
        mBonded = false;
        mConnectAttempts = 0;
        sTarget = device.getAddress();

        mHandler.postDelayed(mTimeoutRunnable, BOND_TIMEOUT_MS);
        if (!device.createBond(transport)) {
            Log.w(TAG, "createBond failed for " + device);
            finishTarget(false);
        }
    }

    private void onBondStateChanged(BluetoothDevice device, int state) {
        if (device != null && state == BluetoothDevice.BOND_NONE) {
            mUnbonding.remove(device.getAddress());
        }
        if (mTarget == null || !mTarget.equals(device)) {
            return;
        }
        if (state == BluetoothDevice.BOND_BONDED) {
            Log.i(TAG, "Bonded " + device);
            mBonded = true;
            connectInput();
        } else if (state == BluetoothDevice.BOND_NONE) {
            Log.w(TAG, "Pairing failed for " + device);
            mBonded = false;
            finishTarget(false);
        }
    }

    private void connectInput() {
        mHandler.removeCallbacks(mTimeoutRunnable);
        mHandler.postDelayed(mTimeoutRunnable, CONNECT_TIMEOUT_MS);
        if (mInput == null) {
            return;
        }
        if (mInput.getConnectionState(mTarget) == BluetoothProfile.STATE_CONNECTED) {
            finishTarget(true);
            return;
        }
        mConnectAttempts++;
        mInput.setPriority(mTarget, BluetoothProfile.PRIORITY_AUTO_CONNECT);
        if (!mInput.connect(mTarget)) {
            Log.w(TAG, "HID connect not started for " + mTarget);
        }
    }

    private void onInputStateChanged(BluetoothDevice device, int state) {
        if (mTarget != null && mTarget.equals(device)
                && state == BluetoothProfile.STATE_CONNECTED) {
            finishTarget(true);
        }
    }

    private void finishTarget(boolean success) {
        mHandler.removeCallbacks(mTimeoutRunnable);

        BluetoothDevice device = mTarget;
        boolean fromScreen = mTargetFromScreen;
        mTarget = null;
        sTarget = null;
        if (device == null) {
            return;
        }

        if (success) {
            Log.i(TAG, "Connected " + device);
            Toast.makeText(this, getString(R.string.device_connected, mTargetLabel),
                    Toast.LENGTH_LONG).show();
            if (fromScreen) {
                mDismissPending = true;
                mHandler.postDelayed(mDismissRunnable, DISMISS_DELAY_MS);
            }
        } else {
            if (mBonded) {
                device.removeBond();
            }
            mRetryAfter.put(device.getAddress(), SystemClock.elapsedRealtime() + RETRY_DELAY_MS);
        }
        mBonded = false;

        update();
    }

    // Callbacks

    private final Runnable mUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            update();
        }
    };

    private final Runnable mTimeoutRunnable = new Runnable() {
        @Override
        public void run() {
            if (mTarget == null) {
                return;
            }
            if (!mBonded) {
                Log.w(TAG, "Pairing timed out for " + mTarget);
                mTarget.cancelBondProcess();
                finishTarget(false);
            } else if (mConnectAttempts < CONNECT_ATTEMPTS) {
                Log.w(TAG, "Retrying HID connection to " + mTarget);
                connectInput();
            } else {
                Log.w(TAG, "HID connection timed out for " + mTarget);
                finishTarget(false);
            }
        }
    };

    private final Runnable mDismissRunnable = new Runnable() {
        @Override
        public void run() {
            mDismissPending = false;
            if (Remotes.isAddAccessoryOnTop(PairingService.this)) {
                Log.i(TAG, "Closing pairing screen");
                Remotes.sendBack(PairingService.this);
            }
            update();
        }
    };

    private final ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            onLeResult(result);
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.w(TAG, "LE scan failed: " + errorCode);
            mScanner = null;
            mScanMode = -1;
        }
    };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                onDeviceFound(intent);
            } else if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {
                onBondStateChanged(device, intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                        BluetoothDevice.ERROR));
            } else if (BluetoothInputDevice.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
                onInputStateChanged(device, intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1));
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                if (mDiscovering) {
                    mDiscovering = false;
                    mNextDiscovery = SystemClock.elapsedRealtime() + DISCOVERY_GAP_MS;
                }
            } else if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, -1);
                if (state == BluetoothAdapter.STATE_ON) {
                    update();
                } else if (state == BluetoothAdapter.STATE_TURNING_OFF) {
                    mScanner = null;
                    mScanMode = -1;
                    mDiscovering = false;
                    if (mTarget != null) {
                        finishTarget(false);
                    }
                }
            }
        }
    };

    private final BluetoothProfile.ServiceListener mProfileListener =
            new BluetoothProfile.ServiceListener() {
        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            mInput = (BluetoothInputDevice) proxy;
            if (mTarget != null && mBonded) {
                connectInput();
            }
        }

        @Override
        public void onServiceDisconnected(int profile) {
            mInput = null;
        }
    };

    private final InputManager.InputDeviceListener mInputListener =
            new InputManager.InputDeviceListener() {
        @Override
        public void onInputDeviceAdded(int deviceId) {
            update();
        }

        @Override
        public void onInputDeviceRemoved(int deviceId) {
            update();
        }

        @Override
        public void onInputDeviceChanged(int deviceId) {
        }
    };
}

/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.wifi;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Handler;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.util.Log;

public class WifiRemoteService extends Service implements WifiP2pManager.ChannelListener {

    private static final String TAG = "WifiRemoteService";

    public static final String ACTION_PAIR = "org.lineageos.hearth.wifi.action.PAIR";
    public static final String ACTION_PAIRING_CHANGED =
            "org.lineageos.hearth.wifi.action.PAIRING_CHANGED";
    public static final String EXTRA_PAIRING = "pairing";

    private static final String KEY_REMOTE_PAIRED = "remote_paired";

    private static final long PAIRING_TIMEOUT_MS = 120 * 1000;
    private static final long RETRY_MS = 10 * 1000;

    private final Handler mHandler = new Handler();

    private WifiP2pManager mManager;
    private WifiP2pManager.Channel mChannel;
    private SharedPreferences mPrefs;

    private boolean mEnabled;
    private boolean mGroupOwner;
    private boolean mPairing;
    private boolean mPairRequested;
    private boolean mAutoPairChecked;
    private int mClients;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        mManager = getSystemService(WifiP2pManager.class);
        mChannel = mManager.initialize(this, getMainLooper(), this);

        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        registerReceiver(mReceiver, filter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && ACTION_PAIR.equals(intent.getAction())) {
            mPairRequested = true;
            checkGroup();
        }
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        mHandler.removeCallbacksAndMessages(null);
        unregisterReceiver(mReceiver);
        setPairing(false);
        super.onDestroy();
    }

    @Override
    public void onChannelDisconnected() {
        Log.w(TAG, "Wi-Fi Direct channel lost, reconnecting");
        mChannel = mManager.initialize(this, getMainLooper(), this);
        checkGroup();
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()) {
                case WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION:
                    mEnabled = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE,
                            WifiP2pManager.WIFI_P2P_STATE_DISABLED)
                            == WifiP2pManager.WIFI_P2P_STATE_ENABLED;
                    if (!mEnabled) {
                        mGroupOwner = false;
                        setPairing(false);
                    }
                    break;
                case WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION:
                    WifiP2pGroup group =
                            intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_GROUP);
                    onGroupChanged(group);
                    break;
            }
            checkGroup();
        }
    };

    private void onGroupChanged(WifiP2pGroup group) {
        mGroupOwner = group != null && group.isGroupOwner();
        int clients = mGroupOwner ? group.getClientList().size() : 0;
        if (clients > 0) {
            mPrefs.edit().putBoolean(KEY_REMOTE_PAIRED, true).apply();
        }
        if (clients > mClients && mPairing) {
            Log.i(TAG, "Remote connected, closing the pairing window");
            setPairing(false);
        }
        mClients = clients;
    }

    private void checkGroup() {
        mHandler.removeCallbacks(mGroupCheck);
        mHandler.post(mGroupCheck);
    }

    private final Runnable mGroupCheck = new Runnable() {
        @Override
        public void run() {
            if (!mEnabled) {
                return;
            }
            mManager.requestGroupInfo(mChannel, new WifiP2pManager.GroupInfoListener() {
                @Override
                public void onGroupInfoAvailable(WifiP2pGroup group) {
                    if (group == null) {
                        createGroup();
                    } else if (group.isGroupOwner()) {
                        mGroupOwner = true;
                        maybeStartPairing();
                    }
                }
            });
        }
    };

    private void createGroup() {
        Log.i(TAG, "Starting the remote group");
        mManager.createGroup(mChannel, new WifiP2pManager.ActionListener() {
            @Override
            public void onSuccess() {
            }

            @Override
            public void onFailure(int reason) {
                Log.w(TAG, "Failed to start the remote group: " + reason);
                mHandler.removeCallbacks(mGroupCheck);
                mHandler.postDelayed(mGroupCheck, RETRY_MS);
            }
        });
    }

    private void maybeStartPairing() {
        if (!mAutoPairChecked) {
            mAutoPairChecked = true;
            if (!mPrefs.getBoolean(KEY_REMOTE_PAIRED, false)) {
                Log.i(TAG, "No remote paired yet");
                mPairRequested = true;
            }
        }
        if (!mPairRequested) {
            return;
        }
        mPairRequested = false;

        WpsInfo wps = new WpsInfo();
        wps.setup = WpsInfo.PBC;
        mManager.startWps(mChannel, wps, new WifiP2pManager.ActionListener() {
            @Override
            public void onSuccess() {
                Log.i(TAG, "Pairing window open");
                setPairing(true);
            }

            @Override
            public void onFailure(int reason) {
                Log.w(TAG, "Failed to open the pairing window: " + reason);
            }
        });
    }

    private void setPairing(boolean pairing) {
        mHandler.removeCallbacks(mPairingTimeout);
        if (pairing) {
            mHandler.postDelayed(mPairingTimeout, PAIRING_TIMEOUT_MS);
        }
        if (mPairing == pairing) {
            return;
        }
        mPairing = pairing;
        sendBroadcast(new Intent(ACTION_PAIRING_CHANGED)
                .setPackage(getPackageName())
                .putExtra(EXTRA_PAIRING, pairing));
    }

    private final Runnable mPairingTimeout = new Runnable() {
        @Override
        public void run() {
            setPairing(false);
        }
    };
}

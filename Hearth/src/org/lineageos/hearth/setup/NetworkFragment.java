/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.setup;

import android.app.Activity;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v17.leanback.app.GuidedStepFragment;
import android.support.v17.leanback.widget.GuidanceStylist;
import android.support.v17.leanback.widget.GuidedAction;

import java.util.ArrayList;
import java.util.List;

import org.lineageos.hearth.R;

public class NetworkFragment extends GuidedStepFragment {

    private static final long ACTION_NEXT = 1;
    private static final long ACTION_WIFI = 2;

    private static final String ACTION_SETUP_WIFI = "com.android.net.wifi.SETUP_WIFI_NETWORK";
    private static final int REQUEST_WIFI = 1;

    private final Handler mHandler = new Handler();

    private ConnectivityManager mConnectivityManager;
    private boolean mConnected;

    @Override
    public GuidanceStylist.Guidance onCreateGuidance(Bundle savedInstanceState) {
        return new GuidanceStylist.Guidance(getString(R.string.setup_network_title),
                getString(R.string.setup_network_summary), getString(R.string.setup_breadcrumb),
                getActivity().getDrawable(R.drawable.ic_hearth_mark));
    }

    @Override
    public void onCreateActions(List<GuidedAction> actions, Bundle savedInstanceState) {
        mConnectivityManager = getActivity().getSystemService(ConnectivityManager.class);
        actions.addAll(buildActions());
    }

    @Override
    public void onResume() {
        super.onResume();
        mConnectivityManager.registerDefaultNetworkCallback(mNetworkCallback);
        update();
    }

    @Override
    public void onPause() {
        mConnectivityManager.unregisterNetworkCallback(mNetworkCallback);
        mHandler.removeCallbacksAndMessages(null);
        super.onPause();
    }

    @Override
    public void onGuidedActionClicked(GuidedAction action) {
        if (action.getId() == ACTION_WIFI) {
            startActivityForResult(new Intent(ACTION_SETUP_WIFI), REQUEST_WIFI);
        } else if (action.getId() == ACTION_NEXT) {
            GuidedStepFragment.add(getFragmentManager(), new DoneFragment());
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_WIFI && resultCode == Activity.RESULT_OK) {
            GuidedStepFragment.add(getFragmentManager(), new DoneFragment());
        }
    }

    private List<GuidedAction> buildActions() {
        List<GuidedAction> actions = new ArrayList<>();
        NetworkInfo info = mConnectivityManager.getActiveNetworkInfo();
        mConnected = info != null && info.isConnected();
        if (mConnected) {
            actions.add(new GuidedAction.Builder(getActivity())
                    .id(ACTION_NEXT)
                    .title(R.string.setup_continue)
                    .build());
        }
        actions.add(new GuidedAction.Builder(getActivity())
                .id(ACTION_WIFI)
                .title(mConnected ? R.string.setup_network_change : R.string.setup_network_wifi)
                .build());
        if (!mConnected) {
            actions.add(new GuidedAction.Builder(getActivity())
                    .id(ACTION_NEXT)
                    .title(R.string.setup_skip)
                    .build());
        }
        return actions;
    }

    private void update() {
        NetworkInfo info = mConnectivityManager.getActiveNetworkInfo();
        boolean connected = info != null && info.isConnected();
        String summary;
        if (!connected) {
            summary = getString(R.string.setup_network_summary);
        } else if (info.getType() == ConnectivityManager.TYPE_ETHERNET) {
            summary = getString(R.string.setup_network_ethernet);
        } else {
            String ssid = getActivity().getSystemService(WifiManager.class)
                    .getConnectionInfo().getSSID();
            summary = getString(R.string.setup_network_wifi_connected,
                    ssid.replaceAll("^\"|\"$", ""));
        }
        getGuidanceStylist().getDescriptionView().setText(summary);
        if (connected != mConnected) {
            setActions(buildActions());
        }
    }

    private final ConnectivityManager.NetworkCallback mNetworkCallback =
            new ConnectivityManager.NetworkCallback() {
        @Override
        public void onAvailable(Network network) {
            mHandler.post(mUpdate);
        }

        @Override
        public void onLost(Network network) {
            mHandler.post(mUpdate);
        }
    };

    private final Runnable mUpdate = new Runnable() {
        @Override
        public void run() {
            if (isResumed()) {
                update();
            }
        }
    };
}

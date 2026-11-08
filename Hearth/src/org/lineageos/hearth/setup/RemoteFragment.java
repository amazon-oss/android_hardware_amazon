/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.setup;

import android.hardware.input.InputManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v17.leanback.app.GuidedStepFragment;
import android.support.v17.leanback.widget.GuidanceStylist;
import android.support.v17.leanback.widget.GuidedAction;

import java.util.ArrayList;
import java.util.List;

import org.lineageos.hearth.Pairing;
import org.lineageos.hearth.R;
import org.lineageos.hearth.remotes.RemoteDevice;
import org.lineageos.hearth.remotes.RemoteDevices;

public class RemoteFragment extends GuidedStepFragment
        implements InputManager.InputDeviceListener, RemoteDevices.Callback {

    private static final long ACTION_NEXT = 1;
    private static final long ACTION_SEARCHING = 2;
    private static final long ACTION_REMOTE = 100;

    private static final long REOPEN_MS = 110 * 1000;
    private static final long REFRESH_MS = 3 * 1000;

    private final Handler mHandler = new Handler();

    private InputManager mInputManager;
    private List<RemoteDevice> mShown;

    @Override
    public GuidanceStylist.Guidance onCreateGuidance(Bundle savedInstanceState) {
        return new GuidanceStylist.Guidance(getString(R.string.setup_remote_title),
                getString(R.string.setup_remote_summary), getString(R.string.setup_breadcrumb),
                null);
    }

    @Override
    public void onCreateActions(List<GuidedAction> actions, Bundle savedInstanceState) {
        actions.addAll(buildActions(new ArrayList<RemoteDevice>()));
    }

    @Override
    public void onResume() {
        super.onResume();
        mInputManager = getActivity().getSystemService(InputManager.class);
        mInputManager.registerInputDeviceListener(this, mHandler);
        mReopen.run();
        mRefresh.run();
    }

    @Override
    public void onPause() {
        mHandler.removeCallbacksAndMessages(null);
        mInputManager.unregisterInputDeviceListener(this);
        super.onPause();
    }

    @Override
    public void onGuidedActionClicked(GuidedAction action) {
        if (action.getId() == ACTION_NEXT) {
            GuidedStepFragment.add(getFragmentManager(), new NetworkFragment());
        }
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        refresh();
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        refresh();
    }

    @Override
    public void onRemotesScanned(List<RemoteDevice> remotes) {
        if (!isResumed() || remotes.equals(mShown)) {
            return;
        }
        mShown = remotes;
        List<GuidedAction> actions = buildActions(remotes);
        setActions(actions);
        setSelectedActionPosition(actions.size() - 1);
    }

    private List<GuidedAction> buildActions(List<RemoteDevice> remotes) {
        List<GuidedAction> actions = new ArrayList<>();
        if (remotes.isEmpty()) {
            actions.add(new GuidedAction.Builder(getActivity())
                    .id(ACTION_SEARCHING)
                    .title(R.string.setup_remote_searching)
                    .description(R.string.setup_remote_searching_summary)
                    .infoOnly(true)
                    .build());
        }
        for (int i = 0; i < remotes.size(); i++) {
            RemoteDevice remote = remotes.get(i);
            actions.add(new GuidedAction.Builder(getActivity())
                    .id(ACTION_REMOTE + i)
                    .title(remote.name)
                    .description(remote.getSummary(getActivity()))
                    .infoOnly(true)
                    .build());
        }
        actions.add(new GuidedAction.Builder(getActivity())
                .id(ACTION_NEXT)
                .title(remotes.isEmpty() ? R.string.setup_skip : R.string.setup_continue)
                .build());
        return actions;
    }

    private void refresh() {
        RemoteDevices.scanAsync(getActivity().getApplicationContext(), this);
    }

    private final Runnable mReopen = new Runnable() {
        @Override
        public void run() {
            Pairing.start(getActivity());
            mHandler.postDelayed(this, REOPEN_MS);
        }
    };

    private final Runnable mRefresh = new Runnable() {
        @Override
        public void run() {
            refresh();
            mHandler.postDelayed(this, REFRESH_MS);
        }
    };
}

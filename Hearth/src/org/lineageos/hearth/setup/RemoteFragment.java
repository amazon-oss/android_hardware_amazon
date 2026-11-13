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
import android.view.InputDevice;

import java.util.List;

import org.lineageos.hearth.Pairing;
import org.lineageos.hearth.R;

public class RemoteFragment extends GuidedStepFragment
        implements InputManager.InputDeviceListener {

    private static final long ACTION_NEXT = 1;

    private static final long REOPEN_MS = 110 * 1000;
    private static final long ADVANCE_MS = 2 * 1000;

    private final Handler mHandler = new Handler();

    private InputManager mInputManager;
    private boolean mConnected;

    @Override
    public GuidanceStylist.Guidance onCreateGuidance(Bundle savedInstanceState) {
        return new GuidanceStylist.Guidance(getString(R.string.setup_remote_title),
                getString(R.string.setup_remote_summary), getString(R.string.setup_breadcrumb),
                getActivity().getDrawable(R.drawable.ic_hearth_mark));
    }

    @Override
    public void onCreateActions(List<GuidedAction> actions, Bundle savedInstanceState) {
        actions.add(new GuidedAction.Builder(getActivity())
                .id(ACTION_NEXT)
                .title(R.string.setup_skip)
                .build());
    }

    @Override
    public void onResume() {
        super.onResume();
        mInputManager = getActivity().getSystemService(InputManager.class);
        mInputManager.registerInputDeviceListener(this, mHandler);
        for (int id : mInputManager.getInputDeviceIds()) {
            onInputDeviceAdded(id);
        }
        mReopen.run();
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
            next();
        }
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        InputDevice device = InputDevice.getDevice(deviceId);
        if (mConnected || !isRemote(device)) {
            return;
        }
        mConnected = true;
        getGuidanceStylist().getDescriptionView().setText(
                getString(R.string.setup_remote_connected, device.getName()));
        GuidedAction action = findActionById(ACTION_NEXT);
        action.setTitle(getString(R.string.setup_continue));
        notifyActionChanged(findActionPositionById(ACTION_NEXT));
        mHandler.postDelayed(mAdvance, ADVANCE_MS);
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
    }

    private static boolean isRemote(InputDevice device) {
        return device != null && !device.isVirtual() && device.isExternal()
                && (device.supportsSource(InputDevice.SOURCE_DPAD)
                || device.supportsSource(InputDevice.SOURCE_GAMEPAD)
                || device.getKeyboardType() == InputDevice.KEYBOARD_TYPE_ALPHABETIC);
    }

    private void next() {
        mHandler.removeCallbacksAndMessages(null);
        if (isResumed()) {
            GuidedStepFragment.add(getFragmentManager(), new NetworkFragment());
        }
    }

    private final Runnable mReopen = new Runnable() {
        @Override
        public void run() {
            Pairing.start(getActivity());
            mHandler.postDelayed(this, REOPEN_MS);
        }
    };

    private final Runnable mAdvance = new Runnable() {
        @Override
        public void run() {
            next();
        }
    };
}

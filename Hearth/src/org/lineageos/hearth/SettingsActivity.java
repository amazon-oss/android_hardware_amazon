/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth;

import android.app.Activity;
import android.app.Fragment;
import android.hardware.input.InputManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemProperties;
import android.support.v14.preference.PreferenceFragment;
import android.support.v14.preference.SwitchPreference;
import android.support.v17.preference.LeanbackPreferenceFragment;
import android.support.v17.preference.LeanbackSettingsFragment;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceCategory;
import android.support.v7.preference.PreferenceScreen;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

import org.lineageos.hearth.remotes.RemoteDevice;
import org.lineageos.hearth.remotes.RemoteDevices;

public class SettingsActivity extends Activity {

    public static final String KEY_LOW_BATTERY_ALERTS = "low_battery_alerts";
    public static final String KEY_STATUS_LED = "status_led";

    private static final String KEY_HARDWARE = "hardware";
    private static final String KEY_PAIR_REMOTE = "pair_remote";
    private static final String KEY_REMOTES = "remotes";
    private static final String KEY_USB_DEVICE_MODE = "usb_device_mode";
    private static final String PROP_USB_DEVICE_MODE = "persist.sys.usb.debugging";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (savedInstanceState == null) {
            getFragmentManager().beginTransaction()
                    .add(android.R.id.content, new SettingsFragment())
                    .commit();
        }
    }

    public static class SettingsFragment extends LeanbackSettingsFragment {

        @Override
        public void onPreferenceStartInitialScreen() {
            startPreferenceFragment(new MainFragment());
        }

        @Override
        public boolean onPreferenceStartFragment(PreferenceFragment caller, Preference pref) {
            Fragment f = Fragment.instantiate(getActivity(), pref.getFragment(), pref.getExtras());
            f.setTargetFragment(caller, 0);
            startPreferenceFragment(f);
            return true;
        }

        @Override
        public boolean onPreferenceStartScreen(PreferenceFragment caller, PreferenceScreen pref) {
            return false;
        }
    }

    public static class MainFragment extends LeanbackPreferenceFragment
            implements Preference.OnPreferenceChangeListener, InputManager.InputDeviceListener,
            RemoteDevices.Callback {

        private static final long REFRESH_MS = 5 * 1000;

        private final Handler mHandler = new Handler();
        private final List<Preference> mRemotePrefs = new ArrayList<>();

        private PreferenceCategory mRemotes;
        private List<RemoteDevice> mShown;
        private InputManager mInputManager;

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.device_settings, null);

            mRemotes = (PreferenceCategory) findPreference(KEY_REMOTES);
            mInputManager = getActivity().getSystemService(InputManager.class);

            PreferenceCategory hardware = (PreferenceCategory) findPreference(KEY_HARDWARE);
            if (!Features.hasLed()) {
                hardware.removePreference(findPreference(KEY_STATUS_LED));
            }

            SwitchPreference usb = (SwitchPreference) findPreference(KEY_USB_DEVICE_MODE);
            if (Features.hasUsbRole()) {
                usb.setChecked(SystemProperties.getBoolean(PROP_USB_DEVICE_MODE, false));
                usb.setOnPreferenceChangeListener(this);
            } else {
                hardware.removePreference(usb);
            }

            if (hardware.getPreferenceCount() == 0) {
                getPreferenceScreen().removePreference(hardware);
            }
        }

        @Override
        public void onResume() {
            super.onResume();
            mInputManager.registerInputDeviceListener(this, mHandler);
            mRefresh.run();
        }

        @Override
        public void onPause() {
            mHandler.removeCallbacks(mRefresh);
            mInputManager.unregisterInputDeviceListener(this);
            super.onPause();
        }

        @Override
        public boolean onPreferenceTreeClick(Preference preference) {
            if (KEY_PAIR_REMOTE.equals(preference.getKey())) {
                Pairing.start(getActivity());
                Toast.makeText(getActivity(), R.string.pair_remote_started,
                        Toast.LENGTH_LONG).show();
                return true;
            }
            return super.onPreferenceTreeClick(preference);
        }

        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            if (KEY_USB_DEVICE_MODE.equals(preference.getKey())) {
                SystemProperties.set(PROP_USB_DEVICE_MODE, (Boolean) newValue ? "y" : "n");
            }
            return true;
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
            for (Preference pref : mRemotePrefs) {
                mRemotes.removePreference(pref);
            }
            mRemotePrefs.clear();
            for (int i = 0; i < remotes.size(); i++) {
                RemoteDevice remote = remotes.get(i);
                Preference pref = new Preference(getPreferenceManager().getContext());
                pref.setTitle(remote.name);
                pref.setSummary(remote.getSummary(getActivity()));
                pref.setOrder(i);
                pref.setSelectable(false);
                pref.setPersistent(false);
                mRemotes.addPreference(pref);
                mRemotePrefs.add(pref);
            }
        }

        private void refresh() {
            RemoteDevices.scanAsync(getActivity().getApplicationContext(), this);
        }

        private final Runnable mRefresh = new Runnable() {
            @Override
            public void run() {
                refresh();
                mHandler.postDelayed(this, REFRESH_MS);
            }
        };
    }
}

/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.os.SystemProperties;
import android.support.v14.preference.PreferenceFragment;
import android.support.v14.preference.SwitchPreference;
import android.support.v17.preference.LeanbackPreferenceFragment;
import android.support.v17.preference.LeanbackSettingsFragment;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceCategory;
import android.support.v7.preference.PreferenceScreen;

public class SettingsActivity extends Activity {

    public static final String KEY_STATUS_LED = "status_led";

    private static final String KEY_HARDWARE = "hardware";
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
            implements Preference.OnPreferenceChangeListener {

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.device_settings, null);

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
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            if (KEY_USB_DEVICE_MODE.equals(preference.getKey())) {
                SystemProperties.set(PROP_USB_DEVICE_MODE, (Boolean) newValue ? "y" : "n");
            }
            return true;
        }
    }
}

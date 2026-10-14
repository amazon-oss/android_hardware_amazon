/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;

public class CameraShutterFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private static final String KEY_LAUNCH = "camera_shutter_launch";
    private static final String PROP_LAUNCH = "persist.sys.camera.shutter_launch";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.camera_shutter);

        SwitchPreference launch = (SwitchPreference) findPreference(KEY_LAUNCH);
        launch.setChecked(SystemProperties.getBoolean(PROP_LAUNCH, false));
        launch.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        SystemProperties.set(PROP_LAUNCH, (Boolean) newValue ? "1" : "0");
        return true;
    }
}

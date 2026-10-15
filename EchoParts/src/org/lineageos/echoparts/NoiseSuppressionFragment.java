/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;

public class NoiseSuppressionFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private static final String KEY_ENABLE = "noise_suppression_enable";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.noise_suppression);

        SwitchPreference enable = (SwitchPreference) findPreference(KEY_ENABLE);
        enable.setChecked(NoiseSuppression.isEnabled());
        enable.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        NoiseSuppression.setEnabled(getActivity(), (Boolean) newValue);
        return true;
    }
}

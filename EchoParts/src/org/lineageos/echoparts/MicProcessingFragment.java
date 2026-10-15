/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;

public class MicProcessingFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private static final String KEY_MODE = "mic_processing_mode";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.mic_processing);

        ListPreference mode = (ListPreference) findPreference(KEY_MODE);
        mode.setValue(MicProcessing.getMode());
        mode.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        MicProcessing.setMode(getActivity(), (String) newValue);
        return true;
    }
}

/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.echoparts;

import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;

public class BluetoothAudioFragment extends PreferenceFragment
        implements Preference.OnPreferenceChangeListener {

    private static final String KEY_MODE = "bluetooth_audio_mode";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.bluetooth_audio);

        ListPreference mode = (ListPreference) findPreference(KEY_MODE);
        mode.setValue(String.valueOf(BluetoothAudio.getMode(getActivity())));
        mode.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        BluetoothAudio.setMode(getActivity(), Integer.parseInt((String) newValue));
        return true;
    }
}

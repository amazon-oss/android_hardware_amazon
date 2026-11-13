/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.setup;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.provider.Settings;
import android.support.v17.leanback.app.GuidedStepFragment;

public class SetupActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Settings.Global.getInt(getContentResolver(),
                Settings.Global.DEVICE_PROVISIONED, 0) == 1) {
            complete();
            return;
        }
        if (savedInstanceState == null) {
            GuidedStepFragment.addAsRoot(this, new RemoteFragment(), android.R.id.content);
        }
    }

    @Override
    public void onBackPressed() {
        if (getFragmentManager().getBackStackEntryCount() > 0) {
            super.onBackPressed();
        }
    }

    void complete() {
        ContentResolver resolver = getContentResolver();
        Settings.Global.putInt(resolver, Settings.Global.DEVICE_PROVISIONED, 1);
        Settings.Secure.putInt(resolver, Settings.Secure.USER_SETUP_COMPLETE, 1);

        getPackageManager().setComponentEnabledSetting(new ComponentName(this, getClass()),
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP);

        startActivity(new Intent(Intent.ACTION_MAIN)
                .addCategory(Intent.CATEGORY_HOME)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
        finish();
    }
}

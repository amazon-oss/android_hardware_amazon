/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.remotes;

import android.content.Context;

import org.lineageos.hearth.R;

public final class RemoteDevice {

    public final String name;
    public int battery = -1;
    public boolean charging;

    RemoteDevice(String name) {
        this.name = name;
    }

    public String getSummary(Context context) {
        if (battery < 0) {
            return context.getString(R.string.remote_connected);
        }
        return context.getString(charging ? R.string.remote_battery_charging
                : R.string.remote_battery, battery);
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof RemoteDevice)) {
            return false;
        }
        RemoteDevice other = (RemoteDevice) o;
        return name.equals(other.name) && battery == other.battery && charging == other.charging;
    }

    @Override
    public int hashCode() {
        return name.hashCode() * 31 + battery;
    }
}

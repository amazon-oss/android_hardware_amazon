/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.remotes;

import android.content.Context;
import android.hardware.input.InputManager;
import android.os.AsyncTask;
import android.view.InputDevice;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public final class RemoteDevices {

    public interface Callback {
        void onRemotesScanned(List<RemoteDevice> remotes);
    }

    private static final File POWER_SUPPLY = new File("/sys/class/power_supply");

    private RemoteDevices() {
    }

    public static boolean isRemote(InputDevice device) {
        return device != null && !device.isVirtual() && device.isExternal()
                && (device.supportsSource(InputDevice.SOURCE_DPAD)
                || device.supportsSource(InputDevice.SOURCE_GAMEPAD)
                || device.getKeyboardType() == InputDevice.KEYBOARD_TYPE_ALPHABETIC);
    }

    public static List<RemoteDevice> scan(Context context) {
        List<RemoteDevice> remotes = new ArrayList<>();
        InputManager inputManager = context.getSystemService(InputManager.class);
        for (int id : inputManager.getInputDeviceIds()) {
            InputDevice device = InputDevice.getDevice(id);
            if (isRemote(device) && find(remotes, device.getName()) == null) {
                remotes.add(new RemoteDevice(device.getName()));
            }
        }

        File[] supplies = POWER_SUPPLY.listFiles();
        if (supplies == null) {
            return remotes;
        }
        for (File supply : supplies) {
            if (!"Battery".equals(read(supply, "type")) || !"Device".equals(read(supply, "scope"))) {
                continue;
            }
            String model = read(supply, "model_name");
            int capacity = parse(read(supply, "capacity"));
            if (model == null || capacity < 0) {
                continue;
            }
            RemoteDevice remote = find(remotes, model);
            if (remote == null) {
                remote = new RemoteDevice(model);
                remotes.add(remote);
            }
            remote.battery = capacity;
            String status = read(supply, "status");
            remote.charging = "Charging".equals(status) || "Full".equals(status);
        }
        return remotes;
    }

    public static void scanAsync(final Context context, final Callback callback) {
        new AsyncTask<Void, Void, List<RemoteDevice>>() {
            @Override
            protected List<RemoteDevice> doInBackground(Void... params) {
                return scan(context);
            }

            @Override
            protected void onPostExecute(List<RemoteDevice> remotes) {
                callback.onRemotesScanned(remotes);
            }
        }.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    private static RemoteDevice find(List<RemoteDevice> remotes, String name) {
        for (RemoteDevice remote : remotes) {
            if (remote.name.startsWith(name) || name.startsWith(remote.name)) {
                return remote;
            }
        }
        return null;
    }

    private static String read(File dir, String name) {
        try (BufferedReader reader = new BufferedReader(new FileReader(new File(dir, name)))) {
            String line = reader.readLine();
            return line != null ? line.trim() : null;
        } catch (IOException e) {
            return null;
        }
    }

    private static int parse(String value) {
        try {
            return value != null ? Integer.parseInt(value) : -1;
        } catch (NumberFormatException e) {
            return -1;
        }
    }
}

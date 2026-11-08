/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.remotes;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.BitmapFactory;
import android.hardware.input.InputManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.UEventObserver;
import android.preference.PreferenceManager;
import android.widget.Toast;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.lineageos.hearth.R;
import org.lineageos.hearth.SettingsActivity;

public class BatteryService extends Service
        implements InputManager.InputDeviceListener,
        SharedPreferences.OnSharedPreferenceChangeListener {

    private static final String NOTIFICATION_TAG = "remote_battery";

    private static final int LOW_LEVEL = 15;
    private static final int RECOVERED_LEVEL = 25;
    private static final long POLL_MS = 15 * 60 * 1000;

    private final Handler mMainHandler = new Handler();
    private final Set<String> mWarned = new HashSet<>();

    private HandlerThread mThread;
    private Handler mHandler;
    private InputManager mInputManager;
    private NotificationManager mNotificationManager;
    private SharedPreferences mPrefs;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mThread = new HandlerThread("RemoteBattery");
        mThread.start();
        mHandler = new Handler(mThread.getLooper());

        mNotificationManager = getSystemService(NotificationManager.class);
        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        mPrefs.registerOnSharedPreferenceChangeListener(this);

        mInputManager = getSystemService(InputManager.class);
        mInputManager.registerInputDeviceListener(this, mMainHandler);
        mObserver.startObserving("SUBSYSTEM=power_supply");

        check();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        mObserver.stopObserving();
        mInputManager.unregisterInputDeviceListener(this);
        mPrefs.unregisterOnSharedPreferenceChangeListener(this);
        mThread.quitSafely();
        super.onDestroy();
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        check();
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        check();
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
        if (SettingsActivity.KEY_LOW_BATTERY_ALERTS.equals(key)) {
            check();
        }
    }

    private final UEventObserver mObserver = new UEventObserver() {
        @Override
        public void onUEvent(UEvent event) {
            check();
        }
    };

    private void check() {
        mHandler.removeCallbacks(mCheck);
        mHandler.post(mCheck);
    }

    private final Runnable mCheck = new Runnable() {
        @Override
        public void run() {
            boolean enabled = mPrefs.getBoolean(SettingsActivity.KEY_LOW_BATTERY_ALERTS, true);
            List<RemoteDevice> remotes = RemoteDevices.scan(BatteryService.this);
            Set<String> present = new HashSet<>();
            for (RemoteDevice remote : remotes) {
                present.add(remote.name);
                if (remote.battery < 0) {
                    continue;
                }
                boolean low = enabled && !remote.charging && remote.battery <= LOW_LEVEL;
                boolean recovered = !enabled || remote.charging
                        || remote.battery >= RECOVERED_LEVEL;
                if (low && mWarned.add(remote.name)) {
                    warn(remote);
                } else if (recovered && mWarned.remove(remote.name)) {
                    mNotificationManager.cancel(NOTIFICATION_TAG, remote.name.hashCode());
                }
            }
            for (String name : new HashSet<>(mWarned)) {
                if (!present.contains(name)) {
                    mWarned.remove(name);
                    mNotificationManager.cancel(NOTIFICATION_TAG, name.hashCode());
                }
            }
            mHandler.postDelayed(this, POLL_MS);
        }
    };

    private void warn(RemoteDevice remote) {
        final String title = getString(R.string.battery_low_title, remote.name);
        final String text = getString(R.string.battery_low_text, remote.battery);

        PendingIntent intent = PendingIntent.getActivity(this, 0,
                new Intent(this, SettingsActivity.class), PendingIntent.FLAG_UPDATE_CURRENT);
        Notification notification = new Notification.Builder(this)
                .setSmallIcon(R.drawable.ic_battery_alert)
                .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.banner))
                .setContentTitle(title)
                .setContentText(text)
                .setContentIntent(intent)
                .setCategory(Notification.CATEGORY_RECOMMENDATION)
                .setColor(getColor(R.color.hearth_accent))
                .setPriority(Notification.PRIORITY_HIGH)
                .setLocalOnly(true)
                .setOngoing(true)
                .build();
        mNotificationManager.notify(NOTIFICATION_TAG, remote.name.hashCode(), notification);

        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(BatteryService.this, title + "\n" + text, Toast.LENGTH_LONG).show();
            }
        });
    }
}

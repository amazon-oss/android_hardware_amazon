/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.pen

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.Handler
import android.os.IBinder
import android.os.UEventObserver
import android.util.Log

class PenService : Service() {
    private val notificationManager by lazy { getSystemService(NotificationManager::class.java) }

    private val lowBatteryThreshold by lazy {
        resources.getInteger(R.integer.config_penLowBatteryThreshold)
    }

    private val handler by lazy { Handler(mainLooper) }

    private val observer =
        object : UEventObserver() {
            override fun onUEvent(event: UEvent) {
                val level = event.get("STYLUS_LEVEL")?.toIntOrNull() ?: return

                Log.d(
                    TAG,
                    "Pen ${event.get("STYLUS_SERIAL")?.trim()} " +
                        "(fw ${event.get("STYLUS_FW_VERSION")}) battery level: $level"
                )

                handler.post { updateBatteryLevel(level) }
            }
        }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onCreate() {
        super.onCreate()

        notificationManager.createNotificationChannel(
            NotificationChannel(
                NOTIFICATION_CHANNEL_ID,
                getString(R.string.pen_notification_channel),
                NotificationManager.IMPORTANCE_HIGH,
            )
        )

        observer.startObserving("DEVPATH=/devices/virtual/stylus_uevent/stylus_dev")
    }

    override fun onDestroy() {
        super.onDestroy()

        observer.stopObserving()
    }

    private fun updateBatteryLevel(level: Int) {
        if (level > lowBatteryThreshold) {
            notificationManager.cancel(NOTIFICATION_ID)
            return
        }

        val notification =
            Notification.Builder(this, NOTIFICATION_CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_stylus)
                .setContentTitle(getString(R.string.pen_low_battery_title))
                .setContentText(getString(R.string.pen_low_battery_text, level))
                .setOnlyAlertOnce(true)
                .setAutoCancel(true)
                .build()
        notificationManager.notify(NOTIFICATION_ID, notification)
    }

    companion object {
        private const val TAG = "AmazonPenService"
        private const val NOTIFICATION_CHANNEL_ID = "AmazonPen"
        private const val NOTIFICATION_ID = 1000
    }
}

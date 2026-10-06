/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.keyhandler

import android.app.ActivityTaskManager
import android.app.KeyguardManager
import android.app.WindowConfiguration
import android.content.ActivityNotFoundException
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.input.InputManager
import android.hardware.usb.UsbConstants
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbEndpoint
import android.hardware.usb.UsbManager
import android.os.Handler
import android.os.HandlerThread
import android.os.SystemClock
import android.os.UserHandle
import android.provider.Settings
import android.util.Log
import android.view.InputDevice
import android.view.KeyCharacterMap
import android.view.KeyEvent
import com.android.internal.os.DeviceKeyHandler

class KeyHandler(private val context: Context) : DeviceKeyHandler {
    private val handler = Handler(HandlerThread(TAG).apply { start() }.looper)

    private val inputManager by lazy { context.getSystemService(InputManager::class.java)!! }
    private val keyguardManager by lazy { context.getSystemService(KeyguardManager::class.java)!! }
    private val usbManager by lazy { context.getSystemService(UsbManager::class.java)!! }

    private val remoteAppButtons by lazy { RemoteAppButtons(context) }

    private var connection: UsbDeviceConnection? = null
    private var endpoint: UsbEndpoint? = null

    private var trackpadDisabled: Boolean
        get() =
            Settings.Secure.getIntForUser(
                context.contentResolver,
                DISABLE_TRACKPAD,
                0,
                UserHandle.USER_CURRENT,
            ) != 0
        set(value) {
            Settings.Secure.putIntForUser(
                context.contentResolver,
                DISABLE_TRACKPAD,
                if (value) 1 else 0,
                UserHandle.USER_CURRENT,
            )
        }

    private val receiver =
        object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                try {
                    when (intent.action) {
                        UsbManager.ACTION_USB_DEVICE_ATTACHED -> {
                            if (intent.usbDevice.isAmazonKeyboard) applyTrackpadState()
                        }
                        UsbManager.ACTION_USB_DEVICE_DETACHED -> {
                            if (intent.usbDevice.isAmazonKeyboard) closeConnection()
                        }
                        Intent.ACTION_LOCKED_BOOT_COMPLETED,
                        Intent.ACTION_USER_SWITCHED -> applyTrackpadState()
                    }
                } catch (e: Exception) {
                    Log.e(TAG, "Failed to handle ${intent.action}", e)
                }
            }
        }

    init {
        context.registerReceiverForAllUsers(
            receiver,
            IntentFilter().apply {
                addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED)
                addAction(UsbManager.ACTION_USB_DEVICE_DETACHED)
                addAction(Intent.ACTION_LOCKED_BOOT_COMPLETED)
                addAction(Intent.ACTION_USER_SWITCHED)
            },
            null,
            handler,
            Context.RECEIVER_NOT_EXPORTED,
        )
    }

    override fun handleKeyEvent(event: KeyEvent): KeyEvent? {
        val device = inputManager.getInputDevice(event.deviceId)
        if (device.isFireTvRemote && RemoteAppButtons.handles(event.scanCode)) {
            if (event.action == KeyEvent.ACTION_DOWN && event.repeatCount == 0 && canLaunch()) {
                handler.post {
                    runCatching { remoteAppButtons.launch(device!!, event.scanCode) }.onFailure {
                        Log.e(TAG, "Failed to handle app button ${event.scanCode}", it)
                    }
                }
            }

            return null
        }

        val action: () -> Unit =
            when (event.scanCode) {
                SCANCODE_FILES -> ::launchFiles
                SCANCODE_TOGGLE_TRACKPAD -> ::toggleTrackpad
                SCANCODE_SPLIT_SCREEN -> ::toggleSplitScreen
                else -> return event
            }

        if (!device.isAmazonKeyboard) {
            return event
        }

        if (event.action == KeyEvent.ACTION_DOWN && event.repeatCount == 0) {
            handler.post {
                runCatching(action).onFailure {
                    Log.e(TAG, "Failed to handle scancode ${event.scanCode}", it)
                }
            }
        }

        return null
    }

    private fun canLaunch() =
        !keyguardManager.isKeyguardLocked &&
            Settings.Secure.getIntForUser(
                context.contentResolver,
                Settings.Secure.USER_SETUP_COMPLETE,
                0,
                UserHandle.USER_CURRENT,
            ) != 0

    private fun launchFiles() {
        if (!canLaunch()) return

        val intent =
            Intent.makeMainSelectorActivity(Intent.ACTION_MAIN, Intent.CATEGORY_APP_FILES)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)

        try {
            context.startActivityAsUser(intent, UserHandle.CURRENT)
        } catch (e: ActivityNotFoundException) {
            Log.w(TAG, "No files app found", e)
        }
    }

    private fun toggleSplitScreen() {
        if (!canLaunch()) return

        val task = ActivityTaskManager.getInstance().getTasks(1).firstOrNull()
        val inSplit = task?.windowingMode == WindowConfiguration.WINDOWING_MODE_MULTI_WINDOW

        injectShortcut(if (inSplit) KeyEvent.KEYCODE_DPAD_UP else KeyEvent.KEYCODE_DPAD_LEFT)
    }

    private fun injectShortcut(keyCode: Int) {
        val now = SystemClock.uptimeMillis()

        for (action in intArrayOf(KeyEvent.ACTION_DOWN, KeyEvent.ACTION_UP)) {
            inputManager.injectInputEvent(
                KeyEvent(
                    now,
                    now,
                    action,
                    keyCode,
                    0,
                    SHORTCUT_META_STATE,
                    KeyCharacterMap.VIRTUAL_KEYBOARD,
                    0,
                    KeyEvent.FLAG_FROM_SYSTEM,
                    InputDevice.SOURCE_KEYBOARD,
                ),
                InputManager.INJECT_INPUT_EVENT_MODE_ASYNC,
            )
        }
    }

    private fun toggleTrackpad() {
        val disabled = !trackpadDisabled

        if (setTrackpadEnabled(!disabled)) {
            trackpadDisabled = disabled
        }
    }

    private fun applyTrackpadState() {
        setTrackpadEnabled(!trackpadDisabled)
    }

    private fun setTrackpadEnabled(enabled: Boolean): Boolean {
        val connection = connection ?: openConnection() ?: return false
        val data =
            byteArrayOf(
                TRACKPAD_REPORT_ID,
                TRACKPAD_PADDING,
                TRACKPAD_FUNCTION,
                if (enabled) TRACKPAD_ENABLE else TRACKPAD_DISABLE,
            )

        if (connection.bulkTransfer(endpoint, data, data.size, USB_TIMEOUT_MS) != data.size) {
            Log.e(TAG, "Failed to ${if (enabled) "enable" else "disable"} trackpad")
            closeConnection()
            return false
        }

        return true
    }

    private fun openConnection(): UsbDeviceConnection? {
        val device = usbManager.deviceList.values.firstOrNull { it.isAmazonKeyboard } ?: return null
        if (device.interfaceCount <= TRACKPAD_INTERFACE) return null

        val usbInterface = device.getInterface(TRACKPAD_INTERFACE)
        val endpoint =
            (0 until usbInterface.endpointCount)
                .map { usbInterface.getEndpoint(it) }
                .firstOrNull { it.direction == UsbConstants.USB_DIR_OUT } ?: return null

        val connection = usbManager.openDevice(device) ?: return null
        if (!connection.claimInterface(usbInterface, true)) {
            Log.e(TAG, "Failed to claim trackpad interface")
            connection.close()
            return null
        }

        this.connection = connection
        this.endpoint = endpoint

        return connection
    }

    private fun closeConnection() {
        connection?.close()
        connection = null
        endpoint = null
    }

    private val Intent.usbDevice: UsbDevice?
        get() = getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice::class.java)

    private val InputDevice?.isAmazonKeyboard: Boolean
        get() = this != null && vendorId == VENDOR_ID && productId == PRODUCT_ID

    private val InputDevice?.isFireTvRemote: Boolean
        get() = this != null && vendorId == RemoteAppButtons.VENDOR_ID

    private val UsbDevice?.isAmazonKeyboard: Boolean
        get() = this != null && vendorId == VENDOR_ID && productId == PRODUCT_ID

    companion object {
        private const val TAG = "AmazonKeyHandler"

        private const val VENDOR_ID = 0x1949
        private const val PRODUCT_ID = 0x042b

        private const val SCANCODE_FILES = 144
        private const val SCANCODE_TOGGLE_TRACKPAD = 192
        private const val SCANCODE_SPLIT_SCREEN = 194

        private const val SHORTCUT_META_STATE =
            KeyEvent.META_META_ON or
                KeyEvent.META_META_LEFT_ON or
                KeyEvent.META_CTRL_ON or
                KeyEvent.META_CTRL_LEFT_ON

        private const val DISABLE_TRACKPAD = "disable_trackpad"

        private const val TRACKPAD_INTERFACE = 3
        private const val TRACKPAD_REPORT_ID: Byte = 0x10
        private const val TRACKPAD_PADDING: Byte = 0x0b
        private const val TRACKPAD_FUNCTION: Byte = 0x01
        private const val TRACKPAD_ENABLE: Byte = 0x01
        private const val TRACKPAD_DISABLE: Byte = 0x02

        private const val USB_TIMEOUT_MS = 500
    }
}

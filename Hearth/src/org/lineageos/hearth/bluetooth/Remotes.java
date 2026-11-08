/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.bluetooth;

import android.app.ActivityManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.hardware.input.InputManager;
import android.os.ParcelUuid;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.view.InputDevice;
import android.view.KeyEvent;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Set;

import org.lineageos.hearth.R;

final class Remotes {

    static final int AMAZON_COMPANY_ID = 0x0171;
    static final int AMAZON_GAME_CONTROLLER_PID = 0x0419;

    private static final String PROP_AUTO = "persist.hearth.bt_autopair";

    private static final String TV_SETTINGS = "com.android.tv.settings";
    private static final String ADD_ACCESSORY =
            "com.android.tv.settings.accessories.AddAccessoryActivity";
    private static final String PAIRING_DIALOG =
            "com.android.tv.settings.accessories.BluetoothPairingDialog";

    private static final int MAJOR_PERIPHERAL = BluetoothClass.Device.Major.PERIPHERAL;
    private static final int TV_SETTINGS_MINOR_MASK = 0xcc;

    private static final ParcelUuid HID =
            ParcelUuid.fromString("00001124-0000-1000-8000-00805f9b34fb");
    private static final ParcelUuid HOGP =
            ParcelUuid.fromString("00001812-0000-1000-8000-00805f9b34fb");

    private Remotes() {}

    static boolean isAutoPairingEnabled() {
        return SystemProperties.getBoolean(PROP_AUTO, true);
    }

    // Input devices

    static boolean hasConnectedInput(Context context) {
        InputManager im = context.getSystemService(InputManager.class);
        for (int id : im.getInputDeviceIds()) {
            InputDevice device = im.getInputDevice(id);
            if (device == null || device.isVirtual() || !device.isExternal()) {
                continue;
            }
            int sources = device.getSources();
            if (hasSource(sources, InputDevice.SOURCE_DPAD)
                    || hasSource(sources, InputDevice.SOURCE_GAMEPAD)
                    || hasSource(sources, InputDevice.SOURCE_KEYBOARD)) {
                return true;
            }
        }
        return false;
    }

    private static boolean hasSource(int sources, int source) {
        return (sources & source) == source;
    }

    static boolean hasBondedInput(BluetoothAdapter adapter) {
        Set<BluetoothDevice> bonded = adapter.getBondedDevices();
        if (bonded == null) {
            return false;
        }
        for (BluetoothDevice device : bonded) {
            if (isPeripheral(device.getBluetoothClass())) {
                return true;
            }
            ParcelUuid[] uuids = device.getUuids();
            if (uuids == null) {
                continue;
            }
            for (ParcelUuid uuid : uuids) {
                if (HID.equals(uuid) || HOGP.equals(uuid)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Device classes

    static boolean isPeripheral(BluetoothClass cls) {
        return cls != null && cls.getMajorDeviceClass() == MAJOR_PERIPHERAL;
    }

    static boolean isKeyboard(BluetoothClass cls) {
        return isPeripheral(cls) && (cls.getDeviceClass() & 0x40) != 0;
    }

    static boolean isInputClass(BluetoothClass cls) {
        return isPeripheral(cls) && (cls.getDeviceClass() & TV_SETTINGS_MINOR_MASK) != 0;
    }

    static boolean isPairingCandidate(BluetoothDevice device) {
        BluetoothClass cls = device.getBluetoothClass();
        if (isPeripheral(cls)) {
            return true;
        }
        return device.getType() == BluetoothDevice.DEVICE_TYPE_LE
                && (cls == null || cls.getMajorDeviceClass()
                        == BluetoothClass.Device.Major.UNCATEGORIZED);
    }

    // TvSettings

    static boolean isPairingScreenOnTop(Context context) {
        ComponentName top = getTopActivity(context);
        return top != null && TV_SETTINGS.equals(top.getPackageName())
                && (ADD_ACCESSORY.equals(top.getClassName())
                        || PAIRING_DIALOG.equals(top.getClassName()));
    }

    static boolean isAddAccessoryOnTop(Context context) {
        ComponentName top = getTopActivity(context);
        return top != null && TV_SETTINGS.equals(top.getPackageName())
                && ADD_ACCESSORY.equals(top.getClassName());
    }

    private static ComponentName getTopActivity(Context context) {
        ActivityManager am = context.getSystemService(ActivityManager.class);
        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(1);
        if (tasks == null || tasks.isEmpty()) {
            return null;
        }
        return tasks.get(0).topActivity;
    }

    static void sendBack(Context context) {
        InputManager im = context.getSystemService(InputManager.class);
        long now = SystemClock.uptimeMillis();
        im.injectInputEvent(new KeyEvent(now, now, KeyEvent.ACTION_DOWN,
                KeyEvent.KEYCODE_BACK, 0), InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
        im.injectInputEvent(new KeyEvent(now, now, KeyEvent.ACTION_UP,
                KeyEvent.KEYCODE_BACK, 0), InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
    }

    // Advertising data

    static final class Advert {
        int flags;
        int appearance = -1;
        boolean hidListed;
        boolean hidComplete;
        int amazonPid = -1;
        String name;
        String raw = "";

        static Advert parse(byte[] data) {
            Advert adv = new Advert();
            if (data == null) {
                return adv;
            }
            int pos = 0;
            while (pos < data.length) {
                int len = data[pos] & 0xff;
                if (len == 0 || pos + 1 + len > data.length) {
                    break;
                }
                int type = data[pos + 1] & 0xff;
                int start = pos + 2;
                int end = pos + 1 + len;
                switch (type) {
                    case 0x01:
                        if (end > start) {
                            adv.flags = data[start] & 0xff;
                        }
                        break;
                    case 0x02:
                    case 0x03:
                        for (int i = start; i + 1 < end; i += 2) {
                            if (u16(data, i) == 0x1812) {
                                adv.hidListed = true;
                                adv.hidComplete |= type == 0x03;
                            }
                        }
                        break;
                    case 0x08:
                    case 0x09:
                        if (adv.name == null || type == 0x09) {
                            adv.name = new String(data, start, end - start,
                                    StandardCharsets.UTF_8);
                        }
                        break;
                    case 0x19:
                        if (end - start >= 2) {
                            adv.appearance = u16(data, start);
                        }
                        break;
                    case 0xff:
                        if (end - start >= 2 && u16(data, start) == AMAZON_COMPANY_ID) {
                            adv.amazonPid = end - start >= 4
                                    ? ((data[start + 2] & 0xff) << 8) | (data[start + 3] & 0xff)
                                    : 0;
                        }
                        break;
                }
                pos = end;
            }
            adv.raw = hex(data, pos);
            return adv;
        }

        boolean isDiscoverable() {
            return (flags & 0x03) != 0;
        }

        boolean isAmazon() {
            return amazonPid >= 0;
        }

        boolean isInput() {
            return hidListed || (deviceClass() & 0x1f00) == 0x0500;
        }

        int deviceClass() {
            switch (appearance) {
                case 0x0180:
                    return 0x050c;
                case 0x02c0:
                case 0x03c0:
                case 0x03c8:
                    return 0x0500;
                case 0x03c1:
                    return 0x0540;
                case 0x03c2:
                    return 0x0580;
                case 0x03c3:
                    return 0x0504;
                case 0x03c4:
                    return 0x0508;
                case 0x03c5:
                    return 0x0514;
                case 0x03c6:
                    return 0x0518;
                case 0x03c7:
                    return 0x051c;
                case -1:
                    return hidComplete ? 0x0500 : 0;
                default:
                    return 0;
            }
        }

        boolean isListedByTvSettings(BluetoothDevice device) {
            int cls = deviceClass();
            return isDiscoverable() && (name != null || device.getName() != null)
                    && (cls & 0x1f00) == 0x0500 && (cls & TV_SETTINGS_MINOR_MASK) != 0;
        }

        String label(Context context, BluetoothDevice device) {
            if (amazonPid == AMAZON_GAME_CONTROLLER_PID) {
                return context.getString(R.string.amazon_game_controller);
            }
            if (isAmazon()) {
                return context.getString(R.string.amazon_remote);
            }
            String deviceName = device.getName();
            return deviceName != null ? deviceName : name != null ? name : device.getAddress();
        }

        @Override
        public String toString() {
            return String.format("flags=0x%02x appearance=0x%04x hid=%b name=%s pid=0x%04x raw=%s",
                    flags, appearance & 0xffff, hidListed, name, amazonPid & 0xffff, raw);
        }

        private static int u16(byte[] data, int pos) {
            return (data[pos] & 0xff) | ((data[pos + 1] & 0xff) << 8);
        }

        private static String hex(byte[] data, int len) {
            StringBuilder sb = new StringBuilder(len * 2);
            for (int i = 0; i < len; i++) {
                sb.append(String.format("%02x", data[i] & 0xff));
            }
            return sb.toString();
        }
    }
}

#!/vendor/bin/sh
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

head -c 12 /proc/idme/bt_mac_addr | sed 's/../&:/g; s/:$//' > /data/vendor/bluetooth/bdaddr

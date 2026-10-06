/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.keyhandler

import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.UserHandle
import android.util.Log
import android.view.InputDevice
import java.io.File

class RemoteAppButtons(private val context: Context) {
    fun launch(device: InputDevice, scanCode: Int) {
        val button = BUTTONS[scanCode] ?: return
        val sku = readSku(device)
        val tags = sku?.let { REMOTE_APP_SKU_TAGS[it] } ?: REMOTE_APP_DEFAULT_TAGS
        val tag = tags.getOrNull(button) ?: REMOTE_APP_DEFAULT_TAGS.getOrNull(button)

        Log.d(TAG, "Button ${button + 1} of remote SKU ${sku?.let { "0x%04x".format(it) }} -> $tag")

        val intent = tag?.let(::intentFor)
        if (intent == null) {
            Log.i(TAG, "No app for $tag")
            return
        }

        try {
            context.startActivityAsUser(intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK), UserHandle.CURRENT)
        } catch (e: ActivityNotFoundException) {
            Log.w(TAG, "Nothing handles $tag", e)
        }
    }

    private fun intentFor(tag: String): Intent? {
        val pm = context.packageManager
        val candidates =
            when (tag) {
                "apps" ->
                    listOf(
                        Intent(Intent.ACTION_ALL_APPS),
                        Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_HOME),
                    )
                "download" ->
                    listOf(
                        Intent.makeMainSelectorActivity(Intent.ACTION_MAIN, Intent.CATEGORY_APP_MARKET)
                    )
                "silk" ->
                    listOf(
                        Intent.makeMainSelectorActivity(Intent.ACTION_MAIN, Intent.CATEGORY_APP_BROWSER)
                    )
                else -> {
                    val packages = PACKAGES[tag].orEmpty()
                    packages.mapNotNull {
                        pm.getLeanbackLaunchIntentForPackage(it) ?: pm.getLaunchIntentForPackage(it)
                    } +
                        packages.take(1).map {
                            Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=$it"))
                        }
                }
            }

        return candidates.firstOrNull { it.resolveActivity(pm) != null }
    }

    // The remote reports its SKU as the input device version
    private fun readSku(device: InputDevice): Int? =
        File(SYSFS_INPUT)
            .listFiles { file -> file.name.startsWith("input") }
            ?.firstOrNull {
                it.readId("vendor") == device.vendorId &&
                    it.readId("product") == device.productId &&
                    File(it, "name").readText().trim() == device.name
            }
            ?.readId("version")

    private fun File.readId(name: String): Int? =
        runCatching { File(this, "id/$name").readText().trim().toInt(16) }.getOrNull()

    companion object {
        private const val TAG = "AmazonKeyHandler"

        private const val SYSFS_INPUT = "/sys/class/input"

        const val VENDOR_ID = 0x0171

        // Scan codes of KEYCODE_APP_1 onwards, as mapped by FireOS' key layouts
        private val BUTTONS =
            mapOf(
                249 to 0,
                250 to 1,
                251 to 2,
                253 to 3,
                316 to 4,
                672 to 5,
                673 to 6,
                674 to 7,
                675 to 8,
                688 to 9,
                744 to 0,
                745 to 1,
                746 to 2,
                747 to 3,
            )

        // Android TV packages for FireOS' app tags, in order of preference
        private val PACKAGES =
            mapOf(
                "abema" to listOf("tv.abema"),
                "amazonmusic" to listOf("com.amazon.music.tv"),
                "crave" to listOf("ca.bellmedia.cravetv"),
                "ctv" to listOf("ca.bellmedia.ctv"),
                "dazn" to listOf("com.dazn"),
                "directv" to listOf("com.att.tv"),
                "disney" to listOf("com.disney.disneyplus"),
                "explorefvp" to listOf("uk.co.freeview.explore"),
                "freely" to listOf("uk.co.freeview.explore"),
                "hbo" to listOf("com.wbd.stream", "com.hbo.hbonow"),
                "hulu" to listOf("com.hulu.livingroomplus"),
                "imdb" to listOf("com.amazon.amazonvideo.livingroom"),
                "livetv" to listOf("com.google.android.tv"),
                "netflix" to listOf("com.netflix.ninja"),
                "opappfreely" to listOf("uk.co.freeview.explore"),
                "peacock" to listOf("com.peacocktv.peacockandroid"),
                "primevideo" to listOf("com.amazon.amazonvideo.livingroom"),
                "psvue" to listOf("com.google.android.tv"),
                "sky" to listOf("br.com.skymais"),
                "sony" to listOf("com.sonyliv"),
                "tver" to listOf("jp.co.tver.tvapp"),
                "tvnow" to listOf("de.cbc.tvnow"),
                "tvplus" to listOf("com.tcl.tv.plus"),
                "unext" to listOf("jp.unext.mediaplayer"),
                "youtube" to listOf("com.google.android.youtube.tv"),
                "zee5" to listOf("com.graymatrix.did"),
            )

        fun handles(scanCode: Int) = scanCode in BUTTONS
    }
}

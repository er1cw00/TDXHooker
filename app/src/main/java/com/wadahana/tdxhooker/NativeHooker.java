package com.wadahana.tdxhooker;

import android.util.Log;

import de.robv.android.xposed.XposedBridge;

public class NativeHooker {
    static String kTag = "HookLoader";
    static boolean mInitialized = false;
    static void load() {
        if (mInitialized) {
            Log.i(kTag, "NativeHooker has been initalized!");
        }
        Log.i(kTag, "NativeHooker jni loading!");
        try {
            System.loadLibrary( "tdx-hooker");
            mInitialized = true;
        } catch(Exception e) {
            XposedBridge.log("load NativeHooker jni fail " + e.toString());
        }
    }
    static public native int test();
    static public native int hook();
}

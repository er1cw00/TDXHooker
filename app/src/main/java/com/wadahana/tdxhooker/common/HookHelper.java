package com.wadahana.tdxhooker.common;

import android.util.Log;

import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;

public class HookHelper {
    final static String kTag = "HookHelper";
    static public void dumpCallStack() {
        java.util.Map<Thread, StackTraceElement[]> ts = Thread.getAllStackTraces();
        StackTraceElement[] elements = ts.get(Thread.currentThread());
        for (StackTraceElement e : elements) {
            //Log.i(kTag, e.toString());
            XposedBridge.log(kTag + " " + e.toString());
        }
    }
}

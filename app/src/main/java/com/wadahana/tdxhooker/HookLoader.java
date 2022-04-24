package com.wadahana.tdxhooker;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class HookLoader implements IXposedHookLoadPackage {
    static String kTag = "HookLoader";
    final String packgeName = "com.tdx.AndroidNew";
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if (!lpparam.packageName.equals(packgeName)) {
            XposedBridge.log("not hook package:" + lpparam.packageName);
            return;
        }
        XposedBridge.log("hook package:" + lpparam.packageName);
    }
}
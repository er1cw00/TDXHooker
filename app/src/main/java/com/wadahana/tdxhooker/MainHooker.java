package com.wadahana.tdxhooker;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class MainHooker implements IXposedHookLoadPackage {
    static String kTag = "MainHooker";
    final String packgeName = "com.tdx.AndroidNew";
    private NativeHooker nativeHooker = new NativeHooker();
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if (!lpparam.packageName.equals(packgeName)) {
            XposedBridge.log("TDXHooker not hook package:" + lpparam.packageName);
            return;
        }
        XposedBridge.log("TDXHooker init");
        XposedBridge.log("TDXHooker hook package:" + lpparam.packageName);
        NativeHooker.load();
        nativeHooker.hook();
    }
}
package com.wadahana.tdxhooker;

import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.wadahana.tdxhooker.common.HookHelper;

public class MainActivity extends AppCompatActivity {
    static String kTag = "TdxHooker";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.i(kTag, "MainActivity onCreate");
        //Log.i(kTag, "IMEI:" + getIMEI(this));
    }
    public static final String getIMEI(Context context) {
        HookHelper.dumpCallStack();
        try {
            //实例化TelephonyManager对象
            TelephonyManager telephonyManager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            //获取IMEI号
            String imei = telephonyManager.getDeviceId();
            //在次做个验证，也不是什么时候都能获取到的啊
            if (imei == null) {
                imei = "";
            }
            return imei;
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }
}
package com.wadahana.tdxhooker;

import android.content.Context;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.wadahana.tdxhooker.common.HookHelper;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {
    static String kTag = "TdxHooker";
    private Button updateButton;
    private Button testButton;
    private NativeHooker nativeHooker = new NativeHooker();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        updateButton = (Button)findViewById(R.id.UpdateButton);
        updateButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                copyLibrarys();
            }
        });

        testButton = (Button)findViewById(R.id.TestButton);
        testButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                NativeHooker.load();
                nativeHooker.hook();
            }
        });
        Log.i(kTag, "MainActivity onCreate");

    }
    public void copyLibrarys() {
        String appPath = getApplicationContext().getPackageResourcePath();
        int i = appPath.indexOf("/base.apk");
        if (i == -1) {
            Log.e(kTag, "appPath: " + appPath + " malformed!");
            return ;
        }
        String basePath = appPath.substring(0, i);
        String libPath = basePath + "/lib/arm64";
        String dstPath = Environment.getRootDirectory().getPath() ;
        Log.i(kTag, "MainActivity app path:" + libPath);
        Log.i(kTag, "MainActivity dst path:" + dstPath);
        //copy(libPath + "/libtdx-hooker.so", dstPath + "/lib64/libtdx-hooker.so");
        String cmd = "cp " + libPath + "/libtdx-hooker.so" + " " + dstPath + "/lib64/libtdx-hooker.so" + "\n";
        Log.i(kTag, "MainActivity cmd: " + cmd);
        try {
            Process p = Runtime.getRuntime().exec("su");
            DataOutputStream dos = new DataOutputStream(p.getOutputStream());
            dos.writeBytes(cmd);
            dos.flush();
        } catch (IOException e) {
            Log.e(kTag, "MainActivity copy libtdxhooker.so fail: " + e);
        }
        return ;
    }
    public void copy(String src, String dst) {
        try {
            File inFile = new File(src);
            File outFile = new File(dst);
            InputStream inputStream = new FileInputStream(inFile);
            OutputStream outputStream = new FileOutputStream(outFile);
            byte[] buffer= new byte[1024];
            while(true) {
                int len = inputStream.read(buffer);
                if (len == -1) {
                    break;
                }
                outputStream.write(buffer,0, len);
            }
            inputStream.close();
            outputStream.close();
        } catch (IOException e) {
            Log.e(kTag, "MainActivity copy file fail: " + e);
        }
        return;
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
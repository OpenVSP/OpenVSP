package com.angelcode.android;

import android.app.Activity;
import android.util.Log;
import android.os.Bundle;

public class AndroidTests extends Activity
{
	Thread th; 
	
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

    	Log.i("AS_ANDROID_JAVA", "ON CREATE");
    	
        th = new Thread(new Runnable() {
            public void run() {
            	runTests();
            }
        });
        th.start();
    }
    
    public native void runTests();

    static {
        System.loadLibrary("AndroidTests");
    }
}
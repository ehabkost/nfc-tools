package net.raisama.nfc.mfoc;

import android.app.Activity;
import android.os.Bundle;
import net.raisama.nfc.mfoc.NativeImplementation;

public class AndroidMfocActivity extends Activity {
	FakeStdio fake_stdio;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        NativeImplementation.singleton().setRunningActivity(this);
        NativeImplementation.singleton().callMain();
    }
}
package net.raisama.nfc.mfoc;

import android.app.Activity;
import android.os.Bundle;
import net.raisama.nfc.mfoc.NativeImplementation;
import android.widget.TextView;

public class AndroidMfocActivity extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        NativeImplementation i = new NativeImplementation();
        TextView t = (TextView)findViewById(R.id.sumResult);
        t.setText(java.lang.String.format("%d", i.sum(10, 20)));
        
        i.callMain();
    }
}
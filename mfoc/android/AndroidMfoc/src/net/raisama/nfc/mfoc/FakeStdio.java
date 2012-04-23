package net.raisama.nfc.mfoc;

import 	android.widget.TextView;

public class FakeStdio {
	AndroidMfocActivity runningActivity;
	
	public void printUiMessage(String s)
	{
		TextView t = (TextView)runningActivity.findViewById(R.id.textOutput);
		t.append(s);
	}
	
	public void setRunningActivity(AndroidMfocActivity a)
	{
		runningActivity = a;
	}
	public FakeFile fopen(String filename, String mode)
	{
		return new UiFakeFile(this, filename);
	}
}

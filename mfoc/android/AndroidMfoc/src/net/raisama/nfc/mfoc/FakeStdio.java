package net.raisama.nfc.mfoc;

public class FakeStdio {
	AndroidMfocActivity runningActivity;
	
	public void printUiMessage(String s)
	{
		runningActivity.printUiMessage(s);
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

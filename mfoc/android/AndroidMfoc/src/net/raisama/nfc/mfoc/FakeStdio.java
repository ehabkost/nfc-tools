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
		//runningActivity.printUiMessage(String.format("fopen(%s, %s) called", filename, mode));
		return new DataFakeFile(runningActivity, filename);
	}
}

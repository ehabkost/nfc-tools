package net.raisama.nfc.mfoc;

public class NativeImplementation {

	/** Call the main() function of fmoc
	 */
	FakeFile stdout;
	FakeFile stderr;
	FakeStdio fake_stdio;
	FakeLibNFC fake_nfc;
	
	static NativeImplementation singleton;
	
	native public int nativeMain();
	native public void setFakeStdioObject(FakeStdio f, FakeFile stdout, FakeFile stderr);
	native public void setFakeLibNFC(FakeLibNFC f);
	
	public void callMain()
	{
		int r = nativeMain();
		fake_stdio.printUiMessage(String.format("program exited with status %d\n", r));
	}

	public void setRunningActivity(AndroidMfocActivity a)
	{
		fake_stdio.setRunningActivity(a);
		fake_nfc = new FakeLibNFC(a);
        setFakeLibNFC(fake_nfc);
	}
	
	static public NativeImplementation singleton()
	{
		return singleton;
	}

	private NativeImplementation() {
		fake_stdio = new FakeStdio();
		stdout = new UiFakeFile(fake_stdio, "stdout");
		stderr = new UiFakeFile(fake_stdio, "stderr");
        setFakeStdioObject(fake_stdio, stdout, stderr);
	}
	
	static {
		  System.loadLibrary("mfoc");
		  singleton = new NativeImplementation();
	}
}
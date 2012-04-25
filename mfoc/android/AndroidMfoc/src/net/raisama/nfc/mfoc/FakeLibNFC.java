package net.raisama.nfc.mfoc;

public class FakeLibNFC {
	AndroidMfocActivity mActivity;
	
	public FakeLibNFC(AndroidMfocActivity aActivity)
	{
		mActivity = aActivity;
	}
	
	public NfcDevice nfc_open()
	{
		return new NfcDevice(mActivity);
	}
}
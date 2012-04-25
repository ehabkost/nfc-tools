package net.raisama.nfc.mfoc;

public class DataFakeFile extends FakeFile {

	AndroidMfocActivity mActivity;
	String mFilename;
	
	public DataFakeFile(AndroidMfocActivity aActivity, String aFilename) {
		mActivity = aActivity;
		mFilename = aFilename;
	}
	
	@Override
	public void print(byte[] s) {
		mActivity.printUiMessage(String.format("Writing data to %s: ", mFilename));
		for (int i = 0; i < s.length; i++) {
			mActivity.printUiMessage(String.format(" %02x", s[i]));
		}
		mActivity.printUiMessage("\n");
	}

}

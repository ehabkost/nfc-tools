package net.raisama.nfc.mfoc;

public class NfcDevice {

	AndroidMfocActivity mActivity;
	boolean mHandleCrc = true;
	boolean mHandleParity = true;
	boolean mEasyFraming = true;
	
	final static int NP_HANDLE_CRC = 3;
	final static int NP_HANDLE_PARITY = 4;
	final static int NP_ACTIVATE_FIELD = 5;
	final static int NP_ACTIVATE_CRYPTO1 = 6;
	final static int NP_INFINITE_SELECT = 7;
	final static int NP_EASY_FRAMING = 11;
	
	public NfcDevice(AndroidMfocActivity aActivity)
	{
		mActivity = aActivity;
	}
	
	public int initiator_init()
	{
		return 0;
	}
	
	public NfcTarget initiator_select_passive_target()
	{
		return new NfcTarget(mActivity);
	}
	
	public void set_property(int property, boolean value)
	{
		switch (property)
		{
			case NP_HANDLE_CRC:
				mHandleCrc = value;
			break;
			case NP_HANDLE_PARITY:
				mHandleParity = value;
			break;
			case NP_EASY_FRAMING:
				mEasyFraming = value;
			break;
			case NP_INFINITE_SELECT:
			case NP_ACTIVATE_FIELD:
				/* Properties that are simply ignored */
			break;
			default:
				mActivity.printUiMessage(String.format("Unknown property: %d\n", property));
		}
	}
}
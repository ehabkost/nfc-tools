package net.raisama.nfc.mfoc;

import java.lang.reflect.Method;

import android.nfc.Tag;
import android.nfc.tech.MifareClassic;

public class NfcDevice {

	AndroidMfocActivity mActivity;
	boolean mHandleCrc = true;
	boolean mHandleParity = true;
	boolean mEasyFraming = true;
	Tag tag;
	MifareClassic mfc;
	Method transceiveMethod;
	static final int phNfc_eMifareRaw = 0x00;
	
	final static int NP_HANDLE_CRC = 3;
	final static int NP_HANDLE_PARITY = 4;
	final static int NP_ACTIVATE_FIELD = 5;
	final static int NP_ACTIVATE_CRYPTO1 = 6;
	final static int NP_INFINITE_SELECT = 7;
	final static int NP_EASY_FRAMING = 11;
	
	public NfcDevice(AndroidMfocActivity aActivity)
	{
		mActivity = aActivity;
		tag = aActivity.tag();
		mfc = aActivity.mfc();
		try {
			transceiveMethod = MifareClassic.class.getSuperclass().getDeclaredMethod("transceive", new Class[] { byte[].class, boolean.class });
		} catch (NoSuchMethodException e) {
			mActivity.printUiMessage("couldn't find transceive(byte[], boolean) method :(\n");
			transceiveMethod = null;
		}
		transceiveMethod.setAccessible(true);
	}
	
	public int initiator_init()
	{
		if (transceiveMethod == null)
			return -1;
		return 0;
	}
	
	public NfcTarget initiator_select_passive_target()
	{
		return new NfcTarget(mActivity);
	}
	
	public byte[] initiator_transceive_bytes(byte[] tx, int timeout) throws java.lang.IllegalAccessException
	{
	
		/*
		mActivity.printUiMessage("Tx:");
		for (int i = 0; i < tx.length; i++) {
			mActivity.printUiMessage(String.format(" %02x", tx[i]));
		}
		mActivity.printUiMessage("\n");
		*/
		
		byte[] response;
		byte[] realtx;
		boolean raw;
		if (mEasyFraming) {
			// Easy Framing use the "command mode" (raw=false) that makes the device handle the auth details itself
			raw = false;
			realtx = tx;
		} else {
			// non-easy framing will try to use raw mode, but the specific method depends on CRC:
			if (!mHandleCrc) {
				// raw=true sets CRC automatically and we can't avoid it, so emulate raw mode using phNfc_eMifareRaw
				// (see com_android_nfc_NativeNfcTag_doTransceive() at platform/packages/apps/Nfc on Android source)
				raw = false;
				realtx = new byte[tx.length+2];
				realtx[0] = phNfc_eMifareRaw;
				realtx[0] = 0; // address
				System.arraycopy(tx, 0, realtx, 2, tx.length);
			} else {
				// auto-CRC is set? good, so we can simply use the raw mode as-is (I guess?)
				raw = true;
				realtx = tx;
			}
		}
		try {
			response = (byte[])transceiveMethod.invoke(mfc, realtx, raw);
		} catch (java.lang.reflect.InvocationTargetException ie) {
			//Throwable e = ie.getCause();
			//mActivity.printUiMessage(String.format("transceive(byte[], boolean) raised: %s: %s\n", e.getClass().getName(), e.getMessage()));
			return null;			
		} catch (java.lang.IllegalAccessException e) {
			mActivity.printUiMessage(String.format("couldn't call transceive(byte[], boolean) method: %s: %s\n", e.getClass().getName(), e.getMessage()));
			throw e;
		}	
		
		return response;
	}
	
	public void set_property(int property, boolean value) throws Exception
	{
		switch (property)
		{
			case NP_HANDLE_CRC:
				mHandleCrc = value;
			break;
			case NP_HANDLE_PARITY:
				if (!value) {
					mActivity.printUiMessage("we can't implement NP_HANDLE_PARITY=false mode on Android. Sorry.\n");
					throw new Exception("NP_HANDLE_PARITY unsupported");
				}
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
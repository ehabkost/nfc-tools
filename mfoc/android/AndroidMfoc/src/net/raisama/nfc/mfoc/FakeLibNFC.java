package net.raisama.nfc.mfoc;

public class FakeLibNFC {
	public NfcDevice nfc_open()
	{
		return new NfcDevice();
	}
}
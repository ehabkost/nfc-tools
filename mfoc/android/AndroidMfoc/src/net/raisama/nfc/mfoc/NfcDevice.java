package net.raisama.nfc.mfoc;

public class NfcDevice {

	public int initiator_init()
	{
		return 0;
	}
	
	public NfcTarget initiator_select_passive_target()
	{
		return new NfcTarget();
	}
}

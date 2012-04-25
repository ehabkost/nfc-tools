package net.raisama.nfc.mfoc;
import android.nfc.Tag;
import android.nfc.tech.NfcA;

public class NfcTarget {
	AndroidMfocActivity mActivity;
	Tag tag;
	NfcA nfca;
	
	
	public NfcTarget(AndroidMfocActivity aActivity)
	{
		mActivity = aActivity;
		tag = aActivity.tag();
		nfca = NfcA.get(tag);
	}
	
	
	// I don't know why it's inverted, but libnfc/nfc-utils is expecting
	// the ATQA bytes to be in the reverse order:
	public int getAtqa0()
	{
		return nfca.getAtqa()[1]; 
	}
	
	public int getAtqa1()
	{
		return nfca.getAtqa()[0];
	}
	
	public int getSak()
	{
		return nfca.getSak();
	}
	
	byte[] getUid()
	{
		return tag.getId();
	}
	
	byte[] getAts()
	{
		// I don't know how the heck to get this information from Android
		return new byte[] {};
	}
	
	static final int NBR_UNDEFINED = 0;
	
	public int getBaudRate()
	{
		// I don't know if I can get this inf from Android, either
		return NBR_UNDEFINED;
	}
}

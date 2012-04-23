package net.raisama.nfc.mfoc;
import java.io.UnsupportedEncodingException;;

/** FakeFile implementation that prints messages to the UI
 * @author ehabkost
 *
 */
public class UiFakeFile extends FakeFile {
	String mPrefix;
	FakeStdio mStdio;
	
	public UiFakeFile(FakeStdio stdio, String prefix)
	{
		mPrefix = prefix;
		mStdio = stdio;
	}
	public void print(byte[] s)
	{
		//mStdio.printUiMessage(mPrefix);
		//mStdio.printUiMessage(": ");
		try {
			mStdio.printUiMessage(new String(s, "UTF-8"));
		} catch (UnsupportedEncodingException e) {
			mStdio.printUiMessage("INTERNAL ERROR. This shouldn't have happened, sorry.");
		}
	}
}

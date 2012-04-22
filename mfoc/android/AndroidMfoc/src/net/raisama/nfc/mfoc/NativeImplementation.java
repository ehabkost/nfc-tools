package net.raisama.nfc.mfoc;

public class NativeImplementation {
	native public int sum(int a, int b);
	
	/** Call the main() function of fmoc
	 */
	native public void callMain();

	static {
		  System.loadLibrary("mfoc");
	}

}
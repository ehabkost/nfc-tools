package net.raisama.nfc.mfoc;

public class NativeImplementation {
	native public int sum(int a, int b);

	static {
		  System.loadLibrary("mfoc");
	}

}
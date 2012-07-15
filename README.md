A (tentative) Android mfoc port
===============================

This was the result of me trying to port the mfoc tool to Android, by simply running the C code using NDK, under a small wrapper that provides the libc and libnfc calls.

Status
------

Don't be so excited: all this code can do is to (poorly) probe for default keys. I don't know if it's completely working, as it didn't get much testing.

It **can't** run the actual cracking code, because it depends on the ability of disabling parity check and sending/receiving raw parity bits, and this doesn't seem to be possible using the Android API.


APK
---

If you still want to see it running, you can get an APK at:
https://github.com/downloads/ehabkost/nfc-tools/AndroidMfoc.apk

/** Fake libnfc implementation
 *
 * See http://code.google.com/p/libnfc/ for libnfc source and docs.
 *
 * Copyright (C) 2012, Eduardo Habkost
 * Includes parts of libnfc, Copyright (C) 2009, Roel Verdult
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <nfc/nfc.h>

#include <stdlib.h>
#include <stdio.h>

#include <jni.h>

#define IMPLEMENT_ME do { fprintf(stderr, "ERROR: function %s not implemented. exiting.\n", __FUNCTION__); abort(); } while (0)

jobject fake_libnfc;


/* Helper to call simple int method */
static int call_method_int(JNIEnv *env, jobject obj, const char *name)
{
	int r;
	jclass cls = (*env)->GetObjectClass(env, obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, name, "()I");
	if (mid == NULL)
		abort();
	r = (*env)->CallIntMethod(env, obj, mid);
	(*env)->DeleteLocalRef(env, cls);
	return r;
}

JNIEXPORT void JNICALL Java_net_raisama_nfc_mfoc_NativeImplementation_setFakeLibNFC
  (JNIEnv *env, jobject obj, jobject nfc)
{
	fake_libnfc = (*env)->NewGlobalRef(env, nfc);
}

void nfc_perror (const nfc_device *pnd, const char *s)
{
	fprintf(stderr, "nfc perror: %s\n", s?s:"(null)");;
}

struct nfc_device
{
	char *connstring;
	jobject obj;
};

nfc_device *nfc_open (nfc_context *context, const nfc_connstring connstring)
{
	fprintf(stderr, "nfc_open: connstring: %s\n", connstring?connstring:"(null)");
	nfc_device *newdev = malloc(sizeof(nfc_device));
	if (!newdev)
		abort();

	memset(newdev, 0, sizeof(*newdev));
	if (connstring)
		newdev->connstring = strdup(connstring);

	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, fake_libnfc);
    jmethodID mid = (*env)->GetMethodID(env, cls, "nfc_open", "()Lnet/raisama/nfc/mfoc/NfcDevice;");
	if (mid == NULL)
		abort();
    jobject obj = (*env)->CallObjectMethod(env, fake_libnfc, mid);
    if (!obj)
    	abort();
    newdev->obj = (*env)->NewGlobalRef(env, obj);

	(*env)->DeleteLocalRef(env, obj);
	(*env)->DeleteLocalRef(env, cls);
	return newdev;
}

void nfc_close (nfc_device *pnd)
{
	if (pnd->connstring)
		free(pnd->connstring);
	free(pnd);
}

int nfc_initiator_init (nfc_device *pnd)
{
	return call_method_int(global_env, pnd->obj, "initiator_init");
}

int nfc_initiator_select_passive_target (nfc_device *pnd, const nfc_modulation nm, const uint8_t *pbtInitData, const size_t szInitData, nfc_target *pnt)
{
	int ret;

	//fprintf(stderr, "nfc_initiator_select_passive_target called: %p, modulation: %d/%d, initdata: %p, szInitData: %ld, target: %p\n", pnd, (int)nm.nmt, (int)nm.nbr, pbtInitData, (long)szInitData, pnt);

	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, pnd->obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, "initiator_select_passive_target", "()Lnet/raisama/nfc/mfoc/NfcTarget;");
	if (mid == NULL)
		abort();
	jobject ti = (*env)->CallObjectMethod(env, pnd->obj, mid);
	(*env)->DeleteLocalRef(env, cls);

	cls = (*env)->GetObjectClass(env, ti);

	memset(pnt, 0, sizeof(*pnt));

	#define field(f, fn) pnt->f = (call_method_int(env, ti, #fn))

	field(nti.nai.abtAtqa[0], getAtqa0);
	field(nti.nai.abtAtqa[1], getAtqa1);
	field(nti.nai.btSak, getSak);

	mid = (*env)->GetMethodID(env, cls, "getUid", "()[B");
	jbyteArray uid = (*env)->CallObjectMethod(env, ti, mid);
	int uidlen = (*env)->GetArrayLength(env, uid);
	if (uidlen > 10) uidlen = 10;
	pnt->nti.nai.szUidLen = uidlen;
	(*env)->GetByteArrayRegion(env, uid, 0, uidlen, pnt->nti.nai.abtUid);

	mid = (*env)->GetMethodID(env, cls, "getAts", "()[B");
	jbyteArray ats = (*env)->CallObjectMethod(env, ti, mid);
	int atslen = (*env)->GetArrayLength(env, ats);
	if (atslen > 10) atslen = 10;
	pnt->nti.nai.szAtsLen = atslen;
	(*env)->GetByteArrayRegion(env, ats, 0, atslen, pnt->nti.nai.abtAts);

	pnt->nm.nmt = NMT_ISO14443A;
	field(nm.nbr, getBaudRate);

	(*env)->DeleteLocalRef(env, uid);
	(*env)->DeleteLocalRef(env, ats);
	(*env)->DeleteLocalRef(env, cls);
	(*env)->DeleteLocalRef(env, ti);
	return 0;
}


/* NOTE: we will probably need some tricks to be able to send raw commands
 * without CRC using the Android API. I hope the system allow us to do
 * that.  :(
 */


int nfc_initiator_transceive_bytes (nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, size_t *pszRx, int timeout)
{
	int r = 0;
	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, pnd->obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, "initiator_transceive_bytes", "([BI)[B");
	if (mid == NULL)
		abort();

	jbyteArray tx = (*env)->NewByteArray(env, szTx);
	(*env)->SetByteArrayRegion(env, tx, 0, szTx, pbtTx);
	jbyteArray rx = (*env)->CallObjectMethod(env, pnd->obj, mid, tx, timeout);
	if (!rx) {
		//fprintf(stderr, "null response from initiator_transceive_bytes\n");
		r = NFC_ERFTRANS;
		goto out;
	}
	if ((*env)->ExceptionCheck(env)) {
		(*env)->ExceptionClear(env);
		fprintf(stderr, "Exception on initiator_transceive_bytes\n");
		r = NFC_EIO;
		goto out;
	}
	size_t rxlen = (*env)->GetArrayLength(env, rx);
	if (rxlen > *pszRx)
		rxlen = *pszRx;

	(*env)->GetByteArrayRegion(env, rx, 0, rxlen, pbtRx);
out:
	(*env)->DeleteLocalRef(env, rx);
	(*env)->DeleteLocalRef(env, tx);
	(*env)->DeleteLocalRef(env, cls);
	return r;
}

int nfc_initiator_transceive_bits (nfc_device *pnd, const uint8_t *pbtTx, const size_t szTxBits, const uint8_t *pbtTxPar, uint8_t *pbtRx, uint8_t *pbtRxPar)
{
	IMPLEMENT_ME;
}

void nfc_init(nfc_context *context)
{
	fprintf(stderr, "nfc_init called\n");
}

void nfc_exit(nfc_context *context)
{
	fprintf(stderr, "nfc_exit called\n");
}


int nfc_device_set_property_bool (nfc_device *pnd, const nfc_property property, const bool bEnable)
{
	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, pnd->obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, "set_property", "(IZ)V");
	if (mid == NULL)
		abort();
	(*env)->CallVoidMethod(env, pnd->obj, mid, (jint)property, (bool)bEnable);
	(*env)->DeleteLocalRef(env, cls);
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->ExceptionClear(env);
		fprintf(stderr, "Exception when setting bool property\n");
		return NFC_EIO;
	}
	return 0;
}

static void iso14443a_crc (uint8_t *pbtData, size_t szLen, uint8_t *pbtCrc)
{
  uint8_t  bt;
  uint32_t wCrc = 0x6363;

  do {
    bt = *pbtData++;
    bt = (bt ^ (uint8_t) (wCrc & 0x00FF));
    bt = (bt ^ (bt << 4));
    wCrc = (wCrc >> 8) ^ ((uint32_t) bt << 8) ^ ((uint32_t) bt << 3) ^ ((uint32_t) bt >> 4);
  } while (--szLen);

  *pbtCrc++ = (uint8_t) (wCrc & 0xFF);
  *pbtCrc = (uint8_t) ((wCrc >> 8) & 0xFF);
}

void iso14443a_crc_append (uint8_t *pbtData, size_t szLen)
{
	iso14443a_crc (pbtData, szLen, pbtData + szLen);
}
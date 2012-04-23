/** Fake libnfc implementation
 *
 * See http://code.google.com/p/libnfc/ for libnfc source and docs.
 */

#include <nfc/nfc.h>

#include <stdlib.h>
#include <stdio.h>

#include <jni.h>

#define IMPLEMENT_ME do { fprintf(stderr, "ERROR: function %s not implemented. exiting.\n", __FUNCTION__); exit(42); } while (0)

jobject fake_libnfc;

JNIEXPORT void JNICALL Java_net_raisama_nfc_mfoc_NativeImplementation_setFakeLibNFC
  (JNIEnv *env, jobject obj, jobject nfc)
{
	fake_libnfc = (*env)->NewGlobalRef(env, nfc);
}

void nfc_perror (const nfc_device *pnd, const char *s)
{
	fprintf(stderr, "nfc_perror called for device %p: %s\n", pnd, s?s:"(null)");;
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
    newdev->obj = (*env)->CallObjectMethod(env, fake_libnfc, mid);
    if (!newdev->obj)
    	abort();
    newdev->obj = (*env)->NewGlobalRef(env, newdev->obj);

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
	fprintf(stderr, "nfc_initiator init called on dev %p. obj is: %p\n", pnd, pnd->obj);
	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, pnd->obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, "initiator_init", "()I");
	if (mid == NULL)
		abort();
	return (*env)->CallIntMethod(env, pnd->obj, mid);
}

int nfc_initiator_select_passive_target (nfc_device *pnd, const nfc_modulation nm, const uint8_t *pbtInitData, const size_t szInitData, nfc_target *pnt)
{
	int ret;

	fprintf(stderr, "nfc_initiator_select_passive_target called: %p, modulation: %d/%d, initdata: %p, szInitData: %ld, target: %p\n", pnd, (int)nm.nmt, (int)nm.nbr, pbtInitData, (long)szInitData, pnt);

	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, pnd->obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, "initiator_select_passive_target", "()Lnet/raisama/nfc/mfoc/NfcTarget;");
	if (mid == NULL)
		abort();
	jobject target_info = (*env)->CallObjectMethod(env, pnd->obj, mid);

/*
Fields to fill up:
	nfc_target_info nti;
		nfc_iso14443a_info nai;
			uint8_t  abtAtqa[2];
			uint8_t  btSak;
			size_t  szUidLen;
			uint8_t  abtUid[10];
			size_t  szAtsLen;
			uint8_t  abtAts[254]; // Maximal theoretical ATS is FSD-2, FSD=256 for FSDI=8 in RATS
	nfc_modulation nm;
		nfc_modulation_type nmt;
			NMT_ISO14443A = 1,
		nfc_baud_rate nbr;

*/
	/*TODO: fill data from the card on *pnt, here */
	IMPLEMENT_ME;
	memset(pnt, 0, sizeof(*pnt));
	return 0;
}


/* NOTE: we will probably need some tricks to be able to send raw commands
 * without CRC using the Android API. I hope the system allow us to do
 * that.  :(
 */


int nfc_initiator_transceive_bytes (nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, size_t *pszRx, int timeout)
{
	IMPLEMENT_ME;
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
	fprintf(stderr, "set_property_bool(%d, %d) called\n", property, bEnable);
}

void iso14443a_crc_append (uint8_t *pbtData, size_t szLen)
{
	IMPLEMENT_ME;
}
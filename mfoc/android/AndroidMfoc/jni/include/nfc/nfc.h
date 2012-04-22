#include "nfc-types.h"

/* Error codes */
/** @ingroup error
 * @hideinitializer
 * Success (no error)
 */
#define NFC_SUCCESS			 0
/** @ingroup error
 * @hideinitializer
 * Input / output error, device will not be usable anymore 
 */
#define NFC_EIO				-1
/** @ingroup error
 * @hideinitializer
 * Invalid argument(s) 
 */
#define NFC_EINVARG			-2
/** @ingroup error
 * @hideinitializer
 *  Operation not supported by device 
 */
#define NFC_EDEVNOTSUPP			-3
/** @ingroup error
 * @hideinitializer
 * No such device 
 */
#define NFC_ENOTSUCHDEV			-4
/** @ingroup error
 * @hideinitializer
 * Buffer overflow 
 */
#define NFC_EOVFLOW			-5
/** @ingroup error
 * @hideinitializer
 * Operation timed out 
 */
#define NFC_ETIMEOUT			-6
/** @ingroup error
 * @hideinitializer
 * Operation aborted (by user) 
 */
#define NFC_EOPABORTED			-7
/** @ingroup error
 * @hideinitializer
 * Not (yet) implemented 
 */
#define NFC_ENOTIMPL			-8
/** @ingroup error
 * @hideinitializer
 * Target released 
 */
#define NFC_ETGRELEASED			-10     
/** @ingroup error
 * @hideinitializer
 * Error while RF transmission 
 */
#define NFC_ERFTRANS			-20
/** @ingroup error
 * @hideinitializer
 * Device's internal chip error 
 */
#define NFC_ECHIP			-90


void nfc_init(nfc_context *context);
void nfc_exit(nfc_context *context);

int nfc_initiator_init (nfc_device *pnd);
int nfc_initiator_select_passive_target (nfc_device *pnd, const nfc_modulation nm, const uint8_t *pbtInitData, const size_t szInitData, nfc_target *pnt);
int nfc_initiator_transceive_bytes (nfc_device *pnd, const uint8_t *pbtTx, const size_t szTx, uint8_t *pbtRx, size_t *pszRx, int timeout);
int nfc_initiator_transceive_bits (nfc_device *pnd, const uint8_t *pbtTx, const size_t szTxBits, const uint8_t *pbtTxPar, uint8_t *pbtRx, uint8_t *pbtRxPar);

nfc_device *nfc_open (nfc_context *context, const nfc_connstring connstring);
void nfc_close (nfc_device *pnd);

void nfc_perror (const nfc_device *pnd, const char *s);

int nfc_device_set_property_int (nfc_device *pnd, const nfc_property property, const int value);
int nfc_device_set_property_bool (nfc_device *pnd, const nfc_property property, const bool bEnable);

void iso14443a_crc_append (uint8_t *pbtData, size_t szLen);
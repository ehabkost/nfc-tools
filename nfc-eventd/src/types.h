#ifndef __TYPES_H__
#define __TYPES_H__

#include <nfc/nfc.h>

typedef enum {
  EVENT_TAG_INSERTED,
  EVENT_TAG_REMOVED,
  EVENT_EXPIRE_TIME
} nem_event_t;

typedef struct {
  nfc_modulation_t modulation;
  nfc_target_info_t ti;
} tag_t;

#endif

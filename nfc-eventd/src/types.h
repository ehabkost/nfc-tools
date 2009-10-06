#ifndef __TYPES_H__
#define __TYPES_H__

#include <libnfc/libnfc.h>

typedef enum {
  EVENT_TAG_INSERTED,
  EVENT_TAG_REMOVED,
  EVENT_EXPIRE_TIME
} nem_event_t;

typedef struct {
  init_modulation im;
  tag_info ti;
} tag_t;

#endif

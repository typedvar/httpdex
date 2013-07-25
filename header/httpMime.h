/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPMIME_H
#define HTTPMIME_H

#include "httpCommon.h"
#include "httpMemsys.h"

#include "httpLog.h"

/* Refer RFC 2045-48 */

typedef struct
{
   unsigned char *ext;
   unsigned char *type;
   unsigned char *subtype;
}http_mime_t;

typedef struct
{
   http_mime_t *table[MAX_MIME_TYPES];
   int numTypes;
}http_mime_table_t;

typedef enum
{
   MAX_TYPE_DICT_LEN = 10,
   MAX_SUBTYPE_DICT_LEN = 459
}TYPES_IDX;

/* Function declarations */

int hx_init_mime_table(hx_logsys_t *log, hx_allocator_t *allocator);

void hx_print_mime_table(hx_logsys_t *logsys, int log_level);

void hx_print_mime_type(hx_logsys_t *logsys, http_mime_t *mime, int log_level);

http_mime_t *hx_lookup_ext_mime(hx_logsys_t *log, const unsigned char *ext);

http_mime_t *hx_lookup_resource_mime(hx_logsys_t *log,
                           const unsigned char *resource);

const unsigned char *hx_get_mime_type(const http_mime_t *mime);

const unsigned char *hx_get_mime_sub_type(const http_mime_t *mime);

#endif
/* End of File */

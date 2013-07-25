/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPDIRLISTER_H
#define HTTPDIRLISTER_H

#include "httpList.h"
#include "httpRequest.h"
#include "httpThreads.h"

typedef struct
{
   /* parent dir */
   unsigned char *parent;

   /* root of the entity */
   unsigned char *entroot;

   /* the href */
   unsigned char *href;

   /* the icon */
   unsigned char *icon;

   /* the entity name */
   unsigned char *ent;

   /* entity size */
   size_t         size;

   /* Last modification time */
   time_t         lmt;

   /* Permission string */
   unsigned char *mode;

   /* is dir? */
   int isdir;
}direlem_t;

int hx_get_dir_list(http_request_t *request_object,
                  hx_list_t** list,
                  hx_lock_t *lock);

void hx_dir_lister(void *data,
                  unsigned char **line,
                  hx_allocator_t *allocator);
#endif
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPVHOST_H
#define HTTPVHOST_H

#include "httpLog.h"

typedef struct
{
   unsigned char *host;
   unsigned char *docroot;
   unsigned char *scriptroot;

   /* Full path of the error page
      for this virtual host */
   unsigned char *errpage;

   /* Pointer to the log system */
   hx_logsys_t   *log;
}hx_vhost_t;

/* function declarations */

hx_vhost_t * hx_getVhost(hx_logsys_t *log, const unsigned char *hostname);

int hx_addVhost(hx_logsys_t *log,
                const unsigned char *hostname,
                const unsigned char *docroot,
                const unsigned char *scriptroot,
                const unsigned char *logfile,
                const unsigned char *errpage,
                const unsigned char *loglevels);

int hx_add_default_vhost(hx_logsys_t *log);

int hx_create_vhost_sys(hx_logsys_t *log);

void hx_destroyVhostSys(hx_logsys_t *log);

#endif
/* End of File */

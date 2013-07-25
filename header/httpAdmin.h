/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifndef HTTPADMIN_H
#define HTTPADMIN_H

#include "httpCommon.h"
#include "httpInclude.h"

/* The httpDex Administration Protocol */
/*
   The Admin message is formed as follows:

   message     = len command SP [arguments]

   len         = 4 * (digit)
   command     = reload_config      |
                 restart_log        |
                 shutdown

   arguments   = *(argument SP)
   argument    = *(alphanumeric)
*/
typedef enum
{
   ADM_OK,
   ADM_ERR,
   ADM_WARN
} hx_adminresp_t;


/* HTTP Administration Request structure */
typedef struct
{
   /* the socket */
   SOCKET sock;

   /* the allocator */
   hx_allocator_t *allocator;

   /* message length */
   int msg_len;

   /* client Address */
   unsigned char *client_address;

   /* time stamp */
   unsigned char  time[MAX_TIME_LEN];

   /* command idx */
   int cmdidx;

   /* num args */
   int argc;

   /* actual arguments */
   unsigned char **argv;

   /* response code */
   hx_adminresp_t resp;

   /* response body */
   unsigned char *body;

   /* response length */
   long bodylen;

   /* request */
   unsigned char *request;

   /* request len */
   long requestLen;

} http_admin_t;

/* Administration Function Pointer */
typedef int (*fadmin_t) (http_admin_t *a);

/* Admin Functions */
int hx_reload_cfg (http_admin_t *a);
int hx_reloadVhost (http_admin_t *a);
int hx_shutdown (http_admin_t *a);
int hx_restart_log (http_admin_t *a);

/* function declarations */
http_admin_t* hx_create_admin_object (hx_allocator_t *allocator);

int hx_parse_admin (http_admin_t *a);

int hx_execute_admin (http_admin_t *a);

/* switchoff func declaration */
extern void hx_switch_off(int type, hx_allocator_t *allocator);

/* function to change the shutdown
   state */
extern void hx_signal_shutdown(int state);

#endif
/* End of File */

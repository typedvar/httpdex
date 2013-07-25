#ifndef HTTPINCLUDE_H
#define HTTPINCLUDE_H

#include "httpLog.h"
#include "httpMemsys.h"
#include "httpUtils.h"

/* EXTERN VARIABLES */
extern int                 server_shutdown;
extern int                 http_done;
extern int                 admin_done;

extern const unsigned char *VERSION;
extern const unsigned char *SERVER;

extern int                 errno;

extern hx_lock_t           *lock_num_clients;
extern hx_lock_t           *lock_accept_http;
extern hx_lock_t           *lock_accept_admin;
extern hx_lock_t           *lock_errno;
extern hx_lock_t           *lock_shutdown;

extern int                 g_num_clients_being_served;
extern SOCKET              serverSock;
extern SOCKET              adminSock;

extern hx_logsys_t         *server_log_sys;
extern hx_vhost_t          *defaultVhost;
extern hx_allocator_t      *server_allocator;

/* END EXTERN */

#endif
/* End of File */

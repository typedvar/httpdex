/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifndef HTTPCONN_H
#define HTTPCONN_H

#include "httpInclude.h"

#ifdef _HX_WINTHREADS_

DWORD WINAPI hx_accept_http_conn(LPVOID param);
DWORD WINAPI hx_accept_admin_conn(LPVOID param);

#elif defined(_PTHREADS_)

void *hx_accept_http_conn(void *param);
void *hx_accept_admin_conn(void *param);

#endif /* _HX_WINTHREADS_ */

extern void hx_signal_shutdown(int state);

#endif
/* End of File */

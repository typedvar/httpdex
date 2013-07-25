/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include "httpMemsys.h"

#define HTTP_Q_LEN           10
#define ADMIN_Q_LEN          2

#ifdef _HX_WINTHREADS_

DWORD WINAPI
hx_spawn_admin(LPVOID param);

DWORD WINAPI
hx_spawn_workers(LPVOID param);

#elif defined(_PTHREADS_)

void *
hx_spawn_admin(void *param);

void *
hx_spawn_workers(void *param);

#endif

extern void hx_switch_off(int type, hx_allocator_t *allocator);

#endif
/* End of File */

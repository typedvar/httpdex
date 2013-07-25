/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */
#ifndef HTTPLOCK_H
#define HTTPLOCK_H

#ifdef _PTHREADS_
#include <pthread.h>
#endif

#ifdef _HX_WIN32_
#include <windows.h>
#endif

#include "httpMemsys.h"

/* The lock structure */
typedef struct
{
#ifdef _HX_WINTHREADS_
   CRITICAL_SECTION *lk;
#elif defined(_PTHREADS_)
   pthread_mutex_t  *lk;
#endif

   hx_allocator_t   *a;
} hx_lock_t;

/* Function declaration */
/* Lock operations */
hx_lock_t *hx_create_lock(void);
void hx_destroy_lock(hx_lock_t *lock);
void hx_lock(hx_lock_t *lock);
void hx_unlock(hx_lock_t *lock);

#endif
/* End of File */

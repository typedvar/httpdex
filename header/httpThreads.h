/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */
#ifndef HTTPTHREADS_H
#define HTTPTHREADS_H

#ifdef _PTHREADS_
#include <pthread.h>
#endif

#ifdef _HX_WIN32_
#include <windows.h>
#endif

#include "httpMemsys.h"
#include "httpLog.h"

/* The thread function typedef */
#ifdef _HX_WINTHREADS_
typedef DWORD WINAPI (*fthread_t)(void*);
#elif defined(_PTHREADS_)
typedef void* (*fthread_t)(void *);
#endif

/* The thread structure */
typedef struct
{
#ifdef _HX_WINTHREADS_
   HANDLE            *handle;
   DWORD             *id;
#elif defined(_PTHREADS_)
   pthread_t         *ptr;
#endif

   void              *param;
   hx_allocator_t    *a;
   fthread_t         func;
} hx_thread_t;

/* Function Declarations */
/* Thread operations */
hx_thread_t *hx_create_thread(hx_allocator_t *a,
                             fthread_t func,
                             void *param);
void hx_exitThread(void *status, hx_logsys_t *log);
int hx_get_thread_id(hx_thread_t *thread);
int hx_wait_for_threads(hx_allocator_t *a,
                      hx_thread_t **arr,
                      int count,
                      hx_logsys_t *log);

int hx_threadSigMask(void);

#endif
/* End of File */

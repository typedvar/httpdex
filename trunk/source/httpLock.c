/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include "httpLock.h"
#include "httpInclude.h"
#include <errno.h>

/* LOCK Function Definitions */
hx_lock_t *
hx_create_lock(void)
{
   hx_allocator_t *a;
   hx_lock_t      *lock;
   int retval;

   /* create the allocator */
   a = hx_create_allocator("Lock");

   if(!a)
      return NULL;

   /* create the lock */
   lock = (hx_lock_t *)hx_alloc_mem(sizeof(hx_lock_t), a);
   memset(lock, 0x00, sizeof(hx_lock_t));

   if(!lock)
      return NULL;

   /* assign the allocator */
   lock->a = a;

   /* create the locking mechanism */
#ifdef _HX_WINTHREADS_
   lock->lk = (CRITICAL_SECTION *)hx_alloc_mem(sizeof(CRITICAL_SECTION), a);

   if(!lock->lk)
      return NULL;

   /* Intialize the critical section */
   memset(lock->lk, 0x00, sizeof(CRITICAL_SECTION));
   InitializeCriticalSection(lock->lk);

#elif defined(_PTHREADS_)
   lock->lk = (pthread_mutex_t *)hx_alloc_mem(sizeof(pthread_mutex_t), a);

   if(!lock->lk)
      return NULL;

   /* Initialize the mutex */
   memset(lock->lk, 0x00, sizeof(pthread_mutex_t));
   retval = pthread_mutex_init(lock->lk, NULL);

   if(retval)
   {
      pthread_mutex_destroy(lock->lk);
      hx_destroy_allocator(a);
      return NULL;
   }
#endif

   return lock;
}

void
hx_destroy_lock(hx_lock_t *lock)
{
   if(!lock)
      return;

#ifdef _HX_WINTHREADS_
   DeleteCriticalSection(lock->lk);
#elif defined(_PTHREADS_)
   pthread_mutex_destroy(lock->lk);
#endif
   hx_destroy_allocator(lock->a);
   return;
}

/* locking and unlocking functions */
void
hx_lock(hx_lock_t *lock)
{
   if(!lock)
      return;

#ifdef _HX_WINTHREADS_
   EnterCriticalSection(lock->lk);
#elif defined(_PTHREADS_)
   pthread_mutex_lock(lock->lk);
#endif
}

void
hx_unlock(hx_lock_t *lock)
{
   if(!lock)
      return;

#ifdef _HX_WINTHREADS_
   LeaveCriticalSection(lock->lk);
#elif defined(_PTHREADS_)
   pthread_mutex_unlock(lock->lk);
#endif
}

/* End of File */

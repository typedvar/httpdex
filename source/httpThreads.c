/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include "httpThreads.h"
#include "httpInclude.h"

#include <errno.h>
#include <signal.h>

#ifdef _HX_WIN32_
#include <process.h>
#elif defined (_HX_UNIX_)
#include <unistd.h>
#include <sys/types.h>
#endif

/* THREAD Function Definitions */

/* hx_create_thread
   The pthreads are created in joinable state (default)
   to execute thread join later with call to
   hx_wait_for_threads */
hx_thread_t *
hx_create_thread(hx_allocator_t *a, fthread_t func, void *param)
{
   hx_thread_t    *thread;
   int            retval;

   if(!a || !func)
      return NULL;

   /* create a thread object */
   thread = (hx_thread_t *)hx_alloc_mem(sizeof(hx_thread_t), a);

   if(!thread)
      return NULL;

   /* clean the area */
   memset (thread, 0x00, sizeof(hx_thread_t));

   /* assign the allocator */
   thread->a = a;
   thread->func = func;
   thread->param = param;

#ifdef _HX_WINTHREADS_
   thread->handle = hx_alloc_mem(sizeof(HANDLE), a);

   if(!thread->handle)
   {
      return NULL;
   }

   thread->id = hx_alloc_mem(sizeof(DWORD), a);

   if(!thread->id)
   {
      return NULL;
   }

   /* invoke the thread function */
   *(thread->handle) = CreateThread(NULL,          /*default security attributes */
                                    0,             /*use default stack size      */
                                    thread->func,  /*thread function             */
                                    thread->param, /*argument to thread function */
                                    0,             /*use default creation flags  */
                                    thread->id);   /*the thread identifier       */

   if(NULL == *thread->handle)
   {
      return NULL;
   }

#elif defined(_PTHREADS_)

   /* allocate space to hold the thread pointer */
   thread->ptr = (pthread_t *)hx_alloc_mem(sizeof(pthread_t), a);

   if(!thread->ptr)
   {
      return NULL;
   }

   memset (thread->ptr, 0x00, sizeof(pthread_t));

   /* invoke the thread function */
   retval = pthread_create(thread->ptr,
                           NULL,
                           thread->func,
                           thread->param);

   if(retval)
   {
      return NULL;
   }
#endif

   return thread;
}

/* hx_wait_for_threads
   Generic function to make a thread
   wait for all threads in the group
   to finish execution. For the win32
   implementation, the unused handles
   are closed */

int hx_wait_for_threads(hx_allocator_t *a,
                      hx_thread_t **arr,
                      int count,
                      hx_logsys_t *log)
{
   const char *rname = "hx_wait_for_threads ()";
   int retval = OK;
   int i;

#ifdef _HX_WINTHREADS_
   HANDLE    *handle_arr;
#endif

   /* Precondition check */
   if(!a || !arr || count <= 0)
      return HX_ERR_INVALID_PARAM;

   hx_log_msg(log, HX_LCALL, rname, "Called");

#ifdef _HX_WINTHREADS_
   /* create the array of handles */
   handle_arr = (HANDLE *)hx_alloc_mem((sizeof(HANDLE) * count), a);

   if(!handle_arr)
      return HX_ERR_MEM_ALLOC;

   /* copy the handles from the hx_thread_t object
      to the handle array */
   for(i = 0; i < count; ++i)
   {
      handle_arr[i] = *(arr[i]->handle);
      if(!handle_arr[i])
      {
         retval = HX_ERR_INVALID_THRD_HNDL;
         break;
      }
   }

   if(OK == retval)
   {
      /* call the waiting function */
      if(WaitForMultipleObjects((DWORD) count,
                                handle_arr,
                                TRUE,
                                INFINITE) == WAIT_FAILED)
      {
         hx_WIN32Err(log);
         retval = HX_ERR_WAIT_FAILED;
      }
   }


   if(OK == retval)
   {
      /* Close all open thread handles */
      for(i = 0; i < count; ++i)
      {
         if(!CloseHandle(*(arr[i]->handle)))
            retval = HX_ERR_CLOSE_HANDLE;
      }
   }
#elif defined(_PTHREADS_)
   for(i = 0; i < count; ++i)
   {
      if(!((arr[i])->ptr))
      {
         hx_log_msg(log,
                   HX_LDEBG,
                   rname,
                   "Invalid pthread [%08d]", hx_get_thread_id(arr[i]));

         retval = HX_ERR_INVALID_THRD_PTR;
         break;
      }

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Joining pthread [%08d]", hx_get_thread_id(arr[i]));

      retval = pthread_join(*((arr[i])->ptr), (void **)NULL);

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Joined pthread [%08d]", hx_get_thread_id(arr[i]));

      if(retval)
      {
         hx_log_msg(log,
                   HX_LDEBG,
                   rname,
                   "pthread_join error");

         if(retval == ESRCH)
            retval = HX_ERR_JOIN_FAILED_SRCH;
         else
            retval = HX_ERR_UNKNOWN;
         break;
      }
      else
         retval = OK;
   }
#endif
   return retval;
}

void hx_exitThread(void *status,
                   hx_logsys_t *log)
{
   const char *rname = "hx_exitThread ()";

#ifdef _HX_WINTHREADS_
   ExitThread((DWORD)status);
#elif defined(_PTHREADS_)
   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Calling pthread_exit ()");
   pthread_exit(status);
#endif
}

int hx_get_thread_id(hx_thread_t *thread)
{
#ifdef _HX_WINTHREADS_
   return *(thread->id);
#elif defined(_PTHREADS_)
   return (int)thread->ptr;
#endif
}

/* hx_threadSigMask
   The worker threads or the admin threads
   will not do any signal processing. In case
   the system receives a signal, the main thread
   will handle the signal and will process it.
   This function masks all signals for the threads.
*/
int hx_threadSigMask(void)
{
   int retval = 0;

#ifdef _PTHREADS_
#ifdef _HX_UNIX_
   sigset_t set;

   /* Clear the sigset */
   sigemptyset(&set);

   /* Mask all available signals */
   sigfillset(&set);

   retval = pthread_sigmask(SIG_BLOCK, &set, (sigset_t *)NULL);
#endif
#endif
   return retval;
}

/* End of File */

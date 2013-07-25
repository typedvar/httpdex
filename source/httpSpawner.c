/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _HX_WIN32_
#include <windows.h>
#endif

#include "httpCommon.h"
#include "httpInclude.h"
#include "httpSock.h"

#include "httpConn.h"
#include "httpSpawner.h"
#include "httpConf.h"

/*
   This routine spawns the admin
   thread which listens for administrative
   requests on a specified port
*/
#ifdef _HX_WINTHREADS_
DWORD WINAPI
hx_spawn_admin(LPVOID param)
#elif defined(_PTHREADS_)
void *
hx_spawn_admin(void *param)
#endif
{
   const unsigned char *rname = "hx_spawn_admin ()";

   short port;
   int count;
   int status = OK;
   int maxAdmThreads;

   hx_thread_t **thread_arr;
   hx_allocator_t *allocator = (hx_allocator_t *)param;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* get the administration interface port from the config */
   port = (short) atoi((unsigned char *)hx_get_conf_val(server_log_sys,
                                                      IDX_ADMIN_PORT));

   /* get the maximum admin threads that require to run */
   maxAdmThreads = atoi((unsigned char *)hx_get_conf_val(server_log_sys,
                                                       IDX_NUM_ADM_THRDS));

   /* Create a passive TCP socket listening */
   adminSock = hx_serverSock(server_log_sys, port, ADMIN_Q_LEN);

   /* Check to see if the returned socket
      is ok or not */
   if(adminSock != ERRC)
   {
      thread_arr = (hx_thread_t **)hx_alloc_mem(
                           (maxAdmThreads * (sizeof(hx_thread_t *))),
                           allocator);

      for(count = 0; count < maxAdmThreads; ++count)
      {
         thread_arr[count] = hx_create_thread(allocator,
                                             hx_accept_admin_conn,
                                             server_log_sys);

         if(thread_arr[count] == NULL)
         {
            hx_log_msg(server_log_sys,
                      HX_LERRO,
                      rname,
                      "hx_create_thread () failed");

            status = ERRC;
            break;
         }

         hx_log_msg(server_log_sys,
                   HX_LDEBG,
                   rname,
                   "Thread id is [%09d]", hx_get_thread_id(thread_arr[count]));
      }

      if(OK == status)
         hx_log_msg(server_log_sys,
                   HX_LVERB,
                   rname,
                   "ONLINE -> Admin Interface");
      else
      {
         hx_log_msg(server_log_sys,
                   HX_LERRO,
                   rname,
                   "FAILED to start Admin Interface");

         hx_switch_off(ABORTIVE_CLOSE, NULL);
      }

      /* Make the threads wait till all the threads in the group
         complete the task */
      status = hx_wait_for_threads(allocator, thread_arr, count, server_log_sys);

      if(OK != status)
      {
         hx_log_msg(server_log_sys,
                   HX_LERRO,
                   rname,
                   "Failed to wait: %s",
                   hx_getErrorString(status));

         status = ERRC;
      }
      else
         hx_log_msg(server_log_sys,
                   HX_LDEBG,
                   rname,
                   "Returned successfully after waiting");
   }
   else
   {
      /* set the server shutdown state */
      hx_signal_shutdown(HX_SHUTDN_PENDING);
   }

   /* signal that the admin threads have
      completed their job */
   admin_done = HX_TRUE;
   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Set admin_done to TRUE");

   /* check for the shutdown state */
   if(HX_SHUTDN_PENDING == server_shutdown)
      hx_switch_off(NORMAL_CLOSE, NULL);

#ifdef _HX_WINTHREADS_
   return status;
#elif defined(_PTHREADS_)
   return (void *)status;
#endif
}

/*
   This routine spawns the worker
   threads which listens for incoming
   requests on the designated HTTP
   protocol port
*/
#ifdef _HX_WINTHREADS_
DWORD WINAPI
hx_spawn_workers(LPVOID param)
#elif defined(_PTHREADS_)
void *
hx_spawn_workers(void *param)
#endif
{
   const unsigned char *rname = "hx_spawn_workers ()";

   short port;
   int count;
   int status = OK;
   int maxSrvThreads;

   hx_thread_t **thread_arr;

   hx_allocator_t *allocator = (hx_allocator_t *)param;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* get the service interface port from the config */
   port = (short) atoi((unsigned char *)hx_get_conf_val(server_log_sys,
                                                      IDX_DEFAULT_HTTP_PORT));

   /* get the number of threads required to be spawned */
   maxSrvThreads = atoi((unsigned char *)hx_get_conf_val(server_log_sys,
                                                       IDX_NUM_SRV_THRDS));
   /* Create a passive TCP socket */
   serverSock = hx_serverSock(server_log_sys, port, HTTP_Q_LEN);

   /* Perform error checking */
   if(serverSock != ERRC)
   {
      thread_arr = (hx_thread_t **)hx_alloc_mem((maxSrvThreads *
                                             (sizeof(hx_thread_t *))),
                                             allocator);

      for(count = 0; count < maxSrvThreads; ++count)
      {
         thread_arr[count] = hx_create_thread(allocator,
                                             hx_accept_http_conn,
                                             server_log_sys);

         if(thread_arr[count] == NULL)
         {
            hx_log_msg(server_log_sys,
                      HX_LERRO,
                      rname,
                      "hx_create_thread () failed");

            status = ERRC;
            break;
         }

         hx_log_msg(server_log_sys,
                   HX_LDEBG,
                   rname,
                   "Thread id is [%09d]", hx_get_thread_id(thread_arr[count]));
      }

      if(OK == status)
         hx_log_msg(server_log_sys,
                   HX_LVERB,
                   rname,
                   "ONLINE -> Service Interface");
      else
      {
         hx_log_msg(server_log_sys,
                   HX_LERRO,
                   rname,
                   "FAILED to start Service Interface");

         hx_switch_off(ABORTIVE_CLOSE, NULL);
      }

      /* Make the threads wait till all the threads in the group
         complete the task */
      status = hx_wait_for_threads(allocator, thread_arr, count, server_log_sys);

      hx_log_msg(server_log_sys,
                HX_LDEBG,
                rname,
                "Returned from hx_wait_for_threads ()");

      if(OK != status)
      {
         hx_log_msg(server_log_sys,
                   HX_LERRO,
                   rname,
                   "Failed to wait: %s",
                   hx_getErrorString(status));
      }
      else
         hx_log_msg(server_log_sys,
                   HX_LDEBG,
                   rname,
                   "Returned successfully after waiting");
   }
   else
   {
      /* set the server shutdown state */
      hx_signal_shutdown(HX_SHUTDN_PENDING);
   }

   /* signal that the worker threads have
      completed their job */
   http_done = HX_TRUE;

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Set http_done to TRUE");

   /* check for the shutdown state */
   if(HX_SHUTDN_PENDING == server_shutdown)
      hx_switch_off(NORMAL_CLOSE, NULL);

#ifdef _HX_WINTHREADS_
   return status;
#elif defined(_PTHREADS_)
   return (void *)status;
#endif
}
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include <stdio.h>

#ifdef _HX_UNIX_
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include "httpConn.h"
#include "httpCommon.h"
#include "httpInclude.h"

#include "httpSock.h"
#include "httpProcessor.h"

/*
   hx_accept_http_conn
   This function is the main thread procedure.
   The thread accepts an incoming connection,
   processes it, and again waits for another
   connection. The thread will terminate on
   accept failure, indicating some problem
   with the server socket
*/
#ifdef _HX_WINTHREADS_
DWORD WINAPI
hx_accept_http_conn(LPVOID param)
#elif defined(_PTHREADS_)
void *
hx_accept_http_conn(void *param)
#endif
{
   const unsigned char *rname = "hx_accept_http_conn ()";
   hx_logsys_t *log = (hx_logsys_t *)param;

   SOCKET client_socket;
   int addrlen;

   struct sockaddr_in client_address;
   int retval;

   unsigned char *clientAddrStr;

   /* the per request allocator */
   hx_allocator_t *allocator;

   /* block INT and SEGV signals */
   retval = hx_threadSigMask();

   if(retval)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Failed to block signals for thread");

      hx_signal_shutdown(HX_SHUTDN_PENDING);
   }

   addrlen = sizeof(struct sockaddr_in);

   memset(&client_address, 0, addrlen);

   /* Infinite Thread Loop */
   while(HX_SHUTDN_NOTPENDING == server_shutdown)
   {
      /* accept the incoming connection request */
      hx_lock(lock_accept_http);

      client_socket = accept(serverSock,
                          (struct sockaddr *)&client_address,
                          &addrlen);

      hx_unlock(lock_accept_http);

      if(ERRC == client_socket)
      {
         if(server_shutdown == HX_SHUTDN_NOTPENDING)
            hx_sockErr(log,
                       "accept() failed in hx_accept_http_conn()",
                       hx_get_socket_error());
         break;
      }

      /* Create the allocator */
      allocator = hx_create_allocator("Request Allocator");

      /* increment client count */
      hx_lock(lock_num_clients);
      g_num_clients_being_served++;
      hx_unlock(lock_num_clients);

      clientAddrStr = hx_strdup((unsigned char *)
                                 inet_ntoa(client_address.sin_addr),
                                 allocator);

      hx_log_msg(server_log_sys,
                      HX_LDEBG,
                      rname,
                      "\n\n");

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Received connection from %s [on socket %d]",
                clientAddrStr,
                client_socket);

      /* process the incoming connection */
      retval = hx_process_http(client_socket, clientAddrStr, allocator);

      /* shutdown client socket and close it */
      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "hx_process_http() returned %d",
                retval);

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Closing socket %d",
                client_socket);

      hx_sleep(SOCK_CLOSE_DELAY);

      hx_sockClose(client_socket, HX_SEND);

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Closed socket %d",
                client_socket);

      /* decrement client count */
      hx_lock(lock_num_clients);
      g_num_clients_being_served--;
      hx_unlock(lock_num_clients);

      hx_destroy_allocator(allocator);
      allocator = NULL;
   }

   retval = NORMAL_CLOSE;

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Exiting Thread Function");

   hx_exitThread((void *)retval, log);
}

/*
   hx_accept_admin_conn
   This function is invoked to process
   administration requests directed to
   httpDeX.
*/
#ifdef _HX_WINTHREADS_
DWORD WINAPI
hx_accept_admin_conn(LPVOID param)
#elif defined(_PTHREADS_)
void *
hx_accept_admin_conn(void *param)
#endif
{
   const unsigned char *rname = "hx_accept_admin_conn ()";
   hx_logsys_t *log = (hx_logsys_t *)param;

   SOCKET client_socket;
   int addrlen;

   struct sockaddr_in client_address;
   int retval;
   unsigned char *clientAddrStr;

   /* the per request allocator */
   hx_allocator_t *allocator;

   /* block INT and SEGV signals */
   retval = hx_threadSigMask();

   if(retval)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Failed to block signals for thread");

      hx_signal_shutdown(HX_SHUTDN_PENDING);
   }

   addrlen = sizeof(struct sockaddr_in);

   memset(&client_address, 0, addrlen);

   /* Controlled Thread Loop */
   while(HX_SHUTDN_NOTPENDING == server_shutdown)
   {
      /* accept the incoming connection request */
      hx_lock(lock_accept_admin);

      /* to break a thread out of this
         accept, kill the socket */
      client_socket = accept(adminSock,
                          (struct sockaddr *)&client_address,
                          &addrlen);

      hx_unlock(lock_accept_admin);

      if(client_socket == ERRC)
      {
         if(server_shutdown == HX_SHUTDN_NOTPENDING)
            hx_sockErr(log,
                       "accept() failed in hx_accept_admin_conn()",
                       hx_get_socket_error());
         break;
      }

      /* Create the allocator */
      allocator = hx_create_allocator("admin");

      clientAddrStr = hx_strdup(inet_ntoa(client_address.sin_addr), allocator);

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Received Administration request from %s [on socket %d]",
                clientAddrStr,
                client_socket);

      /* process the incoming connection */
      retval = hx_process_admin(client_socket, clientAddrStr, allocator);

      /* shutdown the server
         since there's a failure */
      if(retval == HX_ERR_CONFIG_LOAD)
      {
         hx_signal_shutdown(HX_SHUTDN_PENDING);
      }

      /* shutdown client socket and close it */
      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "hx_process_admin () returned %d",
                retval);

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Closing socket %d...",
                client_socket);

      hx_sleep(SOCK_CLOSE_DELAY);

      hx_sockClose(client_socket, HX_SEND);

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Closed socket %d...",
                client_socket);

      hx_destroy_allocator(allocator);
      allocator = NULL;
   }

   retval = NORMAL_CLOSE;

   if(HX_SHUTDN_PENDING == server_shutdown)
   {

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Returning to spawner, shutdown pending");

      /* Kill the admin socket so that
         other threads can come out of it */
      if(adminSock)
      {
         hx_sockClose(adminSock, HX_BOTH);
         adminSock = 0x00;
      }
   }

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Exiting Thread Function");

   hx_exitThread((void *)retval, log);
}
/* End of File */

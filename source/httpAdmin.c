/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "httpAdmin.h"
#include "httpInclude.h"
#include "httpConf.h"
#include "httpSock.h"

static const unsigned char *admin_method_strings[MAX_ADMIN_METHODS] =
{
   "reload_config",
   "restart_log",
   "shutdown",
};

static const fadmin_t admin_methods[MAX_ADMIN_METHODS] =
{
   hx_reload_cfg   ,
   hx_restart_log  ,
   hx_shutdown
};

int
hx_reload_cfg (http_admin_t *a)
{
   int retval;
   retval = hx_reload_config(server_log_sys);
   return retval;
}

int
hx_shutdown (http_admin_t *a)
{
   const unsigned char *rname = "hx_shutdown ()";

   hx_log_msg(server_log_sys,
             HX_LWARN,
             rname,
             "Received SHUTDOWN instruction from %s",
             a->client_address);

   /* set the SHUTDOWN to PENDING */
   hx_signal_shutdown(HX_SHUTDN_PENDING);

   return OK;
}

int
hx_restart_log (http_admin_t *a)
{
   return OK;
}

/* hx_create_admin_object ()
   Creates a http_admin_t Object
*/
http_admin_t*
hx_create_admin_object (hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_createReqObj ()";

   http_admin_t *a;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* allocate space for the
      request object */
   if((a = (http_admin_t *)hx_alloc_mem(sizeof(http_admin_t), allocator)) == NULL)
      return NULL;

   /* initiliaze the request object */
   memset(a, 0x00, sizeof(http_admin_t));

   a->allocator = allocator;
   a->requestLen = 0;

   /* Create the request buffer */
   a->request = (unsigned char *)hx_alloc_mem(MAX_ADMIN_REQUEST_LEN,
                                           a->allocator);

   if(!a->request)
      return NULL;

   return a;
}

/*
   hx_print_admin ()
   Displays the http_admin_t Object.
*/

void
hx_print_admin (http_admin_t *a, int log_level)
{
   const unsigned char *rname = "hx_print_admin ()";

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   if(!a)
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "Invalid Admin Object");
      return;
   }

   if(!hx_check_log_level(server_log_sys, log_level))
      return;

   hx_log_msg(server_log_sys,
             log_level,
             rname,
             "===========================================");

   hx_log_msg(server_log_sys,
             log_level,
             rname,
             "Incoming request from %s",
             a->client_address);

   if (a->cmdidx > -1)
      hx_log_msg(server_log_sys,
                log_level,
                rname,
                "Method : %s",
                method_names[a->cmdidx]);
   else
      hx_log_msg(server_log_sys,
                log_level,
                rname,
                "Method : INVALID METHOD");


   hx_log_msg(server_log_sys,
             log_level,
             rname,
             "===========================================\n");
}

static int
hx_get_admin_data (http_admin_t *a,
                 unsigned char *buffer,
                 long *bytes_read,
                 long capacity)
{
   const unsigned char *rname = "hx_get_admin_data ()";

   int err = 0;
   int retval;
   unsigned char *pch;

   pch = buffer;
   *bytes_read = 0;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

#ifdef _HX_WIN32_
   do
   {
      retval = hx_sock_read (a->sock, pch, capacity);
      if(retval == SOCKET_ERROR)
         err = hx_get_socket_error();
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
   }
   while((err != WSAEWOULDBLOCK) && capacity );
#elif defined (_HX_UNIX_)
   do
   {
      retval = hx_sock_read (a->sock, pch, capacity);
      if(retval == -1)
         err = hx_get_socket_error();
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
   }
   while((err != EAGAIN) && capacity );
#endif

   return retval;
}

int
hx_execute_admin (http_admin_t *a)
{
   const unsigned char *rname = "hx_execute_admin ()";

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   return (*admin_methods[a->cmdidx])(a);
}

/*
   hx_parse_admin()
   parse the incoming char buffer of length len
   as a http request. Form a structure of the same
   and send it back. If request is not a valid http
   request return HX_ERR_INVALID_REQUEST.
*/
int
hx_parse_admin (http_admin_t *a)
{
   const unsigned char *rname = "hx_parse_admin ()";

   unsigned char *pch;
   int retval;
   int cmdlen;

   long bytesread = 0;
   long capacity;
   long msg_len;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* extract the message length */
   capacity = MAX_MSG_ADMIN_LEN_SIZE;
   retval = hx_get_admin_data(a, a->request, &bytesread, capacity);

#ifdef _HX_WIN32_
   if(SOCKET_ERROR == retval)
#elif defined (_HX_UNIX_)
   if(-1 == retval)
#endif
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "Error Reading data");
      return HX_ERR_ADM_INVALID_MSG;
   }

   if(bytesread < capacity)
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "BAD Message");
      return HX_ERR_ADM_INVALID_MSG;
   }

   /* null terminate the string to extract len */
   a->request[MAX_MSG_ADMIN_LEN_SIZE] = '\0';
   msg_len = atoi(a->request);

   if((msg_len <= 0) ||
      (msg_len > MAX_ADMIN_REQUEST_LEN))
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "BAD Message Length");
      return HX_ERR_ADM_INVALID_LEN;
   }

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Message length is: %d",
             msg_len);

   /* get the message */
   capacity = msg_len;
   retval = hx_get_admin_data(a, a->request, &bytesread, capacity);

#ifdef _HX_WIN32_

   if(SOCKET_ERROR == retval)

#elif defined (_HX_UNIX_)

   if(-1 == retval)

#endif
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "Error Reading data");
      return HX_ERR_ADM_INVALID_MSG;
   }

   if(bytesread != msg_len)
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "Invalid Admin message");
      return HX_ERR_ADM_INVALID_MSG;
   }

   /* Display request data */
   hx_log_msg(server_log_sys, HX_LDEBG, rname, "ADMIN START");
   hx_hex_dump(server_log_sys, a->request, msg_len, HX_LDUMP);
   hx_log_msg(server_log_sys, HX_LDEBG, rname, "ADMIN END");

   /* Get the command */
   pch = a->request;
   a->requestLen = msg_len;

   while(*pch != SP && msg_len)
   {
      --msg_len;
      ++pch;
   }

   /* Null terminate the command */
   *pch = '\0';

   /* match the command string in the dictionary */
   if( NOMATCH == (retval = hx_match_string_idx(admin_method_strings,
                                           MAX_ADMIN_METHODS,
                                           a->request)))
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "BAD Command");
      return HX_ERR_ADM_INVALID_CMD;
   }
   else
   {
      a->cmdidx = retval;
      hx_log_msg(server_log_sys,
                HX_LDEBG,
                rname,
                "Command is %s",
                admin_method_strings[a->cmdidx]);
   }

   return HX_PARSE_OK;
}
/* End of File */

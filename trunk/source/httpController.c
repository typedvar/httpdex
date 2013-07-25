/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifdef _HX_UNIX_
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#endif

#include "httpCommon.h"
#include "httpController.h"
#include "httpSock.h"

static const unsigned char *VERSION   = "httpC version 1.0 "
                                        "[httpdEx Controller Console]";
static const unsigned char *COPYRIGHT = "Copyright (c) 2003";

static int
hx_getServerData (hx_logsys_t *log,
                  SOCKET s,
                  unsigned char *buffer,
                  long *bytes_read,
                  long capacity)
{
   const unsigned char *rname = "hx_getServerData ()";

   int retval;
   unsigned char *pch;

   pch = buffer;
   *bytes_read = 0;

   hx_log_msg(log, HX_LCALL, rname, "Called");

#ifdef _HX_WIN32_
   do
   {
      retval = hx_sock_read (s, pch, capacity);
      if(retval == SOCKET_ERROR)
      {
         retval = ERRC;
         break;
      }
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
   }
   while( capacity );
#elif defined(_HX_UNIX_)
   do
   {
      retval = hx_sock_read (s, pch, capacity);
      if(retval == -1)
      {
         retval = ERRC;
         break;
      }
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
   }
   while( capacity );
#endif

   return retval;
}

static void
hx_getServerResponse(hx_logsys_t *log, SOCKET s)
{
   const char *rname = "hx_getServerResponse ()";
   int capacity;
   unsigned char readbuff[MAX_ADMIN_RESPONSE_LEN];
   unsigned char datalen[MAX_MSG_ADMIN_LEN_SIZE];
   long readcount = 0;
   int msg_len;
   int retval;

   /* extract the message length */
   capacity = MAX_MSG_ADMIN_LEN_SIZE;

   retval = hx_getServerData (log,
                              s,
                              readbuff,
                              &readcount,
                              capacity);

   if(ERRC == retval)
   {
      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Server OFFLINE");
      return;
   }

   /* null terminate the string to extract len */
   readbuff[MAX_MSG_ADMIN_LEN_SIZE] = '\0';
   msg_len = atoi(readbuff);

   if((msg_len <= 0) ||
      (msg_len > MAX_ADMIN_REQUEST_LEN))
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "BAD Message Length");
      return;
   }

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Message length is: %d",
             msg_len);

   /* get the message */
   capacity = msg_len;
   hx_getServerData (log,
                     s,
                     readbuff,
                     &readcount,
                     capacity);

   /* Check to see if there is an error */
   retval = readbuff[0] - '0';

   /* if retval !0, then error */
   if(retval)
   {
      hx_log_msg(log, HX_LWARN, rname, "%.*s", (readcount - 2), &readbuff[2]);
   }

   /* Display request data */
   hx_log_msg(log, HX_LDEBG, rname, "Server RESPONSE Start");
   hx_hex_dump(log, readbuff, readcount, HX_LDUMP);
   hx_log_msg(log, HX_LDEBG, rname, "Server RESPONSE End");

   return;
}

int hx_start_client(hx_logsys_t *log, const unsigned char *host, long port)
{
   const char *rname = "hx_start_client ()";
   struct sockaddr_in caddr;
   int addrlen;
   int count;
   int c;
   int done = 0;

   SOCKET s;

   unsigned char data2send[MAX_ADMIN_REQUEST_LEN];
   unsigned char buffer[MAX_ADMIN_REQUEST_LEN];
   unsigned char datalen[MAX_MSG_ADMIN_LEN_SIZE + 1];
   unsigned char *pch;

   hx_log_msg(log, HX_LWARN, rname, "%s", VERSION);
   hx_log_msg(log, HX_LWARN, rname, "%s", COPYRIGHT);
   hx_log_msg(log, HX_LWARN, rname, "exit terminates session\n");

   /* clear the memory */
   memset(buffer, 0x00, MAX_ADMIN_REQUEST_LEN);
   memset(datalen, 0x00, MAX_MSG_ADMIN_LEN_SIZE);
   memset(data2send, 0x00, MAX_ADMIN_REQUEST_LEN);

   while(!done)
   {
      pch = data2send;
      *buffer = '\0';

      printf(">");
      while(1)
      {
         c = getchar();

         *pch = c;

         if( *pch == '\n')
         {
            *pch = '\0';

            sprintf(datalen, "%04d", strlen(data2send));
            strcat(buffer, datalen);

            if(!hx_strcmpi(data2send, "exit"))
            {
               done = 1;
               break;
            }

            *pch = CR;
            *(pch + 1) = LF;
            *(pch + 2) = '\0';

            strcat(buffer, data2send);

            /* connect to the host */
            s = hx_clientSock(log, host, port);

            if(ERRC != s)
            {
               /* send the request */
               hx_sockWrite (s, buffer, strlen(buffer));

               /* get the response */
               hx_getServerResponse(log, s);
            }
            else
            {
               done = 1;
            }

            /* close the socket */
            hx_sockClose(s, HX_BOTH);
            break;
         }
         else
            ++pch;
      }
   }
   return 1;
}
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include <errno.h>

#include "httpCGI.h"
#include "httpEnv.h"
#include "httpConf.h"
#include "httpCommon.h"
#include "httpInclude.h"
#include "httpRequest.h"
#include "httpExecutor.h"

const unsigned char *cgi_metaVars[MAX_META_VARS] =
{
   "AUTH_TYPE",
   "CONTENT_LENGTH",
   "CONTENT_TYPE",
   "GATEWAY_INTERFACE",
   "PATH_INFO",
   "PATH_TRANSLATED",
   "QUERY_STRING",
   "REMOTE_ADDR",
   "REMOTE_HOST",
   "REMOTE_IDENT",
   "REMOTE_USER",
   "REQUEST_METHOD",
   "SCRIPT_NAME",
   "SERVER_NAME",
   "SERVER_PORT",
   "SERVER_PROTOCOL",
   "SERVER_SOFTWARE"
};

static int
hx_get_child_data(hx_logsys_t *log,
                hx_process_t *child,
                unsigned char *buffer,
                long *buflen)
{
   const char    *rname = "hx_get_child_data ()";

   long rem = *buflen;
   long totRead = 0;

   unsigned char *pbuf = buffer;
   unsigned char *errStr;

   int timeout;
   int retval;

#ifdef _HX_WIN32_

   DWORD read = 0;
   HANDLE out;

#elif defined(_HX_UNIX_)

   int out;

#endif

   hx_log_msg(log, HX_LCALL, rname, "Called");

   /* Retrieve the server timeout from the
      config table */
   timeout = atoi((unsigned char *)hx_get_conf_val(log, IDX_SERVER_TIMEOUT));

#ifdef _HX_WIN32_

   out = child->childHandles[HX_STDOUT];

   while(1)
   {
      retval = ReadFile(out, pbuf, rem, &read, NULL);

      if(!retval || read == 0)
      {
         /* End of file Encountered */
         break;
      }

      pbuf += read;
      rem  -= read;
      totRead += read;

      if(rem <= 0)
         return HX_MOREDATA;
   }

#elif defined (_HX_UNIX_)

   out = child->parentStreams[HX_STDOUT];

   while(1)
   {
      retval = hx_readNonBlock(out, timeout, pbuf, rem);

      if(!retval || retval < 0)
      {
         /* End of file Encountered */
         if(retval < 0)
         {
            switch(retval)
            {
               case HX_ERR_CHILDREAD_TIMED_OUT:
                  errStr = "Child process read timed out";
                  break;
               case HX_ERR_CHILDWRITE_TIMED_OUT:
                  errStr = "Child process write timed out";
                  break;
               case HX_ERR_CHILDREAD:
                  errStr = "Error reading from child pipe";
                  break;
               case HX_ERR_IOCTL:
                  errStr = "Error performing ioctl on pipe";
                  break;
               case HX_ERR_SELECT:
                  errStr = "Error performing select";
                  break;
               default:
                  errStr = "Unkown error reading from pipe";
                  break;
            }

            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "%s [%s]",
                      errStr,
                      strerror(errno));
         }
         break;
      }

      pbuf += retval;
      rem  -= retval;
      totRead += retval;

      if(rem <= 0)
         return HX_MOREDATA;
   }

#endif

   *buflen = totRead;

   if(!retval)
      return HX_ENDOFDATA;
   else
      return retval;
}

static int
hx_getChildErrData(hx_logsys_t *log,
                   hx_process_t *child,
                   unsigned char *buffer,
                   long *buflen)
{
   const char    *rname = "hx_getChildErrData ()";

   long rem = *buflen;
   long totRead = 0;

   unsigned char *pbuf = buffer;
   unsigned char *errStr;

   int retval;
   int timeout;

#ifdef _HX_WIN32_

   DWORD read = 0;
   HANDLE out;

#elif defined(_HX_UNIX_)

   int out;

#endif

   hx_log_msg(log, HX_LCALL, rname, "Called");

   /* Retrieve the server timeout from the
      config table */
   timeout = atoi((unsigned char *)hx_get_conf_val(log, IDX_SERVER_TIMEOUT));

#ifdef _HX_WIN32_

   out = child->childHandles[HX_STDERR];

   while(1)
   {
      retval = ReadFile(out, pbuf, rem, &read, NULL);

      if(!retval || read == 0)
      {
         /* End of file Encountered */
         break;
      }

      pbuf += read;
      rem  -= read;
      totRead += read;

      if(rem <= 0)
         return HX_MOREDATA;
   }

#elif defined (_HX_UNIX_)

   out = child->parentStreams[HX_STDERR];

   while(1)
   {
      retval = hx_readNonBlock(out, timeout, pbuf, rem);

      if(!retval || retval < 0)
      {
         /* End of file Encountered */
         if(retval < 0)
         {
            switch(retval)
            {
               case HX_ERR_CHILDREAD_TIMED_OUT :
                  errStr = "Child read timed out";
                  break;
               case HX_ERR_CHILDWRITE_TIMED_OUT :
                  errStr = "Child write timed out";
                  break;
               case HX_ERR_CHILDREAD :
                  errStr = "Error reading from child pipe";
                  break;
               case HX_ERR_IOCTL :
                  errStr = "Error changing pipe mode";
                  break;
               default:
                  errStr = "Unkown error reading from pipe";
            }

            hx_log_msg(log,
                      HX_LDEBG,
                      rname,
                      "Read Error <%s> [%s]",
                      errStr,
                      strerror(errno));
         }
         break;
      }

      pbuf += retval;
      rem  -= retval;
      totRead += retval;

      if(rem <= 0)
         return HX_MOREDATA;
   }

#endif

   *buflen = totRead;

   if(!retval)
      return HX_ENDOFDATA;
   else
      return retval;
}

static int
hx_writeToChild(hx_logsys_t *log,
                hx_process_t *child,
                unsigned char *buffer,
                long buflen)
{
   const char    *rname = "hx_writeToChild ()";

   u_long bytesWritten = 0;
   u_long rem = buflen;

   unsigned char *pbuf = buffer;
   unsigned char *errStr;

   int retval;
   int timeout;

#ifdef _HX_WIN32_
   HANDLE in;
#elif defined(_HX_UNIX_)
   int in;
#endif

   hx_log_msg(log, HX_LCALL, rname, "Called");
   hx_log_msg(log, HX_LCALL, rname, "To Write %d bytes", buflen);

   /* Retrieve the server timeout from the
      config table */
   timeout = atoi((unsigned char *)hx_get_conf_val(log, IDX_SERVER_TIMEOUT));

#ifdef _HX_WIN32_

   in = child->childHandles[HX_STDIN];

   /* Send the body data to the
      child process */
   WriteFile(in, pbuf, rem, (LPDWORD) &bytesWritten, NULL);

#elif defined(_HX_UNIX_)

   in = child->parentStreams[HX_STDIN];

   while(1)
   {

      /* Send the body data to the
         child process */
      retval = hx_writeNonBlock(in, timeout, pbuf, rem);

      if(!retval || retval < 0)
      {
         /* End of file Encountered */
         if(retval < 0)
         {
            switch(retval)
            {
               case HX_ERR_CHILDWRITE_TIMED_OUT :
                  errStr = "Child write timed out";
                  break;
               case HX_ERR_CHILDWRITE :
                  errStr = "Error writing to pipe";
                  break;
               case HX_ERR_IOCTL :
                  errStr = "Error changing pipe mode";
                  break;
               default:
                  errStr = "Unkown error writing to pipe";
            }

            hx_log_msg(log,
                      HX_LDEBG,
                      rname,
                      "Write Error <%s>",
                      errStr);

            return retval;
         }
      }

      pbuf += retval;
      rem  -= retval;
      bytesWritten += retval;

      /* All data has been written to the
         stream */
      if(rem <= 0)
         break;
   }

#endif

   if(bytesWritten == 0)
      hx_log_msg(log,
                HX_LWARN,
                rname,
                "ZERO bytes passed to child process");
   else
      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "%d bytes passed to child process", bytesWritten);

   return OK;
}

static int
hx_makeEnv(http_request_t *request_object)
{
   const char    *rname = "hx_makeEnv ()";

   int retval;
   hx_envpair_t  *envpair;

   unsigned char *body;
   unsigned char  port[MAX_PORT_STR_LEN];
   unsigned char *scriptName;
   unsigned char *software;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* create the environment list */
   request_object->env = hx_create_list();

   if(!request_object->env)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "hx_create_list () failed");
      return ERRC;
   }

   if(IDX_GET == request_object->method_idx)
   {
      /* Check to see the existence of the
         query string */
      if(request_object->uri_object->queryStr)
      {

         /* form the QUERY_STRING env pair */
         envpair = hx_create_env_pair(request_object->vhost->log,
                                    cgi_metaVars[IDX_MV_QUERY_STRING],
                                    request_object->uri_object->queryStr,
                                    request_object->allocator);

         /* add it to the list */
         retval = hx_add(request_object->env, envpair);

         if(retval != OK)
         {
            hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "hx_add () failed");
            return ERRC;
         }
      }
      else
      {
         /* Add blank query string */
         envpair = (hx_envpair_t *)hx_alloc_mem(sizeof(hx_envpair_t),
                                             request_object->allocator);
         envpair->nameval = hx_strdup("QUERY_STRING=", request_object->allocator);
         hx_add(request_object->env, envpair);
      }
   }
   else if(IDX_POST == request_object->method_idx)
   {
      /* Add blank query string */
      envpair = (hx_envpair_t *)hx_alloc_mem(sizeof(hx_envpair_t), request_object->allocator);
      envpair->nameval = hx_strdup("QUERY_STRING=", request_object->allocator);
      hx_add(request_object->env, envpair);
   }

   /* add the remaining environment
      elements */

   /* GATEWAY_INTERFACE */
   envpair = hx_create_env_pair(request_object->vhost->log,
                              cgi_metaVars[IDX_MV_GATEWAY_INTERFACE],
                              "CGI/1.1", request_object->allocator);
   hx_add(request_object->env, envpair);

   /* REQUEST_METHOD */
   envpair = hx_create_env_pair(request_object->vhost->log,
                              cgi_metaVars[IDX_MV_REQUEST_METHOD],
                              method_names[request_object->method_idx],
                              request_object->allocator);
   hx_add(request_object->env, envpair);

   /* SERVER_NAME */
   envpair = hx_create_env_pair(request_object->vhost->log,
                              cgi_metaVars[IDX_MV_SERVER_NAME],
                              request_object->uri_object->host,
                              request_object->allocator);
   hx_add(request_object->env, envpair);

   /* SERVER_PORT */
   sprintf(port, "%d", request_object->uri_object->port);
   envpair = hx_create_env_pair(request_object->vhost->log,
                              cgi_metaVars[IDX_MV_SERVER_PORT],
                              port,
                              request_object->allocator);
   hx_add(request_object->env, envpair);

   /* SERVER_PROTOCOL */
   envpair = hx_create_env_pair(request_object->vhost->log,
                              cgi_metaVars[IDX_MV_SERVER_PROTOCOL],
                              (unsigned char *)hx_get_conf_val(request_object->vhost->log,
                                            IDX_HTTP_VERSION),
                              request_object->allocator);
   hx_add(request_object->env, envpair);

   /* SERVER_SOFTWARE */
   software = hx_alloc_mem((strlen(SERVER) + strlen(VERSION) + 2), request_object->allocator);
   *software = '\0';
   strcat(software, SERVER);
   strcat(software, "/");
   strcat(software, VERSION);

   envpair = hx_create_env_pair(request_object->vhost->log,
                              cgi_metaVars[IDX_MV_SERVER_SOFTWARE],
                              software,
                              request_object->allocator);
   hx_add(request_object->env, envpair);

   /* SCRIPT_NAME */
   if(request_object->uri_object->resource)
   {
      scriptName = (unsigned char *)hx_alloc_mem(strlen(request_object->uri_object->resource) + 2,
                                              request_object->allocator);

      *scriptName = '\0';
      strcat(scriptName, request_object->uri_object->resource);

      envpair = hx_create_env_pair(request_object->vhost->log,
                                 cgi_metaVars[IDX_MV_SCRIPT_NAME],
                                 scriptName,
                                 request_object->allocator);
      hx_add(request_object->env, envpair);
      hx_free_mem(scriptName, request_object->allocator);
   }

   /* REMOTE_ADDR */
   if(request_object->client_address)
   {
      envpair = hx_create_env_pair(request_object->vhost->log,
                                 cgi_metaVars[IDX_MV_REMOTE_ADDR],
                                 request_object->client_address, request_object->allocator);
      hx_add(request_object->env, envpair);
   }

   if(request_object->body_len)
   {
      /* CONTENT_LENGTH */
      envpair = hx_create_env_pair(request_object->vhost->log,
                                 cgi_metaVars[IDX_MV_CONTENT_LENGTH],
                                 hx_get_header_value(IDX_CONTENT_LENGTH, request_object), request_object->allocator);
      hx_add(request_object->env, envpair);

      /* CONTENT_TYPE */
      envpair = hx_create_env_pair(request_object->vhost->log,
                                 cgi_metaVars[IDX_MV_CONTENT_TYPE],
                                 hx_get_header_value(IDX_CONTENT_TYPE, request_object), request_object->allocator);
      hx_add(request_object->env, envpair);
   }

   return OK;
}

int
hx_formRespObj(http_response_t *response_object,
               http_request_t *request_object,
               const unsigned char *content_type)
{
   const unsigned char *rname = "hx_formRespObj ()";

   unsigned char *buffer;
   unsigned char  content_length[MAX_CONTENT_LENGTH_LEN];

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* get the content length string */
   *content_length = '\0';
   sprintf(content_length, "%d", response_object->body_len);

   /* form the response */
   /* allocate memory for buffer */
   buffer = (unsigned char *) hx_alloc_mem (MAX_STATUS_LINE_LEN +
                                         MAX_TOTAL_HEADER_LEN,
                                         request_object->allocator);

   if (!buffer)
   {
      return HX_ERR_MEM_ALLOC;
   }

   /* Add the status line to the
      response object */
   response_object->statusLine = hx_create_status_line(request_object->vhost->log,
                                            IDX_200,
                                            request_object->uri_object->resource,
                                            request_object->allocator);
   /* form the headers */
   *buffer = '\0';

   hx_add_response_header(request_object->vhost->log, buffer, IDX_CONTENT_TYPE, content_type);

   /* Add the response time */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_DATE, response_object->time);

   /* Add the server name */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_SERVER, SERVER);

   /* Add content length */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_CONTENT_LENGTH, content_length);

   /* Add location */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_LOCATION, "localhost");

   /* Add Terminating CRLF */
   strcat(buffer, CRLF);

   /* Embed the formed header to the
      response object */
   response_object->headers = hx_strdup (buffer, request_object->allocator);

   /* deallocate the memory */
   hx_free_mem(buffer, request_object->allocator);

   return OK;
}

int
hx_handle_cgi (http_request_t *request_object, http_response_t *response_object)
{
   unsigned const char     *rname = "hx_handle_cgi ()";
   hx_process_t   *child;
   int            retval;
   long           len;

   unsigned char  *buffer;
   unsigned char  *pbuf;
   unsigned char  content_type[MAX_CONTENT_TYPE_LEN];
   unsigned char  *pch;
   unsigned char  *start;
   unsigned char  *end;
   unsigned char  *val;
   http_request_t    *req;
   hx_bodydata_t  *body;
   hx_list_t      *bodyList;
   int            i;
   int            nodeCnt;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* Create the environment */
   retval = hx_makeEnv(request_object);

   if(retval != OK)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "hx_makeEnv () failed");
      hx_form_error_response(response_object, request_object, IDX_501, rname);
      return ERRC;
   }

   retval = hx_invokeCGI(request_object, &child);

   if(retval != OK)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "hx_invokeCGI () failed");
      hx_form_error_response(response_object, request_object, hx_get_response_code(retval), rname);
      hx_cleanProc(request_object->vhost->log, child);
      return ERRC;
   }

   hx_log_msg(request_object->vhost->log,
             HX_LDEBG,
             rname,
             "Script successfully invoked");

   /* Write data to child STDIN */
   if(request_object->body_len)
   {
      pch = request_object->body;
      len = request_object->body_len;

      retval = hx_writeToChild(request_object->vhost->log,
                               child,
                               pch,
                               len);

      if(retval <= ERRC)
      {
         hx_form_error_response(response_object, request_object, IDX_500, rname);
         hx_cleanProc(request_object->vhost->log, child);
         return ERRC;
      }
   }

   /* Create a list to hold the body data */
   bodyList = hx_create_list();
   len = 0;

   /* Read the data from the spawned CGI process */
   do
   {
      body = hx_alloc_mem(sizeof(hx_bodydata_t), request_object->allocator);
      body->data = hx_alloc_mem(MAX_BUFF_LEN, request_object->allocator);
      body->dataLen = MAX_BUFF_LEN;

      retval = hx_get_child_data(request_object->vhost->log,
                               child,
                               body->data,
                               &body->dataLen);
      if(retval < 0)
      {
         /* kill a brute child */
         hx_kill_child(request_object->vhost->log, child);

         hx_log_msg(request_object->vhost->log,
                   HX_LWARN,
                   rname,
                   "CGI process [ \"%s\" ] terminated",
                   child->cmdLine);

         /* The CGI timed out! */
         hx_form_error_response(response_object, request_object, IDX_500, rname);
         hx_cleanProc(request_object->vhost->log, child);
         return ERRC;
      }
      else
      {
         if(body->dataLen > 0)
         {
            hx_add(bodyList, body);
            /* Calculate the total length of the buffer required for
               holding the body contents */
            len += body->dataLen;
         }
         else
         {
            /* Get error data from stderr */
            hx_getChildErrData(request_object->vhost->log,
                               child,
                               body->data,
                               &body->dataLen);

            hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "CGI process error");
            hx_form_error_response(response_object, request_object, IDX_500, rname);
            hx_cleanProc(request_object->vhost->log, child);
            return ERRC;
         }
      }
   }
   while((retval != HX_ENDOFDATA) && (retval > ERRC));

   /* Clean up the child process */
   hx_cleanProc(request_object->vhost->log, child);

   if(len > 0)
   {
      /* Allocate memory for the response body */
      buffer = (unsigned char *)hx_alloc_mem(len, request_object->allocator);

      if(!buffer)
      {
         hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "Memory Allocation failed");
         hx_form_error_response(response_object, request_object, IDX_500, rname);
         return ERRC;
      }

      nodeCnt = hx_size(bodyList);
      pbuf = buffer;

      /* copy the data to the buffer */
      for (i = 0; i < nodeCnt; ++i)
      {
         body = (hx_bodydata_t *)hx_get_pos(bodyList, i);
         memcpy(pbuf, body->data, body->dataLen);
         pbuf += body->dataLen;
      }

      hx_destroy_list(bodyList);

      /* Dump the output to the log */
      hx_hex_dump(request_object->vhost->log, buffer, len, HX_LDUMP);

      /* Get the content type */
      pch = buffer;

      /* Skip initial whitespace */
      while(isspace(*pch))
         ++pch;

      /* check the script output type
         whether parsed headers or
         non-parsed headers */
      response_object->type = hx_checkOuput(pch, len);

      if(response_object->type == HX_NONPARSED)
      {
         /* the script output as it is
            to the response buffer */
         response_object->buffer_len = len;
         response_object->buffer = (unsigned char *)hx_alloc_mem(len, response_object->allocator);
         memcpy(response_object->buffer, pch, response_object->buffer_len);
      }
      else
      {
         /* get the headers */
         if(!hx_strncmpi("content-type", pch, strlen("content-type")))
         {
            /* get the content type */
            if((start = memchr(pch, ':', len)) != NULL)
            {
               /* skip the ':' */
               ++start;

               /* skip any white space */
               while(isspace(*start))
                  ++start;

               /* find the CRLF */
               end = start;

#ifdef _HX_WIN32_
               while(*end != CR)
                  ++end;
#elif defined(_HX_UNIX_)
               while(*end != LF)
                  ++end;
#endif

#ifdef _HX_WIN32_
               if((end[0] == CR) &&
                  (end[1] == LF))
               {
                  /*end of the content type string */
                  memcpy(content_type, start, (end - start));
                  content_type[end - start] = '\0';
               }
               else
               {
                  hx_form_error_response(response_object, request_object, IDX_500, rname);
                  return ERRC;
               }

               /* advance the ptr to point to the
                  start of the message body by finding
                  the next CRLF */
               start = end + 2;

               while(*start != CR && (start < (pch + len)))
                  ++start;

               if(start == (pch + len))
               {
                  hx_form_error_response(response_object, request_object, IDX_500, rname);
                  return ERRC;
               }

               if((start[0] == CR) &&
                  (start[1] == LF))
               {
                  start += 2;

                  /* copy the script output to the
                     response body */
                  response_object->body_len = len - (start - pch);
                  response_object->body = (unsigned char *)hx_alloc_mem(response_object->body_len,
                                                            response_object->allocator);
                  memcpy(response_object->body, start, response_object->body_len);
               }
               else
               {
                  hx_form_error_response(response_object, request_object, IDX_500, rname);
                  return ERRC;
               }
#elif defined(_HX_UNIX_)
               if(end[0] == LF)
               {
                  /*end of the content type string */
                  memcpy(content_type, start, (end - start));
                  content_type[end - start] = '\0';
               }
               else
               {
                  hx_form_error_response(response_object, request_object, IDX_500, rname);
                  return ERRC;
               }

               /* advance the ptr to point to the
                  start of the message body by finding
                  the next LF */
               start = end + 1;

               while(*start != LF && (start < (pch + len)))
                  ++start;

               if(start == (pch + len))
               {
                  hx_form_error_response(response_object, request_object, IDX_500, rname);
                  return ERRC;
               }

               if(start[0] == LF)
               {
                  start++;

                  /* copy the script output to the
                     response body */
                  response_object->body_len = len - (start - pch);
                  response_object->body = (unsigned char *)hx_alloc_mem(response_object->body_len,
                                                            response_object->allocator);
                  memcpy(response_object->body, start, response_object->body_len);
               }
               else
               {
                  hx_form_error_response(response_object, request_object, IDX_500, rname);
                  return ERRC;
               }
#endif
            }
            else
            {
               /* the ':' in the content-type header field
                  is mandatory */
               hx_form_error_response(response_object, request_object, IDX_500, rname);
               return ERRC;
            }
         }
         else
         {
            strcpy(content_type, "text/html");
            response_object->body_len = len;

            if(response_object->body_len)
            {
               response_object->body = (unsigned char *)hx_alloc_mem(len,
                                                         response_object->allocator);
               memcpy(response_object->body, pch, response_object->body_len);
            }
         }
      }

      /* form the response object */
      retval = hx_formRespObj(response_object, request_object, content_type);

      if(retval != OK)
      {
         hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "hx_formRespObj () failed");
         hx_form_error_response(response_object, request_object, IDX_500, rname);
         return ERRC;
      }
   }
   return OK;
}
/* End of File */

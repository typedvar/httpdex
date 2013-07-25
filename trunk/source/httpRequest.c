/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "httpRequest.h"
#include "httpConf.h"
#include "httpMime.h"
#include "httpSock.h"

const unsigned char *method_names[MAX_METHODS] =
{
   "OPTIONS",
   "HEAD",
   "POST",
   "PUT",
   "DELETE",
   "TRACE",
   "GET",
   "CONNECT"
};

/* hx_createReqObj ()
   Creates a http_request_t Object
*/
http_request_t *
hx_createReqObj(hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_createReqObj ()";

   http_request_t *request_object;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* allocate space for the
      request object */
   if((request_object = (http_request_t *)hx_alloc_mem(sizeof(http_request_t), allocator)) == NULL)
      return NULL;

   /* initiliaze the request object */
   memset(request_object, 0x00, sizeof(http_request_t));

   request_object->method_idx = NOMATCH;
   request_object->allocator = allocator;
   request_object->requestLen = 0;

   /* Create the request buffer */
   request_object->request = (unsigned char *)hx_alloc_mem(MAX_BUFF_LEN, request_object->allocator);

   if(!request_object->request)
      return NULL;

   /* Attach the default vhost system to the
      newly created request object. This
      Virtual Host system will be replaced
      by the original one after vhost lookup
      is performed */
   request_object->vhost = defaultVhost;

   return request_object;
}

/* hx_print_uri ()
   display the components of a URI Object
*/
void
hx_print_uri(hx_logsys_t *logsys, http_uri_t *uri_object, int log_level)
{
   const unsigned char *rname = "hx_print_uri ()";

   if(!uri_object)
      return;
   if(uri_object->host)
      hx_log_msg(logsys, log_level, rname, "[host      :%s]", uri_object->host);
   hx_log_msg(logsys, log_level, rname, "[port      :%d]", uri_object->port);
   if(uri_object->resource)
      hx_log_msg(logsys, log_level, rname, "[resource  :%s]", uri_object->resource);
   if(uri_object->queryStr)
      hx_log_msg(logsys, log_level, rname, "[query     :%s]", uri_object->queryStr);
}

void
hx_printHeaders(http_request_t *request_object, int log_level)
{
   const unsigned char *rname = "hx_printHeaders ()";
   char *space = " ";
   int i;

   if(hx_check_log_level(request_object->vhost->log, log_level))
   {
      if(request_object->num_headers)
      {
         hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "START Headers->");
         for(i = 0; i < request_object->num_headers; ++i)
         {
            if((strlen(request_object->header[i]->value) +
                MAX_HEADER_NAME_LEN) < MAX_LOG_BUFF_LEN)
            {
               hx_log_msg(request_object->vhost->log,
                         HX_LDEBG,
                         rname,
                         "   [%03d] %s: %s",
                         i + 1,
                         header_names_table[request_object->header[i]->header_idx],
                         ((request_object->header[i]->value) ?
                         (char *)(request_object->header[i]->value) : space));
            }
            else
            {
               hx_log_msg(request_object->vhost->log,
                         HX_LDEBG,
                         rname,
                         "   [%03d] %s: ",
                         i + 1,
                         header_names_table[request_object->header[i]->header_idx]);

               hx_hex_dump(request_object->vhost->log,
                          request_object->header[i]->value,
                          strlen(request_object->header[i]->value),
                          HX_LDUMP);
            }
         }
         hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "END Headers->");
      }
   }
}

/*
   hx_print_request ()
   Displays the http_request_t Object.
*/
void
hx_print_request (http_request_t *request_object, int loglvl)
{
   const unsigned char *rname = "hx_print_request ()";

   if(!hx_check_log_level(request_object->vhost->log, loglvl))
      return;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   if(!request_object)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "Invalid request object");
      return;
   }

   hx_log_msg(request_object->vhost->log,
             loglvl,
             rname,
             "=============="
             "=============="
             "==============");

   hx_log_msg(request_object->vhost->log,
             loglvl,
             rname,
             "Incoming request from %s",
             request_object->client_address);

   if (request_object->method_idx > -1)
      hx_log_msg(request_object->vhost->log,
                loglvl,
                rname,
                "Method : %s",
                method_names[request_object->method_idx]);
   else
      hx_log_msg(request_object->vhost->log,
                loglvl,
                rname,
                "Method : INVALID METHOD");

   if(request_object->uriLen)
   {
      hx_log_msg(request_object->vhost->log,
                loglvl,
                rname,
                "URI : %.*s",
                request_object->uriLen,
                request_object->uri);

      hx_print_uri(request_object->vhost->log, request_object->uri_object, loglvl);
   }

   hx_print_mime_type(request_object->vhost->log, request_object->mime_object, loglvl);

   if(request_object->filename)
      hx_log_msg(request_object->vhost->log,
                loglvl,
                rname,
                "Filename : %s",
                request_object->filename);

   if(request_object->version_len)
      hx_log_msg(request_object->vhost->log,
                loglvl,
                rname,
                "Version : %.*s",
                request_object->version_len,
                request_object->version);

   hx_log_msg(request_object->vhost->log,
             loglvl,
             rname,
             "Request line : %s",
             request_object->request_line);

   hx_printHeaders(request_object, loglvl);

   if(request_object->body_len)
   {
      hx_log_msg(request_object->vhost->log, loglvl, rname, "BODY START:");
      hx_hex_dump(request_object->vhost->log, request_object->body, request_object->body_len, HX_LDUMP);
      hx_log_msg(request_object->vhost->log, loglvl, rname, "  BODY END:");
   }

   hx_log_msg(request_object->vhost->log,
             loglvl,
             rname,
             "=============="
             "=============="
             "==============");
}

static int
hx_getRequestData(http_request_t *request_object,
                  unsigned char *buffer,
                  long *bytes_read,
                  long capacity)
{
   const unsigned char *rname = "hx_getRequestData ()";

   int err = 0;
   int retval;
   unsigned char *pch;

   pch = buffer;
   *bytes_read = 0;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

#ifdef _HX_WIN32_
   do
   {
      retval = hx_sock_read (request_object->sock, pch, capacity);
      if(retval == SOCKET_ERROR)
         err = hx_get_socket_error();
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
      else
         break;
   }
   while((err != WSAEWOULDBLOCK) && capacity);
#elif defined (_HX_UNIX_)
   do
   {
      retval = hx_sock_read (request_object->sock, pch, capacity);
      if(retval == -1)
         err = hx_get_socket_error();
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
      else
         break;
   }
   while((err != EAGAIN) && capacity );
#endif

   /* Display request data */
   hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "Request START");
   hx_hex_dump(request_object->vhost->log, buffer, *bytes_read, HX_LDUMP);
   hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "Request END");

   return retval;
}

static int
hx_getLine(http_request_t *request_object,
           unsigned char *buffer,
           long *bytes_read,
           long capacity)
{
   const unsigned char *rname = "hx_getLine ()";

   int err = 0;
   int retval;
   unsigned char *pch;
   unsigned char *end;
   hx_bool_t done = HX_FALSE;

   pch = buffer;
   *bytes_read = 0;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

#ifdef _HX_WIN32_
   do
   {
      retval = hx_sock_read (request_object->sock, pch, capacity);

      if(retval == SOCKET_ERROR)
         err = hx_get_socket_error();
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
      else
      {
         retval = HX_ERR_CLIENT;
         break;
      }

      /* Check whether we got the newline */
      end = pch;
      while(end > buffer)
      {
         if((LF == *end) &&
            (CR == *(end - 1)))
         {
            /* CRLF encountered */
            done = HX_TRUE;
            break;
         }

         --end;
      }
   }
   while((done == HX_FALSE) &&
         (err != WSAEWOULDBLOCK) &&
         (capacity > 0));
#elif defined(_HX_UNIX_)
   do
   {
      retval = hx_sock_read (request_object->sock, pch, capacity);

      if(retval == -1)
         err = hx_get_socket_error();
      else if (retval > 0)
      {
         capacity -= retval;
         pch += retval;
         *bytes_read += retval;
      }
      else
      {
         retval = HX_ERR_CLIENT;
         break;
      }

      /* Check whether we got the newline */
      end = pch;
      while(end > buffer)
      {
         if((LF == *end) &&
            (CR == *(end - 1)))
         {
            /* CRLF encountered */
            done = HX_TRUE;
            break;
         }

         --end;
      }
   }
   while((done == HX_FALSE) &&
         (err != EAGAIN) &&
         (capacity > 0));
#endif

   /* Display request data */
   hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "Request START");
   hx_hex_dump(request_object->vhost->log, buffer, *bytes_read, HX_LDUMP);
   hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "Request END");

   return retval;
}

/* hx_parse_uri ()
   This function parses the request Line
   and breaks the URI string into its
   components
*/
int
hx_parse_uri (http_request_t *request_object)
{
   const unsigned char *rname = "hx_parse_uri ()";

   unsigned char *uri;
   unsigned char *pch;

   unsigned char *startptr;
   unsigned char *endptr;

   int remLen = request_object->uriLen;

   hx_log_msg(request_object->vhost->log,
             HX_LCALL,
             rname,
             "Called on \"%.*s\"",
             request_object->uriLen,
             request_object->uri);

   /* allocate space for the uri object */
   request_object->uri_object = (http_uri_t *)hx_alloc_mem(sizeof(http_uri_t), request_object->allocator);

   /* create a null terminated uri string */
   uri = (unsigned char *) hx_alloc_mem (request_object->uriLen + 1, request_object->allocator);
   memcpy (uri, request_object->uri, request_object->uriLen);
   uri[request_object->uriLen] = '\0';

   /* assign a ptr for string manipulation */
   startptr = uri;

   /* initialize the uri_object */
   memset (request_object->uri_object, 0x00, sizeof(http_uri_t));

   /* Check to see if the resource
      name is fully quantified or relative */
   /* If uri starts with '/' then its relative */
   if ( startptr[0] == '/' )
   {
      endptr = startptr + remLen;
      *endptr = '\0';

      /* resource name is relative */
      /* check to see if there is a query string */
      if((pch = memchr((startptr + 1), '?', remLen)) != NULL)
      {
         /* if '?abd' is the resource name then error
            out */
         if(pch == (startptr + 1))
            return HX_ERR_INVALID_URI;

         /* if we only located a '?' and no args */
         if((endptr - pch) < 2)
            return HX_ERR_INVALID_URI;

         /* get the resource name */
         *pch = '\0';

         request_object->uri_object->resource = (unsigned char *) hx_alloc_mem (pch - startptr,
                                                            request_object->allocator);
         strcpy(request_object->uri_object->resource, startptr + 1);

         /* skip the ':' */
         pch++;

         /* get the query args */
         request_object->uri_object->queryStr = (unsigned char *) hx_alloc_mem (endptr - pch + 1,
                                                            request_object->allocator);
         strcpy(request_object->uri_object->queryStr, pch);
      }
      else
      {
         /* get the resource name */
         request_object->uri_object->resource = hx_strdup(startptr, request_object->allocator);
      }
   }
   else
   {
      /* URI -> http://hostname[:port]/[resource[?args]] */
      /* currently we don't support username:password */
      /* uri is fully quantified */
      /* get the scheme */
#ifdef _HX_WIN32_
      if(strncmpi("http://", startptr, 7))
#else
      if(strncasecmp("http://", startptr, 7))
#endif
         return HX_ERR_INVALID_URI;

      /* adjust the remaining len */
      remLen -= 7;
      startptr += 7;

      if(remLen < 2)
         return HX_ERR_INVALID_URI;

      /* get the host[:port] */
      /* Locate the '/' terminating the host[:port] */
      if ((endptr = memchr (startptr, '/', remLen)) == NULL)
      {
         return HX_ERR_INVALID_URI;
      }

      /*      url-> http://.{+}/... */
      /* startptr->        ^     */
      /*   endptr->            ^ */

      /* check to see the occurence of ':' */
      if((pch = memchr(startptr, ':', endptr - startptr)) != NULL)
      {
         /*      url-> http://hostname:port/... */
         /* startptr->        ^              */
         /*      pch->                ^      */
         /*   endptr->                     ^ */

         /* copy the host name */
         *pch = '\0';
         request_object->uri_object->host = (unsigned char *) hx_alloc_mem (pch - startptr + 1,
                                                        request_object->allocator);
         strcpy(request_object->uri_object->host, startptr);

         /* colon is supplied but no port number! */
         /*         -> http://localhost:/... */
         /*         ->                 ^^    */
         if((endptr - pch) < 2)
            return HX_ERR_INVALID_URI;

         /* skip the ':' */
         pch++;

         /* copy the port */
         *endptr = '\0';
         request_object->uri_object->port = (short)atoi(pch);
      }
      else
      {
         /* copy the host name */
         *endptr = '\0';
         request_object->uri_object->host = (unsigned char *) hx_alloc_mem (endptr - startptr + 1,
                                                        request_object->allocator);
         strcpy(request_object->uri_object->host, startptr);

         /* set the port to the default */
         request_object->uri_object->port = (short)atoi((unsigned char *)
                                        hx_get_conf_val(request_object->vhost->log,
                                                      IDX_DEFAULT_HTTP_PORT));
      }

      /* Advance the ptr */
      remLen -= endptr - startptr;
      startptr = endptr;

      /* reassign the startptr value to '/' */
      *startptr = '/';

      if(remLen > 1)
      {
         endptr += remLen;

         /*      url-> http://hostname[:port]/resource[?args] */
         /* startptr->                       ^                */
         /*   endptr->                                      ^ */

         /* null terminate the string */
         *endptr = '\0';

         /* check to see if there is a query arg */
         if((pch = memchr(startptr, '?', remLen)) != NULL)
         {
            /* if we only located a '?' and no args */
            if((endptr - pch) < 2)
               return HX_ERR_INVALID_URI;

            *pch = '\0';

            /* get the resource name */
            request_object->uri_object->resource = hx_strdup(startptr, request_object->allocator);

            /* skip the '?' */
            ++pch;

            /* get the query args */
            request_object->uri_object->queryStr = hx_strdup(pch, request_object->allocator);
         }
         else
         {
            /* get the resource name */
            request_object->uri_object->resource = hx_strdup(startptr, request_object->allocator);
         }
      }
      else if(remLen == 1)
      {
         request_object->uri_object->resource = hx_strdup("/", request_object->allocator);
      }
   }

   /* Transform %xx sequences in the
      resource string to its appropriate
      form */
   if(request_object->uri_object->resource)
   {
      hx_unescape(request_object->vhost->log,
                  request_object->uri_object->resource,
                  request_object->allocator);

      if(hx_checkres(request_object->vhost->log, request_object->uri_object->resource, request_object->allocator) != OK)
      {
         hx_log_msg(request_object->vhost->log,
                   HX_LERRO,
                   rname,
                   "Invalid URI %s requested",
                   request_object->uri_object->resource);

         return HX_ERR_INVALID_URI;
      }

      /* free the resource object */
      if(!strlen(request_object->uri_object->resource))
      {
         hx_free_mem(request_object->uri_object->resource, request_object->allocator);
         request_object->uri_object->resource = NULL;
      }

      /* get the Mime type of the requested
         resource */
      request_object->mime_object = hx_lookup_resource_mime(request_object->vhost->log,
                                    request_object->uri_object->resource);
      if(!request_object->mime_object)
         hx_log_msg(request_object->vhost->log,
                   HX_LWARN,
                   rname,
                   "Mime type of the requested resource could not "
                   "be determined");
   }

   return HX_PARSE_OK;
}

/*
   hx_add_header ()
   Adds a header string to the HTTP Request
   object.
*/
int
hx_add_header (http_request_t *request_object, const unsigned char *request_header)
{
   const unsigned char *rname = "hx_add_header ()";

   http_header_t *newHeader;
   unsigned char *startptr;
   unsigned char *pch;

   unsigned char *temp;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   newHeader = (http_header_t *)hx_alloc_mem(sizeof(http_header_t),
                                       request_object->allocator);

   if(!newHeader)
      return HX_ERR_INTERNAL;

   temp= (unsigned char *)hx_alloc_mem(strlen(request_header) + 1,
                                    request_object->allocator);

   if (!temp)
      return HX_ERR_INTERNAL;

   /* copy the header into the temp
      for operations */
   strcpy(temp, request_header);

   startptr = temp;

   pch = strchr(startptr, ':');

   /* If colon not found then the header is
      invalid */
   if(!pch)
      return HX_ERR_INVALID_HEADER;

   /* replace ':' with null */
   *pch = '\0';

   /* Locate the appropriate header
      from the header dictionary */
   if((newHeader->header_idx = hx_match_string_idx (header_names_table,
                                               MAX_HEADERS,
                                               startptr)) == NOMATCH)
   {
      return HX_ERR_INVALID_HEADER;
   }

   /* message-header = field-name ":" [ field-value ] */
   /* as such field-value is optional */
   /* skip the ':' */
   startptr = pch + 1;

   /* skip any LWS */
   while(isspace(*startptr) && *startptr)
      startptr++;

   if(strlen(startptr))
      newHeader->value = hx_strdup(startptr, request_object->allocator);

   request_object->header[request_object->num_headers] = newHeader;
   request_object->num_headers++;
   return request_object->num_headers;
}

/* hx_get_header_value ()
   precondition: headers must be parsed into the http_request_t
                 object
*/
const unsigned char *
hx_get_header_value (int index, http_request_t *request_object)
{
   const unsigned char *rname = "hx_get_header_value ()";
   unsigned char *value = 0;
   int   counter;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   if (!request_object->num_headers)
      return NULL;

   for (counter = 0; counter < request_object->num_headers; counter++)
   {
      if(request_object->header[counter]->header_idx == index)
      {
         value = request_object->header[counter]->value;
         break;
      }
   }

   return value;
}

/*
   hx_parse_body()
   parse the request and extracts the body
   of the http request. The extracted body
   is then copied into the request object.
*/
int
hx_parse_body (http_request_t *request_object)
{
   const unsigned char *rname = "hx_parse_body ()";

   unsigned char *val;
   long contentLen;
   long remainingLen;
   long bytes_read = 0;

   int retval;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* Check to see if we have
      any content-length or
      transfer encoding headers */
   val = (unsigned char *)hx_get_header_value(IDX_CONTENT_LENGTH, request_object);

   if(!val)
   {
      val = (unsigned char *)hx_get_header_value(IDX_TRANSFER_ENCODING, request_object);

      if(!val)
      {
         request_object->requestLen = 0;
         return HX_PARSE_OK;
      }
   }
   else
   {
      contentLen = atol(val);
   }

   if(contentLen > 0)
   {
      /* retrieve remaining data */
      remainingLen = contentLen - request_object->requestLen;

      /* allocate the body buffer */
      request_object->body = (unsigned char *)hx_alloc_mem(contentLen, request_object->allocator);

      /* copy existing data if any */
      if(request_object->requestLen)
      {
         memcpy(request_object->body, request_object->request, request_object->requestLen);
         request_object->body_len += request_object->requestLen;
      }

      if(remainingLen > 0)
      {
         retval = hx_getRequestData(request_object,
                                    (request_object->body + request_object->body_len),
                                    &bytes_read,
                                    remainingLen);
         request_object->body_len += bytes_read;

         if(retval == ERRC)
            return HX_ERR_INTERNAL;

         if(!retval)
            return HX_ERR_CLIENT;
      }
   }

   return HX_PARSE_OK;
}

/*
   hx_parse_headers()
   parse the request and extracts the headers.
   Forms a structure of the same. If request
   is not a valid http request returns
   HX_ERR_INVALID_REQUEST.
*/
int
hx_parse_headers (http_request_t *request_object)
{
   const unsigned char *rname = "hx_parse_headers ()";

   unsigned char request_header[MAX_HEADER_LEN + 1];
   hx_list_t *headerList;

   int i = 0, j;
   int counter = 0;
   int char_cnt;
   int retval;

   unsigned char ch;
   unsigned char *request = request_object->request;
   unsigned char *buffer;
   unsigned char *pbuf;

   long remLen = request_object->requestLen;
   long totHeaderLen = 0;

   hx_bool_t foundHeader = HX_FALSE;
   hx_bool_t isCurrCRLF = HX_FALSE;
   hx_bool_t isCurrSpace = HX_FALSE;
   hx_bool_t isCurrHeader = HX_FALSE;
   hx_bool_t extractedHeaders = HX_FALSE;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* initialize the header count to 0 */
   request_object->num_headers = 0;

   while(!extractedHeaders)
   {
      if(remLen <= 0)
      {
         /* Get request header data */
         retval = hx_getLine(request_object, request_object->request, &request_object->requestLen, MAX_BUFF_LEN);

         if(retval == ERRC || retval == HX_ERR_CLIENT)
            return HX_ERR_INTERNAL;

         i = 0;
         remLen = request_object->requestLen;
      }

      /* create a list to hold the
         current header data */
      headerList = hx_create_list();
      request_header[0] = '\0';

      /* extract headers one by one */
      while(!isCurrHeader)
      {
         if(remLen <= 0)
         {
            retval = hx_getLine(request_object, request_object->request, &request_object->requestLen, MAX_BUFF_LEN);

            if(retval == ERRC || retval == HX_ERR_CLIENT)
               return HX_ERR_INTERNAL;

            i = 0;
            remLen = request_object->requestLen;
         }

         ch =  request[i];

         if(isspace(ch))
            isCurrSpace = HX_TRUE;

         /* This check happens only once */
         if(!foundHeader)
         {
            if(!isCurrSpace &&
               ch > 31 &&
               ch != 127 &&
               ch != CR &&
               ch != LF)
               foundHeader = HX_TRUE;
         }

         /*
            set isCurrCRLF on occurence of CRLF
         */
         if(memcmp((request + i), CRLF, 2) == 0)
         {
            isCurrHeader = HX_TRUE;

            if (isCurrCRLF)
              extractedHeaders = HX_TRUE;
            else
              isCurrCRLF = HX_TRUE;

            i += 2;
            remLen -= 2;
            break;
         }
         else
         {
            ++i;
            --remLen;
            isCurrCRLF = HX_FALSE;
            isCurrHeader = HX_FALSE;
         }

         if(counter < MAX_HEADER_LEN)
            request_header[counter++] = ch;
         else
         {
            request_header[counter + 1] = '\0';
            totHeaderLen += strlen(request_header);
            hx_add(headerList, hx_strdup(request_header, request_object->allocator));
            counter = 0;
         }
      }

      /* header has not been found */
      if(!foundHeader)
         break;

      /* null terminate the current header */
      request_header[counter] = '\0';

      totHeaderLen += strlen(request_header);

      /* add it to the header list */
      hx_add(headerList, hx_strdup(request_header, request_object->allocator));

      /* add the currently extracted header */
      if((counter > 0) && (isCurrHeader == HX_TRUE))
      {
         /* allocate a buffer large enough to hold all
            header fragments */
         buffer = (unsigned char *)hx_alloc_mem(totHeaderLen + 1, request_object->allocator);
         *buffer = '\0';

         /* extract the data from the list
            into the buffer */
         for (j = 0; j < hx_size(headerList); ++j)
         {
            strcat(buffer, (unsigned char *)hx_get_pos(headerList, j));
         }

         hx_add_header(request_object, buffer);

         /* reset the counter */
         counter = 0;
         totHeaderLen = 0;
         isCurrHeader = HX_FALSE;
      }

      /* delete the list */
      hx_destroy_list(headerList);
   }

   if(HX_FALSE == isCurrCRLF)
   {
      hx_log_msg(request_object->vhost->log,
                HX_LERRO,
                rname,
                "Invalid Header");
      return HX_ERR_INVALID_HEADER;
   }

   /* modify the request buffer for
      processing remaining data */
   /* Update the position */
   request_object->requestLen = remLen;

   /* move the memory data to the start */
   memmove(request_object->request, (request_object->request + i), request_object->requestLen);


   return HX_PARSE_OK;
}

/*
   hx_parse_request_line()
   parse the request and extracts the request line
   of the http request. Forms a structure of the
   same. If request is not a valid http request
   return HX_ERR_INVALID_REQUEST.
*/
int
hx_parse_request_line (http_request_t *request_object)
{
   const unsigned char *rname = "hx_parse_request_line ()";

   int idx;
   int counter;
   int retval;
   int timeLen;
   int remLen;

   const unsigned char *request = request_object->request;

   unsigned char ch;
   unsigned char request_line[MAX_REQUEST_LINE_LEN + 1];
   unsigned char *startptr = request_line;
   unsigned char *pch;

   hx_bool_t isCRLF = HX_FALSE;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* get the request line */
   hx_getLine(request_object, request_object->request, &request_object->requestLen, MAX_REQUEST_LINE_LEN);

   /* if nothing is read */
   if(!request_object->requestLen)
      return HX_ERR_BAD_REQUEST;

   /* embed the request reception time */
   timeLen = hx_getTime(request_object->time, MAX_TIME_LEN);
   request_object->time[timeLen] = '\0';

   /* get the request line */
   /* Request-Line = Method SP Request-URI SP HTTP-Version CRLF */
   /* note: all components of the request line are mandatory */
   for(idx = 0, counter = 0; idx < request_object->requestLen; ++idx)
   {
      ch = request[idx];

      if(ch == CR)
      {
         ch = request[idx + 1];
         if(ch == LF)
         {
            /* found terminating CRLF */
            isCRLF = HX_TRUE;
            /* increment the index */
            idx += 2;
            break;
         }
         else
         {
            /* invalid CR found in request line */
            return HX_ERR_BAD_REQUEST;
         }
      }
      request_line[counter++] = ch;
   }

   /* If terminating CRLF is not found
      within the request return
      REQUEST URI LARGE */
   if(!isCRLF)
   {
      /* copy the request line till now
         into the request object */
      request_object->request_line = (unsigned char *)hx_alloc_mem(MAX_REQUEST_LINE_LEN + 1,
                                                  request_object->allocator);
      memcpy(request_object->request_line, request_line, MAX_REQUEST_LINE_LEN);
      request_object->request_line[MAX_REQUEST_LINE_LEN] = '\0';
      return HX_ERR_REQUEST_URI_LARGE;
   }

   /* modify the request buffer for
      processing remaining data */
   /* Update the position */
   request_object->requestLen -= idx;

   /* move the memory data to the start */
   memmove(request_object->request, (request_object->request + idx), request_object->requestLen);

   request_line[counter] = '\0';
   request_object->request_line = hx_strdup(request_line, request_object->allocator);

   /* get the length of the request Line */
   remLen = counter;

   /* if method is not present then the request is
      invalid */
   if((pch = memchr(startptr, ' ', remLen)) == NULL)
      return HX_ERR_BAD_REQUEST;

   if((request_object->method_idx = hx_matchIdx (method_names,
                                    MAX_METHODS,
                                    startptr,
                                    (pch - startptr - 1))) == NOMATCH)
   {

      hx_log_msg(request_object->vhost->log,
                HX_LERRO,
                rname,
                "Invalid method in request");

      return HX_ERR_BAD_REQUEST;
   }

   if(request_object->method_idx == IDX_HEAD)
      return HX_PARSE_COMPLETE;

   /* extract the request uri */
   remLen -= pch - startptr;
   startptr = pch + 1;
   pch = memchr(startptr, ' ', remLen);

   /* if uri is not present then the request is
      invalid */
   if(!pch)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "uri not in request");
      return HX_ERR_BAD_REQUEST;
   }

   request_object->uriLen = pch - startptr;

   /* remove trailing '/' from the URI
      The URI may contain only a '/'. In
      that case do not remove it
   if((request_object->uriLen > 1) && (startptr[request_object->uriLen - 1] == '/'))
      --request_object->uriLen;
   */

   if(request_object->uriLen)
   {
      request_object->uri = hx_strndup(startptr, request_object->uriLen, request_object->allocator);

      /* break the URI into its components */
      retval = hx_parse_uri (request_object);

      if(retval == HX_ERR_INVALID_URI)
         return HX_ERR_INVALID_REQUEST;
   }

   /* extract the HTTP Version */
   remLen -= pch - startptr;
   startptr = pch + 1;

   if(!remLen)
      return HX_ERR_BAD_REQUEST;

   request_object->version_len = remLen;
   request_object->version = hx_strndup(startptr, remLen, request_object->allocator);

   return HX_PARSE_OK;
}

/*
   hx_parse_request()
   parse the incoming char buffer of length len
   as a http request. Form a structure of the same
   and send it back. If request is not a valid http
   request return HX_ERR_INVALID_REQUEST.
*/
int
hx_parse_request (http_request_t *request_object)
{
   const unsigned char *rname = "hx_parse_request ()";

   int retval;
   const unsigned char *hostinfo;
   unsigned char *hostname;
   unsigned char *pch;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* parse the request line */
   retval = hx_parse_request_line(request_object);

   if (retval == HX_PARSE_COMPLETE)
   {
      retval = HX_PARSE_OK;
      return retval;
   }

   if (retval != HX_PARSE_OK)
      return retval;

   hx_log_msg(request_object->vhost->log,
             HX_LDEBG,
             rname,
             "Request line PARSED");

   /* Parse the header data */
   retval = hx_parse_headers(request_object);

   /* check to if we have the gathered the
      proper host info, if not get it from the
      header */
   if(!request_object->uri_object->host)
   {
      /* need to extract host info
         from the header */
      hostinfo = hx_get_header_value(IDX_HOST, request_object);

      if(!hostinfo)
      {
         /* get the default host name */
         hostname = (unsigned char *)hx_get_conf_val(request_object->vhost->log,
                                                   IDX_DEFAULT_HOST_NAME);

         request_object->uri_object->host = hx_strdup(hostname, request_object->allocator);

         if(!request_object->uri_object->host)
            return HX_ERR_INTERNAL;

         /* set the port to the default */
         request_object->uri_object->port = (short)atoi((unsigned char *)
                                       hx_get_conf_val(request_object->vhost->log,
                                                     IDX_DEFAULT_HTTP_PORT));
      }
      else
      {
         hostname = hx_strdup(hostinfo, request_object->allocator);

         /* host:port is found */
         if((pch = strchr(hostname, ':')) != NULL)
         {
            /* get the host */
            *pch = '\0';
            request_object->uri_object->host = hx_strdup(hostname, request_object->allocator);

            if(!request_object->uri_object->host)
               return HX_ERR_INTERNAL;

            /* get the port */
            /* skip the ':' */
            pch++;
            request_object->uri_object->port = (short)atoi(pch);
         }
         else
         {
            request_object->uri_object->host = hx_strdup(hostname, request_object->allocator);

            if(!request_object->uri_object->host)
               return HX_ERR_INTERNAL;

            /* set the port to the default */
            request_object->uri_object->port = (short)atoi((unsigned char *)
                                     hx_get_conf_val(request_object->vhost->log,
                                                   IDX_DEFAULT_HTTP_PORT));
         }
      }
   }

   if(retval != HX_PARSE_OK)
      return HX_ERR_INVALID_REQUEST;

   hx_log_msg(request_object->vhost->log,
             HX_LDEBG,
             rname,
             "Request headers PARSED");

   /* right now hx_parse_body doesn't return
      anything useful, but keep the option
      open for future */
   retval = hx_parse_body(request_object);

   /* Check to see if the client
      has errored out */
   if(retval == HX_ERR_CLIENT)
      return retval;

   if(retval != HX_PARSE_OK)
      return HX_ERR_INVALID_REQUEST;

   return retval;
}
/* End of File */

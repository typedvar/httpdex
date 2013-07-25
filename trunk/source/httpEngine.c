/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpEngine.h"
#include "httpResponse.h"
#include "httpInclude.h"

/* HTTP Method handler function table */
const fhandler_t g_engine[MAX_METHODS] =
{
   hx_OPTIONS,
   hx_HEAD,     /* Implemented */
   hx_POST,
   hx_PUT,
   hx_DELETE,
   hx_TRACE,
   hx_GET,      /* Implemented */
   hx_CONNECT,
};

int hx_OPTIONS (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_OPTIONS ()";

   hx_log_msg(request_object->vhost->log,
             HX_LWARN,
             rname,
             "%s NOT IMPLEMENTED",
             method_names[request_object->method_idx]);
   hx_form_error_response(response_object, request_object, IDX_501, rname);
   return ERRC;
}

int hx_HEAD (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_HEAD ()";

   char buffer[MAX_STATUS_LINE_LEN + MAX_TOTAL_HEADER_LEN];

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   *buffer = '\0';

   /* send only headers */

   /* form the status line */
   response_object->statusLine = hx_create_status_line(request_object->vhost->log,
                                            IDX_200,
                                            (const unsigned char *)NULL,
                                            request_object->allocator);

   /* form the headers */
   *buffer = '\0';

   /* Add content length */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_CONNECTION, "close");

   /* Add the content type */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_CONTENT_TYPE, "text/html");

   /* Add the response time */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_DATE, response_object->time);

   /* Add the server name */
   hx_add_response_header(request_object->vhost->log, buffer, IDX_SERVER, SERVER);

   /* Add Terminating CRLF */
   strcat(buffer, CRLF);

   /* Embed the formed header to the
   response object */
   response_object->headers = hx_strdup (buffer, request_object->allocator);

   return OK;
}

int hx_POST (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_POST ()";

   hx_log_msg(request_object->vhost->log,
             HX_LWARN,
             rname,
             "%s NOT IMPLEMENTED",
             method_names[request_object->method_idx]);

   hx_form_error_response(response_object, request_object, IDX_501, rname);
   return ERRC;
}

int hx_PUT (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_PUT ()";

   hx_log_msg(request_object->vhost->log,
             HX_LWARN, rname,
             "%s NOT IMPLEMENTED",
             method_names[request_object->method_idx]);

   hx_form_error_response(response_object, request_object, IDX_501, rname);
   return ERRC;
}

int hx_DELETE (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_DELETE ()";

   hx_log_msg(request_object->vhost->log,
             HX_LWARN,
             rname,
             "%s NOT IMPLEMENTED",
             method_names[request_object->method_idx]);

   hx_form_error_response(response_object, request_object, IDX_501, rname);
   return ERRC;
}

int hx_TRACE (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_TRACE ()";

   hx_log_msg(request_object->vhost->log,
             HX_LWARN,
             rname,
             "%s NOT IMPLEMENTED",
             method_names[request_object->method_idx]);

   hx_form_error_response(response_object, request_object, IDX_501, rname);
   return ERRC;
}

int hx_GET (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_GET ()";

   int retval;

   unsigned char *buffer;
   char  content_length[MAX_CONTENT_LENGTH_LEN];
   char  content_type[MAX_CONTENT_TYPE_LEN];

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* get the contents of the resource */
   retval = hx_read_file(request_object->vhost->log,
                        request_object->filename,
                        &response_object->body,
                        &(response_object->body_len),
                        request_object->allocator,
                        lock_errno);

   if(retval != OK)
   {
      hx_form_error_response(response_object, request_object, hx_get_response_code(retval), rname);
      return ERRC;
   }


   hx_log_msg(request_object->vhost->log, HX_LDEBG,
             rname,
             "%d bytes read from file into buffer",
             response_object->body_len);

   if(hx_check_log_level(request_object->vhost->log, HX_LDUMP))
      hx_hex_dump(request_object->vhost->log, response_object->body, response_object->body_len, HX_LDUMP);

   /* get the content length string */
   *content_length = '\0';
   sprintf(content_length, "%d", response_object->body_len);

   /* form the response */
   /* allocate memory for buffer */
   buffer = (unsigned char *) hx_alloc_mem ( MAX_STATUS_LINE_LEN +
                                             MAX_TOTAL_HEADER_LEN,
                                             request_object->allocator );

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

   if(request_object->mime_object)
   {
      /* form the content type string */
      *content_type = '\0';

      strcat (content_type, hx_get_mime_type(request_object->mime_object));
      strcat (content_type, "/");
      strcat (content_type, hx_get_mime_sub_type(request_object->mime_object));

      /* Add the content type */
      hx_add_response_header(request_object->vhost->log, buffer, IDX_CONTENT_TYPE, content_type);
   }
   else
   {
      hx_add_response_header(request_object->vhost->log, buffer, IDX_CONTENT_TYPE, "text/plain");
   }

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

int hx_CONNECT (http_request_t *request_object, http_response_t *response_object)
{
   const unsigned char *rname = "hx_CONNECT ()";

   hx_log_msg(request_object->vhost->log,
             HX_LWARN,
             rname,
             "%s NOT IMPLEMENTED",
             method_names[request_object->method_idx]);

   hx_form_error_response(response_object, request_object, IDX_501, rname);
   return ERRC;
}

/* End of File */

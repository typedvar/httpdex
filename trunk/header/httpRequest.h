/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/types.h>
#include <sys/stat.h>

#include "httpCommon.h"
#include "httpProcessor.h"
#include "httpMime.h"
#include "httpEnv.h"
#include "httpVhost.h"

/* Index into the method_names Array */
#define IDX_OPTIONS          0
#define IDX_HEAD             1
#define IDX_POST             2
#define IDX_PUT              3
#define IDX_DELETE           4
#define IDX_TRACE            5
#define IDX_GET              6
#define IDX_CONNECT          7


extern const unsigned char *method_names[MAX_METHODS];

typedef struct
{
   unsigned char *host;
   unsigned short port;

   unsigned char *queryStr;
   unsigned char *resource;
}http_uri_t;

/* HTTP Request structure */
typedef struct
{
   /* the allocator */
   hx_allocator_t *allocator;

   /* the socket */
   SOCKET         sock;

   /* client Address */
   unsigned char *client_address;

   /* time stamp */
   unsigned char  time[MAX_TIME_LEN];

   /* index into the method array */
   int            method_idx;

   /* the uri string as obtained
      from the raw request */
   unsigned char *uri;
   int            uriLen;

   /* contains the parsed uri
      components */
   http_uri_t       *uri_object;

   /* resource type: static
      or dynamic */
   hx_category_t  resource_type;

   /* filename */
   unsigned char *filename;

   /* extension */
   unsigned char *extension;

   /* contains the details of
      the requested resource */
   hx_bool_t      is_dir;
   struct stat    resource_stat;

   /* Mime type details of the
      requested resource */
   http_mime_t      *mime_object;

   /* The http version of the
      request */
   unsigned char *version;
   int            version_len;

   /* Null terminated, contains the
      request line */
   unsigned char *request_line;

   /* Headers in the request */
   http_header_t    *header[MAX_HEADERS];
   int            num_headers;

   /* The request body if any */
   unsigned char *body;
   long           body_len;

   /* The environment */
   hx_list_t     *env;

   /* The buffer to store request parts */
   unsigned char *buffer;
   long           buffer_len;

   unsigned char *request;
   long           requestLen;

   /* The virtual host object */
   hx_vhost_t    *vhost;

   /* Pointer to the log system to
      which all messages about this
      request will be logged */
   hx_logsys_t   *log;

} http_request_t;

/* function declarations */

void hx_print_request (http_request_t *request_object, int log_level);

void hx_print_uri(hx_logsys_t *logsys, http_uri_t *uri_object, int log_level);

http_request_t *hx_createReqObj(hx_allocator_t *allocator);

int hx_add_header(http_request_t *http_request,
                 const unsigned char *request_header);

int hx_parse_body (http_request_t *http_request);

int hx_parse_headers (http_request_t *http_request);

int hx_parse_request_line (http_request_t *http_request);

int hx_parse_request (http_request_t *http_request);

int hx_parse_uri(http_request_t *request_object);

const unsigned char *hx_get_header_value (int index, http_request_t *request_object);

#endif
/* End of File */

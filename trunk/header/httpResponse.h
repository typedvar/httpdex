/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPRESONSE_H
#define HTTPRESONSE_H

#include "httpProcessor.h"
#include "httpRequest.h"

extern const http_response_code_t Responses[MAX_RESPONSE_CODES];

/* HTTP Response Structure */
typedef struct
{
   /* whether parsed or non parsed */
   hx_output_t    type;

   hx_allocator_t *allocator;
   unsigned char time[MAX_TIME_LEN];
   const http_response_code_t *code;

   /* Null terminated, contains the
      status line to be sent to the
      client */
   unsigned char *statusLine;

   /* Response Header String */
   unsigned char *headers;

   /* Contain the body of the response */
   unsigned char *body;

   long           body_len;

   /* response headers */
   /*http_header_t *header[MAX_HEADERS];
   int num_headers;*/

   /* the main response buffer which
      will be written to the socket. This
      buffer will be generated using the
      formatResponse function */
   unsigned char *buffer;
   long           buffer_len;

   /* pointer to the request object
      for which this response is formed */
   http_request_t   *request_object;
} http_response_t;

/* function declarations */

http_response_t *hx_createResObj(http_request_t *request_object);

void hx_form_error_response (http_response_t *response_object,
                         http_request_t *request_object,
                         int errIndex,
                         const unsigned char *caller);

int hx_formDirListing (http_response_t *response_object, http_request_t *request_object);

void hx_add_response_header (hx_logsys_t *log,
                       unsigned char *buffer,
                       int idx,
                       const unsigned char *str);

void hx_printResponse (http_response_t *httpRes, int loglvl);

int hx_sendErrResponse (SOCKET client_socket, http_response_t *response_object);

int hx_sendResponse (SOCKET client_socket, http_response_t *response_object);

char* hx_create_status_line(hx_logsys_t *log,
                          int status_index,
                          const unsigned char *resource,
                          hx_allocator_t *allocator);
#endif
/* End of File */

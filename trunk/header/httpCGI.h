/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifndef HTTPCGI_H
#define HTTPCGI_H

#include "httpRequest.h"
#include "httpResponse.h"

#define IDX_MV_AUTH_TYPE               0
#define IDX_MV_CONTENT_LENGTH          1
#define IDX_MV_CONTENT_TYPE            2
#define IDX_MV_GATEWAY_INTERFACE       3
#define IDX_MV_PATH_INFO               4
#define IDX_MV_PATH_TRANSLATED         5
#define IDX_MV_QUERY_STRING            6
#define IDX_MV_REMOTE_ADDR             7
#define IDX_MV_REMOTE_HOST             8
#define IDX_MV_REMOTE_IDENT            9
#define IDX_MV_REMOTE_USER             10
#define IDX_MV_REQUEST_METHOD          11
#define IDX_MV_SCRIPT_NAME             12
#define IDX_MV_SERVER_NAME             13
#define IDX_MV_SERVER_PORT             14
#define IDX_MV_SERVER_PROTOCOL         15
#define IDX_MV_SERVER_SOFTWARE         16

typedef struct
{
   unsigned char *data;
   long dataLen;
} hx_bodydata_t;

int hx_handle_cgi (http_request_t *request_object, http_response_t *response_object);

#endif
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifndef HTTPENGINE_H
#define HTTPENGINE_H

#include "httpRequest.h"
#include "httpResponse.h"

typedef int (*fhandler_t) (http_request_t *request_object, http_response_t *response_object);

/* array of handler functions */
extern const fhandler_t g_engine[MAX_METHODS];

/* function declarations */
int hx_OPTIONS (http_request_t *request_object, http_response_t *response_object);

int hx_HEAD (http_request_t *request_object, http_response_t *response_object);

int hx_POST (http_request_t *request_object, http_response_t *response_object);

int hx_PUT (http_request_t *request_object, http_response_t *response_object);

int hx_DELETE (http_request_t *request_object, http_response_t *response_object);

int hx_TRACE (http_request_t *request_object, http_response_t *response_object);

int hx_GET (http_request_t *request_object, http_response_t *response_object);

int hx_CONNECT (http_request_t *request_object, http_response_t *response_object);

#endif
/* End of File */

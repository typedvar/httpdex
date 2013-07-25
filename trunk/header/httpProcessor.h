/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPPROCESSOR_H
#define HTTPPROCESSOR_H

#include "httpCommon.h"
#include "httpMemsys.h"

/* Index into HTTP Response Codes Array */
#define IDX_100                   0       /* Continue                         */
#define IDX_101                   1       /* Switching Protocols              */
#define IDX_200                   2       /* OK                               */
#define IDX_201                   3       /* Created                          */
#define IDX_202                   4       /* Accepted                         */
#define IDX_203                   5       /* Non-Authoritative Information    */
#define IDX_204                   6       /* No Content                       */
#define IDX_205                   7       /* Reset Content                    */
#define IDX_206                   8       /* Partial Content                  */
#define IDX_300                   9       /* Multiple Choices                 */
#define IDX_301                  10       /* Moved Permanently                */
#define IDX_302                  11       /* Found                            */
#define IDX_303                  12       /* See Other                        */
#define IDX_304                  13       /* Not Modified                     */
#define IDX_305                  14       /* Use Proxy                        */
#define IDX_307                  15       /* Temporary Redirect               */
#define IDX_400                  16       /* Bad Request                      */
#define IDX_401                  17       /* Unauthorized                     */
#define IDX_402                  18       /* Payment Required                 */
#define IDX_403                  19       /* Forbidden                        */
#define IDX_404                  20       /* Not Found                        */
#define IDX_405                  21       /* Method Not Allowed               */
#define IDX_406                  22       /* Not Acceptable                   */
#define IDX_407                  23       /* Proxy Authentication Required    */
#define IDX_408                  24       /* Request Time-out                 */
#define IDX_409                  25       /* Conflict                         */
#define IDX_410                  26       /* Gone                             */
#define IDX_411                  27       /* Length Required                  */
#define IDX_412                  28       /* Precondition Failed              */
#define IDX_413                  29       /* Request Entity Too Large         */
#define IDX_414                  30       /* Request-URI Too Large            */
#define IDX_415                  31       /* Unsupported Media Type           */
#define IDX_416                  32       /* Requested range not satisfiable  */
#define IDX_417                  33       /* Expectation Failed               */
#define IDX_500                  34       /* Internal Server Error            */
#define IDX_501                  35       /* Not Implemented                  */
#define IDX_502                  36       /* Bad Gateway                      */
#define IDX_503                  37       /* Service Unavailable              */
#define IDX_504                  38       /* Gateway Time-out                 */
#define IDX_505                  39       /* HTTP Version not supported       */

/* Index into the Headers Array */
#define IDX_CACHE_CONTROL        0
#define IDX_CONNECTION           1
#define IDX_DATE                 2
#define IDX_PRAGMA               3
#define IDX_TRAILER              4
#define IDX_TRANSFER_ENCODING    5
#define IDX_UPGRADE              6
#define IDX_VIA                  7
#define IDX_WARNING              8
#define IDX_ACCEPT               9
#define IDX_ACCEPT_CHARSET       10
#define IDX_ACCEPT_ENCODING      11
#define IDX_ACCEPT_LANGUAGE      12
#define IDX_AUTHORIZATION        13
#define IDX_EXPECT               14
#define IDX_FROM                 15
#define IDX_HOST                 16
#define IDX_IF_MATCH             17
#define IDX_IF_MODIFIED_SINCE    18
#define IDX_IF_NONE_MATCH        19
#define IDX_IF_RANGE             20
#define IDX_IF_UNMODIFIED_SINCE  21
#define IDX_MAX_FORWARDS         22
#define IDX_PROXY_AUTHORIZATION  23
#define IDX_RANGE                24
#define IDX_REFERER              25
#define IDX_TE                   26
#define IDX_USER_AGENT           27
#define IDX_ACCEPT_RANGES        28
#define IDX_AGE                  29
#define IDX_ETAG                 30
#define IDX_LOCATION             31
#define IDX_PROXY_AUTHENTICATE   32
#define IDX_RETRY_AFTER          33
#define IDX_SERVER               34
#define IDX_VARY                 35
#define IDX_WWW_AUTHENTICATE     36
#define IDX_ALLOW                37
#define IDX_CONTENT_ENCODING     38
#define IDX_CONTENT_LANGUAGE     39
#define IDX_CONTENT_LENGTH       40
#define IDX_CONTENT_LOCATION     41
#define IDX_CONTENT_MD5          42
#define IDX_CONTENT_RANGE        43
#define IDX_CONTENT_TYPE         44
#define IDX_EXPIRES              45
#define IDX_LAST_MODIFIED        46
#define IDX_CLIENT_DATE          47

extern const unsigned char *header_names_table[MAX_HEADERS];

typedef struct
{
   int header_idx;
   unsigned char *value;
} http_header_t;

typedef struct
{
   int  code;
   char description[MAX_RESPONSE_DESC_LEN];
} http_response_code_t;

/* function declarations */
/* processes a connected client */
int hx_process_http(SOCKET client_socket,
                     const unsigned char *client_address,
                     hx_allocator_t *allocator);

int hx_process_admin(SOCKET client_socket,
                    const unsigned char *client_address,
                    hx_allocator_t *allocator);

#endif
/* End of File */

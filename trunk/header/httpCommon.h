/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifndef HTTPCOMMON_H
#define HTTPCOMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#ifdef _HX_WIN32_
#undef _HX_UNIX_

#include <winsock2.h>

#elif defined(_HX_UNIX_)
#undef _HX_WIN32_
#include <sys/socket.h>
typedef int SOCKET;
#endif

#define CR        '\x0D'
#define LF        '\x0A'
#define SP        '\x20'
#define HT        '\x09'

#define CRLF      "\x0D\x0A"

#define OK                       1
#define ERRC                    -1
#define NOMATCH                 -1

/* run limits */
#define MAX_REQS                 5
#define MAX_SMALL_BUFF_LEN       256
#define MAX_BUFF_LEN             8192
#define MAX_TIME_LEN             40
#define MAX_ALLOCNAME            32
#define MAX_DIRLINE_LEN          1024
#define MAX_BODYSTART_LEN        1024
#define MAX_BODYEND_LEN          1024

#ifdef _HX_WIN32_
#define MAX_MODESTR_LEN          4
#elif defined(_HX_UNIX_)
#define MAX_MODESTR_LEN          10
#endif

/* CGI Limits */
#define MAX_META_VARS            17

/* HTTPDeX System limits */
#define MAX_FILENAME             256
#define MAX_EXTENSION            64
#define MAX_CONF_LINE_LEN        256
#define MAX_LOG_PREFIX_LEN       64
#define MAX_LOG_BUFF_LEN         4096
#define MAX_FUNCNAME_LEN         22
#define MAX_INTERPRETER_NAME     1024
#define MAX_CMDLINE_LEN          2048
#define MAX_FILLER_LEN           64
#define MAX_LVLSTR_LEN           6
#define MAX_HOSTNAME_LEN         256

#define MAX_SOCKERR_CODES        62

#define SOCK_CLOSE_DELAY         0.001   /* in seconds */
#define TIMEOUT_RECV             60      /* in seconds */
#define TIMEOUT_SEND             60      /* in seconds */

/* Admin interface limits */
#define MAX_ADMIN_METHODS        3
#define MAX_MSG_ADMIN_LEN_SIZE   4
#define MAX_MSG_ADMIN_TYPE_SIZE  4
#define MAX_ADMIN_REQUEST_LEN    1024
#define MAX_ADMIN_RESPONSE_LEN   1024

/* HTTP Protocol imposed limits */
#define MAX_REQUEST_LINE_LEN     8192
#define MAX_STATUS_LINE_LEN      256
#define MAX_REQUEST_BODY         8192
#define MAX_HEADER_NAME          32

#define MAX_HEADERS              48
#define MAX_HEADER_NAME_LEN      256
#define MAX_HEADER_LEN           MAX_REQUEST_LINE_LEN + MAX_HEADER_NAME_LEN + 4
#define MAX_TOTAL_HEADER_LEN     2048

#define MAX_METHODS              8

#define MAX_RESPONSE_DESC_LEN    64
#define MAX_RESPONSE_CODES       40
#define MAX_ERROR_RESPONSE_LEN   512
#define MAX_ERROR_PAGEBODY_LEN   512
#define MAX_CONTENT_LENGTH_LEN   10
#define MAX_CONTENT_TYPE_LEN     64
#define MAX_PORT_STR_LEN         7
#define MAX_MIME_TYPES           256
#define MAX_HREF_LEN             256

#define MAX_NOT_IMPLEMENTED      4

/* sock close enum */
typedef enum
{
   HX_SEND,
   HX_READ,
   HX_BOTH
}hx_sockclose_t;

/* log prefix */
typedef enum
{
   HX_LOGPREFIX_NO = 0,
   HX_LOGPREFIX_YES
}hx_logprefix_t;

/* shutdown states */
typedef enum
{
   HX_SHUTDN_NOTPENDING = 0,
   HX_SHUTDN_PENDING,
   HX_SHUTDN_INITIATED,
   HX_SHUTDN_COMPLETE
}hx_stopstatus_t;

/* buffer cond */
typedef enum
{
   HX_ENDOFDATA,
   HX_MOREDATA
}hx_datastatus_t;

/* script output type */
typedef enum
{
   HX_PARSED,
   HX_NONPARSED
}hx_output_t;

/* boolean type */
typedef enum
{
    HX_FALSE = 0,
    HX_TRUE
} hx_bool_t;

/* type of server close */
typedef enum
{
    NORMAL_CLOSE,
    ABORTIVE_CLOSE
} hx_clientclose_t;

/* vhost parameter indes */
typedef enum
{
   VHOSTPARM_HOSTNAME,
   VHOSTPARM_DOCROOT,
   VHOSTPARM_SCRIPTROOT,
   VHOSTPARM_LOGFILE,
   VHOSTPARM_ERRPAGE,
   MAX_VHOST_PARMS
} hx_vhostparmidx_t;

/* error codes */
typedef enum
{
   HX_ERR_CHILDREAD_TIMED_OUT=-10,
   HX_ERR_CHILDWRITE_TIMED_OUT=-11,
   HX_ERR_CHILDREAD=-12,
   HX_ERR_CHILDWRITE=-13,
   HX_ERR_IOCTL=-14,
   HX_ERR_SELECT=-15,
   HX_ERR_SYSTEM=-16,
   HX_ERR_MEM_ALLOC=32,
   HX_ERR_INVALID_REQUEST,
   HX_ERR_BAD_REQUEST,
   HX_ERR_REQUEST_URI_LARGE,
   HX_ERR_INVALID_HEADER,
   HX_ERR_INVALID_METHOD,
   HX_ERR_INVALID_URI,
   HX_ERR_INDEX_FILE_LIST,
   HX_ERR_INTERNAL,
   HX_ERR_FILE_OPEN,
   HX_ERR_FILE_READ,
   HX_ERR_MIMETABLE_INITED,
   HX_ERR_MIMEFILE_NOT_FOUND,
   HX_ERR_INVALID_MIME_LINE,
   HX_ERR_INVALID_MIME_FILE,
   HX_ERR_FILE_FORBIDDEN,
   HX_ERR_FILE_NOEXIST,
   HX_ERR_INSUFF_BUFFLEN,
   HX_ERR_CREATEPIPE,
   HX_ERR_CREATEPROCESS,
   HX_ERR_STREAM_NUM,
   HX_ERR_PIPEFUNC,
   HX_ERR_PROCESS_CREATE,
   HX_ERR_INVALID_PARAM,
   HX_ERR_INVALID_THRD_HNDL,
   HX_ERR_INVALID_THRD_PTR,
   HX_ERR_WAIT_FAILED,
   HX_ERR_JOIN_FAILED_SRCH,
   HX_ERR_JOIN_FAILED_DEADLK,
   HX_ERR_CLOSE_HANDLE,
   HX_ERR_INTERPRETER,
   HX_ERR_HANDLEFUNC,
   HX_ERR_INVALID_ALLOCATOR,
   HX_ERR_INVALID_FREE,
   HX_ERR_READFILE,
   HX_ERR_CONFROOT_NOT_FOUND,
   HX_ERR_CONFIG_DOC_ROOT,
   HX_ERR_CONFIG_INITED,
   HX_ERR_CONFIG_CREATE,
   HX_ERR_CONFIG_LOAD,
   HX_ERR_CONFIG_FUNC,
   HX_ERR_CONFIG_KEY,
   HX_ERR_CONFIG_DATA,
   HX_ERR_CONFIG_NUM_SRV_THREADS,
   HX_ERR_CONFIG_NUM_ADM_THREADS,
   HX_ERR_CONFIG_SERVER_TIMEOUT,
   HX_ERR_CONFIG_ADMIN_PORT,
   HX_ERR_CONFIG_HTTP_PORT,
   HX_ERR_CONFIG_DEFERR_PAGE,
   HX_ERR_ADM_INVALID_LEN,
   HX_ERR_ADM_INVALID_MSG,
   HX_ERR_ADM_INVALID_CMD,
   HX_ERR_VHOST_READY,
   HX_ERR_VHOST_DOCROOT,
   HX_ERR_VHOST_NOTREADY,
   HX_ERR_VHOST_ADD,
   HX_ERR_VHOST_NOEXIST,
   HX_ERR_CREATE_LOG,
   HX_ERR_PID_FILECREATE,
   HX_ERR_GETPID,
   HX_ERR_CLIENT,
   HX_ERR_UNKNOWN,
   MAX_ERR_CODES
} hx_errcodes_t;

typedef enum
{
   HX_PARSE_OK=MAX_ERR_CODES+1,
   HX_PARSE_COMPLETE,
   MAX_RESP_CODES
} hx_respcodes_t;

typedef enum
{
   HX_DYNAMIC,
   HX_STATIC
} hx_category_t;

#endif
/* End of File */

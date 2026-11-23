# httpdEx - API Reference

## 1. Introduction

This document describes the public APIs and interfaces of the httpdEx server. The API is organized by subsystem, with each section covering the functions, types, and constants provided by that module.

## 2. Common Types and Constants

### 2.1 Basic Types

```c
/* Boolean type */
typedef enum {
    HX_FALSE = 0,
    HX_TRUE
} hx_bool_t;

/* Return codes */
#define OK 1
#define ERRC -1
#define NOMATCH -1

/* Socket close modes */
typedef enum {
   HX_SEND,
   HX_READ,
   HX_BOTH
} hx_sockclose_t;

/* Log prefix mode */
typedef enum {
   HX_LOGPREFIX_NO = 0,
   HX_LOGPREFIX_YES
} hx_logprefix_t;

/* Shutdown states */
typedef enum {
   HX_SHUTDN_NOTPENDING = 0,
   HX_SHUTDN_PENDING,
   HX_SHUTDN_INITIATED,
   HX_SHUTDN_COMPLETE
} hx_stopstatus_t;

/* Buffer condition */
typedef enum {
   HX_ENDOFDATA,
   HX_MOREDATA
} hx_datastatus_t;

/* Script output type */
typedef enum {
   HX_PARSED,
   HX_NONPARSED
} hx_output_t;

/* Client close type */
typedef enum {
    NORMAL_CLOSE,
    ABORTIVE_CLOSE
} hx_clientclose_t;

/* Resource category */
typedef enum {
   HX_DYNAMIC,
   HX_STATIC
} hx_category_t;
```

### 2.2 Limits

| Constant | Value | Description |
|----------|-------|-------------|
| MAX_REQS | 5 | Max requests |
| MAX_SMALL_BUFF_LEN | 256 | Small buffer length |
| MAX_BUFF_LEN | 8192 | Buffer length |
| MAX_TIME_LEN | 40 | Time string length |
| MAX_ALLOCNAME | 32 | Allocator name length |
| MAX_DIRLINE_LEN | 1024 | Directory line length |
| MAX_FILENAME | 256 | Filename length |
| MAX_EXTENSION | 64 | Extension length |
| MAX_CONF_LINE_LEN | 256 | Config line length |
| MAX_LOG_PREFIX_LEN | 64 | Log prefix length |
| MAX_LOG_BUFF_LEN | 4096 | Log buffer length |
| MAX_FUNCNAME_LEN | 22 | Function name length |
| MAX_INTERPRETER_NAME | 1024 | Interpreter name length |
| MAX_CMDLINE_LEN | 2048 | Command line length |
| MAX_HOSTNAME_LEN | 256 | Hostname length |
| MAX_SOCKERR_CODES | 62 | Socket error codes |
| MAX_METHODS | 8 | HTTP methods |
| MAX_HEADERS | 48 | HTTP headers |
| MAX_HEADER_NAME | 32 | Header name length |
| MAX_HEADER_LEN | 8448 | Header total length |
| MAX_TOTAL_HEADER_LEN | 2048 | Total header length |
| MAX_REQUEST_LINE_LEN | 8192 | Request line length |
| MAX_STATUS_LINE_LEN | 256 | Status line length |
| MAX_REQUEST_BODY | 8192 | Request body length |
| MAX_RESPONSE_CODES | 40 | Response codes |
| MAX_RESPONSE_DESC_LEN | 64 | Response description length |
| MAX_ERROR_RESPONSE_LEN | 512 | Error response length |
| MAX_ERROR_PAGEBODY_LEN | 512 | Error page body length |
| MAX_CONTENT_LENGTH_LEN | 10 | Content length string length |
| MAX_CONTENT_TYPE_LEN | 64 | Content type length |
| MAX_PORT_STR_LEN | 7 | Port string length |
| MAX_MIME_TYPES | 256 | MIME types |
| MAX_HREF_LEN | 256 | HREF length |
| MAX_NOT_IMPLEMENTED | 4 | Not implemented list |
| MAX_ADMIN_METHODS | 3 | Admin methods |
| MAX_MSG_ADMIN_LEN_SIZE | 4 | Admin message length size |
| MAX_MSG_ADMIN_TYPE_SIZE | 4 | Admin message type size |
| MAX_ADMIN_REQUEST_LEN | 1024 | Admin request length |
| MAX_ADMIN_RESPONSE_LEN | 1024 | Admin response length |
| MAX_META_VARS | 17 | CGI meta variables |
| TIMEOUT_RECV | 60 | Receive timeout (sec) |
| TIMEOUT_SEND | 60 | Send timeout (sec) |
| SOCK_CLOSE_DELAY | 0.001 | Socket close delay (sec) |
| HTTP_Q_LEN | - | HTTP queue length |
| ADMIN_Q_LEN | - | Admin queue length |

### 2.3 Error Codes

```c
typedef enum {
   HX_ERR_CHILDREAD_TIMED_OUT = -10,
   HX_ERR_CHILDWRITE_TIMED_OUT = -11,
   HX_ERR_CHILDREAD = -12,
   HX_ERR_CHILDWRITE = -13,
   HX_ERR_IOCTL = -14,
   HX_ERR_SELECT = -15,
   HX_ERR_SYSTEM = -16,
   HX_ERR_MEM_ALLOC = 32,
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
```

### 2.4 Response Codes

```c
typedef enum {
   HX_PARSE_OK = MAX_ERR_CODES + 1,
   HX_PARSE_COMPLETE,
   MAX_RESP_CODES
} hx_respcodes_t;
```

### 2.5 HTTP Response Codes

| Code | Description |
|------|-------------|
| 100 | Continue |
| 101 | Switching Protocols |
| 200 | OK |
| 201 | Created |
| 202 | Accepted |
| 203 | Non-Authoritative Information |
| 204 | No Content |
| 205 | Reset Content |
| 206 | Partial Content |
| 300 | Multiple Choices |
| 301 | Moved Permanently |
| 302 | Found |
| 303 | See Other |
| 304 | Not Modified |
| 305 | Use Proxy |
| 307 | Temporary Redirect |
| 400 | Bad Request |
| 401 | Unauthorized |
| 402 | Payment Required |
| 403 | Forbidden |
| 404 | Not Found |
| 405 | Method Not Allowed |
| 406 | Not Acceptable |
| 407 | Proxy Authentication Required |
| 408 | Request Time-out |
| 409 | Conflict |
| 410 | Gone |
| 411 | Length Required |
| 412 | Precondition Failed |
| 413 | Request Entity Too Large |
| 414 | Request-URI Too Large |
| 415 | Unsupported Media Type |
| 416 | Requested range not satisfiable |
| 417 | Expectation Failed |
| 500 | Internal Server Error |
| 501 | Not Implemented |
| 502 | Bad Gateway |
| 503 | Service Unavailable |
| 504 | Gateway Time-out |
| 505 | HTTP Version not supported |

## 3. Memory System API

### 3.1 Types

```c
typedef struct {
#ifdef HX_DEBUG_MEMSYS
   unsigned char  file[MAX_FILENAME];
   int            line;
   int            reuseCount;
#endif
   size_t         requested;
   size_t         allocated;
   hx_bool_t      isFree;
   hx_mem_t      *next;
   void          *block;
} hx_mem_t;

typedef struct {
#ifdef HX_DEBUG_MEMSYS
   unsigned char  file[MAX_FILENAME];
   int            line;
#endif
   unsigned char  name[MAX_ALLOCNAME];
   long           poolId;
   hx_mem_t      *head;
   hx_mem_t      *tail;
   long           numElts;
} hx_allocator_t;
```

### 3.2 Functions

| Function | Description |
|----------|-------------|
| `hx_create_allocator(name)` | Create a named memory allocator |
| `hx_destroy_allocator(allocator)` | Destroy an allocator and free all memory |
| `hx_alloc_mem(size, allocator)` | Allocate memory from an allocator |
| `hx_free_mem(ptr, allocator)` | Free memory to an allocator |

When `HX_DEBUG_MEMSYS` is defined, debug versions with file/line tracking are used:
- `hx_create_allocator_debug(name, file, line)`
- `hx_destroy_allocator_debug(allocator, file, line)`
- `hx_alloc_mem_debug(size, allocator, file, line)`
- `hx_free_mem_debug(ptr, allocator, file, line)`

## 4. Logging System API

### 4.1 Types

```c
typedef enum {
   HX_LOGLVL_01 through HX_LOGLVL_32
} hx_loglvl_t;

#define HX_LERRO  HX_LOGLVL_01  /* Errors (always on) */
#define HX_LWARN  HX_LOGLVL_02  /* Warnings (always on) */
#define HX_LVERB  HX_LOGLVL_03  /* Verbose */
#define HX_LDEBG  HX_LOGLVL_04  /* Debug */
#define HX_LDMIM  HX_LOGLVL_05  /* MIME */
#define HX_LCALL  HX_LOGLVL_06  /* Call trace */
#define HX_LREQS  HX_LOGLVL_07  /* Requests */
#define HX_LRESP  HX_LOGLVL_08  /* Responses */
#define HX_LDUMP  HX_LOGLVL_09  /* Hex dump */
#define HX_LSYST  HX_LOGLVL_10  /* System (always on) */

typedef struct {
   hx_allocator_t    *allocator;
   unsigned char     *name;
   unsigned char     *logfile;
   FILE              *fp;
   hx_lock_t         *lock;
   unsigned char     prefix[MAX_LOG_PREFIX_LEN];
   unsigned char     timebuf[MAX_TIME_LEN];
   unsigned char     funcbuf[MAX_FUNCNAME_LEN];
   unsigned char     level[MAX_LVLSTR_LEN + 1];
   unsigned char     *logbuf1;
   unsigned char     *logbuf2;
   long              mask;
   int               needPrefix;
} hx_logsys_t;
```

### 4.2 Functions

| Function | Description |
|----------|-------------|
| `hx_create_log_sys(log, filename, name, needPrefix)` | Create a log system |
| `hx_destroy_log_sys(logsys)` | Destroy a log system |
| `hx_check_log_level(logsys, log_level)` | Check if log level is enabled |
| `hx_reset_log_level(logsys)` | Reset all log levels |
| `hx_set_log_level(logsys, loglvl)` | Set a log level |
| `hx_set_log_level_all(logsys)` | Set all log levels |
| `hx_set_log_mask(logsys, mask)` | Set log mask |
| `hx_get_log_mask(logsys)` | Get log mask |
| `hx_log_msg(logsys, loglvl, func, format, ...)` | Log a message |
| `hx_hex_dump(logsys, buffer, len, log_level)` | Hex dump buffer to log |

### 4.3 Log Level Strings

| String | Level |
|--------|-------|
| erro | HX_LERRO |
| warn | HX_LWARN |
| verb | HX_LVERB |
| debg | HX_LDEBG |
| mime | HX_LDMIM |
| call | HX_LCALL |
| reqs | HX_LREQS |
| resp | HX_LRESP |
| dump | HX_LDUMP |
| syst | HX_LSYST |

## 5. Locking API

### 5.1 Types

```c
typedef struct {
#ifdef _HX_WINTHREADS_
   CRITICAL_SECTION *lk;
#elif defined(_PTHREADS_)
   pthread_mutex_t  *lk;
#endif
   hx_allocator_t   *a;
} hx_lock_t;
```

### 5.2 Functions

| Function | Description |
|----------|-------------|
| `hx_create_lock()` | Create a lock |
| `hx_destroy_lock(lock)` | Destroy a lock |
| `hx_lock(lock)` | Acquire lock |
| `hx_unlock(lock)` | Release lock |

## 6. List API

### 6.1 Types

```c
typedef struct node_t_ node_t;
struct node_t_ {
   node_t *prv;
   node_t *nxt;
   void   *elem;
};

typedef struct {
   hx_allocator_t *allocator;
   node_t         *head;
   node_t         *tail;
   int            entries;
   hx_lock_t      *lock;
} hx_list_t;
```

### 6.2 Functions

| Function | Description |
|----------|-------------|
| `hx_create_list()` | Create a list |
| `hx_destroy_list(list)` | Destroy a list |
| `hx_add(list, data)` | Append to end |
| `hx_add_first(list, data)` | Prepend to front |
| `hx_remove(list)` | Remove from tail |
| `hx_remove_first(list)` | Remove from head |
| `hx_get(list)` | Get tail element |
| `hx_get_first(list)` | Get head element |
| `hx_get_pos(list, position)` | Get element at position |
| `hx_clear(list)` | Clear all elements |
| `hx_print(log, list, printer, level)` | Print list contents |
| `hx_find(list, data, comparator)` | Find element using comparator |
| `hx_size(list)` | Get number of elements |

## 7. Threading API

### 7.1 Types

```c
#ifdef _HX_WINTHREADS_
typedef DWORD WINAPI (*fthread_t)(void*);
#elif defined(_PTHREADS_)
typedef void* (*fthread_t)(void *);
#endif

typedef struct {
#ifdef _HX_WINTHREADS_
   HANDLE *handle;
   DWORD  *id;
#elif defined(_PTHREADS_)
   pthread_t *ptr;
#endif
   void *param;
   hx_allocator_t *a;
   fthread_t func;
} hx_thread_t;
```

### 7.2 Functions

| Function | Description |
|----------|-------------|
| `hx_create_thread(a, func, param)` | Create a thread |
| `hx_exitThread(status, log)` | Exit a thread |
| `hx_get_thread_id(thread)` | Get thread ID |
| `hx_wait_for_threads(a, arr, count, log)` | Wait for all threads in array |
| `hx_threadSigMask()` | Mask all signals for thread |

## 8. Socket API

### 8.1 Types

```c
typedef struct sockErr {
    unsigned char *desc;
    int error_num;
} hx_sockerr_t;
```

### 8.2 Functions

| Function | Description |
|----------|-------------|
| `hx_sock_init()` | Initialize socket system |
| `hx_sockCleanup()` | Cleanup socket system |
| `hx_sockClose(s, how)` | Close socket |
| `hx_serverSock(log, port, queuelen)` | Create server socket |
| `hx_clientSock(log, host, port)` | Create client socket |
| `hx_sockErr(log, errstr, error)` | Log socket error |
| `hx_get_socket_error()` | Get last socket error |
| `hx_sock_read(s, buff, len)` | Read from socket |
| `hx_sockWrite(s, response, len)` | Write to socket |

## 9. Configuration API

### 9.1 Types

```c
typedef struct {
   unsigned char *name;
   unsigned char *value;
} hx_config_record_t;

typedef struct {
   unsigned char *cmd;
   fconf_t func;
} hx_cmd_t;

typedef struct {
   hx_allocator_t *a;
   unsigned char *filename;
   FILE *fptr;
   void **table;
} hx_config_t;
```

### 9.2 Configuration Record Indices

| Index | Name | Description |
|-------|------|-------------|
| IDX_NUM_SRV_THRDS | Number of server threads |
| IDX_NUM_ADM_THRDS | Number of admin threads |
| IDX_DOC_ROOT | Document root |
| IDX_CGI_ROOT | CGI root |
| IDX_SERVER_TIMEOUT | Server timeout |
| IDX_DEFAULT_HTTP_PORT | Default HTTP port |
| IDX_DEFAULT_HOST_NAME | Default hostname |
| IDX_DEFAULT_ERR_PAGE | Default error page |
| IDX_CONF_ROOT | Config root |
| IDX_MIME_FILE_NAME | MIME file name |
| IDX_HTTP_VERSION | HTTP version |
| IDX_PATH_SEPARATOR | Path separator |
| IDX_INDEX_FILES | Index files |
| IDX_ADMIN_PORT | Admin port |
| IDX_HTTP_SERVICE_PORT | HTTP service port |
| IDX_LOG_LEVEL | Log level |
| IDX_VHOST | Virtual host |

### 9.3 Functions

| Function | Description |
|----------|-------------|
| `hx_load_config(filename, logsys)` | Load configuration from file |
| `hx_reload_config(logsys)` | Reload configuration |
| `hx_destroy_config(logsys)` | Destroy configuration |
| `hx_get_conf_val(log, index)` | Get config value by index |

### 9.4 Config File Format

```
# Comments start with #
DIRECTIVE=value

# VHOST format:
VHOST=hostname,docroot,scriptroot,logfile,[loglevels]
```

### 9.5 Config Directives

| Directive | Type | Description |
|-----------|------|-------------|
| SERVER_THREADS | int | Number of HTTP worker threads |
| ADMIN_THREADS | int | Number of admin worker threads |
| DOCUMENT_ROOT | string | Default document root path |
| CGI_ROOT | string | Default CGI root path |
| SERVER_TIMEOUT | int | Server timeout in seconds |
| DEFAULT_HTTP_PORT | int | Default HTTP port |
| DEFAULT_HOST_NAME | string | Default hostname |
| DEFAULT_ERR_PAGE | string | Default error page |
| CONFIG_ROOT | string | Configuration root directory |
| MIME_FILE_NAME | string | MIME data filename |
| HTTP_VERSION | string | HTTP version string |
| PATH_SEPARATOR | string | Path separator character |
| INDEX_FILES | string | Comma-separated index files |
| ADMIN_PORT | int | Administration port |
| HTTP_SERVICE_PORT | int | HTTP service port |
| LOG_LEVEL | string | Comma-separated log levels |
| VHOST | string | Virtual host definition |

## 10. Virtual Host API

### 10.1 Types

```c
typedef struct {
   unsigned char *host;
   unsigned char *docroot;
   unsigned char *scriptroot;
   unsigned char *errpage;
   hx_logsys_t   *log;
} hx_vhost_t;
```

### 10.2 Functions

| Function | Description |
|----------|-------------|
| `hx_getVhost(log, hostname)` | Look up vhost by hostname |
| `hx_addVhost(log, hostname, docroot, scriptroot, logfile, errpage, loglevels)` | Add a vhost |
| `hx_add_default_vhost(log)` | Create default vhost from config |
| `hx_create_vhost_sys(log)` | Create vhost system |
| `hx_destroyVhostSys(log)` | Destroy vhost system |

### 10.3 Vhost Parameter Indices

| Index | Name |
|-------|------|
| VHOSTPARM_HOSTNAME | Hostname |
| VHOSTPARM_DOCROOT | Document root |
| VHOSTPARM_SCRIPTROOT | Script root |
| VHOSTPARM_LOGFILE | Log file |
| VHOSTPARM_ERRPAGE | Error page |
| MAX_VHOST_PARMS | Maximum parameters |

## 11. Request API

### 11.1 Types

```c
typedef struct {
   unsigned char *host;
   unsigned short port;
   unsigned char *queryStr;
   unsigned char *resource;
} http_uri_t;

typedef struct {
   hx_allocator_t *allocator;
   SOCKET sock;
   unsigned char *client_address;
   unsigned char time[MAX_TIME_LEN];
   int method_idx;
   unsigned char *uri;
   int uriLen;
   http_uri_t *uri_object;
   hx_category_t resource_type;
   unsigned char *filename;
   unsigned char *extension;
   hx_bool_t is_dir;
   struct stat resource_stat;
   http_mime_t *mime_object;
   unsigned char *version;
   int version_len;
   unsigned char *request_line;
   http_header_t *header[MAX_HEADERS];
   int num_headers;
   unsigned char *body;
   long body_len;
   hx_list_t *env;
   unsigned char *buffer;
   long buffer_len;
   unsigned char *request;
   long requestLen;
   hx_vhost_t *vhost;
   hx_logsys_t *log;
} http_request_t;

typedef struct {
   int header_idx;
   unsigned char *value;
} http_header_t;
```

### 11.2 Method Indices

| Index | Name | Macro |
|-------|------|-------|
| 0 | OPTIONS | IDX_OPTIONS |
| 1 | HEAD | IDX_HEAD |
| 2 | POST | IDX_POST |
| 3 | PUT | IDX_PUT |
| 4 | DELETE | IDX_DELETE |
| 5 | TRACE | IDX_TRACE |
| 6 | GET | IDX_GET |
| 7 | CONNECT | IDX_CONNECT |

### 11.3 Functions

| Function | Description |
|----------|-------------|
| `hx_createReqObj(allocator)` | Create request object |
| `hx_parse_request(request)` | Parse HTTP request |
| `hx_parse_request_line(request)` | Parse request line |
| `hx_parse_headers(request)` | Parse headers |
| `hx_parse_body(request)` | Parse request body |
| `hx_parse_uri(request)` | Parse URI |
| `hx_add_header(request, header)` | Add header to request |
| `hx_get_header_value(index, request)` | Get header value |
| `hx_print_request(request, level)` | Print request to log |
| `hx_print_uri(logsys, uri_object, level)` | Print URI to log |

## 12. Response API

### 12.1 Types

```c
typedef struct {
   int code;
   char description[MAX_RESPONSE_DESC_LEN];
} http_response_code_t;

typedef struct {
   hx_output_t type;
   hx_allocator_t *allocator;
   unsigned char time[MAX_TIME_LEN];
   const http_response_code_t *code;
   unsigned char *statusLine;
   unsigned char *headers;
   unsigned char *body;
   long body_len;
   unsigned char *buffer;
   long buffer_len;
   http_request_t *request_object;
} http_response_t;
```

### 12.2 Functions

| Function | Description |
|----------|-------------|
| `hx_createResObj(request_object)` | Create response object |
| `hx_form_error_response(resp, req, errIndex, caller)` | Form error response |
| `hx_formDirListing(resp, req)` | Form directory listing |
| `hx_add_response_header(log, buffer, idx, str)` | Add response header |
| `hx_printResponse(httpRes, loglvl)` | Print response to log |
| `hx_sendErrResponse(sock, resp)` | Send error response |
| `hx_sendResponse(sock, resp)` | Send response |
| `hx_create_status_line(log, status_index, resource, allocator)` | Create status line |

### 12.3 Response Code Indices

| Index | Code | Description |
|-------|------|-------------|
| IDX_100 | 100 | Continue |
| IDX_200 | 200 | OK |
| IDX_400 | 400 | Bad Request |
| IDX_404 | 404 | Not Found |
| IDX_500 | 500 | Internal Server Error |
| IDX_501 | 501 | Not Implemented |
| ... | ... | ... |

## 13. Processor API

### 13.1 Types

```c
typedef int (*fhandler_t)(http_request_t *request_object, http_response_t *response_object);
```

### 13.2 Functions

| Function | Description |
|----------|-------------|
| `hx_process_http(client_socket, client_address, allocator)` | Process HTTP connection |
| `hx_process_admin(client_socket, client_address, allocator)` | Process admin connection |
| `hx_processRequest(request, response)` | Process request (route to handler) |

## 14. Engine API

### 14.1 Types

```c
typedef int (*fhandler_t)(http_request_t *request_object, http_response_t *response_object);
extern const fhandler_t g_engine[MAX_METHODS];
```

### 14.2 Handler Functions

| Function | Method | Status |
|----------|--------|--------|
| `hx_OPTIONS()` | OPTIONS | Not Implemented (501) |
| `hx_HEAD()` | HEAD | Implemented |
| `hx_POST()` | POST | Not Implemented (501) |
| `hx_PUT()` | PUT | Not Implemented (501) |
| `hx_DELETE()` | DELETE | Not Implemented (501) |
| `hx_TRACE()` | TRACE | Not Implemented (501) |
| `hx_GET()` | GET | Implemented |
| `hx_CONNECT()` | CONNECT | Not Implemented (501) |

## 15. CGI API

### 15.1 Types

```c
typedef enum {
   HX_STDIN = 0,
   HX_STDOUT = 1,
   HX_STDERR = 2
} hx_stdstr_t;

typedef enum {
   HX_SCRIPT,
   HX_BIN,
   HX_UNKNOWN
} hx_script_t;

typedef struct {
#ifdef _HX_WIN32_
   STARTUPINFO *sinfo;
   PROCESS_INFORMATION *pinfo;
   HANDLE childHandles[3];
   unsigned char *env_block;
#elif defined(_HX_UNIX_)
   int childStreams[3];
   int parentStreams[3];
   unsigned char **env_block;
#endif
   int pid;
   unsigned char *cmdLine;
   unsigned char *arg;
   unsigned char *interpreter;
   unsigned char *path;
   unsigned char *dir;
   hx_allocator_t *allocator;
   hx_script_t type;
} hx_process_t;
```

### 15.2 CGI Meta Variables

| Index | Name | Macro |
|-------|------|-------|
| 0 | AUTH_TYPE | IDX_MV_AUTH_TYPE |
| 1 | CONTENT_LENGTH | IDX_MV_CONTENT_LENGTH |
| 2 | CONTENT_TYPE | IDX_MV_CONTENT_TYPE |
| 3 | GATEWAY_INTERFACE | IDX_MV_GATEWAY_INTERFACE |
| 4 | PATH_INFO | IDX_MV_PATH_INFO |
| 5 | PATH_TRANSLATED | IDX_MV_PATH_TRANSLATED |
| 6 | QUERY_STRING | IDX_MV_QUERY_STRING |
| 7 | REMOTE_ADDR | IDX_MV_REMOTE_ADDR |
| 8 | REMOTE_HOST | IDX_MV_REMOTE_HOST |
| 9 | REMOTE_IDENT | IDX_MV_REMOTE_IDENT |
| 10 | REMOTE_USER | IDX_MV_REMOTE_USER |
| 11 | REQUEST_METHOD | IDX_MV_REQUEST_METHOD |
| 12 | SCRIPT_NAME | IDX_MV_SCRIPT_NAME |
| 13 | SERVER_NAME | IDX_MV_SERVER_NAME |
| 14 | SERVER_PORT | IDX_MV_SERVER_PORT |
| 15 | SERVER_PROTOCOL | IDX_MV_SERVER_PROTOCOL |
| 16 | SERVER_SOFTWARE | IDX_MV_SERVER_SOFTWARE |

### 15.3 Functions

| Function | Description |
|----------|-------------|
| `hx_handle_cgi(request, response)` | Handle CGI request |
| `hx_invokeCGI(request, childProc)` | Invoke CGI process |
| `hx_cleanProc(log, child)` | Clean up process |
| `hx_kill_child(log, child)` | Kill child process |

## 16. Executor API

### 16.1 Functions

| Function | Description |
|----------|-------------|
| `hx_invokeCGI(request, childProc)` | Invoke CGI process |
| `hx_cleanProc(log, child)` | Clean up process resources |
| `hx_kill_child(log, child)` | Kill child process |

## 17. Environment API

### 17.1 Types

```c
typedef struct {
   unsigned char *nameval;
} hx_envpair_t;
```

### 17.2 Functions

| Function | Description |
|----------|-------------|
| `hx_env_from_body(log, environ, body, allocator)` | Parse environment from request body |
| `hx_create_env_pair(log, name, value, allocator)` | Create environment pair |
| `hx_create_env_element(log, elem, allocator)` | Create environment element |
| `hx_create_win32_env_block(environ, env_block, allocator)` | Create Win32 env block |
| `hx_create_unix_env_block(environ, env_block, allocator)` | Create UNIX env block |

## 18. MIME API

### 18.1 Types

```c
typedef struct {
   unsigned char *ext;
   unsigned char *type;
   unsigned char *subtype;
} http_mime_t;

typedef struct {
   http_mime_t *table[MAX_MIME_TYPES];
   int numTypes;
} http_mime_table_t;
```

### 18.2 Functions

| Function | Description |
|----------|-------------|
| `hx_init_mime_table(log, allocator)` | Initialize MIME table from file |
| `hx_lookup_ext_mime(log, ext)` | Look up MIME by extension |
| `hx_lookup_resource_mime(log, resource)` | Look up MIME by resource path |
| `hx_get_mime_type(mime)` | Get MIME type |
| `hx_get_mime_sub_type(mime)` | Get MIME subtype |
| `hx_print_mime_type(logsys, mime, level)` | Print MIME type to log |
| `hx_print_mime_table(logsys, level)` | Print MIME table to log |

## 19. Admin API

### 19.1 Types

```c
typedef enum {
   ADM_OK,
   ADM_ERR,
   ADM_WARN
} hx_adminresp_t;

typedef struct {
   SOCKET sock;
   hx_allocator_t *allocator;
   int msg_len;
   unsigned char *client_address;
   unsigned char time[MAX_TIME_LEN];
   int cmdidx;
   int argc;
   unsigned char **argv;
   hx_adminresp_t resp;
   unsigned char *body;
   long bodylen;
   unsigned char *request;
   long requestLen;
} http_admin_t;

typedef int (*fadmin_t)(http_admin_t *a);
```

### 19.2 Admin Commands

| Command | Function | Description |
|---------|----------|-------------|
| reload_config | hx_reload_cfg() | Reload configuration |
| restart_log | hx_restart_log() | Restart logging |
| shutdown | hx_shutdown() | Shutdown server |

### 19.3 Functions

| Function | Description |
|----------|-------------|
| `hx_create_admin_object(allocator)` | Create admin object |
| `hx_parse_admin(admin)` | Parse admin message |
| `hx_execute_admin(admin)` | Execute admin command |
| `hx_reload_cfg(admin)` | Reload config handler |
| `hx_restart_log(admin)` | Restart log handler |
| `hx_shutdown(admin)` | Shutdown handler |

## 20. Utility API

### 20.1 String Functions

| Function | Description |
|----------|-------------|
| `hx_strndup(source, length, allocator)` | Duplicate string with length |
| `hx_strdup(source, allocator)` | Duplicate string |
| `hx_trim(source, allocator)` | Trim whitespace |
| `hx_strcmpi(arg1, arg2)` | Case-insensitive compare |
| `hx_strncmpi(arg1, arg2, len)` | Case-insensitive compare n chars |

### 20.2 Time Functions

| Function | Description |
|----------|-------------|
| `hx_getTime(timestamp, len)` | Get current time string |

### 20.3 Matching Functions

| Function | Description |
|----------|-------------|
| `hx_matchIdx(dict, dictLen, word, wordLen)` | Match word in dictionary |
| `hx_match_string_idx(dict, dictLen, word)` | Match string in dictionary |

### 20.4 File Functions

| Function | Description |
|----------|-------------|
| `hx_getResDetails(request, lock)` | Get resource details |
| `hx_getFileStat(log, resource, rstat, lock)` | Get file statistics |
| `hx_read_file(log, filename, buffer, datalen, allocator, lock)` | Read file |
| `hx_checkres(log, resource, allocator)` | Check resource |
| `hx_getIndexFile(request, idxList, separator, lock)` | Get index file |

### 20.5 Other Functions

| Function | Description |
|----------|-------------|
| `hx_hex_dump(log, buffer, len, level)` | Hex dump to log |
| `hx_createPage(buffer, title, body, bufflen, lock)` | Create HTML page |
| `hx_escape(log, dest, source, toEsc)` | Escape string |
| `hx_convert(log, string, from, to)` | Convert character encoding |
| `hx_getSysErrCode(errcode)` | Get system error code |
| `hx_get_response_code(errcode)` | Get HTTP response code |
| `hx_isLocalhost(vhostname)` | Check if hostname is localhost |
| `hx_checkOuput(buffer, buffLen)` | Check output type |
| `hx_getErrorString(errno)` | Get error string |
| `hx_sleep(secs)` | Sleep for seconds |
| `hx_readNonBlock(fd, timeout, buf, buflen)` | Non-blocking read |
| `hx_writeNonBlock(fd, timeout, buf, buflen)` | Non-blocking write |

### 20.6 Platform-Specific Functions

| Function | Platform | Description |
|----------|----------|-------------|
| `hx_isDrive(path)` | Win32 | Check if path is a drive |
| `hx_WIN32Err(log)` | Win32 | Log Win32 error |
| `hx_printWIN32EnvBlock(log, env_block)` | Win32 | Print Win32 env block |
| `hx_printUNIXEnvBlock(log, env_block)` | UNIX | Print UNIX env block |

## 21. HTML API

### 21.1 Functions

| Function | Description |
|----------|-------------|
| `hx_paginate(log, title, body, allocator)` | Create HTML page with template |

## 22. Dirlister API

### 22.1 Types

```c
typedef struct {
   unsigned char *parent;
   unsigned char *entroot;
   unsigned char *href;
   unsigned char *icon;
   unsigned char *ent;
   size_t size;
   time_t lmt;
   unsigned char *mode;
   int isdir;
} direlem_t;
```

### 22.2 Functions

| Function | Description |
|----------|-------------|
| `hx_get_dir_list(request, list, lock)` | Get directory listing |
| `hx_dir_lister(data, line, allocator)` | List directory entries |

## 23. Main API

### 23.1 Global Variables

```c
extern int server_shutdown;
extern int http_done;
extern int admin_done;
extern const unsigned char *VERSION;      // "V1.0"
extern const unsigned char *SERVER;       // "httpdEx"
extern hx_logsys_t *server_log_sys;
extern hx_vhost_t *defaultVhost;
extern SOCKET serverSock;
extern SOCKET adminSock;
extern hx_allocator_t *server_allocator;
extern hx_lock_t *lock_num_clients;
extern hx_lock_t *lock_accept_http;
extern hx_lock_t *lock_accept_admin;
extern hx_lock_t *lock_errno;
extern hx_lock_t *lock_shutdown;
extern int g_num_clients_being_served;
```

### 23.2 Functions

| Function | Description |
|----------|-------------|
| `hx_create_pid_file(log)` | Create PID file |
| `hx_signal_shutdown(state)` | Set shutdown state |
| `hx_switch_off(type, allocator)` | Shutdown server |
| `hx_sigusr1(sig)` | SIGUSR1 handler |
| `hx_sigint(sig)` | SIGINT handler |
| `hx_sigsegv(sig)` | SIGSEGV handler |
| `hx_install_signal_handlers()` | Install signal handlers |
| `hx_cmd_usage()` | Print usage |
| `hx_init_admin_iface(allocator)` | Initialize admin interface |
| `hx_init_http_iface(allocator)` | Initialize HTTP interface |
| `main(argc, argv)` | Server entry point |

## 24. Connection API

### 24.1 Functions

| Function | Description |
|----------|-------------|
| `hx_accept_http_conn(param)` | Accept HTTP connection (thread) |
| `hx_accept_admin_conn(param)` | Accept admin connection (thread) |

## 25. Spawner API

### 25.1 Functions

| Function | Description |
|----------|-------------|
| `hx_spawn_admin(param)` | Spawn admin thread |
| `hx_spawn_workers(param)` | Spawn HTTP worker threads |

## 26. Admin Client API

### 26.1 Functions

| Function | Description |
|----------|-------------|
| `hx_start_client(log, host, port)` | Start admin client (telnet-style) |

## 27. Error Handling

### 27.1 Error Code Lookup

```c
const unsigned char *hx_getErrorString(int errno);
```

Returns a human-readable string for the given error code.

### 27.2 Response Code Lookup

```c
int hx_get_response_code(int errcode);
```

Maps internal error codes to HTTP response codes.

## 28. Include Header

### 28.1 httpInclude.h

This header provides cross-module access to global variables and includes the core headers:

```c
#include "httpLog.h"
#include "httpMemsys.h"
#include "httpUtils.h"
```

External declarations:
- `server_shutdown`, `http_done`, `admin_done`
- `VERSION`, `SERVER`
- `errno`
- `lock_num_clients`, `lock_accept_http`, `lock_accept_admin`, `lock_errno`, `lock_shutdown`
- `g_num_clients_being_served`
- `serverSock`, `adminSock`
- `server_log_sys`
- `defaultVhost`
- `server_allocator`

---
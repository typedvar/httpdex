# httpdEx - Reference Documentation

## 1. Source File Reference

### 1.1 Source Files

| File | Module | Description |
|------|--------|-------------|
| `source/httpMain.c` | Main | Server entry point, initialization, shutdown |
| `source/httpConf.c` | Configuration | Config loading, reloading, validation |
| `source/httpMemsys.c` | Memory | Custom memory pool allocator |
| `source/httpLog.c` | Logging | Multi-level structured logging |
| `source/httpSock.c` | Networking | Socket abstraction |
| `source/httpThreads.c` | Threading | Cross-platform thread management |
| `source/httpLock.c` | Locking | Mutual exclusion primitives |
| `source/httpList.c` | Lists | Doubly-linked list data structure |
| `source/httpUtils.c` | Utilities | String, time, file utilities |
| `source/httpConn.c` | Connection | Connection acceptance, client processing |
| `source/httpProcessor.c` | Processor | HTTP request routing |
| `source/httpRequest.c` | Request | HTTP request parsing |
| `source/httpResponse.c` | Response | HTTP response generation |
| `source/httpEngine.c` | Engine | HTTP method dispatch |
| `source/httpHTML.c` | HTML | HTML page generation |
| `source/httpDirlister.c` | Dirlister | Directory listing |
| `source/httpVhost.c` | Vhost | Virtual host management |
| `source/httpCGI.c` | CGI | CGI script execution |
| `source/httpExecutor.c` | Executor | Process spawning |
| `source/httpEnv.c` | Environment | Environment block creation |
| `source/httpMime.c` | MIME | MIME type lookup |
| `source/httpAdmin.c` | Admin | Admin protocol and commands |
| `source/httpController.c` | Controller | Admin client |
| `source/httpClient.c` | Client | Client socket handling |
| `source/memwatch.c` | Memwatch | Memory debugging |

### 1.2 Header Files

| File | Module | Description |
|------|--------|-------------|
| `header/httpCommon.h` | Common | Basic types, constants, macros |
| `header/httpConf.h` | Configuration | Config types and function declarations |
| `header/httpMemsys.h` | Memory | Allocator types |
| `header/httpLog.h` | Logging | Log system types |
| `header/httpSock.h` | Networking | Socket function declarations |
| `header/httpThreads.h` | Threading | Thread types and declarations |
| `header/httpLock.h` | Locking | Lock types and declarations |
| `header/httpList.h` | Lists | List types and declarations |
| `header/httpUtils.h` | Utilities | Utility function declarations |
| `header/httpConn.h` | Connection | Connection function declarations |
| `header/httpProcessor.h` | Processor | Processor function declarations |
| `header/httpRequest.h` | Request | Request types and declarations |
| `header/httpResponse.h` | Response | Response types and declarations |
| `header/httpProcessor.h` | Processor | Processor types |
| `header/httpEngine.h` | Engine | Engine function declarations |
| `header/httpHTML.h` | HTML | HTML function declarations |
| `header/httpDirlister.h` | Dirlister | Dirlister types |
| `header/httpVhost.h` | Vhost | Vhost types and declarations |
| `header/httpCGI.h` | CGI | CGI function declarations |
| `header/httpExecutor.h` | Executor | Executor types and declarations |
| `header/httpEnv.h` | Environment | Environment types |
| `header/httpMime.h` | MIME | MIME types and declarations |
| `header/httpAdmin.h` | Admin | Admin types and declarations |
| `header/httpInclude.h` | Include | Cross-module global declarations |
| `header/memwatch.h` | Memwatch | Memory debugging |

## 2. Data Structure Reference

### 2.1 http_request_t

```c
typedef struct {
   hx_allocator_t *allocator;   // Memory allocator
   SOCKET sock;                 // Client socket
   unsigned char *client_address; // Client IP
   unsigned char time[MAX_TIME_LEN]; // Timestamp
   int method_idx;              // HTTP method index
   unsigned char *uri;          // Raw URI
   int uriLen;                  // URI length
   http_uri_t *uri_object;      // Parsed URI components
   hx_category_t resource_type; // Static or dynamic
   unsigned char *filename;     // Resource filename
   unsigned char *extension;    // File extension
   hx_bool_t is_dir;            // Is directory?
   struct stat resource_stat;   // File statistics
   http_mime_t *mime_object;    // MIME type info
   unsigned char *version;      // HTTP version
   int version_len;             // Version length
   unsigned char *request_line; // Request line
   http_header_t *header[MAX_HEADERS]; // Headers
   int num_headers;             // Header count
   unsigned char *body;         // Request body
   long body_len;               // Body length
   hx_list_t *env;              // Environment variables
   unsigned char *buffer;       // Working buffer
   long buffer_len;             // Buffer length
   unsigned char *request;      // Raw request
   long requestLen;             // Request length
   hx_vhost_t *vhost;           // Virtual host
   hx_logsys_t *log;            // Log system
} http_request_t;
```

### 2.2 http_response_t

```c
typedef struct {
   hx_output_t type;            // PARSED or NONPARSED
   hx_allocator_t *allocator;   // Memory allocator
   unsigned char time[MAX_TIME_LEN]; // Timestamp
   const http_response_code_t *code; // Response code
   unsigned char *statusLine;   // Status line
   unsigned char *headers;      // Response headers
   unsigned char *body;         // Response body
   long body_len;               // Body length
   unsigned char *buffer;       // Working buffer
   long buffer_len;             // Buffer length
   http_request_t *request_object; // Associated request
} http_response_t;
```

### 2.3 hx_vhost_t

```c
typedef struct {
   unsigned char *host;         // Virtual host name
   unsigned char *docroot;      // Document root
   unsigned char *scriptroot;   // CGI root
   unsigned char *errpage;      // Error page path
   hx_logsys_t *log;            // Per-vhost log
} hx_vhost_t;
```

### 2.4 hx_config_t

```c
typedef struct {
   hx_allocator_t *a;           // Allocator
   unsigned char *filename;     // Config filename
   FILE *fptr;                  // Config file pointer
   void **table;                // Config values (indexed)
} hx_config_t;
```

### 2.5 hx_logsys_t

```c
typedef struct {
   hx_allocator_t *allocator;   // Allocator
   unsigned char *name;         // Log name
   unsigned char *logfile;      // Log file path
   FILE *fp;                    // Log file pointer
   hx_lock_t *lock;             // Log lock
   unsigned char prefix[MAX_LOG_PREFIX_LEN];
   unsigned char timebuf[MAX_TIME_LEN];
   unsigned char funcbuf[MAX_FUNCNAME_LEN];
   unsigned char level[MAX_LVLSTR_LEN + 1];
   unsigned char *logbuf1;
   unsigned char *logbuf2;
   long mask;                   // Log level mask
   int needPrefix;              // Need prefix flag
} hx_logsys_t;
```

### 2.6 hx_allocator_t

```c
typedef struct {
#ifdef HX_DEBUG_MEMSYS
   unsigned char file[MAX_FILENAME];
   int line;
#endif
   unsigned char name[MAX_ALLOCNAME];
   long poolId;
   hx_mem_t *head;
   hx_mem_t *tail;
   long numElts;
} hx_allocator_t;
```

### 2.7 http_uri_t

```c
typedef struct {
   unsigned char *host;     // Hostname
   unsigned short port;     // Port
   unsigned char *queryStr; // Query string
   unsigned char *resource; // Resource path
} http_uri_t;
```

### 2.8 http_header_t

```c
typedef struct {
   int header_idx;    // Header index
   unsigned char *value; // Header value
} http_header_t;
```

### 2.9 http_mime_t

```c
typedef struct {
   unsigned char *ext;    // Extension
   unsigned char *type;   // MIME type
   unsigned char *subtype; // MIME subtype
} http_mime_t;
```

### 2.10 direlem_t

```c
typedef struct {
   unsigned char *parent;   // Parent directory
   unsigned char *entroot;  // Entity root
   unsigned char *href;     // HREF link
   unsigned char *icon;     // Icon
   unsigned char *ent;      // Entity name
   size_t size;             // Entity size
   time_t lmt;              // Last modification
   unsigned char *mode;     // Permission mode
   int isdir;               // Is directory?
} direlem_t;
```

### 2.11 hx_process_t

```c
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

### 2.12 http_admin_t

```c
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
```

## 3. Constant Reference

### 3.1 HTTP Method Indices

| Index | Method | Macro |
|-------|--------|-------|
| 0 | OPTIONS | IDX_OPTIONS |
| 1 | HEAD | IDX_HEAD |
| 2 | POST | IDX_POST |
| 3 | PUT | IDX_PUT |
| 4 | DELETE | IDX_DELETE |
| 5 | TRACE | IDX_TRACE |
| 6 | GET | IDX_GET |
| 7 | CONNECT | IDX_CONNECT |

### 3.2 Header Indices

| Index | Header |
|-------|--------|
| 0 | Cache-Control |
| 1 | Connection |
| 2 | Date |
| 3 | Pragma |
| 4 | Trailer |
| 5 | Transfer-Encoding |
| 6 | Upgrade |
| 7 | Via |
| 8 | Warning |
| 9 | Accept |
| 10 | Accept-Charset |
| 11 | Accept-Encoding |
| 12 | Accept-Language |
| 13 | Authorization |
| 14 | Expect |
| 15 | From |
| 16 | Host |
| 17 | If-Match |
| 18 | If-Modified-Since |
| 19 | If-None-Match |
| 20 | If-Range |
| 21 | If-Unmodified-Since |
| 22 | Max-Forwards |
| 23 | Proxy-Authorization |
| 24 | Range |
| 25 | Referer |
| 26 | TE |
| 27 | User-Agent |
| 28 | Accept-Ranges |
| 29 | Age |
| 30 | ETag |
| 31 | Location |
| 32 | Proxy-Authenticate |
| 33 | Retry-After |
| 34 | Server |
| 35 | Vary |
| 36 | WWW-Authenticate |
| 37 | Allow |
| 38 | Content-Encoding |
| 39 | Content-Language |
| 40 | Content-Length |
| 41 | Content-Location |
| 42 | Content-MD5 |
| 43 | Content-Range |
| 44 | Content-Type |
| 45 | Expires |
| 46 | Last-Modified |
| 47 | Client-Date |

### 3.3 Response Code Indices

| Index | Code | Description |
|-------|------|-------------|
| 0 | 100 | Continue |
| 1 | 101 | Switching Protocols |
| 2 | 200 | OK |
| 3 | 201 | Created |
| 4 | 202 | Accepted |
| 5 | 203 | Non-Authoritative Information |
| 6 | 204 | No Content |
| 7 | 205 | Reset Content |
| 8 | 206 | Partial Content |
| 9 | 300 | Multiple Choices |
| 10 | 301 | Moved Permanently |
| 11 | 302 | Found |
| 12 | 303 | See Other |
| 13 | 304 | Not Modified |
| 14 | 305 | Use Proxy |
| 15 | 307 | Temporary Redirect |
| 16 | 400 | Bad Request |
| 17 | 401 | Unauthorized |
| 18 | 402 | Payment Required |
| 19 | 403 | Forbidden |
| 20 | 404 | Not Found |
| 21 | 405 | Method Not Allowed |
| 22 | 406 | Not Acceptable |
| 23 | 407 | Proxy Authentication Required |
| 24 | 408 | Request Time-out |
| 25 | 409 | Conflict |
| 26 | 410 | Gone |
| 27 | 411 | Length Required |
| 28 | 412 | Precondition Failed |
| 29 | 413 | Request Entity Too Large |
| 30 | 414 | Request-URI Too Large |
| 31 | 415 | Unsupported Media Type |
| 32 | 416 | Requested range not satisfiable |
| 33 | 417 | Expectation Failed |
| 34 | 500 | Internal Server Error |
| 35 | 501 | Not Implemented |
| 36 | 502 | Bad Gateway |
| 37 | 503 | Service Unavailable |
| 38 | 504 | Gateway Time-out |
| 39 | 505 | HTTP Version not supported |

### 3.4 Log Levels

| Level | String | Macro |
|-------|--------|-------|
| 1 | erro | HX_LERRO |
| 2 | warn | HX_LWARN |
| 3 | verb | HX_LVERB |
| 4 | debg | HX_LDEBG |
| 5 | mime | HX_LDMIM |
| 6 | call | HX_LCALL |
| 7 | reqs | HX_LREQS |
| 8 | resp | HX_LRESP |
| 9 | dump | HX_LDUMP |
| 10 | syst | HX_LSYST |

### 3.5 Config Record Indices

| Index | Name |
|-------|------|
| 0 | IDX_NUM_SRV_THRDS |
| 1 | IDX_NUM_ADM_THRDS |
| 2 | IDX_DOC_ROOT |
| 3 | IDX_CGI_ROOT |
| 4 | IDX_SERVER_TIMEOUT |
| 5 | IDX_DEFAULT_HTTP_PORT |
| 6 | IDX_DEFAULT_HOST_NAME |
| 7 | IDX_DEFAULT_ERR_PAGE |
| 8 | IDX_CONF_ROOT |
| 9 | IDX_MIME_FILE_NAME |
| 10 | IDX_HTTP_VERSION |
| 11 | IDX_PATH_SEPARATOR |
| 12 | IDX_INDEX_FILES |
| 13 | IDX_ADMIN_PORT |
| 14 | IDX_HTTP_SERVICE_PORT |
| 15 | IDX_LOG_LEVEL |
| 16 | IDX_VHOST |

### 3.6 VHOST Parameter Indices

| Index | Name |
|-------|------|
| 0 | VHOSTPARM_HOSTNAME |
| 1 | VHOSTPARM_DOCROOT |
| 2 | VHOSTPARM_SCRIPTROOT |
| 3 | VHOSTPARM_LOGFILE |
| 4 | VHOSTPARM_ERRPAGE |

### 3.7 CGI Meta Variable Indices

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

## 4. Macro Reference

### 4.1 Return Codes

| Macro | Value | Description |
|-------|-------|-------------|
| OK | 1 | Success |
| ERRC | -1 | Error |
| NOMATCH | -1 | No match |

### 4.2 Buffer Sizes

| Macro | Value | Description |
|-------|-------|-------------|
| MAX_BUFF_LEN | 8192 | Buffer length |
| MAX_REQUEST_LINE_LEN | 8192 | Request line length |
| MAX_STATUS_LINE_LEN | 256 | Status line length |
| MAX_REQUEST_BODY | 8192 | Request body length |
| MAX_HEADER_LEN | 8448 | Header total length |
| MAX_TOTAL_HEADER_LEN | 2048 | Total header length |
| MAX_LOG_BUFF_LEN | 4096 | Log buffer length |
| MAX_FILENAME | 256 | Filename length |
| MAX_TIME_LEN | 40 | Time string length |
| MAX_PATH_LEN | - | Path length |

### 4.3 Timeouts

| Macro | Value | Description |
|-------|-------|-------------|
| TIMEOUT_RECV | 60 | Receive timeout (sec) |
| TIMEOUT_SEND | 60 | Send timeout (sec) |
| SOCK_CLOSE_DELAY | 0.001 | Socket close delay (sec) |

### 4.4 Socket Error Codes

Winsock error codes (WSAExxx) are defined in `httpSock.c` for Windows platforms. Standard errno values are used on UNIX.

## 5. Enumeration Reference

### 5.1 hx_errcodes_t

Error codes for the server:

| Code | Value | Description |
|------|-------|-------------|
| HX_ERR_CHILDREAD_TIMED_OUT | -10 | Child read timed out |
| HX_ERR_CHILDWRITE_TIMED_OUT | -11 | Child write timed out |
| HX_ERR_CHILDREAD | -12 | Child read error |
| HX_ERR_CHILDWRITE | -13 | Child write error |
| HX_ERR_IOCTL | -14 | IOCTL error |
| HX_ERR_SELECT | -15 | Select error |
| HX_ERR_SYSTEM | -16 | System error |
| HX_ERR_MEM_ALLOC | 32 | Memory allocation error |
| HX_ERR_INVALID_REQUEST | - | Invalid request |
| HX_ERR_BAD_REQUEST | - | Bad request |
| HX_ERR_REQUEST_URI_LARGE | - | Request URI too large |
| HX_ERR_INVALID_HEADER | - | Invalid header |
| HX_ERR_INVALID_METHOD | - | Invalid method |
| HX_ERR_INVALID_URI | - | Invalid URI |
| HX_ERR_INDEX_FILE_LIST | - | Index file list error |
| HX_ERR_INTERNAL | - | Internal error |
| HX_ERR_FILE_OPEN | - | File open error |
| HX_ERR_FILE_READ | - | File read error |
| HX_ERR_MIMETABLE_INITED | - | MIME table already initialized |
| HX_ERR_MIMEFILE_NOT_FOUND | - | MIME file not found |
| HX_ERR_INVALID_MIME_LINE | - | Invalid MIME line |
| HX_ERR_INVALID_MIME_FILE | - | Invalid MIME file |
| HX_ERR_FILE_FORBIDDEN | - | File forbidden |
| HX_ERR_FILE_NOEXIST | - | File does not exist |
| HX_ERR_INSUFF_BUFFLEN | - | Insufficient buffer length |
| HX_ERR_CREATEPIPE | - | Pipe creation error |
| HX_ERR_CREATEPROCESS | - | Process creation error |
| HX_ERR_STREAM_NUM | - | Stream number error |
| HX_ERR_PIPEFUNC | - | Pipe function error |
| HX_ERR_PROCESS_CREATE | - | Process create error |
| HX_ERR_INVALID_PARAM | - | Invalid parameter |
| HX_ERR_INVALID_THRD_HNDL | - | Invalid thread handle |
| HX_ERR_INVALID_THRD_PTR | - | Invalid thread pointer |
| HX_ERR_WAIT_FAILED | - | Wait failed |
| HX_ERR_JOIN_FAILED_SRCH | - | Join failed (search) |
| HX_ERR_JOIN_FAILED_DEADLK | - | Join failed (deadlock) |
| HX_ERR_CLOSE_HANDLE | - | Close handle error |
| HX_ERR_INTERPRETER | - | Interpreter error |
| HX_ERR_HANDLEFUNC | - | Handle function error |
| HX_ERR_INVALID_ALLOCATOR | - | Invalid allocator |
| HX_ERR_INVALID_FREE | - | Invalid free |
| HX_ERR_READFILE | - | Read file error |
| HX_ERR_CONFROOT_NOT_FOUND | - | Config root not found |
| HX_ERR_CONFIG_DOC_ROOT | - | Config doc root error |
| HX_ERR_CONFIG_INITED | - | Config already initialized |
| HX_ERR_CONFIG_CREATE | - | Config create error |
| HX_ERR_CONFIG_LOAD | - | Config load error |
| HX_ERR_CONFIG_FUNC | - | Config function error |
| HX_ERR_CONFIG_KEY | - | Config key error |
| HX_ERR_CONFIG_DATA | - | Config data error |
| HX_ERR_CONFIG_NUM_SRV_THREADS | - | Config server threads error |
| HX_ERR_CONFIG_NUM_ADM_THREADS | - | Config admin threads error |
| HX_ERR_CONFIG_SERVER_TIMEOUT | - | Config timeout error |
| HX_ERR_CONFIG_ADMIN_PORT | - | Config admin port error |
| HX_ERR_CONFIG_HTTP_PORT | - | Config HTTP port error |
| HX_ERR_CONFIG_DEFERR_PAGE | - | Config error page error |
| HX_ERR_ADM_INVALID_LEN | - | Admin invalid length |
| HX_ERR_ADM_INVALID_MSG | - | Admin invalid message |
| HX_ERR_ADM_INVALID_CMD | - | Admin invalid command |
| HX_ERR_VHOST_READY | - | Vhost ready |
| HX_ERR_VHOST_DOCROOT | - | Vhost doc root error |
| HX_ERR_VHOST_NOTREADY | - | Vhost not ready |
| HX_ERR_VHOST_ADD | - | Vhost add error |
| HX_ERR_VHOST_NOEXIST | - | Vhost does not exist |
| HX_ERR_CREATE_LOG | - | Log create error |
| HX_ERR_PID_FILECREATE | - | PID file create error |
| HX_ERR_GETPID | - | Get PID error |
| HX_ERR_CLIENT | - | Client error |
| HX_ERR_UNKNOWN | - | Unknown error |

## 6. Function Reference Summary

### 6.1 Core Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_create_allocator(name)` | httpMemsys.c | Create allocator |
| `hx_destroy_allocator(allocator)` | httpMemsys.c | Destroy allocator |
| `hx_alloc_mem(size, allocator)` | httpMemsys.c | Allocate memory |
| `hx_free_mem(ptr, allocator)` | httpMemsys.c | Free memory |
| `hx_create_log_sys(log, filename, name, needPrefix)` | httpLog.c | Create log system |
| `hx_destroy_log_sys(logsys)` | httpLog.c | Destroy log system |
| `hx_log_msg(logsys, loglvl, func, format, ...)` | httpLog.c | Log message |
| `hx_hex_dump(logsys, buffer, len, level)` | httpLog.c | Hex dump |
| `hx_create_lock()` | httpLock.c | Create lock |
| `hx_destroy_lock(lock)` | httpLock.c | Destroy lock |
| `hx_lock(lock)` | httpLock.c | Acquire lock |
| `hx_unlock(lock)` | httpLock.c | Release lock |
| `hx_create_list()` | httpList.c | Create list |
| `hx_destroy_list(list)` | httpList.c | Destroy list |
| `hx_add(list, data)` | httpList.c | Append to list |
| `hx_remove(list)` | httpList.c | Remove from tail |
| `hx_remove_first(list)` | httpList.c | Remove from head |
| `hx_create_thread(a, func, param)` | httpThreads.c | Create thread |
| `hx_sock_init()` | httpSock.c | Initialize sockets |
| `hx_serverSock(log, port, queuelen)` | httpSock.c | Create server socket |
| `hx_clientSock(log, host, port)` | httpSock.c | Create client socket |
| `hx_sock_read(s, buff, len)` | httpSock.c | Read from socket |
| `hx_sockWrite(s, response, len)` | httpSock.c | Write to socket |

### 6.2 Configuration Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_load_config(filename, logsys)` | httpConf.c | Load config |
| `hx_reload_config(logsys)` | httpConf.c | Reload config |
| `hx_destroy_config(logsys)` | httpConf.c | Destroy config |
| `hx_get_conf_val(log, index)` | httpConf.c | Get config value |

### 6.3 Virtual Host Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_getVhost(log, hostname)` | httpVhost.c | Look up vhost |
| `hx_addVhost(log, ...)` | httpVhost.c | Add vhost |
| `hx_add_default_vhost(log)` | httpVhost.c | Add default vhost |
| `hx_create_vhost_sys(log)` | httpVhost.c | Create vhost system |
| `hx_destroyVhostSys(log)` | httpVhost.c | Destroy vhost system |

### 6.4 Request Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_createReqObj(allocator)` | httpRequest.c | Create request object |
| `hx_parse_request(request)` | httpRequest.c | Parse request |
| `hx_parse_request_line(request)` | httpRequest.c | Parse request line |
| `hx_parse_headers(request)` | httpRequest.c | Parse headers |
| `hx_parse_body(request)` | httpRequest.c | Parse body |
| `hx_parse_uri(request)` | httpRequest.c | Parse URI |
| `hx_add_header(request, header)` | httpRequest.c | Add header |
| `hx_get_header_value(index, request)` | httpRequest.c | Get header value |
| `hx_print_request(request, level)` | httpRequest.c | Print request |

### 6.5 Response Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_createResObj(request_object)` | httpResponse.c | Create response object |
| `hx_form_error_response(resp, req, errIndex, caller)` | httpResponse.c | Form error response |
| `hx_formDirListing(resp, req)` | httpResponse.c | Form directory listing |
| `hx_add_response_header(log, buffer, idx, str)` | httpResponse.c | Add response header |
| `hx_printResponse(httpRes, loglvl)` | httpResponse.c | Print response |
| `hx_sendErrResponse(sock, resp)` | httpResponse.c | Send error response |
| `hx_sendResponse(sock, resp)` | httpResponse.c | Send response |
| `hx_create_status_line(log, status_index, resource, allocator)` | httpResponse.c | Create status line |

### 6.6 Processor Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_process_http(client_socket, client_address, allocator)` | httpProcessor.c | Process HTTP connection |
| `hx_process_admin(client_socket, client_address, allocator)` | httpProcessor.c | Process admin connection |
| `hx_processRequest(request, response)` | httpProcessor.c | Process request |

### 6.7 Engine Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_OPTIONS(request, response)` | httpEngine.c | OPTIONS handler |
| `hx_HEAD(request, response)` | httpEngine.c | HEAD handler |
| `hx_GET(request, response)` | httpEngine.c | GET handler |
| `hx_POST(request, response)` | httpEngine.c | POST handler |
| `hx_PUT(request, response)` | httpEngine.c | PUT handler |
| `hx_DELETE(request, response)` | httpEngine.c | DELETE handler |
| `hx_TRACE(request, response)` | httpEngine.c | TRACE handler |
| `hx_CONNECT(request, response)` | httpEngine.c | CONNECT handler |

### 6.8 CGI Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_handle_cgi(request, response)` | httpCGI.c | Handle CGI |
| `hx_invokeCGI(request, childProc)` | httpExecutor.c | Invoke CGI |
| `hx_cleanProc(log, child)` | httpExecutor.c | Clean process |
| `hx_kill_child(log, child)` | httpExecutor.c | Kill child |

### 6.9 MIME Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_init_mime_table(log, allocator)` | httpMime.c | Initialize MIME table |
| `hx_lookup_ext_mime(log, ext)` | httpMime.c | Look up MIME by extension |
| `hx_lookup_resource_mime(log, resource)` | httpMime.c | Look up MIME by resource |
| `hx_get_mime_type(mime)` | httpMime.c | Get MIME type |
| `hx_get_mime_sub_type(mime)` | httpMime.c | Get MIME subtype |

### 6.10 Admin Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_reload_cfg(admin)` | httpAdmin.c | Reload config |
| `hx_restart_log(admin)` | httpAdmin.c | Restart log |
| `hx_shutdown(admin)` | httpAdmin.c | Shutdown |
| `hx_create_admin_object(allocator)` | httpAdmin.c | Create admin object |
| `hx_parse_admin(admin)` | httpAdmin.c | Parse admin message |
| `hx_execute_admin(admin)` | httpAdmin.c | Execute admin command |

### 6.11 Utility Functions

| Function | File | Description |
|----------|------|-------------|
| `hx_strdup(source, allocator)` | httpUtils.c | Duplicate string |
| `hx_strndup(source, length, allocator)` | httpUtils.c | Duplicate string with length |
| `hx_trim(source, allocator)` | httpUtils.c | Trim whitespace |
| `hx_strcmpi(arg1, arg2)` | httpUtils.c | Case-insensitive compare |
| `hx_getTime(timestamp, len)` | httpUtils.c | Get time string |
| `hx_matchIdx(dict, dictLen, word, wordLen)` | httpUtils.c | Match word |
| `hx_read_file(log, filename, buffer, datalen, allocator, lock)` | httpUtils.c | Read file |
| `hx_getResDetails(request, lock)` | httpUtils.c | Get resource details |
| `hx_getFileStat(log, resource, rstat, lock)` | httpUtils.c | Get file stat |
| `hx_escape(log, dest, source, toEsc)` | httpUtils.c | Escape string |
| `hx_convert(log, string, from, to)` | httpUtils.c | Convert encoding |
| `hx_getSysErrCode(errcode)` | httpUtils.c | Get system error code |
| `hx_get_response_code(errcode)` | httpUtils.c | Get HTTP response code |
| `hx_isLocalhost(vhostname)` | httpUtils.c | Check localhost |
| `hx_checkOuput(buffer, buffLen)` | httpUtils.c | Check output type |
| `hx_getErrorString(errno)` | httpUtils.c | Get error string |
| `hx_sleep(secs)` | httpUtils.c | Sleep |
| `hx_createPage(buffer, title, body, bufflen, lock)` | httpUtils.c | Create HTML page |

## 7. Global Variable Reference

| Variable | Type | Description |
|----------|------|-------------|
| `server_shutdown` | int | Shutdown state |
| `http_done` | int | HTTP threads done |
| `admin_done` | int | Admin threads done |
| `VERSION` | const unsigned char * | "V1.0" |
| `SERVER` | const unsigned char * | "httpdEx" |
| `server_log_sys` | hx_logsys_t * | Main log system |
| `defaultVhost` | hx_vhost_t * | Default virtual host |
| `serverSock` | SOCKET | HTTP server socket |
| `adminSock` | SOCKET | Admin server socket |
| `server_allocator` | hx_allocator_t * | Main allocator |
| `lock_num_clients` | hx_lock_t * | Client count lock |
| `lock_accept_http` | hx_lock_t * | HTTP accept lock |
| `lock_accept_admin` | hx_lock_t * | Admin accept lock |
| `lock_errno` | hx_lock_t * | ERRNO lock |
| `lock_shutdown` | hx_lock_t * | Shutdown lock |
| `g_num_clients_being_served` | int | Client count |
| `mimeTable` | http_mime_table_t * | MIME table |
| `g_config` | hx_config_t * | Configuration |
| `g_engine[MAX_METHODS]` | fhandler_t[] | Method handler table |
| `header_names_table[MAX_HEADERS]` | const unsigned char *[] | Header names |
| `method_names[MAX_METHODS]` | const unsigned char *[] | Method names |
| `Responses[MAX_RESPONSE_CODES]` | http_response_code_t[] | Response codes |
| `NotImplemented[MAX_NOT_IMPLEMENTED]` | const unsigned char *[] | Not implemented extensions |

## 8. Build Reference

### 8.1 Makefile Targets

| Target | Description |
|--------|-------------|
| `all` | Build both httpdex and httpc |
| `httpdex` | Build server only |
| `httpc` | Build client only |
| `clean` | Remove object files and binaries |
| `rebuild` | Clean and rebuild |
| `winclean` | Windows clean |

### 8.2 Object File Dependencies

The makefile specifies dependencies for each object file, ensuring proper recompilation when headers change.

### 8.3 Compilation Command Pattern

```bash
gcc -g -c -o objs/httpMain.obj -D_HX_UNIX_ -D_PTHREADS_ -Iheader source/httpMain.c
```

### 8.4 Link Command Pattern

```bash
gcc -g -o bin/httpdex objs/httpMain.obj objs/httpSpawner.obj ... -lpthread
```

## 9. Directory Reference

### 9.1 Source Files

```
source/
├── httpMain.c
├── httpConf.c
├── httpMemsys.c
├── httpLog.c
├── httpSock.c
├── httpThreads.c
├── httpLock.c
├── httpList.c
├── httpUtils.c
├── httpConn.c
├── httpProcessor.c
├── httpRequest.c
├── httpResponse.c
├── httpEngine.c
├── httpHTML.c
├── httpDirlister.c
├── httpVhost.c
├── httpCGI.c
├── httpExecutor.c
├── httpEnv.c
├── httpMime.c
├── httpAdmin.c
├── httpController.c
├── httpClient.c
└── memwatch.c
```

### 9.2 Header Files

```
header/
├── httpCommon.h
├── httpConf.h
├── httpMemsys.h
├── httpLog.h
├── httpSock.h
├── httpThreads.h
├── httpLock.h
├── httpList.h
├── httpUtils.h
├── httpConn.h
├── httpProcessor.h
├── httpRequest.h
├── httpResponse.h
├── httpEngine.h
├── httpHTML.h
├── httpDirlister.h
├── httpVhost.h
├── httpCGI.h
├── httpExecutor.h
├── httpEnv.h
├── httpMime.h
├── httpAdmin.h
├── httpInclude.h
└── memwatch.h
```

### 9.3 Configuration Files

```
conf/
├── httpdex.conf    # Main config (text format)
├── httpdex-conf.xml # XML config (unused)
└── mime.data       # MIME type definitions
```

### 9.4 Web Root

```
webroot/
├── index.htm       # Default home page
├── logo.gif        # Server logo
├── error.gif       # Error icon
└── vhosts/         # Virtual host directories
```

### 9.5 Resources

```
resources/
├── logo.gif        # Server logo
└── logo.jpg        # Server logo
```

### 9.6 Scripts

```
scripts/
├── hdstart         # Start server script
├── hdstop          # Stop server script
└── hdlog           # Log viewer script
```

### 9.7 Tests

```
tests/
└── testURLs.txt    # Test URLs
```

### 9.8 Build Files

```
build/
├── makefile.lnx    # Linux makefile
├── makefile.cygwin # Cygwin makefile
├── httpdex.dsw     # Visual Studio workspace
└── httpdex.sln     # Visual Studio solution
```

## 10. Protocol Reference

### 10.1 HTTP Request Format

```
METHOD /resource HTTP/1.1\r\n
Host: hostname\r\n
Header: value\r\n
\r\n
[body]
```

### 10.2 HTTP Response Format

```
HTTP/1.1 200 OK\r\n
Header: value\r\n
\r\n
[body]
```

### 10.3 Admin Protocol

```
<4-digit length><command> [arguments]\r\n
```

Example: `0008reload_config`

Response: `<4-digit length><status><message>`

Where status: 0=OK, 1=ERROR, 2=WARN

## 11. File Format Reference

### 11.1 Config File Format

```
# Comment
DIRECTIVE=value
VHOST=hostname,docroot,scriptroot,logfile,[loglevels]
```

### 11.2 MIME Data Format

```
# Comment
MIME/type ext1 ext2 ext3
```

### 11.3 PID File Format

```
<pid>
```

## 12. Signal Reference

| Signal | Handler | Action |
|--------|---------|--------|
| SIGINT | hx_sigint() | Normal shutdown |
| SIGUSR1 | hx_sigusr1() | Normal shutdown |
| SIGSEGV | hx_sigsegv() | Abortive shutdown |

## 13. Thread Reference

| Thread | Function | Description |
|--------|----------|-------------|
| Admin spawner | hx_spawn_admin() | Listens for admin connections |
| Admin workers | hx_accept_admin_conn() | Process admin requests |
| HTTP spawner | hx_spawn_workers() | Listens for HTTP connections |
| HTTP workers | hx_accept_http_conn() | Process HTTP requests |

## 14. Error String Reference

The `hx_getErrorString()` function returns human-readable strings for error codes. These strings are defined in `httpUtils.c`.

## 15. Version Information

- **Server**: httpdEx V1.0
- **Copyright**: (c) 2003 Avinandan Sengupta
- **Status**: Experimental

---
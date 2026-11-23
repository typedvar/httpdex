# httpdEx - Architecture Documentation

## 1. System Architecture Overview

httpdEx follows a modular, layered architecture designed for clarity, maintainability, and extensibility. The system is composed of several interacting subsystems, each with well-defined responsibilities.

## 2. Module Decomposition

The server is decomposed into the following modules:

### 2.1 Core Subsystems

| Module | Files | Responsibility |
|--------|-------|----------------|
| **Main** | `source/httpMain.c` | Server entry point, initialization, shutdown, signal handling |
| **Configuration** | `source/httpConf.c`, `header/httpConf.h` | Dynamic config loading, reloading, validation |
| **Memory System** | `source/httpMemsys.c`, `header/httpMemsys.h` | Custom memory pool allocator |
| **Logging** | `source/httpLog.c`, `header/httpLog.h` | Multi-level structured logging |
| **Networking** | `source/httpSock.c`, `header/httpSock.h` | Socket abstraction (TCP server/client) |
| **Threading** | `source/httpThreads.c`, `header/httpThreads.h` | Cross-platform thread management |
| **Locking** | `source/httpLock.c`, `header/httpLock.h` | Mutual exclusion primitives |
| **Lists** | `source/httpList.c`, `header/httpList.h` | Doubly-linked list data structure |

### 2.2 Request Processing

| Module | Files | Responsibility |
|--------|-------|----------------|
| **Connection** | `source/httpConn.c`, `header/httpConn.h` | Connection acceptance, client processing |
| **Processor** | `source/httpProcessor.c`, `header/httpProcessor.h` | HTTP request routing to handlers |
| **Request** | `source/httpRequest.c`, `header/httpRequest.h` | HTTP request parsing (line, headers, URI, body) |
| **Response** | `source/httpResponse.c`, `header/httpResponse.h` | HTTP response generation and sending |
| **Engine** | `source/httpEngine.c`, `header/httpEngine.h` | HTTP method handler dispatch table |
| **HTML** | `source/httpHTML.c`, `header/httpHTML.h` | HTML page generation utilities |
| **Dirlister** | `source/httpDirlister.c`, `header/httpDirlister.h` | Directory listing generation |
| **Utils** | `source/httpUtils.c`, `header/httpUtils.h` | String, time, and file utilities |

### 2.3 Virtual Hosting

| Module | Files | Responsibility |
|--------|-------|----------------|
| **Vhost** | `source/httpVhost.c`, `header/httpVhost.h` | Virtual host management and lookup |

### 2.4 Dynamic Content

| Module | Files | Responsibility |
|--------|-------|----------------|
| **CGI** | `source/httpCGI.c`, `header/httpCGI.h` | CGI script execution |
| **Executor** | `source/httpExecutor.c`, `header/httpExecutor.h` | Process spawning and management |
| **Environment** | `source/httpEnv.c`, `header/httpEnv.h` | Environment block creation (Win32/UNIX) |

### 2.5 MIME Handling

| Module | Files | Responsibility |
|--------|-------|----------------|
| **MIME** | `source/httpMime.c`, `header/httpMime.h` | MIME type lookup from mime.data |

### 2.6 Administration

| Module | Files | Responsibility |
|--------|-------|----------------|
| **Admin** | `source/httpAdmin.c`, `header/httpAdmin.h` | Admin protocol, commands (reload, restart_log, shutdown) |
| **Controller** | `source/httpController.c`, `header/httpController.h` | Admin client (telnet-style) |
| **Client** | `source/httpClient.c` | Admin client socket handling |

### 2.7 Memory Debugging

| Module | Files | Responsibility |
|--------|-------|----------------|
| **Memwatch** | `source/memwatch.c`, `header/memwatch.h` | Memory allocation debugging |

## 3. Data Flow

### 3.1 Request Processing Flow

```
Client Connection
      │
      ▼
hx_accept_http_conn()  ← Accept connection on server socket
      │
      ▼
hx_process_http()      ← Create request object, parse request
      │
      ▼
hx_parse_request()     ← Parse request line, headers, body, URI
      │
      ▼
hx_processRequest()    ← Route to static or dynamic handler
      │
      ├─ Static Resource ──────────────┐
      │                                ▼
      │                         hx_GET() ← Read file, form response
      │                                │
      │                                ▼
      │                         hx_sendResponse() ← Send to client
      │
      └─ Dynamic Resource (CGI) ───────┐
                                       ▼
                                hx_handle_cgi() ← Execute CGI script
                                       │
                                       ▼
                                hx_sendResponse() ← Send to client
```

### 3.2 Virtual Host Resolution Flow

```
Request arrives with Host header
      │
      ▼
Extract hostname from Host header
      │
      ▼
Check if hostname is localhost
      ├─ Yes ───► Use defaultVhost
      └─ No ─────► hx_getVhost() ← Look up in vhost list
                      │
                      ▼
              Vhost found ───► Use vhost config
              │
              ▼
          Vhost not found ───► Fall back to defaultVhost
```

### 3.3 Configuration Loading Flow

```
Server starts
      │
      ▼
hx_load_config() ← Read config file line by line
      │
      ▼
For each line:
      ├─ Match command in cmdTable
      │   └─ Call handler function (e.g., hx_set_doc_root)
      │
      └─ VHOST command ───► hx_set_vhost_data() ← Parse vhost params
                              │
                              ▼
                    hx_addVhost() ← Create vhost object, add to list
```

### 3.4 Administration Flow

```
Admin client connects
      │
      ▼
hx_accept_admin_conn() ← Accept admin connection
      │
      ▼
hx_process_admin() ← Parse admin command
      │
      ▼
Command dispatch:
      ├─ reload_config ───► hx_reload_cfg() ← Reload config
      ├─ restart_log    ───► hx_restart_log() ← Restart logging
      └─ shutdown       ───► hx_shutdown() ← Signal shutdown
```

## 4. Threading Model

The server uses a multi-threaded model:

### 4.1 Thread Types

| Thread Type | Count | Function |
|-------------|-------|----------|
| **Admin Spawner** | 1 | Listens for admin connections |
| **Admin Workers** | Configurable (ADMIN_THREADS) | Process admin requests |
| **HTTP Spawner** | 1 | Listens for HTTP connections |
| **HTTP Workers** | Configurable (SERVER_THREADS) | Process HTTP requests |

### 4.2 Thread Lifecycle

1. **Main thread**: Initializes server, creates spawner threads, waits for shutdown
2. **Spawner threads**: Create listening sockets, spawn worker threads, wait for workers
3. **Worker threads**: Accept connections in a loop, process each connection, close socket

### 4.3 Synchronization

- **Locks**: Used for mutual exclusion on shared resources (client count, accept sockets, errno, shutdown state)
- **Lists**: Thread-safe doubly-linked lists with internal locking
- **Log system**: Thread-safe with per-log lock

## 5. Memory Management

### 5.1 Custom Allocator

The server uses a custom memory pool allocator (`hx_allocator_t`) instead of standard `malloc`/`free`:

- **Allocator**: Named memory pool with head/tail linked list of allocated blocks
- **Allocation**: `hx_alloc_mem()` allocates from the named pool
- **Deallocation**: `hx_free_mem()` returns memory to the pool
- **Destroy**: `hx_destroy_allocator()` frees all memory in the pool

### 5.2 Allocator Hierarchy

```
server_allocator (global, created in main)
├─ Request allocator (per request, created in worker threads)
├─ Log allocator (per log system)
├─ Lock allocator (per lock)
└─ List allocator (per list)
```

### 5.3 Memory Debugging

When `HX_DEBUG_MEMSYS` is defined:
- Allocation tracking with file/line information
- Reuse count tracking for memory blocks
- Debug versions of all allocator functions

## 6. Logging System

### 6.1 Log Levels

| Level | Macro | Description |
|-------|-------|-------------|
| 1 | HX_LERRO | Errors (always on) |
| 2 | HX_LWARN | Warnings (always on) |
| 3 | HX_LVERB | Verbose information |
| 4 | HX_LDEBG | Debug messages |
| 5 | HX_LDMIM | MIME table data |
| 6 | HX_LCALL | Function call tracing |
| 7 | HX_LREQS | Request details |
| 8 | HX_LRESP | Response details |
| 9 | HX_LDUMP | Hex dumps |
| 10 | HX_LSYST | System messages (always on) |
| 11-32 | HX_LOGLVL_11-32 | Reserved for future use |

### 6.2 Log Configuration

Log levels are configured via the `LOG_LEVEL` directive in the config file, e.g.:
```
LOG_LEVEL=default, reqs, call, verb, dump
```

### 6.3 Log Message Format

Each log message includes:
- Timestamp
- Function name (prefix)
- Log level
- Message text

## 7. Virtual Hosting

### 7.1 Vhost Configuration

Virtual hosts are configured in the config file using the `VHOST` directive:
```
VHOST=hostname,docroot,scriptroot,logfile,[loglevels]
```

Example:
```
VHOST=www.example.com,C:/www/example,scripts,C:/logs/example.log,default,reqs,verb
```

### 7.2 Vhost Structure

```c
typedef struct {
   unsigned char *host;       // Virtual host name
   unsigned char *docroot;    // Document root directory
   unsigned char *scriptroot; // CGI script root directory
   unsigned char *errpage;    // Error page path
   hx_logsys_t   *log;        // Per-vhost log system
} hx_vhost_t;
```

### 7.3 Default Vhost

A default vhost is created with values from the global configuration (DOCUMENT_ROOT, CGI_ROOT, DEFAULT_HOST_NAME, DEFAULT_ERR_PAGE).

### 7.4 Vhost Resolution

1. Request arrives with Host header
2. Extract hostname (strip port if present)
3. If localhost → use defaultVhost
4. Otherwise → lookup in vhost list
5. If not found → fall back to defaultVhost

## 8. CGI Support

### 8.1 CGI Execution Model

1. Request for a resource under the CGI root is identified as dynamic
2. The CGI handler (`hx_handle_cgi()`) is invoked
3. The executor determines the interpreter (shebang line or binary)
4. A child process is spawned with:
   - Standard input/output/error pipes
   - CGI environment variables
   - Working directory set to script directory
5. Output is read from the child's stdout
6. Errors are read from the child's stderr
7. Response is formed from the CGI output

### 8.2 CGI Environment Variables

| Variable | Description |
|----------|-------------|
| AUTH_TYPE | Authentication type |
| CONTENT_LENGTH | Request body length |
| CONTENT_TYPE | Request content type |
| GATEWAY_INTERFACE | CGI version |
| PATH_INFO | Extra path info |
| PATH_TRANSLATED | Translated path |
| QUERY_STRING | Query string |
| REMOTE_ADDR | Client IP address |
| REMOTE_HOST | Client hostname |
| REMOTE_IDENT | Remote user |
| REMOTE_USER | Authenticated user |
| REQUEST_METHOD | HTTP method |
| SCRIPT_NAME | Script name |
| SERVER_NAME | Server hostname |
| SERVER_PORT | Server port |
| SERVER_PROTOCOL | HTTP protocol version |
| SERVER_SOFTWARE | Server software name |

### 8.3 Interpreter Detection

The executor detects the interpreter type:
1. **Win32**: Check for `.com`/`.exe` extension → binary
2. **Shebang (`#!`)**: Parse interpreter path from first line
3. **ELF magic**: UNIX executable → binary
4. **Default**: Execute the file itself

## 9. MIME Handling

### 9.1 MIME Table

MIME types are loaded from an external file (`mime.data`) at startup. Each entry maps a file extension to a MIME type/subtype pair.

### 9.2 MIME Format

```
# MIME type        Extension
text/html          html htm
image/jpeg         jpeg jpg jpe
application/pdf    pdf
```

### 9.3 MIME Lookup

MIME type is determined by:
1. Looking up the file extension in the MIME table
2. If not found, default to `text/plain`

## 10. Error Handling

### 10.1 Error Codes

The server defines a comprehensive set of error codes:

| Code | Meaning |
|------|---------|
| HX_ERR_MEM_ALLOC | Memory allocation failure |
| HX_ERR_INVALID_REQUEST | Invalid HTTP request |
| HX_ERR_BAD_REQUEST | Bad request (400) |
| HX_ERR_FILE_NOT_FOUND | File not found (404) |
| HX_ERR_VHOST_NOEXIST | Virtual host not found |
| HX_ERR_CONFIG_* | Configuration errors |
| HX_ERR_SOCK_* | Socket errors |

### 10.2 HTTP Response Codes

The server supports standard HTTP response codes (100-505) with descriptive messages.

### 10.3 Error Response Format

Error responses include:
- Status line (e.g., `HTTP/1.1 404 Not Found`)
- HTML body with server version and error description

## 11. Signal Handling

The server handles the following signals:

| Signal | Handler | Action |
|--------|---------|--------|
| SIGINT | hx_sigint() | Normal shutdown |
| SIGUSR1 | hx_sigusr1() | Normal shutdown |
| SIGSEGV | hx_sigsegv() | Abortive shutdown |

All signal handlers:
1. Reset the signal handler
2. Mask all signals
3. Log the signal
4. Set shutdown state to PENDING
5. Call hx_switch_off() for graceful cleanup

## 12. Admin Protocol

The administration interface uses a simple text-based protocol over TCP:

### 12.1 Message Format

```
<length><command> [arguments]
```

Where:
- `length`: 4-digit message length
- `command`: One of `reload_config`, `restart_log`, `shutdown`
- `arguments`: Space-separated arguments (optional)

### 12.2 Commands

| Command | Description |
|---------|-------------|
| reload_config | Reload configuration from disk |
| restart_log | Restart logging system |
| shutdown | Shut down the server |

### 12.3 Response

The server responds with:
- 4-digit length prefix
- Status code (0=OK, 1=ERROR, 2=WARN)
- Response message

## 13. Build Configuration

### 13.1 Compilation Flags

| Flag | Description |
|------|-------------|
| `-D_HX_UNIX_` | UNIX platform (BSD sockets) |
| `-D_HX_WIN32_` | Windows platform (Winsock) |
| `-D_PTHREADS_` | POSIX threads |
| `-D_HX_WINTHREADS_` | Win32 threads |
| `-DMEMWATCH` | Enable memory watch |
| `-DHX_DEBUG_MEMSYS` | Enable memory debugging |

### 13.2 Link Libraries

| Platform | Libraries |
|----------|-----------|
| UNIX | `-lpthread` |
| Windows | Winsock2 (`ws2_32.lib`) |

## 14. Ports and Configuration

### 14.1 Default Ports

| Port | Purpose |
|------|---------|
| 80 | HTTP service port |
| 10002 | Administration port |

### 14.2 Configuration Directives

| Directive | Description | Example |
|-----------|-------------|---------|
| SERVER_THREADS | Number of worker threads | `SERVER_THREADS=10` |
| ADMIN_THREADS | Number of admin threads | `ADMIN_THREADS=2` |
| DOCUMENT_ROOT | Default document root | `DOCUMENT_ROOT=/var/www` |
| CGI_ROOT | CGI script root | `CGI_ROOT=/var/www/cgi-bin` |
| DEFAULT_HTTP_PORT | HTTP port | `DEFAULT_HTTP_PORT=80` |
| DEFAULT_HOST_NAME | Default hostname | `DEFAULT_HOST_NAME=localhost` |
| DEFAULT_ERR_PAGE | Default error page | `DEFAULT_ERR_PAGE=error.htm` |
| CONFIG_ROOT | Config file directory | `CONFIG_ROOT=../conf` |
| MIME_FILE_NAME | MIME data file | `MIME_FILE_NAME=mime.data` |
| HTTP_VERSION | HTTP version | `HTTP_VERSION=HTTP/1.1` |
| PATH_SEPARATOR | Path separator | `PATH_SEPARATOR=/` |
| INDEX_FILES | Index file list | `INDEX_FILES=index.html,index.htm` |
| ADMIN_PORT | Admin port | `ADMIN_PORT=10002` |
| HTTP_SERVICE_PORT | Service port | `HTTP_SERVICE_PORT=80` |
| SERVER_TIMEOUT | Server timeout (sec) | `SERVER_TIMEOUT=300` |
| LOG_LEVEL | Log levels | `LOG_LEVEL=default,reqs,verb` |
| VHOST | Virtual host entry | `VHOST=host,docroot,scriptroot,logfile` |

## 15. Directory Structure

```
httpdex/
├── source/          # C source files
├── header/          # C header files
├── conf/            # Configuration files
├── docs/            # Documentation
├── webroot/         # Default document root
│   ├── index.htm
│   ├── logo.gif
│   ├── error.gif
│   └── vhosts/      # Virtual host directories
├── resources/       # Resource files
│   ├── logo.gif
│   └── logo.jpg
├── scripts/         # Shell scripts
│   ├── hdstart
│   ├── hdstop
│   └── hdlog
├── tests/           # Test files
├── build/           # Build files
│   ├── makefile.lnx
│   ├── makefile.cygwin
│   ├── httpdex.dsw
│   └── httpdex.sln
├── httpd/           # MSVC project (server)
└── httpc/           # MSVC project (client)
```

## 16. Key Design Decisions

### 16.1 Custom Memory Allocator

The server uses a custom memory pool allocator instead of standard malloc/free to:
- Provide deterministic memory management
- Enable memory debugging and leak detection
- Support named allocators for tracking allocations by subsystem

### 16.2 Thread-per-Connection Model

Each worker thread handles one connection at a time in a loop. This provides:
- Simple request processing logic
- Isolation between connections
- Easy error handling per connection

### 16.3 Separate Admin Interface

The admin interface runs on a separate port with its own thread pool to:
- Isolate admin traffic from HTTP traffic
- Allow admin operations during high HTTP load
- Provide secure access control point

### 16.4 Dynamic Configuration

Configuration can be reloaded at runtime via the admin interface without restarting the server, enabling:
- Zero-downtime configuration changes
- Hot-reloading of virtual hosts
- Runtime log level changes

---
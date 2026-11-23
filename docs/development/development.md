# httpdEx - Development Documentation

## 1. Introduction

This document provides guidance for developers working on the httpdEx codebase. It covers the build process, coding standards, code organization, and contribution guidelines.

## 2. Build System

### 2.1 Build Targets

The project builds two executables:

| Target | Description |
|--------|-------------|
| `httpdex` | Main HTTP daemon server |
| `httpc` | HTTP administration client |

### 2.2 Build Methods

#### Linux/UNIX

```bash
cd httpdex/build
make
```

This uses `makefile.lnx` and requires:
- GCC compiler
- pthreads library (`-lpthread`)

#### Cygwin/Windows

```bash
cd httpdex/build
make
```

This uses `makefile.cygwin` and requires:
- Cygwin GCC compiler
- pthreads library

#### Windows/MSVC

Open the solution file `httpdex.sln` in Visual Studio. Two projects are included:
- `httpd` - Server project
- `httpc` - Client project

### 2.3 Compilation Flags

| Flag | Platform | Description |
|------|----------|-------------|
| `-D_HX_UNIX_` | UNIX | Use BSD sockets |
| `-D_HX_WIN32_` | Windows | Use Winsock |
| `-D_PTHREADS_` | UNIX | Use POSIX threads |
| `-D_HX_WINTHREADS_` | Windows | Use Win32 threads |
| `-DMEMWATCH` | All | Enable memory watch |
| `-DHX_DEBUG_MEMSYS` | All | Enable memory debugging |

### 2.4 Build Directory Structure

```
build/
├── makefile.lnx     # Linux makefile
├── makefile.cygwin  # Cygwin makefile
├── httpdex.dsw      # Visual Studio workspace
└── httpdex.sln      # Visual Studio solution
```

### 2.5 Output Directory

Compiled binaries are placed in:
- `bin/httpdex` - Server executable
- `bin/client/httpc` - Client executable

### 2.6 Clean Build

```bash
make clean      # Remove object files and binaries
make rebuild    # Clean and rebuild
```

## 3. Source Code Organization

### 3.1 Directory Structure

```
source/     # C source files (.c)
header/     # C header files (.h)
```

### 3.2 File Naming Convention

- Source files: `http<Module>.c` (e.g., `httpConf.c`, `httpRequest.c`)
- Header files: `http<Module>.h` (e.g., `httpConf.h`, `httpRequest.h`)
- Exception: `memwatch.c`/`memwatch.h` for memory debugging

### 3.3 Module Dependencies

The dependency graph is roughly:

```
httpMain.c (top level)
├── httpSpawner.c (thread spawning)
├── httpConn.c (connection handling)
├── httpConf.c (configuration)
├── httpVhost.c (virtual hosting)
├── httpMime.c (MIME handling)
├── httpLog.c (logging)
├── httpMemsys.c (memory management)
├── httpThreads.c (threading)
├── httpLock.c (locking)
├── httpList.c (linked lists)
├── httpUtils.c (utilities)
├── httpProcessor.c (request routing)
├── httpRequest.c (request parsing)
├── httpResponse.c (response generation)
├── httpEngine.c (method dispatch)
├── httpHTML.c (HTML generation)
├── httpDirlister.c (directory listing)
├── httpCGI.c (CGI handling)
├── httpExecutor.c (process execution)
├── httpEnv.c (environment)
├── httpAdmin.c (admin commands)
├── httpController.c (admin client)
└── httpClient.c (client socket)
```

### 3.4 Include Dependencies

Each source file includes the headers it needs. The `httpInclude.h` header provides cross-module access to global variables.

## 4. Coding Standards

### 4.1 Header Guards

All header files use include guards:

```c
#ifndef HTTPMODULE_H
#define HTTPMODULE_H
// ... content ...
#endif
/* End of File */
```

### 4.2 Copyright Header

All source files begin with:

```c
/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */
```

### 4.3 Function Documentation

Each function has a comment block describing its purpose:

```c
/* hx_function_name
   Description of what the function does
*/
```

### 4.4 Variable Naming

- **Global variables**: Lowercase with underscores (e.g., `server_shutdown`, `g_num_clients_being_served`)
- **Local variables**: Lowercase (e.g., `retval`, `count`, `port`)
- **Constants**: Uppercase with underscores (e.g., `MAX_BUFF_LEN`, `OK`, `ERRC`)
- **Types**: `hx_` prefix with lowercase (e.g., `hx_allocator_t`, `hx_logsys_t`)
- **Structures**: `hx_` prefix (e.g., `hx_vhost_t`, `hx_config_t`)

### 4.5 Function Naming

All functions use the `hx_` prefix:
- `hx_create_allocator()` - Create allocator
- `hx_parse_request()` - Parse request
- `hx_process_http()` - Process HTTP connection

### 4.6 Code Style

- 4-space indentation
- Braces on new lines (K&R style for function bodies, Allman for blocks)
- Maximum line length: ~80 characters
- Comments above code blocks

### 4.7 Error Handling

Functions return `OK` (1) on success and `ERRC` (-1) on failure. Specific error codes are defined in the `hx_errcodes_t` enum.

```c
int retval = hx_some_function();
if(retval != OK) {
   // Handle error
}
```

### 4.8 Memory Management

- All memory is allocated through the custom allocator (`hx_alloc_mem()`)
- Memory is freed using `hx_free_mem()`
- Allocators are named for tracking (e.g., "Request Allocator", "Lock")
- Allocators are destroyed when no longer needed

### 4.9 Thread Safety

- Shared resources are protected by locks (`hx_lock()`/`hx_unlock()`)
- Lists have internal locks
- Log system has per-log locks
- Worker threads are isolated (each has its own allocator)

## 5. Key Data Structures

### 5.1 Request Object (`http_request_t`)

The central data structure for HTTP request processing. Contains:
- Socket and client address
- Parsed request components (line, headers, body, URI)
- Resource information (filename, mime type, stat)
- Virtual host association
- Allocator for request-specific memory

### 5.2 Response Object (`http_response_t`)

Contains the HTTP response to be sent:
- Status line and headers
- Body content
- Associated request object
- Allocator for response memory

### 5.3 Virtual Host (`hx_vhost_t`)

```c
typedef struct {
   unsigned char *host;       // Virtual host name
   unsigned char *docroot;    // Document root
   unsigned char *scriptroot; // CGI root
   unsigned char *errpage;    // Error page path
   hx_logsys_t   *log;        // Per-vhost log
} hx_vhost_t;
```

### 5.4 Configuration (`hx_config_t`)

```c
typedef struct {
   hx_allocator_t *a;
   unsigned char *filename;
   FILE *fptr;
   void **table;  // Indexed by HX_CONF_REC_IDX
} hx_config_t;
```

### 5.5 Log System (`hx_logsys_t`)

```c
typedef struct {
   hx_allocator_t    *allocator;
   unsigned char     *name;
   unsigned char     *logfile;
   FILE              *fp;
   hx_lock_t         *lock;
   // ... formatting buffers ...
   long              mask;
   int               needPrefix;
} hx_logsys_t;
```

### 5.6 MIME Table (`http_mime_table_t`)

```c
typedef struct {
   http_mime_t *table[MAX_MIME_TYPES];
   int numTypes;
} http_mime_table_t;
```

## 6. Function Call Conventions

### 6.1 Return Values

| Value | Meaning |
|-------|---------|
| OK (1) | Success |
| ERRC (-1) | General error |
| Specific error codes | Specific failure (see error codes) |

### 6.2 Parameter Conventions

- **Input parameters**: Passed by value or const pointer
- **Output parameters**: Passed by pointer
- **Log parameter**: First parameter in most functions (`hx_logsys_t *log`)
- **Allocator parameter**: Usually the last parameter (`hx_allocator_t *allocator`)

### 6.3 Logging Convention

Most functions begin with:
```c
const char *rname = "hx_function_name ()";
hx_log_msg(log, HX_LCALL, rname, "Called");
```

## 7. Code Flow

### 7.1 Server Startup

```
main()
  │
  ├─ hx_create_allocator("main")
  ├─ hx_create_log_sys()
  ├─ hx_sock_init()
  ├─ hx_load_config()
  ├─ hx_create_pid_file()
  ├─ hx_install_signal_handlers()
  ├─ Create locks
  ├─ hx_init_mime_table()
  ├─ hx_init_admin_iface()  → admin thread
  └─ hx_init_http_iface()   → HTTP worker threads
     └─ Wait for shutdown
```

### 7.2 Request Processing

```
hx_accept_http_conn() (worker thread)
  │
  ├─ accept() client connection
  ├─ hx_create_allocator("Request Allocator")
  ├─ hx_process_http()
  │   ├─ hx_createReqObj()
  │   ├─ hx_parse_request()
  │   ├─ hx_processRequest()
  │   │   ├─ Static: hx_GET() → hx_sendResponse()
  │   │   └─ Dynamic: hx_handle_cgi() → hx_sendResponse()
  │   └─ hx_destroy_allocator()
  ├─ hx_sockClose()
  └─ Loop: accept next connection
```

### 7.3 Virtual Host Resolution

```
Request with Host header
  │
  ├─ Extract hostname
  ├─ localhost? → defaultVhost
  └─ hx_getVhost(hostname)
       ├─ Found → Use vhost
       └─ Not found → defaultVhost + error log
```

## 8. Debugging

### 8.1 Logging

The server uses a multi-level logging system. Log levels can be controlled via the `LOG_LEVEL` directive in the config file.

### 8.2 Memory Debugging

When `HX_DEBUG_MEMSYS` is defined:
- All allocations are tracked with file/line information
- Memory leaks can be detected
- Reuse counts track how many times blocks are reused

### 8.3 Hex Dump

The `hx_hex_dump()` function can be used to dump buffer contents to the log at the `HX_LDUMP` level.

### 8.4 Signal Handling

The server handles:
- `SIGINT` - Normal shutdown
- `SIGUSR1` - Normal shutdown
- `SIGSEGV` - Abortive shutdown (for debugging crashes)

## 9. Testing

### 9.1 Test Files

The `tests/` directory contains:
- `testURLs.txt` - Test URLs for manual testing

### 9.2 Manual Testing

1. Start the server: `./httpdex`
2. Connect with a browser or curl: `http://localhost:80/`
3. Test virtual hosts: `http://www.example.com/`
4. Test CGI: `http://localhost/cgi-bin/test.cgi`
5. Test admin: Connect to port 10002 via telnet

### 9.3 Test Commands

```bash
# Test basic HTTP request
curl http://localhost:80/

# Test directory listing
curl http://localhost:80/testdir/

# Test admin command (reload config)
echo -n "0008reload_config" | nc localhost 10002
```

## 10. Contributing

### 10.1 Code Changes

1. Fork the repository
2. Create a feature branch
3. Make changes following coding standards
4. Test the changes
5. Submit a pull request

### 10.2 Bug Fixes

When reporting or fixing bugs, include:
- Steps to reproduce
- Expected behavior
- Actual behavior
- Patch or fix description

### 10.3 Documentation

When adding new features:
1. Update the API reference
2. Update the configuration documentation
3. Add examples where appropriate
4. Update the TODO list if applicable

## 11. Known Issues

| Issue | Description | Status |
|-------|-------------|--------|
| Linked list remove functions | `hx_remove()` and `hx_remove_first()` crash on single-node lists | Fixed |
| `hx_remove()` | `--list->entries` runs even when list is empty | Fixed |
| `hx_remove_first()` | `node->nxt->prv` dereferences NULL on single-node list | Fixed |
| POST/PUT/DELETE/TRACE/OPTIONS/CONNECT | Not implemented (return 501) | Open |
| HTTPS/TLS support | Not supported | Open |
| Connection keep-alive | Not supported | Open |
| HTTP/1.1 chunked encoding | Not supported | Open |

## 12. Future Enhancements

### 12.1 Planned Features

From the TODO list:
- [x] Memory pool allocator
- [x] Dynamic config loading
- [x] Config file loader module
- [ ] Generic hashtable implementation
- [x] Virtual host support
- [x] Code modularization
- [x] Case-insensitive matchIndex
- [x] HTTP/1.1 Host header check
- [ ] Access control
- [ ] Robust file ops module
- [x] Per-vhost logger
- [x] Document requirement analysis and design

### 12.2 Potential Enhancements

- HTTPS/TLS support
- HTTP/1.1 keep-alive connections
- Connection pooling
- Request filtering/access control
- Load balancing
- Caching
- WebSocket support
- REST API for administration
- Configuration validation schema
- Unit tests

## 13. Release History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2003 | Initial release |

## 14. License

Copyright (c) 2003 Avinandan Sengupta. All rights reserved.

This is experimental software. Use at your own risk.

---
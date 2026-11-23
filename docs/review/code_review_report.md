# httpdEx - Code Review: Vulnerabilities and Bugs Report

## 1. Executive Summary

This report documents the findings of a comprehensive security and bug scan of the httpdEx source code. The scan covered all source files in the `source/` directory.

## 2. Critical Bugs Fixed

### 2.1 Linked List Remove Functions (`source/httpList.c`)

**Bug in `hx_remove()`**:
- When removing from a single-node list, `node->prv` is NULL, and `node->prv->nxt = NULL` dereferences NULL → crash
- `--list->entries` ran even when the list was empty

**Bug in `hx_remove_first()`**:
- When removing from a single-node list, `node->nxt` is NULL, and `node->nxt->prv = NULL` dereferences NULL → crash

**Fix Applied**: Added NULL checks for `node->prv` and `node->nxt`, and only decrement entries when a node was actually removed.

## 3. Vulnerabilities

### 3.1 Buffer Overflow via Unsafe String Functions

Multiple locations use unsafe string functions (`strcpy`, `strcat`, `sprintf`) without bounds checking. These can lead to buffer overflows if input data exceeds expected buffer sizes.

**Affected files**:
- `source/httpRequest.c` - Multiple `strcpy`/`strcat` calls for URI parsing
- `source/httpProcessor.c` - Path construction with `strcat`
- `source/httpCGI.c` - Command line construction with `strcat`
- `source/httpExecutor.c` - Command line construction with `strcat`
- `source/httpDirlister.c` - Path and href construction with `strcat`
- `source/httpLog.c` - `vsprintf` and `strcat` for log formatting
- `source/httpUtils.c` - `strcpy` and `strcat` for string operations
- `source/httpEnv.c` - `strcpy` and `strcat` for environment block
- `source/memwatch.c` - `sprintf` and `strcat` for debug output

**Risk**: HIGH - An attacker controlling input data could cause buffer overflows leading to crashes or code execution.

**Mitigation**: Replace unsafe functions with bounded alternatives (e.g., `strncpy`, `strncat`, `snprintf`) and add explicit length checks.

### 3.2 Path Traversal via Directory Listing (`source/httpDirlister.c`)

The directory listing function constructs file paths using user-supplied input:

```c
strcat(fname, dirpath);
strcat(fname, (unsigned char *)hx_get_conf_val(log, IDX_PATH_SEPARATOR));
strcat(fname, entname);
```

The `entname` comes from `readdir()` which reads directory entries. However, if the directory listing is used to list paths outside the document root, an attacker could potentially access sensitive files.

**Risk**: MEDIUM - Path traversal if directory listing is not properly bounded to the document root.

**Mitigation**: Ensure all file access is chrooted or bounded to the document root. Validate that constructed paths resolve within the allowed directory tree.

### 3.3 Command Injection via CGI Execution (`source/httpExecutor.c`)

The CGI executor constructs command lines by concatenating interpreter paths and filenames:

```c
strcat(cmdLine, process->interpreter);
strcat(cmdLine, " ");
strcat(cmdLine, arg);
```

The interpreter path comes from the shebang line of the CGI script. If an attacker can upload a CGI script with a malicious shebang line (e.g., `#!/bin/sh -c 'rm -rf /'`), the server would execute arbitrary commands.

**Risk**: HIGH - Arbitrary command execution through CGI script shebang injection.

**Mitigation**: Validate and sanitize interpreter paths from shebang lines. Restrict CGI execution to a whitelist of allowed interpreters.

### 3.4 Unsafe CGI Environment Variables (`source/httpEnv.c`)

The environment block creation does not sanitize environment variable names or values:

```c
strcpy(pch, entry->nameval);
```

Environment variables come from HTTP request headers. An attacker could inject newlines or null bytes into environment variables, potentially affecting the child process.

**Risk**: MEDIUM - Environment variable injection could affect CGI execution.

**Mitigation**: Sanitize environment variable names and values before passing to child processes.

### 3.5 Unvalidated Virtual Host Input (`source/httpConf.c`)

The VHOST configuration parsing does not validate input fields:

```c
VHOST=hostname,docroot,scriptroot,logfile,[loglevels]
```

The hostname, docroot, scriptroot, and logfile fields are used directly without validation. An attacker with config file access could set arbitrary paths.

**Risk**: MEDIUM - Path traversal or arbitrary file access through malicious config.

**Mitigation**: Validate all VHOST fields. Ensure paths are absolute and within allowed directories.

### 3.6 Missing Input Validation for HTTP Headers (`source/httpRequest.c`)

HTTP headers are stored and logged without validation. The `hx_printHeaders` function logs header values directly:

```c
hx_log_msg(request_object->vhost->log, HX_LDEBG, rname,
           "   [%03d] %s: %s",
           i + 1,
           header_names_table[request_object->header[i]->header_idx],
           ((request_object->header[i]->value) ? ...
```

**Risk**: LOW - Log injection if header values contain newline characters.

**Mitigation**: Sanitize header values before logging. Strip or escape newline characters.

### 3.7 Memory Allocation Without Size Validation (`source/httpMemsys.c`)

The memory allocator does not validate allocation sizes:

```c
void *hx_alloc_mem(size_t n, hx_allocator_t *allocator)
```

An attacker could potentially trigger very large allocations through crafted requests, leading to denial of service.

**Risk**: MEDIUM - Memory exhaustion through allocation attacks.

**Mitigation**: Add maximum allocation size limits and validate allocation requests.

## 4. Bugs

### 4.1 Uninitialized Variables

Several functions use variables that may be uninitialized in certain code paths:

**`source/httpConf.c` - `hx_set_vhost_data()`**:
- `hostname`, `docroot`, `scriptroot`, `logfile`, `errpage` may be NULL if not provided
- The function checks `if(!hostname)` but doesn't check other fields before use

### 4.2 Missing NULL Checks

**`source/httpList.c`**:
- `hx_add()` does not check if `hx_createNode()` returns NULL before using the node
- `hx_add_first()` has the same issue

**Fix**: Add NULL checks after `hx_createNode()` calls.

### 4.3 Off-by-One in Buffer Sizes

**`source/httpUtils.c` - `hx_strdup()`**:
```c
result = (unsigned char *)hx_alloc_mem (strlen(source) + 1, allocator);
strcpy (result, source);
```
This is correct, but similar patterns in other files may have off-by-one errors.

### 4.4 Resource Leaks

**`source/httpLog.c` - `hx_create_log_sys()`**:
- If `fopen()` fails, the function destroys the log system but the allocator may not be properly cleaned up in all error paths

**`source/httpConf.c`**:
- Configuration reload may leak memory from previous configuration if not properly cleaned up

### 4.5 Thread Safety Issues

**`source/httpConf.c`**:
- The global `g_config` variable is accessed without synchronization in a multi-threaded environment
- Configuration reload could race with request processing

### 4.6 Missing Error Handling

**`source/httpSock.c`**:
- Socket functions may not handle all error conditions properly
- `hx_sock_read()` and `hx_sockWrite()` may not handle partial reads/writes correctly

### 4.7 CGI Process Cleanup

**`source/httpExecutor.c`**:
- The `hx_cleanProc()` function may not properly clean up all resources in error paths
- Child processes may become zombies if not properly waited on

## 5. Design Issues

### 5.1 No Access Control

The server has no access control mechanism. Any client can connect to any port and make any request.

**Impact**: Unauthorized access to sensitive resources.

### 5.2 No Rate Limiting

There is no rate limiting on connections or requests.

**Impact**: Denial of service through connection flooding.

### 5.3 No Input Size Limits

While some buffers have maximum sizes (e.g., `MAX_BUFF_LEN`), there are no overall limits on request body size or header count.

**Impact**: Memory exhaustion through large requests.

### 5.4 Cleartext Admin Protocol

The admin protocol is unencrypted and unauthenticated.

**Impact**: Anyone with network access can issue admin commands (reload_config, shutdown).

## 6. Recommendations

### 6.1 Immediate Fixes

1. **Fix linked list bugs** (already applied): Add NULL checks in `hx_remove()` and `hx_remove_first()`.

2. **Replace unsafe string functions**: Replace `strcpy`, `strcat`, `sprintf` with bounded alternatives throughout the codebase.

3. **Validate CGI interpreter paths**: Sanitize shebang line interpreter paths before execution.

4. **Add input validation**: Validate all user-supplied input (headers, URIs, query strings).

### 6.2 Short-term Fixes

1. **Add path traversal protection**: Ensure all file access is bounded to the document root.

2. **Add rate limiting**: Implement connection and request rate limiting.

3. **Add authentication to admin interface**: Require authentication for admin commands.

4. **Add maximum request size limits**: Limit request body and header sizes.

### 6.3 Long-term Improvements

1. **Implement HTTPS/TLS**: Add encrypted communication support.

2. **Implement access control**: Add IP-based or authentication-based access control.

3. **Add request logging audit**: Implement detailed request logging for security auditing.

4. **Implement memory limits**: Add per-connection and global memory limits.

## 7. Severity Summary

| Severity | Count | Description |
|----------|-------|-------------|
| Critical | 2 | Linked list NULL dereference crashes (fixed) |
| High | 2 | Buffer overflow via unsafe string functions, CGI command injection |
| Medium | 4 | Path traversal, env var injection, config validation, memory exhaustion |
| Low | 2 | Log injection, missing error handling |
| Info | 4 | Design issues (no access control, no rate limiting, no input limits, cleartext admin) |

## 8. Files Reviewed

| File | Issues Found |
|------|-------------|
| `source/httpList.c` | 2 critical (fixed), 2 medium |
| `source/httpRequest.c` | 3 medium, 2 low |
| `source/httpProcessor.c` | 2 medium, 1 low |
| `source/httpCGI.c` | 2 high, 2 medium |
| `source/httpExecutor.c` | 2 high, 2 medium |
| `source/httpDirlister.c` | 2 medium |
| `source/httpLog.c` | 2 medium, 1 low |
| `source/httpUtils.c` | 2 medium |
| `source/httpEnv.c` | 2 medium |
| `source/httpConf.c` | 2 medium, 1 low |
| `source/httpMime.c` | 1 medium |
| `source/httpAdmin.c` | 1 medium |
| `source/httpMemsys.c` | 1 medium |
| `source/memwatch.c` | 1 low |

## 9. Conclusion

The httpdEx codebase has several vulnerabilities that should be addressed before production use. The most critical issues are:

1. **Buffer overflows** via unsafe string functions throughout the codebase
2. **CGI command injection** through unsanitized shebang line interpreter paths
3. **Path traversal** risks in directory listing and file access
4. **Missing access control** and authentication

The linked list bugs have been fixed. All other issues require code changes to input validation, string handling, and security controls.

---
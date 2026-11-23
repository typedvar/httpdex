# httpdEx - Troubleshooting Guide

## 1. Introduction

This guide helps diagnose and resolve common issues with the httpdEx server.

## 2. Server Startup Issues

### 2.1 Server Fails to Start

**Symptoms**: Server process exits immediately after starting.

**Possible Causes**:
1. Configuration file not found
2. Log directory missing
3. MIME data file not found
4. Socket initialization failure
5. Permission issues

**Solutions**:
1. Check config file path: The server looks for `../conf/httpdex.conf` relative to the executable
2. Create the log directory: `mkdir -p ../logs/`
3. Verify MIME file exists: `conf/mime.data` must be present
4. Check that the port is not already in use
5. Ensure proper file permissions

**Diagnosis**: Check the log file `../logs/httpdex.log` for error messages.

### 2.2 "Failed to create log system" Error

**Symptoms**: Server logs "Failed to create log system" and exits.

**Cause**: The log directory `../logs/` does not exist or is not writable.

**Solution**:
```bash
mkdir -p ../logs
touch ../logs/httpdex.log
chmod 644 ../logs/httpdex.log
```

### 2.3 "Error loading config data" Error

**Symptoms**: Server logs "Error loading config data" and exits.

**Cause**: The config file `../conf/httpdex.conf` is missing or has syntax errors.

**Solution**:
1. Verify the config file exists at the expected path
2. Check for syntax errors in the config file
3. Ensure forward slashes are used in paths

### 2.4 "Mime data Load failed" Error

**Symptoms**: Server logs "Mime data Load failed" and exits.

**Cause**: The MIME data file `mime.data` is missing from the config root.

**Solution**:
1. Verify `conf/mime.data` exists
2. Check the `MIME_FILE_NAME` directive in the config
3. Ensure the file is readable

### 2.5 Port Already in Use

**Symptoms**: Server fails to bind to the HTTP port.

**Cause**: Another process is using port 80 (or the configured port).

**Solution**:
1. Check for existing processes: `netstat -an | grep :80`
2. Kill the existing process or change the port in the config
3. For admin port conflicts, check port 10002

## 3. Request Processing Issues

### 3.1 404 Not Found

**Symptoms**: Requests return 404 for existing files.

**Possible Causes**:
1. Document root not configured correctly
2. File path issues (forward vs backward slashes)
3. Virtual host not found, falling back to default

**Solutions**:
1. Verify `DOCUMENT_ROOT` is set correctly
2. Check virtual host configuration if using vhosts
3. Ensure paths use forward slashes

### 3.2 Directory Listing Instead of File

**Symptoms**: Requesting a file returns a directory listing instead.

**Possible Causes**:
1. Index file not found for directory requests
2. Resource is actually a directory

**Solutions**:
1. Ensure index files (index.html, index.htm) exist in the directory
2. Verify the requested path is a file, not a directory

### 3.3 CGI Not Executing

**Symptoms**: CGI scripts return source code instead of executing.

**Possible Causes**:
1. Script not in the CGI root
2. Script lacks execute permissions (UNIX)
3. Interpreter not detected correctly

**Solutions**:
1. Verify the script is under `CGI_ROOT`
2. Set execute permissions: `chmod +x script.cgi`
3. Check shebang line (`#!/path/to/interpreter`)
4. Verify interpreter is in PATH

### 3.4 MIME Type Not Resolved

**Symptoms**: Files served with `text/plain` instead of correct MIME type.

**Possible Causes**:
1. MIME data file not loaded
2. Extension not in MIME table
3. File extension case mismatch

**Solutions**:
1. Verify `MIME_FILE_NAME` points to correct file
2. Check extension in `mime.data`
3. Extensions are case-sensitive in the current implementation

## 4. Virtual Host Issues

### 4.1 Virtual Host Not Working

**Symptoms**: Requests to a virtual host hostname fall back to default.

**Possible Causes**:
1. Host header not sent by client (HTTP/1.0)
2. VHOST syntax error in config
3. Hostname mismatch (case sensitivity)

**Solutions**:
1. Ensure client sends Host header (HTTP/1.1+)
2. Verify VHOST format: `hostname,docroot,scriptroot,logfile,[loglevels]`
3. Hostname matching is case-insensitive

### 4.2 Multiple Virtual Hosts

**Symptoms**: Only one virtual host works.

**Possible Causes**:
1. VHOST directives not parsed correctly
2. Vhost list not populated

**Solutions**:
1. Verify each VHOST line is properly formatted
2. Check that all vhosts are added to the vhost system

## 5. Admin Interface Issues

### 5.1 Admin Connection Refused

**Symptoms**: Cannot connect to admin port (10002).

**Possible Causes**:
1. Admin thread not started
2. Firewall blocking port
3. Port already in use

**Solutions**:
1. Verify `ADMIN_THREADS` > 0 in config
2. Check firewall rules
3. Check for port conflicts

### 5.2 Admin Commands Not Working

**Symptoms**: Admin commands (reload_config, shutdown) have no effect.

**Possible Causes**:
1. Invalid command format
2. Wrong message length prefix

**Solution**:
The admin protocol format is:
```
<4-digit length><command>
```

Example: `0008reload_config`

### 5.3 Reload Config Fails

**Symptoms**: Configuration changes not applied after reload.

**Possible Causes**:
1. Config file has syntax errors
2. Reload fails silently

**Solutions**:
1. Verify config file is valid
2. Check logs for reload errors
3. Try restarting the server instead

## 6. Logging Issues

### 6.1 No Log Output

**Symptoms**: Log file is empty or not created.

**Possible Causes**:
1. Log level mask disabled all levels
2. Log file not writable
3. Log directory missing

**Solutions**:
1. Check `LOG_LEVEL` directive
2. Verify log file permissions
3. Create log directory

### 6.2 Log File Too Large

**Symptoms**: Log file grows uncontrollably.

**Solution**:
1. Use log rotation (external tool)
2. Reduce log verbosity
3. Restart server periodically to reopen log file

## 7. Memory Issues

### 7.1 Out of Memory

**Symptoms**: Server crashes with memory allocation errors.

**Possible Causes**:
1. Memory leak in request processing
2. Too many concurrent connections
3. Large request bodies

**Solutions**:
1. Check for memory leaks with `HX_DEBUG_MEMSYS`
2. Reduce `SERVER_THREADS` or `ADMIN_THREADS`
3. Limit request body size

### 7.2 Memory Debugging

To enable memory debugging:
1. Define `HX_DEBUG_MEMSYS` at compile time
2. Rebuild the server
3. Monitor allocations in the log

## 8. Threading Issues

### 8.1 Thread Exhaustion

**Symptoms**: Server stops accepting connections.

**Possible Causes**:
1. All worker threads busy
2. Thread creation failure

**Solutions**:
1. Increase `SERVER_THREADS`
2. Check system thread limits
3. Monitor active connections

### 8.2 Deadlock

**Symptoms**: Server hangs.

**Possible Causes**:
1. Lock not released
2. Thread waiting indefinitely

**Solutions**:
1. Check lock acquisition/release pairs
2. Review thread wait logic

## 9. Platform-Specific Issues

### 9.1 Windows

**Issue**: Socket errors (WSAExxx)

**Solution**: Check Winsock error codes in `httpSock.c` error table.

**Issue**: Path separators

**Solution**: Always use forward slashes (`/`) in config files.

### 9.2 UNIX/Linux

**Issue**: Permission denied on files

**Solution**: Check file and directory permissions.

**Issue**: Thread creation failure

**Solution**: Check pthreads library and thread limits.

## 10. Debugging Steps

### 10.1 Enable Debug Logging

Set the log level to include debug:
```
LOG_LEVEL=default,reqs,call,verb,dump,resp,debg
```

### 10.2 Check Log Files

```bash
# View server log
tail -f ../logs/httpdex.log

# Check for errors
grep "ERRO" ../logs/httpdex.log
grep "WARN" ../logs/httpdex.log
```

### 10.3 Test with curl

```bash
# Test basic request
curl -v http://localhost:80/

# Test with headers
curl -v -H "Host: www.example.com" http://localhost:80/

# Test admin command
echo -n "0008reload_config" | nc localhost 10002
```

### 10.4 Check Process Status

```bash
# Check if server is running
ps aux | grep httpdex

# Check listening ports
netstat -an | grep -E ':80|:10002'
```

### 10.5 Memory Debugging

Rebuild with `HX_DEBUG_MEMSYS` and monitor:
```bash
# Look for allocation errors in log
grep "MEM_ALLOC" ../logs/httpdex.log
```

## 11. Common Error Messages

| Message | Cause | Solution |
|---------|-------|----------|
| "Failed to create log system" | Log directory missing | Create `../logs/` |
| "Error loading config data" | Config file missing/invalid | Check config path and syntax |
| "Mime data Load failed" | MIME file missing | Check `MIME_FILE_NAME` path |
| "PID file could not be opened" | Permission issue | Check write permissions |
| "Virtual host lookup failed" | Vhost not configured | Check VHOST directives |
| "Resource does not have execution permission" | CGI script not executable | `chmod +x` |
| "accept() failed" | Socket issue | Check port availability |

## 12. Emergency Procedures

### 12.1 Server Won't Start

1. Check config file syntax
2. Verify all required files exist
3. Check log directory permissions
4. Try running with minimal config

### 12.2 Server Hangs

1. Check logs for stuck requests
2. Send SIGUSR1 to initiate shutdown
3. If unresponsive, send SIGKILL
4. Check for deadlocks in threading code

### 12.3 Memory Exhaustion

1. Restart the server to clear memory
2. Enable memory debugging
3. Monitor allocations
4. Fix memory leaks

## 13. Log Analysis

### 13.1 Log Levels

```
erro  - Errors (always logged)
warn  - Warnings (always logged)
verb  - Verbose information
debg  - Debug messages
mime  - MIME table data
call  - Function call tracing
reqs  - Request details
resp  - Response details
dump  - Hex dumps
syst  - System messages (always logged)
```

### 13.2 Common Log Patterns

| Pattern | Meaning |
|---------|---------|
| "ONLINE -> Service Interface" | HTTP server started |
| "ONLINE -> Admin Interface" | Admin server started |
| "OFFLINE -> Service Interface" | HTTP server stopped |
| "OFFLINE -> Admin Interface" | Admin server stopped |
| "Shutdown initiated" | Signal received |
| "Request START"/"Request END" | Request processing |
| "MIME TABLE START"/"MIME TABLE END" | MIME table loaded |

## 14. Maintenance

### 14.1 Log Rotation

The server does not implement log rotation. Use external tools:
```bash
# Example: rename and restart
mv ../logs/httpdex.log ../logs/httpdex.log.old
# Send SIGHUP or restart server
```

### 14.2 Configuration Changes

To change configuration without restart:
1. Edit `conf/httpdex.conf`
2. Send `reload_config` via admin interface
3. Verify changes in logs

### 14.3 Server Restart

```bash
# Graceful shutdown
kill -USR1 <pid>

# Or via admin
echo -n "0008shutdown" | nc localhost 10002

# Start again
./hdstart
```

## 15. Getting Help

When reporting issues, include:
1. OS and architecture
2. Build method (make/MSVC/Cygwin)
3. Config file contents
4. Log output (especially errors)
5. Steps to reproduce
6. Expected vs actual behavior

---
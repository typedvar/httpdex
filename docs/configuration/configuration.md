# httpdEx - Configuration Documentation

## 1. Introduction

httpdEx uses a text-based configuration file that is loaded at startup and can be reloaded dynamically via the admin interface. The configuration controls server behavior, virtual hosting, logging, and MIME type resolution.

## 2. Configuration Files

### 2.1 Primary Configuration File

**File**: `conf/httpdex.conf`

This is the main configuration file containing server-wide settings and virtual host definitions.

**Default path**: `../conf/httpdex.conf` (relative to the server executable)

The path can be overridden via the `_HX_CONFIG_FILE_` macro at compile time.

### 2.2 XML Configuration File

**File**: `conf/httpdex-conf.xml`

An XML-based alternative configuration format. This is not currently used by the server but is provided as a reference.

### 2.3 MIME Data File

**File**: `conf/mime.data`

Contains MIME type definitions mapping file extensions to MIME types/subtypes.

**Default path**: Resides in the config root directory.

## 3. Configuration File Format

### 3.1 General Format

```
# Comments start with #
DIRECTIVE=value

# VHOST format:
VHOST=hostname,docroot,scriptroot,logfile,[loglevels]
```

### 3.2 Rules

- Forward slashes should be used for paths
- Filenames are case-sensitive
- Comments start with `#`
- VHOST parameters are comma-separated
- Log levels for VHOST are optional

## 4. Server-Wide Directives

### 4.1 Thread Settings

| Directive | Description | Default | Example |
|-----------|-------------|---------|---------|
| `SERVER_THREADS` | Number of HTTP worker threads | - | `SERVER_THREADS=10` |
| `ADMIN_THREADS` | Number of admin worker threads | - | `ADMIN_THREADS=2` |

### 4.2 Path Settings

| Directive | Description | Default | Example |
|-----------|-------------|---------|---------|
| `DOCUMENT_ROOT` | Default document root | - | `DOCUMENT_ROOT=/var/www` |
| `CGI_ROOT` | Default CGI script root | - | `CGI_ROOT=/var/www/cgi-bin` |
| `CONFIG_ROOT` | Configuration directory | - | `CONFIG_ROOT=../conf` |

### 4.3 Network Settings

| Directive | Description | Default | Example |
|-----------|-------------|---------|---------|
| `DEFAULT_HTTP_PORT` | HTTP service port | - | `DEFAULT_HTTP_PORT=80` |
| `HTTP_SERVICE_PORT` | HTTP service port | - | `HTTP_SERVICE_PORT=80` |
| `ADMIN_PORT` | Administration port | - | `ADMIN_PORT=10002` |
| `DEFAULT_HOST_NAME` | Default hostname | - | `DEFAULT_HOST_NAME=localhost` |

### 4.4 Other Settings

| Directive | Description | Default | Example |
|-----------|-------------|---------|---------|
| `SERVER_TIMEOUT` | Server timeout (seconds) | - | `SERVER_TIMEOUT=300` |
| `DEFAULT_ERR_PAGE` | Default error page | - | `DEFAULT_ERR_PAGE=error.htm` |
| `MIME_FILE_NAME` | MIME data filename | - | `MIME_FILE_NAME=mime.data` |
| `HTTP_VERSION` | HTTP version string | - | `HTTP_VERSION=HTTP/1.1` |
| `PATH_SEPARATOR` | Path separator | - | `PATH_SEPARATOR=/` |
| `INDEX_FILES` | Comma-separated index files | - | `INDEX_FILES=index.html,index.htm` |
| `LOG_LEVEL` | Comma-separated log levels | - | `LOG_LEVEL=default,reqs,verb` |

## 5. Virtual Host Directives

### 5.1 VHOST Format

```
VHOST=hostname,docroot,scriptroot,logfile,[loglevels]
```

| Parameter | Description | Required |
|-----------|-------------|----------|
| hostname | Virtual host name | Yes |
| docroot | Document root path | Yes |
| scriptroot | CGI script root path | Yes |
| logfile | Service log file path | Yes |
| loglevels | Comma-separated log levels | No |

### 5.2 Example

```
VHOST=www.example.com,C:/www/example,scripts,C:/logs/example.log,default,reqs,verb
VHOST=www.test.com,C:/www/test,scripts,C:/logs/test.log
```

### 5.3 VHOST Parameter Indices

| Index | Name |
|-------|------|
| VHOSTPARM_HOSTNAME | Hostname |
| VHOSTPARM_DOCROOT | Document root |
| VHOSTPARM_SCRIPTROOT | Script root |
| VHOSTPARM_LOGFILE | Log file |
| VHOSTPARM_ERRPAGE | Error page |
| MAX_VHOST_PARMS | Maximum parameters (5) |

## 6. Log Levels

### 6.1 Available Log Levels

| Level | String | Description |
|-------|--------|-------------|
| erro | HX_LERRO | Errors (always on) |
| warn | HX_LWARN | Warnings (always on) |
| verb | HX_LVERB | Verbose information |
| debg | HX_LDEBG | Debug messages |
| mime | HX_LDMIM | MIME table data |
| call | HX_LCALL | Function call tracing |
| reqs | HX_LREQS | Request details |
| resp | HX_LRESP | Response details |
| dump | HX_LDUMP | Hex dumps |
| syst | HX_LSYST | System messages (always on) |

### 6.2 Default Log Level

When no log level is specified, the `default` level is used which enables all standard levels.

### 6.3 Example

```
# Enable all standard logging
LOG_LEVEL=default,reqs,call,verb,dump,resp

# Enable only errors and warnings
LOG_LEVEL=erro,warn
```

## 7. XML Configuration Format

The `httpdex-conf.xml` file provides an XML-based alternative:

```xml
<httpdex-config>
	<srvthreads>10</srvthreads>
	<admthreads>1</admthreads>
	<sysroot>D:/Personal_Workspace/httpd/bin</sysroot>
	<docroot>E:/httpdex</docroot>
	<srvport>9001</srvport>
	<admport>9099</admport>
	<vhost>
		<hostname>www.hola.com</hostname>
		<docroot>E:/WEBROOT/vhosts/hola</docroot>
		<srvlog>E:/WEBROOT/vhosts/hola/log/service.log</srvlog>
		<errlog>E:/WEBROOT/vhosts/hola/log/error.log</errlog>
	</vhost>
	<vhost>
		<hostname>www.jupiter.com</hostname>
		<docroot>E:/WEBROOT/vhosts/jupiter</docroot>
		<srvlog>E:/WEBROOT/vhosts/jupiter/log/service.log</srvlog>
		<errlog>E:/WEBROOT/vhosts/jupiter/log/error.log</errlog>
	</vhost>
</httpdex-config>
```

**Note**: The XML format is not currently used by the server. The text-based `httpdex.conf` is the active format.

## 8. MIME Data Format

The `mime.data` file maps MIME types to file extensions:

```
# MIME type        Extension
text/html          html htm
image/jpeg         jpeg jpg jpe
image/png          png
application/pdf    pdf
text/plain         asc txt
application/octet-stream bin dms lha lzh exe class so dll
```

**Format**:
- Lines starting with `#` are comments
- Each line: MIME type followed by whitespace-separated extensions
- Extensions can be multiple, separated by spaces

## 9. Configuration Loading Process

### 9.1 Startup Loading

1. Server starts and calls `hx_load_config()`
2. Opens the config file (`_HX_CONFIG_FILE_`)
3. Reads each line and matches against the command table
4. For each matching directive, calls the handler function
5. VHOST directives are parsed by `hx_set_vhost_data()`
6. Virtual hosts are added to the vhost system

### 9.2 Command Table

The server uses a command table to map directives to handler functions:

```c
static const hx_cmd_t cmdTable[] = {
   { "SERVER_THREADS"   , hx_set_num_http_threads   },
   { "ADMIN_THREADS"    , hx_set_num_admin_threads  },
   { "DOCUMENT_ROOT"    , hx_set_doc_root           },
   { "CGI_ROOT"         , hx_set_cgi_root           },
   { "SERVER_TIMEOUT"   , hx_set_server_timeout     },
   { "DEFAULT_HTTP_PORT", hx_set_default_http_port  },
   { "DEFAULT_HOST_NAME", hx_set_host_name          },
   { "DEFAULT_ERR_PAGE" , hx_set_default_error_page },
   { "CONFIG_ROOT"      , hx_set_config_root        },
   { "MIME_FILE_NAME"   , hx_set_mime_file          },
   { "HTTP_VERSION"     , hx_set_http_version       },
   { "PATH_SEPARATOR"   , hx_set_path_separator     },
   { "INDEX_FILES"      , hx_set_index_files        },
   { "ADMIN_PORT"       , hx_set_admin_port         },
   { "HTTP_SERVICE_PORT", hx_set_http_port          },
   { "LOG_LEVEL"        , hx_set_log_level_config   },
   { "VHOST"            , hx_set_vhost_data         },
   { NULL               , NULL                      }
};
```

### 9.3 Dynamic Reload

Configuration can be reloaded at runtime via the admin interface:

1. Admin client connects to the admin port
2. Sends the `reload_config` command
3. Server calls `hx_reload_config()` which:
   - Re-reads the config file
   - Clears existing configuration
   - Re-applies all settings
   - Rebuilds the vhost system

## 10. Configuration Validation

The server performs validation on configuration values:

| Directive | Validation |
|-----------|------------|
| SERVER_THREADS | Must be > 0 |
| ADMIN_THREADS | Must be > 0 |
| SERVER_TIMEOUT | Must be > 0 |
| DEFAULT_HTTP_PORT | Must be > 0 |
| ADMIN_PORT | Must be > 0 |
| HTTP_SERVICE_PORT | Must be > 0 |
| DOCUMENT_ROOT | Must not be empty |
| DEFAULT_ERR_PAGE | Must not be empty |
| LOG_LEVEL | Each level must be valid |

## 11. Example Configuration

### 11.1 Minimal Configuration

```
SERVER_THREADS=10
ADMIN_THREADS=2
DOCUMENT_ROOT=/var/www
CGI_ROOT=/var/www/cgi-bin
DEFAULT_HTTP_PORT=80
DEFAULT_HOST_NAME=localhost
DEFAULT_ERR_PAGE=error.htm
CONFIG_ROOT=../conf
MIME_FILE_NAME=mime.data
HTTP_VERSION=HTTP/1.1
PATH_SEPARATOR=/
INDEX_FILES=index.html,index.htm
ADMIN_PORT=10002
HTTP_SERVICE_PORT=80
SERVER_TIMEOUT=300
LOG_LEVEL=default,reqs,call,verb,dump,resp
```

### 11.2 Configuration with Virtual Hosts

```
SERVER_THREADS=10
ADMIN_THREADS=2
DOCUMENT_ROOT=/var/www
CGI_ROOT=/var/www/cgi-bin
DEFAULT_HTTP_PORT=80
DEFAULT_HOST_NAME=localhost
DEFAULT_ERR_PAGE=error.htm
CONFIG_ROOT=../conf
MIME_FILE_NAME=mime.data
HTTP_VERSION=HTTP/1.1
PATH_SEPARATOR=/
INDEX_FILES=index.html,index.htm
ADMIN_PORT=10002
HTTP_SERVICE_PORT=80
SERVER_TIMEOUT=300
LOG_LEVEL=default,reqs,call,verb,dump,resp

# Virtual host for www.example.com
VHOST=www.example.com,/var/www/example,/var/www/example/cgi-bin,/var/log/example.log,default,reqs,verb

# Virtual host for www.test.com
VHOST=www.test.com,/var/www/test,/var/www/test/cgi-bin,/var/log/test.log
```

## 12. Log File Location

The main server log file is specified in `httpMain.c`:
```c
server_log_sys = hx_create_log_sys(NULL, "../logs/httpdex.log", "HTTPDEX MAIN", HX_LOGPREFIX_YES);
```

The log directory (`../logs/`) must exist or the server will fail to start.

## 13. PID File

The server creates a PID file at startup:
- **Filename**: `httpdEx.pid`
- **Location**: Current working directory
- **Purpose**: Stores the server process ID for management scripts

## 14. Configuration File Locations

| File | Purpose | Default Location |
|------|---------|------------------|
| httpdex.conf | Main server configuration | `conf/httpdex.conf` |
| httpdex-conf.xml | XML configuration (unused) | `conf/httpdex-conf.xml` |
| mime.data | MIME type definitions | `conf/mime.data` |
| httpdex.log | Server log file | `../logs/httpdex.log` |
| httpdEx.pid | PID file | Current directory |

## 15. Environment Variables

The server does not use environment variables for configuration. All settings are read from the configuration file.

## 16. Platform-Specific Notes

### 16.1 Windows

- Paths use forward slashes (`/`) even on Windows
- The `PATH_SEPARATOR` directive should be `/`
- Socket errors use Winsock error codes (WSAExxx)

### 16.2 UNIX/Linux

- Paths use forward slashes (`/`)
- Socket errors use standard errno values
- Threads use pthreads

## 17. Configuration Troubleshooting

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| Server fails to start | Config file not found | Check `_HX_CONFIG_FILE_` path |
| Server fails to start | Log directory missing | Create `../logs/` directory |
| Virtual host not working | VHOST syntax error | Check comma-separated format |
| MIME types not resolved | MIME file not found | Check `MIME_FILE_NAME` path |
| Config reload fails | Config file locked | Ensure file is accessible |
| Log levels not working | Invalid level name | Check level string against table |

## 18. Security Considerations

- Configuration files should have restrictive permissions
- The admin port should be firewalled or bound to localhost
- Document root should be outside the web-accessible area
- Log files should be rotated regularly
- The `CONFIG_ROOT` should not be accessible via HTTP

## 19. Migration Notes

When upgrading from a previous version:

1. **Config format**: The text-based format has remained stable
2. **MIME file**: The `mime.data` format is RFC 2045-48 compliant
3. **VHOST format**: The comma-separated format is stable
4. **Log levels**: Level names are consistent across versions

---
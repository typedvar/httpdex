# httpdEx - Project Overview

## 1. Introduction

httpdEx (experimental HTTP daemon) is a lightweight, modular HTTP server developed in C. It is designed to serve static and dynamic content over HTTP/1.1, with support for virtual hosting, CGI execution, MIME type resolution, and a multi-threaded request processing model.

The server is built with a modular architecture consisting of distinct subsystems:
- **Configuration**: Dynamic loading and reloading of server and virtual host settings
- **Memory Management**: Custom memory pool allocator for controlled allocations
- **Logging**: Multi-level logging system with configurable log masks
- **Networking**: Socket abstraction for TCP connections
- **Threading**: Cross-platform thread management for worker and admin threads
- **Request Processing**: HTTP request parsing, routing, and response generation
- **Virtual Hosting**: Support for multiple virtual hosts with per-host configuration
- **CGI Support**: Dynamic content execution via CGI scripts
- **MIME Handling**: MIME type lookup and resolution
- **Administration**: Remote administration interface for configuration reload and shutdown

## 2. Features

- HTTP/1.1 protocol support
- Multi-threaded server with configurable worker and admin threads
- Virtual hosting with per-vhost document roots, CGI roots, and log files
- Dynamic configuration reload without server restart
- CGI script execution with environment variable support
- MIME type resolution from external mime.data file
- Directory listing for static content
- Remote administration interface (telnet-based)
- Graceful and abortive shutdown via signals (SIGINT, SIGUSR1, SIGSEGV)
- Custom memory pool allocator with optional debugging
- Structured logging with multiple log levels
- Cross-platform support (Windows and UNIX)

## 3. Architecture

The server follows a layered architecture:

```
┌─────────────────────────────────────────┐
│           Administration Interface       │
│  (telnet-based, config reload, shutdown) │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│           HTTP Service Interface         │
│  (worker threads, request processing)    │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│         Request Processing Core          │
│  (parse → route → process → respond)     │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│         Virtual Host System              │
│  (per-vhost config, routing)             │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│         Configuration System             │
│  (dynamic load/reload, validation)       │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│         Memory Management System         │
│  (custom allocator, pools)               │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│         Logging System                   │
│  (multi-level, configurable)             │
└─────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────┐
│         Networking (Sockets)             │
│  (TCP server/client, Winsock/BSD)        │
└─────────────────────────────────────────┘
```

## 4. Build System

The project can be built on:
- **Linux/UNIX**: Using `make` with `makefile.lnx`
- **Windows/Cygwin**: Using `make` with `makefile.cygwin`
- **Windows/MSVC**: Using Visual Studio solution (`httpdex.sln`) and project files (`httpd.vcproj`, `httpc.vcproj`)

Build targets:
- `httpdex` - Main HTTP daemon server
- `httpc` - HTTP administration client

## 5. Project Structure

```
httpdex/
├── source/          # Source code files (.c)
├── header/          # Header files (.h)
├── conf/            # Configuration files
│   ├── httpdex.conf     # Text-based config
│   ├── httpdex-conf.xml # XML-based config
│   └── mime.data        # MIME type definitions
├── docs/            # Documentation (this folder)
├── webroot/         # Default document root
│   ├── index.htm      # Default home page
│   ├── error.gif      # Error icon
│   ├── logo.gif       # Server logo
│   └── vhosts/        # Virtual host directories
├── resources/       # Resource files
│   ├── logo.gif       # Server logo
│   └── logo.jpg       # Server logo
├── scripts/         # Shell scripts
│   ├── hdstart        # Server start script
│   ├── hdstop         # Server stop script
│   └── hdlog          # Log viewer script
├── tests/           # Test files
│   └── testURLs.txt   # Test URLs
├── build/           # Build files
│   ├── makefile.lnx   # Linux makefile
│   ├── makefile.cygwin # Cygwin makefile
│   ├── httpdex.dsw    # Visual Studio workspace
│   └── httpdex.sln    # Visual Studio solution
└── httpd/           # MSVC project for httpd
    └── httpd.vcproj
└── httpc/           # MSVC project for httpc
    └── httpc.vcproj
```

## 6. License

Copyright (c) 2003 Avinandan Sengupta. All rights reserved.

This is experimental software. Use at your own risk.

---
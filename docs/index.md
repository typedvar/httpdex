# httpdEx - Documentation Index

This is the master index for the httpdEx documentation tree. All documentation is organized in the OKF (Open Knowledge Framework) structure.

## 1. Documentation Structure

```
docs/
├── overview.md                 # Project overview
├── architecture/
│   └── architecture.md         # System architecture
├── api/
│   └── api.md                  # API reference
├── configuration/
│   └── configuration.md        # Configuration guide
├── development/
│   └── development.md          # Development guide
├── troubleshooting/
│   └── troubleshooting.md      # Troubleshooting guide
├── reference/
│   └── reference.md            # Reference documentation
└── guides/
    └── (guides go here)
```

## 2. Quick Navigation

### For Users
- [Overview](./overview.md) - Project introduction and features
- [Configuration](./configuration/configuration.md) - How to configure the server
- [Troubleshooting](./troubleshooting/troubleshooting.md) - Common issues and solutions

### For Developers
- [Architecture](./architecture/architecture.md) - System architecture and design
- [API Reference](./api/api.md) - Complete API documentation
- [Development](./development/development.md) - Build process and coding standards
- [Reference](./reference/reference.md) - Data structures, constants, and reference

## 3. Documentation by Topic

| Topic | Document |
|-------|----------|
| Project overview | `docs/overview.md` |
| System architecture | `docs/architecture/architecture.md` |
| API reference | `docs/api/api.md` |
| Configuration | `docs/configuration/configuration.md` |
| Development guide | `docs/development/development.md` |
| Troubleshooting | `docs/troubleshooting/troubleshooting.md` |
| Reference | `docs/reference/reference.md` |

## 4. Key Concepts

### 4.1 Architecture

The server follows a modular, layered architecture with distinct subsystems:
- Core subsystems (memory, logging, networking, threading)
- Request processing (connection, processor, request, response)
- Virtual hosting
- Dynamic content (CGI)
- MIME handling
- Administration

### 4.2 Configuration

Configuration is loaded from a text-based config file at startup and can be reloaded dynamically via the admin interface. Virtual hosts are defined using the `VHOST` directive.

### 4.3 Request Processing

HTTP requests are parsed, routed to the appropriate handler (static file or CGI), and a response is generated and sent to the client.

### 4.4 Memory Management

The server uses a custom memory pool allocator instead of standard malloc/free for deterministic memory management and debugging support.

### 4.5 Logging

A multi-level logging system with configurable log levels provides structured logging with timestamps and function prefixes.

## 5. Code Bugs Fixed

During documentation review, the following bugs were identified and fixed:

### 5.1 Linked List Remove Functions (`source/httpList.c`)

**Bug in `hx_remove()`**:
- When removing from a single-node list, `node->prv` is NULL, and `node->prv->nxt = NULL` dereferences NULL → crash
- `--list->entries` ran even when the list was empty

**Fix**: Added NULL checks for `node->prv` and `node->nxt`, and only decrement entries when a node was actually removed.

**Bug in `hx_remove_first()`**:
- When removing from a single-node list, `node->nxt` is NULL, and `node->nxt->prv = NULL` dereferences NULL → crash

**Fix**: Added NULL checks for `node->nxt` and `node->prv`, and only decrement entries when a node was actually removed.

## 6. Contributing

To contribute to this documentation:

1. Fork the repository
2. Create a branch for your changes
3. Make documentation improvements
4. Submit a pull request

## 7. License

Copyright (c) 2003 Avinandan Sengupta. All rights reserved.

This documentation describes experimental software. Use at your own risk.

---
/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPEXECUTOR_H
#define HTTPEXECUTOR_H

#include <sys/types.h>

#include "httpInclude.h"

/* std stream nos */
typedef enum
{
   HX_STDIN = 0,
   HX_STDOUT = 1,
   HX_STDERR = 2
}hx_stdstr_t;

/* script type */
typedef enum
{
   HX_SCRIPT,
   HX_BIN,
   HX_UNKNOWN
}hx_script_t;

/* process details */
typedef struct
{
#ifdef _HX_WIN32_

   STARTUPINFO         *sinfo;
   PROCESS_INFORMATION *pinfo;
   HANDLE               childHandles[3];
   unsigned char       *env_block;

#elif defined(_HX_UNIX_)

   int                  childStreams[3];
   int                  parentStreams[3];
   unsigned char      **env_block;

#endif

   int                  pid;
   unsigned char       *cmdLine;
   unsigned char       *arg;
   unsigned char       *interpreter;
   unsigned char       *path;
   unsigned char       *dir;
   hx_allocator_t      *allocator;
   hx_script_t          type;
}hx_process_t;

/* Function declaration */
int hx_invokeCGI (http_request_t *request, hx_process_t **childProc);

int hx_cleanProc(hx_logsys_t *log, hx_process_t *child);

int hx_kill_child(hx_logsys_t *log, hx_process_t *child);

#endif
/* End of File */

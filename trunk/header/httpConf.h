/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#ifndef HTTPCONF_H
#define HTTPCONF_H

#include "httpMemsys.h"
#include "httpLog.h"

#ifndef _HX_CONFIG_FILE_
#define _HX_CONFIG_FILE_ "../conf/httpdex.conf"
#endif

typedef int (*fconf_t)(hx_logsys_t *log, const unsigned char *value);

typedef struct
{
   unsigned char     *name;
   unsigned char     *value;
}hx_config_record_t;

typedef struct
{
   unsigned char     *cmd;
   fconf_t        func;
}hx_cmd_t;

typedef struct
{
   hx_allocator_t    *a;

   unsigned char     *filename;
   FILE              *fptr;

   /* table to contain
      the pointers to various
      configuration data */
   void              **table;
}hx_config_t;

typedef enum
{
   IDX_NUM_SRV_THRDS = 0,
   IDX_NUM_ADM_THRDS,
   IDX_DOC_ROOT,
   IDX_CGI_ROOT,
   IDX_SERVER_TIMEOUT,
   IDX_DEFAULT_HTTP_PORT,
   IDX_DEFAULT_HOST_NAME,
   IDX_DEFAULT_ERR_PAGE,
   IDX_CONF_ROOT,
   IDX_MIME_FILE_NAME,
   IDX_HTTP_VERSION,
   IDX_PATH_SEPARATOR,
   IDX_INDEX_FILES,
   IDX_ADMIN_PORT,
   IDX_HTTP_SERVICE_PORT,
   IDX_LOG_LEVEL,
   IDX_VHOST,
   MAX_CONF_RECS
}HX_CONF_REC_IDX;

/* Function declarations */
void *hx_get_conf_val(hx_logsys_t *log, int index);
int   hx_load_config(const unsigned char *filename, hx_logsys_t *logsys);
int   hx_reload_config(hx_logsys_t *logsys);
void  hx_destroy_config(hx_logsys_t *logsys);

/* Command Handlers */
int hx_set_log_level_config (hx_logsys_t *log, const unsigned char *data);

int hx_set_num_http_threads(hx_logsys_t *log, const unsigned char *data);

int hx_set_num_admin_threads(hx_logsys_t *log, const unsigned char *data);

int hx_set_doc_root(hx_logsys_t *log, const unsigned char *data);

int hx_set_cgi_root(hx_logsys_t *log, const unsigned char *data);

int hx_set_default_http_port(hx_logsys_t *log, const unsigned char *data);

int hx_set_default_error_page(hx_logsys_t *log, const unsigned char *data);

int hx_set_host_name(hx_logsys_t *log, const unsigned char *data);

int hx_set_config_root(hx_logsys_t *log, const unsigned char *data);

int hx_set_mime_file(hx_logsys_t *log, const unsigned char *data);

int hx_set_http_version(hx_logsys_t *log, const unsigned char *data);

int hx_set_path_separator(hx_logsys_t *log, const unsigned char *data);

int hx_set_index_files(hx_logsys_t *log, const unsigned char *data);

int hx_set_admin_port(hx_logsys_t *log, const unsigned char *data);

int hx_set_http_port(hx_logsys_t *log, const unsigned char *data);

int hx_set_vhost_data(hx_logsys_t *log, const unsigned char *data);

int hx_set_server_timeout(hx_logsys_t *log, const unsigned char *data);

#endif
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include "httpConf.h"
#include "httpInclude.h"
#include "httpVhost.h"

/* Global configuration variable */
static hx_config_t *g_config = NULL;

static const hx_cmd_t cmdTable[] =
{
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
/* { "ERRPAGE"          , hx_setErrPage             }, Server wide error page */
   { NULL               , NULL                      }
};

void *hx_get_conf_val(hx_logsys_t *log, int index)
{
   const char *rname = "hx_create_config ()";
   return g_config->table[index];
}

int hx_set_server_timeout(hx_logsys_t *log,
                        const unsigned char *data)
{
   const char *rname = "hx_set_server_timeout ()";
   unsigned char *trimmed;
   int timeout;

   /* trim the value */
   trimmed = hx_trim((unsigned char *)data, g_config->a);
   if(!trimmed)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Server timeout not specified in Config file");
      return HX_ERR_CONFIG_SERVER_TIMEOUT;
   }

   /* perform validation */
   timeout = atoi(trimmed);

   if(timeout <= 0)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Server timeout should be greater than 0");
      return HX_ERR_CONFIG_SERVER_TIMEOUT;
   }

   hx_log_msg(log, HX_LDEBG, rname, "Server timeout set to %d seconds", timeout);

   g_config->table[IDX_SERVER_TIMEOUT] = hx_strdup(trimmed, g_config->a);
   return OK;
}

int hx_set_vhost_data(hx_logsys_t *log,
                    const unsigned char *data)
{
   const char *rname = "hx_set_vhost_data ()";

   unsigned char *hostname = NULL;
   unsigned char *docroot = NULL;
   unsigned char *scriptroot = NULL;
   unsigned char *logfile = NULL;
   unsigned char *loglevels = NULL;
   unsigned char *errpage = NULL;

   unsigned char *dup = hx_strdup(data, g_config->a);
   unsigned char *pch;
   unsigned char *startptr;
   unsigned char *endptr;

   int count = 0;

   hx_log_msg(log, HX_LCALL, rname, "Called on %s", data);

   /* Break the data into corresponding
      parts */
   startptr = dup;

   /* serially get the vhost data using
      comma as a separator */
   while((pch = strchr(startptr, ',')) != NULL && (count < MAX_VHOST_PARMS))
   {
      *pch = '\0';

      endptr = pch - 1;

      /* trim all trailing spaces */
      while(isspace(*endptr))
         *endptr-- = '\0';

      /* skip initial whitespace */
      while(isspace(*startptr) && startptr)
         ++startptr;

      switch(count)
      {
         case VHOSTPARM_HOSTNAME:
            hostname = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_DOCROOT:
            docroot = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_SCRIPTROOT:
            scriptroot = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_LOGFILE:
            logfile = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_ERRPAGE:
            errpage = hx_strdup(startptr, g_config->a);
            break;
      }
      startptr = pch + 1;
      ++count;
   }

   /* skip initial whitespace */
   while(isspace(*startptr) && startptr)
      ++startptr;

   if(strlen(startptr) && (count < MAX_VHOST_PARMS))
   {
      switch(count)
      {
         case VHOSTPARM_HOSTNAME:
            hostname = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_DOCROOT:
            docroot = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_SCRIPTROOT:
            scriptroot = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_LOGFILE:
            logfile = hx_strdup(startptr, g_config->a);
            break;
         case VHOSTPARM_ERRPAGE:
            errpage = hx_strdup(startptr, g_config->a);
            break;
      }
   }

   /* skip initial whitespace */
   while(isspace(*startptr) && startptr)
      ++startptr;

   /* Extract the log levels for the virtual host */
   if(count == MAX_VHOST_PARMS && strlen(startptr))
   {
      loglevels = hx_strdup(startptr, g_config->a);
   }

   if(!hostname)
   {
      hx_log_msg(log, HX_LWARN, rname, "Virtual Host data not found");
      return OK;
   }

   /* Create a vhost object and add it
      to the system-wide vhost system */
   if(OK != hx_addVhost(log,
                        hostname,
                        docroot,
                        scriptroot,
                        logfile,
                        errpage,
                        loglevels))
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Failed to add Virtual Host Data for %s",
                hostname);

      return ERRC;
   }

   return OK;
}

int hx_set_log_level_config (hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_log_level_config ()";

   unsigned char *loglevels = hx_strdup(data, g_config->a);
   unsigned char *startptr = loglevels;
   unsigned char *pch;
   unsigned char *endptr;

   int level;

   /* reset all log levels */

   hx_reset_log_level(log);

   while((pch = strchr(startptr, ',')) != NULL)
   {
      *pch = '\0';

      endptr = pch - 1;

      /* trim all trailing spaces */
      while(isspace(*endptr))
         *endptr-- = '\0';

      /* skip initial whitespace */
      while(isspace(*startptr) && startptr)
         ++startptr;

      if(strlen(startptr))
      {
         level = hx_get_log_level_from_string(startptr);

         if(NOMATCH != level)
         {
            hx_set_log_level(log, level);
         }
         else
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "Invalid LOG LEVEL \"%s\"",
                      startptr);
      }
      startptr = pch + 1;
   }

   /* skip initial whitespace */
   while(isspace(*startptr) && startptr)
      ++startptr;

   if(strlen(startptr))
   {
      level = hx_get_log_level_from_string(startptr);

      if(NOMATCH != level)
         hx_set_log_level(log, level);
      else
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "Invalid LOG LEVEL \"%s\"",
                   startptr);

   }

   hx_free_mem(loglevels, g_config->a);

   return OK;
}

int hx_set_num_http_threads(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_num_http_threads ()";

   int numthreads;
   unsigned char *trimmed = hx_trim((unsigned char *)data, g_config->a);

   if(!trimmed)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Number of SERVER threads not specified in Config file");
      return HX_ERR_CONFIG_NUM_SRV_THREADS;
   }

   /* perform validation */
   numthreads = atoi(trimmed);

   if(numthreads <= 0)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Number of SERVER threads should be greater than 0");
      return HX_ERR_CONFIG_NUM_SRV_THREADS;
   }

   hx_log_msg(log, HX_LDEBG, rname, "Number of SERVER threads %d", numthreads);

   g_config->table[IDX_NUM_SRV_THRDS] = hx_strdup(trimmed, g_config->a);
   return OK;
}

int hx_set_num_admin_threads(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_num_admin_threads ()";

   int numthreads;
   unsigned char *trimmed = hx_trim((unsigned char *)data, g_config->a);

   if(!trimmed)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Number of ADMIN threads not specified in Config file");
      return HX_ERR_CONFIG_NUM_ADM_THREADS;
   }
   /* perform validation */
   numthreads = atoi(trimmed);

   if(numthreads <= 0)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Number of ADMIN threads should be greater than 0");
      return HX_ERR_CONFIG_NUM_ADM_THREADS;
   }

   hx_log_msg(log, HX_LDEBG, rname, "Number of ADMIN threads %d", numthreads);

   g_config->table[IDX_NUM_ADM_THRDS] = hx_strdup(trimmed, g_config->a);
   return OK;
}

int hx_set_doc_root(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_doc_root ()";

   hx_log_msg(log, HX_LCALL, rname, "Called");

   if(!strlen(data))
      return HX_ERR_CONFIG_DOC_ROOT;

   g_config->table[IDX_DOC_ROOT] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_cgi_root(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_CGI_ROOT] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_default_http_port(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_default_http_port ()";
   int port;
   unsigned char *trimmed = hx_trim((unsigned char *)data, g_config->a);

   if(!trimmed)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Number of SERVER threads not specified in Config file");
      return HX_ERR_CONFIG_HTTP_PORT;
   }

   /* perform validation */
   port = atoi(trimmed);

   if(port <= 0)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Default HTTP Port should be greater than 0");
      return HX_ERR_CONFIG_HTTP_PORT;
   }

   g_config->table[IDX_DEFAULT_HTTP_PORT] = hx_strdup(trimmed, g_config->a);
   return OK;
}

int hx_set_default_error_page(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_default_error_page ()";
   unsigned char *trimmed = hx_trim((unsigned char *)data, g_config->a);

   if(!trimmed)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Sever default error page not provided");
      return HX_ERR_CONFIG_DEFERR_PAGE;
   }

   /* Add the parameter to the table */
   g_config->table[IDX_DEFAULT_ERR_PAGE] = hx_strdup(trimmed, g_config->a);
   return OK;
}

int hx_set_host_name(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_DEFAULT_HOST_NAME] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_config_root(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_CONF_ROOT] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_mime_file(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_MIME_FILE_NAME] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_http_version(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_HTTP_VERSION] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_path_separator(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_PATH_SEPARATOR] = hx_strdup(data, g_config->a);
   return OK;
}

int hx_set_index_files(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_index_files ()";

   hx_list_t *idxList;
   unsigned char *file;
   unsigned char *fileList = hx_strdup(data, g_config->a);
   unsigned char *startptr = fileList;
   unsigned char *pch;
   unsigned char *endptr;

   hx_log_msg(log, HX_LCALL, rname, "Called on %s", data);

   idxList = hx_create_list();
   if(!idxList)
   {
      hx_log_msg(log, HX_LERRO, rname, "Error Creating Index File List");
      return HX_ERR_MEM_ALLOC;
   }

   /* separate the data and add them to the list */
   while((pch = strchr(startptr, ',')) != NULL)
   {
      *pch = '\0';

      endptr = pch - 1;

      /* trim all trailing spaces */
      while(isspace(*endptr))
         *endptr-- = '\0';

      /* skip initial whitespace */
      while(isspace(*startptr) && startptr)
         ++startptr;

      if(strlen(startptr))
      {
         file = hx_strdup(startptr, g_config->a);
         hx_add(idxList, file);
      }
      startptr = pch + 1;
   }

   /* skip initial whitespace */
   while(isspace(*startptr) && startptr)
      ++startptr;

   if(strlen(startptr))
   {
      file = hx_strdup(startptr, g_config->a);
      hx_add(idxList, file);
   }

   /* free the fileList string */
   hx_free_mem(fileList, g_config->a);

   /* Add the newly created list
      to the conf table */

   g_config->table[IDX_INDEX_FILES] = idxList;

   return OK;
}

int hx_set_admin_port(hx_logsys_t *log, const unsigned char *data)
{
   const char *rname = "hx_set_admin_port ()";

   int port;
   unsigned char *trimmed = hx_trim((unsigned char *)data, g_config->a);

   if(!trimmed)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Number of SERVER threads not specified in Config file");
      return HX_ERR_CONFIG_ADMIN_PORT;
   }

   /* perform validation */
   port = atoi(trimmed);

   if(port <= 0)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Default ADMIN Port should be greater than 0");
      return HX_ERR_CONFIG_ADMIN_PORT;
   }

   g_config->table[IDX_ADMIN_PORT] = hx_strdup(trimmed, g_config->a);
   return OK;
}

int hx_set_http_port(hx_logsys_t *log, const unsigned char *data)
{
   g_config->table[IDX_HTTP_SERVICE_PORT] = hx_strdup(data, g_config->a);
   return OK;
}

static int
hx_create_config(hx_logsys_t *logsys)
{
   const char    *rname = "hx_create_config ()";

   hx_allocator_t *a;

   hx_log_msg(logsys, HX_LCALL, rname, "Called");

   a = hx_create_allocator("config");

   if(!a)
   {
      hx_log_msg(logsys, HX_LERRO, rname, "Error Creating Allocator");
      return HX_ERR_MEM_ALLOC;
   }

   g_config = hx_alloc_mem(sizeof(hx_config_t), a);

   if(!g_config)
      return HX_ERR_MEM_ALLOC;

   /* clear the memory */
   memset(g_config, 0x00, sizeof(hx_config_t));

   /* assign the allocator */
   g_config->a = a;

   /* create the table to hold the
      configuration values */
   g_config->table = hx_alloc_mem((sizeof(void *) * MAX_CONF_RECS),
                            g_config->a);

   if(!g_config->table)
   {
      hx_log_msg(logsys, HX_LERRO, rname, "Error Creating Table");
      return HX_ERR_MEM_ALLOC;
   }

   /* initialize the table */
   memset(g_config->table, 0x00, (sizeof(void *) * MAX_CONF_RECS));

   return OK;
}

void
hx_destroy_config(hx_logsys_t *logsys)
{
   const char    *rname = "hx_destroy_config ()";
   int         i;
   hx_list_t  *idxList;

   hx_log_msg(logsys, HX_LCALL, rname, "Called");

   /* Destroy the index file list */
   idxList = (hx_list_t *)hx_get_conf_val(logsys, IDX_INDEX_FILES);
   if(idxList)
   {
      hx_destroy_list(idxList);
      idxList = 0;
   }

   /* garbage collect */
   hx_destroy_allocator(g_config->a);
   g_config = NULL;

   hx_log_msg(logsys,
             HX_LSYST,
             rname,
             "OFFLINE -> Configuration System");
   return;
}

static int
hx_add_entry(unsigned char *line, hx_logsys_t *logsys)
{
   const char    *rname = "hx_add_entry ()";

   int i;
   unsigned char *name;
   unsigned char *value;
   unsigned char *pch;
   int found = 0;
   int retval;

   hx_log_msg(logsys, HX_LCALL, rname, "Called on line >%s<", line);

   /* Get the name */
   if((pch = strchr(line, '=')) != NULL)
   {
      *pch = '\0';
      name = hx_strdup(line, g_config->a);
      value = hx_strdup(pch + 1, g_config->a);

      for(i = 0; cmdTable[i].cmd; ++i)
      {
         if(!hx_strcmpi(cmdTable[i].cmd, name))
         {
            found = 1;
            break;
         }
      }

      if(found)
      {
         /*execute the function */
         retval = (cmdTable[i].func)(logsys, value);
         if(OK != retval)
            return HX_ERR_CONFIG_FUNC;
      }
      else
      {
         hx_log_msg(logsys,
                   HX_LERRO,
                   rname,
                   "Invalid config key =>%s<=", name);
         return HX_ERR_CONFIG_KEY;
      }
   }
   return OK;
}

static int
hx_read_config(hx_logsys_t *logsys)
{
   const char    *rname = "hx_read_config ()";

   unsigned char line[MAX_CONF_LINE_LEN];
   unsigned char *lineStart;
   unsigned char *cond;

   int lineNum = 0;
   int retval;

   hx_log_msg(logsys, HX_LCALL, rname, "Called");

   g_config->fptr = fopen(g_config->filename, "r");

   if(!g_config->fptr)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Unable to open Configuration file \"%s\"",
                g_config->filename);

      return HX_ERR_FILE_OPEN;
   }

   do
   {
      ++lineNum;
      cond = fgets(line, MAX_CONF_LINE_LEN, g_config->fptr);
      lineStart = line;

      /* remove the newline */
      if(line[strlen(line) - 1] == '\n')
         line[strlen(line) - 1] = '\0';

      /* Skip initial whitespaces */
      while(isspace(*lineStart))
         ++lineStart;

      /* if line is a comment
         carry on */
      if (*lineStart == '#')
         continue;

      if(!strlen(line))
         continue;

      retval = hx_add_entry (line, logsys);

      if(retval != OK)
      {
         hx_log_msg(server_log_sys,
                   HX_LERRO,
                   rname,
                   "Error in Configuration file %s : %03d",
                   g_config->filename,
                   lineNum);

         fclose(g_config->fptr);
         return HX_ERR_CONFIG_DATA;
      }
      if(feof(g_config->fptr))
         break;
   }
   while(cond);

   if(ferror(g_config->fptr))
   {
      fclose(g_config->fptr);
      return HX_ERR_FILE_READ;
   }

   fclose(g_config->fptr);

   return OK;
}

int
hx_load_config(const unsigned char *filename, hx_logsys_t *logsys)
{
   const char    *rname = "hx_load_config ()";

   hx_log_msg(logsys, HX_LCALL, rname, "Called");

   if(g_config)
   {
      hx_log_msg(logsys,
                HX_LWARN,
                rname,
                "Configuration System Already active");

      return HX_ERR_CONFIG_INITED;
   }

   /* create the config */
   if(OK != hx_create_config(logsys))
   {
      hx_log_msg(logsys,
                HX_LERRO,
                rname,
                "Error creating configuration system");

      return HX_ERR_CONFIG_CREATE;
   }

   /* fill in the filename */
   g_config->filename = hx_strdup(filename, g_config->a);

   /* initialize the virtual host system */
   if(OK != hx_create_vhost_sys(logsys))
      return ERRC;

   /* open the config file and load data */
   if(OK != hx_read_config(logsys))
   {
      hx_log_msg(logsys,
                HX_LERRO,
                rname,
                "Error loading configuration system data");

      return HX_ERR_CONFIG_LOAD;
   }

   /* create the server wide default virtual host */
   if(OK != hx_add_default_vhost(logsys))
   {
      hx_log_msg(logsys,
                HX_LERRO,
                rname,
                "Error creating default virtual host");
      return HX_ERR_CONFIG_LOAD;
   }

   return OK;
}

int
hx_reload_config(hx_logsys_t *logsys)
{
   const char    *rname = "hx_reload_config ()";
   hx_list_t     *idxList;

   hx_log_msg(logsys, HX_LCALL, rname, "Called");

   /* Destroy the index file list */
   idxList = (hx_list_t *)hx_get_conf_val(logsys, IDX_INDEX_FILES);
   if(idxList)
   {
      hx_destroy_list(idxList);
      idxList = 0;
   }

   /* destroy the existing virtual host system */
   hx_destroyVhostSys(logsys);

   /* initialize the virtual host system */
   if(OK != hx_create_vhost_sys(logsys))
      return ERRC;

   /* open the config file and load data */
   if(OK != hx_read_config(logsys))
   {
      hx_log_msg(logsys,
                HX_LERRO,
                rname,
                "Error Reloading configuration system data");
      return HX_ERR_CONFIG_LOAD;
   }

   return OK;
}
/* End of File */


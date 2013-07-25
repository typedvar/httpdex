/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#include "httpVhost.h"
#include "httpInclude.h"
#include "httpConf.h"

/* the module scope list which
   will contain all vhost data */
typedef struct
{
   hx_list_t      *list;
   hx_allocator_t *a;
}hx_vhostsys_t;

static hx_vhostsys_t *vhostsys = NULL;

/* function declarations */
/* Comparator function for vhost comparison */
/* Host names are case insensitive */
static int
vhostcomp(void *arg1, void *arg2)
{
   hx_vhost_t *h1 = (hx_vhost_t *)arg1;
   hx_vhost_t *h2 = (hx_vhost_t *)arg2;

   return hx_strcmpi(h1->host, h2->host);
}

/* get the vhost object from the
   list depending on the hostname */
hx_vhost_t *
hx_getVhost(hx_logsys_t *log, const unsigned char *hostname)
{
   const char *rname = "hx_getVhost ()";

   hx_vhost_t *result;
   hx_vhost_t *dummy;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", hostname);

   /* create a dummy vhost object */
   dummy = hx_alloc_mem(sizeof(hx_vhost_t), vhostsys->a);

   /* assign the host name to the dummy
      object */
   dummy->host = hx_strdup(hostname, vhostsys->a);

   /* find the corresponding vhost object in the
      list */
   return (hx_vhost_t *) hx_find(vhostsys->list, dummy, vhostcomp);
}

/* This object is server wide and is
   allocated on the server's memory
   allocator */
int
hx_add_default_vhost(hx_logsys_t *log)
{
   const char *rname = "hx_add_default_vhost ()";
   hx_allocator_t *allocator = server_allocator;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   /* Check to see if the default
      Virtual host object exists */
   if(defaultVhost)
   {
      /* the object must be garbage collected
         before new values are assigned to it */
      hx_free_mem(defaultVhost->host, allocator);
      hx_free_mem(defaultVhost->docroot, allocator);
      hx_free_mem(defaultVhost->scriptroot, allocator);

      if(defaultVhost->errpage)
         hx_free_mem(defaultVhost->errpage, allocator);

      defaultVhost->log = NULL;
      hx_free_mem(defaultVhost, allocator);
   }

   /* Create the default vhost object */
   defaultVhost = (hx_vhost_t *)hx_alloc_mem(sizeof(hx_vhost_t), allocator);

   if(!defaultVhost)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Default VHOST object creation failed");

      return ERRC;
   }

   /* Assign the defalt values */
   defaultVhost->host = hx_strdup((unsigned char *)
                                  hx_get_conf_val(server_log_sys,
                                                IDX_DEFAULT_HOST_NAME),
                                  allocator);
   defaultVhost->docroot = hx_strdup((unsigned char *)
                                     hx_get_conf_val(server_log_sys,
                                                   IDX_DOC_ROOT),
                                     allocator);
   defaultVhost->scriptroot = hx_strdup((unsigned char *)
                                        hx_get_conf_val(server_log_sys,
                                                      IDX_CGI_ROOT),
                                        allocator);
   defaultVhost->log = server_log_sys;


   defaultVhost->errpage = hx_strdup((unsigned char *)
                                     hx_get_conf_val(server_log_sys,
                                                   IDX_DEFAULT_ERR_PAGE),
                                     allocator);

   /* Check to see if the default error page
      is provided */
   if(!hx_strcmpi(defaultVhost->errpage, "default"))
   {
      hx_free_mem(defaultVhost->errpage, allocator);
      defaultVhost->errpage = NULL;
   }
   return OK;
}

/* returns the success or failure */
int
hx_addVhost(hx_logsys_t *log,
            const unsigned char *hostname,
            const unsigned char *docroot,
            const unsigned char *scriptroot,
            const unsigned char *logfile,
            const unsigned char *errpage,
            const unsigned char *loglevels)
{
   const char *rname = "hx_addVhost ()";
   hx_vhost_t *vhost;
   int         retval;
   struct stat resource_stat;
   unsigned char *page = NULL;

   /* Check if vhost system is created */
   if(!vhostsys)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "vhostsys not created");
      return HX_ERR_VHOST_NOTREADY;
   }

   /* Create a new vhost object */
   vhost = hx_alloc_mem(sizeof(hx_vhost_t), vhostsys->a);

   if(!vhost)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Virtual Host %s Object creation failed", hostname);

      return HX_ERR_MEM_ALLOC;
   }

   /* Assign the host name
      for this object */
   vhost->host = hx_strdup(hostname, vhostsys->a);

   /* Assign the document root
      for this object */
   if(docroot)
   {
      retval = hx_getFileStat(log,
                              docroot,
                              &resource_stat,
                              lock_errno);
      if(OK != retval)
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "virtual host %s document root does not exist",
                   hostname);

         return HX_ERR_VHOST_DOCROOT;
      }

      if(!(S_ISDIR(resource_stat.st_mode)))
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "virtual host %s document root is not a directory",
                   hostname);

         return HX_ERR_VHOST_DOCROOT;
      }

      vhost->docroot = hx_strdup(docroot, vhostsys->a);
   }
   else
      vhost->docroot = NULL;

   /* Assign the script root
      for this object */
   if(scriptroot)
      vhost->scriptroot = hx_strdup(scriptroot, vhostsys->a);
   else
      vhost->scriptroot = NULL;

   /* Object created - try creating a log system
      for this virtual host */
   if(logfile)
   {
      vhost->log = hx_create_log_sys(server_log_sys,
                                   logfile,
                                   vhost->host,
                                   HX_LOGPREFIX_YES);

      if(!vhost->log)
      {
         hx_log_msg(server_log_sys,
                   HX_LERRO,
                   rname,
                   "Virtual host %s log creation failed", hostname);

         return HX_ERR_CREATE_LOG;
      }
      /* reset the log levels */
      hx_reset_log_level(vhost->log);

      /* Check if any log level has been assigned to
         this virtual host object, if found set the
         appropriate log level else set the default log
         levels */
      if(loglevels)
         hx_set_log_level_config (vhost->log, loglevels);
   }
   else
      vhost->log = server_log_sys;

   /* Assign the error page if any */
   /* check to see if the page is the "default"
      error page. If yes, check whether
      a server default error page has been
      supplied. If yes, assign that as the
      error page for the vhost. If no, assign
      the error page to null. This mean the
      error page will be generated by httpdEx
   */
   page = hx_trim((unsigned char *)errpage, vhostsys->a);

   if(!hx_strcmpi("default", page))
   {
      /* default error page assigned */
      vhost->errpage = NULL;
      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Virtual Host Object %s Error page set to default",
                hostname);
   }
   else
   {
      /* the actual error page is
         assigned */
      vhost->errpage = page;
      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Virtual Host Object %s Error page set to %s",
                hostname,
                page);
   }

   /* Add this object to the virtual
      host object list */
   retval = hx_add(vhostsys->list, (void *)vhost);

   if(ERRC == retval)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Virtual Host Object %s addition failed", hostname);

      return HX_ERR_VHOST_ADD;
   }

   return OK;
}

static int
hx_destroyVhost(hx_logsys_t *log, hx_vhost_t *vhost)
{
   const char *rname = "hx_destroyVHost ()";
   int         retval;

   /* Check if vhost system is created */
   if(!vhostsys)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "vhostsys not created");
      return HX_ERR_VHOST_NOTREADY;
   }

   /* destroy the log system for the vhost */
   if(vhost->log)
   {
      hx_log_msg(vhost->log,
                HX_LSYST,
                rname,
                "OFFLINE -> Virtual Host %s", vhost->host);

      hx_destroy_log_sys(vhost->log);
      vhost->log = NULL;
   }

   return OK;
}

int
hx_create_vhost_sys(hx_logsys_t *log)
{
   const char *rname = "hx_create_vhost_sys ()";

   hx_allocator_t *a;

   /* check whether system is already
      up and running */
   if(vhostsys)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "vhostsys already created");

      return HX_ERR_VHOST_READY;
   }

   /* create the allocator system */
   a = hx_create_allocator("vhostsys");

   if(!a)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "vhostsys allocator creation failed");

      return HX_ERR_MEM_ALLOC;
   }

   /* create the vhost system */
   vhostsys = hx_alloc_mem(sizeof(hx_vhostsys_t), a);

   if(!vhostsys)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "vhostsys creation failed");

      return HX_ERR_MEM_ALLOC;
   }

   /* clear the system */
   memset(vhostsys, 0x00, sizeof(hx_vhostsys_t));

   /* assign the allocator */
   vhostsys->a = a;

   /* create the container list */
   vhostsys->list = hx_create_list();
   if (!vhostsys->list)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "vhostsys list creation failed");

      return HX_ERR_MEM_ALLOC;
   }

   hx_log_msg(log,
             HX_LVERB,
             rname,
             "ONLINE -> Virtual Host System");

   return OK;
}

void
hx_destroyVhostSys(hx_logsys_t *log)
{
   const char *rname = "hx_destroyVhostSys ()";
   hx_vhost_t *vhost;
   int entries;
   int i;

   if(!vhostsys)
      return;

   if(vhostsys->list)
   {
      entries = hx_size(vhostsys->list);

      for(i = 0; i < entries; ++i)
      {
         vhost = (hx_vhost_t *)hx_get_pos(vhostsys->list, i);
         hx_log_msg(log,
                   HX_LDEBG,
                   rname,
                   "Initiating Virtual Host %s Cleanup", vhost->host);

         hx_destroyVhost(log, vhost);
      }

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Initiating vhostsys list Cleanup");

      hx_destroy_list(vhostsys->list);
   }

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Initiating vhostsys garbage collector");

   if(vhostsys->a)
      hx_destroy_allocator(vhostsys->a);

   /* set the vhostsys to null */
   vhostsys = NULL;

   hx_log_msg(log,
             HX_LSYST,
             rname,
             "OFFLINE -> Virtual Host System");

   return;
}

/* End of File */

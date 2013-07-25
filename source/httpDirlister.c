/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef _HX_UNIX_
#include <unistd.h>
#include <errno.h>
#endif

#include "httpDirlister.h"
#include "httpInclude.h"
#include "httpConf.h"

static char*
hx_get_mode_string(hx_logsys_t *log, mode_t mode, hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_get_mode_string ()";
   unsigned char *mode_string;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   mode_string = (unsigned char *)hx_alloc_mem(MAX_MODESTR_LEN + 1, allocator);

   if(!mode_string)
      return NULL;

   memset(mode_string, '-', MAX_MODESTR_LEN);

   mode_string[MAX_MODESTR_LEN] = '\0';

   if(S_ISDIR(mode))
      mode_string[0] = 'd';

   if(mode & S_IRUSR)
      mode_string[1] = 'r';

   if(mode & S_IWUSR)
      mode_string[2] = 'w';

   if(mode & S_IXUSR)
      mode_string[3] = 'x';

#ifdef _HX_UNIX_
   if(mode & S_IRGRP)
      mode_string[4] = 'r';

   if(mode & S_IWGRP)
      mode_string[5] = 'w';

   if(mode & S_IXGRP)
      mode_string[6] = 'x';

   if(mode & S_IROTH)
      mode_string[7] = 'r';

   if(mode & S_IWOTH)
      mode_string[8] = 'w';

   if(mode & S_IXOTH)
      mode_string[9] = 'x';
#endif

   return mode_string;
}

static int
hx_getDirEDetails(hx_logsys_t *log,
                  const unsigned char *dirpath,
                  const unsigned char *entname,
                  direlem_t *curr,
                  hx_allocator_t *allocator,
                  hx_lock_t *lock)
{
   const unsigned char *rname = "hx_getDirEDetails ()";

   char href[MAX_HREF_LEN];
   char fname[MAX_FILENAME];
   struct stat st;

   int retval;

   *href = '\0';

   /*
      check if the curr element is
      a top dir and create the href
   */
   if(!strcmp(entname, ".."))
   {
      curr->ent = hx_strdup("Up...", allocator);
      strcat(href, curr->entroot);
      strcat(href, (unsigned char *)hx_get_conf_val(log, IDX_PATH_SEPARATOR));
      strcat(href, "..");
   }
   else
   {
      curr->ent = hx_strdup(entname, allocator);

      if(!strcmp(curr->entroot, "/"))
      {
         strcat(href, curr->entroot);
         strcat(href, entname);
      }
      else
      {
         strcat(href, curr->entroot);
         strcat(href, (unsigned char *)hx_get_conf_val(log, IDX_PATH_SEPARATOR));
         strcat(href, curr->ent);
      }
   }

   curr->href = hx_strdup(href, allocator);

   *fname = '\0';
   strcat(fname, dirpath);
   strcat(fname, (unsigned char *)hx_get_conf_val(log, IDX_PATH_SEPARATOR));
   strcat(fname, entname);

   retval = hx_getFileStat(log, fname, &st, lock /* cserr */);

   if (retval == OK)
   {
      curr->size = st.st_size;
      curr->mode = hx_get_mode_string(log, st.st_mode, allocator);
      curr->isdir = ((S_ISDIR(st.st_mode))?1:0);
      curr->lmt = st.st_mtime;
   }
   return retval;
}

int
hx_get_dir_list(http_request_t *request_object,
              hx_list_t** list,
              hx_lock_t *lock)
{
   const unsigned char *rname = "hx_get_dir_list ()";

   DIR *dir;
   struct dirent *dent;

   direlem_t *curr;
   hx_list_t *dirlist;

   unsigned char *root;
   const unsigned char *dirpath = request_object->filename;

   int retval;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* assign the list value to NULL */
   *list = NULL;

   root = hx_strdup(request_object->uri_object->resource, request_object->allocator);

   hx_removeTrail(root);

   /* create the list */
   dirlist = hx_create_list();

   if(!dirlist)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "Could not create list\n");
      return HX_ERR_LISTCREATE;
   }

   /* open the directory for reading
      and fetch the errno */
   hx_lock(lock);
   dir = opendir(dirpath);
   retval = errno;
   hx_unlock(lock);

   if(dir == NULL)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "%s", rname, strerror(errno));
      hx_destroy_list(dirlist);
      return hx_getSysErrCode(retval);
   }

   while((dent = readdir(dir)) != NULL)
   {
      /* Ignore current directory */
      if(!strcmp(dent->d_name, "."))
         continue;

      /* We won't add an Up..
         entry if we are at
         Web Server ROOT
      */
      if(!strcmp(dent->d_name, ".."))
      {
         if(!strcmp(root, "/"))
            continue;
      }

      /* create a direlem */
      curr = (direlem_t *)hx_alloc_mem(sizeof(direlem_t), request_object->allocator);

      /* set current root */
      curr->entroot = hx_strdup(root, request_object->allocator);

      retval = hx_getDirEDetails (request_object->vhost->log,
                                  dirpath,
                                  dent->d_name,
                                  curr,
                                  request_object->allocator,
                                  lock);

      if(retval == OK)
      {
         retval = hx_add(dirlist, (void *)curr);
         if(retval != OK)
         {
            hx_log_msg(request_object->vhost->log,
                      HX_LERRO,
                      rname,
                      "Could not add directory entry to list\n");
            return HX_ERR_INTERNAL;
         }
      }
   }

   /* close the directory */
   closedir(dir);
   hx_free_mem(root, request_object->allocator);

   /*
   hx_print(request_object->vhost->log, dirlist, (fprinter_t)hx_dir_lister, HX_LDEBG);
   */

   *list = dirlist;
   return OK;
}


void
hx_dir_lister(void *data, unsigned char **line, hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_dir_lister ()";
   direlem_t *elem = (direlem_t *)data;
   unsigned char *lmt;

   if(!data)
      return;

   if(!line)
      return;

   lmt = hx_strdup(ctime(&elem->lmt), allocator);

   *line = (unsigned char *)hx_alloc_mem(MAX_DIRLINE_LEN, allocator);

   sprintf(*line,
           "<tr>\n"
#ifdef _HX_UNIX_
               "<td>%s</td>\n"                  /* mode           */
#endif
               "<td>"
                  "<a href=\"%s\">%s</a>"       /* href and name  */
               "</td>\n"
               "<td align=\"right\">%d</td>\n"  /* bytes          */
               "<td align=\"right\">%s</td>\n"  /* last mod time  */
           "</tr>\n",
#ifdef _HX_UNIX_
           elem->mode,
#endif
           elem->href,
           elem->ent,
           elem->size,
           lmt);
   hx_free_mem(lmt, allocator);
}
/* End of File */

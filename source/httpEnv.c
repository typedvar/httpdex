/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include "httpEnv.h"
#include "httpInclude.h"

/*
   hx_env_printer
   prints the environment
   line by line
*/
void
hx_env_printer(void *data, unsigned char **line, hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_env_printer ()";
   hx_envpair_t *elem = (hx_envpair_t *)data;
   int tab = 3;

   if(!elem)
      return;

   if(!line)
      return;

   *line = hx_alloc_mem((strlen(elem->nameval) + tab + 1), allocator);

   sprintf(*line, "   %s", elem->nameval);
}

/*
   hx_create_unix_env_block
   creates a UNIX exec family
   compatible environment block
*/
int
hx_create_unix_env_block(hx_list_t *environ,
                      unsigned char ***env_block,
                      hx_allocator_t *allocator)
{
   int            numel;
   int            i;
   hx_envpair_t   *entry;
   unsigned char  **pch;

   /* get the number of elements in
      the environment list, take into
      account the null terminator */
   numel = hx_size(environ) + 1;

   /* allocate space for the env_block an
      array of null terminated char
      pointers */
   *env_block = (unsigned char **)hx_alloc_mem((numel * sizeof(unsigned char *)),
                                            allocator);
   if(!*env_block)
      return HX_ERR_MEM_ALLOC;

   pch = *env_block;

   for(i = 0; i < (numel - 1); ++i, ++pch)
   {
      entry = (hx_envpair_t *)hx_get_pos(environ, i);
      *pch = hx_strdup(entry->nameval, allocator);

      if(!*pch)
         return HX_ERR_MEM_ALLOC;
   }

   *pch = (unsigned char *)NULL;

   return OK;
}

/*
   hx_create_win32_env_block
   creates a WIN32 CreateProcess ()
   compatible environment block
*/
int
hx_create_win32_env_block(hx_list_t *environ,
                       unsigned char **env_block,
                       hx_allocator_t *allocator)
{
   int            blockLen = 1;
   int            i;
   int            envcount = hx_size(environ);
   hx_envpair_t  *entry;
   unsigned char *pch;

   for (i = 0; i < envcount; ++i)
   {
      entry = (hx_envpair_t *)hx_get_pos(environ, i);
      blockLen += strlen(entry->nameval) + 1;
   }

   /* allocate space for the env_block and include the
      terminating null character */
   *env_block = (unsigned char *)hx_alloc_mem(blockLen + 1, allocator);

   if(!*env_block)
      return HX_ERR_MEM_ALLOC;

   pch = *env_block;

   for(i = 0; i < envcount; ++i)
   {
      entry = (hx_envpair_t *)hx_get_pos(environ, i);
      strcpy(pch, entry->nameval);
      pch += strlen(entry->nameval) + 1;
   }

   *pch = '\0';

   return OK;
}

/* hx_create_env_element
   creates a string suitable to be added
   as either the name or value component
   to the environment
*/
unsigned char *
hx_create_env_element(hx_logsys_t *log,
               const unsigned char *elem,
               hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_create_env_element ()";

   unsigned char *buf1;
   unsigned char *buf2;

   unsigned char *retstr;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   buf1 = hx_strdup(elem, allocator);
   hx_convert(log, buf1, SP, '+');

   /* escape all '=' */
   /* allocate enough space
      assuming everything needs to be escaped */
   buf2 = (unsigned char *)hx_alloc_mem((strlen(elem) * 3) + 1, allocator);
   hx_escape(log, buf2, buf1, '=');

   retstr = hx_strdup(buf2, allocator);

   /* deallocate mem */
   hx_free_mem(buf1, allocator);
   hx_free_mem(buf2, allocator);

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "\"%s\" is converted to \"%s\"",
             elem,
             retstr);

   return retstr;
}

/* hx_create_env_pair
   Create a hx_envpair_t object from the
   supplied name value pair
*/
hx_envpair_t*
hx_create_env_pair (hx_logsys_t *log,
                  const unsigned char *name,
                  const unsigned char *value,
                  hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_create_env_pair ()";
   unsigned char *processedName;
   unsigned char *processedVal;

   hx_envpair_t *entry;

   if(!name || !value)
      return NULL;

   hx_log_msg(log,
             HX_LCALL,
             rname,
             "Called on name=\"%s\" value=\"%s\"",
             name,
             value);

   /* create env entry */
   entry = (hx_envpair_t *)hx_alloc_mem(sizeof(hx_envpair_t), allocator);
   if(!entry)
      return NULL;

   /* create the env elements */
   processedName = hx_create_env_element(log, name, allocator);
   processedVal  = hx_create_env_element(log, value, allocator);

   /* allocate memory for the env pair */
   entry->nameval = (unsigned char *)hx_alloc_mem(strlen(processedName) +
                                               strlen(processedVal) + 2,
                                               allocator);
   *entry->nameval = '\0';

   strcat(entry->nameval, processedName);
   strcat(entry->nameval, "=");
   strcat(entry->nameval, processedVal);

   hx_log_msg(log, HX_LCALL, rname, "%s", entry->nameval);

   return entry;
}

/* hx_env_from_body
   This function extracts the name-value pairs
   from the body of a post request and stuffs
   them into the environment object
*/
int
hx_env_from_body (hx_logsys_t *log,
                hx_list_t *environ,
                const unsigned char *body,
                hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_env_from_body ()";

   hx_envpair_t *entry;

   unsigned char *dup = hx_strdup(body, allocator);
   unsigned char *startptr = dup;
   unsigned char *pch;

   int retval;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   if(!body || !startptr || !environ)
      return ERRC;

   while(*startptr)
   {
      /* allocate memory for the
         environment entry */
      entry = (hx_envpair_t *)hx_alloc_mem(sizeof(hx_envpair_t), allocator);

      if((pch = strchr(startptr, '&')) != NULL)
      {
         *pch = '\0';

         entry->nameval = hx_strdup(startptr, allocator);

         /* add the entry to the environment list */
         retval = hx_add(environ, (void *)entry);

         if(retval != OK)
            return ERRC;

         /* place the startptr to the start of the next
            name-value pair */
         startptr = pch + 1;
      }
      else
      {
         entry->nameval = hx_strdup(startptr, allocator);

         /* add the entry to the environment list */
         retval = hx_add(environ, (void *)entry);

         if(retval != OK)
            return ERRC;

         break;
      }
   }

   return OK;
}
/* End of File */

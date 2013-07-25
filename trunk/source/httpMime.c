/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "httpCommon.h"
#include "httpInclude.h"
#include "httpConf.h"
#include "httpMime.h"

/* partial Implementation of rfc 2045/46/47/48 */
http_mime_table_t *mimeTable = NULL;

const unsigned char *
hx_get_mime_type(const http_mime_t *mime)
{
   return mime->type;
}

const unsigned char *
hx_get_mime_sub_type(const http_mime_t *mime)
{
   return mime->subtype;
}

void
hx_print_mime_type(hx_logsys_t *logsys, http_mime_t *mime, int log_level)
{
   const unsigned char *rname = "hx_print_mime_type ()";

   if(!mime)
      return;

   if(mime->ext)
      hx_log_msg(logsys, log_level, rname, "Extension: %s", mime->ext);
   if(mime->type)
      hx_log_msg(logsys, log_level, rname, "     Type: %s", mime->type);
   if(mime->subtype)
      hx_log_msg(logsys, log_level,
                rname,
                "  SubType: %s",
                mime->subtype);
}

void
hx_print_mime_table(hx_logsys_t *logsys, int log_level)
{
   const unsigned char *rname = "hx_print_mime_table ()";
   int count;

   if(hx_check_log_level(logsys, log_level))
   {
      hx_log_msg(logsys, log_level, rname, "MIME TABLE START");
      for(count = 0; count < mimeTable->numTypes; count++)
      {
         hx_print_mime_type(logsys, mimeTable->table[count], log_level);
      }
      hx_log_msg(logsys, log_level, rname, "MIME TABLE END");
   }
}

static int
hx_addMime (hx_logsys_t *log,
            const unsigned char *line,
            hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_addMime ()";

   unsigned char *type;
   int typeLen;

   unsigned char *subType;
   int subTypeLen;

   unsigned char *subTypeEnd;

   unsigned char *extString;
   unsigned char *ext;
   unsigned char *pch;
   unsigned char *extStart;

   http_mime_t *newMime;

   if(hx_check_log_level(log, HX_LDMIM) && hx_check_log_level(server_log_sys, HX_LDEBG))
      hx_log_msg(log, HX_LDMIM, rname, "Called on \"%s\"", line);

   /* The type starts at the line start */
   type = (unsigned char *) line;

   /* Get the subType */
   subType = strchr(type, '/');
   if(!subType)
      return HX_ERR_INVALID_MIME_LINE;

   typeLen = subType - type;

   /* Skip the '/' */
   subType++;
   subTypeEnd = subType;

   /* Find the first whitespace after
      the subtype start, this will be
      the subtype end */
   while(!isspace(*subTypeEnd) && *subTypeEnd)
      subTypeEnd++;

   /* Null encountered before subType End
      indicates this type does not have any
      extension associated, so return */
   if(!(*subTypeEnd))
      return OK;

   subTypeLen = subTypeEnd - subType;

   ext = subTypeEnd;

   /* skip whitespaces */
   while(isspace(*ext) && *ext)
      ++ext;

   if(*ext)
   {
      /* copy the remaining line as the ext string */
      extString = hx_strdup(ext, allocator);

      if(hx_check_log_level(log, HX_LDMIM) && hx_check_log_level(server_log_sys, HX_LDEBG))
         hx_log_msg(log,
                   HX_LDMIM,
                   rname,
                   "Extension is \"%s\"",
                   extString);

      /* position the pointer appropriately */
      extStart = extString;

      /* for each extension found
         create a HTTPMimeType obj,
         and add the corresponding data */
      while((pch = strchr(extStart, ' ')) != NULL)
      {
         *pch = '\0';

         newMime = (http_mime_t *) hx_alloc_mem (sizeof(http_mime_t), allocator);

         /* set the type */
         newMime->type = hx_strndup(type, typeLen, allocator);

         /* set the subtype */
         newMime->subtype = hx_strndup(subType, subTypeLen, allocator);

         /* copy the extension */
         newMime->ext = hx_strdup(extStart, allocator);

         mimeTable->table[mimeTable->numTypes++] = newMime;

         /* adjust the pointer */
         extStart = pch + 1;
      }

      if(strlen(extStart) > 0)
      {
         newMime = (http_mime_t *) hx_alloc_mem (sizeof(http_mime_t), allocator);

         /* set the type */
         newMime->type = hx_strndup(type, typeLen, allocator);

         /* set the subtype */
         newMime->subtype = hx_strndup(subType, subTypeLen, allocator);

         /* copy the extension */
         newMime->ext = hx_strdup(extStart, allocator);

         mimeTable->table[mimeTable->numTypes++] = newMime;
      }

      hx_free_mem(extString, allocator);
   }

   return OK;
}

int
hx_init_mime_table(hx_logsys_t *log, hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_init_mime_table ()";
   FILE *fptr;
   unsigned char *mimeFileName;
   unsigned char *confRoot;

   char filename[MAX_FILENAME];
   char line[MAX_CONF_LINE_LEN];

   unsigned char *lineStart;
   unsigned char *cond;

   int retval;
   int lineNum = 0;

   /* get the absolute file path */
   mimeFileName = (unsigned char *) hx_get_conf_val(log, IDX_MIME_FILE_NAME);

   if(!mimeFileName)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "MIME file name not in config");
      return HX_ERR_MIMEFILE_NOT_FOUND;
   }

   confRoot = (unsigned char *) hx_get_conf_val(log, IDX_CONF_ROOT);

   if(!confRoot)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Configuration directory not in config");
      return HX_ERR_CONFROOT_NOT_FOUND;
   }


   *filename = '\0';
   strcat(filename, confRoot);
   strcat(filename, (unsigned char *)hx_get_conf_val(log, IDX_PATH_SEPARATOR));
   strcat(filename, mimeFileName);

   if((fptr = fopen (filename, "r")) == NULL)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Invalid MIME filename \"%s\"",
                filename);
      return HX_ERR_FILE_OPEN;
   }

   mimeTable = (http_mime_table_t *)hx_alloc_mem(sizeof(http_mime_table_t), allocator);
   memset(mimeTable, 0x00, sizeof(http_mime_table_t));

   if (!mimeTable)
   {
      fclose(fptr);
      return HX_ERR_MEM_ALLOC;
   }

   do
   {
      ++lineNum;
      cond = fgets(line, MAX_CONF_LINE_LEN, fptr);
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

      retval = hx_addMime(log, line, allocator);

      if(retval != OK)
      {
         fclose(fptr);
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "Error in MIME file line %d",
                   lineNum);
         return HX_ERR_INVALID_MIME_FILE;
      }
      if(feof(fptr))
         break;
   }
   while(cond);

   if(ferror(fptr))
   {
      fclose(fptr);
      return HX_ERR_FILE_READ;
   }

   fclose(fptr);
   hx_log_msg(log, HX_LDEBG, rname, "MIME Table loaded");
   hx_print_mime_table(log, HX_LDMIM);
   return OK;
}

/*
   hx_lookup_ext_mime
   looks up a mime object given the
   extension
*/
http_mime_t *
hx_lookup_ext_mime(hx_logsys_t *log, const unsigned char *ext)
{
   const unsigned char *rname = "hx_lookup_ext_mime ()";
   int count;
   http_mime_t *mime = NULL;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", ext);

   for (count = 0; count < mimeTable->numTypes; ++count)
   {
      if(!hx_strcmpi(mimeTable->table[count]->ext, ext))
      {
         mime = mimeTable->table[count];
         break;
      }
   }

   return mime;
}

/*
   hx_lookup_resource_mime
   looks up a mime object given the
   resource name
*/
http_mime_t *
hx_lookup_resource_mime(hx_logsys_t *log, const unsigned char *resource)
{
   const unsigned char *rname = "hx_lookup_resource_mime ()";

   int count;
   http_mime_t *mime = NULL;
   unsigned char *extension;
   unsigned char *pch;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", resource);

   /* Retrieve the extension from the
      resource name */
   pch = strrchr(resource, '/');

   /* locate the start of the filename */
   if(!pch)
      pch = (unsigned char *)resource;
   else
   {
      /* skip the '/' */
      ++pch;
   }

   if(strlen(pch))
   {
      /* locate the last '.' */
      extension = strrchr(pch + 1, '.');

      if(extension)
      {
         /* skip the '.' */
         ++extension;

         if(strlen(extension))
         {
            /* look up the mime type
               for the extension */
            mime = hx_lookup_ext_mime(log, extension);
         }
      }
   }

   return mime;
}

/* End of File */

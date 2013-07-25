/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include "httpCommon.h"
#include "httpInclude.h"

#ifdef _HX_UNIX_
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#endif

/* hx_trim
   trim preceeding and trailing spaces
   in a C string
*/
unsigned char *
hx_trim(unsigned char *source, hx_allocator_t *allocator)
{
   unsigned char *startptr = source;
   unsigned char *endptr;
   unsigned char *result = NULL;

   if(!source || !strlen(source))
      return NULL;

   /* trim preceeding spaces */
   while(isspace(*startptr) && (*startptr))
      ++startptr;

   /* position the endptr */
   endptr = &source[strlen(source)];

   /* trim following spaces */
   while((endptr < startptr) &&
         (isspace(*endptr)))
      --endptr;

   if((endptr - startptr))
      result = hx_strndup(startptr, (endptr - startptr), allocator);

   return result;
}

/* hx_strndup
   memory controlled version of strndup
*/
unsigned char *
hx_strndup(const unsigned char *source, int length, hx_allocator_t *allocator)
{
   unsigned char *result;
   result = (unsigned char *)hx_alloc_mem (length + 1, allocator);
   memcpy(result, source, length);
   result[length] = '\0';
   return result;
}

/* hx_strdup
   memory controlled version of strdup
*/
unsigned char *
hx_strdup(const unsigned char *source, hx_allocator_t *allocator)
{
   unsigned char *result;
   result = (unsigned char *)hx_alloc_mem (strlen(source) + 1, allocator);
   strcpy (result, source);
   return result;
}

/* hx_strcmpi
   OS independent case insensitive
   string comparison
*/
int hx_strcmpi(const unsigned char *arg1, const unsigned char *arg2)
{
#ifdef _HX_WIN32_
   return strcmpi(arg1, arg2);
#elif defined(_HX_UNIX_)
   return strcasecmp(arg1, arg2);
#endif
}

/* hx_strcmpi
   OS independent case insensitive
   string comparison for 'len'
   characters
*/
int hx_strncmpi(const unsigned char *arg1, const unsigned char *arg2, int len)
{
#ifdef _HX_WIN32_
   return strncmpi(arg1, arg2, len);
#elif defined(_HX_UNIX_)
   return strncasecmp(arg1, arg2, len);
#endif
}

/* hx_matchIdx
   finds the index of the word
   in the dictionary provided
   using memory comparison
*/
int
hx_matchIdx(const unsigned char *dict[],
            int dictLen,
            const unsigned char *word,
            int wordLen)
{
   int idx, position = NOMATCH;

   for(idx = 0; idx < dictLen; idx ++)
   {
      if(dict[idx])
      {
         if(!(memcmp(word, dict[idx], wordLen)))
         {
            position = idx;
            break;
         }
      }
   }
   return position;
}

/* hx_matchIdx
   finds the index of the word
   in the dictionary provided
   using string comparison
*/
int
hx_match_string_idx(const unsigned char *dict[],
                    int dictLen,
                    const unsigned char *word)
{
   int idx, position = NOMATCH;

   for(idx = 0; idx < dictLen; idx ++)
   {
      if(dict[idx])
      {
         if(!(hx_strcmpi(word, dict[idx])))
         {
            position = idx;
            break;
         }
      }
   }
   return position;
}

/* hx_getTime
   populates the string supplied
   with the current time
*/
size_t
hx_getTime(unsigned char *timestamp, int len)
{
   time_t now;
   struct tm *tm_now;
   size_t retval;

   const unsigned char *format = "%a, %d %h %Y %H:%M:%S GMT";

   now = time(NULL);
   tm_now = gmtime(&now);
   retval = strftime(timestamp, len, format, tm_now);
   return retval;
}

/* hx_getFileStat
   performs a stat on a specific
   file and returns the result
   in the stat struct supplied
*/
int
hx_getFileStat(hx_logsys_t *log,
               const unsigned char *resource,
               struct stat *rstat,
               hx_lock_t *lock)
{
   const unsigned char *rname = "hx_getFileStat ()";
   int retval;
   int err;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   /* clear the stat struct */
   memset(rstat, 0x00, sizeof(struct stat));

   /* Protect stat function call
      and errno retrieval */
   hx_lock(lock);
   retval = stat(resource, rstat);
   err = errno;
   hx_unlock(lock);

   if(retval)
   {
      /* set the value to zero */
      rstat->st_mode = 0;
      return hx_getSysErrCode(err);
   }

   return OK;
}

/* hx_getResDetails
   Calls a stat-wrapper on a resource
   requested in the HTTP request
   and checks whether the resource
   is a directory or not
*/
int
hx_getResDetails(http_request_t *request_object, hx_lock_t *lock)
{
   const unsigned char *rname = "hx_getResDetails ()";
   int retval;
   int err;
   unsigned char *resource = request_object->filename;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* clear the structure */
   memset(&request_object->resource_stat, 0x00, sizeof(struct stat));

   retval = hx_getFileStat(request_object->vhost->log,
                           resource,
                           &request_object->resource_stat,
                           lock);
   if(OK != retval)
   {
      hx_log_msg(request_object->vhost->log,
                HX_LERRO,
                rname,
                "Error locating \"%s\" [%s]",
                resource,
                strerror(retval));

      return retval;
   }

   if(S_ISDIR(request_object->resource_stat.st_mode))
      request_object->is_dir = HX_TRUE;

   return OK;
}

/* hx_hex_dump
   Generates a hex dump of a
   given buffer depending on
   the loglevel
*/
void
hx_hex_dump(hx_logsys_t *log, unsigned char *buffer, long len, int log_level)
{
   const unsigned char *rname = "hx_hex_dump ()";

   int count;
   int remlen = len;
   int toprint;
   int offset = 0;
   const int linesize = 16;

   /* filler */
   unsigned char filler[MAX_FILLER_LEN];
   unsigned char line[MAX_SMALL_BUFF_LEN];
   unsigned char buf[MAX_SMALL_BUFF_LEN];

   /* space fill the filler */
   memset(filler, SP, MAX_FILLER_LEN);
   memset(line, 0x00, MAX_SMALL_BUFF_LEN);
   memset(buf, 0x00, MAX_SMALL_BUFF_LEN);

   hx_log_msg(log, log_level, " ", "\nSTART DUMP");

   while(remlen > 0)
   {
      if(remlen < linesize)
         toprint = remlen;
      else
         toprint = linesize;

      *line = '\0';

      /* print the leader space */
      strcat(line, "   ");

      for(count = 0; count < toprint; count++)
      {
         sprintf(buf, "%02x ", *(buffer + offset + count ));
         strcat(line, buf);
      }

      /* align the dump */
      if(remlen < linesize)
      {
         sprintf(buf, "%.*s", (linesize - toprint)*3, filler);
         strcat(line, buf);
      }

      strcat(line, "   ");

      for(count = 0; count < toprint; count++)
      {
         if(!isprint(*(buffer + offset + count)))
         {
            strcat(line, ".");
         }
         else
         {
            sprintf(buf, "%c", *(buffer + offset + count));
            strcat(line, buf);
         }
      }
      hx_log_msg(log, log_level, "---", "%s", line);

      offset += toprint;
      remlen -= toprint;
   }

   hx_log_msg(log, log_level, "  ", "END DUMP\n");
}

/*
   hx_removeTrail
   Removes trailing forward slashes from
   a filename. Required for proper stat-ing
*/
void
hx_removeTrail(unsigned char *filename)
{
   unsigned char *endptr = filename + (strlen(filename) - 1);

   while((endptr != filename) &&
         (*endptr == '/'))
   {
      --endptr;
   }

   if(endptr != filename)
      *(endptr + 1) = '\0';
}


/*
   hx_hex2dec
   convert a hex digit into
   its corresponding decimal value
*/
int
hx_hex2dec (char c)
{
   if (c >= '0' && c <= '9')
      return (c - '0');
   else if ( c >= 'A' && c <= 'F')
      return (10 + (c - 'A'));
   else if ( c >= 'a' && c <= 'f')
      return (10 + (c - 'a'));
   else
      return -1;
}

/*
   hx_getSysErrCode
   Converts OS/Library generated errors
   into corresponding httpDex system errors
*/

int hx_getSysErrCode(int errcode)
{
   int retval;
   switch(errcode)
   {
      case ENOENT:
         retval = HX_ERR_FILE_NOEXIST;
         break;
      case EACCES:
         retval = HX_ERR_FILE_FORBIDDEN;
         break;
      default:
         retval = HX_ERR_INTERNAL;
         break;
   }
   return retval;
}

/*
   hx_get_response_code
   Converts internal error codes into
   corresponding HTTP Response codes
*/
int hx_get_response_code(int errcode)
{
   int retval;

   switch(errcode)
   {
      case HX_ERR_REQUEST_URI_LARGE:
         retval = IDX_414;
         break;

      case HX_ERR_INVALID_REQUEST:
      case HX_ERR_BAD_REQUEST:
      case HX_ERR_INVALID_HEADER:
      case HX_ERR_INVALID_METHOD:
      case HX_ERR_INVALID_URI:
         retval = IDX_400;
         break;

      case HX_ERR_FILE_FORBIDDEN:
         retval = IDX_403;
         break;

      case HX_ERR_FILE_NOEXIST:
      case HX_ERR_VHOST_NOEXIST:
         retval = IDX_404;
         break;

      case HX_ERR_MEM_ALLOC:
      case HX_ERR_INSUFF_BUFFLEN:
      case HX_ERR_INTERNAL:
      case HX_ERR_FILE_OPEN:
      case HX_ERR_FILE_READ:
      case HX_ERR_CREATEPIPE:
      case HX_ERR_CREATEPROCESS:
      case HX_ERR_STREAM_NUM:
      case HX_ERR_PIPEFUNC:
      case HX_ERR_PROCESS_CREATE:
      case HX_ERR_HANDLEFUNC:
      case HX_ERR_INTERPRETER:
      case HX_ERR_INDEX_FILE_LIST:
         retval = IDX_500; /* Internal Server Error */
         break;

      default:
         retval = IDX_200;
         break;
   }

   return retval;
}

/* hx_read_file ()
   Reads the contents of a file into memory
   using C buffered routines
*/
int hx_read_file(hx_logsys_t *log,
                const unsigned char *filename,
                unsigned char **buffer,
                long *datalen,
                hx_allocator_t *allocator,
                hx_lock_t *lock)
{
   const unsigned char *rname = "hx_read_file ()";

   FILE *fptr;
   int retval;

   long bytes_to_read;
   long bytes_read;

   struct stat    rstat;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   *datalen = 0;

   /* stat the resource once again */
   retval = hx_getFileStat(log, filename, &rstat, lock);

   if(retval != OK)
   {
      return retval;
   }

   /* get the size of the resource */
   bytes_to_read = rstat.st_size;

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "stat() returned filesize as %d",
             bytes_to_read);


   /* allocate the same size for the buffer */
   if(bytes_to_read > 0)
   {
      hx_log_msg(log,
                HX_LDEBG, rname,
                "Opening file \"%s\"",
                filename);


      hx_lock(lock);
      fptr = fopen(filename, "rb");
      retval = errno;
      hx_unlock(lock);

      if (!fptr)
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "Unable to open file \"%s\" for reading in binary",
                   filename);
         return hx_getSysErrCode(retval);
      }

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "File \"%s\" opened for reading in binary",
                filename);


      *buffer = (unsigned char *)hx_alloc_mem(bytes_to_read, allocator);

      if(!(*buffer))
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "Error allocating memory for file buffer");
         fclose(fptr);
         return HX_ERR_MEM_ALLOC;
      }

      hx_log_msg(log,
                HX_LDEBG,
                rname,
                "Going to read file into buffer");

      /* retrieve the content into the
         response object */
      bytes_read = fread(*buffer, 1, bytes_to_read, fptr);

      /* close the file */
      fclose(fptr);

      /* set the length of the data */
      *datalen = bytes_read;
   }

   return OK;
}

/* hx_convert
   Convert all occurences of a character
   (from) to another character (to) within
   the string (string)
*/
int
hx_convert(hx_logsys_t *log, unsigned char *string, char from, char to)
{
   const unsigned char *rname = "hx_convert ()";

   unsigned char *pch = string;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", string);

   if(!string)
      return ERRC;

   while(*pch)
   {
      if(from == *pch)
         *pch = to;

      ++pch;
   }

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "After converting '%c' to '%c' the string is \"%s\"",
             from,
             to,
             string);

   return OK;
}

/*
   hx_unescape ()
   converts all occurences of %xx
   to its corresponding ascii
   equivalent in string (string)
*/
void
hx_unescape (hx_logsys_t *log,
             unsigned char *string,
             hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_unescape ()";
   unsigned char *startptr = string;
   unsigned char *dup = hx_alloc_mem(strlen(string) + 1, allocator);
   unsigned char *ch = dup;

   char val;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", string);

   while(*startptr)
   {
      if('%' == *startptr)
      {
         val =  (char)(hx_hex2dec(*(++startptr)) * 16);
         val += (char)(hx_hex2dec(*(++startptr)));
         *ch++ = val;
         ++startptr;
      }
      else
         *ch++ = *startptr++;
   }
   *ch = '\0';

   strcpy(string, dup);
   hx_free_mem(dup, allocator);

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "converted string is \"%s\"",
             string);
}

/* hx_escape
   convert all occurences of the
   character (toEsc) within the
   string (source) to its %xx
   representation, and returns
   the converted string (dest)
*/
int
hx_escape(hx_logsys_t *log,
          unsigned char *dest,
          unsigned char *source,
          char toEsc)
{
   const unsigned char *rname = "hx_escape ()";

   unsigned char *psource = source;
   unsigned char *pdest = dest;
   char str[4];

   if (!source || !dest)
      return ERRC;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", source);

   while(*psource)
   {
      if(toEsc == *psource)
      {
         sprintf(str, "%%%x", *psource);

         *pdest++ = str[0];
         *pdest++ = str[1];
         *pdest++ = str[2];
      }
      else
         *pdest++ = *psource;

      ++psource;
   }

   *pdest = '\0';

   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Converted string is \"%s\"",
             dest);

   return OK;
}

/* hx_checkres
   check the resource requested
   for security holes, if not compliant
   return HX_ERR_INVALID_URI */
int
hx_checkres (hx_logsys_t *log,
             unsigned char *resource,
             hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_checkres ()";

   int level = -1;
   unsigned char *dup = hx_strdup(resource, allocator);
   unsigned char *startptr = dup;
   unsigned char *ch;

   hx_log_msg(log, HX_LCALL, rname, "Called on \"%s\"", dup);

   /* quick check */
   if(strlen(startptr) >= 2)
   {
      if(startptr[0] == '/' &&
         startptr[1] == '.' &&
       ((startptr[2] == '.') || (startptr[2] == '\0')))
      {
         hx_free_mem(dup, allocator);
         return HX_ERR_INVALID_URI;
      }
   }

   /* level check */
   while ((*startptr) != '\0')
   {
      /* break resource string
         using '/' */
      if((ch = strchr(startptr, '/')) != NULL)
      {
         *ch = '\0';

         if(!strlen(startptr))
         {
            startptr = ch + 1;
            continue;
         }

         if(!strcmp(startptr, ".."))
            --level;
         else if(strcmp(startptr, "."))
            ++level;

         startptr = ch + 1;
      }
      else
      {
         if(strlen(startptr) > 0)
         {
            if(!strcmp(startptr, ".."))
               --level;
            else
            {
               ++level;
               break;
            }
         }

         ++startptr;
      }
   }

   hx_log_msg(log, HX_LDEBG, rname, "Level is %d", level);

   if(level < -1)
   {
      hx_free_mem(dup, allocator);
      return HX_ERR_INVALID_URI;
   }
   else if (level == -1)
   {
      strcpy(resource, "/");
   }

   hx_free_mem(dup, allocator);

   return OK;
}

/* hx_checkOutput
   Performs a check on the data
   returned by a CGI child process
   and classifies the output into
   "Parsed Headers" or "Non Parsed Headers"
*/
hx_output_t
hx_checkOuput(const unsigned char *buffer, long buffLen)
{
   unsigned char *pch = (unsigned char *)buffer;

   /* skip initial whitespaces if any */
   while(isspace(*pch))
      ++pch;

   if((pch[0] == 'H' || 'h') &&
      (pch[1] == 'T' || 't') &&
      (pch[2] == 'T' || 't') &&
      (pch[3] == 'P' || 'p') &&
      (pch[4] == '/'))
   {
      /* Version OK: advance the ptr */
      pch += 6;

#ifdef _HX_WIN32_
      while(*pch != CR)
         ++pch;

      if((pch[0] == CR) &&
         (pch[1] == LF))
      {
         /* found CRLF terminating the
            status line. Look for the last CRLF
            and confirm it is a NPH */
         while(*pch != CR)
            ++pch;

         if((pch[0] == CR) &&
            (pch[1] == LF))
         {
            return HX_NONPARSED;
         }
      }
#elif defined(_HX_UNIX_)
      while(*pch != LF)
         ++pch;

      if(pch[0] == LF)
      {
         /* found CRLF terminating the
            status line. Look for the last CRLF
            and confirm it is a NPH */
         while(*pch != LF)
            ++pch;

         if(pch[0] == LF)
         {
            return HX_NONPARSED;
         }
      }
#endif
   }

   return HX_PARSED;
}

/* The following function have meaning
   in the WIN32 environment only */

#ifdef _HX_WIN32_

/* hx_isDrive
   Determines whether the given
   resource is a drive or not
*/
int
hx_isDrive(const unsigned char *path)
{
   if(strlen(path) > 4)
      return 0;
   else
   {
      if((path[1] == ':') &&
         (path[2] == '/') &&
         (path[3] == '\0'))
         return 1;
   }

   return 0;
}

/* hx_WIN32Err
   Logs a win32 error into the
   appropriate log system
*/
void
hx_WIN32Err(hx_logsys_t *log)
{
   const unsigned char *rname = "hx_WIN32Err ()";
   unsigned char *error = NULL;
   DWORD errCode = GetLastError();

   FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS),
                  NULL,
                  errCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  (LPTSTR) &error,
                  0,
                  NULL);

   if(error)
   {
      hx_log_msg(log, HX_LERRO, rname, "[%d] %s", errCode, error);
   }
}

/* hx_printWIN32EnvBlock
   Prints the WIN32 environment block
*/
void
hx_printWIN32EnvBlock(hx_logsys_t *log, unsigned char *env_block)
{
   unsigned char *pch = env_block;
   long blockLen;

   do
   {
      blockLen = strlen(pch);
      if(blockLen)
      {
         hx_hex_dump(log, pch, blockLen, HX_LDUMP);
         pch += blockLen + 1;
      }
   }
   while(blockLen);
}

#elif defined(_HX_UNIX_)

/* hx_printUNIXEnvBlock
   Prints the UNIX environment block
*/
void
hx_printUNIXEnvBlock(hx_logsys_t *log, unsigned char **env_block)
{
   unsigned char **pch = env_block;
   long blockLen;

   while(*pch)
   {
      blockLen = strlen(*pch);
      if(blockLen)
      {
         hx_hex_dump(log, *pch, blockLen, HX_LDUMP);
      }
      ++pch;
   }
}

/*
   hx_readNonBlock
   UNIX only
   Read from a pipe on a non
   blocking way
*/
u_long
hx_readNonBlock(int fd, int timeout, void *buf, long buflen)
{
   const char *rname = "hx_readNonBlock ()";

   struct timeval tv;
   unsigned long  todo = 1;
   fd_set         fdset;
   ssize_t        retval;
   int            err;
   int            retry;

   /* set pipe to non blocking mode */
   retval = ioctl(fd, FIONBIO, (u_long *)&todo);

   if(retval)
   {
      /* unable to make pipe non blocking */
      return HX_ERR_IOCTL;
   }

   do
   {
      retry = 0;

      /* reset the values */
      FD_ZERO(&fdset);
      FD_SET(fd, &fdset);

      /* set select timeval, should not
         depend on the values in fdset
         and tv after select returns */
      tv.tv_sec = timeout;
      tv.tv_usec = 0;

      retval = select(fd + 1, &fdset, NULL, NULL, &tv);

      if(!retval)
      {
         /* read timed out. Before returning
            set the buffer state to Blocking
         */
         ioctl(fd, FIONBIO, (u_long *)&todo);
         return HX_ERR_CHILDREAD_TIMED_OUT;
      }
      else
      {
         if(FD_ISSET(fd, &fdset))
         {
            retval = read(fd, buf, buflen);

            /* Error */
            if (retval == 1)
            {
               switch(errno)
               {
                  case EINTR:
                  case EAGAIN:
                     retry = 1;
                     break;
                  case EINVAL:
                  case EBADF:
                  case EFAULT:
                     retval = HX_ERR_SYSTEM;
                     break;
                  default:
                     retval = ERRC;
                     break;
               }
            }
         }
      }
   }
   while (retry);

   /* revert back to original mode */
   ioctl(fd, FIONBIO, (u_long *)&todo);

   return retval;
}

/* hx_writeNonBlock
   UNIX only
   Perform non blocking write on the file
   pointed by the file descriptor fd
*/

u_long
hx_writeNonBlock(int fd, int timeout, void *buf, long buflen)
{
   const char *rname = "hx_writeNonBlock ()";

   struct timeval tv;
   unsigned long  todo = 1;
   fd_set         fdset;
   ssize_t        retval;
   int            err;
   int            retry;

   /* set pipe to non blocking mode */
   retval = ioctl(fd, FIONBIO, (u_long *)&todo);

   if(retval)
   {
      /* unable to make pipe non blocking */
      return HX_ERR_IOCTL;
   }

   /* set todo to 0, blocking mode */
   todo = 0;

   /* set select timeval */
   tv.tv_sec = timeout;
   tv.tv_usec = 0;

   retval = write(fd, buf, buflen);

   if(retval == -1)
   {
      err = errno;

      if(err == EAGAIN)
      {
         do
         {
            retry = 0;
            FD_ZERO(&fdset);
            FD_SET(fd, &fdset);

            retval = select(fd + 1, NULL, &fdset, NULL, &tv);

            if(!retval)
            {
               /* write timed out. Before returning
                  set the pipe state to Blocking
               */
               ioctl(fd, FIONBIO, (u_long *)&todo);
               return HX_ERR_CHILDWRITE_TIMED_OUT;
            }
            else
            {
               if(FD_ISSET(fd, &fdset))
               {
                  retval = write(fd, buf, buflen);
                  if (retval == 1)
                  {
                     retval = ERRC;
                  }
               }
            }
         }
         while (retry);
      }
      else
         retval = HX_ERR_CHILDWRITE;
   }

   /* revert back to original mode */
   ioctl(fd, FIONBIO, (u_long *)&todo);

   return retval;
}

#endif

/* hx_getIndexFile
   Performs a lookup of the index file
   for a specific directory taking into
   account the allowable index files as
   specified in the configuration file.
   These files are supplied to the function
   in form of a list.
*/
int hx_getIndexFile(http_request_t *request_object,
                    hx_list_t *idxList,
                    unsigned char *separator,
                    hx_lock_t *lock)
{
   const unsigned char *rname = "hx_getIndexFile ()";

   /* structure to contain stat
      data */
   struct stat   rstat;

   /* list containing the index files */
   int            idxCount;
   int            i;
   int            retval;

   unsigned char *file;
   unsigned char  absfilename[MAX_FILENAME];

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   idxCount = hx_size(idxList);

   /* error out if list is empty */
   if(idxCount < 1)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "Index file list empty");
      return HX_ERR_INDEX_FILE_LIST;
   }

   /* for each entry in the list
      check for the existence of the
      file
   */
   for (i = 0; i < idxCount; ++i)
   {
      file = (unsigned char *)hx_get_pos(idxList, i);

      if(file)
      {
         /* check for index.html */
         *absfilename = '\0';
         strcat(absfilename, request_object->filename);
         strcat(absfilename, separator);
         strcat(absfilename, file);
         retval = hx_getFileStat(request_object->vhost->log,
                                 absfilename,
                                 &rstat,
                                 lock);
         if(OK == retval)
         {
            break;
         }
      }
   }

   if(OK == retval)
   {
      /* form the actual filename */
      hx_free_mem(request_object->filename, request_object->allocator);
      request_object->filename = hx_strdup(absfilename, request_object->allocator);
   }

   return retval;
}

/* hx_getErrorString
   Generated verbose error string
   supplied the error code
*/
const unsigned char *
hx_getErrorString(int errno)
{
   unsigned char *retstring;

   switch(errno)
   {
   case HX_ERR_INVALID_PARAM:
      retstring = "Invalid parameter";
      break;
   case HX_ERR_INVALID_THRD_HNDL:
      retstring = "Invalid Thread Handle";
      break;
   case HX_ERR_INVALID_THRD_PTR:
      retstring = "Invalid Thread Pointer";
      break;
   case HX_ERR_WAIT_FAILED:
      retstring = "WaitForMultipleObjects() failed";
      break;
   case HX_ERR_JOIN_FAILED_SRCH:
      retstring = "pthread_join() failed [search]";
      break;
   case HX_ERR_JOIN_FAILED_DEADLK:
      retstring = "pthread_join() failed [deadlock]";
      break;
   case HX_ERR_CLOSE_HANDLE:
      retstring = "Invalid Thread Handle";
      break;
   default:
      retstring = "Unknown Error";
      break;
   }

   return retstring;
}

/* OS independent sleep */
void hx_sleep(float secs)
{
   struct timeval t;
   float  seconds = 0;
   int    microsecs = 0;

   if(!secs || secs < 0)
      return;

#ifdef _HX_WIN32_
   Sleep(secs * 1000);
#elif defined(_HX_UNIX_)

   /* extract the seconds and microseconds */

   seconds = (int)secs;
   microsecs = (secs - seconds) * 100;

   t.tv_sec = seconds;
   t.tv_usec = microsecs;

   select(0, NULL, NULL, NULL, &t);
#endif
}

/* hx_isLocalhost
   Determines whether the host being requested
   is the current host or not taking into
   consideration all aliases and ips
*/
int hx_isLocalhost(unsigned char *vhostname)
{
   unsigned char hostname[MAX_HOSTNAME_LEN];
   int count = 0;

   struct hostent *entity;
   struct in_addr address;

   /* get the current system name */
   *hostname = '\0';
   gethostname(hostname, MAX_HOSTNAME_LEN);

   /* check to see if it matches with the
      following */
   if(!hx_strcmpi("127.0.0.1", vhostname) ||
      !hx_strcmpi("localhost", vhostname) ||
      !hx_strcmpi(hostname, vhostname))
   {
      return HX_TRUE;
   }

   /* initialize the address */
   memset(&address, 0x00, sizeof(struct in_addr));

   /* get the host entity
      and resolve the name against
      either the aliased ips or names */
   if(hostname[0])
   {
      entity = gethostbyname(hostname);

      /* check the name against the aliases */
      while(entity->h_aliases[count])
      {
         if(!hx_strcmpi(vhostname, entity->h_aliases[count]))
            return HX_TRUE;
         ++count;
      }

      /* reset count */
      count = 0;

      /* check the name againts the ips*/
      while(entity->h_addr_list[count])
      {
         /* get the address */
         memcpy(&address, entity->h_addr_list[count], entity->h_length);
         if(!hx_strcmpi(vhostname, inet_ntoa(address)))
            return HX_TRUE;
         ++count;
      }
   }

   return HX_FALSE;
}
/* End of file */

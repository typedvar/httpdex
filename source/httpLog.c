/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _HX_WIN32_
#include <windows.h>
#endif

#include "httpMemsys.h"
#include "httpLog.h"
#include "httpUtils.h"
#include "httpConf.h"

static const unsigned char *log_levels_table[HX_MAX_LOG_LVL] =
{
   "ERRO",   "WARN",   "VERB",   "DEBG",
   "MIME",   "CALL",   "REQS",   "RESP",
   "DUMP",   "SYST",   NULL,     NULL,
   NULL,     NULL,     NULL,     NULL,
   NULL,     NULL,     NULL,     NULL,
   NULL,     NULL,     NULL,     NULL,
   NULL,     NULL,     NULL,     NULL,
   NULL,     NULL,     NULL,     NULL
};

static long hx_getMask(int loglvl)
{
   long mask = 1;
   mask = mask << loglvl;
   return mask;
}

static void hx_writeLog(FILE* fp, const unsigned char *msg)
{
   fprintf(fp, "%s\n", msg);
   fflush(fp);
}

int hx_check_log_level(hx_logsys_t *logsys, int log_level)
{
   int retval;

   hx_lock(logsys->lock);
   retval = logsys->mask & hx_getMask(log_level);
   hx_unlock(logsys->lock);

   return retval;
}

hx_logsys_t *
hx_create_log_sys(hx_logsys_t *log,
                const unsigned char *filename,
                const unsigned char *name,
                int needPrefix)
{
   const unsigned char *rname = "hx_create_log_sys ()";
   unsigned char        allocatorName[MAX_ALLOCNAME];

   hx_logsys_t          *logsys;
   hx_allocator_t       *allocator;
   unsigned char        *suffix = " logsys";
   int                  isLogSTDOUT;
   int                  isLogSTDERR;

   *allocatorName = '\0';

   /* Prevent allocator name overflow */
   strncat(allocatorName,
           name,
           (MAX_ALLOCNAME - strlen(suffix) - 1));
   strcat(allocatorName, suffix);

   allocator = hx_create_allocator(allocatorName);

   if(!allocator)
      return NULL;

   logsys = (hx_logsys_t *)hx_alloc_mem(sizeof(hx_logsys_t), allocator);

   if(!logsys)
      return NULL;

   memset(logsys, 0x00, sizeof(hx_logsys_t));

   /* set the allocator */
   logsys->allocator = allocator;

   /* create the lock */
   logsys->lock = hx_create_lock();
   if(!logsys->lock)
   {
      hx_destroy_allocator(logsys->allocator);
      return NULL;
   }

   /* set the logfile name */
   logsys->logfile = hx_strdup(filename, allocator);
   logsys->name = hx_strdup(name, allocator);

   /* init the mask to trap errors
      and warnings and system messages */
   logsys->mask = 515;

   /* check where the logsys has to log
      the output */
   isLogSTDOUT = !(hx_strcmpi(logsys->logfile, "stdout"));
   isLogSTDERR = !(hx_strcmpi(logsys->logfile, "stderr"));

   if(isLogSTDOUT || isLogSTDERR)
   {
      /* the log system has to output
         to either the standard output
         or the standard error */
      if(isLogSTDOUT)
         logsys->fp = stdout;
      else
         logsys->fp = stderr;
   }
   else
   {
      /* the log system has to output to
         a regular file on the filesystem,
         so open the file */
      logsys->fp = fopen(filename, "w+");
   }

   /* Check if prefix is required for
      this log system */
   logsys->needPrefix = needPrefix;

   if(!logsys->fp)
   {
      if(log)
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "Failed to open file %s: %s",
                   filename,
                   strerror(errno));
      else
         fprintf(stderr,
                 "%s : Failed to open file %s: %s",
                 rname,
                 filename,
                 strerror(errno));

      hx_destroy_log_sys(logsys);
      return NULL;
   }

   logsys->logbuf1 = (unsigned char *)hx_alloc_mem(MAX_LOG_BUFF_LEN, allocator);
   logsys->logbuf2 = (unsigned char *)hx_alloc_mem(MAX_LOG_BUFF_LEN, allocator);

   if(HX_LOGPREFIX_YES == needPrefix)
      hx_log_msg( logsys,
                  HX_LSYST,
                  rname,
                  "ONLINE -> Log System %s",
                  logsys->name );

   return logsys;
}

void
hx_destroy_log_sys(hx_logsys_t *logsys)
{
   const unsigned char *rname = "hx_destroy_log_sys ()";

   if(!logsys)
      return;

   if(logsys->fp &&
      (HX_LOGPREFIX_YES == logsys->needPrefix))
      hx_log_msg(logsys,
                HX_LSYST,
                rname,
                "OFFLINE -> Log System %s",
                logsys->name);

   /* destroy the lock */
   if(logsys->lock)
   {
      hx_destroy_lock(logsys->lock);
      logsys->lock = NULL;
   }

   /* close the log file */
   if(logsys->fp)
      fclose(logsys->fp);

   /* garbage collect */
   hx_destroy_allocator(logsys->allocator);
   logsys = NULL;
}

void
hx_reset_log_level(hx_logsys_t *logsys)
{
   hx_lock(logsys->lock);
   logsys->mask = 515;
   hx_unlock(logsys->lock);
}

void
hx_set_log_level(hx_logsys_t *logsys, int loglvl)
{
   hx_lock(logsys->lock);
   logsys->mask |= hx_getMask(loglvl);
   hx_unlock(logsys->lock);
}

void
hx_set_log_mask(hx_logsys_t *logsys, long mask)
{
   hx_lock(logsys->lock);
   logsys->mask = mask;
   hx_unlock(logsys->lock);
}

void
hx_set_log_level_all(hx_logsys_t *logsys)
{
   int i;
   hx_lock(logsys->lock);
   for(i = 0; i < HX_MAX_LOG_LVL; ++i)
   {
      logsys->mask |= hx_getMask(i);
   }
   hx_unlock(logsys->lock);
}

long hx_get_log_mask(hx_logsys_t *logsys)
{
   long retval;
   hx_lock(logsys->lock);
   retval = logsys->mask;
   hx_unlock(logsys->lock);
   return retval;
}

void hx_log_msg( hx_logsys_t *logsys,
                 int loglvl,
                 const unsigned char *func,
                 const unsigned char *format, ... )
{
   va_list l;

   if(!logsys)
      return;

   hx_lock(logsys->lock);

   if(logsys->mask & hx_getMask(loglvl))
   {
      *logsys->logbuf1 = '\0';
      *logsys->logbuf2 = '\0';
      *logsys->level   = '\0';

      va_start(l, format);
      vsprintf(logsys->logbuf1, format, l);
      va_end(l);

      /* Check if prefix is required and
         the log level */
      if(loglvl != HX_LDUMP && logsys->needPrefix)
      {
         *logsys->prefix = '\0';

         /* get the time and the func */
         hx_getTime(logsys->timebuf, MAX_TIME_LEN);
         memset(logsys->funcbuf, ' ', MAX_FUNCNAME_LEN);
         memcpy(logsys->funcbuf, func, strlen(func));
         logsys->funcbuf[MAX_FUNCNAME_LEN - 1] = '\0';

         /* create the prefix */
         strcat(logsys->prefix, "[");
         strcat(logsys->prefix, logsys->timebuf);
         strcat(logsys->prefix, " - ");
         strcat(logsys->prefix, logsys->funcbuf);
         strcat(logsys->prefix, "]");
         strcat(logsys->prefix, ": ");

         sprintf(logsys->level, "[%4s] ", log_levels_table[loglvl]);
         strcat(logsys->logbuf2, logsys->level);
         strcat(logsys->logbuf2, logsys->prefix);
      }
      strcat(logsys->logbuf2, logsys->logbuf1);
      hx_writeLog(logsys->fp, logsys->logbuf2);
   }

   hx_unlock(logsys->lock);
}

int hx_get_log_level_from_string(const unsigned char *str)
{
   return hx_match_string_idx(log_levels_table,
                        HX_MAX_LOG_LVL,
                        str);
}
/* End of File */


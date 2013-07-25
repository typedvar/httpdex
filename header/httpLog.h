/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPLOG_H
#define HTTPLOG_H

#include "httpMemsys.h"
#include "httpLock.h"

/* The various log levels */
typedef enum
{
   HX_LOGLVL_01,
   HX_LOGLVL_02,
   HX_LOGLVL_03,
   HX_LOGLVL_04,
   HX_LOGLVL_05,
   HX_LOGLVL_06,
   HX_LOGLVL_07,
   HX_LOGLVL_08,
   HX_LOGLVL_09,
   HX_LOGLVL_10,
   HX_LOGLVL_11,
   HX_LOGLVL_12,
   HX_LOGLVL_13,
   HX_LOGLVL_14,
   HX_LOGLVL_15,
   HX_LOGLVL_16,
   HX_LOGLVL_17,
   HX_LOGLVL_18,
   HX_LOGLVL_19,
   HX_LOGLVL_20,
   HX_LOGLVL_21,
   HX_LOGLVL_22,
   HX_LOGLVL_23,
   HX_LOGLVL_24,
   HX_LOGLVL_25,
   HX_LOGLVL_26,
   HX_LOGLVL_27,
   HX_LOGLVL_28,
   HX_LOGLVL_29,
   HX_LOGLVL_30,
   HX_LOGLVL_31,
   HX_LOGLVL_32,
   HX_MAX_LOG_LVL
}hx_loglvl_t;

#define HX_LERRO        HX_LOGLVL_01
#define HX_LWARN        HX_LOGLVL_02
#define HX_LVERB        HX_LOGLVL_03
#define HX_LDEBG        HX_LOGLVL_04
#define HX_LDMIM        HX_LOGLVL_05
#define HX_LCALL        HX_LOGLVL_06
#define HX_LREQS        HX_LOGLVL_07
#define HX_LRESP        HX_LOGLVL_08
#define HX_LDUMP        HX_LOGLVL_09
#define HX_LSYST        HX_LOGLVL_10

/* The log system structure */
typedef struct
{
   hx_allocator_t    *allocator;
   unsigned char     *name;
   unsigned char     *logfile;
   FILE              *fp;

   hx_lock_t         *lock;

   unsigned char      prefix [MAX_LOG_PREFIX_LEN];
   unsigned char      timebuf[MAX_TIME_LEN];
   unsigned char      funcbuf[MAX_FUNCNAME_LEN];
   unsigned char      level  [MAX_LVLSTR_LEN + 1];
   unsigned char     *logbuf1;
   unsigned char     *logbuf2;

   long               mask;
   int                needPrefix;
}hx_logsys_t;

extern hx_logsys_t   *server_log_sys;

/* Function declarations */
int hx_check_log_level(hx_logsys_t *logsys, int log_level);

hx_logsys_t * hx_create_log_sys(hx_logsys_t *log,
                              const unsigned char *filename,
                              const unsigned char *name,
                              int needPrefix);

void hx_destroy_log_sys(hx_logsys_t *logsys);

void hx_reset_log_level(hx_logsys_t *logsys);

int hx_get_log_level_from_string(const unsigned char *str);

void hx_set_log_level(hx_logsys_t *logsys, int loglvl);

void hx_set_log_level_all(hx_logsys_t *logsys);

void hx_set_log_mask(hx_logsys_t *logsys, long mask);

long hx_get_log_mask(hx_logsys_t *logsys);

void hx_log_msg(hx_logsys_t *logsys,
               int loglvl,
               const unsigned char *func,
               const unsigned char *format, ...);

#endif
/* End of File */

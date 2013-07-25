/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <stdlib.h>

#include "httpRequest.h"
#include "httpList.h"
#include "httpThreads.h"

/* Function declarations */
unsigned char *hx_strndup(const unsigned char *source,
                          int length,
                          hx_allocator_t *allocator);

unsigned char *hx_strdup(const unsigned char *source,
                         hx_allocator_t *allocator);

unsigned char *hx_trim(unsigned char *source, hx_allocator_t *allocator);


int hx_strcmpi(const unsigned char *arg1, const unsigned char *arg2);

int hx_strncmpi(const unsigned char *arg1, const unsigned char *arg2, int len);

size_t hx_getTime(unsigned char *timestamp, int len);

int hx_matchIdx(const unsigned char *dict[],
                int dictLen,
                const unsigned char *word,
                int wordLen);

int hx_match_string_idx(const unsigned char *dict[],
                   int dictLen,
                   const unsigned char *word);

int hx_getResDetails(http_request_t *request_object, hx_lock_t *lock);

int hx_getFileStat(hx_logsys_t *log,
                   const unsigned char *resource,
                   struct stat *rstat,
                   hx_lock_t *lock);

void hx_hex_dump(hx_logsys_t *log, unsigned char *buffer, long len, int log_level);

int hx_createPage(unsigned char *buffer,
                  const unsigned char *title,
                  const unsigned char *body,
                  long bufflen,
                  hx_lock_t *lock);

#ifdef _HX_WIN32_
int hx_isDrive(const unsigned char *path);
#endif

void hx_removeTrail(unsigned char *filename);

int hx_hexval(char c);

void hx_unescape (hx_logsys_t *log,
                  unsigned char *string,
                  hx_allocator_t *allocator);

int hx_getIndexFile(http_request_t *request_object,
                    hx_list_t *idxList,
                    unsigned char *separator,
                    hx_lock_t *lock);

int hx_escape(hx_logsys_t *log,
              unsigned char *dest,
              unsigned char *source,
              char toEsc);

int hx_convert(hx_logsys_t *log,
               unsigned char *string,
               char from,
               char to);

int hx_getSysErrCode(int errcode);

int hx_get_response_code(int errcode);

int hx_read_file(hx_logsys_t *log,
                const unsigned char *filename,
                unsigned char **buffer,
                long *datalen,
                hx_allocator_t *allocator,
                hx_lock_t *lock);

int hx_checkres (hx_logsys_t *log,
                 unsigned char *resource,
                 hx_allocator_t *allocator);

int hx_isLocalhost(unsigned char *vhostname);

hx_output_t hx_checkOuput(const unsigned char *buffer, long buffLen);

const unsigned char *hx_getErrorString(int errno);

void hx_sleep(float secs);

u_long hx_readNonBlock(int fd, int timeout, void *buf, long buflen);

u_long hx_writeNonBlock(int fd, int timeout, void *buf, long buflen);

#ifdef _HX_WIN32_

void hx_WIN32Err(hx_logsys_t *log);
void hx_printWIN32EnvBlock(hx_logsys_t *log, unsigned char *env_block);

#elif defined(_HX_UNIX_)

void hx_printUNIXEnvBlock(hx_logsys_t *log, unsigned char **env_block);

#endif

#endif
/* End of File */

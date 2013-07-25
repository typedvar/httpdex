/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPENV_H
#define HTTPENV_H

#include "httpMemsys.h"
#include "httpList.h"

/* single environment
   entity */
typedef struct
{
   unsigned char *nameval;
}hx_envpair_t;

/* Function Declarations */
void hx_env_printer(void *data,
                   unsigned char **line,
                   hx_allocator_t *allocator);

int hx_env_from_body (hx_logsys_t *log,
                    hx_list_t *environ,
                    const unsigned char *body,
                    hx_allocator_t *allocator);

hx_envpair_t* hx_create_env_pair (hx_logsys_t *log,
                                const unsigned char *name,
                                const unsigned char *value,
                                hx_allocator_t *allocator);

unsigned char * hx_create_env_element(hx_logsys_t *log,
                               const unsigned char *elem,
                               hx_allocator_t *allocator);

int hx_create_win32_env_block(hx_list_t *environ,
                      unsigned char **env_block,
                      hx_allocator_t *allocator);

int hx_create_unix_env_block(hx_list_t *environ,
                      unsigned char ***env_block,
                      hx_allocator_t *allocator);

#endif
/* End of File */

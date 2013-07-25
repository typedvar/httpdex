/*
 * Copyright (c) 2003 Sengupta. All rights reserved.
 *
 */

#ifndef HTTPMEMSYS_H
#define HTTPMEMSYS_H

#include "httpCommon.h"

typedef struct strdata_t   hx_mem_t;

struct strdata_t
{
#ifdef HX_DEBUG_MEMSYS
   unsigned char  file[MAX_FILENAME];
   int            line;
   int            reuseCount;
#endif

   size_t         requested;
   size_t         allocated;
   hx_bool_t      isFree;
   hx_mem_t      *next;
   void          *block;
};

typedef struct
{

#ifdef HX_DEBUG_MEMSYS
   unsigned char  file[MAX_FILENAME];
   int            line;
#endif

   unsigned char  name[MAX_ALLOCNAME];

   /* pool id */
   long           poolId;

   /* head of data list */
   hx_mem_t      *head;

   /* tail of data list */
   hx_mem_t      *tail;

   /* num of elements */
   long           numElts;
}hx_allocator_t;

/* Function Declarations */

/* If memory system debugging is turned on.
   This is a compile time feature */
#ifdef HX_DEBUG_MEMSYS

#define hx_create_allocator(x)  hx_create_allocator_debug(x,__FILE__,__LINE__)
#define hx_destroy_allocator(x) hx_destroy_allocator_debug(x,__FILE__,__LINE__)

#define hx_alloc_mem(x,y)     hx_alloc_mem_debug(x,y,__FILE__,__LINE__)
#define hx_free_mem(x,y)      hx_free_mem_debug(x,y,__FILE__,__LINE__)

hx_allocator_t *hx_create_allocator_debug(const unsigned char *name,
                                 const unsigned char *file,
                                 int line);
int hx_destroy_allocator_debug(hx_allocator_t *allocator, const unsigned char *file, int line);

void *hx_alloc_mem_debug(size_t n, hx_allocator_t *allocator, const unsigned char *file, int line);
int   hx_free_mem_debug(void *ptr, hx_allocator_t *allocator, const unsigned char *file, int line);

#else

hx_allocator_t *hx_create_allocator(const unsigned char *name);
int hx_destroy_allocator(hx_allocator_t *allocator);

void *hx_alloc_mem(size_t n, hx_allocator_t *allocator);
int   hx_free_mem(void *ptr, hx_allocator_t *allocator);

#endif

#endif
/* End of File */

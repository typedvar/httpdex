/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include "httpMemsys.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define HX_WORDSIZE 16

static const unsigned char signature[HX_WORDSIZE + 1] = "httpDeXMEMORYsYs";
static const unsigned char pattern[HX_WORDSIZE + 1]   = "-|-|-|-|-|-|-|-|";

static hx_mem_t *hx_createMemNode(size_t sz)
{
   hx_mem_t *data;

   data = (hx_mem_t *)malloc(sizeof(hx_mem_t));

   if(!data)
      return NULL;

   /* init the mem */
   memset(data, 0x00, sizeof(hx_mem_t));

   /* allocate the block */
   data->block = malloc(sz);

   if(!data->block)
      return NULL;

   return data;
}

static hx_mem_t *
hx_searchFreeNode(size_t sz, hx_allocator_t *allocator)
{
   hx_mem_t *data;

   if(!allocator)
      return NULL;

   data = allocator->head;

   while(data)
   {
      if((data->allocated == sz) && (data->isFree == HX_TRUE))
         break;

      data = data->next;
   }

   return data;
}

static hx_mem_t *
hx_searchNode(void *ptr, hx_allocator_t *allocator)
{
   hx_mem_t *data;

   if(!allocator)
      return NULL;

   data = allocator->head;

   while(data)
   {
      if(data->block == ptr)
         break;

      data = data->next;
   }

   return data;
}

#ifdef HX_DEBUG_MEMSYS

static int
hx_detectOverwrite(hx_mem_t *data)
{
   int signwrite = 0;
   int patwrite = 0;
   void *patstart;
   void *signstart;
   size_t patlen;
   int retval = 0;

   /* calculate the patstart */
   signstart = (char *)data->block + data->requested;

   /* check for sign overwrite */
   signwrite = memcmp(signstart, signature, strlen(signature));

   /* check for pattern overwrite */
   if(data->allocated > (data->requested + strlen(signature)))
   {
      patstart = (char *)data->block + data->requested + strlen(signature);
      patlen = data->allocated - (data->requested + strlen(signature));
      patwrite = memcmp(patstart, pattern, patlen);
   }

   if(signwrite)
      retval += -1;

   if(patwrite)
      retval += -2;

   switch(retval)
   {
   case -1:
      fprintf(stderr,
              "%-55s - OVERWRITE WARNING [0x%08x]: SIGN             [ CREATED At %-30s : %04d ]\n",
              " ",
              data->block,
              data->file,
              data->line);
      break;
   case -2:
      fprintf(stderr,
              "%-55s - OVERWRITE WARNING [0x%08x]: PATTERN          [ CREATED At %-30s : %04d ]\n",
              " ",
              data->block,
              data->file,
              data->line);
      break;

   case -3:
      fprintf(stderr,
              "%-55s - OVERWRITE WARNING [0x%08x]: SIGN and PATTERN [ CREATED At %-30s : %04d ]\n",
              " ",
              data->block,
              data->file,
              data->line);
      break;
   }

   return retval;
}

hx_allocator_t *
hx_create_allocator_debug(const unsigned char *name,
                 const unsigned char *file,
                 int line)
{
   const unsigned char *rname = "CREATE ";
   hx_allocator_t *ptr;

   fprintf(stderr,
           "%-8s [ From %-30s : %04d ] : ----- \"%-24s\" -----\n",
           rname,
           file,
           line,
           name);

   ptr = (hx_allocator_t *)malloc(sizeof(hx_allocator_t));
   memset(ptr, 0x00, sizeof(hx_allocator_t));

   /* stamp position of allocator creation and its name */
   strncpy(ptr->name, name, MAX_ALLOCNAME - 1);
   strncpy(ptr->file, file, MAX_FILENAME - 1);
   ptr->line = line;

   return ptr;
}

int
hx_destroy_allocator_debug(hx_allocator_t *allocator,
                  const unsigned char *file,
                  int line)
{
   const unsigned char *rname = "DELETE ";
   int totalReuse = 0;
   int totalBytesReused = 0;
   int retval;

   hx_mem_t *data, *nxt;

   fprintf(stderr,
           "%-8s [ From %-30s : %04d ] : ----- \"%-24s\" [ CREATED At %-30s : %04d ]\n",
           rname,
           file,
           line,
           allocator->name,
           allocator->file,
           allocator->line);

   if (!allocator)
      return HX_ERR_INVALID_ALLOCATOR;

   data = allocator->head;

   while(data)
   {
      if(data->reuseCount)
      {
         totalBytesReused += (data->reuseCount * data->requested);

         fprintf(stderr,
                 "%-55s - %7d BYTES ->  REUSED / %5d REUSE COUNT\n",
                 " ",
                 (data->reuseCount * data->requested),
                 data->reuseCount);

         /* increment the total reuse count
            for this allocator */
         totalReuse += data->reuseCount;
      }

      /* free the current block */
      if(data->block)
      {
         retval = hx_detectOverwrite(data);

         if(!retval)
         {
            free(data->block);
         }
      }

      /* get the next elem in the list */
      nxt = data->next;

      /* free the node */
      free(data);

      /* null the node */
      data = nxt;
   }

   fprintf(stderr,
           "%-8s [ From %-30s : %04d ] : %7d BYTES ->  REUSED / %5d REUSE COUNT\n",
           rname,
           file,
           line,
           totalBytesReused,
           totalReuse);

   /* delete the allocator */
   free(allocator);

   return OK;
}

void *
hx_alloc_mem_debug(size_t n,
           hx_allocator_t *allocator,
           const unsigned char *file,
           int line)
{
   const unsigned char *rname = "ALLOC ";

   hx_mem_t *data;
   size_t   actualSize;
   size_t   temp;
   size_t   patternlen;

   fprintf(stderr,
           "%-8s [ From %-30s : %04d ] : %7d BYTES -> ",
           rname,
           file,
           line,
           n);

   if(!allocator)
      return NULL;

   /* The actual allocation size is a
      multiple of the word size, to minimize
      fragmentation, and enhance reuse and big
      enough to contain the system signature */

   temp = n + strlen(signature);

   if(temp <= HX_WORDSIZE)
      actualSize = HX_WORDSIZE;
   else
   {
      if(temp % HX_WORDSIZE)
         actualSize = temp + (HX_WORDSIZE - (temp % HX_WORDSIZE));
      else
         actualSize = temp;
   }

   /* First search a node
      which is free and is enough big */
   data = hx_searchFreeNode(actualSize, allocator);

   if(data)
   {
      /* update the node status */
      data->isFree = HX_FALSE;

      /* increment the reuse count */
      ++data->reuseCount;

      fprintf(stderr, " FOUND       ");
   }
   else
   {
      fprintf(stderr, " NOT FOUND   ");

      /* Node of the required
         size is not found create
         a new node */
      data = hx_createMemNode(actualSize);

      if(!data)
         return NULL;

      /* Initialize the reuse count */
      data->reuseCount  = 0;

      if(allocator->tail)
         allocator->tail->next = data;
      else
      {
         allocator->head = data;
         allocator->tail = data;
      }

      /* update the new tail */
      allocator->tail = data;
      ++allocator->numElts;
   }

   /* Update Node Status */
   data->isFree      = HX_FALSE;
   data->allocated   = actualSize;
   data->requested   = n;
   strcpy(data->file, file);
   data->line = line;

   /* Allocated block ptr address */
   fprintf(stderr, "[0x%08x]\n", data->block);

   /* copy the signature to the
      block end to detect overwrite */
   memcpy((char *)data->block + n, signature, strlen(signature));

   /* copy the pattern to fill in the
      remaining space */
   if(actualSize > temp)
   {
      patternlen = actualSize - temp;
      memcpy((char *)data->block + temp, pattern, patternlen);
   }

   return data->block;
}

int
hx_free_mem_debug(void *ptr,
          hx_allocator_t *allocator,
          const unsigned char *file,
          int line)
{
   const unsigned char *rname = "FREE ";
   hx_mem_t *data;
   int retval;

   fprintf(stderr,
           "%-8s [ From %-30s : %04d ] : ",
           rname,
           file,
           line);

   if(!allocator)
      return HX_ERR_INVALID_ALLOCATOR;

   data = hx_searchNode(ptr, allocator);

   if(!data)
   {
      fprintf(stderr, "NODE NOT FOUND\n",
              " ");

      return HX_ERR_INVALID_FREE;
   }

   fprintf(stderr,
           "%7d BYTES ->  ALLOCATED   [0x%08x] %-20s:%04d\n",
           data->requested,
           data->block,
           data->file,
           data->line);

   /* Update the node status */
   if(data->isFree == HX_TRUE)
   {
      fprintf(stderr,
              "%-56s: ALREADY FREED\n",
              " ");
   }
   else
      data->isFree = HX_TRUE;

   return OK;
}
#else
hx_allocator_t *hx_create_allocator(const unsigned char *name)
{
   hx_allocator_t *ptr;

   ptr = (hx_allocator_t *)malloc(sizeof(hx_allocator_t));
   memset(ptr, 0x00, sizeof(hx_allocator_t));

   /* stamp the allocator name */
   strncpy(ptr->name, name, MAX_ALLOCNAME - 1);
   return ptr;
}

int hx_destroy_allocator(hx_allocator_t *allocator)
{
   hx_mem_t *data, *nxt;

   if (!allocator)
      return HX_ERR_INVALID_ALLOCATOR;

   data = allocator->head;

   while(data)
   {
      /* free the current block */
      if(data->block)
         free(data->block);

      /* get the next elem in the list */
      nxt = data->next;

      /* free the node */
      free(data);

      /* null the node */
      data = nxt;
   }

   /* delete the allocator */
   free(allocator);

   return OK;
}

void *hx_alloc_mem(size_t n, hx_allocator_t *allocator)
{
   hx_mem_t *data;
   size_t   actualSize;

   if(!allocator)
      return NULL;

   /* The actual allocation size is a
      multiple of the word size, to minimize
      fragmentation, and enhance reuse */
   if(n <= HX_WORDSIZE)
      actualSize = HX_WORDSIZE;
   else
   {
      if(n % HX_WORDSIZE)
         actualSize = n + (HX_WORDSIZE - (n % HX_WORDSIZE));
      else
         actualSize = n;
   }

   /* First search a node
      which is free and is enough big */
   data = hx_searchFreeNode(actualSize, allocator);

   if(data)
   {
      /* update the node status */
      data->isFree = HX_FALSE;
   }
   else
   {
      /* Node of the required
         size is not found create
         a new node */
      data = hx_createMemNode(actualSize);

      if(!data)
         return NULL;

      if(allocator->tail)
         allocator->tail->next = data;
      else
      {
         allocator->head = data;
         allocator->tail = data;
      }

      /* update the new tail */
      allocator->tail = data;
      ++allocator->numElts;
   }

   /* Update Node Status */
   data->isFree      = HX_FALSE;
   data->allocated   = actualSize;
   data->requested   = n;

   return data->block;
}

int hx_free_mem(void *ptr, hx_allocator_t *allocator)
{
   const unsigned char *rname = "FREE ";
   hx_mem_t *data;

   if(!allocator)
      return HX_ERR_INVALID_ALLOCATOR;

   data = hx_searchNode(ptr, allocator);

   if(!data)
   {
      return HX_ERR_INVALID_FREE;
   }

   /* Update the node status */
   if(data->isFree == HX_TRUE)
   {
      fprintf(stderr, "FREEING ALREADY FREED NODE\n");
   }
   else
      data->isFree = HX_TRUE;

   return OK;
}
#endif
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include "httpCommon.h"
#include "httpInclude.h"
#include "httpList.h"

static node_t *
hx_createNode(void *data, hx_allocator_t *allocator)
{
   node_t *node;

   node = (node_t *)hx_alloc_mem(sizeof(node_t), allocator);

   if(node)
   {
      node->elem = data;
   }

   return node;
}

static void *
hx_deleteNode(node_t *node, hx_allocator_t *allocator)
{
   void *data;

   if (!node)
      return NULL;

   data = node->elem;
   hx_free_mem(node, allocator);

   return data;
}

hx_list_t *
hx_create_list()
{
   hx_list_t *list;
   hx_allocator_t *allocator;

   /* create the allocator */
   allocator = hx_create_allocator("list");

   if(!allocator)
      return NULL;

   list = (hx_list_t *)hx_alloc_mem(sizeof(hx_list_t), allocator);

   if(!list)
   {
      hx_destroy_allocator(allocator);
      allocator = NULL;
      return NULL;
   }

   /* zero all the constituent elements */
   memset(list, 0x00, sizeof(hx_list_t));

   list->allocator = allocator;

   list->lock = hx_create_lock();
   if(!list->lock)
      return NULL;

   return list;
}

void
hx_destroy_list(hx_list_t *list)
{
   hx_allocator_t *allocator;

   if(!list)
      return;

   allocator = list->allocator;

   /* destroy the lock */
   if (list->lock)
   {
      hx_destroy_lock(list->lock);
      list->lock = NULL;
   }

   hx_destroy_allocator(allocator);
   allocator = NULL;
}

/* appends to the end of the list */
int
hx_add(hx_list_t *list, void *data)
{
   int retval = ERRC;
   node_t *node;

   if(!list)
      return retval;

   hx_lock(list->lock);
   node = hx_createNode(data, list->allocator);

   if(node)
   {
      /* create the head node ? */
      if(!list->head)
      {
         list->head = node;
         list->head->nxt = NULL;
         list->head->prv = NULL;
      }
      else
      {
         /* create tail node ? */
         if(!list->tail)
         {
            list->tail = node;
            /* establish forward pointers */
            list->head->nxt = list->tail;
            list->tail->nxt = NULL;

            /* establish reverse pointers */
            list->tail->prv = list->head;
         }
         else
         {
            /* regular node addition */
            /* establish forward pointers */
            list->tail->nxt = node;
            node->nxt = NULL;

            /* establish reverse pointers */
            node->prv = list->tail;
            list->tail = node;
         }
      }
      ++list->entries;
      retval = OK;
   }
   hx_unlock(list->lock);

   return retval;
}

int
hx_add_first(hx_list_t *list, void *data)
{
   node_t *node;
   int retval = ERRC;

   if(!list)
      return retval;

   hx_lock(list->lock);
   node = hx_createNode(data, list->allocator);

   if(node)
   {
      /* no head node ? */
      if(!list->head)
      {
         list->head = node;
         list->head->nxt = NULL;
         list->head->prv = NULL;
      }
      else
      {
         /* no tail node ?
            make current head the tail
            and new node as head */
         if(!list->tail)
         {
            /* establish forward pointers */
            node->nxt = list->head;

            /* establish reverse pointers */
            node->prv = NULL;
            list->head->prv = node;

            /* swap head and tail */
            list->tail = list->head;

            /* make node current head */
            list->head = node;
         }
         else
         {
            /* regular node addition */
            /* establish forward pointers */
            node->nxt = list->head;

            /* establish reverse pointers */
            node->prv = NULL;
            list->head->prv = node;

            /* make node current head */
            list->head = node;
         }
      }
      ++list->entries;
      retval = OK;
   }
   hx_unlock(list->lock);

   return retval;
}

/*int hx_add_pos(hx_list_t *list, void *data, int position)*/
/*{*/
/*   if(!list)*/
/*      return ERRC;*/
/*   return ERRC;*/
/*}*/

/* remove from tail */
void *
hx_remove(hx_list_t *list)
{
   void *data = NULL;
   node_t *node;

   if(!list)
      return NULL;

   hx_lock(list->lock);
   node = list->tail;
   if(node)
   {
      /* make the next of the tail NULL */
      node->prv->nxt = NULL;
      list->tail = node->prv;

      /* get the data and free the node */
      data = hx_deleteNode(node, list->allocator);
   }
   --list->entries;
   hx_unlock(list->lock);

   return data;
}

void *
hx_remove_first(hx_list_t *list)
{
   void *data = NULL;
   node_t *node;

   if(!list)
      return NULL;

   hx_lock(list->lock);
   node = list->head;

   if(node)
   {
      node->nxt->prv = NULL;
      list->head = node->nxt;

      /* get the data and free the node */
      data = hx_deleteNode(node, list->allocator);
      --list->entries;
   }
   hx_unlock(list->lock);

   return data;
}

/*void *hx_remove_pos(hx_list_t *list, int position)*/
/*{*/
/*   if(!list)*/
/*      return ERRC;*/
/*   */
/*   return NULL;*/
/*}*/

/* get the value of the last node */
void *
hx_get(hx_list_t *list)
{
   void *data = NULL;

   if(!list)
      return NULL;

   hx_lock(list->lock);

   if(list->tail)
      data = list->tail->elem;
   else if(list->head)
      data = list->head->elem;
   hx_unlock(list->lock);

   return data;
}

void *
hx_get_first(hx_list_t *list)
{
   void *data = NULL;

   if(!list)
      return NULL;

   hx_lock(list->lock);
   if(list->head)
      data = list->head->elem;
   hx_unlock(list->lock);

   return data;
}

void *
hx_get_pos(hx_list_t *list, int position)
{
   void *data = NULL;
   node_t *node;
   int count;

   if(!list)
      return NULL;

   hx_lock(list->lock);

   node = list->head;

   if(position < list->entries && node)
   {
      for(count = 0; count < position; count++)
      {
         node = node->nxt;
      }

      if(node)
         data = node->elem;
   }

   hx_unlock(list->lock);
   return data;
}

void *hx_find(hx_list_t *list, void *data, fcomparator_t func)
{
   void *obj;
   int count, entries;
   void *result = NULL;

   if(!list)
      return NULL;

   entries = hx_size(list);

   for(count = 0; count < entries; count++)
   {
      obj = hx_get_pos(list, count);

      /* use comparator */
      if(obj)
      {
         /* the function should return
            0 on successful match */
         if(!(*func)(data, obj))
         {
            result = obj;
            break;
         }
      }
   }

   return result;
}

int
hx_size(hx_list_t *list)
{
   int retval;
   if(!list)
      return ERRC;

   hx_lock(list->lock);
   retval = list->entries;
   hx_unlock(list->lock);

   return retval;
}

void
hx_print(hx_logsys_t *log, hx_list_t *list, fprinter_t printer, int log_level)
{
   const unsigned char *rname = "hx_print ()";
   void *data;
   int count, entries;
   unsigned char *line;

   entries = hx_size(list);

   for(count = 0; count < entries; count++)
   {
      data = hx_get_pos(list, count);
      if(data)
      {
         (*printer)(data, &line, list->allocator);
         hx_log_msg(log, log_level, rname, "%s", line);
      }
   }
}
/* End of File */

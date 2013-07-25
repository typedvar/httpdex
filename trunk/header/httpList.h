/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPLIST_H
#define HTTPLIST_H

#include "httpCommon.h"
#include "httpMemsys.h"
#include "httpVhost.h"
#include "httpThreads.h"

typedef enum
{
   HX_ERR_LISTCREATE,
   HX_ERR_LISTADD,
   HX_ERR_LISTDEL,
   HX_ERR_LISTREM
}HX_LIST_ERR_CODES;

typedef struct node_t_ node_t;
struct node_t_
{
   node_t *prv;
   node_t *nxt;
   void   *elem;
};

typedef struct
{
   /* allocator */
   hx_allocator_t *allocator;
   node_t         *head;
   node_t         *tail;
   int            entries;

   /* sync mechanism */
   hx_lock_t      *lock;
}hx_list_t;

typedef int (*fcomparator_t)(void *arg1, void *arg2);
typedef void (*fprinter_t)(void *data,
                          unsigned char **line,
                          hx_allocator_t *allocator);

/* Function Declarations */
hx_list_t *hx_create_list(void);
void hx_destroy_list(hx_list_t *ptr);

/* appends to the end of the list */
int hx_add(hx_list_t *list, void *data);
/* appends to the front of the list */
int hx_add_first(hx_list_t *list, void *data);
/* insert an element at a particular position */
int hx_add_pos(hx_list_t *list, void *data, int position);

/* removes from the list end */
void *hx_remove(hx_list_t *list);
void *hx_remove_first(hx_list_t *list);
void *hx_remove_pos(hx_list_t *list, int position);

/* gets the list end */
void *hx_get(hx_list_t *list);
void *hx_get_first(hx_list_t *list);
void *hx_get_pos(hx_list_t *list, int position);

/* clears the list */
void  hx_clear(hx_list_t *list);

/* prints the list contents using the printer provided */
void  hx_print(hx_logsys_t *log,
               hx_list_t *list,
               fprinter_t printer,
               int log_level);

/* performs sequential search on the list
   using the comparator provided */
void *hx_find(hx_list_t *list, void *data, fcomparator_t func);

/* return the size of the list */
int hx_size(hx_list_t *list);

#endif
/* End of File */

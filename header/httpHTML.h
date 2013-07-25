/* 
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */
 
#ifndef HTTPHTML_H
#define HTTPHTML_H

#include "httpMemsys.h"

unsigned char *
hx_paginate(hx_logsys_t *log,
            const unsigned char *title, 
            const unsigned char *body,
            hx_allocator_t *allocator);

#endif
/* End of File */

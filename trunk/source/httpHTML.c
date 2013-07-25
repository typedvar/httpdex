/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <string.h>

#include "httpCommon.h"
#include "httpInclude.h"
#include "httpHTML.h"

unsigned char *
hx_paginate(hx_logsys_t *log,
            const unsigned char *title,
            const unsigned char *body,
            hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_paginate ()";
   long reqlen;

   unsigned char *htmlstart = "<html><head><title>";
   unsigned char *titleend  = "</title>";
   unsigned char *headend   = "</head>";
   unsigned char *bodystart = "<body "
                              "bottommargin = 0 "
                              "leftmargin   = 0 "
                              "rightmargin  = 0 "
                              "topmargin    = 0 "
                              "marginwidth  = 0 "
                              "marginheight = 0>"
                              "<img src=\"/logo.gif\">"
                              "<hr>";
   unsigned char *htmlend   = "</body></html>";

   /* link to the default style sheet */
   unsigned char *link = "<link rel=\"stylesheet\" href=\"/style/style.css\" type=\"text/css\">";

   unsigned char *page;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   reqlen = strlen(htmlstart) +
            strlen(title)     +
            strlen(titleend)  +
            strlen(link)      +
            strlen(headend)   +
            strlen(bodystart) +
            strlen(body)      +
            strlen(htmlend);

   /* allocate required space */
   page = hx_alloc_mem(reqlen + 1, allocator);

   if(!page)
   {
      return NULL;
   }

   *page = '\0';

   strcat(page, htmlstart);
   strcat(page, title);
   strcat(page, titleend);
   strcat(page, link);
   strcat(page, headend);
   strcat(page, bodystart);
   strcat(page, body);
   strcat(page, htmlend);

   return page;
}
/* End of File */

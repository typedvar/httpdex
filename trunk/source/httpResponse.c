/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "httpSock.h"
#include "httpCommon.h"
#include "httpInclude.h"
#include "httpResponse.h"
#include "httpRequest.h"
#include "httpConf.h"
#include "httpHTML.h"
#include "httpDirlister.h"

const unsigned char *errPageTitle = "httpdEx/1.0 - Error Report ";
const unsigned char *errPageBody1 = "         <table width=100%>"
                                    "            <tr>"
                                    "               <td valign=middle align=middle bgcolor=#dddddd>"
                                    "                  <h2>"
                                    "                     httpdEx/1.0 - Error report "
                                    "                  </h2>"
                                    "               </td>"
                                    "            </tr>"
                                    "            <tr>"
                                    "               <td valign=middle align=middle bgcolor=#ffffff>"
                                    "                  <h3>";
const unsigned char *errPageBody2 = "                  </h3>"
                                    "               </td>"
                                    "            </tr>"
                                    "         </table>";

const http_response_code_t Responses[MAX_RESPONSE_CODES] =
{
   {100, "Continue"                         },
   {101, "Switching Protocols"              },
   {200, "OK"                               },
   {201, "Created"                          },
   {202, "Accepted"                         },
   {203, "Non-Authoritative Information"    },
   {204, "No Content"                       },
   {205, "Reset Content"                    },
   {206, "Partial Content"                  },
   {300, "Multiple Choices"                 },
   {301, "Moved Permanently"                },
   {302, "Found"                            },
   {303, "See Other"                        },
   {304, "Not Modified"                     },
   {305, "Use Proxy"                        },
   {307, "Temporary Redirect"               },
   {400, "Bad Request"                      },
   {401, "Unauthorized"                     },
   {402, "Payment Required"                 },
   {403, "Forbidden"                        },
   {404, "Not Found"                        },
   {405, "Method Not Allowed"               },
   {406, "Not Acceptable"                   },
   {407, "Proxy Authentication Required"    },
   {408, "Request Time-out"                 },
   {409, "Conflict"                         },
   {410, "Gone"                             },
   {411, "Length Required"                  },
   {412, "Precondition Failed"              },
   {413, "Request Entity Too Large"         },
   {414, "Request-URI Too Large"            },
   {415, "Unsupported Media Type"           },
   {416, "Requested range not satisfiable"  },
   {417, "Expectation Failed"               },
   {500, "Internal Server Error"            },
   {501, "Not Implemented"                  },
   {502, "Bad Gateway"                      },
   {503, "Service Unavailable"              },
   {504, "Gateway Time-out"                 },
   {505, "HTTP Version not supported"       }
};

http_response_t *
hx_createResObj(http_request_t *request_object)
{
   http_response_t *response_object;

   if ((response_object = (http_response_t *)hx_alloc_mem(sizeof(http_response_t),
                                           request_object->allocator)) == NULL)
      return NULL;

   /* initiliaze the request object */
   memset(response_object, 0x00, sizeof(http_response_t));

   response_object->allocator = request_object->allocator;

   /* initialize the output type to
      parsed */
   response_object->type = HX_PARSED;

   /* set the request object */
   response_object->request_object = request_object;

   return response_object;
}

/*
int
destroyResponseObject(http_response_t *http_request)
{
}
*/

/*
   hx_displayHTTPResp
   Prints the contents of the Response
   Object to the log file
*/
void
hx_printResponse (http_response_t *httpRes, int loglvl)
{
   const unsigned char *rname = "hx_printResponse ()";
   hx_logsys_t *log = httpRes->request_object->vhost->log;

   if(!hx_check_log_level(log, loglvl))
      return;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   if(!httpRes)
   {
      hx_log_msg(log, HX_LERRO, rname, "Invalid response object");
      return;
   }
   hx_log_msg(log,
             loglvl,
             rname,
             "=====================================================");

   if(httpRes->type == HX_NONPARSED)
   {
      hx_log_msg(log, loglvl, rname, "HEADER TYPE: NON PARSED");

      hx_hex_dump(log, httpRes->buffer, httpRes->buffer_len, HX_LDUMP);
   }
   else if(httpRes->type == HX_PARSED)
   {
      hx_log_msg(log, loglvl, rname, "HEADER TYPE: PARSED");

      if(httpRes->statusLine)
         hx_log_msg(log, loglvl, rname, "%s", httpRes->statusLine);

      if(httpRes->headers)
         hx_log_msg(log, loglvl, rname, "%s", httpRes->headers);

      if(hx_check_log_level(log, HX_LDUMP))
      {
         if(httpRes->body_len)
            hx_hex_dump(log, httpRes->body, httpRes->body_len, HX_LDUMP);
      }
      hx_log_msg(log,
                loglvl,
                rname,
                "Body Length %d",
                httpRes->body_len);
   }
   hx_log_msg(log,
             loglvl,
             rname,
             "=====================================================");
}

void
hx_add_response_header (hx_logsys_t *log,
                  unsigned char *buffer,
                  int idx,
                  const unsigned char *str)
{
   const unsigned char *rname = "hx_add_response_header ()";
   char temp[MAX_SMALL_BUFF_LEN];

   hx_log_msg(log, HX_LCALL, rname, "Called");

   sprintf(temp, "%s: %s%s", header_names_table[idx], str, CRLF);
   strcat(buffer, temp);
}

char*
hx_create_status_line(hx_logsys_t *log,
                    int status_index,
                    const unsigned char *resource,
                    hx_allocator_t *allocator)
{
   unsigned char *statusline;

   statusline = (unsigned char *) hx_alloc_mem (MAX_STATUS_LINE_LEN, allocator);

   if(!statusline)
      return NULL;

   *statusline = '\0';

   sprintf(statusline,
           "%s %d",
           (unsigned char *)hx_get_conf_val(log, IDX_HTTP_VERSION),
           Responses[status_index].code);

   switch (status_index)
   {
      case IDX_404:
         if(resource)
         {
            strcat(statusline, " ");
            strcat(statusline, resource);
         }
         break;
      case IDX_400:
      case IDX_501:
      default:
         break;
   }

   strcat(statusline, CRLF);

   return statusline;
}

void
hx_form_error_response (http_response_t *response_object,
                    http_request_t *request_object,
                    int errIndex,
                    const unsigned char *caller)
{
   const unsigned char *rname = "hx_form_error_response ()";

   unsigned char *page;
   unsigned char *pageBody;

   unsigned char  content_length[MAX_CONTENT_LENGTH_LEN];
   unsigned char  headers[MAX_HEADER_LEN];
   unsigned char  filler[513];

   int retval;
   hx_logsys_t *log = request_object->vhost->log;

   hx_log_msg(log, HX_LCALL, rname, "Called from %s", caller);


   /* check to see if the vhost has
      an error page assigned. If there is
      no error page assigned generate the
      default error page
      Note: In case a custom error page
      is sent back, the response code should
      be set to 200
   */

   if(request_object->vhost->errpage)
   {
      /* set the status line */
      if(errIndex == IDX_404)
         response_object->statusLine = hx_create_status_line(log,
                                                  errIndex,
                                                  request_object->uri_object->resource,
                                                  request_object->allocator);
      else
         response_object->statusLine = hx_create_status_line(log,
                                                  errIndex,
                                                  NULL,
                                                  request_object->allocator);

      /* fetch the assigned error page */
      retval = hx_read_file(log,
                           request_object->vhost->errpage,
                           &response_object->body,
                           &response_object->body_len,
                           request_object->allocator,
                           lock_errno);

      if(retval != OK)
         hx_log_msg(log, HX_LERRO, rname, "Error fetching default error page");
   }
   else
   {
      /* set the status line */
      if(errIndex == IDX_404)
         response_object->statusLine = hx_create_status_line(log,
                                                  IDX_200,
                                                  request_object->uri_object->resource,
                                                  request_object->allocator);
      else
         response_object->statusLine = hx_create_status_line(log,
                                                  IDX_200,
                                                  NULL,
                                                  request_object->allocator);

      /* allocate memory to the page */
      pageBody = (unsigned char *) hx_alloc_mem(MAX_ERROR_PAGEBODY_LEN,
                                             request_object->allocator);
      /* generate the error page */
      *pageBody = '\0';

      sprintf(pageBody, "%s Resource %.*s %d %s %s",
              errPageBody1,
              request_object->uriLen, request_object->uri,
              Responses[errIndex].code,
              Responses[errIndex].description,
              errPageBody2);

      /* create the error page */
      page = hx_paginate(request_object->vhost->log,
                         errPageTitle,
                         pageBody,
                         request_object->allocator);

      if(page)
      {
         /* Add the body to the response object */
         response_object->body_len = strlen(page);
         response_object->body = hx_strdup(page, request_object->allocator);
      }
   }

   /* 3. form the headers */
   *headers = '\0';

   /* Add the content type */
   hx_add_response_header(log, headers, IDX_CONTENT_TYPE, "text/html");

   /* Add connection data */
   hx_add_response_header(log, headers, IDX_CONNECTION, "close");

   /* Add the server date */
   hx_add_response_header(log, headers, IDX_DATE, response_object->time);

   /* Add the server name */
   hx_add_response_header(log, headers, IDX_SERVER, SERVER);

   /* form  and add the content length string */
   sprintf(content_length, "%d", response_object->body_len);
   hx_add_response_header(log, headers, IDX_CONTENT_LENGTH, content_length);

   /* Add Terminating CRLF */
   strcat(headers, CRLF);

   /* Add the headers to the
      response object */
   response_object->headers = hx_strdup (headers, request_object->allocator);

   hx_free_mem (pageBody, request_object->allocator);
   hx_free_mem (page, request_object->allocator);
}

int
hx_formDirListing (http_response_t *response_object, http_request_t *request_object)
{
   const unsigned char *rname = "hx_formDirListing ()";

   unsigned char *page;
   unsigned char  content_length[MAX_CONTENT_LENGTH_LEN];
   unsigned char  title[MAX_SMALL_BUFF_LEN];
   unsigned char  headers[MAX_HEADER_LEN];
   unsigned char  *line;
   unsigned char  *body;
   unsigned char  *start;
   unsigned char  *end;

   hx_list_t   *dirlist;

   void *data;
   long bodysize = 0;

   int retval;
   int count, entries;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   /* 1. get dir list data */
   retval = hx_get_dir_list(request_object, &dirlist, lock_errno/*&csErrNo*/);

   if(OK != retval)
   {
      return retval;
   }

   /* Create page Title */
   *title = '\0';
   sprintf(title, "Directory Listing for %s", request_object->uri_object->resource);

   /* Create temp buffers to hold body start and end */
   start = hx_alloc_mem(MAX_BODYSTART_LEN, request_object->allocator);
   *start = '\0';

   end = hx_alloc_mem(MAX_BODYEND_LEN, request_object->allocator);
   *end = '\0';

   /* Create page body start */
   sprintf(start,
           "<center><h3>Directory Listing for %s</h3></center>\n"
           "<table width=\"90%\" align=\"center\">\n",
           request_object->uri_object->resource);

   strcat(start, "<tr bgcolor=\"#cccccc\">"
#ifdef _HX_UNIX_
                     "<td><b>Access Mode</td>"
#endif
                     "<td><b>Filename</td>"
                     "<td align=\"right\"><b>Size</td>"
                     "<td align=\"right\"><b>Last Modified</td>"
                "</tr>\n");

   entries = hx_size(dirlist);

   /* calculate the listing size in
      characters */
   for(count = 0; count < entries; count++)
   {
      data = hx_get_pos(dirlist, count);
      if(data)
      {
         hx_dir_lister(data, &line, request_object->allocator);
         bodysize += strlen(line);
         hx_free_mem(line, request_object->allocator);
      }
   }

   /* Create page body end */
   strcat(end, "<tr bgcolor=\"#cccccc\" align=\"right\">"
#ifdef _HX_UNIX_
                "<td colspan=\"4\">"
#else
                "<td colspan=\"3\">"
#endif
                "<b>");

   strcat(end, SERVER);
   strcat(end, " / ");
   strcat(end, VERSION);

   strcat(end, "</td></tr>");
   strcat(end, "</table>\n");

   /* Allocate memory to hold listing */
   bodysize += strlen(start) + strlen(end) + 1;
   body = hx_alloc_mem(bodysize, request_object->allocator);
   *body = '\0';

   /* Transfer contents into the body */
   strcat(body, start);

   for(count = 0; count < entries; count++)
   {
      data = hx_get_pos(dirlist, count);
      if(data)
      {
         hx_dir_lister(data, &line, request_object->allocator);
         strcat(body, line);
         hx_free_mem(line, request_object->allocator);
      }
   }
   strcat(body, end);

   /* Create the Dir List Page */
   page = hx_paginate(request_object->vhost->log,
                      title,
                      body,
                      request_object->allocator);

   if(!page)
   {
      return HX_ERR_MEM_ALLOC;
   }

   response_object->body_len = strlen (page);
   response_object->body = hx_strdup (page, request_object->allocator);

   /* Free the body buffer */
   hx_free_mem(body, request_object->allocator);

   /* Free the page buffer */
   hx_free_mem(page, request_object->allocator);

   /* Free the dir list */
   hx_destroy_list(dirlist);

   /* 2. create the status line */
   response_object->statusLine = hx_create_status_line(request_object->vhost->log,
                                            IDX_200,
                                            request_object->uri_object->resource,
                                            request_object->allocator);

   /* 3. form the headers */
   *headers = '\0';

   /* Add the content type */
   hx_add_response_header(request_object->vhost->log, headers, IDX_CONTENT_TYPE, "text/html");

   /* Add connection data */
   hx_add_response_header(request_object->vhost->log, headers, IDX_CONNECTION, "close");

   /* Add the server date */
   hx_add_response_header(request_object->vhost->log, headers, IDX_DATE, response_object->time);

   /* Add the server name */
   hx_add_response_header(request_object->vhost->log, headers, IDX_SERVER, SERVER);

   /* form the content length string  and add it */
   sprintf(content_length, "%d", response_object->body_len);
   hx_add_response_header(request_object->vhost->log, headers, IDX_CONTENT_LENGTH, content_length);

   /* Add Terminating CRLF */
   strcat(headers, CRLF);

   /* Add the headers to the
      response object */
   response_object->headers = hx_strdup (headers, request_object->allocator);

   return OK;
}

int
hx_sendResponse (SOCKET client_socket, http_response_t *response_object)
{
   const unsigned char *rname = "hx_sendResponse ()";
   int retval;
   char buffer[MAX_STATUS_LINE_LEN + MAX_TOTAL_HEADER_LEN];
   hx_logsys_t *log = response_object->request_object->vhost->log;


   hx_log_msg(log, HX_LCALL, rname, "Called");

   if(!response_object)
   {
      hx_log_msg(log, HX_LERRO, rname, "Invalid response object");
      return ERRC;
   }

   if(response_object->type == HX_PARSED)
   {
      *buffer = '\0';

      if(!response_object->statusLine)
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "Invalid status line in response object");
         return ERRC;
      }

      /* send the status line and the headers*/
      strcat(buffer, response_object->statusLine);

      if(response_object->headers)
         strcat(buffer, response_object->headers);

      retval = hx_sockWrite (client_socket,
                             buffer,
                             strlen(buffer));
      if (!retval)
      {
         hx_log_msg(log, HX_LERRO, rname, "SOCKET WRITE FAILURE");
         return retval;
      }

      /* send the body */
      if (response_object->body_len)
      {
         retval = hx_sockWrite (client_socket,
                                response_object->body,
                                response_object->body_len);
      }

      if (!retval)
      {
          hx_log_msg(log, HX_LERRO, rname, "SOCKET WRITE FAILURE");
      }
   }
   else if(response_object->type == HX_NONPARSED)
   {
      /* send the buffer data */
      retval = hx_sockWrite (client_socket,
                             response_object->buffer,
                             response_object->buffer_len);
      if (!retval)
      {
         hx_log_msg(log, HX_LERRO, rname, "SOCKET WRITE FAILURE");
         return retval;
      }
   }

   return retval;
}

int
hx_sendErrResponse (SOCKET client_socket, http_response_t *response_object)
{
   const unsigned char *rname = "hx_sendErrResponse ()";
   int retval;
   char buffer[MAX_STATUS_LINE_LEN + MAX_TOTAL_HEADER_LEN];
   hx_logsys_t *log = response_object->request_object->vhost->log;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   *buffer = '\0';

   /* send the status line and the headers*/
   strcat(buffer, response_object->statusLine);

   if(response_object->headers)
      strcat(buffer, response_object->headers);

   if(response_object->body_len)
   {
      strncat(buffer, response_object->body, response_object->body_len);
   }

   hx_log_msg(log, HX_LDEBG, rname, "Writing to socket: \n=>%s<=", buffer);

   /* send the data */
   retval = hx_sockWrite (client_socket,
                          buffer,
                          strlen(buffer));
   if (!retval)
   {
      hx_log_msg(log, HX_LERRO, rname, "SOCKET WRITE FAILURE");
   }

   return retval;
}
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#include "httpCommon.h"
#include "httpInclude.h"
#include "httpProcessor.h"
#include "httpRequest.h"
#include "httpAdmin.h"
#include "httpResponse.h"
#include "httpEngine.h"
#include "httpConf.h"
#include "httpVhost.h"
#include "httpSock.h"
#include "httpCGI.h"

const unsigned char *header_names_table[MAX_HEADERS] =
{
   "Cache-Control",        /* GENERAL HEADERS = 0 */
   "Connection",
   "Date",
   "Pragma",
   "Trailer",
   "Transfer-Encoding",
   "Upgrade",
   "Via",
   "Warning",
   "Accept",               /* REQUEST HEADERS = 9 */
   "Accept-Charset",
   "Accept-Encoding",
   "Accept-Language",
   "Authorization",
   "Expect",
   "From",
   "Host",
   "If-Match",
   "If-Modified-Since",
   "If-None-Match",
   "If-Range",
   "If-Unmodified-Since",
   "Max-Forwards",
   "Proxy-Authorization",
   "Range",
   "Referer",
   "TE",
   "User-Agent",
   "Accept-Ranges",        /* RESPONSE HEADERS = 28 */
   "Age",
   "ETag",
   "Location",
   "Proxy-Authenticate",
   "Retry-After",
   "Server",
   "Vary",
   "WWW-Authenticate",
   "Allow",                /* ENTITY HEADERS = 37 */
   "Content-Encoding",
   "Content-Language",
   "Content-Length",
   "Content-Location",
   "Content-MD5",
   "Content-Range",
   "Content-Type",
   "Expires",
   "Last-Modified",
   "Client-Date"
};

static const unsigned char *NotImplemented[] =
{
   "jsp",
   "class",
   "dummy",
   "asp"
};

/* hx_processRequest ()
   process the request depending on the
   method of the HTTP request */

static int
hx_processRequest(http_request_t *request_object, http_response_t *response_object)
{
   const  char    *rname = "hx_processRequest ()";
   unsigned char  *indexFile;
   unsigned char  scriptPrefix[MAX_FILENAME];

   int            retval = OK;
   int            idx;

   unsigned char  filename[MAX_FILENAME];
   unsigned char  *separator = (unsigned char *)
                                hx_get_conf_val(request_object->vhost->log,
                                              IDX_PATH_SEPARATOR);

   hx_list_t      *indexFileList;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");

   hx_log_msg(request_object->vhost->log, HX_LVERB, rname, " ");
   hx_log_msg(request_object->vhost->log,
             HX_LVERB,
             rname,
             "Servicing request %s",
             request_object->request_line);

   if(!separator)
      separator = "/";

   if(request_object->extension)
   {
      /* check if the requested resource
         delivery is implemented or not */
      retval = hx_match_string_idx(NotImplemented,
                              MAX_NOT_IMPLEMENTED,
                              request_object->extension);

      if (retval >= 0)
      {
         hx_form_error_response(response_object, request_object, IDX_501, rname);
         return ERRC;
      }
   }

   /* Get full path of the
      requested resource */
   *filename = '\0';

   if(request_object->vhost->docroot)
      strcat(filename, request_object->vhost->docroot);
   else
      strcat(filename, (unsigned char *)hx_get_conf_val(request_object->vhost->log,
                                                      IDX_DOC_ROOT));

   /* preassign the resource type */
   request_object->resource_type = HX_STATIC;

   /* Check if the resource content is dynamically
      generated or not: currently using the logic
      that the resource name in this case will start
      with the name of CGI-ROOT */

   if(request_object->uri_object->resource)
   {
      if('/' == request_object->uri_object->resource[0])
         strcpy(scriptPrefix, &request_object->uri_object->resource[1]);
      else
         strcpy(scriptPrefix, request_object->uri_object->resource);

      if(!strncmp(request_object->vhost->scriptroot,
                  scriptPrefix,
                  strlen(request_object->vhost->scriptroot)))
      {
         /* resource is dynamic */
         request_object->resource_type = HX_DYNAMIC;
      }

      if('/' != request_object->uri_object->resource[0])
         strcat(filename, separator);

      strcat(filename, request_object->uri_object->resource);

      /*
         Check to see if we have a
         trailing '/'. If we have it
         remove it provided it does not refer
         to a drive.
      */
#ifdef _HX_WIN32_
      if(!hx_isDrive(request_object->uri_object->resource))
      {
#endif
         hx_removeTrail(filename);
#ifdef _HX_WIN32_
      }
#endif
   }

   /* copy the filename to the
      request object */
   if(strlen(filename))
      request_object->filename = hx_strdup(filename, request_object->allocator);

   /* Retrieve the statistics
      of the requested resource */
   if(request_object->filename)
      retval = hx_getResDetails(request_object, lock_errno);

   if(retval != OK)
   {
      idx = hx_get_response_code(retval);
      hx_form_error_response(response_object, request_object, idx, rname);
      retval = ERRC;
   }
   else
   {
      /* check to see if resource name is present
         if not present try files in index list
         if not form directory listing */
      if(request_object->is_dir == HX_TRUE)
      {
         /* get the index file list */
         indexFileList = (hx_list_t *)hx_get_conf_val(request_object->vhost->log,
                                                    IDX_INDEX_FILES);

         /* reset the resource type to
            dynamic */
         request_object->resource_type = HX_STATIC;

         if(!indexFileList)
         {
            hx_log_msg(request_object->vhost->log,
                      HX_LWARN,
                      rname,
                      "Index file list not found");
            retval = HX_ERR_FILE_NOEXIST;
         }
         else
         {
            /* try to fetch a valid index file
               and populate the resource name field
               in the request object */
            retval = hx_getIndexFile(request_object,
                                     indexFileList,
                                     separator,
                                     lock_errno);
         }

         /* no valid index file,
            confirmed that a directory
            listing is requested */
         if (retval != OK)
         {
            if (retval == HX_ERR_FILE_NOEXIST)
            {
               retval = hx_formDirListing (response_object, request_object);
               if(retval == OK)
                  return OK;
            }
            /* An error occured */
            hx_form_error_response(response_object, request_object, hx_get_response_code(retval), rname);
            return ERRC;
         }
         else
         {
            /* Retrieve the mime type of
               the resource if no mime type
               is found */
            if(!request_object->mime_object)
            {
               request_object->mime_object = hx_lookup_resource_mime(request_object->vhost->log,
                                                  request_object->filename);
            }
         }
      }

      /* Perform CGI handling if required */
      if(HX_DYNAMIC == request_object->resource_type)
      {
         /* check to see if the script or
            the executable has execution
            permission bit set. If not
            error out immediately */
#ifdef _HX_UNIX_
         if(!(request_object->resource_stat.st_mode & S_IXOTH))
         {
            hx_log_msg(request_object->vhost->log,
                      HX_LERRO,
                      rname,
                      "Resource does not have execution permission");

            hx_form_error_response(response_object, request_object, IDX_500, rname);
            return ERRC;
         }
#endif
         retval = hx_handle_cgi(request_object, response_object);
      }
      /* resource type is static */
      else
      {
         retval = (*(g_engine[request_object->method_idx]))(request_object, response_object);
      }
   }

   return retval;
}

/*
hx_process_http()
processes a connected
client
*/
int
hx_process_http(SOCKET client_socket,
                 const unsigned char *client_address,
                 hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_process_http ()";

   http_response_t *response_object;
   http_request_t *request_object;

   unsigned char *vhostname = NULL;
   unsigned char *pch;
   unsigned char *headerval;

   int retval;
   int timeLen;
   int messageLength;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* Create the request object and initialize it */
   if ((request_object = hx_createReqObj(allocator)) == NULL)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Could not allocate memory for http_request_t Object");

      return HX_ERR_MEM_ALLOC;
   }

   hx_log_msg(server_log_sys, HX_LDEBG, rname, "Created Request Object");

   /* set the socket */
   request_object->sock = client_socket;

   /* copy the client Address */
   request_object->client_address = hx_strdup(client_address, request_object->allocator);

   /* parse the request */
   retval = hx_parse_request(request_object);

   /* check to see if the client has
      an error */
   if(retval == HX_ERR_CLIENT)
      return retval;

   /* Create the response object and initialize it */
   if ((response_object = hx_createResObj(request_object)) == NULL)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Could not allocate memory for http_response_t Object");
      return HX_ERR_MEM_ALLOC;
   }

   /* embed the response generation time */
   timeLen = hx_getTime(response_object->time, MAX_TIME_LEN);
   response_object->time[timeLen] = '\0';

   /* get the virtual host data for the request */
   if(request_object->num_headers > 0)
   {
      headerval = (unsigned char *)hx_get_header_value(IDX_HOST, request_object);
      if(headerval)
         vhostname = (unsigned char *)hx_strdup(headerval, request_object->allocator);
   }

   if(vhostname)
   {
      /* Extract virtual host name */
      if((pch = strchr(vhostname, ':')) != NULL)
         *pch = '\0';

      /* Check to see what is being asked for */
      if(HX_TRUE == hx_isLocalhost(vhostname))
      {
         request_object->vhost = defaultVhost;
      }
      else
      {
         /* Attach the actual virtual host system
            to the request object */
         request_object->vhost = hx_getVhost(request_object->vhost->log, vhostname);

         if(!request_object->vhost)
         {
            hx_log_msg(server_log_sys,
                      HX_LERRO,
                      rname,
                      "Virtual host lookup failed");
            request_object->vhost = defaultVhost;
            retval = HX_ERR_VHOST_NOEXIST;
         }
      }
   }
   else
   {
      /* Check to see if the version of the
         request is 1.1 or greater, if so,
         return bad request */
      /* assign default virtual host object
         to the request */
      request_object->vhost = defaultVhost;
   }

   if(retval == HX_PARSE_OK)
   {
      /* process the request and accordingly
         fill in the response Object */
      retval = hx_processRequest(request_object, response_object);
   }
   else
   {
      hx_form_error_response(response_object, request_object, hx_get_response_code(retval), rname);
      retval = ERRC;
   }

   /* display the request object */
   hx_print_request(request_object, HX_LREQS);

   /* display the request object */
   hx_printResponse(response_object, HX_LRESP);

   /* format the response
      by extracting the data from
      response struct */
   if(retval == ERRC)
   {
      retval = hx_sendErrResponse(client_socket, response_object);
      hx_log_msg(server_log_sys,
                HX_LDEBG,
                rname,
                "Error Response sent back to client");
   }
   else
   {
      retval = hx_sendResponse(client_socket, response_object);
   }

   /* destroy the environement list */
   hx_destroy_list(request_object->env);

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Environment list destroyed");

   /* return */
   return retval;
}

/*
   hx_process_admin()
   processes a connected
   client
*/
int
hx_process_admin(SOCKET client_socket,
                const unsigned char *client_address,
                hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_process_admin ()";
   int retval;
   http_admin_t *a;
   unsigned char buffer[MAX_ADMIN_RESPONSE_LEN];
   unsigned char response[MAX_ADMIN_RESPONSE_LEN];
   unsigned char lenstr[MAX_MSG_ADMIN_LEN_SIZE + 1];

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   /* Create the request object and initialize it */
   if ((a = hx_create_admin_object(allocator)) == NULL)
   {
      hx_log_msg(server_log_sys,
                HX_LDEBG,
                rname,
                "Could not allocate memory for http_admin_t Object");
      hx_sockWrite(a->sock, "1:Server Error", strlen("1:Server Error"));
      return HX_ERR_MEM_ALLOC;
   }

   hx_log_msg(server_log_sys, HX_LDEBG, rname, "Created Admin Object");

   /* set the socket */
   a->sock = client_socket;

   /* copy the client Address */
   a->client_address = hx_strdup(client_address, a->allocator);

   /* parse the request */
   retval = hx_parse_admin(a);

   /* initialize the string */
   *buffer = '\0';
   *response = '\0';

   if(HX_PARSE_OK == retval)
   {
      sprintf(response, "0:OK");
      retval = hx_execute_admin(a);
   }
   else
   {
      /* send error response back to the
         client */

      switch(retval)
      {
         case HX_ERR_ADM_INVALID_CMD:
            sprintf(response, "1:Invalid Command");
            break;
         case HX_ERR_ADM_INVALID_MSG:
            sprintf(response, "1:Invalid Message");
            break;
         case HX_ERR_ADM_INVALID_LEN:
            sprintf(response, "1:Invalid Length");
            break;
         default:
            sprintf(response, "1:Unkown Error");
            break;
      }

      /* assign the retval */
      retval = ERRC;
   }

   /* send response back to the client */
   sprintf(lenstr, "%04d", strlen(response));
   strcat(buffer, lenstr);
   strcat(buffer, response);
   hx_sockWrite(a->sock, buffer, strlen(buffer));

   return retval;
}
/* End of File */

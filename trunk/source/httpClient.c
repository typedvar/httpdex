/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 */

#include "httpController.h"
#include "httpSock.h"


/* START global variables section */
hx_logsys_t *server_log_sys;
/* END global variables section */

void hx_stop(hx_logsys_t *log)
{
   const unsigned char *rname = "hx_stop ()";

   hx_sockCleanup();

   if(log)
      hx_destroy_log_sys(log);
}

int main(int argc, char *argv[])
{
   const char *rname = "main ()";
   long port;
   char *host;
   char *URL;
   char *command;

   hx_logsys_t *log;

   /* close the standard error */
   fclose(stderr);

   /* Start Logging engine */
   if((log = hx_create_log_sys((hx_logsys_t *)NULL,
                             "stdout",
                             "httpc",
                             HX_LOGPREFIX_NO)) == NULL)
   {
      return -1;
   }

   server_log_sys = log;

   if(argc < 3)
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Usage: httpc host port");
      hx_stop(log);
      return -1;
   }

   /* Initialize the Sockets Subsystem
      required on WIN32 platforms */
   if(OK != hx_sock_init())
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Failed to Initialize TCP Subsystem");
      hx_stop(log);
      return -1;
   }

   host = argv[1];
   port = atol(argv[2]);

   /* Main client function */
   hx_start_client(log, host, port);

   hx_stop(log);

   return 0;
}
/* End of File */

/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdio.h>
#include <signal.h>

#ifdef _HX_WIN32_
#include <process.h>
#elif defined (_HX_UNIX_)
#include <unistd.h>
#include <sys/types.h>
#endif

#include "httpSpawner.h"
#include "httpConf.h"
#include "httpUtils.h"
#include "httpSock.h"
#include "httpMime.h"
#include "httpLog.h"
#include "httpMemsys.h"
#include "httpThreads.h"
#include "httpLock.h"

/* START global variables section */
const unsigned char *VERSION = "V1.0";
const unsigned char *SERVER  = "httpdEx";
const unsigned char *PIDFILE = "httpdEx.pid";

/* variable to indicate whether a shutdown
   is pending */
int server_shutdown = HX_SHUTDN_NOTPENDING;
int http_done = HX_FALSE;
int admin_done = HX_FALSE;

hx_allocator_t *server_allocator = NULL;

/* The main server log system */
hx_logsys_t *server_log_sys;

/* System wide vhost object */
hx_vhost_t  *defaultVhost;

/* Server socket for HTTP */
SOCKET serverSock = 0;

/* Admin Socket for HTTPDeX */
SOCKET adminSock = 0;

/* Count to keep track of number of clients
   being served */
int g_num_clients_being_served = 0;

/* Lock variables */
hx_lock_t *lock_num_clients = NULL;
hx_lock_t *lock_accept_http = NULL;
hx_lock_t *lock_accept_admin = NULL;
hx_lock_t *lock_errno = NULL;
hx_lock_t *lock_shutdown = NULL;

/* END global variables section */

/* hx_create_pid_file
   Creates the PID file containing
   the PID of the currently running
   server
*/
static int
hx_create_pid_file(hx_logsys_t *log)
{
   const char *rname = "hx_create_pid_file ()";

   u_long srvpid = 0;
   FILE *fptr;

   srvpid = getpid();

   if(srvpid)
   {
      fptr = fopen(PIDFILE, "w");

      if(!fptr)
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "PID file could not be opened");
         return HX_ERR_PID_FILECREATE;
      }

      fprintf(fptr, "%d", srvpid);
      fflush(fptr);
      fclose(fptr);
   }
   else
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Server PID could not be retrieved");
      return HX_ERR_GETPID;
   }

   return OK;
}

void
hx_signal_shutdown(int state)
{
   hx_lock(lock_shutdown);
   server_shutdown = state;
   hx_unlock(lock_shutdown);
}

/* hx_switch_off
   Performs server shutdown and resource
   deallocation
*/
void
hx_switch_off(int type, hx_allocator_t *allocator)
{
   const char *rname = "hx_switch_off ()";

   hx_signal_shutdown(HX_SHUTDN_INITIATED);

   switch(type)
   {
   case NORMAL_CLOSE:
      hx_log_msg(server_log_sys,
                HX_LSYST,
                rname,
                "Initiating NORMAL SHUTDOWN %s %s",
                SERVER,
                VERSION);
      break;
   case ABORTIVE_CLOSE:
      if(server_log_sys)
         hx_log_msg(server_log_sys,
                   HX_LSYST,
                   rname,
                   "Initiating ABORTIVE SHUTDOWN %s %s",
                   SERVER,
                   VERSION);
      else
         fprintf(stderr,
                 "Initiating ABORTIVE SHUTDOWN %s %s",
                 SERVER,
                 VERSION);
      break;
   default:
      break;
   }

   /* close the server socket */
   if(serverSock)
      hx_sockClose(serverSock, HX_BOTH);

   hx_log_msg(server_log_sys,
             HX_LSYST,
             rname,
             "OFFLINE -> Service Interface ");

   if(adminSock)
      hx_sockClose(adminSock, HX_BOTH);

   hx_log_msg(server_log_sys,
             HX_LSYST,
             rname,
             "OFFLINE -> Admin Interface");

   hx_sockCleanup();

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Waiting for Admin and Worker threads to complete");


   /* Wait for the worker threads and the
      admin threads to complete their job */
   while((http_done == HX_FALSE) ||
         (admin_done == HX_FALSE))
   {
      hx_sleep(0.1);
   }

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Admin and Worker threads completed processing");

   /* destroy the virtual host system */
   hx_destroyVhostSys(server_log_sys);

   /* destroy the configuration system */
   hx_destroy_config(server_log_sys);

   hx_log_msg(server_log_sys,
             HX_LSYST,
             rname,
             "OFFLINE -> httpDeX");

   if(server_log_sys)
   {
      hx_destroy_log_sys(server_log_sys);
      server_log_sys = NULL;
   }

   /* Destroy the locks*/
   if(lock_errno)
   {
      hx_destroy_lock(lock_errno);
      lock_errno = NULL;
   }

   if(lock_num_clients)
   {
      hx_destroy_lock(lock_num_clients);
      lock_num_clients = NULL;
   }

   if(lock_accept_http)
   {
      hx_destroy_lock(lock_accept_http);
      lock_accept_http = NULL;
   }

   if(lock_accept_admin)
   {
      hx_destroy_lock(lock_accept_admin);
      lock_accept_admin = NULL;
   }

   if(lock_shutdown)
   {
      hx_destroy_lock(lock_shutdown);
      lock_shutdown = NULL;
   }

   /* destroy the currently passed
      allocator */
   if(allocator)
   {
      /* Check to see if this is
         the server Allocator */
      if(allocator == server_allocator)
      {
         server_allocator = NULL;
      }

      hx_destroy_allocator(allocator);
      allocator = NULL;
   }

   /* destroy the server_allocator
      if not garbage collected */
   if(server_allocator)
   {
      hx_destroy_allocator(server_allocator);
      server_allocator = NULL;
   }

   /* Set server shutdown stage to 2
      so that main thread can exit */
   hx_signal_shutdown(HX_SHUTDN_COMPLETE);
}

static void hx_sig_mask(void)
{
#ifdef _HX_UNIX_
   sigset_t maskset;
   sigset_t oldset;

   /* fill the sigsets */
   sigfillset(&maskset);

   /* block all unwanted signals */
   sigprocmask(SIG_SETMASK, &maskset, &oldset);
#endif
}

void
hx_sigusr1(int sig)
{
   const unsigned char *rname = "hx_SUGUSR1 ()";

   /* reset the signal handler */
   signal(SIGUSR1, hx_sigusr1);

   /* mask all signals when
      handling this */
   hx_sig_mask();

   if(server_log_sys)
      hx_log_msg(server_log_sys,
                HX_LSYST,
                rname,
                "Shutdown initiated (signal:%03d)",
                sig);

   hx_signal_shutdown(HX_SHUTDN_PENDING);

   hx_switch_off(NORMAL_CLOSE, server_allocator);
}

void
hx_sigint(int sig)
{
   const unsigned char *rname = "hx_sigint ()";

   /* reset the signal handler */
   signal(SIGINT, hx_sigint);

   /* mask all signals when
      handling this */
   hx_sig_mask();

   if(server_log_sys)
      hx_log_msg(server_log_sys,
                HX_LSYST,
                rname,
                "Shutdown initiated (signal:%03d)",
                sig);

   hx_signal_shutdown(HX_SHUTDN_PENDING);
   hx_switch_off(NORMAL_CLOSE, server_allocator);
}

void
hx_sigsegv(int sig)
{
   const unsigned char *rname = "hx_sigsegv ()";

   /* reset the signal handler */
   signal(SIGSEGV, hx_sigsegv);

   /* mask all signals when
      handling this */
   hx_sig_mask();

   if(server_log_sys)
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Forced shutdown initiated (signal:%03d)",
                sig);

   hx_signal_shutdown(HX_SHUTDN_PENDING);
   hx_switch_off(ABORTIVE_CLOSE, server_allocator);
}

static void
hx_install_signal_handlers(void)
{
   /* install signal handlers */
   signal(SIGINT, hx_sigint);
   signal(SIGSEGV, hx_sigsegv);
   signal(SIGUSR1, hx_sigusr1);
}

void
hx_cmd_usage(void)
{
   printf("usage: httpd <port>\n");
}

/* Create a thread to activate
   the admin interface */
static int
hx_init_admin_iface(hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_init_admin_iface ()";

   hx_thread_t *thread;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   thread = hx_create_thread(allocator, hx_spawn_admin, (void *)allocator);

   if(!thread)
   {
      hx_log_msg(server_log_sys, HX_LDEBG, rname, "hx_create_thread () failed");
      return -1;
   }

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Admin spawner id is %d",
             hx_get_thread_id(thread));

   return 0;
}

/* Create a thread to activate
   the HTTP service interface */
static int
hx_init_http_iface(hx_allocator_t *allocator)
{
   const unsigned char *rname = "hx_init_http_iface ()";

   hx_thread_t *thread;

   hx_log_msg(server_log_sys, HX_LCALL, rname, "Called");

   thread = hx_create_thread(allocator, hx_spawn_workers, (void *)allocator);

   if(!thread)
   {
      hx_log_msg(server_log_sys, HX_LDEBG, rname, "hx_create_thread () failed");
      return -1;
   }

   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Worker spawner id is %d",
             hx_get_thread_id(thread));

   return 0;
}

int
main(int argc, char *argv[])
{
   const char  *rname = "main ()";
   hx_allocator_t *allocator;

   int retval;

   /* Create Servers Main memory allocator */
   if((allocator = hx_create_allocator("main")) == NULL)
   {
      fprintf(stderr, "Main allocator creation failed\n");
      hx_switch_off(ABORTIVE_CLOSE, allocator);
      return 0;
   }

   server_allocator = allocator;

   /* Start Logging engine */
   if((server_log_sys = hx_create_log_sys((hx_logsys_t *)NULL,
                                      "../logs/httpdex.log",
                                      "HTTPDEX MAIN",
                                      HX_LOGPREFIX_YES)) == NULL)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Failed to create log system");

      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* Set all log levels */
   hx_set_log_level_all(server_log_sys);

   /* Initialize the TCP system */
   if(OK != hx_sock_init())
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Failed to Initialize the TCP System");
      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* Load configuration data */
   retval = hx_load_config(_HX_CONFIG_FILE_, server_log_sys);

   if(retval != OK)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Error loading config data");
      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* Create the PID file */
   retval = hx_create_pid_file(server_log_sys);

   if(retval != OK)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Error creating pid file");
      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* install all signal handlers */
   hx_install_signal_handlers();

   /* create the locks */
   lock_num_clients  = hx_create_lock();

   if(!lock_num_clients)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Num Client Lock creation failed");
      hx_switch_off(NORMAL_CLOSE, allocator);
   }

   lock_accept_http = hx_create_lock();

   if(!lock_accept_http)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Accept Lock creation failed");
      hx_switch_off(NORMAL_CLOSE, allocator);
   }

   lock_accept_admin = hx_create_lock();

   if(!lock_accept_admin)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "Accept Admin Lock creation failed");
      hx_switch_off(NORMAL_CLOSE, allocator);
   }

   lock_errno = hx_create_lock();

   if(!lock_errno)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "ERRNO Lock creation failed");
      hx_switch_off(NORMAL_CLOSE, allocator);
   }

   lock_shutdown = hx_create_lock();

   if(!lock_shutdown)
   {
      hx_log_msg(server_log_sys,
                HX_LERRO,
                rname,
                "ERRNO Lock creation failed");
      hx_switch_off(NORMAL_CLOSE, allocator);
   }

   /* daemonize self */

   /* Load Mime data */
   retval = hx_init_mime_table(server_log_sys, allocator);

   if(retval != OK)
   {
      hx_log_msg(server_log_sys, HX_LERRO, rname, "Mime data Load failed");
      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* Activate Admin System */
   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Starting Administrative Interface %s", VERSION);


   retval = hx_init_admin_iface(allocator);

   if(retval == -1)
   {
      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* Activate Server System */
   hx_log_msg(server_log_sys,
             HX_LDEBG,
             rname,
             "Starting Request Processing Interface %s", VERSION);

   retval = hx_init_http_iface(allocator);

   if(retval == -1)
   {
      admin_done = HX_TRUE;
      http_done = HX_TRUE;
      hx_switch_off(NORMAL_CLOSE, allocator);
      return 0;
   }

   /* Wait till the server
      has completely shutdown
   */
   while(server_shutdown != HX_SHUTDN_COMPLETE)
   {
      hx_sleep(0.1);
   }

   return 0;
}
/* End of File */

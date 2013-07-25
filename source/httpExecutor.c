/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */
#include "httpExecutor.h"
#include "httpInclude.h"

#ifdef _HX_UNIX_
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

static void
hx_displayProcess (hx_logsys_t *log, hx_process_t *process, int loglvl)
{
   const unsigned char *rname = "hx_displayProcess ()";

   if(!hx_check_log_level(log, loglvl))
      return;

   hx_log_msg(log, loglvl, rname, "--------------------------------------");
   hx_log_msg(log, loglvl, rname, "    Interpreter: %s", process->interpreter);

   if(process->path)
      hx_log_msg(log, loglvl, rname, "           Path: %s", process->path);

   hx_log_msg(log, loglvl, rname, "   Command Line: %s", process->cmdLine);

   if(process->arg)
      hx_log_msg(log, loglvl, rname, "      Arguments: %s", process->arg);

   if(process->dir)
      hx_log_msg(log, loglvl, rname, "      Directory: %s", process->dir);

   /* Print the environment block */
   hx_log_msg(log, loglvl, rname, "Env START: ->");

#ifdef _HX_WIN32_

   hx_printWIN32EnvBlock(log, process->env_block);

#elif defined(_HX_UNIX_)

   hx_printUNIXEnvBlock(log, process->env_block);

#endif

   hx_log_msg(log, loglvl, rname, "  Env END: ->");
   hx_log_msg(log, loglvl, rname, "--------------------------------------");
}

static int
hx_getInterpreter (http_request_t *request_object, hx_process_t *process)
{
   const char        *rname = "hx_getInterpreter ()";

   FILE              *file;

   unsigned char     *pch;
   unsigned char     *imageName;
   unsigned char     *extension;
   unsigned char     *filename;
   unsigned char     firstLine[MAX_INTERPRETER_NAME];
   unsigned char     cmdLine[MAX_CMDLINE_LEN];
   unsigned char     arg[MAX_CMDLINE_LEN];

#ifdef _HX_WIN32_

   IMAGE_DOS_HEADER  *hdr;

#endif

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called on \"%s\"", request_object->filename);

   /* duplicate the filename */
   filename = hx_strdup(request_object->filename, request_object->allocator);

   if(!filename)
   {
      hx_log_msg(request_object->vhost->log, HX_LERRO, rname, "Memory Allocation Failed");
      return HX_ERR_MEM_ALLOC;
   }

   /* initialize the file type */
   process->type = HX_UNKNOWN;

   /* Check the file type */
   imageName = strrchr(filename, '/');

   if(!imageName)
      imageName = strrchr(filename, '\\');

   if(!imageName)
      imageName = filename;
   else
      /* Advance the slash */
      ++imageName;

#ifdef _HX_WIN32_

   /* Get the extension */
   extension = strrchr(imageName, '.');

   if(extension)
   {
      if((!hx_strcmpi(".com", extension)) ||
         (!hx_strcmpi(".exe", extension)))
      {
         process->interpreter = hx_strdup(imageName, request_object->allocator);
         process->path = NULL;
         process->type = HX_BIN;
      }
   }

#endif

   if(process->type == HX_UNKNOWN)
   {
      /* open the file */
      file = fopen(filename, "rb");

      /* get the first line of the file */
      fgets(firstLine, MAX_INTERPRETER_NAME, file);

      /* close the file */
      fclose(file);

      /* delete newline and whitespace */
      pch = &firstLine[strlen(firstLine) - 1];

      while(isspace(*pch) && pch >= firstLine)
      {
         *pch-- = '\0';
      }

      hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "%s", firstLine);

      /*check to see if it starts with !# */
      if('#' == firstLine[0] && '!' == firstLine[1])
      {
         /* get the interpreter name */
         if((pch = strrchr(firstLine, '/')) != NULL)
         {
            if(strlen(pch))
            {
               process->interpreter = hx_strdup(pch + 1, request_object->allocator);
               process->path = hx_strdup(firstLine + 2, request_object->allocator);
               process->type = HX_SCRIPT;
            }
         }
         else
         {
            /* The executable name is not
               prepended by the path */
            if(strlen(firstLine + 2))
            {
               process->interpreter = hx_strdup(firstLine + 2, request_object->allocator);
               process->path = NULL;
               process->type = HX_SCRIPT;
            }
         }
      }
      else
      {

#ifdef _HX_WIN32_

         if(strlen(firstLine) >= sizeof(IMAGE_DOS_HEADER))
         {
            /* check for WIN32 executable sign */
            hdr = (IMAGE_DOS_HEADER*) firstLine;

            if(hdr->e_magic == IMAGE_DOS_SIGNATURE)
            {
               process->interpreter = hx_strdup(imageName, request_object->allocator);
               process->path = hx_strdup(filename, request_object->allocator);
               process->type = HX_BIN;
            }
         }

#elif defined (_HX_UNIX_)

         if(!hx_strncmpi("elf", firstLine, 3))
         {
            process->type = HX_BIN;
         }

         /* Set the default interpreter as self */
         process->interpreter = hx_strdup(imageName, request_object->allocator);
         process->path = hx_strdup(filename, request_object->allocator);
#endif

      }
   }

   *cmdLine = '\0';
   *arg = '\0';

   if(process->type != HX_UNKNOWN)
   {
      /* form the argument */
      if(process->type == HX_SCRIPT)
      {

#ifdef _HX_WIN32_

         strcat(arg, "\"");
#endif

         strcat(arg, request_object->filename);

#ifdef _HX_WIN32_
         strcat(arg, "\"");
#endif
         process->arg = hx_strdup(arg, process->allocator);
      }

      /* create the command line */
      switch(process->type)
      {
         case HX_SCRIPT:
            strcat(cmdLine, process->interpreter);
            strcat(cmdLine, " ");
            strcat(cmdLine, arg);
            break;
         case HX_UNKNOWN:
         case HX_BIN:
            strcat(cmdLine, "\"");
            strcat(cmdLine, request_object->filename);
            strcat(cmdLine, "\"");
            break;
      }

      process->cmdLine = hx_strdup(cmdLine, process->allocator);

      /* get the execution directory */
      pch = strrchr(filename, '/');

      if(pch)
      {
         *pch = '\0';
         process->dir = hx_strdup(filename, process->allocator);
      }
      else
         process->dir = NULL;
   }

   hx_free_mem(filename, request_object->allocator);
   return OK;
}

#ifdef _HX_WIN32_
static int
hx_createWIN32StdStr (hx_logsys_t *log,
                 DWORD  stdHANDLE,
                 HANDLE *hR,     /* read Handle */
                 HANDLE *hW,     /* write Handle */
                 HANDLE *hO)     /* original Handle */
{
   const unsigned char *rname = "hx_createWIN32StdStr ()";

   SECURITY_ATTRIBUTES sattr;

   /* Dup-ed handles */
   HANDLE hRead, hWrite, hDup;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   /* Initialize the security settings */
   memset(&sattr, 0x00, sizeof(sattr));

   /* Initialize the security attribute */
   sattr.nLength = sizeof(sattr);
   sattr.bInheritHandle = TRUE;
   sattr.lpSecurityDescriptor = NULL;

   if(stdHANDLE != STD_INPUT_HANDLE  &&
      stdHANDLE != STD_OUTPUT_HANDLE &&
      stdHANDLE != STD_ERROR_HANDLE )
   {
      hx_log_msg(log, HX_LERRO, rname, "Invalid standard stream type");
      return HX_ERR_STREAM_NUM;
   }

   /* Save the original Handle */
   *hO = GetStdHandle(stdHANDLE);
   if(INVALID_HANDLE_VALUE == *hO)
   {
      hx_log_msg(log, HX_LERRO, rname, "GetStdHandle() failed");
      hx_WIN32Err(log);
      return HX_ERR_HANDLEFUNC;
   }

   if(!CreatePipe(&hRead,   /* Read End */
                  &hWrite,  /* Write End */
                  &sattr,
                  0))
   {
      hx_log_msg(log, HX_LERRO, rname, "CreatePipe() failed");
      hx_WIN32Err(log);
      return HX_ERR_CREATEPIPE;
   }

   switch(stdHANDLE)
   {
      case STD_INPUT_HANDLE:
      {
         if(!SetStdHandle(STD_INPUT_HANDLE, &hRead))
         {
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "SetStdHandle() failed at %d",
                      __LINE__);

            hx_WIN32Err(log);
            return HX_ERR_HANDLEFUNC;
         }

         /* Make write end non inheritable and close
            inheritable write handle */
         if(!(DuplicateHandle(GetCurrentProcess(),
                              hWrite,                 /* Original Handle */
                              GetCurrentProcess(),
                              &hDup,                  /* Duplicated handle */
                              0,
                              FALSE,                  /* Set Inheritable to false */
                              DUPLICATE_SAME_ACCESS)))/* Access Rights */
         {
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "DuplicateHandle() failed at %d",
                      __LINE__);

            hx_WIN32Err(log);
            return HX_ERR_HANDLEFUNC;
         }

         /* Close the original handle */
         CloseHandle(hWrite);

         /* Copy the handles */
         *hR = hRead;
         *hW = hDup;
         break;
      }
      case STD_OUTPUT_HANDLE:
      {
         if(!SetStdHandle(STD_OUTPUT_HANDLE, &hWrite))
         {
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "SetStdHandle() failed at %d",
                      __LINE__);

            hx_WIN32Err(log);
            return HX_ERR_HANDLEFUNC;
         }

         /* Make read end non-inheritable and close
            inheritable read handle */
         if(!(DuplicateHandle(GetCurrentProcess(),
                              hRead,                  /* Original Handle */
                              GetCurrentProcess(),
                              &hDup,                  /* Duplicated handle */
                              0,
                              FALSE,                  /* Set Inheritable to false */
                              DUPLICATE_SAME_ACCESS)))/* Access Rights */
         {
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "DuplicateHandle() failed at %d",
                      __LINE__);

            hx_WIN32Err(log);
            return HX_ERR_HANDLEFUNC;
         }

         /* Close the original handle */
         CloseHandle(hRead);

         /* Copy the handles */
         *hW = hWrite;
         *hR = hDup;
         break;
      }
      case STD_ERROR_HANDLE:
      {
         if(!SetStdHandle(STD_ERROR_HANDLE, &hWrite))
         {
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "SetStdHandle() failed at %d",
                      __LINE__);

            hx_WIN32Err(log);
            return HX_ERR_HANDLEFUNC;
         }

         /* Make write end non inheritable and close
            inheritable write handle */
         if(!(DuplicateHandle(GetCurrentProcess(),
                              hRead,                  /* Original Handle */
                              GetCurrentProcess(),
                              &hDup,                  /* Duplicated handle */
                              0,
                              FALSE,                  /* Set Inheritable to false */
                              DUPLICATE_SAME_ACCESS)))/* Access Rights */
         {
            hx_log_msg(log,
                      HX_LERRO,
                      rname,
                      "DuplicateHandle() failed at %d",
                      __LINE__);

            hx_WIN32Err(log);
            return HX_ERR_HANDLEFUNC;
         }

         /* Close the original handle */
         CloseHandle(hRead);

         /* Copy the handles */
         *hW = hWrite;
         *hR = hDup;
         break;
      }
   }
   return OK;
}

static int
hx_initWIN32Process (HANDLE *streams,
                hx_process_t **process,
                http_request_t *request)
{
   const unsigned char *rname = "hx_initWIN32Process ()";
   int                  retval;

   STARTUPINFO          *si;

   hx_log_msg(request->vhost->log, HX_LCALL, rname, "Called");

   /* allocate mem for the process structure */
   *process = (hx_process_t *)hx_alloc_mem(sizeof(hx_process_t),
                                        request->allocator);
   if(!(*process))
      return HX_ERR_MEM_ALLOC;

   /* clean the mem */
   memset(*process, 0x00, sizeof(hx_process_t));

   /* set the allocator */
   (*process)->allocator = request->allocator;

   /* allocate mem for the PROCESS INFO struct */
   (*process)->pinfo = (PROCESS_INFORMATION *)hx_alloc_mem(sizeof
                                                        (PROCESS_INFORMATION),
                                                        (*process)->allocator);
   if(!(*process)->pinfo)
      return HX_ERR_MEM_ALLOC;

   /* clean the memory */
   memset((*process)->pinfo, 0x00, sizeof(PROCESS_INFORMATION));

   /* allocate mem for the STARTUP INFO struct */
   (*process)->sinfo = (STARTUPINFO *)hx_alloc_mem(sizeof(STARTUPINFO),
                                                (*process)->allocator);
   if(!(*process)->sinfo)
      return HX_ERR_MEM_ALLOC;

   si = (*process)->sinfo;

   /* clean the memory */
   memset(si, 0x00, sizeof(STARTUPINFO));

   /* Initialize the STARTUP INFO structure */
   si->cb          = sizeof(si);
   si->dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
   si->wShowWindow = SW_HIDE;

   /* Associate the child std streams */
   si->hStdInput   = streams[HX_STDIN];
   si->hStdOutput  = streams[HX_STDOUT];
   si->hStdError   = streams[HX_STDERR];

   /* create the environment block */
   retval = hx_create_win32_env_block(request->env,
                              &((*process)->env_block),
                              (*process)->allocator);
   if(retval != OK)
      return retval;

   /* get the interpreter name and file type */
   retval = hx_getInterpreter(request, (*process));

   if(retval != OK)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "Error while determining the interpreter");
      return HX_ERR_INTERPRETER;
   }

   if((*process)->type == HX_UNKNOWN)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "Could not determine interpreter for \"%s\"",
                request->filename);
      return HX_ERR_INTERPRETER;
   }

   return OK;
}
#elif defined (_HX_UNIX_)

/* hx_initUNIXProcess
   Initializes the process structure,
   determined the interpreter used to invoke
   a script
*/
static int
hx_initUNIXProcess (hx_process_t **process,
                    http_request_t *request)
{
   const unsigned char *rname = "hx_initUNIXProcess ()";
   int                  retval;

   hx_log_msg(request->vhost->log, HX_LCALL, rname, "Called");

   /* allocate mem for the process structure */
   *process = (hx_process_t *)hx_alloc_mem(sizeof(hx_process_t),
                                        request->allocator);
   if(!(*process))
      return HX_ERR_MEM_ALLOC;

   /* clean the mem */
   memset(*process, 0x00, sizeof(hx_process_t));

   /* set the allocator */
   (*process)->allocator = request->allocator;

   /* get the interpreter name and file type */
   retval = hx_getInterpreter(request, (*process));

   if(retval != OK)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "Error while determining the interpreter");
      return HX_ERR_INTERPRETER;
   }

   if((*process)->type == HX_UNKNOWN)
   {
      hx_log_msg(request->vhost->log,
                HX_LWARN,
                rname,
                "Could not determine interpreter for \"%s\". Trying to execute as a binary",
                request->filename);
   }

   /* Populate the enviroment */
   retval = hx_create_unix_env_block(request->env,
                                  &((*process)->env_block),
                                  (*process)->allocator);

   if(retval != OK)
      return retval;

   return OK;
}
#endif

/* hx_createProcess
   Creates an process in an
   OS dependent manner.
*/
static int
hx_createProcess (http_request_t *request_object, hx_process_t *process)
{
   const unsigned char *rname = "hx_createProcess ()";

   int retval;
   int status;

   hx_log_msg(request_object->vhost->log, HX_LCALL, rname, "Called");
   hx_log_msg(request_object->vhost->log, HX_LDEBG, rname, "Process details");
   hx_displayProcess(request_object->vhost->log, process, HX_LDEBG);

#ifdef _HX_WIN32_

   /* Create the actual process */
   if(!CreateProcess(NULL,                    /* ApplicationName     */
                     process->cmdLine,        /* CommandLine         */
                     NULL,                    /* ProcessAttributes   */
                     NULL,                    /* ThreadAttributes    */
                     TRUE,                    /* InheritHandles      */
                     0,                       /* dwCreationFlags     */
                     process->env_block,       /* Environment         */
                     process->dir,            /* CurrentDirectory    */
                     process->sinfo,          /* StartupInfo         */
                     process->pinfo))         /* ProcessInformation  */
   {
      hx_log_msg(request_object->vhost->log,
                HX_LERRO,
                rname,
                "CreateProcess() failed at %d",
                __LINE__);

      hx_WIN32Err(request_object->vhost->log);
      return HX_ERR_PROCESS_CREATE;
   }

   /* Close unused handles */
   CloseHandle(process->pinfo->hProcess);
   CloseHandle(process->pinfo->hThread);

#elif defined(_HX_UNIX_)

   /* fork a child */
   retval = fork();

   if(0 == retval)
   {
      /* Duplicate the pipe ends to refer to
         standard streams */
      if(dup2(process->childStreams[HX_STDIN], STDIN_FILENO) == -1)
      {
         hx_log_msg(request_object->vhost->log,
                   HX_LERRO,
                   rname,
                   "Error trying to duplicate standard input [%s]",
                   strerror(errno));
         exit(-1);
      }


      if(dup2(process->childStreams[HX_STDOUT], STDOUT_FILENO) == -1)
      {
         hx_log_msg(request_object->vhost->log,
                   HX_LERRO,
                   rname,
                   "Error trying to duplicate standard output [%s]",
                   strerror(errno));
         exit(-1);
      }

      if(dup2(process->childStreams[HX_STDERR], STDERR_FILENO) == -1)
      {
         hx_log_msg(request_object->vhost->log,
                   HX_LERRO,
                   rname,
                   "Error trying to duplicate standard error [%s]",
                   strerror(errno));
         exit(-1);
      }

      /* Execute the process */
      if(process->type == HX_SCRIPT)
      {
         hx_log_msg(request_object->vhost->log,
                   HX_LDEBG,
                   rname,
                   "Trying to invoke script %s %s",
                   process->interpreter,
                   process->arg);

         retval = execle(process->path,
                         process->interpreter,
                         process->arg,
                         NULL,
                         process->env_block);
      }
      else if(process->type == HX_BIN ||
              process->type == HX_UNKNOWN)
      {
         hx_log_msg(request_object->vhost->log,
                   HX_LDEBG,
                   rname,
                   "Trying to invoke executable %s",
                   process->interpreter);

         retval = execle(process->path,
                         process->interpreter,
                         NULL,
                         process->env_block);
      }

      hx_log_msg(request_object->vhost->log,
                HX_LERRO,
                rname,
                "Exec failed [%s]", strerror(errno));
      exit(-1);
   }
   else if(retval > 0)
   {
      /* Set the pid of the child process.
         This will be used later to collect
         the exit status
      */
      process->pid = retval;
   }
   else
   {
      hx_log_msg(request_object->vhost->log,
                HX_LERRO,
                rname,
                "fork() failed at %d [%s]",
                __LINE__,
                strerror(errno));
      return HX_ERR_PROCESS_CREATE;
   }
#endif

   hx_log_msg(request_object->vhost->log,
             HX_LDEBG,
             rname,
             "Successfully created CGI process");

   return OK;
}

/* hx_cleanProc
   Cleans a child process
*/
int
hx_cleanProc(hx_logsys_t *log, hx_process_t *child)
{
   const unsigned char *rname = "hx_cleanProc ()";
   int status = 0;

   hx_log_msg(log, HX_LCALL, rname, "Called");

   if(!child)
      return ERRC;

#ifdef _HX_WIN32_

   /* close the standard handles */
   CloseHandle(child->childHandles[HX_STDOUT]);
   CloseHandle(child->childHandles[HX_STDIN]);
   CloseHandle(child->childHandles[HX_STDERR]);

#elif defined(_HX_UNIX_)

   /* close the standard streams */
   close(child->parentStreams[HX_STDOUT]);
   close(child->parentStreams[HX_STDIN]);
   close(child->parentStreams[HX_STDERR]);

   /* collect the child exit status
      and deliver zombies from suffering */
   hx_log_msg(log,
             HX_LDEBG,
             rname,
             "Retrieving exit status for child proc %d",
             child->pid);

   waitpid(child->pid, &status, 0);

   if(!(WIFEXITED(status)))
   {
      hx_log_msg(log,
                HX_LERRO,
                rname,
                "Error with CGI process");
   }
   else
   {
      status = WEXITSTATUS(status);
      hx_log_msg(log, HX_LDEBG, rname, "CGI process returned %d", status);
   }

#endif

   return OK;
}

/* hx_kill_child
   Terminates a child CGI
   process
*/
int
hx_kill_child(hx_logsys_t *log, hx_process_t *child)
{
   const unsigned char *rname = "hx_kill_child ()";

   hx_log_msg(log, HX_LCALL, rname, "Called");

   if(!child)
      return ERRC;

#ifdef _HX_UNIX_

   kill(child->pid, SIGKILL);

#endif

   return OK;
}

int
hx_invokeCGI (http_request_t *request, hx_process_t **childProc)
{
   const unsigned char *rname = "hx_invokeCGI ()";
   int retval;

#ifdef _HX_WIN32_

   /* Child 0,1,2 Read Ends */
   HANDLE hChildSTDINr, hChildSTDOUTr, hChildSTDERRr;

   /* Child 0,1,2 Write Ends */
   HANDLE hChildSTDINw, hChildSTDOUTw, hChildSTDERRw;

   /* Current STDIN, STDOUT, and STDERR handles */
   HANDLE hSTDIN, hSTDOUT, hSTDERR;

   HANDLE *std;

#elif defined(_HX_UNIX_)

   /* Child 0,1,2 Read Ends */
   int cstdout[2], cstdin[2], cstderr[2];

#endif

   hx_log_msg(request->vhost->log, HX_LCALL, rname, "Called");

#ifdef _HX_WIN32_

   /* allocate mem for the handles */
   std = (HANDLE *)hx_alloc_mem((3 * sizeof(HANDLE)), request->allocator);

   /* Create a pipe and set its read
      end to be the STDIN of the child */
   retval = hx_createWIN32StdStr (request->vhost->log,
                                  STD_INPUT_HANDLE,
                                  &hChildSTDINr,     /* read Handle */
                                  &hChildSTDINw,     /* write Handle */
                                  &hSTDIN);      /* original Handle */

   if(retval != OK)
      return retval;

   /* Create a pipe and set its write
      end to be the STDOUT of the child */
   retval = hx_createWIN32StdStr (request->vhost->log,
                                  STD_OUTPUT_HANDLE,
                                  &hChildSTDOUTr,     /* read Handle */
                                  &hChildSTDOUTw,     /* write Handle */
                                  &hSTDOUT);      /* original Handle */

   if(retval != OK)
   {
      CloseHandle(hChildSTDINr);
      CloseHandle(hChildSTDINw);
      return retval;
   }

   /* Create a pipe and set its write
      end to be the STDERR of the child */
   retval = hx_createWIN32StdStr (request->vhost->log,
                                  STD_ERROR_HANDLE,
                                  &hChildSTDERRr,     /* read Handle */
                                  &hChildSTDERRw,     /* write Handle */
                                  &hSTDERR);      /* original Handle */

   if(retval != OK)
   {
      CloseHandle(hChildSTDINr);
      CloseHandle(hChildSTDINw);
      CloseHandle(hChildSTDOUTr);
      CloseHandle(hChildSTDOUTw);
      return retval;
   }

   /* copy the handles required by the
      child to the handles array */
   std[HX_STDIN]  = hChildSTDINr;
   std[HX_STDOUT] = hChildSTDOUTw;
   std[HX_STDERR] = hChildSTDERRw;

   /* Initialize the process structure
      and populate with requisite details */
   retval =  hx_initWIN32Process(std, childProc, request);

   if(retval != OK)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "Could not initialize Process structure");

      CloseHandle(hChildSTDINr);
      CloseHandle(hChildSTDINw);
      CloseHandle(hChildSTDOUTr);
      CloseHandle(hChildSTDOUTw);
      CloseHandle(hChildSTDERRr);
      CloseHandle(hChildSTDERRw);
      return retval;
   }

   /* CALL CreateProcess */
   retval = hx_createProcess(request, *childProc);

   if(retval != OK)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "hx_createProcess () failed");

      CloseHandle(hChildSTDINw);
      CloseHandle(hChildSTDINr);

      CloseHandle(hChildSTDOUTw);
      CloseHandle(hChildSTDOUTr);

      CloseHandle(hChildSTDERRw);
      CloseHandle(hChildSTDERRr);

      return retval;
   }

   /* Closed the unused pipe ends in the
      parent process */
   CloseHandle(hChildSTDINr);
   CloseHandle(hChildSTDOUTw);
   CloseHandle(hChildSTDERRw);

   /* Restore the STDIN */
   if(!SetStdHandle(STD_INPUT_HANDLE, hSTDIN))
      return HX_ERR_PIPEFUNC;

   /* Restore the STDOUT */
   if(!SetStdHandle(STD_OUTPUT_HANDLE, hSTDOUT))
      return HX_ERR_PIPEFUNC;

   /* Restore the STDERR */
   if(!SetStdHandle(STD_ERROR_HANDLE, hSTDERR))
      return HX_ERR_PIPEFUNC;

   /* Set the handles using which self will
      communicate with the child */
   (*childProc)->childHandles[HX_STDIN] = hChildSTDINw;
   (*childProc)->childHandles[HX_STDOUT] = hChildSTDOUTr;
   (*childProc)->childHandles[HX_STDERR] = hChildSTDERRr;

#elif defined(_HX_UNIX_)

   /* Create a pipe and set its read
      end to be the STDIN of the child */
   retval = pipe(cstdin);

   if(retval)
      return retval;

   /* Create a pipe and set its write
      end to be the STDOUT of the child */
   retval = pipe(cstdout);

   if(retval)
   {
      close(cstdin[0]);
      close(cstdin[1]);
      return retval;
   }

   /* Create a pipe and set its write
      end to be the STDOUT of the child */
   retval = pipe(cstderr);

   if(retval)
   {
      close(cstdin[0]);
      close(cstdin[1]);
      close(cstdout[0]);
      close(cstdout[1]);
      return retval;
   }

   /* Initialize the process structure
      and populate with requisite details */
   retval =  hx_initUNIXProcess(childProc, request);

   if(retval != OK)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "Could not initialize Process structure");

      close(cstdin[0]);
      close(cstdin[1]);
      close(cstdout[0]);
      close(cstdout[1]);
      close(cstderr[0]);
      close(cstderr[1]);
      return retval;
   }

   /* copy the handles required by the
      child to the handles array */
   (*childProc)->childStreams[HX_STDIN] = cstdin[0];
   (*childProc)->childStreams[HX_STDOUT] = cstdout[1];
   (*childProc)->childStreams[HX_STDERR] = cstderr[1];

   /* Set the handles using which self will
      communicate with the child */
   (*childProc)->parentStreams[HX_STDIN] = cstdin[1];
   (*childProc)->parentStreams[HX_STDOUT] = cstdout[0];
   (*childProc)->parentStreams[HX_STDERR] = cstderr[0];

   /* CALL CreateProcess */
   retval = hx_createProcess(request, *childProc);

   /* Closed the unused pipe ends in the
      parent process */
   close(cstdin[0]);
   close(cstdout[1]);
   close(cstderr[1]);

   if(retval != OK)
   {
      hx_log_msg(request->vhost->log,
                HX_LERRO,
                rname,
                "hx_createProcess () failed");

      close(cstdin[1]);
      close(cstdout[0]);
      close(cstderr[0]);
      return retval;
   }

#endif
   /* Return back to the CGI handler */
   return OK;
}
/* End of File */

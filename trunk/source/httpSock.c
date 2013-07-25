/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#include <stdio.h>

#ifdef _HX_UNIX_
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#endif

#include "httpCommon.h"
#include "httpInclude.h"

#include "httpSock.h"
#include "httpLog.h"

#ifdef _HX_WIN32_
static hx_sockerr_t errCode[] =
{
/*00*/  { "A blocking operation was interrupted by a call to WSACancelBlockingCall", WSAEINTR                                                                                                                            },
/*01*/  { "The file handle supplied is not valid", WSAEBADF                                                                                                                                                              },
/*02*/  { "An attempt was made to access a socket in a way forbidden by its access permissions", WSAEACCES                                                                                                               },
/*03*/  { "The system detected an invalid pointer address in attempting to use a pointer argument in a call", WSAEFAULT                                                                                                  },
/*04*/  { "An invalid argument was supplied", WSAEINVAL                                                                                                                                                                  },
/*05*/  { "Too many open sockets", WSAEMFILE                                                                                                                                                                             },
/*06*/  { "A non-blocking socket operation could not be completed immediately", WSAEWOULDBLOCK                                                                                                                           },
/*07*/  { "A blocking operation is currently executing", WSAEINPROGRESS                                                                                                                                                  },
/*08*/  { "An operation was attempted on a non-blocking socket that already had an operation in progress", WSAEALREADY                                                                                                   },
/*09*/  { "An operation was attempted on something that is not a socket", WSAENOTSOCK                                                                                                                                    },
/*10*/  { "A required address was omitted from an operation on a socket", WSAEDESTADDRREQ                                                                                                                                },
/*11*/  { "A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram into was smaller than the datagram itself", WSAEMSGSIZE },
/*12*/  { "A protocol was specified in the socket function call that does not support the semantics of the socket type requested", WSAEPROTOTYPE                                                                         },
/*13*/  { "An unknown, invalid, or unsupported option or level was specified in a getsockopt or setsockopt call", WSAENOPROTOOPT                                                                                         },
/*14*/  { "The requested protocol has not been configured into the system, or no implementation for it exists", WSAEPROTONOSUPPORT                                                                                       },
/*15*/  { "The support for the specified socket type does not exist in this address family", WSAESOCKTNOSUPPORT                                                                                                          },
/*16*/  { "The attempted operation is not supported for the type of object referenced", WSAEOPNOTSUPP                                                                                                                    },
/*17*/  { "The protocol family has not been configured into the system or no implementation for it exists", WSAEPFNOSUPPORT                                                                                              },
/*18*/  { "An address incompatible with the requested protocol was used", WSAEAFNOSUPPORT                                                                                                                                },
/*29*/  { "Only one usage of each socket address (protocol/network address/port) is normally permitted", WSAEADDRINUSE                                                                                                   },
/*20*/  { "The requested address is not valid in its context", WSAEADDRNOTAVAIL                                                                                                                                          },
/*21*/  { "A socket operation encountered a dead network", WSAENETDOWN                                                                                                                                                   },
/*22*/  { "A socket operation was attempted to an unreachable network", WSAENETUNREACH                                                                                                                                   },
/*23*/  { "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress", WSAENETRESET                                                                              },
/*24*/  { "An established connection was aborted by the software in your host machine", WSAECONNABORTED                                                                                                                  },
/*25*/  { "An existing connection was forcibly closed by the remote host", WSAECONNRESET                                                                                                                                 },
/*26*/  { "An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full", WSAENOBUFS                                                                    },
/*27*/  { "A connect request was made on an already connected socket", WSAEISCONN                                                                                                                                        },
/*28*/  { "A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied", WSAENOTCONN                        },
/*39*/  { "A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call", WSAESHUTDOWN                                                 },
/*30*/  { "Too many references to some kernel object", WSAETOOMANYREFS                                                                                                                                                   },
/*31*/  { "A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond", WSAETIMEDOUT         },
/*32*/  { "No connection could be made because the target machine actively refused it", WSAECONNREFUSED                                                                                                                  },
/*33*/  { "Cannot translate name", WSAELOOP                                                                                                                                                                              },
/*34*/  { "Name component or name was too long", WSAENAMETOOLONG                                                                                                                                                         },
/*35*/  { "A socket operation failed because the destination host was down", WSAEHOSTDOWN                                                                                                                                },
/*36*/  { "A socket operation was attempted to an unreachable host", WSAEHOSTUNREACH                                                                                                                                     },
/*37*/  { "Cannot remove a directory that is not empty", WSAENOTEMPTY                                                                                                                                                    },
/*38*/  { "A Windows Sockets implementation may have a limit on the number of applications that may use it simultaneously", WSAEPROCLIM                                                                                  },
/*49*/  { "Ran out of quota", WSAEUSERS                                                                                                                                                                                  },
/*40*/  { "Ran out of disk quota", WSAEDQUOT                                                                                                                                                                             },
/*41*/  { "File handle reference is no longer available", WSAESTALE                                                                                                                                                      },
/*42*/  { "Item is not available locally", WSAEREMOTE                                                                                                                                                                    },
/*43*/  { "WSAStartup cannot function at this time because the underlying system it uses to provide network services is currently unavailable", WSASYSNOTREADY                                                           },
/*44*/  { "The Windows Sockets version requested is not supported", WSAVERNOTSUPPORTED                                                                                                                                   },
/*45*/  { "Either the application has not Called WSAStartup, or WSAStartup failed", WSANOTINITIALISED                                                                                                                    },
/*46*/  { "Returned by WSARecv or WSARecvFrom to indicate the remote party has initiated a graceful shutdown sequence", WSAEDISCON                                                                                       },
/*47*/  { "No more results can be returned by WSALookupServiceNext", WSAENOMORE                                                                                                                                          },
/*48*/  { "A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled", WSAECANCELLED                                                                                       },
/*59*/  { "The procedure call table is invalid", WSAEINVALIDPROCTABLE                                                                                                                                                    },
/*50*/  { "The requested service provider is invalid", WSAEINVALIDPROVIDER                                                                                                                                               },
/*51*/  { "The requested service provider could not be loaded or initialized", WSAEPROVIDERFAILEDINIT                                                                                                                    },
/*52*/  { "A system call that should never fail has failed", WSASYSCALLFAILURE                                                                                                                                           },
/*53*/  { "No such service is known. The service cannot be found in the specified name space", WSASERVICE_NOT_FOUND                                                                                                      },
/*54*/  { "The specified class was not found", WSATYPE_NOT_FOUND                                                                                                                                                         },
/*55*/  { "No more results can be returned by WSALookupServiceNext", WSA_E_NO_MORE                                                                                                                                       },
/*56*/  { "A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled", WSA_E_CANCELLED                                                                                     },
/*57*/  { "A database query failed because it was actively refused", WSAEREFUSED                                                                                                                                         },
/*58*/  { "No such host is known", WSAHOST_NOT_FOUND                                                                                                                                                                     },
/*69*/  { "This is usually a temporary error during hostname resolution and means that the local server did not receive a response from an authoritative server", WSATRY_AGAIN                                           },
/*60*/  { "A non-recoverable error occurred during a database lookup", WSANO_RECOVERY                                                                                                                                    },
/*61*/  { "The requested name is valid and was found in the database, but it does not have the correct associated data being resolved for", WSANO_DATA                                                                   },
/*62*/  { NULL, NULL                                                                                                                                                                                                     }
};
#endif

/* Initializes the Sockets
   Subsystem for win32
   platforms
*/
int
hx_sock_init(void)
{
#ifdef _HX_WIN32_
   WSADATA wsaData;

   if((WSAStartup((MAKEWORD(2,2)), &wsaData)) != 0)
   {
      return ERRC;
   }
#endif
   return OK;
}

/* Cleanup the Sockets Subsystem
   for win32 platforms
*/
void
hx_sockCleanup(void)
{
#ifdef _HX_WIN32_
   WSACleanup();
#endif
   return;
}

/* close a socket */
void
hx_sockClose(SOCKET s, int how)
{
   int direction = HX_BOTH;

#ifdef _HX_WIN32_
   switch(how)
   {
      case HX_BOTH:
         direction = SD_BOTH;
         break;
      case HX_SEND:
         direction = SD_SEND;
         break;
      case HX_READ:
         direction = SD_RECEIVE;
         break;
      default:
         break;
   }
   shutdown(s, direction);
   closesocket(s);
#elif defined(_HX_UNIX_)
   switch(how)
   {
      case HX_BOTH:
         direction = SHUT_RDWR;
         break;
      case HX_SEND:
         direction = SHUT_WR;
         break;
      case HX_READ:
         direction = SHUT_RD;
         break;
      default:
         break;
   }
   shutdown(s, direction);
   close(s);
#endif

}

/*
    Creates a passive TCP socket listening on arg <port>
    of connection queue length <queuelen>
*/
SOCKET
hx_serverSock(hx_logsys_t *log, short port, int queuelen)
{
    struct sockaddr_in serveraddr;
    SOCKET sock;
    int retval;

    /* create socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);

    if(sock == ERRC)
    {
        hx_sockErr(log, "socket() failed", hx_get_socket_error());
        return ERRC;
    }

    /* initialize the address */
    /* and assign it value    */
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    /* bind the socket to the local addr */
    retval = bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if(retval == ERRC)
    {
        hx_sockErr(log, "bind() failed", hx_get_socket_error());
        return ERRC;
    }

    /* place the socket in listening mode */
    retval = listen(sock, queuelen);
    if(retval == ERRC)
    {
        hx_sockErr(log, "listen() failed", hx_get_socket_error());
        return ERRC;
    }

    return sock;
}

/*
    Creates a TCP socket connected to
    a specific host:port
*/
SOCKET
hx_clientSock(hx_logsys_t *log, const unsigned char *host, short port)
{
   const char *rname = "hx_clientSock ()";
   SOCKET sock;
   struct sockaddr_in serveraddr;
   struct hostent *pHostent;
   struct servent *pServent;
   int retval;

   /* create socket */
   sock = socket(PF_INET, SOCK_STREAM, 0);

   if(sock == ERRC)
   {
      hx_sockErr(log, "Failed to create socket", hx_get_socket_error());
      return ERRC;
   }

   /* initialize the address */
   memset(&serveraddr, 0, sizeof(serveraddr));
   /* set family */
   serveraddr.sin_family = AF_INET;

   /* get server address */
   if((pHostent = gethostbyname(host)) != NULL)
   {
      memcpy(&serveraddr.sin_addr, pHostent->h_addr, pHostent->h_length);
   }
   else if((serveraddr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
   {
      hx_log_msg(log, HX_LERRO, rname, "Failed to lookup Server Address");
      return ERRC;
   }

   /* set port */
   serveraddr.sin_port = htons(port);

   /* Connect the socket to the server */
   retval = connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

   if(retval)
   {
      hx_sockErr(log, "Failed to connect to Server", hx_get_socket_error());
      return ERRC;
   }
   return sock;
}

/*
   hx_get_socket_error
   OS independent socket error
   retrieval procedure
*/

int hx_get_socket_error(void)
{
#ifdef _HX_WIN32_
   return WSAGetLastError();
#elif defined(_HX_UNIX_)
   return errno;
#endif
}

int hx_sockErr(hx_logsys_t *log, const unsigned char * errstr, int error)
{
   const unsigned char *rname = "hx_sockErr ()";
#ifdef _HX_WIN32_
   int count;
   for(count = 0; errCode[count].desc; count++)
   {
      if(error == errCode[count].error_num)
      {
         hx_log_msg(log,
                   HX_LERRO,
                   rname,
                   "%s [ %s (at %d) ]\n",
                   errstr,
                   errCode[count].desc,
                   count);
         return error;
      }
   }
#elif defined(_HX_UNIX_)
   /* NO Error */
   hx_log_msg(log,
             HX_LERRO,
             rname,
             "%s [%s]\n",
             errstr,
             strerror(error));
#endif
   return 0;
}

/*
   hx_sock_read
   extracts the request data from the client
   connection.
   returns SOCKET_ERROR if error occurs,
   or if there is no data to read else
   returns the number of bytes read.
*/
int
hx_sock_read (SOCKET client_socket, unsigned char * buff, int len)
{
    /* set todo to non blocking mode */
    int todo = 1;
    int retval;
    int err;
    int retry;

    struct timeval tv;

    fd_set fdset;

#ifdef _HX_WIN32_
    /* set socket to non blocking mode */
    retval = ioctlsocket(client_socket, FIONBIO, (u_long *)&todo);
    if(retval == SOCKET_ERROR)
        return retval;

    /* set todo to 0, blocking mode */
    todo = 0;

    retval = recv(client_socket, buff, len, 0);

    if(retval == SOCKET_ERROR)
    {
        err = hx_get_socket_error();

        if(err == WSAEWOULDBLOCK)
        {
            /*  try to extract data from the socket
                till the TIMEOUT VAL */
            do
            {
                retry = 0;
                FD_ZERO(&fdset);
                FD_SET(client_socket, &fdset);

                /* set select timeval */
                tv.tv_sec = TIMEOUT_RECV;
                tv.tv_usec = 0;

                /* waits for TIMEOUT_RECV secs before checking */
                retval = select(FD_SETSIZE, &fdset, NULL, NULL, &tv);

                if (retval == SOCKET_ERROR)
                {
                    err = hx_get_socket_error();
                }
                else if (retval == 0)
                {
                    /* before returning set the
                       socket state to Blocking */
                    ioctlsocket(client_socket, FIONBIO, (u_long*)&todo);
                    WSASetLastError(WSAEWOULDBLOCK);
                    return (SOCKET_ERROR);
                }
                else
                {
                    retval = recv(client_socket, buff, len, 0);
                    if (retval == SOCKET_ERROR)
                    {
                        err = hx_get_socket_error();
                        /* select returned read won't block
                           but it actually is blocking */
                        if (err == WSAEWOULDBLOCK)
                        {
                            retry = 1;
                            hx_sleep(0.1);
                        }
                        /*  we break out of the loop
                            cause something bad happened
                            on the socket */
                    }
                }
            } while (retry);
        }
    }

    ioctlsocket(client_socket, FIONBIO, (u_long*)&todo);

    if (retval == SOCKET_ERROR)
        WSASetLastError(err);

#elif defined(_HX_UNIX_)

    /* set socket to non blocking mode */
    retval = ioctl(client_socket, FIONBIO, (u_long *)&todo);
    if(retval == -1)
        return retval;

    /* set todo to 0, blocking mode */
    todo = 0;

    retval = recv(client_socket, buff, len, 0);

    if(retval == -1)
    {
        err = hx_get_socket_error();

        if(err == EAGAIN)
        {
            /*  try to extract data from the socket
                till the TIMEOUT VAL */
            do
            {
                retry = 0;
                FD_ZERO(&fdset);
                FD_SET(client_socket, &fdset);

                /* set select timeval */
                tv.tv_sec = TIMEOUT_RECV;
                tv.tv_usec = 0;

                /* waits for TIMEOUT_RECV secs before checking */
                retval = select(FD_SETSIZE, &fdset, NULL, NULL, &tv);

                if (retval == -1)
                {
                    err = hx_get_socket_error();
                }
                else if (retval == 0)
                {
                    /* before returning set the
                       socket state to Blocking */
                    ioctl(client_socket, FIONBIO, (u_long*)&todo);
                    return -1;
                }
                else
                {
                    retval = recv(client_socket, buff, len, 0);
                    if (retval == -1)
                    {
                        err = hx_get_socket_error();
                        /* select returned read won't block
                           but it actually is blocking */
                        if (err == EAGAIN)
                        {
                            retry = 1;
                            hx_sleep(0.1);
                        }
                        /*  we break out of the loop
                            cause something bad happened
                            on the socket */
                    }
                }
            } while (retry);
        }
    }

    ioctl(client_socket, FIONBIO, (u_long*)&todo);

#endif
    return (retval);

}

/*
   hx_sockWrite
*/

int
hx_sockWrite (SOCKET client_socket, unsigned char *buff, int len)
{

    /* set todo to non blocking mode */
    int todo = 1;
    int retval;
    int err;
    int retry;

    struct timeval tv;

    fd_set fdset;

#ifdef _HX_WIN32_
    /* set socket to non blocking mode */
    retval = ioctlsocket(client_socket, FIONBIO, (u_long *)&todo);
    if(retval == SOCKET_ERROR)
        return retval;

    /* set todo to 0, blocking mode */
    todo = 0;

    retval = send(client_socket, buff, len, 0);

    if(retval == SOCKET_ERROR)
    {
        err = hx_get_socket_error();

        if(err == WSAEWOULDBLOCK)
        {
            /*  try to extract data from the socket
                till the TIMEOUT VAL */
            do
            {
                retry = 0;
                FD_ZERO(&fdset);
                FD_SET(client_socket, &fdset);

                /* set select timeval */
                tv.tv_sec = TIMEOUT_SEND;
                tv.tv_usec = 0;

                /* waits for TIMEOUT_RECV secs before checking */
                retval = select(FD_SETSIZE, &fdset, NULL, NULL, &tv);

                if (retval == SOCKET_ERROR)
                {
                    err = hx_get_socket_error();
                }
                else if (retval == 0)
                {
                    /* before returning set the
                       socket state to Blocking */
                    ioctlsocket(client_socket, FIONBIO, (u_long*)&todo);
                    WSASetLastError(WSAEWOULDBLOCK);
                    return (SOCKET_ERROR);
                }
                else
                {
                    retval = send(client_socket, buff, len, 0);
                    if (retval == SOCKET_ERROR)
                    {
                        err = hx_get_socket_error();
                        /* select returned read won't block
                           but it actually is blocking */
                        if (err == WSAEWOULDBLOCK)
                        {
                            retry = 1;
                            hx_sleep(0.1);
                        }
                        /*  we break out of the loop
                            cause something bad happened
                            on the socket */
                    }
                }
            } while (retry);
        }
    }


    ioctlsocket(client_socket, FIONBIO, (u_long*)&todo);

    if (retval == SOCKET_ERROR)
        WSASetLastError(err);
#elif defined(_HX_UNIX_)
    /* set socket to non blocking mode */
    retval = ioctl(client_socket, FIONBIO, (u_long *)&todo);
    if(retval == -1)
        return retval;

    /* set todo to 0, blocking mode */
    todo = 0;

    retval = send(client_socket, buff, len, 0);

    if(retval == -1)
    {
        err = hx_get_socket_error();

        if(err == EAGAIN)
        {
            /*  try to extract data from the socket
                till the TIMEOUT VAL */
            do
            {
                retry = 0;
                FD_ZERO(&fdset);
                FD_SET(client_socket, &fdset);

                /* set select timeval */
                tv.tv_sec = TIMEOUT_SEND;
                tv.tv_usec = 0;

                /* waits for TIMEOUT_RECV secs before checking */
                retval = select(FD_SETSIZE, &fdset, NULL, NULL, &tv);

                if (retval == -1)
                {
                    err = hx_get_socket_error();
                }
                else if (retval == 0)
                {
                    /* before returning set the
                       socket state to Blocking */
                    ioctl(client_socket, FIONBIO, (u_long*)&todo);
                    return -1;
                }
                else
                {
                    retval = send(client_socket, buff, len, 0);
                    if (retval == -1)
                    {
                        err = hx_get_socket_error();
                        /* select returned read won't block
                           but it actually is blocking */
                        if (err == EAGAIN)
                        {
                            retry = 1;
                            hx_sleep(0.1);
                        }
                        /*  we break out of the loop
                            cause something bad happened
                            on the socket */
                    }
                }
            } while (retry);
        }
    }


    ioctl(client_socket, FIONBIO, (u_long*)&todo);

#endif
    return (retval);
}
/* End of File */

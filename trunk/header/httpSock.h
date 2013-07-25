/*
 * Copyright (c) 2003 Avinandan Sengupta. All rights reserved.
 *
 */

#ifndef HTTPSOCK_H
#define HTTPSOCK_H

#ifdef _HX_WIN32_
#include <winsock2.h>
#elif defined(_HX_UNIX_)
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "httpCommon.h"
#include "httpInclude.h"

typedef struct sockErr
{
    unsigned char *desc;
    int error_num;
}hx_sockerr_t;

int hx_sock_init(void);
void hx_sockCleanup(void);
void hx_sockClose(SOCKET s, int how);


SOCKET hx_serverSock(hx_logsys_t *log,
                     short port,
                     int queuelen);
SOCKET hx_clientSock(hx_logsys_t *log,
                     const unsigned char *host,
                     short port);

int hx_sockErr(hx_logsys_t *log, const unsigned char * errstr, int error);
int hx_get_socket_error(void);
int hx_sock_read (SOCKET client_socket, unsigned char * buff, int len);
int hx_sockWrite (SOCKET client_socket, unsigned char *response, int len);

#endif
/* End of file */

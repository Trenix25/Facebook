/*

     setup_af_inet6.c

     This function creates a pair of sockets in the AF_INET6 domain
     and connects them to each other.  If a socket file descriptor
     is not -1 then it will presume that the socket already exists
     and proceed to try to connect it.

     Written by Matthew Campbell.

*/

#ifndef _SETUP_AF_INET6_C
#define _SETUP_AF_INET6_C

#include "sockets.h"

int setup_af_inet6( int *csock_fd, int *lsock_fd, int *ssock_fd,
                    int domain, int *type, void *address, int initial )
{
     errno = ENOSYS;
     return ( -1 );
}

#endif  /* _SETUP_AF_INET6_C */

/* EOF setup_af_inet6.c */

/*

     setup_af_bluetooth.c

     This function creates sockets in the AF_BLUETOOTH domain
     and connects them to each other.  If a socket file descriptor
     is not -1 then it will presume that the socket already exists
     and proceed to try to connect it.

     Written by Matthew Campbell.

*/

#ifndef _SETUP_AF_BLUETOOTH_C
#define _SETUP_AF_BLUETOOTH_C

#include "sockets.h"

int setup_af_bluetooth( int *csock_fd, int *lsock_fd, int *ssock_fd,
                        int domain, int *type, void *address, int initial )
{
     errno = ENOSYS;
     return ( -1 );
}

#endif  /* _SETUP_AF_BLUETOOTH_C */

/* EOF setup_af_bluetooth.c */

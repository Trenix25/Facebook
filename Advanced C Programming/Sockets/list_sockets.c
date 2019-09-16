/*

     list_sockets.c
     This function lists the socket file descriptor numbers.
     Written by Matthew Campbell.

*/

#ifndef _LIST_SOCKETS_C
#define _LIST_SOCKETS_C

#include "sockets.h"

void list_sockets( int *csock_fd, int *lsock_fd, int *ssock_fd )
{
     if ( csock_fd == NULL || lsock_fd == NULL || ssock_fd == NULL )
     {
          printf( "\nNull pointer passed to list_sockets().\n\n" );
          errno = EINVAL;
          return;
     }
     printf( "\n*csock_fd: %d, *lsock_fd: %d, *ssock_fd: %d.\n\n",
             *csock_fd, *lsock_fd, *ssock_fd );
     errno = 0;
     return;
}

#endif  /* _LIST_SOCKETS_C */

/* EOF list_sockets.c */

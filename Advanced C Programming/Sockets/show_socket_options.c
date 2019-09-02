/*

     show_socket_options.c
     Displays some socket data.
     Written by Matthew Campbell.

*/

#ifndef _SHOW_SOCKET_OPTIONS_C
#define _SHOW_SOCKET_OPTIONS_C

#include "sockets.h"

#ifdef SHOW_SOCKET_OPTIONS

void show_socket_options( const int sock_fd, const int domain,
                          const int sock_type, const char *sock_name )
{
     if ( sock_fd < ( -1 ) )
     {
          errno = EINVAL;
          return;
     }
     if ( domain < 0 )
     {
          errno = EINVAL;
          return;
     }
     if ( sock_type < 0 )
     {
          errno = EINVAL;
          return;
     }
     if ( sock_name == NULL )
     {
          errno = EFAULT;
          return;
     }
     if ( sock_name[ 0 ] == 0 )
     {
          errno = ENODATA;
          return;
     }

     errno = ENOSYS;
     return;
}

#endif  /* SHOW_SOCKET_OPTIONS */

#endif  /* _SHOW_SOCKET_OPTIONS_C */

/* EOF show_socket_options.c */

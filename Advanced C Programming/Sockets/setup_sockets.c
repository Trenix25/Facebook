/*

     setup_sockets.c

     This function calls the correct function to create the sockets
     and get them connected.  This function can be called at the
     beginning of the program to get everything started, or later on
     if a connection is lost.

     Written by Matthew Campbell.

*/

#ifndef _SETUP_SOCKETS_C
#define _SETUP_SOCKETS_C

#include "sockets.h"

int setup_sockets( int *csock_fd, int *lsock_fd, int *ssock_fd,
                   int domain, int *type, void *address, int initial )
{
     int ret;

     if ( domain < 1 || domain > MAX_DOMAINS )
     {
          errno = EINVAL;
          return ( -1 );
     }
     if ( csock_fd == NULL || lsock_fd == NULL || ssock_fd == NULL )
     {
          errno = EFAULT;
          return ( -1 );
     }
     if ( *csock_fd < ( -1 ) || *lsock_fd < ( -1 ) || *ssock_fd < ( -1 ) )
     {
          errno = EINVAL;
          return ( -1 );
     }
     if ( type == NULL )
     {
          errno = EFAULT;
          return ( -1 );
     }
     if ( address == NULL )
     {
          errno = EFAULT;
          return ( -1 );
     }
     if ( initial != 0 && initial != 1 )
     {
          errno = EINVAL;
          return ( -1 );
     }

     switch( domain )
     {
           case 1: ret = setup_af_bluetooth( csock_fd, lsock_fd,
                                             ssock_fd, domain, type,
                                             address, initial );
                   break;
           case 2: ret = setup_af_inet( csock_fd, lsock_fd,
                                        ssock_fd, domain, type,
                                        address, initial );
                   break;
           case 3: ret = setup_af_inet6( csock_fd, lsock_fd,
                                         ssock_fd, domain, type,
                                         address, initial );
                   break;
           case 4: ret = setup_af_unix( csock_fd, lsock_fd,
                                        ssock_fd, domain, type,
                                        address, initial );
                   break;
          default: printf( "\n\
setup_sockets(): Error: Default case reached in switch() statement.\n\n" );
                   errno = 0;
                   ret = ( -1 );
                   break;
     }
     return ret;
}

#endif /* _SETUP_SOCKETS_C */

/* EOF setup_sockets.c */

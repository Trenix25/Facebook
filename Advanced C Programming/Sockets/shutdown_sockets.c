/*

     shutdown_sockets.c

     This function is called when the socket
     connections need to be shut down.

     Written by Matthew Campbell.

*/

#ifndef _SHUTDOWN_SOCKETS_C
#define _SHUTDOWN_SOCKETS_C

#include "sockets.h"

int shutdown_sockets( int *csock_fd, int *lsock_fd, int *ssock_fd,
                      int domain, int type )
{
     int ret, save_errno;
     struct stat sock_file;

     if ( domain < 1 || domain > ( MAX_DOMAINS + 1 ) )  /* Exit is 5. */
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
     if ( type < 0 )
     {
          errno = EINVAL;
          return ( -1 );
     }

#if 0

     /* Sync the client socket if it is open. */

     if ( *csock_fd >= 0 && type != SOCK_DGRAM )
     {
          ret = fsync( *csock_fd );
          if ( ret != 0 )
          {
               save_errno = errno;
               if ( save_errno != EINVAL )
               {
                    printf( "\n\
Something went wrong when trying to write out any remaining data in the\n\
client socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
                    errno = 0;
                    return ( -1 );
               }
               else
               {

#ifdef DEBUG

                    printf( "\
fsync( *csock_fd ) rejected as an invalid argument.\n" );

#endif

               }    /* if ( save_errno != EINVAL ) */

          }    /* if ( ret != 0 ) */

     }    /* if ( *csock_fd >= 0 ) */

     /* Sync the server socket if it is open. */

     if ( *ssock_fd >= 0 && type != SOCK_DGRAM )
     {
          ret = fsync( *ssock_fd );
          if ( ret != 0 )
          {
               save_errno = errno;
               if ( save_errno != EINVAL )
               {
                    printf( "\n\
Something went wrong when trying to write out any remaining data in the\n\
server socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
                    errno = 0;
                    return ( -1 );
               }
               else
               {

#ifdef DEBUG

                    printf( "\
fsync( *ssock_fd ) rejected as an invalid argument.\n" );

#endif

               }    /* if ( save_errno != EINVAL ) */

          }    /* if ( ret != 0 ) */

     }    /* if ( *ssock_fd >= 0 ) */

#endif

     /* Remove the socket file if it exists. */

     if ( domain == 4 )  /* AF_UNIX */
     {
          errno = 0;
          ret = stat( SOCK_NAME, &sock_file );
          if ( ret != 0 )
          {
               save_errno = errno;
               if ( save_errno != 0 && save_errno != ENOENT )
               {
                    printf( "\n\
Something went wrong when trying to stat the socket file \"%s\".\n",
                            SOCK_NAME );
                    printf( "Error: %s.\n\n", strerror( save_errno ) );
                    errno = 0;
                    return ( -1 );
               }
               else
               {
                    errno = 0;
               }
          }
          else
          {
               if ( S_ISSOCK( sock_file.st_mode ) )
               {
                    errno = 0;
                    ret = unlink( SOCK_NAME );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when trying to remove the socket file \"%s\".\n",
                                 SOCK_NAME );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                         errno = 0;
                         return ( -1 );
                    }
                    else  /* ret == 0 */
                    {

#ifdef DEBUG

                         printf( "\
Successfully removed the socket file \"%s\".\n", SOCK_NAME );

#endif

                    }  /* if ( ret != 0 ) */
               }
               else  /* The file is not a socket. */
               {
                    printf( "\nSocket file \"%s\" is not a socket.\n\n",
                            SOCK_NAME );
                    errno = 0;
                    return ( -1 );

               }    /* if ( S_ISSOCK( sock_file.st_mode ) ) */

          }    /* if ( ret != 0 ) */

     }    /* if ( domain == 4 ) */

     /* Close the server socket if it is open. */

     if ( *ssock_fd >= 0 )
     {
          ret = close( *ssock_fd );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when trying to close the server socket.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }
               printf( "\n" );
               errno = 0;
               return ( -1 );
          }
          *ssock_fd = -1;
     }

     /* Close the server's listening socket if it is open. */

     if ( *lsock_fd >= 0 )
     {
          ret = close( *lsock_fd );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when trying to close the server's listening socket.\
\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }
               printf( "\n" );
               errno = 0;
               return ( -1 );
          }
          *lsock_fd = -1;
     }

     /* Close the client socket if it is open. */

     if ( *csock_fd >= 0 )
     {
          ret = close( *csock_fd );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when trying to close the client socket.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }
               printf( "\n" );
               errno = 0;
               return ( -1 );
          }
          *csock_fd = -1;
     }

     return 0;
}

#endif /* _SHUTDOWN_SOCKETS_C */

/* EOF shutdown_sockets.c */

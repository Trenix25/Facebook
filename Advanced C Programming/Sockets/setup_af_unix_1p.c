/*

     setup_af_unix_1p.c

     This function creates sockets in the AF_UNIX domain and
     connects them to each other.  If a socket file descriptor
     is not -1 then it will presume that the socket already exists
     and proceed to try to connect it.

     There are two files that define setup_af_unix().  The single
     process version might be unstable because the call to connect(2)
     might cause the program to hang on some systems so the two
     process version was created.  You must define one and only
     one version to use in the Makefile.

     Written by Matthew Campbell.

*/

#ifndef _SETUP_AF_UNIX_1P_C
#define _SETUP_AF_UNIX_1P_C

#include "sockets.h"

int setup_af_unix( int *csock_fd, int *lsock_fd, int *ssock_fd,
                   int domain, int *type, void *address, int initial )
{
     static char buffer[ 80 ], path[ 1025 ];
     int already_listening = 0, exit_loop, len = 1025;
     int num, opt, ret, save_errno, sock_type;
     socklen_t size;
     struct sockaddr server;

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
     if ( domain < 1 || domain > MAX_DOMAINS )
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

     /* Find out what type of AF_UNIX socket to use. */

     if ( initial == 1 )
     {
          exit_loop = 0;
          do
          {
               printf( "\
What type of AF_UNIX socket would you like to use?\n\n" );
               printf( "1) Stream\n" );
               printf( "2) Datagram\n" );
               printf( "3) Sequential Packet\n\n" );
               errno = 0;
               ret = read_stdin( buffer, 80, ">> ", 1 );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong while reading your input.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
                    errno = 0;
                    return ( -1 );
               }
               ret = sscanf( buffer, "%d", &num );
               if ( ret != 1 )
               {
                    printf( "\n\
That is not a valid option.  Please try again.\n\n" );
               }
               else
               {
                    switch( num )
                    {
                          case 1: sock_type = SOCK_STREAM;
                                  exit_loop = 1;
                                  break;
                          case 2: sock_type = SOCK_DGRAM;
                                  exit_loop = 1;
                                  break;
                          case 3: sock_type = SOCK_SEQPACKET;
                                  exit_loop = 1;
                                  break;
                         default: printf( "\n\
That is not a valid option.  Please try again.\n\n" );
                                  break;
                    }    /* switch( num ) */

               }    /* if ( ret != 1 ) */

          }    while( exit_loop == 0 );

          *type = sock_type;
     }
     else  /* initial == 0 */
     {
          sock_type = *type;
     }

#ifdef DEBUG

     printf( "\nUsing socket type: %d ", sock_type );
     if ( sock_type == SOCK_STREAM )
     {
          printf( "(Stream)\n" );  /* Type 1 */
     }
     else if ( sock_type == SOCK_DGRAM )
     {
          printf( "(Datagram)\n" );  /* Type 2 */
     }
     else  /* sock_type == SOCK_SEQPACKET */
     {
          printf( "(Sequential Packet)\n" );  /* Type 5 */
     }

#endif

     size = sizeof( server );

     /* Set the current working directory. */

     errno = 0;
     ret = chdir( USE_DIR );
     if ( ret != 0 )
     {
          save_errno = errno;
          printf( "\n\
Failed to change the current working directory to \"%s/\"\n", USE_DIR );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
          errno = 0;
          return ( -1 );
     }

     /* Open the server's listening socket if it is currently closed. */

     if ( sock_type != SOCK_DGRAM )
     {
          if ( *lsock_fd == ( -1 ) )
          {
               errno = 0;
               ret = socket( AF_UNIX, sock_type, 0 );
               if ( ret < 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when opening the server's listening socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
                    errno = 0;
                    return ( -1 );
               }

               *lsock_fd = ret;

#ifdef DEBUG

               printf( "\
The server's listening socket has been opened.\n" );

#endif

          }
          else
          {
               already_listening = 1;
          }
     }
     else  /* sock_type == SOCK_DGRAM */
     {
          if ( *ssock_fd == ( -1 ) )
          {
               errno = 0;
               ret = socket( AF_UNIX, sock_type, 0 );
               if ( ret < 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when opening the server socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
                    errno = 0;
                    return ( -1 );
               }

               *ssock_fd = ret;

#ifdef DEBUG

               printf( "The server socket has been opened.\n" );

#endif

               /* Set the server socket to nonblocking mode. */

               errno = 0;
               ret = fcntl( *ssock_fd, F_SETFD, O_NONBLOCK );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the server socket to nonblocking mode.\
\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }

#ifdef DEBUG

                    printf( "\nShutting down sockets.\n" );

#else

                    printf( "\n" );

#endif

                    ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd,
                                            domain, *type );

#ifdef DEBUG

                    if ( ret == 0 )
                    {
                         printf( "\n" );
                    }

#endif

                    errno = 0;
                    return ( -1 );

               }    /* if ( ret != 0 ) */

#ifdef DEBUG

               printf( "\
The server socket has been set to nonblocking mode.\n" );

#endif

          }
          else
          {
               already_listening = 1;
          }

     }    /* if ( sock_type != SOCK_DGRAM ) */

     /* Set up the server's listening socket's address/name. */

     memset( &server, 0, size );  /* Clear the data space. */
     server.sa_family = AF_UNIX;
     strncpy( server.sa_data, SOCK_NAME, ( strlen( SOCK_NAME ) + 1 ) );

     if ( already_listening == 0 )
     {
          memset( address, 0, ADDR_SIZE );   /* Clear the data space. */
          memcpy( address, &server, size );  /* Make a copy.          */

#ifdef DEBUG

          printf( "\
Using: server.sa_family: %d (AF_UNIX), server.sa_data: \"%s\"\n",
                  server.sa_family, server.sa_data );

          if ( getcwd( path, len ) == NULL )
          {
               printf( "\n\
Failed to determine the current working directory.\n" );
          }
          else
          {
               printf( "Current working directory: \"%s/\"\n", path );
               printf( "Full path name for socket file: \"%s/%s\"\n",
                       path, SOCK_NAME );
          }

#endif

          /*

               Bind the server's listening socket to an address/name
               so it can listen, unless we're using datagram sockets
               in which case we need to bind the server socket to the
               socket file instead.

               bind(2) creates the socket file.

          */

          errno = 0;
          if ( sock_type != SOCK_DGRAM )
          {
               ret = bind( *lsock_fd, &server, size );
          }
          else
          {
               ret = bind( *ssock_fd, &server, size );
          }
          if ( ret != 0 )
          {
               save_errno = errno;
               if ( sock_type != SOCK_DGRAM )
               {
                    printf( "\n\
Something went wrong when binding the server's listening socket.\n" );
               }
               else
               {
                    printf( "\n\
Something went wrong when binding the server socket.\n" );
               }
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }

#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret != 0 ) */

#ifdef DEBUG

          if ( sock_type != SOCK_DGRAM )
          {
               printf( "The server's listening socket has been bound.\n" );
          }
          else
          {
               printf( "The server socket has been bound.\n" );
          }

#endif

          /*

               Tell the server's listening socket to start listening,
               unless we're using datagram sockets.  listen(2) doesn't
               use them.

          */

          if ( sock_type != SOCK_DGRAM )
          {
               errno = 0;
               ret = listen( *lsock_fd, LISTEN_BACKLOG );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when telling the server's listening socket to listen.\
\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }

#ifdef DEBUG

                    printf( "\nShutting down sockets.\n" );

#else

                    printf( "\n" );

#endif

                    ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd,
                                            domain, *type );

#ifdef DEBUG

                    if ( ret == 0 )
                    {
                         printf( "\n" );
                    }

#endif

                    errno = 0;
                    return ( -1 );

               }    /* if ( ret != 0 ) */

#ifdef DEBUG

               printf( "\
The server's listening socket is listening for new connections.\n" );

#endif

          }    /* if ( sock_type != SOCK_DGRAM ) */

     }    /* if ( already_listening == 0 ) */

     /* Open the client side if it is currently closed. */

     if ( *csock_fd == ( -1 ) )
     {
          /*

               I am well aware of the fact that I could use
               ( sock_type | O_NONBLOCK ) in the socket(2)
               function call which could save me a call to fcntl(2)
               afterwards, but I did it this way because I wanted
               to take things step by step.

          */

          errno = 0;
          ret = socket( AF_UNIX, sock_type, 0 );
          if ( ret < 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when opening the client socket.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }

#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret < 0 ) */

          *csock_fd = ret;

#ifdef DEBUG

          printf( "The client socket has been opened.\n" );

#endif

     }

     /* Set the client socket to nonblocking mode. */

     errno = 0;
     ret = fcntl( *csock_fd, F_SETFD, O_NONBLOCK );
     if ( ret != 0 )
     {
          save_errno = errno;
          printf( "\n\
Something went wrong when setting the client socket to nonblocking mode.\
\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }

#ifdef DEBUG

          printf( "\nShutting down sockets.\n" );

#else

          printf( "\n" );

#endif

          ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                  *type );

#ifdef DEBUG

          if ( ret == 0 )
          {
               printf( "\n" );
          }

#endif

          errno = 0;
          return ( -1 );

     }    /* if ( ret != 0 ) */

#ifdef DEBUG

     printf( "The client socket has been set to nonblocking mode.\n" );

#endif

     /*

          Connect the two sockets to each
          other, unless we are using datagrams.

     */

     if ( sock_type != SOCK_DGRAM )
     {

#ifdef DEBUG

          printf( "\
The client socket will now attempt to connect to the server.\n" );

#endif

          errno = 0;
          ret = connect( *csock_fd, &server, size );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong while trying to connect the two sockets.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }
#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret != 0 ) */

          /* Accept the connection, unless we are using datagrams. */

#ifdef DEBUG

          printf( "\
The server will now attempt to accept the connection.\n" );

#endif

          errno = 0;
          ret = accept( *lsock_fd, &server, &size );
          if ( ret < 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong while trying to accept the new connection.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }

#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret < 0 ) */

#ifdef DEBUG

          printf( "Connection accepted.\n" );

#endif

          *ssock_fd = ret;

          /* Set the new server socket to nonblocking mode. */

          errno = 0;
          ret = fcntl( *ssock_fd, F_SETFD, O_NONBLOCK );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when setting the server socket to nonblocking mode.\
\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }

#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret != 0 ) */

#ifdef DEBUG

          printf( "\
The server socket has been set to nonblocking mode.\n" );

#endif

          /* Set the socket option SO_KEEPALIVE. */

          /* Set the option on the server socket first. */

          size = sizeof( opt );
          opt = 1;
          errno = 0;
          ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_KEEPALIVE, &opt,
                            size );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when setting the SO_KEEPALIVE option for the server.\
\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }

#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret != 0 ) */

#ifdef DEBUG

          printf( "\
The SO_KEEPALIVE option has been set on the server socket.\n" );

#endif

          /* Set the client socket option SO_KEEPALIVE. */

          size = sizeof( opt );
          opt = 1;
          errno = 0;
          ret = setsockopt( *csock_fd, SOL_SOCKET, SO_KEEPALIVE, &opt,
                            size );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when setting the SO_KEEPALIVE option for the client.\
\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }

#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret != 0 ) */

#ifdef DEBUG

          printf( "\
The SO_KEEPALIVE option has been set on the client socket.\n" );

#endif

     }  /* if ( sock_type != SOCK_DGRAM ) */

#ifdef USE_DEFAULT_TARGET_AF_UNIX

     else  /* sock_type == SOCK_DGRAM */
     {
          /*

               Set the default target for the
               client socket with connect(2).

          */

          errno = 0;
          ret = connect( *csock_fd, &server, size );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong while trying to set\
 the default target for the client.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               printf( "\nShutting down sockets.\n" );

#else

               printf( "\n" );

#endif

               ret = shutdown_sockets( csock_fd, lsock_fd, ssock_fd, domain,
                                       *type );

#ifdef DEBUG

               if ( ret == 0 )
               {
                    printf( "\n" );
               }
#endif

               errno = 0;
               return ( -1 );

          }    /* if ( ret != 0 ) */

#ifdef DEBUG

          printf( "\
The default target for the client socket has\
 been set to the server socket.\n" );

#endif

     }    /* if ( sock_type != SOCK_DGRAM ) */

#endif  /* USE_DEFAULT_TARGET_AF_UNIX */

#ifdef DEBUG

     printf( "\nSetup complete.\n" );
     list_sockets( csock_fd, lsock_fd, ssock_fd );

#endif

     return 0;
}

#endif /* _SETUP_AF_UNIX_1P_C */

/* EOF setup_af_unix_1p.c */

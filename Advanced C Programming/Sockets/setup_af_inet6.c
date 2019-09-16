/*

     setup_af_inet6.c

     This function creates sockets in the AF_INET6 domain and connects
     them to each other.  If a socket file descriptor is not -1 then
     it will presume that the socket already exists and proceed to try
     to connect it.

     Written by Matthew Campbell.

*/

#ifndef _SETUP_AF_INET6_C
#define _SETUP_AF_INET6_C

#include "sockets.h"

int setup_af_inet6( int *csock_fd, int *lsock_fd, int *ssock_fd,
                    int domain, int *type, void *address, int initial )
{
     static char buffer[ 64 ];
     char ip_str[ 48 ];
     int already_listening, endian, exit_loop;
     int num, opt, ret, save_errno, setup_address, sock_type;
     pid_t pid;
     unsigned short int server_port;
     socklen_t size;
     static int use_client = ( -1 ), use_server = ( -1 );
     struct sockaddr_in6 server;

#if defined( SHOW_CONNECTIONS ) && defined( DEBUG )

     struct sockaddr_in6 client_addr, listen_addr, server_addr;

#endif

#ifdef DEBUG

     int count;

     char abox[ 16 ];

     union short_box
     {
          uint16_t num;
          char bytes[ 2 ];
     } sbox;

#endif

     /* Check our function parameters. */

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

     already_listening = 0;

     /* Determine whether this system uses big or little endianess. */

     endian = detect_endian();

     if ( initial == 1 )
     {
          memset( address, 0, ADDR_SIZE );  /* Clear the data space. */
     }
     else  /* Restore the saved information. */
     {
          memcpy( ( void * )( &server ), address, sizeof( server ) );

          /* Restore the numeric IPv6 address string. */

          errno = 0;
          if ( inet_ntop( AF_INET6, &( server.sin6_addr.s6_addr ), ip_str,
                          48 ) == NULL )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when calling inet_ntop(3).\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }
               printf( "\n" );
               errno = 0;  /* Don't show the same error twice. */
               return ( -1 );

          }    /* if ( inet_ntop( AF_INET6, &( server.sin6_addr.s6_addr ),
                                  ip_str, 48 ) == NULL ) */

          if ( endian == 1 )
          {
               server_port = ntohs( server.sin6_port );
          }
          else
          {
               server_port = server.sin6_port;
          }

          sock_type = *type;
     }

     /* Find out if we need a client, a server, or both. */

     if ( initial == 1 )
     {
          exit_loop = 0;
          do
          {
               printf( "How would you like to set this up?\n\n" );
               printf( "1) Run a client and a server on this device.\n" );
               printf( "\
2) Run a client on this device and a server on another device.\n" );
               printf( "\
3) Run a client on another device and a server on this device.\n\n" );
               errno = 0;
               ret = read_stdin( buffer, 64, ">> ", 1 );
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
                    errno = 0;  /* Don't show the same error twice. */
                    return ( -1 );
               }
               ret = sscanf( buffer, "%d", &num );
               if ( ret != 1 )
               {
                    printf( "\n\
That is not a valid input.  Please try again.\n\n" );
               }
               else
               {
                    switch( num )
                    {
                          case 1: use_client = 1;
                                  use_server = 1;
                                  exit_loop = 1;
                                  break;
                          case 2: use_client = 1;
                                  use_server = 0;
                                  exit_loop = 1;
                                  break;
                          case 3: use_client = 0;
                                  use_server = 1;
                                  exit_loop = 1;
                                  break;
                         default: printf( "\n\
That is not a valid option.  Please try again.\n\n" );
                                  break;
                    }    /* switch( num ) */

               }    /* if ( ret != 1 ) */

          }    while( exit_loop == 0 );

          /* Find out what type of AF_INET6 socket to use. */

          exit_loop = 0;
          do
          {
               printf( "\n\
What type of AF_INET6 socket would you like to use?\n\n" );
               printf( "1) Stream\n" );
               printf( "2) Datagram\n\n" );
               errno = 0;
               ret = read_stdin( buffer, 64, ">> ", 1 );
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
                    errno = 0;  /* Don't show the same error twice. */
                    return ( -1 );
               }
               ret = sscanf( buffer, "%d", &num );
               if ( ret != 1 )
               {
                    printf( "\n\
That is not valid input.  Please try again.\n" );
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
                         default: printf( "\n\
That is not a valid option.  Please try again.\n" );
                                  break;
                    }    /* switch( num ) */

               }    /* if ( ret != 1 ) */

          }    while( exit_loop == 0 );

          *type = sock_type;
          setup_address = 1;

          if ( sock_type == SOCK_DGRAM )
          {
               if ( use_server == 0 )
               {

#ifndef USE_DEFAULT_TARGET_AF_INET6

                    setup_address = 0;

#endif

               }    /* if ( use_server == 0 ) */

          }    /* if ( sock_type == SOCK_DGRAM ) */

          if ( setup_address == 1 )
          {
               /* Find out what address to use. */

               exit_loop = 0;
               do
               {
                    if ( use_server == 1 )
                    {
                         printf( "\n\
What numeric IPv6 address would you like the server to use?\n\
Please use hexadecimal words and colons.  Do not specify a port number.\n\
\n" );
                    }
                    else
                    {
                         printf( "\n\
What numeric IPv6 address would you like the client to connect to?\n\
Please use hexadecimal words and colons.  Do not specify a port number.\n\
\n" );
                    }
                    errno = 0;
                    ret = read_stdin( buffer, 64, ">> ", 1 );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while reading your input.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                         errno = 0;  /* Don't show the same error twice. */
                         return ( -1 );
                    }

                    /* Clear the data space. */

                    memset( &server, 0, sizeof( server ) );
                    server.sin6_family = AF_INET6;

                    /* Convert the address string to a network address. */

                    errno = 0;
                    ret = inet_pton( AF_INET6, buffer,
                                     &( server.sin6_addr.s6_addr ) );
                    if ( ret != 1 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when calling inet_pton(3).\n" );
                         if ( ret == 0 )
                         {
                              printf( "\
The information you have provided is not a valid\n\
network address for a numeric IPv6 address.\n" );
                         }
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "Please try again.\n" );
                    }
                    else
                    {
                         exit_loop = 1;

                    }    /* if ( ret != 1 ) */

               }    while( exit_loop == 0 );

               /*

                    Make a copy of the numeric IPv6 address string
                    after the library functions have processed it first.

               */

               errno = 0;
               if ( inet_ntop( AF_INET6, &( server.sin6_addr.s6_addr ),
                               ip_str, 48 ) == NULL )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when calling inet_ntop(3).\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
                    errno = 0;  /* Don't show the same error twice. */
                    return ( -1 );

               }    /* if ( inet_ntop( AF_INET6,
                                       &( server.sin6_addr.s6_addr ),
                                       ip_str, 48 ) == NULL ) */

               /* Find out what port number to use. */

               exit_loop = 0;
               do
               {
                    if ( use_server == 1 )
                    {
                         printf( "\n\
What numeric port number would you like the server to use?\n\
Please specify a number that is greater than 1024 and less than 65536.\
\n\n" );
                    }
                    else
                    {
                         printf( "\n\
What numeric port number would you like the client to connect to?\n\
Please specify a number that is greater than 1024 and less than 65536.\
\n\n" );
                    }
                    errno = 0;
                    ret = read_stdin( buffer, 64, ">> ", 1 );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while reading your input.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                         errno = 0;  /* Don't show the same error twice. */
                         return ( -1 );
                    }
                    ret = sscanf( buffer, "%d", &num );
                    if ( ret != 1 )
                    {
                         printf( "\n\
That is not valid input.  Please try again.\n" );
                    }
                    else
                    {
                         if ( num < 1025 || num > 65535 )
                         {
                              printf( "\n\
That is not an acceptable port number.  Please try again.\n" );
                         }
                         else
                         {
                              exit_loop = 1;
                              server_port = ( uint16_t )num;
                              if ( endian == 1 )
                              {
                                   /*

                                        Convert from little endian
                                        to network endian.

                                   */

                                   server.sin6_port = htons( server_port );
                              }
                              else
                              {
                                   server.sin6_port = server_port;
                              }

                         }    /* if ( num < 1025 || num > 65535 ) */

                    }    /* if ( ret != 1 ) */

               }    while( exit_loop == 0 );

               /* Save the server's address information. */

               memcpy( address, ( void * )( &server ), sizeof( server ) );

          }    /* if ( setup_address == 1 ) */

     }    /* if ( initial == 1 ) */

#if defined( DEBUG ) && defined( SHOW_CONNECTIONS )

     if ( sock_type != SOCK_DGRAM )
     {
          memcpy( ( void * )( &listen_addr ), ( void * )( &server ),
                  sizeof( server ) );
     }
     else if ( setup_address == 1 )
     {
          memcpy( ( void * )( &server_addr ), ( void * )( &server ),
                  sizeof( server ) );
     }

#endif

#ifdef DEBUG

     if ( sock_type != SOCK_DGRAM )
     {
          if ( use_server == 1 )
          {
               printf( "\nThe server will be using [%s]:%u.\n\n", ip_str,
                       server_port );
          }
          else
          {
               printf( "\nThe client will be connecting to [%s]:%u.\n\n",
                       ip_str, server_port );
          }

          printf( "Using server address: " );

          memcpy( ( void * )( &abox ),
                  ( void * )( &server.sin6_addr.s6_addr ),
                  sizeof( server.sin6_addr.s6_addr ) );

          for( count = 0; count < 14; count += 2 )
          {
               printf( "%02x%02x:", ( unsigned char )abox[ count ],
                                    ( unsigned char )abox[ ( count + 1 ) ] );
          }
          printf( "%02x%02x\n", ( unsigned char )abox[ count ],
                                ( unsigned char )abox[ ( count + 1 ) ] );

          printf( "and server port: " );
          sbox.num = server.sin6_port;
          for( count = 0; count < 2; count++ )
          {
               printf( "%02X ", ( unsigned char )sbox.bytes[ count ] );
          }
          if ( use_server == 1 )
          {
               printf( "for the server's listening socket.\n\n" );
          }
          else
          {
               printf( "for the client socket's target address.\n\n" );
          }
     }
     else  /* sock_type == SOCK_DGRAM */
     {
          if ( use_server == 1 )
          {
               printf( "\nThe server will be using [%s]:%u.\n\n", ip_str,
                       server_port );

               printf( "Using server address: " );

               memcpy( ( void * )( &abox ),
                       ( void * )( &server.sin6_addr.s6_addr ),
                       sizeof( server.sin6_addr.s6_addr ) );

               for( count = 0; count < 14; count += 2 )
               {
                    printf( "%02x%02x:", ( unsigned char )abox[ count ],
                            ( unsigned char )abox[ ( count + 1 ) ] );
               }
               printf( "%02x%02x\n", ( unsigned char )abox[ count ],
                       ( unsigned char )abox[ ( count + 1 ) ] );

               printf( "and server port: " );
               sbox.num = server.sin6_port;
               for( count = 0; count < 2; count++ )
               {
                    printf( "%02X ", ( unsigned char )sbox.bytes[ count ] );
               }
               printf( "for the server socket.\n\n" );
          }
          else if ( setup_address == 1 )
          {
               printf( "\nThe client socket will be seeking [%s]:%u.\n\n",
                       ip_str, server_port );

               printf( "Using server address: " );

               memcpy( ( void * )( &abox ),
                       ( void * )( &server.sin6_addr.s6_addr ),
                       sizeof( server.sin6_addr.s6_addr ) );

               for( count = 0; count < 14; count += 2 )
               {
                    printf( "%02x%02x:", ( unsigned char )abox[ count ],
                            ( unsigned char )abox[ ( count + 1 ) ] );
               }
               printf( "%02x%02x\n", ( unsigned char )abox[ count ],
                       ( unsigned char )abox[ ( count + 1 ) ] );

               printf( "and server port: " );
               sbox.num = server.sin6_port;
               for( count = 0; count < 2; count++ )
               {
                    printf( "%02X ", ( unsigned char )sbox.bytes[ count ] );
               }
               printf( "for the client socket's target address.\n\n" );

          }    /* if ( use_server == 1 ) else if ( setup_address == 1 ) */

     }    /* if ( sock_type != SOCK_DGRAM ) */

#ifndef USE_DEFAULT_TARGET_AF_INET6

     if ( sock_type == SOCK_DGRAM && use_server == 0 )
     {
          printf( "\n" );
     }

#else

     if ( sock_type == SOCK_DGRAM && initial == 0 && use_server == 0 )
     {
          printf( "\n" );
     }

#endif

     printf( "Using socket type: %d ", sock_type );
     if ( sock_type == SOCK_STREAM )
     {
          printf( "(Stream)\n" );  /* Type 1 */
     }
     else  /* sock_type == SOCK_DGRAM */
     {
          printf( "(Datagram)\n" );  /* Type 2 */
     }

     if ( sock_type == SOCK_STREAM )
     {
          if ( use_server == 0 )
          {
               printf( "\n" );
          }
          else if ( initial == 0 )
          {
               printf( "\n" );
          }
     }
     else  /* sock_type == SOCK_DGRAM */
     {
          if ( use_server == 0 )
          {
               printf( "\n" );
          }
          else if ( initial == 0 && use_client == 1 )
          {
               printf( "\n" );
          }

     }    /* if ( sock_type == SOCK_STREAM ) */

#endif  /* DEBUG */

     /* Open the server's listening socket if it is currently closed. */

     if ( use_server == 1 )
     {
          if ( sock_type != SOCK_DGRAM )
          {
               if ( *lsock_fd == ( -1 ) )
               {
                    errno = 0;
                    ret = socket( AF_INET6, sock_type, 0 );
                    if ( ret < 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when opening the server's listening socket.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                         return ( -1 );
                    }

                    *lsock_fd = ret;

#ifdef DEBUG

                    printf( "\n\
The server's listening socket has been opened.\n" );

#endif

#ifdef USE_DONTROUTE_AF_INET6

                    /*

                         Set the SO_DONTROUTE option on
                         the server's listening socket.

                    */

                    opt = 1;
                    errno = 0;
                    ret = setsockopt( *lsock_fd, SOL_SOCKET, SO_DONTROUTE,
                                      &opt, sizeof( opt ) );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when setting the SO_DONTROUTE\n\
option on the server's listening socket.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain, *type );

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
The SO_DONTROUTE option has been set on the server's listening socket.\n" );

#endif

#endif  /* USE_DONTROUTE_AF_INET6 */

               }
               else  /* *lsock != ( -1 ) */
               {
                    already_listening = 1;
               }
          }
          else  /* sock_type == SOCK_DGRAM */
          {
               if ( *ssock_fd == ( -1 ) )
               {
                    errno = 0;
                    ret = socket( AF_INET6, sock_type, 0 );
                    if ( ret < 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when opening the server socket.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                         return ( -1 );
                    }

                    *ssock_fd = ret;

#ifdef DEBUG

                    printf( "\nThe server socket has been opened.\n" );

#endif

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
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain,
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

#ifdef USE_BROADCAST_AF_INET6

                    /* Set the server socket option SO_BROADCAST. */

                    opt = 1;
                    errno = 0;
                    ret = setsockopt( *ssock_fd, SOL_SOCKET,
                                      SO_BROADCAST, &opt, sizeof( opt ) );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when setting the\n\
SO_BROADCAST option on the server socket.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain,
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
The SO_BROADCAST option has been set on the server socket.\n" );

#endif

#endif  /* USE_BROADCAST_AF_INET6 */

#ifdef USE_DONTROUTE_AF_INET6

                    /* Set the SO_DONTROUTE option on the server socket. */

                    opt = 1;
                    errno = 0;
                    ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_DONTROUTE,
                                      &opt, sizeof( opt ) );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when setting the\n\
SO_DONTROUTE option on the server socket.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain, *type );

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
The SO_DONTROUTE option has been set on the server socket.\n" );

#endif

#endif  /* USE_DONTROUTE_AF_INET6 */

               }
               else  /* *ssock != ( -1 ) */
               {
                    already_listening = 1;
               }

          }    /* if ( sock_type != SOCK_DGRAM ) */

          if ( already_listening == 0 )
          {
               /*

                    Bind the server's listening socket to an address so
                    it can listen, unless we're using datagram sockets
                    in which case we need to bind the server socket to
                    the network port instead.

               */

               errno = 0;
               if ( sock_type != SOCK_DGRAM )
               {
                    ret = bind( *lsock_fd,
                                ( struct sockaddr * )( &server ),
                                sizeof( server ) );
               }
               else
               {
                    ret = bind( *ssock_fd,
                                ( struct sockaddr * )( &server ),
                                sizeof( server ) );
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
                         printf( "Error: %s.\n",
                                 strerror( save_errno ) );
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

               if ( sock_type != SOCK_DGRAM )
               {
                    printf( "\
The server's listening socket has been bound.\n" );
               }
               else
               {
                    printf( "The server socket has been bound.\n" );
               }

#endif

               /*

                    Tell the server's listening socket to start
                    listening, unless we're using datagram sockets.
                    listen(2) doesn't use them.

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
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain,
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
The server's listening socket is listening for new connections.\n" );

#endif

               }    /* if ( sock_type != SOCK_DGRAM ) */

          }    /* if ( already_listening == 0 ) */

     }    /* if ( use_server == 1 ) */

     if ( use_client == 1 )
     {
          /* Open the client socket if it is currently closed. */

          if ( *csock_fd == ( -1 ) )
          {
               errno = 0;
               ret = socket( AF_INET6, sock_type, 0 );
               if ( ret < 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when opening the client socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n",
                                 strerror( save_errno ) );
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

               }    /* if ( ret < 0 ) */

               *csock_fd = ret;

#ifdef DEBUG

               printf( "The client socket has been opened.\n" );

#endif

          }    /* if ( *csock_fd == ( -1 ) ) */

          /*

               Connect the client socket to the
               server unless we are using datagrams.

          */

          if ( sock_type != SOCK_DGRAM )
          {

#ifdef USE_DONTROUTE_AF_INET6

               /* Set the SO_DONTROUTE option on the client socket. */

               opt = 1;
               errno = 0;
               ret = setsockopt( *csock_fd, SOL_SOCKET, SO_DONTROUTE, &opt,
                                 sizeof( opt ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the\n\
SO_DONTROUTE option on the client socket.\n" );
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
The SO_DONTROUTE option has been set on the client socket.\n" );

#endif

#endif  /* USE_DONTROUTE_AF_INET6 */

               if ( use_server == 1 )
               {

                    /* Create a child process to call connect(2). */

                    errno = 0;
                    pid = fork();
                    save_errno = errno;
                    if ( pid == ( -1 ) )
                    {
                         printf( "\n\
Something went wrong when trying to create the child process.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain,
                                                 *type );

#ifdef DEBUG

                         if ( ret == 0 )
                         {
                              printf( "\n" );
                         }

#endif

                         errno = 0;
                         return ( -1 );

                    }
                    else if ( pid == 0 )  /* Child process */
                    {

                         /* Give the server a chance to call accept(2). */

                         sleep( 1 );

#ifdef DEBUG

                         if ( initial == 1 )
                         {
                              printf( "Trying to connect to %s...\n",
                                      ip_str );
                         }
                         else
                         {
                              printf( "Trying to reconnect to %s...\n",
                                      ip_str );
                         }

#endif

                         errno = 0;
                         ret = connect( *csock_fd,
                                        ( struct sockaddr * )( &server ),
                                        sizeof( server ) );
                         if ( ret != 0 )
                         {
                              save_errno = errno;
                              printf( "\n\
Something went wrong while trying to connect to the server.\n" );
                              if ( save_errno != 0 )
                              {
                                   printf( "Error: %s.\n",
                                           strerror( save_errno ) );
                              }

                              /* Wake up the parent process. */

                              errno = 0;
                              ret = kill( getppid(), SIGALRM );
                              save_errno = errno;

                              if ( ret != 0 )
                              {
                                   printf( "\n\
Something went wrong when trying to wake up the parent process.\n" );
                                   if ( save_errno != 0 )
                                   {
                                        printf( "Error: %s.\n",
                                                strerror( save_errno ) );
                                   }

#ifdef DEBUG

                                   printf( "\nShutting down sockets.\n" );

#else

                                   printf( "\n" );

#endif

                                   ret = shutdown_sockets( csock_fd,
                                                           lsock_fd,
                                                           ssock_fd, domain,
                                                           *type );

#ifdef DEBUG

                                   if ( ret == 0 )
                                   {
                                        printf( "\n" );
                                   }
#endif

                                   errno = 0;
                                   return ( -1 );

                              }  /* if ( ret != 0 ) */

                              /* Stop the child process. */

                              _exit( EXIT_FAILURE );

                         }  /* if ( ret != 0 ) */

                         /* Stop the child process. */

                         _exit( EXIT_SUCCESS );

                    }
                    else  /* Parent process, pid > 0. */
                    {

#ifdef DEBUG

                         printf( "\
The server is ready and waiting to accept a new connection.\n" );

#endif

                         size = sizeof( server );
                         errno = 0;
                         ret = accept( *lsock_fd,
                                       ( struct sockaddr * )( &server ),
                                       &size );
                         if ( ret < 0 )
                         {
                              save_errno = errno;
                              printf( "\n\
Something went wrong when calling accept(2).\n" );
                              if ( save_errno != 0 )
                              {
                                   printf( "Error: %s.\n",
                                           strerror( save_errno ) );
                              }

                              /* Shut down the child process. */

                              errno = 0;
                              ret = kill( pid, SIGTERM );
                              save_errno = errno;

                              if ( ret != 0 )
                              {
                                   printf( "\n\
Something went wrong while trying to shut down the child process.\n" );
                                   if ( save_errno != 0 )
                                   {
                                        printf( "Error: %s.\n",
                                                strerror( save_errno ) );
                                   }
                              }
                              else  /* Wait for the child process. */
                              {

#ifdef WAIT_FOR_CHILD

#ifdef DEBUG

                                   printf( "\n\
Waiting for the child process...\n" );

#endif

                                   errno = 0;
                                   ret = waitpid( ( -1 ), NULL, 0 );
                                   save_errno = errno;

#ifdef SHOW_WAIT_ERRORS

                                   if ( ret == ( -1 ) )
                                   {
                                        printf( "\n\
Something went wrong while waiting for the child process.\n" );
                                        if ( save_errno != 0 )
                                        {
                                             printf( "Error: %s.\n",
                                                     strerror( save_errno )
                                                   );
                                        }
                                   }

#endif

#ifdef DEBUG

                                   else if ( ret == pid )
                                   {
                                        printf( "\
The child process has exited.\n" );
                                   }

#endif

#endif  /* WAIT_FOR_CHILD */

#ifdef DEBUG

                                   printf( "\nShutting down sockets.\n" );

#else

                                   printf( "\n" );

#endif

                                   ret = shutdown_sockets( csock_fd,
                                                           lsock_fd,
                                                           ssock_fd, domain,
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

                         }    /* if ( ret < 0 ) */

                         *ssock_fd = ret;

#ifdef DEBUG

                         if ( initial == 1 )
                         {
                              printf( "Connection to %s accepted.\n",
                                      ip_str );
                         }
                         else
                         {
                              printf( "Reconnected to %s.\n", ip_str );
                         }

#endif

                         /* Wait for the child process. */

#ifdef WAIT_FOR_CHILD

#ifdef DEBUG

                         printf( "Waiting for the child process...\n" );

#endif

                         errno = 0;
                         ret = waitpid( ( -1 ), NULL, 0 );
                         save_errno = errno;

#ifdef SHOW_WAIT_ERRORS

                         if ( ret == ( -1 ) )
                         {
                              printf( "\n\
Something went wrong while waiting for the child process.\n" );
                              if ( save_errno != 0 )
                              {
                                   printf( "Error: %s.\n",
                                           strerror( save_errno ) );
                              }
                         }

#endif

#ifdef DEBUG

                         else if ( ret == pid )
                         {
                              printf( "The child process has exited.\n" );
                         }

#endif

#endif  /* WAIT_FOR_CHILD */

#if defined( DEBUG ) && defined( SHOW_CONNECTIONS )

                         /* Save the server's address information. */

                         memcpy( ( void * )( &server_addr ),
                                 ( void * )( &server ),
                                 sizeof( server ) );

#endif

                    }    /* if ( pid == ( -1 ) ) */

               }
               else  /* use_server == 0 */
               {
                    /* Try to connect to the remote server. */

#ifdef DEBUG

                    if ( initial == 1 )
                    {
                         printf( "Trying to connect to %s...\n", ip_str );
                    }
                    else
                    {
                         printf( "Trying to reconnect to %s...\n", ip_str );
                    }

#endif

                    errno = 0;
                    ret = connect( *csock_fd,
                                   ( struct sockaddr * )( &server ),
                                   sizeof( server ) );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while trying to connect to the server.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain, *type );

#ifdef DEBUG

                         if ( ret == 0 )
                         {
                              printf( "\n" );
                         }
#endif

                         errno = 0;
                         return ( -1 );

                    }
                    else  /* ret == 0 */
                    {

#ifdef DEBUG

                         if ( initial == 1 )
                         {
                              printf( "Connection to %s accepted.\n",
                                      ip_str );
                         }
                         else
                         {
                              printf( "Reconnected to %s.\n", ip_str );
                         }

#endif

                    }    /* if ( ret != 0 ) */

               }    /* if ( use_server == 1 ) */

          }    /* if ( sock_type != SOCK_DGRAM ) */

     }    /* if ( use_client == 1 ) */

     /* Accept the connection if we're not using datagrams. */

     if ( use_server == 1 )
     {
          if ( sock_type != SOCK_DGRAM )
          {
               if ( use_client == 0 )
               {

#ifdef DEBUG

               printf( "\
The server is ready and waiting to accept a new connection.\n" );

#endif

                    size = sizeof( server );
                    errno = 0;
                    ret = accept( *lsock_fd,
                                  ( struct sockaddr * )( &server ),
                                  &size );
                    if ( ret < 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while trying to accept the new connection.\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }

#ifdef DEBUG

                         printf( "\nShutting down sockets.\n" );

#else

                         printf( "\n" );

#endif

                         ret = shutdown_sockets( csock_fd, lsock_fd,
                                                 ssock_fd, domain,
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

                    *ssock_fd = ret;

#if defined( DEBUG ) && defined( SHOW_CONNECTIONS )

                    /* Save the server's address information. */

                    memcpy( ( void * )( &server_addr ),
                            ( void * )( &server ),
                            sizeof( server ) );

#endif

               }  /* if ( use_client == 0 ) */

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

               /* Set the server socket option SO_KEEPALIVE. */

               opt = 1;
               errno = 0;
               ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_KEEPALIVE,
                                 &opt, sizeof( opt ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the\n\
SO_KEEPALIVE option on the server socket.\n" );
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
The SO_KEEPALIVE option has been set on the server socket.\n" );

#endif

#ifdef USE_DONTROUTE_AF_INET6

          /* Set the SO_DONTROUTE option on the server socket. */

          opt = 1;
          errno = 0;
          ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_DONTROUTE, &opt,
                            sizeof( opt ) );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when setting the\n\
SO_DONTROUTE option on the server socket.\n" );
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
The SO_DONTROUTE option has been set on the server socket.\n" );

#endif

#endif  /* USE_DONTROUTE_AF_INET6 */

          }    /* if ( sock_type != SOCK_DGRAM ) */

     }    /* if ( use_server == 1 ) */

     if ( use_client == 1 )
     {
          if ( sock_type != SOCK_DGRAM )
          {
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
The client socket has been set to nonblocking mode.\n" );

#endif

               /* Set the client socket option SO_KEEPALIVE. */

               opt = 1;
               errno = 0;
               ret = setsockopt( *csock_fd, SOL_SOCKET, SO_KEEPALIVE,
                                 &opt, sizeof( opt ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the\n\
SO_KEEPALIVE option on the client socket.\n" );
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
The SO_KEEPALIVE option has been set on the client socket.\n" );

#endif

          }
          else  /* sock_type == SOCK_DGRAM */
          {
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
The client socket has been set to nonblocking mode.\n" );

#endif

#ifdef USE_BROADCAST_AF_INET6

               /* Set the SO_BROADCAST option on the client socket. */

               opt = 1;
               errno = 0;
               ret = setsockopt( *csock_fd, SOL_SOCKET, SO_BROADCAST, &opt,
                                 sizeof( opt ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the\n\
SO_BROADCAST option on the client socket.\n" );
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
The SO_BROADCAST option has been set on the client socket.\n" );

#endif

#endif  /* USE_BROADCAST_AF_INET6 */

#ifdef USE_DONTROUTE_AF_INET6

               /* Set the SO_DONTROUTE option on the client socket. */

               opt = 1;
               errno = 0;
               ret = setsockopt( *csock_fd, SOL_SOCKET, SO_DONTROUTE, &opt,
                                 sizeof( opt ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the\n\
SO_DONTROUTE option on the client socket.\n" );
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
The SO_DONTROUTE option has been set on the client socket.\n" );

#endif

#endif  /* USE_DONTROUTE_AF_INET6 */

#ifdef USE_DEFAULT_TARGET_AF_INET6

               /*

                    Set the default target for the
                    client socket with connect(2).

               */

               errno = 0;
               ret = connect( *csock_fd, ( struct sockaddr * )( &server ),
                              sizeof( server ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong while trying to set \
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
The default target for the client socket has\
 been set to the server socket.\n" );

#endif

#endif  /*  USE_DEFAULT_TARGET_AF_INET6 */

          }    /* if ( sock_type != SOCK_DGRAM ) */

     }    /* if ( use_client == 1 ) */

#if defined( DEBUG ) && defined( SHOW_CONNECTIONS )

     /* Datagram sockets aren't connected. */

     if ( sock_type != SOCK_DGRAM )
     {
          size = sizeof( server );

          printf( "\n" );

          if ( use_server == 1 )
          {
               /* Show the server's listening socket. */

               errno = 0;
               if ( inet_ntop( AF_INET6,
                               &( listen_addr.sin6_addr.s6_addr ), buffer,
                               64 ) == NULL )
               {
                    save_errno = errno;
                    printf( "\
Something went wrong while using inet_ntop(3).\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
               }
               else
               {
                    printf( "\
The server's listening socket address is listed as:\n[%s]:", buffer );
                    if ( endian == 1 )
                    {
                         printf( "\
%u.\n\n", ( uint16_t )ntohs( listen_addr.sin6_port ) );
                    }
                    else
                    {
                         printf( "\
%u.\n\n", ( uint16_t )listen_addr.sin6_port );
                    }
               }    /* if ( inet_ntop( AF_INET6,
                                       &( listen_addr.sin6_addr.s6_addr ),
                                       buffer, 64 ) == NULL ) */

               /* Show the server socket. */

               errno = 0;
               if ( inet_ntop( AF_INET6,
                               &( server_addr.sin6_addr.s6_addr ), buffer,
                               64 ) == NULL )
               {
                    save_errno = errno;
                    printf( "\
Something went wrong while using inet_ntop(3).\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
               }
               else
               {
                    printf( "\
The server socket's address is listed as:\n[%s]:", buffer );
                    if ( endian == 1 )
                    {
                         printf( "\
%u.\n\n", ( uint16_t )ntohs( server_addr.sin6_port ) );
                    }
                    else
                    {
                         printf( "\
%u.\n\n", ( uint16_t )server_addr.sin6_port );
                    }
               }    /* if ( inet_ntop( AF_INET6,
                                       &( server_addr.sin6_addr.s6_addr ),
                                       buffer, 64 ) == NULL ) */

          }    /* if ( use_server == 1 ) */

          if ( use_client == 1 )
          {
               errno = 0;
               ret = getsockname( *csock_fd,
                                  ( struct sockaddr * )( &client_addr ),
                                  &size );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\
Something went wrong when calling getsockname(3) for the client socket.\
\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
               }
               else
               {
                    errno = 0;
                    if ( inet_ntop( AF_INET6,
                                    &( client_addr.sin6_addr.s6_addr ),
                                    buffer, 64 ) == NULL )
                    {
                         save_errno = errno;
                         printf( "\
Something went wrong while using inet_ntop(3).\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                    }
                    else
                    {
                         printf( "\
The client socket's address is listed as\n[%s]:", buffer );
                         if ( endian == 1 )
                         {
                              printf( "\
%u.\n", ( uint16_t )ntohs( client_addr.sin6_port ) );
                         }
                         else
                         {
                              printf( "\
%u.\n", ( uint16_t )client_addr.sin6_port );
                         }

                    }    /* if ( inet_ntop( AF_INET6,
                                    &( client_addr.sin6_addr.s6_addr ),
                                    buffer, 64 ) == NULL ) */

               }    /* if ( ret != 0 ) */

          }    /* if ( use_client == 1 ) */

     }    /* if ( sock_type != SOCK_DGRAM ) */

#endif  /* defined( DEBUG ) && defined( SHOW_CONNECTIONS ) */

#ifdef DEBUG

     printf( "\nSetup complete.\n" );
     list_sockets( csock_fd, lsock_fd, ssock_fd );

#endif

     errno = 0;
     return 0;
}

#endif  /* _SETUP_AF_INET6_C */

/* EOF setup_af_inet6.c */

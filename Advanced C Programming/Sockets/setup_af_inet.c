/*

     setup_af_inet.c

     This function creates sockets in the AF_INET domain and connects
     them to each other.  If a socket file descriptor is not -1 then
     it will presume that the socket already exists and proceed to try
     to connect it.

     Written by Matthew Campbell.

*/

#ifndef _SETUP_AF_INET_C
#define _SETUP_AF_INET_C

#include "sockets.h"

/* This function sure has a lot of local variables. */

int setup_af_inet( int *csock_fd, int *lsock_fd, int *ssock_fd,
                   int domain, int *type, void *address, int initial )
{
     static char buffer[ 80 ], server_name[ 128 ], service_name[ 16 ];
     char ip_str[ 16 ], *str_ptr;
     int already_listening = 0, exit_loop, len;
     int num, opt, pos, ret, save_errno, sock_type;
     pid_t pid;
     unsigned short int server_port;
     socklen_t size;
     static int use_client = ( -1 ), use_server = ( -1 );
     static int use_localhost = ( -1 );
     struct addrinfo hints, *list_ptr, *res;
     struct sockaddr_in *addr1, *addr2, server, *server_ptr;

#if defined( SHOW_CONNECTIONS ) && defined( DEBUG )

     struct sockaddr_in client_addr, listen_addr, server_addr;

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

     if ( initial == 1 )
     {
          memset( address, 0, ADDR_SIZE );  /* Clear the data space. */
     }
     else  /* Restore the saved information. */
     {
          memcpy( ( void * )( &server ), address,
                  sizeof( struct sockaddr_in ) );
     }

     server_ptr = ( struct sockaddr_in * )address;
     size = sizeof( struct sockaddr_in );

     /* Find out if we need a client, a server, or both. */

     if ( initial == 1 || use_client == ( -1 ) || use_server == ( -1 ) )
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
                    errno = 0;  /* Don't show the same error twice. */
                    return ( -1 );
               }
               ret = sscanf( buffer, "%d", &num );
               if ( ret != 1 )
               {
                    printf( "\
That is not a valid option.  Please try again.\n\n" );
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
                         default: printf( "\
That is not a valid option.  Please try again.\n\n" );
                                  break;
                    }    /* switch( num ) */

               }    /* if ( ret != 1 ) */

          }    while( exit_loop == 0 );

     }    /* if ( initial == 1 || use_client == ( -1 ) ||
                  use_server == ( -1 ) ) */

     /*

          If both the client and the server will be running on this
          device, find out if the user wants to use the default
          IPv4 address for localhost of 127.0.0.1.

     */

     if ( use_client == 1 && use_server == 1 && use_localhost == ( -1 ) )
     {
          exit_loop = 0;
          do
          {
               printf( "\n\
Would you like to use the default localhost address?\n\n" );
               printf( "\
1) Use the default localhost address of 127.0.0.1.\n" );
               printf( "\
2) Try to use another IPv4 address used by this device.\n\n" );
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
                    errno = 0;  /* Don't show the same error twice. */
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
                          case 1: use_localhost = 1;
                                  exit_loop = 1;
                                  break;
                          case 2: use_localhost = 0;
                                  exit_loop = 1;
                                  break;
                         default: printf( "\
That is not a valid option.  Please try again.\n\n" );
                                  break;
                    }    /* switch( num ) */

               }    /* if ( ret != 1 ) */

          }    while( exit_loop == 0 );

     }    /* if ( initial == 1 || ( use_client == 1 && use_server == 1 &&
                                    use_localhost == ( -1 ) ) ) */

     /* Find out what type of AF_INET socket to use. */

     if ( initial == 1 )
     {
          exit_loop = 0;
          do
          {
               printf( "\n\
What type of AF_INET socket would you like to use?\n\n" );
               printf( "1) Stream\n" );
               printf( "2) Datagram\n\n" );
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
                    errno = 0;  /* Don't show the same error twice. */
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
                         default: printf( "\
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
     else
     {
          printf( "(Unknown)\n" );
     }

#endif

     /* Find out what port the server socket should use. */

     if ( use_server == 1 )
     {
          if ( initial == 1 )
          {
               exit_loop = 0;
               do
               {
                    if ( sock_type != SOCK_DGRAM )
                    {
                         printf( "\n\
What port would you like the server to listen on?\n\n" );
                    }
                    else
                    {
                         printf( "\n\
What port would you like the server to use?\n\n" );
                    }
                    errno = 0;
                    ret = read_stdin( buffer, 80, ">> ", 1 );
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
                         if ( num < 1025 || num > 65535 )
                         {
                              printf( "\n\
That is not an acceptable port number.\n\
Please choose a port greater than 1024 and less than 65536.\n\n" );
                         }
                         else
                         {
                              server_port = ( unsigned short int )num;
                              exit_loop = 1;
                         }

                    }    /* if ( ret != 1 ) */

               }    while( exit_loop == 0 );
          }
          else  /* initial == 0 */
          {
               server_port = server_ptr->sin_port;
          }

#ifdef DEBUG

          if ( sock_type != SOCK_DGRAM )
          {
               printf( "\nUsing server listening port: %u.\n",
                       ( unsigned short int )server_port );
          }
          else
          {
               printf( "\nUsing server port: %u.\n",
                       ( unsigned short int )server_port );
          }

#endif

     }    /* if ( use_server == 1 ) */

     /* Find out what address the user wants to connect to. */

     if ( initial == 1 && use_client == 1 )
     {
          if ( use_localhost != 1 )
          {
               exit_loop = 0;
               do
               {
                    printf( "\n\
What address would you like to connect to?\n\n" );
                    errno = 0;
                    ret = read_stdin( server_name, 128, ">> ", 1 );
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
                         errno = 0;  /* Don't show the error twice. */
                         return ( -1 );
                    }

                    /* See if the user specified a port number. */

                    len = strlen( server_name );
                    for( pos = ( len - 1 ); pos >= 0 &&
                         server_name[ pos ] != ':'; pos-- );
                    if ( pos >= 0 )
                    {
                         str_ptr = &server_name[ ( pos + 1 ) ];
                         if ( sscanf( str_ptr, "%d", &num ) == 1 )
                         {
                              if ( use_server == 1 && num != server_port )
                              {
                                   printf( "\n\
The port number you have specified with the address of the server does\n\
not match the port number that you used earlier when asked which port\n\
number the server should " );
                                   if ( sock_type != SOCK_DGRAM )
                                   {
                                        printf( "listen on.\n\n" );
                                   }
                                   else
                                   {
                                        printf( "use.\n\n" );
                                   }
                              }
                              else
                              {
                                   if ( num < 1025 || num > 65535 )
                                   {
                                        printf( "\n\
That is not an acceptable port number.\n\
Please choose a port greater than 1024 and less than 65536.\n\n" );
                                   }
                                   else
                                   {
                                        strncpy( service_name, str_ptr,
                                                 15 );
                                        server_name[ pos ] = 0;
                                        exit_loop = 1;
                                   }
                              }
                         }
                         else  /* Not a number. */
                         {
                              len = strlen( str_ptr );
                              if ( len > 14 )
                              {
                                   printf( "\n\
The service name that you have requested is too long.\n\n" );
                              }
                              else
                              {
                                   strncpy( service_name, str_ptr, len );
                                   exit_loop = 1;
                              }
                         }    /* if ( sscanf( str_ptr, "%d",
                                              &num ) == 1 ) */
                    }    /* if ( pos >= 0 ) */
                    else  /* No port number or service was specified. */
                    {
                         snprintf( service_name, 15, "%u",
                                   ( uint16_t )server_port );
                         exit_loop = 1;
                    }
               }    while( exit_loop == 0 );
          }
          else  /* use_localhost == 1 */
          {
               strcpy( server_name, "localhost" );
               snprintf( service_name, 15, "%u",
                         ( uint16_t )server_port );
          }

     }    /* if ( initial == 1 && use_client == 1 ) */

#ifdef DEBUG

     if ( use_client == 1 )
     {
          printf( "\nUsing server_name: \"%s\", port: \"%s\"\n\n",
                  server_name, service_name );
     }
     printf( "use_client: %d, use_server: %d, use_localhost: %d.\n\n",
             use_client, use_server, use_localhost );

#endif

     /* Open the server's listening socket if it is currently closed. */

     if ( use_server == 1 )
     {
          if ( sock_type != SOCK_DGRAM )
          {
               if ( *lsock_fd == ( -1 ) )
               {
                    errno = 0;
                    ret = socket( AF_INET, sock_type, 0 );
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

                    printf( "\
The server's listening socket has been opened.\n" );

#endif

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
                    ret = socket( AF_INET, sock_type, 0 );
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

                    printf( "The server socket has been opened.\n" );

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

#ifdef USE_BROADCAST_AF_INET

                    /* Set the server socket option SO_BROADCAST. */

                    size = sizeof( opt );
                    opt = 1;
                    errno = 0;
                    ret = setsockopt( *ssock_fd, SOL_SOCKET,
                                      SO_BROADCAST, &opt, size );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when setting the SO_BROADCAST option for the server.\
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
The SO_BROADCAST option has been set on the server socket.\n" );

#endif

#endif  /* USE_BROADCAST_AF_INET */

               }
               else  /* *ssock != ( -1 ) */
               {
                    already_listening = 1;
               }

#ifdef USE_DONTROUTE_AF_INET

               /* Set the SO_DONTROUTE option on the server socket. */

               size = sizeof( opt );
               opt = 1;
               errno = 0;
               ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_DONTROUTE,
                                 &opt, size );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the SO_DONTROUTE option for the server.\
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
The SO_DONTROUTE option has been set on the server socket.\n" );

#endif

#endif  /* USE_DONTROUTE_AF_INET */

          }    /* if ( sock_type != SOCK_DGRAM ) */

          if ( already_listening == 0 )
          {
               /*

                    Set up the server's listening
                    socket's address information.

               */

               memset( &server, 0, size );
               server.sin_family = AF_INET;
               server.sin_port = server_port;

               errno = 0;
               ret = inet_pton( AF_INET, "127.0.0.1",
                                &server.sin_addr.s_addr );
               if ( ret != 1 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when calling inet_pton(3).\n" );
                    if ( ret == 0 )
                    {
                         printf( "\
The specified host is not a valid network \
address for a numeric IPv4 address.\n" );
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

               }    /* if ( ret != 1 ) */

               /*

                    Bind the server's listening socket to an address so
                    it can listen, unless we're using datagram sockets
                    in which case we need to bind the server socket to
                    the network port instead.

               */

               size = sizeof( server );

               errno = 0;
               if ( sock_type != SOCK_DGRAM )
               {
                    ret = bind( *lsock_fd,
                                ( struct sockaddr * )( &server ), size );
               }
               else
               {
                    ret = bind( *ssock_fd,
                                ( struct sockaddr * )( &server ), size );
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

               /* Save the server's address information. */

               memcpy( address, ( void * )( &server ), size );

          }    /* if ( already_listening == 0 ) */

     }    /* if ( use_server == 1 ) */

     if ( use_client == 1 )
     {
          /* Find the numeric IP address for the server. */

          memset( &hints, 0, sizeof( struct addrinfo ) );
          hints.ai_flags = AI_CANONNAME | AI_PASSIVE;
          hints.ai_family = AF_INET;
          hints.ai_socktype = sock_type;

          errno = 0;
          ret = getaddrinfo( server_name, service_name, &hints, &res );
          save_errno = errno;
          if ( ret != 0 )
          {
               printf( "\n\
Something went wrong when calling getaddrinfo(3).\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s (%d).\n", strerror( save_errno ),
                            save_errno );
               }
               printf( "Error message: %s. (%d)\n\n",
                       gai_strerror( ret ), ret );

#ifdef DEBUG

               printf( "Shutting down sockets.\n" );

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

          /* We can't connect to a server that we can't find. */

          if ( res == NULL )
          {
               printf( "\nServer not found.\n\n" );

#ifdef DEBUG

               printf( "Shutting down sockets.\n" );

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

          }    /* if ( res == NULL ) */

          /* Store the printable string IP address in ip_str. */

          addr1 = ( struct sockaddr_in * )res->ai_addr;
          errno = 0;
          if ( inet_ntop( res->ai_family, &( addr1->sin_addr.s_addr ),
                          buffer, 128 ) == NULL )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong while using inet_ntop(3).\n" );
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

               freeaddrinfo( res );
               errno = 0;
               return ( -1 );
          }
          else  /* inet_ntop() != NULL */
          {
               memset( ip_str, 0, 16 );
               strncpy( ip_str, buffer, 15 );
          }

          /* Make sure the host address is unique. */

          list_ptr = res->ai_next;
          addr1 = ( struct sockaddr_in * )res;

          while( list_ptr != NULL )
          {
               addr2 = ( struct sockaddr_in * )list_ptr;

               if ( addr1->sin_addr.s_addr != addr2->sin_addr.s_addr )
               {
                    printf( "\n\
The host specification that you have provided has produced \
ambiguous results.  Please be more specific.\n" );

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

                    freeaddrinfo( res );
                    errno = 0;
                    return ( -1 );

               }    /* if ( addr1->sin_addr.s_addr !=
                            addr2->sin_addr.s_addr ) */

               list_ptr = list_ptr->ai_next;

          }    /* while( list_ptr != NULL ) */

          /* Set up the target information. */

          memset( &server, 0, sizeof( struct sockaddr_in ) );
          server.sin_family = AF_INET;
          server.sin_port = server_port;
          server.sin_addr.s_addr = addr1->sin_addr.s_addr;

          /* We won't need this anymore. */

          freeaddrinfo( res );

          /* Open the client socket if it is currently closed. */

          if ( *csock_fd == ( -1 ) )
          {
               errno = 0;
               ret = socket( AF_INET, sock_type, 0 );
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
               if ( use_server == 1 )
               {
                    /* Create a child process to call accept(2). */

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

#ifdef DEBUG

     printf( "\
The server is ready and waiting to accept a new connection.\n" );

#endif

                         errno = 0;
                         ret = accept( *lsock_fd,
                                       ( struct sockaddr * )( &server ),
                                       &size );
                         save_errno = errno;

                         if ( ret < 0 )
                         {
                              printf( "\n\
Something went wrong when calling accept(2).\n" );
                              if ( save_errno != 0 )
                              {
                                   printf( "Error: %s.\n",
                                           strerror( save_errno ) );
                              }
                              exit( EXIT_FAILURE );
                         }
                         printf( "New server socket fd: %d.\n", ret );
                         exit( EXIT_SUCCESS );
                    }
                    else  /* pid > 0 */
                    {

#ifdef DEBUG

                         printf( "\
Waiting for ten seconds to let the child process do its job.\n" );

#endif

                         sleep( 10 );
                    }

               }    /* if ( use_server == 1 ) */

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
                              sizeof( struct sockaddr_in ) );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong while trying to connect to the server.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
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

                         printf( "\nWaiting for the child process...\n" );

#endif

                         errno = 0;
                         ret = waitpid( ( -1 ), NULL, 0 );
                         save_errno = errno;

#ifdef SHOW_WAIT_ERRORS

                         if ( ret == ( -1 ) )
                         {
                              printf( "\n\
Something went wrong when waiting for the child process.\n" );
                              if ( save_errno != 0 )
                              {
                                   printf( "Error: %s.\n",
                                           strerror( save_errno ) );
                              }
                         }

#endif

#ifdef DEBUG

                         if ( ret == pid )
                         {
                              printf( "The child process has exited.\n" );
                         }

#endif

#endif  /* WAIT_FOR_CHILD */

                    }  /* if ( ret != 0 ) */

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

               }
               else  /* ret == 0 */
               {
                    /* We couldn't pass it back from the child process. */

                    *ssock_fd = *csock_fd + 1;
               }

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

               }  /* if ( use_client == 0 ) */

#ifdef DEBUG

               if ( initial == 1 )
               {
                    printf( "Connection to %s accepted.\n", server_name );
               }
               else
               {
                    printf( "Reconnected to %s.\n", server_name );
               }

#endif

#if defined( SHOW_CONNECTIONS ) && defined( DEBUG )

               /* Save the server's address information. */

               memcpy( ( void * )( &server_addr ), ( void * )( &server ),
                       sizeof( struct sockaddr_in ) );

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

               size = sizeof( opt );
               opt = 1;
               errno = 0;
               ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_KEEPALIVE,
                                 &opt, size );
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

               size = sizeof( opt );
               opt = 1;
               errno = 0;
               ret = setsockopt( *csock_fd, SOL_SOCKET, SO_KEEPALIVE,
                                 &opt, size );
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
               /* Open the client socket if it is currently closed. */

               if ( *csock_fd == ( -1 ) )
               {
                    errno = 0;
                    ret = socket( AF_INET, sock_type, 0 );
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

                    *csock_fd = ret;

#ifdef DEBUG

                    printf( "The client socket has been opened.\n" );

#endif

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
The client socket has been set to nonblocking mode.\n" );

#endif

#ifdef USE_BROADCAST_AF_INET

                    /*

                         Set the SO_BROADCAST option
                         on the client socket.

                    */

                    size = sizeof( opt );
                    opt = 1;
                    errno = 0;
                    ret = setsockopt( *csock_fd, SOL_SOCKET,
                                      SO_BROADCAST, &opt, size );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong when setting the SO_BROADCAST option for the client.\
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
The SO_BROADCAST option has been set on the client socket.\n" );

#endif

#endif  /* USE_BROADCAST_AF_INET */

#ifdef USE_DEFAULT_TARGET_AF_INET

                    /*

                         Set the default target for the
                         client socket with connect(2).

                    */

                    errno = 0;
                    ret = connect( *csock_fd,
                                   ( struct sockaddr * )( &server ),
                                   sizeof( struct sockaddr_in ) );
                    if ( ret != 0 )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while trying to set \
the default target for the client.\n" );
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
The default target for the client socket has\
 been set to the server socket.\n" );

#endif

#endif  /*  USE_DEFAULT_TARGET_AF_INET */

               }    /* if ( *csock_fd == ( -1 ) ) */

          }    /* if ( sock_type != SOCK_DGRAM ) */

     }    /* if ( use_client == 1 ) */

#ifdef USE_DONTROUTE_AF_INET

     if ( use_server == 1 )
     {
          if ( sock_type != SOCK_DGRAM )
          {
               /* Set the SO_DONTROUTE option on the server socket. */

               size = sizeof( opt );
               opt = 1;
               errno = 0;
               ret = setsockopt( *ssock_fd, SOL_SOCKET, SO_DONTROUTE,
                                 &opt, size );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when setting the SO_DONTROUTE option for the server.\
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
The SO_DONTROUTE option has been set on the server socket.\n" );

#endif

          }  /* if ( sock_type != SOCK_DGRAM ) */

     }    /* if ( use_server == 1 ) */

     if ( use_client == 1 )
     {
          /* Set the SO_DONTROUTE option on the client socket. */

          size = sizeof( opt );
          opt = 1;
          errno = 0;
          ret = setsockopt( *csock_fd, SOL_SOCKET, SO_DONTROUTE, &opt,
                            size );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong when setting the SO_DONTROUTE option for the client.\
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
The SO_DONTROUTE option has been set on the client socket.\n" );

#endif

     }    /* if ( use_client == 1 ) */

#endif  /* USE_DONTROUTE_AF_INET */

#ifdef SHOW_CONNECTIONS

#ifdef DEBUG

     /* Datagram sockets aren't connected. */

     if ( sock_type != SOCK_DGRAM )
     {
          size = sizeof( struct sockaddr_in );

          printf( "\n" );

          if ( use_server == 1 )
          {
               /* Show the server's listening socket. */

               errno = 0;
               ret = getsockname( *lsock_fd,
                                  ( struct sockaddr * )( &listen_addr ),
                                  &size );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when calling getsockname(3) \
for the server's listening socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
               }
               else
               {
                    errno = 0;
                    if ( inet_ntop( AF_INET,
                                    &( listen_addr.sin_addr.s_addr ),
                                    buffer, 128 ) == NULL )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while using inet_ntop(3).\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                    }
                    else
                    {
                         printf( "\
The server's listening socket address is listed as %s:%u.\n", buffer,
                                 ( uint16_t )listen_addr.sin_port );
                    }

               }    /* if ( ret != 0 ) */

               /* Show the server socket. */
/*
               errno = 0;
               ret = getsockname( *ssock_fd,
                                  ( struct sockaddr * )( &server_addr ),
                                  &size );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\n\
Something went wrong when calling getsockname(3) for the server socket.\
\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
               }
               else
               {
*/
                    errno = 0;
                    if ( inet_ntop( AF_INET,
                                    &( server_addr.sin_addr.s_addr ),
                                    buffer, 128 ) == NULL )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while using inet_ntop(3).\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                    }
                    else
                    {
                         printf( "\
The server socket address is listed as %s:%u.\n", buffer,
                                 ( uint16_t )server_addr.sin_port );
                    }

//               }    /* if ( ret != 0 ) */

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
                    printf( "\n\
Something went wrong when calling getsockname(3) for the client socket.\
\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n", strerror( save_errno ) );
                    }
                    printf( "\n" );
               }
               else
               {
                    errno = 0;
                    if ( inet_ntop( AF_INET,
                                    &( client_addr.sin_addr.s_addr ),
                                    buffer, 128 ) == NULL )
                    {
                         save_errno = errno;
                         printf( "\n\
Something went wrong while using inet_ntop(3).\n" );
                         if ( save_errno != 0 )
                         {
                              printf( "Error: %s.\n",
                                      strerror( save_errno ) );
                         }
                         printf( "\n" );
                    }
                    else
                    {
                         printf( "\
The client socket address is listed as %s:%u.\n", buffer,
                                 ( uint16_t )client_addr.sin_port );
                    }

               }    /* if ( ret != 0 ) */

          }    /* if ( use_client == 1 ) */

     }    /* if ( sock_type != SOCK_DGRAM ) */

#endif  /* DEBUG */

#endif  /* SHOW_CONNECTIONS */

#ifdef DEBUG

     printf( "\nSetup complete.\n" );
     list_sockets( csock_fd, lsock_fd, ssock_fd );

#endif

     errno = 0;
     return 0;
}

#endif  /* _SETUP_AF_INET_C */

/* EOF setup_af_inet.c */

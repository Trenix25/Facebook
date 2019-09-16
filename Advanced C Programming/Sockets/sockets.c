/*

     sockets.c
     Playing with sockets.
     Written by Matthew Campbell.

     I started writing this file on Sunday November 11, 2018.

*/

/* Include the custom include file for this program. */

#include "sockets.h"

/* Global variables: */

/*
_Atomic sig_io_received;
_Atomic sig_urg_received;
*/

int sig_io_received;
int sig_urg_received;

/* Function definitions: */

int main( void )
{
     uint8_t address[ ADDR_SIZE ];
     static char buffer[ 80 ];

     /* client socket, server's listening socket, and server socket */

     int csock_fd = -1, lsock_fd = -1, ssock_fd = -1;

     int domain = 0, exit_loop, len = 80, ret, save_errno, type = 0;
     struct sigaction alrm_new, io_new, urg_new;

     /* Initialize these global variables: */

     sig_io_received = 0;
     sig_urg_received = 0;

     /* Set up the signal handling functions: */

     /* Set up SIGALRM: */

     /* Clear the data space. */

     memset( &alrm_new, 0, sizeof( alrm_new ) );

     /* Set the new signal handling function. */

     alrm_new.sa_handler = catch_sigalrm;

#ifdef DEBUG

     printf( "Calling sigaction(2) to set the catch for SIGALRM.\n" );

#endif

     ret = sigaction( SIGALRM, &alrm_new, NULL );
     if ( ret != 0 )
     {
          save_errno = errno;
          printf( "\
Something went wrong when trying to setup catch_sigalrm().\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
          exit( EXIT_FAILURE );
     }

     /* Set up SIGIO: */

     memset( &io_new, 0, sizeof( io_new ) );  /* Clear the data space. */
     io_new.sa_handler = catch_sigio;

#ifdef DEBUG

     printf( "Calling sigaction(2) to set the catch for SIGIO.\n" );

#endif

     ret = sigaction( SIGIO, &io_new, NULL );
     if ( ret != 0 )
     {
          save_errno = errno;
          printf( "\
Something went wrong when trying to setup catch_sigio().\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
          exit( EXIT_FAILURE );
     }

     /* Set up SIGURG: */

     memset( &urg_new, 0, sizeof( urg_new ) );  /* Clear the data space. */
     urg_new.sa_handler = catch_sigurg;

#ifdef DEBUG

     printf( "Calling sigaction(2) to set the catch for SIGURG.\n" );

#endif

     ret = sigaction( SIGURG, &urg_new, NULL );
     if ( ret != 0 )
     {
          save_errno = errno;
          printf( "\
Something went wrong when trying to setup catch_sigurg().\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
          exit( EXIT_FAILURE );
     }

#ifdef DEBUG

     printf( "\n" );

#endif

#ifdef TEST_SIGNALS

     /* Test the new signal handlers. */

     printf( "Calling raise( SIGIO ) (1)\n" );
     ret = raise( SIGIO );
     save_errno = errno;
     printf( "Returned from raise( SIGIO ) (1)\n" );

     if ( ret != 0 )
     {
          printf( "Something went wrong when calling raise( SIGIO ).\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
     }

     printf( "\nret: %d, sig_io_received: %d, sig_urg_received: %d.\n\n",
             ret, sig_io_received, sig_urg_received );

     printf( "Calling raise( SIGIO ) (2)\n" );
     ret = raise( SIGIO );
     save_errno = errno;
     printf( "Returned from raise( SIGIO ) (2)\n" );

     if ( ret != 0 )
     {
          printf( "Something went wrong when calling raise( SIGIO ).\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
     }

     printf( "\nret: %d, sig_io_received: %d, sig_urg_received: %d.\n\n",
             ret, sig_io_received, sig_urg_received );

     printf( "Calling raise( SIGURG ) (1)\n" );
     ret = raise( SIGURG );
     save_errno = errno;
     printf( "Returned from raise( SIGURG ) (1)\n" );

     if ( ret != 0 )
     {
          printf( "Something went wrong when calling raise( SIGURG ).\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
     }

     printf( "\nret: %d, sig_io_received: %d, sig_urg_received: %d.\n\n",
             ret, sig_io_received, sig_urg_received );

     printf( "Calling raise( SIGURG ) (2)\n" );
     ret = raise( SIGURG );
     save_errno = errno;
     printf( "Returned from raise( SIGURG ) (2)\n" );

     if ( ret != 0 )
     {
          printf( "Something went wrong when calling raise( SIGURG ).\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }
          printf( "\n" );
     }

     printf( "\nret: %d, sig_io_received: %d, sig_urg_received: %d.\n\n",
             ret, sig_io_received, sig_urg_received );

     /* Reset sig_io_received and sig_urg_received. */

     sig_io_received = 0;
     sig_urg_received = 0;

#endif  /* TEST_SIGNALS */

     exit_loop = 0;
     do
     {
          /* Print the domain menu and seek input from the user. */

          print_domain_menu();
          ret = read_stdin( buffer, len, ">> ", 1 );
          if ( ret != 0 )
          {
               save_errno = errno;
               printf( "\n\
Something went wrong while reading your choice.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }
               printf( "\nProgram failed.  Exiting.\n\n" );
               exit( EXIT_FAILURE );
          }
          if ( sscanf( buffer, "%d", &domain ) != 1 )
          {
               printf( "\n\
That is not valid input.  Please try again.\n\n" );
          }
          else if ( domain < 1 || domain > ( MAX_DOMAINS + 1 ) )
          {
               printf( "\n\
That is not a valid choice.  Please try again.\n\n" );
          }
          else
          {
               exit_loop = 1;
          }
     }    while( exit_loop == 0 );

     if ( domain == ( MAX_DOMAINS + 1 ) )  /* The user chose to exit. */
     {

#ifdef DEBUG

          list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#else

          printf( "\n" );

#endif

          printf( "Successful exit.\n\n" );
          exit( EXIT_SUCCESS );
     }

     /* Open the sockets and get them connected, if applicable. */

     printf( "\n" );
     errno = 0;
     ret = setup_sockets( &csock_fd, &lsock_fd, &ssock_fd, domain, &type,
                          &address, 1 );

     if ( ret == ( -1 ) )
     {
          save_errno = errno;
          printf( "\
Something went wrong while trying to open the sockets.\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }

#ifdef DEBUG

          list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#endif

          printf( "Program failed.  Exiting.\n\n" );
          exit( EXIT_FAILURE );
     }

     if ( csock_fd != ( -1 ) || type != SOCK_DGRAM )
     {
          /* Test the reconnection process. */

#ifdef DEBUG

          printf( "Simulating a broken socket connection.\n" );

#endif

          /* Datagram sockets don't actually have a connection anyway. */

          if ( csock_fd != ( -1 ) )
          {
               ret = close( csock_fd );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\nFailed to close the client socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n" , strerror( save_errno ) );
                    }

#ifdef DEBUG

                    list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#endif

                    printf( "Program failed.  Exiting.\n\n" );
                    exit( EXIT_FAILURE );
               }
               else
               {
                    csock_fd = -1;

#ifdef DEBUG

                    printf( "Client socket closed.\n" );

#endif

               }    /* if ( ret != 0 ) */

          }    /* if ( csock_fd != ( -1 ) ) */

          if ( ssock_fd != ( -1 ) && type != SOCK_DGRAM )
          {
               ret = close( ssock_fd );
               if ( ret != 0 )
               {
                    save_errno = errno;
                    printf( "\nFailed to close the server socket.\n" );
                    if ( save_errno != 0 )
                    {
                         printf( "Error: %s.\n" , strerror( save_errno ) );
                    }

#ifdef DEBUG

                    list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#endif

                    printf( "Program failed.  Exiting.\n\n" );
                    exit( EXIT_FAILURE );
               }
               else
               {
                    ssock_fd = -1;

#ifdef DEBUG

                    printf( "Server socket closed.\n" );

#endif

               }    /* if ( ret != 0 ) */

          }    /* if ( ssock_fd != ( -1 ) && type != SOCK_DGRAM ) */

          /* Now try to reconect. */

#ifdef DEBUG

          printf( "Reconnecting.\n" );

#endif

          errno = 0;
          ret = setup_sockets( &csock_fd, &lsock_fd, &ssock_fd, domain,
                               &type, &address, 0 );

          if ( ret == ( -1 ) )
          {
               save_errno = errno;
               printf( "\
Something went wrong while trying to open the sockets.\n" );
               if ( save_errno != 0 )
               {
                    printf( "Error: %s.\n", strerror( save_errno ) );
               }

#ifdef DEBUG

               list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#endif

               printf( "Program failed.  Exiting.\n\n" );
               exit( EXIT_FAILURE );

          }    /* if ( ret == ( -1 ) ) */

     }    /* if ( csock_fd != ( -1 ) || type != SOCK_DGRAM ) */







     /* Shutdown any active sockets. */

#ifdef DEBUG

     printf( "Shutting down.\n" );

#endif

     errno = 0;
     ret = shutdown_sockets( &csock_fd, &lsock_fd, &ssock_fd, domain,
                             type );

     if ( ret == ( -1 ) )
     {
          save_errno = errno;
          printf( "\n\
Something went wrong while trying to close the sockets.\n" );
          if ( save_errno != 0 )
          {
               printf( "Error: %s.\n", strerror( save_errno ) );
          }

#ifdef DEBUG

          list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#endif

          printf( "Program failed.  Exiting.\n\n" );
          exit( EXIT_FAILURE );
     }

     /* And we're done. */

#ifdef DEBUG

     list_sockets( &csock_fd, &lsock_fd, &ssock_fd );

#else

     printf( "\n" );

#endif

     printf( "Successful exit.\n\n" );
     exit( EXIT_SUCCESS );
}

/* This is our signal hangling function for SIGALRM. */

void catch_sigalrm( int sig_num )
{
     return;
}

/* This is our signal hangling function for SIGIO. */

void catch_sigio( int sig_num )
{
     sig_io_received++;
     return;
}

/* This is our signal handling function for SIGURG. */

void catch_sigurg( int sig_num )
{
     sig_urg_received++;
     return;
}

/* EOF sockets.c */

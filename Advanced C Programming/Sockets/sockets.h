/*

     sockets.h

     Lists include files and function prototypes
     used by sockets.c and its support files.

     Written by Matthew Campbell.

*/

#ifndef _SOCKETS_H
#define _SOCKETS_H

/* kill(2) and sigaction(2) both need this. */

#define _POSIX_SOURCE

/* _exit(2) needs this. */

#define _POSIX_C_SOURCE 200112L

/* Gather the necessary header files. */

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

/* Make sure these are defined: */

#ifndef ENOENT
#define ENOENT 2
#endif

#ifndef EINTR
#define EINTR 4
#endif

#ifndef EFAULT
#define EFAULT 14
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef SIGALRM
#define SIGALRM 14
#endif

#ifndef SIGTERM
#define SIGTERM 15
#endif

/* Define DEBUG to include debugging output. */

#define DEBUG

/* Define TEST_SIGNALS to test the new signal handlers. */

#undef TEST_SIGNALS

/*

     Define USE_BROADCAST_AF_socket_domain
     if you want to allow datagram broadcasts.

*/

#define USE_BROADCAST_AF_BLUETOOTH
#define USE_BROADCAST_AF_INET
#define USE_BROADCAST_AF_INET6

/*

     Define USE_DEFAULT_TARGET_AF_socket_domain if you
     want to set a default target for datagram sockets.

*/

#define USE_DEFAULT_TARGET_AF_BLUETOOTH
#define USE_DEFAULT_TARGET_AF_INET
#define USE_DEFAULT_TARGET_AF_INET6
#define USE_DEFAULT_TARGET_AF_UNIX

/*

     Define USE_DONTROUTE_AF_socket_domain if
     you want the SO_DONTROUTE socket option set.

*/

#undef USE_DONTROUTE_AF_INET
#undef USE_DONTROUTE_AF_INET6

/* Define SHOW_CONNECTIONS to show connected socket address information. */

#define SHOW_CONNECTIONS

#ifdef SHOW_CONNECTIONS
#ifndef DEBUG
#error Defining SHOW_CONNECTIONS requires also defining DEBUG.
#endif
#endif

/* Define SHOW_SOCKET_OPTIONS to include show_socket_options(). */

#define SHOW_SOCKET_OPTIONS

/* Define WAIT_FOR_CHILD if you want to wait for the child process. */

#define WAIT_FOR_CHILD

/* Define SHOW_WAIT_ERRORS if you want to see errors from waitpid(2). */

#ifdef WAIT_FOR_CHILD
#define SHOW_WAIT_ERRORS
#else
#undef SHOW_WAIT_ERRORS
#endif

/* Defines the number of socket domains. */

#define MAX_DOMAINS 4

/* Defines the socket file name to use for AF_UNIX sockets. */

#define SOCK_NAME "socket"

/* Defines the directory to use for the AF_UNIX socket file. */

#define USE_DIR "/data/data/com.n0n3m4.droidc/files"

/*

     Defines the number of backlogged
     connections that are allowed to wait.

*/

#define LISTEN_BACKLOG 10

/*

     At the time this program was written, the sockaddr_in6 was
     the largest socket address structure used by this program.
     This will be used to define a buffer of sufficient size to
     hold a target address for any socket domain used in this
     program.

*/

#define ADDR_SIZE sizeof( struct sockaddr_in6 )

/* Function prototypes: */

int detect_endian( void );

int invert_endian( void *buffer, int size );

int read_stdin( char *buffer, const int length,
                const char *prompt, const int reprompt );

int setup_af_bluetooth( int *csock_fd, int *lsock_fd, int *ssock_fd,
                        int domain, int *type, void *address,
                        int initial );

int setup_af_inet( int *csock_fd, int *lsock_fd, int *ssock_fd,
                   int domain, int *type, void *address,
                   int initial );

int setup_af_inet6( int *csock_fd, int *lsock_fd, int *ssock_fd,
                    int domain, int *type, void *address,
                    int initial );

int setup_af_unix( int *csock_fd, int *lsock_fd, int *ssock_fd,
                   int domain, int *type, void *address,
                   int initial );

int setup_sockets( int *csock_fd, int *lsock_fd, int *ssock_fd,
                   int domain, int *type, void *address,
                   int initial );

int shutdown_sockets( int *csock_fd, int *lsock_fd,
                      int *ssock_fd, int domain, int type );

void catch_sigalrm( int sig_num );

void catch_sigio( int sig_num );

void catch_sigurg( int sig_num );

void list_sockets( int *csock_fd, int *lsock_fd, int *ssock_fd );

void print_domain_menu( void );

#ifdef SHOW_SOCKET_OPTIONS

void show_socket_options( const int sock_fd, const int domain,
                          const int sock_type, const char *sock_name );

#endif

#endif /* _SOCKETS_H */

/* EOF sockets.h */

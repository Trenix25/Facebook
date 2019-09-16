/*

     print_domain_menu.c
     This function prints the domain menu.
     Written by Matthew Campbell.

*/

#ifndef _PRINT_DOMAIN_MENU_C
#define _PRINT_DOMAIN_MENU_C

#include "sockets.h"

void print_domain_menu( void )
{
     printf( "What socket domain would you like to use?\n\n" );
     printf( "1) AF_BLUETOOTH (Nearby Bluetooth communications)\n" );
     printf( "2) AF_INET (IPv4)\n" );
     printf( "3) AF_INET6 (IPv6)\n" );
     printf( "4) AF_UNIX or AF_LOCAL (Local communications)\n" );
     printf( "5) Exit\n\n" );
     return;
}

#endif /* _PRINT_DOMAIN_MENU_C */

/* EOF print_domain_menu.c */

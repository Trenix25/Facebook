#include <stdio.h>
#include <stdlib.h>

#define BCD_TO_DEC( x ) ( ( x / 16 * 10 ) + ( x % 16 ) )
#define DEC_TO_BCD( x ) ( ( x / 10 * 16 ) + ( x % 10 ) )

#define USAGE "Usage: test_bcd <number between 0 to 255 inclusive>\n"

int main( int argc, char **argv )
{
     char bcd_num8, num8;
     int num32;

     if ( argc != 2 )
     {
          printf( "%s", USAGE );
          return 1;
     }

     if ( sscanf( argv[ 1 ], "%d", &num32 ) != 1 )
     {
          printf( "That's not an integer number.\n" );
          return 1;
     }

     if ( num32 < 0 || num32 > 255 )
     {
          printf( "That number is out of range.\n" );
          return 1;
     }

     num8 = ( char )num32;
     bcd_num8 = DEC_TO_BCD( num8 );

     printf( "num: %d %08X.\nBCD: %08X.\n", num32, ( unsigned int )num32,
             ( unsigned int )bcd_num8 );

     num8 = BCD_TO_DEC( bcd_num8 );

     printf( "DEC: %08X.\n", ( unsigned int )num8 );

     return 0;
}

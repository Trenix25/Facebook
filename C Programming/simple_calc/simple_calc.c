/*

     simple_calc.c
     Performs a simple math calculation.
     Rewritten by Matthew Campbell for the C Programing group.

*/

#include <stdio.h>

int main( void )
{
     double num1, num2, result;
     int choice, ret;

     printf( "Please enter the first number: " );
     fflush( stdout );
     ret = scanf( "%lf", &num1 );

     if ( ret != 1 )
     {
          printf( "\nThat is not a valid entry.\n" );
          return 1;
     }

     printf( "\nPlease choose the operator:\n\n" );
     printf( "1) +     3) *\n" );
     printf( "2) -     4) /\n\n" );
     printf( ">> " );
     fflush( stdout );
     ret = scanf( "%d", &choice );

     if ( ret != 1 )
     {
          printf( "\nThat is not a valid entry.\n" );
          return 1;
     }

     if ( choice < 1 || choice > 4 )
     {
          printf( "\nThat is not an available option.\n" );
          return 1;
     }

     printf( "\nPlease enter the second number: " );
     fflush( stdout );
     ret = scanf( "%lf", &num2 );

     if ( ret != 1 )
     {
          printf( "\nThat is not a valid entry.\n" );
          return 1;
     }

     if ( choice == 1 )
     {
          result = num1 + num2;
          printf( "\n%lf + %lf is %lf\n", num1, num2, result );
     }
     else if ( choice == 2 )
     {
          result = num1 - num2;
          printf( "\n%lf - %lf is %lf\n", num1, num2, result );
     }
     else if ( choice == 3 )
     {
          result = num1 * num2;
          printf( "\n%lf * %lf is %lf\n", num1, num2, result );
     }
     else  /* choice == 4 */
     {

          if ( num2 >= -0.0000000000001 && num2 <= 0.0000000000001 )
          {
               printf( "\n\
The second number cannot be zero when dividing.\n" );
               return 1;
          }

          result = num1 / num2;
          printf( "\n%lf / %lf is %lf\n", num1, num2, result );
     }

     return 0;
}

/* EOF simple_calc.c */

/*

     day_of_week.c
     Written by Matthew Campbell.

*/

#include <stdio.h>
#include <stdlib.h>

/*

     Formula variables:

     h is the day of the week.  Saturday is 0, Sunday is 1...

     m is the month.  March is 3, April is 4.  January is 13 from the
                      previous year.  December is 14 from the previous
                      year.

     q is the day of the month.

     y is the year.

*/

int main( void )
{
     char days[ 7 ][ 10 ] = { "Sunday", "Monday", "Tuesday", "Wednesday",
                              "Thursday", "Friday", "Saturday" };

     char invalid_day[ 60 ] = "\n\
That is not a valid day of the month.  Please try again.\n\n";

     char months[ 12 ][ 10 ] = { "January", "February", "March", "April",
                                 "May", "June", "July", "August", "September",
                                 "October", "November", "December" };

     int exit_loop, h, m, q, y;

     /* Ask the user for the month. */

     exit_loop = 0;

     do
     {
          printf( "\
 1) January\n\
 2) February\n\
 3) March\n\
 4) April\n\
 5) May\n\
 6) June\n\
 7) July\n\
 8) August\n\
 9) September\n\
10) October\n\
11) November\n\
12) December\n\n" );

          printf( "Please enter the month: " );
          fflush( stdout );

          if ( scanf( "%d", &m ) != 1 )
          {
               printf( "\n\
That is not an integer number.  Please try again.\n\n" );
               getchar();
          }
          else
          {
               if ( m < 1 || m > 12 )
               {
                    printf( "\n\
That is not an available choice.  Please try again.\n\n" );
               }
               else
               {
                    exit_loop = 1;
                    printf( "\n" );
               }

          }    /* if ( scanf( "%d", &m ) != 1 ) */

     }    while( exit_loop == 0 );

     /* Ask the user for the year. */

     exit_loop = 0;

     do
     {
          printf( "Please enter the year: " );
          fflush( stdout );

          if ( scanf( "%d", &y ) != 1 )
          {
               printf( "\n\
That is not an integer number.  Please try again.\n\n" );
               getchar();
          }
          else
          {
               if ( y < 1900 )
               {
                    printf( "\n\
The year must be greater than or equal to 1900.  Please try again.\n\n" );
               }
               else
               {
                    exit_loop = 1;
                    printf( "\n" );
               }

          }    /* if ( scanf( "%d", &y ) != 1 ) */

     }    while( exit_loop == 0 );

     /* Ask the user for the day of the month. */

     exit_loop = 0;

     do
     {
          printf( "Please enter the day of the month: " );
          fflush( stdout );

          if ( scanf( "%d", &q ) != 1 )
          {
               printf( "\n\
That is not an integer number.  Please try again.\n\n" );
               getchar();
          }
          else
          {
               if ( q < 1 )
               {
                    printf( "%s", invalid_day );
               }
               else
               {
                    if ( m == 1  || m == 3 || m == 5 || m == 7 || m == 8 ||
                         m == 10 || m == 12 )
                    {
                         if ( q > 31 )
                         {
                              printf( "%s", invalid_day );
                         }
                         else
                         {
                              exit_loop = 1;
                              printf( "\n" );
                         }
                    }
                    else if ( m == 2 )
                    {
                         /* See if it's a leap year. */

                         if ( ( y % 100 ) == 0 )
                         {
                              if ( ( y % 400 ) == 0 )
                              {
                                   if ( q > 29 )
                                   {
                                        printf( "%s", invalid_day );
                                   }
                                   else
                                   {
                                        exit_loop = 1;
                                        printf( "\n" );
                                   }
                              }
                              else
                              {
                                   if ( q > 28 )
                                   {
                                        printf( "%s", invalid_day );
                                   }
                                   else
                                   {
                                        exit_loop = 1;
                                        printf( "\n" );
                                   }

                              }    /* if ( ( y % 400 ) == 0 ) */
                         }
                         else  /* ( y % 100 ) != 0 */
                         {
                              if ( ( y % 4 ) == 0 )
                              {
                                   if ( q > 29 )
                                   {
                                        printf( "%s", invalid_day );
                                   }
                                   else
                                   {
                                        exit_loop = 1;
                                        printf( "\n" );
                                   }
                              }
                              else
                              {
                                   if ( q > 28 )
                                   {
                                        printf( "%s", invalid_day );
                                   }
                                   else
                                   {
                                        exit_loop = 1;
                                        printf( "\n" );
                                   }

                              }    /* if ( ( y % 4 ) == 0 ) */

                         }    /* if ( ( y % 100 ) == 0 ) */
                    }
                    else  /* m is a 30 day month */
                    {
                         if ( q > 30 )
                         {
                              printf( "%s", invalid_day );
                         }
                         else
                         {
                              exit_loop = 1;
                              printf( "\n" );
                         }

                    }    /* if ( month has 31 days ) */

               }    /* if ( q < 1 ) */

          }    /* if ( scanf( "%d", &q ) != 1 ) */

     }    while( exit_loop == 0 );

     /* Adjust the month value. */

     if ( m < 3 )
     {
          m = m + 12;
          y = y - 1;
     }

     /* Find the day of the week. */

     /* The formula provided in the assignment was wrong. */

     h = ( q + ( ( 13 * ( m + 1 ) ) / 5 ) + ( y % 100 ) +
               ( ( y % 100 ) / 4 ) + ( ( y / 100 ) / 4 ) -
               ( 2 * ( y / 100 ) ) ) % 7;

     /* Check the result. */

     if ( h < 0 || h > 6 )
     {
          printf( "\
The formula appears to be defective.  Invalid result: %d\n", h );
          return 1;
     }

     /* Adjust the day of the week so Sunday is zero. */

     if ( h == 0 )
     {
          h = 6;
     }
     else
     {
          h = h - 1;
     }

     /* Return to the normal month number. */

     if ( m > 12 )
     {
          m = m - 12;
          y = y + 1;
     }

     /* Print the answer. */

     printf( "The day of the week for %s %d, %d is %s.\n",
             months[ ( m - 1 ) ], q, y, days[ h ] );

     return 0;
}

/* EOF day_of_week.c */

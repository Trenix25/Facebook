/*

     make_query_string.c
     This is a piece of a much larger open source C library file.
     Written by Matthew Campbell.

*/

#include "query_string.h"

/*

  >> Search string: #JJ

  >> Parameters:

     void *labels_vptr, void *values_vptr, const int label_length,
     const int value_length.

     labels_vptr is a void pointer that points to an array of null
     terminated label strings.

     values_vptr is a void pointer that points to an array of null
     terminated value strings.

     label_length is the fixed length of the label strings in bytes
     if the labels array uses a contiguous block of memory to hold
     an array of null terminated label strings of fixed length
     packed back to back.

     value_length is the fixed length of the value strings in bytes
     if the values array uses a contiguous block of memory to hold
     an array of null terminated value strings of fixed length
     packed back to back.

     It is also allowed to use an array of character pointers for
     the labels and values arrays.  It is also allowed to mix and
     match so one array uses a contiguous block of memory to hold
     an array of null terminated fixed length strings packed back
     to back and the other array uses an array of character pointers.

     If an array that uses a contiguous block of memory to hold an
     array of null terminated fixed length strings is used then the
     associated label or value length must be specified in the
     correct integer parameter and the last string in the array of
     null terminated fixed length character strings must be an empty
     string, meaning that the first byte in the string must be a null
     byte.

     An array that uses a contiguous block of memory to hold an
     array of null terminated fixed length strings is a two
     dimensional array of character strings.  It may be created in
     essentially one of two ways.  It may be defined as a local
     variable on the stack as char strings[ 5 ][ 10 ]; for an array
     that holds 5 null terminated fixed length character strings,
     including the empty string at the end of the array, each of
     which may hold up to 10 bytes of character data including the
     null byte at the end of the character string.  It may also be
     created by using dynamically allocated memory from the heap
     with char **strings; strings = malloc( 5 * 10 * sizeof( char ) );
     to create an array of the same size to hold the same data in
     the same way.  The program that uses this option could use
     a character pointer char *str; to point to the beginning of
     the character string in use.  If the calling program has
     created an array using the first method then it may use the
     sizeof() operator to specify the value for the label or value
     length parameter when calling this function by using
     sizeof( strings[ 0 ] ).

     If an array uses an array of character pointers then the last
     element of the array must be a null pointer and the associated
     label or value length integer must be zero.  If an array of
     character pointers is used then each pointer, except for the
     null pointer at the end of the array, must point to a null
     terminated character string.  The last string in the list of
     labels or values does not need to be an empty string if an
     array of character pointers is used for that array.  The label
     or value strings may be variable length null terminated
     character strings if an array of character pointers is used for
     that array.

     An array that uses an array of character pointers may be created
     by char **strings; strings = malloc( 5 * sizeof( char * ) ); to
     create an array of 5 character pointers that, except for the null
     pointer in the last element of the array, each point to a null
     terminated character string.  Always check what malloc(3)
     returns before attempting to use it to ensure that the program
     has been given something that it can use.  Do not type cast what
     malloc(3) returns unless your compiler demands that you do so.

     Label strings, except for the last character string that marks
     the end of the array if label_length holds a number greater than
     zero, must not be empty strings.  Value strings may be empty
     strings.  The number of label strings must equal the number of
     value strings.

     This function will count the number of label strings in the
     array that holds the list of label strings which is why the
     last string, in the case of label_length holding a value
     greater than zero, must be an empty string, or a null pointer
     in the case of label_length holding a value of zero.

  >> Purpose:

     This function accepts lists of labels and values, and the
     list of label strings must end with an empty string in the
     second dimension of the array or a NULL pointer depending
     on how the data is organized in memory, and creates a new
     query string using dynamically allocated memory from the
     heap which must be freed when the calling program is
     finished using it.  Both lists must contain the same number
     of character strings.

     If USE_PLUS is defined in query_string.h then the + character
     will be used to replace the space character in the new query
     string instead of the percent encoded string %20.  Some web
     servers may not accept a query string that uses this option.

     If USE_SEMICOLON is defined in query_string.h then the
     semicolon character ; will be used instead of the ampersand
     character & to separate pairs of label/value strings in the
     new query string.  Some web servers may not accept a query
     string that uses this option.

     This function uses the defined type address_t defined in
     query_string.h for address pointer arithmetic if the numeric
     label or value length parameter used when calling this function
     is greater than zero.

     This function cannot detect if the values array is empty if
     value_length is greater than zero and the first value string
     happens to be an empty string since value strings are allowed
     to be empty strings.  This function might cause the program to
     crash with a segmentation fault if it is given invalid values
     that it cannot reliably detect.  It can only do such much, and
     given the operational constraints that it is required to work
     with, it does the best it can.

     A query string uses percent encoding to encode certain byte
     values for use with a web server.  Only certain byte values may
     be used by a web browser when sending form submission data to
     a web server using the GET method.  Other byte values must use
     percent encoding.  This function creates a new query string from
     the lists of labels and values to satisfy this requirement.

  >> Returns:

     Returns a new null terminated query string that uses dynamically
     allocated memory from the heap which must be freed when the
     calling program is finished using it, or NULL if an error occurs.

     The format of the new query string depends on whether certain
     instructions are defined in query_string.h as indicated above.

  >> Sets errno:

     Sets errno to EFAULT if a null pointer is received for either
     array.  Sets errno to EINVAL if label_length or value_length
     is less than zero.  Sets errno to ENODATA if the labels or
     values array is empty.  Please note the exception mentioned
     above.  Sets errno to ENOMEM if malloc(3) fails to allocate
     the memory for the new query string.

*/

char *make_query_string( void *labels_vptr, void *values_vptr,
                         const int label_length, const int value_length )
{
     char first, *label, **labels, second, *str, *value, **values;
     int count, length, num_labels, num_values, pos1, pos2, pos3;

     /* Make sure we have received valid labels and values. */

     if ( labels_vptr == NULL || values_vptr == NULL )
     {
          errno = EFAULT;
          return NULL;
     }

     /* Make sure both of these values are valid. */

     if ( label_length < 0 || value_length < 0 )
     {
          errno = EINVAL;
          return NULL;
     }

     /* Type cast the arrays to make them useful. */

     labels = ( char ** )labels_vptr;
     values = ( char ** )values_vptr;

     if ( label_length == 0 )
     {
          label = labels[ 0 ];
     }
     else
     {
          label = ( char * )labels;
     }

     if ( value_length == 0 )
     {
          value = values[ 0 ];
     }
     else
     {
          value = ( char * )values;
     }

     /* See if either array is empty.  Value strings may be empty. */

     if ( ( label_length == 0 && labels[ 0 ] == NULL ) ||
          ( label_length > 0  && label[ 0 ] == 0     ) ||
          ( value_length == 0 && values[ 0 ] == NULL ) )
     {
          errno = ENODATA;  /* Empty array. */
          return NULL;
     }

     /* Find out how much memory the query string will need: */

     /* Count the labels and find out how much space they use. */

     length = num_labels = pos1 = 0;
     while( ( label_length == 0 && labels[ pos1 ] != NULL ) ||
            ( label_length > 0  && label[ 0 ] != 0        ) )
     {
          pos2 = 0;
          while( label[ pos2 ] != 0 )
          {
               if (   label[ pos2 ] == '*'   ||
                      label[ pos2 ] == '-'   ||
                      label[ pos2 ] == '.'   ||
                    ( label[ pos2 ] >= '0' &&
                      label[ pos2 ] <= '9' ) ||
                    ( label[ pos2 ] >= 'A' &&
                      label[ pos2 ] <= 'Z' ) ||
                    ( label[ pos2 ] >= 'a' &&
                      label[ pos2 ] <= 'z' ) ||
                      label[ pos2 ] == '_' )
               {
                    length++;
               }
               else  /* Percent encoding is required. */
               {
                    if ( label[ pos2 ] == 32 )  /* Space */
                    {

#ifdef USE_PLUS

                         length++;

#else

                         length = length + 3;

#endif

                    }
                    else  /* Not a space character. */
                    {
                         length = length + 3;

                    }    /* if ( label[ pos2 ] == 32 ) */
               }
               pos2++;  /* Move to the next byte in the current label. */

          }    /* while( label[ pos2 ] != 0 ) */

          /* Move to the next label in the list. */

          pos1++;

          if ( label_length == 0 )
          {
               label = labels[ pos1 ];
          }
          else
          {
               label = ( char * )( ( address_t )labels +
                                   ( address_t )( label_length * pos1 ) );
          }

          /* Make room for the equal sign. */

          length++;

          /* Count the number of label strings in the array. */

          num_labels++;

     }    /*  while( ( label_length == 0 && labels[ pos1 ] != NULL ) ||
                     ( label_length > 0  && label[ 0 ] != 0        ) ) */

     /* Count the values and find out how much space they use. */

     num_values = pos1 = 0;
     while( num_values < num_labels )  /* Values can be empty strings. */
     {
          pos2 = 0;
          while( value[ pos2 ] != 0 )
          {
               if (   value[ pos2 ] == '*'   ||
                      value[ pos2 ] == '-'   ||
                      value[ pos2 ] == '.'   ||
                    ( value[ pos2 ] >= '0' &&
                      value[ pos2 ] <= '9' ) ||
                    ( value[ pos2 ] >= 'A' &&
                      value[ pos2 ] <= 'Z' ) ||
                    ( value[ pos2 ] >= 'a' &&
                      value[ pos2 ] <= 'z' ) ||
                      value[ pos2 ] == '_' )
               {
                    length++;
               }
               else  /* Percent encoding is required. */
               {
                    if ( value[ pos2 ] == 32 )  /* Space */
                    {

#ifdef USE_PLUS

                         length++;

#else

                         length = length + 3;

#endif

                    }
                    else  /* Not a space character. */
                    {
                         length = length + 3;

                    }    /* if ( values[ pos1 ][ pos2 ] == 32 ) */
               }
               pos2++;  /* Move to the next byte in the current value. */

          }    /* while( value[ pos2 ] != 0 ) */

          /* Move to the next value in the list. */

          pos1++;

          if ( value_length == 0 )
          {
               value = values[ pos1 ];
          }
          else
          {
               value = ( char * )( ( address_t )values +
                                   ( address_t )( value_length * pos1 ) );
          }

          /* Make room for the semicolon or ampersand character. */

          length++;

          /* Count the number of value strings in the array. */

          num_values++;

     }    /*  while( num_values < num_labels ) */

     /* Allocate the memory for the query string. */

     str = malloc( length );  /* Room for the null byte is included. */
     if ( str == NULL )
     {
          /* malloc(3) should have already set errno. */

          return NULL;
     }

     /* Build the new query string. */

     if ( label_length == 0 )
     {
          label = labels[ 0 ];
     }
     else
     {
          label = ( char * )labels;
     }

     if ( value_length == 0 )
     {
          value = values[ 0 ];
     }
     else
     {
          value = ( char * )values;
     }

     pos1 = pos2 = 0;
     for( count = 0; count < num_labels; count++ )
     {
          /* Copy the label. */

          pos3 = 0;
          while( label[ pos3 ] != 0 )
          {
               if (   label[ pos3 ] == '*'   ||
                      label[ pos3 ] == '-'   ||
                      label[ pos3 ] == '.'   ||
                    ( label[ pos3 ] >= '0' &&
                      label[ pos3 ] <= '9' ) ||
                    ( label[ pos3 ] >= 'A' &&
                      label[ pos3 ] <= 'Z' ) ||
                    ( label[ pos3 ] >= 'a' &&
                      label[ pos3 ] <= 'z' ) ||
                      label[ pos3 ] == '_' )
               {
                    str[ ( pos1++ ) ] = label[ ( pos3++ ) ];
               }
               else  /* Percent encoding is required. */
               {
                    if ( label[ pos3 ] == 32 )  /* Space */
                    {

#ifdef USE_PLUS

                         str[ ( pos1++ ) ] = '+';
                         pos3++;

#else

                         str[ ( pos1++ ) ] = '%';
                         str[ ( pos1++ ) ] = '2';
                         str[ ( pos1++ ) ] = '0';
                         pos3++;

#endif

                    }
                    else  /* Not a space character. */
                    {
                         /* Convert the byte value to hexadecimal. */

                         first = ( label[ pos3 ] & 0xF0 ) / 16;
                         second = label[ pos3 ] & 0x0F;

                         /* Store the value. */

                         str[ ( pos1++ ) ] = '%';
                         str[ ( pos1++ ) ] = first;
                         str[ ( pos1++ ) ] = second;
                         pos3++;

                    }    /* if ( label[ pos3 ] == 32 ) */

               }    /* if (   label[ pos3 ] == '*'   ||
                              label[ pos3 ] == '-'   ||
                              label[ pos3 ] == '.'   ||
                            ( label[ pos3 ] >= '0' &&
                              label[ pos3 ] <= '9' ) ||
                            ( label[ pos3 ] >= 'A' &&
                              label[ pos3 ] <= 'Z' ) ||
                            ( label[ pos3 ] >= 'a' &&
                              label[ pos3 ] <= 'z' ) ||
                              label[ pos3 ] == '_' ) */

          }    /* while( label[ pos3 ] != 0 ) */

          str[ ( pos1++ ) ] = '=';

          /* Copy the value. */

          pos3 = 0;
          while( value[ pos3 ] != 0 )
          {
               if (   value[ pos3 ] == '*'   ||
                      value[ pos3 ] == '-'   ||
                      value[ pos3 ] == '.'   ||
                    ( value[ pos3 ] >= '0' &&
                      value[ pos3 ] <= '9' ) ||
                    ( value[ pos3 ] >= 'A' &&
                      value[ pos3 ] <= 'Z' ) ||
                    ( value[ pos3 ] >= 'a' &&
                      value[ pos3 ] <= 'z' ) ||
                      value[ pos3 ] == '_' )
               {
                    str[ ( pos1++ ) ] = value[ ( pos3++ ) ];
               }
               else  /* Percent encoding is required. */
               {
                    if ( value[ pos3 ] == 32 )  /* Space */
                    {

#ifdef USE_PLUS

                         str[ ( pos1++ ) ] = '+';
                         pos3++;

#else

                         str[ ( pos1++ ) ] = '%';
                         str[ ( pos1++ ) ] = '2';
                         str[ ( pos1++ ) ] = '0';
                         pos3++;

#endif

                    }
                    else  /* Not a space character. */
                    {
                         /* Convert the byte value to hexadecimal. */

                         first = ( value[ pos3 ] & 0xF0 ) / 16;
                         second = value[ pos3 ] & 0x0F;

                         /* Store the value. */

                         str[ ( pos1++ ) ] = '%';
                         str[ ( pos1++ ) ] = first;
                         str[ ( pos1++ ) ] = second;
                         pos3++;

                    }    /* if ( value[ pos3 ] == 32 ) */

               }    /* if (   value[ pos3 ] == '*'   ||
                              value[ pos3 ] == '-'   ||
                              value[ pos3 ] == '.'   ||
                            ( value[ pos3 ] >= '0' &&
                              value[ pos3 ] <= '9' ) ||
                            ( value[ pos3 ] >= 'A' &&
                              value[ pos3 ] <= 'Z' ) ||
                            ( value[ pos3 ] >= 'a' &&
                              value[ pos3 ] <= 'z' ) ||
                              value[ pos3 ] == '_' ) */

          }    /* while( value[ pos3 ] != 0 ) */

#ifdef USE_SEMICOLON

          str[ ( pos1++ ) ] = ';';

#else

          str[ ( pos1++ ) ] = '&';

#endif

          pos2++;  /* Move to the next label/value pair. */

          if ( label_length == 0 )
          {
               label = labels[ pos2 ];
          }
          else
          {
               label = ( char * )( ( address_t )labels +
                                   ( address_t )( label_length * pos2 ) );
          }

          if ( value_length == 0 )
          {
               value = values[ pos2 ];
          }
          else
          {
               value = ( char * )( ( address_t )values +
                                   ( address_t )( value_length * pos2 ) );
          }

     }    /* for( count = 0; count < num_labels; count++ ) */

     /* Put a null byte on the end of the query string. */

     str[ ( --pos1 ) ] = 0;  /* This replaces the last ; or &. */

     /* Return the new query string. */

     return str;
}

/* EOF make_query_string.c */

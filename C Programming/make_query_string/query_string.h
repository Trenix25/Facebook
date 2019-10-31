/*

     query_string.h
     This is the header file for the query_string.c library file.
     Written by Matthew Campbell.

*/

#ifndef _QUERY_STRING_H
#define _QUERY_STRING_H

/* Include what we need: */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Make sure these are defined: */

#ifndef EFAULT
#define EFAULT 14
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef ENODATA
#define ENODATA 61
#endif

#ifndef EILSEQ
#define EILSEQ 84
#endif

/* Define the characters to block: */

#define QS_SINGLE_QUOTE 0x01
#define QS_DOUBLE_QUOTES 0x02
#define QS_GRAVE_QUOTE 0x04

#define QS_ALL_QUOTES 0x07

#define QS_LESS_THAN 0x08
#define QS_GREATER_THAN 0x10
#define QS_SLASH 0x20
#define QS_AMPERSAND 0x40
#define QS_SEMICOLON 0x80

#define QS_ALL_HTML 0xF8

#define QS_ALL 0xFF

/*

     Define USE_PLUS if you want any created query strings to replace a
     space character with a plus character instead of replacing a space
     character with %20.

*/

#undef USE_PLUS

/*

     Define USE_SEMICOLON if you want any created query strings to use a            semicolon instead of an ampersand character to separate label/value
     pairs.

*/

#undef USE_SEMICOLON

/*

     Change this if unsigned int is not compatible with
     the address pointers on the target architecture.

     It will be used for type casting
     for address pointer arithmetic.

*/

typedef unsigned int address_t;

/* Define the node structure: */

struct _qs_node  /* query string node */
{
     char *label;
     char *value;
     struct _qs_node *next;
};

typedef struct _qs_node qs_node;

/* Function prototypes: */

int       count_duplicate_labels( char *label, char **labels );

int       count_list_nodes( qs_node *list );

int       decode_value_string( char *str, const int block );

int      *find_duplicate_label_indexes( char *label, char **labels );

int       find_index_by_label( char *label, char **labels,
                               const int last_found );

qs_node  *find_node_by_label( char *label, qs_node *list,
                              qs_node *last_found );

void      free_query_string_list( qs_node *list );

char     *identify( qs_node *list );

char    **make_label_list( qs_node *list );

char     *make_query_string( void *labels_vptr, void *values_vptr,
                             const int label_length,
                             const int value_length );

qs_node  *make_query_string_list( const char *str );

char    **make_value_list( qs_node *list );

#endif  /* _QUERY_STRING_H */

/* EOF query_string.h */

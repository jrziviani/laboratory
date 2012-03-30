#include <stdio.h>
#include <limits.h>

#include "bf.h"

static int push( stack *st, char operator )
{
    if ( operator != '>' )
        return 0;

    // stack pointer is on the limit
    if ( st->sp >= PSIZE ) {

        fprintf( stderr, "Error: This code overflows the "
                         "the stack and cannot continue\n");
        return -1;
    }

    // increment the stack pointer
    ++st->sp;

    return 0;
}

static int pop( stack *st, char operator )
{
    if ( operator != '<' )
        return 0;

    // stack pointer is on the limit
    if ( st->sp == 0 ) {

        fprintf( stderr, "Error: This code underflows the "
                         "stack and cannot continue\n");

        return -1;
    }

    // decrement the stack pointer
    --st->sp;

    return 0;
}

static int increment( stack *st, char operator )
{
    if ( operator != '+' )
        return 0;

    // cannot increment over UCHAR_MAX
    // ** This check cannot be done because some brainf*ck programs take
    // advantage of this jump from UCHAR_MAX -> 0
    /*
    if ( st->program[ st->sp ] == UCHAR_MAX ) {

        fprintf( stderr, "Error: The value cannot be higher than "
                          "%d\n", UCHAR_MAX);
        return -1;
    }
    */

    // increment the value
    ++st->program[ st->sp ];

    return 0;
}

static int decrement( stack *st, char operator )
{
    if ( operator != '-' )
        return 0;

    // cannot decrement under 0
    // ** This check cannot be done because some brainf*ck programs take
    // advantage of this jump from 0 -> UCHAR_MAX
    /*
    if ( st->program[ st->sp ] == 0 ) {

        fprintf( stderr, "Error: The value cannot be lower than "
                         "%d\n", 0);
        return -1;
    }
    */

    // decrement the value
    --st->program[ st->sp ];

    return 0;
}

static int print_char( stack *st, char operator )
{
    if ( operator != '.' )
        return -1;

    // print the value from the stack
    putchar( st->program[ st->sp ] );

    return 0;
}

static int get_char( stack *st, char operator )
{
    if ( operator != ',' )
        return 0;

    // put a char from stdin into the stack
    st->program[ st->sp ] = getchar();

    return 0;
}

static int loop( stack *st, char **buffer )
{
    if ( **buffer != '[' )
        return 0;

    // step the initial bracket '['
    ++*buffer;

    while ( st->program[ st->sp ] ) {

        execute ( st, *buffer );
    }

    // the whole loop scope "[...[..]..[.]]" has been processed, 
    // it is time now to take the program to the closing bracket
    int in = 0;
    while ( **buffer ) {

        if ( **buffer == '[' )
            in++;
        else if ( **buffer == ']' && in )
            in--;
        else if (**buffer == ']' && !in)
            break;

        ++*buffer;
    }

    return 0;
}

static int execute( stack *st, char *buffer )
{
    do {
        // exit loop conditions
        if ( *buffer == ']' || *buffer == 0 )
            return 0;

        push       ( st, *buffer );
        pop        ( st, *buffer );
        increment  ( st, *buffer );
        decrement  ( st, *buffer );
        print_char ( st, *buffer );
        get_char   ( st, *buffer ); 
        loop       ( st, &buffer );

    } while ( ++buffer );

    return 0;
}

int main(int argc, char *argv[])
{
    long file_size = 0;
    char buffer[ PSIZE ];

    stack bfprogram;

    // source file must be passed by parameter
    if (argc != 2) {
        
        fprintf( stderr, "\nusage: ./bf source.bf\n" );
        return 1;
    }

    FILE *source;
    if (( source = fopen( argv[1], "rb" )) == NULL ) {

        fprintf( stderr, "Error: Source file cannot be read\n" );
        return 2;
    }

    // get the source file size
    fseek( source, 0 ,SEEK_END );
    file_size = ftell( source );
    rewind( source );

    // the source file content will be stored in a PSIZE buffer
    // so it is safer to check its size
    if ( file_size > PSIZE ) {

        fprintf( stderr, "Error: This program can interpret program\n "
                         "with less the 30,000 instructions so far\n" );
        return 3;
    }

    // put the source file contents in the buffer
    if ( fread( buffer, 1, file_size, source ) != file_size ) {

        fprintf( stderr, "Error: Internal error\n" );
        fclose( source );
        return 4;
    }

    fclose( source );

    buffer[file_size] = 0;

    // interpret the program
    execute( &bfprogram, buffer );

    return 0;
}

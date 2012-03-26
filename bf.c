#include <stdio.h>
#include <limits.h>

#define PSIZE 30000
int stack[ PSIZE ];
int *sp = stack;

int increment_ptr( int **ptr, int *c, char operator )
{
    if ( operator != '>' )
        return 0;

    // check if we will not overflow our stack
    // TODO: runtime error log
    if ( *c >= PSIZE )
        return -1;

    // increment the stack pointer
    ++*ptr;

    // increment the counter
    ++*c;
    
    return 0;
}

int decrement_ptr( int **ptr, int *c, char operator )
{
    if ( operator != '<' )
        return 0;

    // check if we will not underflow our stack
    // TODO: runtime error log
    if ( c <= 0 )
        return -1;

    // decrement the stack pointer
    --*ptr;

    // decrement the counter
    --*c;

    return 0;
}

int increment( int *ptr, char operator )
{
    if ( operator != '+' )
        return 0;

    // we cannot increment if we are on the maximum already
    // TODO: runtime error log
    if ( *ptr == INT_MAX )
        return -1;

    // increment the value
    ++*ptr;

    return 0;
}

int decrement( int *ptr, char operator )
{
    if ( operator != '-' )
        return 0;

    // we cannot decrement if we are on the minimum already
    // TODO: runtime error log
    if ( *ptr == INT_MIN )
        return -1;

    // decrement the value
    --*ptr;
    return 0;
}

int print_char( int *ptr, char operator )
{
    if ( operator != '.' )
        return -1;

    // print the value pointed by the stack pointer
    putchar( *ptr );
//    printf("%d, ", *ptr);

    return 0;
}

int get_char( int *ptr, char operator )
{
    if ( operator != ',' )
        return 0;

    // put a char from the stdin in a room pointed 
    // by the stack pointer
    *ptr = getchar();

    return 0;
}

int execute( char *buffer, int *c );
int loop( int *ptr, char **buffer, int *c )
{
    if ( **buffer != '[' )
        return 0;

    int tmp = *ptr;

    ++*buffer;
    
    while ( tmp > 0 )
        tmp = execute ( *buffer, c );

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

int execute( char *buffer, int *c )
{
    do {

        // exit loop conditions
        if ( *buffer == ']' )
            return *sp;

        if ( *buffer == 0 )
            return -1;

        increment_ptr( &sp, c, *buffer );
        decrement_ptr( &sp, c, *buffer );
        increment    ( sp, *buffer );
        decrement    ( sp, *buffer );
        print_char   ( sp, *buffer );
        get_char     ( sp, *buffer ); 
        loop         ( sp, &buffer, c );

    } while ( ++buffer );

    return 0;
}

int main(int argc, char *argv[])
{
    int c = 0;
    long file_size = 0;
    char buffer[ PSIZE ];

    // source file must be passed by parameter
    // TODO: runtime error log
    if (argc != 2)
        return 1;

    FILE *source;
    // TODO: runtime error log
    if (( source = fopen( argv[1], "rb" )) == NULL )
        return 2;

    // get the source file size
    fseek( source, 0 ,SEEK_END );
    file_size = ftell( source );
    rewind( source );

    // the source file content will be stored in a PSIZE buffer
    // so it is safer to check its size
    // TODO: runtime error log
    if ( file_size > PSIZE )
        return 3;

    // put the source file contents in the buffer
    // TODO: runtime error log
    if ( fread( buffer, 1, file_size, source ) != file_size ) {
        fclose( source );
        return 4;
    }

    fclose( source );

    buffer[file_size] = 0;

    // interpret the program
    execute( buffer, &c );

    return 0;
}

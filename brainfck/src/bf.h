#ifndef BF_H_
#define BF_H

#define PSIZE 30000

typedef struct {

    int sp;
    unsigned char program[ PSIZE ];

} stack;

static int increment  ( stack *, char );
static int decrement  ( stack *, char );
static int push       ( stack *, char );
static int pop        ( stack *, char );
static int print_char ( stack *, char );
static int get_char   ( stack *, char );
static int loop       ( stack *, char ** );
static int execute    ( stack *, char * );

#endif

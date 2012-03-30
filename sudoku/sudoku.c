#include <stdio.h>
#include <string.h>

#define NUM 9
#define COL NUM
#define LIN NUM

typedef unsigned char byte;

// sudoku 9x9 dummy game
byte game[LIN][COL];

// a sudoku game to be resolved
byte ngame[LIN][COL] = {
    { 9, 4, 0, 1, 0, 2, 0, 5, 8 },
    { 6, 0, 0, 0, 5, 0, 0, 0, 4 },
    { 0, 0, 2, 4, 0, 3, 1, 0, 0 },
    { 0, 2, 0, 0, 0, 0, 0, 6, 0 },
    { 5, 0, 8, 0, 2, 0, 4, 0, 1 },
    { 0, 6, 0, 0, 0, 0, 0, 8, 5 },
    { 0, 0, 1, 6, 0, 8, 7, 0, 0 },
    { 7, 0, 0, 0, 4, 0, 0, 0, 3 },
    { 4, 3, 0, 5, 0, 9, 0, 1, 2 }
};

// checks it the number is valid (it has not been used in the same line, column
// or 3x3 box
byte is_valid(byte g[][COL], byte number, byte line, byte column)
{
    byte i, j;

    // this is a small trick to take advantage of cast from float to int. 
    // By truncating (casting) the value and multiplicating it again we
    // will be able to find the lower box boundary
    byte x_box = (byte)(line / 3 * 3);
    byte y_box = (byte)(column / 3 * 3);

    // check if number is in the line
    for (j = 0; j < COL; ++j) {
        if (g[line][j] == number)
            return 0;
    }

    // check if number is in the column
    for (i = 0; i < LIN; ++i) {
        if (g[i][column] == number)
            return 0;
    }

    // check if the number is in the 3x3 box
    // +-----+
    // | 123 |
    // | 456 |
    // | 789 |
    // +-----+
    for (i = x_box; i < x_box + 3; ++i) {
        for (j = y_box; j < y_box + 3; ++j) {
            if (g[i][j] == number)
                return 0;
        }
    }

    // the number is ok to use in the given cell
    return 1;
}

// gets the next cell waiting to be filled
void get_empty_cell(byte g[][COL], byte *line, byte *column)
{
    byte i, j;

    *line = *column = NUM;

    for (i = 0; i < LIN; ++i) {
        for (j = 0; j < COL; ++j) {

            // the first empty cell found, return with line and column
            if (g[i][j] == 0) {
                *line = i;
                *column = j;

                return;
            }
        }
    }
}

// resolves a sudoku game
int resolve_game(byte line, byte column)
{
    byte j;

    // try to find an place to be filled in the game
    get_empty_cell(ngame, &line, &column);

    // no place left, it means we are done
    if (line == NUM || column == NUM)
        return 1;
    
    // try all possible values for this row (line x column)
    for (j = 1; j <= NUM; ++j) {

        // check if this number is ok for the place
        if (is_valid(ngame, j, line, column)) {

            // assing this valid value
            ngame[line][column] = j;

            // try the next empty cell 
            if (resolve_game(line, column))
                return 1;

            // if the code hit this point it means we got a failure, so we will
            // reset this field and start the next iteration
            ngame[line][column] = 0;
        }
    }

    return 0;
}

int dummy_game(byte value, byte line, byte column)
{
    byte j;

    // this is the stop condition, once we reach the end return true
    // TODO: check for any 0 field instead of simply checking
    // game[line][column] != 0
    if (column >= COL-1 && line >= LIN-1 && game[line][column] != 0)
        return 1;

    // value is ok to be assigned in the given line x column
    if (is_valid(game, value, line, column)) {

        // assing this valid value
        game[line][column] = value;

        // try the next column, if we reach the last column restart it and go
        // to the next line
        ++column;
        if (column >= COL) {
            column = 0;
            ++line;
        }

        // try all possible values for this row (line x column)
        for (j = 1; j <= NUM; ++j) {

            // if we reach the end point, satisfiyng our stop condition it
            // means the game is done
            if (dummy_game(j, line, column))
                return 1;
        }

        // if the code hit this point it means we got a failure, so we will
        // reset this field to restart the next iteration
        game[line][column] = 0;
    }

    return 0;
}

void print_game(byte g[][COL])
{
    byte i, j;

    for (i = 0; i < LIN; ++i) {

        // put the line separator after other three lines
        if (i % 3 == 0)
            printf("+-------+-------+-------+\n");

        for (j = 0; j < COL; ++j) {

            // put column separator after other three cols
            if (j % 3 == 0)
                printf("| ");

            if (ngame[i][j] == 0)
                printf("  ");
            else
                printf("%d ", g[i][j]);

        }

        // close the last column
        printf("|\n");
    }

    // close the last line
    printf("+-------+-------+-------+\n");
}


int main(int argc, char *argv[])
{
    // set the matrix to 0
    memset(&game, 0, sizeof(byte) * LIN * COL);

    // create a dummy game
    //dummy_game(7, 0, 0);

    printf("SUDOKU GAME:\n");
    print_game(ngame);

    resolve_game(0, 0);

    byte l, c;
    get_empty_cell(ngame, &l, &c);
    if (l != NUM || c != NUM) {
        printf("GAME IMPOSSIBLE TO SOLVE\n");
        return 0;
    }

    printf("GAME SOLVED:\n");
    print_game(ngame);

    return 0;
}

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "pencil.h"

#define ERROR   0
#define INITIAL 1

#define N_STATES 14
#define N_CHARS  26

int trans_table[N_STATES][N_CHARS] = {{ ERROR }};
command_t accept_states[N_STATES] = { NONE };

#define GET_TRANS(ind, chr) trans_table[(ind)][(chr) - 'a']

/*
 * This function is called before anything else, to initialize the
 * state machine. It is certainly possible to create implementations
 * which don't require any initialization, so just leave this blank if
 * you don't need it.
 */
void init_transtab(void)
{
    GET_TRANS(1, 't') = 2;
    GET_TRANS(2, 'u') = 3;
    GET_TRANS(3, 'r') = 4;
    GET_TRANS(4, 'n') = 5;

    GET_TRANS(1, 'm') = 6;
    GET_TRANS(6, 'o') = 7;
    GET_TRANS(7, 'v') = 8;
    GET_TRANS(8, 'e') = 9;
    
    GET_TRANS(1,  'd') = 10;
    GET_TRANS(10, 'r') = 11;
    GET_TRANS(11, 'a') = 12;
    GET_TRANS(12, 'w') = 13;

    accept_states[5] = TURN;
    accept_states[9] = MOVE;
    accept_states[13] = DRAW;
}


/*
 * Return the next token from reading the given input stream.
 * The words to be recognized are 'turn', 'draw' and 'move',
 * while the returned tokens may be TURN, DRAW, MOVE or END (as
 * enumerated in 'pencil.h').
 */
command_t next(FILE *input)
{
    int state = INITIAL, old_state;
    char ch;

    do
    {
        old_state = state;
        
        if ((ch = fgetc(input)) == EOF)
            return END;

        if (isalpha(ch))
        {
            ch = tolower(ch);
            state = GET_TRANS(state, ch);
        }
        else
            break;

    } while (state != ERROR);

    return accept_states[old_state];
}

#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "moves.h"

struct move initComputer()
{
    srand(time(NULL));
}

struct move getComputerMove(struct board *b, struct moveList *ml)
{
    int index = rand() % ml->n_moves;
    return ml->moves[index];
}

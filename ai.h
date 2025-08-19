#ifndef AI_H
#define AI_H

#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"
#include "moves.h"

#define MAX_DEPTH 4

int eval_counter = 0;

void initComputer()
{
    srand(time(NULL));
}

int evaluate(struct board *b)
{
    int total = 0;
    int color = b->white_to_move ? WHITE : BLACK;

    for (int i = 0; i < 64; i++)
    {
        int piece = b->pieces[i];

        int value = 0;
        switch (piece & PIECE_TYPE)
        {
            case KING:
                value = 1000000;
                break;

            // From L. Kaufman,
            // via https://www.chessprogramming.org/Point_Value
            case QUEEN:
                value = 1000;
                break;
            case ROOK:
                value = 525;
                break;
            case KNIGHT:
                value = 350;
                break;
            case BISHOP:
                value = 350;
                break;
            case PAWN:
                value = 100;
                break;
        }

        if ((piece & PIECE_COLOR) == color) { total += value; }
        else { total -= value; }
    }

    eval_counter++;
    return total;
}

int runSearch(struct board *b, int depth, struct move *best_move)
{
    if (depth == 0)
    {
        return evaluate(b);
    }

    struct moveList ml_instance;
    struct moveList *ml = &ml_instance;
    init_movelist(ml);
    genAllMoves(b, ml);

    if (ml->n_moves == 0)
    {
        // Note: it has to be -INT_MAX, not INT_MIN,
        // because -INT_MIN is undefined behavior!
        return isKingInCheck(b) ? -INT_MAX : 0;
    }

    int best_score = INT_MIN;
    int best_index = -1;

    /*
    // Do a basic shuffle by repeatedly swapping moves around.
    for (int i_move = 0; i_move < ml->n_moves; i_move++)
    {
        int rand_index = rand() % ml->n_moves;

        struct move m = ml->moves[i_move];
        ml->moves[i_move] = ml->moves[rand_index];
        ml->moves[rand_index] = m;
    }
    */

    for (int i_move = 0; i_move < ml->n_moves; i_move++)
    {
        struct move m = ml->moves[i_move];
        struct board b2 = *b;

        applyMove(&b2, m);

        int score = -runSearch(&b2, depth-1, NULL);

        if ((score > best_score) || (best_index < 0))
        {
            best_score = score;
            best_index = i_move;
        }
    }

    // If this is the top level, provide the move itself, not just the score
    if (best_move != NULL)
    {
        *best_move = ml->moves[best_index];
    }

    return best_score;
}

struct move getComputerMove(struct board *b)
{
    eval_counter = 0;
    struct move m;
    runSearch(b, MAX_DEPTH, &m);
    return m;
}

#endif // AI_H

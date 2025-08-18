#ifndef AI_H
#define AI_H

#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "board.h"
#include "moves.h"

#define MAX_DEPTH 1

void initComputer()
{
    srand(time(NULL));
}

int evaluate(struct board *b, int color)
{
    int total = 0;
    int enemy_color = (color == WHITE) ? BLACK : WHITE;

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

    return total;
}

struct moveEvaluation
{
    struct move move;
    int score;
};

struct moveEvaluation runSearch(struct board *b, int depth)
{
    struct moveList ml_instance;
    struct moveList *ml = &ml_instance;
    init_movelist(ml);
    genAllMoves(b, ml);

    int best_score = INT_MIN;
    int best_index = -1;

    // Do a basic shuffle by repeatedly swapping moves around.
    for (int i_move = 0; i_move < ml->n_moves; i_move++)
    {
        int rand_index = rand() % ml->n_moves;

        struct move m = ml->moves[i_move];
        ml->moves[i_move] = ml->moves[rand_index];
        ml->moves[rand_index] = m;
    }

    // Sort moves by a 1-ply evaluation.
    // TODO: 2-ply and beyond
    for (int i_move = 0; i_move < ml->n_moves; i_move++)
    {
        struct move m = ml->moves[i_move];
        struct board b2 = *b;

        applyMove(&b2, m);

        int score;
        if (depth > 0)
        {
            score = -runSearch(&b2, depth-1).score;
        }
        else
        {
            score = evaluate(&b2, (b->white_to_move ? WHITE : BLACK));
        }

        /*
        if (depth == 0)
            printf("  ");

        printf("%d: (%c%c -> %c%c) - score %d\n",
                i_move,
                m.from.file + 'a',
                m.from.rank + '1',
                m.to.file + 'a',
                m.to.rank + '1',
                score);
        */

        if ((score > best_score) || (best_index < 0))
        {
            best_score = score;
            best_index = i_move;
        }
    }

    struct moveEvaluation eval = {
        .move = ml->moves[best_index],
        .score = best_score
    };

    return eval;
}

struct move getComputerMove(struct board *b, struct moveList *ml)
{
    sleep(2);
    return runSearch(b, MAX_DEPTH).move;
}

#endif // AI_H

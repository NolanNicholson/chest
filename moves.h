#ifndef MOVES_H
#define MOVES_H

#include "board.h"

struct move
{
    struct coord from;
    struct coord to;
    bool isCapture;
};

// Max possible moves that could be made from one position.
//https://chess.stackexchange.com/questions/4490/maximum-possible-movement-in-a-turn
#define MAX_MOVES 218

struct moveList
{
    struct move moves[MAX_MOVES];
    int n_moves;
};

void initMoveList(struct moveList *list)
{
    memset(list, 0, sizeof(list));
}

void addMove(struct moveList *list, struct move m)
{
    list->moves[list->n_moves++] = m;
}

enum moveType
{
    FREE,
    CAPTURE,
    INVALID
};

enum moveType getMoveType(const struct board *b, int piece, struct coord to, struct moveList *list)
{
    if (to.rank < 0 || to.rank > 7) { return INVALID; }
    if (to.file < 0 || to.file > 7) { return INVALID; }

    int target_piece = get_piece(b, to);

    if ((piece & PIECE_COLOR) == (target_piece & PIECE_COLOR)) { return INVALID; }
    if ((piece & PIECE_COLOR) == NONE) { return FREE; }
    return CAPTURE;
}

bool tryAddMove(const struct board *b, int piece, struct coord from, struct coord to, struct moveList *list)
{
    enum moveType mt = getMoveType(b, piece, to, list);
    if (mt == INVALID) { return false; }
    addMove(list, (struct move) {
            .from = from,
            .to = to,
            .isCapture = (mt == CAPTURE)
            });
}

void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list)
{
    int piece = get_piece(b, from);
    int piece_type = piece & PIECE_TYPE;

    switch (piece_type)
    {
        case KNIGHT:
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank + 1, .file = from.file + 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank + 1, .file = from.file - 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank - 1, .file = from.file + 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank - 1, .file = from.file - 2 }, list);

            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank + 2, .file = from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank + 2, .file = from.file - 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank - 2, .file = from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { .rank = from.rank - 2, .file = from.file - 1 }, list);

            break;
    }
}

#endif // MOVES_H

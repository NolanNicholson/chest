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
    if ((target_piece & PIECE_COLOR) == NONE) { return FREE; }
    return CAPTURE;
}

enum moveType tryAddMove(const struct board *b, int piece, struct coord from, struct coord to, struct moveList *list)
{
    enum moveType mt = getMoveType(b, piece, to, list);

    if (mt == INVALID) { return mt; }

    addMove(list, (struct move) {
            .from = from,
            .to = to,
            .isCapture = (mt == CAPTURE)
            });

    return mt;
}

void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list)
{
    int piece = get_piece(b, from);
    int piece_color = piece & PIECE_COLOR;
    int piece_type = piece & PIECE_TYPE;

    bool slide_ortho = false;
    bool slide_diag = false;

    switch (piece_type)
    {
        case KING:
            // TODO: castling
            tryAddMove(b, piece, from, (struct coord) { from.rank,     from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank,     from.file - 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file     }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file - 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file     }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file - 1 }, list);

            return;

        case KNIGHT:
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file + 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file - 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file + 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file - 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 2, from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 2, from.file - 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 2, from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 2, from.file - 1 }, list);

            return;

        case PAWN:
            // TODO: ordinary captures
            // TODO: en passant
            int dir = (piece_color == WHITE) ? 1 : -1;
            int startRank = (piece_color == WHITE) ? 1 : 6;
            tryAddMove(b, piece, from, (struct coord) { from.rank + dir, from.file }, list);
            if (from.rank == startRank)
            {
                tryAddMove(b, piece, from, (struct coord) { from.rank + 2*dir, from.file }, list);
            }
            return;

        case ROOK:
            slide_ortho = true;
            break;

        case BISHOP:
            slide_diag = true;
            break;

        case QUEEN:
            slide_ortho = true;
            slide_diag = true;
            break;
    }

    struct coord to;

    if (slide_ortho)
    {
        for (to.rank = from.rank, to.file = from.file + 1;
                to.file <= 7; to.file++)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }

        for (to.rank = from.rank, to.file = from.file - 1;
                to.file >= 0; to.file--)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }

        for (to.rank = from.rank + 1, to.file = from.file;
                to.rank <= 7; to.rank++)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }

        for (to.rank = from.rank - 1, to.file = from.file;
                to.rank >= 0; to.rank--)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }
    }

    if (slide_diag)
    {
        for (to.rank = from.rank + 1, to.file = from.file + 1;
                to.rank <= 7 && to.file <= 7;
                to.rank++, to.file++)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }

        for (to.rank = from.rank + 1, to.file = from.file - 1;
                to.rank <= 7 && to.file >= 0;
                to.rank++, to.file--)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }

        for (to.rank = from.rank - 1, to.file = from.file + 1;
                to.rank >= 0 && to.file <= 7;
                to.rank--, to.file++)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }

        for (to.rank = from.rank - 1, to.file = from.file - 1;
                to.rank >= 0 && to.file >= 0;
                to.rank--, to.file--)
        {
            if (tryAddMove(b, piece, from, to, list) != FREE) { break; }
        }
    }
}

#endif // MOVES_H

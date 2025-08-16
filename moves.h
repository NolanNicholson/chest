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

void init_movelist(struct moveList *list)
{
    list->n_moves = 0;
}

void genAllPseudoLegalMoves(struct board *b, int piece_color, struct moveList *list);
void genAllMoves(struct board *b, int piece_color, struct moveList *list);
void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list);
void genPseudoLegalMovesForPiece(const struct board *b, struct coord from, struct moveList *list);


void applyMove(struct board *b, struct move m)
{
    int piece = get_piece(b, m.from);
    set_piece(b, m.to, piece);
    set_piece(b, m.from, NONE);
}

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

enum moveType getMoveType(const struct board *b, int piece, struct move m, struct moveList *list)
{
    struct coord to = m.to;
    if (to.rank < 0 || to.rank > 7) { return INVALID; }
    if (to.file < 0 || to.file > 7) { return INVALID; }

    int friendly_color = piece & PIECE_COLOR;

    // You can't move nothing.
    if (friendly_color == NONE) { return INVALID; }
    int target_piece = get_piece(b, to);

    // You can't capture a piece of your own color.
    if (friendly_color == (target_piece & PIECE_COLOR)) { return INVALID; }

    if ((target_piece & PIECE_COLOR) == NONE) { return FREE; }
    return CAPTURE;
}

enum moveType tryAddMove(const struct board *b, int piece, struct coord from, struct coord to, struct moveList *list)
{
    struct move m = {.from = from, .to = to};
    enum moveType mt = getMoveType(b, piece, m, list);

    if (mt == INVALID) { return mt; }

    addMove(list, (struct move) {
            .from = from,
            .to = to,
            .isCapture = (mt == CAPTURE)
            });

    return mt;
}

void genPseudoLegalMovesForPiece(const struct board *b, struct coord from, struct moveList *list)
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

void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list)
{
    struct moveList listPseudoLegal;
    init_movelist(&listPseudoLegal);

    genPseudoLegalMovesForPiece(b, from, &listPseudoLegal);

    int friendly_color = get_piece(b, from) & PIECE_COLOR;
    int enemy_color = (friendly_color == WHITE) ? BLACK : WHITE;

    for (int i_move = 0; i_move < listPseudoLegal.n_moves; i_move++)
    {
        bool ok = true;

        // Moves which leave the moving piece in check should be disallowed.
        // We do this by making a move on the copy of the board, generating
        // all moves for the opponent, and seeing if any target our king.
        struct board b2 = *b;
        applyMove(&b2, listPseudoLegal.moves[i_move]);

        struct moveList list_opponent;
        init_movelist(&list_opponent);

        genAllPseudoLegalMoves(&b2, enemy_color, &list_opponent);
        for (int i_response = 0; i_response < list_opponent.n_moves; i_response++)
        {
            struct coord to = list_opponent.moves[i_response].to;
            if (get_piece(&b2, to) == (friendly_color | KING))
            {
                ok = false;
                break;
            }
        }

        if (ok)
        {
            addMove(list, listPseudoLegal.moves[i_move]);
        }
    }
}

void genAllPseudoLegalMoves(struct board *b, int piece_color, struct moveList *list)
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            struct coord at = {rank, file};
            if ((get_piece(b, at) & PIECE_COLOR) == piece_color)
            {
                genPseudoLegalMovesForPiece(b, at, list);
            }
        }
    }
}

void genAllMoves(struct board *b, int piece_color, struct moveList *list)
{
    int pieces_examined = 0;

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            struct coord at = {rank, file};
            if ((get_piece(b, at) & PIECE_COLOR) == piece_color)
            {
                genMovesForPiece(b, at, list);
            }
        }
    }
}

#endif // MOVES_H

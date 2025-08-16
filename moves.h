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

bool putsKingInCheck(const struct board *b, struct move m);
void genAllPseudoLegalMoves(struct board *b, struct moveList *list);
void genAllMoves(struct board *b, struct moveList *list);
void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list);
void genPseudoLegalMovesForPiece(const struct board *b, struct coord from, struct moveList *list);
bool isMoveLegal(const struct board *b, struct move m);


void applyMove(struct board *b, struct move m)
{
    int piece = get_piece(b, m.from);
    int target_piece = get_piece(b, m.to);

    set_piece(b, m.from, NONE);
    set_piece(b, m.to, piece);

    // Handle castling
    switch (piece)
    {
        // Moving a king means you can no longer castle in either direction.
        case WHITE | KING:
            if (m.to.file - m.from.file == 2) // Move rook for kingside castle
            {
                set_piece(b, (struct coord) {0, 5}, WHITE | ROOK);
                set_piece(b, (struct coord) {0, 7}, NONE);
            }
            else if (m.to.file - m.from.file == -2) // Move rook for queenside castle
            {
                set_piece(b, (struct coord) {0, 3}, WHITE | ROOK);
                set_piece(b, (struct coord) {0, 0}, NONE);
            }

            b->castles_available &= ~(CASTLE_WK | CASTLE_WQ);
            break;

        case BLACK | KING:
            if (m.to.file - m.from.file == 2) // Move rook for kingside castle
            {
                set_piece(b, (struct coord) {7, 5}, BLACK | ROOK);
                set_piece(b, (struct coord) {7, 7}, NONE);
            }
            else if (m.to.file - m.from.file == -2) // Move rook for queenside castle
            {
                set_piece(b, (struct coord) {7, 3}, BLACK | ROOK);
                set_piece(b, (struct coord) {7, 0}, NONE);
            }

            b->castles_available &= ~(CASTLE_BK | CASTLE_BQ);
            break;

        // Moving a rook means you can no longer castle in that rook's direction.
        case WHITE | ROOK:
            b->castles_available &= ~(m.from.file == 0 ? CASTLE_WQ : CASTLE_WK);
            break;

        case BLACK | ROOK:
            b->castles_available &= ~(m.from.file == 0 ? CASTLE_BQ : CASTLE_BK);
            break;
    }

    // Capturing a rook means the opponent can't castle on that side anymore.
    switch (target_piece)
    {
        case WHITE | ROOK:
            b->castles_available &= ~(m.to.file == 0 ? CASTLE_WQ : CASTLE_WK);
            break;

        case BLACK | ROOK:
            b->castles_available &= ~(m.to.file == 0 ? CASTLE_BQ : CASTLE_BK);
            break;
    }

    b->white_to_move ^= 1;
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
    bool white = (friendly_color & WHITE) > 0;

    // You can't move nothing.
    if (friendly_color == NONE) { return INVALID; }
    int target_piece = get_piece(b, to);

    // You can't capture a piece of your own color.
    if (friendly_color == (target_piece & PIECE_COLOR)) { return INVALID; }

    // All squares between the king and the rook must be vacant.
    if ((piece & PIECE_TYPE) == KING)
    {
        struct coord inbetween = { .rank = m.from.rank };

        // Kingside castle
        if (m.to.file - m.from.file == 2)
        {
            int avail_flag = white ? CASTLE_WK : CASTLE_BK;
            if (!(b->castles_available & avail_flag)) { return INVALID; }

            for (inbetween.file = m.from.file + 1; inbetween.file < 7; inbetween.file++)
            {
                if (get_piece(b, inbetween) != NONE) { return INVALID; }
            }
        }
        // Queenside castle
        else if (m.to.file - m.from.file == -2)
        {
            int avail_flag = white ? CASTLE_WQ : CASTLE_BQ;
            if (!(b->castles_available & avail_flag)) { return INVALID; }

            for (inbetween.file = m.from.file - 1; inbetween.file > 0; inbetween.file--)
            {
                if (get_piece(b, inbetween) != NONE) { return INVALID; }
            }
        }
    }

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

enum moveType tryAddMoveRestricted(const struct board *b, int piece, struct coord from, struct coord to, struct moveList *list, enum moveType requiredType)
{
    struct move m = {.from = from, .to = to};
    enum moveType mt = getMoveType(b, piece, m, list);

    if (mt != requiredType) { return mt; }

    addMove(list, (struct move) {
            .from = from,
            .to = to,
            .isCapture = (mt == CAPTURE)
            });
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
            // Regular moves
            tryAddMove(b, piece, from, (struct coord) { from.rank,     from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank,     from.file - 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file     }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank + 1, from.file - 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file + 1 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file     }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank - 1, from.file - 1 }, list);

            // Castling moves
            tryAddMove(b, piece, from, (struct coord) { from.rank,     from.file + 2 }, list);
            tryAddMove(b, piece, from, (struct coord) { from.rank,     from.file - 2 }, list);

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
            // TODO: en passant
            // TODO: promotions
            bool white = piece_color == WHITE;
            int startRank = white ? 1 : 6;
            int dir = white ? 1 : -1;
            int singlePushRank = from.rank + dir;
            int doublePushRank = singlePushRank + dir;

            // Straight forward single pushes cannot be captures
            tryAddMoveRestricted(b, piece, from, (struct coord) { singlePushRank, from.file }, list, FREE);

            // Diagonal forward pushes must be captures
            tryAddMoveRestricted(b, piece, from, (struct coord) { singlePushRank, from.file-1 }, list, CAPTURE);
            tryAddMoveRestricted(b, piece, from, (struct coord) { singlePushRank, from.file+1 }, list, CAPTURE);

            if (from.rank == startRank)
            {
                tryAddMoveRestricted(b, piece, from, (struct coord) { doublePushRank, from.file }, list, FREE);
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

bool putsKingInCheck(const struct board *b, struct move m)
{
    // Moves which leave the king vulnerable to capture are not allowed.
    // We handle this by making our move on the copy of the board, generating
    // all moves for the opponent, and seeing if any target our king.

    int friendly_color = get_piece(b, m.from) & PIECE_COLOR;

    struct board b2 = *b;
    applyMove(&b2, m);

    struct moveList list_opponent;
    init_movelist(&list_opponent);
    genAllPseudoLegalMoves(&b2, &list_opponent);

    for (int i_response = 0; i_response < list_opponent.n_moves; i_response++)
    {
        struct coord to = list_opponent.moves[i_response].to;
        if (get_piece(&b2, to) == (friendly_color | KING))
        {
            return true;
        }
    }

    return false;
}

void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list)
{
    int piece = get_piece(b, from);

    struct moveList listPseudoLegal;
    init_movelist(&listPseudoLegal);
    genPseudoLegalMovesForPiece(b, from, &listPseudoLegal);

    int friendly_color = piece & PIECE_COLOR;
    int enemy_color = (friendly_color == WHITE) ? BLACK : WHITE;

    for (int i_move = 0; i_move < listPseudoLegal.n_moves; i_move++)
    {
        struct move m = listPseudoLegal.moves[i_move];
        if (isMoveLegal(b, m))
        {
            addMove(list, m);
        }
    }
}

bool isMoveLegal(const struct board *b, struct move m)
{
    int piece = get_piece(b, m.from);
    if ((piece & PIECE_TYPE) == KING)
    {
        struct move tentative = { .from = m.from, .to.rank = m.from.rank };

        // King must not leave, cross over, or arrive at an attacked square.
        if ((m.to.file - m.from.file) == 2)
        {
            for (tentative.to.file = m.from.file; tentative.to.file < 7; tentative.to.file++)
            {
                if (putsKingInCheck(b, tentative)) { return false; }
            }
        }
        else if ((m.to.file - m.from.file) == -2)
        {
            for (tentative.to.file = m.from.file; tentative.to.file > 1; tentative.to.file--)
            {
                if (putsKingInCheck(b, tentative)) { return false; }
            }
        }
    }

    return !putsKingInCheck(b, m);
}


void genAllPseudoLegalMoves(struct board *b, struct moveList *list)
{
    int piece_color = b->white_to_move ? WHITE : BLACK;

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

void genAllMoves(struct board *b, struct moveList *list)
{
    int piece_color = b->white_to_move ? WHITE : BLACK;
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

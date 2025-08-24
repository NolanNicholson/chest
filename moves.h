#ifndef MOVES_H
#define MOVES_H

#include "board.h"

struct move
{
    struct coord from;
    struct coord to;
    int promotion;
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

bool isKingInCheck(const struct board *b);
bool leavesKingInDanger(const struct board *b, struct move m);
void genAllPseudoLegalMoves(const struct board *b, struct moveList *list);
void genAllMoves(const struct board *b, struct moveList *list);
void genMovesForPiece(const struct board *b, struct coord from, struct moveList *list);
void genPseudoLegalMovesForPiece(const struct board *b, struct coord from, struct moveList *list);
bool isMoveLegal(const struct board *b, struct move m);


void applyMove(struct board *b, struct move m)
{
    int piece = get_piece(b, m.from);
    int target_piece = get_piece(b, m.to);
    bool is_ep_capture = (b->ep_target.rank == m.to.rank && b->ep_target.file == m.to.file);

    set_piece(b, m.from, NONE);
    set_piece(b, m.to, piece);
    b->ep_target.file = m.to.file;
    b->ep_target.rank = -1;

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

        case WHITE | PAWN:
            // Apply en-passant capture
            if (is_ep_capture)
            {
                set_piece(b, (struct coord) { 4, m.to.file }, NONE);
            }
            // Set up future en-passant flag
            if (m.from.rank == 1 && m.to.rank == 3)
            {
                b->ep_target.rank = 2;
            }
            break;

        case BLACK | PAWN:
            // Apply en-passant capture
            if (is_ep_capture)
            {
                set_piece(b, (struct coord) { 3, m.to.file }, NONE);
            }
            // Set up future en-passant flag
            if (m.from.rank == 6 && m.to.rank == 4)
            {
                b->ep_target.rank = 5;
            }
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

    if (m.promotion != NONE)
    {
        set_piece(b, m.to, (piece & PIECE_COLOR) | m.promotion);
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
    int piece_type = piece & PIECE_TYPE;
    switch (piece_type)
    {
        case KING:
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

            break;

        case PAWN:
            // En passant is a valid capture even though the target square contains no piece
            if (m.to.rank == b->ep_target.rank && m.to.file == b->ep_target.file)
            {
                return CAPTURE;
            }

            break;
    }

    if ((target_piece & PIECE_COLOR) == NONE) { return FREE; }
    return CAPTURE;
}

/*
 * Add a move - unless it's a pawn promotion, in which case add all the possible promotions.
 */
void addMoveMaybePawnPromo(int piece, struct moveList *list, struct move m)
{
    bool white = (piece & PIECE_COLOR) == WHITE;

    if ((piece & PIECE_TYPE) == PAWN && m.promotion == NONE && m.to.rank == (white ? 7 : 0))
    {
        m.promotion = QUEEN;
        addMove(list, m);
        m.promotion = BISHOP;
        addMove(list, m);
        m.promotion = KNIGHT;
        addMove(list, m);
        m.promotion = ROOK;
        addMove(list, m);
    }
    else
    {
        addMove(list, m);
    }
}

enum moveType tryAddMove(const struct board *b, int piece, struct coord from, struct coord to, struct moveList *list)
{
    struct move m = {.from = from, .to = to, .promotion = NONE};
    enum moveType mt = getMoveType(b, piece, m, list);
    m.isCapture = (mt == CAPTURE);

    if (mt != INVALID)
    {
        addMoveMaybePawnPromo(piece, list, m);
    }

    return mt;
}

enum moveType tryAddMoveRestricted(const struct board *b, int piece, struct coord from, struct coord to, struct moveList *list, enum moveType requiredType)
{
    struct move m = {.from = from, .to = to, .promotion = NONE};
    enum moveType mt = getMoveType(b, piece, m, list);
    m.isCapture = (mt == CAPTURE);

    if (mt == requiredType)
    {
        addMoveMaybePawnPromo(piece, list, m);
    }

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
            bool white = piece_color == WHITE;
            int startRank = white ? 1 : 6;
            int dir = white ? 1 : -1;
            int singlePushRank = from.rank + dir;
            int doublePushRank = singlePushRank + dir;

            // Straight forward single pushes cannot be captures
            enum moveType singlePush =
                tryAddMoveRestricted(b, piece, from, (struct coord) { singlePushRank, from.file }, list, FREE);

            // Diagonal forward pushes must be captures
            tryAddMoveRestricted(b, piece, from, (struct coord) { singlePushRank, from.file-1 }, list, CAPTURE);
            tryAddMoveRestricted(b, piece, from, (struct coord) { singlePushRank, from.file+1 }, list, CAPTURE);

            // Double push
            if (singlePush == FREE && from.rank == startRank)
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

bool canNextMoveDestroyKing(const struct board *b)
{
    int attacker_color = b->white_to_move ? WHITE : BLACK;
    int target_color = b->white_to_move ? BLACK : WHITE;

    // Find the king on the board.
    struct coord king_at;
    bool found_king = false;
    for (king_at.rank = 0; king_at.rank < 8; king_at.rank++)
    {
        for (king_at.file = 0; king_at.file < 8; king_at.file++)
        {
            if (get_piece(b, king_at) == (target_color | KING))
            {
                found_king = true;
                break;
            }
        }

        if (found_king) { break; }
    }

    // Check for pawns. (Don't check straight ahead!)
    int offset_rank_pawn = b->white_to_move ? -1 : 1;
    for (int offset_file = -1; offset_file <= 1; offset_file += 2)
    {
        struct coord at = king_at;
        at.rank += offset_rank_pawn;
        at.file += offset_file;

        if (at.rank < 0) { continue; }
        if (at.rank > 7) { continue; }
        if (at.file < 0) { continue; }
        if (at.file > 7) { continue; }

        int this_piece = get_piece(b, at);
        if (this_piece == (attacker_color | PAWN)) {
            return true;
        }
    }

    // Check for knights.
    const struct coord knight_offsets[] = {
        { -1, -2 }, { 1, -2 },
        { -1, 2 }, { 1, 2 },
        { -2, -1 }, { 2, -1 },
        { -2, 1 }, { 2, 1 },
    };

    for (int i_offset = 0; i_offset < 8; i_offset++)
    {
        struct coord at = king_at;
        struct coord offset = knight_offsets[i_offset];

        at.rank += offset.rank;
        at.file += offset.file;

        if (at.rank < 0) { continue; }
        if (at.rank > 7) { continue; }
        if (at.file < 0) { continue; }
        if (at.file > 7) { continue; }

        int this_piece = get_piece(b, at);
        if (this_piece == (attacker_color | KNIGHT)) { return true; }
    }

    // Here, we sort of scan the board as if the king were a queen. If it
    // encounters a piece that could capture it in that direction, then it is
    // endangered. For example, if we set out from the king diagonally, and
    // encounter an enemy bishop before any other piece in that direction, then
    // the king is endangered.

    const struct coord sliding_offsets[] = {
        { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 },
        { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 }
    };

    for (int i_offset = 0; i_offset < 8; i_offset++)
    {
        struct coord at = king_at;
        struct coord offset = sliding_offsets[i_offset];
        bool diagonal = ((offset.rank + offset.file + 2) % 2) == 0;
        int steps = 0;
        bool finished_slide = false;

        while (true)
        {
            at.rank += offset.rank;
            at.file += offset.file;
            steps++;

            if (at.rank < 0) { break; }
            if (at.rank > 7) { break; }
            if (at.file < 0) { break; }
            if (at.file > 7) { break; }

            int this_piece = get_piece(b, at);
            int this_color = this_piece & PIECE_COLOR;
            int this_type = this_piece & PIECE_TYPE;

            if (this_color == target_color) { break; }
            if (this_color == NONE) { continue; }

            switch (this_type)
            {
                case QUEEN:
                    return true;

                case BISHOP:
                    if (diagonal) { return true; }
                    else { finished_slide = true; break; }

                case ROOK:
                    if (!diagonal) { return true; }
                    else { finished_slide = true; break; }

                case KING:
                    if (steps == 1) { return true; }
                    else { finished_slide = true; break; }

                default: finished_slide = true; break;
            }

            if (finished_slide) { break; }
        }
    }

    return false;
}

bool isKingInCheck(const struct board *b)
{
    // To check whether the king is ~currently~ in check,
    // we flip control of (a copy of) the board without making a move.
    // Then, we see if any of the responses can destroy the king.
    struct board b2 = *b;
    b2.white_to_move ^= 1;

    return canNextMoveDestroyKing(&b2);
}

bool leavesKingInDanger(const struct board *b, struct move m)
{
    struct board b2 = *b;
    applyMove(&b2, m);

    return canNextMoveDestroyKing(&b2);
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
                if (leavesKingInDanger(b, tentative)) { return false; }
            }
        }
        else if ((m.to.file - m.from.file) == -2)
        {
            for (tentative.to.file = m.from.file; tentative.to.file > 1; tentative.to.file--)
            {
                if (leavesKingInDanger(b, tentative)) { return false; }
            }
        }
    }

    return !leavesKingInDanger(b, m);
}


void genAllPseudoLegalMoves(const struct board *b, struct moveList *list)
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

void genAllMoves(const struct board *b, struct moveList *list)
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

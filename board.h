#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define NONE        0x00

#define PAWN        0x01
#define BISHOP      0x02
#define KNIGHT      0x03
#define ROOK        0x04
#define QUEEN       0x05
#define KING        0x06
#define PIECE_TYPE  0x0f

#define BLACK       0x10
#define WHITE       0x20
#define PIECE_COLOR 0x30

#define CASTLE_WK   0x01
#define CASTLE_WQ   0x02
#define CASTLE_BK   0x04
#define CASTLE_BQ   0x08

struct coord
{
    int rank;
    int file;
};

struct board
{
    int pieces[64];
    bool white_to_move;
    int castles_available;
    struct coord ep_target;
};

int get_piece(const struct board *b, struct coord at)
{
    return b->pieces[at.rank*8+at.file];
}

void set_piece(struct board *b, struct coord at, int piece)
{
    b->pieces[at.rank*8 + at.file] = piece;
}

void apply_FEN(struct board *b, const char *fen)
{
    // piece placement
    //                                             side to move
    //                                               castling ability
    //                                                    en passant target
    //                                                      halfmove clock
    //                                                        fullmove counter
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define APPLY_FEN_STATE_PIECE_PLACEMENT 0
#define APPLY_FEN_STATE_SIDE_TO_MOVE 1
#define APPLY_FEN_STATE_CASTLING 2
#define APPLY_FEN_STATE_EN_PASSANT 3
#define APPLY_FEN_STATE_HALFMOVE 4
#define APPLY_FEN_STATE_FULLMOVE 5

    char c;
    int state = APPLY_FEN_STATE_PIECE_PLACEMENT;
    int rank = 7;
    int file = 0;

    for (int i = 0; c = fen[i]; i++)
    {
        if (c == ' ')
        {
            state++;
            continue;
        }

        switch(state)
        {
            case APPLY_FEN_STATE_PIECE_PLACEMENT:
                struct coord here = {.rank = rank, .file = file };
                switch(c)
                {
                    // slash separates rank
                    case '/':
                        rank--;
                        file = 0;
                        break;

                    // digit counts consecutive empty squares
                    case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8':
                        file += (c - '0');
                        break;

                    case 'p': set_piece(b, here, BLACK | PAWN); file++; break;
                    case 'n': set_piece(b, here, BLACK | KNIGHT); file++; break;
                    case 'b': set_piece(b, here, BLACK | BISHOP); file++; break;
                    case 'r': set_piece(b, here, BLACK | ROOK); file++; break;
                    case 'q': set_piece(b, here, BLACK | QUEEN); file++; break;
                    case 'k': set_piece(b, here, BLACK | KING); file++; break;

                    case 'P': set_piece(b, here, WHITE | PAWN); file++; break;
                    case 'N': set_piece(b, here, WHITE | KNIGHT); file++; break;
                    case 'B': set_piece(b, here, WHITE | BISHOP); file++; break;
                    case 'R': set_piece(b, here, WHITE | ROOK); file++; break;
                    case 'Q': set_piece(b, here, WHITE | QUEEN); file++; break;
                    case 'K': set_piece(b, here, WHITE | KING); file++; break;
                }

                break;

            case APPLY_FEN_STATE_SIDE_TO_MOVE:
                // this can only ever be 'w' or 'b'
                b->white_to_move = (c == 'w');
                break;

            case APPLY_FEN_STATE_CASTLING:
                switch (c)
                {
                    case 'K': b->castles_available |= CASTLE_WK; break;
                    case 'Q': b->castles_available |= CASTLE_WQ; break;
                    case 'k': b->castles_available |= CASTLE_BK; break;
                    case 'q': b->castles_available |= CASTLE_BQ; break;
                }
                break;

            case APPLY_FEN_STATE_EN_PASSANT:
                if (c >= '1' && c <= '8')
                {
                    b->ep_target.rank = c - '1';
                }
                else if (c >= 'a' && c <= 'h')
                {
                    b->ep_target.file = c - 'a';
                }
                break;

            case APPLY_FEN_STATE_HALFMOVE:
                // TODO: Apply FEN half-move info
                break;

            case APPLY_FEN_STATE_FULLMOVE:
                // TODO: Apply FEN full-move info
                break;
        }
    }
}

void init_board(struct board *b)
{
    memset(b->pieces, 0, sizeof(b->pieces));
    b->white_to_move = true;
    b->castles_available = 0;
    b->ep_target.rank = -1;
    b->ep_target.file = -1;
}



#endif //BOARD_H

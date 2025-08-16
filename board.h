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

struct board
{
    int pieces[64];
    bool white_to_move;
};

struct coord
{
    int rank;
    int file;
};

int get_piece(const struct board *b, struct coord at)
{
    return b->pieces[at.rank*8+at.file];
}

void set_piece(struct board *b, struct coord at, int piece)
{
    b->pieces[at.rank*8 + at.file] = piece;
}

bool move_piece(struct board *b, struct coord from, struct coord to)
{
    int i_from = from.rank * 8 + from.file;
    int piece = b->pieces[i_from];

    // You can't move nothing.
    if (piece & PIECE_COLOR == NONE || piece & PIECE_TYPE == NONE)
    {
        return false;
    }

    // A piece cannot move into another piece of the same color.
    int dest_piece = get_piece(b, to);
    if (piece & PIECE_COLOR == dest_piece & PIECE_COLOR)
    {
        return false;
    }

    switch (piece & PIECE_TYPE)
    {
        case KNIGHT:
            int diff_rank = abs(from.rank - to.rank);
            int diff_file = abs(from.file - to.file);
            if (!((diff_rank == 1 && diff_file == 2) || (diff_rank == 2 && diff_file == 1)))
            {
                return false;
            }

            break;

        case ROOK:
            if (from.rank == to.rank)
            {
                for (int file = MIN(from.file, to.file); file < MAX(from.file, to.file); file++)
                {
                    if (get_piece(b, (struct coord){to.rank, file}) & PIECE_COLOR != NONE)
                    {
                        return false;
                    }
                }
            }
            else if (from.file == to.file)
            {
                for (int rank = MIN(from.rank, to.rank); rank < MAX(from.rank, to.rank); rank++)
                {
                    if (get_piece(b, (struct coord){rank, to.file}) & PIECE_COLOR != NONE)
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }

            break;
    }

    // Commit the move.
    b->pieces[i_from] = NONE;
    set_piece(b, to, piece);

    return true;
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
                // TODO
                break;

            case APPLY_FEN_STATE_EN_PASSANT:
                // TODO
                break;

            case APPLY_FEN_STATE_HALFMOVE:
                // TODO
                break;

            case APPLY_FEN_STATE_FULLMOVE:
                // TODO
                break;
        }
    }
}

void init_board(struct board *b)
{
    memset(b->pieces, 0, sizeof(b->pieces));
}

void move_algebraic(struct board *b, const char *move)
{
    int i = 0; // index into the move string
    int ptype;

    bool is_kingside_castle = false;
    bool is_queenside_castle = false;

    switch (move[i])
    {
        // For non-pawns, the first letter denotes the piece type
        case 'K': i++; ptype = KING; break;
        case 'Q': i++; ptype = QUEEN; break;
        case 'R': i++; ptype = ROOK; break;
        case 'B': i++; ptype = BISHOP; break;
        case 'N': i++; ptype = KNIGHT; break;

        // Pawn movements start directly with a rank letter.
        case 'a': case 'b': case 'c': case 'd':
        case 'e': case 'f': case 'g': case 'h':
                  ptype = PAWN; break;

        // 0-0 and O-O denote a kingside castle.
        // 0-0 and O-O-O denote a queenside castle.
        case '0': case 'O':
                  char o = move[i++];
                  if (move[i++] == '-' && move[i++] == o)
                  {
                      // castling!
                      if (move[i++] == '-' && move[i++] == o)
                      {
                          is_queenside_castle = true;
                      }
                      else
                      {
                          is_kingside_castle = true;
                      }
                      break;
                  }
                  else
                  {
                      goto unrecognized_algebraic_move;
                  }

        default:
unrecognized_algebraic_move:
                  fprintf(stderr, "Unrecognized move: %s\n", move);
                  return;
    }

    char file_c = move[i++];
    char rank_c = move[i++];

    printf("Moving piece 0x%x to %c%c\n", ptype, file_c, rank_c);

    int file = file_c - 'a';
    int rank = rank_c - '1';

    move_piece(b, (struct coord){0, 6}, (struct coord){rank, file});
}


#endif //BOARD_H

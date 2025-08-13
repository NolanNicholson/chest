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
#define PIECE_TYPE  0x08

#define BLACK       0x10
#define WHITE       0x20
#define PIECE_COLOR 0x30

struct board
{
    int pieces[64];
    bool white_to_move;
};

int get_piece(const struct board *b, int rank, int file)
{
    return b->pieces[rank*8+file];
}

void set_piece(struct board *b, int rank, int file, int piece)
{
    b->pieces[rank*8 + file] = piece;
}

bool move_piece(struct board *b, int rank_from, int file_from, int rank_to, int file_to)
{
    int i_from = rank_from * 8 + file_from;
    int piece = b->pieces[i_from];

    // You can't move nothing.
    if (piece & PIECE_COLOR == NONE || piece & PIECE_TYPE == NONE)
    {
        return false;
    }

    // A piece cannot display another piece of the same color.
    int dest_piece = get_piece(b, rank_to, file_to);
    if (piece & PIECE_COLOR == dest_piece & PIECE_COLOR)
    {
        return false;
    }

    switch (piece & PIECE_TYPE)
    {
        case KNIGHT:
            int diff_rank = abs(rank_from - rank_to);
            int diff_file = abs(file_from - file_to);
            if (!((diff_rank == 1 && diff_file == 2) || (diff_rank == 2 && diff_file == 1)))
            {
                return false;
            }

            break;

        case ROOK:
            if (rank_from == rank_to)
            {
                for (int file = MIN(file_from, file_to); file < MAX(file_from, file_to); file++)
                {
                    if (get_piece(b, rank_to, file) & PIECE_COLOR != NONE)
                    {
                        return false;
                    }
                }
            }
            else if (file_from == file_to)
            {
                for (int rank = MIN(rank_from, rank_to); rank < MAX(rank_from, rank_to); rank++)
                {
                    if (get_piece(b, rank, file_to) & PIECE_COLOR != NONE)
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
    set_piece(b, rank_to, file_to, piece);

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

                    case 'p': set_piece(b, rank, file, BLACK | ROOK); file++; break;
                    case 'n': set_piece(b, rank, file, BLACK | KNIGHT); file++; break;
                    case 'b': set_piece(b, rank, file, BLACK | BISHOP); file++; break;
                    case 'r': set_piece(b, rank, file, BLACK | ROOK); file++; break;
                    case 'q': set_piece(b, rank, file, BLACK | QUEEN); file++; break;
                    case 'k': set_piece(b, rank, file, BLACK | KING); file++; break;

                    case 'P': set_piece(b, rank, file, WHITE | ROOK); file++; break;
                    case 'N': set_piece(b, rank, file, WHITE | KNIGHT); file++; break;
                    case 'B': set_piece(b, rank, file, WHITE | BISHOP); file++; break;
                    case 'R': set_piece(b, rank, file, WHITE | ROOK); file++; break;
                    case 'Q': set_piece(b, rank, file, WHITE | QUEEN); file++; break;
                    case 'K': set_piece(b, rank, file, WHITE | KING); file++; break;
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
    apply_FEN(b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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

    printf("Moving %d to %c%c\n", ptype, file_c, rank_c);

    int file = file_c - 'a';
    int rank = rank_c - '1';

    move_piece(b, 0, 6, rank, file);
}

#define UTF8_WKING      "\u2654"
#define UTF8_WQUEEN     "\u2655"
#define UTF8_WROOK      "\u2656"
#define UTF8_WBISHOP    "\u2657"
#define UTF8_WKNIGHT    "\u2658"
#define UTF8_WPAWN      "\u2659"
#define UTF8_BKING      "\u265A"
#define UTF8_BQUEEN     "\u265B"
#define UTF8_BROOK      "\u265C"
#define UTF8_BBISHOP    "\u265D"
#define UTF8_BKNIGHT    "\u265E"
#define UTF8_BPAWN      "\u265F"

#define ANSI_FG_BLACK "30"
#define ANSI_BG_YELLOW "43"
#define ANSI_BG_WHITE "47"

#define PRINT_BSQUARE   "\x1b[" ANSI_BG_YELLOW ";" ANSI_FG_BLACK "m"
#define PRINT_WSQUARE   "\x1b[" ANSI_BG_WHITE ";" ANSI_FG_BLACK "m"
#define PRINT_RESET     "\x1b[0m"

void print_board(const struct board *b)
{
    bool white_square;

    printf("  a b c d e f g h\n");

    for (int rank = 7; rank >= 0; rank--)
    {
        printf("%d ", rank + 1);

        for (int file = 0; file < 8; file++) // horizontal; A-H
        {
            white_square = (rank + file) % 2 > 0;

            printf(white_square ? PRINT_WSQUARE : PRINT_BSQUARE);

            int piece = get_piece(b, rank, file);

            switch(piece)
            {
                case WHITE | PAWN: printf(UTF8_WPAWN); break;
                case WHITE | BISHOP: printf(UTF8_WBISHOP); break;
                case WHITE | KNIGHT: printf(UTF8_WKNIGHT); break;
                case WHITE | ROOK: printf(UTF8_WROOK); break;
                case WHITE | QUEEN: printf(UTF8_WQUEEN); break;
                case WHITE | KING: printf(UTF8_WKING); break;

                case BLACK | PAWN: printf(UTF8_BPAWN); break;
                case BLACK | BISHOP: printf(UTF8_BBISHOP); break;
                case BLACK | KNIGHT: printf(UTF8_BKNIGHT); break;
                case BLACK | ROOK: printf(UTF8_BROOK); break;
                case BLACK | QUEEN: printf(UTF8_BQUEEN); break;
                case BLACK | KING: printf(UTF8_BKING); break;

                default:     printf(" "); break;
            }

            putc(' ', stdout);
        }
        printf(PRINT_RESET"\n");
    }
    
    printf("\n%s\n", b->white_to_move ? "White to move" : "Black to move");
}

#endif //BOARD_H

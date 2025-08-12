#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum piece_type
{
    NONE,
    PAWN,
    BISHOP,
    KNIGHT,
    ROOK,
    QUEEN,
    KING
};

enum piece_color
{
    EMPTY,
    BLACK,
    WHITE
};

struct piece
{
    enum piece_type type;
    enum piece_color color;
};

struct board
{
    struct piece pieces[64];
    bool white_to_move;
};

void set_piece_type(struct board *b, int rank, int file, enum piece_type pt)
{
    b->pieces[rank*8+file].type = pt;
}

void set_piece_color(struct board *b, int rank, int file, enum piece_color pc)
{
    b->pieces[rank*8+file].color = pc;
}

struct piece get_piece(const struct board *b, int rank, int file)
{
    return b->pieces[rank*8+file];
}

void place_piece(struct board *b, struct piece p, int rank, int file)
{
    int index = rank*8 + file;
    b->pieces[index] = p;
}

bool move_piece(struct board *b, int rank_from, int file_from, int rank_to, int file_to)
{
    int i_from = rank_from * 8 + file_from;
    struct piece p = b->pieces[i_from];

    // You can't move nothing.
    if (p.color == EMPTY || p.type == NONE)
    {
        return false;
    }

    // A piece cannot display another piece of the same color.
    struct piece dest_piece = get_piece(b, rank_to, file_to);
    if (p.color == dest_piece.color)
    {
        return false;
    }

    switch (p.type)
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
                    if (get_piece(b, rank_to, file).color != EMPTY)
                    {
                        return false;
                    }
                }
            }
            else if (file_from == file_to)
            {
                for (int rank = MIN(rank_from, rank_to); rank < MAX(rank_from, rank_to); rank++)
                {
                    if (get_piece(b, rank, file_to).color != EMPTY)
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
    b->pieces[i_from].type = NONE;
    b->pieces[i_from].color = EMPTY;
    place_piece(b, p, rank_to, file_to);

    return true;
}

void init_board(struct board *b)
{
    memset(b->pieces, 0, sizeof(b->pieces));

    for (int file = 0; file < 8; file++)
    {
        for (int rank = 0; rank < 2; rank++)
            set_piece_color(b, rank, file, WHITE);
        for (int rank = 6; rank < 8; rank++)
            set_piece_color(b, rank, file, BLACK);

        set_piece_type(b, 1, file, PAWN);
        set_piece_type(b, 6, file, PAWN);
    }

    for (int rank = 0; rank < 8; rank += 7)
    {
        set_piece_type(b, rank, 0, ROOK);
        set_piece_type(b, rank, 1, KNIGHT);
        set_piece_type(b, rank, 2, BISHOP);
        set_piece_type(b, rank, 3, QUEEN);
        set_piece_type(b, rank, 4, KING);
        set_piece_type(b, rank, 5, BISHOP);
        set_piece_type(b, rank, 6, KNIGHT);
        set_piece_type(b, rank, 7, ROOK);
    }

    b->white_to_move = true;
}

void move_algebraic(struct board *b, const char *move)
{
    int i = 0; // index into the move string
    enum piece_type ptype;

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

            struct piece p = get_piece(b, rank, file);
            bool white = p.color == WHITE;

            switch(p.type)
            {
                case PAWN:   printf(white ? UTF8_WPAWN : UTF8_BPAWN); break;
                case BISHOP: printf(white ? UTF8_WBISHOP : UTF8_BBISHOP); break;
                case KNIGHT: printf(white ? UTF8_WKNIGHT : UTF8_BKNIGHT); break;
                case ROOK:   printf(white ? UTF8_WROOK : UTF8_BROOK); break;
                case QUEEN:  printf(white ? UTF8_WQUEEN : UTF8_BQUEEN); break;
                case KING:   printf(white ? UTF8_WKING : UTF8_BKING); break;
                default:     printf(" "); break;
            }

            putc(' ', stdout);
        }
        printf(PRINT_RESET"\n");
    }
    
    printf("\n%s\n", b->white_to_move ? "White to move" : "Black to move");
}

#endif //BOARD_H

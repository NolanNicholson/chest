#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum piece
{
    NONE,
    WPAWN,
    WBISHOP,
    WKNIGHT,
    WROOK,
    WQUEEN,
    WKING,
    BPAWN,
    BBISHOP,
    BKNIGHT,
    BROOK,
    BQUEEN,
    BKING,
};

struct board
{
    enum piece pieces[64];
};

void place_piece(struct board *b, int rank, int file, enum piece p)
{
    b->pieces[rank*8+file] = p;
}

enum piece get_piece(const struct board *b, int rank, int file)
{
    return b->pieces[rank*8+file];
}

void init_board(struct board *b)
{
    memset(b->pieces, 0, sizeof(b->pieces));
    place_piece(b, 0, 0, WROOK);
    place_piece(b, 0, 7, WROOK);
    place_piece(b, 7, 0, BROOK);
    place_piece(b, 7, 7, BROOK);

    place_piece(b, 0, 1, WKNIGHT);
    place_piece(b, 0, 6, WKNIGHT);
    place_piece(b, 7, 1, BKNIGHT);
    place_piece(b, 7, 6, BKNIGHT);

    place_piece(b, 0, 2, WBISHOP);
    place_piece(b, 0, 5, WBISHOP);
    place_piece(b, 7, 2, BBISHOP);
    place_piece(b, 7, 5, BBISHOP);

    place_piece(b, 0, 3, WQUEEN);
    place_piece(b, 0, 4, WKING);
    place_piece(b, 7, 3, BQUEEN);
    place_piece(b, 7, 4, BKING);

    for (int file = 0; file < 8; file++)
    {
        place_piece(b, 1, file, WPAWN);
        place_piece(b, 6, file, BPAWN);
    }
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

    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++) // horizontal; A-H
        {
            white_square = (rank + file) % 2 > 0;

            printf(white_square ? PRINT_WSQUARE : PRINT_BSQUARE);

            enum piece p = get_piece(b, rank, file);
            switch(p)
            {
                case WPAWN: printf(UTF8_WPAWN); break;
                case BPAWN: printf(UTF8_BPAWN); break;
                case WBISHOP: printf(UTF8_WBISHOP); break;
                case BBISHOP: printf(UTF8_BBISHOP); break;
                case WKNIGHT: printf(UTF8_WKNIGHT); break;
                case BKNIGHT: printf(UTF8_BKNIGHT); break;
                case WROOK: printf(UTF8_WROOK); break;
                case BROOK: printf(UTF8_BROOK); break;
                case WQUEEN: printf(UTF8_WQUEEN); break;
                case BQUEEN: printf(UTF8_BQUEEN); break;
                case WKING: printf(UTF8_WKING); break;
                case BKING: printf(UTF8_BKING); break;
                default: printf(" "); break;
            }

            putc(' ', stdout);
        }
        printf(PRINT_RESET"\n");
    }
}

#endif //BOARD_H

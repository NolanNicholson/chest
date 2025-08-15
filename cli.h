#include "board.h"

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
#define ANSI_BG_GREEN "42"
#define ANSI_BG_YELLOW "43"
#define ANSI_BG_WHITE "47"

#define PRINT_BSQUARE       "\x1b[" ANSI_BG_YELLOW ";" ANSI_FG_BLACK "m"
#define PRINT_WSQUARE       "\x1b[" ANSI_BG_WHITE ";" ANSI_FG_BLACK "m"
#define PRINT_CURSORSQUARE  "\x1b[" ANSI_BG_GREEN ";" ANSI_FG_BLACK "m"
#define PRINT_RESET         "\x1b[0m"

void print_board(const struct board *b, const struct moveList *ml)
{
    bool white_square;

    printf("  a b c d e f g h\n");

    for (int rank = 7; rank >= 0; rank--)
    {
        printf("%d ", rank + 1);

        for (int file = 0; file < 8; file++) // horizontal; A-H
        {
            bool inMoveList = false;
            for (int i = 0; i < ml->n_moves; i++)
            {
                const struct coord *moveto = &(ml->moves[i].to);
                if (moveto->rank == rank && moveto->file == file)
                {
                    inMoveList = true;
                    break;
                }
            }

            white_square = (rank + file) % 2 > 0;

            if (inMoveList) { printf(PRINT_CURSORSQUARE); }
            else if (white_square) { printf(PRINT_WSQUARE); }
            else { printf(PRINT_BSQUARE); }

            int piece = get_piece(b, (struct coord){rank, file});

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

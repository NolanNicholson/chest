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
#define ANSI_BG_HIGREEN "102"
#define ANSI_BG_YELLOW "43"
#define ANSI_BG_WHITE "47"

#define PRINT_BSQUARE       "\x1b[" ANSI_BG_YELLOW ";" ANSI_FG_BLACK "m"
#define PRINT_WSQUARE       "\x1b[" ANSI_BG_WHITE ";" ANSI_FG_BLACK "m"
#define PRINT_MOVESSQUARE   "\x1b[" ANSI_BG_HIGREEN ";" ANSI_FG_BLACK "m"
#define PRINT_SELECTSQUARE  "\x1b[" ANSI_BG_GREEN ";" ANSI_FG_BLACK "m"
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
            bool can_move_to = false;
            bool selected = false;
            for (int i = 0; i < ml->n_moves; i++)
            {
                const struct coord *movefrom = &(ml->moves[i].from);
                const struct coord *moveto = &(ml->moves[i].to);

                if (movefrom->rank == rank && movefrom->file == file)
                {
                    selected = true;
                    break;
                }

                if (moveto->rank == rank && moveto->file == file)
                {
                    can_move_to = true;
                    break;
                }
            }

            white_square = (rank + file) % 2 > 0;

            if (selected) { printf(PRINT_SELECTSQUARE); }
            else if (can_move_to) { printf(PRINT_MOVESSQUARE); }
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
        printf(PRINT_RESET" %d\n", rank + 1);
    }
    printf("  a b c d e f g h\n");
    
    printf("\n%s\n", b->white_to_move ? "White to move" : "Black to move");
}

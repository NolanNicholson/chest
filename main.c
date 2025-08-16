#include "board.h"
#include "moves.h"
#include "cli.h"

bool select_piece(const char *coord_str, struct board *b, struct moveList *ml)
{
    int i = 0;

    char c_file = coord_str[i++];
    if (c_file < 'a' || c_file > 'h') { return false; }
    int file = c_file - 'a';

    char c_rank = coord_str[i++];
    if (c_rank < '1' || c_rank > '8') { return false; }
    int rank = c_rank - '1';

    genMovesForPiece(b, (struct coord) {rank, file}, ml);
}

int main(void)
{
    struct board b;
    init_board(&b);

    // default position
    apply_FEN(&b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Main game loop
    while (true)
    {
        print_board(&b, NULL);

        char cmd[64];

        struct moveList ml;
        init_movelist(&ml);
        genAllMoves(&b, &ml);

        // Prompt loop
        while (true)
        {
            scanf("%s", cmd);

            if (strcmp(cmd, "q") == 0)
            {
                return 0;
            }

            if (move_algebraic(&b, cmd, &ml))
            {
                break;
            }

            printf("Invalid input. Enter a legal move in algebraic notation, or enter 'q' to quit.\n");
        }
    }

    //select_piece("e5", &b, &ml);

    return 0;
}

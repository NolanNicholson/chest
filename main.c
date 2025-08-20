#include "board.h"
#include "moves.h"
#include "cli.h"
#include "ai.h"

// WHITE | BLACK for both humans
// NONE for both computers
#define HUMAN_PLAYER WHITE

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
    initComputer();

    // default position
    apply_FEN(&b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Main game loop
    while (true)
    {
        struct moveList ml;
        init_movelist(&ml);
        genAllMoves(&b, &ml);

        printBoard(&b, NULL);

        if (ml.n_moves == 0)
        {
            // If one side has no legal moves available,
            // that's checkmate if the king is currently in check,
            // and stalemate otherwise.
            // For example: "k7/1R6/2K5/8/8/8/8/8 b - - 0 1" is stalemate.
            if (isKingInCheck(&b))
            {
                printf(PRINT_ALERT "Checkmate! %s wins!" PRINT_RESET "\n", b.white_to_move ? "Black" : "White");
                return 0;
            }
            else
            {
                printf(PRINT_ALERT "Stalemate!" PRINT_RESET "\n");
                return 0;
            }
        }

        char cmd[64];
        int mover = b.white_to_move ? WHITE : BLACK;
        const char *mover_str = b.white_to_move ? "White" : "Black";

        if (mover & HUMAN_PLAYER)
        {
            // Prompt loop
            while (true)
            {
                printf("Enter %s's move (or q to quit): ", mover_str);
                scanf("%s", cmd);

                if (strcmp(cmd, "q") == 0)
                {
                    return 0;
                }

                if (moveAlgebraic(&b, cmd, &ml))
                {
                    break;
                }

                printf("Invalid input. Enter a legal move in algebraic notation, or enter 'q' to quit.\n");
            }
        }
        else
        {
            printf("%s is thinking...\n", mover_str);
            struct move m = getComputerMove(&b);
            printf("(Evaluated %d positions.)\n", eval_counter);
            printMove(&b, m);
            applyMove(&b, m);
        }
    }

    return 0;
}

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

    struct moveList ml;
    ml.n_moves = 0;

    select_piece("g1", &b, &ml);

    print_board(&b, &ml);

    return 0;
}

#include "board.h"

int main(void)
{
    struct board b;
    init_board(&b);

    const char *moves[] = {
        "Nf2", // this should be invalid
        "Nf4", // this should be invalid
        "Nf3", "Nf6",
        /*"c4", "g6",
        "Nc3", "Bg7",
        "d4", "0-0",
        "Bf4", "d5"*/
    };

    print_board(&b);

    for (int i = 0; i < sizeof(moves) / sizeof(moves[0]); i++)
    {
        printf("%s\n", moves[i]);
        move_algebraic(&b, moves[i]);
        print_board(&b);
    }

    return 0;
}

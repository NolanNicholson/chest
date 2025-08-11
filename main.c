#include "board.h"

int main(void)
{
    struct board b;
    init_board(&b);
    print_board(&b);
    return 0;
}

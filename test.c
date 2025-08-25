#include <stdio.h>
#include <time.h>

#include "board.h"
#include "moves.h"

#define MAX_PERFT_DEPTH 4

int num_tests;
int num_success;

long long int perft(const struct board *b, int depth, bool print)
{
    long long int nodes = 0;

    if (depth == 0) { return 1; }

    struct moveList ml;
    init_movelist(&ml);
    genAllMoves(b, &ml);

    if (depth == 1) { return ml.n_moves; }

    for (int i = 0; i < ml.n_moves; i++)
    {
        struct board b2 = *b;
        applyMove(&b2, ml.moves[i]);

        long long int responses = perft(&b2, depth-1, false);
        if (print)
        {
            struct move m = ml.moves[i];
            printf("%c%c%c%c",
                    m.from.file + 'a',
                    m.from.rank + '1',
                    m.to.file + 'a',
                    m.to.rank + '1');
            switch (m.promotion)
            {
                case QUEEN:     putc('q', stdout); break;
                case BISHOP:    putc('b', stdout); break;
                case KNIGHT:    putc('n', stdout); break;
                case ROOK:      putc('r', stdout); break;
            }
            printf(": %lld\n", responses);
        }

        nodes += responses;
    }

    return nodes;
}

struct PerftTest
{
    const char *start_pos;
    int max_depth;
    long long int expected[16];
};

bool runPerftTest(struct PerftTest *test)
{
    num_tests++;

    printf("Perft test on %s\n", test->start_pos);
    struct board b;
    init_board(&b);
    apply_FEN(&b, test->start_pos);

    int max_depth = MIN(test->max_depth, MAX_PERFT_DEPTH);

    for (int depth = 1; depth <= max_depth; depth++)
    {
        long long int perft_result = perft(&b, depth, false);
        if (perft_result == test->expected[depth])
        {
            printf("  depth %d OK - %lld\n", depth, perft_result);
        }
        else
        {
            fprintf(stderr, "  depth %d: expected %lld, got %lld\n",
                    depth, test->expected[depth], perft_result);
            return false;
        }
    }

    num_success++;
    return true;
}

int main()
{
    clock_t start = clock();
    num_tests = 0;
    num_success = 0;

    // Perft positions and results from:
    // https://www.chessprogramming.org/Perft_Results

    // starting position
    struct PerftTest perft_test_1 = {
        .start_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        .max_depth = 8,
        .expected = {
            1,
            20,
            400,
            8902,
            197281,
            4865609,
            119060324, 
            3195901860,
            84998978956
        }
    };

    runPerftTest(&perft_test_1);

    struct PerftTest perft_test_2 = {
        .start_pos = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
        .max_depth = 6,
        .expected = {
            1,
            48,
            2039,
            97862,
            4085603,
            193690690,
            8031647685
        }
    };

    runPerftTest(&perft_test_2);

    struct PerftTest perft_test_3 = {
        .start_pos = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        .max_depth = 8,
        .expected = {
            1,
            14,
            191,
            2812,
            43238,
            674624,
            11030083,
            178633661,
            3009794393
        }
    };

    runPerftTest(&perft_test_3);

    struct PerftTest perft_test_4 = {
        .start_pos = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        .max_depth = 6,
        .expected = {
            1,
            6,
            264,
            9467,
            422333,
            15833292,
            706045033
        }
    };

    runPerftTest(&perft_test_4);

    // mirrored version
    struct PerftTest perft_test_4m = {
        .start_pos = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", // mirrored
        .max_depth = 6,
        .expected = {
            1,
            6,
            264,
            9467,
            422333,
            15833292,
            706045033
        }
    };

    runPerftTest(&perft_test_4m);

    struct PerftTest perft_test_5 = {
        .start_pos = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        .max_depth = 5,
        .expected = {
            1,
            44,
            1486,
            62379,
            2103487,
            89941194
        }
    };

    runPerftTest(&perft_test_5);

    struct PerftTest perft_test_6 = {
        .start_pos = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        .max_depth = 6,
        .expected = {
            1,
            46,
            2079,
            89890,
            3894594,
            164075551,
            6923051137,
            287188994746,
            11923589843526
        }
    };

    runPerftTest(&perft_test_6);

    clock_t stop = clock();
    double duration = (double) (stop - start) / CLOCKS_PER_SEC;

    printf("\nPassed %d of %d tests in %f seconds\n",
            num_success, num_tests, duration);

    return 0;
}

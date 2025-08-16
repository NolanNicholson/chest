#ifndef CLI_H
#define CLI_H

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

    printf("\n  a b c d e f g h\n");

    for (int rank = 7; rank >= 0; rank--)
    {
        printf("%d ", rank + 1);

        for (int file = 0; file < 8; file++) // horizontal; A-H
        {
            bool can_move_to = false;
            bool selected = false;
            if (ml)
            {
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

struct coord coordstr(const char *str)
{
    struct coord output = {
        .file = str[0] - 'a',
        .rank = str[1] - '1'
    };
    return output;
}

bool move_algebraic(struct board *b, const char *move, struct moveList *allLegalMoves)
{
    int i = 0; // index into the move string
    int ptype;

    bool white = b->white_to_move;
    bool is_castle = false;
    bool is_queenside_castle = false;
    bool recognized = true;

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
                      is_castle = true;

                      if (move[i++] == '-' && move[i++] == o)
                      {
                          is_queenside_castle = true;
                      }

                      ptype = KING;
                      break;
                  }
                  else
                  {
                      fprintf(stderr, "Unnrecognized castling attempt? %s\n", move);
                      return false;
                  }

        default:
                  fprintf(stderr, "Unrecognized move: %s\n", move);
                  return false;
    }

    char rank_c, file_c;

    if (is_castle)
    {
        file_c = is_queenside_castle ? 'b' : 'g';
        rank_c = white ? '1' : '8';
    }
    else
    {
        file_c = move[i++];
        rank_c = move[i++];
    }

    int piece = (white ? WHITE : BLACK) | ptype;
    int file = file_c - 'a';
    int rank = rank_c - '1';

    int i_match;
    int n_matching_moves = 0;
    for (int i_move = 0; i_move < allLegalMoves->n_moves; i_move++)
    {
        struct move *m = &(allLegalMoves->moves[i_move]);

        if (m->to.rank != rank) { continue; }
        if (m->to.file != file) { continue; }
        if (get_piece(b, m->from) != piece) { continue; }

        i_match = i_move;
        n_matching_moves++;
    }

    if (n_matching_moves != 1)
    {
        fprintf(stderr, "%d moves match string %s (1 expected)\n", n_matching_moves, move);
        return false;
    }

    applyMove(b, allLegalMoves->moves[i_match]);
    return true;
}

const char *getPieceTypeStr(int piece)
{
    switch(piece & PIECE_TYPE)
    {
        case PAWN: return "Pawn";
        case ROOK: return "Rook";
        case KNIGHT: return "Knight";
        case BISHOP: return "Bishop";
        case QUEEN: return "Queen";
        case KING: return "King";
    }
}

void printMove(struct board *b, struct move m)
{
    int piece = get_piece(b, m.from);
    const char *owner = ((piece & PIECE_COLOR) == WHITE) ? "White" : "Black";
    const char *type = getPieceTypeStr(piece);

    printf("%s's move: %s to %c%c\n", owner, type, m.to.file + 'a', m.to.rank + '1');
}

#endif // CLI_H

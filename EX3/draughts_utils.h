#include "Draughts.h"

// safe_malloc/realloc verifies that tht memory allocation succeeded
void * safe_malloc(size_t size);
void * safe_realloc(void *old_pointer, size_t size);

int is_valid_pos(Pos pos);
int is_opposite(COLOR player, char piece);

extern Move* moves;
extern Move* moves_head;

void add_move(Move* res, Pos piece, Pos* dests, int dests_num);
Pos get_pos_after_capture(Pos from, Pos to);
//Pos next_diag(Pos p, int dir);
int get_capture_moves(Pos start, Pos piece, char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int count, Pos* dests);
void get_man_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece);
void get_king_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece);
Move * get_all_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player);

void print_moves(Move* head);
#include "Draughts.h"

// safe_malloc/realloc verifies that tht memory allocation succeeded
void * safe_malloc(size_t size);
void * safe_realloc(void *old_pointer, size_t size);

extern Move* moves;
extern Move* moves_head;
extern char curr_piece;
extern COLOR curr_player;
extern Move* best_move;


int is_valid_pos(Pos pos);
int is_king(char piece);
int is_opposite(COLOR player, char piece);
int is_EOB(Pos piece, COLOR player);

void clear_old_moves(Move* head);
void add_move(Pos piece, Pos* dests, int move_captures);
Pos get_next_diag(Pos from, Pos to);
int get_capture_moves(Pos start, Pos piece, char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int count, Pos* dests);
void get_man_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece);
void get_king_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece);
Move * get_all_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player);

void print_move(Move* head);
void print_moves(Move* head);
int get_piece_score(char piece, COLOR player);
int calc_score(char board[BOARD_SIZE][BOARD_SIZE], COLOR player);
void duplicate_board(char board1[BOARD_SIZE][BOARD_SIZE], char board2[BOARD_SIZE][BOARD_SIZE]);
int alpha_beta_minimax(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int depth, int alpha, int beta);
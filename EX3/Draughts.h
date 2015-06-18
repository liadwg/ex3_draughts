#ifndef DRAUGHTS_
#define DRAUGHTS_
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define WHITE_M 'm'
#define WHITE_K 'k'
#define BLACK_M 'M'
#define BLACK_K 'K'
#define EMPTY ' '

#define BOARD_SIZE 10

typedef char** board_t;
#define WELCOME_TO_DRAUGHTS "Welcome to Draughts!\n"
#define ENTER_SETTINGS "Enter game settings:\n" 
#define WRONG_MINIMAX_DEPTH "Wrong value for minimax depth. The value should be between 1 to 6\n"
#define WRONG_POSITION "Invalid position on the board\n"
#define NO_DISC "The specified position does not contain your piece\n"
#define ILLEGAL_COMMAND "Illegal command, please try again\n"
#define ILLEGAL_MOVE "Illegal move\n"
#define WROND_BOARD_INITIALIZATION "Wrong board initialization\n"

#define ENTER_YOUR_MOVE "Enter your move:\n" 
#define WHITE_WIN "White player wins!\n" 
#define BLACK_WIN "Black player wins!\n" 
#define perror_message(func_name) (fprintf(stderr, "Error: standard function %s has failed\n", func_name))
#define print_message(message) (printf("%s", message))

#define QUIT 2
#define WIN_POS 0
#define GAME_ON 1

typedef enum { WHITE = 0, BLACK = 1 } COLOR;

typedef struct pos{
	int col;
	int row;
} Pos;

typedef struct move{
	Pos piece;
	Pos* dest;
	int captures;
	struct move* next;
} Move;

// Memory allocation and standard functions monitoring
void add_to_list(void* mem);
void remove_from_list(void* mem);
void * safe_malloc(size_t size);
void * safe_realloc(void *old_pointer, size_t size);
int safe_fgetc(FILE *stream);
void safe_free(void * mem);

// Draughts code
int is_valid_pos(Pos pos);
int is_king(char piece);
int is_opposite(COLOR player, char piece);
int is_EOB(Pos piece, COLOR player);

void clear_old_moves(Move* head);
void add_move(Pos piece, Pos* dests, int move_captures);
Pos get_next_diag(Pos from, Pos to);
Pos get_prev_diag(Pos from, Pos to);
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

int is_valid_board(char board[BOARD_SIZE][BOARD_SIZE]);
void print_board(char board[BOARD_SIZE][BOARD_SIZE]);
void init_board(char board[BOARD_SIZE][BOARD_SIZE]);
void clear_board(char board[BOARD_SIZE][BOARD_SIZE]);
char* input2str(FILE* pFile);
void exc(char* str, char board[BOARD_SIZE][BOARD_SIZE]);
int computer_turn(char board[BOARD_SIZE][BOARD_SIZE], COLOR color);
int user_turn(char board[BOARD_SIZE][BOARD_SIZE], COLOR color);
int is_valid_piece(char board[BOARD_SIZE][BOARD_SIZE], Move * move, COLOR color);
Move * is_valid_move(Move * moves, Move * new_move);
void exc_move(char board[BOARD_SIZE][BOARD_SIZE], Move * move);

// Globals
extern COLOR user_color;
extern int minimax_depth;
extern Move* moves;
extern Move* moves_head;
extern char curr_piece;
extern COLOR curr_player;
extern Move* best_move;

#endif




#ifndef DRAUGHTS_
#define DRAUGHTS_

#include <stdio.h>
#include <stdlib.h>

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
#define WHITE_WIN "white player win!" 
#define BLACK_WIN "black player win!" 
#define perror_message(func_name) (fprintf(stderr, "Error: standard function %s has failed\n", func_name))
#define print_message(message) (printf("%s", message))

#define QUIT 2
#define WIN_POS 0
#define GAME_ON 1

typedef enum { WHITE = 0, BLACK = 1 } COLOR;

//typedef enum { COMPUTER = 0, USER = 1 } TURN;

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


extern COLOR user_color;
extern int minimax_depth;
#endif




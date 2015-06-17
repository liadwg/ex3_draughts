//#include "draughts_utils.h"
#include "Draughts.h"

COLOR user_color = WHITE;
int minimax_depth = 1;

void print_line(){
	int i;
	printf("  |");
	for (i = 1; i < BOARD_SIZE * 4; i++){
		printf("-");
	}
	printf("|\n");
}

void print_board(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i, j;
	print_line();
	for (j = BOARD_SIZE - 1; j >= 0; j--)
	{
		printf((j < 9 ? " %d" : "%d"), j + 1);
		for (i = 0; i < BOARD_SIZE; i++){
			printf("| %c ", board[i][j]);
		}
		printf("|\n");
		print_line();
	}
	printf("   ");
	for (j = 0; j < BOARD_SIZE; j++){
		printf(" %c  ", (char)('a' + j));
	}
	printf("\n");
}

void init_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			if ((i + j) % 2 == 0){
				if (j <= 3){
					board[i][j] = WHITE_M;
				}
				else if (j >= 6){
					board[i][j] = BLACK_M;
				}
				else{
					board[i][j] = EMPTY;
				}
			}
			else{
				board[i][j] = EMPTY;
			}
		}
	}
}

void clear_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			board[i][j] = EMPTY;
		}
	}
}

char* input2str(FILE* pFile)
{
	char *str;
	char ch, pch;
	size_t size = 10;
	size_t len = 0;
	str = malloc(sizeof(char)*size);
	ch = fgetc(pFile);
	pch = '~';
	while (ch != EOF && ch != '\n')
	{
		if ((pch != '~' && pch != ' ') || (ch != ' ')){
			str[len++] = ch;
			if (len == size)
			{
				size = 2 * size;
				str = realloc(str, sizeof(char)*size);
			}
			pch = ch;
			ch = fgetc(pFile);
		}
		else{
			pch = ch;
			ch = fgetc(pFile);
		}
	}
	str[len++] = '\0';
	str = realloc(str, sizeof(char)*len);
	return str;
}

void exc(char* str, char board[BOARD_SIZE][BOARD_SIZE]){
	char * word1;
	word1 = strtok(str, " ");
	if (strcmp(word1, "minimax_depth") == 0){
		int x = atoi(strtok(NULL, " "));
		if (x > 6 || x < 1) printf(WRONG_MINIMAX_DEPTH);
		else minimax_depth = x;
	}
	if (strcmp(word1, "user_color") == 0){
		char * color = strtok(NULL, " ");
		if (strcmp(color, "black") == 0) user_color = BLACK;
	}
	if (strcmp(word1, "clear") == 0) clear_board(board);
	if (strcmp(word1, "rm") == 0){
		char * coor = strtok(NULL, " <>");
		if (coor[0] < 'a' || coor[0] > 'j' || coor[2] < '1' || (coor[2] - '0') > 10) printf(WRONG_POSITION);
		else board[coor[0] - 'a'][coor[2]-'1'] = EMPTY;
	}
	if (strcmp(word1, "set") == 0){
		char * coor = strtok(NULL, " <>");
		if (coor[0] < 'a' || coor[0] > 'j' || coor[2] < '1' || (coor[2] - '0') > 10) printf(WRONG_POSITION);
		char * a = strtok(NULL, " ");
		if (a == NULL) return;
		char * b = strtok(NULL, " ");
		if (strcmp(a, "black") == 0){
			if (strcmp(b, "m") == 0) board[coor[0] - 'a'][coor[2] - '1'] = BLACK_M;
			else  board[coor[0] - 'a'][coor[2] - '1'] = BLACK_K;
		}
		else {
			if (strcmp(b, "m") == 0) board[coor[0] - 'a'][coor[2] - '1'] = WHITE_M;
			else  board[coor[0] - 'a'][coor[2] - '1'] = WHITE_K;
		}
	}
	if (strcmp(word1, "print") == 0) print_board(board);
	return;
}

int computer_turn(char board[BOARD_SIZE][BOARD_SIZE],COLOR color){
	//get_all_moves(board, color);
	//int score = alpha_beta_minimax(board, color, 0, -100, 100);
	alpha_beta_minimax(board, color, 0, -100, 100);
	Move * move2do = best_move;
	int ret_val;
	if (move2do == NULL) ret_val = WIN_POS;
	else{ 
		exc_move(board, move2do);
		printf("Computer: move ");
		print_move(move2do);
		print_board(board);
		ret_val = GAME_ON;
	}
	clear_old_moves(moves_head);
	return ret_val;
}

int user_turn(char board[BOARD_SIZE][BOARD_SIZE], COLOR color){
	get_all_moves(board, color);
	if (moves_head == NULL) return WIN_POS;
	printf(ENTER_YOUR_MOVE);
	char *word1;
	char *command;
	Move* new_move = malloc(sizeof(Move));
	new_move->dest = malloc(sizeof(Pos) * 2 * BOARD_SIZE);
	new_move->next = NULL;
	int ret_val;
	while (1){
		printf("> ");
		command = input2str(stdin);
		word1 = strtok(command, " ");
		if (strcmp(word1, "quit") == 0){
			ret_val = QUIT;
			break;
		}
		if (strcmp(word1, "get_moves") == 0){
			print_moves(moves_head);
			continue;
		}
		if (strcmp(word1, "move") == 0){
			char * piece_str = strtok(NULL, " <>");
			new_move->piece.col = piece_str[0] - 'a';
			new_move->piece.row = piece_str[2] - '1';
			if (!is_valid_pos(new_move->piece)){
				printf(WRONG_POSITION);
				continue;
			}
			int i = 0;
			char * dest_str = strtok(NULL, " <>to[]");
			while (dest_str != NULL){
				new_move->dest[i].col = dest_str[0] - 'a';
				new_move->dest[i].row = dest_str[2] - '1';
				if (!is_valid_pos(new_move->dest[i])){
					i = -1;
					break;
				}
				i++;
				dest_str = strtok(NULL, " <>to[]");
			}
			if (i == -1){
				printf(WRONG_POSITION);
				free(new_move->dest);
				continue;
			}
			if (!is_valid_piece(board, new_move, color)){
				printf(NO_DISC);
				free(new_move->dest);
				continue;
			}
			new_move->dest = realloc(new_move->dest, sizeof(Pos) * i);
			new_move->captures = i;
			Move * move2do = is_valid_move(moves_head, new_move);
			if (move2do == NULL){
				printf(ILLEGAL_MOVE);
				free(new_move->dest);
				continue;
			}
			else{
				exc_move(board, move2do);
				print_board(board);
				ret_val = GAME_ON;
				break;
			}
		}
	}
	clear_old_moves(new_move);
	clear_old_moves(moves_head);
	return ret_val;
}

int is_valid_piece(char board[BOARD_SIZE][BOARD_SIZE], Move * move, COLOR color){
	if (color == BLACK){
		if (board[move->piece.col][move->piece.row] == BLACK_M || board[move->piece.col][move->piece.row] == BLACK_K) return 1;
	}
	if (color == WHITE) {
		if (board[move->piece.col][move->piece.row] == WHITE_M || board[move->piece.col][move->piece.row] == WHITE_K) return 1;
	}
	return 0;
}

Move * is_valid_move(Move * moves, Move * new_move){
	Move * current_move = moves;
	while (current_move != NULL){
		if (current_move->piece.col == new_move->piece.col && 
			current_move->piece.row == new_move->piece.row &&
			(current_move->captures == new_move->captures || (current_move->captures == 0 && new_move->captures == 1))){
			for (int i = 0; i < new_move->captures; i++){
				if (current_move->dest[i].col != new_move->dest[i].col || current_move->dest[i].row != new_move->dest[i].row) break;
				if (i == new_move->captures - 1) return current_move;
			}
		}
		current_move = current_move->next;
	}
	return NULL;
}

void exc_move(char board[BOARD_SIZE][BOARD_SIZE], Move * move){
	Pos cur, cap;
	cur.col = move->piece.col;
	cur.row = move->piece.row;
	int dest_len = move->captures;
	if (dest_len == 0) dest_len = 1;
	for (int i = 0; i < dest_len ; i++){
		board[move->dest[i].col][move->dest[i].row] = board[cur.col][cur.row];
		if (board[cur.col][cur.row] == BLACK_M && move->dest[i].row == 0) board[move->dest[i].col][move->dest[i].row] = BLACK_K;
		if (board[cur.col][cur.row] == WHITE_M && move->dest[i].row == BOARD_SIZE - 1) board[move->dest[i].col][move->dest[i].row] = WHITE_K;
		board[cur.col][cur.row] = EMPTY;
		if (move->captures > 0){
			cap = get_prev_diag(cur, move->dest[i]);
			board[cap.col][cap.row] = EMPTY;
			//board[(current_col + move->dest[i].col) / 2][(current_row + move->dest[i].row) / 2] = EMPTY; //check int derive
		}
		cur.col = move->dest[i].col;
		cur.row = move->dest[i].row;
	}
}

int main(void)
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	printf(ENTER_SETTINGS);
	printf("> ");
	char *command = input2str(stdin);
	int win_pos;

	while (strcmp(command, "quit") != 0 && strcmp(command, "start") != 0){
		exc(command,board);
		free(command);
		printf("> ");
		command = input2str(stdin);
	}
	
	if (strcmp(command, "start") == 0){
		print_board(board);
		while (1){
			if (user_color == WHITE){
				int ret_val = user_turn(board, WHITE);
				if (ret_val == QUIT) break;
				if (ret_val == WIN_POS){
					printf(BLACK_WIN);
					win_pos = 1;
					break;
				}
				if (computer_turn(board, BLACK) == WIN_POS){
					printf(WHITE_WIN);
					win_pos = 1;
					break;
				}
			}
			else{
				if (computer_turn(board, WHITE) == WIN_POS){
					printf(BLACK_WIN);
					win_pos = 1;
					break;
				}
				int ret_val = user_turn(board, BLACK);
				if (ret_val == QUIT) break;
				if (ret_val == WIN_POS){
					printf(WHITE_WIN);
					win_pos = 1;
					break;
				}
			}
		}
	}
	if (win_pos == 1) command = input2str(stdin);
	free(command);
	return 0;
}



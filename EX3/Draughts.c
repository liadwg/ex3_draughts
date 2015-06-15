#include "draughts_utils.h"

COLOR user_color = WHITE;
int minimax_depth = 0;

int main()
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	board[3][3] = WHITE_K;
	//board[2][4] = BLACK_M;
	//board[4][4] = BLACK_M;
	//board[5][7] = EMPTY;
	print_board(board);
	print_message(WRONG_MINIMAX_DEPTH);
	perror_message("TEST");
	Move* m = get_all_moves(board, WHITE);
	print_moves(m);
	scanf_s("DONE");
	return 0;
}

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
	int ch, pch;
	size_t size = 10;
	size_t len = 0;
	str = malloc(sizeof(char)*size);
	ch = fgetc(pFile);
	pch = NULL;
	while (ch != EOF && ch != '\n')
	{
		if ((pch != NULL && pch != ' ') || (ch != ' ')){
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
		if (strcmp(word1, "black") == 0) user_color = BLACK;
		free(color);
	}
	if (strcmp(word1, "clear") == 0) clear_board(board);
	if (strcmp(word1, "rm") == 0){
		char * coor = strtok(NULL, " ,<>");
		if (coor[0] < 'a' || coor[0] > 'j' || (coor[1] - '0') < 1 || (coor[1] - '0') > 10) printf(WRONG_POSITION);
		else board[coor[0] - 'a'][coor[1] - '0'] = EMPTY;
		free(coor);
	}
	if (strcmp(word1, "set") == 0){
		char * coor = strtok(NULL, " ,<>");
		if (coor[0] < 'a' || coor[0] > 'j' || (coor[1] - '0') < 1 || (coor[1] - '0') > 10) printf(WRONG_POSITION);
		char * a = strtok(NULL, " "), b;
		if (a != NULL){
			b = strtok(NULL, " ");
		}
		if (strcmp(a, "black") == 0){
			if (strcmp(b, "m") == 0) board[coor[0] - 'a'][coor[1] - '0'] = BLACK_M;
			else  board[coor[0] - 'a'][coor[1] - '0'] = BLACK_K;
		}
		else {
			if (strcmp(b, "m") == 0) board[coor[0] - 'a'][coor[1] - '0'] = WHITE_M;
			else  board[coor[0] - 'a'][coor[1] - '0'] = WHITE_K;
		}
		free(coor), free(a), free(b);
	}
	if (strcmp(word1, "print") == 0) print_board(board);
	free(word1);
	return;
}


//int main(void)
//{
//	printf(ENTER_SETTINGS);
//	printf("> ");
//	char *command = input2str(stdin);
//
//	while (strcmp(command, "quit") != 0 && strcmp(command, "start") != 0){
//		exc(command);
//		free(command);
//		printf("> ");
//		command = input2str(stdin);
//	}
//	if (strcmp(command, "start") == 0)
//		free(command);
//	//***move to game state***
//	free(command);
//	return 0;
//}
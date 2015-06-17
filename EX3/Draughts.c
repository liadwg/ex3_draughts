#include "Draughts.h"

// safe_funcs verifies that that the original functions succeeded
void * safe_malloc(size_t size){
	void *res = malloc(size);
	if (!res && size != 0){
		perror_message("malloc");
		abort();
	}
	else return res;
}
#define malloc(x) safe_malloc(x)

void * safe_realloc(void *old_pointer, size_t size){
	void *res = realloc(old_pointer, size);
	if (!old_pointer && size != 0){
		free(old_pointer);
		perror_message("realloc");
		abort();
	}
	else return res;
}
#define realloc(x, y) safe_realloc((x), (y))

int safe_fgetc(FILE *stream){
	int res = fgetc(stream);
	if (res == EOF){
		perror_message("fgetc");
		abort();
	}
	else return res;
}
#define fgetc(x) safe_fgetc(x)

#define printf(...) \
	if (printf(__VA_ARGS__) < 0) {perror_message("printf"); abort();} \
		else (void)0


// Globals
Move* moves = NULL;
Move* moves_head = NULL;
char curr_piece;
COLOR curr_player;
Move* best_move;
COLOR user_color = WHITE;
int minimax_depth = 1;


// Helper funcs
int is_valid_pos(Pos pos){
	return (pos.col >= 0 && pos.col < BOARD_SIZE && pos.row >= 0 && pos.row < BOARD_SIZE);
}

int is_king(char piece){
	return (piece == WHITE_K || piece == BLACK_K);
}

int is_opposite(COLOR player, char piece){
	if (player == WHITE && (piece == BLACK_M || piece == BLACK_K)) return 1;
	if (player == BLACK && (piece == WHITE_M || piece == WHITE_K)) return 1;
	return 0;
}

// Helper func - checks if a piece reached the end of the board according to its color
int is_EOB(Pos piece, COLOR player){
	if (player == WHITE) return piece.row == BOARD_SIZE - 1;
	else return piece.row == 0;
}

// frees moves linked list
void clear_old_moves(Move* head){
	if (head != NULL){
		clear_old_moves(head->next);
		free(head->dest);
		free(head);
	}
}

// adds a move to the list, while making sure it contains valid moves only
void add_move(Pos piece, Pos* dests, int move_captures){
	if (moves == NULL){
		moves = malloc(sizeof(Move));
		moves_head = moves;
	}
	else if (moves->captures < move_captures  && abs(piece.row - dests->row) != 1){
		clear_old_moves(moves_head);
		moves = malloc(sizeof(Move));
		moves_head = moves;
	}
	else if ((moves->captures == 0 && abs(piece.row - dests->row) == 1) || (moves->captures == move_captures && abs(piece.row - dests->row) != 1)){
		moves->next = malloc(sizeof(Move));
		moves = moves->next;
	}
	else return;

	moves->piece.col = piece.col;
	moves->piece.row = piece.row;
	if (move_captures == 0){
		moves->dest = malloc(sizeof(Pos));
		moves->dest->row = dests->row;
		moves->dest->col = dests->col;
	}
	else{
		moves->dest = malloc(sizeof(Pos) * move_captures);
		for (int i = 0; i < move_captures; i++){
			moves->dest[i].row = dests[i].row;
			moves->dest[i].col = dests[i].col;
		}
	}

	moves->captures = move_captures;
	moves->next = NULL;
}

// Helper funcs - return the next/prev pos in a diagonal
Pos get_next_diag(Pos from, Pos to){
	Pos res;
	if (from.col - to.col > 0) res.col = to.col - 1;
	else res.col = to.col + 1;
	if (from.row - to.row > 0) res.row = to.row - 1;
	else res.row = to.row + 1;
	return res;
}

Pos get_prev_diag(Pos from, Pos to){
	Pos res;
	if (from.col - to.col > 0) res.col = to.col + 1;
	else res.col = to.col - 1;
	if (from.row - to.row > 0) res.row = to.row + 1;
	else res.row = to.row - 1;
	return res;
}

// finds moves with chained captures
int get_capture_moves(Pos start, Pos piece, char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int count, Pos* dests){
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	int found_now = 0, found_ahead = -1;
	for (int p = 0; p < 4; p++)
		if (is_valid_pos(pos[p]) && is_opposite(player, board[pos[p].col][pos[p].row])){
		Pos new_piece = get_next_diag(piece, pos[p]);
		if (is_valid_pos(new_piece) && board[new_piece.col][new_piece.row] == EMPTY){
			found_now++;
			char tmp = board[pos[p].col][pos[p].row];
			board[pos[p].col][pos[p].row] = EMPTY;
			Pos* new_dests = malloc(sizeof(Pos) * (count + 1));
			for (int i = 0; i < count; i++){
				new_dests[i].row = dests[i].row;
				new_dests[i].col = dests[i].col;
			}
			new_dests[count].row = new_piece.row;
			new_dests[count].col = new_piece.col;
			if (!(is_EOB(new_piece, player) && !is_king(curr_piece))) found_ahead = get_capture_moves(start, new_piece, board, player, count + 1, new_dests);
			if (found_ahead == 0 || (is_EOB(new_piece, player) && !is_king(curr_piece))){
				add_move(start, new_dests, count + 1);
				free(new_dests);
			}
			board[pos[p].col][pos[p].row] = tmp;
		}
		}
	return found_now;
}

// finds all moves of a single man piece
void get_man_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece){
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	int direction = 1, found_ahead = -1;
	if (player == BLACK) direction = -1;

	for (int p = 0; p < 4; p++){
		if (is_valid_pos(pos[p]) && pos[p].row == piece.row + direction && board[pos[p].col][pos[p].row] == EMPTY) add_move(piece, &pos[p], 0);
		else if (is_valid_pos(pos[p]) && is_opposite(player, board[pos[p].col][pos[p].row])){
			char tmp = board[pos[p].col][pos[p].row];
			board[pos[p].col][pos[p].row] = EMPTY;
			board[piece.col][piece.row] = EMPTY;
			Pos new_piece = get_next_diag(piece, pos[p]);
			if (is_valid_pos(new_piece) && board[new_piece.col][new_piece.row] == EMPTY){
				if (!is_EOB(new_piece, player)) found_ahead = get_capture_moves(piece, new_piece, board, player, 1, &new_piece);
				if (found_ahead == 0 || is_EOB(new_piece, player)) add_move(piece, &new_piece, 1);
			}
			board[pos[p].col][pos[p].row] = tmp;
			board[piece.col][piece.row] = curr_piece;
		}
	}
}

// finds all moves of a single king piece
void get_king_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece){
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	Pos curr, new_piece;
	int found_ahead = -1;

	for (int p = 0; p < 4; p++){
		curr = pos[p];
		while (is_valid_pos(curr) && board[curr.col][curr.row] == EMPTY){
			add_move(piece, &curr, 0);
			curr = get_next_diag(piece, curr);
		}
		if (is_valid_pos(curr) && is_opposite(player, board[curr.col][curr.row])){
			char tmp = board[curr.col][curr.row];
			board[curr.col][curr.row] = EMPTY;
			board[piece.col][piece.row] = EMPTY;
			new_piece = get_next_diag(piece, curr);
			if (is_valid_pos(new_piece) && board[new_piece.col][new_piece.row] == EMPTY){
				found_ahead = get_capture_moves(piece, new_piece, board, player, 1, &new_piece);
				if (found_ahead == 0) add_move(piece, &new_piece, 1);
			}
			board[curr.col][curr.row] = tmp;
			board[piece.col][piece.row] = curr_piece;
		}
	}
}

Move * get_all_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player){
	moves_head = NULL;
	moves = NULL;
	Pos p;
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			if (!is_opposite(player, board[i][j]) && board[i][j] != EMPTY){
		p.row = j;
		p.col = i;
		curr_piece = board[i][j];
		if (is_king(curr_piece)) get_king_moves(board, player, p);
		else get_man_moves(board, player, p);
			}
	return moves_head;
}

// prints a single move in a specific format
void print_move(Move* head){
	printf("<%c,%d> to <%c,%d>", head->piece.col + 97, head->piece.row + 1, head->dest[0].col + 97, head->dest[0].row + 1);
	for (int i = 1; i < head->captures; i++){
		printf("<%c,%d>", head->dest[i].col + 97, head->dest[i].row + 1);
	}
	printf("\n");
}

void print_moves(Move* head){
	while (head != NULL){
		print_move(head);
		head = head->next;
	}
}

// calc_score helper func
int get_piece_score(char piece, COLOR player){
	if (player == WHITE) switch (piece){
	case BLACK_M: return -1;
	case BLACK_K: return -3;
	case WHITE_M: return 1;
	case WHITE_K: return 3;
	}
	else switch (piece){
	case BLACK_M: return 1;
	case BLACK_K: return 3;
	case WHITE_M: return -1;
	case WHITE_K: return -3;
	}
	return 0;
}

// calculates the score of the board from a player's prospective
int calc_score(char board[BOARD_SIZE][BOARD_SIZE], COLOR player){
	int score = 0, opposites = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++){
		if (is_opposite(player, board[i][j])) opposites++;
		score += get_piece_score(board[i][j], player);
		}
	if (opposites == 0) return 100;
	return score;
}

// Helper func
void duplicate_board(char board1[BOARD_SIZE][BOARD_SIZE], char board2[BOARD_SIZE][BOARD_SIZE]){
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++) board2[i][j] = board1[i][j];
}

// minimax recursive func, using alpha-beta prunning
int alpha_beta_minimax(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int depth, int alpha, int beta){
	Move* move_list = get_all_moves(board, player);
	Move* curr_move = move_list;
	//printf("---------- minimax level %d player %d ------------\n", depth, player);
	//print_moves(move_list);
	if (depth == 0 && curr_move == NULL){
		best_move = NULL;
		return -100;
	}
	if (curr_move == NULL && player != curr_player) return 100;
	if (curr_move == NULL && player == curr_player) return -100;
	if (depth == minimax_depth || curr_move == NULL){
		clear_old_moves(move_list);
		return calc_score(board, curr_player);
	}

	if (depth == 0){
		best_move = curr_move;
		if (curr_move->next == NULL) return 100;
	}

	int tmp;
	char init_board[BOARD_SIZE][BOARD_SIZE];
	duplicate_board(board, init_board);
	if (depth % 2 == 0){
		while (curr_move != NULL){
			exc_move(board, curr_move);
			//printf("********\nmove by player %d ", player);
			//print_move(curr_move);
			tmp = alpha_beta_minimax(board, (player == 0), depth + 1, alpha, beta);
			//printf("got score %d\n*************\n", tmp);
			if (tmp > alpha){
				alpha = tmp;
				if (depth == 0) best_move = curr_move;
			}
			if (alpha >= beta){
				if (depth != 0) clear_old_moves(move_list);
				else moves_head = move_list;
				duplicate_board(init_board, board);
				return alpha;
			}
			curr_move = curr_move->next;
			duplicate_board(init_board, board);
		}
		if (depth != 0) clear_old_moves(move_list);
		else moves_head = move_list;
		return alpha;
	}
	else{
		while (curr_move != NULL){
			exc_move(board, curr_move);
			//printf("********\nmove by player %d ", player);
			//print_move(curr_move);
			tmp = alpha_beta_minimax(board, (player == 0), depth + 1, alpha, beta);
			//printf("got score %d\n*************\n", tmp);
			if (tmp < beta){
				beta = tmp;
				if (depth == 0) best_move = curr_move;
			}
			if (alpha >= beta){
				if (depth != 0) clear_old_moves(move_list);
				else moves_head = move_list;
				duplicate_board(init_board, board);
				return beta;
			}
			curr_move = curr_move->next;
			duplicate_board(init_board, board);
		}
		if (depth != 0) clear_old_moves(move_list);
		else moves_head = move_list;
		return beta;
	}
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

// clears the board from all pieces
void clear_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int i, j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			board[i][j] = EMPTY;
		}
	}
}

// handles user input (unknown length), returns a string without redundent white spaces after each new line
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

// settings state input loop - gets the user's command and handles it
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
		char * coor1 = strtok(NULL, " <,>");
		char * coor2 = strtok(NULL, " <,>");
		if (coor1[0] < 'a' || coor1[0] > 'j' || atoi(coor2) < 1 || atoi(coor2) > 10) { printf(WRONG_POSITION); }
		else board[coor1[0] - 'a'][atoi(coor2) - 1] = EMPTY;
	}
	if (strcmp(word1, "set") == 0){
		//char * coor = strtok(NULL, " <>");
		char * coor1 = strtok(NULL, " <,>");
		char * coor2 = strtok(NULL, " <,>");
		if (coor1[0] < 'a' || coor1[0] > 'j' || atoi(coor2) < 1 || atoi(coor2) > 10) { printf(WRONG_POSITION); }
		char * a = strtok(NULL, " ");
		if (a == NULL) return;
		char * b = strtok(NULL, " ");
		if (strcmp(a, "black") == 0){
			if (strcmp(b, "m") == 0) board[coor1[0] - 'a'][atoi(coor2)-1] = BLACK_M;
			else  board[coor1[0] - 'a'][atoi(coor2) - 1] = BLACK_K;
		}
		else {
			if (strcmp(b, "m") == 0) board[coor1[0] - 'a'][atoi(coor2)-1] = WHITE_M;
			else  board[coor1[0] - 'a'][atoi(coor2) - 1] = WHITE_K;
		}
	}
	if (strcmp(word1, "print") == 0) print_board(board);
	return;
}

// manages the computer's turn
int computer_turn(char board[BOARD_SIZE][BOARD_SIZE],COLOR color){
	curr_player = color;
	//print_board(board);
	alpha_beta_minimax(board, color, 0, -100, 100);
	//printf("$$$$$$$$$$$$$$$\nmove ");
	//print_move(best_move);
	//printf("got score %d\n$$$$$$$$$$$$$$$$\n", tmp);
	//print_board(board);
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


// manages the users turn, game state user input loop
int user_turn(char board[BOARD_SIZE][BOARD_SIZE], COLOR color){
	//clear_old_moves(moves_head);
	get_all_moves(board, color);
	if (moves_head == NULL) return WIN_POS;
	printf(ENTER_YOUR_MOVE);
	char *word1;
	char *command = NULL;
	Move* new_move = malloc(sizeof(Move));
	new_move->dest = malloc(sizeof(Pos) * 2 * BOARD_SIZE);
	new_move->next = NULL;
	int ret_val;
	while (1){
		//printf("> ");
		if (command != NULL) free(command);
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
			char * piece_coor1 = strtok(NULL, " <,>");
			char * piece_coor2 = strtok(NULL, " <,>");
			new_move->piece.col = piece_coor1[0] - 'a';
			new_move->piece.row = atoi(piece_coor2) - 1;
			if (!is_valid_pos(new_move->piece)){
				printf(WRONG_POSITION);
				continue;
			}
			int i = 0;
			char * dest_coor1 = strtok(NULL, " <,>to");
			char * dest_coor2 = strtok(NULL, " <,>to");
			while (dest_coor1 != NULL){
				new_move->dest[i].col = dest_coor1[0] - 'a';
				new_move->dest[i].row = atoi(dest_coor2) - 1;
				if (!is_valid_pos(new_move->dest[i])){
					i = -1;
					break;
				}
				i++;
				dest_coor1 = strtok(NULL, " <,>[]");
				if (dest_coor1 != NULL) dest_coor2 = strtok(NULL, " <,>[]");
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
	free(command);
	clear_old_moves(new_move);
	clear_old_moves(moves_head);
	return ret_val;
}

// Helper func - checks if a piece belongs to the player
int is_valid_piece(char board[BOARD_SIZE][BOARD_SIZE], Move * move, COLOR color){
	if (color == BLACK){
		if (board[move->piece.col][move->piece.row] == BLACK_M || board[move->piece.col][move->piece.row] == BLACK_K) return 1;
	}
	if (color == WHITE) {
		if (board[move->piece.col][move->piece.row] == WHITE_M || board[move->piece.col][move->piece.row] == WHITE_K) return 1;
	}
	return 0;
}

// checks if a move is in the valid moves list
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

// excutes a specific move on the given board
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
		}
		cur.col = move->dest[i].col;
		cur.row = move->dest[i].row;
	}
}

// safety check before starting the game
int is_valid_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int b_num = 0;
	int w_num =0;
	for (int i = 0; i < BOARD_SIZE; i++){
		for (int j = 0; j < BOARD_SIZE; j++){
			if (board[i][j] == BLACK_M || board[i][j] == BLACK_K) b_num++;
			if (board[i][j] == WHITE_M || board[i][j] == WHITE_K) w_num++;
		}
	}
	if (b_num == 0 || w_num == 0 || b_num > 20 || w_num > 20) return 0;
	return 1;
}

int main(void)
{
	char board[BOARD_SIZE][BOARD_SIZE];
	init_board(board);
	//clear_board(board);
	//board[1][1] = WHITE_M;
	//board[5][1] = BLACK_M;
	//board[7][3] = BLACK_M;
	//minimax_depth = 3;
	//get_all_moves(board, BLACK);
	//printf("init board score %d\n", calc_score(board, BLACK));
	printf(WELCOME_TO_DRAUGHTS);
	printf(ENTER_SETTINGS);
	//printf("> ");
	char *command = input2str(stdin);
	int win_pos;

	while (strcmp(command, "quit") != 0){
		if (strcmp(command, "start") == 0){
			if (is_valid_board(board)) break;
			else{
				printf(WROND_BOARD_INITIALIZATION);
				free(command);
				command = input2str(stdin);
				continue;
			}
		}
		exc(command,board);
		free(command);
		//printf("> ");
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
	if (win_pos == 1){
		free(command);
		command = input2str(stdin);
	}
		free(command);
}



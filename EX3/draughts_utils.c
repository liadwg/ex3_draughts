#include "draughts_utils.h"

// safe_malloc/realloc verifies that tht memory allocation succeeded
void * safe_malloc(size_t size){
	void *res = malloc(size);
	if (!res && size != 0){
		printf("Memory allocation failed, terimanting..");
		//if (head != NULL) clear_memory(head);
		abort();
	}
	else return res;
}
#define malloc(x) safe_malloc(x);

void * safe_realloc(void *old_pointer, size_t size){
	void *res = realloc(old_pointer, size);
	if (!old_pointer && size != 0){
		printf("Memory allocation failed, terimanting..");
		//if (head != NULL) clear_memory(head);
		abort();
	}
	else return res;
}
#define realloc(x, y) safe_realloc((x), (y));

// Globals
Move* moves = NULL;
Move* moves_head = NULL;
char curr_piece;
COLOR curr_player;
Move* best_move;

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

int is_EOB(Pos piece, COLOR player){
	if (player == WHITE) return piece.row == BOARD_SIZE - 1;
	else return piece.row == 0;
}

void clear_old_moves(Move* head){
	if (head != NULL){
		clear_old_moves(head->next);
		free(head->dest);
		free(head);
	}
}

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

Pos get_next_diag(Pos from, Pos to){
	Pos res;
	if (from.col - to.col > 0) res.col = to.col - 1;
	else res.col = to.col + 1;
	if (from.row - to.row > 0) res.row = to.row - 1;
	else res.row = to.row + 1;
	return res;
}

int get_capture_moves(Pos start, Pos piece, char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int count, Pos* dests){
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	int found_now = 0, found_ahead;
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

void get_man_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece){
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	int direction = 1, found_ahead;
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


void get_king_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece){
	Move* res = NULL;
	Move* res_head = res;
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	Pos curr;
	int found_ahead;

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
			Pos new_piece = get_next_diag(piece, curr);
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

void print_move(Move* head){
	printf("Move <%c,%d> to <%c,%d>", head->piece.col + 97, head->piece.row + 1, head->dest[0].col + 97, head->dest[0].row + 1);
	for (int i = 1; i < head->captures; i++){
		printf(", <%c,%d>", head->dest[i].col + 97, head->dest[i].row + 1);
	}
	printf("\n");
}

void print_moves(Move* head){
	while (head != NULL){
		print_move(head);
		head = head->next;
	}
}

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

int calc_score(char board[BOARD_SIZE][BOARD_SIZE], COLOR player){
	if (moves_head == NULL) return -100;
	int score = 0, opposites = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++){
			if (is_opposite(player, board[i][j])) opposites++;
			score += get_piece_score(board[i][j], player);
		}
	if (opposites == 0) return 100;
	return score;
}

void duplicate_board(char board1[BOARD_SIZE][BOARD_SIZE], char board2[BOARD_SIZE][BOARD_SIZE]){
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++) board2[i][j] = board1[i][j];
}

int alpha_beta_minimax(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int depth, int alpha, int beta){
	Move* move_list = get_all_moves(board, player);
	Move* curr_move = move_list;
	if (depth == 0 && curr_move == NULL){
		best_move = NULL;
		return -100;
	}
	if (depth == minimax_depth || curr_move == NULL) return calc_score(board, curr_player);

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
			tmp = alpha_beta_minimax(board, (player == 0), depth + 1, alpha, beta);
			if (tmp > alpha){
				alpha = tmp;
				if (depth == 0) best_move = curr_move;
			}
			if (alpha >= beta){
				if (depth != 0) clear_old_moves(move_list);
				else moves_head = move_list;
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
			tmp = alpha_beta_minimax(board, (player == 0), depth + 1, alpha, beta);
			if (tmp < beta){
				beta = tmp;
				if (depth == 0) best_move = curr_move;
			}
			if (alpha >= beta){
				if (depth != 0) clear_old_moves(move_list);
				else moves_head = move_list;
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

// no prunning minimax, not in use
//
//void duplicate_move_list(Move* head, Move* new_head){
//	if (head != NULL) { new_head = malloc(sizeof(Move)); }
//	else { new_head = NULL; }
//	while (head != NULL){
//		new_head->piece.col = head->piece.col;
//		new_head->piece.row = head->piece.row;
//		new_head->captures = head->captures;
//		if (head->captures < 2){
//			new_head->dest[0].col = head->dest[0].col;
//			new_head->dest[0].row = head->dest[0].row;
//		}
//		else for (int i = 0; i < head->captures; i++){
//			new_head->dest[i].col = head->dest[i].col;
//			new_head->dest[i].row = head->dest[i].row;
//		}
//		if (head->next != NULL) { new_head->next = malloc(sizeof(Move)); }
//		else { new_head->next = NULL; }
//		head = head->next;
//		new_head = new_head->next;
//	}
//}
//
//int minimax_rec(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, int depth, int min_max){
//	get_all_moves(board, player);
//	if (moves_head == NULL) return -100;
//	if (depth == minimax_depth) return calc_score(board, curr_player);
//
//	Move* move_list;
//	Move* curr_move;
//	int mm = -1 * min_max * 200, tmp;
//	duplicate_move_list(moves_head, move_list);
//	curr_move = move_list;
//
//	char init_board[BOARD_SIZE][BOARD_SIZE];
//	duplicate_board(board, init_board);
//	while (curr_move != NULL){
//		exc_move(board, curr_move);
//		tmp = minimax_rec(board, (player == 0), depth + 1, -1 * min_max);
//		if (min_max == 1){
//			if (tmp > mm) mm = tmp;
//		}
//		else if (tmp < mm) mm = tmp;
//		curr_move = curr_move->next;
//		duplicate_board(init_board, board);
//	}
//
//	return mm;
//}
//
//Move* run_minimax(char board[BOARD_SIZE][BOARD_SIZE], COLOR player){
//	if (moves_head == NULL) return NULL;
//	if (moves_head->next == NULL) return moves_head;
//
//	Move* move_list;
//	Move* curr_move;
//	Move* best_move;
//	int max = -200, tmp;
//	curr_player = player;
//	duplicate_move_list(moves_head, move_list);
//	curr_move = move_list;
//
//	char init_board[BOARD_SIZE][BOARD_SIZE];
//	duplicate_board(board, init_board);
//	while (curr_move != NULL){
//		exc_move(board, curr_move);
//		tmp = minimax_rec(board, (player == 0), 1, -1);
//		if (tmp > max){
//			max = tmp;
//			best_move = curr_move;
//		}
//		curr_move = curr_move->next;
//		duplicate_board(init_board, board);
//	}
//
//	clear_old_moves(moves_head);
//	moves_head = move_list;
//
//	return best_move;
//}

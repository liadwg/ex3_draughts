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

int is_valid_pos(Pos pos){
	//return (pos.col > 96 && pos.col <= 96 + BOARD_SIZE && pos.row > 0 && pos.row <= BOARD_SIZE);
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

Move* moves = NULL;
Move* moves_head = NULL;

void clear_old_moves(Move* head){
	if (head != NULL){
		clear_old_moves(head->next);
		free(head->dest);
		free(head);
	}
}

void add_move(Pos piece, Pos* dests, int dests_num){
	if (moves == NULL){
		moves = malloc(sizeof(Move));
		moves_head = moves;
	}
	else if (moves->captures < dests_num  && abs(piece.row - dests->row) != 1){
		clear_old_moves(moves_head);
		moves = malloc(sizeof(Move));
		moves_head = moves;
	}
	else if ((moves->captures == 0 && abs(piece.row - dests->row) == 1) || (moves->captures == dests_num && abs(piece.row - dests->row) != 1)){
		moves->next = malloc(sizeof(Move));
		moves = moves->next;
	}
	else return;

	moves->piece.col = piece.col;
	moves->piece.row = piece.row;
	moves->dest = malloc(sizeof(Pos) * dests_num);
	for (int i = 0; i < dests_num; i++){
		moves->dest[i].row = dests[i].row;
		moves->dest[i].col = dests[i].col;
	}

	if (abs(piece.row - dests->row) == 1) moves->captures = 0;
	else moves->captures = dests_num;
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
				found_ahead = get_capture_moves(start, new_piece, board, player, count + 1, new_dests);
				if (found_ahead == 0){
					add_move(start, new_dests, count + 1);
					free(new_dests);
				}
				board[pos[p].row][pos[p].col] = tmp;
			}
		}
	return found_now;
}

void get_man_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player, Pos piece){
	Move* res = NULL;
	Move* res_head = res;
	Pos pos[4] = { { piece.col - 1, piece.row - 1 }, { piece.col + 1, piece.row - 1 }, { piece.col - 1, piece.row + 1 }, { piece.col + 1, piece.row + 1 } };
	int direction = 1, found_ahead;
	if (player == BLACK) direction = -1;

	for (int p = 0; p < 4; p++){
		if (is_valid_pos(pos[p]) && pos[p].row == piece.row + direction && board[pos[p].col][pos[p].row] == EMPTY) add_move(piece, &pos[p], 1);
		else if (is_valid_pos(pos[p]) && is_opposite(player, board[pos[p].col][pos[p].row])){
			char tmp = board[pos[p].row][pos[p].col];
			board[pos[p].row][pos[p].col] = EMPTY;
			Pos new_piece = get_next_diag(piece, pos[p]);
			if (is_valid_pos(new_piece) && board[new_piece.row][new_piece.col] == EMPTY){
				found_ahead = get_capture_moves(piece, new_piece, board, player, 1, &new_piece);
				if (found_ahead == 0) add_move(piece, &new_piece, 1);
			}
			board[pos[p].row][pos[p].col] = tmp;
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
			add_move(piece, &curr, 1);
			curr = get_next_diag(piece, curr);
			// BUG - once dests_num == 1 and |dest-start| != 1 add_moves thinks its 1 capture and not a simple move..
		}
		if (is_valid_pos(curr) && is_opposite(player, board[curr.col][curr.row])){
			char tmp = board[curr.row][curr.col];
			board[curr.row][curr.col] = EMPTY;
			Pos new_piece = get_next_diag(piece, curr);
			if (is_valid_pos(new_piece) && board[new_piece.row][new_piece.col] == EMPTY){
				found_ahead = get_capture_moves(piece, new_piece, board, player, 1, &new_piece);
				if (found_ahead == 0) add_move(piece, &new_piece, 1);
			}
			board[curr.row][curr.col] = tmp;
		}
	}
}

Move * get_all_moves(char board[BOARD_SIZE][BOARD_SIZE], COLOR player){
	clear_old_moves(moves_head);
	Pos p;
	for (int i = 0; i < BOARD_SIZE; i++)
		for (int j = 0; j < BOARD_SIZE; j++)
			if (!is_opposite(player, board[i][j]) && board[i][j] != EMPTY){
		p.row = j;
		p.col = i;
		if (is_king(board[i][j])) get_king_moves(board, player, p);
		else get_man_moves(board, player, p);
			}
	return moves_head;
}

void print_moves(Move* head){
	while (head != NULL){
		printf("Move <%c,%d> to <%c,%d>", head->piece.col + 97, head->piece.row + 1, head->dest[0].col + 97, head->dest[0].row + 1);
		for (int i = 1; i < head->captures; i++){
			printf(", <%c,%d>", head->dest[i].col + 97, head->dest[i].row + 1);
		}
		printf("\n");
		head = head->next;
	}
}
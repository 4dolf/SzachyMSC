#pragma once
#include <bit>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>

#include "board.h"

using namespace std;

enum class node_type_t
{
	exact = 0,
	lower_bound = 1,
	upper_bound = 2,
};
struct transposition_table_entry
{
	transposition_table_entry() = default;
	transposition_table_entry(const transposition_table_entry&) = default;

	node_type_t node_type;
	move_t move;
	int depth;
	int eval;
};

class Computer
{
	const static int max_depth = 99;
	const static int RFP_margin = 75;
	const static int null_move_pruning_cutof = 2;

	const static int pawns_value = 100;
	const static int knight_value = 300;
	const static int bishop_value = 325;
	const static int rook_value = 500;
	const static int queen_value = 900;
	const static int check_mate_eval = 1'000'000;
	const static int endgame_material_start = 12;

	ChessBoard board;
	vector<board_state_t> board_history;
	vector<board_state_t> board_history_search;
	unordered_map<board_state_t, transposition_table_entry> transposition_table;

	long quiet_history[4096];
	move_t killers[max_depth];

	int eval_count = 0;
	int transposition_count = 0;

	atomic<bool> search_canceled;

	int passed_pawn_bonus[7] = { 0, 120, 80, 50, 30, 15, 15 };

	int mobility_scores[30] =
	{
		0, -10, 0, 0, 6, 3, 3, 0, 0,
		-10, 0, 0, 6, 3, 3,
		0, 0, 0, 0, 7, 4, 3, 0, 0,
		0, 0, 0, 7, 4, 3
	};

	int king_attack_scores[30] =
	{
		0, -100, 0, 0, 16, 36, 23, 0, 0,
		-100, 0, 0, 16, 36, 23,
		0, 0, 0, 0, 0, -10, 18, 0, 0,
		0, 0, 0, 0, -10, 18
	};

	int open_file_scores[30] =
	{
		0, -20, 10, 0, 0, 15, 5, 0, 0,
		-20, 10, 0, 0, 15, 5,
		0, 10, 15, 0, 0, 5, 5, 0, 0,
		10, 15, 0, 0, 5, 5
	};

	board_t white_passed_pawn_masks[64];
	board_t black_passed_pawn_masks[64];
	board_t white_king_attack_mask[64];
	board_t black_king_attack_mask[64];

	int pawns_early_square_table_white[64] = {
		 0,  0,  0,  0,  0,  0,  0,  0,
		50, 50, 50, 50, 50, 50, 50, 50,
		10, 10, 20, 30, 30, 20, 10, 10,
		 5,  5, 10, 25, 25, 10,  5,  5,
		 0,  0,  0, 20, 20,  0,  0,  0,
		 5, -5,-10,  0,  0,-10, -5,  5,
		 5, 10, 10,-20,-20, 10, 10,  5,
		 0,  0,  0,  0,  0,  0,  0,  0
	};
	int pawns_end_square_table_white[64] = {
		 0,  0,  0,  0,  0,  0,  0,  0,
		90, 95, 90, 80, 80, 90, 95, 90,
		50, 70, 50, 45, 45, 50, 70, 50,
		23, 25, 23, 18, 18, 23, 25, 23,
		15, 18, 15, 15, 15, 15, 18, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 18, 18, 15, 15, 15,
		 0,  0,  0,  0,  0,  0,  0,  0
	};
	int rooks_square_table_white[64] = {
		 0,  0,  0,  0,  0,  0,  0,  0,
		 5, 10, 10, 10, 10, 10, 10,  5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		-5,  0,  0,  0,  0,  0,  0, -5,
		 0,  0,  0,  5,  5,  0,  0,  0
	};
	int knights_square_table_white[64] = {
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  0,  0,  0,-20,-40,
		-30,  0, 10, 15, 15, 10,  0,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  0, 15, 20, 20, 15,  0,-30,
		-30,  5, 10, 15, 15, 10,  5,-30,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50,
	};
	int bishops_square_table_white[64] = {
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10,  5,  5, 10, 10,  5,  5,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10,  5,  0,  0,  0,  0,  5,-10,
		-20,-10,-10,-10,-10,-10,-10,-20,
	};
	int queens_square_table_white[64] = {
		-20,-10,-10, -5, -5,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5,  5,  5,  5,  0,-10,
		 -5,  0,  5,  5,  5,  5,  0, -5,
		  0,  0,  5,  5,  5,  5,  0, -5,
		-10,  5,  5,  5,  5,  5,  0,-10,
		-10,  0,  5,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20
	};
	int king_early_square_table_white[64] = {
		-80,-70,-70,-70,-70,-70,-70,-80,
		-60,-60,-60,-60,-60,-60,-60,-60,
		-40,-50,-50,-60,-60,-50,-50,-40,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-20,-30,-30,-40,-40,-30,-30,-20,
		-10,-20,-20,-20,-20,-20,-20,-10,
		 20, 20, -5, -5, -5, -5, 20, 20,
		 20, 30, 10,  0,  0, 10, 30, 20
	};
	int king_end_square_table_white[64] = {
		-50,-40,-30,-20,-20,-30,-40,-50,
		-30,-20,-10,  0,  0,-10,-20,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-30,  0,  0,  0,  0,-30,-30,
		-50,-30,-30,-30,-30,-30,-30,-50
	};
	int pawns_early_square_table_black[64];
	int pawns_end_square_table_black[64];
	int rooks_square_table_black[64];
	int knights_square_table_black[64];
	int bishops_square_table_black[64];
	int queens_square_table_black[64];
	int king_early_square_table_black[64];
	int king_end_square_table_black[64];

	void mirror_array(int* a, int* b)
	{
		for (int i = 0; i < 64; i++)
			b[8 * (7 - i / 8) + (i % 8)] = a[i];
	}

	int get_piece_value(piece_t piece)
	{
		switch (piece)
		{
		case piece_t::empty:
			return 0;
		case piece_t::white_king:
			return 0;
		case piece_t::white_pawn:
			return pawns_value;
		case piece_t::white_knight:
			return knight_value;
		case piece_t::white_bishop:
			return bishop_value;
		case piece_t::white_rook:
			return rook_value;
		case piece_t::white_queen:
			return queen_value;
		case piece_t::black_king:
			return 0;
		case piece_t::black_pawn:
			return pawns_value;
		case piece_t::black_knight:
			return knight_value;
		case piece_t::black_bishop:
			return bishop_value;
		case piece_t::black_rook:
			return rook_value;
		case piece_t::black_queen:
			return queen_value;
		default:
			break;
		}
	}

	int count_material()
	{
		int output = 0;
		output += (int)__popcnt64(board.white & board.pawns) * pawns_value;
		output += (int)__popcnt64(board.white & board.knights) * knight_value;
		output += (int)__popcnt64(board.white & board.bishops) * bishop_value;
		output += (int)__popcnt64(board.white & board.rooks) * rook_value;
		output += (int)__popcnt64(board.white & board.queens) * queen_value;

		output -= (int)__popcnt64(board.black & board.pawns) * pawns_value;
		output -= (int)__popcnt64(board.black & board.knights) * knight_value;
		output -= (int)__popcnt64(board.black & board.bishops) * bishop_value;
		output -= (int)__popcnt64(board.black & board.rooks) * rook_value;
		output -= (int)__popcnt64(board.black & board.queens) * queen_value;

		return output;
	}
	// color: true - white, false - black
	float count_endgame_material(bool colour)
	{
		int output = 0;
		if (colour)
		{
			output += (int)__popcnt64(board.white & board.knights) * 1;
			output += (int)__popcnt64(board.white & board.bishops) * 1;
			output += (int)__popcnt64(board.white & board.rooks) * 2;
			output += (int)__popcnt64(board.white & board.queens) * 4;
		}
		else
		{
			output += (int)__popcnt64(board.black & board.knights) * 1;
			output += (int)__popcnt64(board.black & board.bishops) * 1;
			output += (int)__popcnt64(board.black & board.rooks) * 2;
			output += (int)__popcnt64(board.black & board.queens) * 4;
		}

		return output;
	}

	float get_endgame_weight(float endgame_material)
	{
		const float multiplier = 1.0f / endgame_material_start;
		return 1 - min(1.0f, endgame_material * multiplier);
	}

	int compare_table_with_mask(const int table[64], board_t mask)
	{
		int eval = 0;
		int dif;
		square_t square = -1;

		while (mask)
		{
			dif = board.bit_pos(mask);
			mask = mask >> dif >> 1;
			square += dif + 1;
			int mirror_square = 8 * (7 - square / 8) + (square % 8);

			eval += table[mirror_square];
		}
		return eval;
	}

	// color: true - white, false - black
	int evaluate_piece_square_tables(bool color, float endgame_weight)
	{
		/*
		https://www.chessprogramming.org/Piece-Square_Tables
		aprart of incentivising active play piece square tables also implicidly ealuate space and king safety
		*/
		board_t color_mask = color ? board.white : board.black;
		int eval = compare_table_with_mask(color ? knights_square_table_white : knights_square_table_black, color_mask & board.knights);
		eval += compare_table_with_mask(color ? bishops_square_table_white : bishops_square_table_black, color_mask & board.bishops);
		eval += compare_table_with_mask(color ? rooks_square_table_white : rooks_square_table_black, color_mask & board.rooks);
		eval += compare_table_with_mask(color ? queens_square_table_white : queens_square_table_black, color_mask & board.queens);

		int king_early_eval = compare_table_with_mask(color ? king_early_square_table_white : king_early_square_table_black, color_mask & board.kings);
		int king_end_eval = compare_table_with_mask(color ? king_end_square_table_white : king_end_square_table_black, color_mask & board.kings);
		eval += (int)(king_early_eval * (1 - endgame_weight));
		eval += (int)(king_end_eval * endgame_weight);

		int pawns_early_eval = compare_table_with_mask(color ? pawns_early_square_table_white : pawns_early_square_table_black, color_mask & board.pawns);
		int pawns_end_eval = compare_table_with_mask(color ? pawns_end_square_table_white : pawns_end_square_table_black, color_mask & board.pawns);
		eval += (int)(pawns_early_eval * (1 - endgame_weight));
		eval += (int)(pawns_end_eval * endgame_weight);

		return eval;
	}

	constexpr board_t generate_passed_pawn_mask(square_t square, bool color)
	{
		/*
		generates a mask of every square in front of the pawn on surrounding and current file
		if there is no oppont pawns on this mask it means that the pawn has a clear path to promotion
		and should be given extra value
		*/

		int file = square % 8;
		int rank = square / 8;

		if (rank == 7 || rank == 0)
			return 0;

		board_t file_a_mask = 0x0101010101010101;

		board_t file_mask_center = file_a_mask << file;
		board_t file_mask_left = file_a_mask << max(0, file - 1);
		board_t file_mask_right = file_a_mask << min(7, file + 1);

		board_t files_mask = file_mask_left | file_mask_center | file_mask_right;

		board_t up_mask = 0;
		if (color)
			up_mask = ULLONG_MAX << 8 * (rank + 1);
		else
			up_mask = ULLONG_MAX >> 8 * (7 - rank);

		return files_mask & up_mask;
	}

	// color: true - white, false - black
	board_t generate_king_attack_mask(square_t square, bool color)
	{
		/*
		adds another 3 spaces in front of the current king mask
		a simmilar aproch (6 spaces insted of 3) is used by stockfish
		*/
		board_t mask = board.get_attacking_mask_king(square);

		if (color)
		{
			if (square % 8 && square + 15 < 64)
				mask |= 1ull << (square + 15);
			if (square + 16 < 64)
				mask |= 1ull << (square + 16);
			if (square % 8 != 7 && square + 17 < 64)
				mask |= 1ull << (square + 17);
		}
		else
		{
			if (square % 8 && square - 15 >= 0)
				mask |= 1ull << (square - 15);
			if (square - 16 >= 0)
				mask |= 1ull << (square - 16);
			if (square % 8 != 7 && square - 17 >= 0)
				mask |= 1ull << (square - 17);
		}

		return mask;
	}

	// color: true - white, false - black
	board_t get_passed_pawn_pask(square_t square, bool color)
	{
		if (color)
			return white_passed_pawn_masks[square];
		else
			return black_passed_pawn_masks[square];
	}

	// color: true - white, false - black
	int evaluate_passed_pawns(bool color)
	{
		board_t my_color_mask = color ? board.white : board.black;
		board_t enemy_color_mask = !color ? board.white : board.black;
		board_t my_mask = my_color_mask & board.pawns;
		board_t enemy_mask = enemy_color_mask & board.pawns;

		int eval = 0;
		int dif;
		square_t square = -1;

		while (my_mask)
		{
			dif = board.bit_pos(my_mask);
			my_mask = my_mask >> dif >> 1;
			square += dif + 1;

			board_t passed_pawn_mask = get_passed_pawn_pask(square, color);

			if (!(enemy_mask & passed_pawn_mask))
			{
				int squares_to_promotion = color ? 7 - square / 8 : square / 8;
				eval += passed_pawn_bonus[squares_to_promotion];
			}
		}

		return eval;
	}
	int get_mobility_score(piece_t piece, bool endgame)
	{
		return mobility_scores[(int)piece + (endgame ? 15 : 0)];
	}

	int get_king_attack_score(piece_t piece, bool endgame)
	{
		return king_attack_scores[(int)piece + (endgame ? 15 : 0)];
	}

	board_t get_king_mobility(bool color)
	{
		if (color)
			return white_king_attack_mask[board.get_king_pos(true)];
		else
			return black_king_attack_mask[board.get_king_pos(false)];
	}

	// color: true - white, false - black
	int get_mobility_evaluation(bool color, float endgame_weight)
	{
		bool turn_switch = board.white_to_move != color;
		if (turn_switch)
			board.no_move();

		float eval = 0;
		board_t oppponent_king_mobility = get_king_mobility(!color);

		for (move_t chess_move : board.generate_moves())
		{
			piece_t moveing_piece = board.get_piece_type(board.get_move_from(chess_move));
			square_t target_square = board.get_move_to(chess_move);

			// king mobility is calculated seperitly
			if (moveing_piece == piece_t::black_king || moveing_piece == piece_t::white_king)
				continue;

			// if a piece attacks a square next to oppont king it could indicate attacking chances
			// for this reson it is given extra points
			if (oppponent_king_mobility & (1ull << target_square))
			{
				eval += get_king_attack_score(moveing_piece, false) * (1 - endgame_weight);
				eval += get_king_attack_score(moveing_piece, true) * endgame_weight;
			}

			eval += get_mobility_score(moveing_piece, false) * (1 - endgame_weight);
			eval += get_mobility_score(moveing_piece, true) * endgame_weight;
		}

		if (turn_switch)
			board.undo_move();

		return eval;
	}

	int eval_open_file_positioning(bool color, float endgame_weight)
	{
		float eval = 0;

		board_t my_pawns = board.pawns & (color ? board.white : board.black);
		for (square_t square = 0; square < 63; square++)
		{
			piece_t piece = board.get_piece_type(square);
			if (piece == piece_t::empty)
				continue;

			if ((color ? board.black : board.white) & 1ull << square)
				continue;

			board_t file_mask = 0x0101010101010101ull << (square % 8);
			board_t file_without_start_square = file_mask & ~(1ull << square);
			bool is_semi_open = !(file_without_start_square & my_pawns);
			eval += is_semi_open ? open_file_scores[(int)piece] * (1 - endgame_weight) : 0;
			eval += is_semi_open ? open_file_scores[(int)piece + 15] * (endgame_weight) : 0;
		}

		return (int)eval;
	}

	int calculate_king_safety(bool color, float endgame_weight)
	{
		int eval = 0;
		int dif;
		square_t square = -1;
		int my_king_mobility_sum = 0;

		board_t my_king_mobility = get_king_mobility(color);

		while (my_king_mobility)
		{
			dif = board.bit_pos(my_king_mobility);
			my_king_mobility = my_king_mobility >> dif >> 1;
			square += dif + 1;
			my_king_mobility_sum += board.get_piece_type(square) == piece_t::empty ? 1 : 0;

			// the more pawns there are in front of the king the more protection he recieves
			if (color)
				eval += board.get_piece_type(square) == piece_t::white_pawn ? 20 : 0;
			else
				eval += board.get_piece_type(square) == piece_t::black_pawn ? 20 : 0;
		}

		eval += get_mobility_score(color ? piece_t::white_king : piece_t::black_king, false) * (1 - endgame_weight) * my_king_mobility_sum;
		eval += get_mobility_score(color ? piece_t::white_king : piece_t::black_king, true) * endgame_weight * my_king_mobility_sum;

		return eval;
	}

	int evaluate()
	{
		eval_count++;

		board_state_t board_state;
		board.get_board_state(board_state);
		int eval = count_material();

		// some parts of the evaluation can either be good or bad depending on the phaze of the game e.g. king activity
		float white_endgame_weight = get_endgame_weight(count_endgame_material(true));
		float black_endgame_weight = get_endgame_weight(count_endgame_material(false));
		float endgame_weight = (white_endgame_weight + black_endgame_weight) / 2.0f;

		eval += get_mobility_evaluation(true, endgame_weight);
		eval -= get_mobility_evaluation(false, endgame_weight);

		eval += evaluate_piece_square_tables(true, endgame_weight);
		eval -= evaluate_piece_square_tables(false, endgame_weight);

		eval += eval_open_file_positioning(true, endgame_weight);
		eval -= eval_open_file_positioning(false, endgame_weight);

		eval += calculate_king_safety(true, endgame_weight);
		eval -= calculate_king_safety(false, endgame_weight);

		eval += evaluate_passed_pawns(true) * endgame_weight;
		eval -= evaluate_passed_pawns(false) * endgame_weight;

		int tempo_eval = 15;
		eval += board_state.get_turn() ?
			-tempo_eval * (1.0f - endgame_weight) :
			tempo_eval * (1.0f - endgame_weight);

		return board_state.get_turn() ? eval : -eval;
	}


	// mate is evaluated by getting the mate eval and subtracting the amount of moves played.
	// So if we save the mate eval blindly our program will get the mate eval from the perspective of another position
	// To prevent we store the mate eval from the perspective of the stored position and then convert it (in correct_mate_eval_retrive) to the perspective of the new position
	int correct_mate_eval_storage(int eval, int moves_played)
	{
		if (abs(eval) >= check_mate_eval - max_depth)
		{
			if (eval > 0)
				return eval + moves_played;
			else
				return -(-eval + moves_played);
		}
		return eval;
	}

	int correct_mate_eval_retrive(int eval, int moves_played)
	{
		if (abs(eval) >= check_mate_eval - max_depth)
		{
			if (eval > 0)
				return eval - moves_played;
			else
				return -(-eval - moves_played);
		}
		return eval;
	}


	void store_eval(board_state_t& board_state, int depth, int moves_played, int eval, node_type_t node_type, move_t chess_move)
	{
		transposition_table_entry entry;
		entry.depth = depth;
		entry.eval = correct_mate_eval_storage(eval, moves_played);
		entry.node_type = node_type;
		entry.move = chess_move;

		transposition_table[board_state] = entry;
	}

	pair<int, int> lookup_eval(board_state_t& board_state, int depth, int moves_played, int alpha, int beta)
	{
		auto iter = transposition_table.find(board_state);
		if (iter == transposition_table.end())
			return make_pair(-1, 0);

		transposition_table_entry entry = iter->second;
		if (entry.depth >= depth)
		{
			int eval = correct_mate_eval_retrive(entry.eval, moves_played);

			// due to the fact that we are useing alpha-beta pruning we can't just blindly trust in the past evaluation of the position
			if (entry.node_type == node_type_t::exact)
			{
				transposition_count++;
				return make_pair(0, eval);
			}
			if (entry.node_type == node_type_t::upper_bound && eval <= alpha)
			{
				transposition_count++;
				return make_pair(0, eval);
			}
			if (entry.node_type == node_type_t::lower_bound && eval >= beta)
			{
				transposition_count++;
				return make_pair(0, eval);
			}

		}
		return make_pair(-2, 0);
	}

	move_t lookup_move(board_state_t& board_state)
	{
		auto iter = transposition_table.find(board_state);
		if (iter == transposition_table.end())
			return board.encode_move(piece_no_color_t::empty, 0, 0);
		return iter->second.move;
	}

	void order_moves(vector<move_t>& moves, int depth)
	{
		board_state_t board_state;
		board.get_board_state(board_state);
		vector<move_t> new_moves;
		vector<long long> scores;

		move_t transposition_move = lookup_move(board_state);

		for (move_t chess_move : moves)
		{
			long long move_score = 0;
			square_t target_square = board.get_move_to(chess_move);

			piece_t move_piece_type = board_state.get_piece_type(board.get_move_from(chess_move));
			piece_t capture_piece_type = board_state.get_piece_type(target_square);

			if (chess_move == transposition_move)
				move_score -= 5'000'000'000'000ll;

			else if (capture_piece_type != piece_t::empty)
			{
				long long capture_material = 100'000'000ll * (long long)get_piece_value(capture_piece_type) - 1'000ll * (long long)get_piece_value(move_piece_type);
				move_score -= capture_material;
			}

			else if (depth != -1 && chess_move == killers[depth])
				move_score -= 1'000'000ll;
			else
				move_score -= quiet_history[chess_move % 4096];


			auto iter = lower_bound(scores.begin(), scores.end(), move_score);
			int index = distance(scores.begin(), iter);
			new_moves.insert(new_moves.begin() + index, chess_move);
			scores.insert(iter, move_score);
		}

		moves = new_moves;
	}

	// if the position analyzed by the eval funcion still has playable captures we cant trust the eval 
	// as it can drasticly change in just 1 move. this is why at the end of the swarch we run another search with just capture
	// and only at the end the second search we evaluate the position
	int search_captures(int moves_played, int alpha, int beta)
	{
		board_state_t board_state;
		board.get_board_state(board_state);

		// sometimes every capture is bad so we also have to consider doing nothing as a possibility too
		int eval = evaluate();
		int best_eval = eval;
		if (eval >= beta)
			return beta;

		alpha = max(alpha, eval);

		// returns the evaluation if it has already been calculated
		pair<int, int> transposition_table_eval = lookup_eval(board_state, 0, moves_played, alpha, beta);
		if (transposition_table_eval.first == 0)
			return transposition_table_eval.second;

		bool do_delta_pruning = count_endgame_material(true) + count_endgame_material(false) > 2;

		vector<move_t> moves = board.generate_capture_moves();
		order_moves(moves, -1);

		for (move_t chess_move : moves)
		{
			/*
			https://www.chessprogramming.org/Delta_Pruning
			if our current move is so bad that even if our opponent does nothing we still will have a bad possition
			we can save time by not calculateing it
			we can also exclude later moves due to move ordering
			*/
			int capture_piece_value = get_piece_value(board.get_piece_type(board.get_move_to(chess_move)));
			if (do_delta_pruning && eval < best_eval - capture_piece_value - 250)
				break;

			board.move(chess_move);
			int eval = -search_captures(moves_played, -beta, -alpha);
			board.undo_move();

			if (search_canceled)
				return 0;

			if (eval >= beta)
				return beta;

			alpha = max(alpha, eval);
			best_eval = max(best_eval, eval);
		}

		return best_eval;
	}

public:
	Computer()
	{
		board_history = {};
		transposition_table.clear();

		mirror_array(&pawns_early_square_table_white[0], &pawns_early_square_table_black[0]);
		mirror_array(&pawns_end_square_table_white[0], &pawns_end_square_table_black[0]);
		mirror_array(&knights_square_table_white[0], &knights_square_table_black[0]);
		mirror_array(&bishops_square_table_white[0], &bishops_square_table_black[0]);
		mirror_array(&rooks_square_table_white[0], &rooks_square_table_black[0]);
		mirror_array(&queens_square_table_white[0], &queens_square_table_black[0]);
		mirror_array(&king_early_square_table_white[0], &king_early_square_table_black[0]);
		mirror_array(&king_end_square_table_white[0], &king_end_square_table_black[0]);

		for (int i = 0; i < 64; i++)
		{
			white_passed_pawn_masks[i] = generate_passed_pawn_mask(i, true);
			black_passed_pawn_masks[i] = generate_passed_pawn_mask(i, false);
			white_king_attack_mask[i] = generate_king_attack_mask(i, true);
			black_king_attack_mask[i] = generate_king_attack_mask(i, false);
		}
	}

	int get_eval_count()
	{
		return eval_count;
	}

	void set_board(ChessBoard new_board)
	{
		board = new_board;
		board_history = {};
		transposition_table.clear();
	}

	void play_move_on_board(move_t chess_move)
	{
		board_state_t board_state;
		board.get_board_state(board_state);
		board_history.push_back(board_state);
		board.move(chess_move);
	}

	int search(int depth, int moves_played, int alpha, int beta, bool null_move_allowed = true)
	{
		board_state_t board_state;
		board.get_board_state(board_state);

		bool do_pruning = alpha == beta - 1 && !board.in_check();
		int best_eval = -check_mate_eval;
		int eval = evaluate();

		// checks for 3 move repetition
		if (count(board_history.begin(), board_history.end(), board_state) >= 3)
			return 0;

		// we dont need to check until 3 move repetition 
		// if repeting the position once turns out to be the best we can safely assume that 
		// repeting moves is the best aproach
		if (find(board_history_search.begin(), board_history_search.end(), board_state) != board_history_search.end() && null_move_allowed)
			return 0;

		if (board.in_check())
			depth++;

		// returns the evaluation if it has already been calculated
		pair<int, int> transposition_table_eval = lookup_eval(board_state, depth, moves_played, alpha, beta);
		if (transposition_table_eval.first == 0)
			return transposition_table_eval.second;

		//Internal Iterative Reductions
		// the node hasn't been visited yet so it's likely bad
		else if (transposition_table_eval.first == -1 && depth > 3)
			depth--;

		if (depth <= 0)
			return search_captures(moves_played, alpha, beta);

		vector<move_t> moves = board.generate_moves();

		// checks for checkmate and stalemate
		if (moves.size() == 0)
		{
			if (board.in_check())
				return -check_mate_eval + moves_played;
			return 0;
		}

		// Reverse futility pruning
		if (do_pruning && depth < 7 && eval > beta + depth * RFP_margin)
			return eval;

		// null move pruning
		if (do_pruning && null_move_allowed && eval >= beta && depth > 2 && count_endgame_material(true) + count_endgame_material(false) >= null_move_pruning_cutof)
		{
			board.no_move();
			eval = -search(depth - 4, moves_played + 1, -beta, -alpha, false);
			board.undo_move();

			if (eval >= beta)
				return beta;
		}

		order_moves(moves, depth);

		node_type_t current_eval_type = node_type_t::upper_bound;
		move_t best_move = null_move;
		int moves_searched = 0;
		int quiet_moves_evaluated = 0;
		vector<move_t> quiets_evaluated;
		for (move_t chess_move : moves)
		{
			bool is_capture = board.get_piece_type(board.get_move_to(chess_move)) != piece_t::empty;
			bool is_promotion = board.get_promotion(chess_move) != piece_t::empty;

			board.move(chess_move);
			board_history_search.push_back(board_state);

			bool is_quiet = !(is_capture || is_promotion);
			bool succes = false;

			/*
			https://www.chessprogramming.org/Principal_Variation_Search
			if a move is quiet and not the first move (best move from the previous depth)
			its likely bad for this reson we run it on a shalower depth
			and with a null window wich means we are only looking if a move is better not how much
			*/
			if (depth > 2 && moves_searched > 4 && is_quiet && moves_searched != 0)
			{
				eval = -search(depth - 2, moves_played + 1, -alpha - 1, -alpha, true);
				succes = eval <= alpha;
			}

			if (!succes && moves_searched != 0)
			{
				eval = -search(depth - 1, moves_played + 1, -alpha - 1, -alpha, true);
				succes = !(eval > alpha && eval < beta);
			}

			if (!succes)
				eval = -search(depth - 1, moves_played + 1, -beta, -alpha, true);

			board_history_search.pop_back();
			board.undo_move();

			moves_searched++;

			if (search_canceled)
				return best_eval;

			if (eval > best_eval)
			{
				best_move = chess_move;
				best_eval = eval;
			}

			if (eval >= beta)
			{
				if (is_quiet)
				{
					// sience all positions we are analyzeing are more or less the same
					// we can assume that a move that is good in one variation is likely going to be good in a diffrent one
					quiet_history[chess_move % 4096] += depth * depth;

					for (move_t previous_move : quiets_evaluated)
						quiet_history[previous_move % 4096] -= depth * depth;
					killers[moves_played] = chess_move;
				}

				store_eval(board_state, depth, moves_played, beta, node_type_t::lower_bound, chess_move);
				return beta;
			}
			if (eval > alpha)
			{
				alpha = eval;

				best_move = chess_move;
				current_eval_type = node_type_t::exact;
			}

			if (is_quiet)
			{
				quiets_evaluated.push_back(chess_move);
				quiet_moves_evaluated++;
			}

			// https://www.chessprogramming.org/Late_Move_Reductions
			// 
			if (do_pruning && quiet_moves_evaluated > 3 + depth * depth)
				break;
		}

		store_eval(board_state, depth, moves_played, best_eval, current_eval_type, best_move);
		return best_eval;
	}
	// 1ms - 1440 elo
	// 10ms - 1620 elo
	// 100ms - 1780 elo
	// 1000ms - 1930 elo
	// 5000ms - 2060 elo
	pair<move_t, int> deapening_search(chrono::milliseconds time)
	{
		for (int i = 0; i < 4096; i++)
			quiet_history[i] /= 8;

		for (int i = 0; i < max_depth; i++)
			killers[i] = null_move;

		search_canceled = false;

		thread t1([&]()
			{
				this_thread::sleep_for(time);
				search_canceled = true;
			});

		t1.detach();

		eval_count = 0;
		transposition_count = 0;

		board_history_search = {};

		pair<move_t, int> best_move = make_pair(board.generate_moves()[0], 0);

		board_state_t board_state;
		board.get_board_state(board_state);

		int eval = 0;
		for (int depth = 1; depth < max_depth; depth++)
		{
			// https://www.chessprogramming.org/Aspiration_Windows
			int window = 40;
			while (true)
			{
				int alpha = eval - window;
				int	beta = eval + window;

				eval = search(depth, 0, alpha, beta, true);
				move_t new_best_move = lookup_move(board_state);

				if (search_canceled)
					return best_move;

				if (alpha < eval && eval < beta)
				{
					best_move = make_pair(new_best_move, eval);

					cout << "depth: " << depth << ", eval: " << best_move.second << " current best move: " << board.move_t_to_uci(best_move.first);
					cout << ", eval count: " << eval_count << ", transposition count: " << transposition_count << '\n';
					break;
				}
				window *= 2;
			}
		}
	}
};

#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <array>
#include <vector>
#include <tuple>
#include "board.h"

#include <intrin.h>

using namespace std;

enum class piece_t {
    empty = 0,
    white_king = 1,
    white_pawn = 2,
    white_knight = 3,
    white_bishop = 4,
    white_rook = 5,
    white_queen = 6,
    black_king = 9,
    black_pawn = 10,
    black_knight = 11,
    black_bishop = 12,
    black_rook = 13,
    black_queen = 14,
};

enum class piece_no_color_t {
    empty = 0,
    king = 1,
    pawn = 2,
    knight = 3,
    bishop = 4,
    rook = 5,
    queen = 6,
};

enum class color_t {
    none = 0,
    white = 1,
    black = 2
};
// move_t is used to represent a move
// bits 0-5 represent the starting position of moveing piece
// bits 6-11 represent the end position of moveing piece
// bits 12-15 represent the promoted piece (only used in case of a promotion)
// bits 29-32 represent the moveing piece 
using move_t = uint32_t;

// square_t is simply an intiger from 0-63 representing a square
using square_t = uint32_t;

// move_log_t is used when we need to save a move to the log and save more information in case we want to undo the move later
// bits 0-5 represent the starting position of moveing piece
// bits 6-11 represent the end position of moveing piece
// bits 12-15 represent the promoted piece (only used in case of a promotion)
// bits 16-19 represnt the captured piece (only used in case of a capture)
// bits 20-24 represnt the square where a pawn can take en-passant (when a pawn moves 2 squares the square behind the pawn is saved as a en-passant square)
// bits 25-28 represent castle flags
// bits 29-32 represent the moveing piece
// bits 32+ represent the number of moves played sience last capture or pawn move(used by 50 move rule)
using move_log_t = uint64_t;

// board_t is a bitboard where each bit represents whether or not a square is occupied
using board_t = uint64_t;

#define null_move board.encode_move(piece_no_color_t::empty, 0, 0)

// class used to store the currrent position simmilar to fen notation
// its used as a key in a dictionary for transpositions
// sience its faster to generate than fen strings
struct board_state_t
{
    const uint32_t ep_shift = 20;
    const uint32_t castling_shift = 25;
    const uint32_t meta_shift = 32;
    const uint32_t turn_shift = 31;

    const uint64_t ep_mask = 0x1full;
    const uint64_t castling_mask = 0xfull;
    const uint64_t user_mask = 0x7fffffffull;

    const uint64_t ep_empty = 32;

    uint64_t white;
    uint64_t black;
    uint64_t kings;
    uint64_t queens;
    uint64_t rooks;
    uint64_t bishops;
    uint64_t knights;
    uint64_t pawns;
    uint64_t metadata;

    board_state_t() = default;
    board_state_t(const board_state_t&) = default;
    board_state_t& operator=(const board_state_t& x)
    {
        if (this == &x)
            return *this;

        this->white = x.white;
        this->black = x.black;
        this->kings = x.kings;
        this->queens = x.queens;
        this->rooks = x.rooks;
        this->bishops = x.bishops;
        this->knights = x.knights;
        this->pawns = x.pawns;
        this->metadata = x.metadata;

        return *this;
    };

    unsigned get_ep() const
    {
        auto x = ((metadata >> (meta_shift + ep_shift)) & ep_mask) + 16;

        if (x == ep_empty)
            return 0;

        return (unsigned)x;
    }

    unsigned get_castlings() const
    {
        return (unsigned)((metadata >> (meta_shift + castling_shift)) & castling_mask);
    }

    unsigned get_user_data() const
    {
        return (unsigned)(metadata & user_mask);
    }

    // true - white turn
    // false - black turn
    bool get_turn() const
    {
        return (bool)((metadata >> turn_shift) & 1);
    }

    bool operator==(const board_state_t& x) const
    {
        return
            white == x.white &&
            black == x.black &&
            kings == x.kings &&
            queens == x.queens &&
            rooks == x.rooks &&
            bishops == x.bishops &&
            knights == x.knights &&
            pawns == x.pawns &&
            metadata == x.metadata;
    }

    bool operator!=(const board_state_t& x) const
    {
        return !operator==(x);
    }

    // returns true if a piece at pos is on bitboard b
    constexpr bool is_piece(board_t b, square_t pos)
    {
        return b & (1ull << pos);
    }

    // returns true if a white piece is on square pos
    constexpr bool is_square_white(square_t pos)
    {
        return white & (1ull << pos);
    }

    // returns false if a black piece is on square pos
    constexpr bool is_square_black(square_t pos)
    {
        return black & (1ull << pos);
    }

    piece_t get_piece_type(square_t pos)
    {
        if (is_square_white(pos))
        {
            if (is_piece(pawns, pos))
                return piece_t::white_pawn;
            if (is_piece(knights, pos))
                return piece_t::white_knight;
            if (is_piece(bishops, pos))
                return piece_t::white_bishop;
            if (is_piece(rooks, pos))
                return piece_t::white_rook;
            if (is_piece(queens, pos))
                return piece_t::white_queen;
            if (is_piece(kings, pos))
                return piece_t::white_king;
        }
        else if (is_square_black(pos))
        {
            if (is_piece(pawns, pos))
                return piece_t::black_pawn;
            if (is_piece(knights, pos))
                return piece_t::black_knight;
            if (is_piece(bishops, pos))
                return piece_t::black_bishop;
            if (is_piece(rooks, pos))
                return piece_t::black_rook;
            if (is_piece(queens, pos))
                return piece_t::black_queen;
            if (is_piece(kings, pos))
                return piece_t::black_king;
        }

        return piece_t::empty;
    }
};

class ChessBoard {

    const static uint32_t move_shift = 0;
    const static uint32_t move_from_shift = 0;
    const static uint32_t move_to_shift = 6;
    const static uint32_t promotion_shift = 12;
    const static uint32_t takeover_shift = 16;
    const static uint32_t ep_shift = 20;
    const static uint32_t castling_shift = 25;
    const static uint32_t castle_white_short_shift = 25;
    const static uint32_t castle_white_long_shift = 26;
    const static uint32_t castle_black_short_shift = 27;
    const static uint32_t castle_black_long_shift = 28;
    const static uint32_t moving_piece_shift = 29;
    const static uint32_t last_pawn_move_shift = 32;

    const static move_t ep_empty = 32;

    const static move_t half_move_mask = 0x3fu;
    const static move_t full_move_mask = 0xfffu;
    const static move_t ep_mask = 0x1fu;
    const static move_t piece_mask = 0xfu;
    const static move_t moving_piece_mask = 0xfu;
    const static move_t castle_white_short_bits = 1u << castle_white_short_shift;
    const static move_t castle_white_long_bits = 1u << castle_white_long_shift;
    const static move_t castle_black_short_bits = 1u << castle_black_short_shift;
    const static move_t castle_black_long_bits = 1u << castle_black_long_shift;
    const static move_t castling_bits = 0xfu << castling_shift;
    const static move_t castling_mask = 0xfu;
    const static move_t castle_mask = 1u;


    /*
    to speed up move generation moves are precomputed for each piece exept pawns.
    rooks, bishops and queens movement is also split in to moves for every direction.
    sience every method used is constexpr the values are computed at compile time
    */
    board_t attacking_mask_king[64] = {
        king_am(0, 0), king_am(0, 1), king_am(0, 2), king_am(0, 3), king_am(0, 4), king_am(0, 5), king_am(0, 6), king_am(0, 7),
        king_am(1, 0), king_am(1, 1), king_am(1, 2), king_am(1, 3), king_am(1, 4), king_am(1, 5), king_am(1, 6), king_am(1, 7),
        king_am(2, 0), king_am(2, 1), king_am(2, 2), king_am(2, 3), king_am(2, 4), king_am(2, 5), king_am(2, 6), king_am(2, 7),
        king_am(3, 0), king_am(3, 1), king_am(3, 2), king_am(3, 3), king_am(3, 4), king_am(3, 5), king_am(3, 6), king_am(3, 7),
        king_am(4, 0), king_am(4, 1), king_am(4, 2), king_am(4, 3), king_am(4, 4), king_am(4, 5), king_am(4, 6), king_am(4, 7),
        king_am(5, 0), king_am(5, 1), king_am(5, 2), king_am(5, 3), king_am(5, 4), king_am(5, 5), king_am(5, 6), king_am(5, 7),
        king_am(6, 0), king_am(6, 1), king_am(6, 2), king_am(6, 3), king_am(6, 4), king_am(6, 5), king_am(6, 6), king_am(6, 7),
        king_am(7, 0), king_am(7, 1), king_am(7, 2), king_am(7, 3), king_am(7, 4), king_am(7, 5), king_am(7, 6), king_am(7, 7)
    };

    board_t attacking_mask_knight[64] = {
        knight_am(0, 0), knight_am(0, 1), knight_am(0, 2), knight_am(0, 3), knight_am(0, 4), knight_am(0, 5), knight_am(0, 6), knight_am(0, 7),
        knight_am(1, 0), knight_am(1, 1), knight_am(1, 2), knight_am(1, 3), knight_am(1, 4), knight_am(1, 5), knight_am(1, 6), knight_am(1, 7),
        knight_am(2, 0), knight_am(2, 1), knight_am(2, 2), knight_am(2, 3), knight_am(2, 4), knight_am(2, 5), knight_am(2, 6), knight_am(2, 7),
        knight_am(3, 0), knight_am(3, 1), knight_am(3, 2), knight_am(3, 3), knight_am(3, 4), knight_am(3, 5), knight_am(3, 6), knight_am(3, 7),
        knight_am(4, 0), knight_am(4, 1), knight_am(4, 2), knight_am(4, 3), knight_am(4, 4), knight_am(4, 5), knight_am(4, 6), knight_am(4, 7),
        knight_am(5, 0), knight_am(5, 1), knight_am(5, 2), knight_am(5, 3), knight_am(5, 4), knight_am(5, 5), knight_am(5, 6), knight_am(5, 7),
        knight_am(6, 0), knight_am(6, 1), knight_am(6, 2), knight_am(6, 3), knight_am(6, 4), knight_am(6, 5), knight_am(6, 6), knight_am(6, 7),
        knight_am(7, 0), knight_am(7, 1), knight_am(7, 2), knight_am(7, 3), knight_am(7, 4), knight_am(7, 5), knight_am(7, 6), knight_am(7, 7)
    };

    board_t attacking_mask_rook_left[64] = {
        rook_left_am(0, 0), rook_left_am(0, 1), rook_left_am(0, 2), rook_left_am(0, 3), rook_left_am(0, 4), rook_left_am(0, 5), rook_left_am(0, 6), rook_left_am(0, 7),
        rook_left_am(1, 0), rook_left_am(1, 1), rook_left_am(1, 2), rook_left_am(1, 3), rook_left_am(1, 4), rook_left_am(1, 5), rook_left_am(1, 6), rook_left_am(1, 7),
        rook_left_am(2, 0), rook_left_am(2, 1), rook_left_am(2, 2), rook_left_am(2, 3), rook_left_am(2, 4), rook_left_am(2, 5), rook_left_am(2, 6), rook_left_am(2, 7),
        rook_left_am(3, 0), rook_left_am(3, 1), rook_left_am(3, 2), rook_left_am(3, 3), rook_left_am(3, 4), rook_left_am(3, 5), rook_left_am(3, 6), rook_left_am(3, 7),
        rook_left_am(4, 0), rook_left_am(4, 1), rook_left_am(4, 2), rook_left_am(4, 3), rook_left_am(4, 4), rook_left_am(4, 5), rook_left_am(4, 6), rook_left_am(4, 7),
        rook_left_am(5, 0), rook_left_am(5, 1), rook_left_am(5, 2), rook_left_am(5, 3), rook_left_am(5, 4), rook_left_am(5, 5), rook_left_am(5, 6), rook_left_am(5, 7),
        rook_left_am(6, 0), rook_left_am(6, 1), rook_left_am(6, 2), rook_left_am(6, 3), rook_left_am(6, 4), rook_left_am(6, 5), rook_left_am(6, 6), rook_left_am(6, 7),
        rook_left_am(7, 0), rook_left_am(7, 1), rook_left_am(7, 2), rook_left_am(7, 3), rook_left_am(7, 4), rook_left_am(7, 5), rook_left_am(7, 6), rook_left_am(7, 7)
    };

    board_t attacking_mask_rook_right[64] = {
        rook_right_am(0, 0), rook_right_am(0, 1), rook_right_am(0, 2), rook_right_am(0, 3), rook_right_am(0, 4), rook_right_am(0, 5), rook_right_am(0, 6), rook_right_am(0, 7),
        rook_right_am(1, 0), rook_right_am(1, 1), rook_right_am(1, 2), rook_right_am(1, 3), rook_right_am(1, 4), rook_right_am(1, 5), rook_right_am(1, 6), rook_right_am(1, 7),
        rook_right_am(2, 0), rook_right_am(2, 1), rook_right_am(2, 2), rook_right_am(2, 3), rook_right_am(2, 4), rook_right_am(2, 5), rook_right_am(2, 6), rook_right_am(2, 7),
        rook_right_am(3, 0), rook_right_am(3, 1), rook_right_am(3, 2), rook_right_am(3, 3), rook_right_am(3, 4), rook_right_am(3, 5), rook_right_am(3, 6), rook_right_am(3, 7),
        rook_right_am(4, 0), rook_right_am(4, 1), rook_right_am(4, 2), rook_right_am(4, 3), rook_right_am(4, 4), rook_right_am(4, 5), rook_right_am(4, 6), rook_right_am(4, 7),
        rook_right_am(5, 0), rook_right_am(5, 1), rook_right_am(5, 2), rook_right_am(5, 3), rook_right_am(5, 4), rook_right_am(5, 5), rook_right_am(5, 6), rook_right_am(5, 7),
        rook_right_am(6, 0), rook_right_am(6, 1), rook_right_am(6, 2), rook_right_am(6, 3), rook_right_am(6, 4), rook_right_am(6, 5), rook_right_am(6, 6), rook_right_am(6, 7),
        rook_right_am(7, 0), rook_right_am(7, 1), rook_right_am(7, 2), rook_right_am(7, 3), rook_right_am(7, 4), rook_right_am(7, 5), rook_right_am(7, 6), rook_right_am(7, 7)
    };

    board_t attacking_mask_rook_up[64] = {
        rook_up_am(0, 0), rook_up_am(0, 1), rook_up_am(0, 2), rook_up_am(0, 3), rook_up_am(0, 4), rook_up_am(0, 5), rook_up_am(0, 6), rook_up_am(0, 7),
        rook_up_am(1, 0), rook_up_am(1, 1), rook_up_am(1, 2), rook_up_am(1, 3), rook_up_am(1, 4), rook_up_am(1, 5), rook_up_am(1, 6), rook_up_am(1, 7),
        rook_up_am(2, 0), rook_up_am(2, 1), rook_up_am(2, 2), rook_up_am(2, 3), rook_up_am(2, 4), rook_up_am(2, 5), rook_up_am(2, 6), rook_up_am(2, 7),
        rook_up_am(3, 0), rook_up_am(3, 1), rook_up_am(3, 2), rook_up_am(3, 3), rook_up_am(3, 4), rook_up_am(3, 5), rook_up_am(3, 6), rook_up_am(3, 7),
        rook_up_am(4, 0), rook_up_am(4, 1), rook_up_am(4, 2), rook_up_am(4, 3), rook_up_am(4, 4), rook_up_am(4, 5), rook_up_am(4, 6), rook_up_am(4, 7),
        rook_up_am(5, 0), rook_up_am(5, 1), rook_up_am(5, 2), rook_up_am(5, 3), rook_up_am(5, 4), rook_up_am(5, 5), rook_up_am(5, 6), rook_up_am(5, 7),
        rook_up_am(6, 0), rook_up_am(6, 1), rook_up_am(6, 2), rook_up_am(6, 3), rook_up_am(6, 4), rook_up_am(6, 5), rook_up_am(6, 6), rook_up_am(6, 7),
        rook_up_am(7, 0), rook_up_am(7, 1), rook_up_am(7, 2), rook_up_am(7, 3), rook_up_am(7, 4), rook_up_am(7, 5), rook_up_am(7, 6), rook_up_am(7, 7)
    };

    board_t attacking_mask_rook_down[64] = {
        rook_down_am(0, 0), rook_down_am(0, 1), rook_down_am(0, 2), rook_down_am(0, 3), rook_down_am(0, 4), rook_down_am(0, 5), rook_down_am(0, 6), rook_down_am(0, 7),
        rook_down_am(1, 0), rook_down_am(1, 1), rook_down_am(1, 2), rook_down_am(1, 3), rook_down_am(1, 4), rook_down_am(1, 5), rook_down_am(1, 6), rook_down_am(1, 7),
        rook_down_am(2, 0), rook_down_am(2, 1), rook_down_am(2, 2), rook_down_am(2, 3), rook_down_am(2, 4), rook_down_am(2, 5), rook_down_am(2, 6), rook_down_am(2, 7),
        rook_down_am(3, 0), rook_down_am(3, 1), rook_down_am(3, 2), rook_down_am(3, 3), rook_down_am(3, 4), rook_down_am(3, 5), rook_down_am(3, 6), rook_down_am(3, 7),
        rook_down_am(4, 0), rook_down_am(4, 1), rook_down_am(4, 2), rook_down_am(4, 3), rook_down_am(4, 4), rook_down_am(4, 5), rook_down_am(4, 6), rook_down_am(4, 7),
        rook_down_am(5, 0), rook_down_am(5, 1), rook_down_am(5, 2), rook_down_am(5, 3), rook_down_am(5, 4), rook_down_am(5, 5), rook_down_am(5, 6), rook_down_am(5, 7),
        rook_down_am(6, 0), rook_down_am(6, 1), rook_down_am(6, 2), rook_down_am(6, 3), rook_down_am(6, 4), rook_down_am(6, 5), rook_down_am(6, 6), rook_down_am(6, 7),
        rook_down_am(7, 0), rook_down_am(7, 1), rook_down_am(7, 2), rook_down_am(7, 3), rook_down_am(7, 4), rook_down_am(7, 5), rook_down_am(7, 6), rook_down_am(7, 7)
    };

    board_t attacking_mask_bishop_right_up[64] = {
        bishop_right_up_am(0, 0), bishop_right_up_am(0, 1), bishop_right_up_am(0, 2), bishop_right_up_am(0, 3), bishop_right_up_am(0, 4), bishop_right_up_am(0, 5), bishop_right_up_am(0, 6), bishop_right_up_am(0, 7),
        bishop_right_up_am(1, 0), bishop_right_up_am(1, 1), bishop_right_up_am(1, 2), bishop_right_up_am(1, 3), bishop_right_up_am(1, 4), bishop_right_up_am(1, 5), bishop_right_up_am(1, 6), bishop_right_up_am(1, 7),
        bishop_right_up_am(2, 0), bishop_right_up_am(2, 1), bishop_right_up_am(2, 2), bishop_right_up_am(2, 3), bishop_right_up_am(2, 4), bishop_right_up_am(2, 5), bishop_right_up_am(2, 6), bishop_right_up_am(2, 7),
        bishop_right_up_am(3, 0), bishop_right_up_am(3, 1), bishop_right_up_am(3, 2), bishop_right_up_am(3, 3), bishop_right_up_am(3, 4), bishop_right_up_am(3, 5), bishop_right_up_am(3, 6), bishop_right_up_am(3, 7),
        bishop_right_up_am(4, 0), bishop_right_up_am(4, 1), bishop_right_up_am(4, 2), bishop_right_up_am(4, 3), bishop_right_up_am(4, 4), bishop_right_up_am(4, 5), bishop_right_up_am(4, 6), bishop_right_up_am(4, 7),
        bishop_right_up_am(5, 0), bishop_right_up_am(5, 1), bishop_right_up_am(5, 2), bishop_right_up_am(5, 3), bishop_right_up_am(5, 4), bishop_right_up_am(5, 5), bishop_right_up_am(5, 6), bishop_right_up_am(5, 7),
        bishop_right_up_am(6, 0), bishop_right_up_am(6, 1), bishop_right_up_am(6, 2), bishop_right_up_am(6, 3), bishop_right_up_am(6, 4), bishop_right_up_am(6, 5), bishop_right_up_am(6, 6), bishop_right_up_am(6, 7),
        bishop_right_up_am(7, 0), bishop_right_up_am(7, 1), bishop_right_up_am(7, 2), bishop_right_up_am(7, 3), bishop_right_up_am(7, 4), bishop_right_up_am(7, 5), bishop_right_up_am(7, 6), bishop_right_up_am(7, 7)
    };

    board_t attacking_mask_bishop_right_down[64] = {
        bishop_right_down_am(0, 0), bishop_right_down_am(0, 1), bishop_right_down_am(0, 2), bishop_right_down_am(0, 3), bishop_right_down_am(0, 4), bishop_right_down_am(0, 5), bishop_right_down_am(0, 6), bishop_right_down_am(0, 7),
        bishop_right_down_am(1, 0), bishop_right_down_am(1, 1), bishop_right_down_am(1, 2), bishop_right_down_am(1, 3), bishop_right_down_am(1, 4), bishop_right_down_am(1, 5), bishop_right_down_am(1, 6), bishop_right_down_am(1, 7),
        bishop_right_down_am(2, 0), bishop_right_down_am(2, 1), bishop_right_down_am(2, 2), bishop_right_down_am(2, 3), bishop_right_down_am(2, 4), bishop_right_down_am(2, 5), bishop_right_down_am(2, 6), bishop_right_down_am(2, 7),
        bishop_right_down_am(3, 0), bishop_right_down_am(3, 1), bishop_right_down_am(3, 2), bishop_right_down_am(3, 3), bishop_right_down_am(3, 4), bishop_right_down_am(3, 5), bishop_right_down_am(3, 6), bishop_right_down_am(3, 7),
        bishop_right_down_am(4, 0), bishop_right_down_am(4, 1), bishop_right_down_am(4, 2), bishop_right_down_am(4, 3), bishop_right_down_am(4, 4), bishop_right_down_am(4, 5), bishop_right_down_am(4, 6), bishop_right_down_am(4, 7),
        bishop_right_down_am(5, 0), bishop_right_down_am(5, 1), bishop_right_down_am(5, 2), bishop_right_down_am(5, 3), bishop_right_down_am(5, 4), bishop_right_down_am(5, 5), bishop_right_down_am(5, 6), bishop_right_down_am(5, 7),
        bishop_right_down_am(6, 0), bishop_right_down_am(6, 1), bishop_right_down_am(6, 2), bishop_right_down_am(6, 3), bishop_right_down_am(6, 4), bishop_right_down_am(6, 5), bishop_right_down_am(6, 6), bishop_right_down_am(6, 7),
        bishop_right_down_am(7, 0), bishop_right_down_am(7, 1), bishop_right_down_am(7, 2), bishop_right_down_am(7, 3), bishop_right_down_am(7, 4), bishop_right_down_am(7, 5), bishop_right_down_am(7, 6), bishop_right_down_am(7, 7)
    };

    board_t attacking_mask_bishop_left_down[64] = {
        bishop_left_down_am(0, 0), bishop_left_down_am(0, 1), bishop_left_down_am(0, 2), bishop_left_down_am(0, 3), bishop_left_down_am(0, 4), bishop_left_down_am(0, 5), bishop_left_down_am(0, 6), bishop_left_down_am(0, 7),
        bishop_left_down_am(1, 0), bishop_left_down_am(1, 1), bishop_left_down_am(1, 2), bishop_left_down_am(1, 3), bishop_left_down_am(1, 4), bishop_left_down_am(1, 5), bishop_left_down_am(1, 6), bishop_left_down_am(1, 7),
        bishop_left_down_am(2, 0), bishop_left_down_am(2, 1), bishop_left_down_am(2, 2), bishop_left_down_am(2, 3), bishop_left_down_am(2, 4), bishop_left_down_am(2, 5), bishop_left_down_am(2, 6), bishop_left_down_am(2, 7),
        bishop_left_down_am(3, 0), bishop_left_down_am(3, 1), bishop_left_down_am(3, 2), bishop_left_down_am(3, 3), bishop_left_down_am(3, 4), bishop_left_down_am(3, 5), bishop_left_down_am(3, 6), bishop_left_down_am(3, 7),
        bishop_left_down_am(4, 0), bishop_left_down_am(4, 1), bishop_left_down_am(4, 2), bishop_left_down_am(4, 3), bishop_left_down_am(4, 4), bishop_left_down_am(4, 5), bishop_left_down_am(4, 6), bishop_left_down_am(4, 7),
        bishop_left_down_am(5, 0), bishop_left_down_am(5, 1), bishop_left_down_am(5, 2), bishop_left_down_am(5, 3), bishop_left_down_am(5, 4), bishop_left_down_am(5, 5), bishop_left_down_am(5, 6), bishop_left_down_am(5, 7),
        bishop_left_down_am(6, 0), bishop_left_down_am(6, 1), bishop_left_down_am(6, 2), bishop_left_down_am(6, 3), bishop_left_down_am(6, 4), bishop_left_down_am(6, 5), bishop_left_down_am(6, 6), bishop_left_down_am(6, 7),
        bishop_left_down_am(7, 0), bishop_left_down_am(7, 1), bishop_left_down_am(7, 2), bishop_left_down_am(7, 3), bishop_left_down_am(7, 4), bishop_left_down_am(7, 5), bishop_left_down_am(7, 6), bishop_left_down_am(7, 7)
    };

    board_t attacking_mask_bishop_left_up[64] = {
        bishop_left_up_am(0, 0), bishop_left_up_am(0, 1), bishop_left_up_am(0, 2), bishop_left_up_am(0, 3), bishop_left_up_am(0, 4), bishop_left_up_am(0, 5), bishop_left_up_am(0, 6), bishop_left_up_am(0, 7),
        bishop_left_up_am(1, 0), bishop_left_up_am(1, 1), bishop_left_up_am(1, 2), bishop_left_up_am(1, 3), bishop_left_up_am(1, 4), bishop_left_up_am(1, 5), bishop_left_up_am(1, 6), bishop_left_up_am(1, 7),
        bishop_left_up_am(2, 0), bishop_left_up_am(2, 1), bishop_left_up_am(2, 2), bishop_left_up_am(2, 3), bishop_left_up_am(2, 4), bishop_left_up_am(2, 5), bishop_left_up_am(2, 6), bishop_left_up_am(2, 7),
        bishop_left_up_am(3, 0), bishop_left_up_am(3, 1), bishop_left_up_am(3, 2), bishop_left_up_am(3, 3), bishop_left_up_am(3, 4), bishop_left_up_am(3, 5), bishop_left_up_am(3, 6), bishop_left_up_am(3, 7),
        bishop_left_up_am(4, 0), bishop_left_up_am(4, 1), bishop_left_up_am(4, 2), bishop_left_up_am(4, 3), bishop_left_up_am(4, 4), bishop_left_up_am(4, 5), bishop_left_up_am(4, 6), bishop_left_up_am(4, 7),
        bishop_left_up_am(5, 0), bishop_left_up_am(5, 1), bishop_left_up_am(5, 2), bishop_left_up_am(5, 3), bishop_left_up_am(5, 4), bishop_left_up_am(5, 5), bishop_left_up_am(5, 6), bishop_left_up_am(5, 7),
        bishop_left_up_am(6, 0), bishop_left_up_am(6, 1), bishop_left_up_am(6, 2), bishop_left_up_am(6, 3), bishop_left_up_am(6, 4), bishop_left_up_am(6, 5), bishop_left_up_am(6, 6), bishop_left_up_am(6, 7),
        bishop_left_up_am(7, 0), bishop_left_up_am(7, 1), bishop_left_up_am(7, 2), bishop_left_up_am(7, 3), bishop_left_up_am(7, 4), bishop_left_up_am(7, 5), bishop_left_up_am(7, 6), bishop_left_up_am(7, 7)
    };

    board_t attacking_mask_rook[64] = {
        rook_am(0, 0), rook_am(0, 1), rook_am(0, 2), rook_am(0, 3), rook_am(0, 4), rook_am(0, 5), rook_am(0, 6), rook_am(0, 7),
        rook_am(1, 0), rook_am(1, 1), rook_am(1, 2), rook_am(1, 3), rook_am(1, 4), rook_am(1, 5), rook_am(1, 6), rook_am(1, 7),
        rook_am(2, 0), rook_am(2, 1), rook_am(2, 2), rook_am(2, 3), rook_am(2, 4), rook_am(2, 5), rook_am(2, 6), rook_am(2, 7),
        rook_am(3, 0), rook_am(3, 1), rook_am(3, 2), rook_am(3, 3), rook_am(3, 4), rook_am(3, 5), rook_am(3, 6), rook_am(3, 7),
        rook_am(4, 0), rook_am(4, 1), rook_am(4, 2), rook_am(4, 3), rook_am(4, 4), rook_am(4, 5), rook_am(4, 6), rook_am(4, 7),
        rook_am(5, 0), rook_am(5, 1), rook_am(5, 2), rook_am(5, 3), rook_am(5, 4), rook_am(5, 5), rook_am(5, 6), rook_am(5, 7),
        rook_am(6, 0), rook_am(6, 1), rook_am(6, 2), rook_am(6, 3), rook_am(6, 4), rook_am(6, 5), rook_am(6, 6), rook_am(6, 7),
        rook_am(7, 0), rook_am(7, 1), rook_am(7, 2), rook_am(7, 3), rook_am(7, 4), rook_am(7, 5), rook_am(7, 6), rook_am(7, 7)
    };

    board_t attacking_mask_bishop[64] = {
        bishop_am(0, 0), bishop_am(0, 1), bishop_am(0, 2), bishop_am(0, 3), bishop_am(0, 4), bishop_am(0, 5), bishop_am(0, 6), bishop_am(0, 7),
        bishop_am(1, 0), bishop_am(1, 1), bishop_am(1, 2), bishop_am(1, 3), bishop_am(1, 4), bishop_am(1, 5), bishop_am(1, 6), bishop_am(1, 7),
        bishop_am(2, 0), bishop_am(2, 1), bishop_am(2, 2), bishop_am(2, 3), bishop_am(2, 4), bishop_am(2, 5), bishop_am(2, 6), bishop_am(2, 7),
        bishop_am(3, 0), bishop_am(3, 1), bishop_am(3, 2), bishop_am(3, 3), bishop_am(3, 4), bishop_am(3, 5), bishop_am(3, 6), bishop_am(3, 7),
        bishop_am(4, 0), bishop_am(4, 1), bishop_am(4, 2), bishop_am(4, 3), bishop_am(4, 4), bishop_am(4, 5), bishop_am(4, 6), bishop_am(4, 7),
        bishop_am(5, 0), bishop_am(5, 1), bishop_am(5, 2), bishop_am(5, 3), bishop_am(5, 4), bishop_am(5, 5), bishop_am(5, 6), bishop_am(5, 7),
        bishop_am(6, 0), bishop_am(6, 1), bishop_am(6, 2), bishop_am(6, 3), bishop_am(6, 4), bishop_am(6, 5), bishop_am(6, 6), bishop_am(6, 7),
        bishop_am(7, 0), bishop_am(7, 1), bishop_am(7, 2), bishop_am(7, 3), bishop_am(7, 4), bishop_am(7, 5), bishop_am(7, 6), bishop_am(7, 7)
    };

    board_t attacking_mask_queen[64] = {
        queen_am(0, 0), queen_am(0, 1), queen_am(0, 2), queen_am(0, 3), queen_am(0, 4), queen_am(0, 5), queen_am(0, 6), queen_am(0, 7),
        queen_am(1, 0), queen_am(1, 1), queen_am(1, 2), queen_am(1, 3), queen_am(1, 4), queen_am(1, 5), queen_am(1, 6), queen_am(1, 7),
        queen_am(2, 0), queen_am(2, 1), queen_am(2, 2), queen_am(2, 3), queen_am(2, 4), queen_am(2, 5), queen_am(2, 6), queen_am(2, 7),
        queen_am(3, 0), queen_am(3, 1), queen_am(3, 2), queen_am(3, 3), queen_am(3, 4), queen_am(3, 5), queen_am(3, 6), queen_am(3, 7),
        queen_am(4, 0), queen_am(4, 1), queen_am(4, 2), queen_am(4, 3), queen_am(4, 4), queen_am(4, 5), queen_am(4, 6), queen_am(4, 7),
        queen_am(5, 0), queen_am(5, 1), queen_am(5, 2), queen_am(5, 3), queen_am(5, 4), queen_am(5, 5), queen_am(5, 6), queen_am(5, 7),
        queen_am(6, 0), queen_am(6, 1), queen_am(6, 2), queen_am(6, 3), queen_am(6, 4), queen_am(6, 5), queen_am(6, 6), queen_am(6, 7),
        queen_am(7, 0), queen_am(7, 1), queen_am(7, 2), queen_am(7, 3), queen_am(7, 4), queen_am(7, 5), queen_am(7, 6), queen_am(7, 7)
    };

    constexpr board_t king_am(int row, int col)
    {
        board_t mask = 0;
        if (row > 0)
            mask |= square_mask(row - 1, col);
        if (row < 7)
            mask |= square_mask(row + 1, col);
        if (col > 0)
            mask |= square_mask(row, col - 1);
        if (col < 7)
            mask |= square_mask(row, col + 1);
        if (row > 0 && col > 0)
            mask |= square_mask(row - 1, col - 1);
        if (row > 0 && col < 7)
            mask |= square_mask(row - 1, col + 1);
        if (row < 7 && col > 0)
            mask |= square_mask(row + 1, col - 1);
        if (row < 7 && col < 7)
            mask |= square_mask(row + 1, col + 1);

        return mask;
    }

    constexpr board_t knight_am(int row, int col)
    {
        board_t mask = 0;
        if (row - 2 >= 0 && col - 1 >= 0)
            mask |= square_mask(row - 2, col - 1);
        if (row - 2 >= 0 && col + 1 <= 7)
            mask |= square_mask(row - 2, col + 1);
        if (row - 1 >= 0 && col - 2 >= 0)
            mask |= square_mask(row - 1, col - 2);
        if (row - 1 >= 0 && col + 2 <= 7)
            mask |= square_mask(row - 1, col + 2);
        if (row + 1 <= 7 && col - 2 >= 0)
            mask |= square_mask(row + 1, col - 2);
        if (row + 1 <= 7 && col + 2 <= 7)
            mask |= square_mask(row + 1, col + 2);
        if (row + 2 <= 7 && col - 1 >= 0)
            mask |= square_mask(row + 2, col - 1);
        if (row + 2 <= 7 && col + 1 <= 7)
            mask |= square_mask(row + 2, col + 1);

        return mask;
    }

    constexpr board_t rook_left_am(int row, int col)
    {
        board_t mask = 0;

        for (int i = 0; i < col; ++i)
            bit_set(mask, row, i);

        return mask;
    }

    constexpr board_t rook_right_am(int row, int col)
    {
        board_t mask = 0;

        for (int i = col + 1; i < 8; ++i)
            bit_set(mask, row, i);

        return mask;
    }

    constexpr board_t rook_up_am(int row, int col)
    {
        board_t mask = 0;

        for (int i = row + 1; i < 8; ++i)
            bit_set(mask, i, col);

        return mask;
    }

    constexpr board_t rook_down_am(int row, int col)
    {
        board_t mask = 0;

        for (int i = 0; i < row; ++i)
            bit_set(mask, i, col);

        return mask;
    }

    constexpr board_t bishop_right_up_am(int row, int col)
    {
        board_t mask = 0;

        for (++row, ++col; row < 8 && col < 8; ++row, ++col)
            bit_set(mask, row, col);

        return mask;
    }

    constexpr board_t bishop_right_down_am(int row, int col)
    {
        board_t mask = 0;

        for (--row, ++col; row >= 0 && col < 8; --row, ++col)
            bit_set(mask, row, col);

        return mask;
    }

    constexpr board_t bishop_left_down_am(int row, int col)
    {
        board_t mask = 0;

        for (--row, --col; row >= 0 && col >= 0; --row, --col)
            bit_set(mask, row, col);

        return mask;
    }

    constexpr board_t bishop_left_up_am(int row, int col)
    {
        board_t mask = 0;

        for (++row, --col; row < 8 && col >= 0; ++row, --col)
            bit_set(mask, row, col);

        return mask;
    }

    constexpr board_t rook_am(int row, int col)
    {
        return rook_left_am(row, col) | rook_right_am(row, col) | rook_up_am(row, col) | rook_down_am(row, col);
    }

    constexpr board_t bishop_am(int row, int col)
    {
        return bishop_right_up_am(row, col) | bishop_right_down_am(row, col) | bishop_left_down_am(row, col) | bishop_left_up_am(row, col);
    }

    constexpr board_t queen_am(int row, int col)
    {
        return rook_am(row, col) | bishop_am(row, col);
    }

    // sets pos_from bit to false and pot_to bit to true on bitboard b
    constexpr board_t move_piece_in_bit_mask(board_t b, square_t pos_from, square_t pos_to)
    {
        b &= ~(1ull << pos_from);
        b |= 1ull << pos_to;

        return b;
    }

    constexpr bool is_rook_left_attacked(board_t attacker_mask, board_t mask_blockers, square_t test_pos)
    {
        board_t mask = attacking_mask_rook_left[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_rook_left[test_pos];

        return bit_pos_msb(mask_blockers) < bit_pos_msb(mask);
    }

    constexpr bool is_rook_left_valid(int pos_from_col, square_t pos_from, square_t pos_to)
    {
        board_t mask = attacking_mask_rook_left[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (rooks | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_rook_left[king_pos];

        return bit_pos_msb(mask_blockers) > bit_pos_msb(mask);
    }

    constexpr bool is_rook_right_attacked(board_t attacker_mask, board_t mask_blockers, square_t test_pos)
    {
        board_t mask = attacking_mask_rook_right[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_rook_right[test_pos];

        return bit_pos_lsb(mask_blockers) > bit_pos_lsb(mask);
    }

    constexpr bool is_rook_right_valid(int pos_from_col, square_t pos_from, square_t pos_to)
    {
        board_t mask = attacking_mask_rook_right[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (rooks | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_rook_right[king_pos];

        return bit_pos_lsb(mask_blockers) < bit_pos_lsb(mask);
    }

    constexpr bool is_rook_up_attacked(board_t attacker_mask, board_t mask_blockers, square_t test_pos)
    {
        board_t mask = attacking_mask_rook_up[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_rook_up[test_pos];

        return bit_pos_lsb(mask_blockers) > bit_pos_lsb(mask);
    }

    constexpr bool is_rook_up_valid(int pos_from_row, square_t pos_from, square_t pos_to)
    {
        board_t mask = attacking_mask_rook_up[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (rooks | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_rook_up[king_pos];

        return bit_pos_lsb(mask_blockers) < bit_pos_lsb(mask);
    }

    constexpr bool is_rook_down_attacked(board_t attacker_mask, board_t mask_blockers, square_t test_pos)
    {
        board_t mask = attacking_mask_rook_down[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_rook_down[test_pos];

        return bit_pos_msb(mask_blockers) < bit_pos_msb(mask);
    }

    constexpr bool is_rook_down_valid(int pos_from_row, int pos_from, int pos_to)
    {
        board_t mask = attacking_mask_rook_down[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (rooks | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_rook_down[king_pos];

        return bit_pos_msb(mask_blockers) > bit_pos_msb(mask);
    }

    constexpr bool is_bishop_right_down_attacked(board_t attacker_mask, board_t mask_blockers, int test_pos)
    {
        board_t mask = attacking_mask_bishop_right_down[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_bishop_right_down[test_pos];

        return bit_pos_msb(mask_blockers) < bit_pos_msb(mask);
    }

    constexpr bool is_bishop_right_down_valid(int pos_from_col, int pos_from, int pos_to)
    {
        board_t mask = attacking_mask_bishop_right_down[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (bishops | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_bishop_right_down[king_pos];

        return bit_pos_msb(mask_blockers) > bit_pos_msb(mask);
    }

    constexpr bool is_bishop_right_up_attacked(board_t attacker_mask, board_t mask_blockers, int test_pos)
    {
        board_t mask = attacking_mask_bishop_right_up[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_bishop_right_up[test_pos];

        return bit_pos_lsb(mask_blockers) > bit_pos_lsb(mask);
    }

    constexpr bool is_bishop_right_up_valid(int pos_from_col, int pos_from, int pos_to)
    {
        board_t mask = attacking_mask_bishop_right_up[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (bishops | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_bishop_right_up[king_pos];

        return bit_pos_lsb(mask_blockers) < bit_pos_lsb(mask);
    }

    constexpr bool is_bishop_left_down_attacked(board_t attacker_mask, board_t mask_blockers, int test_pos)
    {
        board_t mask = attacking_mask_bishop_left_down[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_bishop_left_down[test_pos];

        return bit_pos_msb(mask_blockers) < bit_pos_msb(mask);
    }

    constexpr bool is_bishop_left_down_valid(int pos_from_col, int pos_from, int pos_to)
    {
        board_t mask = attacking_mask_bishop_left_down[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (bishops | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_bishop_left_down[king_pos];

        return bit_pos_msb(mask_blockers) > bit_pos_msb(mask);
    }

    constexpr bool is_bishop_left_up_attacked(board_t attacker_mask, board_t mask_blockers, int test_pos)
    {
        board_t mask = attacking_mask_bishop_left_up[test_pos];

        mask &= attacker_mask;

        if (!mask)
            return false;

        mask_blockers &= attacking_mask_bishop_left_up[test_pos];

        return bit_pos_lsb(mask_blockers) > bit_pos_lsb(mask);
    }

    constexpr bool is_bishop_left_up_valid(int pos_from_col, int pos_from, int pos_to)
    {
        board_t mask = attacking_mask_bishop_left_up[king_pos];
        board_t attacker_mask = (white_to_move ? black : white) & (bishops | queens);
        bit_reset(attacker_mask, pos_to);

        mask &= attacker_mask;

        if (!mask)
            return true;

        board_t mask_blockers = (black | white) & ~attacker_mask;
        mask_blockers = move_piece_in_bit_mask(mask_blockers, pos_from, pos_to);
        mask_blockers &= attacking_mask_bishop_left_up[king_pos];

        return bit_pos_lsb(mask_blockers) < bit_pos_lsb(mask);
    }

    // allows to rule out the possibility that a move reveals an attack on the king fast
    // true - for sure it is valid
    // false - do not know
    constexpr bool is_move_valid_fast_unsure(square_t pos_from, square_t pos_to)
    {
        if (all_safe)
            return true;

        if (king_in_check)
            return false;

        if (!(b_need_inspection & (1ull << pos_from)))
            return true;

        board_t takeover_mask = ~(1ull << pos_to);
        if (!(attacking_mask_rook[pos_from] & (opp_color_mask & (queens | rooks) & takeover_mask)) && !(attacking_mask_bishop[pos_from] & (opp_color_mask & (queens | bishops) & takeover_mask)))
            return true;

        return false;
    }

    // checks if a move reveals an attack on the king
    constexpr bool is_move_valid(square_t pos_from, square_t pos_to)
    {
        int pos_from_row = get_row(pos_from);
        int pos_from_col = get_col(pos_from);

        if (pos_from_row == king_pos_row)
        {
            if (pos_from_col < king_pos_col)
                return is_rook_left_valid(pos_from_col, pos_from, pos_to);
            else
                return is_rook_right_valid(pos_from_col, pos_from, pos_to);
        }

        if (pos_from_col == king_pos_col)
        {
            if (pos_from_row < king_pos_row)
                return is_rook_down_valid(pos_from_row, pos_from, pos_to);
            else
                return is_rook_up_valid(pos_from_row, pos_from, pos_to);
        }

        int dif_row = pos_from_row - king_pos_row;
        int dif_col = pos_from_col - king_pos_col;

        if (dif_row == dif_col)
        {
            if (dif_row > 0)
                return is_bishop_right_up_valid(pos_from_col, pos_from, pos_to);
            else
                return is_bishop_left_down_valid(pos_from_col, pos_from, pos_to);
        }

        if (dif_row == -dif_col)
        {
            if (dif_row > 0)
                return is_bishop_left_up_valid(pos_from_col, pos_from, pos_to);
            else
                return is_bishop_right_down_valid(pos_from_col, pos_from, pos_to);
        }

        return true;
    }

    // checks if a move reveals an attack on the king
    // if the king is already in check this method could return the weong result
    // wich is if the king is in check every move is double check at the end of the generate_moves funcion
    void add_move_with_control(vector<move_t>& valid_moves, piece_no_color_t moving_piece, int pos_from, int pos_to)
    {
        if (is_move_valid_fast_unsure(pos_from, pos_to) || is_move_valid(pos_from, pos_to))
            valid_moves.emplace_back(encode_move(moving_piece, pos_from, pos_to));
    }

    // same method as add_move_with_control exept for en-pessant
    void add_move_ep_with_control(vector<move_t>& valid_moves, piece_no_color_t moving_piece, int pos_from, int pos_to)
    {
        int takeover_pos;

        if (pos_to > pos_from)
            takeover_pos = pos_to - 8;
        else
            takeover_pos = pos_to + 8;

        bit_reset(pawns, takeover_pos);
        if (white_to_move)
            bit_reset(black, takeover_pos);
        else
            bit_reset(white, takeover_pos);

        bool valid_val1 = is_move_valid_fast_unsure(pos_from, pos_to) || is_move_valid(pos_from, pos_to);
        bit_set(pawns, takeover_pos);
        if (white_to_move)
            bit_set(black, takeover_pos);
        else
            bit_set(white, takeover_pos);

        if (!valid_val1)
            return;

        bit_reset(pawns, pos_from);
        if (white_to_move)
            bit_reset(white, pos_from);
        else
            bit_reset(black, pos_from);

        bool valid_val2 = is_move_valid_fast_unsure(takeover_pos, pos_to) || is_move_valid(takeover_pos, pos_to);
        bit_set(pawns, pos_from);
        if (white_to_move)
            bit_set(white, pos_from);
        else
            bit_set(black, pos_from);

        if (valid_val2)
            valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to));
    }

    // same method as add_move_with_control exept for promotion
    void add_move_pr_with_control(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, piece_no_color_t moving_piece, int pos_from, int pos_to, color_t piece_color)
    {
        if (is_move_valid_fast_unsure(pos_from, pos_to) || is_move_valid(pos_from, pos_to))
        {
            if (piece_color == color_t::white)
            {
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::white_queen));
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::white_rook));
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::white_bishop));
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::white_knight));
            }
            else
            {
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::black_queen));
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::black_rook));
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::black_bishop));
                valid_moves.push_back(encode_move(moving_piece, pos_from, pos_to, piece_t::black_knight));
            }
        }
    }

    constexpr move_t get_move(move_t x) {
        return (x >> move_shift) & full_move_mask;
    }

    constexpr piece_t get_takeover(move_log_t x) {
        return (piece_t)((x >> takeover_shift) & piece_mask);
    }

    constexpr piece_no_color_t get_moving_piece(move_log_t x) {
        return (piece_no_color_t)((x >> moving_piece_shift) & moving_piece_mask);
    }

    constexpr uint32_t get_last_pawn_move(move_log_t x)
    {
        return (uint32_t)(x >> last_pawn_move_shift);
    }

    constexpr void set_castlings(uint32_t castling_bits)
    {
        castlings |= castling_bits;
    }

    constexpr void reset_castlings(uint32_t castling_bits)
    {
        castlings &= ~castling_bits;
    }

    constexpr move_t encode_ep(uint32_t row, uint32_t col)
    {
        return (row * 8 + col);
    }

    constexpr uint32_t get_ep_row(move_t ep)
    {
        return ep / 8;
    }

    constexpr uint32_t get_ep_col(move_t ep)
    {
        return ep % 8;
    }

    constexpr move_log_t encode_move_log(move_t move, piece_t piece, move_t castlings, square_t en_passant, uint32_t last_pawn_move)
    {
        return move + ((move_log_t)piece << takeover_shift) +
            (move_log_t)(castlings)+
            (move_log_t)((en_passant - 16) << ep_shift) +
            (((move_log_t)last_pawn_move) << last_pawn_move_shift);
    }

    void bit_move(board_t& b, uint32_t pos_from, uint32_t pos_to)
    {
        bit_set(b, pos_to);
        bit_reset(b, pos_from);
    }

    constexpr void bit_set(board_t& b, uint32_t pos)
    {
        b |= 1ull << pos;
    }

    constexpr void bit_set(board_t& b, uint32_t row, uint32_t col)
    {
        b |= 1ull << (8 * row + col);
    }

    constexpr void bit_reset(board_t& b, uint32_t pos)
    {
        b &= ~(1ull << pos);
    }

    constexpr bool is_bit_set(board_t& b, int pos)
    {
        return b & (1ull << pos);
    }

    void bit_move(board_t& b1, board_t& b2, uint32_t pos_from, uint32_t pos_to)
    {
        board_t m_from = ~(1ull << pos_from);
        board_t m_to = 1ull << pos_to;

        b1 |= m_to;
        b2 |= m_to;

        b1 &= m_from;
        b2 &= m_from;
    }

    void bit_set(board_t& b1, board_t& b2, uint32_t pos)
    {
        b1 |= 1ull << pos;
        b2 |= 1ull << pos;
    }

    void bit_reset(board_t& b1, board_t& b2, uint32_t pos)
    {
        b1 &= ~(1ull << pos);
        b2 &= ~(1ull << pos);
    }

    void clear() {
        white = black = kings = queens = rooks = bishops = knights = pawns = 0;
        castlings = 0;
        en_passant = ep_empty;
        white_in_check = false;
        black_in_check = false;
    }

    int bit_pos_lsb(uint64_t x)
    {
        unsigned long index;

        auto r = _BitScanForward64(&index, x);

        if (!r)
            return 64;

        return index;
    }

    int bit_pos_msb(uint64_t x)
    {
        unsigned long index;

        auto r = _BitScanReverse64(&index, x);

        if (!r)
            return -1;

        return index;
    }

    vector<string> split(string s);

    void generate_pawn_white_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos, bool only_captures = false);
    void generate_pawn_black_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos, bool only_captures = false);

    void generate_knight_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);
    void generate_knight_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);

    void generate_bishop_all_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);
    void generate_bishop_capture_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);

    void generate_rook_all_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);
    void generate_rook_capture_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);

    void generate_queen_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);
    void generate_queen_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);

    void generate_king_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);
    void generate_king_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos);

    void add_en_passant(vector<move_t>& valid_moves, vector<move_t>& candidate_moves);

    void add_castleing(vector<move_t>& valid_moves, vector<move_t>& candidate_moves);

    constexpr bool is_same_diagonal_or_line(int pos1, int pos2_row, int pos2_col)
    {
        int pos1_row = get_row(pos1);
        int pos1_col = get_col(pos1);

        if (pos1_row == pos2_row)
            return true;
        if (pos1_col == pos2_col)
            return true;

        return abs(pos1_row - pos2_row) == abs(pos1_col - pos2_col);
    }

    constexpr bool is_en_passant_takeover(uint32_t pos_to, piece_no_color_t moving_piece)
    {
        return en_passant != ep_empty && en_passant == pos_to && moving_piece == piece_no_color_t::pawn;
    }

    constexpr bool is_en_passant_takeover(move_t move)
    {
        return is_en_passant_takeover(get_move_to(move), get_moving_piece(move));
    }

    constexpr bool is_same_diagonal_or_line(int pos1, int pos2)
    {
        return is_same_diagonal_or_line(pos1, pos2 / 8, pos2 % 8);
    }

    void test_board()
    {
        if (white & black)
        {
            visualise();
            int a;
            cin >> a;
            cout << a << "!";
        }
    }

    int king_pos, king_pos_row, king_pos_col;
    int opp_king_pos;
    board_t opp_king_attacking_mask;
    bool white_in_check;
    bool black_in_check;
    bool king_in_check;

    board_t color_mask, opp_color_mask;
    color_t piece_color;

    board_t b_attacking_queens;
    board_t b_attacking_rooks;
    board_t b_attacking_bishops;
    board_t b_need_inspection;
    bool all_safe;

    // allows faster acces a bitboard since we dont need to use a switch statement 
    array<uint64_t*, 16> boards;

    void init_boards()
    {
        for (int i = 0; i < 16; ++i)
            boards[i] = nullptr;

        boards[(int)piece_t::white_king] = &kings;
        boards[(int)piece_t::black_king] = &kings;
        boards[(int)piece_t::white_queen] = &queens;
        boards[(int)piece_t::black_queen] = &queens;
        boards[(int)piece_t::white_rook] = &rooks;
        boards[(int)piece_t::black_rook] = &rooks;
        boards[(int)piece_t::white_bishop] = &bishops;
        boards[(int)piece_t::black_bishop] = &bishops;
        boards[(int)piece_t::white_knight] = &knights;
        boards[(int)piece_t::black_knight] = &knights;
        boards[(int)piece_t::white_pawn] = &pawns;
        boards[(int)piece_t::black_pawn] = &pawns;
    }

public:
    board_t white;
    board_t black;
    board_t kings;
    board_t queens;
    board_t rooks;
    board_t bishops;
    board_t knights;
    board_t pawns;

    unsigned castlings;
    unsigned en_passant;
    int last_pawn_move;
    bool white_to_move;
    vector<move_log_t> move_log;

    ChessBoard() {
        clear();
        init_boards();

    }

    ChessBoard(const ChessBoard& rhs)
    {
        white = rhs.white;
        black = rhs.black;
        kings = rhs.kings;
        queens = rhs.queens;
        rooks = rhs.rooks;
        bishops = rhs.bishops;
        knights = rhs.knights;
        pawns = rhs.pawns;
        castlings = rhs.castlings;
        en_passant = rhs.en_passant;

        init_boards();

        king_pos = rhs.king_pos;
        king_pos_row = rhs.king_pos_row;
        king_pos_col = rhs.king_pos_col;
        opp_king_pos = rhs.opp_king_pos;
        opp_king_attacking_mask = rhs.opp_king_attacking_mask;
        white_in_check = rhs.white_in_check;
        black_in_check = rhs.black_in_check;
        king_in_check = rhs.king_in_check;

        color_mask = rhs.color_mask;
        opp_color_mask = rhs.opp_color_mask;
        piece_color = rhs.piece_color;

        b_attacking_queens = rhs.b_attacking_queens;
        b_attacking_rooks = rhs.b_attacking_rooks;
        b_attacking_bishops = rhs.b_attacking_bishops;
        b_need_inspection = rhs.b_need_inspection;
        all_safe = rhs.all_safe;

        last_pawn_move = rhs.last_pawn_move;
        move_log = rhs.move_log;
        white_to_move = rhs.white_to_move;
    }

    ChessBoard& operator=(const ChessBoard& x)
    {
        white = x.white;
        black = x.black;
        kings = x.kings;
        queens = x.queens;
        rooks = x.rooks;
        bishops = x.bishops;
        knights = x.knights;
        pawns = x.pawns;
        castlings = x.castlings;
        en_passant = x.en_passant;

        init_boards();

        king_pos = x.king_pos;
        king_pos_row = x.king_pos_row;
        king_pos_col = x.king_pos_col;
        opp_king_pos = x.opp_king_pos;
        opp_king_attacking_mask = x.opp_king_attacking_mask;
        white_in_check = x.white_in_check;
        black_in_check = x.black_in_check;
        king_in_check = x.king_in_check;

        color_mask = x.color_mask;
        opp_color_mask = x.opp_color_mask;
        piece_color = x.piece_color;

        b_attacking_queens = x.b_attacking_queens;
        b_attacking_rooks = x.b_attacking_rooks;
        b_attacking_bishops = x.b_attacking_bishops;
        b_need_inspection = x.b_need_inspection;
        all_safe = x.all_safe;

        last_pawn_move = x.last_pawn_move;
        move_log = x.move_log;
        white_to_move = x.white_to_move;

        return *this;
    }

    ChessBoard(ChessBoard&&) = delete;

    ChessBoard& operator=(ChessBoard&&) = delete;

    board_t get_attacking_mask_king(square_t square)
    {
        return attacking_mask_king[square];
    }

    board_t get_attacking_mask_kinght(square_t square)
    {
        return attacking_mask_knight[square];
    }

    board_t get_attacking_mask_bishop(square_t square)
    {
        return attacking_mask_bishop[square];
    }

    board_t get_attacking_mask_rook(square_t square)
    {
        return attacking_mask_rook[square];
    }

    board_t get_attacking_mask_queen(square_t square)
    {
        return attacking_mask_queen[square];
    }

    int bit_pos(uint64_t x)
    {
        return bit_pos_lsb(x);
    }

    constexpr bool is_square_occupied(uint32_t pos)
    {
        return (white | black) & (1ull << pos);
    }

    constexpr bool is_white_pawn(uint32_t pos)
    {
        return (white & pawns) & (1ull << pos);
    }

    constexpr bool is_black_pawn(uint32_t pos)
    {
        return (black & pawns) & (1ull << pos);
    }

    constexpr bool is_piece(board_t b, uint32_t pos)
    {
        return b & (1ull << pos);
    }

    constexpr board_t square_mask(int row, int col)
    {
        return 1ull << (8 * row + col);
    }

    constexpr move_t get_ep(move_t x) {
        return ((x >> ep_shift) & ep_mask) + 16;
    }

    constexpr move_t get_castlings(move_t x)
    {
        return x & castling_bits;
    }

    constexpr move_t get_castle_white_short(move_t x)
    {
        return x & castle_white_short_bits;
    }

    constexpr move_t get_castle_white_long(move_t x)
    {
        return x & castle_white_long_bits;
    }

    constexpr move_t get_castle_black_short(move_t x)
    {
        return x & castle_black_short_bits;
    }

    constexpr move_t get_castle_black_long(move_t x)
    {
        return x & castle_black_long_bits;
    }

    constexpr uint32_t get_row(uint32_t pos) {
        return pos / 8;
    }

    constexpr uint32_t get_col(uint32_t pos) {
        return pos % 8;
    }

    int get_king_pos(bool white_king)
    {
        return bit_pos(kings & (white_king ? white : black));
    }

    constexpr bool is_square_white(uint32_t pos)
    {
        return white & (1ull << pos);
    }

    constexpr bool is_square_black(uint32_t pos)
    {
        return black & (1ull << pos);
    }


    bool in_check();

    // converts a move from move_t to uci notation (*start square* *end square*)
    // for example: e2e4
    string move_t_to_uci(move_t chess_move)
    {
        string s = "";
        square_t square = get_move_from(chess_move);
        s.push_back((char)(square % 8 + (int)'a'));
        s.push_back((char)(square / 8 + (int)'1'));
        square = get_move_to(chess_move);
        s.push_back((char)(square % 8 + (int)'a'));
        s.push_back((char)(square / 8 + (int)'1'));


        piece_t piece = get_promotion(chess_move);
        if (piece != piece_t::empty)
            s.push_back("   nbrq    nbrq"[(int)piece]);

        return s;
    }

    /// converts a move from uci notation to move_t
    move_t uci_to_move_t(string uci_move)
    {
        string promotion_board = " kpnbrq";
        int promotion = 0;

        if (uci_move.size() == 5)
            promotion = (int)promotion_board.find(uci_move[4]);

        if (promotion && uci_move[1] == '1')
            promotion += 8;

        auto piece = get_piece_type(((int)uci_move[1] - '1') * 8 + ((int)uci_move[0] - 'a'));

        move_t chess_move = encode_move((piece_no_color_t)(((int)piece) % 8),
            ((int)uci_move[1] - '1') * 8 + ((int)uci_move[0] - 'a'),
            ((int)uci_move[3] - '1') * 8 + ((int)uci_move[2] - 'a'),
            (piece_t)promotion);

        return chess_move;
    }

    constexpr move_t get_move_from(move_t x) {
        return (x >> move_from_shift) & half_move_mask;
    }

    constexpr move_t get_move_to(move_t x) {
        return (x >> move_to_shift) & half_move_mask;
    }

    constexpr piece_t get_promotion(move_t x) {
        return (piece_t)((x >> promotion_shift) & piece_mask);
    }

    piece_t get_piece_type(uint32_t pos)
    {
        if (is_square_white(pos))
        {
            if (is_piece(pawns, pos))
                return piece_t::white_pawn;
            if (is_piece(knights, pos))
                return piece_t::white_knight;
            if (is_piece(bishops, pos))
                return piece_t::white_bishop;
            if (is_piece(rooks, pos))
                return piece_t::white_rook;
            if (is_piece(queens, pos))
                return piece_t::white_queen;
            if (is_piece(kings, pos))
                return piece_t::white_king;
        }
        else if (is_square_black(pos))
        {
            if (is_piece(pawns, pos))
                return piece_t::black_pawn;
            if (is_piece(knights, pos))
                return piece_t::black_knight;
            if (is_piece(bishops, pos))
                return piece_t::black_bishop;
            if (is_piece(rooks, pos))
                return piece_t::black_rook;
            if (is_piece(queens, pos))
                return piece_t::black_queen;
            if (is_piece(kings, pos))
                return piece_t::black_king;
        }

        return piece_t::empty;
    }

    constexpr move_t encode_move(piece_no_color_t moving_piece, uint32_t from, uint32_t to, piece_t promotion)
    {
        return ((uint32_t)moving_piece << moving_piece_shift) +
            (from << move_from_shift) +
            (to << move_to_shift) +
            ((uint32_t)promotion << promotion_shift);
    }

    constexpr move_t encode_move(piece_no_color_t moving_piece, uint32_t from, uint32_t to)
    {
        return ((uint32_t)moving_piece << moving_piece_shift) +
            (from << move_from_shift) +
            (to << move_to_shift);
    }

    void from_fen(string fen);
    string get_fen();
    void visualise();

    inline bool is_attacked(uint32_t pos, bool white_move);

    vector<move_t> generate_capture_moves();
    vector<move_t> generate_moves();

    void move(move_t move);
    void undo_move();
    void no_move();


    void get_board_state(board_state_t& state)
    {
        state.white = white;
        state.black = black;
        state.kings = kings;
        state.queens = queens;
        state.rooks = rooks;
        state.bishops = bishops;
        state.knights = knights;
        state.pawns = pawns;
        state.metadata = ((uint64_t)castlings) << 32;
        state.metadata += ((uint64_t)(en_passant - 16)) << (32 + ep_shift);
        state.metadata += ((uint64_t)white_to_move) << 31;
    }
};

template <>
struct hash<board_state_t>
{
    size_t operator()(const board_state_t& k) const
    {
        return
            hash<uint64_t>()((k.white * 3) ^ (k.black * 5) ^ (k.kings * 7) ^ (k.queens * 11) ^ (k.rooks * 13) ^ (k.bishops * 17) ^ (k.knights * 19) ^ (k.pawns * 23) ^ (k.metadata * 29));
    }
};
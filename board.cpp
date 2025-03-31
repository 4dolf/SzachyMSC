#include "board.h"

vector<string> ChessBoard::split(string s) {
    auto p = s.begin();
    vector<string> components;
    while (true) {
        auto q = find(p, s.end(), ' ');
        components.push_back(string(p, q));
        if (q == s.end())
            break;

        p = q + 1;

    }
    return components;
}

void ChessBoard::generate_pawn_white_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos, bool only_captures)
{
    int row = get_row(pos);
    int col = get_col(pos);

    // If in check, the moves must be examined later
    // If not in check, some moves are validated in this function
    vector<move_t>& proposed_moves = (!white_in_check) ? valid_moves : candidate_moves;

    if (!only_captures && !is_square_occupied(pos + 8)) {
        if (row == 1 && !is_square_occupied(pos + 16)) {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 8);
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 16);
        }
        else if (row == 6) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos + 8, color_t::white);
        }
        else
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 8);
    }

    if (col > 0 && is_square_black(pos + 7)) {
        if (row == 6) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos + 7, color_t::white);
        }
        else
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 7);
    }
    if (col < 7 && is_square_black(pos + 9)) {
        if (row == 6) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos + 9, color_t::white);
        }
        else
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 9);
    }
}

void ChessBoard::generate_pawn_black_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos, bool only_captures)
{
    int row = get_row(pos);
    int col = get_col(pos);

    // If in check, the moves must be examined later
    // If not in check, some moves are validated in this function
    vector<move_t>& proposed_moves = (!black_in_check) ? valid_moves : candidate_moves;

    if (!only_captures && !is_square_occupied(pos - 8)) {
        if (row == 6 && !is_square_occupied(pos - 16)) {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 8);
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 16);
        }
        else if (row == 1) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos - 8, color_t::black);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 8);
        }
    }

    if (col > 0 && is_square_white(pos - 9)) {
        if (row == 1) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos - 9, color_t::black);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 9);
        }
    }

    if (col < 7 && is_square_white(pos - 7)) {
        if (row == 1) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos - 7, color_t::black);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 7);
        }
    }
}

void ChessBoard::generate_rook_all_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    board_t pieces = white | black;
    board_t opp = white_to_move ? black : white;
    int m, k;

    board_t mask = attacking_mask_rook_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask && is_bit_set(opp, m))
        m += 8;

    for (k = pos + 8; k < m; k += 8)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    mask = attacking_mask_rook_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask && is_bit_set(opp, m))
        m -= 8;

    for (k = pos - 8; k > m; k -= 8)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    mask = attacking_mask_rook_right[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            m += 1;
    }
    else
        m = (pos & ~0x7u) + 8;

    for (k = pos + 1; k < m; ++k)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    mask = attacking_mask_rook_left[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            --m;
    }
    else
        m = (int)(pos & ~7u) - (int)1;

    for (k = pos - 1; k > m; --k)
        add_move_with_control(proposed_moves, moving_piece, pos, k);
}

void ChessBoard::generate_rook_capture_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    board_t pieces = white | black;
    board_t opp = white_to_move ? black : white;
    int m, k;

    board_t mask = attacking_mask_rook_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask && is_bit_set(opp, m))
        add_move_with_control(proposed_moves, moving_piece, pos, m);

    mask = attacking_mask_rook_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask && is_bit_set(opp, m))
        add_move_with_control(proposed_moves, moving_piece, pos, m);

    mask = attacking_mask_rook_right[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    mask = attacking_mask_rook_left[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }
}

void ChessBoard::generate_bishop_all_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // If in check, the moves must be examined later
    // If not in check, some moves are validated in this function
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    board_t pieces = white | black;
    board_t opp = white_to_move ? black : white;
    int m, k;

    board_t mask;

    int row = get_row(pos);
    int col = get_col(pos);

    mask = attacking_mask_bishop_right_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            m += 9;
    }
    else
    {
        m = (8 - (pos & 7u)) * 9 + pos;

        if (m > 63)
            m = 64;
    }

    for (k = pos + 9; k < m; k += 9)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    mask = attacking_mask_bishop_left_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            m -= 9;
    }
    else
    {
        m = (int)pos - (int)(pos & 7u) * 9 - 9;

        if (m < 0)
            m = -1;
    }

    for (k = pos - 9; k > m; k -= 9)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    mask = attacking_mask_bishop_left_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            m += 7;
    }
    else
    {
        m = (int)pos + (int)(pos & 7u) * 7 + 7;

        if (m > 63)
            m = 64;
    }

    for (k = pos + 7; k < m; k += 7)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    mask = attacking_mask_bishop_right_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            m -= 7;
    }
    else
    {
        m = (int)pos - (int)(8 - (pos & 7u)) * 7;

        if (m < 0)
            m = -1;
    }

    for (k = pos - 7; k > m; k -= 7)
        add_move_with_control(proposed_moves, moving_piece, pos, k);
}

void ChessBoard::generate_bishop_capture_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // If in check, the moves must be examined later
    // If not in check, some moves are validated in this function
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    board_t pieces = white | black;
    board_t opp = white_to_move ? black : white;
    int m, k;

    board_t mask;

    int row = get_row(pos);
    int col = get_col(pos);

    mask = attacking_mask_bishop_right_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    mask = attacking_mask_bishop_left_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    mask = attacking_mask_bishop_left_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    mask = attacking_mask_bishop_right_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }
}

void ChessBoard::generate_queen_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    generate_rook_all_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos);
    generate_bishop_all_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos);
}

void ChessBoard::generate_queen_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    generate_rook_capture_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos);
    generate_bishop_capture_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos);
}

void ChessBoard::generate_knight_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    for (board_t b = ~color_mask & attacking_mask_knight[pos]; b; b &= b - 1)
        add_move_with_control(proposed_moves, piece_no_color_t::knight, pos, bit_pos(b));
}

void ChessBoard::generate_knight_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    board_t opp = white_to_move ? black : white;

    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    for (board_t b = ~color_mask & attacking_mask_knight[pos]; b; b &= b - 1)
        if (is_bit_set(opp, bit_pos(b)))
            add_move_with_control(proposed_moves, piece_no_color_t::knight, pos, bit_pos(b));
}

void ChessBoard::generate_king_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    board_t& pieces = (white_to_move ? white : black);
    board_t old_kings = kings;
    board_t old_pieces = pieces;

    for (board_t b = ~color_mask & attacking_mask_king[pos]; b; b &= b - 1)
    {
        int new_pos = bit_pos(b);

        kings = move_piece_in_bit_mask(kings, pos, new_pos);
        pieces = move_piece_in_bit_mask(pieces, pos, new_pos);
        if (!is_attacked(new_pos, white_to_move))
            valid_moves.push_back(encode_move(piece_no_color_t::king, pos, bit_pos(b)));

        kings = old_kings;
        pieces = old_pieces;
    }
}

void ChessBoard::generate_king_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    board_t& pieces = (white_to_move ? white : black);
    board_t old_kings = kings;
    board_t old_pieces = pieces;
    board_t opp = white_to_move ? black : white;

    for (board_t b = ~color_mask & attacking_mask_king[pos]; b; b &= b - 1)
    {
        int new_pos = bit_pos(b);

        if (!is_bit_set(opp, new_pos))
            continue;

        kings = move_piece_in_bit_mask(kings, pos, new_pos);
        pieces = move_piece_in_bit_mask(pieces, pos, new_pos);
        if (!is_attacked(new_pos, white_to_move))
            valid_moves.push_back(encode_move(piece_no_color_t::king, pos, bit_pos(b)));

        kings = old_kings;
        pieces = old_pieces;
    }
}

void ChessBoard::from_fen(string fen) {
    clear();

    vector<string> components = split(fen);

    board_t mask = 1ull << 56;
    board_t base_mask = 1ull << 56;

    for (auto c : components.front()) {
        switch (c)
        {
        case 'k':
            black |= mask;
            kings |= mask;
            mask <<= 1;
            break;

        case 'p':
            black |= mask;
            pawns |= mask;
            mask <<= 1;
            break;

        case 'n':
            black |= mask;
            knights |= mask;
            mask <<= 1;
            break;

        case 'b':
            black |= mask;
            bishops |= mask;
            mask <<= 1;
            break;

        case 'r':
            black |= mask;
            rooks |= mask;
            mask <<= 1;
            break;

        case 'q':
            black |= mask;
            queens |= mask;
            mask <<= 1;
            break;

        case 'K':
            white |= mask;
            kings |= mask;
            mask <<= 1;
            break;

        case 'P':
            white |= mask;
            pawns |= mask;
            mask <<= 1;
            break;

        case 'N':
            white |= mask;
            knights |= mask;
            mask <<= 1;
            break;

        case 'B':
            white |= mask;
            bishops |= mask;
            mask <<= 1;
            break;

        case 'R':
            white |= mask;
            rooks |= mask;
            mask <<= 1;
            break;

        case 'Q':
            white |= mask;
            queens |= mask;
            mask <<= 1;
            break;

        case '/':
            base_mask >>= 8;
            mask = base_mask;
            break;

        default:
            mask <<= c - '0';
        }
    }

    if (components[1] == "w")
        white_to_move = true;
    else
        white_to_move = false;

    reset_castlings(castle_white_short_bits | castle_white_long_bits | castle_black_short_bits | castle_black_long_bits);

    for (char component : components[2]) {
        if (component == 'K')
            set_castlings(castle_white_short_bits);
        if (component == 'Q')
            set_castlings(castle_white_long_bits);
        if (component == 'k')
            set_castlings(castle_black_short_bits);
        if (component == 'q')
            set_castlings(castle_black_long_bits);
    }

    if (components[3] == "-")
        en_passant = ep_empty;
    else {
        en_passant = encode_ep(components[3][1] - '1', components[3][0] - 'a');
    }
}

string ChessBoard::get_fen()
{
    string fen = "";
    int dif = 0;

    for (int i = 0; i < 64; i++)
    {
        int file = i % 8;
        int rank = 7 - (i / 8);
        square_t square = rank * 8 + file;

        if (square % 8 == 0 && square != 56)
        {
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('/');
        }

        switch (get_piece_type(square))
        {
        case piece_t::empty:
            dif++;
            break;

        case piece_t::white_pawn:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('P');
            break;

        case piece_t::white_knight:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('N');
            break;

        case piece_t::white_bishop:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('B');
            break;

        case piece_t::white_rook:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('R');
            break;

        case piece_t::white_queen:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('Q');
            break;

        case piece_t::white_king:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('K');
            break;

        case piece_t::black_pawn:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('p');
            break;

        case piece_t::black_knight:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('n');
            break;

        case piece_t::black_bishop:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('b');
            break;

        case piece_t::black_rook:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('r');
            break;

        case piece_t::black_queen:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('q');
            break;

        case piece_t::black_king:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('k');
            break;
        default:
            break;
        }
    }

    fen.push_back(' ');

    if (white_to_move)
        fen.push_back('w');
    else
        fen.push_back('b');

    fen.push_back(' ');

    if (get_castle_white_short(castlings))
        fen.push_back('K');
    if (get_castle_white_long(castlings))
        fen.push_back('Q');
    if (get_castle_white_short(castlings))
        fen.push_back('k');
    if (get_castle_black_long(castlings))
        fen.push_back('q');
    if (!castlings)
        fen.push_back('-');

    fen.push_back(' ');

    if (en_passant == ep_empty)
        fen.push_back('-');
    else
    {
        int file = en_passant % 8;
        int rank = en_passant / 8;

        fen.push_back("abcdefgh"[file]);
        fen.push_back("12345678"[rank]);

    }

    fen.push_back(' ');
    fen += to_string(last_pawn_move);
    fen.push_back(' ');
    fen += to_string(move_log.size());

    return fen;
}

void ChessBoard::add_en_passant(vector<move_t>& valid_moves, vector<move_t>& candidate_moves)
{
    if (en_passant == ep_empty)
        return;

    vector<move_t>& proposed_moves = ((white_to_move && !white_in_check) || (!white_to_move && !black_in_check)) ? valid_moves : candidate_moves;

    uint32_t ep_row = get_ep_row(en_passant);
    uint32_t ep_col = get_ep_col(en_passant);
    uint32_t pos = en_passant;

    if (ep_row == 5) {
        if (ep_col < 7 && is_white_pawn(pos - 7))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos - 7, pos);
        if (ep_col > 0 && is_white_pawn(pos - 9))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos - 9, pos);
    }
    if (ep_row == 2) {
        if (ep_col < 7 && is_black_pawn(pos + 9))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos + 9, pos);
        if (ep_col > 0 && is_black_pawn(pos + 7))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos + 7, pos);
    }
}

void ChessBoard::add_castleing(vector<move_t>& valid_moves, vector<move_t>& candidate_moves)
{
    if (white_to_move && get_castle_white_short(castlings) && !is_square_occupied(5) && !is_square_occupied(6) && !is_attacked(5, true) && !is_attacked(6, true))
        valid_moves.push_back(encode_move(piece_no_color_t::king, 4, 6));
    if (white_to_move && get_castle_white_long(castlings) && !is_square_occupied(3) && !is_square_occupied(2) && !is_square_occupied(1) && !is_attacked(3, true) && !is_attacked(2, true))
        valid_moves.push_back(encode_move(piece_no_color_t::king, 4, 2));
    if (!white_to_move && get_castle_black_short(castlings) && !is_square_occupied(61) && !is_square_occupied(62) && !is_attacked(61, false) && !is_attacked(62, false))
        valid_moves.push_back(encode_move(piece_no_color_t::king, 60, 62));
    if (!white_to_move && get_castle_black_long(castlings) && !is_square_occupied(59) && !is_square_occupied(58) && !is_square_occupied(57) && !is_attacked(59, false) && !is_attacked(58, false))
        valid_moves.push_back(encode_move(piece_no_color_t::king, 60, 58));
}

void ChessBoard::visualise() {
    string letters = " KPNBRQ  kpnbrq";
    cout << "-------------------------------" << endl;
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            piece_t pi = get_piece_type(i * 8 + j);

            cout << letters[(int)pi] << " | ";
        }
        cout << endl << "-------------------------------" << endl;
    }
    cout << (bool)(castlings & castle_white_short_bits) << endl;
    cout << (bool)(castlings & castle_white_long_bits) << endl;
    cout << (bool)(castlings & castle_black_short_bits) << endl;
    cout << (bool)(castlings & castle_black_long_bits) << endl;
    if (en_passant != ep_empty)
        cout << (char)('a' + get_col(en_passant)) << get_row(en_passant) + 1 << endl;
    else
        cout << "-" << endl;
}

bool ChessBoard::is_attacked(uint32_t pos, bool white_move)
{
    board_t opp = white_move ? black : white;

    // Queen's and rook's attacks
    board_t opp_rook_mask = opp & (queens | rooks);
    board_t mask_blockers = (black | white) & ~opp_rook_mask;

    if (opp_rook_mask & attacking_mask_rook[pos])
    {
        if (is_rook_left_attacked(opp_rook_mask, mask_blockers, pos))
            return true;
        if (is_rook_right_attacked(opp_rook_mask, mask_blockers, pos))
            return true;
        if (is_rook_up_attacked(opp_rook_mask, mask_blockers, pos))
            return true;
        if (is_rook_down_attacked(opp_rook_mask, mask_blockers, pos))
            return true;
    }

    // Queen's and Bishop's attacks
    board_t opp_bishop_mask = opp & (queens | bishops);
    mask_blockers = (black | white) & ~opp_bishop_mask;

    if (opp_bishop_mask & attacking_mask_bishop[pos])
    {
        if (is_bishop_left_down_attacked(opp_bishop_mask, mask_blockers, pos))
            return true;
        if (is_bishop_left_up_attacked(opp_bishop_mask, mask_blockers, pos))
            return true;
        if (is_bishop_right_down_attacked(opp_bishop_mask, mask_blockers, pos))
            return true;
        if (is_bishop_right_up_attacked(opp_bishop_mask, mask_blockers, pos))
            return true;
    }

    // Knight's attacks
    if (opp & attacking_mask_knight[pos] & knights)
        return true;

    // King's attacks
    if (opp_king_attacking_mask & (1ull << pos))
        return true;

    // Pawn's attacks
    int pos_row = pos / 8;
    int pos_col = pos % 8;

    if (white_move) {
        if (pos_row < 7 && pos_col < 7 && is_piece(opp & pawns, pos + 9))
            return true;
        if (pos_row < 7 && pos_col > 0 && is_piece(opp & pawns, pos + 7))
            return true;
    }
    else
    {
        if (pos_row > 0 && pos_col < 7 && is_piece(opp & pawns, pos - 7))
            return true;
        if (pos_row > 0 && pos_col > 0 && is_piece(opp & pawns, pos - 9))
            return true;
    }

    return false;
}

bool ChessBoard::in_check()
{
    color_mask = white_to_move ? white : black;
    king_pos = bit_pos(color_mask & kings);
    return is_attacked(king_pos, white_to_move);
}

vector<move_t> ChessBoard::generate_moves()
{
    vector<move_t> valid_moves;
    vector<move_t> candidate_moves;

    valid_moves.reserve(128);

    color_mask = white_to_move ? white : black;
    opp_color_mask = white_to_move ? black : white;
    board_t b;

    king_pos = bit_pos(color_mask & kings);
    king_pos_row = get_row(king_pos);
    king_pos_col = get_col(king_pos);

    opp_king_pos = bit_pos((white_to_move ? black : white) & kings);
    opp_king_attacking_mask = attacking_mask_king[opp_king_pos];
    piece_color = white_to_move ? color_t::white : color_t::black;

    b_attacking_queens = attacking_mask_queen[king_pos];
    b_attacking_rooks = attacking_mask_rook[king_pos];
    b_attacking_bishops = attacking_mask_bishop[king_pos];
    b_need_inspection = attacking_mask_queen[king_pos];

    all_safe = !(opp_color_mask & (queens | rooks | bishops));

    if (white_to_move)
        king_in_check = white_in_check = is_attacked(king_pos, true);
    else
        king_in_check = black_in_check = is_attacked(king_pos, false);

    if (king_in_check)
        candidate_moves.reserve(64);


    if (white_to_move)
    {
        b = pawns & color_mask;
        switch (__popcnt64(pawns & color_mask))
        {
        case 8:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 7:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 6:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 5:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 4:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 3:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 2:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 1:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        }
    }
    else
    {
        b = pawns & color_mask;
        switch (__popcnt64(pawns & color_mask))
        {
        case 8:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 7:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 6:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 5:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 4:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 3:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 2:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        case 1:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1;
        }
    }

    generate_king_all_moves(valid_moves, candidate_moves, bit_pos(kings & color_mask));

    for (b = bishops & color_mask; b; b &= b - 1)
        generate_bishop_all_moves(piece_no_color_t::bishop, valid_moves, candidate_moves, bit_pos(b));

    for (b = rooks & color_mask; b; b &= b - 1)
        generate_rook_all_moves(piece_no_color_t::rook, valid_moves, candidate_moves, bit_pos(b));

    for (b = queens & color_mask; b; b &= b - 1)
        generate_queen_all_moves(valid_moves, candidate_moves, bit_pos(b));

    for (b = knights & color_mask; b; b &= b - 1)
        generate_knight_all_moves(valid_moves, candidate_moves, bit_pos(b));

    add_en_passant(valid_moves, candidate_moves);

    if (!king_in_check)
        add_castleing(valid_moves, candidate_moves);

    valid_moves.reserve(valid_moves.size() + candidate_moves.size());

    int king_pos = bit_pos(kings & color_mask);
    int king_pos_row = get_row(king_pos);
    int king_pos_col = get_col(king_pos);

    for (const move_t& move_ : candidate_moves) {
        move(move_);
        white_to_move = !white_to_move;
        if (!in_check()) {
            valid_moves.push_back(move_);
        }

        white_to_move = !white_to_move;
        undo_move();
    }

    return valid_moves;
}

vector<move_t> ChessBoard::generate_capture_moves()
{
    vector<move_t> valid_moves;
    vector<move_t> candidate_moves;

    valid_moves.reserve(64);

    color_mask = white_to_move ? white : black;
    opp_color_mask = white_to_move ? black : white;
    board_t b;

    king_pos = bit_pos(color_mask & kings);
    king_pos_row = get_row(king_pos);
    king_pos_col = get_col(king_pos);

    opp_king_pos = bit_pos((white_to_move ? black : white) & kings);
    opp_king_attacking_mask = attacking_mask_king[opp_king_pos];
    piece_color = white_to_move ? color_t::white : color_t::black;

    b_attacking_queens = attacking_mask_queen[king_pos];
    b_attacking_rooks = attacking_mask_rook[king_pos];
    b_attacking_bishops = attacking_mask_bishop[king_pos];
    b_need_inspection = attacking_mask_queen[king_pos];

    all_safe = !(opp_color_mask & (queens | rooks | bishops));

    if (white_to_move)
        king_in_check = white_in_check = is_attacked(king_pos, true);
    else
        king_in_check = black_in_check = is_attacked(king_pos, false);

    if (king_in_check)
        candidate_moves.reserve(32);


    if (white_to_move)
    {
        b = pawns & color_mask;
        switch (__popcnt64(pawns & color_mask))
        {
        case 8:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 7:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 6:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 5:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 4:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 3:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 2:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 1:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        }
    }
    else
    {
        b = pawns & color_mask;
        switch (__popcnt64(pawns & color_mask))
        {
        case 8:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 7:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 6:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 5:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 4:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 3:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 2:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        case 1:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true);
            b &= b - 1;
        }
    }

    generate_king_capture_moves(valid_moves, candidate_moves, bit_pos(kings & color_mask));

    for (b = bishops & color_mask; b; b &= b - 1)
        generate_bishop_capture_moves(piece_no_color_t::bishop, valid_moves, candidate_moves, bit_pos(b));

    for (b = rooks & color_mask; b; b &= b - 1)
        generate_rook_capture_moves(piece_no_color_t::rook, valid_moves, candidate_moves, bit_pos(b));

    for (b = queens & color_mask; b; b &= b - 1)
        generate_queen_capture_moves(valid_moves, candidate_moves, bit_pos(b));

    for (b = knights & color_mask; b; b &= b - 1)
        generate_knight_capture_moves(valid_moves, candidate_moves, bit_pos(b));

    add_en_passant(valid_moves, candidate_moves);

    valid_moves.reserve(valid_moves.size() + candidate_moves.size());

    int king_pos = bit_pos(kings & color_mask);
    int king_pos_row = get_row(king_pos);
    int king_pos_col = get_col(king_pos);

    for (const move_t& move_ : candidate_moves) {
        move(move_);
        white_to_move = !white_to_move;
        if (!in_check()) {
            valid_moves.push_back(move_);
        }

        white_to_move = !white_to_move;
        undo_move();
    }

    return valid_moves;
}

void ChessBoard::move(move_t move)
{
    piece_t takeover = get_piece_type(get_move_to(move));
    piece_no_color_t moving_piece = get_moving_piece(move);
    piece_t moving_piece_color = (piece_t)((unsigned)moving_piece + 8 * !white_to_move);

    if (takeover == piece_t::black_king || takeover == piece_t::white_king)
        cout << "*";

    move_log.push_back(encode_move_log(move, takeover, castlings, en_passant, last_pawn_move));

    auto pos_from = get_move_from(move);
    auto pos_to = get_move_to(move);

    if (moving_piece == piece_no_color_t::pawn)
        last_pawn_move = 0;
    else
        last_pawn_move += 1;

    if (moving_piece == piece_no_color_t::king) {
        if (white_to_move) {
            reset_castlings(castle_white_long_bits | castle_white_short_bits);

            if (pos_from + 2 == pos_to) {
                bit_move(white, rooks, 7, 5);
            }
            else if (pos_from == pos_to + 2) {
                bit_move(white, rooks, 0, 3);
            }
        }
        else {
            reset_castlings(castle_black_long_bits | castle_black_short_bits);

            if (pos_from + 2 == pos_to) {
                bit_move(black, rooks, 63, 61);
            }
            else if (pos_from == pos_to + 2) {
                bit_move(black, rooks, 56, 59);
            }
        }
    }
    else if (moving_piece == piece_no_color_t::rook)
    {
        if (pos_from == 7)
            reset_castlings(castle_white_short_bits);
        else if (pos_from == 0)
            reset_castlings(castle_white_long_bits);
        else if (pos_from == 63)
            reset_castlings(castle_black_short_bits);
        else if (pos_from == 56)
            reset_castlings(castle_black_long_bits);
    }
    else if (moving_piece == piece_no_color_t::pawn)
    {
        piece_t promotion = get_promotion(move);

        if (promotion != piece_t::empty) {
            bit_reset(white_to_move ? white : black, pawns, pos_from);

            if (takeover != piece_t::empty)
            {
                bit_reset(white_to_move ? black : white, *boards[(int)takeover], pos_to);

                if (pos_to == 7)
                    reset_castlings(castle_white_short_bits);
                else if (pos_to == 0)
                    reset_castlings(castle_white_long_bits);
                else if (pos_to == 63)
                    reset_castlings(castle_black_short_bits);
                else if (pos_to == 56)
                    reset_castlings(castle_black_long_bits);
            }

            bit_set(white_to_move ? white : black, *boards[(int)promotion], pos_to);

            white_to_move = !white_to_move;
            en_passant = ep_empty;

            test_board();

            return;
        }

        if (en_passant != ep_empty && en_passant == pos_to) {
            bit_reset(white_to_move ? black : white, pawns, white_to_move ? pos_to - 8 : pos_to + 8);
        }
    }

    if (moving_piece_color == piece_t::white_pawn && pos_to - pos_from == 16)
        en_passant = pos_to - 8;
    else if (moving_piece_color == piece_t::black_pawn && pos_from - pos_to == 16)
        en_passant = pos_to + 8;
    else
        en_passant = ep_empty;

    if (takeover != piece_t::empty)
    {
        bit_reset(white_to_move ? black : white, *boards[(int)takeover], pos_to);

        if (pos_to == 7)
            reset_castlings(castle_white_short_bits);
        else if (pos_to == 0)
            reset_castlings(castle_white_long_bits);
        else if (pos_to == 63)
            reset_castlings(castle_black_short_bits);
        else if (pos_to == 56)
            reset_castlings(castle_black_long_bits);
    }

    switch (moving_piece)
    {
    case piece_no_color_t::pawn:
        bit_move(white_to_move ? white : black, pawns, pos_from, pos_to);
        break;
    case piece_no_color_t::bishop:
        bit_move(white_to_move ? white : black, bishops, pos_from, pos_to);
        break;
    case piece_no_color_t::knight:
        bit_move(white_to_move ? white : black, knights, pos_from, pos_to);
        break;
    case piece_no_color_t::rook:
        bit_move(white_to_move ? white : black, rooks, pos_from, pos_to);
        break;
    case piece_no_color_t::queen:
        bit_move(white_to_move ? white : black, queens, pos_from, pos_to);
        break;
    case piece_no_color_t::king:
        bit_move(white_to_move ? white : black, kings, pos_from, pos_to);
        break;
    }

    white_to_move = !white_to_move;

    test_board();
}

void ChessBoard::no_move()
{
    move_log.push_back(encode_move_log(0, piece_t::empty, castlings, en_passant, last_pawn_move));

    en_passant = ep_empty;
    white_to_move = !white_to_move;
}

void ChessBoard::undo_move() {

    test_board();

    auto move_ = move_log.back();

    move_t move = (move_t)(move_ & 0xffffffffull);
    move_log.pop_back();

    castlings = get_castlings(move);
    en_passant = get_ep(move);

    uint32_t pos_from = get_move_from(move);
    uint32_t pos_to = get_move_to(move);

    uint32_t last_pawn_move_ = get_last_pawn_move(move_);

    last_pawn_move = last_pawn_move_;

    white_to_move = !white_to_move;

    if (pos_from == 0 && pos_to == 0)        // no-move
        return;

    piece_no_color_t moving_piece = get_moving_piece(move);
    piece_t promotion = get_promotion(move);
    piece_t takeover = get_takeover(move);

    if (moving_piece == piece_no_color_t::king) {
        if (white_to_move) {
            if (pos_from + 2 == pos_to) {
                bit_move(white, rooks, 5, 7);
            }
            else if (pos_from == pos_to + 2) {
                bit_move(white, rooks, 3, 0);
            }
        }
        else {
            if (pos_from + 2 == pos_to) {
                bit_move(black, rooks, 61, 63);
            }
            else if (pos_from == pos_to + 2) {
                bit_move(black, rooks, 59, 56);
            }
        }
    }
    else if (promotion != piece_t::empty) {
        if (pos_to >= 56)
            bit_set(white, pawns, pos_from);
        else
            bit_set(black, pawns, pos_from);

        bit_reset(white_to_move ? white : black, *boards[(int)promotion], pos_to);

        if (takeover != piece_t::empty)
            bit_set(white_to_move ? black : white, *boards[(int)takeover], pos_to);

        test_board();

        return;
    }

    if (en_passant != ep_empty && en_passant == pos_to) {
        if (is_white_pawn(pos_to))
            bit_set(black, pawns, pos_to - 8);
        else if (is_black_pawn(pos_to))
            bit_set(white, pawns, pos_to + 8);
    }

    switch (moving_piece)
    {
    case piece_no_color_t::pawn:
        bit_move(white_to_move ? white : black, pawns, pos_to, pos_from);
        break;
    case piece_no_color_t::bishop:
        bit_move(white_to_move ? white : black, bishops, pos_to, pos_from);
        break;
    case piece_no_color_t::knight:
        bit_move(white_to_move ? white : black, knights, pos_to, pos_from);
        break;
    case piece_no_color_t::rook:
        bit_move(white_to_move ? white : black, rooks, pos_to, pos_from);
        break;
    case piece_no_color_t::queen:
        bit_move(white_to_move ? white : black, queens, pos_to, pos_from);
        break;
    case piece_no_color_t::king:
        bit_move(white_to_move ? white : black, kings, pos_to, pos_from);
        break;
    }

    if (takeover != piece_t::empty)
        bit_set(white_to_move ? black : white, *boards[(int)takeover], pos_to);

    test_board();
}
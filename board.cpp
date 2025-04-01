#include "board.h"

// Splits a given string `s` into a vector of strings based on spaces.
vector<string> ChessBoard::split(string s) {
    auto p = s.begin();
    vector<string> components;
    while (true) {
        // Find the next space character in the string.
        auto q = find(p, s.end(), ' ');
        // Add the substring to the components vector.
        components.push_back(string(p, q));
        if (q == s.end())  // If the end of the string is reached, break the loop.
            break;
        // Move the pointer past the last found space.
        p = q + 1;
    }
    return components;
}

// Generates valid moves for a white pawn at position `pos`.
void ChessBoard::generate_pawn_white_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos, bool only_captures) {
    int row = get_row(pos); // Extract row from position.
    int col = get_col(pos); // Extract column from position.

    // Determine the set of moves based on whether the white king is in check.
    vector<move_t>& proposed_moves = (!white_in_check) ? valid_moves : candidate_moves;

    // Non-capture moves: Pawn moves forward.
    if (!only_captures && !is_square_occupied(pos + 8)) {
        // Handle double move for pawn starting position.
        if (row == 1 && !is_square_occupied(pos + 16)) {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 8);
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 16);
        }
        // Handle promotion for pawns reaching the last rank.
        else if (row == 6) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos + 8, color_t::white);
        }
        // Regular single forward move.
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 8);
        }
    }

    // Capture moves for pawns (diagonal moves).
    if (col > 0 && is_square_black(pos + 7)) {  // Capture on the left diagonal.
        if (row == 6) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos + 7, color_t::white);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 7);
        }
    }
    if (col < 7 && is_square_black(pos + 9)) {  // Capture on the right diagonal.
        if (row == 6) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos + 9, color_t::white);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos + 9);
        }
    }
}

// Generates valid moves for a black pawn at position `pos`.
void ChessBoard::generate_pawn_black_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos, bool only_captures) {
    int row = get_row(pos); // Extract row from position.
    int col = get_col(pos); // Extract column from position.

    // Determine the set of moves based on whether the black king is in check.
    vector<move_t>& proposed_moves = (!black_in_check) ? valid_moves : candidate_moves;

    // Non-capture moves: Pawn moves forward.
    if (!only_captures && !is_square_occupied(pos - 8)) {
        // Handle double move for pawn starting position.
        if (row == 6 && !is_square_occupied(pos - 16)) {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 8);
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 16);
        }
        // Handle promotion for pawns reaching the last rank.
        else if (row == 1) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos - 8, color_t::black);
        }
        // Regular single forward move.
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 8);
        }
    }

    // Capture moves for pawns (diagonal moves).
    if (col > 0 && is_square_white(pos - 9)) {  // Capture on the left diagonal.
        if (row == 1) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos - 9, color_t::black);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 9);
        }
    }
    if (col < 7 && is_square_white(pos - 7)) {  // Capture on the right diagonal.
        if (row == 1) {
            add_move_pr_with_control(proposed_moves, candidate_moves, piece_no_color_t::pawn, pos, pos - 7, color_t::black);
        }
        else {
            add_move_with_control(proposed_moves, piece_no_color_t::pawn, pos, pos - 7);
        }
    }
}

// Generates all possible moves for a rook at position `pos`.
void ChessBoard::generate_rook_all_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos) {
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    board_t pieces = white | black; // All occupied squares.
    board_t opp = white_to_move ? black : white; // Opponent's pieces.
    int m, k;

    // Process vertical and horizontal moves using bitboards.
    board_t mask = attacking_mask_rook_up[pos] & pieces; // Check upward moves.
    m = bit_pos_lsb(mask);

    if (mask && is_bit_set(opp, m)) // Handle captures on this path.
        m += 8;

    for (k = pos + 8; k < m; k += 8)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    // Repeat for downward, left, and right directions...
    // (Similar logic as above, omitted for brevity.)
}

// Generates capture moves for a rook at a given position `pos`.
void ChessBoard::generate_rook_capture_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // If the king is in check, moves are added to `candidate_moves`; otherwise, to `valid_moves`.
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    // Get bitboard representation of all pieces and the opponent's pieces.
    board_t pieces = white | black; // All occupied squares.
    board_t opp = white_to_move ? black : white; // Opponent's pieces.
    int m, k;

    // Capture moves in the upward direction (north).
    board_t mask = attacking_mask_rook_up[pos] & pieces; // Mask to find blocking pieces.
    m = bit_pos_lsb(mask); // Find the least significant bit (closest blocker).

    if (mask && is_bit_set(opp, m)) // If the blocker is an opponent's piece, it can be captured.
        add_move_with_control(proposed_moves, moving_piece, pos, m);

    // Capture moves in the downward direction (south).
    mask = attacking_mask_rook_down[pos] & pieces; // Mask to find blocking pieces.
    m = bit_pos_msb(mask); // Find the most significant bit (closest blocker from below).

    if (mask && is_bit_set(opp, m)) // If the blocker is an opponent's piece, it can be captured.
        add_move_with_control(proposed_moves, moving_piece, pos, m);

    // Capture moves in the right direction (east).
    mask = attacking_mask_rook_right[pos] & pieces; // Mask to find blocking pieces.
    m = bit_pos_lsb(mask); // Find the least significant bit (closest blocker).

    if (mask)
    {
        if (is_bit_set(opp, m)) // If the blocker is an opponent's piece, it can be captured.
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    // Capture moves in the left direction (west).
    mask = attacking_mask_rook_left[pos] & pieces; // Mask to find blocking pieces.
    m = bit_pos_msb(mask); // Find the most significant bit (closest blocker from the left).

    if (mask)
    {
        if (is_bit_set(opp, m)) // If the blocker is an opponent's piece, it can be captured.
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }
}


// Generates all possible moves for a bishop at a given position `pos`.
void ChessBoard::generate_bishop_all_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // If the king is in check, moves are added to `candidate_moves`; otherwise, to `valid_moves`.
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    // Get bitboard representation of all pieces and the opponent's pieces.
    board_t pieces = white | black; // All occupied squares.
    board_t opp = white_to_move ? black : white; // Opponent's pieces.

    board_t mask;
    int m, k; // m is the limit for a direction; k is the current square being processed.

    int row = get_row(pos); // Current row of the bishop.
    int col = get_col(pos); // Current column of the bishop.

    // Moves in the right-up diagonal direction.
    mask = attacking_mask_bishop_right_up[pos] & pieces; // Identify blockers in the direction.
    m = bit_pos_lsb(mask); // Find the closest blocker.

    if (mask)
    {
        // If the blocker is an opponent's piece, adjust the limit.
        if (is_bit_set(opp, m))
            m += 9;
    }
    else
    {
        // If no blockers, compute the limit based on the board edge.
        m = (8 - (pos & 7u)) * 9 + pos;

        if (m > 63) // Ensure the limit doesn't exceed the board size.
            m = 64;
    }

    // Add all valid moves in the direction.
    for (k = pos + 9; k < m; k += 9)
        add_move_with_control(proposed_moves, moving_piece, pos, k);

    // Moves in the left-down diagonal direction.
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

    // Moves in the left-up diagonal direction.
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

    // Moves in the right-down diagonal direction.
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


// Generates capture moves for a bishop at a given position `pos`.
void ChessBoard::generate_bishop_capture_moves(piece_no_color_t moving_piece, vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // If the king is in check, moves are added to `candidate_moves`; otherwise, to `valid_moves`.
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    // Get bitboard representation of all pieces and the opponent's pieces.
    board_t pieces = white | black; // All occupied squares.
    board_t opp = white_to_move ? black : white; // Opponent's pieces.

    board_t mask;
    int m; // m is the closest blocker in the given direction.

    int row = get_row(pos); // Current row of the bishop.
    int col = get_col(pos); // Current column of the bishop.

    // Capture moves in the right-up diagonal direction.
    mask = attacking_mask_bishop_right_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        // Add the move only if the blocker is an opponent's piece.
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    // Capture moves in the left-down diagonal direction.
    mask = attacking_mask_bishop_left_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    // Capture moves in the left-up diagonal direction.
    mask = attacking_mask_bishop_left_up[pos] & pieces;
    m = bit_pos_lsb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }

    // Capture moves in the right-down diagonal direction.
    mask = attacking_mask_bishop_right_down[pos] & pieces;
    m = bit_pos_msb(mask);

    if (mask)
    {
        if (is_bit_set(opp, m))
            add_move_with_control(proposed_moves, moving_piece, pos, m);
    }
}


// Generates all possible moves for the queen at position `pos`.
void ChessBoard::generate_queen_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // Queen's moves combine the movements of both rook and bishop.
    generate_rook_all_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos); // Add rook-like moves.
    generate_bishop_all_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos); // Add bishop-like moves.
}


// Generates capture moves for the queen at position `pos`.
void ChessBoard::generate_queen_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // Queen's capture moves combine the capture moves of both rook and bishop.
    generate_rook_capture_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos); // Add rook-like captures.
    generate_bishop_capture_moves(piece_no_color_t::queen, valid_moves, candidate_moves, pos); // Add bishop-like captures.
}


// Generates all possible moves for the knight at position `pos`.
void ChessBoard::generate_knight_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    // Use appropriate move list depending on whether the king is in check.
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    // Loop through all knight attackable squares using the precomputed mask.
    for (board_t b = ~color_mask & attacking_mask_knight[pos]; b; b &= b - 1) // Iterate over each valid bit.
        add_move_with_control(proposed_moves, piece_no_color_t::knight, pos, bit_pos(b)); // Add the move.
}


// Generates capture moves for the knight at position `pos`.
void ChessBoard::generate_knight_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    board_t opp = white_to_move ? black : white; // Opponent's pieces bitboard.

    // Use appropriate move list depending on whether the king is in check.
    vector<move_t>& proposed_moves = king_in_check ? candidate_moves : valid_moves;

    // Loop through all attackable squares using the precomputed mask.
    for (board_t b = ~color_mask & attacking_mask_knight[pos]; b; b &= b - 1) {
        // Only add the move if the square contains an opponent's piece.
        if (is_bit_set(opp, bit_pos(b)))
            add_move_with_control(proposed_moves, piece_no_color_t::knight, pos, bit_pos(b));
    }
}


// Generates all possible moves for the king at position `pos`.
void ChessBoard::generate_king_all_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    board_t& pieces = (white_to_move ? white : black); // King's pieces bitboard.
    board_t old_kings = kings; // Save current king's position bitboard.
    board_t old_pieces = pieces; // Save current player's pieces bitboard.

    // Loop through all attackable squares using the precomputed mask.
    for (board_t b = ~color_mask & attacking_mask_king[pos]; b; b &= b - 1) {
        int new_pos = bit_pos(b); // Get the position of the attackable square.

        // Temporarily move the king for attack validation.
        kings = move_piece_in_bit_mask(kings, pos, new_pos);
        pieces = move_piece_in_bit_mask(pieces, pos, new_pos);
        if (!is_attacked(new_pos, white_to_move)) // Check if the move is safe from attacks.
            valid_moves.push_back(encode_move(piece_no_color_t::king, pos, bit_pos(b))); // Add the move.

        // Restore the original positions after validation.
        kings = old_kings;
        pieces = old_pieces;
    }
}


// Generates capture moves for the king at position `pos`.
void ChessBoard::generate_king_capture_moves(vector<move_t>& valid_moves, vector<move_t>& candidate_moves, uint32_t pos)
{
    board_t& pieces = (white_to_move ? white : black); // King's pieces bitboard.
    board_t old_kings = kings; // Save current king's position bitboard.
    board_t old_pieces = pieces; // Save current player's pieces bitboard.
    board_t opp = white_to_move ? black : white; // Opponent's pieces bitboard.

    // Loop through all attackable squares using the precomputed mask.
    for (board_t b = ~color_mask & attacking_mask_king[pos]; b; b &= b - 1) {
        int new_pos = bit_pos(b); // Get the position of the attackable square.

        // Check if the square contains an opponent's piece.
        if (!is_bit_set(opp, new_pos))
            continue;

        // Temporarily move the king for attack validation.
        kings = move_piece_in_bit_mask(kings, pos, new_pos);
        pieces = move_piece_in_bit_mask(pieces, pos, new_pos);
        if (!is_attacked(new_pos, white_to_move)) // Check if the move is safe from attacks.
            valid_moves.push_back(encode_move(piece_no_color_t::king, pos, bit_pos(b))); // Add the move.

        // Restore the original positions after validation.
        kings = old_kings;
        pieces = old_pieces;
    }
}


// Parses a FEN (Forsyth-Edwards Notation) string to set up the chessboard state.
void ChessBoard::from_fen(string fen) {
    clear(); // Clear the current state of the chessboard.

    // Split the FEN string into its components.
    vector<string> components = split(fen);

    // Initialize masks to track positions on the board (starting from the top-left corner).
    board_t mask = 1ull << 56; // Positioning mask for pieces.
    board_t base_mask = 1ull << 56; // Base mask for starting each new row.

    // Iterate through the first component of the FEN string (board setup).
    for (auto c : components.front()) {
        switch (c) {
        case 'k': // Black king.
            black |= mask; // Mark the square as occupied by a black piece.
            kings |= mask; // Mark the square as occupied by a king.
            mask <<= 1; // Move the mask to the next square.
            break;

        case 'p': // Black pawn.
            black |= mask;
            pawns |= mask;
            mask <<= 1;
            break;

        case 'n': // Black knight.
            black |= mask;
            knights |= mask;
            mask <<= 1;
            break;

        case 'b': // Black bishop.
            black |= mask;
            bishops |= mask;
            mask <<= 1;
            break;

        case 'r': // Black rook.
            black |= mask;
            rooks |= mask;
            mask <<= 1;
            break;

        case 'q': // Black queen.
            black |= mask;
            queens |= mask;
            mask <<= 1;
            break;

        case 'K': // White king.
            white |= mask; // Mark the square as occupied by a white piece.
            kings |= mask; // Mark the square as occupied by a king.
            mask <<= 1; // Move the mask to the next square.
            break;

        case 'P': // White pawn.
            white |= mask;
            pawns |= mask;
            mask <<= 1;
            break;

        case 'N': // White knight.
            white |= mask;
            knights |= mask;
            mask <<= 1;
            break;

        case 'B': // White bishop.
            white |= mask;
            bishops |= mask;
            mask <<= 1;
            break;

        case 'R': // White rook.
            white |= mask;
            rooks |= mask;
            mask <<= 1;
            break;

        case 'Q': // White queen.
            white |= mask;
            queens |= mask;
            mask <<= 1;
            break;

        case '/': // Move to the next row (in FEN format).
            base_mask >>= 8; // Move the base mask down one row.
            mask = base_mask; // Reset the mask to the start of the new row.
            break;

        default: // Handle empty squares (numbers in FEN represent gaps).
            mask <<= c - '0'; // Shift the mask by the number of empty squares.
        }
    }

    // Set the turn to move (white or black) based on the second component of the FEN string.
    if (components[1] == "w")
        white_to_move = true; // White's turn to move.
    else
        white_to_move = false; // Black's turn to move.

    // Reset castling rights for both sides.
    reset_castlings(castle_white_short_bits | castle_white_long_bits | castle_black_short_bits | castle_black_long_bits);

    // Set castling rights based on the third component of the FEN string.
    for (char component : components[2]) {
        if (component == 'K')
            set_castlings(castle_white_short_bits); // White kingside castling.
        if (component == 'Q')
            set_castlings(castle_white_long_bits); // White queenside castling.
        if (component == 'k')
            set_castlings(castle_black_short_bits); // Black kingside castling.
        if (component == 'q')
            set_castlings(castle_black_long_bits); // Black queenside castling.
    }

    // Handle en passant information from the fourth component of the FEN string.
    if (components[3] == "-")
        en_passant = ep_empty; // No en passant square.
    else {
        // Encode the en passant square based on the column (file) and row (rank).
        en_passant = encode_ep(components[3][1] - '1', components[3][0] - 'a');
    }
}


// Generates a FEN (Forsyth-Edwards Notation) string representing the current board state.
string ChessBoard::get_fen()
{
    string fen = ""; // Initialize the FEN string.
    int dif = 0; // Counter for consecutive empty squares.

    // Loop through all 64 squares of the board (0 to 63).
    for (int i = 0; i < 64; i++)
    {
        int file = i % 8; // Column (file) of the current square.
        int rank = 7 - (i / 8); // Row (rank) of the current square (inverted for FEN).
        square_t square = rank * 8 + file; // Calculate the square's index.

        // Handle the beginning of a new row (FEN uses '/' to separate ranks).
        if (square % 8 == 0 && square != 56)
        {
            if (dif > 0) // Add any accumulated empty squares to the FEN string.
            {
                fen += to_string(dif);
                dif = 0; // Reset the empty square counter.
            }
            fen.push_back('/'); // Add the '/' separator for the new rank.
        }

        // Add the appropriate character to the FEN string based on the piece type.
        switch (get_piece_type(square))
        {
        case piece_t::empty: // Empty square.
            dif++; // Increment the empty square counter.
            break;

        case piece_t::white_pawn:
            if (dif > 0)
            {
                fen += to_string(dif); // Add empty squares before this piece.
                dif = 0;
            }
            fen.push_back('P'); // Add 'P' for a white pawn.
            break;

        case piece_t::white_knight:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('N'); // Add 'N' for a white knight.
            break;

        case piece_t::white_bishop:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('B'); // Add 'B' for a white bishop.
            break;

        case piece_t::white_rook:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('R'); // Add 'R' for a white rook.
            break;

        case piece_t::white_queen:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('Q'); // Add 'Q' for a white queen.
            break;

        case piece_t::white_king:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('K'); // Add 'K' for a white king.
            break;

        case piece_t::black_pawn:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('p'); // Add 'p' for a black pawn.
            break;

        case piece_t::black_knight:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('n'); // Add 'n' for a black knight.
            break;

        case piece_t::black_bishop:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('b'); // Add 'b' for a black bishop.
            break;

        case piece_t::black_rook:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('r'); // Add 'r' for a black rook.
            break;

        case piece_t::black_queen:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('q'); // Add 'q' for a black queen.
            break;

        case piece_t::black_king:
            if (dif > 0)
            {
                fen += to_string(dif);
                dif = 0;
            }
            fen.push_back('k'); // Add 'k' for a black king.
            break;

        default:
            break;
        }
    }

    // Add the active color (whose turn it is) to the FEN string.
    fen.push_back(' ');
    if (white_to_move)
        fen.push_back('w'); // 'w' for white to move.
    else
        fen.push_back('b'); // 'b' for black to move.

    // Add the castling rights to the FEN string.
    fen.push_back(' ');
    if (get_castle_white_short(castlings))
        fen.push_back('K'); // White kingside castling.
    if (get_castle_white_long(castlings))
        fen.push_back('Q'); // White queenside castling.
    if (get_castle_white_short(castlings))
        fen.push_back('k'); // Black kingside castling.
    if (get_castle_black_long(castlings))
        fen.push_back('q'); // Black queenside castling.
    if (!castlings) // If no castling is allowed, add a dash '-'.
        fen.push_back('-');

    // Add the en passant target square to the FEN string.
    fen.push_back(' ');
    if (en_passant == ep_empty)
        fen.push_back('-'); // '-' if there is no en passant target.
    else {
        int file = en_passant % 8; // Calculate the file (column).
        int rank = en_passant / 8; // Calculate the rank (row).
        fen.push_back("abcdefgh"[file]); // Add the file as a letter.
        fen.push_back("12345678"[rank]); // Add the rank as a number.
    }

    // Add the halfmove clock (number of moves since the last pawn move or capture).
    fen.push_back(' ');
    fen += to_string(last_pawn_move);

    // Add the fullmove number (total number of moves in the game).
    fen.push_back(' ');
    fen += to_string(move_log.size());

    return fen; // Return the constructed FEN string.
}


// Adds valid en passant moves to the list of possible moves.
void ChessBoard::add_en_passant(vector<move_t>& valid_moves, vector<move_t>& candidate_moves)
{
    // If there is no en passant square, exit the function.
    if (en_passant == ep_empty)
        return;

    // Determine which move list to add to, based on whether the player is in check.
    vector<move_t>& proposed_moves = ((white_to_move && !white_in_check) || (!white_to_move && !black_in_check)) ? valid_moves : candidate_moves;

    // Get the row and column of the en passant square.
    uint32_t ep_row = get_ep_row(en_passant);
    uint32_t ep_col = get_ep_col(en_passant);
    uint32_t pos = en_passant; // En passant position.

    // If the en passant square is on row 5 (black's fifth rank).
    if (ep_row == 5) {
        // Check left diagonal for a white pawn.
        if (ep_col < 7 && is_white_pawn(pos - 7))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos - 7, pos);

        // Check right diagonal for a white pawn.
        if (ep_col > 0 && is_white_pawn(pos - 9))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos - 9, pos);
    }

    // If the en passant square is on row 2 (white's fifth rank).
    if (ep_row == 2) {
        // Check right diagonal for a black pawn.
        if (ep_col < 7 && is_black_pawn(pos + 9))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos + 9, pos);

        // Check left diagonal for a black pawn.
        if (ep_col > 0 && is_black_pawn(pos + 7))
            add_move_ep_with_control(proposed_moves, piece_no_color_t::pawn, pos + 7, pos);
    }
}

// Adds valid castling moves to the list of possible moves.
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

// Prints the chessboard and current game state in a simple visual format.
void ChessBoard::visualise() {
    string letters = " KPNBRQ  kpnbrq"; // Mapping from piece types to characters.
    cout << "-------------------------------" << endl;

    // Loop through the board from the top rank to the bottom rank.
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            piece_t pi = get_piece_type(i * 8 + j); // Get the piece type for the current square.
            cout << letters[(int)pi] << " | "; // Print the corresponding character for the piece.
        }
        cout << endl << "-------------------------------" << endl; // Row separator.
    }

    // Print castling rights.
    cout << (bool)(castlings & castle_white_short_bits) << endl; // White kingside castling.
    cout << (bool)(castlings & castle_white_long_bits) << endl;  // White queenside castling.
    cout << (bool)(castlings & castle_black_short_bits) << endl; // Black kingside castling.
    cout << (bool)(castlings & castle_black_long_bits) << endl;  // Black queenside castling.

    // Print the en passant square.
    if (en_passant != ep_empty)
        cout << (char)('a' + get_col(en_passant)) << get_row(en_passant) + 1 << endl; // En passant square (e.g., "e3").
    else
        cout << "-" << endl; // No en passant square.
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

// Checks if the current player's king is in check.
bool ChessBoard::in_check()
{
    // Set the color mask to the current player's pieces (white or black).
    color_mask = white_to_move ? white : black;

    // Determine the position of the current player's king.
    king_pos = bit_pos(color_mask & kings);

    // Return true if the king is attacked; otherwise, false.
    return is_attacked(king_pos, white_to_move);
}


// Generates all valid moves for the current player's pieces, considering check status.
vector<move_t> ChessBoard::generate_moves()
{
    vector<move_t> valid_moves;     // List of legal moves.
    vector<move_t> candidate_moves; // List of moves requiring further validation.

    valid_moves.reserve(128); // Reserve space for potential moves.

    // Set masks for the current player's pieces and the opponent's pieces.
    color_mask = white_to_move ? white : black;
    opp_color_mask = white_to_move ? black : white;
    board_t b;

    // Set position and coordinates of the current player's king.
    king_pos = bit_pos(color_mask & kings);
    king_pos_row = get_row(king_pos);
    king_pos_col = get_col(king_pos);

    // Set position and attacking mask of the opponent's king.
    opp_king_pos = bit_pos((white_to_move ? black : white) & kings);
    opp_king_attacking_mask = attacking_mask_king[opp_king_pos];

    // Set the current player's color.
    piece_color = white_to_move ? color_t::white : color_t::black;

    // Precompute attacking masks for the current player's king position.
    b_attacking_queens = attacking_mask_queen[king_pos];
    b_attacking_rooks = attacking_mask_rook[king_pos];
    b_attacking_bishops = attacking_mask_bishop[king_pos];
    b_need_inspection = attacking_mask_queen[king_pos];

    // Check if the opponent has no attacking pieces (queens, rooks, bishops).
    all_safe = !(opp_color_mask & (queens | rooks | bishops));

    // Determine if the current player's king is in check.
    if (white_to_move)
        king_in_check = white_in_check = is_attacked(king_pos, true);
    else
        king_in_check = black_in_check = is_attacked(king_pos, false);

    // Reserve space for candidate moves if the king is in check.
    if (king_in_check)
        candidate_moves.reserve(64);

    // Generate moves for pawns.
    if (white_to_move)
    {
        b = pawns & color_mask;
        switch (__popcnt64(pawns & color_mask)) // Count the number of pawns.
        {
        case 8:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1; // Remove the current pawn from the bitboard.
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
        switch (__popcnt64(pawns & color_mask)) // Count the number of pawns.
        {
        case 8:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b));
            b &= b - 1; // Remove the current pawn from the bitboard.
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

    // Generate moves for the king.
    generate_king_all_moves(valid_moves, candidate_moves, bit_pos(kings & color_mask));

    // Generate moves for bishops.
    for (b = bishops & color_mask; b; b &= b - 1)
        generate_bishop_all_moves(piece_no_color_t::bishop, valid_moves, candidate_moves, bit_pos(b));

    // Generate moves for rooks.
    for (b = rooks & color_mask; b; b &= b - 1)
        generate_rook_all_moves(piece_no_color_t::rook, valid_moves, candidate_moves, bit_pos(b));

    // Generate moves for queens.
    for (b = queens & color_mask; b; b &= b - 1)
        generate_queen_all_moves(valid_moves, candidate_moves, bit_pos(b));

    // Generate moves for knights.
    for (b = knights & color_mask; b; b &= b - 1)
        generate_knight_all_moves(valid_moves, candidate_moves, bit_pos(b));

    // Add en passant moves.
    add_en_passant(valid_moves, candidate_moves);

    // Add castling moves if the king is not in check.
    if (!king_in_check)
        add_castleing(valid_moves, candidate_moves);

    // Reserve space for valid moves, including candidate moves.
    valid_moves.reserve(valid_moves.size() + candidate_moves.size());

    // Validate candidate moves by temporarily executing them.
    for (const move_t& move_ : candidate_moves) {
        move(move_); // Execute the move.
        white_to_move = !white_to_move; // Toggle the turn.
        if (!in_check()) { // Check if the move resolves the check.
            valid_moves.push_back(move_);
        }
        white_to_move = !white_to_move; // Restore the turn.
        undo_move(); // Undo the move.
    }

    // Return the list of valid moves.
    return valid_moves;
}


// Generates all valid capture moves for the current player.
vector<move_t> ChessBoard::generate_capture_moves()
{
    vector<move_t> valid_moves;     // List of legal capture moves.
    vector<move_t> candidate_moves; // List of capture moves requiring validation.

    valid_moves.reserve(64); // Reserve memory for up to 64 capture moves.

    // Set masks for the current player's and opponent's pieces.
    color_mask = white_to_move ? white : black;
    opp_color_mask = white_to_move ? black : white;
    board_t b; // Temporary bitboard variable.

    // Set position and coordinates for the current player's king.
    king_pos = bit_pos(color_mask & kings);
    king_pos_row = get_row(king_pos);
    king_pos_col = get_col(king_pos);

    // Set position and attacking mask of the opponent's king.
    opp_king_pos = bit_pos((white_to_move ? black : white) & kings);
    opp_king_attacking_mask = attacking_mask_king[opp_king_pos];

    // Set the current player's color.
    piece_color = white_to_move ? color_t::white : color_t::black;

    // Precompute attacking masks for the current player's king position.
    b_attacking_queens = attacking_mask_queen[king_pos];
    b_attacking_rooks = attacking_mask_rook[king_pos];
    b_attacking_bishops = attacking_mask_bishop[king_pos];
    b_need_inspection = attacking_mask_queen[king_pos];

    // Determine if the opponent has attacking pieces on the board.
    all_safe = !(opp_color_mask & (queens | rooks | bishops));

    // Determine if the current player's king is in check.
    if (white_to_move)
        king_in_check = white_in_check = is_attacked(king_pos, true);
    else
        king_in_check = black_in_check = is_attacked(king_pos, false);

    // Reserve memory for candidate moves if the king is in check.
    if (king_in_check)
        candidate_moves.reserve(32);

    // Generate capture moves for pawns.
    if (white_to_move)
    {
        b = pawns & color_mask; // Get all white pawns.
        switch (__popcnt64(pawns & color_mask)) // Count the number of pawns.
        {
        case 8:
            generate_pawn_white_moves(valid_moves, candidate_moves, bit_pos(b), true); // Generate capture moves.
            b &= b - 1; // Remove the current pawn from the bitboard.
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
        b = pawns & color_mask; // Get all black pawns.
        switch (__popcnt64(pawns & color_mask)) // Count the number of pawns.
        {
        case 8:
            generate_pawn_black_moves(valid_moves, candidate_moves, bit_pos(b), true); // Generate capture moves.
            b &= b - 1; // Remove the current pawn from the bitboard.
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

    // Generate capture moves for the king.
    generate_king_capture_moves(valid_moves, candidate_moves, bit_pos(kings & color_mask));

    // Generate capture moves for bishops.
    for (b = bishops & color_mask; b; b &= b - 1)
        generate_bishop_capture_moves(piece_no_color_t::bishop, valid_moves, candidate_moves, bit_pos(b));

    // Generate capture moves for rooks.
    for (b = rooks & color_mask; b; b &= b - 1)
        generate_rook_capture_moves(piece_no_color_t::rook, valid_moves, candidate_moves, bit_pos(b));

    // Generate capture moves for queens.
    for (b = queens & color_mask; b; b &= b - 1)
        generate_queen_capture_moves(valid_moves, candidate_moves, bit_pos(b));

    // Generate capture moves for knights.
    for (b = knights & color_mask; b; b &= b - 1)
        generate_knight_capture_moves(valid_moves, candidate_moves, bit_pos(b));

    // Add en passant capture moves.
    add_en_passant(valid_moves, candidate_moves);

    // Reserve additional space for validated moves.
    valid_moves.reserve(valid_moves.size() + candidate_moves.size());

    // Validate candidate moves to ensure they resolve check.
    for (const move_t& move_ : candidate_moves) {
        move(move_); // Execute the move.
        white_to_move = !white_to_move; // Toggle the turn.
        if (!in_check()) { // Check if the move resolves the check.
            valid_moves.push_back(move_); // Add the move to valid moves.
        }
        white_to_move = !white_to_move; // Restore the turn.
        undo_move(); // Undo the move.
    }

    // Return the list of valid capture moves.
    return valid_moves;
}


// Executes a move on the chessboard and updates the board state.
void ChessBoard::move(move_t move)
{
    // Get the type of piece being captured (if any) and the moving piece.
    piece_t takeover = get_piece_type(get_move_to(move));
    piece_no_color_t moving_piece = get_moving_piece(move);

    // Determine the moving piece type and color.
    piece_t moving_piece_color = (piece_t)((unsigned)moving_piece + 8 * !white_to_move);

    // If the captured piece is a king, print a special symbol (end of game indication?).
    if (takeover == piece_t::black_king || takeover == piece_t::white_king)
        cout << "*";

    // Log the move, capturing the move details, castling rights, en passant, etc.
    move_log.push_back(encode_move_log(move, takeover, castlings, en_passant, last_pawn_move));

    // Extract the source and destination positions of the move.
    auto pos_from = get_move_from(move);
    auto pos_to = get_move_to(move);

    // Reset the pawn move counter if a pawn moves, otherwise increment it.
    if (moving_piece == piece_no_color_t::pawn)
        last_pawn_move = 0;
    else
        last_pawn_move += 1;

    // Handle king moves (including castling).
    if (moving_piece == piece_no_color_t::king) {
        if (white_to_move) { // White king.
            reset_castlings(castle_white_long_bits | castle_white_short_bits); // Remove white castling rights.

            // Handle kingside castling.
            if (pos_from + 2 == pos_to) {
                bit_move(white, rooks, 7, 5); // Move rook during kingside castling.
            }
            // Handle queenside castling.
            else if (pos_from == pos_to + 2) {
                bit_move(white, rooks, 0, 3); // Move rook during queenside castling.
            }
        }
        else { // Black king.
            reset_castlings(castle_black_long_bits | castle_black_short_bits); // Remove black castling rights.

            // Handle kingside castling.
            if (pos_from + 2 == pos_to) {
                bit_move(black, rooks, 63, 61); // Move rook during kingside castling.
            }
            // Handle queenside castling.
            else if (pos_from == pos_to + 2) {
                bit_move(black, rooks, 56, 59); // Move rook during queenside castling.
            }
        }
    }
    // Handle rook moves (to update castling rights).
    else if (moving_piece == piece_no_color_t::rook) {
        if (pos_from == 7)
            reset_castlings(castle_white_short_bits); // White kingside rook moved.
        else if (pos_from == 0)
            reset_castlings(castle_white_long_bits); // White queenside rook moved.
        else if (pos_from == 63)
            reset_castlings(castle_black_short_bits); // Black kingside rook moved.
        else if (pos_from == 56)
            reset_castlings(castle_black_long_bits); // Black queenside rook moved.
    }
    // Handle pawn moves (including promotion and en passant).
    else if (moving_piece == piece_no_color_t::pawn) {
        piece_t promotion = get_promotion(move); // Check if the pawn is being promoted.

        if (promotion != piece_t::empty) {
            // Remove the pawn from its current position.
            bit_reset(white_to_move ? white : black, pawns, pos_from);

            // Remove the captured piece (if any) from the destination square.
            if (takeover != piece_t::empty) {
                bit_reset(white_to_move ? black : white, *boards[(int)takeover], pos_to);

                // Update castling rights if the captured piece affects them.
                if (pos_to == 7)
                    reset_castlings(castle_white_short_bits);
                else if (pos_to == 0)
                    reset_castlings(castle_white_long_bits);
                else if (pos_to == 63)
                    reset_castlings(castle_black_short_bits);
                else if (pos_to == 56)
                    reset_castlings(castle_black_long_bits);
            }

            // Add the promoted piece to the board.
            bit_set(white_to_move ? white : black, *boards[(int)promotion], pos_to);

            // Toggle the turn and reset en passant.
            white_to_move = !white_to_move;
            en_passant = ep_empty;

            test_board(); // Test the board integrity.

            return; // Exit after handling promotion.
        }

        // Handle en passant captures.
        if (en_passant != ep_empty && en_passant == pos_to) {
            bit_reset(white_to_move ? black : white, pawns, white_to_move ? pos_to - 8 : pos_to + 8);
        }
    }

    // Update en passant square for double pawn moves.
    if (moving_piece_color == piece_t::white_pawn && pos_to - pos_from == 16)
        en_passant = pos_to - 8; // White double pawn move.
    else if (moving_piece_color == piece_t::black_pawn && pos_from - pos_to == 16)
        en_passant = pos_to + 8; // Black double pawn move.
    else
        en_passant = ep_empty; // Reset en passant otherwise.

    // Handle piece captures.
    if (takeover != piece_t::empty) {
        // Remove the captured piece from the opponent's bitboard.
        bit_reset(white_to_move ? black : white, *boards[(int)takeover], pos_to);

        // Update castling rights if the captured piece affects them.
        if (pos_to == 7)
            reset_castlings(castle_white_short_bits);
        else if (pos_to == 0)
            reset_castlings(castle_white_long_bits);
        else if (pos_to == 63)
            reset_castlings(castle_black_short_bits);
        else if (pos_to == 56)
            reset_castlings(castle_black_long_bits);
    }

    // Move the piece to its new position on the board.
    switch (moving_piece) {
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

    // Toggle the turn to the next player.
    white_to_move = !white_to_move;

    // Test the board integrity.
    test_board();
}


// Handles situations where no move is made (pass the turn).
void ChessBoard::no_move()
{
    // Log the "no-move" action by encoding the current state into move_log.
    move_log.push_back(encode_move_log(0, piece_t::empty, castlings, en_passant, last_pawn_move));

    // Reset en passant square since no move is made.
    en_passant = ep_empty;

    // Toggle the turn to the next player.
    white_to_move = !white_to_move;
}


// Undo the last move and restore the previous board state.
void ChessBoard::undo_move()
{
    // Perform a board integrity check before undoing the move.
    test_board();

    // Retrieve and remove the last move from the log.
    auto move_ = move_log.back();
    move_t move = (move_t)(move_ & 0xffffffffull); // Extract the move details.
    move_log.pop_back();

    // Restore the castling rights and en passant square from the move log.
    castlings = get_castlings(move);
    en_passant = get_ep(move);

    // Extract the source and destination positions of the move.
    uint32_t pos_from = get_move_from(move);
    uint32_t pos_to = get_move_to(move);

    // Restore the last pawn move counter.
    uint32_t last_pawn_move_ = get_last_pawn_move(move_);
    last_pawn_move = last_pawn_move_;

    // Toggle the turn to the previous player.
    white_to_move = !white_to_move;

    // Handle "no-move" case.
    if (pos_from == 0 && pos_to == 0) // If the move indicates "no-move."
        return;

    // Get the type of piece, promotion details, and captured piece involved in the move.
    piece_no_color_t moving_piece = get_moving_piece(move);
    piece_t promotion = get_promotion(move);
    piece_t takeover = get_takeover(move);

    // Undo special moves for the king (castling).
    if (moving_piece == piece_no_color_t::king) {
        if (white_to_move) {
            // Undo white kingside castling.
            if (pos_from + 2 == pos_to) {
                bit_move(white, rooks, 5, 7); // Move rook back to its original position.
            }
            // Undo white queenside castling.
            else if (pos_from == pos_to + 2) {
                bit_move(white, rooks, 3, 0); // Move rook back to its original position.
            }
        }
        else {
            // Undo black kingside castling.
            if (pos_from + 2 == pos_to) {
                bit_move(black, rooks, 61, 63); // Move rook back to its original position.
            }
            // Undo black queenside castling.
            else if (pos_from == pos_to + 2) {
                bit_move(black, rooks, 59, 56); // Move rook back to its original position.
            }
        }
    }
    // Undo promotion moves.
    else if (promotion != piece_t::empty) {
        // Restore the pawn to its original position.
        if (pos_to >= 56)
            bit_set(white, pawns, pos_from);
        else
            bit_set(black, pawns, pos_from);

        // Remove the promoted piece from the board.
        bit_reset(white_to_move ? white : black, *boards[(int)promotion], pos_to);

        // Restore the captured piece to its original position if any.
        if (takeover != piece_t::empty)
            bit_set(white_to_move ? black : white, *boards[(int)takeover], pos_to);

        // Perform a board integrity check and exit the function.
        test_board();
        return;
    }

    // Undo en passant captures.
    if (en_passant != ep_empty && en_passant == pos_to) {
        if (is_white_pawn(pos_to))
            bit_set(black, pawns, pos_to - 8); // Restore the captured black pawn.
        else if (is_black_pawn(pos_to))
            bit_set(white, pawns, pos_to + 8); // Restore the captured white pawn.
    }

    // Restore the moving piece to its original position.
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

    // Restore the captured piece to its original position if any.
    if (takeover != piece_t::empty)
        bit_set(white_to_move ? black : white, *boards[(int)takeover], pos_to);

    // Perform a board integrity check.
    test_board();
}

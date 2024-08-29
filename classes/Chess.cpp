#pragma once
#include "Chess.h"
#include "game.h"
#include "Evaluate.h"

Chess::Chess() {}
Chess::~Chess() {}

Bit *Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char *pieces[] = {"pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png"};

    Bit *bit = new Bit();

    const char *pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "b_" : "w_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}
void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * y + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            _grid[y][x].setNotation(indexToNotation(y, x));
        }
    }
    fenToBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    if (gameHasAI())
    {
        setAIPlayer(AI_PLAYER);
    }

    _moves = generateMoves(stateString().c_str(), 'W', true);
    startGame();
}

Player *Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}
//* BOARD TO FEN

std::string Chess::boardToFen() const
{
    std::string fen;
    for (int row = 0; row < 8; ++row)
    {
        if (row > 0)
            fen += '/';
        int EmptySquareCount = 0;
        for (int col = 0; col < 8; ++col)
        {
            auto piece = current_pieceNotation(row, col);
            if (piece == "1")
            {
                ++EmptySquareCount;
            }
            else
            {
                if (EmptySquareCount > 0)
                {
                    fen += std::to_string(EmptySquareCount);
                    EmptySquareCount = 0;
                }
                fen += piece;
            }
        }
        if (EmptySquareCount > 0)
        {
            fen += std::to_string(EmptySquareCount);
        }
    }
    // TODO: add turn, castling availability, en passant target, and move counts.
    return fen;
}

void Chess::fenToBoard(const std::string &fen)
{
    stopGame();
    int row = 0, col = 0;
    int strindex = 0;
    ChessPiece piece = Pawn;
    int color;
    int offset;
    for (int i = 0; i < fen.length(); i++)
    {
        char c = fen[i];
        if (c == '/')
        {
            ++row;
            col = 0;
        }
        else if (std::isdigit(c))
        {
            col += c - '0';
        }
        else
        {
            switch (std::tolower(c))
            { // figure out what piece it is
            case 'p':
                piece = Pawn;
                break;
            case 'n':
                piece = Knight;
                break;
            case 'b':
                piece = Bishop;
                break;
            case 'r':
                piece = Rook;
                break;
            case 'q':
                piece = Queen;
                break;
            case 'k':
                piece = King;
                break;
            case ' ':
                break;
            }
            if (fen[i] == ' ')
                break;
            color = std::islower(c) ? 0 : 1;
            offset = color == 0 ? 128 : 0;
            Bit *bit = PieceForPlayer(color, piece);
            bit->setPosition(_grid[row][col].getPosition());
            bit->setParent(&_grid[row][col]);
            bit->setGameTag(piece + offset);
            _grid[row][col].setBit(bit);
            ++col;
            strindex = i;
        }
    }
    if (strindex == fen.size())
    {
        std::string remainder = fen.substr(strindex, fen.size());
        char *end = new char[remainder.length() + 1];
        std::strcpy(end, remainder.c_str());

        // turn
        char *tok = std::strtok(end, " ");
        if (!(strcmp(tok, "b")))
        {
            _gameOptions.currentTurnNo = 1;
        }
        else if (!(strcmp(tok, "w")))
        {
            _gameOptions.currentTurnNo = 0;
        }
        // castle rights
        tok = std::strtok(nullptr, " ");
        for (int x = 0; x < strlen(tok); x++)
        {
            switch (tok[x])
            {
            case 'Q':
                w_canCastleQ = true;
                break;
            case 'q':
                b_canCastleQ = true;
                break;
            case 'K':
                w_canCastleK = true;
                break;
            case 'k':
                b_canCastleK = true;
                break;
            default:
                break;
            }
        }

        // en passant
        tok = std::strtok(nullptr, " ");
        enPassant = std::string(tok);

        tok = std::strtok(nullptr, " ");
        halfmove = atoi(tok);

        tok = std::strtok(nullptr, " ");
        fullmove = atoi(tok);
        delete[] end;
    }
    return;
}
std::string Chess::initialStateString() { return stateString(); }

std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++)
    {
        for (int x = 0; x < _gameOptions.rowX; x++)
        {
            s += current_pieceNotation(y, x);
        }
    }
    return s;
}
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            int index = y * 8 + x;
            int playerNumber = s[index] - '0';
            if (playerNumber)
            {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            }
            else
            {
                _grid[y][x].setBit(nullptr);
            }
        }
    }
}
//
// Helper to find valid moves for a piece
//
void Chess::addMoveIfValid(const char *state, std::vector<Move> &moves, int fromRow, int fromCol, int toRow, int toCol)
{
    if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8)
    {
        int fromColor = stateColor(state, fromRow, fromCol);
        int toColor = stateColor(state, toRow, toCol);
        if (fromColor != toColor)
        {
            moves.push_back({indexToNotation(fromRow, fromCol), indexToNotation(toRow, toCol)});
        }
    }
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    ChessSquare &srcSquare = static_cast<ChessSquare &>(src);
    for (auto move : _moves)
    {
        if (move.from == srcSquare.getNotation())
        {
            return true;
        }
    }
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare &srcSquare = static_cast<ChessSquare &>(src);
    ChessSquare &dstSquare = static_cast<ChessSquare &>(dst);

    for (auto move : _moves)
    {
        if (move.from == srcSquare.getNotation() && move.to == dstSquare.getNotation())
        {
            return true;
        }
    }
    return false;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare srcSquare = static_cast<ChessSquare &>(src);
    ChessSquare dstSquare = static_cast<ChessSquare &>(dst);
    int fromCol = srcSquare.getColumn();
    int fromRow = srcSquare.getRow();
    int toCol = dstSquare.getColumn();
    int toRow = dstSquare.getRow();
    std::string piece = pieceNotation(stateString().c_str(), dstSquare.getRow(), dstSquare.getColumn());

    // get last move (for en Passant)
    const char *bpieces = "pnbrqk";
    const char *wpieces = "PNBRQK";
    _lastMove = "x-" + srcSquare.getNotation() + "-" + dstSquare.getNotation();
    _lastMove[0] = (bit.gameTag() < 128) ? wpieces[bit.gameTag() - 1] : bpieces[bit.gameTag() - 129];

    for (auto &move : _moves)
    {
        if (move.from == srcSquare.getNotation() && move.to == dstSquare.getNotation() && move.isEnPassant)
        {
            int row = (bit.gameTag() < 128) ? 3 : 4;
            BitHolder &enPassantSquare = getHolderAt(row, toCol);
            Bit *enPassantBit = enPassantSquare.bit();
            if ((enPassantBit->gameTag() & 127) == ChessPiece::Pawn)
            {
                enPassantSquare.destroyBit();
                // move.isEnPassant = false;
                break;
            }
        }
    }

    // Pawn Promotion
    if (piece[0] == 'P' && toRow == 0)
    {
        // for white pawns
        _grid[toRow][toCol].destroyBit();
        Bit *bit = PieceForPlayer(1, Queen);
        bit->setPosition(_grid[toRow][toCol].getPosition());
        bit->setGameTag(Queen);
        _grid[toRow][toCol].setBit(bit);
    }
    if (piece[0] == 'p' && toRow == 7)
    {
        // for black pawns
        _grid[toRow][toCol].destroyBit();
        Bit *bit = PieceForPlayer(0, Queen);
        bit->setPosition(_grid[toRow][toCol].getPosition());
        bit->setGameTag(Queen + 128);
        _grid[toRow][toCol].setBit(bit);
    }

    // some logic for castling
    // int toRow = dstSquare.getRow();
    // std::cout << srcSquare.getRow() << ',' << srcSquare.getColumn() <<  std::endl << piece << std::endl;
    // has a castling move happened, if so move the rook
    if (piece[0] == 'K' && w_kingMoved == false && toCol == 6)
    {
        // white kingside
        Bit *bit = _grid[7][7].bit();
        _grid[7][5].dropBitAtPoint(bit, ImVec2(7, 5));
    }
    if (piece[0] == 'K' && w_kingMoved == false && toCol == 2)
    {
        // white queenside
        Bit *bit = _grid[7][0].bit();
        _grid[7][3].dropBitAtPoint(bit, ImVec2(7, 3));
    }
    if (piece[0] == 'k' && b_kingMoved == false && toCol == 6)
    {
        // black kingside
        Bit *bit = _grid[0][7].bit();
        _grid[0][5].dropBitAtPoint(bit, ImVec2(0, 5));
    }
    if (piece[0] == 'k' && b_kingMoved == false && toCol == 2)
    {
        // Black Queen side castle
        Bit *bit = _grid[0][0].bit();
        _grid[0][3].dropBitAtPoint(bit, ImVec2(0, 3));
    }
    // has rook been captured
    if (dstSquare.getRow() == 7 && dstSquare.getColumn() == 7 && piece[0] != 'R')
    {
        w_kingRookMoved = true;
    }
    else if (dstSquare.getRow() == 7 && dstSquare.getColumn() == 0 && piece[0] != 'R')
    {
        w_queenRookMoved = true;
    }
    else if (dstSquare.getRow() == 0 && dstSquare.getColumn() == 7 && piece[0] != 'r')
    {
        b_kingRookMoved = true;
    }
    else if (dstSquare.getRow() == 0 && dstSquare.getColumn() == 0 && piece[0] != 'r')
    {
        b_queenRookMoved = true;
    }
    // did king move?
    if (piece[0] == 'K')
    {
        w_kingMoved = true;
    }
    else if (piece[0] == 'k')
    {
        b_kingMoved = true;
    }
    // did rook move?
    if (piece[0] == 'R')
    {
        (fromRow == 7 && fromCol == 0) ? w_queenRookMoved = true : w_kingRookMoved = true;
    }
    else if (piece[0] == 'r')
    {
        (fromRow == 0 && fromCol == 0) ? b_queenRookMoved = true : b_kingRookMoved = true;
    }
    Game::bitMovedFromTo(bit, src, dst);
    _moves = generateMoves(stateString().c_str(), (_gameOptions.currentTurnNo & 1) ? 'B' : 'W', true);
}
void Chess::stopGame()
{
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            _grid[y][x].destroyBit();
        }
    }
}

std::string Chess::indexToNotation(int row, int col)
{
    return std::string(1, 'a' + col) + std::string(1, '8' - row);
}

char Chess::oppositeColor(char color)
{
    return (color == 'W') ? 'B' : 'W';
}
std::string Chess::current_pieceNotation(int row, int col) const
{
    const char *pieces = {"0pnbrqkPNBRQK"};
    std::string notation = "";
    Bit *bit = _grid[row][col].bit();
    if (bit)
    {
        int index = bit->gameTag() & 127;
        if (bit->gameTag() < 128)
        {
            notation += pieces[index + 6]; // white
        }
        else
        {
            notation += pieces[index];
        }
        /*
        notation += bit->gameTag() < 128 ? "W" : "B";
        notation += pieces[bit->gameTag()&127];
        */
    }
    else
    {
        notation = "0"; // return "1" if square is empty
    }
    return notation;
}
std::string Chess::pieceNotation(const char *state, int row, int col) const
{
    std::string notation = "";
    return notation += state[(row * 8) + col];
}
/*
bool Chess::isUnderAttack(const char *state, int row, int col, char opColor){
    std::string target = indexToNotation(row, col);
    std::vector<Move> opponentMoves = generateMoves(state, opColor, false);
    for (const auto& move : opponentMoves){
        if (move.to == target){
            return true;
        }
    }
    return false;
}
*/
void Chess::filterOutIllegalMoves(std::vector<Chess::Move> &moves, char color)
{
    char baseState[65];
    std::string copyState = std::string(stateString().c_str());
    int kingSquare = -1;
    for (int i = 0; i < 64; i++)
    {
        if (copyState[i] == 'k' && color == 'B')
        {
            kingSquare = i;
            break;
        }
        if (copyState[i] == 'K' && color == 'W')
        {
            kingSquare = i;
            break;
        }
    }

    for (auto it = moves.begin(); it != moves.end();)
    {
        bool moveBad = false;
        std::strcpy(&baseState[0], copyState.c_str());
        int srcSquare = notationToIndex(it->from);
        int dstSquare = notationToIndex(it->to);
        baseState[dstSquare] = baseState[srcSquare];
        baseState[srcSquare] = '0';

        // Handle the case in which the king is the piece that moved, and it may have left or remained in check
        int updatedKingSquare = kingSquare;
        if (baseState[dstSquare] == 'k' && color == 'B')
        {
            updatedKingSquare = dstSquare;
        }
        else if (baseState[dstSquare] == 'K' && color == 'W')
        {
            updatedKingSquare = dstSquare;
        }

        auto oppositeMoves = generateMoves(baseState, color == 'W' ? 'B' : 'W', false);
        for (auto enemyMoves : oppositeMoves)
        {
            int enemyDst = notationToIndex(enemyMoves.to);
            if (enemyDst == updatedKingSquare)
            {
                moveBad = true;
                break;
            }
        }

        if (moveBad)
        {
            it = moves.erase(it);
        }
        else
        {
            ++it;
        }
        if (moves.empty())
        {
            for (auto enemyMoves : oppositeMoves)
            {
                // const char currentState = copyState.c_str();
                if (isUnderAttack(copyState.c_str(), kingSquare / 8, kingSquare % 8, color == 'W' ? 'B' : 'W'))
                {
                    _currentGameState = Checkmate;
                    return;
                }
            }
            _currentGameState = Stalemate;
        }
    }
}

std::vector<Chess::Move> Chess::generateMoves(const char *state, char color, bool filter)
{
    std::vector<Move> moves;

    int colorAsInt = (color == 'W') ? 1 : -1;

    for (int i = 0; i < 64; i++)
    {
        // std::islower(piece) ? 'B' : 'W';
        int row = i / 8;
        int col = i % 8;
        char piece = state[i];
        int pieceColor = (piece == '0') ? 0 : (piece < 'a') ? 1
                                                            : -1;
        if (pieceColor == colorAsInt)
        {
            if (piece > 'a')
                piece = piece - ('a' - 'A');
            switch (piece)
            {
            case 'N': // Knight
                generateKnightMoves(state, moves, row, col);
                break;
            case 'P': // Pawn
                generatePawnMoves(state, moves, row, col, color, _lastMove);
                break;
            case 'B': // Bishop
                generateBishopMoves(state, moves, row, col);
                break;
            case 'R': // Rook
                generateRookMoves(state, moves, row, col, color);
                break;
            case 'Q': // Queen
                generateQueenMoves(state, moves, row, col);
                break;
            case 'K': // King
                if (filter)
                    generateKingMoves(state, moves, row, col, color);
                break;
            default:
                break;
            }
        }
    }
    if (filter)
    {
        filterOutIllegalMoves(moves, color);
    }
    return moves;
}

void Chess::generateKnightMoves(const char *state, std::vector<Move> &moves, int row, int col)
{
    static const int movesRow[] = {2, 1, -1, -2, -2, -1, 1, 2};
    static const int movesCol[] = {1, 2, 2, 1, -1, -2, -2, -1};

    for (int i = 0; i < 8; ++i)
    {
        int newRow = row + movesRow[i];
        int newCol = col + movesCol[i];
        addMoveIfValid(state, moves, row, col, newRow, newCol);
    }
}

void Chess::generatePawnMoves(const char *state, std::vector<Move> &moves, int row, int col, char color, std::string lastMove)
{
    int direction = (color == 'W') ? -1 : 1;
    int startRow = (color == 'W') ? 6 : 1;
    int colorAsInt = (color == 'W') ? 1 : -1;
    // if (row == oppositeColor(color))
    //  One square
    if (pieceNotation(state, row + direction, col) == "0")
    {
        addMoveIfValid(state, moves, row, col, row + direction, col);

        // Two Squares
        if (row == startRow && pieceNotation(state, row + 2 * direction, col) == "0")
        {
            addMoveIfValid(state, moves, row, col, row + 2 * direction, col);
        }
    }

    // Take with pawn
    for (int i = -1; i <= 1; i += 2)
    { // -1 for left, +1 for right
        if (col + 1 >= 0 && col + i < 8)
        {
            int oppositeColor = (colorAsInt == -1) ? 1 : -1;
            int pieceColor = stateColor(state, row + direction, col + i);
            if (oppositeColor == pieceColor)
            {
                addMoveIfValid(state, moves, row, col, row + direction, col + i);
            }
        }
    }

    // en passant
    if (lastMove.length())
    {
        char lastMovePiece = lastMove[0];
        int lastMoveStartRow = lastMove[3] - '0';
        int lastMoveEndRow = lastMove[6] - '0';
        int lastMoveStartCol = lastMove[2] - 'a';
        int lastMoveEndcol = lastMove[5] - 'a';

        // white pawns en passant
        if (color == 'W' && row == 3)
        {
            // was last move a black pawn moving two squares?
            if (lastMovePiece == 'p' && lastMoveStartRow == 7 && lastMoveEndRow == 5)
            {
                // was the pawn adjacient to the current pawn
                if (lastMoveEndcol == col - 1 || lastMoveEndcol == col + 1)
                {
                    if (row - 1 >= 0 && row - 1 < 8 && lastMoveEndcol >= 0 && lastMoveEndcol < 8)
                    {
                        if (pieceNotation(state, row, col)[0] != pieceNotation(state, row - 1, lastMoveEndcol)[0])
                        {
                            Move enPassantMove = {indexToNotation(row, col), indexToNotation(row + direction, lastMoveEndcol)};
                            enPassantMove.isEnPassant = true;
                            moves.push_back(enPassantMove);
                        }
                    }
                    // void Chess::addMoveIfValid(std::vector<Move>&moves, int fromRow, int fromCol, int toRow, int toCol){
                    // addMoveIfValid(moves, row, col, row-1, lastMoveEndcol);
                }
            }
        }
        // black pawns en passant
        else if (color == 'B' && row == 4)
        {
            // was last move a white paen moving two squares?
            if (lastMovePiece == 'P' && lastMoveStartRow == 2 && lastMoveEndRow == 4)
            {
                if (lastMoveEndcol == col - 1 || lastMoveEndcol == col + 1)
                {
                    if (row - 1 >= 0 && row - 1 < 8 && lastMoveEndcol >= 0 && lastMoveEndcol < 8)
                    {
                        if (pieceNotation(state, row, col)[0] != pieceNotation(state, row - 1, lastMoveEndcol)[0])
                        {
                            Move enPassantMove = {indexToNotation(row, col), indexToNotation(row + direction, lastMoveEndcol)};
                            enPassantMove.isEnPassant = true;
                            moves.push_back(enPassantMove);
                        }
                    }
                    // addMoveIfValid(moves,row,col,row+1, lastMoveEndcol);
                }
            }
        }
    }
}
char Chess::stateNotation(const char *state, int row, int col)
{
    return state[row * 8 + col];
}
int Chess::stateColor(const char *state, int row, int col)
{
    char piece = stateNotation(state, row, col);
    if (piece == '0')
    {
        return 0;
    }
    return (piece < 'a') ? 1 : -1;
}
//
// Helper to gen linear moves for bishops, rooks, and queens
//
void Chess::generateLinearMoves(const char *state, std::vector<Move> &moves, int row, int col, const std::vector<std::pair<int, int>> &directions)
{
    for (auto &dir : directions)
    {
        int currentRow = row + dir.first;
        int currentCol = col + dir.second;
        while (currentRow >= 0 && currentRow < 8 && currentCol >= 0 && currentCol < 8)
        {
            if (pieceNotation(state, currentRow, currentCol) != "0")
            {
                addMoveIfValid(state, moves, row, col, currentRow, currentCol);
                break; // this will make it stop if piece in the way
            }
            addMoveIfValid(state, moves, row, col, currentRow, currentCol);
            currentRow += dir.first;
            currentCol += dir.second;
        }
    }
}

bool Chess::canKingsideCastle(char color)
{
    if (color == 'W' && !w_kingMoved && !w_kingRookMoved)
    { // check has not moved
        if (pieceNotation(stateString().c_str(), 7, 5) == "0" && pieceNotation(stateString().c_str(), 7, 6) == "0")
        { // check that inbetween squares are empty

            if (!isUnderAttack(stateString().c_str(), 7, 4, 'B') && !isUnderAttack(stateString().c_str(), 7, 5, 'B') && !isUnderAttack(stateString().c_str(), 7, 6, 'B'))
            { // check that king isn't in check and is not castling into check
                return true;
            }
        }
    }
    if (color == 'B' && !b_kingMoved && !b_kingRookMoved)
    { // check has not moved

        if (pieceNotation(stateString().c_str(), 0, 5) == "0" && pieceNotation(stateString().c_str(), 0, 6) == "0")
        { // check that inbetween squares are empty
            if (!isUnderAttack(stateString().c_str(), 0, 4, 'W') && !isUnderAttack(stateString().c_str(), 0, 5, 'W') && !isUnderAttack(stateString().c_str(), 0, 6, 'W'))
            { // check that king isn't in check and is not castling into check
                return true;
            }
        }
    }
    return false;
}

bool Chess::canQueensideCastle(char color)
{
    if (color == 'W' && !w_kingMoved && !w_kingRookMoved)
    { // check has not moved

        if (pieceNotation(stateString().c_str(), 7, 1) == "0" && pieceNotation(stateString().c_str(), 7, 2) == "0" && pieceNotation(stateString().c_str(), 7, 3) == "0")
        { // check that inbetween squares are empty

            if (!isUnderAttack(stateString().c_str(), 7, 4, 'B') && !isUnderAttack(stateString().c_str(), 7, 1, 'B') && !isUnderAttack(stateString().c_str(), 7, 2, 'B') && !isUnderAttack(stateString().c_str(), 7, 3, 'B'))
            { // check that king isn't in check and is not castling into check
                return true;
            }
        }
    }
    if (color == 'B' && !b_kingMoved && !b_kingRookMoved)
    { // check has not moved

        if (pieceNotation(stateString().c_str(), 0, 1) == "0" && pieceNotation(stateString().c_str(), 0, 2) == "0" && pieceNotation(stateString().c_str(), 0, 3) == "0")
        { // check that inbetween squares are empty
            if (!isUnderAttack(stateString().c_str(), 0, 4, 'W') && !isUnderAttack(stateString().c_str(), 0, 1, 'W') && !isUnderAttack(stateString().c_str(), 0, 2, 'W') && !isUnderAttack(stateString().c_str(), 0, 3, 'W'))
            { // check that king isn't in check and is not castling into check
                return true;
            }
        }
    }
    return false;
}

void Chess::generateBishopMoves(const char *state, std::vector<Move> &moves, int row, int col)
{
    static const std::vector<std::pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    generateLinearMoves(state, moves, row, col, directions);
}

void Chess::generateRookMoves(const char *state, std::vector<Move> &moves, int row, int col, char color)
{
    static const std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    generateLinearMoves(state, moves, row, col, directions);
}

void Chess::generateQueenMoves(const char *state, std::vector<Move> &moves, int row, int col)
{
    static const std::vector<std::pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    generateLinearMoves(state, moves, row, col, directions);
}

void Chess::generateKingMoves(const char *state, std::vector<Move> &moves, int row, int col, char color)
{
    if (color == 'W' && canKingsideCastle('W'))
    {
        // std::cout << "Kingside move added" << std::endl;
        moves.push_back({"e1", "g1"});
    }
    if (color == 'W' && canQueensideCastle('W'))
    {
        // std::cout << "Queenside move added" << std::endl;
        moves.push_back({"e1", "c1"});
    }
    if (color == 'B' && canKingsideCastle('B'))
    {
        // std::cout << "Kingside move added" << std::endl;
        moves.push_back({"e8", "g8"});
    }
    if (color == 'B' && canQueensideCastle('B'))
    {
        // std::cout << "Queenside move added" << std::endl;
        moves.push_back({"e8", "c8"});
    }
    static const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (auto &dir : directions)
    {
        int newRow = row + dir.first;
        int newCol = col + dir.second;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8)
        {
            addMoveIfValid(state, moves, row, col, newRow, newCol);
        }
    }
}

bool Chess::isUnderAttack(const char *state, int row, int col, char opColor)
{
    std::string target = indexToNotation(row, col);
    std::vector<Move> opponentMoves = generateMoves(state, opColor, false);
    for (const auto &move : opponentMoves)
    {
        if (move.to == target)
        {
            return true;
        }
    }
    return false;
}

/* OLD IS THERE CHECK
std::string Chess::isThereCheck(){
    std::pair<int,int> w_king;
    std::pair<int,int> b_king;
    for (int row=0; row<8; ++row){
        for(int col=0; col<8;++col){
            std::string piece = pieceNotation(row,col);
            if(piece[0] == 'K'){
                w_king = {row, col};
            }
            if(piece[0] == 'k'){
                b_king = {row, col};
            }
        }
    }

    if(isUnderAttack(w_king.first, w_king.second, 'B', 0)){
        return "W+";
    }
    if(isUnderAttack(b_king.first, b_king.second, 'W', 0)){
        return "B+";
    }
    return "";
}
*/
int Chess::notationToIndex(std::string &notation)
{
    int file = notation[0] - 'a';
    int rank = 8 - (notation[1] - '0');
    int square = rank * 8 + file;
    return square;
}

void Chess::updateAI()
{
    char baseState[65];
    int bestMoveScore = -9999999;
    std::string copyState = stateString();
    Move bestMove;
    for (auto move : _moves)
    {
        std::strcpy(&baseState[0], copyState.c_str());
        // black is -1, this is the move for black.
        int srcSquare = notationToIndex(move.from);
        int dstSquare = notationToIndex(move.to);
        baseState[dstSquare] = baseState[srcSquare];
        baseState[srcSquare] = '0';
        _countSearch = 0;
        int bestValue = -negamax(baseState, 3, 1, -9999999, 9999999);
        if (bestValue > bestMoveScore)
        {
            bestMoveScore = bestValue;
            bestMove = move;
        }
    }
    std::cout << "searched " << _countSearch << " moves" << std::endl;
    if (bestMoveScore != -9999999)
    {
        int srcSquare = notationToIndex(bestMove.from);
        int dstSquare = notationToIndex(bestMove.to);
        BitHolder &src = getHolderAt(srcSquare / 8, srcSquare & 7);
        BitHolder &dst = getHolderAt(dstSquare / 8, dstSquare & 7);
        Bit *bit = src.bit();
        dst.dropBitAtPoint(bit, ImVec2(0, 0));
        src.setBit(nullptr);
        bitMovedFromTo(*bit, src, dst);
    }
}

int Chess::evaluateBoard(const char *state)
{

    int score = 0;
    for (int i = 0; i < 64; i++)
    {
        score += pieceScores[state[i]];
    }
    for (int i = 0; i < 64; i++)
    {
        char piece = state[i];
        int j = FLIP(i);
        switch (piece)
        {
        case 'N': // Knight
            score += knightTable[i];
            break;
        case 'n': // Black Knight
            score -= knightTable[FLIP(i)];
            break;
        case 'P': // Pawn
            score += pawnTable[i];
            break;
        case 'p': // black pawn
            score -= pawnTable[FLIP(i)];
            break;
        case 'B': // Bishop
            score += bishopTable[i];
            break;
        case 'b': // black bishop
            score -= bishopTable[FLIP(i)];
            break;
        case 'R': // Rook
            score += rookTable[i];
            break;
        case 'r': // black rook
            score -= rookTable[FLIP(i)];
            break;
        case 'Q': // Queen
            score += queenTable[i];
            break;
        case 'q': // black queen
            score -= queenTable[FLIP(i)];
            break;
        case 'K': // King
            score += kingTable[i];
            break;
        case 'k': // Black King
            score -= kingTable[FLIP(i)];
            break;
        default:
            break;
        }
    }
    return score;
}

int Chess::negamax(char *state, int depth, int playerColor, int alpha, int beta)
{
    _countSearch++;

    // white is 1
    // black is -1
    if (depth == 0)
    {
        int score = evaluateBoard(state);
        return playerColor * score;
    }

    int bestVal = -9999999;
    auto negaMoves = generateMoves(state, (playerColor == 1) ? 'W' : 'B', true);
    for (auto move : negaMoves)
    {
        int srcSquare = notationToIndex(move.from);
        int dstSquare = notationToIndex(move.to);
        // save the move and make the move to the dest square
        char saveMove = state[dstSquare];
        state[dstSquare] = state[srcSquare];
        state[srcSquare] = '0';
        int val = std::max(bestVal, -negamax(state, depth - 1, -playerColor, -beta, -alpha));
        // undo state move
        state[srcSquare] = state[dstSquare];
        state[dstSquare] = saveMove;

        bestVal = std::max(bestVal, val);
        alpha = std::max(alpha, val);

        if (alpha >= beta)
        {
            break;
        }
    }
    return bestVal;
}

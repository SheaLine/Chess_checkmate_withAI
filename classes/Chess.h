#pragma once
#include "Game.h"
#include "ChessSquare.h"
#include <iostream>

const int pieceSize = 64;

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

enum ChessPiece {
    NoPiece = 0,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
};

class Chess : public Game
{
public:

    enum GameState{
        Ongoing,
        Checkmate,
        Stalemate
    };
    GameState _currentGameState;
    //Castling bools
    bool w_kingMoved = false;
    bool w_kingRookMoved = false;
    bool w_queenRookMoved = false;
    bool b_kingMoved = false;
    bool b_kingRookMoved = false;
    bool b_queenRookMoved = false;
    bool w_canCastleK = false;
    bool w_canCastleQ = false;
    bool b_canCastleK = false;
    bool b_canCastleQ = false;
    Chess();
    ~Chess();
    void test();

    struct Move {
        std::string from;
        std::string to;
        bool isEnPassant = false; // sets a flag to reconize when en passant move happens
    };
    

     // set up the board
    void setUpBoard() override;

    Player* checkForWinner() override;
    bool checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    bool actionForEmptyHolder(BitHolder &holder) override {return false;}
    bool canBitMoveFrom(Bit&bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void bitMovedFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override; //16.44
    void stopGame() override;
    void updateAI() override;
    BitHolder &getHolderAt(const int x, const int y) override { return _grid[x][y]; }
    bool End = false; // * Added to Stop piece placement after game over
    std::vector<Chess::Move> _moves;
    std::string boardToFen() const;
    void fenToBoard(const std::string& fen);
    std::string isThereCheck();
private:
    Bit*        PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player*     ownerAt(int index ) const;
    void        scanForMouse();
    void        addMoveIfValid(const char *state, std::vector<Move>&moves, int fromRow, int fromCol, int toRow, int toCol);
    std::string indexToNotation(int row, int col);
    std::string pieceNotation(const char *state, int row, int col) const;
    std::string current_pieceNotation(int row, int col) const;
    void        generateKnightMoves(const char *state, std::vector<Move>&moves, int row, int col);
    void        generatePawnMoves(const char *state, std::vector<Move>&moves, int row, int col, char color, std::string lastMove);
    void        generateLinearMoves(const char *state, std::vector<Move>&moves, int row, int col, const std::vector<std::pair<int,int>> &directions);
    void        generateBishopMoves(const char *state, std::vector<Move>&moves, int row, int col);
    void        generateRookMoves(const char *state, std::vector<Move>&moves, int row, int col, char color);
    bool        canKingsideCastle(char color);
    bool        canQueensideCastle(char color);
    bool        isUnderAttack(const char *state, int row, int col, char opColor);
    void        generateQueenMoves(const char *state, std::vector<Move>&moves, int row, int col);
    void        generateKingMoves(const char *state, std::vector<Move>&moves, int row, int col, char color);
    char        oppositeColor(char color);
    int         negamax(char* state, int depth, int playerColor, int alpha, int beta);
    int         notationToIndex(std::string& notation);
    int         evaluateBoard(const char* state);
    int         stateColor(const char* state, int row, int col);
    char        stateNotation(const char* state, int row, int col);
    void        filterOutIllegalMoves(std::vector<Chess::Move>& moves, char color);

    std::vector<Chess::Move> generateMoves(const char *state, char color, bool filter);
    ChessSquare      _grid[8][8];
    std::string      _lastMove;
    std::string      secondto_LastMove = "";
    std::string      enPassant;
    int             halfmove;
    int             fullmove;
    int             _countSearch;
};

#pragma once
#include "Game.h"
#include "Square.h"
#include <iostream>


//
// the classic game of Tic-Tac-Toe
//
const int pieceSize = 100;
const int HUMAN_PLAYER = -1;
const int AI_PLAYER = 1;
const int INFINTY = 10000;
const int NEG_INFINTY = -10000;

//
// class for the "good" Tic-Tac-Toe AI
//
class TicTacToeAI
{
    public:
        int _grid[3][3];
        bool isBoardFull() const;
        int evalBoard();
        int negamax(TicTacToeAI* state, int depth, int playerColor);
        int ownerAt(int index) const;
        int AICheckForWinner();
};
class TicTacToe : public Game
{
public:
    TicTacToe();
    ~TicTacToe();

    // set up the board
    void setUpBoard() override;

    Player* checkForWinner() override;
    bool checkForDraw() override;
    //TicTacToeAI* clone(); 
    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    bool canBitMoveFrom(Bit&bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void stopGame() override;
    void updateAI() override;
    BitHolder &getHolderAt(const int x, const int y) override { return _grid[x][y]; }
    bool End = false; // * Added to Stop piece placement after game over
    
    TicTacToeAI* clone(){
        TicTacToeAI* newState = new TicTacToeAI();
        std::string newString = stateString();
        for (int y=0; y<3; y++){
            for (int x =0; x<3;x++){
                int index = y * 3 + x;
                int playerNumber = newString[index] - '0';
                newState->_grid[y][x] = playerNumber == 2 ? HUMAN_PLAYER : playerNumber == 1 ? AI_PLAYER : 0;
            }
        }
        return newState;
    }
private:
    Bit *PieceForPlayer(const int playerNumber);
    Player* ownerAt(int index );
    void    scanForMouse();
    Square   _grid[3][3];
};


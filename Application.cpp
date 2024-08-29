#include "Application.h"
#include "imgui/imgui.h"
#include "classes/Chess.h"

namespace ClassGame {
        //
        // our global variables
        //
        Chess *game = nullptr;
        int gameWinner = 0;

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = new Chess();
            game->setUpBoard();
            gameWinner = -1;
            game->_currentGameState = Chess::Ongoing;
            
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame() 
        {
                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

                ImGui::Begin("Settings");
                ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                ImGui::Text("Current Board State: %s", game->stateString().c_str());
                if (game->_currentGameState == Chess::Checkmate){
                    game->getCurrentPlayer()->playerNumber() == 1 ? ImGui::Text("Checkmate! White wins!") : ImGui::Text("Checkmate! Black wins!");
                }else{
                    if (game->_currentGameState == Chess::Stalemate){
                        ImGui::Text("Stalemate!");
                    }
                }

                if (game->checkForDraw()) {
                    game->End = true;
                    ImGui::Text("Game Over!");
                    ImGui::Text("Draw!");
                } else {
                    if (gameWinner != -1) {
                        ImGui::Text("Game Over!");
                        ImGui::Text("Winner: %d", gameWinner);
                    }
                }
                /*
                if (ImGui::Button("Reset Game")) {
                    game->stopGame();
                    game->setUpBoard();
                    gameWinner = -1;
                    game->End = false; // set End back to false;
                }
                */
                if (ImGui::Button("Play Against AI")){
                    game->stopGame();
                    game->setUpBoard();
                    game->_currentGameState = Chess::Ongoing;
                    gameWinner = -1;
                    game->End = false;
                    game->setAIPlayer(1);
                    game->_gameOptions.AIPlaying = true;  
                    game->w_kingMoved = false;
                    game->w_kingRookMoved = false;
                    game->w_queenRookMoved = false;
                    game->b_kingMoved = false;
                    game->b_kingRookMoved = false;
                    game->b_queenRookMoved = false; 
                    game->w_canCastleK = false;
                    game->w_canCastleQ = false;
                    game->b_canCastleK = false;
                    game->b_canCastleQ = false;             
                }
                if (ImGui::Button("Play Against Human")){
                    game->stopGame();
                    game->setUpBoard();
                    game->_currentGameState = Chess::Ongoing;
                    gameWinner = -1;
                    game->End = false;
                    game->_gameOptions.AIPlaying = false;
                    game->w_kingMoved = false;
                    game->w_kingRookMoved = false;
                    game->w_queenRookMoved = false;
                    game->b_kingMoved = false;
                    game->b_kingRookMoved = false;
                    game->b_queenRookMoved = false;
                    game->w_canCastleK = false;
                    game->w_canCastleQ = false;
                    game->b_canCastleK = false;
                    game->b_canCastleQ = false;   
                }
                ImGui::End();

                if (game->gameHasAI() && game->getCurrentPlayer()->isAIPlayer())
                {
                    game->updateAI();
                    //EndOfTurn();
                }

                ImGui::Begin("GameWindow");
                game->drawFrame();
                ImGui::End();
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn() 
        {
            Player *winner = game->checkForWinner();
            if (winner)
            {
                gameWinner = winner->playerNumber();
                game->End = true; // no more piece placement if winner is declared
            }
        }
}

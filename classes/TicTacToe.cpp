#include "TicTacToe.h"
#include "game.h"
#include "Square.h"



TicTacToe::TicTacToe()
{
}

TicTacToe::~TicTacToe()
{
}

//
// make an X or an O piece for the player
//
Bit* TicTacToe::PieceForPlayer(const int playerNumber)
{
    //const char *textures[] = { "rps_rock.png", "rps_paper.png", "rps_scissors.png" };
    //int random = rand() % 2;

    Bit *bit = new Bit();
    // should possibly be cached from player class?
    if (playerNumber == 1){
        bit->LoadTextureFromFile("o.png");
        bit->setGameTag(1);
    }
    if (playerNumber == 0){
        bit->LoadTextureFromFile("x.png");
        bit->setGameTag(2);
    }
    bit->setOwner(getCurrentPlayer());
    bit->setSize(pieceSize, pieceSize);
    return bit;
}

void TicTacToe::setUpBoard()
{
    srand((unsigned int)time(0));
    setNumberOfPlayers(2);
    // this allows us to draw the board correctly
    _gameOptions.rowX = 3;
    _gameOptions.rowY = 3;
    // setup the board
    for (int x=0; x<3; x++) {
        for(int y=0; y<3; y++){
        _grid[x][y].initHolder(ImVec2(pieceSize*(float)x + 100, pieceSize*(float)y + 100), "square.png", x, y);
        }
    }
    // if we have an AI set it up
    if (gameHasAI())
    {
        setAIPlayer(_gameOptions.AIPlayer);
    }
    // setup up turns etc.
    startGame();
}

//
// about the only thing we need to actually fill out for Tic Tac Toe
//
bool TicTacToe::actionForEmptyHolder(BitHolder& holder)
{
    
    if (holder.bit()) {
        return false;
    }
    if (!End) // if game not over you can place a piece
    {
        Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber());
        if (bit) {
            bit->setPosition(holder.getPosition());
            holder.setBit(bit);
            bit->setOwner(getPlayerAt(getCurrentPlayer()->playerNumber()));
            endTurn();
            return true;
        }
    }   
    return false;
}

bool TicTacToe::canBitMoveFrom(Bit& bit, BitHolder& src)
{
    // you can't move anything in Tic Tac Toe
    return false;
}

bool TicTacToe::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // you can't move anything in Tic Tac Toe
    return false;
}

//
// free all the memory used by the game on the heap
//
void TicTacToe::stopGame()
{

    for (int x=0; x<3; x++) {
        for (int y=0; y<3; y++){
            _grid[x][y].destroyBit();
        }
        
    }
}

//
// helper function for the winner check
// checks to see who owns what square
//
Player* TicTacToe::ownerAt(int index )
{
    /*
    * ****For RPS****
    if (index < 0 || index > 1) {
        return nullptr;
    }
    if (!_grid[index].bit()) {
        return nullptr;
    }
    return _grid[index].bit()->getOwner();
    */
    if (!_grid[index/3][index%3].bit()){
        return nullptr;
    }
    return _grid[index/3][index%3].bit()->getOwner();
}

Player* TicTacToe::checkForWinner()
{
    /*
    * ***For RPS***
    int rps0 = _grid[0].bit() ? _grid[0].bit()->gameTag() : 0;
    int rps1 = _grid[1].bit() ? _grid[1].bit()->gameTag() : 0;
    // rock = 1, paper = 2, scissors = 3
    if (rps0 == 0 || rps1 == 0 || rps0 == rps1) {
        return nullptr;
    } else if ((rps0 - rps1 + 3) % 3 == 1) {
        return _players[0];
    } else {
        return _players[1];
    }    
    return nullptr;
    * *****
    */
    static const int WinningTriples[8][3] = {   {0,1,2}, {3,4,5}, {6,7,8},
                                                {0,3,6}, {1,4,7}, {2,5,8},
                                                {0,4,8}, {2,4,6} };
    for (int i = 0; i < 8; i++){
        const int *triple = WinningTriples[i];
        Player *player = ownerAt(triple[0]);
        if (player && player == ownerAt(triple[1]) && player == ownerAt(triple[2]))
        return player;
    }
    return nullptr;
}

bool TicTacToe::checkForDraw()
{
    /*
    int rps0 = _grid[0].bit() ? _grid[0].bit()->gameTag() : 0;
    int rps1 = _grid[1].bit() ? _grid[1].bit()->gameTag() : 0;
    // rock = 1, paper = 2, scissors = 3
    if ((rps0 != 0 && rps1 != 0) && rps0 == rps1) {
        return true;
    }
    return false;
    */
    if (checkForWinner() == nullptr){
        for (int x = 0; x<3; x++){
            for (int y = 0; y<3; y++){
                if (!_grid[x][y].bit()){ // checks to see if there is empty square
                    return false; // if there is an empty square there is no draw
                }
            }
        }
        return true;
    }
    else{
        return false;
    }
}

//
// state strings
//
std::string TicTacToe::initialStateString()
{
    return "00";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string TicTacToe::stateString()
{
    std::string s;
    for (int y=0; y<3; y++) {
        for (int x=0; x<3; x++){
            Bit *bit = _grid[y][x].bit();
            if (bit) {
                s += std::to_string(bit->gameTag());
            } else {
                s += "0";
            }
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void TicTacToe::setStateString(const std::string &s)
{
    for (int y=0; y<3; y++) {
        for (int x= 0; x<3; x++){
            _grid[y][x].setBit( PieceForPlayer(0) );
        
        }
    }
}




//
// This is where the AI decides which square to play on
//

void TicTacToe::updateAI(){

    // * ***************** FOR Negamax AI *********************
    
    int bestVal = -1000;
    Square* bestMove = nullptr;
    for (int y = 0; y<3; y++){
            for (int x = 0; x<3; x++){
                // check if cell is empty
                if (!_grid[y][x].bit()){

                    //make the move
                    _grid[y][x].setBit(PieceForPlayer(getCurrentPlayer()->playerNumber()));
                    TicTacToeAI* newState = this->clone();
                    int moveVal = newState->negamax(newState, 0, 1);
                    delete newState;
                    //undo move for next time
                    _grid[y][x].setBit(nullptr);

                    // if value of the corrent move is better, update best value
                    if (moveVal > bestVal){
                        bestMove = &_grid[y][x];
                        bestVal = moveVal;
                    }
                }
            }            //if (bestMove != nullptr)
                //actionForEmptyHolder(*bestMove);
    }
    if (bestMove == nullptr){
        //std::cout << "BestMove ERROR" << std::endl;
        End = true;
        //stopGame();
        return;
    }
    actionForEmptyHolder(*bestMove);
    //int BestMove_x = bestMove->getX();
    // int BestMove_y = bestMove->getY();
    //std::cout <<   std::endl;
    
     // * ***************** FOR RANDOM AI *********************
     /*
    std::vector<std::pair<int, int>> emptySquares; // declares list for the (x,y) of each empty square
	for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
	        if (!_grid[x][y].bit()){
	            emptySquares.push_back(std::make_pair(x, y)); // if a square is empty add it to the list
			}
		}
	}
    if(!emptySquares.empty()){ //only play if there is an empty square
        srand((unsigned int)time(0));
        int randomIndex = rand() % emptySquares.size(); // find a random index
        std::pair<int, int> randomSquare = emptySquares[randomIndex]; 

        BitHolder &selectedHolder = _grid[randomSquare.first][randomSquare.second];
        actionForEmptyHolder(selectedHolder);
    }
    */
}

//
// Function to see who owns a square used in AI to check for winner
//
int TicTacToeAI::ownerAt(int index) const{
    int row = index / 3;
    int col = index % 3;
    return _grid[row][col];
}

//
// Function to check if the board state is a winner and give score
//
int TicTacToeAI::AICheckForWinner()
{
    static const int WinningTriples[8][3] = {   {0,1,2}, {3,4,5}, {6,7,8},
                                                {0,3,6}, {1,4,7}, {2,5,8},
                                                {0,4,8}, {2,4,6} };
    for (int i = 0; i < 8; i++){
        const int *triple = WinningTriples[i];
        int playerInt = ownerAt(triple[0]);
        if(playerInt != 0 && playerInt == ownerAt(triple[1]) && playerInt == ownerAt(triple[2]))
        {
            return playerInt;
        }
    }
    return 0;
}
//
// Function to see if board is full, checks for draw in AI
//
bool TicTacToeAI::isBoardFull() const{
    for (int y = 0; y<3; y++){
            for (int x = 0; x<3; x++){
                if (!_grid[y][x]){ // checks to see if there is empty square
                    return false; // if there is an empty square there is no draw
                }
            }
        }
        return true;
}

//
// Function to evaluate the board for the AI
//
int TicTacToeAI::evalBoard()
{
    // Check for winner
    int winner = AICheckForWinner();
    if (winner == 0){
        return 0; // No Winner yet
    }
    if (winner == HUMAN_PLAYER){
        return -10; // Human wins, negative score
    }
    else if(winner == AI_PLAYER){
        return 10; // AI wins, positive score
    }
    return -1; //error
}
//
// This is the negamax algorithim
// 
int TicTacToeAI::negamax(TicTacToeAI* state, int depth, int playerColor)
{
    int score = state->evalBoard();

    // if AI wins, human wins, or draw
    if(score == 10) return score - depth;
    if(score == -10) return -score - depth;
    if(state->isBoardFull()) return 0;
    
    int bestVal = -1000; // Min value
    for (int y=0; y<3;y++){
        for(int x=0;x<3;x++){
            // Check if cell is empty
            if(!state->_grid[y][x]){
                // Make the move
                //std::cout << playerColor << std::endl;
                state->_grid[y][x] = playerColor == 1 ? (HUMAN_PLAYER) : (AI_PLAYER);

                bestVal = std::max(bestVal, -negamax(state, depth+1, -playerColor));
                // Undo the move so it can eval other moves
                state->_grid[y][x] = 0;
            }
        }
    }
    return bestVal;
}
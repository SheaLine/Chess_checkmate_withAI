Chess Game Overview:
Graphical game of Chess. Allows for play agaisnt another human.

Development Process:

The biggest challenge with implamenting the AI was dealing with the state strings,
My board indexes starting with A8 so my state string starts with black pieces. This is opposite of the example that was shown in class.
This created an issue when I was converting the notation to index. I had to do a little different math than was shown in class. 
I also found very small bugs in my castling logic while I was testing my AI. Queen side castling didn't work as it should and also the detection of rook movement wasn't working correctly.
These problems were fixed by changing a few of my if statement logic. 

The AI:
I am able to reach a depth of 4. During the mid game the AI takes awhile (at most like 45 seconds), this is probably due to the way I checking wether castling is possible. 
If the castling squares are empty, the game generates moves for the other color to see if it is attacking the castling squares to see if castleing is legal or not. 
I am actually having trouble beating this AI. I played it against some of the AI bots
on chess.com and they also couldn't really beat it. Its hard to know everything becasue castling, en passant, and pawn promotion aren't simulated. 
The AI now knows about check, checkmate, and stalemate. A full game of chess can be played. The AI will play very slow if you run in debug mode, so if you want to play a full game run in release mode.

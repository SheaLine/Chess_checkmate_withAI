#pragma once

#include "BitHolder.h"

class Square : public BitHolder
{
public:
    Square() : BitHolder()
    {
        _column = 0;
        _row = 0;
    }
   int getX(){
        return _row;
    }
    int getY(){
        return _column;
    }
    //Square(const Square& other) : BitHolder(other), _column(other._column), _row(other._row){}
    // initialize the holder with a position, color, and a sprite
    void initHolder(const ImVec2 &position, const char *spriteName, const int column, const int row);

private:
    int _column;
    int _row;
};

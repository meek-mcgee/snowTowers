#ifndef ASCII_TOOLS_H
#define ASCII_TOOLS_H

#include <iostream>
#include <vector>
#include <ncurses.h>

class Player {
    private:
        char sprite;
        bool isDead;
        int x;
        int y;
        int width;
        int height;
        enum Direction{LEFT = 'a', RIGHT = 'd', UP = 'w', DOWN = 's'};
    public:
        Player(int x_pos, int y_pos, int w, int h);
        Player(int x_pos, int y_pos, int w, int h, char charSprite);
        int getX();
        int getY();
        void setXY(int x1, int y1);
        char getSprite();
        void move(char d);
};
class Map {
    private:
        int x;
        int y;
        std::vector< std::vector<char> > map;
    public:
        void addSprites(std::vector<Player> elements);
        void initMap(std::vector< std::vector<char> > yourMap);
        void checkBounds(Player &hero);
        std::vector< std::vector<char> > getMap();
        void printMap(short colorPair1, short colorPair2, short colorPair3);
        Map(int x_size, int y_size);
};

#endif /* ASCII_TOOLS_H */
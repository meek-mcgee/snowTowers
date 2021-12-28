#include "asciiTools.h"

Player::Player(int x_pos, int y_pos, int w, int h){
    x = x_pos;
    y = y_pos;
    width = w;
    height = h;
    isDead = false;
    sprite = 'X';
}
Player::Player(int x_pos, int y_pos, int w, int h, char charSprite){
    x = x_pos;
    y = y_pos;
    width = w;
    height = h;
    isDead = false;
    sprite = charSprite;
}
int Player::getX(){ return x; }
int Player::getY(){ return y; }
void Player::setXY(int x1, int y1){
    x = x1;
    y = y1;
}
char Player::getSprite(){ return sprite; }
void Player::move(char d){
    switch(d){
        case LEFT:
            if(x != 0) x--;
            break;
        case RIGHT: 
            x++;
            break;
        case UP:
            if(y != 0) y--;
            break;
        case DOWN:
            y++;
            break;
        default:
            break;
    }
}
void Map::addSprites(std::vector<Player> elements){
    initMap(map); //clears map 
    for(int i = 0; i < elements.size(); i++){
        if((0 <= elements[i].getX() < x) && (0 <= elements[i].getY() < y))
            map[elements[i].getY()][elements[i].getX()] = elements[i].getSprite();
    }
}
void Map::initMap(std::vector< std::vector<char> > yourMap){
    yourMap.clear(); //clears array of all elements
    for(int i = 0; i < y; i++){//fill map with '.'
        std::vector<char> v;
        for(int j = 0; j < x; j++){
                v.push_back('.');
        }
        yourMap.push_back(v);
    }
    map = yourMap;
}
void Map::checkBounds(Player &hero){
    if(hero.getX() <= 0) hero.setXY(0, hero.getY());
    if(hero.getX() >= x) hero.setXY(x - 1, hero.getY());
    if(hero.getY() <= 0) hero.setXY(hero.getX(), 0);
    if(hero.getY() >= y) hero.setXY(hero.getX(), y - 1);
}
std::vector< std::vector<char> > Map::getMap(){ return map; }
void Map::printMap(short colorPair1, short colorPair2, short colorPair3){
    for(int i = 0; i < y; i++){//column
        for(int j = 0; j < x; j++){//row
            short currentPair = 0;
            if(map[i][j] == '.') currentPair = colorPair1;
            if(map[i][j] == '3') currentPair = colorPair2;
            if(map[i][j] == 'X') currentPair = colorPair3;
            attron(COLOR_PAIR(currentPair));
            mvaddch(i, j, map[i][j]);
            attroff(COLOR_PAIR(currentPair));
        }
    }
}
Map::Map(int x_size, int y_size){
    x = x_size;
    y = y_size;
    initMap(map);
}
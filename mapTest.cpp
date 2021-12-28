#include <math.h>
#include "asciiTools.h"

using namespace std;

const int MAP_X = 9;
const int MAP_Y = 9;

int main(){
    Map gameMap(MAP_X, MAP_Y);
    vector<Player> gameObjects;
    Player hero(5, 5, 1, 1);
    Player enemy(5, 0, 1, 1, '3');
    vector<Player> enemyList;
    enemyList.push_back(enemy);
    gameObjects.push_back(hero);
    gameMap.addSprites(gameObjects);
    gameMap.printMap();

    bool playing = true;
    int frames = 0; //gameplay time counter

    while(playing){ //game loop
        char playerInput;
        cout << "frame: " << frames << endl;
        cout << "use wasd to move, press z to quit" << endl;
        cin >> playerInput;
        if(frames == 15){
            Player newEnemy(rand()%10, 0, 1, 1, '3');
            enemyList.push_back(newEnemy);
            frames = 0;
        }
        if(playerInput != 'z') { //check collisions
            hero.move(playerInput);
            gameMap.checkBounds(hero);
            if(gameMap.getMap()[hero.getY()][hero.getX()] != '.'){ //if overlapping object, reset hero to previous position
                hero.setXY(gameObjects[0].getX(), gameObjects[0].getY());
            }
            for(int i = 0; i < enemyList.size(); i++){
                enemyList[i].move('s');
                gameMap.checkBounds(enemyList[i]);
                if(gameMap.getMap()[enemyList[i].getY()][enemyList[i].getX()] == '3'){
                    enemyList[i] = gameObjects[i + 1]; //+1 to account for fact hero always takes the zero position
                }
            }
        }
        else playing = false;

        gameObjects.clear();
        gameObjects.push_back(hero);
        for(int i = 0; i < enemyList.size(); i++) gameObjects.push_back(enemyList[i]);
        gameMap.addSprites(gameObjects);
        gameMap.printMap();
        cout << "hero vari: " << hero.getX() << " " << hero.getY() << endl;
        cout << "gameObjects hero " << gameObjects[0].getX() << " " << gameObjects[0].getY() << endl;
        cin.clear(); //clear input stream of error flags
        frames++;
    }
    return 0;
}

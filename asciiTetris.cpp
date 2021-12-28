#include <math.h>
#include <thread>
#include "asciiTools.h"
#include "synthVoice.h"

using namespace std;

static int MAP_X;
static int MAP_Y;
bool DRONE_ON;

int readInputCallback(char &input){
    //input = getch();
    input = getch();
    return 0;
}
void prinicipalFn(int (*callbackFn)(char &input), char &input){
    (*callbackFn)(input);
}
/*declare callback function. This is the function that is called
//whenever the PortAudio engine has captured audio data or needs
//more audio data for output.*/
static int asciiTetrisCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    wavetable *data = (wavetable*)userData;
    float *out = (float*)outputBuffer;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
    
    for(int i=0; i<framesPerBuffer; i++ )
    {
        //Trigger envelope scales amplitude with an integer between 0 and 1 i.e. where 1 allows full amplitude and 0 is silence
        *out++ = 0.1 * (data->envelope.triggerEnvelope() * data->amplitude * LookupSine(data, data->phase));
        data->phase += data->phaseIncrement;
        if(data->phase >= 1.0f) data->phase -= 1.0f;
    }
    return paContinue;
}

class Timer {
    private:
        time_t startClock;
    public:
        Timer(){
            startClock = 0;
        }
        void startTimer(){
            startClock = clock();
        }
        double elapsedTime(){
            return (double)(clock() - startClock)/CLOCKS_PER_SEC;
        }
};

void initScale(vector <double> &scale, double maxFreq){
    for(int i = 0; i < MAP_Y; i++){
        if(i == 0) scale.push_back(maxFreq);
        else{
            //don't divide by zero
            double freqDiv = maxFreq/((double)i + 1.0);
            scale.push_back(freqDiv);
        }
        
    }
}
void updateScale(vector <double> &scale, double maxFreq){
    for(int i = 0; i< scale.size(); i++){
        if(i == 0) scale[i] = maxFreq;
        else{
            //don't divide by zero
            double freqDiv = maxFreq/((double)i + 1.0);
            scale[i] = freqDiv;
        }
    }
}
void initSynthVoice(vector <SynthVoice> &voices, vector <double> scale, bool isNotInitialized){
    //initialize synth voices
    for(int i = 0; i < MAP_X; i++){
        if(i == 0) {
            SynthVoice v(isNotInitialized, 0.01, scale[i], 0.0, calcPhaseIncrement(scale[i]));
            voices.push_back(v);
        }
        else{
            //bool should always run false here so don't change the false flag to isNotInitialized
            SynthVoice v(false, 0.01 + (double)i, scale[i], 0.0, calcPhaseIncrement(scale[i]));
            voices.push_back(v);
        }
        //std::cout << "pitch: " << activeVoices[i].oscillator.pitch << endl;
    }
}
void flushEnemies(vector <Player> &e){
    e.clear();
    srand((int)clock());
}
void resetAllVoices(vector <SynthVoice> &voices, vector <double> scale){
    for(int i = 0; i < voices.size(); i++){
        voices[i].endStreaming();
    }
    Pa_Sleep(1000.0);
    initSynthVoice(voices, scale, true);
    for(int i = 0; i < voices.size(); i++){
        voices[i].beginStreaming(false, SAMPLE_RATE, FRAMES_PER_BUFFER, asciiTetrisCallback);
    }
}
void updateSynthVoice(vector <SynthVoice> &voices, vector <double> scale){
    for(int i = 0; i < voices.size(); i++){
        voices[i].oscillator.pitch = scale[i];
        voices[i].oscillator.phaseIncrement = calcPhaseIncrement(scale[i]);
    }
}
double changeScale(){
    clear();
    nocbreak();
    nodelay(stdscr, false);
    echo();
    printw("Enter random frequency (values are doubled so 880 is equal to A440): ");
    char c = 0;
    string freq;
    while(c != '\n'){
        c = getch();
        freq.push_back(c);
    }

    double v = std::stoi(freq);
    cbreak();
    nodelay(stdscr, true);
    noecho();
    clear();
    return v;
}
double changeBPM(){
    clear();
    nocbreak();
    nodelay(stdscr, false);
    echo();
    printw("Enter BPM: ");
    char c = 0;
    string beats;
    while(c != '\n'){
        c = getch();
        beats.push_back(c);
    }

    double v = std::stoi(beats);
    cbreak();
    nodelay(stdscr, true);
    noecho();
    clear();
    return v;
}
void initiateGameConstants(double &freq, double &beatsPerMin){
    echo();
    nodelay(stdscr, false);
    nocbreak();
    printw("# of voices (1-36): ", "***higher values may result in severe pops and glitches***");
    string i;
    char c;
    while(c != '\n'){
        c = getch();
        i.push_back(c);
    }
    MAP_X = MAP_Y = (int)std::stoi(i);
    noecho();
    nodelay(stdscr, true);
    cbreak();
    freq = changeScale();
    beatsPerMin = changeBPM();
    DRONE_ON = false;
}

int main(){
    vector < SynthVoice > activeVoices;
    srand((int)clock());
    //initialize curses
    initscr();
    noecho(); //don't echo text to screen
    nodelay(stdscr, true); //don't wait for input to continue loop
    cbreak(); //switch input in terminal to raw mode
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_WHITE);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    attrset(COLOR_PAIR(4));
    //initialize scale 
    vector <double> scale;
    double maxFreq = 0;
    double bpm = 0;
    initiateGameConstants(maxFreq, bpm);
    initScale(scale, maxFreq);
    
    //initialize synth voices
    initSynthVoice(activeVoices, scale, true);

    //open audio streams
    for(int i = 0; i < MAP_X; i++){
        activeVoices[i].beginStreaming(false, SAMPLE_RATE, FRAMES_PER_BUFFER, asciiTetrisCallback);
    }

    //initialize map
    Map gameMap(MAP_X, MAP_Y);
    vector<Player> gameObjects;
    Player hero(0, (double)(MAP_Y/2.0), 1, 1);
    //Player enemy(5, 0, 1, 1, '3');
    vector<Player> enemyList;
    //enemyList.push_back(enemy);
    gameObjects.push_back(hero);
    gameMap.addSprites(gameObjects);
    gameMap.printMap(1, 2, 3);

    bool playing = true;
    int frames = 1; //gameplay time counter
    double secondsElapsed = 0; //time until new block spawns
    int counter = 0; //total frames elapsed

    Timer watch;
    char playerInput = '0';
    int (*cb)(char &input) = &readInputCallback;
    
    while(playing){ // main loop
        //synth stuff
        /*
        //trigger impulse and shift voice to new pitch
        for(int i = 0; i < MAP_X; i++){
            double freq = (double)i * 100.0;
            activeVoices[i].oscillator.envelope.initVca((double)hero.getX() * 10.0, (double)hero.getY() * 10.0);
            activeVoices[i].oscillator.changePitch(scale[i]);
            if(!playing) break;
        }
        */
        //game stuff
        if(counter == 0){
            watch.startTimer();
        }
        else if(watch.elapsedTime() > (1.0 / (bpm/60.0))){//MAIN PROCESS
            watch.startTimer();
            prinicipalFn(cb, playerInput);
            flushinp(); //flush input buffer of excess characters
            
            if((rand()%2 != 1) || playerInput == ' '){
                //init random variable between 0 and MAP_X
                int random = rand()%MAP_X;
                //Spawn new enemy and trigger that lanes voice
                Player newEnemy(random, 0, 1, 1, '3');
                double a = (double)hero.getX() * 100.0 + 10.0;
                double r = (double)hero.getY() * 100.0 + 10.0;
                activeVoices[random].oscillator.pitch = scale[random];
                activeVoices[random].oscillator.phaseIncrement = calcPhaseIncrement(scale[random]);
                activeVoices[random].oscillator.envelope.initVca(a, r);
                enemyList.push_back(newEnemy);
                frames = 0;
            }
            if(playerInput != 'z') { //check collisions
                if(playerInput == 'j'){
                    //update scale factor
                    updateScale(scale, changeScale());
                    updateSynthVoice(activeVoices, scale);
                }
                if(playerInput == 'f') flushEnemies(enemyList);
                if(playerInput == 'b') bpm = changeBPM();
                if(playerInput == 'r') resetAllVoices(activeVoices, scale);
                if(playerInput == 'o') DRONE_ON = !DRONE_ON; 
                hero.move(playerInput);
                gameMap.checkBounds(hero);
                if(gameMap.getMap()[hero.getY()][hero.getX()] != '.'){ //if overlapping object, reset hero to previous position
                    hero.setXY(gameObjects[0].getX(), gameObjects[0].getY());
                }
                for(int i = 0; i < enemyList.size(); i++){
                    if(enemyList[i].getY() < MAP_Y){
                         enemyList[i].move('s');
                         activeVoices[i].oscillator.pitch *= pow(2.0, -1.0/12.0);
                    }
                    //pitch voice down a step each time enemy falls further
                    if(DRONE_ON) activeVoices[enemyList[i].getX()].oscillator.changePitch(scale[(int)enemyList[i].getY()]);
                    else if (!DRONE_ON) activeVoices[enemyList[i].getX()].oscillator.phaseIncrement = calcPhaseIncrement(activeVoices[enemyList[i].getX()].oscillator.pitch);
                    gameMap.checkBounds(enemyList[i]);
                    if(gameMap.getMap()[enemyList[i].getY()][enemyList[i].getX()] == '3'){
                        if((i + 1) < gameObjects.size())
                            enemyList[i] = gameObjects[i + 1]; //+1 to account for fact hero always takes the zero position
                    }
                }
            }
            else playing = false;

            //reset gameObjects array with new positions of all hero and enemy sprites
            gameObjects.clear();
            gameObjects.push_back(hero);
            for(int i = 0; i < enemyList.size(); i++) gameObjects.push_back(enemyList[i]);
            gameMap.addSprites(gameObjects);
            gameMap.printMap(1, 2, 3);
            
            frames++;
            refresh();
        }
        counter++;
    }
    
    //close audio streams of all synth voices
    for(int i = 0; i < MAP_X; i++){
        activeVoices[i].endStreaming();
    }
    //Terminate all open Portaudio resources (one call per stream opened)
    for(int i = 0; i < MAP_X; i++){
        Pa_Terminate();
    }
    printw("Hope all works well!");
    getch();
    endwin(); //closes curses
    return 0;
}

#include "synthVoice.h"
using namespace std;


int main(){
    cout << "begin" << endl;
    vector<SynthVoice> activeVoices;
    
    bool playing = true;
    int counter = 0;
    
    //initialize synth voices
    int numVoices = 4;
    bool firstVoice = true;
    for(int i = 0; i < numVoices; i++){
        SynthVoice x(firstVoice, 2, 880, 0, calcPhaseIncrement(880));
        activeVoices.push_back(x);
        firstVoice = false;
    }

    firstVoice = true;
    //open audio stream for each voice
    for(int i = 0; i < numVoices; i++){
        activeVoices[i].beginStreaming(firstVoice, SAMPLE_RATE, FRAMES_PER_BUFFER, asciiTetrisCallback);
        firstVoice = false;
    }


    double secondsElapsed = 0;

    //Main loop
    while(playing){
        counter++;
        //cout << synth.oscillator.amplitude << "sin(" << synth.oscillator.pitch << "*x)" << endl;
        char userInput;
        cout << "press wasd to play note, or z to quit" << endl;
        cin >> userInput;
        
        //trigger impulse and shift voice to new pitch
        for(int i = 0; i < numVoices; i++){
            activeVoices[i].oscillator.envelope.initVca(5.0, 30.0);
            if(!calcFreq(activeVoices[i].oscillator, userInput)) {
                cout << "fuckin frequencies" << endl;
                playing = false;
            }
       
            activeVoices[i].oscillator.phaseIncrement = calcPhaseIncrement(activeVoices[i].oscillator.pitch/(double)i);
            if(!playing) break;
        }
    }
    //close audio streams of all synth voices
    for(int i = 0; i < numVoices; i++){
        activeVoices[i].endStreaming();
    }
    Pa_Terminate();
    cout << "All done, hopefully it works!" << endl;
    return 0;

}
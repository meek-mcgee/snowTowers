#include "synthVoice.h"

double calcPhaseIncrement(double freq)
{
    freq = (freq/(double)SAMPLE_RATE); 
    return freq;
}


/*create vca struct that takes attack and release time and creates 
*functional envelope trigger for an oscillator
*/
void vca::initVca(double a, double r){
    attack = a;
    release = r;
    isTriggered = false;
}
double vca::triggerEnvelope(){
    /*
    * Could probably get true millisecond response time by dividing 
    * clock returns by CLOCKS_PER_SEC 
    */
    
    double scalar;

    if(!isTriggered) {
        /*
        * If all of these are not reset upon retriggering
        * then the values from the previous cycle are used
        * causing the attack values to end up dramatically
        * higher than the release (as it is the only value
        * being updated in the first loop of a retriggered sequence)
        */
        startTime = clock();
        newTime = startTime;
        timeInSeconds = 0.0;
        oldTimeInSeconds = 0.0;
        isTriggered = true;
    }
    else{
        newTime = clock(); //converting new time to milliseconds
        timeInSeconds = (double)(newTime - startTime);
    };

    if(attack >= 1.0) {
        scalar = 1/pow(attack, 2);
        attack -= (timeInSeconds - oldTimeInSeconds)/1000;//accounting for milliseconds
    }
    else if(release >= 1.0) {
        scalar = 1 - 1/release;
        release -= (timeInSeconds - oldTimeInSeconds)/1000;//accounting for milliseconds
    }
    else scalar = 0;

    //cout << "Time in milliseconds: " << timeInSeconds/1000 << endl;
    //cout << "attack: " << attack << endl << "release: " << release << endl << "scalar: " << scalar << endl;
    oldTimeInSeconds = timeInSeconds;
    return scalar;

}

void wavetable::initWavetable(double amp, double freq, double phaseShift){
    phase = phaseShift;
    pitch = freq;
    amplitude = amp;
    for (double i = 0; i < TABLE_SIZE; i++){
        //A * sin((2*pi*i) + Phase)
        //freq in Hz = 1/Time. If Sin takes 2pi per second, then Freq = 2pi/B = 1/pitch in Hz. So 2pi * pitch = frequency
        sine[(int)i] = sin((2.0 * 3.14159 * (i/(double)TABLE_SIZE)));
        //std::cout << sine[(int)i] << std::endl;
    }
}
void wavetable::changePitch(double freq){ 
    phaseIncrement = calcPhaseIncrement(freq);
}
SynthVoice::SynthVoice(bool onlyVoice, double amp, double freq, double phaseShift, double phaseInc){
    if(onlyVoice){
        err = Pa_Initialize();
        if(err != paNoError) std::cout << "Couldn't init";
        else std::cout << "initialzied" << std::endl;
    }
    //Initialize system default ouput device
    out.device = Pa_GetDefaultOutputDevice();
    //Initialize output paramaters to defaults
    out.channelCount = 1;
    out.hostApiSpecificStreamInfo = NULL;
    out.sampleFormat = paFloat32;
    out.suggestedLatency = Pa_GetDeviceInfo(out.device)->defaultLowOutputLatency;
    //Initialize oscillator
    oscillator.initWavetable(amp, freq, phaseShift);
    oscillator.sine[TABLE_SIZE] = oscillator.sine[0];
    oscillator.phaseIncrement = phaseInc;
}
SynthVoice::SynthVoice(bool onlyVoice){
    if(onlyVoice){
        err = Pa_Initialize();
        if(err != paNoError) std::cout << "Couldn't init";
        else std::cout << "initialzied" << std::endl;
    }
    //Initialize system default ouput device
    out.device = Pa_GetDefaultOutputDevice();
    //Initialize output paramaters to defaults
    out.channelCount = 1;
    out.hostApiSpecificStreamInfo = NULL;
    out.sampleFormat = paFloat32;
    out.suggestedLatency = Pa_GetDeviceInfo(out.device)->defaultLowOutputLatency;
    //Initialize oscillator
    oscillator.phase = 0;
}
PaError SynthVoice::getError(){ return err; }
PaError SynthVoice::setError(PaError e){ 
    err = e;
    return err;
}
//opens audio stream for synth voice
bool SynthVoice::beginStreaming(bool streaming, int sampleRate, int framesPerBuffer, PaStreamCallback c){
    //std::cout << "good" << std::endl;
    if(!streaming){ //if main stream hasn't been oppened
        err = Pa_OpenStream(&s, NULL, &out, (double)sampleRate, (long)framesPerBuffer, paClipOff, c, &oscillator);
        if(err != paNoError){
            //std::cout << "Couldn't open stream" << std::endl;
            return false;
        } 
        //std::cout << "stream opened" << std::endl;
    }
    //std::cout << "good" << std::endl;
    err = Pa_StartStream(s);
    //if(err != paNoError) //std::cout << "FUCK" << std::endl;
    return true;
    
}
//closes audio stream for synth voice
bool SynthVoice::endStreaming(){
    err = Pa_StopStream(s);
    if(err != paNoError) {
        //std::cout << "couldn't stop stream" << std::endl;
        return false;
    }
    else{
        err = Pa_CloseStream(s);
        return true;
    }
}
/* Convert phase between and 1.0 to sine value
 * using linear interpolation.
 */
float LookupSine( wavetable *data, float phase )
{
    float fIndex = phase*TABLE_SIZE;
    int   index = (int) fIndex;
    float fract = fIndex - index;
    float lo = data->sine[index];
    float hi = data->sine[index+1];
    float val = lo + fract*(hi-lo);
    return val;
}

bool calcFreq(wavetable &osc, char input){
    switch(input){ //adjust scalar and pitch based on user input
            case 'w':
                osc.pitch *= pow(2, 1.0/12.0);//to move up and down 12 tone scale, multiply by 2 to the 1/12 power
                break;
            case 's':
                osc.pitch *= pow(2, -1.0/12.0);
                break;
            case 'a':
                osc.amplitude++;
                break;
            case 'd':
                if(osc.amplitude > 1) osc.amplitude--;
                else osc.amplitude -= 0.1;
                break;
            case 'z':
                return false;
                break;
            default:
                break;
        }

        return true;
}
/*TO-DO LIST
* Make a general function to initialize the portaudio streams and callbacks
* Continue to flesh out oscillator class until it is a fully functional synth voice 
* constructor
* Flesh out VCA until it allows for proper, smooth envelope curves with attack and release
* Afterwards add VCA sustain and decay options to get full proper ADSR
* ----implementing sustain will require the ability to manipulate midi input/keyboard input and see whether
*     some key/signal is held/on or released/off. Perhaps have a key that triggers a bool off whenever a 
*     a note is triggered. i.e. when note is trigged bool is auto set true and note is sustained. when bool key
*     is pressed, then the sustain is set to off and the release cycle is triggered. 
* ----decay could be implemented by simpling diminishing a sustain value (similarly to attack and release) but only
*     once the sustain trig is set to off.
* Begin research into how to make some sort of LPF or LPG
*/

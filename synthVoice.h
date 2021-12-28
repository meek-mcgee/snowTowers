#ifndef SYNTHVOICE_H
#define SYNTHVOICE_H

#include <iostream>
#include <math.h>
#include <vector>
#include "portaudio.h"

static const int SAMPLE_RATE = 44100;
static const int TABLE_SIZE = 400;
static const int FRAMES_PER_BUFFER = 64;

double calcPhaseIncrement(double freq);


/*create vca struct that takes attack and release time and creates 
*functional envelope trigger for an oscillator
*/
struct vca
{
    bool isTriggered;
    double attack;
    double release;
    clock_t startTime;
    clock_t newTime;
    double timeInSeconds;
    double oldTimeInSeconds;
    void initVca(double a, double r);
    double triggerEnvelope();
};

struct wavetable
{
    double sine[TABLE_SIZE + 1];
    double phase;
    double pitch;
    double amplitude;
    double phaseIncrement;
    vca envelope;
    void initWavetable(double amp, double freq, double phaseShift);
    void changePitch(double freq);
};

class SynthVoice {
    private:
        PaStream *s;
        PaStreamParameters out;
        PaError err;
    public:
        wavetable oscillator;
        SynthVoice(bool onlyVoice, double amp, double freq, double phaseShift, double phaseInc);
        SynthVoice(bool onlyVoice);
        PaError getError();
        PaError setError(PaError e);
        //opens audio stream for synth voice
        bool beginStreaming(bool streaming, int sampleRate, int framesPerBuffer, PaStreamCallback c);
        //closes audio stream for synth voice
        bool endStreaming();

        
};
/* Convert phase between and 1.0 to sine value
 * using linear interpolation.
 */
float LookupSine( wavetable *data, float phase );

bool calcFreq(wavetable &osc, char input);
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

#endif /* SYNTHVOICE.H */
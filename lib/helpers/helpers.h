#ifndef H_A
#define H_A

#include <Arduino.h> //needed for Serial.println
#include <string.h> //needed for memcpy

typedef struct
{
    int pl;
    int note;
    bool deleted;
    long measureStart;
    long highestVal;
    int currentVal;
    int nrSamples;
    int positionHighest;
    int diff;
} note;

bool shouldStartWatching(int pl, int note, int currentVal);
bool shouldPlayDeadStroke(note *noteToEvaluate);
void playDeadStroke(note *noteToEvaluate);
bool isAfterTouch(note *noteToEvaluate);
bool shouldPlay(note *noteToEvaluate);
void play(note *noteToEvaluate);
int readInput(int pl, int note);
int calcMidiNote(int pl, int note);
#endif // H_A

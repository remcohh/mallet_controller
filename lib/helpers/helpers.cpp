#include <Arduino.h>
#include <EEPROM.h>
#include <settings.h>

extern int currentVal[16][2];
extern int offSet[16][2];
extern int lastVal[166][2];
extern int nrConstVals[16][2];
extern int highestVal[16][2];
extern long blocked[16][2];
extern long measureStart[16][2];
extern int watching[16][2];
extern int currentVal[16][2];
extern const int midiNumbers[16][2];
extern int currentSettingVal[5];
extern int maxNotes[32];
extern int currentSetting;
extern int sensitivities[32];
int correctedVelocity;
extern int lastHitNote;
extern bool afterTouching[16][2];

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

int calcMidiNote(int pl, int note)
{
    int noteNumber = pl + note * 16;
    return(29 + 12 * currentSettingVal[0] + noteNumber);
}

bool shouldStartWatching(int pl, int note, int currentVal){
    int diff = currentVal - offSet[pl][note];
    return(diff > 20 && blocked[pl][note] == 0 && !afterTouching[pl][note]);
}

bool shouldPlayDeadStroke(note *noteToEvaluate){
    return(false);
}

void playDeadStroke(note *noteToEvaluate){
}

bool isAfterTouch(note *noteToEvaluate){
    if(noteToEvaluate->currentVal > 20 && !afterTouching[noteToEvaluate->pl][noteToEvaluate->note]){
        Serial.println("---> could be aftertouch!");
        return(true);
    }
    else{
        return(false);
    }
}

bool shouldPlay(note *noteToEvaluate){
    watching[noteToEvaluate->pl][noteToEvaluate->note] = false;
    // Serial.println("blocking val");
    // Serial.println(blocked[noteToEvaluate->pl][noteToEvaluate->note]);
    if(noteToEvaluate->highestVal > 20 && blocked[noteToEvaluate->pl][noteToEvaluate->note] == 0)
    {
        Serial.print("returning true, blocked millis: ");
        Serial.println(blocked[noteToEvaluate->pl][noteToEvaluate->note]);
        return(true);
    }
    else
    {
       Serial.println("returning false");
       return(false);
    }
}

void play(note *noteToEvaluate){
    int midiVal;
    int midiNote;
    int noteNumber = noteToEvaluate->pl + noteToEvaluate->note * 16;
    noteToEvaluate->deleted = true;
    int velocity = noteToEvaluate->highestVal;

    int maxHigh = maxNotes[noteNumber] * 10;
    if(velocity > maxHigh){
        maxHigh = velocity;
    }

    float factor = (float)maxHigh/(float)velocity;
    float pw = ((float)sensitivities[noteNumber]-10)/10;
    correctedVelocity = velocity *  pow(factor, pw);

    midiVal = map(correctedVelocity, 0, maxHigh, 1, 127);
    //midiNote =  29 + 12 * currentSettingVal[0] + noteNumber;
    midiNote = calcMidiNote(noteToEvaluate->pl, noteToEvaluate->note);
    usbMIDI.sendNoteOn(midiNote , midiVal, currentSettingVal[1]);
    if(currentSettingVal[2] == 1) {
        usbMIDI.sendNoteOff(midiNote , midiVal, currentSettingVal[1]);
    }
    lastHitNote = noteNumber;
    currentSettingVal[4] = sensitivities[noteNumber];

    Serial.println("playing..................................................... ");
    Serial.print("Notenumber: ");
    Serial.println(noteNumber);
    Serial.print("velocity: ");
    Serial.println(velocity);
    Serial.print("maxhigh: ");
    Serial.println(maxHigh);
    Serial.print("factor: ");
    Serial.println(factor);
    Serial.print("CorrectedVelod ");
    Serial.println(correctedVelocity);
    if(currentSetting == 3)
    {
        Serial.println("----------------adjust highestvalue");
        int hv =  noteToEvaluate->highestVal/10;
        maxNotes[noteNumber] = hv;
        currentSettingVal[3] = hv;
        updateDisplay();
        //EEPROM.write(noteNumber, noteToEvaluate->highestVal/10);
    } 
    // Serial.print("lastVal: ");
    // Serial.print(noteToEvaluate->diff);
    // Serial.print("midinote: ");
    // Serial.print(midiNote);
    // Serial.print(" nrsamples: ");
    // Serial.print(noteToEvaluate->nrSamples);
    // Serial.print(" positionHighest: ");
    // Serial.print(noteToEvaluate->positionHighest);
    // Serial.print("value: ");
    // Serial.println(midiVal);
    //usbMIDI.sendNoteOff(60, midiVal, 1);
}

void sendControlMessage(int message){
    //usbMIDI.sendAfterTouch(10, 1);
    usbMIDI.sendControlChange(64, 1, 1);
}

void setPowerLine(int pl){
}

int readInput(int pl, int note) 

{

    //if( note <=0 && pl <=3 ){
        //Serial.println(pl);
        return(analogRead(note));
    //}
    // else{
    //     int dummy = analogRead(note);
    //     return 0;
    // }

}





template <class logType>
void log(char label, logType var)
{
    Serial.print(millis());
    Serial.print(": ");
    Serial.print(label);
    Serial.print(" = ");
    Serial.print(var);
}

#include <Arduino.h>
#include <EEPROM.h>
#include <settings.h>

extern int currentVal[16][3];
extern int offSet[16][3];
extern int lastVal[166][3];
extern int nrConstVals[16][3];
extern int highestVal[16][3];
extern long blocked[16][3];
extern long measureStart[16][3];
extern int watching[16][3];
extern int currentVal[16][3];
extern const int midiNumbers[16][3];
extern int currentSettingVal[5];
extern int maxNotes[32];
extern int currentSetting;
extern int sensitivities[32];
int correctedVelocity;
extern int lastHitNote;
extern bool afterTouching[16][3];

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
    return (29 + 12 * currentSettingVal[0] + noteNumber);
}

bool shouldStartWatching(int pl, int note, int currentVal)
{
    int diff = currentVal - offSet[pl][note];
    return (diff > 50 && blocked[pl][note] == 0 && !afterTouching[pl][note]);
}

bool shouldPlayDeadStroke(note *noteToEvaluate)
{
    return (false);
}

void playDeadStroke(note *noteToEvaluate)
{
}

bool isAfterTouch(note *noteToEvaluate)
{
    if (noteToEvaluate->diff > 50 && !afterTouching[noteToEvaluate->pl][noteToEvaluate->note])
    {
        Serial.println("---> could be aftertouch!");
        return (true);
    }
    else
    {
        return (false);
    }
}

bool shouldPlay(note *noteToEvaluate)
{
    // Stop watching it
    watching[noteToEvaluate->pl][noteToEvaluate->note] = false;
    if (noteToEvaluate->highestVal > 20 && blocked[noteToEvaluate->pl][noteToEvaluate->note] == 0)
    {
        Serial.print("returning true, blocked millis: ");
        Serial.println(blocked[noteToEvaluate->pl][noteToEvaluate->note]);
        return (true);
    }
    else
    {
        Serial.println("returning false");
        return (false);
    }
}

void play(note *noteToEvaluate)
{
    int midiVal;
    int midiNote;
    int noteNumber = noteToEvaluate->pl + noteToEvaluate->note * 16;
    noteToEvaluate->deleted = true;
    int velocity = noteToEvaluate->highestVal;

    int maxHigh = maxNotes[noteNumber] * 10;
    if (velocity > maxHigh)
    {
        maxHigh = velocity;
    }

    float factor = (float)maxHigh / (float)velocity;
    float pw = ((float)sensitivities[noteNumber] - 10) / 10;
    correctedVelocity = velocity * pow(factor, pw);
    midiVal = map(correctedVelocity, 0, maxHigh, 1, 127);
    midiNote = calcMidiNote(noteToEvaluate->pl, noteToEvaluate->note);
    if (currentSettingVal[2] == 2)
    {
        usbMIDI.sendControlChange(123, 0, currentSettingVal[1] );
    }
    usbMIDI.sendNoteOn(midiNote, midiVal, currentSettingVal[1]);
    if (currentSettingVal[2] == 1)
    {
        usbMIDI.sendNoteOff(midiNote, midiVal, currentSettingVal[1]);
    }
    lastHitNote = noteNumber;
    currentSettingVal[4] = sensitivities[noteNumber];

    Serial.println("Playing note");
    Serial.print("Notenumber: ");
    Serial.println(noteNumber);
    Serial.print(" Velocity: ");
    Serial.println(velocity);
    Serial.print(" Maximum velocity: ");
    Serial.println(maxHigh);
    Serial.print(" Corrected velocity: ");
    Serial.println(correctedVelocity);
    Serial.print(" Number of samples: ");
    Serial.println(noteToEvaluate->nrSamples);

    // If the current setting is 3, we are setting the max value for the notes by hitting them hard
    if (currentSetting == 3)
    {
        int hv = noteToEvaluate->highestVal / 10;
        if(hv >= 100) { hv = 99; } //we cannot display higher
        maxNotes[noteNumber] = hv;
        currentSettingVal[3] = hv;
        updateDisplay();
    }
}

int readInput(int pl, int note)
{
    if(pl > 0 && note == 2){
        return(0);
    }
    else
    {
        return (analogRead(note));
    }
}

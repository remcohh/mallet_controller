#include <Arduino.h>
#include <helpers.h>
#include <constants.h>
#include <settings.h>
#include <TM1637Display.h>
#include <EEPROM.h>

int offSet[16][2];
int lastVal[16][2];
int nrConstVals[16][2];
int highestVal[16][2];
long blocked[16][2];
int watching[16][2];
int currentVal[16][2];
int maxPowerLine;
int maxNote;
int nrEvaluatingNotes;
note activeNotes[4];
int sampleTime;
int blockTime;
int switch1Pin;
int switch2Pin;
int settings[2][10];
int currentSetting;
int currentSettingVal[5];
int maxNotes[32];
int sensitivities[32];
int lastHitNote;
bool afterTouching[16][2];

byte controlPins[] = {
    B00000000, B00000001, B00000010, B00000011, B00000100, B00000101, B00000110, B00000111,
    B00001000, B00001001, B00001010, B00001011, B00001100, B00001101, B00001110, B00001111};

#define CLK 11
#define DIO 12
TM1637Display display(CLK, DIO);

void setup()
{
    Serial.begin(38400);
    maxPowerLine = 15;
    maxNote = 1;
    sampleTime = 5;
    blockTime = 60;
    DDRD = B11111111; // set PORTD (digital 7~0) to outputs
    for (int pw = 0; pw < maxPowerLine; pw++)
    {
        for (int i = 0; i < maxNote; i++)
        {
            currentVal[pw][i] = readInput(pw, i);
            offSet[pw][i] = readInput(pw, i);
            nrConstVals[pw][i] = 0;
            lastVal[pw][i] = offSet[pw][i];
            highestVal[pw][i] = 0;
            watching[pw][i] = 0;
            blocked[pw][i] = 0;
        }
    }

    // filling array with empty notes
    for (int i = 0; i < 4; i++)
    {
        activeNotes[i].pl = 99;
        activeNotes[i].note = 99;
        activeNotes[i].deleted = true;
        activeNotes[i].measureStart = 0;
        activeNotes[i].highestVal = 0;
        activeNotes[i].currentVal = 0;
        activeNotes[i].nrSamples = 0;
        activeNotes[i].positionHighest = 0;
    }
    switch1Pin = 9;
    switch2Pin = 10;
    pinMode(switch1Pin, INPUT_PULLUP);
    pinMode(switch2Pin, INPUT_PULLUP);

    uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
    uint8_t blank[] = {0x00, 0x00, 0x00, 0x00};
    display.setBrightness(0x0f);

    display.showNumberDecEx(101, 0x40, true, 4, 0);
    currentSetting = 0;
    currentSettingVal[0] = 2; //octaaf
    currentSettingVal[1] = 1; //midikanaal
    currentSettingVal[2] = 0; //midi playmode
    currentSettingVal[3] = 0; //set max
    currentSettingVal[4] = 0; //sensitivy
    currentSettingVal[5] = 0; //store values

    for (int address = 0; address <= 31; address++)
    {
        int maxval = EEPROM.read(address);
        // Set a sensible default when not written to eeprom yest
        if (maxval == 255)
        {
            maxval = 99;
        }; 
        Serial.print("Reading maxval: ");
        Serial.println(maxval);
        maxNotes[address] = maxval;
    }
    for (int nr = 0; nr <= 31; nr++)
    {
        int sensitivity = EEPROM.read(nr + 50);
        // Set a sensible default when not written to eeprom yest
        if (sensitivity == 255)
        {
            sensitivity = 10;
        }; 
        Serial.print("Reading sensitivity: ");
        Serial.println(sensitivity);
        sensitivities[nr] = sensitivity;
    }
}

void evaluateActiveNote(note *noteToEvaluate)
{
    if (millis() - noteToEvaluate->measureStart > 20)
    {
        if (shouldPlayDeadStroke(noteToEvaluate))
        {
            playDeadStroke(noteToEvaluate);
        }
        noteToEvaluate->measureStart = 0;
    }
    else
    {
        if (millis() - noteToEvaluate->measureStart > sampleTime)
        {
            if (isAfterTouch(noteToEvaluate))
            {
                // Not sure what synth needs what but this works for pianoteq
                Serial.println("-------------sensing aftertouch");
                usbMIDI.sendNoteOff(calcMidiNote(noteToEvaluate->pl, noteToEvaluate->note), 0, currentSettingVal[1]);
                usbMIDI.sendAfterTouch(0, currentSettingVal[1]);
                noteToEvaluate->deleted = true;
                watching[noteToEvaluate->pl][noteToEvaluate->note] = false;
                afterTouching[noteToEvaluate->pl][noteToEvaluate->note] = true;
            }
            else
            {
                if (shouldPlay(noteToEvaluate))
                {
                    play(noteToEvaluate);
                    blocked[noteToEvaluate->pl][noteToEvaluate->note] = millis();
                }
                noteToEvaluate->deleted = true;
                noteToEvaluate->nrSamples = 0;
            }
        }
        else
        {
            noteToEvaluate->diff = noteToEvaluate->currentVal - offSet[noteToEvaluate->pl][noteToEvaluate->note];
            // Print values for plotter
            Serial.print(noteToEvaluate->currentVal);
            Serial.print(",");
            Serial.println(noteToEvaluate->diff);

            noteToEvaluate->nrSamples = noteToEvaluate->nrSamples + 1;
            if (noteToEvaluate->diff > noteToEvaluate->highestVal)
            {
                noteToEvaluate->highestVal = noteToEvaluate->diff;
                noteToEvaluate->positionHighest = noteToEvaluate->nrSamples;
            }
        }
    }
}

void evaluateActiveNotes(int currentPower)
{
    for (int i = 0; i < 4; i++)
    {
        if (!activeNotes[i].deleted && activeNotes[i].pl == currentPower)
        {
            PORTD = controlPins[activeNotes[i].pl];
            activeNotes[i].currentVal = readInput(activeNotes[i].pl, activeNotes[i].note);
            evaluateActiveNote(&activeNotes[i]);
        }
    }
}

void loop()
{
    handleInterface();
    for (int pl = 0; pl <= maxPowerLine; pl++)
    {
        // Sets the address pins on the multiplexer
        PORTD = controlPins[pl];
        for (int note = 0; note <= maxNote; note++)
        {
            evaluateActiveNotes(pl);
            currentVal[pl][note] = readInput(pl, note);
            if (!watching[pl][note])
            {
                if (shouldStartWatching(pl, note, currentVal[pl][note]))
                {
                    // place note on free space in watch array
                    for (int i = 0; i < 4; i++)
                    {
                        if (activeNotes[i].deleted)
                        {
                            activeNotes[i].deleted = false;
                            activeNotes[i].pl = pl;
                            activeNotes[i].note = note;
                            activeNotes[i].measureStart = millis();
                            activeNotes[i].highestVal = offSet[pl][note] - currentVal[pl][note];
                            watching[pl][note] = true;
                            break;
                        }
                    }
                }
                else
                {
                    if (currentVal[pl][note] < 5)
                    {
                        afterTouching[pl][note] = false;
                    }
                    // if it is not started it could be blocked
                    if (blocked[pl][note] > 0 && millis() - blocked[pl][note] > blockTime)
                    {
                        blocked[pl][note] = 0;
                    }
                    else // normal inactive note
                    {
                    }
                }
            }
            // See if the value is constant, this could mean we need to adjust the offset
            if (currentVal[pl][note] == lastVal[pl][note])
            {
                nrConstVals[pl][note] += 1;

                if (!watching[pl][note] && nrConstVals[pl][note] > 20 && (offSet[pl][note] - currentVal[pl][note] > 10 || offSet[pl][note] - currentVal[pl][note] < -10))
                {
                    nrConstVals[pl][note] = 0;
                    Serial.println("Resetting offset");
                    offSet[pl][note] = currentVal[pl][note];
                }
            }
            lastVal[pl][note] = currentVal[pl][note];
        }
    }
};

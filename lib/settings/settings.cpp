#include <Arduino.h>
#include <TM1637Display.h>
#include <EEPROM.h>

bool button1Pressed;
bool button2Pressed;
extern int currentSetting;
extern int currentSettingVal[5];
extern int switch1Pin;
extern int switch2Pin;
extern TM1637Display display;
int maxSettingVal[6] = {5,15,1,100,20,1};
extern int maxNotes[32];
extern int sensitivities[32];
extern int lastHitNote;

void updateDisplay(){
  Serial.print("updating display ");
  Serial.println(currentSetting);
  Serial.println(currentSettingVal[currentSetting]);

  int nr = (currentSetting + 1) * 100 + currentSettingVal[currentSetting];
  display.showNumberDecEx(nr, 0x40, true, 4, 0);  
}

void handleSettings(){
  switch(currentSetting) {
    case 3:
    {
      Serial.print("** updating max value for note ");
      Serial.print(lastHitNote);
      Serial.print("value: ");
      Serial.println(currentSettingVal[3]);
      maxNotes[lastHitNote] = currentSettingVal[3];
    }
    break;

    case 4:
    {
      Serial.print("** updating sensitivity for note ");
      Serial.print(lastHitNote);
      Serial.print("value: ");
      Serial.println(currentSettingVal[4]);
      sensitivities[lastHitNote] = currentSettingVal[4];
    }
    break;

    case 5:
    {
      if(currentSettingVal[currentSetting] == 1)
      {
        for(int address = 0; address <=31; address ++)
        {
          Serial.print("** writing maxnotes to eeprom ");
          Serial.print("note: ");
          Serial.print(address);
          Serial.print(" value: ");
          Serial.println(maxNotes[address]);
          
          EEPROM.write(address, maxNotes[address]);
        }

        for(int nr = 0; nr <=31; nr ++)
        {
          int address = nr + 50;
          Serial.print("** writing sensitivy to eeprom: ");
          Serial.print("note: ");
          Serial.print(nr);
          Serial.print(" value: ");
          Serial.println(sensitivities[nr]);
          
          EEPROM.write(address, sensitivities[nr]);
        }
        currentSetting = 0;
        currentSettingVal[0] = 1; 
        updateDisplay();

      }
      break;
    } 

    default: // compilation error: jump to default: would enter the scope of 'x'
    break;
}
}




void handleInterface(){
    if(!digitalRead(switch1Pin)){
      //Serial.println("button down");  
      if(!button1Pressed){
          currentSetting = (currentSetting < 5) ? currentSetting + 1 : 0;
          updateDisplay();
      }  
      button1Pressed = true;
    }else{
      button1Pressed = false;
    };

    if(!digitalRead(switch2Pin)){
      //Serial.println("button down");  
      if(!button2Pressed){
          currentSettingVal[currentSetting] = (currentSettingVal[currentSetting] < maxSettingVal[currentSetting]) ? currentSettingVal[currentSetting] + 1 : 0;
          updateDisplay();
          handleSettings();
      }  
      button2Pressed = true;
    }else{
      button2Pressed = false;
    };
};





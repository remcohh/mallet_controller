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
int maxSettingVal[6] = {5, 15, 2, 100, 20, 1};
extern int maxNotes[33];
extern int sensitivities[33];
extern int lastHitNote;

void updateDisplay()
{
  Serial.print("updating display ");
  Serial.println(currentSetting);
  Serial.println(currentSettingVal[currentSetting]);

  int nr = (currentSetting + 1) * 100 + currentSettingVal[currentSetting];
  display.showNumberDecEx(nr, 0x40, true, 4, 0);
}

void handleSettings()
{
  switch (currentSetting)
  {
  case 3:
  {
    // Updating maximum value
    maxNotes[lastHitNote] = currentSettingVal[3];
  }
  break;

  case 4:
  {
    // Updating sensitivity
    sensitivities[lastHitNote] = currentSettingVal[4];
  }
  break;

  case 5:
  {
    // Saving settings to eeprom
    if (currentSettingVal[currentSetting] == 1)
    {
      for (int address = 0; address <= 31; address++)
      {
        EEPROM.write(address, maxNotes[address]);
      }
      for (int nr = 0; nr <= 31; nr++)
      {
        int address = nr + 50;
        EEPROM.write(address, sensitivities[nr]);
      }
      currentSetting = 0;
      currentSettingVal[0] = 1;
      updateDisplay();
    }
    break;
  }

  default:
    break;
  }
}

void handleInterface()
{
  if (!digitalRead(switch1Pin))
  {
    if (!button1Pressed)
    {
      currentSetting = (currentSetting < 5) ? currentSetting + 1 : 0;
      updateDisplay();
    }
    button1Pressed = true;
  }
  else
  {
    button1Pressed = false;
  };

  if (!digitalRead(switch2Pin))
  {
    if (!button2Pressed)
    {
      Serial.print("butoton2 pressed");
      currentSettingVal[currentSetting] = (currentSettingVal[currentSetting] < maxSettingVal[currentSetting]) ? currentSettingVal[currentSetting] + 1 : 0;
      updateDisplay();
      handleSettings();
    }
    button2Pressed = true;
  }
  else
  {
    button2Pressed = false;
  };
};

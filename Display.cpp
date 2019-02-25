#include "Display.h"
#include "LEDBackpack.h"

Display::Display(void) {
  AlphaNum4 a1_4 = AlphaNum4();
}

void Display::init(){
  a1_4.begin(0x71);
  a1_4.setBrightness(15);
 
}

void Display::renderCharArray(char * to_display, int dots) {

  a1_4.clear();
  
  for(byte i=0; i<4; i++){
    a1_4.writeDigitAscii(i, to_display[i], (i==dots));
  }
  
  a1_4.writeDisplay();

}

void  Display::clear(){
  a1_4.clear();
  a1_4.writeDisplay();
}


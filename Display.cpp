#include "Display.h"
#include "LEDBackpack.h"

Display::Display(void) {
  AlphaNum4 a1 = AlphaNum4();
}

void Display::init(){
  a1.begin(0x71);
  a2.begin(0x72);
  a3.begin(0x73);
  a4.begin(0x74);
  
  a1.setBrightness(15);
  a2.setBrightness(15);
  a3.setBrightness(15);
  a4.setBrightness(15);
}

unsigned int Display::size(){
  return 16;
}

void Display::renderCharArray(char * to_display) {

  a1.clear();
  a2.clear();
  a3.clear();
  a4.clear();   
   
  for(byte i=0; i<4; i++){
    a1.writeDigitAscii(i, to_display[i]);
    a2.writeDigitAscii(i+4, to_display[i+4]);
    a3.writeDigitAscii(i+8, to_display[i+8]);
    a4.writeDigitAscii(i+12, to_display[i+12]);
  }
  
  a1.writeDisplay();
  a2.writeDisplay();
  a3.writeDisplay();
  a4.writeDisplay();
  
  Serial.println(to_display);

}

void  Display::clear(){
  a1.clear();
  a2.clear();
  a3.clear();
  a4.clear();   
  a1.writeDisplay();
  a2.writeDisplay();
  a3.writeDisplay();
  a4.writeDisplay();
}


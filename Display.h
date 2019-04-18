#ifndef DISPLAY_H
#define DISPLAY_H


#include "LEDBackpack.h"


class Display {
 public:
  Display(void);
  void init();
  void renderCharArray(char * to_display);
  void renderDelay();
  void clear();
  unsigned int size();
  
 private:
  AlphaNum4 a1;
  AlphaNum4 a2;
  AlphaNum4 a3;
  AlphaNum4 a4;
};

#endif

#pragma once
#include <Arduino.h>
#include "Config.h"

class MultiplexerController {
private:
  bool initialized;
  
public:
  MultiplexerController();
  void begin();
  void selectChannel(int channel);
  void printChannelInfo(int channel);
};
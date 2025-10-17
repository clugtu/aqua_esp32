#include "MultiplexerController.h"

MultiplexerController::MultiplexerController() : initialized(false) {}

void MultiplexerController::begin() {
  if (initialized) return;
  
  // Initialize multiplexer control pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(MUX_EN, OUTPUT);
  
  // Enable multiplexer (active LOW)
  digitalWrite(MUX_EN, LOW);
  
  Serial.println("Multiplexer Controller Initialized");
  Serial.println("  Testing channel selection...");
  
  for (int i = 0; i < 8; i++) {
    selectChannel(i);
    delay(50);
  }
  
  initialized = true;
  Serial.println("  Multiplexer ready");
}

void MultiplexerController::selectChannel(int channel) {
  // Set control pins for multiplexer channel selection
  bool s0 = channel & 0x01;
  bool s1 = (channel >> 1) & 0x01;
  bool s2 = (channel >> 2) & 0x01;
  bool s3 = (channel >> 3) & 0x01;
  
  digitalWrite(MUX_S0, s0);
  digitalWrite(MUX_S1, s1);
  digitalWrite(MUX_S2, s2);
  digitalWrite(MUX_S3, s3);
  
  // Small delay for switching stability
  delayMicroseconds(10);
}

void MultiplexerController::printChannelInfo(int channel) {
  bool s0 = channel & 0x01;
  bool s1 = (channel >> 1) & 0x01;
  bool s2 = (channel >> 2) & 0x01;
  bool s3 = (channel >> 3) & 0x01;
  
  Serial.printf("  [MUX] Channel %d -> S3=%d S2=%d S1=%d S0=%d\n", 
                channel, s3, s2, s1, s0);
}
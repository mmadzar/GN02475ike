#ifndef DIGIPOT_H_
#define DIGIPOT_H_

#define ADDRESS_WIPER_0 B00000000
#define ADDRESS_WIPER_1 B00010000
#define COMMAND_WRITE B00000000

#include <Arduino.h>
#include <SPI.h>
#include "appconfig.h"
#include "status.h"

class DigiPot
{
private:
  Settings settings;
  void digitalPotWrite(int address, byte value);
  int lastdigipot1 = -1;
  int lastdigipot2 = -1;
  double lastLitre1 = -1;
  double lastLitre2 = -1;
  int lastOhmPerLitre1 = 10;
  int lastOhmPerLitre2 = 10;
  int minDiffPos[2][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}; // minimum difference values for group 1 and 2
  bool existsRes(int value, int currentPos);
  void calibratePot(); // internal calibrate logic

public:
  int resVal[358][7]; // R1 wiper position, R2 wp, Rt, estimated litres 1, estimated litres 2 (litres * 10), ike reported litres for 1 and for 2
  DigiPot();
  void init();
  void setup();
  void handle();
  void seedResistorValues();
  void writePot(gpio_num_t CS, uint8_t channel, uint8_t value);
  void calibrate(int potNo); // start calibration process
};

#endif

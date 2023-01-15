// MCP4251 SPI digital potentiometer

#include "DigiPot.h"

DigiPot::DigiPot()
{
}

void DigiPot::setup()
{
  Serial.println("setup digipot mqtt");

  // setup initial minimum difference values against target litres value
  for (size_t i = 0; i < 2; i++)
  {
    minDiffPos[i][0] = 0;
    minDiffPos[i][1] = 0;
    minDiffPos[i][2] = 0;
    minDiffPos[i][3] = 0;
    minDiffPos[i][4] = 0;
  }
  seedResistorValues();
  pinMode(settings.digipot_cs1, OUTPUT);
  pinMode(settings.digipot_cs2, OUTPUT);
  digitalWrite(settings.digipot_cs1, HIGH); // de-select chip
  digitalWrite(settings.digipot_cs2, HIGH); // de-select chip
  SPI.begin(settings.digipot_sck, settings.digipot_sdi, settings.digipot_sdo);
}

void DigiPot::seedResistorValues()
{
  Serial.println("Generate seed...");
  double r1min = 118; // digipot1 wiper resistance (5k)
  double r2min = 118; // digipot2 wiper resistance (5k)
  double r1step = 18;
  double r2step = 18;
  double rcalc = 0;
  double r1calc, r2calc;
  int counter = 0;
  for (int r2 = 0; r2 < 256; r2++)
  {
    for (int r1 = 0; r1 < 256; r1++)
    {
      r1calc = (r1 * r1step) + r1min;
      r2calc = (r2 * r2step) + r2min;
      rcalc = 1 / ((1 / r1calc) + (1 / r2calc) + (1.00 / 560.00));
      rcalc += 3; // measured error in Ohms
      if (rcalc > 53 && rcalc < 421 && !existsRes(rcalc, counter) && counter < 358)
      {
        resVal[counter][0] = r1;
        resVal[counter][1] = r2;
        resVal[counter][2] = rcalc;
        resVal[counter][3] = (rcalc - ((r1min * r2min) / (r1min + r2min))) / status.ohm_per_liter1 * 10;
        resVal[counter][4] = (rcalc - ((r1min * r2min) / (r1min + r2min))) / status.ohm_per_liter2 * 10;
        resVal[counter][5] = -1;
        // Serial.print(counter);
        // Serial.print("\t");
        // Serial.print(resVal[counter][0]);
        // Serial.print("\t");
        // Serial.print(resVal[counter][1]);
        // Serial.print("\t");
        // Serial.print(resVal[counter][2]);
        // Serial.print("\t");
        // Serial.print(resVal[counter][3] / 10.00);
        // Serial.print("\t");
        // Serial.println(resVal[counter][4] / 10.00);
        counter++;
      }
    }
  }
  Serial.println("Generate seed done.");
}

bool DigiPot::existsRes(int value, int currentPos)
{
  for (size_t i = 0; i < currentPos; i++)
  {
    if (resVal[i][2] == value)
      return true;
  }
  return false;
}

void DigiPot::handle()
{
  // regenerate seed if Ohms per Liter changed
  if (lastOhmPerLitre1 != status.ohm_per_liter1)
  {
    lastOhmPerLitre1 = status.ohm_per_liter1;
    lastLitre1 = 0; // update digipot value
    seedResistorValues();
  }
  else if (lastOhmPerLitre2 != status.ohm_per_liter2)
  {
    lastOhmPerLitre2 = status.ohm_per_liter2;
    lastLitre2 = 0; // update digipot value
    seedResistorValues();
  }

  // Update digipots to reflect digipot values if changed
  if (lastdigipot1 != status.digipot1)
  {
    lastdigipot1 = status.digipot1;

    // Serial.print("digipot1change ");
    // Serial.print(lastdigipot1);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][0]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][1]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][2]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][3] / 10.00);
    // Serial.print("\t");
    // Serial.println(minDiffPos[0][4] / 10.00);

    // status.litres1 = minDiffPos[0][3];
    writePot(settings.digipot_cs1, 0, lastdigipot1); // write 5k value
  }
  else if (lastdigipot2 != status.digipot2)
  {
    lastdigipot2 = status.digipot2;

    // Serial.print("digipot2change ");
    // Serial.print(lastdigipot2);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][0]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][1]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][2]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][3] / 10.00);
    // Serial.print("\t");
    // Serial.println(minDiffPos[1][4] / 10.00);

    // status.litres2 = minDiffPos[1][4];
    writePot(settings.digipot_cs1, 1, lastdigipot2); // write 5k value
  }
  // Update digipots to reflect litres if changed
  else if (lastLitre1 != status.litres1)
  {
    lastLitre1 = status.litres1;
    for (int i = 0; i < (358); i++)
    {
      if (abs((status.litres1 * 10) - resVal[i][3]) < abs((status.litres1 * 10) - minDiffPos[0][3]))
      {
        minDiffPos[0][0] = resVal[i][0];
        minDiffPos[0][1] = resVal[i][1];
        minDiffPos[0][2] = resVal[i][2];
        minDiffPos[0][3] = resVal[i][3];
        minDiffPos[0][4] = resVal[i][4];
      }
    }

    // Serial.print("litres1change ");
    // Serial.print(status.litres1);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][0]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][1]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][2]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[0][3] / 10.00);
    // Serial.print("\t");
    // Serial.println(minDiffPos[0][4] / 10.00);

    // status.digipot1 = minDiffPos[0][2];
    writePot(settings.digipot_cs1, 0, minDiffPos[0][0]);
    writePot(settings.digipot_cs1, 1, minDiffPos[0][1]);
  }
  else if (lastLitre2 != status.litres2)
  {
    lastLitre2 = status.litres2;
    for (int i = 0; i < (358); i++)
    {
      if (abs((status.litres2 * 10) - resVal[i][4]) < abs((status.litres2 * 10) - minDiffPos[1][4]))
      {
        minDiffPos[1][0] = resVal[i][0];
        minDiffPos[1][1] = resVal[i][1];
        minDiffPos[1][2] = resVal[i][2];
        minDiffPos[1][3] = resVal[i][3];
        minDiffPos[1][4] = resVal[i][4];
      }
    }

    // Serial.print("litres2change ");
    // Serial.print(status.litres2);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][0]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][1]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][2]);
    // Serial.print("\t");
    // Serial.print(minDiffPos[1][3] / 10.00);
    // Serial.print("\t");
    // Serial.println(minDiffPos[1][4] / 10.00);

    // status.digipot2 = minDiffPos[1][2];

    writePot(settings.digipot_cs2, 0, minDiffPos[1][0]);
    writePot(settings.digipot_cs2, 1, minDiffPos[1][1]);
  }
}

void DigiPot::writePot(gpio_num_t CS, uint8_t channel, uint8_t value)
{
  byte cmdByte = B00000000;
  byte dataByte = B00000000;
  if (value > 255)
    cmdByte |= B00000001;
  else
    dataByte = (byte)(value & 0X00FF);

  if (channel == 0)
    cmdByte = cmdByte | ADDRESS_WIPER_0 | COMMAND_WRITE;
  else
    cmdByte = cmdByte | ADDRESS_WIPER_1 | COMMAND_WRITE;
  const uint8_t buf[2] = {cmdByte, dataByte};
  uint8_t res[2] = {0b0, 0b0};

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS, LOW); // select chip
  SPI.transferBytes(buf, res, 2);
  // SPI.transfer(cmdByte);
  // SPI.transfer(dataByte);
  digitalWrite(CS, HIGH); // deselect chip
  SPI.endTransaction();
  // Serial.print("Received: ");
  // Serial.print(res[0], 2);
  // Serial.println(res[1], 2);
}

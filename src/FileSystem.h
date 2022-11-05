#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "appconfig.h"

class FileSystem
{
public:
  FileSystem();
  void save();
  void load();
  void setup();
  String getValue(String data, char separator, int index);
  String* loadMock();

private:
  String formatData();
};

#endif /* FILESYSTEM_H_ */

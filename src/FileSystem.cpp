#include "FileSystem.h"

String filePath = "/appconfig.csv";

FileSystem::FileSystem()
{
}

String *FileSystem::loadMock()
{
  // 229
  //                    chk?
  //msg DD A5 00 1E ... F5 C0 77
  //uint8_t res[229][31];

  // File file = SPIFFS.open("/log_cell.csv", "r");
  // String fileContent = file.readString();
  // Split the string into substrings
  // while (fileContent.length() > 0)
  // {
  //   int index = fileContent.indexOf(';');
  //   if (index == -1) // No space found
  //   {
  //     strs[StringCount++] = str;
  //     break;
  //   }
  //   else
  //   {
  //     strs[StringCount++] = str.substring(0, index);
  //     str = str.substring(index+1);
  //   }
  // }

  //file.close();
  //SPIFFS.end();

}

void FileSystem::setup()
{
}

void FileSystem::save()
{
  if (SPIFFS.begin())
  {
    size_t usedBefore = SPIFFS.usedBytes();
    String fc = formatData();
    char buff[fc.length() + 1];
    fc.toCharArray(buff, fc.length() + 1);
    File file = SPIFFS.open(filePath, "w");
    file.write((uint8_t *)buff, fc.length() + 1);
    file.close();
    size_t used = SPIFFS.usedBytes();
    size_t total = SPIFFS.totalBytes();
    SPIFFS.end();

    Serial.println((String) "Saving... Used before: " + usedBefore + "  after: " + used + "/" + total);
  }
}

void FileSystem::load()
{
  if (SPIFFS.begin()) // on first sketch do SPIFFS.begin(true) to format flash.
  {
    if (!SPIFFS.exists(filePath))
    {
      SPIFFS.end();
      save();
      SPIFFS.begin();
    }
    // else //uncomment to save changed settings on first upload
    // {
    //   save();
    // }

    File file = SPIFFS.open(filePath, "r");
    String fileContent = file.readString();
    file.close();
    size_t used = SPIFFS.usedBytes();
    size_t total = SPIFFS.totalBytes();
    SPIFFS.end();
    // alarmsSettings.lowTempThreshold = getValue(fileContent, ';', 0).toInt();
    // alarmsSettings.highTempThreshold = getValue(fileContent, ';', 1).toInt();
    // alarmsSettings.alarmsInterval = getValue(fileContent, ';', 2).toInt();
    // alarmsSettings.alarmDuration = getValue(fileContent, ';', 3).toInt();
    // alarmsSettings.alarmCooldown = getValue(fileContent, ';', 4).toInt();
    Serial.println((String) "*** Loaded settings: " + formatData() + "  used: " + used + "/" + total);
  }
}

String FileSystem::formatData()
{
  return ""; /*((String)alarmsSettings.lowTempThreshold + ";" +
          alarmsSettings.highTempThreshold + ";" +
          alarmsSettings.alarmsInterval + ";" +
          alarmsSettings.alarmDuration + ";" +
          alarmsSettings.alarmCooldown);*/
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String FileSystem::getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

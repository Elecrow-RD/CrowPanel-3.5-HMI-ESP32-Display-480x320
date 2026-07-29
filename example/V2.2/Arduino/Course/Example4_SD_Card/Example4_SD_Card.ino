#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

/*---------------------------------------------------------------
 * SD card SPI configuration
 * These pins connect the ESP32 SPI controller to the onboard card slot.
 *--------------------------------------------------------------*/
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define SD_CS 5

/**
 * @brief Start the serial monitor, SPI bus, and SD card demonstration.
 *
 * Arduino calls this function once after power-up or reset. A return value of
 * one from SD_init() means the card could not be mounted or identified.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(9600);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  delay(100);

  if (SD_init() == 1) {
    Serial.println("Card Mount Failed");
  } else {
    Serial.println("initialize SD Card successfully");
  }
}

/**
 * @brief Leave the card contents unchanged after the startup test.
 *
 * Arduino calls this function repeatedly after setup(). All card inspection is
 * intentionally performed once during startup.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}

/**
 * @brief Mount the SD card, report its capacity, and list stored files.
 *
 * setup() calls this function once after the SPI bus has stabilized. Only read
 * operations are active; the commented examples show optional file operations.
 *
 * @param None.
 * @return 0 when the card is ready, or 1 when mounting or detection fails.
 */
int SD_init() {
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return 1;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No TF card attached");
    return 1;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("TF Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 2);

  //  listDir(SD, "/", 0);
  //  createDir(SD, "/mydir");
  //  listDir(SD, "/", 0);
  //  removeDir(SD, "/mydir");
  //  listDir(SD, "/", 2);
  //  writeFile(SD, "/hello.txt", "Hello ");
  //  appendFile(SD, "/hello.txt", "World!\n");
  //  readFile(SD, "/hello.txt");
  //  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  //  Serial.println("SD init over.");

  return 0;
}

/**
 * @brief Recursively list files below a directory.
 *
 * SD_init() calls this function after a successful mount. Directory recursion
 * stops when levels reaches zero, which prevents an unrestricted tree walk.
 *
 * @param fs Mounted filesystem that owns the directory.
 * @param dirname Absolute directory path to inspect.
 * @param levels Maximum number of nested directory levels to visit.
 * @return Nothing.
 */
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  //  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    // Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
//  i = 0;
  while (file) {
    if (file.isDirectory()) {
      // Serial.print("  DIR : ");
      // Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("FILE: ");
      Serial.print(file.name());
//    lcd.setCursor(0, 2 * i);
//    lcd.printf("FILE:%s", file.name());

      Serial.print("SIZE: ");
      Serial.println(file.size());
//    lcd.setCursor(180, 2 * i);
//    lcd.printf("SIZE:%d", file.size());
//    i += 16;
    }

    file = root.openNextFile();
  }
}

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LSM6DS3TRC.h>

//convert image to byte array using this link: https://javl.github.io/image2cpp/
// array should be 240x240, and select 565 RBG (2 bytes per pixel)
// create .h files and paste the row below (replace array with whatever you generated in step above, and title monkey and horse appropriately [2 separate .h files])
//    const uint16_t monkey[57600] PROGMEM = { 0x1234, 0x5678, /* ... */ };

#include "monkey.h"   // const uint16_t monkey[57600] PROGMEM
#include "horse.h"    // const uint16_t horse[57600]  PROGMEM

// Tilt thresholds in m/s^2 (gap between them prevents flicker)
const float TILT_ON  = 3.5;
const float TILT_OFF = 2.0;

// CLUE display pins (fixed on the board)
#define CLUE_TFT_CS        31
#define CLUE_TFT_DC        32
#define CLUE_TFT_RST       33
#define CLUE_TFT_BACKLIGHT 34

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, CLUE_TFT_CS, CLUE_TFT_DC, CLUE_TFT_RST);
Adafruit_LSM6DS3TRC lsm6ds3trc;   // newer CLUE boards
Adafruit_LSM6DS33 lsm6ds33;       // older CLUE boards
Adafruit_LSM6DS *imu = nullptr;

bool tilted = false;

void showImage(const uint16_t *img) {
  tft.drawRGBBitmap(0, 0, img, 240, 240);
}

void setup() {
  pinMode(CLUE_TFT_BACKLIGHT, OUTPUT);
  digitalWrite(CLUE_TFT_BACKLIGHT, HIGH);

  tft.init(240, 240);
  tft.setRotation(1);

  if (lsm6ds3trc.begin_I2C()) {
    imu = &lsm6ds3trc;
  } else if (lsm6ds33.begin_I2C()) {
    imu = &lsm6ds33;
  } else {
    tft.fillScreen(ST77XX_RED);
    while (1) delay(10);   // no accelerometer found
  }

  showImage(monkey);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu->getEvent(&accel, &gyro, &temp);

  float amount = max(fabs(accel.acceleration.x), fabs(accel.acceleration.y));

  if (!tilted && amount > TILT_ON) {
    tilted = true;
    showImage(horse);
  } else if (tilted && amount < TILT_OFF) {
    tilted = false;
    showImage(monkey);
  }

  delay(20);
}

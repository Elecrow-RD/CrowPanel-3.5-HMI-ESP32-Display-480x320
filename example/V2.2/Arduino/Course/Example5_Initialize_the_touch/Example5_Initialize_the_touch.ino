#include <TFT_eSPI.h>

/*---------------------------------------------------------------
 * Touchscreen state and calibration
 * The stored calibration maps raw controller readings to screen pixels.
 *--------------------------------------------------------------*/
TFT_eSPI lcd = TFT_eSPI();
uint16_t touchX, touchY;
uint16_t calData[5] = {557, 3263, 369, 3493, 3};

/**
 * @brief Initialize the LCD and load the saved touch calibration.
 *
 * Arduino calls this function once after power-up or reset. The interactive
 * calibration call remains disabled because valid values are already stored.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.setRotation(1);

  /*-------------------------------------------------------------
   * Select one calibration method
   * Enable touch_calibrate() only when new panel values are needed.
   *------------------------------------------------------------*/
//  touch_calibrate();
  lcd.setTouch(calData);
}

/**
 * @brief Read touches and print their calibrated pixel coordinates.
 *
 * Arduino calls this function repeatedly after setup(). A pressure threshold
 * of 600 filters light contact and electrical noise.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  bool touched = lcd.getTouch(&touchX, &touchY, 600);
  if (touched) {
    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

/**
 * @brief Run the TFT_eSPI corner calibration and print reusable values.
 *
 * setup() may call this function instead of lcd.setTouch() when the panel is
 * first commissioned or its orientation changes. Follow the on-screen targets,
 * then copy the printed array into the global calData definition.
 *
 * @param None.
 * @return Nothing.
 */
void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;
  Serial.println("Touch-screen calibration");
  Serial.println("Please touch the corners as directed");

//  lv_timer_handler();
  lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  Serial.println("calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)");
  Serial.println();
  Serial.println();
  Serial.println("//Use this calibration code in setup():");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println();
  Serial.println();
}

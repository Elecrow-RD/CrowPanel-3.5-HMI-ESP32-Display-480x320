#include <U8g2lib.h>
#include <Wire.h>

/*---------------------------------------------------------------
 * OLED bus and display configuration
 * The software I2C instance uses the board's SDA and SCL pins.
 *--------------------------------------------------------------*/
#define I2C_SDA 22
#define I2C_SCL 21

// Provides drawing and page-buffer control for the 128 x 64 SSD1306 OLED.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/I2C_SCL, /*data=*/I2C_SDA, /*reset=*/U8X8_PIN_NONE);

/**
 * @brief Initialize the OLED and scroll the ELECROW text across the screen.
 *
 * Arduino calls this function once after power-up or reset. U8g2 redraws the
 * full page for each horizontal position so the text appears to move left.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setFontDirection(0);

  /*-------------------------------------------------------------
   * Render the scrolling title
   * Move the starting x-coordinate left by 20 pixels per frame.
   *------------------------------------------------------------*/
  for (int i = 128; i > -78; i -= 20) {
    u8g2.firstPage();
    do {
      u8g2.drawStr(i, 25, "ELECROW");
      delay(2);
    } while (u8g2.nextPage());
  }
}

/**
 * @brief Leave the final OLED frame unchanged.
 *
 * Arduino calls this function repeatedly after setup(). The animation is a
 * one-shot startup demonstration, so no recurring work is required.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <Crowbits_DHT20.h>
#include "ui.h"

/*---------------------------------------------------------------
 * Application state
 * SquareLine button callbacks update led, and loop() applies it to GPIO25.
 *--------------------------------------------------------------*/
int led;

/*---------------------------------------------------------------
 * Display geometry and partial render buffer
 * One eighth of a frame limits RAM use while LVGL refreshes in sections.
 *--------------------------------------------------------------*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;
static lv_color_t buf1[screenWidth * screenHeight / 8];

/*---------------------------------------------------------------
 * Hardware drivers and calibration
 * TFT_eSPI shares the LCD and touch controller; DHT20 uses I2C.
 *--------------------------------------------------------------*/
TFT_eSPI lcd = TFT_eSPI();
Crowbits_DHT20 dht20;
uint16_t calData[5] = {353, 3568, 269, 3491, 7};

// Stores the most recent calibrated touch position in screen pixels.
uint16_t touchX, touchY;

/**
 * @brief Transfer an LVGL render area to the LCD.
 *
 * LVGL calls this function whenever part of the display buffer is ready. The
 * ready notification is essential because LVGL must not reuse the buffer while
 * TFT_eSPI is still sending its pixels.
 *
 * @param disp LVGL display that requested the transfer.
 * @param area Inclusive screen rectangle to update.
 * @param px_map RGB565 pixel data for the requested rectangle.
 * @return Nothing.
 */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushColors((uint16_t *)px_map, w * h, true);
  lcd.endWrite();

  lv_display_flush_ready(disp);
}

/**
 * @brief Provide the current touchscreen state and position to LVGL.
 *
 * LVGL calls this function while processing input devices. A pressure threshold
 * of 600 rejects light contact; valid coordinates are also printed for testing.
 *
 * @param indev LVGL input device requesting a sample.
 * @param data Output structure that receives press state and coordinates.
 * @return Nothing.
 */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  bool touched = lcd.getTouch(&touchX, &touchY, 600);
  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

/**
 * @brief Initialize the sensor, display, touch input, LVGL, and generated UI.
 *
 * Arduino calls this function once after power-up or reset. The sequence makes
 * the hardware drivers available before LVGL registers its display and input
 * callbacks, then creates the SquareLine Studio objects last.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);

  /*-------------------------------------------------------------
   * Prepare the controlled output and DHT20 sensor
   * GPIO25 follows the UI button state; I2C uses SDA22 and SCL21.
   *------------------------------------------------------------*/
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  Wire.begin(22, 21);
  dht20.begin();

  /*-------------------------------------------------------------
   * Start LVGL and the physical display
   * millis() supplies the scheduler tick required by LVGL timers.
   *------------------------------------------------------------*/
  lv_init();
  lv_tick_set_cb(millis);

  lcd.begin();
  lcd.fillScreen(TFT_BLACK);
  delay(300);
  lcd.setTouch(calData);
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  lcd.setRotation(1);

  /*-------------------------------------------------------------
   * Connect LVGL rendering and touch input to TFT_eSPI
   * The generated UI is created only after both drivers are registered.
   *------------------------------------------------------------*/
  lv_display_t *disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();
}

/**
 * @brief Refresh sensor labels, apply the UI output state, and service LVGL.
 *
 * Arduino calls this function repeatedly after setup(). Each pass reads the
 * DHT20, updates both labels, drives GPIO25 from the latest button event, and
 * gives LVGL time to process rendering and touch events.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  Serial.print(led);

  char DHT_buffer[6];
  int a = (int)dht20.getTemperature();
  int b = (int)dht20.getHumidity();
  snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
  lv_label_set_text(ui_Label1, DHT_buffer);
  snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
  lv_label_set_text(ui_Label2, DHT_buffer);

  if (led == 1) {
    digitalWrite(25, HIGH);
  }

  if (led == 0) {
    digitalWrite(25, LOW);
  }

  lv_timer_handler();
  delay(10);
}

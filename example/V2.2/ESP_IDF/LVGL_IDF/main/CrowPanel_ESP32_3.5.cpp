#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <DHT20.h>
#include "ui.h"

/*---------------------------------------------------------------
 * Display and scheduler configuration
 * The panel uses landscape 480 x 320 coordinates and a 5 ms task period.
 *--------------------------------------------------------------*/
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320
#define LVGL_TICK_PERIOD 5

/*---------------------------------------------------------------
 * Touchscreen calibration
 * These values map XPT2046 raw samples to the landscape display coordinates.
 *--------------------------------------------------------------*/
uint16_t calData[5] = {353, 3568, 269, 3491, 7};

/*---------------------------------------------------------------
 * Hardware drivers and shared application state
 * UI callbacks update led; loop() applies that state to GPIO25.
 *--------------------------------------------------------------*/
TFT_eSPI lcd = TFT_eSPI();
DHT20 dht20;
int led = 0;

/*---------------------------------------------------------------
 * LVGL partial rendering resources
 * One eighth of a frame limits RAM use while LVGL refreshes in sections.
 *--------------------------------------------------------------*/
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 8)
lv_color_t draw_buf[DRAW_BUF_SIZE];

// Identifies the LVGL display connected to TFT_eSPI.
static lv_display_t *disp;
// Identifies the LVGL pointer device connected to the touch controller.
static lv_indev_t *indev;

// References the generated UI label that shows temperature in degrees Celsius.
lv_obj_t *temp_label;
// References the generated UI label that shows relative humidity as a percentage.
lv_obj_t *humi_label;

// Stores the most recent calibrated touch position in screen pixels.
uint16_t touchX, touchY;

/**
 * @brief Read the DHT20 and update the UI labels and serial diagnostics.
 *
 * loop() calls this function approximately every two seconds. Integer values
 * match the label design and provide a stable, compact teaching display.
 *
 * @param None.
 * @return Nothing.
 */
void update_sensor_values() {
  int temperature = dht20.getTemperature();
  int humidity = dht20.getHumidity();

  lv_label_set_text_fmt(temp_label, "%d", temperature);
  lv_label_set_text_fmt(humi_label, "%d", humidity);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

/**
 * @brief Provide the current touchscreen state and position to LVGL.
 *
 * LVGL calls this function while processing its registered input device. The
 * transition check prints one message per press or release rather than flooding
 * the serial monitor while a finger remains on the panel.
 *
 * @param indev LVGL input device requesting a sample.
 * @param data Output structure that receives press state and coordinates.
 * @return Nothing.
 */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  (void)indev;

#ifdef TOUCH_CS
  static bool was_pressed = false;
  bool pressed = false;

  if (lcd.getTouch(&touchX, &touchY)) {
    pressed = true;
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  if (pressed && !was_pressed) {
    Serial.printf("Touch DOWN: %u, %u\n", touchX, touchY);
  } else if (!pressed && was_pressed) {
    Serial.println("Touch UP");
  }
  was_pressed = pressed;
#else
  // Keep LVGL released when the build excludes the touch controller.
  data->state = LV_INDEV_STATE_RELEASED;
#endif
}

/**
 * @brief Transfer an LVGL render area to the LCD.
 *
 * LVGL calls this function whenever a partial render buffer is ready. The ready
 * notification allows LVGL to reuse the buffer after TFT_eSPI accepts its data.
 *
 * @param disp LVGL display that requested the transfer.
 * @param area Inclusive screen rectangle to update.
 * @param px_map RGB565 pixel data for the requested rectangle.
 * @return Nothing.
 */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  lcd.pushImage(area->x1, area->y1, w, h, (uint16_t *)px_map);
  lv_display_flush_ready(disp);
}

/**
 * @brief Initialize the Arduino-compatible drivers and generated LVGL UI.
 *
 * app_main() calls this function once after initArduino(). Hardware drivers are
 * prepared before LVGL registers its callbacks, and UI objects are created last
 * so their labels can be referenced safely.
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
  digitalWrite(25, LOW);

  Wire.setPins(22, 21);
  Wire.begin();
  dht20.begin();

  /*-------------------------------------------------------------
   * Start the LCD, touch controller, and backlight
   * Byte swapping matches the RGB565 ordering expected by the panel.
   *------------------------------------------------------------*/
  lcd.begin();
  lcd.setRotation(1);
  lcd.setSwapBytes(true);
  lcd.fillScreen(TFT_BLACK);
  delay(300);

  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
#ifdef TOUCH_CS
  lcd.setTouch(calData);
#endif

  /*-------------------------------------------------------------
   * Register LVGL display and input drivers
   * millis() provides the tick source used by LVGL timers.
   *------------------------------------------------------------*/
  lv_init();
  lv_tick_set_cb(millis);

  disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();

  temp_label = ui_Label1;
  lv_label_set_text(temp_label, "--");

  humi_label = ui_Label2;
  lv_label_set_text(humi_label, "--");
}

/**
 * @brief Service periodic sensor, output, and LVGL work.
 *
 * app_main() calls this function from its FreeRTOS task. Sensor reads are limited
 * to one every two seconds while UI rendering and output control run every pass.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  static unsigned long last_update = 0;

  if (millis() - last_update > 2000) {
    update_sensor_values();
    last_update = millis();
  }

  digitalWrite(25, led ? HIGH : LOW);
  lv_timer_handler();
}

/**
 * @brief Enter the Arduino-style application from ESP-IDF.
 *
 * ESP-IDF calls this function once after system startup. It initializes the
 * Arduino component, performs one-time setup, then runs the application loop in
 * the current FreeRTOS task with a 5 ms delay to share processor time.
 *
 * @param None.
 * @return Nothing.
 */
extern "C" void app_main() {
  initArduino();
  setup();

  while (true) {
    loop();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

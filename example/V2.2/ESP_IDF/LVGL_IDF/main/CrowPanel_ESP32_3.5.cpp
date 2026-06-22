#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <DHT20.h>
#include "ui.h"  // UI creation function

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320
#define LVGL_TICK_PERIOD 5


uint16_t calData[5] = {353, 3568, 269, 3491, 7};


TFT_eSPI lcd = TFT_eSPI(); 
DHT20 dht20;
int led = 0;

// Screen buffer (recommended to be 1/10 to 1/4 of screen size)
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 8)
lv_color_t draw_buf[DRAW_BUF_SIZE];

// LVGL display and input device
static lv_disp_draw_buf_t disp_draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;
static lv_disp_t *disp;
static lv_indev_t *indev;

// Real-time temperature and humidity label objects
lv_obj_t *temp_label;
lv_obj_t *humi_label;

// Sensor update
void update_sensor_values() {
  int temperature = dht20.getTemperature();
  int humidity = dht20.getHumidity();

  // If you have corresponding labels in ui.h, you can uncomment the following to update the UI:
  lv_label_set_text_fmt(temp_label, "%d", temperature);
  lv_label_set_text_fmt(humi_label, "%d", humidity);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

// Touch reading
uint16_t touchX, touchY;
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  (void)indev_drv;

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
  data->state = LV_INDEV_STATE_RELEASED;
#endif
}

// Refresh function
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  lcd.pushImage(area->x1, area->y1, w, h, (uint16_t *)color_p);
  lv_disp_flush_ready(disp_drv);
}



void setup() {
  Serial.begin(115200);

    // IO pin setup
    pinMode(25, OUTPUT);
    digitalWrite(25, LOW);

  // Initialize I2C
  Wire.setPins(22, 21);
  Wire.begin();
  dht20.begin();

  // Initialize LCD
  lcd.begin();
  lcd.setRotation(1);        // Adjust screen orientation if needed
  lcd.setSwapBytes(true);    // Swap RGB and BGR data
  lcd.fillScreen(TFT_BLACK);
  delay(300);

  // Backlight control
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
#ifdef TOUCH_CS
  lcd.setTouch(calData);
#endif

  // Initialize LVGL
  lv_init();

  // Create display driver
  lv_disp_draw_buf_init(&disp_draw_buf, draw_buf, NULL, DRAW_BUF_SIZE);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_WIDTH;
  disp_drv.ver_res = SCREEN_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &disp_draw_buf;
  disp = lv_disp_drv_register(&disp_drv);

  // Register touch input device
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  indev = lv_indev_drv_register(&indev_drv);

  // Initialize UI
  ui_init();

  temp_label = ui_Label1;
  lv_label_set_text(temp_label, "--");

  humi_label = ui_Label2;
  lv_label_set_text(humi_label, "--");
}

void loop() {
  static unsigned long last_update = 0;

  if (millis() - last_update > 2000) {
    update_sensor_values();
    last_update = millis();
  }

  digitalWrite(25, led ? HIGH : LOW);
  lv_tick_inc(LVGL_TICK_PERIOD);
  lv_timer_handler();  // Handle LVGL tasks
}

// Entry point for ESP-IDF
extern "C" void app_main() {
  initArduino();
  setup();
  while (true) {
    loop();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

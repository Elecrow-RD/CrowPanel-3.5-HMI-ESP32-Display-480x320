#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <DHT20.h>

/*---------------------------------------------------------------
 * Generated user interface
 * ui_init() creates the labels and button callbacks used below.
 *--------------------------------------------------------------*/
#include "ui.h"

// Stores the output state selected by the generated ON/OFF button callbacks.
int led;

/*---------------------------------------------------------------
 * Display geometry and rendering buffer
 * The buffer holds eight rows so LVGL can refresh the panel in small blocks.
 *--------------------------------------------------------------*/
static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;

static lv_color_t buf1[screenWidth * 8];

// Drives the ILI9488 LCD and its XPT2046 touch controller.
TFT_eSPI lcd = TFT_eSPI();
// Reads the DHT20 sensor on the board I2C bus.
DHT20 dht20;
// Maps raw touch readings to the 480 x 320 landscape coordinates.
uint16_t calData[5] = { 353, 3568, 269, 3491, 7  };

/**
 * @brief Return the millisecond time source required by LVGL.
 *
 * LVGL calls this callback whenever it advances timers and input processing.
 * PlatformIO's Arduino framework supplies the monotonic millis() counter.
 *
 * @param None.
 * @return Elapsed milliseconds since the board started.
 */
static uint32_t lv_tick_get_ms()
{
    return millis();
}

/**
 * @brief Transfer one rendered LVGL area to the LCD.
 *
 * LVGL calls this function after rendering a partial buffer. The final ready
 * notification allows LVGL to reuse the buffer for its next render operation.
 *
 * @param disp LVGL display that requested the transfer.
 * @param area Inclusive rectangle to refresh.
 * @param px_map RGB565 pixel data for the rectangle.
 * @return Nothing.
 */
void my_disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.pushColors(reinterpret_cast<uint16_t *>(px_map), w * h, true);
    lcd.endWrite();

    lv_display_flush_ready(disp);
}

// Stores the latest calibrated touch point reported by TFT_eSPI.
uint16_t touchX, touchY;
/**
 * @brief Read the calibrated touch state and pass it to LVGL.
 *
 * LVGL calls this function while polling the pointer input device. A pressure
 * threshold of 600 filters light contact; valid coordinates are printed so the
 * calibration can be checked before diagnosing UI events.
 *
 * @param indev LVGL input device requesting a sample.
 * @param data Output state and coordinate structure.
 * @return Nothing.
 */
void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    (void)indev;
    bool touched = lcd.getTouch(&touchX, &touchY, 600);

    if(!touched) {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    else {
        data->state = LV_INDEV_STATE_PRESSED;

        // Pass calibrated pixel coordinates to the active LVGL input device.
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print("Data x ");
        Serial.println(touchX);

        Serial.print("Data y ");
        Serial.println(touchY);
    }
}

/**
 * @brief Initialize serial ports, I2C, LCD, touch, LVGL, and the generated UI.
 *
 * PlatformIO calls this function once after the Arduino framework starts. The
 * physical drivers are ready before LVGL registers display and input callbacks;
 * ui_init() is called last so its objects can be used by loop().
 *
 * @param None.
 * @return Nothing.
 */
void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600);

    /*-------------------------------------------------------------
     * Prepare the controlled output and DHT20 I2C sensor
     * GPIO25 follows the UI button state; SDA22 and SCL21 feed the sensor.
     *------------------------------------------------------------*/
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);

    Wire.begin(22, 21);
    dht20.begin();

    lv_init();
    lv_tick_set_cb(lv_tick_get_ms);

    lcd.begin();
    lcd.fillScreen(TFT_BLACK);
    delay(300);
    lcd.setTouch(calData);
    // GPIO27 controls the LCD backlight on this board.
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
    lcd.setRotation(1);

    lv_display_t * disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

    ui_init();
}

/**
 * @brief Update sensor labels, apply button state, and service LVGL.
 *
 * PlatformIO calls this function repeatedly after setup(). Each pass reads the
 * DHT20, updates both generated labels, drives GPIO25 from led, and gives LVGL
 * time to render and process touch events.
 *
 * @param None.
 * @return Nothing.
 */
void loop()
{
    Serial.print(led);
    char DHT_buffer[12];
    int a = (int)dht20.getTemperature();
    int b = (int)dht20.getHumidity();
    snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
    lv_label_set_text(ui_Label1, DHT_buffer);
    snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
    lv_label_set_text(ui_Label2, DHT_buffer);

    if(led == 1) {
        digitalWrite(25, HIGH);
    }
    if(led == 0) {
        digitalWrite(25, LOW);
    }

    lv_timer_handler();
    delay(10);
}

idf_component_register(
    SRCS "CrowPanel_ESP32_3.5.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino-esp32 TFT_eSPI lvgl-3 ui DHT20
)

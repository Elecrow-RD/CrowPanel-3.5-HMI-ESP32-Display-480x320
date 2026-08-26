# CrowPanel ESP32 3.5-inch LVGL 环境监控面板

这是一个基于 ESP-IDF、Arduino-ESP32、LVGL 和 TFT_eSPI 的 CrowPanel ESP32 3.5 英寸示例项目。界面显示 DHT20 温湿度，并可通过触摸按钮控制 GPIO25。

## 硬件

- Elecrow CrowPanel ESP32 3.5-inch HMI，480×320
- ESP32-WROVER / 8 MB PSRAM
- ILI9488 SPI LCD
- XPT2046 电阻触摸屏
- DHT20 温湿度传感器

本仓库默认配置适用于 CrowPanel 3.5-inch **硬件 v2.2**：

| 功能 | GPIO |
| --- | ---: |
| TFT MISO | 33 |
| TFT MOSI | 13 |
| TFT SCLK | 14 |
| TFT CS | 15 |
| TFT DC | 2 |
| TFT Backlight | 27 |
| Touch CS | 12 |
| I²C SDA | 22 |
| I²C SCL | 21 |
| UI 控制输出 | 25 |

旧版 CrowPanel 的 TFT MISO 和 Touch CS 与 v2.2 对调，请根据 Elecrow 官方资料修改 `sdkconfig.defaults`。

## 开发环境

- ESP-IDF 5.5.4
- `espressif/arduino-esp32` 3.3.8（由 Component Manager 自动下载）
- 4 MB 或更大 Flash

首次构建时 ESP-IDF 会根据 `main/idf_component.yml` 和 `dependencies.lock` 下载托管依赖，因此需要网络连接。

## 构建与烧录

先打开已初始化 ESP-IDF 5.5.4 环境的终端，然后执行：

```bash
idf.py set-target esp32
idf.py build
idf.py -p COM40 flash monitor
```

Linux/macOS 请将 `COM40` 换成实际串口，例如 `/dev/ttyUSB0`。退出串口监视器使用 `Ctrl+]`。

## 目录

- `main/`：应用入口、DHT20 读取、LVGL 显示和触摸逻辑
- `components/ui/`：SquareLine Studio 生成的界面及图片资源
- `components/DHT20/`：DHT20 驱动
- `components/TFT_eSPI/`：适配 ESP-IDF 的 TFT_eSPI
- `components/lvgl-3/`：LVGL 核心源码
- `sdkconfig.defaults`：CrowPanel v2.2 的可复现默认配置

`build/`、`managed_components/` 和本机 IDE 设置不会提交，均可在构建时重新生成。

## 第三方许可证

LVGL 和 TFT_eSPI 的原始许可证保留在各自组件目录中。项目应用代码使用根目录中的 MIT License。

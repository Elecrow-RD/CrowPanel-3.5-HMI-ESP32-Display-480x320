# CrowPanel ESP32 Display 3.5 V2.2 Product Hardware Driver Notes

| Item | Content |
|---|---|
| Document Version | V1.0 |
| Document Date | 2026-07-29 |
| Author | Codex (compiled from cross-referencing project materials) |
| Applicable Hardware | CrowPanel ESP32 Display 3.5, schematic/PCB V2.2 (2024-07-04) |
| Software Baseline | Board-level examples in `Arduino/Course` and accompanying libraries in the repository |
| Determination Principle | Working code takes priority; schematics used for electrical connection review; devices without code evidence are described only for their hardware function, with no speculation on driver parameters |

## 1. Document Purpose and Evidence Boundaries

This document is intended for hardware maintenance, Arduino driver porting, and onboarding of new team members. Cross-validation uses the following primary sources:

1. `V2.2/CrowPanel ESP32 Display-3.5-V2.2-20240704.sch`: Eagle schematic source file, the primary basis for nets and device connections.
2. `V2.2/CrowPanel ESP32 Display-3.5-V2.2-20240704.brd`: PCB layout source file, used to confirm this material set corresponds to V2.2.
3. `V2.2/CrowPanel ESP32 Display-3.5-V2.2-20240704.pdf`: Released schematic.
4. `Arduino/Course/Example1...Example8`, `LVGL_Arduino3.5`: Board-verified examples, serving as the highest-priority evidence for pin assignments and runtime parameters.
5. `Arduino/libraries/TFT_eSPI/User_Setup.h`: The actual compiled configuration for LCD, touch, and backlight.

"Verified" in this document means the repository delivers the code as a usable Course example; this work did not connect physical hardware and re-flash. Third-party libraries' own examples are not considered evidence of on-board devices. OLED, GPS, and DHT20 are peripherals connected through expansion interfaces, not on-board chips in the schematic.

## 2. Software Stack and Overall Architecture

- MCU: ESP32-WROVER-B module (schematic reference designator U24; the device library name still carries `ESP32-WROOM-32-N4`, identified by combining value and wireless/PSRAM requirements).
- Application framework: Arduino-ESP32; the lower layer is provided by the ESP-IDF HAL/drivers for GPIO, SPI, I2C, UART, DAC, Wi-Fi, and BLE.
- GUI: LVGL 9.1.0 → TFT_eSPI 2.5.43 → Arduino-ESP32 SPI/GPIO.
- File system: Arduino SD 1.3.0 → FS/SPI.
- External OLED: U8g2 2.36.19, example uses software I2C.
- Touch: The actual example uses TFT_eSPI's built-in XPT2046 support; the repository also includes XPT2046_Touchscreen 1.4, but the board-level example does not directly use it.

## 3. Peripheral Overview

| Category | Peripheral / Ref Designator | Interface and Main Pins | Code Status | Remarks |
|---|---|---|---|---|
| Main Control | ESP32-WROVER-B / U24 | 3.3 V; GPIO Matrix | In use | Wi-Fi, BLE, DAC, SPI, UART, I2C all provided by it |
| Display | 3.5-inch 480×320 TFT / J1, TFT-18PIN1 | SPI: 14/13/33, CS15, DC2 | Verified | Code selects ILI9488, rotation 1 |
| Touch | XPT2046/XP2046 / U3 | SPI: CLK14, DIN13, DOUT33, CS12; IRQ36 | Verified polling | Schematic net names have mislabeled CS/DOUT |
| Backlight | Q6 + LEDK | GPIO27, active high | Verified, turns on with `lcd.begin()` | N-MOS low-side control, PWM can be added |
| TF/microSD | SD1 | SPI: SCK18, MOSI23, MISO19, CS5 | Verified | Independent of TFT/touch pin group |
| Audio | SC8002B / U2, speaker connector J8 | GPIO26/DAC2 → amplifier | Verified | 8-bit DAC lookup-table sine wave |
| USB-to-Serial | USB-C J3 + CH340G U6 | UART0: TX1, RX3; DTR/RTS auto-download | Hardware-fixed | USB only via CH340G, not native ESP32 USB |
| Buttons | K1 BOOT, K4 RESET | GPIO0→GND; EN→GND | Hardware-fixed | Boot strap/reset, not recommended for use as ordinary pushbuttons |
| Indicator | LED1 | 3.3 V → R3 → LED1 → GND | Hardware-fixed | Power LED; not connected to GPIO25 of Example1 |
| Expansion I2C | J6 | SCL21, SDA22, 3.3 V, GND | OLED/DHT20 verified | Schematic contains series/pull-up network |
| Expansion Analog/Digital | J7 | GPIO25, GPIO32, 3.3 V, GND | GPIO25 example verified | GPIO32 can be ADC1_CH4/Touch9 |
| Expansion UART | J10 | RX3, TX1, 3.3 V, GND | GPS example verified | Conflicts with download/log UART |
| Wireless | ESP32 2.4 GHz Wi-Fi + BLE | Module internal RF | Verified | Does not occupy external GPIO |
| Power Input | USB-C VBUS, Li-ion battery J5 | VBUS/BAT+ → VIN → 3.3 V | Hardware-fixed | Includes power OR-ing/backflow protection |
| Charging | 4054A / U26 | VBUS→BAT+, PROG=2 kΩ | No software driver | Linear single-cell Li-ion charger |
| 3.3 V Regulator | HM3420B / U1 + L4 | VIN→3V3 | No software driver | Switching regulator, EN pulled up by hardware |

## 4. GPIO/Bus Authoritative Mapping

| GPIO | Code Definition | Schematic Electrical Connection | Direction / Alternate Function | Notes |
|---:|---|---|---|---|
| 0 | BOOT | K1, auto-download Q10 | Boot strap input | Pulled low during download; do not add heavy load |
| 1 | UART0 TX; GPS_TX | U24 TXD0, J10-2, CH340 RXD | UART output | GPS example shares with log/download |
| 2 | TFT_DC | LCD D/C | SPI control output | ESP32 boot strap pin, peripherals must not force a wrong level |
| 3 | UART0 RX; GPS_RX | U24 RXD0, J10-1, CH340 TXD | UART input | GPS may drive simultaneously with CH340, bus contention risk |
| 5 | SD_CS | microSD DAT3/CS | SPI chip-select output | Note strap level at boot |
| 12 | TOUCH_CS | XPT2046 CS | SPI chip-select output | Schematic net name mislabeled as TFT_SDO/TP_OUT; also MTDI boot strap pin |
| 13 | TFT_MOSI, touch DIN | LCD SDA, XPT2046 DIN | HSPI MOSI output | TFT and touch shared |
| 14 | TFT_SCLK, touch DCLK | LCD SCL, XPT2046 DCLK | HSPI SCK output | TFT and touch shared |
| 15 | TFT_CS | LCD CS | SPI chip-select output | Boot strap pin |
| 18 | SD_SCK | microSD CLK | VSPI SCK output | SD dedicated bus group |
| 19 | SD_MISO | microSD DAT0 | VSPI MISO input | SD dedicated bus group |
| 21 | I2C_SCL | J6-1, via R26 | I2C clock / open-drain | Example OLED uses software I2C; DHT20 uses hardware Wire |
| 22 | I2C_SDA | J6-2, via R27 | I2C data / open-drain | 3.3 V logic |
| 23 | SD_MOSI | microSD CMD | VSPI MOSI output | SD dedicated bus group |
| 25 | `D_PIN` | J7-1, via R18; DAC1/ADC2_CH8 | Example is push-pull output | Not connected to schematic LED1 |
| 26 | SPEAKER/DAC2 | SC8002B input network | 8-bit DAC analog output | Should not be configured as an ordinary digital output to play audio |
| 27 | TFT_BL | Q6 gate, pull-down via R38 | Active-high output / PWM optional | High level turns on |
| 32 | Expansion A/D | J7-2, via R25 | ADC1_CH4 / digital IO | ADC1 usable alongside Wi-Fi |
| 33 | TFT_MISO, touch DOUT | XPT2046 DOUT; LCD MISO as defined in code | SPI input | Schematic net name mislabeled as TP_CS |
| 34 | Expansion pad | U24 GPIO34 | ADC1_CH6, input only | No internal pull-up/down |
| 35 | Expansion pad | U24 GPIO35 | ADC1_CH7, input only | No internal pull-up/down |
| 36 | TP_IRQ | XPT2046 PENIRQ, pull-up via R10 | Input only / ADC1_CH0 | Code currently polls, no interrupt registered |
| 39 | Expansion pad | U24 GPIO39 | ADC1_CH3, input only | No internal pull-up/down |

## 5. Detailed Driver Description

### 5.1 ESP32 MCU, Wi-Fi and BLE

Wi-Fi and BLE are both internal ESP32 module peripherals, with no external control pins. The Wi-Fi example calls `WiFi.begin()`, enables auto-reconnect, and blocks waiting for `WL_CONNECTED`; the target network must be a 2.4 GHz network supported by ESP32. The SSID/password in the example are demo values; for mass production they should be moved to secure configuration storage, and hardcoding real credentials is prohibited.

The BLE example establishes a GATT Server: device name `ESP32SPI-BLE`, Service UUID `6479571c-2e6d-4b34-abe9-c35116712345`, Characteristic UUID `826f072d-f87c-4ae6-a416-6ffdcaa02d73`, properties READ/WRITE/NOTIFY, initial value `ELECROW`. It depends on the Arduino-ESP32 BLEDevice/BLEServer software layer.

```cpp
WiFi.begin(ssid, password);
WiFi.setAutoReconnect(true);

BLEDevice::init("ESP32SPI-BLE");
BLEService *svc = BLEDevice::createServer()->createService(SERVICE_UUID);
```

### 5.2 TFT LCD (ILI9488)

The actual compiled configuration is in `TFT_eSPI/User_Setup.h`: ILI9488, 320×480 portrait native resolution; examples uniformly use `setRotation(1)` to obtain 480×320 landscape coordinates. The interface is hardware SPI: MOSI=13, MISO=33, SCLK=14, CS=15, DC=2; RESET=-1, meaning TFT RESET is hardware-linked to the board-level EN_RESET and is not independently controlled by a GPIO. Write clock 15,999,999 Hz, read clock 20 MHz.

```cpp
#define ILI9488_DRIVER
#define TFT_MISO 33
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define SPI_FREQUENCY 15999999

TFT_eSPI lcd;
lcd.begin();
lcd.setRotation(1);
```

The initialization order should be: establish serial (optional) → `lcd.begin()` → clear screen → set touch calibration → set rotation/GUI. The current LVGL example calls `setRotation(1)` after `setTouch()`; when porting, the calibration array must match the final rotation and cannot be reused directly across orientations.

ILI9488's SDO is typically unreliably tri-stated; this board's display and touch share SPI lines and must rely on correct chip select, and must not be directly paralleled onto the SD's separate SPI pin group. If touch readings are abnormal, first use an oscilloscope to check whether GPIO33 is driven by the LCD when TFT CS is pulled high.

### 5.3 Resistive Touch and XPT2046

U3 is marked XP2046, and the software uses it as an XPT2046-compatible controller. It shares SCLK=14, MOSI/DIN=13, MISO/DOUT=33 with the TFT, with an independent CS=12; PENIRQ=GPIO36. TFT_eSPI configures the touch SPI at 600 kHz. The current code calls `getTouch(..., 600)` polling; the value 600 is a pressure threshold, not an interrupt trigger parameter; the GPIO36 IRQ is not currently used.

```cpp
#define TOUCH_CS 12
#define SPI_TOUCH_FREQUENCY 600000
uint16_t calData[5] = {557, 3263, 369, 3493, 3};
lcd.setRotation(1);
lcd.setTouch(calData);
bool pressed = lcd.getTouch(&x, &y, 600);
```

For a new screen, a different batch, or after changing rotation, run `lcd.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15)` to save the new five-tuple. If switching to interrupt mode, GPIO36 is input-only with no internal pull-up/down; on-board R10 pulls PENIRQ up to 3.3 V, typically active on the falling edge or low level; before formal implementation it should still be confirmed against the controller datasheet and actual measurements.

### 5.4 Backlight

GPIO27 drives the 2N7002 Q6 via R38; Q6's low-side switch controls LEDK, so the code defines `TFT_BACKLIGHT_ON HIGH`. `lcd.begin()` sets it high. For dimming, Arduino LEDC PWM can be used, but avoid frequencies too low that cause visible flicker; the project does not provide a verified PWM frequency, so it is recommended to validate EMI, brightness, and temperature rise starting from 10–20 kHz.

### 5.5 microSD/TF Card

The SD card uses a separate hardware SPI: SCK18, MISO19, MOSI23, CS5. Initialization first calls `SPI.begin(18,19,23)`, delays 100 ms, then `SD.begin(5)`; the example serial is 9600 baud, and after a successful mount it reads the card type, capacity, and recursively lists directories two levels deep. `SD.begin()` passes no frequency and uses the library/Arduino core default; do not infer an unverified frequency beyond what the documentation shows.

```cpp
SPI.begin(18, 19, 23);
delay(100);
if (!SD.begin(5)) { /* mount failed */ }
```

The card socket operates at 3.3 V, with DAT/CMD having board-level pull-ups. Hot-plugging, write power-loss protection, and maximum capacity/filesystem type are not verified in the project; after porting, a FAT32/capacity matrix and write power-loss tests need to be added.

### 5.6 Audio DAC and SC8002B Amplifier

GPIO26 is ESP32 DAC_CHANNEL_2, fed through an input network to the SC8002B mono amplifier; J8 is the differential speaker output VO1/VO2. The example enables the 8-bit DAC and outputs via a 256-point sine table: sample rate = `frequency × 256`; a 1 kHz tone has a theoretical sample rate of 256 ksample/s, uses point-by-point `delayMicroseconds()`, plays for 1 s then returns to the midpoint 128, and pauses for 2 s.

```cpp
dac_output_enable(DAC_CHANNEL_2);
dac_output_voltage(DAC_CHANNEL_2, sineWave[i & 0xFF]);
```

It depends on ESP-IDF `driver/dac.h`, not Arduino PWM. SC8002B's SHUTDOWN is connected to AGND in the schematic and cannot be turned off by software. J8 is a bridged/differential output; neither end is ground. Do not connect either speaker end to GND, nor directly to a single-ended active speaker input.

### 5.7 I2C Expansion, OLED and DHT20

J6: 1=SCL/GPIO21, 2=SDA/GPIO22, 3=3.3 V, 4=GND. The schematic net names match the code variable order.

The OLED example is an external SSD1306 128×64 with no independent RESET, using the U8g2 software I2C constructor; the clock is not explicitly specified, and the rate is determined by the U8g2 software implementation:

```cpp
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(
    U8G2_R0, /* clock */ 21, /* data */ 22, U8X8_PIN_NONE);
u8g2.begin();
```

The DHT20 in the LVGL example uses hardware Wire: `Wire.begin(22, 21); dht20.begin();`. The Arduino API parameter order is SDA, SCL, so it is still SDA22/SCL21. The DHT20 library works at the device standard address, but the board-level code does not explicitly write the address; the maintenance documentation does not treat the inferred address as a verified parameter. Before paralleling multiple I2C modules, account for the equivalent pull-up resistance and address conflicts; all signals must be 3.3 V only.

### 5.8 GPIO25 Example and Expansion Port J7

Course Example1 configures GPIO25 as a push-pull output, toggling every 500 ms. The schematic shows J7: 1=GPIO25, 2=GPIO32, 3=3.3 V, 4=GND; therefore this example can drive an external LED/load connected to J7-1.

```cpp
pinMode(25, OUTPUT);
digitalWrite(25, HIGH);
delay(500);
digitalWrite(25, LOW);
```

The schematic LED1 is a 3.3 V power LED and cannot be controlled by GPIO25. A GPIO push-pull can only drive small-current loads; external LEDs must have a current-limiting resistor in series, and relays/motors etc. must add transistors/MOSFETs and flyback protection.

### 5.9 UART0, GPS, CH340G and Auto-Download

J10: 1=GPIO3/RX0, 2=GPIO1/TX0, 3=3.3 V, 4=GND. The GPS example maps the UART2 peripheral to the same physical pins, using 9600 baud, 8N1:

```cpp
HardwareSerial gpsSerial(2);
gpsSerial.begin(9600, SERIAL_8N1, /* RX */ 3, /* TX */ 1);
```

It receives NMEA, verifies the XOR checksum, parses GGA/RMC/VTG, and displays them on the TFT. GPIO1/3 are also hard-wired to the CH340G, and `Serial` by default also uses UART0; the example simultaneously does `Serial.begin(115200)` and GPS at 9600, causing different purposes/baud rates on the same pin group. Although Arduino declares `gpsSerial` as UART2, the GPIO Matrix still maps UART2 to GPIO1/3, electrically conflicting with UART0/CH340. When deploying GPS, disconnect/isolate the CH340 path or use other available GPIO, and re-verify; do not connect the GPS module's 5 V UART directly to a 3.3 V RX.

The CH340G's DTR/RTS connect to EN/IO0 via Q9/Q10, implementing Arduino auto-reset and download. K1 manually pulls GPIO0 low, and K4 manually pulls EN low.

### 5.10 Power, Charging and Protection

- USB-C J3 provides VBUS and USB D+/D−; D+/D− connect only to the CH340G.
- U26 (4054A) is a single-cell Li-ion linear charger, VBUS input, BAT to J5 BAT+, PROG resistor R5=2 kΩ. The actual charging current depends on the specific 4054A model formula and device tolerance; the schematic does not provide sufficient evidence, so no definite value is claimed here.
- Q3 (P-MOS) and D5 (1N5817) form the USB/battery power-selection and reverse-isolation path, converging to VIN.
- U1 (HM3420B) and L4 convert VIN to 3.3 V; EN shares the net with VIN, enabled on power-up, with no software control.
- SC8002B additionally uses an audio power domain labeled 3.3V/AGND; the digital 3V3 and 3.3V names differ, and the layout/reflow design must not be arbitrarily shorted together.

Use only a single-cell Li-ion battery, and connect to J5 after confirming polarity. Pay attention to linear charger temperature rise during charging; USB supply capability, screen backlight, amplifier peak, and external module current must be budgeted together. The expansion port 3.3 V is not a 5 V-tolerant supply.

## 6. Schematic—Code Discrepancies and Rulings

| ID | Schematic / Label | Working Code | Ruling and Reason |
|---|---|---|---|
| D01 | Net `IO33_TP_CS` actually connects to U3 DOUT; net `IO12_TFT_SDO/TP_OUT` actually connects to U3 CS | `TFT_MISO=33`, `TOUCH_CS=12` | Use code and pinref electrical connection as the basis: GPIO33=DOUT/MISO, GPIO12=CS. |
| D02 | U3 device name written as `XP2046` | TFT_eSPI drives touch as XPT2046-compatible | Compatible usage; handle per the XPT2046 SPI protocol when porting, and check the BOM/physical silkscreen for the procurement part number separately |
| D03 | LED1 fixed to 3V3, via R3 to ground | Example1 comment says GPIO25 drives onboard indicator | Driver definition still follows code GPIO25, but it only reaches J7-1; the "onboard indicator" comment is inconsistent with the V2.2 schematic, possibly the example carried over from another hardware revision or a comment error |
| D04 | LCD RESET hardware-linked to EN_RESET | `TFT_RST=-1` | Consistent; do not port to template defaults such as GPIO4 |
| D05 | Touch PENIRQ to GPIO36 | Example only uses `getTouch()` polling | Code prevails, interrupt not used currently; GPIO36 reserved for future optimization |
| D06 | J10 connects UART0 GPIO1/3 and CH340 | GPS uses UART2 mapped to GPIO1/3, and simultaneously enables Serial/UART0 | Pin definitions recorded by code, but a real contention risk exists; production design should change pins or isolate, and cannot be treated as concurrently usable |
| D07 | MCU symbol deviceset name contains WROOM-32-N4, value is ESP32-WROVER-B | Examples/GUI resources imply a WROVER-class config with PSRAM | Material identification follows value/physical/BOM; board target and PSRAM option must be confirmed on actual hardware |

## 7. Risks and Notes

1. **Boot strap pins**: GPIO0, 2, 5, 12, 15 are sensitive during reset sampling. External devices must not force a wrong level at startup, especially GPIO12, which may affect flash voltage configuration.
2.
 **UART hardware conflict**: GPIO1/3 are simultaneously connected to CH340, J10, and UART0, while the GPS example also maps to UART2; this is the highest-priority remediation item.

3. **ILI9488 MISO tri-state risk**: The touch DOUT and possibly the LCD SDO share GPIO33. When bus contention occurs, consider disconnecting the LCD SDO or adjusting the hardware, rather than merely masking the issue by lowering the frequency.

4. **Voltage**: ESP32 GPIO, SD, touch, and I2C expansion all operate at 3.3 V; any 5 V signal must be level-shifted.

5. **Input-only GPIO**: Pins 34/35/36/39 cannot output and have no internal pull-up; interrupt inputs must rely on a defined external bias.

6. **ADC/Wi-Fi**: ADC2 channels such as GPIO25/4 may become unavailable while Wi-Fi is active; prefer the ADC1 channels on GPIO32/34/35/36/39.

7. **GPIO drive capability**: Extended pins such as GPIO25 are suitable only for logic/low-current use and must not directly drive speakers, motors, relays, or high-power LEDs.

8. **Amplifier output**: J8 is a BTL differential output; grounding either terminal may damage the amplifier.

9. **Battery safety**: Connect only a matched single-cell Li-ion battery; the schematic shows a charger but provides no proof of cell protection, so prefer cells with a protection board.

10. **Shared bus concurrency**: LVGL refresh and touch reads are managed as transactions by TFT_eSPI; when adding new tasks or concurrent RTOS access, mutual exclusion must be applied to avoid switching the chip select while a DMA/flush operation is still in progress.

11. **Calibration data**: The touch calibration array is tied to screen orientation, panel, and mechanical assembly, and cannot serve as a universal constant across all production batches.

12. **Library configuration location**: TFT pins are defined inside the library's `User_Setup.h`, which is easily overwritten when upgrading or replacing the library; it is recommended to later migrate to a standalone board-level setup file selected via `User_Setup_Select.h`.

## 8. Porting and Power-On Verification Checklist

1. Verify the physical PCB silkscreen revision is V2.2 and record the silkscreen of the ESP32 module and the LCD/touch chips.

2. In Arduino-ESP32, select the board type matching WROVER/PSRAM, then first verify download, reset, and serial log output.

3. With no load, measure VBUS, BAT+, VIN, and 3V3; only connect expansion modules after confirming that 3V3 is stable.

4. Run LCD, backlight, touch, SD, audio, I2C, Wi-Fi, and BLE in sequence; do not combine all tasks from the start.

5. For the LCD, use ILI9488 with SPI write ≈ 16 MHz; verify solid colors, orientation, color order, and long-duration refresh.

6. For touch, use CLK14/DIN13/DOUT33/CS12 at 600 kHz, re-calibrate, and test the four corners, dragging, and long-press.

7. For SD, use 18/19/23/5; cover no-card, hot-plug, different capacities, directories, and power-loss writes.

8. For audio, first connect a speaker of suitable impedance across the two terminals of J8, test at low volume and short duration, and monitor the SC8002B temperature rise.

9. Before I2C scanning, confirm 3.3 V and pull-ups; verify the OLED/DHT20 both individually and in parallel.

10. Do not drive GPIO1/3 concurrently with CH340/UART0 for GPS; if debugging is unavoidable, switch to other serial pins or use hardware isolation.

11. During Wi-Fi/BLE stress runs, re-test ADC, SD, GUI refresh, and power-supply ripple.

12. Backfill the final Arduino core version, compiled board type, Flash/PSRAM settings, library versions, touch calibration values, and measured current into the release record.

## 9. Items Pending Supplement / On-Site Testing

- Per-example test records from re-flashing the physical unit, the Arduino-ESP32 core version, and the compile menu options.

- Confirmation of the LCD panel controller and XPT2046 physical silkscreen / procurement part numbers.

- 4054A actual charge current, termination voltage, thermal testing, and battery protection scheme.

- HM3420B output capability, ripple, USB/battery switching transients, and overall peak power consumption.

- SD card capacity / file-system compatibility matrix.

- Actual accessibility and recommended uses of the GPIO34/35/39 pads or test points.

- Hardware remediation or an official software pin scheme for the GPS UART conflict.

Until the above items are completed, this document may serve as a driver-porting baseline but should not replace the mass-production hardware verification report.

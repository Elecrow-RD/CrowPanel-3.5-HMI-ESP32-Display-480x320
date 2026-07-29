#include <TFT_eSPI.h>
#include <Arduino.h>

/*---------------------------------------------------------------
 * GPS serial connection
 * UART2 receives NMEA data through GPIO3 and transmits through GPIO1.
 *--------------------------------------------------------------*/
#define GPS_RX 3
#define GPS_TX 1
HardwareSerial gpsSerial(2);

// Provides drawing access to the onboard 480 x 320 LCD.
TFT_eSPI lcd = TFT_eSPI();

/*---------------------------------------------------------------
 * Display geometry
 * These dimensions define the coordinate space used by drawScreen().
 *--------------------------------------------------------------*/
#define SCREEN_W 480
#define SCREEN_H 320

// Accumulates one NMEA sentence without dynamic memory allocation.
char nmeaLine[128];
// Points to the next free byte in nmeaLine.
byte nmeaIndex = 0;

/*---------------------------------------------------------------
 * Parsed navigation state
 * GGA, RMC, and VTG sentences update the fields shown on the LCD.
 *--------------------------------------------------------------*/
struct {
  bool valid = false;
  float lat = 0;
  float lon = 0;
  char latDir = 'N';
  char lonDir = 'E';
  float alt = 0;
  float speed = 0;
  uint8_t sats = 0;
  uint8_t fixType = 0; // 0 = no fix, 1 = GPS fix, 2 = differential GPS fix.
  char timeStr[10] = "--:--:--";
  char dateStr[12] = "----/--/--";
} gps;

/**
 * @brief Validate one complete NMEA sentence with its XOR checksum.
 *
 * handleNMEA() calls this function before parsing a received line. Rejecting
 * damaged sentences prevents partial serial data from reaching the display.
 *
 * @param line Null-terminated NMEA sentence beginning with '$'.
 * @return true when the calculated checksum matches the received checksum.
 * @return false when the sentence is incomplete or corrupted.
 */
bool checkNMEA(const char* line) {
  const char* star = strchr(line, '*');
  if (!star || strlen(star) < 3) return false;
  
  byte calc = 0;
  for (const char* p = line + 1; *p && *p != '*'; p++) {
    calc ^= *p;
  }
  
  byte recv = (byte)strtol(star + 1, NULL, 16);
  return calc == recv;
}

/**
 * @brief Convert an NMEA degrees-minutes coordinate to decimal degrees.
 *
 * GGA and RMC parsers call this function after extracting a coordinate. South
 * and west positions become negative so the result follows common map notation.
 *
 * @param dm Coordinate in ddmm.mmmm or dddmm.mmmm format.
 * @param dir Hemisphere letter: N, S, E, or W.
 * @return Signed coordinate in decimal degrees, or 0 for invalid input.
 */
float dmToDd(const char* dm, char dir) {
  if (!dm || strlen(dm) < 3) return 0;
  float val = atof(dm);
  int deg = (int)(val / 100);
  float min = val - deg * 100;
  float dd = deg + min / 60.0;
  return (dir == 'S' || dir == 'W') ? -dd : dd;
}

/**
 * @brief Parse fix quality, satellites, altitude, time, and coordinates from GGA.
 *
 * handleNMEA() calls this function for a checksum-verified GPGGA or GNGGA
 * sentence. strtok() edits the sentence buffer while walking its fields.
 *
 * @param p Writable, null-terminated GGA sentence.
 * @return Nothing.
 */
void parseGGA(char* p) {
  // $GPGGA,hhmmss.ss,lat,N,lon,E,fix,sats,hdop,alt,M,...
  char* tok = strtok(p, ","); // $GPGGA
  tok = strtok(NULL, ","); // time
  if (tok && strlen(tok) >= 6) {
    snprintf(gps.timeStr, sizeof(gps.timeStr), "%c%c:%c%c:%c%c",
             tok[0], tok[1], tok[2], tok[3], tok[4], tok[5]);
  }
  tok = strtok(NULL, ","); // lat
  char* lat = tok;
  tok = strtok(NULL, ","); // N/S
  char latD = tok ? tok[0] : 'N';
  tok = strtok(NULL, ","); // lon
  char* lon = tok;
  tok = strtok(NULL, ","); // E/W
  char lonD = tok ? tok[0] : 'E';
  tok = strtok(NULL, ","); // fix
  gps.fixType = tok ? atoi(tok) : 0;
  gps.valid = (gps.fixType > 0);
  tok = strtok(NULL, ","); // sats
  gps.sats = tok ? atoi(tok) : 0;
  tok = strtok(NULL, ","); // hdop
  tok = strtok(NULL, ","); // alt
  gps.alt = (tok && strlen(tok) > 0) ? atof(tok) : 0;
  
  if (gps.valid) {
    gps.lat = dmToDd(lat, latD);
    gps.lon = dmToDd(lon, lonD);
    gps.latDir = latD;
    gps.lonDir = lonD;
  }
}

/**
 * @brief Parse validity, position, speed, and date from an RMC sentence.
 *
 * handleNMEA() calls this function for a checksum-verified GPRMC or GNRMC
 * sentence. Speed is converted from knots to kilometres per hour.
 *
 * @param p Writable, null-terminated RMC sentence.
 * @return Nothing.
 */
void parseRMC(char* p) {
  // $GPRMC,time,status,lat,N,lon,E,speed,course,date,...
  char* tok = strtok(p, ","); // $GPRMC
  tok = strtok(NULL, ","); // time
  tok = strtok(NULL, ","); // status A/V
  gps.valid = (tok && tok[0] == 'A');
  tok = strtok(NULL, ","); // lat
  char* lat = tok;
  tok = strtok(NULL, ","); // N/S
  char latD = tok ? tok[0] : 'N';
  tok = strtok(NULL, ","); // lon
  char* lon = tok;
  tok = strtok(NULL, ","); // E/W
  char lonD = tok ? tok[0] : 'E';
  tok = strtok(NULL, ","); // speed knots
  gps.speed = (tok && strlen(tok) > 0) ? atof(tok) * 1.852 : 0;
  tok = strtok(NULL, ","); // course
  tok = strtok(NULL, ","); // date
  if (tok && strlen(tok) == 6) {
    snprintf(gps.dateStr, sizeof(gps.dateStr), "20%c%c/%c%c/%c%c",
             tok[4], tok[5], tok[2], tok[3], tok[0], tok[1]);
  }
  
  if (gps.valid) {
    gps.lat = dmToDd(lat, latD);
    gps.lon = dmToDd(lon, lonD);
    gps.latDir = latD;
    gps.lonDir = lonD;
  }
}

/**
 * @brief Read the kilometres-per-hour field from a VTG sentence.
 *
 * handleNMEA() calls this function when VTG data is available. The result acts
 * as an additional speed source alongside RMC.
 *
 * @param p Writable, null-terminated VTG sentence.
 * @return Nothing.
 */
void parseVTG(char* p) {
  char* tok = strtok(p, ","); // $GPVTG
  tok = strtok(NULL, ","); // true track
  tok = strtok(NULL, ","); // T
  tok = strtok(NULL, ","); // mag track
  tok = strtok(NULL, ","); // M
  tok = strtok(NULL, ","); // speed knots
  tok = strtok(NULL, ","); // N
  tok = strtok(NULL, ","); // speed km/h
  if (tok && strlen(tok) > 0) {
    gps.speed = atof(tok);
  }
}

/**
 * @brief Validate and dispatch the NMEA sentence stored in nmeaLine.
 *
 * loop() calls this function after receiving a line terminator. Short or
 * checksum-invalid lines are discarded, and sentence types not shown on the
 * display are intentionally ignored.
 *
 * @param None.
 * @return Nothing.
 */
void handleNMEA() {
  if (nmeaIndex < 10) return;
  nmeaLine[nmeaIndex] = '\0';
  
  if (!checkNMEA(nmeaLine)) return;
  
  /*-------------------------------------------------------------
   * Dispatch supported navigation sentences
   * Accept both the legacy GP talker ID and multi-constellation GN ID.
   *------------------------------------------------------------*/
  if (strncmp(nmeaLine, "$GPGGA", 6) == 0 || strncmp(nmeaLine, "$GNGGA", 6) == 0) {
    parseGGA(nmeaLine);
  }
  else if (strncmp(nmeaLine, "$GPRMC", 6) == 0 || strncmp(nmeaLine, "$GNRMC", 6) == 0) {
    parseRMC(nmeaLine);
  }
  else if (strncmp(nmeaLine, "$GPVTG", 6) == 0 || strncmp(nmeaLine, "$GNVTG", 6) == 0) {
    parseVTG(nmeaLine);
  }
  // GSA, GSV, GLL, and other sentences do not feed this screen.
}

/**
 * @brief Redraw the LCD with either acquisition status or navigation data.
 *
 * loop() calls this function every 800 ms. A valid fix shows coordinates,
 * altitude, speed, satellite count, and date; otherwise the screen explains
 * that the receiver is still acquiring satellites.
 *
 * @param None.
 * @return Nothing.
 */
void drawScreen() {
  // Retained for the original screen-state tracking design.
  static bool lastValid = false;
  
  // A full redraw keeps stale values off screen and is fast enough here.
  lcd.fillScreen(TFT_WHITE);
  
  // Title bar
  lcd.fillRect(0, 0, SCREEN_W, 36, gps.valid ? 0x1B5E : 0xC000); 
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.drawString(gps.valid ? "  GPS LOCKED" : "  NO SIGNAL", 8, 8);
  
  // Keep the latest receiver time visible in both fix states.
  lcd.setTextSize(1);
  lcd.drawString(gps.timeStr, 240, 14);
  
  // Main content
  lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  lcd.setTextSize(2);
  int y = 48;
  const int lh = 26; // line height
  char buf[48];
  
  if (!gps.valid) {
    // Hide invalid navigation values until the receiver reports a valid fix.
    lcd.setTextSize(3);
    lcd.setTextColor(0xC000); 
    lcd.drawString("Acquiring...", 50, 100);
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_DARKGREY);
    snprintf(buf, sizeof(buf), "Satellites: %d", gps.sats);
    lcd.drawString(buf, 80, 150);
    lcd.drawString("Please wait...", 80, 180);
    return;
  }
  
  /*-------------------------------------------------------------
   * Draw valid navigation data
   * Coordinates use the largest font; supporting values remain compact.
   *------------------------------------------------------------*/
  lcd.setTextSize(3);
  snprintf(buf, sizeof(buf), "%.5f", gps.lat);
  lcd.drawString(buf, 10, y);
  y += 36;
  
  snprintf(buf, sizeof(buf), "%.5f", gps.lon);
  lcd.drawString(buf, 10, y);
  y += 44;
  
  // Divider line
  lcd.fillRect(10, y - 4, 300, 2, TFT_LIGHTGREY);
  
  // Detailed info (small font)
  lcd.setTextSize(2);
  
  // Altitude
  snprintf(buf, sizeof(buf), "ALT  %.1f m", gps.alt);
  lcd.drawString(buf, 10, y);
  
  // Speed
  snprintf(buf, sizeof(buf), "SPD  %.1f", gps.speed);
  lcd.drawString(buf, 170, y);
  lcd.setTextSize(1);
  lcd.drawString("km/h", 280, y + 8);
  lcd.setTextSize(2);
  y += lh;
  
  // Satellite count + Date
  snprintf(buf, sizeof(buf), "SAT  %d", gps.sats);
  lcd.drawString(buf, 10, y);
  
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString(gps.dateStr, 200, y + 8);
}

/**
 * @brief Initialize the debug port, GPS UART, LCD, and startup screen.
 *
 * Arduino calls this function once after power-up or reset. The startup screen
 * remains visible until loop() performs the first scheduled redraw.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  
  lcd.begin();
  lcd.setRotation(1);
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  
  /*-------------------------------------------------------------
   * Show immediate startup feedback
   * This confirms the LCD path before a satellite fix is available.
   *------------------------------------------------------------*/
  lcd.fillScreen(TFT_WHITE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setTextSize(2);
  lcd.drawString("GPS Display", 100, 90);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Waiting for satellites...", 85, 130);
  
  Serial.println("GPS Display ready");
}

/**
 * @brief Assemble incoming NMEA lines and refresh the navigation display.
 *
 * Arduino calls this function repeatedly after setup(). Serial bytes are stored
 * until a line terminator arrives, then a bounded 800 ms display interval keeps
 * the screen readable while reception continues at full speed.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  /*-------------------------------------------------------------
   * Assemble one complete NMEA sentence
   * The size check reserves one byte for the terminating null character.
   *------------------------------------------------------------*/
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (c == '\n' || c == '\r') {
      if (nmeaIndex > 0) {
        handleNMEA();
        nmeaIndex = 0;
      }
    } else if (nmeaIndex < sizeof(nmeaLine) - 1) {
      nmeaLine[nmeaIndex++] = c;
    }
  }
  
  // Limit full-screen redraws to reduce visible flicker.
  static uint32_t lastDraw = 0;
  if (millis() - lastDraw > 800) {
    drawScreen();
    lastDraw = millis();
  }
}

#include <TFT_eSPI.h>
#include <Arduino.h>

// GPS uses hardware serial port 2
#define GPS_RX 3
#define GPS_TX 1
HardwareSerial gpsSerial(2);

TFT_eSPI lcd = TFT_eSPI();

// Screen dimensions
#define SCREEN_W 480
#define SCREEN_H 320

// GPS parsing variables
char nmeaLine[128];
byte nmeaIndex = 0;

// Parsed valid data
struct {
  bool valid = false;
  float lat = 0;
  float lon = 0;
  char latDir = 'N';
  char lonDir = 'E';
  float alt = 0;
  float speed = 0;
  uint8_t sats = 0;
  uint8_t fixType = 0; // 0=无, 1=GPS, 2=DGPS
  char timeStr[10] = "--:--:--";
  char dateStr[12] = "----/--/--";
} gps;

// Calculate NMEA checksum
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

// Convert DM format to decimal degrees
float dmToDd(const char* dm, char dir) {
  if (!dm || strlen(dm) < 3) return 0;
  float val = atof(dm);
  int deg = (int)(val / 100);
  float min = val - deg * 100;
  float dd = deg + min / 60.0;
  return (dir == 'S' || dir == 'W') ? -dd : dd;
}

// Parse GGA
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

// Parse RMC
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

// Parse VTG (backup speed)
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

// Process one NMEA line
void handleNMEA() {
  if (nmeaIndex < 10) return;
  nmeaLine[nmeaIndex] = '\0';
  
  if (!checkNMEA(nmeaLine)) return;
  
  // Only parse sentences we care about
  if (strncmp(nmeaLine, "$GPGGA", 6) == 0 || strncmp(nmeaLine, "$GNGGA", 6) == 0) {
    parseGGA(nmeaLine);
  }
  else if (strncmp(nmeaLine, "$GPRMC", 6) == 0 || strncmp(nmeaLine, "$GNRMC", 6) == 0) {
    parseRMC(nmeaLine);
  }
  else if (strncmp(nmeaLine, "$GPVTG", 6) == 0 || strncmp(nmeaLine, "$GNVTG", 6) == 0) {
    parseVTG(nmeaLine);
  }
  // Ignore GSA/GSV/GLL etc. not displayed on screen
}

// Refresh screen display
void drawScreen() {
  static bool lastValid = false;
  
  // Clear screen only on status change or data update to reduce flicker
  // Simple handling: redraw every time (TFT_eSPI is fast enough)
  lcd.fillScreen(TFT_WHITE);
  
  // Title bar
  lcd.fillRect(0, 0, SCREEN_W, 36, gps.valid ? 0x1B5E : 0xC000); 
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.drawString(gps.valid ? "  GPS LOCKED" : "  NO SIGNAL", 8, 8);
  
  // Timestamp on status change
  lcd.setTextSize(1);
  lcd.drawString(gps.timeStr, 240, 14);
  
  // Main content
  lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  lcd.setTextSize(2);
  int y = 48;
  const int lh = 26; // line height
  char buf[48];
  
  if (!gps.valid) {
    // When not positioned: large text prompt, reduce information clutter
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
  
  // === Positioned: concise display of key data ===
  
  // Coordinates (largest font)
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

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  
  lcd.begin();
  lcd.setRotation(1);
  pinMode(27, OUTPUT);
  digitalWrite(27, HIGH);
  
  // Startup screen
  lcd.fillScreen(TFT_WHITE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setTextSize(2);
  lcd.drawString("GPS Display", 100, 90);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Waiting for satellites...", 85, 130);
  
  Serial.println("GPS Display ready");
}

void loop() {
  // Read GPS
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
  
  // Refresh screen every 800ms (comfortable for eyes, no flicker)
  static uint32_t lastDraw = 0;
  if (millis() - lastDraw > 800) {
    drawScreen();
    lastDraw = millis();
  }
}
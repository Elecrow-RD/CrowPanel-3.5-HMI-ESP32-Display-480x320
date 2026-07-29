#include <WiFi.h>

/*---------------------------------------------------------------
 * Wireless network credentials
 * Replace these demonstration values with the local 2.4 GHz network.
 *--------------------------------------------------------------*/
const char *ssid = "elecrow888";
const char *password = "elecrow2014";

/**
 * @brief Connect the ESP32 to Wi-Fi and report its assigned IP address.
 *
 * Arduino calls this function once after power-up or reset. Execution remains
 * in the connection loop until the access point accepts the device.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println("connecting");
  }

  Serial.println("WiFi is connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
//  WiFi.disconnect();
}

/**
 * @brief Keep the sketch active after the one-time connection procedure.
 *
 * Arduino calls this function repeatedly after setup(). Automatic reconnection
 * is handled by the Wi-Fi stack, so no foreground work is required here.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}

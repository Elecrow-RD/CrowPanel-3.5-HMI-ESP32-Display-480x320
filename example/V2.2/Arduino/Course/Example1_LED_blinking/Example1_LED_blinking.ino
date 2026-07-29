/*---------------------------------------------------------------
 * LED output configuration
 * GPIO25 drives the onboard indicator used by this example.
 *--------------------------------------------------------------*/
#define D_PIN 25

/**
 * @brief Prepare the serial port and LED output.
 *
 * Arduino calls this function once after power-up or reset. The pin must be
 * configured as an output before the repeating blink sequence starts.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);
  pinMode(D_PIN, OUTPUT);
}

/**
 * @brief Blink the LED with equal on and off intervals.
 *
 * Arduino calls this function repeatedly after setup(). Each complete cycle
 * keeps the LED on for 500 ms and off for 500 ms.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
  digitalWrite(D_PIN, HIGH);
  delay(500);
  digitalWrite(D_PIN, LOW);
  delay(500);
}

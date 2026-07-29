#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <BLECharacteristic.h>

/*---------------------------------------------------------------
 * BLE identity and attribute UUIDs
 * A client discovers the advertised service and accesses its characteristic.
 *--------------------------------------------------------------*/
#define bleServerName "ESP32SPI-BLE"
#define SERVICE_UUID "6479571c-2e6d-4b34-abe9-c35116712345"
#define CHARACTERISTIC_UUID "826f072d-f87c-4ae6-a416-6ffdcaa02d73"

// Points to the advertising controller used to make the service discoverable.
BLEAdvertising* pAdvertising = NULL;
// Owns BLE connections and dispatches connection callbacks.
BLEServer* pServer = NULL;
// Groups the characteristic under the UUID advertised by this example.
BLEService *pService = NULL;
// Stores the readable, writable, and notifiable ELECROW value.
BLECharacteristic* pCharacteristic = NULL;
// Records whether a client currently has an active server connection.
bool connected_state = false;

/*---------------------------------------------------------------
 * BLE connection state callbacks
 * The BLE stack invokes this class when a client connects or disconnects.
 *--------------------------------------------------------------*/
class MyServerCallbacks: public BLEServerCallbacks {
  /**
   * @brief Record that a BLE client has connected.
   *
   * The BLE stack calls this function when it accepts a connection.
   *
   * @param pServer Server that accepted the connection.
   * @return Nothing.
   */
  void onConnect(BLEServer *pServer) {
    connected_state = true;
  }

  /**
   * @brief Record that the BLE client has disconnected.
   *
   * The BLE stack calls this function when the active connection closes.
   *
   * @param pServer Server whose connection closed.
   * @return Nothing.
   */
  void onDisconnect(BLEServer *pServer) {
    connected_state = false;
  }
};

/**
 * @brief Create and advertise the BLE service and characteristic.
 *
 * Arduino calls this function once after power-up or reset. A client can read
 * the initial ELECROW value, write a new value, or subscribe to notifications.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(115200);

  BLEDevice::init(bleServerName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->setValue("ELECROW");

  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  pService->start();
  //  pAdvertising->stop();
  //  pService->stop();
}

/**
 * @brief Keep the BLE stack active after one-time server configuration.
 *
 * Arduino calls this function repeatedly after setup(). BLE events are handled
 * asynchronously by the stack and the registered callbacks.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}

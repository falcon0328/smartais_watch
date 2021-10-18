#include <M5StickC.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

extern const unsigned char safe_48[2304];
extern const unsigned char danger_48[4608];
extern const unsigned char caution_48[4608];

// アイコンのサイズ
// 仮に48が指定されている場合は48x48の画像を利用するという意味になる
#define ICON_SIZE           48
// 各UI要素間のマージン
#define MARGIN              8

#define DISPLAY_NAME        "SmartAIS-Watch"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// BLE
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

void drawSafeIcon() {
  sprite.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), BLACK);
  sprite.pushImage(8, 16, ICON_SIZE, ICON_SIZE, (uint16_t *)safe_48);
  sprite.drawString("Safety", (MARGIN + ICON_SIZE) + MARGIN, ((MARGIN + ICON_SIZE) / 2), 4);
  sprite.pushSprite(0, 0);
}

void drawDangerIcon() {
  sprite.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), BLACK);
  sprite.pushImage(8, 16, ICON_SIZE, ICON_SIZE, (uint16_t *)danger_48);
  sprite.drawString("Danger", (MARGIN + ICON_SIZE) + MARGIN, ((MARGIN + ICON_SIZE) / 2), 4);
  sprite.pushSprite(0, 0);
}

void drawCautionIcon() {
  sprite.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), BLACK);
  sprite.pushImage(8, 16, ICON_SIZE, ICON_SIZE, (uint16_t *)caution_48);
  sprite.drawString("Caution", (MARGIN + ICON_SIZE) + MARGIN, ((MARGIN + ICON_SIZE) / 2), 4);
  sprite.pushSprite(0, 0);
}

class SmartAisWatchServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class SmartAisWatchCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        if (rxValue == "0") {
          drawSafeIcon();
        } else if (rxValue == "1") {
          drawDangerIcon();
        } else if (rxValue == "2") {
          drawCautionIcon();
        }
      }
    }
};

void setup() {
  // Serial.begin(115200);

  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.setSwapBytes(false);
  sprite.setSwapBytes(true);
  sprite.createSprite(M5.Lcd.width(), M5.Lcd.height());

    // Create the BLE Device
  BLEDevice::init(DISPLAY_NAME);
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new SmartAisWatchServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pRxCharacteristic->setCallbacks(new SmartAisWatchCallbacks());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
}

void loop() {
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    
    // Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}


//#define SHOW_FREE_MEMORY

#ifdef SHOW_FREE_MEMORY
#include <MemoryFree.h>
#endif

// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>
#include <TTP229.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

const int SCL_PIN = 2;  // The pin number of the clock pin.
const int SDO_PIN = 3;  // The pin number of the data pin.

TTP229 ttp229(SCL_PIN, SDO_PIN); // TTP229(sclPin, sdoPin)
// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9
int lines=0;
const char * bleVal;char telNumber[20]="";int telIndex=0;

// create peripheral instance, see pinouts above
BLEPeripheral                    blePeripheral       = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
BLEService  testService         = BLEService("f62b9bb4-1486-43a0-b042-cc9b62e21e14");
// create counter characteristic
BLECharacteristic   testCharacteristic  =BLECharacteristic("3bbf43e5-6e4f-40ef-9e48-129eeecefd5e", BLERead | BLEWrite | BLEWriteWithoutResponse | BLENotify,bleVal  /*| BLEIndicate*/);
// create user description descriptor for characteristic
BLEDescriptor testDescriptor      = BLEDescriptor("2901", "counter");

// last counter update time
unsigned long long               lastSent            = 0;
void msg(String s);
void msgl(String s);
void blePeripheralConnectHandler(BLECentral& central);
void blePeripheralDisconnectHandler(BLECentral& central);
void characteristicWritten(BLECentral& central, BLECharacteristic& characteristic);
void characteristicSubscribed(BLECentral& central, BLECharacteristic& characteristic);
void characteristicUnsubscribed(BLECentral& central, BLECharacteristic& characteristic);
void setup() {
  Serial.begin(9600);
#if defined (__AVR_ATmega32U4__)
  delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
#endif
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  delay(2000);
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  // Clear the buffer.
  display.clearDisplay();

  blePeripheral.setLocalName("test");
#if 1
  blePeripheral.setAdvertisedServiceUuid(testService.uuid());
#else
  const char manufacturerData[4] = {0x12, 0x34, 0x56, 0x78};
  blePeripheral.setManufacturerData(manufacturerData, sizeof(manufacturerData));
#endif

  // set device name and appearance
  blePeripheral.setDeviceName("Test");
  blePeripheral.setAppearance(0x0080);

  // add service, characteristic, and decriptor to peripheral
  blePeripheral.addAttribute(testService);
  blePeripheral.addAttribute(testCharacteristic);
  blePeripheral.addAttribute(testDescriptor);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  testCharacteristic.setEventHandler(BLEWritten, characteristicWritten);
  testCharacteristic.setEventHandler(BLESubscribed, characteristicSubscribed);
  testCharacteristic.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);

  // set initial value for characteristic
  testCharacteristic.setValue(0);

  // begin initialization
  blePeripheral.begin();

  msgl(F("BLE Peripheral"));

#ifdef SHOW_FREE_MEMORY
  msg(F("Free memory = "));
  msgl(freeMemory());
#endif
}
void loop() {
  uint8_t key = ttp229.GetKey16(); // Non Blocking
  char a[5];itoa(key,a,10);
 // msgl(a);

  BLECentral central = blePeripheral.central();

  if (central) {
    // central connected to peripheral
    msg(F("Connected to central: "));
    msgl(central.address());

    // reset counter value
    testCharacteristic.setValue(0);

    while (central.connected()) {
      // central still connected to peripheral
      if (testCharacteristic.written()) {
        // central wrote new value to characteristic
        msgl(F("counter written, reset"));
        msgl(F(testCharacteristic.value()));

         // reset counter value
        lastSent = 0;
        testCharacteristic.setValue(0);
      }

      if (millis() > 1000 && (millis() - 1000) > lastSent) {
        // atleast one second has passed since last increment
        lastSent = millis();
          uint8_t pKey= key; 
          key = ttp229.GetKey16(); // Non Blocking
          if (key!=pKey && key!=0){
          telNumber[telIndex++]=key-10;
          }
          if(telIndex>15) telIndex=0;
          testCharacteristic.setValue(telNumber);}
        
        

      }
    }

    // central disconnected
    msg(F("Disconnected from central: "));
    msgl(central.address());
  
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  msg(F("Connected event, central: "));
  msgl(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  msg(F("Disconnected event, central: "));
  msgl(central.address());
}

void characteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic value written event handler
  msg(F("Characteristic event, writen: "));
  //msgl(testCharacteristic.value(), DEC);
  //msgl(testCharacteristic.value());
  memcpy(telNumber,testCharacteristic.value(),20);
           Serial.print(telNumber);
           display.print(telNumber);
           

}

void characteristicSubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic subscribed event handler
  msgl(F("Characteristic event, subscribed"));
}

void characteristicUnsubscribed(BLECentral& central, BLECharacteristic& characteristic) {
  // characteristic unsubscribed event handler
  msgl(F("Characteristic event, unsubscribed"));
}
void msgl (String s){
  if(lines++>12){
    lines=0;
    display.clearDisplay();
    display.setCursor(0,0);
  }
  Serial.println(s);
  display.println(s);
  display.display();
}
void msg (String s){
    if(lines++>12){
    lines=0;
    display.clearDisplay();
    display.setCursor(0,0);
  }
 
  Serial.print(s);
  display.print(s);
  display.display();
}





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
const char *bleTX,*bleRX;char telNumber[15]="";char charMessage[200]="";int telIndex=0;

// create peripheral instance, see pinouts above
BLEPeripheral                    blePeripheral       = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
BLEService informationService("180A");
BLECharacteristic modelCharacteristic("2A24", BLERead, "v1.0");
BLECharacteristic manufacturerCharacteristic("2A29", BLERead, "Noname Supplier");
BLECharacteristic serialNumberCharacteristic("2A25", BLERead, "2.71828");
BLEService  telephoneService         = BLEService("f62b9bb4-1486-43a0-b042-cc9b62e21e14");
// create counter characteristic
BLECharacteristic   txCharacteristic  =BLECharacteristic("3bbf43e5-6e4f-40ef-9e48-129eeecefd5e",   BLENotify| BLEIndicate,bleTX  /*| BLEIndicate*/);
BLECharacteristic   rxCharacteristic  =BLECharacteristic("0783b03e-8535-b5a0-7140-a304d2495cb8",BLEWriteWithoutResponse /*| B,LENotify| BLEIndicate  | BLEIndicate*/,bleRX);

// create user description descriptor for characteristic
BLEDescriptor testDescriptor      = BLEDescriptor("2901", "counter");
    BLEService uartService = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    BLEDescriptor uartNameDescriptor = BLEDescriptor("2901", "UART");
    BLECharacteristic uart_rxCharacteristic = BLECharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWriteWithoutResponse, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
    BLEDescriptor uart_rxNameDescriptor = BLEDescriptor("2901", "RX - Receive Data (Write)");
    BLECharacteristic uart_txCharacteristic = BLECharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
    BLEDescriptor uart_txNameDescriptor = BLEDescriptor("2901", "TX - Transfer Data (Notify)");

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
  Serial.begin(115200);
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

  blePeripheral.setLocalName("Telephone Receiver");
#if 1
  blePeripheral.setAdvertisedServiceUuid(telephoneService.uuid());
#else
  const char manufacturerData[4] = {0x12, 0x34, 0x56, 0x78};
  blePeripheral.setManufacturerData(manufacturerData, sizeof(manufacturerData));
#endif

  // set device name and appearance
  blePeripheral.setDeviceName("Telephone");
  blePeripheral.setAppearance(0x0080);

  // add service, characteristic, and decriptor to peripheral
  blePeripheral.addAttribute(telephoneService);
  blePeripheral.addAttribute(txCharacteristic);
  blePeripheral.addAttribute(testDescriptor);
  blePeripheral.addAttribute(rxCharacteristic);
  blePeripheral.addAttribute(testDescriptor);
  blePeripheral.addAttribute(informationService);
  blePeripheral.addAttribute(modelCharacteristic);
  blePeripheral.addAttribute(manufacturerCharacteristic);
  blePeripheral.addAttribute(serialNumberCharacteristic);
  blePeripheral.addAttribute(uartService);
  blePeripheral.addAttribute(uartNameDescriptor);
  blePeripheral.setAdvertisedServiceUuid(uartService.uuid());
  blePeripheral.addAttribute(uart_rxCharacteristic);
  blePeripheral.addAttribute(uart_rxNameDescriptor);
  uart_rxCharacteristic.setEventHandler(BLEWritten, characteristicWritten);
  blePeripheral.addAttribute(uart_txCharacteristic);
  blePeripheral.addAttribute(uart_txNameDescriptor);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  // assign event handlers for characteristic
  txCharacteristic.setEventHandler(BLEWritten, characteristicWritten);
  txCharacteristic.setEventHandler(BLESubscribed, characteristicSubscribed);
  txCharacteristic.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);
  rxCharacteristic.setEventHandler(BLEWritten, characteristicWritten);
  rxCharacteristic.setEventHandler(BLESubscribed, characteristicSubscribed);
  rxCharacteristic.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);
  uart_txCharacteristic.setEventHandler(BLEWritten, characteristicWritten);
  uart_txCharacteristic.setEventHandler(BLESubscribed, characteristicSubscribed);
  uart_txCharacteristic.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);
  uart_rxCharacteristic.setEventHandler(BLEWritten, characteristicWritten);
  uart_rxCharacteristic.setEventHandler(BLESubscribed, characteristicSubscribed);
  uart_rxCharacteristic.setEventHandler(BLEUnsubscribed, characteristicUnsubscribed);

  // set initial value for characteristic
  txCharacteristic.setValue("");

  // begin initialization
  blePeripheral.begin();

  msgl(F("BLE Peripheral"));

#ifdef SHOW_FREE_MEMORY

  msg(F("Free memory = "));
  msgl(freeMemory());
#endif
}
void loop() {
  blePeripheral.poll();
  uint8_t key = ttp229.GetKey16(); // Non Blocking
  char a[5];itoa(key,a,10);
 // msgl(a);

  BLECentral central = blePeripheral.central();

 
    // reset counter value
    int timeinsec=millis()/1000;
    char test[10];itoa(timeinsec,a,10);
    txCharacteristic.setValue(a);
    uart_txCharacteristic.setValue(a);
//    Serial.println(a);

    
      // central still connected to peripheral
      if (txCharacteristic.written()) {
        // central wrote new value to characteristic
        msgl(F("counter written, calling call back function. millis()="));
        msgl(F(millis()));
      }
         // reset counter value
       // lastSent = 0;
        //txCharacteristic.setValue(0);
  

      if (millis() > 1000 && (millis() - 1000) > lastSent) {
        // atleast one second has passed since last increment
        lastSent = millis();
          uint8_t pKey= key; 
          key = ttp229.GetKey16(); // Non Blocking
          if (key!=pKey && key!=0){
          telNumber[telIndex++]=key-10;
          }
          if(telIndex>15) telIndex=0;
          txCharacteristic.setValue(telNumber);}
        
        

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
  msg(F("Ch event, wr:\n "));
  //msgl(txCharacteristic.value(), DEC);
  //msgl(txCharacteristic.value());
 
  Serial.println(characteristic.valueLength());
  memcpy(charMessage,characteristic.value(),characteristic.valueLength());
  charMessage[characteristic.valueLength()]='\0';
  msgl(charMessage);
  //         Serial.println(charMessage);
  //         display.println(charMessage);
           

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
  if(lines++>8){
    lines=0;
    display.clearDisplay();
    display.setCursor(0,0);
  }
  Serial.println(s);
  display.println(s);
  display.display();
}
void msg (String s){
    if(lines++>9){
    lines=0;
    display.clearDisplay();
    display.setCursor(0,0);
  }
 
  Serial.print(s);
  display.print(s);
  display.display();
}




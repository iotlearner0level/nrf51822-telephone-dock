# bletest
Experiments for connecting mobile phone with nrf51822 using ble 


Hello everyone,

this project is my experiments on communication of a nrf51822 based board (currently using waveshare ble400 ) and mit app inventor 2 bluetooth low energy. Also connected is a ssd1306 based lcd 128x64 which prints same messages as printed on the serial monitor.

a fake uuid is generated for ble services and charactersitics from here https://www.uuidgenerator.net/

right now, it sends millis to the ble characteristics and prints any messages received from the mobile phone to oled and the serial monitor.


my ultimate aim is to print incoming phone calls and text messages on the oled so that when there is any new incoming phone call, the number is displayed on the oled so one doesn't have to look at the phone....




On the arduino side for the nrf51822 (waveshare ble400 board), i use this board definition:https://github.com/sandeepmistry/arduino-nRF5
and a library for wrapping up nordic functions: https://github.com/sandeepmistry/arduino-BLEPeripheral

for the mit ai2 part, we need ble extension (see extension section on the mit ai2 page: http://appinventor.mit.edu/extensions/

There are many example on how to use the ble extension with arduino ble devices: 
* http://www.instructables.com/id/Build-Android-Bluetooth-App-for-Arduino/
* http://don.github.io/slides/2016-06-18-arduino-ble/#/
* http://www.appinventor.tw/ble_analogread
* https://create.arduino.cc/projecthub/45349/control-rgb-led-by-dragging-arduino-101-app-inventor-98ab0b
* https://create.arduino.cc/projecthub/alexis-santiago-allende/ble-clock-with-arduino-and-app-inventor-a724a3

This is a very good introduction to the ble component of the MIT appinventor 2:
* http://ai2inventor.blogspot.in/2017/06/controlling-bluetooth-low-energy.html


Adding phone support is easy with the Social>Phone number palette in the Designer. We have to add a When PhoneCallStarted block and write the PhoneNumber & Status to to bluetooth characteristic, which is also read by the arduino code.

For dialling from the keypad on the arduino, we have to send the number using another characteristic which is read by the ai2 app.

Right now app inventor doesn't support services, but there is an experimental version ( http://services.appinventor.mit.edu/ ) which supports services. Some information here: https://ai2gsoc16.wordpress.com/app-inventor-services/ So that means, on the android side, app can be called every time a phone call is made/received and the data could be sent via ble notify. This can be done by using Add Task.. 


i've  very limited knowledge, if you have any ideas, comments or suggestions, please write in the issues tab!


thank you.

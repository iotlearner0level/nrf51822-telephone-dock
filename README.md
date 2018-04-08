# bletest
Test app for connecting ble 
Hello everyone,

this project is my experiments on communication of a nrf51822 based board (currently using waveshare ble400 ) and mit app inventor 2 bluetooth low energy. Also connected is a ssd1306 based lcd 128x64 which prints same messages as printed on the serial monitor.

a fake uuid is generated for ble services and charactersitics from here https://www.uuidgenerator.net/

right now, it sends millis to the ble characteristics and prints any messages received from the mobile phone to oled and the serial monitor.


my ultimate aim is to print incoming phone calls and text messages on the oled so that when there is any new incoming phone call, the number is displayed on the oled so one doesn't have to look at the phone....


i've  very limited knowledge, if you have any ideas, comments or suggestions, please write in the issues tab!

thank you.

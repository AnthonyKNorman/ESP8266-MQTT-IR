# ESP8266-MQTT-IR
A Sony infra-red controller available via MQTT

Using the Arduino platform, program an ATTiny85 with ATTINY85_IR_I2C.ino

Load irmqtt.py onto your ESP82666 - I used a WEMOS D1 Mini

You can see how to hook it up here http://www.indianbeantree.co.uk/wiki/index.php?title=WEMOS_D1_mini_IoT_IR_Controller

Make sure you use 3v3 for the 4k7 ohm pullup resistors on the two I2C lines

You can find a typical set of codes for a Sony TV remote here http://lirc.sourceforge.net/remotes/sony/RM-ED009


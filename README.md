# EnergyMeter
A DIY attempt to make electronic energy meter.

### Build and run
```
arduino-cli compile -b arduino:avr:nano . && arduino-cli upload -v -p /dev/ttyUSB* -b arduino:avr:nano:cpu=atmega328old . && arduino-cli monitor -p /dev/ttyUSB* --config 115200
```

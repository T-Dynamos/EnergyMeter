# EnergyMeter

A DIY attempt to make an electronic energy meter using Arduino.

## Features

- Measures voltage, current, power, and total energy consumed.
- Displays real-time values on an OLED screen.
- Stores total energy across restarts.
- On boot, if D7 (or chosen pin) is connected to GND, the energy counter is reset.

---

## Build and Run

```bash
arduino-cli compile -b arduino:avr:nano . \
&& arduino-cli upload -v -p /dev/ttyUSB* -b arduino:avr:nano:cpu=atmega328old . \
&& arduino-cli monitor -p /dev/ttyUSB* --config 115200
```
> Make sure the correct port /dev/ttyUSB* is used and accessible.

## Schematic
![image](https://github.com/user-attachments/assets/1ea62a03-5247-47bb-b224-0f632963e95c)


## Ratings

- **Input Voltage**: 0 V – 30 V
- **Current Range**: 0 A – ~8 A
- **Shunt Resistor**: 0.05 Ω, ≥2 W

## Photos

| Front View               | Back View (ugly)         |
|--------------------------|--------------------------|
| ![front](https://github.com/user-attachments/assets/a7649455-0d92-4a3b-a447-72aacb4be3af) | ![Back](https://github.com/user-attachments/assets/a0a0514a-56bf-4ef6-9556-4b03d5cca84b) |



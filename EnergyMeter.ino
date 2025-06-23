#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <EEPROM.h>
#include <math.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

float SHUNT = 0.05;
float RATIO = 8.880;
int readings = 100;
float rd_delay = 4;
float V_MULT = 6.2574;

unsigned long previousMillis = 0;
float totalEnergyWh = 0;

const int resetPin = 7;       // D7 pin
const int eepromAddr = 0;     // EEPROM address to store totalEnergyWh (4 bytes)

// ===== VCC Reading (internal 1.1V ref method) =====
long readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  long result = ADCL;
  result |= ADCH << 8;
  return 1105259UL / result;
}

// ===== EEPROM float save/load =====
void writeFloatToEEPROM(int address, float value) {
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(float); i++) {
    EEPROM.update(address + i, p[i]);
  }
}

float readFloatFromEEPROM(int address) {
  float value;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(float); i++) {
    p[i] = EEPROM.read(address + i);
  }
  return value;
}

void setup() {
  pinMode(resetPin, INPUT_PULLUP);
  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();

  delay(100); // Let pin settle

  if (digitalRead(resetPin) == LOW) {
    // Clear EEPROM if D7 is LOW on boot
    totalEnergyWh = 0;
    writeFloatToEEPROM(eepromAddr, totalEnergyWh);
    Serial.println("EEPROM cleared.");
  } else {
    totalEnergyWh = readFloatFromEEPROM(eepromAddr);
    Serial.print("Restored Energy: ");
    Serial.println(totalEnergyWh);
  }

  previousMillis = millis();
}

void loop() {
  float ref_step = (float)readVcc() / 1023000;

  long i_sum = 0;
  long v_sum = 0;
  for (int i = 0; i < readings; i++) {
    i_sum += analogRead(A3);
    v_sum += analogRead(A2);
    delay(rd_delay);
  }

  float avgCurrent = (((float)i_sum * ref_step) / readings) / (SHUNT * (1 + RATIO));  
  float avgVoltage = ((float)v_sum * ref_step) / readings;
  avgVoltage = avgVoltage * V_MULT;

  unsigned long currentMillis = millis();
  float dT = (float)(currentMillis - previousMillis) / 1000.0;
  float dE = avgVoltage * avgCurrent * dT;
  totalEnergyWh += dE / 3600.0;
  previousMillis = currentMillis;

  // Save to EEPROM every loop
  writeFloatToEEPROM(eepromAddr, totalEnergyWh);

  float power = avgVoltage * avgCurrent;

  String currentText = (avgCurrent >= 1.0) ?
    "I: " + String(avgCurrent, 3) + " A" :
    "I: " + String(avgCurrent * 1000, 0) + " mA";

  String voltageText = "V: " + String(avgVoltage, 2) + " V";
  String energyText = "E: " + String(totalEnergyWh, 2) + " wH";
  String powerText = "P: " + String(power, 2) + " W";

  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(currentText);
  display.setCursor(0, 24);
  display.print(voltageText);

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print(energyText);
  display.setCursor(0, 56);
  display.print(powerText);

  display.display();
}

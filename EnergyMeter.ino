#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <math.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

float SHUNT = 0.05;
float RATIO = 10;
int readings = 200;
float rd_delay = 2;

unsigned long previousMillis = 0;
float totalEnergyWh = 0;

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1105259UL/ result; // Back-calculate AVcc in mV
  return result;
}

void calibrate() {
  // https://forum.arduino.cc/t/how-to-know-vcc-voltage-in-arduino/344001/7
  // Read "1.1V" reference against AREF
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  unsigned int reading = ADC;
  // unsigned int reading = ADCL | (ADCH << 8);
  float fraction = reading / 1024.0;
  Serial.println(F("Use a meter to measure the voltage between GND and AREF pins."));
  Serial.print(F("Multiply that voltage by "));
  Serial.print(fraction, 7);
  Serial.println(F(" to get the value of the internal voltage reference."));
  Serial.println(F("Then multiply that by 1204000 to get the constant to use in readVCC()."));
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();
  // calibrate
  calibrate();
  previousMillis = millis();
}

void loop() {
  float v_ref = (float) readVcc() / 1000;
  
  // current
  float i_sum = 0;
  float v_sum = 0;
  for (int i = 0; i < readings; i++) {
    float voltage = analogRead(A3) * (v_ref / 1023);
    float current = voltage / (SHUNT * (1 + RATIO));
    i_sum += current;
    v_sum += analogRead(A2) * (v_ref / 1023);
    delay(rd_delay);
  }
  float avgCurrent = i_sum / readings;  
  float avgVoltage = v_sum / readings;
  
  unsigned long currentMillis = millis();
  // energy = integral P(t)dt
  // dE = V * I * dT
  // dE <- Delta energy
  float dT = (float)(currentMillis - previousMillis)/1000;
  float dE = (avgVoltage * avgCurrent * dT);
  totalEnergyWh += dE / 3600.0; // in watt hour
  previousMillis = currentMillis;
  
  // prepare text for display
  String currentText;
  if (avgCurrent >= 1.0) {
    currentText = "I: " + String(avgCurrent, 3) + " A";
  } else {
    currentText = "I: " + String(avgCurrent * 1000, 0) + " mA";
  }
  String energyText = "E: " + String(totalEnergyWh, 2) + " wH";
  String voltageText = "V: " + String(avgVoltage, 2) + " v";

  // Clear display and prepare for text
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(0, 0); // 1
  display.print(currentText.c_str());

  display.setCursor(0, 24); // 2
  display.print(voltageText.c_str());

  display.setCursor(0, 48); // 3
  display.print(energyText.c_str());

  display.display();
  Serial.println(dE, 6);
}

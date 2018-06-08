#include <odroid_go.h>
#include <driver/adc.h>

#define RESISTANCE_NUM    2
#define ADC_DB_11_RATE    3.9
#define ADC_WIDTH_12_RATE 4096

void setup() {
  // put your setup code here, to run once:
  GO.begin();

  GO.lcd.setTextSize(2);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

double readBatteryVoltage() {
  return adc1_get_raw(ADC1_CHANNEL_0) * RESISTANCE_NUM * ADC_DB_11_RATE / ADC_WIDTH_12_RATE;
}

void showBatteryVoltage(double voltage) {
  GO.lcd.clear();
  GO.lcd.setCursor(0, 0);

  GO.lcd.printf("Current Voltage: %1.3lf V \n", voltage);
}

void loop() {
  // put your main code here, to run repeatedly:

  showBatteryVoltage(readBatteryVoltage());
  delay(1000);
}

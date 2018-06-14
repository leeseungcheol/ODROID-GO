#include <odroid_go.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define RESISTANCE_NUM    2
#define DEFAULT_VREF      1100
#define NO_OF_SAMPLES     64

static esp_adc_cal_characteristics_t adc_chars;

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    GO.lcd.setTextSize(2);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
}

double readBatteryVoltage() {
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t) ADC1_CHANNEL_0);
    }
    adc_reading /= NO_OF_SAMPLES;

    return (double) esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) * RESISTANCE_NUM / 1000;
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

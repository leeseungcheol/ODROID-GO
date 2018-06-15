#include <LiquidCrystal_I2C.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define RESISTANCE_NUM  2
#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   64

static esp_adc_cal_characteristics_t adc_chars;

LiquidCrystal_I2C *lcd;

void setup() {
    // put your setup code here, to run once:
    uint8_t lcd_addr;

    Wire.begin(15, 4);
    Serial.begin(115200);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);

    lcd_addr = getAddress();
    if (lcd_addr < 1) {
        Serial.println("Does not find any slave address of the LCD");
        return;
    }

    lcd = new LiquidCrystal_I2C(lcd_addr, 16, 2);

    lcd->init();
    lcd->backlight();
    lcd->setCursor(0, 0);
    lcd->print("Hello, ODROID-GO");
}

uint8_t getAddress() {
    Wire.beginTransmission(0x27);
    uint8_t lcd_addr = 0;
    if (!Wire.endTransmission()) {
        lcd_addr = 0x27;
    } else {
        Wire.beginTransmission(0x3f);
        if (!Wire.endTransmission()) {
            lcd_addr = 0x3f;
        }
    }
    return lcd_addr;
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
    lcd->setCursor(0, 1);
    lcd->printf("Battery: %1.2lf V \n", voltage);
}

void loop() {
    // put your main code here, to run repeatedly:
    showBatteryVoltage(readBatteryVoltage());
    delay(1000);
}

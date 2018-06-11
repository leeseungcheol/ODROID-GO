#include <odroid_go.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define RESISTANCE_NUM    2

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_0;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static void check_efuse()
{
  //Check TP is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
    printf("eFuse Two Point: Supported\n");
  } else {
    printf("eFuse Two Point: NOT supported\n");
  }

  //Check Vref is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
    printf("eFuse Vref: Supported\n");
  } else {
    printf("eFuse Vref: NOT supported\n");
  }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
    printf("Characterized using Two Point Value\n");
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    printf("Characterized using eFuse Vref\n");
  } else {
    printf("Characterized using Default Vref\n");
  }
}

void setup() {
  // put your setup code here, to run once:
  GO.begin();

  GO.lcd.setTextSize(2);

  check_efuse();
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten,
                            ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
  print_char_val_type(val_type);
}

double readBatteryVoltage() {
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw((adc1_channel_t)channel);
  }
  adc_reading /= NO_OF_SAMPLES;
  //Convert adc_reading to voltage in mV
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
  printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
  return (double)voltage * RESISTANCE_NUM / 1000;
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

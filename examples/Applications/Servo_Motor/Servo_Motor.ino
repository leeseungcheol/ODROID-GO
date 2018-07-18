/* ODROID-GO with Servo Motor */
#include <odroid_go.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define RESISTANCE_NUM  2
#define DEFAULT_VREF    1100

static esp_adc_cal_characteristics_t adc_chars;

/* Header 10 : P2-4 */
#define SOFT_PWM_PORT2  15
/* Header 10 : P2-5 */
#define SOFT_PWM_PORT1  4

#define SERVO_MAX_uSEC  2000
#define SERVO_MIN_uSEC  1000
#define SERVO_SET_STEP  50

/* Battery low is 3.5V */
#define BATTERY_LOW_VALUE 35
#define BATTERY_MAX_VALUE 42

#define SERVO_FREQ_CNT  4
const unsigned short ServoFreq_Hz[SERVO_FREQ_CNT] = {
    50, 100, 200, 500
};

unsigned int SetFreq = 0;
unsigned int SetDuty = 0;

bool ServoAllwaysOn = false;
bool ServoEnable    = false;

unsigned int RefreshTime    = 0;
unsigned int LastPulse      = 0;
unsigned int BatteryVoltage = 0;

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    GO.lcd.setTextFont(4);
    GO.lcd.setTextSize(1);

    GO.lcd.setTextColor(WHITE);
    GO.lcd.setCursor(0, 0);
    GO.lcd.print("===== Servo Motor Test =====");

    GO.lcd.setTextColor(YELLOW);
    GO.lcd.setCursor(0, 32);
    GO.lcd.print("Battery Level : ");
    GO.lcd.setCursor(0, 64);
    GO.lcd.print("Servo Freq [unit : Hz]");
    GO.lcd.setCursor(0, 160);
    GO.lcd.print("Servo Duty [unit : uSec]");

    /* Default value setup */
    SetDuty = (SERVO_MAX_uSEC + SERVO_MIN_uSEC) / 2;
    RefreshTime = (1000 / ServoFreq_Hz[SetFreq]) - 1;

    /* Pin mode output setup for PWM */
    pinMode(SOFT_PWM_PORT1, OUTPUT);
    pinMode(SOFT_PWM_PORT2, OUTPUT);

    /* ODROID-GO Battery Check : ADC Port Init */
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);
    BatteryVoltage = readBatteryVoltage();

    display_status_update(BatteryVoltage, ServoFreq_Hz[SetFreq], SetDuty, ServoAllwaysOn | ServoEnable);
}

unsigned int readBatteryVoltage() {
    unsigned int adc_reading = 0;
    double voltage = 0;

    adc_reading = adc1_get_raw((adc1_channel_t) ADC1_CHANNEL_0);
    /* Battery voltage value * 10. eg) 3.5123412 -> return 35; */
    /*
        voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) * RESISTANCE_NUM / 1000;
    */
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) * RESISTANCE_NUM / 100;

    return (unsigned int)(voltage);
}

void display_status_update(unsigned int battery, unsigned int freq, unsigned int duty, bool servo_status) {
    if (battery) {
        /* Update battery value */
        GO.lcd.setTextSize(1);
        GO.lcd.fillRect(160, 32, 160, 32, 0x0000);
        if (battery > BATTERY_LOW_VALUE)
            GO.lcd.setTextColor(GREEN);
        else
            GO.lcd.setTextColor(RED);
        GO.lcd.setCursor(160, 32);
        GO.lcd.printf("%d.%d V", battery / 10, battery % 10);
    }

    if (servo_status)
        GO.lcd.setTextColor(GREEN);
    else
        GO.lcd.setTextColor(RED);

    if (freq) {
        GO.lcd.setTextSize(2);
        GO.lcd.fillRect(0, 96, 320, 64, 0x0000);
        GO.lcd.setCursor(0, 96);
        GO.lcd.print(freq);
        GO.lcd.print(" Hz ");
    }

    if (duty) {
        GO.lcd.setTextSize(2);
        GO.lcd.fillRect(0, 192, 320, 64, 0x0000);
        GO.lcd.setCursor(0, 192);
        GO.lcd.print(duty);
        GO.lcd.print(" uSec ");
    }
}

void check_servo_status() {
    int value;
    bool update_flage = false;

    /* Servo On/Off Control (Button A) */
    value = GO.BtnB.isPressed() ? true : false;
    if (ServoEnable != value) {
        ServoEnable = value;
        update_flage = true;
    }

    value = GO.BtnA.wasPressed() ? true : false;
    if (value) {
        ServoAllwaysOn = ServoAllwaysOn ? false : true;
        update_flage = true;
    }

    if (update_flage)
        display_status_update(0, ServoFreq_Hz[SetFreq], SetDuty, ServoAllwaysOn | ServoEnable);
}

void check_servo_freq() {
    int value;

    /* JOY UP/DOWN : Servo Freq Control */
    if ((value = GO.JOY_Y.wasAxisPressed())) {
        SetFreq += SERVO_FREQ_CNT;

        SetFreq += (value == 2) ? (+1) : (-1);

        SetFreq %= SERVO_FREQ_CNT;

        /* MAX 1Khz (unit : uSSec) */
        RefreshTime = (1000 / ServoFreq_Hz[SetFreq]) - 1;

        ServoEnable = false;
        display_status_update(0, ServoFreq_Hz[SetFreq], 0, ServoAllwaysOn | ServoEnable);
    }
}

void check_servo_duty() {
    int value;

    /* JOY LEFT/RIGHT : Servo Duty Control */
    if ((value = GO.JOY_X.wasAxisPressed())) {
        SetDuty += (value == 2) ? (-SERVO_SET_STEP) : SERVO_SET_STEP;
        if (SetDuty < SERVO_MIN_uSEC)
            SetDuty = SERVO_MAX_uSEC;
        if (SetDuty > SERVO_MAX_uSEC)
            SetDuty = SERVO_MIN_uSEC;

        ServoEnable = false;
        display_status_update(0, 0, SetDuty, ServoAllwaysOn | ServoEnable);
    }
}

void check_battery_status() {
    unsigned int value = readBatteryVoltage();

    if (BatteryVoltage != value) {
        if ((BatteryVoltage > value) || (value == BATTERY_MAX_VALUE)) {
            BatteryVoltage = value;
            display_status_update(BatteryVoltage, 0, 0, 0);
        }
    }
}

void servo_pwm_control() {
    if (millis() - LastPulse >= RefreshTime) {
        digitalWrite(SOFT_PWM_PORT1, HIGH);
        digitalWrite(SOFT_PWM_PORT2, HIGH);
        delayMicroseconds(SetDuty);
        digitalWrite(SOFT_PWM_PORT1, LOW);
        digitalWrite(SOFT_PWM_PORT2, LOW);
        LastPulse = millis();
    }
}

void loop() {
    /* Key, Speaker, Axis update */
    GO.update();
    /* ODROID-GO battery check */
    check_battery_status();
    /* Servo on/off control check */
    check_servo_status();
    /* Servo freq control check */
    check_servo_freq();
    /* Servo duty control check */
    check_servo_duty();

    /* Servo pwm port control */
    if (ServoEnable || ServoAllwaysOn)
        servo_pwm_control();
    else
        delay(100);
}

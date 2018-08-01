#define PIN_BLUE_LED  2
#define PWM_CHANNEL   1

int dutyCycle = 0;

void setup() {
    // put your setup code here, to run once:

    pinMode(PIN_BLUE_LED, OUTPUT);
    digitalWrite(PIN_BLUE_LED, HIGH);

    ledcAttachPin(PIN_BLUE_LED, PWM_CHANNEL);
    ledcSetup(PWM_CHANNEL, 12000, 8); // 12 kHz PWM, 8-bit resolution
}

void loop() {
    // put your main code here, to run repeatedly:

    for (dutyCycle = 0; dutyCycle < 255; dutyCycle++) {
        ledcWrite(PWM_CHANNEL, dutyCycle);
        delay(3);
    }
    for (dutyCycle = 255; dutyCycle > 0; dutyCycle--) {
        ledcWrite(PWM_CHANNEL, dutyCycle);
        delay(3);
    }
}

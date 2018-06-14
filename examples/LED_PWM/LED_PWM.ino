#define PIN_BLUE_LED  2
#define PIN_CHANNEL   1

int pinVal = 0;

void setup() {
    // put your setup code here, to run once:

    pinMode(PIN_BLUE_LED, OUTPUT);
    digitalWrite(PIN_BLUE_LED, HIGH);

    ledcAttachPin(PIN_BLUE_LED, PIN_CHANNEL);
    ledcSetup(PIN_CHANNEL, 12000, 8);
}

void loop() {
    // put your main code here, to run repeatedly:

    for (; pinVal >= 0; pinVal--) {
        ledcWrite(PIN_CHANNEL, pinVal);
        delay(3);
    }
    for (; pinVal < 256; pinVal++) {
        ledcWrite(PIN_CHANNEL, pinVal);
        delay(3);
    }
}

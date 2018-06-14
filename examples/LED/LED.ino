#define PIN_BLUE_LED 2

void setup() {
    // put your setup code here, to run once:

    pinMode(PIN_BLUE_LED, OUTPUT);
}

void loop() {
    // put your main code here, to run repeatedly:

    digitalWrite(PIN_BLUE_LED, HIGH);
    delay(500);
    digitalWrite(PIN_BLUE_LED, LOW);
    delay(500);
}

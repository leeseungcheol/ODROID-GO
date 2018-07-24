#include <odroid_go.h>

#define PIN_BLUE_LED  2
#define PIN_PIR_INPUT 4

int prePirValue;
int pirValue;
int sensorBlockingTimeout;

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    GO.lcd.setTextFont(4);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(WHITE);
    GO.lcd.setCursor(0, 0);
    GO.lcd.print("= PIR Motion Detector Test =");

    pinMode(PIN_BLUE_LED, OUTPUT);
    pinMode(PIN_PIR_INPUT, INPUT);
    digitalWrite(PIN_BLUE_LED, LOW);

    GO.lcd.setTextSize(2);
    GO.lcd.setTextColor(BLUE);
    for (int i = 60; i > 0; i--) {
        GO.lcd.fillRect(0, 100, 320, 48, BLACK);
        GO.lcd.setCursor(6, 100);
        GO.lcd.printf("Initilizing... %2d", i);
        delay(1000);
    }

    displayStatusUpdate();
}

void displayStatusUpdate() {
    GO.lcd.fillRect(0, 100, 320, 56, BLACK);
    GO.lcd.setTextSize(2);

    if (pirValue) {
        GO.lcd.setTextColor(GREEN);
        GO.lcd.setCursor(58, 100);
        GO.lcd.print("Detected");

        sensorBlockingTimeout = 3;
    } else {
        GO.lcd.setTextColor(RED);
        GO.lcd.setCursor(14, 100);
        GO.lcd.print("Not Detected");

        while (prePirValue) {
            GO.lcd.fillRect(0, 170, 320, 32, BLACK);
            GO.lcd.setTextSize(2);

            if (sensorBlockingTimeout == 0) {
                break;
            } else {
                GO.lcd.setTextSize(1);
                GO.lcd.setCursor(100, 170);
                GO.lcd.printf("Timeout: %d", sensorBlockingTimeout);

                delay(1000);
                sensorBlockingTimeout--;
            }
        }
    }
}

void loop() {
    // put your main code here, to run repeatedly:
    pirValue = digitalRead(PIN_PIR_INPUT);
    if (prePirValue != pirValue) {
        displayStatusUpdate();
        prePirValue = pirValue;
    }

    digitalWrite(PIN_BLUE_LED, pirValue);
}
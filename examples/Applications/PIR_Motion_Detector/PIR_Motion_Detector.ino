#include <odroid_go.h>

#define PIN_BLUE_LED  2
#define PIN_PIR_INPUT 4

int prePirValue;
int pirValue;

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

    displayStatusUpdate();
}

void displayStatusUpdate() {
    GO.lcd.fillRect(0, 100, 320, 48, BLACK);
    GO.lcd.setTextSize(2);

    if (pirValue) {
        GO.lcd.setTextColor(GREEN);
        GO.lcd.setCursor(58, 100);
        GO.lcd.print("Detected");
    } else {
        GO.lcd.setTextColor(RED);
        GO.lcd.setCursor(16, 100);
        GO.lcd.print("Not Detected");
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

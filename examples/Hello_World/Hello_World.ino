#include <odroid_go.h>

uint8_t idx;
uint8_t rotate;

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    GO.lcd.println("Hello, ODROID-GO");

    delay(1000);
}

void displayGO() {
    GO.lcd.clearDisplay();
    GO.lcd.setRotation(rotate + 4);
    GO.lcd.setCursor(30, 40);

    if (idx) {
        GO.lcd.setTextSize(1);
        GO.lcd.setTextFont(4);
        GO.lcd.setTextColor(MAGENTA);
    } else {
        GO.lcd.setTextSize(2);
        GO.lcd.setTextFont(1);
        GO.lcd.setTextColor(GREEN);
    }
    GO.lcd.print("Hello, ODROID-GO");

    idx = !idx;
    rotate++;
    rotate = rotate % 4;

    delay(1000);
}

void loop() {
    // put your main code here, to run repeatedly:
    displayGO();
}

/* ODROID-GO with Ultrasonic Sensor */
#include <odroid_go.h>
#include <sensors/Ultrasonic.h>

/* Header 10 : P2-4 */
#define TRIG_IO_PORT  15
/* Header 10 : P2-5 */
#define ECHO_IO_PORT  4

Ultrasonic ultrasonic(TRIG_IO_PORT, ECHO_IO_PORT);
float distance = 0;

void setup() {
    // put your setup code here, to run once:
    GO.begin();
    GO.lcd.setTextFont(4);
    GO.lcd.setCursor(0, 0);
    GO.lcd.setTextColor(WHITE);
    GO.lcd.print("===== Ultrasonic Sensor =====");
}

void display_distance() {
    float distance_new = ultrasonic.distanceRead(CM);

    GO.lcd.setCursor(0, 32);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(YELLOW);
    GO.lcd.print("Distance [unit : inch]");
    GO.lcd.setCursor(0, 128);
    GO.lcd.print("Distance [unit : cm]");

    if (!distance_new)
        GO.lcd.setTextColor(RED);
    else  {
        GO.lcd.setTextColor(GREEN);
        GO.lcd.fillRect(0, 64, 320, 64, 0x0000);
        GO.lcd.fillRect(0, 160, 320, 64, 0x0000);
        distance = distance_new;
    }
    GO.lcd.setTextSize(2);
    GO.lcd.setCursor(0, 64);
    /* unit convert cm to inch : 1 cm = 0.3937 inch */
    GO.lcd.print(distance * 0.3937);
    GO.lcd.print(" inch ");

    GO.lcd.setCursor(0, 160);
    GO.lcd.print(distance);
    GO.lcd.print(" cm ");
}

void loop() {
    // put your main code here, to run repeatedly:
    display_distance();
    delay(1000);
}

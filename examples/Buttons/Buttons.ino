#include <odroid_go.h>

void setup() {
    // put your setup code here, to run once:
    GO.begin();
    GO.lcd.setTextSize(2);
}

void displayButtons() {
    GO.lcd.clear();
    GO.lcd.setCursor(0, 0);

    GO.lcd.println("/* Direction pad */");
    GO.lcd.printf("Joy-Y-Up: %s \n", (GO.JOY_Y.isAxisPressed() == 2) ? "Pressed" : " ");
    GO.lcd.printf("Joy-Y-Down: %s \n", (GO.JOY_Y.isAxisPressed() == 1) ? "Pressed" : " ");
    GO.lcd.printf("Joy-X-Left: %s \n", (GO.JOY_X.isAxisPressed() == 2) ? "Pressed" : " ");
    GO.lcd.printf("Joy-X-Right: %s \n", (GO.JOY_X.isAxisPressed() == 1) ? "Pressed" : " ");
    GO.lcd.println("");
    GO.lcd.println("/* Function key */");
    GO.lcd.printf("Menu: %s \n", (GO.BtnMenu.isPressed() == 1) ? "Pressed" : " ");
    GO.lcd.printf("Volume: %s \n", (GO.BtnVolume.isPressed() == 1) ? "Pressed" : " ");
    GO.lcd.printf("Select: %s \n", (GO.BtnSelect.isPressed() == 1) ? "Pressed" : " ");
    GO.lcd.printf("Start: %s \n", (GO.BtnStart.isPressed() == 1) ? "Pressed" : " ");
    GO.lcd.println("");
    GO.lcd.println("/* Actions */");
    GO.lcd.printf("B: %s \n", (GO.BtnB.isPressed() == 1) ? "Pressed" : " ");
    GO.lcd.printf("A: %s \n", (GO.BtnA.isPressed() == 1) ? "Pressed" : " ");
}

void loop() {
    // put your main code here, to run repeatedly:
    GO.update();
    displayButtons();
    delay(1000);
}

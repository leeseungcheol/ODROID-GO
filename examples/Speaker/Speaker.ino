#include <odroid_go.h>

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    GO.lcd.printf("ODROID-GO Speaker test:\r\n");
    GO.Speaker.setVolume(8);
    GO.Speaker.playMusic(m5stack_startup_music, 25000);
}

void loop() {
    // put your main code here, to run repeatedly:

    if (GO.BtnA.wasPressed()) {
        GO.lcd.printf("wasPressed: A \r\n");
        GO.Speaker.beep();
    }

    if (GO.BtnB.wasPressed()) {
        GO.lcd.printf("wasPressed: B \r\n");
        GO.Speaker.tone(3000, 200);
    }

    if (GO.BtnStart.wasPressed()) {
        GO.lcd.printf("wasPressed: Start \r\n");
        GO.Speaker.playMusic(m5stack_startup_music, 25000);
    }

    GO.update();
}

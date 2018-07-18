#include <odroid_go.h>
#include "sensors/ir/IRremote.h"

const int RECV_PIN = 4;

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    GO.lcd.setTextDatum(MC_DATUM);
    GO.lcd.setTextFont(4);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(WHITE);
    GO.lcd.drawString("======   IR Receiver Test   ======", 120, 0);

    Serial.println("Enabling IRin");
    irrecv.enableIRIn(); // Start the receiver
    Serial.println("Enabled IRin");

    GO.lcd.setTextSize(2);
    GO.lcd.setTextColor(RED);
    displayStatusUpdate("NO SIGNAL");
}

void decodeSignal() {
    GO.lcd.setTextColor(GREEN);
    switch (results.value) {
        case 0x4DB23BC4:
            displayStatusUpdate("POWER");
            break;
        case 0x4DB211EE:
            displayStatusUpdate("MUTE");
            break;
        case 0x4DB241BE:
            displayStatusUpdate("HOME");
            break;
        case 0x4DB2738C:
            displayStatusUpdate("OK");
            break;
        case 0x4DB253AC:
            displayStatusUpdate("UP");
            break;
        case 0x4DB29966:
            displayStatusUpdate("LEFT");
            break;
        case 0x4DB2837C:
            displayStatusUpdate("RIGHT");
            break;
        case 0x4DB24BB4:
            displayStatusUpdate("DOWN");
            break;
        case 0x4DB2A35C:
            displayStatusUpdate("MENU");
            break;
        case 0x4DB259A6:
            displayStatusUpdate("BACK");
            break;
        case 0x4DB2817E:
            displayStatusUpdate("VOL DOWN");
            break;
        case 0x4DB201FE:
            displayStatusUpdate("VOL UP");
            break;
        case 0xFFFFFFFF:
            GO.lcd.setTextColor(BLUE);
            displayStatusUpdate("REFEATED");
            break;
        case 0x4DB2:
            GO.lcd.setTextColor(RED);
            displayStatusUpdate("BROKEN");
            break;
        default:
            GO.lcd.setTextColor(RED);
            displayStatusUpdate("UNKNOWN");
            break;
    }
}

void displayStatusUpdate(String result) {
    GO.lcd.fillRect(0, 92, 320, 48, BLACK);
    GO.lcd.drawString(result, 160, 122);
}

void loop() {
    // put your main code here, to run repeatedly:
    if (irrecv.decode(&results)) {
        Serial.print("received: ");
        Serial.println(results.value, HEX);

        decodeSignal();
        irrecv.resume(); // Receive the next value
    }
    delay(100);
}

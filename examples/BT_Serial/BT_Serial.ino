#include <odroid_go.h>
#include "BluetoothSerial.h"

BluetoothSerial serialBT;

void setup() {
    GO.begin();

    serialBT.begin("ODROID-GO");
    Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
    if (Serial.available()) {
        serialBT.write(Serial.read());
    }
    if (serialBT.available()) {
        Serial.write(serialBT.read());
    }
    delay(20);
}

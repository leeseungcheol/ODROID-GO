#include "BluetoothSerial.h"

BluetoothSerial serialBT;

void setup() {
    Serial.begin(115200);

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

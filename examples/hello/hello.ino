#include <odroid_go.h>

void setup() {
  // put your setup code here, to run once:

  GO.begin();
  GO.lcd.setTextFont(4);

  GO.lcd.print("Hello, ODROID-GO");
  pinMode(35, INPUT);
  pinMode(34, INPUT);

}
//4095
//1763

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("IO34 :");
  Serial.println(analogRead(34));
  delay(1000);

}

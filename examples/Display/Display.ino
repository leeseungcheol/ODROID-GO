#include <odroid_go.h>

// the setup routine runs once when M5Stack starts up
void setup() {
  
  // initialize the M5Stack object
  GO.begin();
  GO.lcd.setRotation(7);
  GO.lcd.drawBitmap(0, 0, 240, 320, (uint16_t *)gImage_logoM5);
  delay(500);

  // Lcd display
  GO.lcd.fillScreen(WHITE);
  delay(500);
  GO.lcd.fillScreen(RED);
  delay(500);
  GO.lcd.fillScreen(GREEN);
  delay(500);
  GO.lcd.fillScreen(BLUE);
  delay(500);
  GO.lcd.fillScreen(BLACK);
  delay(500);

  // text print
  GO.lcd.fillScreen(BLACK);
  GO.lcd.setCursor(10, 10);
  GO.lcd.setTextColor(WHITE);
  GO.lcd.setTextSize(1);
  GO.lcd.printf("Display Test!");

  // draw graphic
  delay(1000);
  GO.lcd.drawRect(100, 100, 50, 50, BLUE);
  delay(1000);
  GO.lcd.fillRect(100, 100, 50, 50, BLUE);
  delay(1000);
  GO.lcd.drawCircle(100, 100, 50, RED);
  delay(1000);
  GO.lcd.fillCircle(100, 100, 50, RED);
  delay(1000);
  GO.lcd.drawTriangle(30, 30, 180, 100, 80, 150, YELLOW);
  delay(1000);
  GO.lcd.fillTriangle(30, 30, 180, 100, 80, 150, YELLOW);

}

// the loop routine runs over and over again forever
void loop(){

  //rand draw 
  GO.lcd.fillTriangle(random(GO.lcd.width()-1), random(GO.lcd.height()-1), random(GO.lcd.width()-1), random(GO.lcd.height()-1), random(GO.lcd.width()-1), random(GO.lcd.height()-1), random(0xfffe));

  GO.update();
}

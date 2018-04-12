#include <odroid_go.h>

#define NOTE_D0 -1
#define NOTE_D1 294
#define NOTE_D2 330
#define NOTE_D3 350
#define NOTE_D4 393
#define NOTE_D5 441
#define NOTE_D6 495
#define NOTE_D7 556

#define NOTE_DL1 147
#define NOTE_DL2 165
#define NOTE_DL3 175
#define NOTE_DL4 196
#define NOTE_DL5 221
#define NOTE_DL6 248
#define NOTE_DL7 278

#define NOTE_DH1 589
#define NOTE_DH2 661
#define NOTE_DH3 700
#define NOTE_DH4 786
#define NOTE_DH5 882
#define NOTE_DH6 990
#define NOTE_DH7 112


void setup() {
  // Initialize the M5Stack object
  GO.begin();
  
  GO.lcd.printf("M5Stack Speaker test:\r\n");
  GO.Speaker.setVolume(8);
  GO.Speaker.playMusic(m5stack_startup_music, 25000);
}

void loop() {
  
  if(GO.BtnA.wasPressed()) {
    GO.lcd.printf("wasPressed A \r\n");
    GO.Speaker.beep(); //beep
  }

  if(GO.BtnB.wasPressed())
  {
    GO.lcd.printf("wasPressed B \r\n");
    GO.Speaker.tone(3000, 200); //frequency 3000, with a duration of 200ms
  }

  if(GO.BtnC.wasPressed())
  {
    GO.lcd.printf("wasPressed C \r\n");
    GO.Speaker.playMusic(m5stack_startup_music, 25000); //play the M5Stack startup sound
  }
  GO.update();
  
}

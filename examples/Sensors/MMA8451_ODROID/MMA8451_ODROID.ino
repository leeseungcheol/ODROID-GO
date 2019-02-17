#include <odroid_go.h>
#include <sensors/Adafruit_Sensor.h>
#include <sensors/Adafruit_MMA8451.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define SERIAL_OUT false  //print also to serial port

//pins
#define PIN_STATUS_LED  2
#define PIN_I2C_SDA    15
#define PIN_I2C_SCL     4

//display resolution
#define WIDTH  320
#define HEIGHT 240

//voltage
#define RESISTANCE_NUM 2
#define DEFAULT_VREF   1100
#define NO_OF_SAMPLES  64

Adafruit_MMA8451 mma = Adafruit_MMA8451();
static esp_adc_cal_characteristics_t adc_chars;

int count;
int refreshDisplayCounter;
int XArray[WIDTH];
int YArray[WIDTH];
int ZArray[WIDTH];

boolean showX = true;
boolean showY = true;
boolean showZ = true;
boolean showO = true;

int minMap = -2048;
int maxMap = +2048;

uint8_t brightness = 50;

void setup(void) {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  GO.begin();

  GO.lcd.setTextSize(2);
  GO.lcd.setCharCursor(0, 0);
  GO.lcd.setBrightness(brightness);

  println("Adafruit MMA8451 test!");
  if (! mma.begin()) {
    println("MMA8451 not found!");
    while (1);
  }
  println("MMA8451 found!");

  mma.setRange(MMA8451_RANGE_8_G);
  println("Range = " + String(2 << mma.getRange()) + "G");

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);

  delay(1);
  GO.lcd.clear();
}

void loop() {
  // Read the 'raw' data in 14-bit counts
  mma.read();

  if (SERIAL_OUT) {
    Serial.print("X: \t" + String(mma.x) + "\t");
    Serial.print("Y: \t" + String(mma.y) + "\t");
    Serial.print("Z: \t" + String(mma.z) + "\t");
    Serial.println();
  }

  // Get a new sensor event
  sensors_event_t event;
  mma.getEvent(&event);

  // Display the results (acceleration is measured in m/s^2)
  //  if (SERIAL_OUT) {
  //    Serial.print("Xm: \t" + String(event.acceleration.x) + "\t");
  //    Serial.print("Ym: \t" + String(event.acceleration.y) + "\t");
  //    Serial.print("Zm: \t" + String(event.acceleration.z) + "\t");
  //    Serial.println("m/s^2");
  //  }

  // Get the orientation of the sensor
  uint8_t o = mma.getOrientation();

  XArray[0] = map(mma.x, minMap, maxMap, 0, HEIGHT);
  YArray[0] = map(mma.y, minMap, maxMap, 0, HEIGHT);
  ZArray[0] = map(mma.z, minMap, maxMap, 0, HEIGHT);

  if (brightness > 1) {
    refreshDisplayCounter++;
    if (refreshDisplayCounter > WIDTH / 4) {
      refreshDisplayCounter = 0;

      GO.lcd.clear();
      GO.lcd.setTextSize(1);

      // Display the results (acceleration is measured in m/s^2)
      if (showX) {
        GO.lcd.setCharCursor(0, 0);
        GO.lcd.setTextColor(ILI9341_RED);
        GO.lcd.print("X: " + String(mma.x));
        GO.lcd.setCharCursor(20, 0);
        GO.lcd.print(String(event.acceleration.x) + " m/s^2");
      }

      if (showY) {
        GO.lcd.setCharCursor(0, 1);
        GO.lcd.setTextColor(ILI9341_GREEN);
        GO.lcd.print("Y: " + String(mma.y));
        GO.lcd.setCharCursor(20, 1);
        GO.lcd.print(String(event.acceleration.y) + " m/s^2");
      }

      if (showZ) {
        GO.lcd.setCharCursor(0, 2);
        GO.lcd.setTextColor(ILI9341_BLUE);
        GO.lcd.print("Z: " + String(mma.z));
        GO.lcd.setCharCursor(20, 2);
        GO.lcd.print(String(event.acceleration.z) + " m/s^2");
      }

      if (showO) {
        GO.lcd.setTextColor(ILI9341_WHITE);
        GO.lcd.setCharCursor(0, 3);
        printOrientation(o);
      }

      // print the lines
      for (count = 2; count <= WIDTH; count++ ) {
        if (showX)
          GO.lcd.drawLine(WIDTH - count, HEIGHT - XArray[count - 2], WIDTH - count, HEIGHT - XArray[count - 1], ILI9341_RED);
        if (showY)
          GO.lcd.drawLine(WIDTH - count, HEIGHT - YArray[count - 2], WIDTH - count, HEIGHT - YArray[count - 1], ILI9341_GREEN);
        if (showZ)
          GO.lcd.drawLine(WIDTH - count, HEIGHT - ZArray[count - 2], WIDTH - count, HEIGHT - ZArray[count - 1], ILI9341_BLUE);
      }
      showBatteryVoltage(readBatteryVoltage());
      showXYZButtons();
    }
  }

  //ZOOM in and out
  zoomABButtons();

  //shift the array
  for (count = WIDTH; count >= 1; count--)  {
    XArray[count - 1] = XArray[count - 2];
    YArray[count - 1] = YArray[count - 2];
    ZArray[count - 1] = ZArray[count - 2];
  }

  adjustBrightness();
}

double readBatteryVoltage() {
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw((adc1_channel_t) ADC1_CHANNEL_0);
  }
  adc_reading /= NO_OF_SAMPLES;

  return (double) esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) * RESISTANCE_NUM / 1000;
}

void showBatteryVoltage(double voltage) {
  GO.lcd.setTextColor(ILI9341_WHITE);
  GO.lcd.setCharCursor(46, 0);
  GO.lcd.printf("%1.3lf V", voltage);
}

void showXYZButtons() {
  GO.lcd.drawRect(0, 226, 50, 15, ILI9341_RED);
  if (showX) {
    GO.lcd.fillRect(2, 228, 46, 12, ILI9341_RED);
  }
  GO.lcd.setTextColor(ILI9341_WHITE);
  GO.lcd.setCharCursor(4, 29);
  GO.lcd.print("X");
  if (GO.BtnMenu.isPressed() == 1) {
    showX = !showX;
    delay(10);
  }

  GO.lcd.drawRect(70, 226, 50, 15, ILI9341_GREEN);
  if (showY) {
    GO.lcd.fillRect(72, 228, 46, 12, ILI9341_GREEN);
  }
  GO.lcd.setCharCursor(15, 29);
  GO.lcd.print("Y");
  if (GO.BtnVolume.isPressed() == 1 ) {
    showY = !showY;
    delay(10);
  }

  GO.lcd.drawRect(200, 226, 50, 15, ILI9341_BLUE);
  if (showZ) {
    GO.lcd.fillRect(202, 228, 46, 12, ILI9341_BLUE);
  }
  GO.lcd.setCharCursor(37, 29);
  GO.lcd.print("Z");
  if (GO.BtnSelect.isPressed() == 1 ) {
    showZ = !showZ;
    delay(10);
  }

  GO.lcd.drawRect(270, 226, 50, 15, ILI9341_WHITE);
  if (showO) {
    GO.lcd.fillRect(272, 228, 46, 12, ILI9341_LIGHTGREY);
  }
  GO.lcd.setCharCursor(49, 29);
  GO.lcd.print("O");
  if (GO.BtnStart.isPressed() == 1 ) {
    showO = !showO;
    delay(10);
  }
}

void adjustBrightness() {
  //Joy-Y-Up
  GO.update();
  if (GO.JOY_Y.isAxisPressed() == 2 && brightness < 254 ) {
    brightness++;
    GO.lcd.setBrightness(brightness);
  }
  //Joy-Y-Down
  if (GO.JOY_Y.isAxisPressed() == 1 && brightness >= 1 ) {
    brightness--;
    GO.lcd.setBrightness(brightness);
  }
}

void printOrientation(uint8_t o) {
  switch (o) {
    case MMA8451_PL_PUF:
      GO.lcd.println("Portrait Up Front");
      break;
    case MMA8451_PL_PUB:
      GO.lcd.println("Portrait Up Back");
      break;
    case MMA8451_PL_PDF:
      GO.lcd.println("Portrait Down Front");
      break;
    case MMA8451_PL_PDB:
      GO.lcd.println("Portrait Down Back");
      break;
    case MMA8451_PL_LRF:
      GO.lcd.println("Landscape Right Front");
      break;
    case MMA8451_PL_LRB:
      GO.lcd.println("Landscape Right Back");
      break;
    case MMA8451_PL_LLF:
      GO.lcd.println("Landscape Left Front");
      break;
    case MMA8451_PL_LLB:
      GO.lcd.println("Landscape Left Back");
      break;
  }
}

void zoomABButtons() {
  GO.lcd.setTextColor(ILI9341_WHITE);
  GO.lcd.setCharCursor(52, 27);
  GO.lcd.print("-");
  GO.lcd.setCharCursor(52, 2);
  GO.lcd.print("+");
  if (GO.BtnA.isPressed() == 1 && maxMap > 10) {
    minMap += 2;
    maxMap -= 2;
  }
  else if (GO.BtnB.isPressed() == 1 && maxMap <= 2048) {
    minMap -= 2;
    maxMap += 2;
  }
}

void print(String value) {
  GO.lcd.print(value);
  if (SERIAL_OUT) {
    Serial.print(value);
  }
}

void println(String value) {
  GO.lcd.println(value);
  if (SERIAL_OUT) {
    Serial.println(value);
  }
}
#include <odroid_go.h>
#include "sensors/Adafruit_TCS34725.h"

//- #define CALI_ENABLE
//- #define PROCESSING_ENABLE
#define LCD_ENABLE

#if defined(PROCESSING_ENABLE)
#undef LCD_ENABLE
#endif

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);

unsigned long preMillis = 0;
unsigned long curMillis = 0;
int rCal, gCal, bCal;
uint16_t clear, red, green, blue;
unsigned char step = 0;
int cali(void);

int cali(void) {

    uint32_t sum;
    float r, g, b;

    switch (step) {

        case 0:

            curMillis = millis();
            if (curMillis - preMillis >= 5000) {

                step = 1;

                tcs.setInterrupt(false);      // turn on LED
                tcs.getRawData(&red, &green, &blue, &clear);
                tcs.setInterrupt(true);  // turn off LED

                GO.lcd.clearDisplay();
                GO.lcd.setCursor(30, 50);
                GO.lcd.println("Put on GREEN");

                sum = clear;
                r = red; r /= sum;
                g = green; g /= sum;
                b = blue; b /= sum;
                r *= 256; g *= 256; b *= 256;

                rCal = r;

                preMillis = millis();
            }
            break;

        case 1:

            curMillis = millis();
            if (curMillis - preMillis >= 5000) {

                step = 2;

                tcs.setInterrupt(false);      // turn on LED
                //- delay(60);  // takes 50ms to read
                tcs.getRawData(&red, &green, &blue, &clear);
                tcs.setInterrupt(true);  // turn off LED

                GO.lcd.clearDisplay();
                GO.lcd.setCursor(30, 50);
                GO.lcd.println("Put on BLUE");

                sum = clear;
                r = red; r /= sum;
                g = green; g /= sum;
                b = blue; b /= sum;
                r *= 256; g *= 256; b *= 256;

                gCal = g;

                preMillis = millis();
            }
            break;

        case 2:

            curMillis = millis();
            if (curMillis - preMillis >= 5000) {
                step = 3;

                tcs.setInterrupt(false);      // turn on LED
                tcs.getRawData(&red, &green, &blue, &clear);
                tcs.setInterrupt(true);  // turn off LED

                sum = clear;
                r = red; r /= sum;
                g = green; g /= sum;
                b = blue; b /= sum;
                r *= 256; g *= 256; b *= 256;

                GO.lcd.clearDisplay();

                bCal = b;
            }
            break;

        case 3:

            return 0;

        default:;
    }

    return 1;
}

void setup() {
    GO.begin();
    Serial.println("Color View Test!");

    if (tcs.begin()) {
        Serial.println("Found sensor");
    } else {
        Serial.println("No TCS34725 found ... check your connections");
        while (1); // halt!
    }

    GO.lcd.setCursor(30, 50);
    GO.lcd.setTextSize(2);
    GO.lcd.println("Hello, ODROID-GO");

#if defined(CALI_ENABLE)
    delay(1000);

    GO.lcd.clearDisplay();
    GO.lcd.setCursor(30, 50);
    GO.lcd.print("Put on RED");
    preMillis = millis();

    while (cali());

    GO.lcd.clearDisplay();
    GO.lcd.setTextColor(WHITE);
    GO.lcd.setCursor(0, 40);
    GO.lcd.setTextSize(2);
    GO.lcd.print("Cali:R("); GO.lcd.print(int(rCal), DEC);
    GO.lcd.print(") G("); GO.lcd.print(gCal, DEC);
    GO.lcd.print(") B("); GO.lcd.print(bCal, DEC);
    GO.lcd.println(")");
#endif

    delay(2000);
}

void loop() {

    //- uint16_t clear, red, green, blue;

    tcs.setInterrupt(false);      // turn on LED
    //- delay(60);  // takes 50ms to read
    tcs.getRawData(&red, &green, &blue, &clear);
    tcs.setInterrupt(true);  // turn off LED

#if !defined(LCD_ENABLE)
    Serial.print("C:\t"); Serial.print(clear);
    Serial.print("\tR:\t"); Serial.print(red);
    Serial.print("\tG:\t"); Serial.print(green);
    Serial.print("\tB:\t"); Serial.print(blue);
#endif

    // Figure out some basic hex code for visualization
    uint32_t sum = clear;
    float r, g, b;
    r = red; r /= sum;
    g = green; g /= sum;
    b = blue; b /= sum;
    r *= 256; g *= 256; b *= 256;

#if defined(CALI_ENABLE)
    r = r - rCal + 256;
    g = g - gCal + 256;
    b = b - bCal + 256;
#endif

#if !defined(LCD_ENABLE)
    Serial.print("\t");
    Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
    Serial.println();
#endif

    //Serial.print((int)r ); Serial.print(" "); Serial.print((int)g);Serial.print(" ");  Serial.println((int)b );

#if defined(LCD_ENABLE)

    uint16_t colorTemp, lux, rgb565;

    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    rgb565 = (((int)r) & 0xF8) << 8 | (((int)g) & 0xFC) << 3 | ((int)b) >> 3;

    //- GO.lcd.clearDisplay();
    GO.lcd.fillScreen(rgb565);
    GO.lcd.setTextColor(WHITE);
    GO.lcd.setCursor(0, 40);
    GO.lcd.setTextSize(2);
    GO.lcd.print("* RGB565(Hex) : 0x");
    GO.lcd.println(rgb565, HEX);
    GO.lcd.println(" ");

    GO.lcd.print("* Color Temp : "); GO.lcd.println(colorTemp, DEC);
    GO.lcd.print("* Lux : "); GO.lcd.println(lux, DEC);
    GO.lcd.println(" ");

    GO.lcd.print("* R - raw : "); GO.lcd.print(int(r), DEC); GO.lcd.print(" - "); GO.lcd.println(red, DEC);
    GO.lcd.print("* G - raw : "); GO.lcd.print(int(g), DEC); GO.lcd.print(" - "); GO.lcd.println(green, DEC);
    GO.lcd.print("* B - raw : "); GO.lcd.print(int(b), DEC); GO.lcd.print(" - "); GO.lcd.println(blue, DEC);
    GO.lcd.print("* Clear   : "); GO.lcd.print(int(clear), DEC);

    delay(1000);

#endif
}


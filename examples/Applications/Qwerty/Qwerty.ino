#include <odroid_go.h>
#include "sensors/tca8418.h"
#include "sensors/RTClib.h"
#include "sensors/Wire.h"

#define __GO_LCD__
#define __RTC_ON__
#define __KEY_NUM__ 55

KEYS Keypad;
uint8_t checkedNum = 0;

const int PIN_I2C_SDA = 15;
const int PIN_I2C_SCL = 12;
const int PIN_INT = 4;

volatile bool KeyInt = false;

struct st_keyCheck {
    uint8_t code;
    uint8_t ckBtn;
    int32_t x;
    int32_t y;
    uint8_t c;
};

typedef struct st_keyCheck;
st_keyCheck keyCheck[__KEY_NUM__] = {
    {0x1,  0, 10,  10, '\`'}, // 1. `
    {0x2,  0, 35,  10, '1' }, // 2. 1
    {0x3,  0, 60,  10, '2' }, // 3. 2
    {0x4,  0, 85,  10, '3' }, // 4. 3
    {0x5,  0, 110, 10, '4' }, // 5. 4
    {0x6,  0, 135, 10, '5' }, // 6. 5
    {0x7,  0, 160, 10, '6' }, // 7. 6
    {0x8,  0, 185, 10, '7' }, // 8. 7
    {0xB,  0, 210, 10, '8' }, // 9. 8
    {0xC,  0, 235, 10, '9' }, // 10. 9
    {0xD,  0, 260, 10, '0' }, // 11. 0

    {0xE,  0, 10,  30, 'e' }, // 12. Esc
    {0xF,  0, 35,  30, 'Q' }, // 13. Q
    {0x10, 0, 60,  30, 'W' }, // 14. W
    {0x11, 0, 85,  30, 'E' }, // 15. E
    {0x12, 0, 110, 30, 'R' }, // 16. R
    {0x15, 0, 135, 30, 'T' }, // 17. T
    {0x16, 0, 160, 30, 'Y' }, // 18. Y
    {0x17, 0, 185, 30, 'U' }, // 19. U
    {0x18, 0, 210, 30, 'I' }, // 20. I
    {0x19, 0, 235, 30, 'O' }, // 21. O
    {0x1A, 0, 260, 30, 'P' }, // 22. P

    {0x1B, 0, 10,  50, 'c' }, // 23. Ctrl
    {0x1C, 0, 35,  50, 'A' }, // 24. A
    {0x1F, 0, 60,  50, 'S' }, // 25. S
    {0x20, 0, 85,  50, 'D' }, // 26. D
    {0x21, 0, 110, 50, 'F' }, // 27. F
    {0x22, 0, 135, 50, 'G' }, // 28. G
    {0x23, 0, 160, 50, 'H' }, // 29. H
    {0x24, 0, 185, 50, 'J' }, // 30. J
    {0x25, 0, 210, 50, 'K' }, // 31. K
    {0x26, 0, 235, 50, 'L' }, // 32. L
    {0x29, 0, 260, 50, 'b' }, // 33. BS

    {0x2A, 0, 10,  70, 'a' }, // 34. Alt
    {0x2B, 0, 35,  70, 'Z' }, // 35. Z
    {0x2C, 0, 60,  70, 'X' }, // 36. X
    {0x2D, 0, 85,  70, 'C' }, // 37. C
    {0x2E, 0, 110, 70, 'V' }, // 38. V
    {0x2F, 0, 135, 70, 'B' }, // 39. B
    {0x30, 0, 160, 70, 'N' }, // 40. N
    {0x33, 0, 185, 70, 'M' }, // 41. M
    {0x34, 0, 210, 70, '\|'}, // 42. |
    {0x35, 0, 235, 70, 'e' }, // 43. Enter
    {0x35, 0, 260, 70, '\ '}, // 43. Enter

    {0x36, 0, 10,  90, 'u' }, // 44. UP
    {0x37, 0, 35,  90, '\;'}, // 45. ;
    {0x38, 0, 60,  90, '\''}, // 46. '
    {0x39, 0, 85,  90, '\-'}, // 47. -
    {0x3A, 0, 110, 90, '\='}, // 48. =
    {0x3D, 0, 135, 90, 's' }, // 49. SPACE
    {0x3E, 0, 160, 90, '\,'}, // 50. ,
    {0x3F, 0, 185, 90, '\.'}, // 51. .
    {0x40, 0, 210, 90, '\/'}, // 52. /
    {0x41, 0, 235, 90, '\['}, // 53. [
    {0x42, 0, 260, 90, '\]'}, // 54. ]
};

PCF8563 rtc;
DateTime alarm_l;
char buf[100];

void KeyISR(void);
void setup(void);
void loop(void);
void setAlarm(int);

void KeyISR(void) {  //Keypad Interrupt Service Routine
    KeyInt = true;
}

#if defined(__RTC_ON__)
void setAlarm(int value) {
    strncpy(buf, "DD.MM.YYYY hh:mm:ss\0", 100);
    DateTime now = DateTime(rtc.now());
    rtc.adjust(now);
#if defined(__GO_LCD__)
    GO.lcd.println(now.format(buf));
#else
    Serial.println(now.format(buf));
#endif
}
#endif

void setup() {

#if defined(__GO_LCD__)
    GO.begin();
    GO.lcd.setCursor(20, 50);
    GO.lcd.setTextSize(2);
    GO.lcd.setTextColor(GREEN);
    GO.lcd.println("Hello, ODROID-GO QWERTY");
#else
    Serial.begin(9600);
#endif

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Keypad.begin(ROW0 | ROW1 | ROW2 | ROW3 | ROW4 | ROW5 | ROW6,
                 COL0 | COL1 | COL2 | COL3 | COL4 | COL5 | COL6 | COL7,
                 CFG_KE_IEN | CFG_OVR_FLOW_IEN | CFG_INT_CFG | CFG_OVR_FLOW_M);

    Keypad.writeByte(0x80, REG_GPIO_DIR1); //- ROW7 GPIO data direction OUTPUT
    Keypad.writeByte(0x80, REG_GPIO_DAT_OUT1); //- St LED ON
    Keypad.writeByte(0x3, REG_GPIO_DIR3); //- COL8, COL9 GPIO data direction OUTPUT
    Keypad.writeByte(0x3, REG_GPIO_DAT_OUT3); //- Aa, Fn LED ON

    pinMode(4, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(4), KeyISR, FALLING);

    delay(1000);

#if defined(__GO_LCD__)
    GO.lcd.clearDisplay();

    for (uint8_t i = 0; i < 55; i++) {
        GO.lcd.drawRect(keyCheck[i].x + 10, keyCheck[i].y + 5, 20, 18, 0xffff);
        GO.lcd.drawChar(keyCheck[i].c, keyCheck[i].x + 15, keyCheck[i].y + 7);
    }
#endif

    Keypad.writeByte(0x0, REG_GPIO_DAT_OUT1); //- St LED off
    Keypad.writeByte(0x0, REG_GPIO_DAT_OUT3); //- Aa, Fn LED OFF

#if defined(__GO_LCD__)
    GO.lcd.fillRect(20, 140, 320, 40, 0x0000);
    GO.lcd.setCursor(20, 140);
#endif
    //- Serial.begin(9600);

#if defined(__RTC_ON__)
    rtc.begin();
    if (!rtc.isrunning()) {
#if defined(__GO_LCD__)
        GO.lcd.println("RTC is NOT running!");
#endif
    } else {
#if defined(__GO_LCD__)
        GO.lcd.println("RTC is running");
#endif
        //- DateTime now = DateTime(2018, 10, 8, 11, 55, 0);
        DateTime now = DateTime(rtc.now());
        rtc.adjust(now);

        GO.lcd.setCursor(20, 160);
        setAlarm(1);
    }
#endif

}

void loop() {
#if 0
    //Check for Interrupt flag and process
    if (KeyInt) {
        uint8_t key;

        key = Keypad.readKeypad(); //Get first keycode from FIFO
        Serial.print("Keyboard ISR...Key:");
        Serial.print((key & 0x7F), HEX); //print keycode masking the key down/key up bit (bit7)
        if (key & 0x80) {
            Serial.println(" key down");
        } else {
            Serial.println(" key up");
        }
        KeyInt = false; //Reset Our Interrupt flag
        Keypad.clearInterruptStatus(); //Reset TCA8418 Interrupt Status Register.
    }
    //Do other processing
    ;
#else

    if (KeyInt) {
        uint8_t key;
        key = Keypad.readKeypad(); //Get first keycode from FIFO
#if defined(__GO_LCD__)
        GO.lcd.fillRect(20, 200, 320, 20, 0x0000);
        GO.lcd.setCursor(20, 200);
        GO.lcd.setTextSize(2);
        GO.lcd.print(key & 0x7F, HEX);
        //- GO.lcd.setCursor(20, 200);
        if (key & 0x80)
            GO.lcd.println(" KEY DOWN");
        else
            GO.lcd.println(" KEY UP");

        for (uint8_t i = 0; i < 55; i++) {

            if (!(keyCheck[i].ckBtn & 0x04)) {

                if (keyCheck[i].code == key) {

                    if (key & 0x80)
                        keyCheck[i].ckBtn |= 0x01;
                    else
                        keyCheck[i].ckBtn |= 0x02;

                    if (keyCheck[i].ckBtn & 0x03) {
                        GO.lcd.fillRect(keyCheck[i].x + 10, keyCheck[i].y + 5, 20, 18, 0xffff);
                        keyCheck[i].ckBtn |= 0x04;

                        if (++checkedNum >= __KEY_NUM__) {
                            detachInterrupt(digitalPinToInterrupt(PIN_INT));

                            GO.lcd.clearDisplay();
                            GO.lcd.println("OKAY, DONE");
                        }
                    }
                }
            }
        }
#else
        if (key & 0x80)
            Serial.println(" KEY DOWN");
        else
            Serial.println(" KEY UP");
#endif
        KeyInt = false; //Reset Our Interrupt flag
        Keypad.clearInterruptStatus(); //Reset TCA8418 Interrupt Status Register.
#if defined(__RTC_ON__)
#if defined(__GO_LCD__)
        GO.lcd.fillRect(20, 160, 320, 20, 0x0000);
        GO.lcd.setCursor(20, 160);
#endif
        setAlarm(1);
#endif

    }

#endif
}

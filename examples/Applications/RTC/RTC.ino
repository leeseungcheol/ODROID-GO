/*
 * RTC Test program for ODROID-GO with PCF8563 RTC Shield
 * Keymap:
 *  - in idle
 *   - Menu: Enable alarm. When it pushed for a first time, it enters to set up alarm mode automatically.
 *  - in idle (alarm enabled)
 *   - Menu: Disable alarm.
 *   - Select: Pressed it for 2 seconds, enter to set up new alarm.
 *   - A, B: When alarm is beeping, it stops sound.
 *  - in setting up alarm
 *   - Dpad: Move cursor or change an unit.
 *   - A: Finish.
 *  - please refer to buttonEventHandler() function.
 */

#include <odroid_go.h>
#include <WiFi.h>
#include "time.h"
#include "web/SimpleTimer.h"
#include "sensors/Rtc_Pcf8563.h"

/* User defined constants */
/*
 * If you want to get NTP server time,
 * enter SSID and its password for access to internet.
 */
#define USE_NTP_TIME        true
#if USE_NTP_TIME
#define WIFI_SSID           "******"
#define WIFI_PASSWORD       "******"
#define NTP_SERVER          "pool.ntp.org"
#define UTC_TIME_OFFSET     9
#endif

/*
 * Subtracting this value for aligning where the clock places in.
 * 0 to make clock time places in center.
 * 25 to make clock time including indicators places in center.
 * Enter between 0 ~ 50.
 */
#define CLOCK_POS_OFFSET    25

#define CLOCK_SCRN_COLOR    BLACK
#define CLOCK_DATE_COLOR    GREENYELLOW
#define CLOCK_TIME_COLOR    GREEN
#define CLOCK_24H_COLOR     GREENYELLOW
#define ALARM_IND_COLOR     ORANGE
#define ALARM_SET_COLOR     ORANGE

#define ALARM_RING_VOLUME   6
#define ALARM_RING_FREQ_HZ  3000
#define ALARM_RING_DUR      250
#define ALARM_RING_INTERVAL 500
#define ALARM_RING_COUNT    3

#define DEFAULT_DATE_DAY    1
#define DEFAULT_DATE_WDAY   0 // weekday (0 -> sun, 6 -> sat)
#define DEFAULT_DATE_MON    1
#define DEFAULT_DATE_CEN    0  // century (1 -> 1900, 0 -> 2000)
#define DEFAULT_DATE_YEAR   18
#define DEFAULT_TIME_HOUR   9
#define DEFAULT_TIME_MIN    0
#define DEFAULT_TIME_SEC    0

/* DO NOT CHANGE */
#define ARR_LENGTH(array)   (sizeof(array) / sizeof(array[0]))
const uint8_t  PIN_DAC_SD    = 25;
const uint8_t  PIN_ALARM_INT = 4;
const uint8_t  DELAY_NONE    = 0;
const uint8_t  DELAY_SHORT   = 500;
const uint16_t DELAY_LONG    = 2000;
const uint8_t  ALARM_IGNORE  = 99; // equivalent to RTCC_NO_ALARM

enum TIME_UPDATE_UNIT {
    UPDATE_HOUR,
    UPDATE_MINUTE,
    UPDATE_SECOND
};

enum ALARM_SET_CURSOR {
    CURSOR_NONE,
    CURSOR_HOUR,
    CURSOR_MINUTE,
    CURSOR_WEEKDAY
};
uint8_t alarmSetCursor = CURSOR_NONE;
bool colorToggleIndicator = false;

enum ALARM_SET_DPAD_DIRECTION {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
};

const String DAYS_OF_THE_WEEK[7] = {
    "Sun.", "Mon.", "Tue.", "Wed.",
    "Thu.", "Fri.", "Sat."
};

SimpleTimer timer;
int displayUpdateTimer;
int displayAlarmSetClockBlinkingTimer;
int alarmIntSoundTimer;
uint8_t curAlarmSoundCount = 0;

String curDate;
uint8_t curWeekday;
uint8_t curTime[3]; // hour, min, sec

String preDate;
uint8_t preWeekday;
uint8_t preTime[3]; // hour, min, sec

uint8_t alarmTime[2] = { ALARM_IGNORE, ALARM_IGNORE }; // hour, min
uint8_t alarmWeekday = ALARM_IGNORE;

Rtc_Pcf8563 rtc;

#if USE_NTP_TIME
bool initWifi() {
    String status = "";
    int loadingCnt = 0;

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        if (loadingCnt == 20) {
            // waiting for 10 seconds
            displayWifiStatusMsg("Connect failed", DELAY_SHORT);

            return false;
        }

        if (status.length() == 5) {
            status = "";
        }
        status.concat('.');
        loadingCnt++;
        displayWifiStatusMsg(status, DELAY_NONE);

        delay(500);
    }
    displayWifiStatusMsg("Connected", DELAY_SHORT);

    return true;
}

void displayWifiStatusMsg(String status, int delayTimeout) {
    displayClearClockScreen();

    GO.lcd.setTextSize(2);
    GO.lcd.setTextColor(CLOCK_TIME_COLOR);
    GO.lcd.drawString(status, 160, 128);

    delay(delayTimeout);
}
#endif

void setup() {
    // put your setup code here, to run once:
    GO.begin();

    // mute speaker
    pinMode(PIN_DAC_SD, OUTPUT);
    digitalWrite(PIN_DAC_SD, LOW);

    // enable the interrupt pin
    pinMode(PIN_ALARM_INT, INPUT_PULLUP);
    attachInterrupt(PIN_ALARM_INT, alarmIntHandler, FALLING);

    GO.lcd.setTextDatum(MC_DATUM);
    GO.lcd.setTextFont(4);
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(WHITE);
    GO.lcd.drawString("========       RTC Test      ========", 160, 0);

    GO.Speaker.setVolume(ALARM_RING_VOLUME);

    rtc.initClock();
#if USE_NTP_TIME
    if (initWifi()) {
        configTime(UTC_TIME_OFFSET * 3600, 0, NTP_SERVER);

        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            displayWifiStatusMsg("NTP error!!", DELAY_LONG);
            setDefaultDateTime();
        } else {
            rtc.setDate(timeinfo.tm_mday, timeinfo.tm_wday, timeinfo.tm_mon + 1, 0, timeinfo.tm_year % 100);
            rtc.setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    } else {
        setDefaultDateTime();
    }
#else
    setDefaultDateTime();
#endif

    displayClearClockScreen();
    displayTimeSeparator();
    display24hIndicator();

    displayUpdateTimer = timer.setInterval(1000, displayUpdateHandler);
    displayAlarmSetClockBlinkingTimer = timer.setInterval(125, displayAlarmSetClockBlinkingHandler);
    alarmIntSoundTimer = timer.setInterval(ALARM_RING_INTERVAL, alarmIntSoundHandler);

    timer.disable(displayAlarmSetClockBlinkingTimer);
    timer.disable(alarmIntSoundTimer);
}

void setDefaultDateTime() {
    rtc.setDate(DEFAULT_DATE_DAY, DEFAULT_DATE_WDAY, DEFAULT_DATE_MON,
                DEFAULT_DATE_CEN, DEFAULT_DATE_YEAR);
    rtc.setTime(DEFAULT_TIME_HOUR, DEFAULT_TIME_MIN, DEFAULT_TIME_SEC);
}

void displayUpdateHandler() {
    curDate = String(rtc.formatDate(RTCC_DATE_ASIA));
    curWeekday = rtc.getWeekday();
    curTime[0] = rtc.getHour();
    curTime[1] = rtc.getMinute();
    curTime[2] = rtc.getSecond();

    if (preDate != curDate || preWeekday != curWeekday) {

        displayUpdateDate();
        preDate = curDate;
        preWeekday = curWeekday;
    }

    if (preTime[0] != curTime[0]) {
        displayUpdateTime(UPDATE_HOUR);
        preTime[0] = curTime[0];
    } else if (preTime[1] != curTime[1]) {
        displayUpdateTime(UPDATE_MINUTE);
        preTime[1] = curTime[1];
    } else {
        displayUpdateTime(UPDATE_SECOND);
        preTime[2] = curTime[2];
    }
}

void displayClearClockScreen() {
    GO.lcd.fillRect(0, 80, 320, 80, CLOCK_SCRN_COLOR);
}

void displayUpdateDate() {
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(CLOCK_DATE_COLOR);

    displayDate();
    displayWeekday();
}

void displayDate() {
    GO.lcd.fillRect(50 - CLOCK_POS_OFFSET, 80, 150, 32, CLOCK_SCRN_COLOR);
    GO.lcd.drawString(curDate, 125 - CLOCK_POS_OFFSET, 102); // center: (125, 96)
}

void displayWeekday() {
    GO.lcd.fillRect(200 - CLOCK_POS_OFFSET, 80, 70, 32, CLOCK_SCRN_COLOR);
    GO.lcd.drawString(DAYS_OF_THE_WEEK[curWeekday], 235 - CLOCK_POS_OFFSET, 102); // center: (235, 96)
}

void displayUpdateTime(int which) {
    GO.lcd.setTextSize(2);
    GO.lcd.setTextColor(CLOCK_TIME_COLOR);

    switch (which) {
        case UPDATE_HOUR:
            displayTimeHour();
        case UPDATE_MINUTE:
            displayTimeMinute();
        case UPDATE_SECOND:
            GO.lcd.fillRect(200 - CLOCK_POS_OFFSET, 112, 70, 48, CLOCK_SCRN_COLOR);
            GO.lcd.drawString(addPaddingToTimeUnit(String(curTime[2])), 235 - CLOCK_POS_OFFSET, 142); // center: (235, 136)
            break;
        default:
            break;
    }
}

void displayTimeHour() {
    GO.lcd.fillRect(50 - CLOCK_POS_OFFSET, 112, 70, 48, CLOCK_SCRN_COLOR);
    GO.lcd.drawString(addPaddingToTimeUnit(String(curTime[0])), 85 - CLOCK_POS_OFFSET, 142); // center: (85, 136)
}

void displayTimeMinute() {
    GO.lcd.fillRect(125 - CLOCK_POS_OFFSET, 112, 70, 48, CLOCK_SCRN_COLOR);
    GO.lcd.drawString(addPaddingToTimeUnit(String(curTime[1])), 160 - CLOCK_POS_OFFSET, 142); // center: (160, 136)
}

void displayTimeSeparator() {
    GO.lcd.setTextSize(2);
    GO.lcd.setTextColor(CLOCK_TIME_COLOR);

    GO.lcd.fillRect(120 - CLOCK_POS_OFFSET, 112, 5, 48, CLOCK_SCRN_COLOR);
    GO.lcd.fillRect(195 - CLOCK_POS_OFFSET, 112, 5, 48, CLOCK_SCRN_COLOR);

    GO.lcd.drawString(":", 123 - CLOCK_POS_OFFSET, 139); // center: (123, 136)
    GO.lcd.drawString(":", 198 - CLOCK_POS_OFFSET, 139); // center: (198, 136)
}

void display24hIndicator() {
    GO.lcd.setTextSize(1);
    GO.lcd.setTextColor(CLOCK_24H_COLOR);

    GO.lcd.fillRect(270 - CLOCK_POS_OFFSET, 136, 50, 24, CLOCK_SCRN_COLOR);
    GO.lcd.drawString("24h", 295 - CLOCK_POS_OFFSET, 150); // center: (295, 148)
}

void displayAlarmIndicator(bool toggle) {
    GO.lcd.fillRect(270 - CLOCK_POS_OFFSET, 112, 50, 24, CLOCK_SCRN_COLOR);

    if (toggle) {
        GO.lcd.setTextSize(1);
        GO.lcd.setTextColor(ALARM_IND_COLOR);

        GO.lcd.drawString("ALM", 295 - CLOCK_POS_OFFSET, 126); // center: (295, 124)
    }
}

void displayAlarmDateTime(bool toggle) {
    GO.lcd.fillRect(0, 160, 320, 32, CLOCK_SCRN_COLOR);

    if (toggle) {
        uint8_t hour = rtc.getAlarmHour();
        uint8_t minute = rtc.getAlarmMinute();
        uint8_t weekday = rtc.getAlarmWeekday();

        GO.lcd.setTextSize(1);
        GO.lcd.setTextColor(ALARM_SET_COLOR);
        GO.lcd.drawString((hour > 23 ? "00" : addPaddingToTimeUnit(String(hour))) +
                          String(" : ") +
                          (minute > 59 ? "00" : addPaddingToTimeUnit(String(minute))) +
                          String(", ") +
                          DAYS_OF_THE_WEEK[weekday],
                          160, 176);
    }
}

void displayAlarmSetClockBlinkingHandler() {
    if (alarmSetCursor != CURSOR_NONE) {
        if (colorToggleIndicator)
            GO.lcd.setTextColor(CLOCK_SCRN_COLOR);
        else
            GO.lcd.setTextColor(ALARM_SET_COLOR);
        colorToggleIndicator = !colorToggleIndicator;

        switch (alarmSetCursor) {
            case CURSOR_HOUR:
                GO.lcd.setTextSize(2);
                displayTimeHour();
                break;
            case CURSOR_MINUTE:
                GO.lcd.setTextSize(2);
                displayTimeMinute();
                break;
            case CURSOR_WEEKDAY:
                GO.lcd.setTextSize(1);
                displayWeekday();
                break;
        }
    }
}

void enableAlarm() {
    Serial.println("Alarm: enabled");
    if (alarmTime[0] == ALARM_IGNORE && alarmTime[1]  == ALARM_IGNORE) {
        // If an user activates alarm for a first time
        setAlarm(curTime[0], curTime[1], curWeekday);
    } else {
        rtc.enableAlarm();
    }
}

void disableAlarm() {
    Serial.println("Alarm: disabled");
    rtc.clearAlarm();
}

void setAlarm(int hour, int min, int weekday) {
    Serial.println("Alarm: start setting up");
    Serial.printf("Alarm: %d h %d m, ", alarmTime[0], alarmTime[1]);
    Serial.println(alarmWeekday >= ARR_LENGTH(DAYS_OF_THE_WEEK) ? "unavailable" : DAYS_OF_THE_WEEK[alarmWeekday]);
    Serial.println("Alarm: displayUpdateTimer disabled");
    Serial.println("Alarm: displayAlarmSetClockBlinkingTimer enabled");
    timer.disable(displayUpdateTimer);
    timer.enable(displayAlarmSetClockBlinkingTimer);

    curTime[0] = alarmTime[0] = hour;
    curTime[1] = alarmTime[1] = min;
    curTime[2] = 0;
    curWeekday = alarmWeekday = weekday;
    displayUpdateTime(UPDATE_SECOND);

    GO.lcd.setTextColor(ALARM_SET_COLOR);
    GO.lcd.setTextSize(2);
    displayTimeHour();
    displayTimeMinute();
    GO.lcd.setTextSize(1);
    displayWeekday();

    alarmSetCursor = CURSOR_HOUR;

    while (!rtc.alarmEnabled()) {
        timer.run();
        buttonEventHandler();
    }

    curTime[0] = rtc.getHour();
    curTime[1] = rtc.getMinute();
    curTime[2] = rtc.getSecond();
    curWeekday = rtc.getWeekday();
    displayUpdateTime(UPDATE_HOUR);
    displayUpdateDate();
    alarmSetCursor = CURSOR_NONE;

    timer.disable(displayAlarmSetClockBlinkingTimer);
    timer.enable(displayUpdateTimer);
    Serial.println("Alarm: displayAlarmSetClockBlinkingTimer disabled");
    Serial.println("Alarm: displayUpdateTimer enabled");
}

void alarmIntHandler() {
    Serial.println("Alarm: alarm interrupted");
    Serial.println("Alarm: alarmIntSoundTimer enabled");

    curAlarmSoundCount = 0;
    timer.enable(alarmIntSoundTimer);
}

void alarmIntSoundHandler() {
    Serial.println("Alarm: start alarm sound");
    // de-mute speaker
    pinMode(PIN_DAC_SD, OUTPUT);
    digitalWrite(PIN_DAC_SD, HIGH);

    GO.Speaker.tone(ALARM_RING_FREQ_HZ, ALARM_RING_DUR);

    curAlarmSoundCount++;
    Serial.printf("Alarm: ring count %3d \n", curAlarmSoundCount);
    if (curAlarmSoundCount == ALARM_RING_COUNT + 1)
        alarmSoundStop();
}

void alarmSoundStop() {
    Serial.println("Alarm: alarmIntSoundTimer disabled");
    timer.disable(alarmIntSoundTimer);

    // mute speaker
    pinMode(PIN_DAC_SD, OUTPUT);
    digitalWrite(PIN_DAC_SD, LOW);
}

void buttonEventHandler() {
    GO.update();

    if (timer.isEnabled(displayUpdateTimer)) {
        // If in idle
        if (GO.BtnMenu.wasPressed()) {
            if (rtc.alarmEnabled()) {
                disableAlarm();
                displayAlarmIndicator(false);
                displayAlarmDateTime(false);
            } else {
                enableAlarm();
                displayAlarmIndicator(true);
                displayAlarmDateTime(true);
            }
        }

        if (GO.BtnSelect.pressedFor(2000)) {
            if (rtc.alarmEnabled()) {
                disableAlarm();
                setAlarm(alarmTime[0], alarmTime[1], alarmWeekday);
            }
        }
    } else {
        // If an user is setting up for the alarm
        if (GO.BtnA.wasPressed()) {
            // min, hour, day, weekday
            rtc.setAlarm(alarmTime[1], alarmTime[0], ALARM_IGNORE, alarmWeekday);
            displayAlarmDateTime(true);

            Serial.println("Alarm: alarm set");
            Serial.printf("Alarm: %d h %d m, ", alarmTime[0], alarmTime[1]);
            Serial.println(alarmWeekday >= ARR_LENGTH(DAYS_OF_THE_WEEK) ? "unavailable" : DAYS_OF_THE_WEEK[alarmWeekday]);
        }

        if (GO.JOY_Y.wasAxisPressed() == 2) {
            // Dpad UP
            alarmSetTimeHelper(DIRECTION_UP);
        }

        if (GO.JOY_Y.wasAxisPressed() == 1) {
            // Dpad DOWN
            alarmSetTimeHelper(DIRECTION_DOWN);
        }

        if (GO.JOY_X.wasAxisPressed() == 2) {
            // Dpad LEFT
            alarmSetCursorHelper(DIRECTION_LEFT);
        }

        if (GO.JOY_X.wasAxisPressed() == 1) {
            // Dpad RIGHT
            alarmSetCursorHelper(DIRECTION_RIGHT);
        }
    }

    if (timer.isEnabled(alarmIntSoundTimer)) {
        // If alarm is ringing
        if (GO.BtnA.wasPressed() || GO.BtnB.wasPressed()) {
            alarmSoundStop();
        }
    }
}

void alarmSetCursorHelper(int direction) {
    timer.disable(displayAlarmSetClockBlinkingTimer);

    GO.lcd.setTextColor(ALARM_SET_COLOR);
    switch (alarmSetCursor) {
        case CURSOR_HOUR:
        case CURSOR_MINUTE:
            GO.lcd.setTextSize(2);
            displayTimeHour();
            displayTimeMinute();
            break;
        case CURSOR_WEEKDAY:
            GO.lcd.setTextSize(1);
            displayWeekday();
            break;
        default:
            break;
    }

    switch (direction) {
        case DIRECTION_LEFT:
            if (alarmSetCursor == CURSOR_HOUR)
                alarmSetCursor = CURSOR_WEEKDAY;
            else
                alarmSetCursor--;
            break;
        case DIRECTION_RIGHT:
            if (alarmSetCursor == CURSOR_WEEKDAY)
                alarmSetCursor = CURSOR_HOUR;
            else
                alarmSetCursor++;
            break;
        default:
            break;
    }

    timer.enable(displayAlarmSetClockBlinkingTimer);
}

void alarmSetTimeHelper(int direction) {
    switch (alarmSetCursor) {
        case CURSOR_HOUR: {
                switch (direction) {
                    case DIRECTION_UP:
                        if (curTime[0] == 23)
                            curTime[0] = 0;
                        else
                            curTime[0]++;
                        break;
                    case DIRECTION_DOWN:
                        if (curTime[0] == 0)
                            curTime[0] = 23;
                        else
                            curTime[0]--;
                        break;
                    default:
                        break;
                }

                alarmTime[0] = curTime[0];
            }
            break;
        case CURSOR_MINUTE: {
                switch (direction) {
                    case DIRECTION_UP:
                        if (curTime[1] == 59)
                            curTime[1] = 0;
                        else
                            curTime[1]++;
                        break;
                    case DIRECTION_DOWN:
                        if (curTime[1] == 0)
                            curTime[1] = 59;
                        else
                            curTime[1]--;
                        break;
                    default:
                        break;
                }

                alarmTime[1] = curTime[1];
            }
            break;
        case CURSOR_WEEKDAY: {
                switch (direction) {
                    case DIRECTION_UP:
                        if (curWeekday == ARR_LENGTH(DAYS_OF_THE_WEEK) - 1)
                            curWeekday = 0;
                        else
                            curWeekday++;
                        break;
                    case DIRECTION_DOWN:
                        if (curWeekday == 0)
                            curWeekday = ARR_LENGTH(DAYS_OF_THE_WEEK) - 1;
                        else
                            curWeekday--;
                        break;
                    default:
                        break;
                }

                alarmWeekday = curWeekday;
            }
            break;
        default:
            break;
    }
}

String addPaddingToTimeUnit(String unit) {
    return unit.length() == 1 ? '0' + unit : unit;
}

void loop() {
    // put your main code here, to run repeatedly:
    timer.run();

    buttonEventHandler();
}

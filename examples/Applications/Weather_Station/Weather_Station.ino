#include <odroid_go.h>
#include "sensors/Wire.h"
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include "web/WebSocketsServer.h"
#include "web/WebServer.h"
#include "web/SimpleTimer.h"
#include "WeatherBoard.h"

const double FW_VERSION = 1.0;
const String CHIP_ID = String((uint16_t)(ESP.getEfuseMac() >> 32), HEX);

const int PIN_STATUS_LED = 2;
const int PIN_DAC_SD = 25;
const int PIN_I2C_SDA = 15;
const int PIN_I2C_SCL = 4;

const char SGN_SAVE_NET_CONFIGS = 'n';
const char SGN_PAGE_STATE = 'p';
const char SGN_DATA_PVI = 'd';
const char SGN_FW_VERSION = 'f';

const char *DEFAULT_IP = "192.168.4.1";
const char *DEFAULT_AP_SSID_PREFIX = "ODROID_GO_";
const char *DEFAULT_AP_PASSWD = "12345678";

const int DEFAULT_WEBSERVER_PORT = 80;
const int DEFAULT_WEBSOCKET_PORT = 81;

const char *CONFIGS_PATH = "/go_setup/network_configs";

char apSsid[20];
char apPasswd[20];
IPAddress ip;

WebServer *webServer;
WebSocketsServer webSocket = WebSocketsServer(DEFAULT_WEBSOCKET_PORT);
SimpleTimer timer;
WeatherBoard weatherBoard;

uint8_t statusLed = LOW;
bool isSocketConnected = false;
uint16_t backgroundColor;

struct socketClients {
    bool isConnected;
    uint8_t page;
} socketClients[5];

void displayLabels() {
    GO.lcd.setCharCursor(0, 1);
    GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
    GO.lcd.println("Si1132");
    GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
    GO.lcd.println("UV Index : ");
    GO.lcd.println("Visible :");
    GO.lcd.println("IR :");

    if (weatherBoard.revision == 1) {
        GO.lcd.setCharCursor(0, 6);
        GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
        GO.lcd.println("Si7020");
        GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
        GO.lcd.println("Temp : ");
        GO.lcd.println("Humidity :");
        GO.lcd.setCharCursor(0, 10);
        GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
        GO.lcd.println("BMP180");
        GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
        GO.lcd.println("Temp : ");
        GO.lcd.println("Pressure :");
        GO.lcd.println("Altitude :");
    } else if (weatherBoard.revision == 2) {
        GO.lcd.setCharCursor(0, 6);
        GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
        GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
        GO.lcd.println("BME280");
        GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
        GO.lcd.println("Temp : ");
        GO.lcd.println("Pressure :");
        GO.lcd.println("Humidity :");
        GO.lcd.println("Altitude :");
    }
}

void displayBMP180() {
    GO.lcd.setTextColor(TFT_CYAN, backgroundColor);
    GO.lcd.setCharCursor(7, 11);
    GO.lcd.print(weatherBoard.BMP180Temperature);
    GO.lcd.println(" *C  ");
    delay(20);

    GO.lcd.setCharCursor(11, 12);
    GO.lcd.print(weatherBoard.BMP180Pressure);
    GO.lcd.println(" Pa  ");
    delay(20);

    GO.lcd.setCharCursor(11, 13);
    GO.lcd.print(weatherBoard.BMP180Altitude);
    GO.lcd.println(" meters   ");
    delay(20);
}

void displaySi7020() {
    GO.lcd.setTextColor(TFT_YELLOW, backgroundColor);
    GO.lcd.setCharCursor(7, 7);
    GO.lcd.print(weatherBoard.Si7020Temperature);
    GO.lcd.println(" *C  ");
    delay(20);

    GO.lcd.setCharCursor(11, 8);
    GO.lcd.print(weatherBoard.Si7020Humidity);
    GO.lcd.println(" %  \n");
    delay(20);
}

void displaySi1132() {
    GO.lcd.setTextColor(TFT_RED, backgroundColor);
    GO.lcd.setCharCursor(11, 2);
    GO.lcd.print(weatherBoard.Si1132UVIndex);
    GO.lcd.println("  ");
    delay(20);

    GO.lcd.setCharCursor(10, 3);
    GO.lcd.print(weatherBoard.Si1132Visible);
    GO.lcd.println(" Lux  ");
    delay(20);

    GO.lcd.setCharCursor(5, 4);
    GO.lcd.print(weatherBoard.Si1132IR);
    GO.lcd.println(" Lux  \n");
    delay(20);
}

void displayBME280() {
    GO.lcd.setTextColor(TFT_CYAN, backgroundColor);
    GO.lcd.setCharCursor(7, 7);
    GO.lcd.print(weatherBoard.BME280Temperature);
    GO.lcd.println(" *C  ");
    delay(20);

    GO.lcd.setCharCursor(11, 8);
    GO.lcd.print(weatherBoard.BME280Pressure);
    GO.lcd.println(" hPa  ");
    delay(20);

    GO.lcd.setCharCursor(11, 9);
    GO.lcd.print(weatherBoard.BME280Humidity);
    GO.lcd.println(" %   ");
    delay(20);

    GO.lcd.setCharCursor(11, 10);
    GO.lcd.print(weatherBoard.BME280Altitude);
    GO.lcd.println(" m   ");
    delay(20);
}

void setup() {
    GO.begin();
    GO.lcd.setTextSize(2);

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    pinMode(PIN_DAC_SD, OUTPUT);
    digitalWrite(PIN_DAC_SD, LOW);

    Serial.print("WeatherBoard Revision: ");
    Serial.println(weatherBoard.begin());

    for (uint8_t t = 3; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
        Serial.flush();
        delay(1000);
    }

    if (SPIFFS.begin() && listSPIFFS(SPIFFS, "/", 0)) {
        readNetworkConfigs();

        initWifi();
        initWebServer();
        initWebSocket();

        timer.setInterval(1000, weatherStationMainHandler);
        displayLabels();
    } else {
        Serial.println("SPIFFS mount failed.");
    }
}

bool writeDefaultNetworkConfigs() {
    File f = SPIFFS.open(CONFIGS_PATH, "w");

    if (f) {
        f.println("ipaddr=\"" + String(DEFAULT_IP) + "\"");
        f.println("ssid=\"" + String(DEFAULT_AP_SSID_PREFIX) + CHIP_ID + "\"");
        f.println("passwd=\"" + String(DEFAULT_AP_PASSWD) + "\"");

        f.flush();
        f.close();

        return true;
    } else {
        return false;
    }
}

bool writeNetworkConfigs(String ip, String ssid, String passwd) {
    File f = SPIFFS.open(CONFIGS_PATH, "w");

    if (f) {
        f.println("ipaddr=\"" + ip + "\"");
        f.println("ssid=\"" + ssid + "\"");
        f.println("passwd=\"" + passwd + "\"");

        f.flush();
        f.close();

        return true;
    } else {
        return false;
    }
}

void readNetworkConfigs() {
    Serial.println("/* Network Configuration */");

    if (!SPIFFS.exists(CONFIGS_PATH)) {
        Serial.println("Configuration file not found. Trying to create a new file.");

        if (writeDefaultNetworkConfigs() && SPIFFS.exists(CONFIGS_PATH)) {
            Serial.println("Configuration file created.");
        } else {
            Serial.println("File creation failed.");
            return;
        }
    }

    File f = SPIFFS.open(CONFIGS_PATH, "r");
    int readIp[4];

    if (!f) {
        Serial.println("File open failed.");

        return;
    }

    f.findUntil("ipaddr", "\r\n");
    f.seek(2, SeekCur);
    readIp[0] = f.readStringUntil('.').toInt();
    readIp[1] = f.readStringUntil('.').toInt();
    readIp[2] = f.readStringUntil('.').toInt();
    readIp[3] = f.readStringUntil('"').toInt();
    ip = IPAddress(readIp[0], readIp[1], readIp[2], readIp[3]);

    f.findUntil("ssid", "\r\n");
    f.seek(6, SeekCur);
    f.readStringUntil('"').toCharArray(apSsid, 20);

    f.findUntil("passwd", "\r\n");
    f.seek(8, SeekCur);
    f.readStringUntil('"').toCharArray(apPasswd, 20);
    f.close();

    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("SSID: ");
    Serial.println(apSsid);
    Serial.print("Password: ");
    Serial.println(apPasswd);
}

void initWifi() {
    Serial.println("/* Initializing WiFi */");

    IPAddress gateway(ip[0], ip[1], ip[2], 1);
    IPAddress subnet(255, 255, 255, 0);

    if (WiFi.softAPConfig(ip, gateway, subnet) && WiFi.softAP(apSsid, apPasswd)) {
        Serial.println("WiFi initilized.");
    } else {
        Serial.println("WiFi initilizing failed.");
    }
}

void initWebServer() {
    Serial.println("/* Initializing Web Server */");

    webServer = new WebServer(DEFAULT_WEBSERVER_PORT);
    webServer->on("/list", HTTP_GET, webServerFileListHandler);
    webServer->onNotFound([]() {
        if (!webServerFileReadHandler(webServer->uri()))
            webServer->send(404, "text/plain", "FileNotFound");
    });
    webServer->begin();

    Serial.println("Web server initilized.");
}

void webServerFileListHandler() {
    if (!webServer->hasArg("dir")) {
        webServerReturnFail("BAD ARGS");
        return;
    }
    String path = webServer->arg("dir");
    if (path != "/" && !SPIFFS.exists((char *)path.c_str())) {
        webServerReturnFail("BAD PATH");
        return;
    }
    File dir = SPIFFS.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory()) {
        dir.close();
        webServerReturnFail("NOT DIR");
        return;
    }
    dir.rewindDirectory();

    String output = "[";
    for (int cnt = 0; true; ++cnt) {
        File entry = dir.openNextFile();
        if (!entry)
            break;

        if (cnt > 0)
            output += ',';

        output += "{\"type\":\"";
        output += (entry.isDirectory()) ? "dir" : "file";
        output += "\",\"name\":\"";
        // Ignore '/' prefix
        output += entry.name() + 1;
        output += "\"";
        output += "}";
        entry.close();
    }
    output += "]";
    webServer->send(200, "text/json", output);
    dir.close();
}

bool webServerFileReadHandler(String path) {
    if (path.endsWith("/")) path += "index.html";

    String contentType = webServerGetContentType(path);
    String pathWithGz = path + ".gz";

    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
        if (SPIFFS.exists(pathWithGz))
            path += ".gz";

        File file = SPIFFS.open(path, "r");
        size_t sent = webServer->streamFile(file, contentType);
        file.close();

        return true;
    }

    return false;
}

String webServerGetContentType(String filename) {
    if (webServer->hasArg("download")) return "application/octet-stream";
    else if (filename.endsWith(".htm")) return "text/html";
    else if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".xml")) return "text/xml";
    else if (filename.endsWith(".pdf")) return "application/x-pdf";
    else if (filename.endsWith(".zip")) return "application/x-zip";
    else if (filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}

void webServerReturnFail(String msg) {
    webServer->send(500, "text/plain", msg + "\r\n");
}

void initWebSocket() {
    Serial.println("/* Initializing Web Sockets */");

    webSocket.begin();
    webSocket.onEvent(webSocketHandler);

    Serial.println("Web sockets initilized.");
}

void webSocketHandler(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("WebSocket: Disconnected: %u \r\n", num);
            socketClients[num].isConnected = false;
            break;
        case WStype_CONNECTED:
            Serial.printf("WebSocket: Connected: %u from %d.%d.%d.%d: url: %s \r\n",
                          num, ip[0], ip[1], ip[2], ip[3], payload);
            socketClients[num].isConnected = true;
            break;
        case WStype_TEXT: {
                String data = String((char *) &payload[0]);
                switch (data.charAt(0)) {
                    case SGN_PAGE_STATE: {
                        socketClients[num].page = data.charAt(1) - 48;

                        if (socketClients[num].page == 0) {
                            webSocket.sendTXT(num, String(SGN_FW_VERSION) + FW_VERSION);
                        } else if (socketClients[num].page == 1) {
                            webSocket.sendTXT(num,
                                              String(SGN_SAVE_NET_CONFIGS)
                                              + String(apSsid) + ","
                                              + ip.toString() + ","
                                              + String(apPasswd));
                        }
                        break;
                    }
                    case SGN_SAVE_NET_CONFIGS: {
                        String _ssid = data.substring(1, data.indexOf(','));
                        data.remove(1, data.indexOf(','));
                        String _ip = data.substring(1, data.indexOf(','));
                        data.remove(1, data.indexOf(','));
                        String _passwd = data.substring(1, data.indexOf(','));

                        if ((String(DEFAULT_IP) != _ip) ||
                            (String(DEFAULT_AP_SSID_PREFIX) + CHIP_ID != _ssid) ||
                            (String(DEFAULT_AP_PASSWD) != _passwd)) {

                            if (writeNetworkConfigs(_ip, _ssid, _passwd)) {
                                Serial.println("WebSocket: Saving a new network configuration successed.");
                                reboot();
                            } else {
                                Serial.println("WebSocket: Saving a new network configuration failed.");
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
        case WStype_BIN:
            break;
        default:
            break;
    }
}

bool listSPIFFS(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.println("/* Initializing SPIFFS */");
    Serial.printf("dirname: %s \r\n", dirname);
    Serial.printf("levels: %d \r\n", levels);

    File root = fs.open(dirname);

    if (!root) {
        Serial.println("Failed to open directory.");
        return false;
    }

    if (!root.isDirectory()) {
        Serial.println("Not a directory.");
        return false;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listSPIFFS(fs, file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }

    return true;
}

void checkSocketConnected() {
    for (int i = 0; i < 5; i++) {
        if (socketClients[i].isConnected) {
            isSocketConnected = true;
            return;
        }
    }

    isSocketConnected = false;
}

void reboot() {
    Serial.println("ODROID GO will reboot.");
    ESP.restart();
}

void weatherStationMainHandler() {
    if (WiFi.softAPgetStationNum() > 0) {
        if (isSocketConnected)
            statusLed = !statusLed;
        else
            statusLed = HIGH;
    } else
        statusLed = LOW;
    digitalWrite(PIN_STATUS_LED, statusLed);

    weatherBoard.getSi1132();
    if (weatherBoard.revision == 1) {
        weatherBoard.getBMP180();
        weatherBoard.getSi7020();
    } else if (weatherBoard.revision == 2) {
        weatherBoard.getBME280();
    }

    displaySi1132();
    if (weatherBoard.revision == 1) {
        displayBMP180();
        displaySi7020();
    } else if (weatherBoard.revision == 2) {
        displayBME280();
    }

    if (isSocketConnected) {
        String data = String(SGN_DATA_PVI);
        data += String(weatherBoard.BME280Temperature, 2);
        data += "," + String(weatherBoard.BME280Pressure, 1);
        data += "," + String(weatherBoard.BME280Humidity, 2);
        data += "," + String(weatherBoard.BME280Altitude, 2);
        data += "," + String(weatherBoard.Si1132UVIndex, 2);
        data += "," + String(weatherBoard.Si1132Visible);
        data += "," + String(weatherBoard.Si1132IR);

        for (int i = 0; i < 5; i++) {
            if (socketClients[i].isConnected && (socketClients[i].page == 0))
                webSocket.sendTXT(i, data);
        }
    }
}

void loop() {
    checkSocketConnected();

    webServer->handleClient();
    webSocket.loop();

    timer.run();
}

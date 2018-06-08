#include <odroid_go.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SimpleTimer.h>
#include "Weather_Board.h"
#include <Wire.h>

// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)

#define DBG_OUTPUT_PORT Serial
#define FwVersion   1.0

File fsUploadFile;

char ssid[20];
char password[20];
WebServer *server;
WebSocketsServer webSocket = WebSocketsServer(81);
IPAddress ip = IPAddress(192, 168, 4, 1);

#define SET_DEFAULT_VOLTAGE 'v'
#define SET_VOLTAGE         'w'
#define SAVE_NETWORKS       'n'
#define CMD_ONOFF           'o'
#define SET_AUTORUN         'a'
#define PAGE_STATE          'p'
#define DATA_PVI            'd'
#define MEASUREWATTHOUR     'm'
#define FW_VERSION          'f'

#define LED_BLUE 2
uint8_t onoff;

uint8_t connectedWeb;

uint8_t ledPin = 5;
uint8_t pwm = 255;
uint8_t textSize = 2;
uint8_t rotation = 1;

float battery = 0;
float oldBattery;
uint8_t batteryCnt;
uint8_t batteryState;
uint8_t count;

uint16_t foregroundColor, backgroundColor;

unsigned char errorState;
unsigned int ipaddr[4];

SimpleTimer timer;
Weather_Board wb;

struct client_go {
    uint8_t connected;
    uint8_t page;
};

struct client_go client_go[3];

void wifi_connection_status(void)
{
    if (WiFi.softAPgetStationNum() > 0) {
        if (connectedWeb) {
            onoff = !onoff;
        } else {
            onoff = HIGH;
        }
    } else {
        onoff = LOW;
    }
    digitalWrite(LED_BLUE, onoff);
}

void setup()
{
    GO.begin();
    GO.lcd.setTextSize(2);
    Serial.print("revision :");
    Serial.println(wb.begin());
    

    Wire.begin(15, 4);

    for (uint8_t t = 3; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n\r", t);
        Serial.flush();
        delay(1000);
    }

    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_BLUE, LOW);
    pinMode(25, OUTPUT);
    digitalWrite(25, LOW);

    SPIFFS.begin();
    {
        listDir(SPIFFS, "/", 0);
    }

    
    initODROID_GO();
    
    webserver_init();

    timer.setInterval(1000, handler);
    displayLabel();
}

void handler(void)
{
    wifi_connection_status();
    if (connectedWeb) {
        String data = String(DATA_PVI);
        data += String(wb.BME280Temperature, 3) + "," + String(wb.BME280Pressure, 3) + "," + String(wb.BME280Humidity, 3) + "," + String(wb.BME280Altitude, 3);
//        send_data_to_clients(data, 0);
        Serial.println(data);
    }
    //available_sensors();
}

bool available_sensors()
{
    byte count = 0;
    for (byte i = 1; i < 128; i++) {
        Wire.beginTransmission(i);
        if (Wire.endTransmission() == 0)
        {
            Serial.print ("Found address: ");
             Serial.print (i, DEC);
             Serial.print (" (0x");
             Serial.print (i, HEX);
             Serial.println (")");
             count++;
        }
        delay(5);
    }
    if (count > 0) {
        Serial.println("I2C available!");
    } else {
        Serial.println("I2C not available!");
    }
}

void loop(void)
{
    
    server->handleClient();
    
    webSocket.loop();
    timer.run();
/*
    GO.update();

    displaySi1132();
    
    if (wb.revision == 1) {
        displayBMP180();
        displaySi7020();
    } else if (wb.revision == 2) {
        displayBME280();
    }

    wb.getSi1132();
    if (wb.revision == 1) {
        wb.getBMP180();
        wb.getSi7020();
    } else if(wb.revision == 2) {
        wb.getBME280();
    }
    */
}

void initODROID_GO(void)
{
    if (isFirstBoot())
    {
        Serial.println("init fs!!");
        fs_init();
    }
    else
    {
        readNetworkConfig();
    }
}

void fs_init(void)
{
    uint64_t chipid;
    chipid = ESP.getEfuseMac();
    String ssidTemp = "ssid=\"ODROID_GO_" + String((uint16_t)(chipid>>32), HEX) + "\"";

    File f = SPIFFS.open("/js/settings.js", "w");
    f.println(ssidTemp.c_str());
    f.println("ipaddr=\"192.168.4.1\"");
    f.println("passwd=\"12345678\"");
    f.flush();
    f.close();
    readNetworkConfig();
}

void handleClientData(uint8_t num, String data)
{
    Serial.println(data);
    switch (data.charAt(0)) {
        case PAGE_STATE:
            client_go[num].page = data.charAt(1) -48;
            sendStatus(num, data.charAt(1) -48);
            break;
    }
}

bool isFirstBoot()
{
    File f = SPIFFS.open("/txt/settings.txt", "r");

    f.seek(0, SeekSet);
    f.findUntil("firstboot", "\n\r");
    f.seek(1, SeekCur);
    int value = f.readStringUntil('\n').toInt();

    bool result;
    if (value)
    {
        result = true;
    }
    else
    {
        result = false;
    }

    f.close();

    return result;
}

void readNetworkConfig(void)
{
    File f = SPIFFS.open("/js/settings.js", "r");

    f.findUntil("ssid", "\n\r");

    f.seek(2, SeekCur);
    f.readStringUntil('"').toCharArray(ssid, 20);
    Serial.print("ssid ? :");
    Serial.println(ssid);

    f.findUntil("ipaddr", "\n\r");
    f.seek(2, SeekCur);
    ipaddr[0] = f.readStringUntil('.').toInt();
    ipaddr[1] = f.readStringUntil('.').toInt();
    ipaddr[2] = f.readStringUntil('.').toInt();
    ipaddr[3] = f.readStringUntil('"').toInt();
    //ip = IPAddress(ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
    ip = IPAddress(192, 168, 4, 1);
    server = new WebServer(80);

    f.findUntil("passwd", "\n\r");
    f.seek(2, SeekCur);
    f.readStringUntil('"').toCharArray(password, 20);
    f.close();
}

void webserver_init(void)
{
    IPAddress gateway(ip[0], ip[1], ip[2], ip[3]);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gateway, subnet);
    
    WiFi.softAP(ssid, password);
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.softAPIP());

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    server->on("/list", HTTP_GET, handleFileList);
    
    server->onNotFound([](){
        if(!handleFileRead(server->uri()))
            server->send(404, "text/plain", "FileNotFound");
    });
    server->begin();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
    switch (type) {
        case WStype_DISCONNECTED :
            Serial.printf("[%u] Disconnected!\n\r", num);
            client_go[num].connected = 0;
            connectedWeb = 0;
            break;
        case WStype_CONNECTED : {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n\r",
                                  num, ip[0], ip[1], ip[2], ip[3], payload);
                client_go[num].connected = 1;
                connectedWeb = 1;
                break;
            }
        case WStype_TEXT : {
                handleClientData(num, String((char *)&payload[0]));
                break;
            }
        case WStype_BIN :
            Serial.printf("[%u] get binary lenght: %u\n\r", num, lenght);
//            hexdump(payload, lenght);
            break;
    }
}

void sendStatus(uint8_t num, uint8_t page)
{
    if (!page) {
        webSocket.sendTXT(num, String(FW_VERSION) + FwVersion);
    } else if (page) {
        webSocket.sendTXT(num, String(SAVE_NETWORKS) + String(ssid) + "," + ip.toString() + "," + String(password));
    }
}

void send_data_to_clients(String str, uint8_t page)
{
    for (int i = 0; i < 5; i++) {
        if (client_go[i].connected && (client_go[i].page == page))
            webSocket.sendTXT(i, str);
    }
}

void displayLabel()
{
    GO.lcd.setCharCursor(0, 1);
    GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
    GO.lcd.println("Si1132");
    GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
    GO.lcd.println("UV Index : ");
    GO.lcd.println("Visible :");
    GO.lcd.println("IR :");

    if (wb.revision == 1) {
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
    } else if (wb.revision == 2) {
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

void displayBMP180()
{
    GO.lcd.setTextColor(TFT_CYAN, backgroundColor);
    GO.lcd.setCharCursor(7, 11);
    GO.lcd.print(wb.BMP180Temperature);
    GO.lcd.println(" *C  ");
    delay(20);

    GO.lcd.setCharCursor(11, 12);
    GO.lcd.print(wb.BMP180Pressure);
    GO.lcd.println(" Pa  ");
    delay(20);

    GO.lcd.setCharCursor(11, 13);
    GO.lcd.print(wb.BMP180Altitude);
    GO.lcd.println(" meters   ");
    delay(20);
}

void displaySi7020()
{
    GO.lcd.setTextColor(TFT_YELLOW, backgroundColor);
    GO.lcd.setCharCursor(7, 7);
    GO.lcd.print(wb.Si7020Temperature);
    GO.lcd.println(" *C  ");
    delay(20);

    GO.lcd.setCharCursor(11, 8);
    GO.lcd.print(wb.Si7020Humidity);
    GO.lcd.println(" %  \n");
    delay(20);
}

void displaySi1132()
{
    GO.lcd.setTextColor(TFT_RED, backgroundColor);
    GO.lcd.setCharCursor(11, 2);
    GO.lcd.print(wb.Si1132UVIndex);
    GO.lcd.println("  ");
    delay(20);

    GO.lcd.setCharCursor(10, 3);
    GO.lcd.print(wb.Si1132Visible);
    GO.lcd.println(" Lux  ");
    delay(20);

    GO.lcd.setCharCursor(5, 4);
    GO.lcd.print(wb.Si1132IR);
    GO.lcd.println(" Lux  \n");
    delay(20);
}

void displayBME280()
{
    GO.lcd.setTextColor(TFT_CYAN, backgroundColor);
    GO.lcd.setCharCursor(7, 7);
    GO.lcd.print(wb.BME280Temperature);
    GO.lcd.println(" *C  ");
    delay(20);

    GO.lcd.setCharCursor(11, 8);
    GO.lcd.print(wb.BME280Pressure);
    GO.lcd.println(" hPa  ");
    delay(20);

    GO.lcd.setCharCursor(11, 9);
    GO.lcd.print(wb.BME280Humidity);
    GO.lcd.println(" %   ");
    delay(20);

    GO.lcd.setCharCursor(11, 10);
    GO.lcd.print(wb.BME280Altitude);
    GO.lcd.println(" m   ");
    delay(20);
}

String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server->hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server->streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(server->uri() != "/edit") return;
  HTTPUpload& upload = server->upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void returnFail(String msg) {
  server->send(500, "text/plain", msg + "\r\n");
}

void handleFileList() {
  if(!server->hasArg("dir")) {
    returnFail("BAD ARGS");
    return;
  }
  String path = server->arg("dir");
  if(path != "/" && !SPIFFS.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  File dir = SPIFFS.open((char *)path.c_str());
  path = String();
  if(!dir.isDirectory()){
    dir.close();
    returnFail("NOT DIR");
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
    output += entry.name()+1;
    output += "\"";
    output += "}";
    entry.close();
  }
  output += "]";
  server->send(200, "text/json", output);
  dir.close();
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  DBG_OUTPUT_PORT.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    DBG_OUTPUT_PORT.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    DBG_OUTPUT_PORT.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      DBG_OUTPUT_PORT.print("  DIR : ");
      DBG_OUTPUT_PORT.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      DBG_OUTPUT_PORT.print("  FILE: ");
      DBG_OUTPUT_PORT.print(file.name());
      DBG_OUTPUT_PORT.print("  SIZE: ");
      DBG_OUTPUT_PORT.println(file.size());
    }
    file = root.openNextFile();
  }
}
